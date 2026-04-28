import type { ViewOrientation } from "./types";

export const ICON_STROKE = 1.75;

export const VIEW_ORIENTATIONS: Array<{ id: ViewOrientation; label: string; shortcut: string }> = [
  { id: "top", label: "Top", shortcut: "T" },
  { id: "bottom", label: "Bottom", shortcut: "B" },
  { id: "left", label: "Left", shortcut: "L" },
  { id: "right", label: "Right", shortcut: "R" },
  { id: "front", label: "Front", shortcut: "F" },
  { id: "back", label: "Back", shortcut: "K" },
];
