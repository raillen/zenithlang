import { FolderTree } from "lucide-react";
import React from "react";
import { ICON_STROKE } from "../constants";
import type { ProjectAsset, SceneEntity } from "../types";
import { assetKindIcon } from "../utils/assets";
import { entityIcon } from "../utils/entity";
import { PanelHeader } from "./shared";

export function NavigatorPanel({
  assets,
  entities,
  selectedEntityId,
  onSelectEntity,
}: {
  assets: ProjectAsset[];
  entities: SceneEntity[];
  selectedEntityId: string;
  onSelectEntity: (entity: SceneEntity) => void;
}) {
  const sceneAsset = assets.find((asset) => asset.kind === "scene");

  return (
    <aside className="side-panel navigator-panel">
      <PanelHeader icon={<FolderTree size={13} strokeWidth={ICON_STROKE} />} title="Navigator" meta="Scene" />
      <div className="navigator-section scene-tree">
        {sceneAsset ? (
          <div className="tree-row scene-root">
            {assetKindIcon(sceneAsset.kind, 12)}
            <span>{sceneAsset.name}</span>
            <small>scene</small>
          </div>
        ) : null}
        {entities.map((entity) => (
          <button
            className={`tree-row scene-child ${selectedEntityId === entity.id ? "selected" : ""}`}
            key={entity.id}
            onClick={() => onSelectEntity(entity)}
          >
            {entityIcon(entity)}
            <span>{entity.name}</span>
            <small>{entity.layer}</small>
          </button>
        ))}
      </div>
    </aside>
  );
}
