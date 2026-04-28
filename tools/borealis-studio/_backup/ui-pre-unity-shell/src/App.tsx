import React, { PointerEvent, useEffect } from "react";
import { configureBorealisCatalog } from "./borealisCatalog";
import { pollRuntimePreview } from "./backend";
import { AssetDragPreview, BottomDock } from "./components/BottomDock";
import { InspectorPanel } from "./components/InspectorPanel";
import { MenuBar } from "./components/MenuBar";
import { NavigatorPanel } from "./components/NavigatorPanel";
import { SceneViewport } from "./components/SceneViewport";
import { ScriptEditorWindow } from "./components/ScriptEditor";
import { SettingsWindow } from "./components/SettingsWindow";
import { StageToolbar } from "./components/StageToolbar";
import { StartScreen } from "./components/StartScreen";
import { useStudioStore, useSelectedEntity, useActiveScript } from "./store/useStudioStore";
import { modeForShortcut } from "./utils/keybinds";
import { normalizePreviewStatus } from "./utils/preview";
import { clamp } from "./utils/viewport";

export function BorealisStudioApp() {
  const store = useStudioStore();
  const selectedEntity = useSelectedEntity();
  const activeScript = useActiveScript();

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

  function startResize(axis: "left" | "right" | "bottom", event: PointerEvent<HTMLDivElement>) {
    event.preventDefault();
    const startX = event.clientX;
    const startY = event.clientY;
    const initial = store.layout[axis];

    function onMove(moveEvent: globalThis.PointerEvent) {
      const delta = axis === "bottom" ? startY - moveEvent.clientY : moveEvent.clientX - startX;
      if (axis === "left") useStudioStore.setState({ layout: { ...useStudioStore.getState().layout, left: clamp(initial + delta, 220, 420) } });
      else if (axis === "right") useStudioStore.setState({ layout: { ...useStudioStore.getState().layout, right: clamp(initial - delta, 240, 420) } });
      else useStudioStore.setState({ layout: { ...useStudioStore.getState().layout, bottom: clamp(initial + delta, 156, 320) } });
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
      className={`studio-shell theme-${store.preferences.theme}`}
      style={
        {
          "--left-width": `${store.layout.left}px`,
          "--right-width": `${store.layout.right}px`,
          "--bottom-height": `${store.layout.bottom}px`,
        } as React.CSSProperties
      }
    >
      <MenuBar
        bridgeStatus={store.bridgeStatus}
        projectName={store.snapshot.projectName}
        onOpenDefaultProject={store.openDefaultProject}
        onOpenSettings={() => store.setSettingsOpen(true)}
        onShowHome={() => store.setHomeVisible(true)}
      />

      {store.homeVisible ? (
        <StartScreen
          bridgeStatus={store.bridgeStatus}
          busy={store.homeBusy}
          error={store.homeError}
          home={store.home}
          onCreateProject={store.createProject}
          onOpenDefaultProject={store.openDefaultProject}
          onOpenProject={store.openProject}
        />
      ) : (
        <div className="studio-workspace">
          <NavigatorPanel
            assets={store.snapshot.assets}
            entities={store.snapshot.scene.entities}
            selectedEntityId={store.selectedEntityId}
            onSelectEntity={store.selectEntity}
          />

        <div className="resize-handle resize-handle-vertical resize-handle-left" onPointerDown={(event) => startResize("left", event)} />

        <main className="stage-panel">
          <StageToolbar
            mode={store.mode}
            previewBusy={store.previewBusy}
            previewStatus={store.previewStatus}
            sceneDirty={store.sceneDirty}
            savingScene={store.savingScene}
            viewMode={store.viewMode}
            selectedEntity={selectedEntity}
            onModeChange={store.setMode}
            onPausePreview={store.pausePreview}
            onSaveScene={store.saveScene}
            onStartPreview={store.startPreview}
            onStopPreview={store.stopPreview}
            onViewModeChange={store.setViewMode}
          />

          <div className="stage-content">
            <section className="viewport-wrap">
              <SceneViewport
                entities={store.snapshot.scene.entities}
                scene={store.snapshot.scene}
                mode={store.mode}
                viewMode={store.viewMode}
                selectedEntityId={store.selectedEntityId}
                preferences={store.preferences}
                dragState={store.dragState}
                onAssetDrop={store.createEntityFromAsset}
                onDragStateChange={store.setDragState}
                onClearSelection={store.clearSelection}
                onSelectEntity={store.selectEntity}
                onUpdateTransform={(id, transform) => {
                  store.updateScene((scene) => ({
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
          assets={store.snapshot.assets}
          entity={selectedEntity}
          scene={store.snapshot.scene}
          scripts={store.snapshot.scripts}
          onAttachScript={store.attachScript}
          onComponentChange={(index, component) => {
            if (!selectedEntity) return;
            store.updateSelectedEntity({
              components: selectedEntity.components.map((current, currentIndex) =>
                currentIndex === index ? component : current,
              ),
            });
          }}
          onEntityChange={store.updateSelectedEntity}
          onSceneChange={(patch) => store.updateScene((scene) => ({ ...scene, ...patch }))}
          onTransformChange={store.updateSelectedTransform}
        />

        <div className="resize-handle resize-handle-horizontal" onPointerDown={(event) => startResize("bottom", event)} />

        <BottomDock
          activeTab={store.bottomTab}
          assets={store.snapshot.assets}
          console={store.snapshot.console}
          scripts={store.snapshot.scripts}
          onAssetDrop={store.createEntityFromAsset}
          onAssetPointerDown={beginAssetDrag}
          onOpenScript={store.openScriptEditor}
          onScriptSelect={store.setSelectedScriptPath}
          onTabChange={store.setBottomTab}
        />
        </div>
      )}

      {!store.homeVisible && store.assetDrag ? <AssetDragPreview asset={store.assetDrag.asset} x={store.assetDrag.x} y={store.assetDrag.y} /> : null}
      {!store.homeVisible && store.scriptEditorOpen ? (
        <ScriptEditorWindow
          activeScript={activeScript}
          scripts={store.snapshot.scripts}
          selectedEntity={selectedEntity}
          onAttachScript={store.attachScript}
          onClose={() => store.setScriptEditorOpen(false)}
          onSaveScript={store.saveScript}
          onScriptChange={store.updateScriptContent}
          onScriptSelect={store.setSelectedScriptPath}
        />
      ) : null}
      {store.settingsOpen ? (
        <SettingsWindow
          preferences={store.preferences}
          onClose={() => store.setSettingsOpen(false)}
          onPreferencesChange={store.setPreferences}
        />
      ) : null}
    </div>
  );
}
