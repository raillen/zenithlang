import { useMemo, useState } from 'react';
import {
  AppWindow,
  BugBeetle,
  CaretRight,
  Circle,
  GitBranch,
  MagnifyingGlass,
} from '@phosphor-icons/react';
import { FileTree } from './FileTree';
import { type FileEntry, type SidebarView, useProjectStore } from '../store/useProjectStore';
import { uiMarker } from '../utils/debugSelectors';

interface SidebarPaneProps {
  activeView: SidebarView;
}

function flattenFiles(entries: FileEntry[]): Array<{ name: string; path: string }> {
  const files: Array<{ name: string; path: string }> = [];

  const visit = (items: FileEntry[]) => {
    items.forEach((item) => {
      if (item.is_directory) {
        visit(item.children ?? []);
      } else {
        files.push({ name: item.name, path: item.path });
      }
    });
  };

  visit(entries);
  return files;
}

export function SidebarPane({ activeView }: SidebarPaneProps) {
  const { fileTree, openFile, buildLogs, isBuilding, activeFilePath, dirtyFiles } = useProjectStore();
  const [query, setQuery] = useState('');

  const files = useMemo(() => flattenFiles(fileTree), [fileTree]);
  const filteredFiles = useMemo(() => {
    const normalizedQuery = query.trim().toLowerCase();
    if (!normalizedQuery) {
      return files.slice(0, 80);
    }

    return files.filter((file) => {
      const name = file.name.toLowerCase();
      const path = file.path.toLowerCase();
      return name.includes(normalizedQuery) || path.includes(normalizedQuery);
    });
  }, [files, query]);

  if (activeView === 'explorer') {
    return <FileTree />;
  }

  if (activeView === 'search') {
    return (
      <div className="flex h-full min-h-0 flex-col" {...uiMarker('sidebar-pane', { view: activeView })}>
        <PaneHeader icon={MagnifyingGlass} label="Search" title="Find files" description="Jump across the workspace by name or path." />

        <div className="border-b px-3 py-3" style={{ borderColor: 'var(--border)' }} {...uiMarker('sidebar-search-controls')}>
          <input
            type="text"
            value={query}
            onChange={(event) => setQuery(event.target.value)}
            placeholder="Search files"
            className="ide-input h-10 w-full rounded-xl px-3 text-sm transition"
            {...uiMarker('sidebar-search-input', { queryLength: query.length })}
          />
        </div>

        <div className="min-h-0 flex-1 overflow-y-auto px-2 py-2" {...uiMarker('sidebar-search-results', { count: filteredFiles.length })}>
          {filteredFiles.length === 0 ? (
            <PaneEmptyState title="No files matched" description="Try another name, extension or path fragment." />
          ) : (
            <div className="space-y-1">
              {filteredFiles.map((file) => (
                <button
                  key={file.path}
                  type="button"
                  onClick={() => openFile(file.path)}
                  className="flex w-full items-start gap-3 rounded-xl px-3 py-2.5 text-left text-sm transition"
                  style={{ color: 'var(--text-secondary)' }}
                  {...uiMarker('sidebar-search-result', { filePath: file.path })}
                >
                  <MagnifyingGlass size={15} style={{ color: 'var(--text-quaternary)' }} />
                  <div className="min-w-0">
                    <div className="truncate">{file.name}</div>
                    <div className="truncate text-[11px]" style={{ color: 'var(--text-quaternary)' }}>
                      {file.path}
                    </div>
                  </div>
                </button>
              ))}
            </div>
          )}
        </div>
      </div>
    );
  }

  if (activeView === 'sourceControl') {
    return (
      <div className="flex h-full min-h-0 flex-col" {...uiMarker('sidebar-pane', { view: activeView })}>
        <PaneHeader
          icon={GitBranch}
          label="Source control"
          title="Repository"
          description="A quieter shell for staged changes, branch info and diffs."
        />

        <div className="min-h-0 flex-1 overflow-y-auto px-3 py-3" {...uiMarker('sidebar-source-control-content')}>
          <PaneSection title="Working tree">
            <PaneEmptyState
              title="SCM integration pending"
              description="The layout is ready, but file-level git status and commit actions still need backend integration."
            />
          </PaneSection>

          <PaneSection title="Unsaved context">
            <div className="space-y-1">
              {(activeFilePath ? [activeFilePath, ...dirtyFiles.filter((file) => file !== activeFilePath)] : dirtyFiles)
                .slice(0, 6)
                .map((path) => (
                  <div key={path} className="flex items-center gap-3 px-3 py-2 text-sm" style={{ color: 'var(--text-secondary)' }} {...uiMarker('sidebar-unsaved-item', { filePath: path })}>
                    <Circle size={10} weight="fill" style={{ color: 'var(--accent)' }} />
                    <div className="min-w-0">
                      <div className="truncate">{path.split(/[/\\]/).pop()}</div>
                      <div className="truncate text-[11px]" style={{ color: 'var(--text-quaternary)' }}>
                        {path}
                      </div>
                    </div>
                  </div>
                ))}
            </div>
          </PaneSection>
        </div>
      </div>
    );
  }

  if (activeView === 'run') {
    return (
      <div className="flex h-full min-h-0 flex-col" {...uiMarker('sidebar-pane', { view: activeView })}>
        <PaneHeader icon={BugBeetle} label="Run" title="Build pipeline" description="Run the compiler from the toolbar and inspect output here." />

        <div className="min-h-0 flex-1 overflow-y-auto px-3 py-3" {...uiMarker('sidebar-run-content')}>
          <PaneSection title="Status">
            <div className="flex items-center gap-3 px-3 py-2 text-sm" style={{ color: 'var(--text-secondary)' }} {...uiMarker('sidebar-run-status', { building: isBuilding })}>
              <span className={`h-2.5 w-2.5 rounded-full ${isBuilding ? 'animate-pulse' : ''}`} style={{ background: 'var(--accent)' }} />
              <span>{isBuilding ? 'Build in progress' : 'Ready to build'}</span>
            </div>
          </PaneSection>

          <PaneSection title="Last output">
            <div className="rounded-2xl p-3 text-[12px] leading-5" style={{ color: 'var(--text-secondary)' }} {...uiMarker('sidebar-run-output')}>
              <pre className="whitespace-pre-wrap break-words font-mono" {...uiMarker('sidebar-run-output-text')}>
                {buildLogs || 'No build output yet. Run the active file to populate this pane.'}
              </pre>
            </div>
          </PaneSection>
        </div>
      </div>
    );
  }

  return (
    <div className="flex h-full min-h-0 flex-col" {...uiMarker('sidebar-pane', { view: activeView })}>
      <PaneHeader
        icon={AppWindow}
        label="Tools"
        title="Extensions"
        description="A future surface for language tools and project capabilities."
      />

      <div className="min-h-0 flex-1 overflow-y-auto px-3 py-3" {...uiMarker('sidebar-extensions-content')}>
        <PaneEmptyState
          title="Extension host not connected"
          description="Install, search and enable flows still need a proper extension system."
        />
      </div>
    </div>
  );
}

function PaneHeader({
  icon: Icon,
  label,
  title,
  description,
}: {
  icon: typeof MagnifyingGlass;
  label: string;
  title: string;
  description: string;
}) {
  return (
    <div className="border-b px-4 py-4" style={{ borderColor: 'var(--border)' }} {...uiMarker('sidebar-pane-header', { label: label.toLowerCase().replace(/\s+/g, '-') })}>
      <div className="flex items-center gap-2 text-[12px]" style={{ color: 'var(--text-tertiary)' }}>
        <Icon size={14} />
        <span>{label}</span>
      </div>
      <div className="mt-2 text-[15px] font-medium" style={{ color: 'var(--text-primary)' }}>
        {title}
      </div>
      <div className="mt-1 text-[12px] leading-5" style={{ color: 'var(--text-secondary)' }}>
        {description}
      </div>
    </div>
  );
}

function PaneSection({ title, children }: { title: string; children: React.ReactNode }) {
  return (
    <section className="mb-4" {...uiMarker('sidebar-pane-section', { title: title.toLowerCase().replace(/\s+/g, '-') })}>
      <div className="mb-2 flex items-center gap-2 px-1 text-[12px]" style={{ color: 'var(--text-tertiary)' }} {...uiMarker('sidebar-pane-section-label')}>
        <CaretRight size={12} />
        <span>{title}</span>
      </div>
      <div className="ide-soft-surface rounded-[1.25rem]" {...uiMarker('sidebar-pane-section-body')}>{children}</div>
    </section>
  );
}

function PaneEmptyState({ title, description }: { title: string; description: string }) {
  return (
    <div className="px-3 py-4" {...uiMarker('sidebar-empty-state')}>
      <div className="text-sm" style={{ color: 'var(--text-primary)' }} {...uiMarker('sidebar-empty-state-title')}>
        {title}
      </div>
      <div className="mt-1 text-[12px] leading-5" style={{ color: 'var(--text-secondary)' }} {...uiMarker('sidebar-empty-state-description')}>
        {description}
      </div>
    </div>
  );
}
