import { Archive, BadgeAlert, Check, Circle, FileCode2, Terminal, ZoomIn } from "lucide-react";
import React, { PointerEvent, useEffect, useRef, useState } from "react";
import { ICON_STROKE } from "../constants";
import type { BottomTab, ConsoleLine, ProjectAsset, ScriptDocument } from "../types";
import { assetKindIcon } from "../utils/assets";
import { clamp } from "../utils/viewport";
import { DockTab } from "./shared";

export function BottomDock({
  activeTab,
  assets,
  console,
  scripts,
  onAssetDrop,
  onAssetPointerDown,
  onOpenScript,
  onScriptSelect,
  onTabChange,
}: {
  activeTab: BottomTab;
  assets: ProjectAsset[];
  console: ConsoleLine[];
  scripts: ScriptDocument[];
  onAssetDrop: (asset: ProjectAsset) => void;
  onAssetPointerDown: (asset: ProjectAsset, event: PointerEvent<HTMLElement>) => void;
  onOpenScript: (path: string) => void;
  onScriptSelect: (path: string) => void;
  onTabChange: (tab: BottomTab) => void;
}) {
  const [assetTileSize, setAssetTileSize] = useState(112);
  const assetPanelRef = useRef<HTMLDivElement | null>(null);
  const assetIconSize = clamp(Math.round(assetTileSize * 0.22), 18, 38);

  function changeAssetTileSize(delta: number) {
    setAssetTileSize((current) => clamp(current + delta, 88, 168));
  }

  useEffect(() => {
    const panel = assetPanelRef.current;
    if (!panel) return;

    function handleAssetWheel(event: WheelEvent) {
      if (!event.altKey) return;
      event.preventDefault();
      changeAssetTileSize(event.deltaY < 0 ? 8 : -8);
    }

    panel.addEventListener("wheel", handleAssetWheel, { passive: false });
    return () => panel.removeEventListener("wheel", handleAssetWheel);
  }, []);

  return (
    <section className="bottom-dock">
      <div className="bottom-tabs">
        <DockTab active={activeTab === "assets"} icon={<Archive size={14} strokeWidth={ICON_STROKE} />} onClick={() => onTabChange("assets")}>
          Assets
        </DockTab>
        <DockTab active={activeTab === "console"} icon={<Terminal size={14} strokeWidth={ICON_STROKE} />} onClick={() => onTabChange("console")}>
          Console
        </DockTab>
        <DockTab active={activeTab === "problems"} icon={<BadgeAlert size={14} strokeWidth={ICON_STROKE} />} onClick={() => onTabChange("problems")}>
          Problems
        </DockTab>
      </div>
      {activeTab === "assets" ? (
        <div
          className="asset-panel"
          ref={assetPanelRef}
          style={
            {
              "--asset-tile-size": `${assetTileSize}px`,
              "--asset-icon-size": `${assetIconSize}px`,
            } as React.CSSProperties
          }
        >
          <div className="asset-panel-toolbar">
            <ZoomIn size={13} strokeWidth={ICON_STROKE} />
            <input
              aria-label="Asset icon size"
              min={88}
              max={168}
              type="range"
              value={assetTileSize}
              onChange={(event) => setAssetTileSize(Number(event.target.value))}
            />
          </div>
          <div className="asset-grid">
            {assets.map((asset) => (
              <AssetTile
                asset={asset}
                iconSize={assetIconSize}
                key={asset.id}
                onDrop={onAssetDrop}
                onOpenScript={onOpenScript}
                onPointerDown={onAssetPointerDown}
                onScriptSelect={onScriptSelect}
              />
            ))}
          </div>
        </div>
      ) : null}
      {activeTab === "console" ? <ConsoleView lines={console} /> : null}
      {activeTab === "problems" ? <ProblemsView scripts={scripts} /> : null}
    </section>
  );
}

function AssetTile({
  asset,
  iconSize,
  onDrop,
  onOpenScript,
  onPointerDown,
  onScriptSelect,
}: {
  asset: ProjectAsset;
  iconSize: number;
  onDrop: (asset: ProjectAsset) => void;
  onOpenScript: (path: string) => void;
  onPointerDown: (asset: ProjectAsset, event: PointerEvent<HTMLElement>) => void;
  onScriptSelect: (path: string) => void;
}) {
  return (
    <button
      className="asset-tile"
      draggable
      onClick={() => {
        if (asset.kind === "script") onScriptSelect(asset.path);
      }}
      onDoubleClick={() => {
        if (asset.kind === "script") onOpenScript(asset.path);
        if (asset.kind === "model" || asset.kind === "texture") onDrop(asset);
      }}
      onPointerDown={(event) => onPointerDown(asset, event)}
      onDragStart={(event) => {
        event.dataTransfer.setData("application/x-borealis-asset", JSON.stringify(asset));
      }}
    >
      <span>{assetKindIcon(asset.kind, iconSize)}</span>
      <strong>{asset.name}</strong>
      <small>{asset.path}</small>
    </button>
  );
}

export function AssetDragPreview({ asset, x, y }: { asset: ProjectAsset; x: number; y: number }) {
  return (
    <div
      className="asset-drag-preview"
      style={{
        left: x + 14,
        top: y + 14,
      }}
    >
      {assetKindIcon(asset.kind, 16)}
      <span>{asset.name}</span>
    </div>
  );
}

function ConsoleView({ lines }: { lines: ConsoleLine[] }) {
  return (
    <div className="console-view">
      {lines.map((line) => (
        <div className={`console-line ${line.level}`} key={line.id}>
          <span>{line.source}</span>
          <p>{line.message}</p>
        </div>
      ))}
    </div>
  );
}

function ProblemsView({ scripts }: { scripts: ScriptDocument[] }) {
  const dirtyScripts = scripts.filter((script) => script.dirty);
  return (
    <div className="problems-view">
      {dirtyScripts.length === 0 ? (
        <div className="empty-state inline">
          <Check size={16} strokeWidth={ICON_STROKE} />
          No script problems tracked.
        </div>
      ) : (
        dirtyScripts.map((script) => (
          <div className="problem-row" key={script.path}>
            <Circle size={8} fill="currentColor" strokeWidth={0} />
            <span>{script.name} has unsaved edits.</span>
          </div>
        ))
      )}
    </div>
  );
}
