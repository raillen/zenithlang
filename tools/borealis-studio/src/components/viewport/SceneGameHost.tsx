import { Activity, AlertCircle, Play, RadioTower } from "lucide-react";
import React from "react";
import { ICON_STROKE } from "../../constants";
import { SceneViewport } from "../SceneViewport";
import { StatusPill } from "../shared";
import type {
  ConsoleLine,
  DragState,
  ProjectAsset,
  PreviewStatus,
  SceneDocument,
  SceneEntity,
  StudioMode,
  StudioPreferences,
  Transform3D,
  ViewMode,
  ViewportTab,
} from "../../types";
import { previewStatusLabel, previewStatusTone } from "../../utils/preview";
import { ViewportTabs } from "./ViewportTabs";
import { ViewportToolbar } from "./ViewportToolbar";

export function SceneGameHost({
  activeTab,
  consoleLines,
  dragState,
  entities,
  gizmos,
  mode,
  preferences,
  previewStatus,
  scene,
  selectedEntityId,
  spaceMode,
  pivotMode,
  viewMode,
  onAssetDrop,
  onClearSelection,
  onDragStateChange,
  onSelectEntity,
  onSetActiveTab,
  onSetGizmos,
  onSetPivotMode,
  onSetSpaceMode,
  onSetViewMode,
  onUpdateTransform,
}: {
  activeTab: ViewportTab;
  consoleLines: ConsoleLine[];
  dragState: DragState | null;
  entities: SceneEntity[];
  gizmos: boolean;
  mode: StudioMode;
  preferences: StudioPreferences;
  previewStatus: PreviewStatus;
  scene: SceneDocument;
  selectedEntityId: string;
  spaceMode: "global" | "local";
  pivotMode: "pivot" | "center";
  viewMode: ViewMode;
  onAssetDrop: (asset: ProjectAsset, transform?: Partial<Transform3D>) => void;
  onClearSelection: () => void;
  onDragStateChange: (state: DragState | null) => void;
  onSelectEntity: (entity: SceneEntity) => void;
  onSetActiveTab: (tab: ViewportTab) => void;
  onSetGizmos: (enabled: boolean) => void;
  onSetPivotMode: (mode: "pivot" | "center") => void;
  onSetSpaceMode: (mode: "global" | "local") => void;
  onSetViewMode: (mode: ViewMode) => void;
  onUpdateTransform: (id: string, transform: Transform3D) => void;
}) {
  const previewLines = consoleLines.filter((line) => line.source === "preview").slice(-6).reverse();

  return (
    <section className="panel-shell scene-panel">
      <div className="scene-panel-top">
        <ViewportTabs activeTab={activeTab} onChange={onSetActiveTab} />
        <ViewportToolbar
          gizmos={gizmos}
          onSetGizmos={onSetGizmos}
          onSetPivotMode={onSetPivotMode}
          onSetSpaceMode={onSetSpaceMode}
          onSetViewMode={onSetViewMode}
          pivotMode={pivotMode}
          spaceMode={spaceMode}
          viewMode={viewMode}
        />
      </div>
      <div className="scene-panel-body">
        {activeTab === "scene" ? (
          <SceneViewport
            dragState={dragState}
            entities={entities}
            gizmos={gizmos}
            mode={mode}
            onAssetDrop={onAssetDrop}
            onClearSelection={onClearSelection}
            onDragStateChange={onDragStateChange}
            onSelectEntity={onSelectEntity}
            onUpdateTransform={onUpdateTransform}
            preferences={preferences}
            scene={scene}
            selectedEntityId={selectedEntityId}
            pivotMode={pivotMode}
            spaceMode={spaceMode}
            viewMode={viewMode}
          />
        ) : (
          <div className="game-tab-surface">
            <div className="game-tab-hero">
              <div>
                <span className="game-tab-kicker">Runtime Preview</span>
                <h2>Game view</h2>
                <p>
                  O runner atual ainda abre externamente. Esta aba mostra o estado do Play mode, eventos recentes e o
                  contexto da cena ativa para o fluxo do Borealis Studio ficar alinhado ao modelo de engine/editor.
                </p>
              </div>
              <div className="game-tab-statuses">
                <StatusPill tone={previewStatusTone(previewStatus)}>{previewStatusLabel(previewStatus)}</StatusPill>
                <StatusPill>{scene.name}</StatusPill>
                <StatusPill>{entities.length} entities</StatusPill>
              </div>
            </div>
            <div className="game-tab-preview-card">
              <div className="game-tab-preview-viewport">
                <Play size={44} strokeWidth={1.4} />
                <strong>Preview runtime</strong>
                <span>Use Play no header para iniciar ou atualizar o runner.</span>
              </div>
              <div className="game-tab-info-grid">
                <div className="game-tab-info-card">
                  <RadioTower size={16} strokeWidth={ICON_STROKE} />
                  <strong>Bridge</strong>
                  <span>Preview via JSONL + process bridge</span>
                </div>
                <div className="game-tab-info-card">
                  <Activity size={16} strokeWidth={ICON_STROKE} />
                  <strong>Status</strong>
                  <span>{previewStatusLabel(previewStatus)}</span>
                </div>
                <div className="game-tab-info-card">
                  <AlertCircle size={16} strokeWidth={ICON_STROKE} />
                  <strong>Events</strong>
                  <span>{previewLines.length} recent preview messages</span>
                </div>
              </div>
            </div>
            <div className="game-tab-log-list">
              {previewLines.length === 0 ? (
                <div className="empty-state inline">No preview events yet.</div>
              ) : (
                previewLines.map((line) => (
                  <div className={`game-tab-log ${line.level}`} key={line.id}>
                    <strong>{line.level.toUpperCase()}</strong>
                    <span>{line.message}</span>
                  </div>
                ))
              )}
            </div>
          </div>
        )}
      </div>
    </section>
  );
}
