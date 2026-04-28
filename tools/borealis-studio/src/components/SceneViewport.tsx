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
  ViewportPivotMode,
  ViewportRectSelectionState,
  ViewportSpaceMode,
} from "../types";
import { clamp, snapTransform } from "../utils/viewport";
import { Viewport2D } from "./Viewport2D";
import { Viewport3D } from "./Viewport3D";

export function SceneViewport({
  entities,
  gizmos,
  scene,
  mode,
  pivotMode,
  spaceMode,
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
  gizmos: boolean;
  scene: SceneDocument;
  mode: StudioMode;
  pivotMode: ViewportPivotMode;
  spaceMode: ViewportSpaceMode;
  viewMode: ViewMode;
  selectedEntityId: string;
  preferences: StudioPreferences;
  dragState: DragState | null;
  onAssetDrop: (asset: ProjectAsset, transform?: Partial<Transform3D>) => void;
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
  const [rectSelection, setRectSelection] = useState<ViewportRectSelectionState | null>(null);
  const activeCamera = viewportCameras[viewMode];

  function handleEntityPointerDown(entity: SceneEntity, event: PointerEvent<HTMLButtonElement>) {
    if (viewMode === "2d" && mode === "hand" && event.button === 0) {
      event.preventDefault();
      event.stopPropagation();
      beginViewportPan(event);
      return;
    }

    if (viewMode === "2d" && mode === "rect" && event.button === 0) {
      event.preventDefault();
      event.stopPropagation();
      beginRectSelection(event);
      return;
    }

    event.preventDefault();
    event.stopPropagation();
    event.currentTarget.setPointerCapture(event.pointerId);
    onSelectEntity(entity);
    if (viewMode !== "2d" && mode !== "move") return;
    if (viewMode === "2d" && !["move", "rotate", "scale"].includes(mode)) return;
    onDragStateChange({
      id: entity.id,
      kind: mode === "rotate" ? "rotate" : mode === "scale" ? "scale" : "move",
      startX: event.clientX,
      startY: event.clientY,
      originX: entity.transform.x,
      originY: entity.transform.y,
      originRotationZ: entity.transform.rotationZ,
      originScaleX: entity.transform.scaleX,
      originScaleY: entity.transform.scaleY,
    });
  }

  function handleEntityPointerMove(entity: SceneEntity, event: PointerEvent<HTMLButtonElement>) {
    if (!dragState || dragState.id !== entity.id) return;
    const deltaX = event.clientX - dragState.startX;
    const deltaY = event.clientY - dragState.startY;

    if (viewMode === "2d") {
      const kind = dragState.kind ?? mode;
      if (kind === "rotate") {
        onUpdateTransform(entity.id, {
          ...entity.transform,
          rotationZ: Math.round((dragState.originRotationZ ?? entity.transform.rotationZ) + deltaX * 0.45),
        });
        return;
      }

      if (kind === "scale") {
        const scaleDelta = (deltaX - deltaY) / 160;
        const nextScaleX = clamp((dragState.originScaleX ?? entity.transform.scaleX) + scaleDelta, 0.1, 8);
        const nextScaleY = clamp((dragState.originScaleY ?? entity.transform.scaleY) + scaleDelta, 0.1, 8);
        onUpdateTransform(entity.id, {
          ...entity.transform,
          scaleX: Number(nextScaleX.toFixed(2)),
          scaleY: Number(nextScaleY.toFixed(2)),
        });
        return;
      }

      const next = snap2DTransform(
        {
          ...entity.transform,
          x: Math.round(dragState.originX + deltaX / (0.72 * activeCamera.zoom)),
          y: Math.round(dragState.originY + deltaY / (0.5 * activeCamera.zoom)),
        },
        entity,
      );
      onUpdateTransform(entity.id, next);
      return;
    }

    if (mode === "move") {
      onUpdateTransform(entity.id, {
        ...entity.transform,
        x: Math.round(dragState.originX + deltaX * 0.75 / activeCamera.zoom),
        y: Math.round(dragState.originY + deltaY * 0.75 / activeCamera.zoom),
      });
    }
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
    if (viewMode === "2d" && mode === "rect" && event.button === 0) {
      event.preventDefault();
      beginRectSelection(event);
      return;
    }

    if (event.button !== 1 && !event.altKey && !(viewMode === "2d" && mode === "hand" && event.button === 0)) return;
    event.preventDefault();
    beginViewportPan(event);
  }

  function beginViewportPan(event: PointerEvent<HTMLElement>) {
    viewportRef.current?.setPointerCapture(event.pointerId);
    setViewportPanState({
      pointerId: event.pointerId,
      startX: event.clientX,
      startY: event.clientY,
      originPanX: activeCamera.panX,
      originPanY: activeCamera.panY,
    });
  }

  function beginRectSelection(event: PointerEvent<HTMLElement>) {
    viewportRef.current?.setPointerCapture(event.pointerId);
    setRectSelection({
      startX: event.clientX,
      startY: event.clientY,
      currentX: event.clientX,
      currentY: event.clientY,
    });
  }

  function handleViewportPointerMove(event: PointerEvent<HTMLDivElement>) {
    if (rectSelection) {
      setRectSelection((current) => current ? { ...current, currentX: event.clientX, currentY: event.clientY } : null);
      return;
    }

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

  function endViewportPointer(event: PointerEvent<HTMLDivElement>) {
    if (rectSelection) {
      const selected = entityInsideRect(rectSelection);
      if (selected) {
        onSelectEntity(selected);
      } else {
        onClearSelection();
      }
      setRectSelection(null);
    }
    setViewportPanState(null);
    if (event.currentTarget.hasPointerCapture(event.pointerId)) {
      event.currentTarget.releasePointerCapture(event.pointerId);
    }
  }

  function handleDrop(event: React.DragEvent<HTMLDivElement>) {
    event.preventDefault();
    const raw = event.dataTransfer.getData("application/x-borealis-asset");
    if (!raw) return;
    const placement = viewMode === "2d" ? screenToScene2D(event.clientX, event.clientY) : undefined;
    onAssetDrop(JSON.parse(raw) as ProjectAsset, placement);
  }

  function screenToScene2D(clientX: number, clientY: number): Partial<Transform3D> {
    const rect = viewportRef.current?.getBoundingClientRect();
    if (!rect) return {};
    const localX = (clientX - rect.left - rect.width / 2) / activeCamera.zoom;
    const localY = (clientY - rect.top - rect.height / 2) / activeCamera.zoom;
    return {
      x: Math.round((localX - activeCamera.panX) / 0.72),
      y: Math.round((localY - activeCamera.panY) / 0.5),
      z: 0,
    };
  }

  function snap2DTransform(transform: Transform3D, entity: SceneEntity): Transform3D {
    const snapToGrid = preferences.snapMode === "grid" || preferences.snapMode === "grid-object";
    const snapToObject = preferences.snapMode === "object" || preferences.snapMode === "grid-object";
    return snapTransform(transform, entity, entities, { gridSize: 1, snapToGrid, snapToObject });
  }

  function entityInsideRect(selection: ViewportRectSelectionState): SceneEntity | null {
    const minX = Math.min(selection.startX, selection.currentX);
    const maxX = Math.max(selection.startX, selection.currentX);
    const minY = Math.min(selection.startY, selection.currentY);
    const maxY = Math.max(selection.startY, selection.currentY);
    const viewportRect = viewportRef.current?.getBoundingClientRect();
    if (!viewportRect) return null;

    return [...entities].reverse().find((entity) => {
      const screenX = viewportRect.left + viewportRect.width / 2 + (entity.transform.x * 0.72 + activeCamera.panX) * activeCamera.zoom;
      const screenY = viewportRect.top + viewportRect.height / 2 + (entity.transform.y * 0.5 + activeCamera.panY) * activeCamera.zoom;
      return screenX >= minX && screenX <= maxX && screenY >= minY && screenY <= maxY;
    }) ?? null;
  }

  const viewportProps = {
    camera: activeCamera,
    dragState,
    entities,
    gizmos,
    mode,
    pivotMode,
    preferences,
    selectedEntityId,
    spaceMode,
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
          onPointerUp: endViewportPointer,
          onPointerCancel: endViewportPointer,
          onWheel: handleViewportWheel,
        }
      : {};

  return (
    <div
      className={`scene-viewport viewport-${viewMode} viewport-tool-${mode}`}
      onDragOver={(event) => event.preventDefault()}
      onDrop={handleDrop}
      ref={viewportRef}
      {...viewport2DHandlers}
      style={
        {
          "--viewport-pan-x": `${activeCamera.panX * activeCamera.zoom}px`,
          "--viewport-pan-y": `${activeCamera.panY * activeCamera.zoom}px`,
          "--viewport-zoom": activeCamera.zoom,
          "--viewport-grid-line": gridColor(preferences.gridColor, preferences.gridOpacity),
          "--viewport-grid-section": gridColor(preferences.gridColor, Math.min(1, preferences.gridOpacity * 1.55)),
        } as React.CSSProperties
      }
    >
      {viewMode === "3d" ? <Viewport3D {...viewportProps} scene={scene} /> : <Viewport2D {...viewportProps} />}
      {viewMode === "2d" && rectSelection ? (
        <div
          className="viewport-selection-rect"
          style={{
            left: `${Math.min(rectSelection.startX, rectSelection.currentX) - (viewportRef.current?.getBoundingClientRect().left ?? 0)}px`,
            top: `${Math.min(rectSelection.startY, rectSelection.currentY) - (viewportRef.current?.getBoundingClientRect().top ?? 0)}px`,
            width: `${Math.abs(rectSelection.currentX - rectSelection.startX)}px`,
            height: `${Math.abs(rectSelection.currentY - rectSelection.startY)}px`,
          }}
        />
      ) : null}
    </div>
  );
}

function gridColor(hex: string, opacity: number): string {
  const normalized = hex.replace("#", "").trim();
  const safe = normalized.length === 3
    ? normalized.split("").map((char) => `${char}${char}`).join("")
    : normalized.padEnd(6, "0").slice(0, 6);
  const value = Number.parseInt(safe, 16);
  const red = (value >> 16) & 255;
  const green = (value >> 8) & 255;
  const blue = value & 255;
  return `rgba(${red}, ${green}, ${blue}, ${Math.min(1, Math.max(0, opacity))})`;
}
