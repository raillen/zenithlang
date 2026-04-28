import type { SceneAudioSettings, SceneDocument, SceneEnvironment, SceneRenderSettings } from "../types";

export function defaultSceneEnvironment(): SceneEnvironment {
  return {
    skybox: {},
    ambient: {},
    fog: {},
    weather: {},
  };
}

export function defaultSceneRender(): SceneRenderSettings {
  return {
    quality: {},
    postfx: {},
    camera: {},
  };
}

export function sceneColorValue(value: Record<string, unknown>, key: string, fallback: string): string {
  const item = value[key];
  return typeof item === "string" && item.startsWith("#") ? item : fallback;
}

export function sceneNumberValue(value: Record<string, unknown>, key: string, fallback: number): number {
  const item = value[key];
  return typeof item === "number" && Number.isFinite(item) ? item : fallback;
}

export function sceneBooleanValue(value: Record<string, unknown>, key: string, fallback: boolean): boolean {
  const item = value[key];
  return typeof item === "boolean" ? item : fallback;
}

export function sceneSettingSection(key: string): "environment" | "render" | "audio" {
  if (key in defaultSceneEnvironment()) return "environment";
  if (key in defaultSceneRender()) return "render";
  return "audio";
}

export function sceneSettingValues(scene: SceneDocument, key: string): Record<string, unknown> {
  const section = sceneSettingSection(key);
  const value =
    section === "environment" ? scene.environment[key as keyof SceneEnvironment] :
    section === "render" ? scene.render[key as keyof SceneRenderSettings] :
    scene.audio[key as keyof SceneAudioSettings];
  return value && typeof value === "object" && !Array.isArray(value) ? value : {};
}
