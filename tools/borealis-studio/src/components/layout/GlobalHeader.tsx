import {
  Boxes,
  ChevronDown,
  Globe,
  Layers3,
  LayoutGrid,
  Pause,
  Play,
  Settings2,
  SkipForward,
} from "lucide-react";
import React from "react";
import { ICON_STROKE } from "../../constants";
import type { BridgeStatus, PreviewStatus, ProjectAsset, SceneEntity, ScriptDocument } from "../../types";
import { previewStatusLabel, previewStatusTone } from "../../utils/preview";
import { CommandSearchButton } from "../CommandSearchButton";
import { StatusPill } from "../shared";

export function GlobalHeader({
  bridgeStatus,
  previewBusy,
  previewStatus,
  projectName,
  assets,
  entities,
  scripts,
  onOpenDefaultProject,
  onOpenSettings,
  onResetLayout,
  onShowHome,
  onStartPreview,
  onPausePreview,
  onSelectEntity,
  onOpenScript,
}: {
  bridgeStatus: BridgeStatus;
  previewBusy: boolean;
  previewStatus: PreviewStatus;
  projectName: string;
  assets: ProjectAsset[];
  entities: SceneEntity[];
  scripts: ScriptDocument[];
  onOpenDefaultProject: () => void;
  onOpenSettings: () => void;
  onResetLayout: () => void;
  onShowHome: () => void;
  onStartPreview: () => void;
  onPausePreview: () => void;
  onSelectEntity: (id: string) => void;
  onOpenScript: (path: string) => void;
}) {
  const playActive = ["starting", "loading", "ready", "playing"].includes(previewStatus);
  const pauseActive = previewStatus === "paused";
  const canPause = ["ready", "playing", "paused"].includes(previewStatus);

  return (
    <header className="global-header" data-tauri-drag-region>
      <div className="header-left">
        <button className="header-pill header-brand" onClick={onShowHome} type="button">
          <Boxes size={16} strokeWidth={ICON_STROKE} />
          <strong>Borealis Studio</strong>
          <ChevronDown size={14} strokeWidth={ICON_STROKE} />
        </button>
        <button className="header-pill" onClick={onOpenDefaultProject} type="button">
          <span className="header-pill-label">Project</span>
          <strong>{projectName}</strong>
          <ChevronDown size={14} strokeWidth={ICON_STROKE} />
        </button>
        <button className="header-pill" type="button">
          <span className="header-pill-label">Branch</span>
          <strong>main</strong>
          <ChevronDown size={14} strokeWidth={ICON_STROKE} />
        </button>
        <div className="header-actions-inline">
          <CommandSearchButton
            assets={assets}
            entities={entities}
            scripts={scripts}
            onOpenScript={onOpenScript}
            onSelectEntity={onSelectEntity}
          />
          <button className="top-icon-button" title="Network services" type="button">
            <Globe size={15} strokeWidth={ICON_STROKE} />
          </button>
        </div>
      </div>

      <div className="header-transport">
        <button
          aria-pressed={playActive}
          className={`transport-button ${playActive ? "active" : ""}`}
          disabled={previewBusy}
          onClick={onStartPreview}
          title="Play"
          type="button"
        >
          <Play size={15} strokeWidth={ICON_STROKE} />
        </button>
        <button
          aria-pressed={pauseActive}
          className={`transport-button ${pauseActive ? "active" : ""}`}
          disabled={previewBusy || !canPause}
          onClick={onPausePreview}
          title="Pause"
          type="button"
        >
          <Pause size={15} strokeWidth={ICON_STROKE} />
        </button>
        <button className="transport-button" disabled title="Step" type="button">
          <SkipForward size={15} strokeWidth={ICON_STROKE} />
        </button>
      </div>

      <div className="header-right">
        <button className="header-pill" type="button">
          <Layers3 size={14} strokeWidth={ICON_STROKE} />
          <span>Layers</span>
          <ChevronDown size={14} strokeWidth={ICON_STROKE} />
        </button>
        <button className="header-pill" onClick={onResetLayout} type="button">
          <LayoutGrid size={14} strokeWidth={ICON_STROKE} />
          <span>Layout</span>
          <ChevronDown size={14} strokeWidth={ICON_STROKE} />
        </button>
        <button className="top-icon-button" onClick={onOpenSettings} title="Studio settings" type="button">
          <Settings2 size={15} strokeWidth={ICON_STROKE} />
        </button>
        <StatusPill tone={previewStatusTone(previewStatus)}>{previewStatusLabel(previewStatus)}</StatusPill>
        <StatusPill tone={bridgeStatus === "tauri" ? "good" : "warn"}>{bridgeStatus === "tauri" ? "Desktop" : "Browser"}</StatusPill>
      </div>
    </header>
  );
}
