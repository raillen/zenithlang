import { Activity, AlertTriangle, HardDriveDownload, Layers3, Save } from "lucide-react";
import React from "react";
import { ICON_STROKE } from "../../constants";
import type { BridgeStatus, PreviewStatus, SelectionTarget } from "../../types";
import { previewStatusLabel, previewStatusTone } from "../../utils/preview";
import { StatusPill } from "../shared";

export function StatusBar({
  bridgeStatus,
  errors,
  previewStatus,
  projectName,
  savingScene,
  sceneDirty,
  selectionTarget,
  selectedEntityName,
  warnings,
}: {
  bridgeStatus: BridgeStatus;
  errors: number;
  previewStatus: PreviewStatus;
  projectName: string;
  savingScene: boolean;
  sceneDirty: boolean;
  selectionTarget: SelectionTarget;
  selectedEntityName?: string;
  warnings: number;
}) {
  const selectionLabel =
    selectionTarget.kind === "scene-root"
      ? "Scene root"
      : selectionTarget.kind === "entity"
        ? selectedEntityName || selectionTarget.entityId || "Entity"
        : "No selection";

  return (
    <footer className="status-bar-unity">
      <div className="status-bar-group">
        <span className="status-bar-item">
          <AlertTriangle size={13} strokeWidth={ICON_STROKE} /> {warnings} warnings
        </span>
        <span className="status-bar-item error">
          <AlertTriangle size={13} strokeWidth={ICON_STROKE} /> {errors} errors
        </span>
        <StatusPill tone={previewStatusTone(previewStatus)}>{previewStatusLabel(previewStatus)}</StatusPill>
      </div>
      <div className="status-bar-group">
        <span className="status-bar-item">
          <HardDriveDownload size={13} strokeWidth={ICON_STROKE} /> {projectName}
        </span>
        <span className="status-bar-item">
          <Layers3 size={13} strokeWidth={ICON_STROKE} /> {selectionLabel}
        </span>
        <span className="status-bar-item">
          <Activity size={13} strokeWidth={ICON_STROKE} /> {bridgeStatus === "tauri" ? "Desktop runtime" : "Browser fallback"}
        </span>
        <span className={`status-bar-item ${sceneDirty ? "warn" : "good"}`}>
          <Save size={13} strokeWidth={ICON_STROKE} /> {savingScene ? "Saving scene..." : sceneDirty ? "Unsaved scene" : "Scene saved"}
        </span>
      </div>
    </footer>
  );
}
