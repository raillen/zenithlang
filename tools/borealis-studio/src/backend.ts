import type { ScriptDocument, StudioSnapshot } from "./types";

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

export async function writeScriptDocument(script: ScriptDocument): Promise<void> {
  if (!isTauriRuntime()) {
    throw new Error("Tauri runtime is not available");
  }

  await invokeCommand<void>("write_text_file", {
    path: script.path,
    content: script.content,
  });
}
