import { serializeSceneDocument } from "./sceneSerialization";
import type {
  NewProjectRequest,
  PreviewCommandResult,
  SceneDocument,
  ScriptDocument,
  StudioHome,
  StudioSnapshot,
} from "./types";

declare global {
  interface Window {
    __TAURI_INTERNALS__?: unknown;
  }
}

export function isTauriRuntime(): boolean {
  return typeof window !== "undefined" && Boolean(window.__TAURI_INTERNALS__);
}

async function invokeCommand<T>(command: string, args?: Record<string, unknown>): Promise<T> {
  const { invoke } = await import("@tauri-apps/api/core");
  return invoke<T>(command, args);
}

export async function loadStudioSnapshot(projectPath?: string): Promise<StudioSnapshot> {
  if (!isTauriRuntime()) {
    throw new Error("Tauri runtime is not available");
  }

  return invokeCommand<StudioSnapshot>("load_studio_snapshot", { projectPath });
}

export async function loadStudioHome(): Promise<StudioHome> {
  if (!isTauriRuntime()) {
    throw new Error("Tauri runtime is not available");
  }

  return invokeCommand<StudioHome>("load_studio_home");
}

export async function readScriptDocument(path: string): Promise<ScriptDocument> {
  if (!isTauriRuntime()) {
    throw new Error("Tauri runtime is not available");
  }

  const content = await invokeCommand<string>("read_text_file", { path });
  return {
    path,
    name: path.split("/").pop() ?? path,
    content,
  };
}

export async function writeScriptDocument(script: ScriptDocument, projectRoot?: string): Promise<void> {
  if (!isTauriRuntime()) {
    throw new Error("Tauri runtime is not available");
  }

  const args = projectRoot
    ? { projectRoot, path: script.path, content: script.content }
    : { path: script.path, content: script.content };
  await invokeCommand<void>(projectRoot ? "write_project_text_file" : "write_text_file", args);
}

export async function writeSceneDocument(scene: SceneDocument, projectRoot?: string): Promise<void> {
  if (!isTauriRuntime()) {
    throw new Error("Tauri runtime is not available");
  }

  const args = projectRoot
    ? { projectRoot, path: scene.path, content: serializeSceneDocument(scene) }
    : { path: scene.path, content: serializeSceneDocument(scene) };
  await invokeCommand<void>(projectRoot ? "write_project_text_file" : "write_text_file", args);
}

export async function createBorealisProject(request: NewProjectRequest): Promise<StudioSnapshot> {
  if (!isTauriRuntime()) {
    throw new Error("Tauri runtime is not available");
  }

  return invokeCommand<StudioSnapshot>("create_borealis_project", {
    projectName: request.projectName,
    parentDir: request.parentDir,
    templateId: request.templateId,
  });
}

export async function startRuntimePreview(projectPath: string, scene: SceneDocument): Promise<PreviewCommandResult> {
  if (!isTauriRuntime()) {
    throw new Error("Tauri runtime is not available");
  }

  return invokeCommand<PreviewCommandResult>("start_preview", {
    projectPath,
    scenePath: scene.path,
    sceneJson: serializeSceneDocument(scene),
  });
}

export async function pauseRuntimePreview(): Promise<PreviewCommandResult> {
  if (!isTauriRuntime()) {
    throw new Error("Tauri runtime is not available");
  }

  return invokeCommand<PreviewCommandResult>("pause_preview");
}

export async function stopRuntimePreview(): Promise<PreviewCommandResult> {
  if (!isTauriRuntime()) {
    throw new Error("Tauri runtime is not available");
  }

  return invokeCommand<PreviewCommandResult>("stop_preview");
}

export async function pollRuntimePreview(): Promise<PreviewCommandResult> {
  if (!isTauriRuntime()) {
    throw new Error("Tauri runtime is not available");
  }

  return invokeCommand<PreviewCommandResult>("poll_preview");
}
