import { ChevronDown, ChevronRight, Folder, FolderOpen, Search, ZoomIn } from "lucide-react";
import React, { PointerEvent, useMemo, useState } from "react";
import { ICON_STROKE } from "../../constants";
import type { ProjectAsset, ScriptDocument } from "../../types";
import { assetKindIcon } from "../../utils/assets";

interface FolderNode {
  name: string;
  path: string;
  children: FolderNode[];
}

function buildFolderTree(assets: ProjectAsset[]): FolderNode[] {
  const root = new Map<string, FolderNode>();

  function ensureNode(parts: string[], index: number, map: Map<string, FolderNode>, prefix: string): void {
    if (index >= parts.length) return;
    const name = parts[index];
    const path = prefix ? `${prefix}/${name}` : name;
    let node = map.get(path);
    if (!node) {
      node = { name, path, children: [] };
      map.set(path, node);
    }

    if (index + 1 < parts.length) {
      const childMap = new Map(node.children.map((child) => [child.path, child]));
      ensureNode(parts, index + 1, childMap, path);
      node.children = Array.from(childMap.values()).sort((a, b) => a.name.localeCompare(b.name));
    }
  }

  assets.forEach((asset) => {
    const folderPath = asset.path.includes("/") ? asset.path.split("/").slice(0, -1) : [];
    if (folderPath.length > 0) {
      ensureNode(folderPath, 0, root, "");
    }
  });

  return Array.from(root.values()).sort((a, b) => a.name.localeCompare(b.name));
}

export function ProjectPanel({
  assets,
  onAssetDrop,
  onAssetPointerDown,
  onOpenScript,
  onScriptSelect,
  scripts,
}: {
  assets: ProjectAsset[];
  onAssetDrop: (asset: ProjectAsset) => void;
  onAssetPointerDown: (asset: ProjectAsset, event: PointerEvent<HTMLElement>) => void;
  onOpenScript: (path: string) => void;
  onScriptSelect: (path: string) => void;
  scripts: ScriptDocument[];
}) {
  const [search, setSearch] = useState("");
  const [selectedFolder, setSelectedFolder] = useState("");
  const [expandedFolders, setExpandedFolders] = useState<string[]>([]);
  const [tileSize, setTileSize] = useState(112);
  const folderTree = useMemo(() => buildFolderTree(assets), [assets]);

  const visibleAssets = useMemo(() => {
    const normalized = search.trim().toLowerCase();
    return assets.filter((asset) => {
      const inFolder = !selectedFolder || asset.path.startsWith(`${selectedFolder}/`) || asset.path === selectedFolder;
      const matches = !normalized || asset.name.toLowerCase().includes(normalized) || asset.path.toLowerCase().includes(normalized);
      return inFolder && matches;
    });
  }, [assets, search, selectedFolder]);

  const breadcrumbs = selectedFolder ? selectedFolder.split("/") : ["Assets"];

  return (
    <div className="project-shell">
      <div className="project-sidebar">
        <button className={`project-folder-row ${selectedFolder === "" ? "selected" : ""}`} onClick={() => setSelectedFolder("")} type="button">
          <FolderOpen size={14} strokeWidth={ICON_STROKE} />
          <span>Assets</span>
        </button>
        {folderTree.map((node) => (
          <FolderBranch
            expandedFolders={expandedFolders}
            key={node.path}
            node={node}
            onSelect={setSelectedFolder}
            onToggle={(path) =>
              setExpandedFolders((current) =>
                current.includes(path) ? current.filter((item) => item !== path) : [...current, path],
              )
            }
            selectedFolder={selectedFolder}
          />
        ))}
      </div>
      <div className="project-browser">
        <div className="project-browser-toolbar">
          <div className="project-breadcrumbs">
            {breadcrumbs.map((segment, index) => (
              <span key={`${segment}-${index}`}>{segment}</span>
            ))}
          </div>
          <div className="project-browser-actions">
            <label className="project-search">
              <Search size={14} strokeWidth={ICON_STROKE} />
              <input onChange={(event) => setSearch(event.target.value)} placeholder="Search assets" value={search} />
            </label>
            <label className="project-zoom">
              <ZoomIn size={14} strokeWidth={ICON_STROKE} />
              <input max={152} min={88} onChange={(event) => setTileSize(Number(event.target.value))} type="range" value={tileSize} />
            </label>
          </div>
        </div>
        <div
          className="project-grid"
          style={
            {
              "--asset-tile-size": `${tileSize}px`,
              gridTemplateColumns: `repeat(auto-fill, minmax(${tileSize}px, ${tileSize}px))`,
            } as React.CSSProperties
          }
        >
          {visibleAssets.map((asset) => (
            <button
              className="project-asset-card"
              draggable
              key={asset.id}
              onClick={() => {
                if (asset.kind === "script") onScriptSelect(asset.path);
              }}
              onDoubleClick={() => {
                if (asset.kind === "script") {
                  onOpenScript(asset.path);
                  return;
                }
                onAssetDrop(asset);
              }}
              onPointerDown={(event) => onAssetPointerDown(asset, event)}
              onDragStart={(event) => {
                event.dataTransfer.setData("application/x-borealis-asset", JSON.stringify(asset));
              }}
              type="button"
            >
              <span className="project-asset-thumb">{assetKindIcon(asset.kind, Math.max(22, Math.round(tileSize * 0.26)))}</span>
              <strong>{asset.name}</strong>
              <small>{asset.path}</small>
            </button>
          ))}
          {visibleAssets.length === 0 ? <div className="empty-state">No assets found in this folder.</div> : null}
        </div>
      </div>
    </div>
  );
}

function FolderBranch({
  expandedFolders,
  node,
  onSelect,
  onToggle,
  selectedFolder,
}: {
  expandedFolders: string[];
  node: FolderNode;
  onSelect: (path: string) => void;
  onToggle: (path: string) => void;
  selectedFolder: string;
}) {
  const expanded = expandedFolders.includes(node.path);
  return (
    <div>
      <button className={`project-folder-row ${selectedFolder === node.path ? "selected" : ""}`} onClick={() => onSelect(node.path)} type="button">
        <span
          className="project-folder-expander"
          onClick={(event) => {
            event.preventDefault();
            event.stopPropagation();
            onToggle(node.path);
          }}
        >
          {node.children.length > 0 ? expanded ? <ChevronDown size={14} strokeWidth={ICON_STROKE} /> : <ChevronRight size={14} strokeWidth={ICON_STROKE} /> : <span className="hierarchy-expander-spacer" />}
        </span>
        {expanded ? <FolderOpen size={14} strokeWidth={ICON_STROKE} /> : <Folder size={14} strokeWidth={ICON_STROKE} />}
        <span>{node.name}</span>
      </button>
      {expanded && node.children.length > 0 ? (
        <div className="project-folder-children">
          {node.children.map((child) => (
            <FolderBranch
              expandedFolders={expandedFolders}
              key={child.path}
              node={child}
              onSelect={onSelect}
              onToggle={onToggle}
              selectedFolder={selectedFolder}
            />
          ))}
        </div>
      ) : null}
    </div>
  );
}

export function AnimationPanel({ scripts }: { scripts: ScriptDocument[] }) {
  return (
    <div className="animation-panel-placeholder">
      <strong>Animation</strong>
      <span>{scripts.length} script files ready for future timeline binding.</span>
      <div className="animation-timeline-preview" aria-hidden="true">
        <div />
        <div />
        <div />
      </div>
    </div>
  );
}
