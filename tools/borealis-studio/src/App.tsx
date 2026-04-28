import React, { PointerEvent, useEffect } from "react";
import { configureBorealisCatalog } from "./borealisCatalog";
import { pollRuntimePreview } from "./backend";
import { GlobalHeader } from "./components/layout/GlobalHeader";
import { DockLayout } from "./components/layout/DockLayout";
import { ToolRail } from "./components/layout/ToolRail";
import { ScriptEditorWindow } from "./components/ScriptEditor";
import { SettingsWindow } from "./components/SettingsWindow";
import { StartScreen } from "./components/StartScreen";
import { StatusBar } from "./components/status/StatusBar";
import { useActiveScript, useSceneRootSelected, useSelectedEntity, useStudioStore } from "./store/useStudioStore";
import { modeForShortcut } from "./utils/keybinds";
import { normalizePreviewStatus } from "./utils/preview";

export function BorealisStudioApp() {
  const store = useStudioStore();
  const selectedEntity = useSelectedEntity();
  const activeScript = useActiveScript();
  const sceneRootSelected = useSceneRootSelected();
  const consoleLines = useStudioStore((state) => state.snapshot.console);
  const consoleCounts = {
    warnings: consoleLines.filter((line) => line.level === "warn").length,
    errors: consoleLines.filter((line) => line.level === "error").length,
  };

  useEffect(() => {
    configureBorealisCatalog(store.home.editorManifest);
  }, [store.home.editorManifest]);

  useEffect(() => store.initBridge(), []);

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
      const shortcutMode = modeForShortcut(store.preferences.shortcutTemplate, key);
      if (shortcutMode) store.setMode(shortcutMode);
    }
    window.addEventListener("keydown", handleToolShortcut);
    return () => window.removeEventListener("keydown", handleToolShortcut);
  }, [store.preferences.shortcutTemplate]);

  useEffect(() => {
    if (store.bridgeStatus !== "tauri") return;
    if (!["starting", "loading", "ready", "playing", "paused"].includes(store.previewStatus)) return;
    const timer = window.setInterval(() => {
      pollRuntimePreview()
        .then((result) => {
          if (result.events.length > 0) {
            useStudioStore.getState().applyPreviewResult(result);
          } else {
            useStudioStore.getState().setPreviewStatus(normalizePreviewStatus(result.status));
          }
        })
        .catch(() => {
          useStudioStore.getState().setPreviewStatus("error");
        });
    }, 1200);
    return () => window.clearInterval(timer);
  }, [store.bridgeStatus, store.previewStatus]);

  useEffect(() => {
    if (!store.assetDrag) return;
    const draggedAsset = store.assetDrag.asset;

    function onMove(event: globalThis.PointerEvent) {
      const current = useStudioStore.getState().assetDrag;
      if (current) {
        useStudioStore.setState({ assetDrag: { ...current, x: event.clientX, y: event.clientY } });
      }
    }

    function onUp(event: globalThis.PointerEvent) {
      const target = document.elementFromPoint(event.clientX, event.clientY);
      if (target instanceof Element && target.closest(".scene-viewport")) {
        useStudioStore.getState().createEntityFromAsset(draggedAsset);
      }
      useStudioStore.setState({ assetDrag: null });
    }

    window.addEventListener("pointermove", onMove);
    window.addEventListener("pointerup", onUp);
    window.addEventListener("pointercancel", onUp);
    return () => {
      window.removeEventListener("pointermove", onMove);
      window.removeEventListener("pointerup", onUp);
      window.removeEventListener("pointercancel", onUp);
    };
  }, [store.assetDrag?.asset?.id]);

  function beginAssetDrag(asset: import("./types").ProjectAsset, event: PointerEvent<HTMLElement>) {
    if (asset.kind === "script") return;
    store.setAssetDrag({ asset, x: event.clientX, y: event.clientY });
  }

  return (
    <div className={`studio-shell-unity theme-${store.preferences.theme}`}>
      <GlobalHeader
        assets={store.snapshot.assets}
        bridgeStatus={store.bridgeStatus}
        entities={store.snapshot.scene.entities}
        onOpenDefaultProject={store.openDefaultProject}
        onOpenScript={store.openScriptEditor}
        onOpenSettings={() => store.setSettingsOpen(true)}
        onPausePreview={store.pausePreview}
        onResetLayout={store.resetLayout}
        onSelectEntity={store.setSelectedEntityId}
        onShowHome={() => store.setHomeVisible(true)}
        onStartPreview={store.startPreview}
        previewBusy={store.previewBusy}
        previewStatus={store.previewStatus}
        projectName={store.snapshot.projectName}
        scripts={store.snapshot.scripts}
      />

      {store.homeVisible ? (
        <StartScreen
          bridgeStatus={store.bridgeStatus}
          busy={store.homeBusy}
          error={store.homeError}
          home={store.home}
          recentProjects={store.recentProjects}
          onClearRecentProjects={store.clearRecentProjects}
          onCreateProject={store.createProject}
          onOpenDefaultProject={store.openDefaultProject}
          onOpenProject={store.openProject}
        />
      ) : (
        <div className="studio-workspace-unity">
          <ToolRail mode={store.mode} onModeChange={store.setMode} />
          <DockLayout
            assetDrag={store.assetDrag}
            assets={store.snapshot.assets}
            consoleLines={store.snapshot.console}
            dragState={store.dragState}
            entities={store.snapshot.scene.entities}
            expandedEntityIds={store.expandedEntityIds}
            hierarchySearch={store.hierarchySearch}
            layout={store.layout}
            mode={store.mode}
            onAssetDrop={store.createEntityFromAsset}
            onAssetPointerDown={beginAssetDrag}
            onAttachScript={store.attachScript}
            onClearConsole={store.clearConsole}
            onClearSelection={store.clearSelection}
            onComponentChange={(index, component) => {
              if (!selectedEntity) return;
              store.updateSelectedEntity({
                components: selectedEntity.components.map((current, currentIndex) =>
                  currentIndex === index ? component : current,
                ),
              });
            }}
            onDockTabActivate={store.setDockActiveTab}
            onDragStateChange={store.setDragState}
            onEntityChange={store.updateSelectedEntity}
            onMoveDockPanel={store.moveDockPanel}
            onOpenScript={store.openScriptEditor}
            onReparentEntity={(entityId, newParentId) => void store.reparentEntity(entityId, newParentId)}
            onResizeRegion={store.resizeDockRegion}
            onSceneChange={(patch) => store.updateScene((scene) => ({ ...scene, ...patch }))}
            onScriptSelect={store.setSelectedScriptPath}
            onSelectEntity={store.selectEntity}
            onSelectSceneRoot={store.selectSceneRoot}
            onSetHierarchySearch={store.setHierarchySearch}
            onSetMode={store.setMode}
            onSetViewMode={store.setViewMode}
            onSetViewportGizmos={store.setViewportGizmos}
            onSetViewportPivotMode={store.setViewportPivotMode}
            onSetViewportSpaceMode={store.setViewportSpaceMode}
            onSetViewportTab={store.setViewportTab}
            onToggleHierarchyExpanded={store.toggleHierarchyExpanded}
            onToggleRegionCollapse={store.toggleDockCollapse}
            onTransformChange={store.updateSelectedTransform}
            onUpdateTransform={(id, transform) => {
              store.updateScene((scene) => ({
                ...scene,
                entities: scene.entities.map((entity) => (entity.id === id ? { ...entity, transform } : entity)),
              }));
            }}
            preferences={store.preferences}
            previewStatus={store.previewStatus}
            scene={store.snapshot.scene}
            sceneRootSelected={sceneRootSelected}
            scripts={store.snapshot.scripts}
            selectedEntity={selectedEntity}
            selectedEntityId={store.selectedEntityId}
            selectionTarget={store.selectionTarget}
            viewMode={store.viewMode}
            viewportGizmos={store.viewportGizmos}
            viewportPivotMode={store.viewportPivotMode}
            viewportSpaceMode={store.viewportSpaceMode}
            viewportTab={store.viewportTab}
          />
        </div>
      )}

      {!store.homeVisible && store.scriptEditorOpen ? (
        <ScriptEditorWindow
          activeScript={activeScript}
          onAttachScript={store.attachScript}
          onClose={() => store.setScriptEditorOpen(false)}
          onSaveScript={store.saveScript}
          onScriptChange={store.updateScriptContent}
          onScriptSelect={store.setSelectedScriptPath}
          scripts={store.snapshot.scripts}
          selectedEntity={selectedEntity}
        />
      ) : null}

      {store.settingsOpen ? (
        <SettingsWindow
          onClose={() => store.setSettingsOpen(false)}
          onPreferencesChange={store.setPreferences}
          preferences={store.preferences}
        />
      ) : null}

      {!store.homeVisible ? (
        <StatusBar
          bridgeStatus={store.bridgeStatus}
          errors={consoleCounts.errors}
          previewStatus={store.previewStatus}
          projectName={store.snapshot.projectName}
          savingScene={store.savingScene}
          sceneDirty={store.sceneDirty}
          selectedEntityName={selectedEntity?.name}
          selectionTarget={store.selectionTarget}
          warnings={consoleCounts.warnings}
        />
      ) : null}
    </div>
  );
}
