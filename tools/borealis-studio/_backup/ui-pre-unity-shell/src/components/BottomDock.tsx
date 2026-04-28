import { Archive, BadgeAlert, Check, Circle, FileCode2, Terminal, Trash2, ZoomIn } from "lucide-react";
import React, { PointerEvent, useCallback, useEffect, useMemo, useRef, useState } from "react";
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
  onClearConsole,
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
  onClearConsole?: () => void;
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
          <ConsoleCounters lines={console} />
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
      {activeTab === "console" ? <ConsoleView lines={console} onClear={onClearConsole} /> : null}
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

function ConsoleCounters({ lines }: { lines: ConsoleLine[] }) {
  const warnCount = lines.filter((l) => l.level === "warn").length;
  const errorCount = lines.filter((l) => l.level === "error").length;
  if (warnCount === 0 && errorCount === 0) return null;
  return (
    <span className="console-counters">
      {warnCount > 0 ? <span className="counter-warn">{warnCount}</span> : null}
      {errorCount > 0 ? <span className="counter-error">{errorCount}</span> : null}
    </span>
  );
}

type ConsoleFilter = "all" | "info" | "warn" | "error";
type SourceFilter = "all" | "studio" | "preview" | "script";

function ConsoleView({ lines, onClear }: { lines: ConsoleLine[]; onClear?: () => void }) {
  const [levelFilter, setLevelFilter] = useState<ConsoleFilter>("all");
  const [sourceFilter, setSourceFilter] = useState<SourceFilter>("all");
  const [autoScroll, setAutoScroll] = useState(true);
  const scrollRef = useRef<HTMLDivElement | null>(null);

  const filtered = useMemo(
    () =>
      lines.filter((line) => {
        if (levelFilter !== "all" && line.level !== levelFilter) return false;
        if (sourceFilter !== "all" && line.source !== sourceFilter) return false;
        return true;
      }),
    [lines, levelFilter, sourceFilter],
  );

  useEffect(() => {
    if (autoScroll && scrollRef.current) {
      scrollRef.current.scrollTop = scrollRef.current.scrollHeight;
    }
  }, [filtered, autoScroll]);

  const handleScroll = useCallback(() => {
    const el = scrollRef.current;
    if (!el) return;
    const atBottom = el.scrollHeight - el.scrollTop - el.clientHeight < 24;
    setAutoScroll(atBottom);
  }, []);

  return (
    <div className="console-view">
      <div className="console-toolbar">
        <select value={levelFilter} onChange={(e) => setLevelFilter(e.target.value as ConsoleFilter)}>
          <option value="all">All levels</option>
          <option value="info">Info</option>
          <option value="warn">Warn</option>
          <option value="error">Error</option>
        </select>
        <select value={sourceFilter} onChange={(e) => setSourceFilter(e.target.value as SourceFilter)}>
          <option value="all">All sources</option>
          <option value="studio">Studio</option>
          <option value="preview">Preview</option>
          <option value="script">Script</option>
        </select>
        <button
          className={autoScroll ? "active" : ""}
          onClick={() => setAutoScroll((v) => !v)}
          title={autoScroll ? "Auto-scroll ON" : "Auto-scroll OFF"}
          type="button"
        >
          Auto
        </button>
        {onClear ? (
          <button onClick={onClear} title="Clear console" type="button">
            <Trash2 size={12} strokeWidth={ICON_STROKE} />
          </button>
        ) : null}
        <span className="console-count">{filtered.length} / {lines.length}</span>
      </div>
      <div className="console-lines" ref={scrollRef} onScroll={handleScroll}>
        {filtered.map((line) => (
          <div className={`console-line ${line.level}`} key={line.id}>
            <span className="console-timestamp">
              {new Date(line.id).toLocaleTimeString("en-US", { hour12: false, hour: "2-digit", minute: "2-digit", second: "2-digit" })}
            </span>
            <span className="console-source">{line.source}</span>
            <p>{line.message}</p>
          </div>
        ))}
      </div>
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
