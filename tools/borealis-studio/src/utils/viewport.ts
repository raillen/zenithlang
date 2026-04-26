import type { SceneEntity, SnapSettings, Transform3D } from "../types";
import { entityBounds } from "./entity";

export function clamp(value: number, min: number, max: number): number {
  return Math.min(Math.max(value, min), max);
}

export function snapValue(value: number, step: number): number {
  if (step <= 0) return value;
  return Math.round(value / step) * step;
}

export function gridMetersToSceneUnits(gridSize: number): number {
  return gridSize / 0.04;
}

export function project3d(transform: Transform3D): { x: number; y: number } {
  return {
    x: transform.x * 0.55 - transform.y * 0.34,
    y: transform.x * 0.1 + transform.y * 0.22 - transform.z * 0.7,
  };
}

export function project2d(transform: Transform3D): { x: number; y: number } {
  return {
    x: transform.x * 0.72,
    y: transform.y * 0.5,
  };
}

export function snapTransform(
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

export function snapTransformToObjects(
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
