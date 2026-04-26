import type { AssetKind, ComponentFieldSchema, ProjectAsset, SceneComponent, ScriptDocument } from "../types";

export function formatComponentValue(value: unknown): string {
  if (value === undefined || value === null || value === "") return "none";
  if (typeof value === "string" || typeof value === "number" || typeof value === "boolean") return String(value);
  return JSON.stringify(value);
}

export function componentInputValue(value: unknown): string | number {
  if (typeof value === "number" || typeof value === "string") return value;
  if (typeof value === "boolean") return String(value);
  return "";
}

export function isAssetKind(value: unknown): value is AssetKind {
  return (
    value === "scene" ||
    value === "script" ||
    value === "model" ||
    value === "texture" ||
    value === "audio" ||
    value === "shader" ||
    value === "cubemap" ||
    value === "data"
  );
}

export function assetOptionsForComponentField(
  component: SceneComponent,
  field: ComponentFieldSchema,
  assets: ProjectAsset[],
  scripts: ScriptDocument[],
): string[] {
  if (field.key === "script" || component.kind === "script") {
    return scripts.map((script) => script.path);
  }

  let wantedKind: AssetKind | null = null;
  if (isAssetKind(field.assetKind)) wantedKind = field.assetKind;
  else if (component.kind === "model3d") wantedKind = "model";
  else if (component.kind === "sprite") wantedKind = "texture";
  else if (component.kind === "audio" || component.kind === "audio3d") wantedKind = "audio";

  return assets
    .filter((asset) => asset.kind !== "scene" && (!wantedKind || asset.kind === wantedKind))
    .map((asset) => asset.path);
}

export function includeCurrentOption(options: string[], value: string): string[] {
  if (!value || options.includes(value)) return options;
  return [value, ...options];
}
