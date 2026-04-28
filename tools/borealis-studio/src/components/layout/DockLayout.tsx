import React, { PointerEvent, useMemo } from "react";
import { ChevronDown, ChevronRight } from "lucide-react";
import { clamp } from "../../utils/viewport";
import type {
  AssetDragState,
  ConsoleLine,
  DockPanelId,
  DockRegionId,
  DragState,
  LayoutState,
  PreviewStatus,
  ProjectAsset,
  SceneComponent,
  SceneDocument,
  SceneEntity,
  ScriptDocument,
  SelectionTarget,
  StudioMode,
  StudioPreferences,
  Transform3D,
  ViewMode,
  ViewportTab,
} from "../../types";
import { HierarchyPanel } from "../hierarchy/HierarchyPanel";
import { InspectorPanel } from "../inspector/InspectorPanel";
import { ConsolePanel } from "../project/ConsolePanel";
import { AnimationPanel, ProjectPanel } from "../project/ProjectPanel";
import { SceneGameHost } from "../viewport/SceneGameHost";

export function DockLayout({
  assetDrag,
  assets,
  consoleLines,
  dragState,
  entities,
  expandedEntityIds,
  hierarchySearch,
  layout,
  mode,
  preferences,
  previewStatus,
  scene,
  sceneRootSelected,
  scripts,
  selectedEntity,
  selectedEntityId,
  selectionTarget,
  viewMode,
  viewportGizmos,
  viewportPivotMode,
  viewportSpaceMode,
  viewportTab,
  onAssetDrop,
  onAssetPointerDown,
  onAttachScript,
  onClearConsole,
  onClearSelection,
  onComponentChange,
  onDockTabActivate,
  onDragStateChange,
  onEntityChange,
  onMoveDockPanel,
  onOpenScript,
  onReparentEntity,
  onResizeRegion,
  onSceneChange,
  onScriptSelect,
  onSelectEntity,
  onSelectSceneRoot,
  onSetHierarchySearch,
  onSetMode,
  onSetViewMode,
  onSetViewportGizmos,
  onSetViewportPivotMode,
  onSetViewportSpaceMode,
  onSetViewportTab,
  onToggleHierarchyExpanded,
  onToggleRegionCollapse,
  onTransformChange,
  onUpdateTransform,
}: {
  assetDrag: AssetDragState | null;
  assets: ProjectAsset[];
  consoleLines: ConsoleLine[];
  dragState: DragState | null;
  entities: SceneEntity[];
  expandedEntityIds: string[];
  hierarchySearch: string;
  layout: LayoutState;
  mode: StudioMode;
  preferences: StudioPreferences;
  previewStatus: PreviewStatus;
  scene: SceneDocument;
  sceneRootSelected: boolean;
  scripts: ScriptDocument[];
  selectedEntity: SceneEntity | null;
  selectedEntityId: string;
  selectionTarget: SelectionTarget;
  viewMode: ViewMode;
  viewportGizmos: boolean;
  viewportPivotMode: "pivot" | "center";
  viewportSpaceMode: "global" | "local";
  viewportTab: ViewportTab;
  onAssetDrop: (asset: ProjectAsset, transform?: Partial<Transform3D>) => void;
  onAssetPointerDown: (asset: ProjectAsset, event: PointerEvent<HTMLElement>) => void;
  onAttachScript: (path: string) => void;
  onClearConsole: () => void;
  onClearSelection: () => void;
  onComponentChange: (index: number, component: SceneComponent) => void;
  onDockTabActivate: (region: DockRegionId, panelId: DockPanelId) => void;
  onDragStateChange: (state: DragState | null) => void;
  onEntityChange: (patch: Partial<SceneEntity>) => void;
  onMoveDockPanel: (panelId: DockPanelId, region: DockRegionId) => void;
  onOpenScript: (path: string) => void;
  onReparentEntity: (entityId: string, newParentId?: string) => void;
  onResizeRegion: (region: DockRegionId, size: number) => void;
  onSceneChange: (patch: Partial<SceneDocument>) => void;
  onScriptSelect: (path: string) => void;
  onSelectEntity: (entity: SceneEntity) => void;
  onSelectSceneRoot: () => void;
  onSetHierarchySearch: (value: string) => void;
  onSetMode: (mode: StudioMode) => void;
  onSetViewMode: (mode: ViewMode) => void;
  onSetViewportGizmos: (enabled: boolean) => void;
  onSetViewportPivotMode: (mode: "pivot" | "center") => void;
  onSetViewportSpaceMode: (mode: "global" | "local") => void;
  onSetViewportTab: (tab: ViewportTab) => void;
  onToggleHierarchyExpanded: (entityId: string) => void;
  onToggleRegionCollapse: (region: DockRegionId) => void;
  onTransformChange: (patch: Partial<Transform3D>) => void;
  onUpdateTransform: (id: string, transform: Transform3D) => void;
}) {
  const leftWidth = layout.docks.leftDock.collapsed ? 0 : layout.docks.leftDock.size;
  const rightWidth = layout.docks.rightDock.collapsed ? 0 : layout.docks.rightDock.size;
  const bottomHeight = layout.docks.bottomDock.collapsed ? 0 : layout.docks.bottomDock.size;
  const leftColumns = leftWidth > 0 ? `${leftWidth}px 6px minmax(0, 1fr)` : "minmax(0, 1fr)";
  const workspaceColumns = rightWidth > 0 ? `minmax(0, 1fr) 6px ${rightWidth}px` : "minmax(0, 1fr)";
  const workspaceRows = bottomHeight > 0 ? `minmax(0, 1fr) 6px ${bottomHeight}px` : "minmax(0, 1fr)";

  const panelRenderers = useMemo<Record<DockPanelId, React.ReactNode>>(
    () => ({
      hierarchy: (
        <HierarchyPanel
          entities={entities}
          expandedEntityIds={expandedEntityIds}
          onReparentEntity={onReparentEntity}
          onSearchChange={onSetHierarchySearch}
          onSelectEntity={onSelectEntity}
          onSelectSceneRoot={onSelectSceneRoot}
          onToggleExpanded={onToggleHierarchyExpanded}
          sceneName={scene.name}
          sceneRootSelected={sceneRootSelected}
          search={hierarchySearch}
          selectedEntityId={selectedEntityId}
        />
      ),
      scene: (
        <SceneGameHost
          activeTab={viewportTab}
          consoleLines={consoleLines}
          dragState={dragState}
          entities={entities}
          gizmos={viewportGizmos}
          mode={mode}
          onAssetDrop={onAssetDrop}
          onClearSelection={onClearSelection}
          onDragStateChange={onDragStateChange}
          onSelectEntity={onSelectEntity}
          onSetActiveTab={onSetViewportTab}
          onSetGizmos={onSetViewportGizmos}
          onSetPivotMode={onSetViewportPivotMode}
          onSetSpaceMode={onSetViewportSpaceMode}
          onSetViewMode={onSetViewMode}
          onUpdateTransform={onUpdateTransform}
          pivotMode={viewportPivotMode}
          preferences={preferences}
          previewStatus={previewStatus}
          scene={scene}
          selectedEntityId={selectedEntityId}
          spaceMode={viewportSpaceMode}
          viewMode={viewMode}
        />
      ),
      game: <div />,
      inspector: (
        <InspectorPanel
          assets={assets}
          entity={selectedEntity}
          onAttachScript={onAttachScript}
          onComponentChange={onComponentChange}
          onEntityChange={onEntityChange}
          onSceneChange={onSceneChange}
          onTransformChange={onTransformChange}
          scene={scene}
          sceneRootSelected={sceneRootSelected || selectionTarget.kind === "scene-root"}
          scripts={scripts}
        />
      ),
      project: (
        <ProjectPanel
          assets={assets}
          onAssetDrop={onAssetDrop}
          onAssetPointerDown={onAssetPointerDown}
          onOpenScript={onOpenScript}
          onScriptSelect={onScriptSelect}
          scripts={scripts}
        />
      ),
      console: <ConsolePanel lines={consoleLines} onClear={onClearConsole} />,
      animation: <AnimationPanel scripts={scripts} />,
    }),
    [
      assetDrag,
      assets,
      consoleLines,
      dragState,
      entities,
      expandedEntityIds,
      hierarchySearch,
      mode,
      onAssetDrop,
      onAssetPointerDown,
      onAttachScript,
      onClearConsole,
      onClearSelection,
      onComponentChange,
      onDragStateChange,
      onEntityChange,
      onOpenScript,
      onReparentEntity,
      onSceneChange,
      onScriptSelect,
      onSelectEntity,
      onSelectSceneRoot,
      onSetHierarchySearch,
      onSetViewMode,
      onSetViewportGizmos,
      onSetViewportPivotMode,
      onSetViewportSpaceMode,
      onSetViewportTab,
      onToggleHierarchyExpanded,
      onTransformChange,
      onUpdateTransform,
      preferences,
      previewStatus,
      scene,
      sceneRootSelected,
      scripts,
      selectedEntity,
      selectedEntityId,
      selectionTarget,
      viewMode,
      viewportGizmos,
      viewportPivotMode,
      viewportSpaceMode,
      viewportTab,
    ],
  );

  function beginResize(region: DockRegionId, event: PointerEvent<HTMLDivElement>) {
    event.preventDefault();
    const startX = event.clientX;
    const startY = event.clientY;
    const initial = layout.docks[region].size;

    function onMove(moveEvent: globalThis.PointerEvent) {
      if (region === "leftDock") {
        onResizeRegion(region, clamp(initial + (moveEvent.clientX - startX), 240, 460));
      } else if (region === "rightDock") {
        onResizeRegion(region, clamp(initial - (moveEvent.clientX - startX), 280, 460));
      } else {
        onResizeRegion(region, clamp(initial + (startY - moveEvent.clientY), 180, 360));
      }
    }

    function onUp() {
      window.removeEventListener("pointermove", onMove);
      window.removeEventListener("pointerup", onUp);
    }

    window.addEventListener("pointermove", onMove);
    window.addEventListener("pointerup", onUp);
  }

  return (
    <div className="dock-shell">
      <div className="dock-workspace-grid" style={{ gridTemplateColumns: workspaceColumns }}>
        <div className="dock-work-area" style={{ gridTemplateRows: workspaceRows }}>
          <div className="dock-top-area" style={{ gridTemplateColumns: leftColumns }}>
            {leftWidth > 0 ? (
              <DockRegionFrame
                activeTab={layout.docks.leftDock.activeTab}
                collapsed={layout.docks.leftDock.collapsed}
                panelRenderers={panelRenderers}
                region="leftDock"
                tabs={layout.docks.leftDock.tabs}
                title="Left dock"
                onActivate={onDockTabActivate}
                onMoveDockPanel={onMoveDockPanel}
                onToggleCollapse={onToggleRegionCollapse}
              />
            ) : null}
            {leftWidth > 0 ? <div className="dock-resize-handle vertical" onPointerDown={(event) => beginResize("leftDock", event)} /> : null}
            <div className="dock-center-stage">{panelRenderers.scene}</div>
          </div>
          {bottomHeight > 0 ? <div className="dock-resize-handle horizontal" onPointerDown={(event) => beginResize("bottomDock", event)} /> : null}
          {bottomHeight > 0 ? (
            <DockRegionFrame
              activeTab={layout.docks.bottomDock.activeTab}
              collapsed={layout.docks.bottomDock.collapsed}
              panelRenderers={panelRenderers}
              region="bottomDock"
              tabs={layout.docks.bottomDock.tabs}
              title="Bottom dock"
              onActivate={onDockTabActivate}
              onMoveDockPanel={onMoveDockPanel}
              onToggleCollapse={onToggleRegionCollapse}
            />
          ) : null}
        </div>
        {rightWidth > 0 ? <div className="dock-resize-handle vertical reverse" onPointerDown={(event) => beginResize("rightDock", event)} /> : null}
        {rightWidth > 0 ? (
          <DockRegionFrame
            activeTab={layout.docks.rightDock.activeTab}
            collapsed={layout.docks.rightDock.collapsed}
            panelRenderers={panelRenderers}
            region="rightDock"
            tabs={layout.docks.rightDock.tabs}
            title="Right dock"
            onActivate={onDockTabActivate}
            onMoveDockPanel={onMoveDockPanel}
            onToggleCollapse={onToggleRegionCollapse}
          />
        ) : null}
      </div>
    </div>
  );
}

function DockRegionFrame({
  activeTab,
  collapsed,
  panelRenderers,
  region,
  tabs,
  title,
  onActivate,
  onMoveDockPanel,
  onToggleCollapse,
}: {
  activeTab: DockPanelId;
  collapsed: boolean;
  panelRenderers: Record<DockPanelId, React.ReactNode>;
  region: DockRegionId;
  tabs: DockPanelId[];
  title: string;
  onActivate: (region: DockRegionId, panelId: DockPanelId) => void;
  onMoveDockPanel: (panelId: DockPanelId, region: DockRegionId) => void;
  onToggleCollapse: (region: DockRegionId) => void;
}) {
  return (
    <section
      className={`dock-region dock-region-${region}`}
      onDragOver={(event) => event.preventDefault()}
      onDrop={(event) => {
        event.preventDefault();
        const panelId = event.dataTransfer.getData("application/x-borealis-panel") as DockPanelId;
        if (panelId) onMoveDockPanel(panelId, region);
      }}
    >
      <div className="dock-tabbar">
        <div className="dock-tab-buttons">
          {tabs.map((tab) => (
            <button
              className={`dock-tab-button ${activeTab === tab ? "active" : ""}`}
              draggable={tab !== "scene" && tab !== "game"}
              key={tab}
              onClick={() => onActivate(region, tab)}
              onDragStart={(event) => {
                event.dataTransfer.setData("application/x-borealis-panel", tab);
              }}
              type="button"
            >
              {panelLabel(tab)}
            </button>
          ))}
        </div>
        <button className="dock-tab-collapse" onClick={() => onToggleCollapse(region)} title={title} type="button">
          {collapsed ? <ChevronRight size={14} /> : <ChevronDown size={14} />}
        </button>
      </div>
      <div className="dock-region-content">{panelRenderers[activeTab]}</div>
    </section>
  );
}

function panelLabel(panelId: DockPanelId): string {
  switch (panelId) {
    case "hierarchy":
      return "Hierarchy";
    case "scene":
      return "Scene";
    case "game":
      return "Game";
    case "inspector":
      return "Inspector";
    case "project":
      return "Project";
    case "console":
      return "Console";
    case "animation":
      return "Animation";
    default:
      return panelId;
  }
}
