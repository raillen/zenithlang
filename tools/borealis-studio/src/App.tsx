import { AnimatePresence, motion } from "framer-motion";
import { Canvas, useThree } from "@react-three/fiber";
import {
  Environment,
  GizmoHelper,
  GizmoViewport,
  Grid,
  OrbitControls,
  OrthographicCamera,
  PerspectiveCamera,
  TransformControls,
} from "@react-three/drei";
import { MOUSE, Vector3 } from "three";
import {
  Activity,
  Archive,
  BadgeAlert,
  BookOpen,
  Box,
  Braces,
  Check,
  ChevronDown,
  Circle,
  Code2,
  Command,
  Component,
  Cuboid,
  Database,
  FilePlus2,
  FileCode2,
  FolderOpen,
  FolderTree,
  Home,
  Layers3,
  LayoutTemplate,
  Maximize2,
  MousePointer2,
  Move3D,
  Pause,
  Play,
  RotateCcw,
  Save,
  Scale3D,
  Search,
  Settings2,
  Square,
  Terminal,
  WandSparkles,
  X,
  ZoomIn,
} from "lucide-react";
import { PointerEvent, useEffect, useMemo, useRef, useState } from "react";
import type { Group } from "three";
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
} from "./backend";
import {
  borealisComponents,
  borealisSceneSettings,
  configureBorealisCatalog,
  componentSchema,
  componentSummary,
  componentValue,
  createComponentFromSchema,
  setComponentValue,
} from "./borealisCatalog";
import { createMockHome, createMockSnapshot } from "./mockData";
import type {
  AssetKind,
  BottomTab,
  ComponentFieldSchema,
  ConsoleLine,
  DocumentationLink,
  NewProjectRequest,
  PreviewCommandResult,
  PreviewEvent,
  PreviewStatus,
  ProjectTemplateId,
  ProjectTemplate,
  ProjectAsset,
  SceneComponent,
  SceneDocument,
  SceneEnvironment,
  SceneEntity,
  SceneAudioSettings,
  SceneRenderSettings,
  ScriptDocument,
  StudioHome,
  StudioMode,
  StudioSnapshot,
  Transform3D,
} from "./types";

const ICON_STROKE = 1.75;

type BridgeStatus = "loading" | "tauri" | "browser" | "error";

interface LayoutState {
  left: number;
  right: number;
  bottom: number;
}

interface DragState {
  id: string;
  startX: number;
  startY: number;
  originX: number;
  originY: number;
}

interface AssetDragState {
  asset: ProjectAsset;
  x: number;
  y: number;
}

interface ViewportCamera {
  panX: number;
  panY: number;
  zoom: number;
}

interface ViewportPanState {
  pointerId: number;
  startX: number;
  startY: number;
  originPanX: number;
  originPanY: number;
}

type ViewMode = "2d" | "3d";
type ViewProjection = "perspective" | "orthographic" | "isometric";
type ViewOrientation = "free" | "top" | "bottom" | "left" | "right" | "front" | "back";
type RenderMode = "wireframe" | "color" | "texture" | "light";
type ShortcutTemplate = "blender" | "3dsmax" | "maya";
type ThemeMode = "codex" | "xcode" | "unity-dark";
type SnapMode = "grid" | "object" | "grid-object";
type SnapSettings = {
  gridSize: number;
  snapToGrid: boolean;
  snapToObject: boolean;
};
type StudioPreferences = {
  gizmoSize: number;
  ptzSpeed: number;
  shortcutTemplate: ShortcutTemplate;
  snapMode: SnapMode;
  theme: ThemeMode;
};

const VIEW_ORIENTATIONS: Array<{ id: ViewOrientation; label: string; shortcut: string }> = [
  { id: "top", label: "Top", shortcut: "T" },
  { id: "bottom", label: "Bottom", shortcut: "B" },
  { id: "left", label: "Left", shortcut: "L" },
  { id: "right", label: "Right", shortcut: "R" },
  { id: "front", label: "Front", shortcut: "F" },
  { id: "back", label: "Back", shortcut: "K" },
];

export function BorealisStudioApp() {
  const [snapshot, setSnapshot] = useState<StudioSnapshot>(() => createMockSnapshot());
  const [home, setHome] = useState<StudioHome>(() => createMockHome());
  const [, setCatalogVersion] = useState(0);
  const [homeVisible, setHomeVisible] = useState(true);
  const [homeBusy, setHomeBusy] = useState(false);
  const [homeError, setHomeError] = useState<string | null>(null);
  const [bridgeStatus, setBridgeStatus] = useState<BridgeStatus>("loading");
  const [selectedEntityId, setSelectedEntityId] = useState("player");
  const [selectedScriptPath, setSelectedScriptPath] = useState("src/app/player_controller.zt");
  const [viewMode, setViewMode] = useState<ViewMode>("3d");
  const [bottomTab, setBottomTab] = useState<BottomTab>("assets");
  const [mode, setMode] = useState<StudioMode>("move");
  const [scriptEditorOpen, setScriptEditorOpen] = useState(false);
  const [settingsOpen, setSettingsOpen] = useState(false);
  const [sceneDirty, setSceneDirty] = useState(false);
  const [savingScene, setSavingScene] = useState(false);
  const [previewStatus, setPreviewStatus] = useState<PreviewStatus>("idle");
  const [previewBusy, setPreviewBusy] = useState(false);
  const [preferences, setPreferences] = useState<StudioPreferences>({
    gizmoSize: 0.75,
    ptzSpeed: 1,
    shortcutTemplate: "blender",
    snapMode: "grid-object",
    theme: "codex",
  });
  const [layout, setLayout] = useState<LayoutState>({
    left: 280,
    right: 292,
    bottom: 218,
  });
  const [dragState, setDragState] = useState<DragState | null>(null);
  const [assetDrag, setAssetDrag] = useState<AssetDragState | null>(null);

  useEffect(() => {
    configureBorealisCatalog(home.editorManifest);
    setCatalogVersion((version) => version + 1);
  }, [home.editorManifest]);

  useEffect(() => {
    let cancelled = false;
    loadStudioHome()
      .then((loadedHome) => {
        if (cancelled) return;
        setHome(loadedHome);
        setBridgeStatus("tauri");
      })
      .catch(() => {
        if (cancelled) return;
        setBridgeStatus(isTauriRuntime() ? "error" : "browser");
        setHome(createMockHome());
      });

    return () => {
      cancelled = true;
    };
  }, []);

  useEffect(() => {
    function handleToolShortcut(event: KeyboardEvent) {
      const target = event.target;
      if (
        target instanceof HTMLInputElement ||
        target instanceof HTMLTextAreaElement ||
        target instanceof HTMLSelectElement ||
        (target instanceof HTMLElement && target.isContentEditable)
      ) {
        return;
      }

      const key = event.key.toLowerCase();
      const shortcutMode = modeForShortcut(preferences.shortcutTemplate, key);
      if (shortcutMode) setMode(shortcutMode);
    }

    window.addEventListener("keydown", handleToolShortcut);
    return () => window.removeEventListener("keydown", handleToolShortcut);
  }, [preferences.shortcutTemplate]);

  const selectedEntity = useMemo(
    () => snapshot.scene.entities.find((entity) => entity.id === selectedEntityId) ?? null,
    [selectedEntityId, snapshot.scene.entities],
  );

  const activeScriptPath = selectedScriptPath || (selectedEntity ? entityScript(selectedEntity) ?? "" : "");
  const activeScript = useMemo(
    () => snapshot.scripts.find((script) => script.path === activeScriptPath) ?? snapshot.scripts[0],
    [activeScriptPath, snapshot.scripts],
  );

  function appendConsole(line: Omit<ConsoleLine, "id">) {
    setSnapshot((current) => ({
      ...current,
      console: [
        ...current.console,
        {
          ...line,
          id: `line-${Date.now()}-${current.console.length}`,
        },
      ].slice(-80),
    }));
  }

  function applySnapshot(loaded: StudioSnapshot) {
    setSnapshot(loaded);
    setSceneDirty(false);
    setPreviewStatus("idle");
    setHomeVisible(false);
    setHomeError(null);

    const firstEntity = loaded.scene.entities[0];
    if (firstEntity) {
      setSelectedEntityId(firstEntity.id);
      setSelectedScriptPath(entityScript(firstEntity) ?? loaded.scripts[0]?.path ?? "");
    } else {
      setSelectedEntityId("");
      setSelectedScriptPath(loaded.scripts[0]?.path ?? "");
    }
  }

  function useMockProject(projectPath: string, message: string) {
    const mock = createMockSnapshot();
    applySnapshot({
      ...mock,
      projectPath,
      console: [
        ...mock.console,
        {
          id: `mock-project-${Date.now()}`,
          level: "warn",
          source: "studio",
          message,
        },
      ],
    });
    setBridgeStatus("browser");
  }

  async function openProject(projectPath: string) {
    const trimmed = projectPath.trim();
    if (!trimmed) {
      setHomeError("Informe o caminho de um arquivo zenith.ztproj ou de uma pasta de projeto.");
      return;
    }

    setHomeBusy(true);
    setHomeError(null);
    try {
      const loaded = await loadStudioSnapshot(trimmed);
      applySnapshot(loaded);
      setBridgeStatus("tauri");
    } catch (error) {
      if (bridgeStatus === "browser") {
        useMockProject(trimmed, `Browser mode opened mock data for ${trimmed}.`);
      } else {
        setHomeError(`Nao foi possivel abrir o projeto: ${String(error)}`);
      }
    } finally {
      setHomeBusy(false);
    }
  }

  async function openDefaultProject() {
    await openProject(home.defaultProjectPath);
  }

  async function createProject(request: NewProjectRequest) {
    if (!request.projectName.trim()) {
      setHomeError("Informe um nome para o projeto.");
      return;
    }

    setHomeBusy(true);
    setHomeError(null);
    try {
      const loaded = await createBorealisProject(request);
      applySnapshot(loaded);
      setBridgeStatus("tauri");
    } catch (error) {
      if (bridgeStatus === "browser") {
        const mock = createMockSnapshot();
        applySnapshot({
          ...mock,
          projectName: request.projectName,
          projectPath: `${request.parentDir || home.defaultProjectsDir}/${request.projectName}/zenith.ztproj`,
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
        setBridgeStatus("browser");
      } else {
        setHomeError(`Nao foi possivel criar o projeto: ${String(error)}`);
      }
    } finally {
      setHomeBusy(false);
    }
  }

  function updateScene(updater: (scene: SceneDocument) => SceneDocument) {
    setSceneDirty(true);
    setSnapshot((current) => ({
      ...current,
      scene: updater(current.scene),
    }));
  }

  function selectEntity(entity: SceneEntity) {
    setSelectedEntityId(entity.id);
    const script = entityScript(entity);
    if (script) {
      setSelectedScriptPath(script);
    }
  }

  function clearSelection() {
    setSelectedEntityId("");
    setDragState(null);
  }

  function updateSelectedEntity(patch: Partial<SceneEntity>) {
    if (!selectedEntity) return;
    updateScene((scene) => ({
      ...scene,
      entities: scene.entities.map((entity) =>
        entity.id === selectedEntity.id ? { ...entity, ...patch } : entity,
      ),
    }));
  }

  function updateSelectedTransform(patch: Partial<Transform3D>) {
    if (!selectedEntity) return;
    updateSelectedEntity({
      transform: {
        ...selectedEntity.transform,
        ...patch,
      },
    });
  }

  function updateScriptContent(path: string, content: string) {
    setSnapshot((current) => ({
      ...current,
      scripts: current.scripts.map((script) =>
        script.path === path ? { ...script, content, dirty: true } : script,
      ),
    }));
  }

  async function saveScript(script: ScriptDocument) {
    try {
      await writeScriptDocument(script, snapshot.projectRoot);
      setSnapshot((current) => ({
        ...current,
        scripts: current.scripts.map((item) =>
          item.path === script.path ? { ...item, dirty: false } : item,
        ),
      }));
      appendConsole({
        level: "info",
        source: "studio",
        message: `Saved ${script.path}`,
      });
    } catch (error) {
      appendConsole({
        level: bridgeStatus === "browser" ? "warn" : "error",
        source: "studio",
        message:
          bridgeStatus === "browser"
            ? "Browser mode keeps script edits in memory."
            : `Could not save ${script.path}: ${String(error)}`,
      });
    }
  }

  async function saveScene() {
    setSavingScene(true);
    try {
      await writeSceneDocument(snapshot.scene, snapshot.projectRoot);
      setSceneDirty(false);
      appendConsole({
        level: "info",
        source: "studio",
        message: `Saved ${snapshot.scene.path}`,
      });
    } catch (error) {
      appendConsole({
        level: bridgeStatus === "browser" ? "warn" : "error",
        source: "studio",
        message:
          bridgeStatus === "browser"
            ? "Browser mode keeps scene edits in memory."
            : `Could not save ${snapshot.scene.path}: ${String(error)}`,
      });
    } finally {
      setSavingScene(false);
    }
  }

  async function startPreview() {
    setPreviewBusy(true);
    setPreviewStatus("starting");
    try {
      const result = await startRuntimePreview(snapshot.projectPath, snapshot.scene);
      applyPreviewResult(result, "Preview started with the current scene draft.");
    } catch (error) {
      setPreviewStatus(bridgeStatus === "browser" ? "unavailable" : "error");
      appendConsole({
        level: bridgeStatus === "browser" ? "warn" : "error",
        source: "preview",
        message:
          bridgeStatus === "browser"
            ? "Play mode needs the Tauri desktop app. Browser mode keeps the editor preview-only."
            : `Could not start preview: ${String(error)}`,
      });
    } finally {
      setPreviewBusy(false);
    }
  }

  async function pausePreview() {
    setPreviewBusy(true);
    try {
      const result = await pauseRuntimePreview();
      applyPreviewResult(result, "Preview paused.");
    } catch (error) {
      setPreviewStatus("error");
      appendConsole({
        level: "error",
        source: "preview",
        message: `Could not pause preview: ${String(error)}`,
      });
    } finally {
      setPreviewBusy(false);
    }
  }

  async function stopPreview() {
    setPreviewBusy(true);
    try {
      const result = await stopRuntimePreview();
      applyPreviewResult(result, "Preview stopped.");
    } catch (error) {
      setPreviewStatus("error");
      appendConsole({
        level: "error",
        source: "preview",
        message: `Could not stop preview: ${String(error)}`,
      });
    } finally {
      setPreviewBusy(false);
    }
  }

  function applyPreviewResult(result: PreviewCommandResult, fallbackMessage?: string) {
    const nextStatus = normalizePreviewStatus(result.status);
    setPreviewStatus(nextStatus);

    if (result.events.length === 0 && fallbackMessage) {
      appendConsole({
        level: "info",
        source: "preview",
        message: fallbackMessage,
      });
      return;
    }

    result.events.forEach((event) => {
      appendConsole(previewEventToConsoleLine(event, result.runner));
    });
  }

  useEffect(() => {
    if (bridgeStatus !== "tauri") return;
    if (!["starting", "loading", "ready", "playing", "paused"].includes(previewStatus)) return;

    const timer = window.setInterval(() => {
      pollRuntimePreview()
        .then((result) => {
          if (result.events.length > 0) {
            applyPreviewResult(result);
          } else {
            setPreviewStatus(normalizePreviewStatus(result.status));
          }
        })
        .catch(() => {
          setPreviewStatus("error");
        });
    }, 1200);

    return () => window.clearInterval(timer);
  }, [bridgeStatus, previewStatus]);

  function attachScript(path: string) {
    if (!selectedEntity) return;
    const withoutScript = selectedEntity.components.filter((component) => component.kind !== "script");
    updateSelectedEntity({
      components: [...withoutScript, { kind: "script", script: path }],
    });
    setSelectedScriptPath(path);
    appendConsole({
      level: "info",
      source: "studio",
      message: `Attached ${path} to ${selectedEntity.name}`,
    });
  }

  function openScriptEditor(path: string) {
    setSelectedScriptPath(path);
    setScriptEditorOpen(true);
  }

  function createEntityFromAsset(asset: ProjectAsset) {
    const count = snapshot.scene.entities.length + 1;
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

    updateScene((scene) => ({
      ...scene,
      entities: [...scene.entities, entity],
    }));
    setSelectedEntityId(entity.id);
    appendConsole({
      level: "info",
      source: "studio",
      message: `Added ${asset.path} to Scene View`,
    });
  }

  useEffect(() => {
    if (!assetDrag) return;
    const draggedAsset = assetDrag.asset;

    function onMove(event: globalThis.PointerEvent) {
      setAssetDrag((current) =>
        current
          ? {
              ...current,
              x: event.clientX,
              y: event.clientY,
            }
          : current,
      );
    }

    function onUp(event: globalThis.PointerEvent) {
      const target = document.elementFromPoint(event.clientX, event.clientY);
      if (target instanceof Element && target.closest(".scene-viewport")) {
        createEntityFromAsset(draggedAsset);
      }
      setAssetDrag(null);
    }

    window.addEventListener("pointermove", onMove);
    window.addEventListener("pointerup", onUp);
    window.addEventListener("pointercancel", onUp);

    return () => {
      window.removeEventListener("pointermove", onMove);
      window.removeEventListener("pointerup", onUp);
      window.removeEventListener("pointercancel", onUp);
    };
  }, [assetDrag?.asset.id]);

  function beginAssetDrag(asset: ProjectAsset, event: PointerEvent<HTMLElement>) {
    if (asset.kind === "script") return;
    setAssetDrag({
      asset,
      x: event.clientX,
      y: event.clientY,
    });
  }

  function startResize(axis: "left" | "right" | "bottom", event: PointerEvent<HTMLDivElement>) {
    event.preventDefault();
    const startX = event.clientX;
    const startY = event.clientY;
    const initial = layout[axis];

    function onMove(moveEvent: globalThis.PointerEvent) {
      const delta = axis === "bottom" ? startY - moveEvent.clientY : moveEvent.clientX - startX;
      setLayout((current) => {
        if (axis === "left") return { ...current, left: clamp(initial + delta, 220, 420) };
        if (axis === "right") return { ...current, right: clamp(initial - delta, 240, 420) };
        return { ...current, bottom: clamp(initial + delta, 156, 320) };
      });
    }

    function onUp() {
      window.removeEventListener("pointermove", onMove);
      window.removeEventListener("pointerup", onUp);
    }

    window.addEventListener("pointermove", onMove);
    window.addEventListener("pointerup", onUp);
  }

  return (
    <div
      className={`studio-shell theme-${preferences.theme}`}
      style={
        {
          "--left-width": `${layout.left}px`,
          "--right-width": `${layout.right}px`,
          "--bottom-height": `${layout.bottom}px`,
        } as React.CSSProperties
      }
    >
      <MenuBar
        bridgeStatus={bridgeStatus}
        projectName={snapshot.projectName}
        onOpenDefaultProject={openDefaultProject}
        onOpenSettings={() => setSettingsOpen(true)}
        onShowHome={() => setHomeVisible(true)}
      />

      {homeVisible ? (
        <StartScreen
          bridgeStatus={bridgeStatus}
          busy={homeBusy}
          error={homeError}
          home={home}
          onCreateProject={createProject}
          onOpenDefaultProject={openDefaultProject}
          onOpenProject={openProject}
        />
      ) : (
        <div className="studio-workspace">
          <NavigatorPanel
            assets={snapshot.assets}
            entities={snapshot.scene.entities}
            selectedEntityId={selectedEntityId}
            onSelectEntity={selectEntity}
          />

        <div className="resize-handle resize-handle-vertical resize-handle-left" onPointerDown={(event) => startResize("left", event)} />

        <main className="stage-panel">
          <StageToolbar
            mode={mode}
            previewBusy={previewBusy}
            previewStatus={previewStatus}
            sceneDirty={sceneDirty}
            savingScene={savingScene}
            viewMode={viewMode}
            selectedEntity={selectedEntity}
            onModeChange={setMode}
            onPausePreview={pausePreview}
            onSaveScene={saveScene}
            onStartPreview={startPreview}
            onStopPreview={stopPreview}
            onViewModeChange={setViewMode}
          />

          <div className="stage-content">
            <section className="viewport-wrap">
              <SceneViewport
                entities={snapshot.scene.entities}
                scene={snapshot.scene}
                mode={mode}
                viewMode={viewMode}
                selectedEntityId={selectedEntityId}
                preferences={preferences}
                dragState={dragState}
                onAssetDrop={createEntityFromAsset}
                onDragStateChange={setDragState}
                onClearSelection={clearSelection}
                onSelectEntity={selectEntity}
                onUpdateTransform={(id, transform) => {
                  updateScene((scene) => ({
                    ...scene,
                    entities: scene.entities.map((entity) => (entity.id === id ? { ...entity, transform } : entity)),
                  }));
                }}
              />
            </section>
          </div>
        </main>

        <div className="resize-handle resize-handle-vertical resize-handle-right" onPointerDown={(event) => startResize("right", event)} />

        <InspectorPanel
          assets={snapshot.assets}
          entity={selectedEntity}
          scene={snapshot.scene}
          scripts={snapshot.scripts}
          onAttachScript={attachScript}
          onComponentChange={(index, component) => {
            if (!selectedEntity) return;
            updateSelectedEntity({
              components: selectedEntity.components.map((current, currentIndex) =>
                currentIndex === index ? component : current,
              ),
            });
          }}
          onEntityChange={updateSelectedEntity}
          onSceneChange={(patch) => updateScene((scene) => ({ ...scene, ...patch }))}
          onTransformChange={updateSelectedTransform}
        />

        <div className="resize-handle resize-handle-horizontal" onPointerDown={(event) => startResize("bottom", event)} />

        <BottomDock
          activeTab={bottomTab}
          assets={snapshot.assets}
          console={snapshot.console}
          scripts={snapshot.scripts}
          onAssetDrop={createEntityFromAsset}
          onAssetPointerDown={beginAssetDrag}
          onOpenScript={openScriptEditor}
          onScriptSelect={setSelectedScriptPath}
          onTabChange={setBottomTab}
        />
        </div>
      )}

      {!homeVisible && assetDrag ? <AssetDragPreview asset={assetDrag.asset} x={assetDrag.x} y={assetDrag.y} /> : null}
      {!homeVisible && scriptEditorOpen ? (
        <ScriptEditorWindow
          activeScript={activeScript}
          scripts={snapshot.scripts}
          selectedEntity={selectedEntity}
          onAttachScript={attachScript}
          onClose={() => setScriptEditorOpen(false)}
          onSaveScript={saveScript}
          onScriptChange={updateScriptContent}
          onScriptSelect={setSelectedScriptPath}
        />
      ) : null}
      {settingsOpen ? (
        <SettingsWindow
          preferences={preferences}
          onClose={() => setSettingsOpen(false)}
          onPreferencesChange={(patch) => setPreferences((current) => ({ ...current, ...patch }))}
        />
      ) : null}
    </div>
  );
}

function MenuBar({
  bridgeStatus,
  projectName,
  onOpenDefaultProject,
  onOpenSettings,
  onShowHome,
}: {
  bridgeStatus: BridgeStatus;
  projectName: string;
  onOpenDefaultProject: () => void;
  onOpenSettings: () => void;
  onShowHome: () => void;
}) {
  const menus = [
    { label: "Start", onClick: onShowHome },
    { label: "Open", onClick: () => void onOpenDefaultProject() },
    { label: "Edit", onClick: onOpenSettings },
    { label: "Assets" },
    { label: "GameObject" },
    { label: "Component" },
    { label: "Window" },
    { label: "Help", onClick: onShowHome },
  ];

  return (
    <nav aria-label="Application menu" className="menu-bar visible" data-tauri-drag-region>
      <div className="menu-items">
        {menus.map((menu) => (
          <button key={menu.label} onClick={menu.onClick}>
            {menu.label}
          </button>
        ))}
      </div>
      <span className="menu-project">{projectName}</span>
      <StatusPill tone={bridgeStatus === "tauri" ? "good" : "warn"}>
        {bridgeStatus === "tauri" ? "Tauri" : "Browser"}
      </StatusPill>
    </nav>
  );
}

function StartScreen({
  bridgeStatus,
  busy,
  error,
  home,
  onCreateProject,
  onOpenDefaultProject,
  onOpenProject,
}: {
  bridgeStatus: BridgeStatus;
  busy: boolean;
  error: string | null;
  home: StudioHome;
  onCreateProject: (request: NewProjectRequest) => void;
  onOpenDefaultProject: () => void;
  onOpenProject: (projectPath: string) => void;
}) {
  const firstTemplate: ProjectTemplate = home.templates[0] ?? {
    id: "empty3d",
    name: "Empty 3D",
    summary: "Cena 3D limpa com camera e cubo.",
    defaultName: "Projeto Borealis 3D",
    tags: ["3D", "starter"],
  };
  const [projectPath, setProjectPath] = useState(home.defaultProjectPath);
  const [projectName, setProjectName] = useState(firstTemplate.defaultName);
  const [parentDir, setParentDir] = useState(home.defaultProjectsDir);
  const [templateId, setTemplateId] = useState<ProjectTemplateId>(firstTemplate.id);
  const selectedTemplate = home.templates.find((template) => template.id === templateId) ?? firstTemplate;
  const runtimeTone = home.runtimeMode === "missing" ? "warn" : "good";
  const runtimeLabel = home.runtimeMode === "sdk" ? "SDK" : home.runtimeMode === "repo-dev" ? "Dev repo" : "Runtime";

  useEffect(() => {
    setProjectPath(home.defaultProjectPath);
    setParentDir(home.defaultProjectsDir);
  }, [home.defaultProjectPath, home.defaultProjectsDir]);

  useEffect(() => {
    if (!home.templates.some((template) => template.id === templateId)) {
      setTemplateId(firstTemplate.id);
      setProjectName((current) => (current.trim() ? current : firstTemplate.defaultName));
    }
  }, [firstTemplate.defaultName, firstTemplate.id, home.templates, templateId]);

  function chooseTemplate(template: ProjectTemplate) {
    setTemplateId(template.id);
    setProjectName((current) => (current.trim() ? current : template.defaultName));
  }

  return (
    <main className="start-screen">
      <section className="start-hero">
        <div className="start-kicker">
          <Home size={15} strokeWidth={ICON_STROKE} />
          Borealis Studio
        </div>
        <h1>Comece por um projeto real.</h1>
        <p>
          Abra um zenith.ztproj, crie um projeto novo ou use o pacote Borealis como amostra. O preview
          precisa desse caminho correto para iniciar o runner.
        </p>
        <div className="start-actions">
          <button className="start-primary" disabled={busy} onClick={() => void onOpenDefaultProject()} type="button">
            <FolderOpen size={15} strokeWidth={ICON_STROKE} />
            Abrir Borealis
          </button>
          <button className="start-secondary" onClick={() => setProjectPath(home.defaultProjectPath)} type="button">
            Usar caminho padrao
          </button>
        </div>
        <div className="start-meta">
          <StatusPill tone={bridgeStatus === "tauri" ? "good" : "warn"}>
            {bridgeStatus === "tauri" ? "Ponte desktop pronta" : "Fallback browser"}
          </StatusPill>
          <StatusPill tone={runtimeTone}>{runtimeLabel}</StatusPill>
          <span>{home.workspaceRoot}</span>
        </div>
        <div className="start-runtime">
          <span>{home.runtimeStatus}</span>
          {home.sdkRoot ? <small>SDK: {home.sdkRoot}</small> : null}
          {!home.sdkRoot && home.repoRoot ? <small>Repo: {home.repoRoot}</small> : null}
          {home.editorManifest.source ? <small>Editor manifest: {home.editorManifest.source}</small> : null}
        </div>
      </section>

      <section className="start-grid">
        <form
          className="start-panel start-open-panel"
          onSubmit={(event) => {
            event.preventDefault();
            void onOpenProject(projectPath);
          }}
        >
          <div className="start-panel-title">
            <FolderOpen size={16} strokeWidth={ICON_STROKE} />
            <div>
              <strong>Abrir projeto</strong>
              <span>Arquivo zenith.ztproj ou pasta de projeto.</span>
            </div>
          </div>
          <label className="start-field">
            <span>Caminho</span>
            <input value={projectPath} onChange={(event) => setProjectPath(event.target.value)} />
          </label>
          <button className="start-panel-action" disabled={busy} type="submit">
            Abrir
          </button>
        </form>

        <form
          className="start-panel start-new-panel"
          onSubmit={(event) => {
            event.preventDefault();
            void onCreateProject({ projectName, parentDir, templateId });
          }}
        >
          <div className="start-panel-title">
            <FilePlus2 size={16} strokeWidth={ICON_STROKE} />
            <div>
              <strong>Novo projeto</strong>
              <span>Cria pastas, manifesto, cena e script inicial.</span>
            </div>
          </div>
          <label className="start-field">
            <span>Nome</span>
            <input value={projectName} onChange={(event) => setProjectName(event.target.value)} />
          </label>
          <label className="start-field">
            <span>Pasta</span>
            <input value={parentDir} onChange={(event) => setParentDir(event.target.value)} />
          </label>
          <label className="start-field">
            <span>Template</span>
            <select value={templateId} onChange={(event) => setTemplateId(event.target.value as ProjectTemplateId)}>
              {home.templates.map((template) => (
                <option key={template.id} value={template.id}>
                  {template.name}
                </option>
              ))}
            </select>
          </label>
          <p>{selectedTemplate.summary}</p>
          <button className="start-panel-action" disabled={busy} type="submit">
            Criar e abrir
          </button>
        </form>

        <section className="start-panel start-template-panel">
          <div className="start-panel-title">
            <LayoutTemplate size={16} strokeWidth={ICON_STROKE} />
            <div>
              <strong>Templates</strong>
              <span>Escolha a base antes de criar.</span>
            </div>
          </div>
          <div className="template-list">
            {home.templates.map((template) => (
              <button
                className={template.id === templateId ? "selected" : ""}
                key={template.id}
                onClick={() => chooseTemplate(template)}
                type="button"
              >
                <strong>{template.name}</strong>
                <span>{template.summary}</span>
                <small>{template.tags.join(" / ")}</small>
              </button>
            ))}
          </div>
        </section>

        <section className="start-panel start-docs-panel">
          <div className="start-panel-title">
            <BookOpen size={16} strokeWidth={ICON_STROKE} />
            <div>
              <strong>Documentacao</strong>
              <span>Arquivos locais para consulta rapida.</span>
            </div>
          </div>
          <div className="docs-list">
            {home.docs.map((doc) => (
              <DocumentationRow doc={doc} key={doc.path} />
            ))}
          </div>
        </section>
      </section>

      {error ? (
        <div className="start-error" role="alert">
          {error}
        </div>
      ) : null}
    </main>
  );
}

function DocumentationRow({ doc }: { doc: DocumentationLink }) {
  return (
    <div className="doc-row">
      <strong>{doc.title}</strong>
      <span>{doc.summary}</span>
      <code>{doc.path}</code>
    </div>
  );
}

function SettingsWindow({
  preferences,
  onClose,
  onPreferencesChange,
}: {
  preferences: StudioPreferences;
  onClose: () => void;
  onPreferencesChange: (patch: Partial<StudioPreferences>) => void;
}) {
  const keybindRows = keybindsForTemplate(preferences.shortcutTemplate);

  return (
    <div className="settings-backdrop" role="presentation">
      <motion.section
        animate={{ opacity: 1, scale: 1 }}
        className="settings-window"
        initial={{ opacity: 0, scale: 0.96 }}
        transition={{ duration: 0.14, ease: [0.16, 1, 0.3, 1] }}
      >
        <header className="settings-header">
          <div>
            <span>Preferences</span>
            <strong>Editor settings</strong>
          </div>
          <button className="top-icon-button" onClick={onClose} title="Close" type="button">
            <X size={15} strokeWidth={ICON_STROKE} />
          </button>
        </header>

        <div className="settings-grid">
          <section className="settings-section">
            <h3>Viewport</h3>
            <label>
              <span>Gizmo size</span>
              <input
                max={1.8}
                min={0.35}
                onChange={(event) => onPreferencesChange({ gizmoSize: Number(event.target.value) })}
                step={0.05}
                type="range"
                value={preferences.gizmoSize}
              />
              <strong>{preferences.gizmoSize.toFixed(2)}</strong>
            </label>
            <label>
              <span>PTZ speed</span>
              <input
                max={2.4}
                min={0.25}
                onChange={(event) => onPreferencesChange({ ptzSpeed: Number(event.target.value) })}
                step={0.05}
                type="range"
                value={preferences.ptzSpeed}
              />
              <strong>{preferences.ptzSpeed.toFixed(2)}</strong>
            </label>
            <label>
              <span>Default snap</span>
              <select
                onChange={(event) => onPreferencesChange({ snapMode: event.target.value as SnapMode })}
                value={preferences.snapMode}
              >
                <option value="grid">Grid</option>
                <option value="object">Object</option>
                <option value="grid-object">Grid + object</option>
              </select>
            </label>
            <label>
              <span>Theme</span>
              <select
                onChange={(event) => onPreferencesChange({ theme: event.target.value as ThemeMode })}
                value={preferences.theme}
              >
                <option value="codex">Codex</option>
                <option value="xcode">Xcode</option>
                <option value="unity-dark">Unity Dark</option>
              </select>
            </label>
          </section>

          <section className="settings-section">
            <h3>Keybinds</h3>
            <label>
              <span>Template</span>
              <select
                onChange={(event) => onPreferencesChange({ shortcutTemplate: event.target.value as ShortcutTemplate })}
                value={preferences.shortcutTemplate}
              >
                <option value="blender">Blender</option>
                <option value="3dsmax">3ds Max</option>
                <option value="maya">Maya</option>
              </select>
            </label>
            <div className="keybind-list">
              {keybindRows.map((row) => (
                <div className="keybind-row" key={`${row.action}-${row.keys}`}>
                  <span>{row.action}</span>
                  <kbd>{row.keys}</kbd>
                </div>
              ))}
            </div>
          </section>
        </div>
      </motion.section>
    </div>
  );
}

function keybindsForTemplate(template: ShortcutTemplate) {
  if (template === "3dsmax") {
    return [
      { action: "Orbit", keys: "Alt + MMB" },
      { action: "Pan", keys: "MMB" },
      { action: "Zoom", keys: "Wheel" },
      { action: "Move", keys: "W" },
      { action: "Rotate", keys: "E" },
      { action: "Scale", keys: "R" },
    ];
  }

  if (template === "maya") {
    return [
      { action: "Orbit", keys: "Alt + LMB" },
      { action: "Pan", keys: "Alt + MMB" },
      { action: "Zoom", keys: "Alt + RMB" },
      { action: "Move", keys: "W" },
      { action: "Rotate", keys: "E" },
      { action: "Scale", keys: "R" },
    ];
  }

  return [
    { action: "Orbit", keys: "MMB" },
    { action: "Pan", keys: "Shift + MMB" },
    { action: "Zoom", keys: "Wheel" },
    { action: "Front", keys: "Numpad 1" },
    { action: "Right", keys: "Numpad 3" },
    { action: "Top", keys: "Numpad 7" },
    { action: "Pie view", keys: "Alt + Z" },
  ];
}

function modeForShortcut(template: ShortcutTemplate, key: string): StudioMode | null {
  if (template === "blender") {
    if (key === "q") return "select";
    if (key === "g") return "move";
    if (key === "r") return "rotate";
    if (key === "s") return "scale";
  }

  if (key === "q") return "select";
  if (key === "w") return "move";
  if (key === "e") return "rotate";
  if (key === "r") return "scale";
  return null;
}

function CommandSearchButton() {
  const [searchOpen, setSearchOpen] = useState(false);
  const searchRef = useRef<HTMLDivElement | null>(null);

  useEffect(() => {
    function handlePointerDown(event: MouseEvent) {
      if (!searchRef.current?.contains(event.target as Node)) setSearchOpen(false);
    }

    function handleKeyDown(event: KeyboardEvent) {
      if (event.key === "Escape") setSearchOpen(false);
    }

    window.addEventListener("pointerdown", handlePointerDown);
    window.addEventListener("keydown", handleKeyDown);
    return () => {
      window.removeEventListener("pointerdown", handlePointerDown);
      window.removeEventListener("keydown", handleKeyDown);
    };
  }, []);

  return (
    <div
      className="search-popover"
      ref={searchRef}
      onKeyDown={(event) => {
        if (event.key === "Escape") setSearchOpen(false);
      }}
    >
      <button
        aria-expanded={searchOpen}
        aria-label="Open command search"
        className={`top-icon-button ${searchOpen ? "active" : ""}`}
        onClick={() => setSearchOpen((open) => !open)}
        title="Search"
      >
        <Search size={15} strokeWidth={ICON_STROKE} />
      </button>
      <AnimatePresence>
        {searchOpen ? (
          <>
            <button
              aria-label="Close command search"
              className="command-popover-backdrop"
              onPointerDown={() => setSearchOpen(false)}
              tabIndex={-1}
              type="button"
            />
            <motion.div
              animate={{ opacity: 1, scale: 1, y: 0 }}
              className="command-popover"
              exit={{ opacity: 0, scale: 0.985, y: -4 }}
              initial={{ opacity: 0, scale: 0.985, y: -4 }}
              role="dialog"
              aria-label="Command search"
              transition={{ duration: 0.14, ease: [0.16, 1, 0.3, 1] }}
            >
              <div className="command-input">
                <Search size={15} strokeWidth={ICON_STROKE} />
                <input
                  autoFocus
                  onKeyDown={(event) => {
                    if (event.key === "Escape") setSearchOpen(false);
                  }}
                  placeholder="Search files, entities, commands"
                />
                <Command size={14} strokeWidth={ICON_STROKE} />
              </div>
              <div className="command-suggestions">
                <button>Open player_controller.zt</button>
                <button>Select Player entity</button>
                <button>Show project assets</button>
              </div>
            </motion.div>
          </>
        ) : null}
      </AnimatePresence>
    </div>
  );
}

function NavigatorPanel({
  assets,
  entities,
  selectedEntityId,
  onSelectEntity,
}: {
  assets: ProjectAsset[];
  entities: SceneEntity[];
  selectedEntityId: string;
  onSelectEntity: (entity: SceneEntity) => void;
}) {
  const sceneAsset = assets.find((asset) => asset.kind === "scene");

  return (
    <aside className="side-panel navigator-panel">
      <PanelHeader icon={<FolderTree size={13} strokeWidth={ICON_STROKE} />} title="Navigator" meta="Scene" />
      <div className="navigator-section scene-tree">
        {sceneAsset ? (
          <div className="tree-row scene-root">
            {assetKindIcon(sceneAsset.kind, 12)}
            <span>{sceneAsset.name}</span>
            <small>scene</small>
          </div>
        ) : null}
        {entities.map((entity) => (
          <button
            className={`tree-row scene-child ${selectedEntityId === entity.id ? "selected" : ""}`}
            key={entity.id}
            onClick={() => onSelectEntity(entity)}
          >
            {entityIcon(entity)}
            <span>{entity.name}</span>
            <small>{entity.layer}</small>
          </button>
        ))}
      </div>
    </aside>
  );
}

function StageToolbar({
  mode,
  previewBusy,
  previewStatus,
  sceneDirty,
  selectedEntity,
  savingScene,
  viewMode,
  onModeChange,
  onPausePreview,
  onSaveScene,
  onStartPreview,
  onStopPreview,
  onViewModeChange,
}: {
  mode: StudioMode;
  previewBusy: boolean;
  previewStatus: PreviewStatus;
  sceneDirty: boolean;
  selectedEntity: SceneEntity | null;
  savingScene: boolean;
  viewMode: ViewMode;
  onModeChange: (mode: StudioMode) => void;
  onPausePreview: () => void;
  onSaveScene: () => void;
  onStartPreview: () => void;
  onStopPreview: () => void;
  onViewModeChange: (mode: ViewMode) => void;
}) {
  const canPausePreview = ["playing", "ready", "paused"].includes(previewStatus);
  const canStopPreview = !["idle", "stopped", "unavailable"].includes(previewStatus);

  return (
    <div className="stage-toolbar">
      <Segmented tabs={["2d", "3d"]} value={viewMode} onChange={onViewModeChange} />
      <div className="viewport-control-center">
        <CommandSearchButton />
        <div className="tool-strip" aria-label="Transform tools">
          <ToolButton active={mode === "select"} label="Select" onClick={() => onModeChange("select")}>
            <MousePointer2 size={15} strokeWidth={ICON_STROKE} />
          </ToolButton>
          <ToolButton active={mode === "move"} label="Move" onClick={() => onModeChange("move")}>
            <Move3D size={15} strokeWidth={ICON_STROKE} />
          </ToolButton>
          <ToolButton active={mode === "rotate"} label="Rotate" onClick={() => onModeChange("rotate")}>
            <RotateCcw size={15} strokeWidth={ICON_STROKE} />
          </ToolButton>
          <ToolButton active={mode === "scale"} label="Scale" onClick={() => onModeChange("scale")}>
            <Scale3D size={15} strokeWidth={ICON_STROKE} />
          </ToolButton>
        </div>
        <div className="transport-controls" aria-label="Preview controls">
          <ToolButton
            active={["starting", "loading", "ready", "playing"].includes(previewStatus)}
            disabled={previewBusy}
            label="Play Preview"
            onClick={onStartPreview}
          >
            <Play size={15} strokeWidth={ICON_STROKE} />
          </ToolButton>
          <ToolButton
            active={previewStatus === "paused"}
            disabled={previewBusy || !canPausePreview}
            label="Pause Preview"
            onClick={onPausePreview}
          >
            <Pause size={15} strokeWidth={ICON_STROKE} />
          </ToolButton>
          <ToolButton
            active={previewStatus === "stopped"}
            disabled={previewBusy || !canStopPreview}
            label="Stop Preview"
            onClick={onStopPreview}
          >
            <Square size={14} strokeWidth={ICON_STROKE} />
          </ToolButton>
        </div>
      </div>
      <div className="stage-status">
        <button className="panel-action stage-save-action" disabled={savingScene} onClick={onSaveScene} title="Save scene">
          <Save size={14} strokeWidth={ICON_STROKE} />
          {savingScene ? "Saving" : "Save Scene"}
        </button>
        <StatusPill>{mode}</StatusPill>
        <StatusPill>{selectedEntity ? selectedEntity.name : "No selection"}</StatusPill>
        <StatusPill tone={sceneDirty ? "warn" : "good"}>{sceneDirty ? "Unsaved scene" : "Scene saved"}</StatusPill>
        <StatusPill tone={previewStatusTone(previewStatus)}>{previewStatusLabel(previewStatus)}</StatusPill>
      </div>
    </div>
  );
}

function SceneViewport({
  entities,
  scene,
  mode,
  viewMode,
  selectedEntityId,
  preferences,
  dragState,
  onAssetDrop,
  onClearSelection,
  onDragStateChange,
  onSelectEntity,
  onUpdateTransform,
}: {
  entities: SceneEntity[];
  scene: SceneDocument;
  mode: StudioMode;
  viewMode: ViewMode;
  selectedEntityId: string;
  preferences: StudioPreferences;
  dragState: DragState | null;
  onAssetDrop: (asset: ProjectAsset) => void;
  onClearSelection: () => void;
  onDragStateChange: (state: DragState | null) => void;
  onSelectEntity: (entity: SceneEntity) => void;
  onUpdateTransform: (id: string, transform: Transform3D) => void;
}) {
  const viewportRef = useRef<HTMLDivElement | null>(null);
  const [viewportCameras, setViewportCameras] = useState<Record<ViewMode, ViewportCamera>>({
    "2d": { panX: 0, panY: 0, zoom: 1 },
    "3d": { panX: 0, panY: 0, zoom: 1 },
  });
  const [viewportPanState, setViewportPanState] = useState<ViewportPanState | null>(null);
  const activeCamera = viewportCameras[viewMode];

  function handleEntityPointerDown(entity: SceneEntity, event: PointerEvent<HTMLButtonElement>) {
    event.preventDefault();
    event.stopPropagation();
    event.currentTarget.setPointerCapture(event.pointerId);
    onSelectEntity(entity);
    if (mode !== "move") return;
    onDragStateChange({
      id: entity.id,
      startX: event.clientX,
      startY: event.clientY,
      originX: entity.transform.x,
      originY: entity.transform.y,
    });
  }

  function handleEntityPointerMove(entity: SceneEntity, event: PointerEvent<HTMLButtonElement>) {
    if (!dragState || dragState.id !== entity.id || mode !== "move") return;
    const deltaX = event.clientX - dragState.startX;
    const deltaY = event.clientY - dragState.startY;
    onUpdateTransform(entity.id, {
      ...entity.transform,
      x: Math.round(dragState.originX + deltaX * 0.75 / activeCamera.zoom),
      y: Math.round(dragState.originY + deltaY * 0.75 / activeCamera.zoom),
    });
  }

  function updateActiveCamera(updater: (camera: ViewportCamera) => ViewportCamera) {
    setViewportCameras((current) => ({
      ...current,
      [viewMode]: updater(current[viewMode]),
    }));
  }

  function handleViewportWheel(event: React.WheelEvent<HTMLDivElement>) {
    event.preventDefault();
    if (event.ctrlKey || event.metaKey) {
      const direction = event.deltaY < 0 ? 1 : -1;
      updateActiveCamera((camera) => ({
        ...camera,
        zoom: clamp(Number((camera.zoom + direction * 0.08).toFixed(2)), 0.45, 2.4),
      }));
    } else {
      updateActiveCamera((camera) => ({
        ...camera,
        panX: Math.round(camera.panX - event.deltaX / camera.zoom),
        panY: Math.round(camera.panY - event.deltaY / camera.zoom),
      }));
    }
  }

  function handleViewportPointerDown(event: PointerEvent<HTMLDivElement>) {
    if (event.button !== 1 && !event.altKey) return;
    event.preventDefault();
    event.currentTarget.setPointerCapture(event.pointerId);
    setViewportPanState({
      pointerId: event.pointerId,
      startX: event.clientX,
      startY: event.clientY,
      originPanX: activeCamera.panX,
      originPanY: activeCamera.panY,
    });
  }

  function handleViewportPointerMove(event: PointerEvent<HTMLDivElement>) {
    if (!viewportPanState || viewportPanState.pointerId !== event.pointerId) return;
    const deltaX = (event.clientX - viewportPanState.startX) / activeCamera.zoom;
    const deltaY = (event.clientY - viewportPanState.startY) / activeCamera.zoom;
    updateActiveCamera((camera) => ({
      ...camera,
      panX: Math.round(viewportPanState.originPanX + deltaX),
      panY: Math.round(viewportPanState.originPanY + deltaY),
    }));
  }

  function resetActiveCamera() {
    updateActiveCamera(() => ({ panX: 0, panY: 0, zoom: 1 }));
  }

  function handleDrop(event: React.DragEvent<HTMLDivElement>) {
    event.preventDefault();
    const raw = event.dataTransfer.getData("application/x-borealis-asset");
    if (!raw) return;
    onAssetDrop(JSON.parse(raw) as ProjectAsset);
  }

  const viewportProps: ViewportRendererProps = {
    camera: activeCamera,
    dragState,
    entities,
    mode,
    preferences,
    selectedEntityId,
    onDragStateChange,
    onEntityPointerDown: handleEntityPointerDown,
    onEntityPointerMove: handleEntityPointerMove,
    onResetCamera: resetActiveCamera,
    onClearSelection,
    onSelectEntity,
    onUpdateTransform,
  };

  const viewport2DHandlers =
    viewMode === "2d"
      ? {
          onPointerDown: handleViewportPointerDown,
          onPointerMove: handleViewportPointerMove,
          onPointerUp: () => setViewportPanState(null),
          onPointerCancel: () => setViewportPanState(null),
          onWheel: handleViewportWheel,
        }
      : {};

  return (
    <div
      className={`scene-viewport viewport-${viewMode}`}
      onDragOver={(event) => event.preventDefault()}
      onDrop={handleDrop}
      ref={viewportRef}
      {...viewport2DHandlers}
      style={
        {
          "--viewport-pan-x": `${activeCamera.panX * activeCamera.zoom}px`,
          "--viewport-pan-y": `${activeCamera.panY * activeCamera.zoom}px`,
          "--viewport-zoom": activeCamera.zoom,
        } as React.CSSProperties
      }
    >
      {viewMode === "3d" ? <Viewport3D {...viewportProps} scene={scene} /> : <Viewport2D {...viewportProps} />}
    </div>
  );
}

interface ViewportRendererProps {
  camera: ViewportCamera;
  dragState: DragState | null;
  entities: SceneEntity[];
  mode: StudioMode;
  preferences: StudioPreferences;
  selectedEntityId: string;
  onDragStateChange: (state: DragState | null) => void;
  onEntityPointerDown: (entity: SceneEntity, event: PointerEvent<HTMLButtonElement>) => void;
  onEntityPointerMove: (entity: SceneEntity, event: PointerEvent<HTMLButtonElement>) => void;
  onResetCamera: () => void;
  onClearSelection: () => void;
  onSelectEntity: (entity: SceneEntity) => void;
  onUpdateTransform: (id: string, transform: Transform3D) => void;
}

function Viewport3D({
  entities,
  mode,
  preferences,
  scene,
  selectedEntityId,
  onClearSelection,
  onResetCamera,
  onSelectEntity,
  onUpdateTransform,
}: ViewportRendererProps & { scene: SceneDocument }) {
  const orbitRef = useRef<any>(null);
  const [projection, setProjection] = useState<ViewProjection>("perspective");
  const [orientation, setOrientation] = useState<ViewOrientation>("free");
  const [renderMode, setRenderMode] = useState<RenderMode>("light");
  const [radialOpen, setRadialOpen] = useState(false);
  const [gridEditing, setGridEditing] = useState(false);
  const [gridDraft, setGridDraft] = useState("25");
  const [transforming, setTransforming] = useState(false);
  const [snapSettings, setSnapSettings] = useState<SnapSettings>({
    gridSize: 1,
    snapToGrid: preferences.snapMode === "grid" || preferences.snapMode === "grid-object",
    snapToObject: preferences.snapMode === "object" || preferences.snapMode === "grid-object",
  });
  const sceneBackground = sceneColorValue(scene.environment.skybox, "color", "#1c1f26");
  const ambientColor = sceneColorValue(scene.environment.ambient, "color", "#ffffff");
  const ambientIntensity = sceneNumberValue(scene.environment.ambient, "intensity", 0.4);
  const fogEnabled = sceneBooleanValue(scene.environment.fog, "enabled", false);
  const fogColor = sceneColorValue(scene.environment.fog, "color", "#9ca3af");
  const fogDensity = sceneNumberValue(scene.environment.fog, "density", 0.05);
  const postfx = scene.render.postfx ?? {};
  const postfxEnabled =
    sceneBooleanValue(postfx, "fxaa", false) ||
    sceneNumberValue(postfx, "bloom", 0) > 0 ||
    sceneNumberValue(postfx, "vignette", 0) > 0;
  const transformMode = mode === "rotate" ? "rotate" : mode === "scale" ? "scale" : "translate";
  const cameraMode = projection === "isometric" ? "isometric" : projection;

  useEffect(() => {
    setSnapSettings((current) => ({
      ...current,
      snapToGrid: preferences.snapMode === "grid" || preferences.snapMode === "grid-object",
      snapToObject: preferences.snapMode === "object" || preferences.snapMode === "grid-object",
    }));
  }, [preferences.snapMode]);

  useEffect(() => {
    function handleViewportMenu(event: KeyboardEvent) {
      const target = event.target;
      if (
        target instanceof HTMLInputElement ||
        target instanceof HTMLTextAreaElement ||
        target instanceof HTMLSelectElement ||
        (target instanceof HTMLElement && target.isContentEditable)
      ) {
        return;
      }

      if (event.altKey && event.key.toLowerCase() === "z") {
        event.preventDefault();
        setRadialOpen((current) => !current);
      }

      if (radialOpen) {
        const selectedView = VIEW_ORIENTATIONS.find((item) => item.shortcut.toLowerCase() === event.key.toLowerCase());
        if (selectedView) {
          event.preventDefault();
          selectOrientation(selectedView.id);
        }
      }

      if (event.code.startsWith("Numpad")) {
        const nextOrientation = numpadOrientation(event.code, event.ctrlKey);
        if (nextOrientation) {
          event.preventDefault();
          selectOrientation(nextOrientation);
        }

        if (event.code === "Numpad5") {
          event.preventDefault();
          setProjection((current) => (current === "orthographic" ? "perspective" : "orthographic"));
        }

        if (event.code === "Numpad9") {
          event.preventDefault();
          setOrientation((current) => oppositeOrientation(current));
        }
      }

      if (event.key === "Escape") {
        setRadialOpen(false);
      }
    }

    window.addEventListener("keydown", handleViewportMenu);
    return () => window.removeEventListener("keydown", handleViewportMenu);
  }, [radialOpen]);

  function selectProjection(nextProjection: ViewProjection) {
    setProjection(nextProjection);
    if (nextProjection === "isometric") {
      setOrientation("free");
    }
  }

  function selectOrientation(nextOrientation: ViewOrientation) {
    setOrientation(nextOrientation);
    setRadialOpen(false);
  }

  function updateSnapSettings(patch: Partial<SnapSettings>) {
    setSnapSettings((current) => ({ ...current, ...patch }));
  }

  function commitGridSize() {
    const nextSize = clamp(Number(gridDraft) || snapSettings.gridSize, 0.25, 10);
    updateSnapSettings({ gridSize: nextSize });
    setGridDraft(String(nextSize));
    setGridEditing(false);
  }

  return (
    <>
      <Canvas className="viewport-canvas" onPointerMissed={onClearSelection} shadows>
        {projection === "perspective" ? (
          <PerspectiveCamera makeDefault position={[10, 8, 10]} fov={38} />
        ) : (
          <OrthographicCamera makeDefault position={[10, 8, 10]} zoom={58} near={0.1} far={1000} />
        )}
        <ViewportCameraController orientation={orientation} projection={projection} controlsRef={orbitRef} />
        <color attach="background" args={[sceneBackground]} />
        {fogEnabled ? <fog attach="fog" args={[fogColor, 4, Math.max(6, 38 - fogDensity * 24)]} /> : null}
        <ambientLight color={ambientColor} intensity={ambientIntensity} />
        <directionalLight castShadow intensity={0.8} position={[10, 12, 10]} />
        <Environment preset="city" />
        <ViewportSceneLights entities={entities} />
        <Grid
          args={[30, 30]}
          cellColor="#2c2c2c"
          cellSize={snapSettings.gridSize}
          fadeDistance={30}
          fadeStrength={1}
          sectionColor="#353535"
          sectionSize={snapSettings.gridSize * 5}
          infiniteGrid
        />
        <mesh rotation={[Math.PI / 2, 0, 0]} position={[0, -0.02, 0]}>
          <planeGeometry args={[60, 0.015]} />
          <meshBasicMaterial color="#df746c" transparent opacity={0.6} />
        </mesh>
        <mesh rotation={[Math.PI / 2, 0, Math.PI / 2]} position={[0, -0.02, 0]}>
          <planeGeometry args={[60, 0.015]} />
          <meshBasicMaterial color="#72c08b" transparent opacity={0.6} />
        </mesh>
        <group>
          {entities.map((entity) => {
            const selected = entity.id === selectedEntityId;
            return (
              <Viewport3DEntity
                entity={entity}
                key={entity.id}
                mode={mode}
                renderMode={renderMode}
                selected={selected}
                snapSettings={snapSettings}
                transformMode={transformMode}
                entities={entities}
                gizmoSize={preferences.gizmoSize}
                onSelect={onSelectEntity}
                onTransformingChange={setTransforming}
                onUpdateTransform={onUpdateTransform}
              />
            );
          })}
        </group>
        <OrbitControls
          ref={orbitRef}
          enabled={!transforming}
          enableDamping
          makeDefault
          maxDistance={40}
          minDistance={2}
          enablePan
          enableRotate
          enableZoom
          panSpeed={preferences.ptzSpeed}
          rotateSpeed={preferences.ptzSpeed}
          zoomSpeed={preferences.ptzSpeed}
          mouseButtons={{
            MIDDLE: MOUSE.ROTATE,
            RIGHT: MOUSE.PAN,
          }}
          screenSpacePanning
        />

        <GizmoHelper alignment="bottom-right" margin={[40, 40]}>
          <GizmoViewport axisColors={["#df746c", "#72c08b", "#68a4ff"]} labelColor="#efefef" />
        </GizmoHelper>
      </Canvas>
      <div className="viewport-overlay-top">
        <StatusPill>Scene 3D</StatusPill>
        <StatusPill>MMB orbit</StatusPill>
        <StatusPill>Shift+MMB pan</StatusPill>
        <StatusPill>{mode === "select" ? "Select" : transformMode}</StatusPill>
        <StatusPill>{cameraMode}</StatusPill>
        <StatusPill>{orientation === "free" ? "Free" : orientation}</StatusPill>
        {fogEnabled ? <StatusPill>Fog</StatusPill> : null}
        {postfxEnabled ? <StatusPill>PostFX</StatusPill> : null}
        <button className="viewport-reset" onClick={onResetCamera}>Reset</button>
      </div>
      <div className="viewport-view-controls" aria-label="3D viewport view controls">
        <label>
          <span>View</span>
          <select value={orientation} onChange={(event) => selectOrientation(event.target.value as ViewOrientation)}>
            <option value="free">Free</option>
            {VIEW_ORIENTATIONS.map((item) => (
              <option key={item.id} value={item.id}>
                {item.label}
              </option>
            ))}
          </select>
        </label>
        <div className="viewport-viewbar">
          {(["perspective", "orthographic", "isometric"] as ViewProjection[]).map((item) => (
            <button
              className={projection === item ? "active" : ""}
              key={item}
              onClick={() => selectProjection(item)}
              type="button"
            >
              {item === "orthographic" ? "Ortho" : item}
            </button>
          ))}
        </div>
        <label>
          <span>Render</span>
          <select value={renderMode} onChange={(event) => setRenderMode(event.target.value as RenderMode)}>
            <option value="wireframe">Wire</option>
            <option value="color">Color</option>
            <option value="texture">Texture</option>
            <option value="light">Light</option>
          </select>
        </label>
      </div>
      <div className="viewport-snap-panel">
        <label>
          <span>Grid m</span>
          <input
            max={10}
            min={0.25}
            onChange={(event) => {
              updateSnapSettings({ gridSize: Number(event.target.value) });
              setGridDraft(event.target.value);
            }}
            step={0.25}
            type="range"
            value={snapSettings.gridSize}
          />
          {gridEditing ? (
            <input
              className="grid-size-input"
              autoFocus
              onBlur={commitGridSize}
              onChange={(event) => setGridDraft(event.target.value)}
              onKeyDown={(event) => {
                if (event.key === "Enter") commitGridSize();
                if (event.key === "Escape") {
                  setGridDraft(String(snapSettings.gridSize));
                  setGridEditing(false);
                }
              }}
              type="number"
              value={gridDraft}
            />
          ) : (
            <strong onDoubleClick={() => setGridEditing(true)} title="Double-click to type grid size">
              {snapSettings.gridSize}
            </strong>
          )}
        </label>
        <button
          className={snapSettings.snapToGrid ? "active" : ""}
          onClick={() => updateSnapSettings({ snapToGrid: !snapSettings.snapToGrid })}
          type="button"
        >
          Snap grid
        </button>
        <button
          className={snapSettings.snapToObject ? "active" : ""}
          onClick={() => updateSnapSettings({ snapToObject: !snapSettings.snapToObject })}
          type="button"
        >
          Snap object
        </button>
      </div>
      <AnimatePresence>
        {radialOpen ? (
          <motion.div
            animate={{ opacity: 1, scale: 1, x: "-50%", y: "-50%" }}
            className="viewport-radial-menu"
            exit={{ opacity: 0, scale: 0.92, x: "-50%", y: "-50%" }}
            initial={{ opacity: 0, scale: 0.92, x: "-50%", y: "-50%" }}
            transition={{ duration: 0.14, ease: [0.16, 1, 0.3, 1] }}
          >
            <button className="radial-center" onClick={() => setRadialOpen(false)} type="button">
              Alt+Z
            </button>
            {VIEW_ORIENTATIONS.map((item, index) => {
              const angle = -90 + index * 60;
              return (
                <button
                  className={`radial-item ${orientation === item.id ? "active" : ""}`}
                  key={item.id}
                  onClick={() => selectOrientation(item.id)}
                  style={{ "--radial-angle": `${angle}deg` } as React.CSSProperties}
                  type="button"
                >
                  <span>{item.label}</span>
                  <small>{item.shortcut}</small>
                </button>
              );
            })}
          </motion.div>
        ) : null}
      </AnimatePresence>
      <div className="viewport-axis axis-x">X</div>
      <div className="viewport-axis axis-y">Y</div>
      <div className="viewport-axis axis-z">Z</div>
      <div className="viewport-footer">
        <span>Viewport 3D</span>
        <span>{entities.length} objects | Scene Collection</span>
      </div>
    </>
  );
}

function ViewportCameraController({
  controlsRef,
  orientation,
  projection,
}: {
  controlsRef: React.MutableRefObject<any>;
  orientation: ViewOrientation;
  projection: ViewProjection;
}) {
  const { camera } = useThree();

  useEffect(() => {
    const position = cameraPositionForView(projection, orientation);
    camera.position.copy(position);
    camera.up.set(0, 1, 0);
    if (orientation === "top") camera.up.set(0, 0, -1);
    if (orientation === "bottom") camera.up.set(0, 0, 1);
    camera.lookAt(0, 0, 0);
    camera.updateProjectionMatrix();

    const controls = controlsRef.current;
    if (controls) {
      controls.target.set(0, 0, 0);
      controls.update();
    }
  }, [camera, controlsRef, orientation, projection]);

  return null;
}

function cameraPositionForView(projection: ViewProjection, orientation: ViewOrientation) {
  if (projection === "isometric") return new Vector3(8, 7, 8);

  const distance = projection === "orthographic" ? 12 : 14;
  switch (orientation) {
    case "top":
      return new Vector3(0, distance, 0.001);
    case "bottom":
      return new Vector3(0, -distance, 0.001);
    case "left":
      return new Vector3(-distance, 0, 0);
    case "right":
      return new Vector3(distance, 0, 0);
    case "front":
      return new Vector3(0, 0, distance);
    case "back":
      return new Vector3(0, 0, -distance);
    case "free":
    default:
      return new Vector3(10, 8, 10);
  }
}

function numpadOrientation(code: string, ctrlKey: boolean): ViewOrientation | null {
  if (code === "Numpad1") return ctrlKey ? "back" : "front";
  if (code === "Numpad3") return ctrlKey ? "left" : "right";
  if (code === "Numpad7") return ctrlKey ? "bottom" : "top";
  return null;
}

function oppositeOrientation(orientation: ViewOrientation): ViewOrientation {
  if (orientation === "front") return "back";
  if (orientation === "back") return "front";
  if (orientation === "right") return "left";
  if (orientation === "left") return "right";
  if (orientation === "top") return "bottom";
  if (orientation === "bottom") return "top";
  return "back";
}

function ViewportSceneLights({ entities }: { entities: SceneEntity[] }) {
  return (
    <>
      {entities.flatMap((entity) =>
        entity.components
          .filter((component) => component.kind === "directional_light" || component.kind === "point_light" || component.kind === "spot_light")
          .map((component, index) => {
            const position: [number, number, number] = [
              entity.transform.x * 0.04,
              entity.transform.z * 0.04 + 0.45,
              entity.transform.y * 0.04,
            ];
            const color = String(componentValue(component, "color") ?? "#ffffff");
            const intensity = Number(componentValue(component, "intensity") ?? 1);

            if (component.kind === "directional_light") {
              return (
                <directionalLight
                  castShadow={componentValue(component, "shadow") === true}
                  color={color}
                  intensity={intensity}
                  key={`${entity.id}-${component.kind}-${index}`}
                  position={position}
                />
              );
            }

            if (component.kind === "spot_light") {
              return (
                <spotLight
                  angle={(Number(componentValue(component, "angle") ?? 45) * Math.PI) / 180}
                  color={color}
                  distance={Number(componentValue(component, "range") ?? 160) * 0.04}
                  intensity={intensity}
                  key={`${entity.id}-${component.kind}-${index}`}
                  penumbra={0.35}
                  position={position}
                />
              );
            }

            return (
              <pointLight
                color={color}
                distance={Number(componentValue(component, "range") ?? 120) * 0.04}
                intensity={intensity}
                key={`${entity.id}-${component.kind}-${index}`}
                position={position}
              />
            );
          }),
      )}
    </>
  );
}

function Viewport3DEntity({
  entities,
  entity,
  gizmoSize,
  mode,
  renderMode,
  selected,
  snapSettings,
  transformMode,
  onSelect,
  onTransformingChange,
  onUpdateTransform,
}: {
  entities: SceneEntity[];
  entity: SceneEntity;
  gizmoSize: number;
  mode: StudioMode;
  renderMode: RenderMode;
  selected: boolean;
  snapSettings: SnapSettings;
  transformMode: "translate" | "rotate" | "scale";
  onSelect: (entity: SceneEntity) => void;
  onTransformingChange: (active: boolean) => void;
  onUpdateTransform: (id: string, transform: Transform3D) => void;
}) {
  const groupRef = useRef<Group | null>(null);
  const [controlTarget, setControlTarget] = useState<Group | null>(null);
  const cameraComponent = entity.components.find((component) => component.kind === "camera3d" || component.kind === "camera2d");
  const lightComponent = entity.components.find((component) =>
    component.kind === "directional_light" || component.kind === "point_light" || component.kind === "spot_light",
  );
  const audioComponent = entity.components.find((component) => component.kind === "audio" || component.kind === "audio3d");
  const isCamera = Boolean(cameraComponent);
  const isLight = Boolean(lightComponent);
  const isAudio = Boolean(audioComponent);
  const isModel = entity.components.some((component) => component.kind === "mesh3d" || component.kind === "model3d");
  const color = selected ? "#68a4ff" : isLight ? "#f5d36b" : isAudio ? "#b48cff" : isCamera ? "#72c08b" : isModel ? "#d9a05c" : "#9aa4b5";
  const position: [number, number, number] = [
    entity.transform.x * 0.04,
    entity.transform.z * 0.04 + 0.45,
    entity.transform.y * 0.04,
  ];
  const scale: [number, number, number] = [
    Math.max(0.25, entity.transform.scaleX),
    Math.max(0.25, entity.transform.scaleZ),
    Math.max(0.25, entity.transform.scaleY),
  ];

  useEffect(() => {
    if (groupRef.current) {
      setControlTarget(groupRef.current);
    }
  }, []);

  useEffect(() => {
    if (!selected || mode === "select") {
      onTransformingChange(false);
    }
  }, [mode, onTransformingChange, selected]);

  useEffect(() => {
    const group = groupRef.current;
    if (!group) return;
    group.position.set(position[0], position[1], position[2]);
    group.rotation.set(
      (entity.transform.rotationX * Math.PI) / 180,
      (entity.transform.rotationY * Math.PI) / 180,
      (entity.transform.rotationZ * Math.PI) / 180,
    );
    group.scale.set(scale[0], scale[1], scale[2]);
  }, [
    entity.transform.rotationX,
    entity.transform.rotationY,
    entity.transform.rotationZ,
    position[0],
    position[1],
    position[2],
    scale[0],
    scale[1],
    scale[2],
  ]);

  function commitTransform() {
    const group = groupRef.current;
    if (!group) return;
    const nextTransform = snapTransform(
      {
        x: Math.round(group.position.x / 0.04),
        y: Math.round(group.position.z / 0.04),
        z: Math.round((group.position.y - 0.45) / 0.04),
        rotationX: Math.round((group.rotation.x * 180) / Math.PI),
        rotationY: Math.round((group.rotation.y * 180) / Math.PI),
        rotationZ: Math.round((group.rotation.z * 180) / Math.PI),
        scaleX: Number(Math.max(0.1, group.scale.x).toFixed(2)),
        scaleY: Number(Math.max(0.1, group.scale.z).toFixed(2)),
        scaleZ: Number(Math.max(0.1, group.scale.y).toFixed(2)),
      },
      entity,
      entities,
      snapSettings,
    );
    onUpdateTransform(entity.id, nextTransform);
  }

  const entityNode = (
    <group ref={groupRef} position={position} rotation={[
      (entity.transform.rotationX * Math.PI) / 180,
      (entity.transform.rotationY * Math.PI) / 180,
      (entity.transform.rotationZ * Math.PI) / 180,
    ]} scale={scale}>
      <ViewportEntityGizmo
        audioComponent={audioComponent}
        cameraComponent={cameraComponent}
        color={color}
        entity={entity}
        lightComponent={lightComponent}
        renderMode={renderMode}
        onSelect={onSelect}
      />
      {selected ? (
        <ViewportSelectionOutline audioComponent={audioComponent} cameraComponent={cameraComponent} lightComponent={lightComponent} />
      ) : null}
    </group>
  );

  if (!selected || mode === "select" || !controlTarget) return entityNode;

  return (
    <>
      {entityNode}
      <TransformControls
        mode={transformMode}
        object={controlTarget}
        onMouseDown={() => onTransformingChange(true)}
        onMouseUp={() => {
          onTransformingChange(false);
          commitTransform();
        }}
        size={gizmoSize}
      />
    </>
  );
}

function ViewportEntityMaterial({ color, renderMode }: { color: string; renderMode: RenderMode }) {
  if (renderMode === "wireframe") {
    return <meshBasicMaterial color={color} wireframe />;
  }

  if (renderMode === "color") {
    return <meshBasicMaterial color={color} />;
  }

  if (renderMode === "texture") {
    return <meshStandardMaterial color={color} roughness={0.72} metalness={0.04} />;
  }

  return <meshStandardMaterial color={color} roughness={0.4} metalness={0.2} />;
}

function Viewport2D({
  camera,
  dragState,
  entities,
  mode,
  selectedEntityId,
  onDragStateChange,
  onEntityPointerDown,
  onEntityPointerMove,
  onResetCamera,
  onSelectEntity,
}: ViewportRendererProps) {
  return (
    <>
      <div className="viewport-grid" />
      <div className="viewport-axis axis-x">X</div>
      <div className="viewport-axis axis-y">Y</div>
      <div className="viewport-overlay-top">
        <StatusPill>Scene 2D</StatusPill>
        <StatusPill>{Math.round(camera.zoom * 100)}%</StatusPill>
        <StatusPill>Orthographic</StatusPill>
        <StatusPill>XY</StatusPill>
        <button className="viewport-reset" onClick={onResetCamera}>Reset</button>
      </div>
      {entities.map((entity) => {
        const selected = entity.id === selectedEntityId;
        const position = project2d(entity.transform);
        const screenX = (position.x + camera.panX) * camera.zoom;
        const screenY = (position.y + camera.panY) * camera.zoom;
        return (
          <button
            className={`scene-object ${selected ? "selected" : ""} ${entityClass(entity)}`}
            key={entity.id}
            onClick={() => onSelectEntity(entity)}
            onPointerDown={(event) => onEntityPointerDown(entity, event)}
            onPointerMove={(event) => onEntityPointerMove(entity, event)}
            onPointerUp={() => onDragStateChange(null)}
            style={{
              left: "50%",
              top: "50%",
              transform: `translate(calc(-50% + ${screenX}px), calc(-50% + ${screenY}px)) scale(${camera.zoom})`,
            }}
          >
            <span className="object-glyph">{entityGlyph(entity)}</span>
            <span className="object-label">{entity.name}</span>
            {selected ? <ObjectGizmo viewMode="2d" /> : null}
          </button>
        );
      })}
      <div className="viewport-footer">
        <span>Viewport 2D</span>
        <span>{entities.length} objects</span>
      </div>
    </>
  );
}

function ObjectGizmo({ viewMode }: { viewMode: ViewMode }) {
  return (
    <div className="object-gizmo" aria-hidden="true">
      <span className="gizmo-x" />
      <span className="gizmo-y" />
      {viewMode === "3d" ? <span className="gizmo-z" /> : null}
    </div>
  );
}

function ScriptEditorWindow({
  activeScript,
  scripts,
  selectedEntity,
  onAttachScript,
  onClose,
  onSaveScript,
  onScriptChange,
  onScriptSelect,
}: {
  activeScript?: ScriptDocument;
  scripts: ScriptDocument[];
  selectedEntity: SceneEntity | null;
  onAttachScript: (path: string) => void;
  onClose: () => void;
  onSaveScript: (script: ScriptDocument) => void;
  onScriptChange: (path: string, content: string) => void;
  onScriptSelect: (path: string) => void;
}) {
  useEffect(() => {
    function onKeyDown(event: KeyboardEvent) {
      if (event.key === "Escape") onClose();
    }

    window.addEventListener("keydown", onKeyDown);
    return () => window.removeEventListener("keydown", onKeyDown);
  }, [onClose]);

  return (
    <div className="script-window-layer" role="dialog" aria-modal="true" aria-label="Script editor">
      <section className="script-window">
        <button className="script-window-close" onClick={onClose} title="Close script editor">
          <X size={16} strokeWidth={ICON_STROKE} />
        </button>
        <CodePanel
          activeScript={activeScript}
          scripts={scripts}
          selectedEntity={selectedEntity}
          onAttachScript={onAttachScript}
          onSaveScript={onSaveScript}
          onScriptChange={onScriptChange}
          onScriptSelect={onScriptSelect}
        />
      </section>
    </div>
  );
}

function CodePanel({
  activeScript,
  scripts,
  selectedEntity,
  onAttachScript,
  onSaveScript,
  onScriptChange,
  onScriptSelect,
}: {
  activeScript?: ScriptDocument;
  scripts: ScriptDocument[];
  selectedEntity: SceneEntity | null;
  onAttachScript: (path: string) => void;
  onSaveScript: (script: ScriptDocument) => void;
  onScriptChange: (path: string, content: string) => void;
  onScriptSelect: (path: string) => void;
}) {
  if (!activeScript) {
    return (
      <section className="code-panel">
        <PanelHeader icon={<Code2 size={15} strokeWidth={ICON_STROKE} />} title="Editor" meta="No script" />
        <div className="empty-state">No script document is open.</div>
      </section>
    );
  }

  const lines = activeScript.content.split("\n");

  return (
    <section className="code-panel">
      <PanelHeader
        icon={<Code2 size={15} strokeWidth={ICON_STROKE} />}
        title="Script Editor"
        meta={selectedEntity ? selectedEntity.name : "Detached"}
      >
        <button className="panel-action" onClick={() => onSaveScript(activeScript)} title="Save script">
          <Save size={14} strokeWidth={ICON_STROKE} />
          Save
        </button>
      </PanelHeader>
      <div className="script-tabs">
        {scripts.map((script) => (
          <button
            className={script.path === activeScript.path ? "selected" : ""}
            key={script.path}
            onClick={() => onScriptSelect(script.path)}
          >
            <FileCode2 size={13} strokeWidth={ICON_STROKE} />
            {script.name}
            {script.dirty ? <Circle size={7} fill="currentColor" strokeWidth={0} /> : null}
          </button>
        ))}
      </div>
      <div className="script-binding">
        <span>Attached to</span>
        <strong>{selectedEntity?.name ?? "No object"}</strong>
        <select value={activeScript.path} onChange={(event) => onAttachScript(event.target.value)}>
          {scripts.map((script) => (
            <option key={script.path} value={script.path}>
              {script.name}
            </option>
          ))}
        </select>
      </div>
      <div className="code-editor">
        <div className="line-gutter">
          {lines.map((_, index) => (
            <span key={index}>{index + 1}</span>
          ))}
        </div>
        <textarea
          aria-label="Zenith script editor"
          spellCheck={false}
          value={activeScript.content}
          onChange={(event) => onScriptChange(activeScript.path, event.target.value)}
        />
      </div>
    </section>
  );
}

function InspectorPanel({
  assets,
  entity,
  scene,
  scripts,
  onAttachScript,
  onComponentChange,
  onEntityChange,
  onSceneChange,
  onTransformChange,
}: {
  assets: ProjectAsset[];
  entity: SceneEntity | null;
  scene: SceneDocument;
  scripts: ScriptDocument[];
  onAttachScript: (path: string) => void;
  onComponentChange: (index: number, component: SceneComponent) => void;
  onEntityChange: (patch: Partial<SceneEntity>) => void;
  onSceneChange: (patch: Partial<SceneDocument>) => void;
  onTransformChange: (patch: Partial<Transform3D>) => void;
}) {
  if (!entity) {
    return (
      <aside className="side-panel inspector-panel">
        <PanelHeader icon={<Settings2 size={15} strokeWidth={ICON_STROKE} />} title="Inspector" meta="Properties" />
        <div className="inspector-scroll">
          <div className="empty-state">No object selected.</div>
          <SceneSettingsPanel assets={assets} scene={scene} scripts={scripts} onSceneChange={onSceneChange} />
        </div>
      </aside>
    );
  }

  const attachedScript = entityScript(entity) ?? "";

  return (
    <aside className="side-panel inspector-panel">
      <PanelHeader icon={<Settings2 size={15} strokeWidth={ICON_STROKE} />} title="Inspector" meta={entity.layer} />
      <div className="inspector-scroll">
        <InspectorGroup title="Identity">
          <TextField label="Name" value={entity.name} onChange={(value) => onEntityChange({ name: value })} />
          <TextField label="Layer" value={entity.layer} onChange={(value) => onEntityChange({ layer: value })} />
          <TextField
            label="Tags"
            value={entity.tags.join(", ")}
            onChange={(value) =>
              onEntityChange({
                tags: value
                  .split(",")
                  .map((tag) => tag.trim())
                  .filter(Boolean),
              })
            }
          />
        </InspectorGroup>

        <InspectorGroup title="Transform">
          <VectorRow
            label="Position"
            values={[entity.transform.x, entity.transform.y, entity.transform.z]}
            onChange={([x, y, z]) => onTransformChange({ x, y, z })}
          />
          <VectorRow
            label="Rotation"
            values={[entity.transform.rotationX, entity.transform.rotationY, entity.transform.rotationZ]}
            onChange={([rotationX, rotationY, rotationZ]) =>
              onTransformChange({ rotationX, rotationY, rotationZ })
            }
          />
          <VectorRow
            label="Scale"
            values={[entity.transform.scaleX, entity.transform.scaleY, entity.transform.scaleZ]}
            onChange={([scaleX, scaleY, scaleZ]) => onTransformChange({ scaleX, scaleY, scaleZ })}
            step={0.1}
          />
        </InspectorGroup>

        <InspectorGroup title="Components" noPadding>
          {entity.components.map((component, index) => (
            <ComponentCard
              assets={assets}
              component={component}
              key={`${component.kind}-${component.asset ?? component.script ?? component.profile ?? index}`}
              scripts={scripts}
              onChange={(nextComponent) => onComponentChange(index, nextComponent)}
              onRemove={() => {
                const next = [...entity.components];
                next.splice(index, 1);
                onEntityChange({ components: next });
              }}
            />
          ))}
          <div className="add-component-row">
            <select
              className="component-input"
              value=""
              onChange={(event) => {
                if (!event.target.value) return;
                onEntityChange({
                  components: [...entity.components, createComponentFromSchema(event.target.value)],
                });
              }}
            >
              <option value="">+ Add Component</option>
              {Object.entries(borealisComponents())
                .filter(([kind]) => !entity.components.some((c) => c.kind === kind))
                .map(([kind, schema]) => (
                  <option key={kind} value={kind}>
                    {kind} - {schema.description}
                  </option>
                ))}
            </select>
          </div>
        </InspectorGroup>

        <InspectorGroup title="Script">
          <label className="field-label" htmlFor="attached-script">
            Attached script
          </label>
          <select id="attached-script" value={attachedScript} onChange={(event) => onAttachScript(event.target.value)}>
            <option value="">None</option>
            {scripts.map((script) => (
              <option key={script.path} value={script.path}>
                {script.name}
              </option>
            ))}
          </select>
        </InspectorGroup>

        <SceneSettingsPanel assets={assets} scene={scene} scripts={scripts} onSceneChange={onSceneChange} />
      </div>
    </aside>
  );
}

function ViewportEntityGizmo({
  audioComponent,
  cameraComponent,
  color,
  entity,
  lightComponent,
  renderMode,
  onSelect,
}: {
  audioComponent?: SceneComponent;
  cameraComponent?: SceneComponent;
  color: string;
  entity: SceneEntity;
  lightComponent?: SceneComponent;
  renderMode: RenderMode;
  onSelect: (entity: SceneEntity) => void;
}) {
  const clickProps = {
    onClick: (event: any) => {
      event.stopPropagation();
      onSelect(entity);
    },
  };

  if (cameraComponent) {
    const fov = Number(componentValue(cameraComponent, "fov") ?? 60);
    const length = clamp(fov / 90, 0.45, 1.4);
    return (
      <group>
        <mesh {...clickProps} rotation={[Math.PI / 2, 0, Math.PI / 4]}>
          <coneGeometry args={[0.42, 0.78, 4]} />
          <meshBasicMaterial color={color} wireframe />
        </mesh>
        <mesh {...clickProps} position={[0, 0, -length * 0.56]} rotation={[Math.PI / 2, 0, 0]}>
          <coneGeometry args={[length * 0.52, length, 4, 1, true]} />
          <meshBasicMaterial color={color} transparent opacity={0.18} wireframe />
        </mesh>
      </group>
    );
  }

  if (lightComponent) {
    const lightColor = String(componentValue(lightComponent, "color") ?? color);
    const range = Number(componentValue(lightComponent, "range") ?? 120) * 0.04;
    if (lightComponent.kind === "directional_light") {
      return (
        <group>
          <mesh {...clickProps}>
            <sphereGeometry args={[0.28, 18, 12]} />
            <meshBasicMaterial color={lightColor} />
          </mesh>
          <mesh {...clickProps} position={[0, 0, -0.55]} rotation={[Math.PI / 2, 0, 0]}>
            <coneGeometry args={[0.22, 0.65, 16]} />
            <meshBasicMaterial color={lightColor} wireframe />
          </mesh>
        </group>
      );
    }

    if (lightComponent.kind === "spot_light") {
      const angle = Number(componentValue(lightComponent, "angle") ?? 45);
      const coneRadius = clamp((angle / 90) * range * 0.35, 0.18, 2.4);
      return (
        <group>
          <mesh {...clickProps}>
            <sphereGeometry args={[0.22, 18, 12]} />
            <meshBasicMaterial color={lightColor} />
          </mesh>
          <mesh {...clickProps} position={[0, 0, -range * 0.45]} rotation={[Math.PI / 2, 0, 0]}>
            <coneGeometry args={[coneRadius, Math.max(0.4, range), 24, 1, true]} />
            <meshBasicMaterial color={lightColor} transparent opacity={0.28} wireframe />
          </mesh>
        </group>
      );
    }

    return (
      <group>
        <mesh {...clickProps}>
          <sphereGeometry args={[0.24, 18, 12]} />
          <meshBasicMaterial color={lightColor} />
        </mesh>
        <mesh {...clickProps}>
          <sphereGeometry args={[Math.max(0.35, range), 24, 16]} />
          <meshBasicMaterial color={lightColor} transparent opacity={0.18} wireframe />
        </mesh>
      </group>
    );
  }

  if (audioComponent) {
    const range = Number(componentValue(audioComponent, "range") ?? 80) * 0.04;
    return (
      <group>
        <mesh {...clickProps}>
          <sphereGeometry args={[0.22, 18, 12]} />
          <meshBasicMaterial color={color} />
        </mesh>
        <mesh {...clickProps}>
          <sphereGeometry args={[Math.max(0.35, range), 24, 16]} />
          <meshBasicMaterial color={color} transparent opacity={0.16} wireframe />
        </mesh>
        <mesh {...clickProps} position={[0, 0.38, 0]}>
          <torusGeometry args={[0.24, 0.025, 8, 24]} />
          <meshBasicMaterial color={color} />
        </mesh>
      </group>
    );
  }

  return (
    <mesh
      castShadow
      {...clickProps}
    >
      <boxGeometry args={[0.9, 0.9, 0.9]} />
      <ViewportEntityMaterial color={color} renderMode={renderMode} />
    </mesh>
  );
}

function ViewportSelectionOutline({
  audioComponent,
  cameraComponent,
  lightComponent,
}: {
  audioComponent?: SceneComponent;
  cameraComponent?: SceneComponent;
  lightComponent?: SceneComponent;
}) {
  if (cameraComponent) {
    return (
      <mesh rotation={[Math.PI / 2, 0, Math.PI / 4]} scale={[1.08, 1.08, 1.08]}>
        <coneGeometry args={[0.42, 0.78, 4]} />
        <meshBasicMaterial color="#68a4ff" wireframe />
      </mesh>
    );
  }

  if (lightComponent || audioComponent) {
    return (
      <mesh>
        <sphereGeometry args={[0.38, 18, 12]} />
        <meshBasicMaterial color="#68a4ff" wireframe />
      </mesh>
    );
  }

  return (
    <mesh scale={[1.05, 1.05, 1.05]}>
      <boxGeometry args={[0.9, 0.9, 0.9]} />
      <meshBasicMaterial color="#68a4ff" wireframe />
    </mesh>
  );
}

function SceneSettingsPanel({
  assets,
  scene,
  scripts,
  onSceneChange,
}: {
  assets: ProjectAsset[];
  scene: SceneDocument;
  scripts: ScriptDocument[];
  onSceneChange: (patch: Partial<SceneDocument>) => void;
}) {
  const settings = borealisSceneSettings();
  const entries = Object.entries(settings);
  if (entries.length === 0) return null;

  return (
    <InspectorGroup title="Scene Settings" noPadding>
      {entries.map(([key, schema]) => {
        const section = sceneSettingSection(key);
        const values = sceneSettingValues(scene, key);
        const pseudoComponent: SceneComponent = { kind: key, properties: values };

        return (
          <div className="component-card" key={key}>
            <div className="component-card-header">
              <Settings2 size={15} strokeWidth={ICON_STROKE} />
              <strong>{schema.label ?? key}</strong>
            </div>
            <div className="component-card-body">
              <div className="component-fields">
                {schema.fields.map((field) => (
                  <div className="component-field" key={field.key}>
                    <span>{field.label}</span>
                    <ComponentFieldControl
                      assets={assets}
                      component={pseudoComponent}
                      field={field}
                      scripts={scripts}
                      onChange={(value) => {
                        const nextComponent = setComponentValue(pseudoComponent, field.key, value);
                        const nextValues = nextComponent.properties ?? {};
                        if (section === "environment") {
                          onSceneChange({ environment: { ...scene.environment, [key]: nextValues } });
                        } else if (section === "render") {
                          onSceneChange({ render: { ...scene.render, [key]: nextValues } });
                        } else {
                          onSceneChange({ audio: { ...scene.audio, [key]: nextValues } });
                        }
                      }}
                    />
                  </div>
                ))}
              </div>
            </div>
          </div>
        );
      })}
    </InspectorGroup>
  );
}

function ComponentCard({
  assets,
  component,
  scripts,
  onChange,
  onRemove,
}: {
  assets: ProjectAsset[];
  component: SceneComponent;
  scripts: ScriptDocument[];
  onChange: (component: SceneComponent) => void;
  onRemove?: () => void;
}) {
  const schema = componentSchema(component);
  const knownKeys = new Set(["kind", "type", ...schema.fields.map((field) => field.key)]);
  const unknownEntries = Object.entries(component.properties ?? {}).filter(([key]) => !knownKeys.has(key));

  return (
    <div className="component-card">
      <div className="component-card-header">
        <Component size={15} strokeWidth={ICON_STROKE} />
        <strong>{component.kind}</strong>
        {onRemove && (
          <button className="remove-btn" onClick={onRemove} title="Remove Component">
            <X size={14} strokeWidth={ICON_STROKE} />
          </button>
        )}
      </div>
      <div className="component-card-body">
        {schema.fields.length > 0 ? (
          <div className="component-fields">
            {schema.fields.map((field) => (
              <div className="component-field" key={field.key}>
                <span>{field.label}</span>
                <ComponentFieldControl
                  assets={assets}
                  component={component}
                  field={field}
                  scripts={scripts}
                  onChange={(value) => onChange(setComponentValue(component, field.key, value))}
                />
              </div>
            ))}
          </div>
        ) : null}
        {unknownEntries.length > 0 ? (
          <div className="component-fields unknown">
            {unknownEntries.map(([key, value]) => (
              <div className="component-field" key={key}>
                <span>{key}</span>
                <code>{formatComponentValue(value)}</code>
              </div>
            ))}
          </div>
        ) : null}
      </div>
    </div>
  );
}

function ComponentFieldControl({
  assets,
  component,
  field,
  scripts,
  onChange,
}: {
  assets: ProjectAsset[];
  component: SceneComponent;
  field: ComponentFieldSchema;
  scripts: ScriptDocument[];
  onChange: (value: unknown) => void;
}) {
  const value = componentValue(component, field.key);

  if (field.kind === "boolean") {
    return (
      <input
        checked={value === true}
        className="component-check"
        onChange={(event) => onChange(event.target.checked)}
        type="checkbox"
      />
    );
  }

  if (field.kind === "number") {
    return (
      <input
        className="component-input"
        max={field.max}
        min={field.min}
        onChange={(event) => onChange(event.target.value === "" ? undefined : Number(event.target.value))}
        step={field.step ?? "any"}
        type="number"
        value={componentInputValue(value)}
      />
    );
  }

  if (field.kind === "color") {
    const currentValue = typeof value === "string" && value.startsWith("#") ? value : "#ffffff";
    return (
      <input
        className="component-input component-color-input"
        onChange={(event) => onChange(event.target.value)}
        type="color"
        value={currentValue}
      />
    );
  }

  if (field.kind === "select") {
    return (
      <select className="component-input" onChange={(event) => onChange(event.target.value)} value={String(value ?? "")}>
        <option value="">None</option>
        {(field.options ?? []).map((option) => (
          <option key={option} value={option}>
            {option}
          </option>
        ))}
      </select>
    );
  }

  if (field.kind === "asset") {
    const currentValue = String(value ?? "");
    const options = includeCurrentOption(assetOptionsForComponentField(component, field, assets, scripts), currentValue);
    return (
      <select
        className="component-input"
        onChange={(event) => onChange(event.target.value || undefined)}
        value={currentValue}
      >
        <option value="">None</option>
        {options.map((option) => (
          <option key={option} value={option}>
            {option}
          </option>
        ))}
      </select>
    );
  }

  return (
    <input
      className="component-input"
      onChange={(event) => onChange(event.target.value)}
      value={componentInputValue(value)}
    />
  );
}

function includeCurrentOption(options: string[], value: string): string[] {
  if (!value || options.includes(value)) return options;
  return [value, ...options];
}

function componentInputValue(value: unknown): string | number {
  if (typeof value === "number" || typeof value === "string") return value;
  if (typeof value === "boolean") return String(value);
  return "";
}

function assetOptionsForComponentField(
  component: SceneComponent,
  field: ComponentFieldSchema,
  assets: ProjectAsset[],
  scripts: ScriptDocument[],
): string[] {
  if (field.key === "script" || component.kind === "script") {
    return scripts.map((script) => script.path);
  }

  let wantedKind: AssetKind | null = null;
  if (isAssetKind(field.assetKind)) wantedKind = field.assetKind;
  else if (component.kind === "model3d") wantedKind = "model";
  else if (component.kind === "sprite") wantedKind = "texture";
  else if (component.kind === "audio" || component.kind === "audio3d") wantedKind = "audio";

  return assets
    .filter((asset) => asset.kind !== "scene" && (!wantedKind || asset.kind === wantedKind))
    .map((asset) => asset.path);
}

function isAssetKind(value: unknown): value is AssetKind {
  return (
    value === "scene" ||
    value === "script" ||
    value === "model" ||
    value === "texture" ||
    value === "audio" ||
    value === "shader" ||
    value === "cubemap" ||
    value === "data"
  );
}

function BottomDock({
  activeTab,
  assets,
  console,
  scripts,
  onAssetDrop,
  onAssetPointerDown,
  onOpenScript,
  onScriptSelect,
  onTabChange,
}: {
  activeTab: BottomTab;
  assets: ProjectAsset[];
  console: ConsoleLine[];
  scripts: ScriptDocument[];
  onAssetDrop: (asset: ProjectAsset) => void;
  onAssetPointerDown: (asset: ProjectAsset, event: PointerEvent<HTMLElement>) => void;
  onOpenScript: (path: string) => void;
  onScriptSelect: (path: string) => void;
  onTabChange: (tab: BottomTab) => void;
}) {
  const [assetTileSize, setAssetTileSize] = useState(112);
  const assetPanelRef = useRef<HTMLDivElement | null>(null);
  const assetIconSize = clamp(Math.round(assetTileSize * 0.22), 18, 38);

  function changeAssetTileSize(delta: number) {
    setAssetTileSize((current) => clamp(current + delta, 88, 168));
  }

  useEffect(() => {
    const panel = assetPanelRef.current;
    if (!panel) return;

    function handleAssetWheel(event: WheelEvent) {
      if (!event.altKey) return;
      event.preventDefault();
      changeAssetTileSize(event.deltaY < 0 ? 8 : -8);
    }

    panel.addEventListener("wheel", handleAssetWheel, { passive: false });
    return () => panel.removeEventListener("wheel", handleAssetWheel);
  }, []);

  return (
    <section className="bottom-dock">
      <div className="bottom-tabs">
        <DockTab active={activeTab === "assets"} icon={<Archive size={14} strokeWidth={ICON_STROKE} />} onClick={() => onTabChange("assets")}>
          Assets
        </DockTab>
        <DockTab active={activeTab === "console"} icon={<Terminal size={14} strokeWidth={ICON_STROKE} />} onClick={() => onTabChange("console")}>
          Console
        </DockTab>
        <DockTab active={activeTab === "problems"} icon={<BadgeAlert size={14} strokeWidth={ICON_STROKE} />} onClick={() => onTabChange("problems")}>
          Problems
        </DockTab>
      </div>
      {activeTab === "assets" ? (
        <div
          className="asset-panel"
          ref={assetPanelRef}
          style={
            {
              "--asset-tile-size": `${assetTileSize}px`,
              "--asset-icon-size": `${assetIconSize}px`,
            } as React.CSSProperties
          }
        >
          <div className="asset-panel-toolbar">
            <ZoomIn size={13} strokeWidth={ICON_STROKE} />
            <input
              aria-label="Asset icon size"
              min={88}
              max={168}
              type="range"
              value={assetTileSize}
              onChange={(event) => setAssetTileSize(Number(event.target.value))}
            />
          </div>
          <div className="asset-grid">
            {assets.map((asset) => (
              <AssetTile
                asset={asset}
                iconSize={assetIconSize}
                key={asset.id}
                onDrop={onAssetDrop}
                onOpenScript={onOpenScript}
                onPointerDown={onAssetPointerDown}
                onScriptSelect={onScriptSelect}
              />
            ))}
          </div>
        </div>
      ) : null}
      {activeTab === "console" ? <ConsoleView lines={console} /> : null}
      {activeTab === "problems" ? <ProblemsView scripts={scripts} /> : null}
    </section>
  );
}

function AssetTile({
  asset,
  iconSize,
  onDrop,
  onOpenScript,
  onPointerDown,
  onScriptSelect,
}: {
  asset: ProjectAsset;
  iconSize: number;
  onDrop: (asset: ProjectAsset) => void;
  onOpenScript: (path: string) => void;
  onPointerDown: (asset: ProjectAsset, event: PointerEvent<HTMLElement>) => void;
  onScriptSelect: (path: string) => void;
}) {
  return (
    <button
      className="asset-tile"
      draggable
      onClick={() => {
        if (asset.kind === "script") onScriptSelect(asset.path);
      }}
      onDoubleClick={() => {
        if (asset.kind === "script") onOpenScript(asset.path);
        if (asset.kind === "model" || asset.kind === "texture") onDrop(asset);
      }}
      onPointerDown={(event) => onPointerDown(asset, event)}
      onDragStart={(event) => {
        event.dataTransfer.setData("application/x-borealis-asset", JSON.stringify(asset));
      }}
    >
      <span>{assetKindIcon(asset.kind, iconSize)}</span>
      <strong>{asset.name}</strong>
      <small>{asset.path}</small>
    </button>
  );
}

function AssetDragPreview({ asset, x, y }: { asset: ProjectAsset; x: number; y: number }) {
  return (
    <div
      className="asset-drag-preview"
      style={{
        left: x + 14,
        top: y + 14,
      }}
    >
      {assetKindIcon(asset.kind, 16)}
      <span>{asset.name}</span>
    </div>
  );
}

function ConsoleView({ lines }: { lines: ConsoleLine[] }) {
  return (
    <div className="console-view">
      {lines.map((line) => (
        <div className={`console-line ${line.level}`} key={line.id}>
          <span>{line.source}</span>
          <p>{line.message}</p>
        </div>
      ))}
    </div>
  );
}

function ProblemsView({ scripts }: { scripts: ScriptDocument[] }) {
  const dirtyScripts = scripts.filter((script) => script.dirty);
  return (
    <div className="problems-view">
      {dirtyScripts.length === 0 ? (
        <div className="empty-state inline">
          <Check size={16} strokeWidth={ICON_STROKE} />
          No script problems tracked.
        </div>
      ) : (
        dirtyScripts.map((script) => (
          <div className="problem-row" key={script.path}>
            <Circle size={8} fill="currentColor" strokeWidth={0} />
            <span>{script.name} has unsaved edits.</span>
          </div>
        ))
      )}
    </div>
  );
}

function PanelHeader({
  children,
  icon,
  meta,
  title,
}: {
  children?: React.ReactNode;
  icon: React.ReactNode;
  meta: string;
  title: string;
}) {
  return (
    <div className="panel-header">
      <div>
        {icon}
        <strong>{title}</strong>
        <span>{meta}</span>
      </div>
      {children}
    </div>
  );
}

function SectionTitle({ label }: { label: string }) {
  return (
    <div className="section-title">
      <ChevronDown size={11} strokeWidth={ICON_STROKE} />
      <span>{label}</span>
    </div>
  );
}

function InspectorGroup({ children, title, noPadding }: { children: React.ReactNode; title: string; noPadding?: boolean }) {
  return (
    <section className="inspector-group">
      <div className="inspector-group-header">
        <ChevronDown size={11} strokeWidth={ICON_STROKE} />
        <h3>{title}</h3>
      </div>
      <div className={`inspector-group-content ${noPadding ? "no-padding" : ""}`}>
        {children}
      </div>
    </section>
  );
}

function TextField({
  label,
  onChange,
  value,
}: {
  label: string;
  onChange: (value: string) => void;
  value: string;
}) {
  return (
    <label className="text-field">
      <span>{label}</span>
      <input value={value} onChange={(event) => onChange(event.target.value)} />
    </label>
  );
}

function VectorRow({
  label,
  onChange,
  step = 1,
  values,
}: {
  label: string;
  onChange: (values: [number, number, number]) => void;
  step?: number;
  values: [number, number, number];
}) {
  return (
    <div className="vector-row">
      <span>{label}</span>
      {(["X", "Y", "Z"] as const).map((axis, index) => (
        <label key={axis}>
          <small>{axis}</small>
          <input
            step={step}
            type="number"
            value={Number(values[index].toFixed(2))}
            onChange={(event) => {
              const next = [...values] as [number, number, number];
              next[index] = Number(event.target.value);
              onChange(next);
            }}
          />
        </label>
      ))}
    </div>
  );
}

function Segmented<T extends string>({
  onChange,
  tabs,
  value,
}: {
  onChange: (value: T) => void;
  tabs: T[];
  value: T;
}) {
  return (
    <div className="segmented">
      {tabs.map((tab) => (
        <button className={value === tab ? "selected" : ""} key={tab} onClick={() => onChange(tab)}>
          {tab}
        </button>
      ))}
    </div>
  );
}

function DockTab({
  active,
  children,
  icon,
  onClick,
}: {
  active: boolean;
  children: React.ReactNode;
  icon: React.ReactNode;
  onClick: () => void;
}) {
  return (
    <button className={active ? "selected" : ""} onClick={onClick}>
      {icon}
      {children}
    </button>
  );
}

function ToolButton({
  active,
  children,
  disabled,
  label,
  onClick,
}: {
  active?: boolean;
  children: React.ReactNode;
  disabled?: boolean;
  label: string;
  onClick?: () => void;
}) {
  return (
    <button
      aria-pressed={active}
      className={`tool-button ${active ? "active" : ""}`}
      disabled={disabled}
      onClick={onClick}
      title={label}
    >
      {children}
    </button>
  );
}

function StatusPill({ children, tone = "neutral" }: { children: React.ReactNode; tone?: "neutral" | "good" | "warn" }) {
  return <span className={`status-pill ${tone}`}>{children}</span>;
}

function normalizePreviewStatus(status: string): PreviewStatus {
  const knownStatuses: PreviewStatus[] = [
    "idle",
    "starting",
    "loading",
    "ready",
    "playing",
    "paused",
    "stopped",
    "exited",
    "error",
    "unavailable",
  ];
  return knownStatuses.includes(status as PreviewStatus) ? (status as PreviewStatus) : "error";
}

function previewStatusTone(status: PreviewStatus): "neutral" | "good" | "warn" {
  if (status === "ready" || status === "playing") return "good";
  if (["paused", "error", "unavailable", "exited"].includes(status)) return "warn";
  return "neutral";
}

function previewStatusLabel(status: PreviewStatus): string {
  const labels: Record<PreviewStatus, string> = {
    idle: "Preview idle",
    starting: "Preview starting",
    loading: "Preview loading",
    ready: "Preview ready",
    playing: "Preview playing",
    paused: "Preview paused",
    stopped: "Preview stopped",
    exited: "Preview exited",
    error: "Preview error",
    unavailable: "Preview unavailable",
  };
  return labels[status];
}

function previewEventToConsoleLine(event: PreviewEvent, runner?: string): Omit<ConsoleLine, "id"> {
  let message = event.raw;

  if (event.kind === "hello" && runner) {
    message = `Preview connected through ${runner}`;
  } else if (event.status) {
    message = `Preview status: ${event.status}`;
    const counts = previewEventCounts(event);
    if (counts) message += ` (${counts})`;
  } else if (event.message) {
    message = event.message;
    const counts = previewEventCounts(event);
    if (counts) message += ` (${counts})`;
  } else if (event.kind) {
    message = `Preview event: ${event.kind}`;
  }

  return {
    level: event.channel === "error" || event.raw.startsWith("[stderr]") ? "error" : "info",
    source: "preview",
    message,
  };
}

function entityScript(entity: SceneEntity): string | undefined {
  return entity.components.find((component) => component.kind === "script")?.script;
}

function formatComponentValue(value: unknown): string {
  if (value === undefined || value === null || value === "") return "none";
  if (typeof value === "string" || typeof value === "number" || typeof value === "boolean") return String(value);
  return JSON.stringify(value);
}

function sceneColorValue(value: Record<string, unknown>, key: string, fallback: string): string {
  const item = value[key];
  return typeof item === "string" && item.startsWith("#") ? item : fallback;
}

function sceneNumberValue(value: Record<string, unknown>, key: string, fallback: number): number {
  const item = value[key];
  return typeof item === "number" && Number.isFinite(item) ? item : fallback;
}

function sceneBooleanValue(value: Record<string, unknown>, key: string, fallback: boolean): boolean {
  const item = value[key];
  return typeof item === "boolean" ? item : fallback;
}

function sceneSettingSection(key: string): "environment" | "render" | "audio" {
  if (key in defaultSceneEnvironment()) return "environment";
  if (key in defaultSceneRender()) return "render";
  return "audio";
}

function sceneSettingValues(scene: SceneDocument, key: string): Record<string, unknown> {
  const section = sceneSettingSection(key);
  const value =
    section === "environment" ? scene.environment[key as keyof SceneEnvironment] :
    section === "render" ? scene.render[key as keyof SceneRenderSettings] :
    scene.audio[key as keyof SceneAudioSettings];
  return value && typeof value === "object" && !Array.isArray(value) ? value : {};
}

function defaultSceneEnvironment(): SceneEnvironment {
  return {
    skybox: {},
    ambient: {},
    fog: {},
    weather: {},
  };
}

function previewEventCounts(event: PreviewEvent): string {
  const parts: string[] = [];
  if (typeof event.entityCount === "number") parts.push(`${event.entityCount} entities`);
  if (typeof event.cameraCount === "number") parts.push(`${event.cameraCount} cameras`);
  if (typeof event.lightCount === "number") parts.push(`${event.lightCount} lights`);
  if (typeof event.audioCount === "number") parts.push(`${event.audioCount} audio`);
  return parts.join(", ");
}

function defaultSceneRender(): SceneRenderSettings {
  return {
    quality: {},
    postfx: {},
    camera: {},
  };
}

function entityClass(entity: SceneEntity): string {
  if (entity.components.some((component) => component.kind === "camera3d" || component.kind === "camera2d")) return "camera";
  if (entity.components.some((component) => component.kind === "cube3d")) return "cube";
  if (entity.components.some((component) => component.kind === "model3d")) return "model";
  if (entity.components.some((component) => component.kind === "audio" || component.kind === "audio3d")) return "audio";
  if (entity.components.some((component) => component.kind === "ui" || component.kind === "ui3d" || component.kind === "hud")) return "ui";
  return "default";
}

function entityGlyph(entity: SceneEntity) {
  if (entity.components.some((component) => component.kind === "camera3d" || component.kind === "camera2d")) {
    return <Maximize2 size={16} strokeWidth={ICON_STROKE} />;
  }
  if (entity.components.some((component) => component.kind === "cube3d")) {
    return <Cuboid size={17} strokeWidth={ICON_STROKE} />;
  }
  if (entity.components.some((component) => component.kind === "audio" || component.kind === "audio3d")) {
    return <Activity size={17} strokeWidth={ICON_STROKE} />;
  }
  if (entity.components.some((component) => component.kind === "ui" || component.kind === "ui3d" || component.kind === "hud")) {
    return <LayoutTemplate size={17} strokeWidth={ICON_STROKE} />;
  }
  return <Box size={17} strokeWidth={ICON_STROKE} />;
}

function entityIcon(entity: SceneEntity) {
  if (entity.components.some((component) => component.kind === "camera3d" || component.kind === "camera2d")) {
    return <Maximize2 size={12} strokeWidth={ICON_STROKE} />;
  }
  if (entity.components.some((component) => component.kind === "script")) {
    return <Braces size={12} strokeWidth={ICON_STROKE} />;
  }
  if (entity.components.some((component) => component.kind === "audio" || component.kind === "audio3d")) {
    return <Activity size={12} strokeWidth={ICON_STROKE} />;
  }
  if (entity.components.some((component) => component.kind === "ui" || component.kind === "ui3d" || component.kind === "hud")) {
    return <LayoutTemplate size={12} strokeWidth={ICON_STROKE} />;
  }
  if (entity.components.some((component) => component.kind === "render3d" || component.kind === "postfx")) {
    return <WandSparkles size={12} strokeWidth={ICON_STROKE} />;
  }
  return <Box size={12} strokeWidth={ICON_STROKE} />;
}

function assetKindIcon(kind: AssetKind, size = 15) {
  const props = { size, strokeWidth: ICON_STROKE };
  if (kind === "script") return <FileCode2 {...props} />;
  if (kind === "model") return <Cuboid {...props} />;
  if (kind === "scene") return <Layers3 {...props} />;
  if (kind === "texture") return <WandSparkles {...props} />;
  if (kind === "audio") return <Activity {...props} />;
  if (kind === "shader" || kind === "cubemap") return <WandSparkles {...props} />;
  return <Database {...props} />;
}

function project3d(transform: Transform3D): { x: number; y: number } {
  return {
    x: transform.x * 0.55 - transform.y * 0.34,
    y: transform.x * 0.1 + transform.y * 0.22 - transform.z * 0.7,
  };
}

function project2d(transform: Transform3D): { x: number; y: number } {
  return {
    x: transform.x * 0.72,
    y: transform.y * 0.5,
  };
}

function snapTransform(
  transform: Transform3D,
  entity: SceneEntity,
  entities: SceneEntity[],
  settings: SnapSettings,
): Transform3D {
  let next = { ...transform };

  if (settings.snapToGrid) {
    const sceneStep = gridMetersToSceneUnits(settings.gridSize);
    next = {
      ...next,
      x: snapValue(next.x, sceneStep),
      y: snapValue(next.y, sceneStep),
      z: snapValue(next.z, sceneStep),
    };
  }

  if (settings.snapToObject) {
    next = snapTransformToObjects(next, entity, entities, settings.gridSize);
  }

  return next;
}

function snapTransformToObjects(
  transform: Transform3D,
  entity: SceneEntity,
  entities: SceneEntity[],
  gridSize: number,
): Transform3D {
  const threshold = clamp(gridMetersToSceneUnits(gridSize) * 0.5, 4, 18);
  const movingBounds = entityBounds({ ...entity, transform });
  let next = { ...transform };
  let best = { axis: "" as "x" | "y" | "z" | "", delta: 0, distance: Number.POSITIVE_INFINITY };

  for (const target of entities) {
    if (target.id === entity.id) continue;
    const targetBounds = entityBounds(target);
    const candidates = [
      { axis: "x" as const, delta: targetBounds.maxX - movingBounds.minX },
      { axis: "x" as const, delta: targetBounds.minX - movingBounds.maxX },
      { axis: "x" as const, delta: target.transform.x - transform.x },
      { axis: "y" as const, delta: targetBounds.maxY - movingBounds.minY },
      { axis: "y" as const, delta: targetBounds.minY - movingBounds.maxY },
      { axis: "y" as const, delta: target.transform.y - transform.y },
      { axis: "z" as const, delta: targetBounds.maxZ - movingBounds.minZ },
      { axis: "z" as const, delta: targetBounds.minZ - movingBounds.maxZ },
      { axis: "z" as const, delta: target.transform.z - transform.z },
    ];

    for (const candidate of candidates) {
      const distance = Math.abs(candidate.delta);
      if (distance < best.distance && distance <= threshold) {
        best = { axis: candidate.axis, delta: candidate.delta, distance };
      }
    }
  }

  if (best.axis) {
    next = {
      ...next,
      [best.axis]: Math.round(next[best.axis] + best.delta),
    };
  }

  return next;
}

function entityBounds(entity: SceneEntity) {
  const halfX = Math.max(0.25, entity.transform.scaleX) * 12.5;
  const halfY = Math.max(0.25, entity.transform.scaleY) * 12.5;
  const halfZ = Math.max(0.25, entity.transform.scaleZ) * 12.5;

  return {
    minX: entity.transform.x - halfX,
    maxX: entity.transform.x + halfX,
    minY: entity.transform.y - halfY,
    maxY: entity.transform.y + halfY,
    minZ: entity.transform.z - halfZ,
    maxZ: entity.transform.z + halfZ,
  };
}

function snapValue(value: number, step: number): number {
  if (step <= 0) return value;
  return Math.round(value / step) * step;
}

function gridMetersToSceneUnits(gridSize: number): number {
  return gridSize / 0.04;
}

function clamp(value: number, min: number, max: number): number {
  return Math.min(Math.max(value, min), max);
}
