import { Hand, Maximize, MousePointer2, Move3D, RotateCcw, Scale3D } from "lucide-react";
import React from "react";
import { ICON_STROKE } from "../../constants";
import type { StudioMode } from "../../types";

const TOOLS: Array<{ id: StudioMode; label: string; icon: React.ReactNode }> = [
  { id: "select", label: "Select", icon: <MousePointer2 size={16} strokeWidth={ICON_STROKE} /> },
  { id: "move", label: "Move", icon: <Move3D size={16} strokeWidth={ICON_STROKE} /> },
  { id: "rotate", label: "Rotate", icon: <RotateCcw size={16} strokeWidth={ICON_STROKE} /> },
  { id: "scale", label: "Scale", icon: <Scale3D size={16} strokeWidth={ICON_STROKE} /> },
  { id: "rect", label: "Rect Tool", icon: <Maximize size={16} strokeWidth={ICON_STROKE} /> },
  { id: "hand", label: "Hand Tool", icon: <Hand size={16} strokeWidth={ICON_STROKE} /> },
];

export function ToolRail({ mode, onModeChange }: { mode: StudioMode; onModeChange: (mode: StudioMode) => void }) {
  return (
    <aside className="tool-rail" aria-label="Scene tools">
      {TOOLS.map((tool) => (
        <button
          aria-pressed={mode === tool.id}
          className={`tool-rail-button ${mode === tool.id ? "active" : ""}`}
          key={tool.id}
          onClick={() => onModeChange(tool.id)}
          title={tool.label}
          type="button"
        >
          {tool.icon}
        </button>
      ))}
    </aside>
  );
}
