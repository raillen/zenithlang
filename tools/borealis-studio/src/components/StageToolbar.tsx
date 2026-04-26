import { MousePointer2, Move3D, Pause, Play, RotateCcw, Save, Scale3D, Square } from "lucide-react";
import React from "react";
import { ICON_STROKE } from "../constants";
import type { PreviewStatus, SceneEntity, StudioMode, ViewMode } from "../types";
import { previewStatusLabel, previewStatusTone } from "../utils/preview";
import { CommandSearchButton } from "./CommandSearchButton";
import { Segmented, StatusPill, ToolButton } from "./shared";

export function StageToolbar({
  mode,
  previewBusy,
  previewStatus,
  sceneDirty,
  selectedEntity,
  savingScene,
  viewMode,
  onModeChange,
  onPausePreview,
  onSaveScene,
  onStartPreview,
  onStopPreview,
  onViewModeChange,
}: {
  mode: StudioMode;
  previewBusy: boolean;
  previewStatus: PreviewStatus;
  sceneDirty: boolean;
  selectedEntity: SceneEntity | null;
  savingScene: boolean;
  viewMode: ViewMode;
  onModeChange: (mode: StudioMode) => void;
  onPausePreview: () => void;
  onSaveScene: () => void;
  onStartPreview: () => void;
  onStopPreview: () => void;
  onViewModeChange: (mode: ViewMode) => void;
}) {
  const canPausePreview = ["playing", "ready", "paused"].includes(previewStatus);
  const canStopPreview = !["idle", "stopped", "unavailable"].includes(previewStatus);

  return (
    <div className="stage-toolbar">
      <Segmented tabs={["2d", "3d"]} value={viewMode} onChange={onViewModeChange} />
      <div className="viewport-control-center">
        <CommandSearchButton />
        <div className="tool-strip" aria-label="Transform tools">
          <ToolButton active={mode === "select"} label="Select" onClick={() => onModeChange("select")}>
            <MousePointer2 size={15} strokeWidth={ICON_STROKE} />
          </ToolButton>
          <ToolButton active={mode === "move"} label="Move" onClick={() => onModeChange("move")}>
            <Move3D size={15} strokeWidth={ICON_STROKE} />
          </ToolButton>
          <ToolButton active={mode === "rotate"} label="Rotate" onClick={() => onModeChange("rotate")}>
            <RotateCcw size={15} strokeWidth={ICON_STROKE} />
          </ToolButton>
          <ToolButton active={mode === "scale"} label="Scale" onClick={() => onModeChange("scale")}>
            <Scale3D size={15} strokeWidth={ICON_STROKE} />
          </ToolButton>
        </div>
        <div className="transport-controls" aria-label="Preview controls">
          <ToolButton
            active={["starting", "loading", "ready", "playing"].includes(previewStatus)}
            disabled={previewBusy}
            label="Play Preview"
            onClick={onStartPreview}
          >
            <Play size={15} strokeWidth={ICON_STROKE} />
          </ToolButton>
          <ToolButton
            active={previewStatus === "paused"}
            disabled={previewBusy || !canPausePreview}
            label="Pause Preview"
            onClick={onPausePreview}
          >
            <Pause size={15} strokeWidth={ICON_STROKE} />
          </ToolButton>
          <ToolButton
            active={previewStatus === "stopped"}
            disabled={previewBusy || !canStopPreview}
            label="Stop Preview"
            onClick={onStopPreview}
          >
            <Square size={14} strokeWidth={ICON_STROKE} />
          </ToolButton>
        </div>
      </div>
      <div className="stage-status">
        <button className="panel-action stage-save-action" disabled={savingScene} onClick={onSaveScene} title="Save scene">
          <Save size={14} strokeWidth={ICON_STROKE} />
          {savingScene ? "Saving" : "Save Scene"}
        </button>
        <StatusPill>{mode}</StatusPill>
        <StatusPill>{selectedEntity ? selectedEntity.name : "No selection"}</StatusPill>
        <StatusPill tone={sceneDirty ? "warn" : "good"}>{sceneDirty ? "Unsaved scene" : "Scene saved"}</StatusPill>
        <StatusPill tone={previewStatusTone(previewStatus)}>{previewStatusLabel(previewStatus)}</StatusPill>
      </div>
    </div>
  );
}
