import { BoxSelect, Settings2 } from "lucide-react";
import React from "react";
import { ICON_STROKE } from "../../constants";

export function ObjectHeader({
  isSceneRoot,
  layer,
  name,
}: {
  isSceneRoot: boolean;
  layer?: string;
  name: string;
}) {
  return (
    <div className="inspector-object-header">
      <div className="inspector-object-icon">{isSceneRoot ? <Settings2 size={16} strokeWidth={ICON_STROKE} /> : <BoxSelect size={16} strokeWidth={ICON_STROKE} />}</div>
      <div>
        <strong>{name}</strong>
        <span>{isSceneRoot ? "Scene document" : layer || "Entity"}</span>
      </div>
    </div>
  );
}
