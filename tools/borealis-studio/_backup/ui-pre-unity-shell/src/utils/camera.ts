import { Vector3 } from "three";
import type { ViewOrientation, ViewProjection } from "../types";

export function cameraPositionForView(projection: ViewProjection, orientation: ViewOrientation) {
  if (projection === "isometric") return new Vector3(8, 7, 8);

  const distance = projection === "orthographic" ? 12 : 14;
  switch (orientation) {
    case "top":
      return new Vector3(0, distance, 0.001);
    case "bottom":
      return new Vector3(0, -distance, 0.001);
    case "left":
      return new Vector3(-distance, 0, 0);
    case "right":
      return new Vector3(distance, 0, 0);
    case "front":
      return new Vector3(0, 0, distance);
    case "back":
      return new Vector3(0, 0, -distance);
    case "free":
    default:
      return new Vector3(10, 8, 10);
  }
}

export function numpadOrientation(code: string, ctrlKey: boolean): ViewOrientation | null {
  if (code === "Numpad1") return ctrlKey ? "back" : "front";
  if (code === "Numpad3") return ctrlKey ? "left" : "right";
  if (code === "Numpad7") return ctrlKey ? "bottom" : "top";
  return null;
}

export function oppositeOrientation(orientation: ViewOrientation): ViewOrientation {
  if (orientation === "front") return "back";
  if (orientation === "back") return "front";
  if (orientation === "right") return "left";
  if (orientation === "left") return "right";
  if (orientation === "top") return "bottom";
  if (orientation === "bottom") return "top";
  return "back";
}
