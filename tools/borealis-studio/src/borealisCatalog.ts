import type { BorealisEditorManifest, ComponentSchema, SceneComponent, SceneSettingSchema } from "./types";

export const BOREALIS_COMPONENTS: Record<string, ComponentSchema> = {
  ai: {
    module: "borealis.game.ai",
    description: "Behavior and perception profile.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  ai3d: {
    module: "borealis.game.ai3d",
    description: "3D Behavior and navigation profile.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  animation: {
    module: "borealis.game.animation",
    description: "2D animation state and playback.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  animation3d: {
    module: "borealis.game.animation3d",
    description: "3D animation controller and blending.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  atmosphere: {
    module: "borealis.game.world3d.atmosphere",
    description: "Fog, time-of-day, and wind settings.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  audio: {
    module: "borealis.game.audio",
    description: "Sound and music playback.",
    fields: [{ key: "asset", label: "Audio Asset", kind: "asset" }],
  },
  audio3d: {
    module: "borealis.game.audio3d",
    description: "Spatial audio emitter.",
    fields: [{ key: "asset", label: "Audio Asset", kind: "asset" }],
  },
  camera2d: {
    module: "borealis.game.camera",
    description: "2D camera component.",
    fields: [{ key: "zoom", label: "Zoom", kind: "number" }],
  },
  camera3d: {
    module: "borealis.game.camera3d",
    description: "3D camera component.",
    fields: [
      {
        key: "projection",
        label: "Projection",
        kind: "select",
        options: ["perspective", "orthographic"],
      },
    ],
  },
  controller: {
    module: "borealis.game.controllers",
    description: "Ready-made gameplay controller.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  controller3d: {
    module: "borealis.game.controllers3d",
    description: "Ready-made 3D gameplay controller.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  cube3d: {
    module: "borealis.game.render3d",
    description: "Primitive cube rendered by Borealis 3D.",
    fields: [{ key: "size", label: "Size", kind: "number" }],
  },
  debug: {
    module: "borealis.game.debug",
    description: "Debug diagnostics and overlays.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  debug3d: {
    module: "borealis.game.debug3d",
    description: "3D specific diagnostics and gizmos.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  editor: {
    module: "borealis.game.editor",
    description: "Editor metadata and grouping.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  events: {
    module: "borealis.game.events",
    description: "Event listener configuration.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  hud: {
    module: "borealis.game.ui.hud",
    description: "In-game overlay widget.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  model3d: {
    module: "borealis.game.assets3d",
    description: "Loaded 3D model asset.",
    fields: [{ key: "asset", label: "Asset", kind: "asset" }],
  },
  physics2d: {
    module: "borealis.game.movement",
    description: "2D physical body and collision.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  physics3d: {
    module: "borealis.game.physics3d",
    description: "3D physics body and colliders.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  postfx: {
    module: "borealis.game.postfx",
    description: "Screen-space visual finishing.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  procedural: {
    module: "borealis.game.procedural",
    description: "Procedural generation settings.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  procedural3d: {
    module: "borealis.game.procedural3d",
    description: "3D procedural terrain and scatter.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  render3d: {
    module: "borealis.game.render3d",
    description: "3D drawing and lighting.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  script: {
    module: "borealis.game.entities",
    description: "Script attached to the entity.",
    fields: [{ key: "script", label: "Script", kind: "asset" }],
  },
  sprite: {
    module: "borealis.game.assets",
    description: "2D sprite asset.",
    fields: [{ key: "asset", label: "Asset", kind: "asset" }],
  },
  ui: {
    module: "borealis.game.ui",
    description: "General interface widget.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  ui3d: {
    module: "borealis.game.ui3d",
    description: "World-space UI projection.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  vehicles: {
    module: "borealis.game.vehicles",
    description: "Vehicle simulation controller.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  weather: {
    module: "borealis.game.world3d.weather",
    description: "Weather zone settings.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  world: {
    module: "borealis.game.world",
    description: "2D tilemap and world structure.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
  world3d: {
    module: "borealis.game.world3d",
    description: "3D world chunks and grid.",
    fields: [{ key: "profile", label: "Profile", kind: "text" }],
  },
};

let activeComponents: Record<string, ComponentSchema> = BOREALIS_COMPONENTS;
let activeSceneSettings: Record<string, SceneSettingSchema> = {};
let activeManifestSource = "fallback:frontend-catalog";

export function configureBorealisCatalog(manifest?: BorealisEditorManifest): void {
  const manifestComponents = normalizeManifestComponents(manifest?.components);
  const manifestSceneSettings = normalizeManifestSceneSettings(manifest?.sceneSettings);
  activeComponents =
    Object.keys(manifestComponents).length > 0
      ? { ...BOREALIS_COMPONENTS, ...manifestComponents }
      : BOREALIS_COMPONENTS;
  activeSceneSettings = manifestSceneSettings;
  activeManifestSource = manifest?.source ?? "fallback:frontend-catalog";
}

export function borealisComponents(): Record<string, ComponentSchema> {
  return activeComponents;
}

export function borealisManifestSource(): string {
  return activeManifestSource;
}

export function borealisSceneSettings(): Record<string, SceneSettingSchema> {
  return activeSceneSettings;
}

export function createComponentFromSchema(kind: string): SceneComponent {
  const schema = activeComponents[kind];
  const properties: Record<string, unknown> = {};
  const component: SceneComponent = {
    kind,
    properties,
  };

  for (const field of schema?.fields ?? []) {
    if (field.default === undefined) continue;
    properties[field.key] = field.default;
    if (field.key === "asset") component.asset = String(field.default);
    if (field.key === "script") component.script = String(field.default);
    if (field.key === "profile") component.profile = String(field.default);
  }

  return component;
}

export function componentSchema(component: SceneComponent): ComponentSchema {
  return (
    activeComponents[component.kind] ?? {
      module: "borealis.engine.ecs",
      description: "Custom ECS component payload.",
      fields: [],
    }
  );
}

function normalizeManifestComponents(
  components?: Record<string, ComponentSchema>,
): Record<string, ComponentSchema> {
  if (!components) return {};

  const normalized: Record<string, ComponentSchema> = {};
  for (const [kind, schema] of Object.entries(components)) {
    if (!schema || !schema.module) continue;
    normalized[kind] = {
      ...schema,
      description: schema.description ?? schema.label ?? kind,
      fields: Array.isArray(schema.fields) ? schema.fields : [],
    };
  }
  return normalized;
}

function normalizeManifestSceneSettings(
  settings?: Record<string, SceneSettingSchema>,
): Record<string, SceneSettingSchema> {
  if (!settings) return {};

  const normalized: Record<string, SceneSettingSchema> = {};
  for (const [key, schema] of Object.entries(settings)) {
    if (!schema || !schema.module) continue;
    normalized[key] = {
      ...schema,
      fields: Array.isArray(schema.fields) ? schema.fields : [],
    };
  }
  return normalized;
}

export function componentValue(component: SceneComponent, key: string): unknown {
  if (key === "asset") return component.asset ?? component.properties?.asset;
  if (key === "script") return component.script ?? component.properties?.script;
  if (key === "profile") return component.profile ?? component.properties?.profile;
  return component.properties?.[key];
}

export function setComponentValue(component: SceneComponent, key: string, value: unknown): SceneComponent {
  const properties = { ...(component.properties ?? {}) };
  const isEmpty = value === undefined || value === null || value === "";

  if (isEmpty) {
    delete properties[key];
  } else {
    properties[key] = value;
  }

  const next: SceneComponent = {
    ...component,
    properties,
  };

  if (key === "asset") {
    next.asset = isEmpty ? undefined : String(value);
  }
  if (key === "script") {
    next.script = isEmpty ? undefined : String(value);
  }
  if (key === "profile") {
    next.profile = isEmpty ? undefined : String(value);
  }

  return next;
}

export function componentSummary(component: SceneComponent): string {
  const schema = componentSchema(component);
  const firstField = schema.fields[0];
  if (!firstField) return schema.module;
  const value = componentValue(component, firstField.key);
  return typeof value === "string" || typeof value === "number" || typeof value === "boolean"
    ? String(value)
    : schema.module;
}
