import { useEffect, useRef, useState } from "react";
import {
  ChevronDown,
  ChevronRight,
  Clipboard,
  Copy,
  Edit3,
  File as FileIcon,
  FileCode,
  FilePlus2,
  Files,
  Folder,
  FolderOpen,
  FolderPlus,
  RefreshCcw,
  Scissors,
  Search,
  Terminal as TerminalIcon,
  Trash2,
  X,
} from "lucide-react";
import * as ContextMenu from "@radix-ui/react-context-menu";
import { useWorkspaceStore, FileEntry } from "../store/useWorkspaceStore";
import { useExecutionStore } from "../store/useExecutionStore";
import { invoke } from "../utils/tauri";

type DraftMode = "file" | "folder" | "rename";

type DraftState = {
  mode: DraftMode;
  parentPath: string;
  target?: FileEntry;
  depth: number;
  initialName: string;
};

type ExplorerCreateEvent = CustomEvent<{ type?: "file" | "folder"; parentPath?: string }>;

type ClipboardMode = "copy" | "cut";

type ExplorerClipboard = {
  mode: ClipboardMode;
  entries: FileEntry[];
};

const pathKey = (path: string) =>
  path.replace(/\\/g, "/").replace(/\/+$/g, "").toLowerCase();

const isSamePath = (left: string, right: string) => pathKey(left) === pathKey(right);

const isSameOrChildPath = (path: string, root: string) => {
  const normalizedPath = pathKey(path);
  const normalizedRoot = pathKey(root);
  return normalizedPath === normalizedRoot || normalizedPath.startsWith(`${normalizedRoot}/`);
};

const remapExpandedPath = (path: string, oldPath: string, newPath: string) => {
  const normalizedPath = pathKey(path);
  const normalizedOldPath = pathKey(oldPath);

  if (normalizedPath === normalizedOldPath) return newPath;

  const oldPrefix = normalizedOldPath.endsWith("/") ? normalizedOldPath : `${normalizedOldPath}/`;
  if (!normalizedPath.startsWith(oldPrefix)) return path;

  const suffix = path.replace(/\\/g, "/").slice(oldPrefix.length);
  return `${newPath.replace(/[\\/]+$/g, "")}/${suffix}`;
};

const getBaseName = (path: string) =>
  path.replace(/[\\/]+$/g, "").split(/[\\/]/).pop() || path || "Workspace";

const getExplorerItemTestId = (path: string) => {
  const normalized = path
    .replace(/\\/g, "/")
    .replace(/^\.(\/|$)/, "")
    .replace(/^\.\.\//, "")
    .replace(/[^a-zA-Z0-9]+/g, "-")
    .replace(/^-+|-+$/g, "")
    .toLowerCase();

  return `explorer-item-${normalized || "root"}`;
};

const getParentPath = (path: string, rootPath: string) => {
  const normalized = path.replace(/[\\/]+$/g, "");
  const lastSeparator = normalized.lastIndexOf("\\");
  const unixSeparator = normalized.lastIndexOf("/");
  const separatorIndex = Math.max(lastSeparator, unixSeparator);
  if (separatorIndex <= 0) return rootPath;
  return normalized.slice(0, separatorIndex);
};

const findEntryByPath = (entries: FileEntry[], targetPath: string): FileEntry | null => {
  for (const entry of entries) {
    if (isSamePath(entry.path, targetPath)) return entry;

    if (entry.children) {
      const match = findEntryByPath(entry.children, targetPath);
      if (match) return match;
    }
  }

  return null;
};

const flattenEntries = (entries: FileEntry[]): FileEntry[] => {
  const next: FileEntry[] = [];
  for (const entry of entries) {
    next.push(entry);
    if (entry.children?.length) {
      next.push(...flattenEntries(entry.children));
    }
  }
  return next;
};

const getChildrenForParent = (entries: FileEntry[], parentPath: string, rootPath: string) => {
  if (isSamePath(parentPath, rootPath)) return entries;
  const parent = findEntryByPath(entries, parentPath);
  return parent?.children || [];
};

const getUniqueName = (
  existingNames: Set<string>,
  preferredName: string,
  duplicate = false
) => {
  const lower = preferredName.toLowerCase();
  if (!existingNames.has(lower) && !duplicate) return preferredName;

  const dotIndex = preferredName.lastIndexOf(".");
  const hasExtension = dotIndex > 0;
  const base = hasExtension ? preferredName.slice(0, dotIndex) : preferredName;
  const extension = hasExtension ? preferredName.slice(dotIndex) : "";
  const duplicateBase = duplicate ? `${base} copy` : base;

  for (let index = 1; index < 1000; index += 1) {
    const suffix = index === 1 ? "" : ` ${index}`;
    const candidate = `${duplicateBase}${suffix}${extension}`;
    if (!existingNames.has(candidate.toLowerCase())) {
      return candidate;
    }
  }

  return `${duplicateBase}${extension}`;
};

const getExistingNames = (entries: FileEntry[], parentPath: string, rootPath: string) =>
  new Set(
    getChildrenForParent(entries, parentPath, rootPath).map((entry) => entry.name.toLowerCase())
  );

const findAncestorPaths = (
  entries: FileEntry[],
  targetPath: string,
  trail: string[] = []
): string[] | null => {
  for (const entry of entries) {
    if (isSamePath(entry.path, targetPath)) return trail;

    if (entry.is_directory && entry.children?.length) {
      const result = findAncestorPaths(entry.children, targetPath, [...trail, entry.path]);
      if (result) return result;
    }
  }

  return null;
};

const findEntryDepth = (
  entries: FileEntry[],
  targetPath: string,
  depth = 0
): number => {
  for (const entry of entries) {
    if (isSamePath(entry.path, targetPath)) return depth;

    if (entry.is_directory && entry.children?.length) {
      const result = findEntryDepth(entry.children, targetPath, depth + 1);
      if (result >= 0) return result;
    }
  }

  return -1;
};

export function FileNavigator() {
  const {
    fileTree,
    setFileTree,
    openFile,
    activeFile,
    currentProjectRoot,
    replacePathReference,
    removePathReferences,
    setSidebarTab,
    setSidebarOpen,
    setSearchFilterPath,
  } = useWorkspaceStore();

  const { addSession } = useExecutionStore();

  const [gitStatus, setGitStatus] = useState<Record<string, string>>({});
  const [expandedPaths, setExpandedPaths] = useState<Set<string>>(new Set());
  const [selectedPaths, setSelectedPaths] = useState<Set<string>>(new Set());
  const [clipboard, setClipboard] = useState<ExplorerClipboard | null>(null);
  const [draft, setDraft] = useState<DraftState | null>(null);
  const [operationError, setOperationError] = useState<string | null>(null);
  const [isLoading, setIsLoading] = useState(false);
  const [dropTargetPath, setDropTargetPath] = useState<string | null>(null);
  const requestIdRef = useRef(0);
  const dragEntriesRef = useRef<FileEntry[]>([]);

  const allEntries = flattenEntries(fileTree);

  const refreshData = async () => {
    const requestId = requestIdRef.current + 1;
    requestIdRef.current = requestId;
    setIsLoading(true);

    try {
      const tree = await invoke<FileEntry[]>("get_file_tree", { rootPath: currentProjectRoot });
      if (requestId !== requestIdRef.current) return;

      setFileTree(tree);
      setOperationError(null);

      try {
        const git = await invoke<Record<string, string>>("get_git_status");
        if (requestId === requestIdRef.current) setGitStatus(git);
      } catch (gitError) {
        console.warn("Failed to fetch git status", gitError);
        if (requestId === requestIdRef.current) setGitStatus({});
      }
    } catch (err) {
      if (requestId === requestIdRef.current) {
        const message = String(err);
        setOperationError(message);
        console.error("Failed to fetch navigator data", err);
      }
    } finally {
      if (requestId === requestIdRef.current) setIsLoading(false);
    }
  };

  const pruneSelection = (paths: Set<string>) => {
    const next = new Set<string>();
    for (const path of paths) {
      if (findEntryByPath(fileTree, path)) next.add(path);
    }
    return next;
  };

  const getActionEntries = (preferred?: FileEntry) => {
    if (preferred && selectedPaths.has(preferred.path)) {
      return allEntries.filter((entry) => selectedPaths.has(entry.path));
    }

    if (preferred) return [preferred];

    if (selectedPaths.size > 0) {
      return allEntries.filter((entry) => selectedPaths.has(entry.path));
    }

    return [];
  };

  const selectSingle = (path: string) => {
    setSelectedPaths(new Set([path]));
  };

  const toggleSelected = (path: string) => {
    setSelectedPaths((previous) => {
      const next = new Set(previous);
      if (next.has(path)) {
        next.delete(path);
      } else {
        next.add(path);
      }
      return next;
    });
  };

  const ensureExpanded = (path: string) => {
    const ancestors = findAncestorPaths(fileTree, path) || [];
    if (ancestors.length === 0) return;

    setExpandedPaths((previous) => {
      const next = new Set(previous);
      for (const ancestor of ancestors) next.add(ancestor);
      return next;
    });
  };

  const openTerminalAtPath = async (path: string, label?: string) => {
    try {
      const session = await invoke<{ sessionId: number; shell: string; cwd: string }>("terminal_create", {
        cwd: path,
      });
      addSession(session.sessionId, session.shell, label || getBaseName(path), undefined, session.cwd);
    } catch (err) {
      console.error("Failed to open terminal at path", err);
    }
  };

  const handleSearchInFolder = (path: string) => {
    const root = currentProjectRoot;
    const cleanPath = path.startsWith(root)
      ? path.slice(root.length).replace(/^[\\/]+/, "")
      : path;

    setSearchFilterPath(cleanPath);
    setSidebarTab("search");
    setSidebarOpen(true);
  };

  const startDraft = (mode: DraftMode, parentPath: string, depth: number, target?: FileEntry) => {
    const preferredName =
      mode === "rename"
        ? target?.name || "Untitled"
        : mode === "folder"
          ? "New Folder"
          : "Untitled.zt";

    const initialName =
      mode === "rename"
        ? preferredName
        : getUniqueName(
            getExistingNames(fileTree, parentPath, currentProjectRoot),
            preferredName
          );

    setOperationError(null);
    setDraft({ mode, parentPath, target, depth, initialName });

    if (mode !== "rename") {
      setExpandedPaths((previous) => {
        const next = new Set(previous);
        next.add(parentPath);
        return next;
      });
    }
  };

  const cancelDraft = () => {
    setDraft(null);
    setOperationError(null);
  };

  const commitDraft = async (rawName: string) => {
    if (!draft) return;

    const name = rawName.trim();
    if (!name) {
      setOperationError("Name cannot be empty.");
      return;
    }

    setOperationError(null);

    try {
      if (draft.mode === "rename") {
        if (!draft.target || draft.target.name === name) {
          cancelDraft();
          return;
        }

        const renamedEntry = await invoke<FileEntry>("rename_path", {
          path: draft.target.path,
          newName: name,
        });

        replacePathReference(draft.target.path, renamedEntry);
        setSelectedPaths(new Set([renamedEntry.path]));
        setExpandedPaths((previous) => {
          const next = new Set<string>();
          previous.forEach((path) =>
            next.add(remapExpandedPath(path, draft.target!.path, renamedEntry.path))
          );
          if (renamedEntry.is_directory) next.add(renamedEntry.path);
          return next;
        });
      } else if (draft.mode === "folder") {
        const folder = await invoke<FileEntry>("create_folder", {
          parentPath: draft.parentPath,
          name,
        });

        setSelectedPaths(new Set([folder.path]));
        setExpandedPaths((previous) => {
          const next = new Set(previous);
          next.add(draft.parentPath);
          next.add(folder.path);
          return next;
        });
      } else {
        const file = await invoke<FileEntry>("create_file", {
          parentPath: draft.parentPath,
          name,
          content: "",
        });

        setSelectedPaths(new Set([file.path]));
        setExpandedPaths((previous) => {
          const next = new Set(previous);
          next.add(draft.parentPath);
          return next;
        });
        openFile(file, true);
      }

      setDraft(null);
      await refreshData();
    } catch (err) {
      const message = String(err);
      setOperationError(message);
      console.error("Explorer operation failed", err);
    }
  };

  const deleteEntries = async (entries: FileEntry[]) => {
    if (entries.length === 0) return;

    const confirmed = window.confirm(
      entries.length === 1
        ? `Delete ${entries[0].is_directory ? "folder" : "file"} "${entries[0].name}" from disk?`
        : `Delete ${entries.length} selected items from disk?`
    );
    if (!confirmed) return;

    setOperationError(null);

    try {
      for (const entry of entries) {
        await invoke("delete_path", {
          path: entry.path,
          recursive: entry.is_directory,
        });

        removePathReferences(entry.path);
      }

      setSelectedPaths(new Set());
      setExpandedPaths((previous) => {
        const next = new Set<string>();
        previous.forEach((path) => {
          if (!entries.some((entry) => isSameOrChildPath(path, entry.path))) next.add(path);
        });
        return next;
      });
      await refreshData();
    } catch (err) {
      const message = String(err);
      setOperationError(message);
      console.error("Failed to delete entry", err);
    }
  };

  const deleteEntry = async (entry: FileEntry) => {
    const actionEntries = getActionEntries(entry);
    await deleteEntries(actionEntries);
  };

  const toggleFolder = (entry: FileEntry) => {
    setExpandedPaths((previous) => {
      const next = new Set(previous);
      if (next.has(entry.path)) {
        next.delete(entry.path);
      } else {
        next.add(entry.path);
      }
      return next;
    });
  };

  const copySelection = (mode: ClipboardMode, preferred?: FileEntry) => {
    const entries = getActionEntries(preferred);
    if (entries.length === 0) return;

    setClipboard({
      mode,
      entries: entries.map((entry) => ({
        name: entry.name,
        path: entry.path,
        is_directory: entry.is_directory,
      })),
    });
  };

  const duplicateSelection = async (preferred?: FileEntry) => {
    const entries = getActionEntries(preferred);
    if (entries.length === 0) return;

    try {
      const nextSelected = new Set<string>();

      for (const entry of entries) {
        const parentPath = getParentPath(entry.path, currentProjectRoot);
        const duplicatedEntry = await invoke<FileEntry>("copy_path", {
          sourcePath: entry.path,
          destinationParentPath: parentPath,
        });
        nextSelected.add(duplicatedEntry.path);
      }

      setSelectedPaths(nextSelected);
      await refreshData();
    } catch (err) {
      setOperationError(String(err));
      console.error("Failed to duplicate selection", err);
    }
  };

  const pasteInto = async (destinationPath: string) => {
    if (!clipboard || clipboard.entries.length === 0) return;

    try {
      const nextSelected = new Set<string>();

      for (const entry of clipboard.entries) {
        const sourceParentPath = getParentPath(entry.path, currentProjectRoot);
        if (clipboard.mode === "cut" && isSamePath(sourceParentPath, destinationPath)) {
          nextSelected.add(entry.path);
          continue;
        }

        const command = clipboard.mode === "cut" ? "move_path" : "copy_path";
        const movedOrCopied = await invoke<FileEntry>(command, {
          sourcePath: entry.path,
          destinationParentPath: destinationPath,
        });

        if (clipboard.mode === "cut") {
          replacePathReference(entry.path, movedOrCopied);
        }

        nextSelected.add(movedOrCopied.path);
      }

      if (clipboard.mode === "cut") {
        setClipboard(null);
      }

      setExpandedPaths((previous) => {
        const next = new Set(previous);
        next.add(destinationPath);
        return next;
      });
      setSelectedPaths(nextSelected);
      await refreshData();
    } catch (err) {
      setOperationError(String(err));
      console.error("Failed to paste into folder", err);
    }
  };

  const moveEntriesInto = async (entries: FileEntry[], destinationPath: string) => {
    if (entries.length === 0) return;

    try {
      const nextSelected = new Set<string>();

      for (const entry of entries) {
        const sourceParentPath = getParentPath(entry.path, currentProjectRoot);
        if (isSamePath(sourceParentPath, destinationPath)) {
          nextSelected.add(entry.path);
          continue;
        }

        const movedEntry = await invoke<FileEntry>("move_path", {
          sourcePath: entry.path,
          destinationParentPath: destinationPath,
        });
        replacePathReference(entry.path, movedEntry);
        nextSelected.add(movedEntry.path);
      }

      setExpandedPaths((previous) => {
        const next = new Set(previous);
        next.add(destinationPath);
        return next;
      });
      setSelectedPaths(nextSelected);
      await refreshData();
    } catch (err) {
      setOperationError(String(err));
      console.error("Failed to move entries", err);
    }
  };

  const getPasteDestination = () => {
    if (selectedPaths.size === 1) {
      const [selectedPath] = Array.from(selectedPaths);
      const selectedEntry = findEntryByPath(fileTree, selectedPath);
      if (selectedEntry?.is_directory) return selectedEntry.path;
      if (selectedEntry) return getParentPath(selectedEntry.path, currentProjectRoot);
    }

    return currentProjectRoot;
  };

  const revealActiveFile = () => {
    if (!activeFile) return;
    ensureExpanded(activeFile.path);
    setSelectedPaths(new Set([activeFile.path]));
  };

  const openFolderInSystemExplorer = async (entry: FileEntry) => {
    const targetPath = entry.is_directory
      ? entry.path
      : getParentPath(entry.path, currentProjectRoot);

    try {
      await invoke("open_in_system_explorer", { path: targetPath });
    } catch (err) {
      setOperationError(String(err));
      console.error("Failed to open folder in system explorer", err);
    }
  };

  useEffect(() => {
    void refreshData();

    const handleFsChange = () => {
      void refreshData();
    };

    window.addEventListener("zenith://fs-changed", handleFsChange);
    return () => window.removeEventListener("zenith://fs-changed", handleFsChange);
  }, [currentProjectRoot]);

  useEffect(() => {
    setExpandedPaths(new Set());
    setSelectedPaths(new Set());
    setClipboard(null);
    setDraft(null);
    setOperationError(null);
  }, [currentProjectRoot]);

  useEffect(() => {
    setSelectedPaths((previous) => pruneSelection(previous));
  }, [fileTree]);

  useEffect(() => {
    const handleCreateRequest = (event: Event) => {
      const detail = (event as ExplorerCreateEvent).detail;
      const type = detail?.type === "folder" ? "folder" : "file";
      const parentPath = detail?.parentPath || currentProjectRoot;
      const parent = findEntryByPath(fileTree, parentPath);
      const depth = parent?.is_directory ? 1 : 0;

      startDraft(type, parentPath, depth);
    };

    window.addEventListener("zenith://explorer-create", handleCreateRequest);
    return () => window.removeEventListener("zenith://explorer-create", handleCreateRequest);
  }, [currentProjectRoot, fileTree]);

  useEffect(() => {
    if (!activeFile) return;
    ensureExpanded(activeFile.path);
    setSelectedPaths((previous) => (previous.size === 0 ? new Set([activeFile.path]) : previous));
  }, [activeFile?.path, fileTree]);

  const handleExplorerKeyDown = (event: React.KeyboardEvent<HTMLDivElement>) => {
    const isModifier = event.ctrlKey || event.metaKey;
    const key = event.key.toLowerCase();

    if (isModifier && key === "c") {
      event.preventDefault();
      copySelection("copy");
      return;
    }

    if (isModifier && key === "x") {
      event.preventDefault();
      copySelection("cut");
      return;
    }

    if (isModifier && key === "v") {
      event.preventDefault();
      void pasteInto(getPasteDestination());
      return;
    }

    if (isModifier && key === "d") {
      event.preventDefault();
      void duplicateSelection();
      return;
    }

    if (event.key === "Delete") {
      event.preventDefault();
      void deleteEntries(getActionEntries());
      return;
    }

    if (event.key === "Escape") {
      event.preventDefault();
      setSelectedPaths(new Set());
    }
  };

  const selectedEntries = getActionEntries();
  const selectedEntry = selectedEntries.length === 1 ? selectedEntries[0] : null;
  const selectedEntryDepth =
    selectedEntry ? Math.max(0, findEntryDepth(fileTree, selectedEntry.path)) : 0;
  const hasSelection = selectedEntries.length > 0;
  const canRenameSelection = selectedEntries.length === 1;

  return (
    <div data-testid="file-navigator" className="flex h-full min-h-0 select-none flex-col overflow-hidden">
      <div className="flex h-8 shrink-0 items-center justify-between border-b border-ide-border px-3">
        <div className="min-w-0">
          <div className="truncate text-[10px] font-bold uppercase tracking-widest text-ide-text-dim">
            {getBaseName(currentProjectRoot)}
          </div>
        </div>

        <div className="flex items-center gap-1">
          <ExplorerButton testId="explorer-new-file" title="New File" onClick={() => startDraft("file", currentProjectRoot, 0)}>
            <FilePlus2 size={14} />
          </ExplorerButton>
          <ExplorerButton testId="explorer-new-folder" title="New Folder" onClick={() => startDraft("folder", currentProjectRoot, 0)}>
            <FolderPlus size={14} />
          </ExplorerButton>
          <ExplorerButton testId="explorer-refresh" title="Refresh Explorer" onClick={() => void refreshData()}>
            <RefreshCcw size={13} className={isLoading ? "animate-spin" : ""} />
          </ExplorerButton>
        </div>
      </div>

      {operationError && (
        <div className="mx-2 mt-2 rounded-lg border border-red-500/20 bg-red-500/10 px-2 py-1.5 text-[11px] text-red-500">
          {operationError}
        </div>
      )}

      {clipboard && (
        <div data-testid="explorer-clipboard-status" className="mx-2 mt-2 rounded-lg border border-ide-border bg-ide-panel/70 px-2 py-1.5 text-[10px] text-ide-text-dim">
          {clipboard.mode === "cut" ? "Cut" : "Copied"} {clipboard.entries.length} item
          {clipboard.entries.length === 1 ? "" : "s"}.
        </div>
      )}

      <ContextMenu.Root>
        <ContextMenu.Trigger asChild>
          <div
            className="min-h-0 flex-1 overflow-auto py-2"
            role="tree"
            aria-label="Workspace files"
            onKeyDownCapture={handleExplorerKeyDown}
            onDragOver={(event) => {
              if (dragEntriesRef.current.length === 0) return;
              event.preventDefault();
              setDropTargetPath(currentProjectRoot);
            }}
            onDrop={(event) => {
              if (dragEntriesRef.current.length === 0) return;
              event.preventDefault();
              void moveEntriesInto(dragEntriesRef.current, currentProjectRoot);
              dragEntriesRef.current = [];
              setDropTargetPath(null);
            }}
            onDragLeave={() => {
              setDropTargetPath((current) =>
                current && isSamePath(current, currentProjectRoot) ? null : current
              );
            }}
          >
            {draft && isSamePath(draft.parentPath, currentProjectRoot) && (
              <DraftRow
                key={`${draft.mode}:${draft.parentPath}:root`}
                draft={draft}
                onCommit={commitDraft}
                onCancel={cancelDraft}
              />
            )}

            {fileTree.map((entry) => (
              <FileNode
                key={entry.path}
                entry={entry}
                depth={0}
                activeFile={activeFile}
                expandedPaths={expandedPaths}
                selectedPaths={selectedPaths}
                draft={draft}
                gitStatusMap={gitStatus}
                dropTargetPath={dropTargetPath}
                setDropTargetPath={setDropTargetPath}
                onOpen={openFile}
                onSelectSingle={selectSingle}
                onToggleSelected={toggleSelected}
                onToggleFolder={toggleFolder}
                onCreate={(mode, parentPath, depth) => startDraft(mode, parentPath, depth)}
                onRename={(target, depth) => startDraft("rename", target.path, depth, target)}
                onDelete={deleteEntry}
                onCopy={copySelection}
                onDuplicate={duplicateSelection}
                onPaste={pasteInto}
                onRevealInSystemExplorer={openFolderInSystemExplorer}
                onOpenTerminal={openTerminalAtPath}
                onSearchInFolder={handleSearchInFolder}
                onCommitDraft={commitDraft}
                onCancelDraft={cancelDraft}
                onDropEntries={moveEntriesInto}
                dragEntriesRef={dragEntriesRef}
              />
            ))}

            {!isLoading && fileTree.length === 0 && !draft && (
              <div className="mx-3 mt-8 rounded-2xl border border-dashed border-ide-border bg-ide-panel/50 p-4 text-center">
                <div className="mx-auto mb-3 flex h-10 w-10 items-center justify-center rounded-2xl bg-primary/10 text-primary">
                  <FolderPlus size={18} />
                </div>
                <div className="text-[12px] font-semibold text-ide-text">Empty workspace</div>
                <p className="mt-1 text-[11px] leading-relaxed text-ide-text-dim">
                  Create a file or folder to start writing in this directory.
                </p>
                <div className="mt-4 flex justify-center gap-2">
                  <button
                    type="button"
                    onClick={() => startDraft("file", currentProjectRoot, 0)}
                    className="rounded-lg bg-primary px-3 py-1.5 text-[11px] font-semibold text-white transition-transform active:scale-95"
                  >
                    New File
                  </button>
                  <button
                    type="button"
                    onClick={() => startDraft("folder", currentProjectRoot, 0)}
                    className="rounded-lg border border-ide-border px-3 py-1.5 text-[11px] font-semibold text-ide-text transition-colors hover:bg-black/5"
                  >
                    New Folder
                  </button>
                </div>
              </div>
            )}

            {isLoading && fileTree.length === 0 && (
              <div className="px-4 py-6 text-center text-[11px] text-ide-text-dim">
                Loading workspace...
              </div>
            )}
          </div>
        </ContextMenu.Trigger>

        <ContextMenu.Portal>
          <ContextMenu.Content
            className="z-[200] min-w-[210px] rounded-xl border border-ide-border bg-ide-panel p-1 shadow-2xl"
            onCloseAutoFocus={(event) => event.preventDefault()}
          >
            <ExplorerContextItem onSelect={() => startDraft("file", currentProjectRoot, 0)}>
              New file
            </ExplorerContextItem>
            <ExplorerContextItem onSelect={() => startDraft("folder", currentProjectRoot, 0)}>
              New folder
            </ExplorerContextItem>
            <ExplorerContextItem disabled={!clipboard} onSelect={() => void pasteInto(getPasteDestination())}>
              Paste
            </ExplorerContextItem>
            <ExplorerContextSeparator />
            <ExplorerContextItem disabled={!hasSelection} onSelect={() => copySelection("copy")}>
              Copy
            </ExplorerContextItem>
            <ExplorerContextItem disabled={!hasSelection} onSelect={() => copySelection("cut")}>
              Cut
            </ExplorerContextItem>
            <ExplorerContextItem disabled={!hasSelection} onSelect={() => void duplicateSelection()}>
              Duplicate
            </ExplorerContextItem>
            <ExplorerContextItem
              disabled={!canRenameSelection || !selectedEntry}
              onSelect={() => {
                if (!selectedEntry) return;
                startDraft("rename", selectedEntry.path, selectedEntryDepth, selectedEntry);
              }}
            >
              Rename
            </ExplorerContextItem>
            <ExplorerContextItem danger disabled={!hasSelection} onSelect={() => void deleteEntries(selectedEntries)}>
              Delete
            </ExplorerContextItem>
            <ExplorerContextSeparator />
            <ExplorerContextItem disabled={!activeFile} onSelect={revealActiveFile}>
              Reveal active file
            </ExplorerContextItem>
            <ExplorerContextItem onSelect={() => openTerminalAtPath(currentProjectRoot, "Workspace")}>
              Open terminal here
            </ExplorerContextItem>
            <ExplorerContextItem onSelect={() => void refreshData()}>
              Refresh explorer
            </ExplorerContextItem>
          </ContextMenu.Content>
        </ContextMenu.Portal>
      </ContextMenu.Root>
    </div>
  );
}

function FileNode({
  entry,
  depth,
  activeFile,
  expandedPaths,
  selectedPaths,
  draft,
  gitStatusMap,
  dropTargetPath,
  setDropTargetPath,
  onOpen,
  onSelectSingle,
  onToggleSelected,
  onToggleFolder,
  onCreate,
  onRename,
  onDelete,
  onCopy,
  onDuplicate,
  onPaste,
  onRevealInSystemExplorer,
  onOpenTerminal,
  onSearchInFolder,
  onCommitDraft,
  onCancelDraft,
  onDropEntries,
  dragEntriesRef,
}: {
  entry: FileEntry;
  depth: number;
  activeFile: FileEntry | null;
  expandedPaths: Set<string>;
  selectedPaths: Set<string>;
  draft: DraftState | null;
  gitStatusMap: Record<string, string>;
  dropTargetPath: string | null;
  setDropTargetPath: React.Dispatch<React.SetStateAction<string | null>>;
  onOpen: (file: FileEntry, isFixed?: boolean) => void;
  onSelectSingle: (path: string) => void;
  onToggleSelected: (path: string) => void;
  onToggleFolder: (entry: FileEntry) => void;
  onCreate: (mode: "file" | "folder", parentPath: string, depth: number) => void;
  onRename: (entry: FileEntry, depth: number) => void;
  onDelete: (entry: FileEntry) => Promise<void>;
  onCopy: (mode: ClipboardMode, preferred?: FileEntry) => void;
  onDuplicate: (preferred?: FileEntry) => Promise<void>;
  onPaste: (destinationPath: string) => Promise<void>;
  onRevealInSystemExplorer: (entry: FileEntry) => Promise<void>;
  onOpenTerminal: (path: string, label: string) => void;
  onSearchInFolder: (path: string) => void;
  onCommitDraft: (name: string) => Promise<void>;
  onCancelDraft: () => void;
  onDropEntries: (entries: FileEntry[], destinationPath: string) => Promise<void>;
  dragEntriesRef: React.MutableRefObject<FileEntry[]>;
}) {
  const isOpen = expandedPaths.has(entry.path);
  const isSelected = selectedPaths.has(entry.path);
  const isActiveFile = activeFile?.path === entry.path;
  const isDropTarget = entry.is_directory && dropTargetPath && isSamePath(dropTargetPath, entry.path);
  const currentStatus = gitStatusMap[entry.path];

  const handleOpen = (isFixed = false) => {
    if (entry.is_directory) {
      onToggleFolder(entry);
    } else {
      onOpen(entry, isFixed);
    }
  };

  const handleSelection = (event?: React.MouseEvent<HTMLDivElement>) => {
    if (event?.ctrlKey || event?.metaKey) {
      onToggleSelected(entry.path);
      return;
    }

    onSelectSingle(entry.path);
  };

  const handleRowClick = (event: React.MouseEvent<HTMLDivElement>) => {
    handleSelection(event);
    handleOpen(false);
  };

  const handleMouseDownCapture = (event: React.MouseEvent<HTMLDivElement>) => {
    if (event.button !== 2) return;
    if (!isSelected) {
      onSelectSingle(entry.path);
    }
  };

  const handleKeyDown = (event: React.KeyboardEvent<HTMLDivElement>) => {
    const isModifier = event.ctrlKey || event.metaKey;
    const key = event.key.toLowerCase();

    if (event.key === "Enter" || event.key === " ") {
      event.preventDefault();
      handleOpen(event.key === "Enter");
      return;
    }

    if (event.key === "F2") {
      event.preventDefault();
      onRename(entry, depth);
      return;
    }

    if (event.key === "Delete") {
      event.preventDefault();
      void onDelete(entry);
      return;
    }

    if (isModifier && key === "c") {
      event.preventDefault();
      onCopy("copy", entry);
      return;
    }

    if (isModifier && key === "x") {
      event.preventDefault();
      onCopy("cut", entry);
      return;
    }

    if (isModifier && key === "d") {
      event.preventDefault();
      void onDuplicate(entry);
      return;
    }

    if (isModifier && key === "v") {
      event.preventDefault();
      const destination = entry.is_directory ? entry.path : getParentPath(entry.path, entry.path);
      void onPaste(destination);
      return;
    }

    if (entry.is_directory && event.key === "ArrowRight" && !isOpen) {
      event.preventDefault();
      onToggleFolder(entry);
      return;
    }

    if (entry.is_directory && event.key === "ArrowLeft" && isOpen) {
      event.preventDefault();
      onToggleFolder(entry);
    }
  };

  const getStatusColor = () => {
    if (!currentStatus) return "";
    if (currentStatus.trim() === "M") return "text-sky-600";
    if (currentStatus.includes("?") || currentStatus.includes("A")) return "text-emerald-600";
    return "text-amber-500";
  };

  const rowClassName = [
    "group flex h-6 items-center gap-1 px-2 outline-none transition-colors",
    isActiveFile ? "bg-primary/10 text-primary" : "text-zinc-600 hover:bg-black/5",
    isSelected && !isActiveFile ? "bg-primary/5 text-ide-text" : "",
    isDropTarget ? "ring-1 ring-inset ring-primary/30 bg-primary/5" : "",
  ]
    .filter(Boolean)
    .join(" ");

  return (
    <div className="flex flex-col">
      {draft?.mode === "rename" && draft.target && isSamePath(draft.target.path, entry.path) ? (
        <DraftRow
          key={`${draft.mode}:${entry.path}`}
          draft={draft}
          onCommit={onCommitDraft}
          onCancel={onCancelDraft}
        />
      ) : (
        <ContextMenu.Root>
          <ContextMenu.Trigger asChild>
            <div
              onMouseDownCapture={handleMouseDownCapture}
              onClick={handleRowClick}
              onDoubleClick={() => !entry.is_directory && onOpen(entry, true)}
              onKeyDown={handleKeyDown}
              draggable
              onDragStart={(event) => {
                event.stopPropagation();
                dragEntriesRef.current = isSelected ? [entry] : [entry];
                onSelectSingle(entry.path);
                event.dataTransfer.effectAllowed = "move";
                event.dataTransfer.setData("text/plain", entry.path);
              }}
              onDragOver={(event) => {
                if (!entry.is_directory) return;
                const draggedEntries = dragEntriesRef.current;
                if (draggedEntries.length === 0) return;
                if (draggedEntries.some((dragged) => isSameOrChildPath(entry.path, dragged.path))) {
                  return;
                }
                event.preventDefault();
                event.stopPropagation();
                setDropTargetPath(entry.path);
              }}
              onDragEnter={(event) => {
                if (!entry.is_directory) return;
                const draggedEntries = dragEntriesRef.current;
                if (draggedEntries.length === 0) return;
                if (draggedEntries.some((dragged) => isSameOrChildPath(entry.path, dragged.path))) {
                  return;
                }
                event.preventDefault();
                event.stopPropagation();
                setDropTargetPath(entry.path);
              }}
              onDrop={(event) => {
                if (!entry.is_directory) return;
                const draggedEntries = dragEntriesRef.current;
                if (draggedEntries.length === 0) return;
                if (draggedEntries.some((dragged) => isSameOrChildPath(entry.path, dragged.path))) {
                  return;
                }
                event.preventDefault();
                event.stopPropagation();
                void onDropEntries(draggedEntries, entry.path);
                dragEntriesRef.current = [];
                setDropTargetPath(null);
              }}
              onDragEnd={() => {
                dragEntriesRef.current = [];
                setDropTargetPath(null);
              }}
              className={rowClassName}
              style={{ paddingLeft: `${depth * 12 + 8}px` }}
              data-testid={getExplorerItemTestId(entry.path)}
              role="treeitem"
              aria-expanded={entry.is_directory ? isOpen : undefined}
              aria-selected={isSelected}
              tabIndex={0}
            >
              <div className="flex h-4 w-4 shrink-0 items-center justify-center">
                {entry.is_directory &&
                  (isOpen ? (
                    <ChevronDown size={14} />
                  ) : (
                    <ChevronRight size={14} className="opacity-40 group-hover:opacity-100" />
                  ))}
              </div>

              <div className="shrink-0 text-primary/70">
                {entry.is_directory ? (
                  isOpen ? <FolderOpen size={14} /> : <Folder size={14} />
                ) : entry.name.endsWith(".zt") ? (
                  <FileCode size={14} className="text-primary" />
                ) : (
                  <FileIcon size={14} />
                )}
              </div>

              <span
                className={`min-w-0 flex-1 truncate text-[11px] ${
                  isActiveFile ? "font-medium" : ""
                } ${getStatusColor()}`}
              >
                {entry.name}
              </span>

              <div className="ml-auto hidden shrink-0 items-center gap-0.5 group-hover:flex group-focus-within:flex">
                {entry.is_directory && (
                  <>
                    <NodeActionButton
                      title={`New file in ${entry.name}`}
                      onClick={(event) => {
                        event.stopPropagation();
                        onCreate("file", entry.path, depth + 1);
                      }}
                    >
                      <FilePlus2 size={12} />
                    </NodeActionButton>
                    <NodeActionButton
                      title={`New folder in ${entry.name}`}
                      onClick={(event) => {
                        event.stopPropagation();
                        onCreate("folder", entry.path, depth + 1);
                      }}
                    >
                      <FolderPlus size={12} />
                    </NodeActionButton>
                  </>
                )}
                <NodeActionButton
                  title={`Duplicate ${entry.name}`}
                  onClick={(event) => {
                    event.stopPropagation();
                    void onDuplicate(entry);
                  }}
                >
                  <Files size={12} />
                </NodeActionButton>
                <NodeActionButton
                  title={`Copy ${entry.name}`}
                  onClick={(event) => {
                    event.stopPropagation();
                    onCopy("copy", entry);
                  }}
                >
                  <Copy size={12} />
                </NodeActionButton>
                <NodeActionButton
                  title={`Cut ${entry.name}`}
                  onClick={(event) => {
                    event.stopPropagation();
                    onCopy("cut", entry);
                  }}
                >
                  <Scissors size={12} />
                </NodeActionButton>
                {entry.is_directory && (
                  <NodeActionButton
                    title={`Paste into ${entry.name}`}
                    onClick={(event) => {
                      event.stopPropagation();
                      void onPaste(entry.path);
                    }}
                  >
                    <Clipboard size={12} />
                  </NodeActionButton>
                )}
                <NodeActionButton
                  title={`Rename ${entry.name}`}
                  onClick={(event) => {
                    event.stopPropagation();
                    onRename(entry, depth);
                  }}
                >
                  <Edit3 size={12} />
                </NodeActionButton>
                <NodeActionButton
                  title={`Delete ${entry.name}`}
                  onClick={(event) => {
                    event.stopPropagation();
                    void onDelete(entry);
                  }}
                >
                  <Trash2 size={12} />
                </NodeActionButton>
                {entry.is_directory && (
                  <>
                    <NodeActionButton
                      title={`Open terminal in ${entry.name}`}
                      onClick={(event) => {
                        event.stopPropagation();
                        onOpenTerminal(entry.path, entry.name);
                      }}
                    >
                      <TerminalIcon size={12} />
                    </NodeActionButton>
                    <NodeActionButton
                      title={`Search in ${entry.name}`}
                      onClick={(event) => {
                        event.stopPropagation();
                        onSearchInFolder(entry.path);
                      }}
                    >
                      <Search size={12} />
                    </NodeActionButton>
                  </>
                )}
              </div>
            </div>
          </ContextMenu.Trigger>

          <ContextMenu.Portal>
            <ContextMenu.Content
              className="z-[200] min-w-[190px] rounded-xl border border-ide-border bg-ide-panel p-1 shadow-2xl"
              onCloseAutoFocus={(event) => event.preventDefault()}
            >
              {!entry.is_directory && (
                <ExplorerContextItem onSelect={() => onOpen(entry, true)}>
                  Open
                </ExplorerContextItem>
              )}
              <ExplorerContextItem onSelect={() => onCopy("copy", entry)}>
                Copy
              </ExplorerContextItem>
              <ExplorerContextItem onSelect={() => onCopy("cut", entry)}>
                Cut
              </ExplorerContextItem>
              <ExplorerContextItem onSelect={() => void onDuplicate(entry)}>
                Duplicate
              </ExplorerContextItem>
              {entry.is_directory && (
                <ExplorerContextItem onSelect={() => void onPaste(entry.path)}>
                  Paste into folder
                </ExplorerContextItem>
              )}
              <ExplorerContextSeparator />
              {entry.is_directory && (
                <>
                  <ExplorerContextItem onSelect={() => onCreate("file", entry.path, depth + 1)}>
                    New file
                  </ExplorerContextItem>
                  <ExplorerContextItem onSelect={() => onCreate("folder", entry.path, depth + 1)}>
                    New folder
                  </ExplorerContextItem>
                  <ExplorerContextItem onSelect={() => onOpenTerminal(entry.path, entry.name)}>
                    Open terminal here
                  </ExplorerContextItem>
                  <ExplorerContextItem onSelect={() => onSearchInFolder(entry.path)}>
                    Search in folder
                  </ExplorerContextItem>
                </>
              )}
              <ExplorerContextItem onSelect={() => onRename(entry, depth)}>
                Rename
              </ExplorerContextItem>
              <ExplorerContextItem onSelect={() => void onRevealInSystemExplorer(entry)}>
                {entry.is_directory ? "Open folder in file explorer" : "Open containing folder in file explorer"}
              </ExplorerContextItem>
              <ExplorerContextItem danger onSelect={() => void onDelete(entry)}>
                Delete
              </ExplorerContextItem>
            </ContextMenu.Content>
          </ContextMenu.Portal>
        </ContextMenu.Root>
      )}

      {entry.is_directory && isOpen && (
        <div className="flex flex-col" role="group">
          {draft && draft.mode !== "rename" && isSamePath(draft.parentPath, entry.path) && (
            <DraftRow
              key={`${draft.mode}:${entry.path}`}
              draft={draft}
              onCommit={onCommitDraft}
              onCancel={onCancelDraft}
            />
          )}

          {(entry.children || []).map((child) => (
            <FileNode
              key={child.path}
              entry={child}
              depth={depth + 1}
              activeFile={activeFile}
              expandedPaths={expandedPaths}
              selectedPaths={selectedPaths}
              draft={draft}
              gitStatusMap={gitStatusMap}
              dropTargetPath={dropTargetPath}
              setDropTargetPath={setDropTargetPath}
              onOpen={onOpen}
              onSelectSingle={onSelectSingle}
              onToggleSelected={onToggleSelected}
              onToggleFolder={onToggleFolder}
              onCreate={onCreate}
              onRename={onRename}
              onDelete={onDelete}
              onCopy={onCopy}
              onDuplicate={onDuplicate}
              onPaste={onPaste}
              onRevealInSystemExplorer={onRevealInSystemExplorer}
              onOpenTerminal={onOpenTerminal}
              onSearchInFolder={onSearchInFolder}
              onCommitDraft={onCommitDraft}
              onCancelDraft={onCancelDraft}
              onDropEntries={onDropEntries}
              dragEntriesRef={dragEntriesRef}
            />
          ))}

          {(entry.children || []).length === 0 && !draft && (
            <div
              className="h-6 truncate px-2 text-[11px] italic text-ide-text-dim/70"
              style={{ paddingLeft: `${(depth + 1) * 12 + 28}px` }}
            >
              Empty folder
            </div>
          )}
        </div>
      )}
    </div>
  );
}

function DraftRow({
  draft,
  onCommit,
  onCancel,
}: {
  draft: DraftState;
  onCommit: (name: string) => Promise<void>;
  onCancel: () => void;
}) {
  const [value, setValue] = useState(draft.initialName);
  const [isSubmitting, setIsSubmitting] = useState(false);
  const inputRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    inputRef.current?.focus();
    inputRef.current?.select();
  }, []);

  const submit = async () => {
    if (isSubmitting) return;

    setIsSubmitting(true);
    await onCommit(value);
    setIsSubmitting(false);
  };

  return (
    <form
      className="flex h-7 items-center gap-1 px-2"
      style={{ paddingLeft: `${draft.depth * 12 + 28}px` }}
      onContextMenuCapture={(event) => event.stopPropagation()}
      onSubmit={(event) => {
        event.preventDefault();
        void submit();
      }}
    >
      <div className="shrink-0 text-primary/70">
        {draft.mode === "folder" ? <Folder size={14} /> : <FileIcon size={14} />}
      </div>
      <input
        ref={inputRef}
        value={value}
        disabled={isSubmitting}
        onChange={(event) => setValue(event.target.value)}
        onBlur={() => {
          if (!isSubmitting) onCancel();
        }}
        onKeyDown={(event) => {
          if (event.key === "Escape") {
            event.preventDefault();
            onCancel();
          }
        }}
        className="h-5 min-w-0 flex-1 rounded border border-primary/40 bg-ide-bg px-1.5 text-[11px] text-ide-text outline-none ring-2 ring-primary/10"
        aria-label={
          draft.mode === "rename"
            ? "Rename item"
            : draft.mode === "folder"
              ? "New folder name"
              : "New file name"
        }
      />
      <button
        type="button"
        onMouseDown={(event) => event.preventDefault()}
        onClick={onCancel}
        className="flex h-5 w-5 items-center justify-center rounded text-ide-text-dim hover:bg-black/5 hover:text-ide-text"
        title="Cancel"
      >
        <X size={12} />
      </button>
    </form>
  );
}

function ExplorerButton({
  title,
  onClick,
  disabled,
  testId,
  children,
}: {
  title: string;
  onClick: () => void;
  disabled?: boolean;
  testId?: string;
  children: React.ReactNode;
}) {
  return (
    <button
      type="button"
      data-testid={testId}
      title={title}
      aria-label={title}
      disabled={disabled}
      onClick={onClick}
      className="flex h-6 w-6 items-center justify-center rounded-md text-ide-text-dim transition-colors hover:bg-black/5 hover:text-ide-text active:scale-95 disabled:cursor-not-allowed disabled:opacity-35"
    >
      {children}
    </button>
  );
}

function NodeActionButton({
  title,
  onClick,
  children,
}: {
  title: string;
  onClick: React.MouseEventHandler<HTMLButtonElement>;
  children: React.ReactNode;
}) {
  return (
    <button
      type="button"
      title={title}
      aria-label={title}
      onClick={onClick}
      className="flex h-5 w-5 items-center justify-center rounded text-ide-text-dim transition-colors hover:bg-black/10 hover:text-ide-text active:scale-95"
    >
      {children}
    </button>
  );
}

function ExplorerContextItem({
  children,
  danger,
  disabled,
  onSelect,
}: {
  children: React.ReactNode;
  danger?: boolean;
  disabled?: boolean;
  onSelect: () => void;
}) {
  return (
    <ContextMenu.Item
      disabled={disabled}
      onSelect={onSelect}
      className={`flex cursor-pointer items-center rounded-lg px-3 py-2 text-[11px] outline-none transition-colors ${
        disabled
          ? "cursor-default text-ide-text-dim opacity-45"
          : danger
            ? "text-red-500 focus:bg-red-500/10"
            : "text-ide-text focus:bg-primary/10 focus:text-primary"
      }`}
    >
      {children}
    </ContextMenu.Item>
  );
}

function ExplorerContextSeparator() {
  return <ContextMenu.Separator className="my-1 h-px bg-ide-border" />;
}
