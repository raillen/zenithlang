import { useEffect, useMemo } from 'react';
import { useProjectStore } from '../store/useProjectStore';
import { getEditorFontStack, getEditorLineHeight } from '../utils/editorPreferences';
import { useDocumentFile } from '../hooks/useDocumentFile';
import { uiMarker } from '../utils/debugSelectors';
import { CodeEditorSurface } from './CodeEditorSurface';
import { getEditorLanguage } from '../utils/editorLanguage';

interface EditorPanelProps {
  path: string;
}

export function EditorPanel({ path }: EditorPanelProps) {
  const { theme, editorFontFamily, editorFontSize, setEditorStatus } = useProjectStore();

  const language = useMemo(() => getEditorLanguage(path), [path]);
  const editorFontStack = useMemo(() => getEditorFontStack(editorFontFamily), [editorFontFamily]);
  const lineHeight = useMemo(() => getEditorLineHeight(editorFontSize), [editorFontSize]);
  const fileName = useMemo(() => path.split(/[/\\]/).pop() || 'untitled.zt', [path]);
  const {
    content,
    setContent,
    isLoading,
    saveDocument,
    saveDocumentAs,
    error,
  } = useDocumentFile({
    path,
    language,
    defaultSaveName: fileName,
  });

  useEffect(() => {
    const handleSave = (event: KeyboardEvent | Event) => {
      if (
        event instanceof KeyboardEvent &&
        (event.ctrlKey || event.metaKey) &&
        event.key.toLowerCase() === 's'
      ) {
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
  }, [saveDocument, saveDocumentAs]);

  if (isLoading) {
    return (
      <div
        className="flex flex-1 items-center justify-center"
        style={{ background: 'color-mix(in srgb, var(--ide-editor) 78%, white 22%)' }}
        {...uiMarker('editor-panel-loading', { filePath: path })}
      >
        <div
          className="animate-pulse text-sm"
          style={{ color: 'var(--text-tertiary)' }}
          {...uiMarker('editor-panel-loading-label')}
        >
          Loading {path.split(/[/\\]/).pop()}...
        </div>
      </div>
    );
  }

  if (error && !content) {
    return (
      <div
        className="flex h-full items-center justify-center px-6"
        {...uiMarker('editor-panel-error-state', { filePath: path })}
      >
        <div
          className="max-w-xl rounded-[28px] border p-6 text-left"
          style={{
            borderColor: 'color-mix(in srgb, var(--danger) 20%, var(--border))',
            background: 'color-mix(in srgb, var(--danger) 10%, white 90%)',
          }}
          {...uiMarker('editor-panel-error-card')}
        >
          <div
            className="text-sm font-medium"
            style={{ color: 'color-mix(in srgb, var(--danger) 80%, black 20%)' }}
            {...uiMarker('editor-panel-error-title')}
          >
            Unable to open document
          </div>
          <div
            className="mt-2 text-sm leading-6"
            style={{ color: 'color-mix(in srgb, var(--danger) 68%, black 32%)' }}
            {...uiMarker('editor-panel-error-message')}
          >
            {error}
          </div>
        </div>
      </div>
    );
  }

  return (
    <div
      className="relative isolate z-20 h-full flex-1 overflow-hidden"
      style={{ background: 'color-mix(in srgb, var(--ide-editor) 78%, white 22%)' }}
      {...uiMarker('editor-panel', { filePath: path, language })}
    >
      {error ? (
        <div
          className="border-b px-4 py-2 text-xs"
          style={{
            borderColor: 'color-mix(in srgb, var(--danger) 18%, var(--border))',
            background: 'color-mix(in srgb, var(--danger) 10%, white 90%)',
            color: 'color-mix(in srgb, var(--danger) 74%, black 26%)',
          }}
          {...uiMarker('editor-panel-inline-error', { filePath: path })}
        >
          {error}
        </div>
      ) : null}

      <CodeEditorSurface
        path={path}
        language={language}
        value={content}
        onChange={setContent}
        onCursorChange={(line, column) => {
          setEditorStatus({
            language,
            line,
            column,
            selection: `Ln ${line}, Col ${column}`,
          });
        }}
        theme={theme}
        fontFamily={editorFontStack}
        fontSize={editorFontSize}
        lineHeight={lineHeight}
      />
    </div>
  );
}
