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
  GitBranch,
  Layers3,
  LayoutPanelLeft,
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
  Triangle,
  WandSparkles,
} from "lucide-react";
import { PointerEvent, useEffect, useMemo, useRef, useState } from "react";
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
  ViewTab,
} from "./types";

const ICON_STROKE = 1.75;

type BridgeStatus = "loading" | "tauri" | "browser" | "error";

interface LayoutState {
  left: number;
  right: number;
  bottom: number;
  code: number;
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

export function BorealisStudioApp() {
  const [snapshot, setSnapshot] = useState<StudioSnapshot>(() => createMockSnapshot());
  const [bridgeStatus, setBridgeStatus] = useState<BridgeStatus>("loading");
  const [selectedEntityId, setSelectedEntityId] = useState("player");
  const [selectedScriptPath, setSelectedScriptPath] = useState("src/app/player_controller.zt");
  const [viewTab, setViewTab] = useState<ViewTab>("scene");
  const [bottomTab, setBottomTab] = useState<BottomTab>("assets");
  const [mode, setMode] = useState<StudioMode>("move");
  const [layout, setLayout] = useState<LayoutState>({
    left: 280,
    right: 292,
    bottom: 218,
    code: 360,
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

  function startResize(axis: "left" | "right" | "bottom" | "code", event: PointerEvent<HTMLDivElement>) {
    event.preventDefault();
    const startX = event.clientX;
    const startY = event.clientY;
    const initial = layout[axis];

    function onMove(moveEvent: globalThis.PointerEvent) {
      const delta = axis === "bottom" ? startY - moveEvent.clientY : moveEvent.clientX - startX;
      setLayout((current) => {
        if (axis === "left") return { ...current, left: clamp(initial + delta, 220, 420) };
        if (axis === "right") return { ...current, right: clamp(initial - delta, 240, 420) };
        if (axis === "code") return { ...current, code: clamp(initial - delta, 320, 620) };
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
          "--code-width": `${layout.code}px`,
        } as React.CSSProperties
      }
    >
      <TopBar
        bridgeStatus={bridgeStatus}
        mode={mode}
        projectName={snapshot.projectName}
        sceneName={snapshot.scene.name}
        onModeChange={setMode}
      />

      <div className="studio-workspace">
        <ActivityRail />

        <NavigatorPanel
          assets={snapshot.assets}
          entities={snapshot.scene.entities}
          selectedEntityId={selectedEntityId}
          selectedScriptPath={activeScriptPath}
          scripts={snapshot.scripts}
          onAssetDrop={createEntityFromAsset}
          onAssetPointerDown={beginAssetDrag}
          onScriptSelect={setSelectedScriptPath}
          onSelectEntity={selectEntity}
        />

        <div className="resize-handle resize-handle-vertical" onPointerDown={(event) => startResize("left", event)} />

        <main className="stage-panel">
          <StageToolbar
            activeTab={viewTab}
            mode={mode}
            selectedEntity={selectedEntity}
            onTabChange={setViewTab}
          />

          <div className="stage-content">
            <section className="viewport-wrap">
              <SceneViewport
                entities={snapshot.scene.entities}
                mode={mode}
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

            <div className="resize-handle resize-handle-code" onPointerDown={(event) => startResize("code", event)} />

            <CodePanel
              activeScript={activeScript}
              scripts={snapshot.scripts}
              selectedEntity={selectedEntity}
              onAttachScript={attachScript}
              onSaveScript={saveScript}
              onScriptChange={updateScriptContent}
              onScriptSelect={setSelectedScriptPath}
            />
          </div>
        </main>

        <div className="resize-handle resize-handle-vertical" onPointerDown={(event) => startResize("right", event)} />

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
          onScriptSelect={setSelectedScriptPath}
          onTabChange={setBottomTab}
        />
      </div>

      {assetDrag ? <AssetDragPreview asset={assetDrag.asset} x={assetDrag.x} y={assetDrag.y} /> : null}
    </div>
  );
}

function TopBar({
  bridgeStatus,
  mode,
  projectName,
  sceneName,
  onModeChange,
}: {
  bridgeStatus: BridgeStatus;
  mode: StudioMode;
  projectName: string;
  sceneName: string;
  onModeChange: (mode: StudioMode) => void;
}) {
  return (
    <header className="top-bar" data-tauri-drag-region>
      <div className="window-controls" aria-hidden="true">
        <span />
        <span />
        <span />
      </div>
      <div className="brand-lockup">
        <div className="app-mark">
          <Triangle size={15} strokeWidth={ICON_STROKE} />
        </div>
        <div>
          <strong>{projectName} Studio</strong>
          <span>{sceneName}</span>
        </div>
      </div>

      <div className="command-center">
        <Search size={15} strokeWidth={ICON_STROKE} />
        <input value="Search files, entities, commands" readOnly aria-label="Command search" />
        <Command size={14} strokeWidth={ICON_STROKE} />
      </div>

      <div className="top-actions">
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
        <div className="toolbar-divider" />
        <ToolButton label="Play">
          <Play size={15} strokeWidth={ICON_STROKE} />
        </ToolButton>
        <ToolButton label="Pause">
          <Pause size={15} strokeWidth={ICON_STROKE} />
        </ToolButton>
        <ToolButton label="Stop">
          <Square size={14} strokeWidth={ICON_STROKE} />
        </ToolButton>
        <StatusPill tone={bridgeStatus === "tauri" ? "good" : "warn"}>
          {bridgeStatus === "tauri" ? "Tauri" : "Browser"}
        </StatusPill>
      </div>
    </header>
  );
}

function ActivityRail() {
  const items = [
    { icon: LayoutPanelLeft, label: "Workspace", active: true },
    { icon: FolderTree, label: "Project" },
    { icon: Code2, label: "Scripts" },
    { icon: Terminal, label: "Console" },
    { icon: GitBranch, label: "Source" },
    { icon: Settings2, label: "Settings" },
  ];

  return (
    <aside className="activity-rail">
      {items.map((item) => {
        const Icon = item.icon;
        return (
          <button className={item.active ? "active" : ""} key={item.label} title={item.label}>
            <Icon size={18} strokeWidth={ICON_STROKE} />
          </button>
        );
      })}
    </aside>
  );
}

function NavigatorPanel({
  assets,
  entities,
  scripts,
  selectedEntityId,
  selectedScriptPath,
  onAssetDrop,
  onAssetPointerDown,
  onScriptSelect,
  onSelectEntity,
}: {
  assets: ProjectAsset[];
  entities: SceneEntity[];
  scripts: ScriptDocument[];
  selectedEntityId: string;
  selectedScriptPath: string;
  onAssetDrop: (asset: ProjectAsset) => void;
  onAssetPointerDown: (asset: ProjectAsset, event: PointerEvent<HTMLElement>) => void;
  onScriptSelect: (path: string) => void;
  onSelectEntity: (entity: SceneEntity) => void;
}) {
  return (
    <aside className="side-panel navigator-panel">
      <PanelHeader icon={<FolderTree size={15} strokeWidth={ICON_STROKE} />} title="Navigator" meta="Project" />
      <div className="navigator-section">
        <SectionTitle label="Scenes" />
        {assets
          .filter((asset) => asset.kind === "scene")
          .map((asset) => (
            <AssetRow asset={asset} key={asset.id} onDrop={onAssetDrop} onPointerDown={onAssetPointerDown} />
          ))}
      </div>
      <div className="navigator-section hierarchy-list">
        <SectionTitle label="Hierarchy" />
        {entities.map((entity) => (
          <button
            className={`tree-row ${selectedEntityId === entity.id ? "selected" : ""}`}
            key={entity.id}
            onClick={() => onSelectEntity(entity)}
          >
            {entityIcon(entity)}
            <span>{entity.name}</span>
            <small>{entity.layer}</small>
          </button>
        ))}
      </div>
      <div className="navigator-section">
        <SectionTitle label="Scripts" />
        {scripts.map((script) => (
          <button
            className={`tree-row ${selectedScriptPath === script.path ? "selected" : ""}`}
            key={script.path}
            onClick={() => onScriptSelect(script.path)}
          >
            <FileCode2 size={15} strokeWidth={ICON_STROKE} />
            <span>{script.name}</span>
            {script.dirty ? <small>dirty</small> : null}
          </button>
        ))}
      </div>
    </aside>
  );
}

function StageToolbar({
  activeTab,
  mode,
  selectedEntity,
  onTabChange,
}: {
  activeTab: ViewTab;
  mode: StudioMode;
  selectedEntity: SceneEntity | null;
  onTabChange: (tab: ViewTab) => void;
}) {
  return (
    <div className="stage-toolbar">
      <Segmented tabs={["scene", "game", "script"]} value={activeTab} onChange={onTabChange} />
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
  selectedEntityId,
  dragState,
  onAssetDrop,
  onDragStateChange,
  onSelectEntity,
  onUpdateTransform,
}: {
  entities: SceneEntity[];
  mode: StudioMode;
  selectedEntityId: string;
  dragState: DragState | null;
  onAssetDrop: (asset: ProjectAsset) => void;
  onDragStateChange: (state: DragState | null) => void;
  onSelectEntity: (entity: SceneEntity) => void;
  onUpdateTransform: (id: string, transform: Transform3D) => void;
}) {
  const viewportRef = useRef<HTMLDivElement | null>(null);

  function handleEntityPointerDown(entity: SceneEntity, event: PointerEvent<HTMLButtonElement>) {
    event.preventDefault();
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
      x: Math.round(dragState.originX + deltaX * 0.75),
      y: Math.round(dragState.originY + deltaY * 0.75),
    });
  }

  function handleDrop(event: React.DragEvent<HTMLDivElement>) {
    event.preventDefault();
    const raw = event.dataTransfer.getData("application/x-borealis-asset");
    if (!raw) return;
    onAssetDrop(JSON.parse(raw) as ProjectAsset);
  }

  return (
    <div
      className="scene-viewport"
      onDragOver={(event) => event.preventDefault()}
      onDrop={handleDrop}
      ref={viewportRef}
    >
      <div className="viewport-grid" />
      <div className="viewport-axis axis-x">X</div>
      <div className="viewport-axis axis-y">Y</div>
      <div className="viewport-axis axis-z">Z</div>
      <div className="viewport-overlay-top">
        <StatusPill>Scene 3D</StatusPill>
        <StatusPill>Global</StatusPill>
        <StatusPill>Pivot</StatusPill>
      </div>
      {entities.map((entity) => {
        const selected = entity.id === selectedEntityId;
        const position = project3d(entity.transform);
        return (
          <button
            className={`scene-object ${selected ? "selected" : ""} ${entityClass(entity)}`}
            key={entity.id}
            onClick={() => onSelectEntity(entity)}
            onPointerDown={(event) => handleEntityPointerDown(entity, event)}
            onPointerMove={(event) => handleEntityPointerMove(entity, event)}
            onPointerUp={() => onDragStateChange(null)}
            style={{
              left: "50%",
              top: "50%",
              transform: `translate(calc(-50% + ${position.x}px), calc(-50% + ${position.y}px))`,
            }}
          >
            <span className="object-glyph">{entityGlyph(entity)}</span>
            <span className="object-label">{entity.name}</span>
            {selected ? <ObjectGizmo /> : null}
          </button>
        );
      })}
      <div className="viewport-footer">
        <span>Viewport</span>
        <span>{entities.length} objects</span>
      </div>
    </div>
  );
}

function ObjectGizmo() {
  return (
    <div className="object-gizmo" aria-hidden="true">
      <span className="gizmo-x" />
      <span className="gizmo-y" />
      <span className="gizmo-z" />
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
  onScriptSelect,
  onTabChange,
}: {
  activeTab: BottomTab;
  assets: ProjectAsset[];
  console: ConsoleLine[];
  scripts: ScriptDocument[];
  onAssetDrop: (asset: ProjectAsset) => void;
  onAssetPointerDown: (asset: ProjectAsset, event: PointerEvent<HTMLElement>) => void;
  onScriptSelect: (path: string) => void;
  onTabChange: (tab: BottomTab) => void;
}) {
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
        <div className="asset-grid">
          {assets.map((asset) => (
            <AssetTile
              asset={asset}
              key={asset.id}
              onDrop={onAssetDrop}
              onPointerDown={onAssetPointerDown}
              onScriptSelect={onScriptSelect}
            />
          ))}
        </div>
      ) : null}
      {activeTab === "console" ? <ConsoleView lines={console} /> : null}
      {activeTab === "problems" ? <ProblemsView scripts={scripts} /> : null}
    </section>
  );
}

function AssetRow({
  asset,
  onDrop,
  onPointerDown,
}: {
  asset: ProjectAsset;
  onDrop: (asset: ProjectAsset) => void;
  onPointerDown: (asset: ProjectAsset, event: PointerEvent<HTMLElement>) => void;
}) {
  return (
    <button
      className="tree-row"
      draggable
      onDoubleClick={() => onDrop(asset)}
      onPointerDown={(event) => onPointerDown(asset, event)}
      onDragStart={(event) => {
        event.dataTransfer.setData("application/x-borealis-asset", JSON.stringify(asset));
      }}
    >
      {assetKindIcon(asset.kind)}
      <span>{asset.name}</span>
      <small>{asset.kind}</small>
    </button>
  );
}

function AssetTile({
  asset,
  onDrop,
  onPointerDown,
  onScriptSelect,
}: {
  asset: ProjectAsset;
  onDrop: (asset: ProjectAsset) => void;
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
        if (asset.kind === "model" || asset.kind === "texture") onDrop(asset);
      }}
      onPointerDown={(event) => onPointerDown(asset, event)}
      onDragStart={(event) => {
        event.dataTransfer.setData("application/x-borealis-asset", JSON.stringify(asset));
      }}
    >
      <span>{assetKindIcon(asset.kind, 22)}</span>
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
      <ChevronDown size={13} strokeWidth={ICON_STROKE} />
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
    return <Maximize2 size={15} strokeWidth={ICON_STROKE} />;
  }
  if (entity.components.some((component) => component.kind === "script")) {
    return <Braces size={15} strokeWidth={ICON_STROKE} />;
  }
  return <Box size={15} strokeWidth={ICON_STROKE} />;
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

function clamp(value: number, min: number, max: number): number {
  return Math.min(Math.max(value, min), max);
}
