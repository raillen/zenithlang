import { create } from "zustand";
import {
  createBorealisProject,
  isTauriRuntime,
  loadStudioHome,
  loadStudioSnapshot,
  pauseRuntimePreview,
  pollRuntimePreview,
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
  DragState,
  LayoutState,
  NewProjectRequest,
  PreviewCommandResult,
  PreviewStatus,
  ProjectAsset,
  SceneComponent,
  SceneDocument,
  SceneEntity,
  ScriptDocument,
  StudioHome,
  StudioMode,
  StudioPreferences,
  StudioSnapshot,
  Transform3D,
  ViewMode,
} from "../types";
import { entityScript } from "../utils/entity";
import { normalizePreviewStatus, previewEventToConsoleLine } from "../utils/preview";

export interface StudioState {
  snapshot: StudioSnapshot;
  home: StudioHome;
  catalogVersion: number;
  homeVisible: boolean;
  homeBusy: boolean;
  homeError: string | null;
  bridgeStatus: BridgeStatus;
  selectedEntityId: string;
  selectedScriptPath: string;
  viewMode: ViewMode;
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
  setSelectedEntityId: (id: string) => void;
  setSelectedScriptPath: (path: string) => void;
  setViewMode: (mode: ViewMode) => void;
  setBottomTab: (tab: BottomTab) => void;
  setMode: (mode: StudioMode) => void;
  setScriptEditorOpen: (open: boolean) => void;
  setSettingsOpen: (open: boolean) => void;
  setPreviewStatus: (status: PreviewStatus) => void;
  setPreferences: (patch: Partial<StudioPreferences>) => void;
  setLayout: (layout: LayoutState) => void;
  setDragState: (state: DragState | null) => void;
  setAssetDrag: (state: AssetDragState | null) => void;

  appendConsole: (line: Omit<ConsoleLine, "id">) => void;
  applySnapshot: (loaded: StudioSnapshot) => void;
  updateScene: (updater: (scene: SceneDocument) => SceneDocument) => void;
  selectEntity: (entity: SceneEntity) => void;
  clearSelection: () => void;
  updateSelectedEntity: (patch: Partial<SceneEntity>) => void;
  updateSelectedTransform: (patch: Partial<Transform3D>) => void;
  updateScriptContent: (path: string, content: string) => void;
  attachScript: (path: string) => void;
  openScriptEditor: (path: string) => void;
  createEntityFromAsset: (asset: ProjectAsset) => void;

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

function selectedEntity(state: StudioState): SceneEntity | null {
  return state.snapshot.scene.entities.find((e) => e.id === state.selectedEntityId) ?? null;
}

export const useStudioStore = create<StudioState & StudioActions>()((set, get) => ({
  snapshot: createMockSnapshot(),
  home: createMockHome(),
  catalogVersion: 0,
  homeVisible: true,
  homeBusy: false,
  homeError: null,
  bridgeStatus: "loading",
  selectedEntityId: "player",
  selectedScriptPath: "src/app/player_controller.zt",
  viewMode: "3d",
  bottomTab: "assets",
  mode: "move",
  scriptEditorOpen: false,
  settingsOpen: false,
  sceneDirty: false,
  savingScene: false,
  previewStatus: "idle",
  previewBusy: false,
  preferences: {
    gizmoSize: 0.75,
    ptzSpeed: 1,
    shortcutTemplate: "blender",
    snapMode: "grid-object",
    theme: "codex",
  },
  layout: { left: 280, right: 292, bottom: 218 },
  dragState: null,
  assetDrag: null,

  setHome: (home) => set({ home }),
  setBridgeStatus: (bridgeStatus) => set({ bridgeStatus }),
  setHomeVisible: (homeVisible) => set({ homeVisible }),
  setHomeError: (homeError) => set({ homeError }),
  setSelectedEntityId: (selectedEntityId) => set({ selectedEntityId }),
  setSelectedScriptPath: (selectedScriptPath) => set({ selectedScriptPath }),
  setViewMode: (viewMode) => set({ viewMode }),
  setBottomTab: (bottomTab) => set({ bottomTab }),
  setMode: (mode) => set({ mode }),
  setScriptEditorOpen: (scriptEditorOpen) => set({ scriptEditorOpen }),
  setSettingsOpen: (settingsOpen) => set({ settingsOpen }),
  setPreviewStatus: (previewStatus) => set({ previewStatus }),
  setPreferences: (patch) => set((s) => ({ preferences: { ...s.preferences, ...patch } })),
  setLayout: (layout) => set({ layout }),
  setDragState: (dragState) => set({ dragState }),
  setAssetDrag: (assetDrag) => set({ assetDrag }),

  appendConsole: (line) =>
    set((s) => ({
      snapshot: {
        ...s.snapshot,
        console: [
          ...s.snapshot.console,
          { ...line, id: `line-${Date.now()}-${s.snapshot.console.length}` },
        ].slice(-80),
      },
    })),

  applySnapshot: (loaded) => {
    const firstEntity = loaded.scene.entities[0];
    set({
      snapshot: loaded,
      sceneDirty: false,
      previewStatus: "idle",
      homeVisible: false,
      homeError: null,
      selectedEntityId: firstEntity?.id ?? "",
      selectedScriptPath: firstEntity
        ? entityScript(firstEntity) ?? loaded.scripts[0]?.path ?? ""
        : loaded.scripts[0]?.path ?? "",
    });
  },

  updateScene: (updater) =>
    set((s) => ({
      sceneDirty: true,
      snapshot: { ...s.snapshot, scene: updater(s.snapshot.scene) },
    })),

  selectEntity: (entity) => {
    const script = entityScript(entity);
    set({
      selectedEntityId: entity.id,
      ...(script ? { selectedScriptPath: script } : {}),
    });
  },

  clearSelection: () => set({ selectedEntityId: "", dragState: null }),

  updateSelectedEntity: (patch) => {
    const entity = selectedEntity(get());
    if (!entity) return;
    get().updateScene((scene) => ({
      ...scene,
      entities: scene.entities.map((e) =>
        e.id === entity.id ? { ...e, ...patch } : e,
      ),
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
    set((s) => ({
      snapshot: {
        ...s.snapshot,
        scripts: s.snapshot.scripts.map((script) =>
          script.path === path ? { ...script, content, dirty: true } : script,
        ),
      },
    })),

  attachScript: (path) => {
    const state = get();
    const entity = selectedEntity(state);
    if (!entity) return;
    const withoutScript = entity.components.filter((c) => c.kind !== "script");
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

  createEntityFromAsset: (asset) => {
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
      },
    };
    state.updateScene((scene) => ({
      ...scene,
      entities: [...scene.entities, entity],
    }));
    set({ selectedEntityId: entity.id });
    state.appendConsole({
      level: "info",
      source: "studio",
      message: `Added ${asset.path} to Scene View`,
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
    return () => { cancelled = true; };
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
    } catch (error) {
      if (state.bridgeStatus === "browser") {
        const mock = createMockSnapshot();
        get().applySnapshot({
          ...mock,
          projectPath: trimmed,
          console: [
            ...mock.console,
            { id: `mock-project-${Date.now()}`, level: "warn", source: "studio", message: `Browser mode opened mock data for ${trimmed}.` },
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
    } catch (error) {
      if (state.bridgeStatus === "browser") {
        const mock = createMockSnapshot();
        get().applySnapshot({
          ...mock,
          projectName: request.projectName,
          projectPath: `${request.parentDir || state.home.defaultProjectsDir}/${request.projectName}/zenith.ztproj`,
          console: [
            ...mock.console,
            { id: `mock-new-project-${Date.now()}`, level: "warn", source: "studio", message: "Browser mode shows a mock project. Use the Tauri app to create files on disk." },
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
      set((s) => ({
        snapshot: {
          ...s.snapshot,
          scripts: s.snapshot.scripts.map((item) =>
            item.path === script.path ? { ...item, dirty: false } : item,
          ),
        },
      }));
      state.appendConsole({ level: "info", source: "studio", message: `Saved ${script.path}` });
    } catch (error) {
      state.appendConsole({
        level: state.bridgeStatus === "browser" ? "warn" : "error",
        source: "studio",
        message: state.bridgeStatus === "browser"
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
        message: state.bridgeStatus === "browser"
          ? "Browser mode keeps scene edits in memory."
          : `Could not save ${state.snapshot.scene.path}: ${String(error)}`,
      });
    } finally {
      set({ savingScene: false });
    }
  },

  startPreview: async () => {
    const state = get();
    set({ previewBusy: true, previewStatus: "starting" });
    try {
      const result = await startRuntimePreview(state.snapshot.projectPath, state.snapshot.scene);
      state.applyPreviewResult(result, "Preview started with the current scene draft.");
    } catch (error) {
      set({ previewStatus: state.bridgeStatus === "browser" ? "unavailable" : "error" });
      state.appendConsole({
        level: state.bridgeStatus === "browser" ? "warn" : "error",
        source: "preview",
        message: state.bridgeStatus === "browser"
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
  return useStudioStore((s) =>
    s.snapshot.scene.entities.find((e) => e.id === s.selectedEntityId) ?? null,
  );
}

export function useActiveScript(): ScriptDocument | undefined {
  return useStudioStore((s) => {
    const entity = s.snapshot.scene.entities.find((e) => e.id === s.selectedEntityId);
    const path = s.selectedScriptPath || (entity ? entityScript(entity) ?? "" : "");
    return s.snapshot.scripts.find((script) => script.path === path) ?? s.snapshot.scripts[0];
  });
}
