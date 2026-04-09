import { useCallback, useEffect, useMemo, useState } from 'react';
import { useProjectStore } from '../store/useProjectStore';
import { safeInvoke as invoke } from '../utils/tauri';

interface UseDocumentFileOptions {
  path: string;
  language: string;
  defaultSaveName: string;
}

export function useDocumentFile({
  path,
  language,
  defaultSaveName,
}: UseDocumentFileOptions) {
  const {
    setEditorStatus,
    setFileDirtyState,
    openFile,
    closeFile,
    setActiveFile,
  } = useProjectStore();

  const [content, setContent] = useState('');
  const [savedContent, setSavedContent] = useState('');
  const [isLoading, setIsLoading] = useState(true);
  const [isSaving, setIsSaving] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const isDirty = useMemo(() => content !== savedContent, [content, savedContent]);

  useEffect(() => {
    let cancelled = false;

    const loadFile = async () => {
      setIsLoading(true);
      setError(null);

      try {
        const text = await invoke<string>('read_file', { path });
        
        if (cancelled) {
          return;
        }

        // Garantir que o conteúdo sempre seja definido
        const contentText = text || '';
        setContent(contentText);
        setSavedContent(contentText);
        setFileDirtyState(path, false);
        setEditorStatus({
          language,
          line: 1,
          column: 1,
          selection: 'Ln 1, Col 1',
        });
      } catch (err) {
        if (cancelled) {
          return;
        }

        const message = err instanceof Error ? err.message : String(err);
        console.error(`[useDocumentFile] Error reading file ${path}:`, err);
        setError(message);
        setContent('');
        setSavedContent('');
      } finally {
        if (!cancelled) {
          setIsLoading(false);
        }
      }
    };

    void loadFile();

    return () => {
      cancelled = true;
    };
  }, [language, path, setEditorStatus, setFileDirtyState]);

  useEffect(() => {
    setFileDirtyState(path, isDirty);
  }, [isDirty, path, setFileDirtyState]);

  const saveDocument = useCallback(async () => {
    if (isSaving || !isDirty) {
      return;
    }

    setIsSaving(true);
    setError(null);

    try {
      await invoke('write_file', { path, content });
      setSavedContent(content);
      setFileDirtyState(path, false);
    } catch (err) {
      const message = err instanceof Error ? err.message : String(err);
      console.error(`Error writing file ${path}:`, err);
      setError(message);
    } finally {
      setIsSaving(false);
    }
  }, [content, isDirty, isSaving, path, setFileDirtyState]);

  const saveDocumentAs = useCallback(async () => {
    if (isSaving) {
      return;
    }

    setIsSaving(true);
    setError(null);

    try {
      const newPath = await invoke<string | null>('pick_save_path', {
        defaultName: defaultSaveName,
      });

      if (!newPath) {
        return;
      }

      await invoke('write_file', { path: newPath, content });
      setSavedContent(content);
      setFileDirtyState(path, false);
      setFileDirtyState(newPath, false);
      openFile(newPath);
      setActiveFile(newPath);
      closeFile(path);
    } catch (err) {
      const message = err instanceof Error ? err.message : String(err);
      console.error(`Error during save-as for ${path}:`, err);
      setError(message);
    } finally {
      setIsSaving(false);
    }
  }, [closeFile, content, defaultSaveName, isSaving, openFile, path, setActiveFile, setFileDirtyState]);

  return {
    content,
    setContent,
    savedContent,
    isLoading,
    isSaving,
    error,
    isDirty,
    saveDocument,
    saveDocumentAs,
  };
}
