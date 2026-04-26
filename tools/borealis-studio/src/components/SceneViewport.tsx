import React, { PointerEvent, useRef, useState } from "react";
import type {
  DragState,
  ProjectAsset,
  SceneDocument,
  SceneEntity,
  StudioMode,
  StudioPreferences,
  Transform3D,
  ViewMode,
  ViewportCamera,
  ViewportPanState,
} from "../types";
import { clamp } from "../utils/viewport";
import { Viewport2D } from "./Viewport2D";
import { Viewport3D } from "./Viewport3D";

export function SceneViewport({
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

  const viewportProps = {
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
