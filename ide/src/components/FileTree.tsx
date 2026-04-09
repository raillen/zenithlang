import { useEffect, useState } from 'react';
import {
  CaretDown,
  CaretRight,
  Circle,
  ClockCounterClockwise,
  File,
  FileCode,
  Folder,
  FolderOpen,
  WarningCircle,
  X,
  ArrowsClockwise,
} from '@phosphor-icons/react';
import { clsx, type ClassValue } from 'clsx';
import { twMerge } from 'tailwind-merge';
import { ContextMenuWrapper } from './ContextMenuWrapper';
import { safeInvoke as invoke } from '../utils/tauri';
import { useProjectStore, type FileEntry } from '../store/useProjectStore';
import { uiMarker } from '../utils/debugSelectors';

function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs));
}

const fallbackTree: FileEntry[] = [
  {
    name: 'src',
    path: 'src',
    is_directory: true,
    children: [{ name: 'main.zt', path: 'src/main.zt', is_directory: false }],
  },
  { name: 'zenith.ztproj', path: 'zenith.ztproj', is_directory: false },
];

function findAncestorPaths(entries: FileEntry[], targetPath: string, ancestors: string[] = []): string[] | null {
  for (const entry of entries) {
    if (entry.path === targetPath) {
      return ancestors;
    }

    if (entry.is_directory && entry.children?.length) {
      const result = findAncestorPaths(entry.children, targetPath, [...ancestors, entry.path]);
      if (result) {
        return result;
      }
    }
  }

  return null;
}

function buildInitialExpandedState(entries: FileEntry[], activeFilePath: string | null) {
  const nextState: Record<string, boolean> = {};

  entries.forEach((entry) => {
    if (entry.is_directory) {
      nextState[entry.path] = true;
    }
  });

  if (activeFilePath) {
    const ancestors = findAncestorPaths(entries, activeFilePath) ?? [];
    ancestors.forEach((path) => {
      nextState[path] = true;
    });
  }

  return nextState;
}

export function FileTree() {
  const { workspaceRoot, fileTree, setFileTree, openFile, activeFilePath, recentFiles, openFiles, setActiveFile, closeFile, focusMode } =
    useProjectStore();
  const [expandedPaths, setExpandedPaths] = useState<Record<string, boolean>>({});
  const [isLoading, setIsLoading] = useState(true);
  const [errorMessage, setErrorMessage] = useState<string | null>(null);
  const [reloadKey, setReloadKey] = useState(0);

  useEffect(() => {
    let cancelled = false;

    const loadFiles = async () => {
      setIsLoading(true);
      setErrorMessage(null);

      try {
        const tree = await invoke<FileEntry[]>('get_file_tree', { rootPath: workspaceRoot });
        if (cancelled) {
          return;
        }

        setFileTree(tree);
        setExpandedPaths((previous) => ({
          ...buildInitialExpandedState(tree, activeFilePath),
          ...previous,
        }));
      } catch (error) {
        if (cancelled) {
          return;
        }

        const message = error instanceof Error ? error.message : String(error);
        console.error('Failed to load file tree:', error);
        setErrorMessage(message);
        setFileTree(fallbackTree);
        setExpandedPaths(buildInitialExpandedState(fallbackTree, activeFilePath));
      } finally {
        if (!cancelled) {
          setIsLoading(false);
        }
      }
    };

    loadFiles();

    return () => {
      cancelled = true;
    };
  }, [reloadKey, setFileTree, workspaceRoot]);

  useEffect(() => {
    if (!activeFilePath || fileTree.length === 0) {
      return;
    }

    const ancestors = findAncestorPaths(fileTree, activeFilePath);
    if (!ancestors?.length) {
      return;
    }

    setExpandedPaths((previous) => {
      const nextState = { ...previous };
      ancestors.forEach((path) => {
        nextState[path] = true;
      });
      return nextState;
    });
  }, [activeFilePath, fileTree]);

  const handleToggle = (path: string) => {
    setExpandedPaths((previous) => ({
      ...previous,
      [path]: !previous[path],
    }));
  };

  return (
    <div className="flex h-full min-h-0 flex-col select-none" {...uiMarker('file-tree', { workspaceRoot, loading: isLoading })}>
      <div className="flex items-center justify-between px-4 py-3" {...uiMarker('file-tree-header')}>
        <div>
          <div className="text-[12px] font-medium" style={{ color: 'var(--text-primary)' }} {...uiMarker('file-tree-title')}>
            Explorer
          </div>
          {!focusMode ? (
            <div className="mt-1 text-[11px]" style={{ color: 'var(--text-tertiary)' }} {...uiMarker('file-tree-summary')}>
              {isLoading ? 'Loading workspace...' : `${fileTree.length} top-level items`}
            </div>
          ) : null}
        </div>

        <button
          type="button"
          onClick={() => setReloadKey((value) => value + 1)}
          className="ide-button flex h-8 w-8 items-center justify-center rounded-full active:scale-[0.98]"
          title="Reload tree"
          aria-label="Reload tree"
          {...uiMarker('file-tree-reload', { loading: isLoading })}
        >
          <ArrowsClockwise size={15} className={isLoading ? 'animate-spin' : ''} />
        </button>
      </div>

      {errorMessage ? (
        <div
          className="mx-3 mb-3 rounded-[1.25rem] border px-3 py-2 text-[12px]"
          style={{
            borderColor: 'color-mix(in srgb, var(--danger) 20%, var(--border))',
            background: 'color-mix(in srgb, var(--danger) 10%, white 90%)',
            color: 'color-mix(in srgb, var(--danger) 76%, black 24%)',
          }}
          {...uiMarker('file-tree-error')}
        >
          <div className="flex items-center gap-2 font-medium" {...uiMarker('file-tree-error-title')}>
            <WarningCircle size={14} weight="fill" />
            Unable to fully load the workspace
          </div>
          <div className="mt-1 break-words opacity-80" {...uiMarker('file-tree-error-message')}>{errorMessage}</div>
        </div>
      ) : null}

      <div className="min-h-0 flex-1 overflow-y-auto px-2 pb-3" {...uiMarker('file-tree-content')}>
        {isLoading ? (
          <div className="space-y-2 px-2 pt-1" {...uiMarker('file-tree-loading-state')}>
            {Array.from({ length: 7 }).map((_, index) => (
              <div
                key={index}
                className="h-9 animate-pulse rounded-xl border"
                style={{
                  borderColor: 'var(--border)',
                  background: 'color-mix(in srgb, var(--ide-panel) 68%, white 32%)',
                }}
                {...uiMarker('file-tree-loading-row', { index })}
              />
            ))}
          </div>
        ) : fileTree.length === 0 ? (
          <div className="ide-soft-surface mx-2 rounded-[1.25rem] px-4 py-5 text-sm" style={{ color: 'var(--text-secondary)' }} {...uiMarker('file-tree-empty-state')}>
            No files were found in the current workspace.
          </div>
        ) : (
          <div className="space-y-4" {...uiMarker('file-tree-sections')}>
            <OpenEditorsSection
              openFiles={openFiles}
              activeFilePath={activeFilePath}
              onActivateFile={setActiveFile}
              onCloseFile={closeFile}
            />

            <RecentFilesSection recentFiles={recentFiles} activeFilePath={activeFilePath} onFileClick={openFile} />

            <SectionLabel label="Folders" />

            <div className="space-y-1">
              {fileTree.map((entry) => (
                <FileTreeNode
                  key={entry.path}
                  entry={entry}
                  depth={0}
                  isExpanded={!!expandedPaths[entry.path]}
                  onToggle={handleToggle}
                  onFileClick={openFile}
                  activeFilePath={activeFilePath}
                  expandedPaths={expandedPaths}
                />
              ))}
            </div>
          </div>
        )}
      </div>
    </div>
  );
}

function SectionLabel({ label, icon }: { label: string; icon?: React.ReactNode }) {
  return (
    <div className="flex items-center gap-2 px-2" {...uiMarker('file-tree-section-label', { label: label.toLowerCase().replace(/\s+/g, '-') })}>
      {icon}
      <div className="text-[12px] font-medium" style={{ color: 'var(--text-tertiary)' }}>
        {label}
      </div>
    </div>
  );
}

function OpenEditorsSection({
  openFiles,
  activeFilePath,
  onActivateFile,
  onCloseFile,
}: {
  openFiles: string[];
  activeFilePath: string | null;
  onActivateFile: (path: string) => void;
  onCloseFile: (path: string) => void;
}) {
  const { focusMode } = useProjectStore();

  return (
    <div {...uiMarker('file-tree-open-editors', { count: openFiles.length })}>
      <SectionLabel label="Open editors" />

      {openFiles.length === 0 ? (
        !focusMode ? (
          <div className="px-3 py-3 text-[12px] leading-5" style={{ color: 'var(--text-tertiary)' }} {...uiMarker('file-tree-open-editors-empty')}>
            Open files show up here for quick focus and close actions.
          </div>
        ) : null
      ) : (
        <div className="mt-1 space-y-1" {...uiMarker('file-tree-open-editors-list')}>
          {openFiles.map((path) => {
            const fileName = path.split(/[/\\]/).pop() || path;
            const isActive = activeFilePath === path;

            return (
              <div
                key={path}
                className="group flex items-center gap-2 rounded-xl px-2 py-2 text-sm transition"
                style={{
                  background: isActive ? 'var(--accent-soft)' : 'transparent',
                  color: isActive ? 'var(--text-primary)' : 'var(--text-secondary)',
                }}
                {...uiMarker('file-tree-open-editor-item', { filePath: path, active: isActive })}
              >
                <Circle size={10} weight={isActive ? 'fill' : 'regular'} style={{ color: isActive ? 'var(--accent)' : 'var(--text-quaternary)' }} />
                <button type="button" onClick={() => onActivateFile(path)} className="min-w-0 flex-1 truncate text-left" {...uiMarker('file-tree-open-editor-activate', { filePath: path })}>
                  {fileName}
                </button>
                <button
                  type="button"
                  onClick={() => onCloseFile(path)}
                  className={`transition ${focusMode ? 'opacity-100' : 'opacity-0 group-hover:opacity-100'}`}
                  style={{ color: 'var(--text-tertiary)' }}
                  aria-label={`Close ${fileName}`}
                  {...uiMarker('file-tree-open-editor-close', { filePath: path })}
                >
                  <X size={12} weight="bold" />
                </button>
              </div>
            );
          })}
        </div>
      )}
    </div>
  );
}

function RecentFilesSection({
  recentFiles,
  activeFilePath,
  onFileClick,
}: {
  recentFiles: string[];
  activeFilePath: string | null;
  onFileClick: (path: string) => void;
}) {
  return (
    <div {...uiMarker('file-tree-recent-files', { count: recentFiles.length })}>
      <SectionLabel label="Recent files" icon={<ClockCounterClockwise size={14} style={{ color: 'var(--text-tertiary)' }} />} />

      {recentFiles.length === 0 ? null : (
        <div className="mt-1 space-y-1" {...uiMarker('file-tree-recent-files-list')}>
          {recentFiles.map((path) => {
            const fileName = path.split(/[/\\]/).pop() || path;
            const isActive = activeFilePath === path;

            return (
              <button
                key={path}
                type="button"
                onClick={() => onFileClick(path)}
                className={cn('flex w-full items-start gap-3 rounded-xl px-3 py-2 text-left transition active:scale-[0.99]')}
                style={{
                  background: isActive ? 'var(--accent-soft)' : 'transparent',
                  color: isActive ? 'var(--text-primary)' : 'var(--text-secondary)',
                }}
                {...uiMarker('file-tree-recent-file', { filePath: path, active: isActive })}
              >
                <span className="mt-0.5 flex h-4 w-4 shrink-0 items-center justify-center">
                  {fileName.endsWith('.zt') ? (
                    <FileCode size={15} style={{ color: 'var(--accent)' }} />
                  ) : (
                    <File size={15} style={{ color: 'var(--text-tertiary)' }} />
                  )}
                </span>

                <span className="min-w-0 flex-1">
                  <span className="block truncate text-sm">{fileName}</span>
                  <span className="block truncate text-[11px]" style={{ color: 'var(--text-tertiary)' }}>
                    {path}
                  </span>
                </span>
              </button>
            );
          })}
        </div>
      )}
    </div>
  );
}

interface FileTreeNodeProps {
  entry: FileEntry;
  depth: number;
  isExpanded: boolean;
  onToggle: (path: string) => void;
  onFileClick: (path: string) => void;
  activeFilePath: string | null;
  expandedPaths: Record<string, boolean>;
}

function FileTreeNode({
  entry,
  depth,
  isExpanded,
  onToggle,
  onFileClick,
  activeFilePath,
  expandedPaths,
}: FileTreeNodeProps) {
  const isActive = activeFilePath === entry.path;
  const hasChildren = !!entry.children?.length;

  const handleClick = () => {
    if (entry.is_directory) {
      onToggle(entry.path);
      return;
    }

    onFileClick(entry.path);
  };

  return (
    <div className="flex flex-col gap-0.5" {...uiMarker('file-tree-node-shell', { path: entry.path, directory: entry.is_directory, depth, expanded: isExpanded, active: isActive })}>
      <ContextMenuWrapper onAction={(action) => console.log(`Action: ${action} on ${entry.path}`)}>
        <button
          type="button"
          aria-expanded={entry.is_directory ? isExpanded : undefined}
          onClick={handleClick}
          className={cn('flex w-full items-center gap-2 rounded-xl border border-transparent text-left text-sm transition-colors')}
          style={{
            borderColor: isActive ? 'color-mix(in srgb, var(--accent) 22%, var(--border))' : 'transparent',
            background: isActive ? 'color-mix(in srgb, var(--accent) 12%, white 88%)' : 'transparent',
            color: isActive ? 'var(--text-primary)' : 'var(--text-secondary)',
            paddingLeft: `${depth * 14 + 10}px`,
            paddingTop: 'var(--sidebar-row-padding-y)',
            paddingBottom: 'var(--sidebar-row-padding-y)',
            paddingRight: '0.6rem',
          }}
          {...uiMarker('file-tree-node', { path: entry.path, kind: entry.is_directory ? 'directory' : 'file', depth, expanded: isExpanded, active: isActive })}
        >
          <span className="flex h-4 w-4 items-center justify-center" style={{ color: 'var(--text-tertiary)' }}>
            {entry.is_directory ? (
              hasChildren ? (
                isExpanded ? (
                  <CaretDown size={12} weight="bold" />
                ) : (
                  <CaretRight size={12} weight="bold" />
                )
              ) : (
                <span className="h-3 w-3" />
              )
            ) : (
              <span className="h-3 w-3" />
            )}
          </span>

          <span className="flex h-4 w-4 items-center justify-center">
            {entry.is_directory ? (
              isExpanded ? (
                <FolderOpen size={15} style={{ color: 'var(--accent)' }} />
              ) : (
                <Folder size={15} style={{ color: 'var(--text-secondary)' }} />
              )
            ) : entry.name.endsWith('.zt') ? (
              <FileCode size={15} style={{ color: 'var(--accent)' }} />
            ) : (
              <File size={15} style={{ color: 'var(--text-tertiary)' }} />
            )}
          </span>

          <span className="min-w-0 flex-1 truncate">{entry.name}</span>
        </button>
      </ContextMenuWrapper>

      {entry.is_directory && isExpanded && hasChildren ? (
        <div className="flex flex-col gap-0.5" {...uiMarker('file-tree-children', { path: entry.path })}>
          {entry.children!.map((child) => (
            <FileTreeNode
              key={child.path}
              entry={child}
              depth={depth + 1}
              isExpanded={!!expandedPaths[child.path]}
              onToggle={onToggle}
              onFileClick={onFileClick}
              activeFilePath={activeFilePath}
              expandedPaths={expandedPaths}
            />
          ))}
        </div>
      ) : null}
    </div>
  );
}
