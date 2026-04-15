import { FileEntry } from "../types/ide";
import { invoke } from "./tauri";

const pendingDirectoryLoads = new Map<string, Promise<FileEntry[]>>();

export function normalizeWorkspacePath(input: string) {
  const normalized = String(input || "")
    .replace(/\\/g, "/")
    .replace(/\/+$/g, "");

  return normalized || "/";
}

function pathKey(input: string) {
  return normalizeWorkspacePath(input).toLowerCase();
}

export function decorateExplorerEntries(entries: FileEntry[]): FileEntry[] {
  return entries.map((entry) => ({
    ...entry,
    isLoaded: entry.is_directory ? Array.isArray(entry.children) : undefined,
    children: entry.children ? decorateExplorerEntries(entry.children) : undefined,
  }));
}

function mergeExplorerEntry(previousEntry: FileEntry | undefined, nextEntry: FileEntry): FileEntry {
  if (!nextEntry.is_directory) {
    return { ...nextEntry };
  }

  const nextChildren = nextEntry.children;
  const previousChildren = previousEntry?.children || [];

  return {
    ...nextEntry,
    isLoaded: nextChildren !== undefined ? true : Boolean(previousEntry?.isLoaded),
    children:
      nextChildren !== undefined
        ? mergeExplorerEntries(previousChildren, nextChildren)
        : previousEntry?.isLoaded
          ? previousChildren
          : undefined,
  };
}

export function mergeExplorerEntries(previousEntries: FileEntry[], nextEntries: FileEntry[]) {
  const previousByPath = new Map(
    previousEntries.map((entry) => [pathKey(entry.path), entry] as const)
  );

  return nextEntries.map((entry) =>
    mergeExplorerEntry(previousByPath.get(pathKey(entry.path)), entry)
  );
}

export async function loadDirectoryEntries(rootPath: string): Promise<FileEntry[]> {
  const normalizedRoot = pathKey(rootPath);
  const pendingRequest = pendingDirectoryLoads.get(normalizedRoot);
  if (pendingRequest) {
    return pendingRequest;
  }

  const request = invoke<FileEntry[]>("get_file_tree", {
    rootPath,
    recursive: false,
  })
    .then((entries) => decorateExplorerEntries(entries))
    .finally(() => {
      pendingDirectoryLoads.delete(normalizedRoot);
    });

  pendingDirectoryLoads.set(normalizedRoot, request);
  return request;
}
