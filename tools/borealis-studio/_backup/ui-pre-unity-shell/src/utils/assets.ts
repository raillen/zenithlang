import React from "react";
import {
  Activity,
  Cuboid,
  Database,
  FileCode2,
  Layers3,
  WandSparkles,
} from "lucide-react";
import type { AssetKind } from "../types";
import { ICON_STROKE } from "../constants";

export function assetKindIcon(kind: AssetKind, size = 15) {
  const props = { size, strokeWidth: ICON_STROKE };
  if (kind === "script") return React.createElement(FileCode2, props);
  if (kind === "model") return React.createElement(Cuboid, props);
  if (kind === "scene") return React.createElement(Layers3, props);
  if (kind === "texture") return React.createElement(WandSparkles, props);
  if (kind === "audio") return React.createElement(Activity, props);
  if (kind === "shader" || kind === "cubemap") return React.createElement(WandSparkles, props);
  return React.createElement(Database, props);
}
