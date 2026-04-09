import { Circle, FileCode } from '@phosphor-icons/react';
import { useProjectStore } from '../store/useProjectStore';
import { uiMarker } from '../utils/debugSelectors';

export function StatusBar() {
  const { activeFilePath, editorStatus, dirtyFiles, isBuilding, focusMode } = useProjectStore();

  const languageLabel = editorStatus.language === 'plaintext' ? 'Plain Text' : editorStatus.language;
  const isDirty = activeFilePath ? dirtyFiles.includes(activeFilePath) : false;

  return (
    <footer
      className="ide-status-surface flex h-8 shrink-0 items-center justify-between border-t px-3 text-[11px]"
      style={{ color: 'var(--text-secondary)' }}
      {...uiMarker('status-bar', { activeFile: activeFilePath ?? 'none', building: isBuilding })}
    >
      <div className="flex min-w-0 items-center gap-3" {...uiMarker('status-bar-left')}>
        <div className="flex min-w-0 items-center gap-2" {...uiMarker('status-bar-file')}>
          <FileCode size={13} style={{ color: 'var(--text-tertiary)' }} />
          <span className="truncate">{activeFilePath ? activeFilePath.split(/[/\\]/).pop() : 'No file selected'}</span>
        </div>

        {activeFilePath ? (
          <div className="flex items-center gap-1.5" style={{ color: isDirty ? 'var(--accent)' : 'var(--text-tertiary)' }} {...uiMarker('status-bar-dirty-indicator', { dirty: isDirty })}>
            <Circle size={10} weight={isDirty ? 'fill' : 'regular'} style={{ color: isDirty ? 'var(--accent)' : 'var(--success)' }} />
            <span>{isDirty ? 'Unsaved' : 'Saved'}</span>
          </div>
        ) : null}

        {!focusMode ? <div {...uiMarker('status-bar-build-state')}>{isBuilding ? 'Build running' : 'Ready'}</div> : null}
      </div>

      <div className="flex items-center gap-3" {...uiMarker('status-bar-right')}>
        {!focusMode ? <div {...uiMarker('status-bar-language')}>{languageLabel}</div> : null}
        <div {...uiMarker('status-bar-selection')}>{editorStatus.selection}</div>
      </div>
    </footer>
  );
}
