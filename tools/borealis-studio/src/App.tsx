import { AnimatePresence, motion } from "framer-motion";
import { Canvas } from "@react-three/fiber";
import { Grid, OrbitControls, PerspectiveCamera, TransformControls } from "@react-three/drei";
import { MOUSE } from "three";
import {
  Activity,
  Archive,
  BadgeAlert,
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
  FileCode2,
  FolderTree,
  Layers3,
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
import { isTauriRuntime, loadStudioSnapshot, writeScriptDocument } from "./backend";
import { createMockSnapshot } from "./mockData";
import type {
  AssetKind,
  BottomTab,
  ConsoleLine,
  ProjectAsset,
  SceneEntity,
  ScriptDocument,
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

export function BorealisStudioApp() {
  const [snapshot, setSnapshot] = useState<StudioSnapshot>(() => createMockSnapshot());
  const [bridgeStatus, setBridgeStatus] = useState<BridgeStatus>("loading");
  const [selectedEntityId, setSelectedEntityId] = useState("player");
  const [selectedScriptPath, setSelectedScriptPath] = useState("src/app/player_controller.zt");
  const [viewMode, setViewMode] = useState<ViewMode>("3d");
  const [bottomTab, setBottomTab] = useState<BottomTab>("assets");
  const [mode, setMode] = useState<StudioMode>("move");
  const [scriptEditorOpen, setScriptEditorOpen] = useState(false);
  const [layout, setLayout] = useState<LayoutState>({
    left: 280,
    right: 292,
    bottom: 218,
  });
  const [dragState, setDragState] = useState<DragState | null>(null);
  const [assetDrag, setAssetDrag] = useState<AssetDragState | null>(null);

  useEffect(() => {
    let cancelled = false;
    loadStudioSnapshot()
      .then((loaded) => {
        if (cancelled) return;
        setSnapshot(loaded);
        setBridgeStatus("tauri");
        const firstEntity = loaded.scene.entities[0];
        if (firstEntity) {
          setSelectedEntityId(firstEntity.id);
          setSelectedScriptPath(entityScript(firstEntity) ?? loaded.scripts[0]?.path ?? "");
        }
      })
      .catch(() => {
        if (cancelled) return;
        setBridgeStatus(isTauriRuntime() ? "error" : "browser");
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
      if (key === "q") setMode("select");
      if (key === "g") setMode("move");
      if (key === "r") setMode("rotate");
      if (key === "s") setMode("scale");
    }

    window.addEventListener("keydown", handleToolShortcut);
    return () => window.removeEventListener("keydown", handleToolShortcut);
  }, []);

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

  function selectEntity(entity: SceneEntity) {
    setSelectedEntityId(entity.id);
    const script = entityScript(entity);
    if (script) {
      setSelectedScriptPath(script);
    }
  }

  function updateSelectedEntity(patch: Partial<SceneEntity>) {
    if (!selectedEntity) return;
    setSnapshot((current) => ({
      ...current,
      scene: {
        ...current.scene,
        entities: current.scene.entities.map((entity) =>
          entity.id === selectedEntity.id ? { ...entity, ...patch } : entity,
        ),
      },
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
      await writeScriptDocument(script);
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

    setSnapshot((current) => ({
      ...current,
      scene: {
        ...current.scene,
        entities: [...current.scene.entities, entity],
      },
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
      className="studio-shell"
      style={
        {
          "--left-width": `${layout.left}px`,
          "--right-width": `${layout.right}px`,
          "--bottom-height": `${layout.bottom}px`,
        } as React.CSSProperties
      }
    >
      <MenuBar bridgeStatus={bridgeStatus} />

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
            viewMode={viewMode}
            selectedEntity={selectedEntity}
            onModeChange={setMode}
            onViewModeChange={setViewMode}
          />

          <div className="stage-content">
            <section className="viewport-wrap">
              <SceneViewport
                entities={snapshot.scene.entities}
                mode={mode}
                viewMode={viewMode}
                selectedEntityId={selectedEntityId}
                dragState={dragState}
                onAssetDrop={createEntityFromAsset}
                onDragStateChange={setDragState}
                onSelectEntity={selectEntity}
                onUpdateTransform={(id, transform) => {
                  setSnapshot((current) => ({
                    ...current,
                    scene: {
                      ...current.scene,
                      entities: current.scene.entities.map((entity) =>
                        entity.id === id ? { ...entity, transform } : entity,
                      ),
                    },
                  }));
                }}
              />
            </section>
          </div>
        </main>

        <div className="resize-handle resize-handle-vertical resize-handle-right" onPointerDown={(event) => startResize("right", event)} />

        <InspectorPanel
          entity={selectedEntity}
          scripts={snapshot.scripts}
          onAttachScript={attachScript}
          onEntityChange={updateSelectedEntity}
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

      {assetDrag ? <AssetDragPreview asset={assetDrag.asset} x={assetDrag.x} y={assetDrag.y} /> : null}
      {scriptEditorOpen ? (
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
    </div>
  );
}

function MenuBar({ bridgeStatus }: { bridgeStatus: BridgeStatus }) {
  const menus = ["File", "Edit", "Assets", "GameObject", "Component", "Window", "Help"];

  return (
    <nav aria-label="Application menu" className="menu-bar visible" data-tauri-drag-region>
      <div className="menu-items">
        {menus.map((menu) => (
          <button key={menu}>{menu}</button>
        ))}
      </div>
      <StatusPill tone={bridgeStatus === "tauri" ? "good" : "warn"}>
        {bridgeStatus === "tauri" ? "Tauri" : "Browser"}
      </StatusPill>
    </nav>
  );
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
  selectedEntity,
  viewMode,
  onModeChange,
  onViewModeChange,
}: {
  mode: StudioMode;
  selectedEntity: SceneEntity | null;
  viewMode: ViewMode;
  onModeChange: (mode: StudioMode) => void;
  onViewModeChange: (mode: ViewMode) => void;
}) {
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
          <ToolButton label="Play">
            <Play size={15} strokeWidth={ICON_STROKE} />
          </ToolButton>
          <ToolButton label="Pause">
            <Pause size={15} strokeWidth={ICON_STROKE} />
          </ToolButton>
          <ToolButton label="Stop">
            <Square size={14} strokeWidth={ICON_STROKE} />
          </ToolButton>
        </div>
      </div>
      <div className="stage-status">
        <StatusPill>{mode}</StatusPill>
        <StatusPill>{selectedEntity ? selectedEntity.name : "No selection"}</StatusPill>
        <StatusPill tone="good">Saved</StatusPill>
      </div>
    </div>
  );
}

function SceneViewport({
  entities,
  mode,
  viewMode,
  selectedEntityId,
  dragState,
  onAssetDrop,
  onDragStateChange,
  onSelectEntity,
  onUpdateTransform,
}: {
  entities: SceneEntity[];
  mode: StudioMode;
  viewMode: ViewMode;
  selectedEntityId: string;
  dragState: DragState | null;
  onAssetDrop: (asset: ProjectAsset) => void;
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
    const direction = event.deltaY < 0 ? 1 : -1;
    updateActiveCamera((camera) => ({
      ...camera,
      zoom: clamp(Number((camera.zoom + direction * 0.08).toFixed(2)), 0.45, 2.4),
    }));
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
    selectedEntityId,
    onDragStateChange,
    onEntityPointerDown: handleEntityPointerDown,
    onEntityPointerMove: handleEntityPointerMove,
    onResetCamera: resetActiveCamera,
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
      {viewMode === "3d" ? <Viewport3D {...viewportProps} /> : <Viewport2D {...viewportProps} />}
    </div>
  );
}

interface ViewportRendererProps {
  camera: ViewportCamera;
  dragState: DragState | null;
  entities: SceneEntity[];
  mode: StudioMode;
  selectedEntityId: string;
  onDragStateChange: (state: DragState | null) => void;
  onEntityPointerDown: (entity: SceneEntity, event: PointerEvent<HTMLButtonElement>) => void;
  onEntityPointerMove: (entity: SceneEntity, event: PointerEvent<HTMLButtonElement>) => void;
  onResetCamera: () => void;
  onSelectEntity: (entity: SceneEntity) => void;
  onUpdateTransform: (id: string, transform: Transform3D) => void;
}

function Viewport3D({
  entities,
  mode,
  selectedEntityId,
  onResetCamera,
  onSelectEntity,
  onUpdateTransform,
}: ViewportRendererProps) {
  const transformMode = mode === "rotate" ? "rotate" : mode === "scale" ? "scale" : "translate";

  return (
    <>
      <Canvas className="viewport-canvas" shadows>
        <PerspectiveCamera makeDefault position={[7.5, 6, 8]} fov={48} />
        <color attach="background" args={["#15171b"]} />
        <ambientLight intensity={0.55} />
        <directionalLight castShadow intensity={1.2} position={[6, 8, 5]} />
        <Grid
          args={[24, 24]}
          cellColor="#343943"
          cellSize={1}
          fadeDistance={24}
          fadeStrength={1.8}
          sectionColor="#44607f"
          sectionSize={4}
        />
        <group>
          {entities.map((entity) => {
            const selected = entity.id === selectedEntityId;
            return (
              <Viewport3DEntity
                entity={entity}
                key={entity.id}
                mode={mode}
                selected={selected}
                transformMode={transformMode}
                onSelect={onSelectEntity}
                onUpdateTransform={onUpdateTransform}
              />
            );
          })}
        </group>
        <OrbitControls
          enableDamping
          makeDefault
          maxDistance={26}
          minDistance={3.5}
          mouseButtons={{
            MIDDLE: MOUSE.ROTATE,
            RIGHT: MOUSE.PAN,
          }}
          screenSpacePanning
        />
      </Canvas>
      <div className="viewport-overlay-top">
        <StatusPill>Scene 3D</StatusPill>
        <StatusPill>MMB Orbit</StatusPill>
        <StatusPill>{mode === "select" ? "Select" : transformMode}</StatusPill>
        <StatusPill>Perspective</StatusPill>
        <StatusPill>Pivot</StatusPill>
        <button className="viewport-reset" onClick={onResetCamera}>Reset</button>
      </div>
      <div className="viewport-axis axis-x">X</div>
      <div className="viewport-axis axis-y">Y</div>
      <div className="viewport-axis axis-z">Z</div>
      <div className="viewport-footer">
        <span>Viewport 3D</span>
        <span>{entities.length} objects</span>
      </div>
    </>
  );
}

function Viewport3DEntity({
  entity,
  mode,
  selected,
  transformMode,
  onSelect,
  onUpdateTransform,
}: {
  entity: SceneEntity;
  mode: StudioMode;
  selected: boolean;
  transformMode: "translate" | "rotate" | "scale";
  onSelect: (entity: SceneEntity) => void;
  onUpdateTransform: (id: string, transform: Transform3D) => void;
}) {
  const groupRef = useRef<Group | null>(null);
  const isCamera = entity.components.some((component) => component.kind === "camera3d");
  const isModel = entity.components.some((component) => component.kind === "mesh3d" || component.kind === "model3d");
  const color = selected ? "#68a4ff" : isCamera ? "#72c08b" : isModel ? "#d9a05c" : "#9aa4b5";
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
    onUpdateTransform(entity.id, {
      x: Math.round(group.position.x / 0.04),
      y: Math.round(group.position.z / 0.04),
      z: Math.round((group.position.y - 0.45) / 0.04),
      rotationX: Math.round((group.rotation.x * 180) / Math.PI),
      rotationY: Math.round((group.rotation.y * 180) / Math.PI),
      rotationZ: Math.round((group.rotation.z * 180) / Math.PI),
      scaleX: Number(Math.max(0.1, group.scale.x).toFixed(2)),
      scaleY: Number(Math.max(0.1, group.scale.z).toFixed(2)),
      scaleZ: Number(Math.max(0.1, group.scale.y).toFixed(2)),
    });
  }

  const entityNode = (
    <group ref={groupRef} position={position} rotation={[
      (entity.transform.rotationX * Math.PI) / 180,
      (entity.transform.rotationY * Math.PI) / 180,
      (entity.transform.rotationZ * Math.PI) / 180,
    ]} scale={scale}>
      <mesh
        castShadow
        onClick={(event) => {
          event.stopPropagation();
          onSelect(entity);
        }}
      >
        {isCamera ? <coneGeometry args={[0.45, 0.9, 4]} /> : <boxGeometry args={[0.9, 0.9, 0.9]} />}
        <meshStandardMaterial color={color} roughness={0.55} metalness={0.08} />
      </mesh>
      {selected ? (
        <group>
          <mesh scale={[1.12, 1.12, 1.12]}>
            {isCamera ? <coneGeometry args={[0.45, 0.9, 4]} /> : <boxGeometry args={[0.9, 0.9, 0.9]} />}
            <meshBasicMaterial color="#68a4ff" wireframe />
          </mesh>
          <mesh position={[0.9, 0, 0]} rotation={[0, 0, Math.PI / 2]}>
            <cylinderGeometry args={[0.025, 0.025, 1.8, 8]} />
            <meshBasicMaterial color="#df746c" />
          </mesh>
          <mesh position={[0, 0.9, 0]}>
            <cylinderGeometry args={[0.025, 0.025, 1.8, 8]} />
            <meshBasicMaterial color="#72c08b" />
          </mesh>
          <mesh position={[0, 0, 0.9]} rotation={[Math.PI / 2, 0, 0]}>
            <cylinderGeometry args={[0.025, 0.025, 1.8, 8]} />
            <meshBasicMaterial color="#68a4ff" />
          </mesh>
        </group>
      ) : null}
    </group>
  );

  if (!selected || mode === "select") return entityNode;

  return (
    <TransformControls mode={transformMode} onObjectChange={commitTransform} size={0.75}>
      {entityNode}
    </TransformControls>
  );
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
  entity,
  scripts,
  onAttachScript,
  onEntityChange,
  onTransformChange,
}: {
  entity: SceneEntity | null;
  scripts: ScriptDocument[];
  onAttachScript: (path: string) => void;
  onEntityChange: (patch: Partial<SceneEntity>) => void;
  onTransformChange: (patch: Partial<Transform3D>) => void;
}) {
  if (!entity) {
    return (
      <aside className="side-panel inspector-panel">
        <PanelHeader icon={<Settings2 size={15} strokeWidth={ICON_STROKE} />} title="Inspector" meta="Properties" />
        <div className="empty-state">No object selected.</div>
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

        <InspectorGroup title="Components">
          {entity.components.map((component) => (
            <div className="component-row" key={`${component.kind}-${component.asset ?? component.script ?? ""}`}>
              <Component size={15} strokeWidth={ICON_STROKE} />
              <div>
                <strong>{component.kind}</strong>
                <span>{component.asset ?? component.script ?? component.profile ?? "configured"}</span>
              </div>
            </div>
          ))}
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
      </div>
    </aside>
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

function InspectorGroup({ children, title }: { children: React.ReactNode; title: string }) {
  return (
    <section className="inspector-group">
      <h3>{title}</h3>
      {children}
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
  label,
  onClick,
}: {
  active?: boolean;
  children: React.ReactNode;
  label: string;
  onClick?: () => void;
}) {
  return (
    <button className={`tool-button ${active ? "active" : ""}`} onClick={onClick} title={label}>
      {children}
    </button>
  );
}

function StatusPill({ children, tone = "neutral" }: { children: React.ReactNode; tone?: "neutral" | "good" | "warn" }) {
  return <span className={`status-pill ${tone}`}>{children}</span>;
}

function entityScript(entity: SceneEntity): string | undefined {
  return entity.components.find((component) => component.kind === "script")?.script;
}

function entityClass(entity: SceneEntity): string {
  if (entity.components.some((component) => component.kind === "camera3d")) return "camera";
  if (entity.components.some((component) => component.kind === "cube3d")) return "cube";
  if (entity.components.some((component) => component.kind === "model3d")) return "model";
  return "default";
}

function entityGlyph(entity: SceneEntity) {
  if (entity.components.some((component) => component.kind === "camera3d")) {
    return <Maximize2 size={16} strokeWidth={ICON_STROKE} />;
  }
  if (entity.components.some((component) => component.kind === "cube3d")) {
    return <Cuboid size={17} strokeWidth={ICON_STROKE} />;
  }
  return <Box size={17} strokeWidth={ICON_STROKE} />;
}

function entityIcon(entity: SceneEntity) {
  if (entity.components.some((component) => component.kind === "camera3d")) {
    return <Maximize2 size={12} strokeWidth={ICON_STROKE} />;
  }
  if (entity.components.some((component) => component.kind === "script")) {
    return <Braces size={12} strokeWidth={ICON_STROKE} />;
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

function clamp(value: number, min: number, max: number): number {
  return Math.min(Math.max(value, min), max);
}
