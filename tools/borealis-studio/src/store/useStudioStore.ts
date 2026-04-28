import { create } from "zustand";
import {
  createBorealisProject,
  getEntityTree,
  isTauriRuntime,
  loadStudioHome,
  loadStudioSnapshot,
  pauseRuntimePreview,
  pollRuntimePreview,
  reparentEntity as reparentEntityCommand,
  startRuntimePreview,
  stopRuntimePreview,
  writeSceneDocument,
  writeScriptDocument,
} from "../backend";
import { configureBorealisCatalog } from "../borealisCatalog";
import { createMockHome, createMockSnapshot } from "../mockData";
import type {
  AssetDragState,
  BottomTab,
  BridgeStatus,
  ConsoleLine,
  DockPanelId,
  DockRegionId,
  DragState,
  FlatTreeNode,
  LayoutState,
  NewProjectRequest,
  PreviewCommandResult,
  PreviewStatus,
  ProjectAsset,
  RecentProject,
  SceneDocument,
  SceneEntity,
  ScriptDocument,
  SelectionTarget,
  StudioHome,
  StudioMode,
  StudioPreferences,
  StudioSnapshot,
  Transform3D,
  ViewMode,
  ViewportPivotMode,
  ViewportSpaceMode,
  ViewportTab,
} from "../types";
import { entityScript } from "../utils/entity";
import { normalizePreviewStatus, previewEventToConsoleLine } from "../utils/preview";

const LAYOUT_STORAGE_PREFIX = "borealis-studio:layout:v2:";
const PREFERENCES_STORAGE_KEY = "borealis-studio:preferences:v1";
const RECENT_PROJECTS_STORAGE_KEY = "borealis-studio:recent-projects:v1";
const MAX_RECENT_PROJECTS = 8;
const DEFAULT_PREFERENCES: StudioPreferences = {
  gizmoSize: 0.75,
  gridColor: "#4b8dff",
  gridOpacity: 0.55,
  ptzSpeed: 1,
  shortcutTemplate: "blender",
  showGrid: true,
  snapMode: "grid-object",
  theme: "unity-light",
};
const DEFAULT_DOCK_TABS: Record<DockRegionId, DockPanelId[]> = {
  leftDock: ["hierarchy"],
  rightDock: ["inspector"],
  bottomDock: ["project", "console", "animation"],
};

function createDefaultLayout(): LayoutState {
  const viewportWidth = typeof window !== "undefined" ? window.innerWidth : 1440;
  const largeDesktop = viewportWidth >= 1600;
  const mediumDesktop = viewportWidth >= 1280;
  const leftWidth = largeDesktop ? 340 : mediumDesktop ? 280 : 260;
  const rightWidth = largeDesktop ? 380 : mediumDesktop ? 320 : 300;
  const bottomHeight = largeDesktop ? 260 : mediumDesktop ? 220 : 200;

  return {
    left: leftWidth,
    right: rightWidth,
    bottom: bottomHeight,
    docks: {
      leftRailWidth: 48,
      leftDock: { size: leftWidth, tabs: [...DEFAULT_DOCK_TABS.leftDock], activeTab: "hierarchy", collapsed: false },
      rightDock: { size: rightWidth, tabs: [...DEFAULT_DOCK_TABS.rightDock], activeTab: "inspector", collapsed: false },
      bottomDock: { size: bottomHeight, tabs: [...DEFAULT_DOCK_TABS.bottomDock], activeTab: "project", collapsed: false },
    },
  };
}

function syncLegacySizes(layout: LayoutState): LayoutState {
  return {
    ...layout,
    left: layout.docks.leftDock.size,
    right: layout.docks.rightDock.size,
    bottom: layout.docks.bottomDock.size,
  };
}

function normalizeLayout(raw: unknown): LayoutState {
  const base = createDefaultLayout();
  if (!raw || typeof raw !== "object") return base;
  const candidate = raw as Partial<LayoutState> & { docks?: Partial<LayoutState["docks"]> };

  const next: LayoutState = {
    ...base,
    left: typeof candidate.left === "number" ? candidate.left : base.left,
    right: typeof candidate.right === "number" ? candidate.right : base.right,
    bottom: typeof candidate.bottom === "number" ? candidate.bottom : base.bottom,
    docks: {
      leftRailWidth:
        typeof candidate.docks?.leftRailWidth === "number" ? candidate.docks.leftRailWidth : base.docks.leftRailWidth,
      leftDock: normalizeRegion(candidate.docks?.leftDock, base.docks.leftDock, DEFAULT_DOCK_TABS.leftDock),
      rightDock: normalizeRegion(candidate.docks?.rightDock, base.docks.rightDock, DEFAULT_DOCK_TABS.rightDock),
      bottomDock: normalizeRegion(candidate.docks?.bottomDock, base.docks.bottomDock, DEFAULT_DOCK_TABS.bottomDock),
    },
  };

  return syncLegacySizes(next);
}

function normalizeRegion(
  raw: unknown,
  fallback: LayoutState["docks"][DockRegionId],
  fallbackTabs: DockPanelId[],
): LayoutState["docks"][DockRegionId] {
  if (!raw || typeof raw !== "object") {
    return { ...fallback, tabs: [...fallback.tabs] };
  }

  const candidate = raw as Partial<LayoutState["docks"][DockRegionId]>;
  const tabs = Array.isArray(candidate.tabs)
    ? candidate.tabs.filter((tab): tab is DockPanelId => typeof tab === "string")
    : [...fallbackTabs];
  const activeTab = tabs.includes(candidate.activeTab as DockPanelId)
    ? (candidate.activeTab as DockPanelId)
    : tabs[0] ?? fallback.activeTab;

  return {
    size: typeof candidate.size === "number" ? candidate.size : fallback.size,
    tabs: tabs.length > 0 ? tabs : [...fallbackTabs],
    activeTab,
    collapsed: typeof candidate.collapsed === "boolean" ? candidate.collapsed : fallback.collapsed,
  };
}

function storageKey(projectPath?: string): string {
  return `${LAYOUT_STORAGE_PREFIX}${projectPath || "global"}`;
}

function readStoredLayout(projectPath?: string): LayoutState {
  if (typeof window === "undefined" || !window.localStorage) {
    return createDefaultLayout();
  }

  const raw = window.localStorage.getItem(storageKey(projectPath));
  if (!raw) {
    return createDefaultLayout();
  }

  try {
    return normalizeLayout(JSON.parse(raw));
  } catch {
    return createDefaultLayout();
  }
}

function persistLayout(projectPath: string | undefined, layout: LayoutState): void {
  if (typeof window === "undefined" || !window.localStorage) return;
  window.localStorage.setItem(storageKey(projectPath), JSON.stringify(syncLegacySizes(layout)));
}

function readStoredPreferences(): StudioPreferences {
  if (typeof window === "undefined" || !window.localStorage) return DEFAULT_PREFERENCES;
  const raw = window.localStorage.getItem(PREFERENCES_STORAGE_KEY);
  if (!raw) return DEFAULT_PREFERENCES;

  try {
    const parsed = JSON.parse(raw) as Partial<StudioPreferences>;
    return {
      ...DEFAULT_PREFERENCES,
      ...parsed,
      gridOpacity:
        typeof parsed.gridOpacity === "number" ? Math.min(1, Math.max(0.05, parsed.gridOpacity)) : DEFAULT_PREFERENCES.gridOpacity,
      gridColor: typeof parsed.gridColor === "string" ? parsed.gridColor : DEFAULT_PREFERENCES.gridColor,
      showGrid: typeof parsed.showGrid === "boolean" ? parsed.showGrid : DEFAULT_PREFERENCES.showGrid,
    };
  } catch {
    return DEFAULT_PREFERENCES;
  }
}

function persistPreferences(preferences: StudioPreferences): void {
  if (typeof window === "undefined" || !window.localStorage) return;
  window.localStorage.setItem(PREFERENCES_STORAGE_KEY, JSON.stringify(preferences));
}

function readStoredRecentProjects(): RecentProject[] {
  if (typeof window === "undefined" || !window.localStorage) return [];
  const raw = window.localStorage.getItem(RECENT_PROJECTS_STORAGE_KEY);
  if (!raw) return [];

  try {
    const parsed = JSON.parse(raw);
    if (!Array.isArray(parsed)) return [];
    return parsed
      .filter((item): item is RecentProject =>
        item &&
        typeof item.name === "string" &&
        typeof item.path === "string" &&
        typeof item.openedAt === "number",
      )
      .slice(0, MAX_RECENT_PROJECTS);
  } catch {
    return [];
  }
}

function persistRecentProjects(projects: RecentProject[]): void {
  if (typeof window === "undefined" || !window.localStorage) return;
  window.localStorage.setItem(RECENT_PROJECTS_STORAGE_KEY, JSON.stringify(projects.slice(0, MAX_RECENT_PROJECTS)));
}

function rememberRecentProject(current: RecentProject[], snapshot: StudioSnapshot): RecentProject[] {
  const projectPath = snapshot.projectPath.trim();
  if (!projectPath) return current;

  const nextProject: RecentProject = {
    name: snapshot.projectName.trim() || snapshot.scene.name || "Borealis Project",
    path: projectPath,
    openedAt: Date.now(),
  };
  const next = [nextProject, ...current.filter((project) => project.path !== projectPath)].slice(0, MAX_RECENT_PROJECTS);
  persistRecentProjects(next);
  return next;
}

function movablePanel(panelId: DockPanelId): boolean {
  return panelId !== "scene" && panelId !== "game";
}

function sceneRootSelection(): SelectionTarget {
  return { kind: "scene-root" };
}

function entitySelection(entityId: string): SelectionTarget {
  return { kind: "entity", entityId };
}

function noneSelection(): SelectionTarget {
  return { kind: "none" };
}

function selectedEntity(state: StudioState): SceneEntity | null {
  const entityId = state.selectionTarget.kind === "entity" ? state.selectionTarget.entityId : state.selectedEntityId;
  if (!entityId) return null;
  return state.snapshot.scene.entities.find((entity) => entity.id === entityId) ?? null;
}

function buildFlatEntityTree(entities: SceneEntity[]): FlatTreeNode[] {
  const entityIds = new Set(entities.map((entity) => entity.id));
  const byParent = new Map<string | undefined, SceneEntity[]>();
  for (const entity of entities) {
    const parentKey = normalizedParentId(entity.parent, entityIds, entity.id);
    const list = byParent.get(parentKey) ?? [];
    list.push(entity);
    byParent.set(parentKey, list);
  }

  for (const list of byParent.values()) {
    list.sort((a, b) => a.name.localeCompare(b.name));
  }

  const result: FlatTreeNode[] = [];
  const walk = (parentId: string | undefined, depth: number) => {
    const children = byParent.get(parentId) ?? [];
    children.forEach((entity) => {
      const nested = byParent.get(entity.id) ?? [];
      result.push({
        id: entity.id,
        name: entity.name,
        layer: entity.layer,
        depth,
        parentId: normalizedParentId(entity.parent, entityIds, entity.id),
        childCount: nested.length,
        hasChildren: nested.length > 0,
      });
      walk(entity.id, depth + 1);
    });
  };

  walk(undefined, 0);
  return result;
}

function normalizedParentId(parent: unknown, entityIds: Set<string>, entityId: string): string | undefined {
  if (typeof parent !== "string") return undefined;
  const normalized = parent.trim();
  if (!normalized || normalized === entityId || !entityIds.has(normalized)) return undefined;
  return normalized;
}

function expandedIdsFromTree(flatTree: FlatTreeNode[]): string[] {
  return flatTree.filter((node) => node.hasChildren).map((node) => node.id);
}

function updateEntityParentLocally(scene: SceneDocument, entityId: string, newParentId?: string): SceneDocument {
  const entityMap = new Map(scene.entities.map((entity) => [entity.id, entity]));
  if (!entityMap.has(entityId)) return scene;
  if (newParentId && !entityMap.has(newParentId)) return scene;
  if (newParentId === entityId) return scene;

  let cursor = newParentId;
  while (cursor) {
    if (cursor === entityId) return scene;
    cursor = entityMap.get(cursor)?.parent;
  }

  return {
    ...scene,
    entities: scene.entities.map((entity) =>
      entity.id === entityId
        ? {
            ...entity,
            parent: newParentId || undefined,
          }
        : entity,
    ),
  };
}

function counts(lines: ConsoleLine[]): { warnings: number; errors: number } {
  return lines.reduce(
    (acc, line) => {
      if (line.level === "warn") acc.warnings += 1;
      if (line.level === "error") acc.errors += 1;
      return acc;
    },
    { warnings: 0, errors: 0 },
  );
}

export interface StudioState {
  snapshot: StudioSnapshot;
  home: StudioHome;
  catalogVersion: number;
  homeVisible: boolean;
  homeBusy: boolean;
  homeError: string | null;
  recentProjects: RecentProject[];
  bridgeStatus: BridgeStatus;
  selectedEntityId: string;
  selectionTarget: SelectionTarget;
  selectedScriptPath: string;
  viewMode: ViewMode;
  viewportTab: ViewportTab;
  viewportPivotMode: ViewportPivotMode;
  viewportSpaceMode: ViewportSpaceMode;
  viewportGizmos: boolean;
  hierarchySearch: string;
  expandedEntityIds: string[];
  flatEntityTree: FlatTreeNode[];
  bottomTab: BottomTab;
  mode: StudioMode;
  scriptEditorOpen: boolean;
  settingsOpen: boolean;
  sceneDirty: boolean;
  savingScene: boolean;
  previewStatus: PreviewStatus;
  previewBusy: boolean;
  preferences: StudioPreferences;
  layout: LayoutState;
  dragState: DragState | null;
  assetDrag: AssetDragState | null;
}

export interface StudioActions {
  setHome: (home: StudioHome) => void;
  setBridgeStatus: (status: BridgeStatus) => void;
  setHomeVisible: (visible: boolean) => void;
  setHomeError: (error: string | null) => void;
  clearRecentProjects: () => void;
  setSelectedEntityId: (id: string) => void;
  setSelectionTarget: (target: SelectionTarget) => void;
  selectSceneRoot: () => void;
  setSelectedScriptPath: (path: string) => void;
  setViewMode: (mode: ViewMode) => void;
  setViewportTab: (tab: ViewportTab) => void;
  setViewportPivotMode: (mode: ViewportPivotMode) => void;
  setViewportSpaceMode: (mode: ViewportSpaceMode) => void;
  setViewportGizmos: (enabled: boolean) => void;
  setHierarchySearch: (value: string) => void;
  toggleHierarchyExpanded: (entityId: string) => void;
  setBottomTab: (tab: BottomTab) => void;
  setMode: (mode: StudioMode) => void;
  setScriptEditorOpen: (open: boolean) => void;
  setSettingsOpen: (open: boolean) => void;
  setPreviewStatus: (status: PreviewStatus) => void;
  setPreferences: (patch: Partial<StudioPreferences>) => void;
  setLayout: (layout: LayoutState) => void;
  resetLayout: () => void;
  setDockActiveTab: (region: DockRegionId, panelId: DockPanelId) => void;
  moveDockPanel: (panelId: DockPanelId, region: DockRegionId) => void;
  resizeDockRegion: (region: DockRegionId, size: number) => void;
  toggleDockCollapse: (region: DockRegionId) => void;
  setDragState: (state: DragState | null) => void;
  setAssetDrag: (state: AssetDragState | null) => void;
  clearConsole: () => void;

  appendConsole: (line: Omit<ConsoleLine, "id">) => void;
  applySnapshot: (loaded: StudioSnapshot) => void;
  refreshEntityTree: () => Promise<void>;
  updateScene: (updater: (scene: SceneDocument) => SceneDocument) => void;
  selectEntity: (entity: SceneEntity) => void;
  clearSelection: () => void;
  updateSelectedEntity: (patch: Partial<SceneEntity>) => void;
  updateSelectedTransform: (patch: Partial<Transform3D>) => void;
  updateScriptContent: (path: string, content: string) => void;
  attachScript: (path: string) => void;
  openScriptEditor: (path: string) => void;
  createEntityFromAsset: (asset: ProjectAsset, transform?: Partial<Transform3D>) => void;
  reparentEntity: (entityId: string, newParentId?: string) => Promise<void>;

  initBridge: () => () => void;
  openProject: (projectPath: string) => Promise<void>;
  openDefaultProject: () => Promise<void>;
  createProject: (request: NewProjectRequest) => Promise<void>;
  saveScript: (script: ScriptDocument) => Promise<void>;
  saveScene: () => Promise<void>;
  startPreview: () => Promise<void>;
  pausePreview: () => Promise<void>;
  stopPreview: () => Promise<void>;
  applyPreviewResult: (result: PreviewCommandResult, fallbackMessage?: string) => void;
}

const initialSnapshot = createMockSnapshot();
const initialTree = buildFlatEntityTree(initialSnapshot.scene.entities);

export const useStudioStore = create<StudioState & StudioActions>()((set, get) => ({
  snapshot: initialSnapshot,
  home: createMockHome(),
  catalogVersion: 0,
  homeVisible: true,
  homeBusy: false,
  homeError: null,
  recentProjects: readStoredRecentProjects(),
  bridgeStatus: "loading",
  selectedEntityId: initialSnapshot.scene.entities[0]?.id ?? "",
  selectionTarget: initialSnapshot.scene.entities[0] ? entitySelection(initialSnapshot.scene.entities[0].id) : sceneRootSelection(),
  selectedScriptPath: "src/app/player_controller.zt",
  viewMode: "3d",
  viewportTab: "scene",
  viewportPivotMode: "pivot",
  viewportSpaceMode: "global",
  viewportGizmos: true,
  hierarchySearch: "",
  expandedEntityIds: expandedIdsFromTree(initialTree),
  flatEntityTree: initialTree,
  bottomTab: "project",
  mode: "move",
  scriptEditorOpen: false,
  settingsOpen: false,
  sceneDirty: false,
  savingScene: false,
  previewStatus: "idle",
  previewBusy: false,
  preferences: readStoredPreferences(),
  layout: readStoredLayout(initialSnapshot.projectPath),
  dragState: null,
  assetDrag: null,

  setHome: (home) => set({ home }),
  setBridgeStatus: (bridgeStatus) => set({ bridgeStatus }),
  setHomeVisible: (homeVisible) => set({ homeVisible }),
  setHomeError: (homeError) => set({ homeError }),
  clearRecentProjects: () => {
    persistRecentProjects([]);
    set({ recentProjects: [] });
  },
  setSelectedEntityId: (selectedEntityId) =>
    set({
      selectedEntityId,
      selectionTarget: selectedEntityId ? entitySelection(selectedEntityId) : noneSelection(),
    }),
  setSelectionTarget: (selectionTarget) =>
    set({
      selectionTarget,
      selectedEntityId: selectionTarget.kind === "entity" ? selectionTarget.entityId ?? "" : "",
    }),
  selectSceneRoot: () => set({ selectionTarget: sceneRootSelection(), selectedEntityId: "" }),
  setSelectedScriptPath: (selectedScriptPath) => set({ selectedScriptPath }),
  setViewMode: (viewMode) => set({ viewMode }),
  setViewportTab: (viewportTab) => set({ viewportTab }),
  setViewportPivotMode: (viewportPivotMode) => set({ viewportPivotMode }),
  setViewportSpaceMode: (viewportSpaceMode) => set({ viewportSpaceMode }),
  setViewportGizmos: (viewportGizmos) => set({ viewportGizmos }),
  setHierarchySearch: (hierarchySearch) => set({ hierarchySearch }),
  toggleHierarchyExpanded: (entityId) =>
    set((state) => ({
      expandedEntityIds: state.expandedEntityIds.includes(entityId)
        ? state.expandedEntityIds.filter((id) => id !== entityId)
        : [...state.expandedEntityIds, entityId],
    })),
  setBottomTab: (bottomTab) => set({ bottomTab }),
  setMode: (mode) => set({ mode }),
  setScriptEditorOpen: (scriptEditorOpen) => set({ scriptEditorOpen }),
  setSettingsOpen: (settingsOpen) => set({ settingsOpen }),
  setPreviewStatus: (previewStatus) => set({ previewStatus }),
  setPreferences: (patch) =>
    set((state) => {
      const preferences = { ...state.preferences, ...patch };
      persistPreferences(preferences);
      return { preferences };
    }),
  setLayout: (layout) => {
    const next = syncLegacySizes(layout);
    persistLayout(get().snapshot.projectPath, next);
    set({ layout: next });
  },
  resetLayout: () => {
    const next = createDefaultLayout();
    persistLayout(get().snapshot.projectPath, next);
    set({ layout: next });
  },
  setDockActiveTab: (region, panelId) =>
    set((state) => {
      const next = {
        ...state.layout,
        docks: {
          ...state.layout.docks,
          [region]: {
            ...state.layout.docks[region],
            activeTab: panelId,
          },
        },
      };
      persistLayout(state.snapshot.projectPath, next);
      return { layout: syncLegacySizes(next) };
    }),
  moveDockPanel: (panelId, region) =>
    set((state) => {
      if (!movablePanel(panelId)) return {};

      const docks = {
        ...state.layout.docks,
        leftDock: { ...state.layout.docks.leftDock, tabs: [...state.layout.docks.leftDock.tabs] },
        rightDock: { ...state.layout.docks.rightDock, tabs: [...state.layout.docks.rightDock.tabs] },
        bottomDock: { ...state.layout.docks.bottomDock, tabs: [...state.layout.docks.bottomDock.tabs] },
      };

      (["leftDock", "rightDock", "bottomDock"] as DockRegionId[]).forEach((key) => {
        const panelRegion = docks[key];
        if (!panelRegion.tabs.includes(panelId)) return;
        panelRegion.tabs = panelRegion.tabs.filter((tab) => tab !== panelId);
        if (panelRegion.tabs.length === 0) {
          panelRegion.tabs = [...DEFAULT_DOCK_TABS[key]];
        }
        if (!panelRegion.tabs.includes(panelRegion.activeTab)) {
          panelRegion.activeTab = panelRegion.tabs[0];
        }
      });

      const target = docks[region];
      if (!target.tabs.includes(panelId)) {
        target.tabs.push(panelId);
      }
      target.activeTab = panelId;
      target.collapsed = false;

      const next = syncLegacySizes({ ...state.layout, docks });
      persistLayout(state.snapshot.projectPath, next);
      return { layout: next };
    }),
  resizeDockRegion: (region, size) =>
    set((state) => {
      const next = {
        ...state.layout,
        docks: {
          ...state.layout.docks,
          [region]: {
            ...state.layout.docks[region],
            size,
          },
        },
      };
      const synced = syncLegacySizes(next);
      persistLayout(state.snapshot.projectPath, synced);
      return { layout: synced };
    }),
  toggleDockCollapse: (region) =>
    set((state) => {
      const next = {
        ...state.layout,
        docks: {
          ...state.layout.docks,
          [region]: {
            ...state.layout.docks[region],
            collapsed: !state.layout.docks[region].collapsed,
          },
        },
      };
      persistLayout(state.snapshot.projectPath, next);
      return { layout: syncLegacySizes(next) };
    }),
  setDragState: (dragState) => set({ dragState }),
  setAssetDrag: (assetDrag) => set({ assetDrag }),
  clearConsole: () =>
    set((state) => ({
      snapshot: {
        ...state.snapshot,
        console: [],
      },
    })),

  appendConsole: (line) =>
    set((state) => ({
      snapshot: {
        ...state.snapshot,
        console: [
          ...state.snapshot.console,
          { ...line, id: `line-${Date.now()}-${state.snapshot.console.length}` },
        ].slice(-150),
      },
    })),

  applySnapshot: (loaded) => {
    const currentRecentProjects = get().recentProjects;
    const tree = buildFlatEntityTree(loaded.scene.entities);
    const firstEntity = loaded.scene.entities[0];
    const selection = firstEntity ? entitySelection(firstEntity.id) : sceneRootSelection();
    const layout = readStoredLayout(loaded.projectPath);
    const recentProjects = rememberRecentProject(currentRecentProjects, loaded);
    set({
      snapshot: loaded,
      recentProjects,
      flatEntityTree: tree,
      expandedEntityIds: expandedIdsFromTree(tree),
      sceneDirty: false,
      previewStatus: "idle",
      homeVisible: false,
      homeError: null,
      selectedEntityId: firstEntity?.id ?? "",
      selectionTarget: selection,
      selectedScriptPath: firstEntity
        ? entityScript(firstEntity) ?? loaded.scripts[0]?.path ?? ""
        : loaded.scripts[0]?.path ?? "",
      layout,
    });
    persistLayout(loaded.projectPath, layout);
  },

  refreshEntityTree: async () => {
    const state = get();
    if (state.bridgeStatus !== "tauri") {
      const tree = buildFlatEntityTree(state.snapshot.scene.entities);
      set({ flatEntityTree: tree });
      return;
    }

    try {
      const tree = await getEntityTree();
      set({ flatEntityTree: tree, expandedEntityIds: expandedIdsFromTree(tree) });
    } catch {
      set({ flatEntityTree: buildFlatEntityTree(state.snapshot.scene.entities) });
    }
  },

  updateScene: (updater) =>
    set((state) => {
      const nextScene = updater(state.snapshot.scene);
      return {
        sceneDirty: true,
        snapshot: { ...state.snapshot, scene: nextScene },
        flatEntityTree: buildFlatEntityTree(nextScene.entities),
      };
    }),

  selectEntity: (entity) => {
    const script = entityScript(entity);
    set({
      selectedEntityId: entity.id,
      selectionTarget: entitySelection(entity.id),
      ...(script ? { selectedScriptPath: script } : {}),
    });
  },

  clearSelection: () => set({ selectedEntityId: "", selectionTarget: noneSelection(), dragState: null }),

  updateSelectedEntity: (patch) => {
    const entity = selectedEntity(get());
    if (!entity) return;
    get().updateScene((scene) => ({
      ...scene,
      entities: scene.entities.map((current) => (current.id === entity.id ? { ...current, ...patch } : current)),
    }));
  },

  updateSelectedTransform: (patch) => {
    const entity = selectedEntity(get());
    if (!entity) return;
    get().updateSelectedEntity({
      transform: { ...entity.transform, ...patch },
    });
  },

  updateScriptContent: (path, content) =>
    set((state) => ({
      snapshot: {
        ...state.snapshot,
        scripts: state.snapshot.scripts.map((script) =>
          script.path === path ? { ...script, content, dirty: true } : script,
        ),
      },
    })),

  attachScript: (path) => {
    const state = get();
    const entity = selectedEntity(state);
    if (!entity) return;
    const withoutScript = entity.components.filter((component) => component.kind !== "script");
    state.updateSelectedEntity({
      components: [...withoutScript, { kind: "script", script: path }],
    });
    set({ selectedScriptPath: path });
    state.appendConsole({
      level: "info",
      source: "studio",
      message: `Attached ${path} to ${entity.name}`,
    });
  },

  openScriptEditor: (path) => set({ selectedScriptPath: path, scriptEditorOpen: true }),

  createEntityFromAsset: (asset, transform) => {
    const state = get();
    const count = state.snapshot.scene.entities.length + 1;
    const isModel = asset.kind === "model";
    const entity: SceneEntity = {
      id: `entity-${count.toString().padStart(2, "0")}`,
      name: asset.name.replace(/\.[^.]+$/, "").replace(/[-_]/g, " "),
      layer: isModel ? "world3d" : "default",
      tags: isModel ? ["3d", "asset"] : ["asset"],
      components: [{ kind: isModel ? "model3d" : "sprite", asset: asset.path }],
      transform: {
        x: 40 + count * 22,
        y: 32 + count * 16,
        z: isModel ? 18 : 0,
        rotationX: 0,
        rotationY: 0,
        rotationZ: 0,
        scaleX: 1,
        scaleY: 1,
        scaleZ: 1,
        ...transform,
      },
    };
    state.updateScene((scene) => ({
      ...scene,
      entities: [...scene.entities, entity],
    }));
    set({ selectedEntityId: entity.id, selectionTarget: entitySelection(entity.id), viewportTab: "scene" });
    state.appendConsole({
      level: "info",
      source: "studio",
      message: `Added ${asset.path} to Scene View`,
    });
  },

  reparentEntity: async (entityId, newParentId) => {
    const state = get();
    if (state.bridgeStatus === "tauri") {
      try {
        await reparentEntityCommand(entityId, newParentId);
      } catch (error) {
        state.appendConsole({
          level: "error",
          source: "studio",
          message: `Could not reparent ${entityId}: ${String(error)}`,
        });
        return;
      }
    }

    state.updateScene((scene) => updateEntityParentLocally(scene, entityId, newParentId));
    await get().refreshEntityTree();
    state.appendConsole({
      level: "info",
      source: "studio",
      message: newParentId
        ? `Moved ${entityId} under ${newParentId}`
        : `Moved ${entityId} to the scene root`,
    });
  },

  initBridge: () => {
    let cancelled = false;
    loadStudioHome()
      .then((loadedHome) => {
        if (cancelled) return;
        configureBorealisCatalog(loadedHome.editorManifest);
        set({ home: loadedHome, bridgeStatus: "tauri", catalogVersion: get().catalogVersion + 1 });
      })
      .catch(() => {
        if (cancelled) return;
        const mock = createMockHome();
        configureBorealisCatalog(mock.editorManifest);
        set({
          bridgeStatus: isTauriRuntime() ? "error" : "browser",
          home: mock,
          catalogVersion: get().catalogVersion + 1,
        });
      });
    return () => {
      cancelled = true;
    };
  },

  openProject: async (projectPath) => {
    const trimmed = projectPath.trim();
    const state = get();
    if (!trimmed) {
      set({ homeError: "Informe o caminho de um arquivo zenith.ztproj ou de uma pasta de projeto." });
      return;
    }
    set({ homeBusy: true, homeError: null });
    try {
      const loaded = await loadStudioSnapshot(trimmed);
      state.applySnapshot(loaded);
      set({ bridgeStatus: "tauri" });
      await get().refreshEntityTree();
    } catch (error) {
      if (state.bridgeStatus === "browser") {
        const mock = createMockSnapshot();
        get().applySnapshot({
          ...mock,
          projectPath: trimmed,
          console: [
            ...mock.console,
            {
              id: `mock-project-${Date.now()}`,
              level: "warn",
              source: "studio",
              message: `Browser mode opened mock data for ${trimmed}.`,
            },
          ],
        });
        set({ bridgeStatus: "browser" });
      } else {
        set({ homeError: `Nao foi possivel abrir o projeto: ${String(error)}` });
      }
    } finally {
      set({ homeBusy: false });
    }
  },

  openDefaultProject: async () => {
    await get().openProject(get().home.defaultProjectPath);
  },

  createProject: async (request) => {
    const state = get();
    if (!request.projectName.trim()) {
      set({ homeError: "Informe um nome para o projeto." });
      return;
    }
    set({ homeBusy: true, homeError: null });
    try {
      const loaded = await createBorealisProject(request);
      state.applySnapshot(loaded);
      set({ bridgeStatus: "tauri" });
      await get().refreshEntityTree();
    } catch (error) {
      if (state.bridgeStatus === "browser") {
        const mock = createMockSnapshot();
        get().applySnapshot({
          ...mock,
          projectName: request.projectName,
          projectPath: `${request.parentDir || state.home.defaultProjectsDir}/${request.projectName}/zenith.ztproj`,
          console: [
            ...mock.console,
            {
              id: `mock-new-project-${Date.now()}`,
              level: "warn",
              source: "studio",
              message: "Browser mode shows a mock project. Use the Tauri app to create files on disk.",
            },
          ],
        });
        set({ bridgeStatus: "browser" });
      } else {
        set({ homeError: `Nao foi possivel criar o projeto: ${String(error)}` });
      }
    } finally {
      set({ homeBusy: false });
    }
  },

  saveScript: async (script) => {
    const state = get();
    try {
      await writeScriptDocument(script, state.snapshot.projectRoot);
      set((current) => ({
        snapshot: {
          ...current.snapshot,
          scripts: current.snapshot.scripts.map((item) =>
            item.path === script.path ? { ...item, dirty: false } : item,
          ),
        },
      }));
      state.appendConsole({ level: "info", source: "studio", message: `Saved ${script.path}` });
    } catch (error) {
      state.appendConsole({
        level: state.bridgeStatus === "browser" ? "warn" : "error",
        source: "studio",
        message:
          state.bridgeStatus === "browser"
            ? "Browser mode keeps script edits in memory."
            : `Could not save ${script.path}: ${String(error)}`,
      });
    }
  },

  saveScene: async () => {
    const state = get();
    set({ savingScene: true });
    try {
      await writeSceneDocument(state.snapshot.scene, state.snapshot.projectRoot);
      set({ sceneDirty: false });
      state.appendConsole({ level: "info", source: "studio", message: `Saved ${state.snapshot.scene.path}` });
    } catch (error) {
      state.appendConsole({
        level: state.bridgeStatus === "browser" ? "warn" : "error",
        source: "studio",
        message:
          state.bridgeStatus === "browser"
            ? "Browser mode keeps scene edits in memory."
            : `Could not save ${state.snapshot.scene.path}: ${String(error)}`,
      });
    } finally {
      set({ savingScene: false });
    }
  },

  startPreview: async () => {
    const state = get();
    set({ previewBusy: true, previewStatus: "starting", viewportTab: "game" });
    try {
      const result = await startRuntimePreview(state.snapshot.projectPath, state.snapshot.scene);
      state.applyPreviewResult(result, "Preview started with the current scene draft.");
    } catch (error) {
      set({ previewStatus: state.bridgeStatus === "browser" ? "unavailable" : "error" });
      state.appendConsole({
        level: state.bridgeStatus === "browser" ? "warn" : "error",
        source: "preview",
        message:
          state.bridgeStatus === "browser"
            ? "Play mode needs the Tauri desktop app. Browser mode keeps the editor preview-only."
            : `Could not start preview: ${String(error)}`,
      });
    } finally {
      set({ previewBusy: false });
    }
  },

  pausePreview: async () => {
    const state = get();
    set({ previewBusy: true });
    try {
      const result = await pauseRuntimePreview();
      state.applyPreviewResult(result, "Preview paused.");
    } catch (error) {
      set({ previewStatus: "error" });
      state.appendConsole({ level: "error", source: "preview", message: `Could not pause preview: ${String(error)}` });
    } finally {
      set({ previewBusy: false });
    }
  },

  stopPreview: async () => {
    const state = get();
    set({ previewBusy: true });
    try {
      const result = await stopRuntimePreview();
      state.applyPreviewResult(result, "Preview stopped.");
    } catch (error) {
      set({ previewStatus: "error" });
      state.appendConsole({ level: "error", source: "preview", message: `Could not stop preview: ${String(error)}` });
    } finally {
      set({ previewBusy: false });
    }
  },

  applyPreviewResult: (result, fallbackMessage?) => {
    const state = get();
    const nextStatus = normalizePreviewStatus(result.status);
    set({ previewStatus: nextStatus });
    if (result.events.length === 0 && fallbackMessage) {
      state.appendConsole({ level: "info", source: "preview", message: fallbackMessage });
      return;
    }
    result.events.forEach((event) => {
      state.appendConsole(previewEventToConsoleLine(event, result.runner));
    });
  },
}));

export function useSelectedEntity(): SceneEntity | null {
  return useStudioStore((state) => selectedEntity(state));
}

export function useSceneRootSelected(): boolean {
  return useStudioStore((state) => state.selectionTarget.kind === "scene-root");
}

export function useActiveScript(): ScriptDocument | undefined {
  return useStudioStore((state) => {
    const entity = selectedEntity(state);
    const path = state.selectedScriptPath || (entity ? entityScript(entity) ?? "" : "");
    return state.snapshot.scripts.find((script) => script.path === path) ?? state.snapshot.scripts[0];
  });
}
