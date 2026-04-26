import {
  Activity,
  Box,
  Braces,
  Cuboid,
  LayoutTemplate,
  Maximize2,
  WandSparkles,
} from "lucide-react";
import React from "react";
import type { SceneEntity } from "../types";
import { ICON_STROKE } from "../constants";

export function entityScript(entity: SceneEntity): string | undefined {
  return entity.components.find((component) => component.kind === "script")?.script;
}

export function entityClass(entity: SceneEntity): string {
  if (entity.components.some((component) => component.kind === "camera3d" || component.kind === "camera2d")) return "camera";
  if (entity.components.some((component) => component.kind === "cube3d")) return "cube";
  if (entity.components.some((component) => component.kind === "model3d")) return "model";
  if (entity.components.some((component) => component.kind === "audio" || component.kind === "audio3d")) return "audio";
  if (entity.components.some((component) => component.kind === "ui" || component.kind === "ui3d" || component.kind === "hud")) return "ui";
  return "default";
}

export function entityGlyph(entity: SceneEntity) {
  if (entity.components.some((component) => component.kind === "camera3d" || component.kind === "camera2d")) {
    return React.createElement(Maximize2, { size: 16, strokeWidth: ICON_STROKE });
  }
  if (entity.components.some((component) => component.kind === "cube3d")) {
    return React.createElement(Cuboid, { size: 17, strokeWidth: ICON_STROKE });
  }
  if (entity.components.some((component) => component.kind === "audio" || component.kind === "audio3d")) {
    return React.createElement(Activity, { size: 17, strokeWidth: ICON_STROKE });
  }
  if (entity.components.some((component) => component.kind === "ui" || component.kind === "ui3d" || component.kind === "hud")) {
    return React.createElement(LayoutTemplate, { size: 17, strokeWidth: ICON_STROKE });
  }
  return React.createElement(Box, { size: 17, strokeWidth: ICON_STROKE });
}

export function entityIcon(entity: SceneEntity) {
  if (entity.components.some((component) => component.kind === "camera3d" || component.kind === "camera2d")) {
    return React.createElement(Maximize2, { size: 12, strokeWidth: ICON_STROKE });
  }
  if (entity.components.some((component) => component.kind === "script")) {
    return React.createElement(Braces, { size: 12, strokeWidth: ICON_STROKE });
  }
  if (entity.components.some((component) => component.kind === "audio" || component.kind === "audio3d")) {
    return React.createElement(Activity, { size: 12, strokeWidth: ICON_STROKE });
  }
  if (entity.components.some((component) => component.kind === "ui" || component.kind === "ui3d" || component.kind === "hud")) {
    return React.createElement(LayoutTemplate, { size: 12, strokeWidth: ICON_STROKE });
  }
  if (entity.components.some((component) => component.kind === "render3d" || component.kind === "postfx")) {
    return React.createElement(WandSparkles, { size: 12, strokeWidth: ICON_STROKE });
  }
  return React.createElement(Box, { size: 12, strokeWidth: ICON_STROKE });
}

export function entityBounds(entity: SceneEntity) {
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
