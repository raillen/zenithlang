import type { ShortcutTemplate, StudioMode } from "../types";

export function keybindsForTemplate(template: ShortcutTemplate) {
  if (template === "3dsmax") {
    return [
      { action: "Orbit", keys: "Alt + MMB" },
      { action: "Pan", keys: "MMB" },
      { action: "Zoom", keys: "Wheel" },
      { action: "Move", keys: "W" },
      { action: "Rotate", keys: "E" },
      { action: "Scale", keys: "R" },
    ];
  }

  if (template === "maya") {
    return [
      { action: "Orbit", keys: "Alt + LMB" },
      { action: "Pan", keys: "Alt + MMB" },
      { action: "Zoom", keys: "Alt + RMB" },
      { action: "Move", keys: "W" },
      { action: "Rotate", keys: "E" },
      { action: "Scale", keys: "R" },
    ];
  }

  return [
    { action: "Orbit", keys: "MMB" },
    { action: "Pan", keys: "Shift + MMB" },
    { action: "Zoom", keys: "Wheel" },
    { action: "Front", keys: "Numpad 1" },
    { action: "Right", keys: "Numpad 3" },
    { action: "Top", keys: "Numpad 7" },
    { action: "Pie view", keys: "Alt + Z" },
  ];
}

export function modeForShortcut(template: ShortcutTemplate, key: string): StudioMode | null {
  if (template === "blender") {
    if (key === "q") return "select";
    if (key === "g") return "move";
    if (key === "r") return "rotate";
    if (key === "s") return "scale";
  }

  if (key === "q") return "select";
  if (key === "w") return "move";
  if (key === "e") return "rotate";
  if (key === "r") return "scale";
  return null;
}
