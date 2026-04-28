import { ChevronDown, Search } from "lucide-react";
import React from "react";
import { ICON_STROKE } from "../../constants";
import { ViewMode, ViewportPivotMode, ViewportSpaceMode } from "../../types";

export function ViewportToolbar({
  gizmos,
  pivotMode,
  spaceMode,
  viewMode,
  onSetGizmos,
  onSetPivotMode,
  onSetSpaceMode,
  onSetViewMode,
}: {
  gizmos: boolean;
  pivotMode: ViewportPivotMode;
  spaceMode: ViewportSpaceMode;
  viewMode: ViewMode;
  onSetGizmos: (enabled: boolean) => void;
  onSetPivotMode: (mode: ViewportPivotMode) => void;
  onSetSpaceMode: (mode: ViewportSpaceMode) => void;
  onSetViewMode: (mode: ViewMode) => void;
}) {
  return (
    <div className="viewport-toolbar-shell">
      <ToolbarSelect
        label="Pivot"
        value={pivotMode}
        onChange={(value) => onSetPivotMode(value as ViewportPivotMode)}
        options={[
          ["pivot", "Pivot"],
          ["center", "Center"],
        ]}
      />
      <ToolbarSelect
        label="Space"
        value={spaceMode}
        onChange={(value) => onSetSpaceMode(value as ViewportSpaceMode)}
        options={[
          ["global", "Global"],
          ["local", "Local"],
        ]}
      />
      <ToolbarSelect
        label="View"
        value={viewMode}
        onChange={(value) => onSetViewMode(value as ViewMode)}
        options={[
          ["3d", "3D"],
          ["2d", "2D"],
        ]}
      />
      <ToolbarSelect
        label="Overlays"
        value={gizmos ? "gizmos-on" : "gizmos-off"}
        onChange={(value) => onSetGizmos(value === "gizmos-on")}
        options={[
          ["gizmos-on", "Gizmos on"],
          ["gizmos-off", "Gizmos off"],
        ]}
      />
      <label className="viewport-toolbar-search">
        <Search size={14} />
        <input placeholder="Search in scene" readOnly value="" />
      </label>
    </div>
  );
}

function ToolbarSelect({
  label,
  onChange,
  options,
  value,
}: {
  label: string;
  onChange: (value: string) => void;
  options: Array<[string, string]>;
  value: string;
}) {
  return (
    <label className="viewport-toolbar-select">
      <span>{label}</span>
      <select onChange={(event) => onChange(event.target.value)} value={value}>
        {options.map(([optionValue, optionLabel]) => (
          <option key={optionValue} value={optionValue}>
            {optionLabel}
          </option>
        ))}
      </select>
      <ChevronDown aria-hidden size={13} strokeWidth={ICON_STROKE} />
    </label>
  );
}
