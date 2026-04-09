import { useEffect, useRef, useState } from 'react';
import {
  BookOpenText,
  FloppyDisk,
  FolderOpen,
  Keyboard,
  Play,
  SidebarSimple,
} from '@phosphor-icons/react';
import { useProjectStore } from '../store/useProjectStore';
import { uiMarker } from '../utils/debugSelectors';

interface TopToolbarProps {
  activeFilePath: string | null;
  isMarkdown: boolean;
  isBuilding: boolean;
  isPanelOpen: boolean;
  isDirty: boolean;
  onRunBuild: () => void;
  onTogglePanel: () => void;
}

export function TopToolbar({
  activeFilePath,
  isMarkdown,
  isBuilding,
  isPanelOpen,
  isDirty,
  onRunBuild,
  onTogglePanel,
}: TopToolbarProps) {
  const { focusMode } = useProjectStore();
  const fileName = activeFilePath?.split(/[/\\]/).pop() ?? null;
  const parentName = activeFilePath?.split(/[/\\]/).filter(Boolean).slice(-2, -1)[0] ?? null;
  const [isFileMenuOpen, setIsFileMenuOpen] = useState(false);
  const fileMenuRef = useRef<HTMLDivElement | null>(null);

  useEffect(() => {
    const handlePointerDown = (event: MouseEvent) => {
      if (!fileMenuRef.current?.contains(event.target as Node)) {
        setIsFileMenuOpen(false);
      }
    };

    const handleShortcuts = (event: KeyboardEvent) => {
      const withMeta = event.ctrlKey || event.metaKey;
      if (!withMeta) {
        return;
      }

      const key = event.key.toLowerCase();
      if (key === 'o' && !event.shiftKey) {
        event.preventDefault();
        window.dispatchEvent(new CustomEvent('ide:open-file-dialog'));
      } else if (key === 'o' && event.shiftKey) {
        event.preventDefault();
        window.dispatchEvent(new CustomEvent('ide:open-folder-dialog'));
      } else if (key === 's' && event.shiftKey) {
        event.preventDefault();
        window.dispatchEvent(new CustomEvent('ide:save-active-document-as'));
      }
    };

    window.addEventListener('mousedown', handlePointerDown);
    window.addEventListener('keydown', handleShortcuts);
    return () => {
      window.removeEventListener('mousedown', handlePointerDown);
      window.removeEventListener('keydown', handleShortcuts);
    };
  }, []);

  const runMenuAction = (action: 'open-folder' | 'open-file' | 'save' | 'save-as') => {
    if (action === 'open-folder') {
      window.dispatchEvent(new CustomEvent('ide:open-folder-dialog'));
    } else if (action === 'open-file') {
      window.dispatchEvent(new CustomEvent('ide:open-file-dialog'));
    } else if (action === 'save') {
      window.dispatchEvent(new CustomEvent('ide:save-active-document'));
    } else {
      window.dispatchEvent(new CustomEvent('ide:save-active-document-as'));
    }

    setIsFileMenuOpen(false);
  };

  return (
    <div className="ide-toolbar-surface flex h-12 shrink-0 items-center justify-between border-b px-4" {...uiMarker('top-toolbar', { activeFile: activeFilePath ?? 'none' })}>
      <div className="flex min-w-0 items-center gap-3" {...uiMarker('top-toolbar-left')}>
        <div ref={fileMenuRef} className="relative" {...uiMarker('top-toolbar-file-menu-shell', { open: isFileMenuOpen })}>
          <button
            type="button"
            onClick={() => setIsFileMenuOpen((open) => !open)}
            className="ide-button flex items-center gap-2 rounded-full px-3 py-1.5 text-[12px]"
            aria-label="Open file menu"
            aria-expanded={isFileMenuOpen}
            {...uiMarker('top-toolbar-action', { action: 'file-menu', open: isFileMenuOpen })}
          >
            <FolderOpen size={14} weight="regular" />
            <span>File</span>
          </button>

          {isFileMenuOpen ? (
            <div
              className="absolute left-0 top-[calc(100%+0.45rem)] z-30 min-w-[220px] rounded-2xl border py-1"
              style={{
                borderColor: 'var(--border)',
                background: 'color-mix(in srgb, var(--ide-panel) 90%, white 10%)',
                boxShadow: 'var(--panel-shadow-strong)',
                backdropFilter: 'blur(22px)',
              }}
              {...uiMarker('top-toolbar-file-menu')}
            >
              <MenuItem
                label="Open Folder..."
                shortcut="Ctrl/Cmd+Shift+O"
                onClick={() => runMenuAction('open-folder')}
                debugId="open-folder"
              />
              <MenuItem
                label="Open File..."
                shortcut="Ctrl/Cmd+O"
                onClick={() => runMenuAction('open-file')}
                debugId="open-file"
              />
              <div className="mx-2 my-1 h-px" style={{ background: 'var(--border)' }} {...uiMarker('top-toolbar-file-menu-divider')} />
              <MenuItem
                label="Save"
                shortcut="Ctrl/Cmd+S"
                onClick={() => runMenuAction('save')}
                disabled={!activeFilePath}
                debugId="save"
              />
              <MenuItem
                label="Save As..."
                shortcut="Ctrl/Cmd+Shift+S"
                onClick={() => runMenuAction('save-as')}
                disabled={!activeFilePath}
                debugId="save-as"
              />
            </div>
          ) : null}
        </div>

        <button
          type="button"
          onClick={() => window.dispatchEvent(new CustomEvent('ide:toggle-command-palette'))}
          className="ide-button flex items-center gap-2 rounded-full px-3 py-1.5 text-[12px]"
          aria-label="Open command palette"
          {...uiMarker('top-toolbar-action', { action: 'command-palette' })}
        >
          <Keyboard size={14} weight="regular" />
          <span>Command Palette</span>
          <span
            className="rounded-full px-2 py-0.5 text-[10px]"
            style={{ background: 'var(--accent-faint)', color: 'var(--text-tertiary)' }}
          >
            Ctrl/Cmd+K
          </span>
        </button>

        {fileName ? (
          <div className="min-w-0" {...uiMarker('top-toolbar-file-context', { filePath: activeFilePath ?? 'none' })}>
            <div className="truncate text-[13px]" style={{ color: 'var(--text-primary)' }} {...uiMarker('top-toolbar-file-name')}>
              {fileName}
            </div>
            {!focusMode && parentName ? (
              <div className="truncate text-[11px]" style={{ color: 'var(--text-tertiary)' }} {...uiMarker('top-toolbar-parent-name')}>
                {parentName}
              </div>
            ) : null}
          </div>
        ) : (
          <div className="text-[12px]" style={{ color: 'var(--text-tertiary)' }} {...uiMarker('top-toolbar-workspace-label')}>
            Workspace
          </div>
        )}
      </div>

      <div className="flex shrink-0 items-center gap-2" {...uiMarker('top-toolbar-right')}>
        <button
          type="button"
          disabled={!activeFilePath}
          onClick={() => window.dispatchEvent(new CustomEvent('ide:save-active-document'))}
          className={`flex items-center gap-2 rounded-full px-3 py-1.5 text-[12px] transition ${
            !activeFilePath
              ? 'ide-button cursor-not-allowed opacity-50'
              : isDirty
                ? 'ide-button-accent'
                : 'ide-button'
          }`}
          aria-label={isDirty ? 'Save file' : 'File saved'}
          {...uiMarker('top-toolbar-action', { action: 'save-document', disabled: !activeFilePath, dirty: isDirty })}
        >
          <FloppyDisk size={14} weight={isDirty ? 'fill' : 'regular'} />
          {!focusMode ? <span>{isDirty ? 'Save' : 'Saved'}</span> : null}
        </button>

        {isMarkdown ? (
          <div className="ide-button flex items-center gap-2 rounded-full px-3 py-1.5 text-[12px]" {...uiMarker('top-toolbar-document-mode', { mode: 'markdown' })}>
            <BookOpenText size={14} weight="regular" />
            {!focusMode ? <span>Markdown</span> : null}
          </div>
        ) : (
          <button
            type="button"
            onClick={onRunBuild}
            disabled={!activeFilePath || isBuilding}
            className={`flex items-center gap-2 rounded-full px-3 py-1.5 text-[12px] transition ${
              !activeFilePath || isBuilding
                ? 'ide-button cursor-not-allowed opacity-50'
                : 'ide-button-accent'
            }`}
            aria-label={isBuilding ? 'Building' : 'Run build'}
            {...uiMarker('top-toolbar-action', { action: 'build', disabled: !activeFilePath || isBuilding, building: isBuilding })}
          >
            <Play size={14} weight="fill" />
            {!focusMode ? <span>{isBuilding ? 'Building' : 'Build'}</span> : null}
          </button>
        )}

        <button
          type="button"
          onClick={onTogglePanel}
          className="ide-button flex items-center gap-2 rounded-full px-3 py-1.5 text-[12px]"
          aria-label={isPanelOpen ? 'Hide utility panel' : 'Show utility panel'}
          {...uiMarker('top-toolbar-action', { action: 'toggle-panel', open: isPanelOpen })}
        >
          <SidebarSimple size={14} weight={isPanelOpen ? 'fill' : 'regular'} />
          {!focusMode ? <span>{isPanelOpen ? 'Panel On' : 'Panel Off'}</span> : null}
        </button>
      </div>
    </div>
  );
}

function MenuItem({
  label,
  shortcut,
  onClick,
  disabled = false,
  debugId,
}: {
  label: string;
  shortcut: string;
  onClick: () => void;
  disabled?: boolean;
  debugId: string;
}) {
  return (
    <button
      type="button"
      onClick={onClick}
      disabled={disabled}
      className={`flex w-full items-center justify-between px-3 py-2 text-left text-[12px] transition ${
        disabled ? 'cursor-not-allowed opacity-45' : 'hover:bg-xcode-accent/10'
      }`}
      style={{ color: 'var(--text-secondary)' }}
      {...uiMarker('top-toolbar-menu-item', { action: debugId, disabled })}
    >
      <span>{label}</span>
      <span style={{ color: 'var(--text-tertiary)' }}>{shortcut}</span>
    </button>
  );
}
