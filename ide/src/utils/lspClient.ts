import { listen, UnlistenFn } from "@tauri-apps/api/event";
import { FileEntry, useWorkspaceStore } from "../store/useWorkspaceStore";
import { detectRuntimeProfileFromFile, getRuntimeProfileForSettingsKey } from "./runtimeProfiles";
import { invoke, isTauriEnvironment } from "./tauri";
import { rememberBuffer } from "./editorBuffers";

export type LspDiagnostic = {
  line: number;
  col: number;
  endLine: number;
  endCol: number;
  message: string;
  severity: "error" | "warning" | "hint";
  code: string;
};

export type LspCompletionItem = {
  label: string;
  detail?: string | null;
  documentation?: string | null;
  insertText?: string | null;
  kind?: number | null;
};

export type LspLocation = {
  filePath: string;
  line: number;
  col: number;
};

export type LspWorkspaceEditResult = {
  files: Array<{
    filePath: string;
    edits: Array<{
      startLine: number;
      startCol: number;
      endLine: number;
      endCol: number;
      newText: string;
    }>;
  }>;
};

export type LspDiagnosticsEvent = {
  filePath: string;
  diagnostics: LspDiagnostic[];
};

function getRuntimeConfig(file: FileEntry, languageId: string, text: string) {
  if (!isTauriEnvironment()) return null;

  const workspace = useWorkspaceStore.getState();
  const runtimeId = detectRuntimeProfileFromFile(file);
  if (runtimeId === "generic") return null;

  const profile = getRuntimeProfileForSettingsKey(runtimeId);
  const extensionConfig = workspace.settings.extensions?.[runtimeId];
  if (!extensionConfig?.isEnabled) return null;

  const lspCommand = extensionConfig.lspPath?.trim() || profile.lspPlaceholder;
  if (!lspCommand) return null;

  return {
    runtimeId,
    rootPath: workspace.currentProjectRoot,
    lspCommand,
    filePath: file.path,
    languageId,
    text,
  };
}

function toPosition(lineNumber: number, column: number) {
  return {
    line: Math.max(0, lineNumber - 1),
    character: Math.max(0, column - 1),
  };
}

export function getModelPath(model: any) {
  const uri = model?.uri;
  if (!uri) return "";

  const candidate = uri.fsPath || uri.path || uri.toString?.() || "";
  const decoded = decodeURIComponent(String(candidate));
  return decoded.replace(/^file:\/+/, "").replace(/^\//, (value) => {
    if (/^[A-Za-z]:/.test(decoded.slice(1))) return "";
    return value;
  });
}

export function getFileContextFromModel(model: any): FileEntry | null {
  const path = getModelPath(model);
  if (!path) return null;
  const name = path.split(/[\\/]/).pop() || "file";
  return { name, path, is_directory: false };
}

export async function syncLspDocument(file: FileEntry, languageId: string, text: string) {
  const config = getRuntimeConfig(file, languageId, text);
  if (!config) return false;

  await invoke("lsp_sync_document", { request: config });
  return true;
}

export async function getLatestLspDiagnostics(file: FileEntry, languageId: string, text: string) {
  const config = getRuntimeConfig(file, languageId, text);
  if (!config) return [];

  return invoke<LspDiagnostic[]>("lsp_latest_diagnostics", {
    runtimeId: config.runtimeId,
    rootPath: config.rootPath,
    lspCommand: config.lspCommand,
    filePath: config.filePath,
  });
}

export async function requestLspCompletions(file: FileEntry, languageId: string, text: string, lineNumber: number, column: number) {
  const config = getRuntimeConfig(file, languageId, text);
  if (!config) return [];

  return invoke<LspCompletionItem[]>("lsp_completion", {
    request: {
      ...config,
      ...toPosition(lineNumber, column),
    },
  });
}

export async function requestLspHover(file: FileEntry, languageId: string, text: string, lineNumber: number, column: number) {
  const config = getRuntimeConfig(file, languageId, text);
  if (!config) return null;

  return invoke<{ contents: string } | null>("lsp_hover", {
    request: {
      ...config,
      ...toPosition(lineNumber, column),
    },
  });
}

export async function requestLspDefinition(file: FileEntry, languageId: string, text: string, lineNumber: number, column: number) {
  const config = getRuntimeConfig(file, languageId, text);
  if (!config) return [];

  return invoke<LspLocation[]>("lsp_definition", {
    request: {
      ...config,
      ...toPosition(lineNumber, column),
    },
  });
}

export async function requestLspReferences(file: FileEntry, languageId: string, text: string, lineNumber: number, column: number) {
  const config = getRuntimeConfig(file, languageId, text);
  if (!config) return [];

  return invoke<LspLocation[]>("lsp_references", {
    request: {
      ...config,
      ...toPosition(lineNumber, column),
    },
  });
}

export async function requestLspRename(file: FileEntry, languageId: string, text: string, lineNumber: number, column: number, newName: string) {
  const config = getRuntimeConfig(file, languageId, text);
  if (!config) return null;

  return invoke<LspWorkspaceEditResult>("lsp_rename", {
    request: {
      ...config,
      ...toPosition(lineNumber, column),
      newName,
    },
  });
}

export async function shutdownLspForFile(file: FileEntry) {
  const workspace = useWorkspaceStore.getState();
  const runtimeId = detectRuntimeProfileFromFile(file);
  const extensionConfig = workspace.settings.extensions?.[runtimeId];
  const profile = getRuntimeProfileForSettingsKey(runtimeId);
  const lspCommand = extensionConfig?.lspPath?.trim() || profile.lspPlaceholder;
  if (!extensionConfig?.isEnabled || !lspCommand) return;

  await invoke("lsp_shutdown", {
    runtimeId,
    rootPath: workspace.currentProjectRoot,
    lspCommand,
  });
}

export async function applyWorkspaceEdit(edit: LspWorkspaceEditResult | null) {
  if (!edit || edit.files.length === 0) return false;

  const workspace = useWorkspaceStore.getState();
  const activePath = workspace.activeFile?.path;

  for (const fileEdit of edit.files) {
    const original = await invoke<string>("read_file", { path: fileEdit.filePath });
    const updated = applyTextEdits(original, fileEdit.edits);
    await invoke("write_file", { path: fileEdit.filePath, content: updated });
    rememberBuffer(fileEdit.filePath, updated);
    workspace.setFileDirty(fileEdit.filePath, false);

    if (activePath && activePath === fileEdit.filePath) {
      window.dispatchEvent(
        new CustomEvent("zenith://editor-replace-content", {
          detail: { path: fileEdit.filePath, content: updated },
        })
      );
    }
  }

  return true;
}

function applyTextEdits(
  content: string,
  edits: Array<{
    startLine: number;
    startCol: number;
    endLine: number;
    endCol: number;
    newText: string;
  }>
) {
  const sorted = [...edits].sort((left, right) => {
    if (left.startLine !== right.startLine) return right.startLine - left.startLine;
    return right.startCol - left.startCol;
  });

  let nextContent = content;
  for (const edit of sorted) {
    const startOffset = getOffset(nextContent, edit.startLine, edit.startCol);
    const endOffset = getOffset(nextContent, edit.endLine, edit.endCol);
    nextContent = `${nextContent.slice(0, startOffset)}${edit.newText}${nextContent.slice(endOffset)}`;
  }
  return nextContent;
}

function getOffset(content: string, line: number, col: number) {
  const lines = content.split(/\r?\n/);
  let offset = 0;
  for (let index = 0; index < line - 1; index += 1) {
    offset += lines[index]?.length ?? 0;
    offset += 1;
  }
  return offset + Math.max(0, col - 1);
}

export async function listenForLspDiagnostics(handler: (payload: LspDiagnosticsEvent) => void): Promise<UnlistenFn | null> {
  if (!isTauriEnvironment()) return null;
  return listen<LspDiagnosticsEvent>("zenith://lsp-diagnostics", (event) => handler(event.payload));
}
