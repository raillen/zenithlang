import React from "react";
import type { ViewportRendererProps } from "../types";
import { entityClass, entityGlyph } from "../utils/entity";
import { project2d } from "../utils/viewport";
import { ObjectGizmo, StatusPill } from "./shared";

export function Viewport2D({
  camera,
  dragState,
  entities,
  gizmos,
  mode,
  preferences,
  selectedEntityId,
  onDragStateChange,
  onEntityPointerDown,
  onEntityPointerMove,
  onResetCamera,
  onSelectEntity,
}: ViewportRendererProps & { gizmos: boolean }) {
  return (
    <>
      {preferences.showGrid ? <div className="viewport-grid" /> : null}
      <div className="viewport-axis axis-x">X</div>
      <div className="viewport-axis axis-y">Y</div>
      <div className="viewport-overlay-top">
        <StatusPill>Scene 2D</StatusPill>
        <StatusPill>{Math.round(camera.zoom * 100)}%</StatusPill>
        <StatusPill>{mode}</StatusPill>
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
              transform: `translate(calc(-50% + ${screenX}px), calc(-50% + ${screenY}px)) rotate(${entity.transform.rotationZ}deg) scale(${camera.zoom * entity.transform.scaleX}, ${camera.zoom * entity.transform.scaleY})`,
            }}
          >
            <span className="object-glyph">{entityGlyph(entity)}</span>
            <span className="object-label">{entity.name}</span>
            {selected && gizmos ? (
              <>
                <ObjectGizmo viewMode="2d" />
                <span className={`object-tool-badge mode-${mode}`}>{mode}</span>
              </>
            ) : null}
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
