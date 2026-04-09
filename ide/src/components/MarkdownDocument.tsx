import { useEffect, useMemo, useState } from 'react';
import ReactMarkdown from 'react-markdown';
import remarkGfm from 'remark-gfm';
import { BookOpen, Columns2, Edit3, Eye, LoaderCircle, Save } from 'lucide-react';
import { useProjectStore } from '../store/useProjectStore';
import { getEditorFontStack, getEditorLineHeight } from '../utils/editorPreferences';
import { useDocumentFile } from '../hooks/useDocumentFile';
import { uiMarker } from '../utils/debugSelectors';
import { CodeEditorSurface } from './CodeEditorSurface';

interface MarkdownDocumentProps {
  path: string;
}

type MarkdownMode = 'read' | 'edit' | 'split';

export function MarkdownDocument({ path }: MarkdownDocumentProps) {
  const { theme, editorFontFamily, editorFontSize, setEditorStatus } = useProjectStore();
  const [mode, setMode] = useState<MarkdownMode>('read');

  const fileName = useMemo(() => path.split(/[/\\]/).pop() || path, [path]);
  const editorFontStack = useMemo(() => getEditorFontStack(editorFontFamily), [editorFontFamily]);
  const lineHeight = useMemo(() => getEditorLineHeight(editorFontSize), [editorFontSize]);
  const {
    content,
    setContent,
    isLoading,
    isSaving,
    error,
    isDirty,
    saveDocument,
    saveDocumentAs,
  } = useDocumentFile({
    path,
    language: 'markdown',
    defaultSaveName: fileName,
  });

  useEffect(() => {
    setMode('read');
  }, [path]);

  useEffect(() => {
    const handleSave = (event: KeyboardEvent | Event) => {
      if (mode === 'read' && event instanceof KeyboardEvent) {
        return;
      }

      if (event instanceof KeyboardEvent && (event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 's') {
        event.preventDefault();
        void saveDocument();
        return;
      }

      if (!(event instanceof KeyboardEvent)) {
        void saveDocument();
      }
    };

    const handleSaveAs = () => {
      void saveDocumentAs();
    };

    window.addEventListener('keydown', handleSave);
    window.addEventListener('ide:save-active-document', handleSave);
    window.addEventListener('ide:save-active-document-as', handleSaveAs);
    return () => {
      window.removeEventListener('keydown', handleSave);
      window.removeEventListener('ide:save-active-document', handleSave);
      window.removeEventListener('ide:save-active-document-as', handleSaveAs);
    };
  }, [mode, saveDocument, saveDocumentAs]);

  if (isLoading) {
    return (
      <div className="flex h-full items-center justify-center" style={{ background: 'color-mix(in srgb, var(--ide-editor) 78%, white 22%)' }} {...uiMarker('markdown-document-loading', { filePath: path })}>
        <div className="ide-button flex items-center gap-3 rounded-full px-4 py-2 text-sm" {...uiMarker('markdown-document-loading-pill')}>
          <LoaderCircle size={14} className="animate-spin" />
          <span>Loading markdown document...</span>
        </div>
      </div>
    );
  }

  if (error && !content) {
    return (
      <div className="flex h-full items-center justify-center px-6" {...uiMarker('markdown-document-error-state', { filePath: path })}>
        <div
          className="max-w-xl rounded-[28px] border p-6 text-left"
          style={{
            borderColor: 'color-mix(in srgb, var(--danger) 20%, var(--border))',
            background: 'color-mix(in srgb, var(--danger) 10%, white 90%)',
          }}
          {...uiMarker('markdown-document-error-card')}
        >
          <div className="text-sm font-medium" style={{ color: 'color-mix(in srgb, var(--danger) 80%, black 20%)' }} {...uiMarker('markdown-document-error-title')}>Unable to open markdown</div>
          <div className="mt-2 text-sm leading-6" style={{ color: 'color-mix(in srgb, var(--danger) 68%, black 32%)' }} {...uiMarker('markdown-document-error-message')}>{error}</div>
        </div>
      </div>
    );
  }

  return (
    <div className="flex h-full min-h-0 flex-col" style={{ background: 'color-mix(in srgb, var(--ide-editor) 78%, white 22%)' }} {...uiMarker('markdown-document', { filePath: path, mode, dirty: isDirty })}>
      <div className="ide-toolbar-surface flex shrink-0 items-center justify-between gap-4 border-b px-5 py-3" {...uiMarker('markdown-document-toolbar')}>
        <div className="min-w-0" {...uiMarker('markdown-document-meta')}>
          <div className="flex items-center gap-2 text-[10px] font-bold uppercase tracking-[0.28em]" style={{ color: 'var(--text-tertiary)' }} {...uiMarker('markdown-document-label')}>
            <BookOpen size={12} />
            <span>Markdown</span>
          </div>
          <div className="mt-1 truncate text-sm" style={{ color: 'var(--text-primary)' }} {...uiMarker('markdown-document-file-name')}>{fileName}</div>
          <div className="mt-1 text-[11px]" style={{ color: 'var(--text-secondary)' }} {...uiMarker('markdown-document-description')}>
            {mode === 'read'
              ? 'Reader mode with formatted markdown preview.'
              : mode === 'edit'
                ? 'Editor mode with Markdown syntax and Ctrl/Cmd+S support.'
                : 'Split mode keeps the editor and preview visible at the same time.'}
          </div>
        </div>

        <div className="flex shrink-0 items-center gap-2" {...uiMarker('markdown-document-actions')}>
          <div className="ide-segment flex items-center rounded-full p-1" {...uiMarker('markdown-document-mode-switch')}>
            <ModeButton
              label="Read"
              icon={Eye}
              isActive={mode === 'read'}
              onClick={() => setMode('read')}
              debugId="read"
            />
            <ModeButton
              label="Edit"
              icon={Edit3}
              isActive={mode === 'edit'}
              onClick={() => setMode('edit')}
              debugId="edit"
            />
            <ModeButton
              label="Split"
              icon={Columns2}
              isActive={mode === 'split'}
              onClick={() => setMode('split')}
              debugId="split"
            />
          </div>

          <button
            type="button"
            onClick={() => void saveDocument()}
            disabled={!isDirty || isSaving}
            className={`flex items-center gap-2 rounded-full px-3 py-2 text-xs transition ${
              !isDirty || isSaving
                ? 'ide-button cursor-not-allowed opacity-50'
                : 'ide-button-accent'
            }`}
            {...uiMarker('markdown-document-save', { dirty: isDirty, saving: isSaving })}
          >
            {isSaving ? <LoaderCircle size={14} className="animate-spin" /> : <Save size={14} />}
            <span>{isSaving ? 'Saving...' : isDirty ? 'Save' : 'Saved'}</span>
          </button>
        </div>
      </div>

      {error && (
        <div
          className="border-b px-5 py-2 text-xs"
          style={{
            borderColor: 'color-mix(in srgb, var(--danger) 18%, var(--border))',
            background: 'color-mix(in srgb, var(--danger) 10%, white 90%)',
            color: 'color-mix(in srgb, var(--danger) 74%, black 26%)',
          }}
          {...uiMarker('markdown-document-inline-error')}
        >
          {error}
        </div>
      )}

      <div className="min-h-0 flex-1" {...uiMarker('markdown-document-body', { mode })}>
        {mode === 'read' ? (
          <MarkdownPreviewPane content={content} />
        ) : mode === 'edit' ? (
          <div className="h-full overflow-hidden" {...uiMarker('markdown-document-edit-pane')}>
            <CodeEditorSurface
              path={path}
              language="markdown"
              value={content}
              onChange={setContent}
              onCursorChange={(line, column) => {
                setEditorStatus({
                  language: 'markdown',
                  line,
                  column,
                  selection: `Ln ${line}, Col ${column}`,
                });
              }}
              theme={theme}
              fontFamily={editorFontStack}
              fontSize={editorFontSize}
              lineHeight={lineHeight}
              wrapLines
            />
          </div>
        ) : (
          <div
            className="grid h-full min-h-0 grid-cols-1 divide-y xl:grid-cols-[minmax(0,1fr)_minmax(0,1fr)] xl:divide-x xl:divide-y-0"
            style={{ borderColor: 'var(--border)' }}
            {...uiMarker('markdown-document-split-layout')}
          >
            <div className="min-h-0 overflow-hidden" {...uiMarker('markdown-document-split-editor')}>
              <CodeEditorSurface
                path={path}
                language="markdown"
                value={content}
                onChange={setContent}
                onCursorChange={(line, column) => {
                  setEditorStatus({
                    language: 'markdown',
                    line,
                    column,
                    selection: `Ln ${line}, Col ${column}`,
                  });
                }}
                theme={theme}
                fontFamily={editorFontStack}
                fontSize={editorFontSize}
                lineHeight={lineHeight}
                wrapLines
              />
            </div>
            <MarkdownPreviewPane content={content} inset />
          </div>
        )}
      </div>
    </div>
  );
}

function MarkdownPreviewPane({
  content,
  inset = false,
}: {
  content: string;
  inset?: boolean;
}) {
  return (
    <div
      className="markdown-preview h-full overflow-y-auto"
      style={inset ? { background: 'color-mix(in srgb, var(--ide-panel) 72%, white 28%)' } : undefined}
      {...uiMarker('markdown-preview-pane', { inset })}
    >
      <article className="prose max-w-none" {...uiMarker('markdown-preview-article')}>
        <ReactMarkdown remarkPlugins={[remarkGfm]}>
          {content || '# Empty markdown\n\nThis file does not have any content yet.'}
        </ReactMarkdown>
      </article>
    </div>
  );
}

function ModeButton({
  label,
  icon: Icon,
  isActive,
  onClick,
  debugId,
}: {
  label: string;
  icon: typeof Eye;
  isActive: boolean;
  onClick: () => void;
  debugId: string;
}) {
  return (
    <button
      type="button"
      onClick={onClick}
      className={`flex items-center gap-2 rounded-full px-3 py-2 text-xs transition ${
        isActive
          ? 'ide-segment-active'
          : 'ide-button-ghost'
      }`}
      style={isActive ? undefined : { color: 'var(--text-secondary)' }}
      {...uiMarker('markdown-mode-button', { mode: debugId, active: isActive })}
    >
      <Icon size={13} />
      <span>{label}</span>
    </button>
  );
}
