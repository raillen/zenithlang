import { useEffect, useRef, useState } from "react";
import Editor, { DiffEditor } from "@monaco-editor/react";
import { GitCommit, X } from "lucide-react";
import { Group, Panel, Separator } from "react-resizable-panels";
import {
  useWorkspaceStore,
  Diagnostic,
  EditorPane as EditorPaneId,
  FileEntry,
} from "../store/useWorkspaceStore";
import { invoke } from "../utils/tauri";
import { TabManager } from "./TabManager";
import { registerZenithLanguage } from "../utils/zenithLang";
import { THEMES, defaultLight } from "../themes";
import { getMonacoTheme } from "../utils/themeEngine";
import { BrandLogo } from "./BrandLogo";
import { useCommandStore } from "../store/useCommandStore";
import { getEditorLanguageForFile } from "../utils/editorLanguages";
import { registerSemanticCompletionProviders } from "../utils/semanticCompletions";
import {
  clearEditorBuffers,
  clearPendingBufferLoad,
  getCachedBuffer,
  getPendingBufferLoad,
  rememberBuffer,
  setPendingBufferLoad,
} from "../utils/editorBuffers";
import {
  getLatestLspDiagnostics,
  listenForLspDiagnostics,
  syncLspDocument,
} from "../utils/lspClient";

const AUTO_SAVE_DELAY_MS = 1200;
let sharedMonaco: any = null;

async function ensureFileContent(path: string) {
  const cached = getCachedBuffer(path);
  if (cached !== undefined) {
    return cached;
  }

  const pending = getPendingBufferLoad(path);
  if (pending) {
    return pending;
  }

  const loadPromise = invoke<string>("read_file", { path })
    .then((content) => {
      rememberBuffer(path, content);
      clearPendingBufferLoad(path);
      return content;
    })
    .catch((error) => {
      clearPendingBufferLoad(path);
      throw error;
    });

  setPendingBufferLoad(path, loadPromise);
  return loadPromise;
}

export function EditorPanel() {
  const {
    primaryFile,
    secondaryFile,
    splitMode,
    focusedPane,
    currentProjectRoot,
    setFocusedPane,
  } = useWorkspaceStore();
  const previousRootRef = useRef(currentProjectRoot);

  useEffect(() => {
    if (previousRootRef.current === currentProjectRoot) return;

    clearEditorBuffers();

    if (sharedMonaco) {
      sharedMonaco.editor.getModels().forEach((model: any) => model.dispose());
    }

    previousRootRef.current = currentProjectRoot;
  }, [currentProjectRoot]);

  const showLanding = !primaryFile && !secondaryFile && splitMode === "single";

  return (
    <div className="flex-1 flex flex-col h-full overflow-hidden bg-ide-bg transition-colors duration-200">
      <TabManager />

      {showLanding ? (
        <EditorLanding />
      ) : splitMode === "single" ? (
        <div className="flex-1 min-h-0">
          <EditorPane
            pane="primary"
            file={primaryFile}
            isFocused
            onFocus={() => setFocusedPane("primary")}
          />
        </div>
      ) : (
        <div className="flex-1 min-h-0">
          <Group
            orientation={splitMode === "vertical" ? "horizontal" : "vertical"}
            className="h-full"
          >
            <Panel defaultSize={50} minSize={20} className="min-h-0">
              <EditorPane
                pane="primary"
                file={primaryFile}
                isFocused={focusedPane === "primary"}
                onFocus={() => setFocusedPane("primary")}
              />
            </Panel>

            <Separator
              className={
                splitMode === "vertical"
                  ? "w-[1px] bg-ide-border hover:bg-primary/30 transition-colors"
                  : "h-[1px] bg-ide-border hover:bg-primary/30 transition-colors"
              }
            />

            <Panel defaultSize={50} minSize={20} className="min-h-0">
              <EditorPane
                pane="secondary"
                file={secondaryFile}
                isFocused={focusedPane === "secondary"}
                onFocus={() => setFocusedPane("secondary")}
              />
            </Panel>
          </Group>
        </div>
      )}
    </div>
  );
}

function EditorPane({
  pane,
  file,
  isFocused,
  onFocus,
}: {
  pane: EditorPaneId;
  file: FileEntry | null;
  isFocused: boolean;
  onFocus: () => void;
}) {
  const {
    settings,
    diagnosticsMap,
    dirtyFiles,
    setFileDirty,
    setFileBuffer,
    setDiagnostics,
    currentProjectRoot,
    breakpoints,
    isZenMode,
    splitMode,
    toggleBreakpoint,
    diffFile,
    closeDiff,
    setCursorPosition
  } = useWorkspaceStore();
  const [content, setContent] = useState("");
  const [originalContent, setOriginalContent] = useState("");
  const [isReady, setIsReady] = useState(!file);
  const [loadError, setLoadError] = useState<string | null>(null);
  const editorRef = useRef<any>(null);
  const monacoRef = useRef<any>(null);
  const fileRef = useRef<FileEntry | null>(file);
  const settingsRef = useRef(settings);
  const autoSaveTimerRef = useRef<number | null>(null);
  const isSavingRef = useRef(false);
  const currentThemeObj = THEMES[settings.theme] || defaultLight;
  const pathSegments = file ? getRelativeSegments(file.path, currentProjectRoot) : [];
  const paneLabelId = `editor-pane-${pane}-label`;
  const isDirty = Boolean(file && dirtyFiles.has(file.path));
  const editorLanguage = getEditorLanguageForFile(file);

  const clearAutoSaveTimer = () => {
    if (autoSaveTimerRef.current !== null) {
      window.clearTimeout(autoSaveTimerRef.current);
      autoSaveTimerRef.current = null;
    }
  };

  const saveCurrentFile = async (reason: string, explicitFile?: FileEntry | null) => {
    const currentFile = explicitFile ?? fileRef.current;
    const editor = editorRef.current;

    if (!currentFile || currentFile.is_directory || !editor) return false;
    if (!useWorkspaceStore.getState().dirtyFiles.has(currentFile.path)) return false;
    if (isSavingRef.current) return false;

    clearAutoSaveTimer();
    isSavingRef.current = true;

    try {
      if (settingsRef.current.editorFormatOnSave) {
        try {
          await editor.getAction?.("editor.action.formatDocument")?.run();
        } catch (formatError) {
          console.warn("[Zenith Keter] Format on save skipped:", formatError);
        }
      }

      const valueToSave = editor.getValue();
      rememberBuffer(currentFile.path, valueToSave);
      setFileBuffer(currentFile.path, valueToSave);
      setContent((current) => (current === valueToSave ? current : valueToSave));

      await invoke("write_file", { path: currentFile.path, content: valueToSave });

      // Sanctuary History Snapshot
      try {
        await invoke("history_save_snapshot", {
          path: currentFile.path,
          content: valueToSave,
          maxSnapshots: settingsRef.current.historyMaxSnapshots,
          retentionDays: settingsRef.current.historyRetentionDays
        });
      } catch (historyErr) {
        console.warn("[Zenith Sanctuary] Snapshot failed:", historyErr);
      }

      const latestValue = editor.getValue();
      const changedDuringSave = latestValue !== valueToSave;

      rememberBuffer(currentFile.path, latestValue);
      setFileBuffer(currentFile.path, latestValue);
      setContent((current) => (current === latestValue ? current : latestValue));
      setFileDirty(currentFile.path, changedDuringSave);

      return !changedDuringSave;
    } catch (err) {
      console.error(`[Zenith Keter] Save Error (${reason}):`, err);
      return false;
    } finally {
      isSavingRef.current = false;
    }
  };

  useEffect(() => {
    fileRef.current = file;
  }, [file]);

  useEffect(() => {
    settingsRef.current = settings;
  }, [settings]);

  useEffect(() => () => clearAutoSaveTimer(), []);

  useEffect(() => {
    if (monacoRef.current) {
      monacoRef.current.editor.defineTheme("zenith-dynamic", getMonacoTheme(currentThemeObj));
      monacoRef.current.editor.setTheme("zenith-dynamic");
    }
  }, [currentThemeObj]);

  useEffect(() => {
    let isMounted = true;

    async function loadFile() {
      if (!file) {
        setContent("");
        setLoadError(null);
        setIsReady(true);
        return;
      }

      const cachedContent = getCachedBuffer(file.path);
      if (cachedContent !== undefined) {
        setContent(cachedContent);
        setFileBuffer(file.path, cachedContent);
        setLoadError(null);
        setIsReady(true);
        return;
      }

      setIsReady(false);
      setLoadError(null);

      try {
        const fileContent = await ensureFileContent(file.path);
        if (!isMounted) return;
        setContent(fileContent);
        setFileBuffer(file.path, fileContent);
        setLoadError(null);
        setIsReady(true);
      } catch (err) {
        if (!isMounted) return;
        console.error("Failed to read file:", err);
        setLoadError(String(err));
        setIsReady(true);
      }
    }

    loadFile();
    return () => {
      isMounted = false;
    };
  }, [file?.path]);

  // Load original content for Diff
  useEffect(() => {
    let isMounted = true;
    if (diffFile && file && diffFile === file.path) {
      invoke<string>("git_read_original", { path: file.path })
        .then(original => {
          if (isMounted) {
            setOriginalContent(original);
          }
        })
        .catch(err => {
          console.error("Failed to load original git content", err);
        });
    } else {
      setOriginalContent("");
    }
    return () => { isMounted = false; };
  }, [diffFile, file?.path]);

  useEffect(() => {
    if (!file || !isReady || loadError || !content || file.is_directory) return;

    const timer = window.setTimeout(async () => {
      try {
        const lspHandled = await syncLspDocument(file, editorLanguage, content).catch(() => false);
        if (lspHandled) {
          const diagnostics = await getLatestLspDiagnostics(file, editorLanguage, content).catch(() => []);
          setDiagnostics(file.path, diagnostics as Diagnostic[]);
          return;
        }

        if (editorLanguage === "zenith") {
          const results = await invoke<Diagnostic[]>("run_diagnostics", {
            path: file.path,
            content,
          });
          setDiagnostics(file.path, results);
          return;
        }

        setDiagnostics(file.path, []);
      } catch (err) {
        console.error("Diagnostics error:", err);
      }
    }, 800);

    return () => window.clearTimeout(timer);
  }, [content, editorLanguage, file?.path, isReady, loadError, setDiagnostics]);

  useEffect(() => {
    if (!file || !editorRef.current || !monacoRef.current) return;

    const markers = (diagnosticsMap[file.path] || []).map((diagnostic) => ({
      message: `${diagnostic.code}: ${diagnostic.message}`,
      severity:
        diagnostic.severity === "error"
          ? monacoRef.current.MarkerSeverity.Error
          : diagnostic.severity === "warning"
            ? monacoRef.current.MarkerSeverity.Warning
            : monacoRef.current.MarkerSeverity.Info,
      startLineNumber: diagnostic.line,
      startColumn: diagnostic.col,
      endLineNumber: diagnostic.line,
      endColumn: diagnostic.col + 1,
    }));

    const model = editorRef.current.getModel();
    if (model) {
      monacoRef.current.editor.setModelMarkers(model, "zenith", markers);
    }
  }, [diagnosticsMap, file?.path]);

  // Sync Breakpoints as Decorations
  useEffect(() => {
    if (!file || !editorRef.current || !monacoRef.current) return;

    const editor = editorRef.current;
    const monaco = monacoRef.current;
    const model = editor.getModel();
    if (!model) return;

    const fileBreakpoints = breakpoints.filter(b => b.path === file.path);
    const decorations = fileBreakpoints.map(bp => ({
      range: new monaco.Range(bp.line, 1, bp.line, 1),
      options: {
        isWholeLine: true,
        glyphMarginClassName: "zenith-breakpoint-glyph",
        glyphMarginHoverMessage: { value: "Breakpoint" },
        stickiness: monaco.editor.TrackedRangeStickiness.NeverGrowsWhenTypingAtEdges
      }
    }));

    const collection = editor.createDecorationsCollection(decorations);
    return () => collection.clear();
  }, [breakpoints, file?.path]);

  // Focus Mode Logic: Dimming non-active lines
  useEffect(() => {
    if (!isZenMode || !editorRef.current || !monacoRef.current || !file) return;

    const editor = editorRef.current;
    const monaco = monacoRef.current;
    let decorations = editor.createDecorationsCollection([]);

    const updateFocusDecorations = () => {
      const position = editor.getPosition();
      if (!position) return;

      const model = editor.getModel();
      if (!model) return;

      const lineCount = model.getLineCount();
      const currentLine = position.lineNumber;

      const newDecorations = [];

      // Before current line
      if (currentLine > 1) {
        newDecorations.push({
          range: new monaco.Range(1, 1, currentLine - 1, model.getLineMaxColumn(currentLine - 1)),
          options: {
            isWholeLine: true,
            inlineClassName: 'zenith-focus-dimmed',
          }
        });
      }

      // After current line
      if (currentLine < lineCount) {
        newDecorations.push({
          range: new monaco.Range(currentLine + 1, 1, lineCount, model.getLineMaxColumn(lineCount)),
          options: {
            isWholeLine: true,
            inlineClassName: 'zenith-focus-dimmed',
          }
        });
      }

      decorations.set(newDecorations);
    };

    const disposable = editor.onDidChangeCursorPosition(updateFocusDecorations);
    updateFocusDecorations(); // Initial state

    return () => {
      disposable.dispose();
      decorations.clear();
    };
  }, [isZenMode, file?.path]);

  useEffect(() => {
    if (isFocused) {
      editorRef.current?.focus();
    }
  }, [isFocused, file?.path]);

  useEffect(() => {
    const editor = editorRef.current;
    const monaco = monacoRef.current;
    const model = editor?.getModel?.();

    if (monaco && model) {
      monaco.editor.setModelLanguage(model, editorLanguage);
    }
  }, [editorLanguage, file?.path]);

  useEffect(() => {
    const handleJump = (event: Event) => {
      if (!isFocused || !editorRef.current) return;

      const detail = (event as CustomEvent<{ line: number; col: number }>).detail;
      editorRef.current.revealLineInCenter(detail.line);
      editorRef.current.setPosition({ lineNumber: detail.line, column: detail.col });
      editorRef.current.focus();
    };

    window.addEventListener("zenith://editor-jump", handleJump as EventListener);

    const handleSetContent = (e: CustomEvent) => {
      const { content: restoredContent } = e.detail;
      if (editorRef.current && isFocused) {
        editorRef.current.setValue(restoredContent);
        if (file) {
          setFileDirty(file.path, true);
          rememberBuffer(file.path, restoredContent);
          setFileBuffer(file.path, restoredContent);
        }
      }
    };
    window.addEventListener("zenith://editor-set-content", handleSetContent as EventListener);

    const handleReplaceContent = (e: CustomEvent<{ path: string; content: string }>) => {
      const currentFile = fileRef.current;
      if (!currentFile || currentFile.path !== e.detail.path) return;

      if (editorRef.current) {
        editorRef.current.setValue(e.detail.content);
      }

      rememberBuffer(e.detail.path, e.detail.content);
      setFileBuffer(e.detail.path, e.detail.content);
      setContent(e.detail.content);
      setFileDirty(e.detail.path, false);
    };
    window.addEventListener("zenith://editor-replace-content", handleReplaceContent as EventListener);

    return () => {
      window.removeEventListener("zenith://editor-jump", handleJump as EventListener);
      window.removeEventListener("zenith://editor-set-content", handleSetContent as EventListener);
      window.removeEventListener("zenith://editor-replace-content", handleReplaceContent as EventListener);
    };
  }, [isFocused, file, setFileBuffer, setFileDirty]);

  useEffect(() => {
    let disposed = false;
    let unlisten: (() => void) | null = null;

    void listenForLspDiagnostics((payload) => {
      if (disposed) return;
      if (!fileRef.current || payload.filePath !== fileRef.current.path) return;
      setDiagnostics(payload.filePath, payload.diagnostics as Diagnostic[]);
    }).then((nextUnlisten) => {
      if (disposed) {
        nextUnlisten?.();
        return;
      }
      unlisten = nextUnlisten;
    });

    return () => {
      disposed = true;
      unlisten?.();
    };
  }, [setDiagnostics]);

  useEffect(() => {
    if (
      settings.autoSave !== "afterDelay" ||
      !file ||
      file.is_directory ||
      !isReady ||
      loadError ||
      !isDirty
    ) {
      clearAutoSaveTimer();
      return;
    }

    clearAutoSaveTimer();
    autoSaveTimerRef.current = window.setTimeout(() => {
      void saveCurrentFile("after-delay");
    }, AUTO_SAVE_DELAY_MS);

    return clearAutoSaveTimer;
  }, [content, file, isDirty, isReady, loadError, settings.autoSave]);

  useEffect(() => {
    const previousFile = file;

    return () => {
      clearAutoSaveTimer();

      if (!previousFile || previousFile.is_directory) return;
      if (settingsRef.current.autoSave !== "onFocusChange") return;
      if (!useWorkspaceStore.getState().dirtyFiles.has(previousFile.path)) return;

      void saveCurrentFile("focus-change", previousFile);
    };
  }, [file?.path]);

  const handleEditorDidMount = (editor: any, monaco: any) => {
    editorRef.current = editor;
    monacoRef.current = monaco;
    sharedMonaco = monaco;

    registerZenithLanguage(monaco);
    registerSemanticCompletionProviders(monaco);

    const model = editor.getModel?.();
    if (model) {
      monaco.editor.setModelLanguage(model, editorLanguage);
    }

    monaco.editor.defineTheme("zenith-dynamic", getMonacoTheme(currentThemeObj));
    monaco.editor.setTheme("zenith-dynamic");

    editor.onDidFocusEditorText(() => {
      onFocus();
      const pos = editor.getPosition();
      if (pos) {
        setCursorPosition({ line: pos.lineNumber, col: pos.column });
      }
    });

    editor.onDidChangeCursorPosition((e: any) => {
      if (isFocused) {
        setCursorPosition({ line: e.position.lineNumber, col: e.position.column });
      }
    });

    editor.onDidBlurEditorText(() => {
      if (settingsRef.current.autoSave === "onFocusChange") {
        void saveCurrentFile("focus-change");
      }
    });

    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyS, async () => {
      await saveCurrentFile("manual");
    });
    // Breakpoint Toggling via Glyph Margin
    editor.onMouseDown((e: any) => {
      if (e.target.type === 2) { // 2 is GlyphMargin
        const line = e.target.position.lineNumber;
        const currentFile = fileRef.current;
        if (currentFile) {
          toggleBreakpoint(currentFile.path, line);
        }
      }
    });

    // VS Code Style: Bridge IDE commands to Monaco
    const keymap = useWorkspaceStore.getState().settings.keymap || {};
    const executeCommand = useCommandStore.getState().executeCommand;

    Object.entries(keymap).forEach(([combo, commandId]) => {
      const keybinding = parseComboToMonaco(monaco, combo);
      if (keybinding) {
        editor.addCommand(keybinding, () => {
          executeCommand(commandId as string);
        });
      }
    });
  };

  const handleEditorChange = (value: string | undefined) => {
    const nextValue = value || "";
    const currentFile = fileRef.current;

    if (currentFile) {
      setFileDirty(currentFile.path, true);
      rememberBuffer(currentFile.path, nextValue);
      setFileBuffer(currentFile.path, nextValue);
    }

    setContent(nextValue);
  };

  const handlePaneKeyDown = (event: React.KeyboardEvent<HTMLDivElement>) => {
    if (event.target !== event.currentTarget) return;

    if (event.key === "Enter" || event.key === " ") {
      event.preventDefault();
      onFocus();
      editorRef.current?.focus();
    }
  };

  return (
    <section
      id={`editor-pane-${pane}`}
      role="region"
      aria-labelledby={paneLabelId}
      tabIndex={0}
      onMouseDown={onFocus}
      onFocusCapture={onFocus}
      onKeyDown={handlePaneKeyDown}
      className={`flex h-full flex-col overflow-hidden bg-ide-bg transition-all duration-200 focus:outline-none ${
        isFocused ? "ring-1 ring-inset ring-primary/25" : ""
      }`}
    >
      {!isZenMode && <PaneHeader pane={pane} isFocused={isFocused} pathSegments={pathSegments} />}

      {!file ? (
        <EmptyPane pane={pane} isFocused={isFocused} />
      ) : !isReady ? (
        <LoadingPane pane={pane} />
      ) : loadError ? (
        <LoadErrorPane pane={pane} error={loadError} />
      ) : isZenMode && splitMode === 'single' ? (
        <div className="flex-1 overflow-hidden relative flex flex-col items-center justify-start pt-8 pb-32 px-4 scrollbar-hide">
           <div className="w-full max-w-[920px] h-full flex flex-col bg-ide-bg rounded-xl border border-ide-border overflow-hidden zenith-sanctuary-editor transition-all duration-700">
              <EditorComponent
                file={file}
                settings={settings}
                editorLanguage={editorLanguage}
                content={content}
                handleEditorDidMount={handleEditorDidMount}
                handleEditorChange={handleEditorChange}
                isZenMode={isZenMode}
              />
           </div>
        </div>
      ) : (
        <div className="flex-1 overflow-hidden relative px-0">
          {diffFile === file.path ? (
            <div className="h-full flex flex-col">
              <div className="h-7 bg-primary/10 border-b border-ide-border flex items-center justify-between px-3">
                <span className="text-[9px] font-bold uppercase tracking-widest text-primary flex items-center gap-2">
                  <GitCommit size={10} />
                  Git Comparison (HEAD â†” Current)
                </span>
                <button 
                  onClick={closeDiff}
                  className="p-1 hover:bg-black/10 rounded text-ide-text-dim hover:text-ide-text transition-colors"
                >
                  <X size={12} />
                </button>
              </div>
              <div className="flex-1 overflow-hidden">
                <DiffEditorComponent
                  original={originalContent}
                  modified={content}
                  language={editorLanguage}
                  settings={settings}
                  isZenMode={isZenMode}
                  onMount={handleEditorDidMount}
                />
              </div>
            </div>
          ) : (
            <EditorComponent
              file={file}
              settings={settings}
              editorLanguage={editorLanguage}
              content={content}
              handleEditorDidMount={handleEditorDidMount}
              handleEditorChange={handleEditorChange}
              isZenMode={isZenMode}
            />
          )}
        </div>
      )}
    </section>
  );
}


function PaneHeader({
  pane,
  isFocused,
  pathSegments,
}: {
  pane: EditorPaneId;
  isFocused: boolean;
  pathSegments: string[];
}) {
  const paneLabel = pane === "primary" ? "Primary Pane" : "Secondary Pane";

  return (
    <div className="h-9 px-3 flex items-center justify-between gap-4 bg-ide-panel/80 border-b border-ide-border">
      <div className="flex items-center gap-2 min-w-0">
        <div className={`h-1.5 w-1.5 rounded-full ${isFocused ? "bg-primary" : "bg-ide-border"}`} />
        <span
          id={`editor-pane-${pane}-label`}
          className={`text-[10px] font-bold uppercase tracking-[0.28em] ${
            isFocused ? "text-primary" : "text-ide-text-dim"
          }`}
        >
          {paneLabel}
        </span>
      </div>

      <div className="flex items-center gap-1.5 min-w-0 text-[10px] text-ide-text-dim">
        {pathSegments.length > 0 ? (
          pathSegments.map((segment, index) => (
            <div key={`${segment}-${index}`} className="flex items-center gap-1.5 min-w-0">
              <span
                className={`truncate ${
                  index === pathSegments.length - 1 ? "text-ide-text font-medium" : "opacity-50"
                }`}
              >
                {segment}
              </span>
              {index < pathSegments.length - 1 && <span className="opacity-20">/</span>}
            </div>
          ))
        ) : (
          <span className="opacity-50">Focus this pane and open a file</span>
        )}
      </div>
    </div>
  );
}

function EmptyPane({ pane, isFocused }: { pane: EditorPaneId; isFocused: boolean }) {
  return (
    <div className="flex-1 flex items-center justify-center px-8 bg-[radial-gradient(circle_at_center,rgba(var(--color-primary-rgb),0.035)_0%,transparent_72%)]">
      <div className="max-w-xs text-center space-y-3">
        <div
          className={`inline-flex items-center rounded-full border px-3 py-1 text-[10px] font-bold uppercase tracking-[0.26em] ${
            isFocused
              ? "border-primary/25 bg-primary/10 text-primary"
              : "border-ide-border bg-ide-panel/70 text-ide-text-dim"
          }`}
        >
          {pane === "primary" ? "Primary" : "Secondary"}
        </div>
        <p className="text-sm text-ide-text">This pane is ready for another file.</p>
        <p className="text-[11px] leading-relaxed text-ide-text-dim">
          Focus it, then open something from the navigator, quick open, search or the tab strip above.
        </p>
      </div>
    </div>
  );
}

function LoadingPane({ pane }: { pane: EditorPaneId }) {
  return (
    <div className="flex-1 flex items-center justify-center px-8 bg-[radial-gradient(circle_at_center,rgba(var(--color-primary-rgb),0.03)_0%,transparent_70%)]">
      <div className="max-w-xs text-center space-y-3">
        <div className="inline-flex items-center gap-2 rounded-full border border-primary/20 bg-primary/8 px-3 py-1 text-[10px] font-bold uppercase tracking-[0.26em] text-primary">
          <span className="h-1.5 w-1.5 rounded-full bg-primary animate-pulse" />
          Loading {pane === "primary" ? "primary" : "secondary"}
        </div>
        <p className="text-sm text-ide-text">Preparing the editor model.</p>
        <p className="text-[11px] leading-relaxed text-ide-text-dim">
          Zenith is restoring the file with its cached editing state and diagnostics context.
        </p>
      </div>
    </div>
  );
}

function LoadErrorPane({ pane, error }: { pane: EditorPaneId; error: string }) {
  return (
    <div className="flex-1 flex items-center justify-center px-8 bg-[radial-gradient(circle_at_center,rgba(239,68,68,0.08)_0%,transparent_72%)]">
      <div className="max-w-md text-center space-y-3">
        <div className="inline-flex items-center rounded-full border border-red-500/20 bg-red-500/10 px-3 py-1 text-[10px] font-bold uppercase tracking-[0.26em] text-red-500">
          Failed to load {pane === "primary" ? "primary" : "secondary"} pane
        </div>
        <p className="text-sm text-ide-text">The editor could not read this file right now.</p>
        <p className="text-[11px] leading-relaxed text-ide-text-dim break-words">{error}</p>
      </div>
    </div>
  );
}

function EditorLanding() {
  return (
    <div className="flex-1 flex flex-col bg-ide-bg transition-colors duration-200 overflow-hidden relative">
      <div className="absolute inset-0 bg-[radial-gradient(circle_at_center,rgba(var(--color-primary-rgb),0.03)_0%,transparent_70%)] pointer-events-none" />

      <div className="flex-1 flex flex-col items-center justify-center text-ide-text-dim select-none z-10 animate-in fade-in zoom-in-95 duration-1000">
        <div className="relative group mb-8 flex justify-center">
          <div className="absolute inset-x-8 top-1/2 h-24 -translate-y-1/2 rounded-full bg-primary/12 blur-3xl" />
          <BrandLogo variant="vertical" className="relative h-32 w-auto opacity-95 transition-transform duration-300 group-hover:scale-[1.02]" alt="Zenith Keter" />
        </div>

        <div className="flex flex-col items-center gap-2 mb-12">
          <div className="h-px w-12 bg-primary/30 rounded-full" />
          <p className="text-[10px] uppercase tracking-[0.3em] font-bold text-primary/60">
            Elevating code toward clarity
          </p>
        </div>

        <div className="grid grid-cols-1 md:grid-cols-2 gap-x-12 gap-y-4 max-w-lg w-full px-8 opacity-60 hover:opacity-100 transition-opacity duration-300">
          <ShortcutHint label="Quick Open" combo={["Ctrl", "P"]} />
          <ShortcutHint label="Show Commands" combo={["Ctrl", "Shift", "P"]} />
          <ShortcutHint label="Split Right" combo={["Ctrl", "Alt", "V"]} />
          <ShortcutHint label="Focus Secondary" combo={["Alt", "2"]} />
        </div>

        <div className="mt-16 text-[9px] text-ide-text-dim/40 border-t border-ide-border/30 pt-4 flex items-center gap-4">
          <span className="flex items-center gap-1.5">
            <div className="w-1.5 h-1.5 rounded-full bg-primary/40" />
            v0.2-ALPHA
          </span>
          <span>OS: {navigator.platform}</span>
        </div>
      </div>
    </div>
  );
}

function ShortcutHint({ label, combo }: { label: string; combo: string[] }) {
  return (
    <div className="flex items-center justify-between text-[11px] group cursor-default">
      <span className="text-ide-text-dim group-hover:text-ide-text transition-colors">{label}</span>
      <div className="flex items-center gap-1 translate-y-[1px]">
        {combo.map((key, index) => (
          <span
            key={index}
            className="min-w-[1.4em] h-[1.4em] flex items-center justify-center px-1 rounded border border-ide-border bg-ide-panel/50 text-ide-text-dim font-mono text-[9px] shadow-[0_1px_0_rgba(255,255,255,0.05)]"
          >
            {key}
          </span>
        ))}
      </div>
    </div>
  );
}

function DiffEditorComponent({ 
  original, 
  modified, 
  language, 
  settings, 
  isZenMode,
  onMount 
}: any) {
  return (
    <DiffEditor
      height="100%"
      original={original}
      modified={modified}
      language={language}
      theme="zenith-dynamic"
      onMount={onMount}
      options={{
        minimap: { enabled: !isZenMode && settings.editorMinimap },
        fontSize: settings.editorFontSize,
        fontFamily: settings.editorFontFamily,
        cursorBlinking: settings.editorCursorBlinking,
        renderSideBySide: true,
        readOnly: false,
        automaticLayout: true,
        fixedOverflowWidgets: true,
        overflowWidgetsDomNode:
          typeof document !== "undefined" ? document.body : undefined,
        scrollBeyondLastLine: false,
        padding: { top: 12, bottom: 12 },
      }}
    />
  );
}

function getRelativeSegments(path: string, root: string) {
  const relativePath = path.startsWith(root) ? path.slice(root.length).replace(/^[\\/]/, "") : path;
  return relativePath.split(/[\\/]/).filter(Boolean);
}

function EditorComponent({
  file,
  settings,
  editorLanguage,
  content,
  handleEditorDidMount,
  handleEditorChange,
  isZenMode
}: any) {
  return (
    <Editor
      height="100%"
      path={file.path}
      defaultLanguage={editorLanguage}
      language={editorLanguage}
      defaultValue={content}
      saveViewState
      keepCurrentModel
      theme="zenith-dynamic"
      onMount={handleEditorDidMount}
      onChange={handleEditorChange}
      options={{
        minimap: { enabled: !isZenMode && settings.editorMinimap },
        fontSize: isZenMode ? settings.editorFontSize + 1 : settings.editorFontSize,
        fontFamily: settings.editorFontFamily,
        fontLigatures: settings.editorFontLigatures,
        cursorBlinking: settings.editorCursorBlinking,
        wordWrap: settings.editorWordWrap ? "on" : "off",
        lineNumbers: isZenMode ? "off" : "on",
        renderLineHighlight: isZenMode ? "none" : "all",
        roundedSelection: true,
        scrollBeyondLastLine: isZenMode ? true : false,
        cursorSurroundingLines: isZenMode ? 999 : 0,
        readOnly: false,
        automaticLayout: true,
        fixedOverflowWidgets: true,
        overflowWidgetsDomNode:
          typeof document !== "undefined" ? document.body : undefined,
        padding: { top: isZenMode ? 40 : 12, bottom: isZenMode ? 40 : 12 },
        glyphMargin: !isZenMode,
        folding: !isZenMode,
        scrollbar: {
          vertical: isZenMode ? "hidden" : "visible",
          horizontal: isZenMode ? "hidden" : "visible",
          useShadows: false,
          verticalSliderSize: 8,
          horizontalSliderSize: 8,
        },
      }}
    />
  );
}

/**
 * Helper to convert our string-based keymaps (ctrl+p) to Monaco keybindings.
 */
function parseComboToMonaco(monaco: any, combo: string): number | null {
  const parts = combo.split('+');
  let result = 0;

  for (const part of parts) {
    const p = part.toLowerCase().trim();
    if (p === 'ctrl') result |= monaco.KeyMod.CtrlCmd;
    else if (p === 'shift') result |= monaco.KeyMod.Shift;
    else if (p === 'alt') result |= monaco.KeyMod.Alt;
    else if (p === 'meta') result |= monaco.KeyMod.WinCtrl;
    else {
      const keyCodeName = `Key${p.toUpperCase()}`;
      if (monaco.KeyCode[keyCodeName]) {
        result |= monaco.KeyCode[keyCodeName];
      } else {
        const special: Record<string, any> = {
          'f5': monaco.KeyCode.F5,
          'f1': monaco.KeyCode.F1,
          'enter': monaco.KeyCode.Enter,
          'escape': monaco.KeyCode.Escape,
          'space': monaco.KeyCode.Space,
          'tab': monaco.KeyCode.Tab,
          ',': monaco.KeyCode.Comma,
          '.': monaco.KeyCode.Period,
          '/': monaco.KeyCode.Slash,
        };
        if (special[p]) {
          result |= special[p];
        } else {
          return null;
        }
      }
    }
  }

  return result > 0 ? result : null;
}


