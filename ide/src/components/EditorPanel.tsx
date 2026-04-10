import { useEffect, useRef, useState } from "react";
import Editor from "@monaco-editor/react";
import { Columns2, Rows3, Square } from "lucide-react";
import { Group, Panel, Separator } from "react-resizable-panels";
import {
  useWorkspaceStore,
  Diagnostic,
  EditorPane as EditorPaneId,
  EditorSplitMode,
  FileEntry,
} from "../store/useWorkspaceStore";
import { invoke } from "../utils/tauri";
import { TabManager } from "./TabManager";
import { registerZenithLanguage } from "../utils/zenithLang";
import { THEMES, defaultLight } from "../themes";
import { getMonacoTheme } from "../utils/themeEngine";
import { BrandLogo } from "./BrandLogo";

export function EditorPanel() {
  const {
    primaryFile,
    secondaryFile,
    splitMode,
    focusedPane,
    setFocusedPane,
    setSplitMode,
  } = useWorkspaceStore();

  const showLanding = !primaryFile && !secondaryFile && splitMode === "single";

  return (
    <div className="flex-1 flex flex-col h-full overflow-hidden bg-ide-bg transition-colors duration-200">
      <TabManager />
      <SplitControls splitMode={splitMode} onChange={setSplitMode} />

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
  const { settings, diagnosticsMap, setFileDirty, setDiagnostics, currentProjectRoot } =
    useWorkspaceStore();
  const [content, setContent] = useState("");
  const editorRef = useRef<any>(null);
  const monacoRef = useRef<any>(null);
  const fileRef = useRef<FileEntry | null>(file);
  const currentThemeObj = THEMES[settings.theme] || defaultLight;
  const pathSegments = file ? getRelativeSegments(file.path, currentProjectRoot) : [];

  useEffect(() => {
    fileRef.current = file;
  }, [file]);

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
        return;
      }

      try {
        const fileContent = await invoke<string>("read_file", { path: file.path });
        if (!isMounted) return;
        setContent(fileContent);
        setFileDirty(file.path, false);
      } catch (err) {
        if (!isMounted) return;
        console.error("Failed to read file:", err);
        setContent(`// Error loading file: ${err}`);
      }
    }

    loadFile();
    return () => {
      isMounted = false;
    };
  }, [file?.path, setFileDirty]);

  useEffect(() => {
    if (!file || !content || file.is_directory) return;

    const timer = window.setTimeout(async () => {
      try {
        const results = await invoke<Diagnostic[]>("run_diagnostics", {
          path: file.path,
          content,
        });
        setDiagnostics(file.path, results);
      } catch (err) {
        console.error("Diagnostics error:", err);
      }
    }, 800);

    return () => window.clearTimeout(timer);
  }, [content, file?.path, setDiagnostics]);

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

  useEffect(() => {
    if (isFocused) {
      editorRef.current?.focus();
    }
  }, [isFocused, file?.path]);

  useEffect(() => {
    const handleJump = (event: Event) => {
      if (!isFocused || !editorRef.current) return;

      const detail = (event as CustomEvent<{ line: number; col: number }>).detail;
      editorRef.current.revealLineInCenter(detail.line);
      editorRef.current.setPosition({ lineNumber: detail.line, column: detail.col });
      editorRef.current.focus();
    };

    window.addEventListener("zenith://editor-jump", handleJump as EventListener);
    return () => window.removeEventListener("zenith://editor-jump", handleJump as EventListener);
  }, [isFocused]);

  const handleEditorDidMount = (editor: any, monaco: any) => {
    editorRef.current = editor;
    monacoRef.current = monaco;
    registerZenithLanguage(monaco);

    monaco.editor.defineTheme("zenith-dynamic", getMonacoTheme(currentThemeObj));
    monaco.editor.setTheme("zenith-dynamic");

    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyS, async () => {
      const currentFile = fileRef.current;
      if (!currentFile) return;

      try {
        await invoke("write_file", { path: currentFile.path, content: editor.getValue() });
        setFileDirty(currentFile.path, false);
      } catch (err) {
        console.error("[Zenith IDE] Save Error:", err);
      }
    });
  };

  const handleEditorChange = (value: string | undefined) => {
    const currentFile = fileRef.current;
    if (currentFile) {
      setFileDirty(currentFile.path, true);
    }
    setContent(value || "");
  };

  return (
    <div
      onMouseDown={onFocus}
      className={`flex h-full flex-col overflow-hidden bg-ide-bg transition-all duration-200 ${
        isFocused ? "ring-1 ring-inset ring-primary/25" : ""
      }`}
    >
      <PaneHeader pane={pane} isFocused={isFocused} pathSegments={pathSegments} />

      {file ? (
        <div className="flex-1 overflow-hidden relative">
          <Editor
            height="100%"
            defaultLanguage="zenith"
            theme="zenith-dynamic"
            value={content}
            onMount={handleEditorDidMount}
            onChange={handleEditorChange}
            options={{
              minimap: { enabled: settings.editorMinimap },
              fontSize: settings.editorFontSize,
              fontFamily: settings.editorFontFamily,
              fontLigatures: settings.editorFontLigatures,
              cursorBlinking: settings.editorCursorBlinking,
              wordWrap: settings.editorWordWrap ? "on" : "off",
              lineNumbers: "on",
              roundedSelection: true,
              scrollBeyondLastLine: false,
              readOnly: false,
              automaticLayout: true,
              padding: { top: 12 },
              glyphMargin: false,
              folding: true,
              renderLineHighlight: "all",
              scrollbar: {
                vertical: "visible",
                horizontal: "visible",
                useShadows: false,
                verticalSliderSize: 8,
                horizontalSliderSize: 8,
              },
            }}
          />
        </div>
      ) : (
        <EmptyPane pane={pane} isFocused={isFocused} />
      )}
    </div>
  );
}

function SplitControls({
  splitMode,
  onChange,
}: {
  splitMode: EditorSplitMode;
  onChange: (mode: EditorSplitMode) => void;
}) {
  return (
    <div className="h-10 px-3 border-b border-ide-border bg-ide-panel/80 flex items-center justify-between gap-3 backdrop-blur-sm">
      <div className="min-w-0">
        <div className="text-[10px] font-bold uppercase tracking-[0.32em] text-ide-text-dim">
          Editor Workspace
        </div>
        <div className="text-[10px] text-ide-text-dim/70 truncate">
          {splitMode === "single"
            ? "Single editor"
            : "Tabs, search and quick open follow the focused pane"}
        </div>
      </div>

      <div className="flex items-center gap-1 rounded-xl border border-ide-border bg-ide-bg/80 p-1">
        <SplitButton
          icon={<Square size={14} />}
          label="Single"
          active={splitMode === "single"}
          onClick={() => onChange("single")}
        />
        <SplitButton
          icon={<Columns2 size={14} />}
          label="Columns"
          active={splitMode === "vertical"}
          onClick={() => onChange("vertical")}
        />
        <SplitButton
          icon={<Rows3 size={14} />}
          label="Rows"
          active={splitMode === "horizontal"}
          onClick={() => onChange("horizontal")}
        />
      </div>
    </div>
  );
}

function SplitButton({
  icon,
  label,
  active,
  onClick,
}: {
  icon: React.ReactNode;
  label: string;
  active: boolean;
  onClick: () => void;
}) {
  return (
    <button
      onClick={onClick}
      title={label}
      className={`flex h-8 w-8 items-center justify-center rounded-lg transition-colors ${
        active
          ? "bg-primary/12 text-primary shadow-sm ring-1 ring-primary/20"
          : "text-ide-text-dim hover:bg-black/5 hover:text-ide-text"
      }`}
    >
      {icon}
    </button>
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
  return (
    <div className="h-9 px-3 flex items-center justify-between gap-4 bg-ide-panel/80 border-b border-ide-border">
      <div className="flex items-center gap-2">
        <div className={`h-1.5 w-1.5 rounded-full ${isFocused ? "bg-primary" : "bg-ide-border"}`} />
        <span
          className={`text-[10px] font-bold uppercase tracking-[0.28em] ${
            isFocused ? "text-primary" : "text-ide-text-dim"
          }`}
        >
          {pane === "primary" ? "Primary Pane" : "Secondary Pane"}
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

function EditorLanding() {
  return (
    <div className="flex-1 flex flex-col bg-ide-bg transition-colors duration-200 overflow-hidden relative">
      <div className="absolute inset-0 bg-[radial-gradient(circle_at_center,rgba(var(--color-primary-rgb),0.03)_0%,transparent_70%)] pointer-events-none" />

      <div className="flex-1 flex flex-col items-center justify-center text-ide-text-dim select-none z-10 animate-in fade-in zoom-in-95 duration-1000">
        <div className="relative group mb-8">
          <div className="absolute -inset-4 bg-primary/20 rounded-full blur-2xl opacity-20 group-hover:opacity-40 transition-opacity duration-500" />
          <div className="relative flex h-24 w-24 items-center justify-center rounded-[2rem] border border-ide-border bg-ide-panel/80 shadow-2xl backdrop-blur-xl">
            <BrandLogo variant="icon" className="h-12 w-12 drop-shadow-lg" />
          </div>
        </div>

        <div className="flex flex-col items-center gap-2 mb-12">
          <BrandLogo variant="lockup" className="h-10 w-auto opacity-90" alt="Zenith IDE Retina" />
          <div className="h-px w-12 bg-primary/30 rounded-full" />
          <p className="text-[10px] uppercase tracking-[0.3em] font-bold text-primary/60">
            Crafting at the speed of thought
          </p>
        </div>

        <div className="grid grid-cols-1 md:grid-cols-2 gap-x-12 gap-y-4 max-w-lg w-full px-8 opacity-60 hover:opacity-100 transition-opacity duration-300">
          <ShortcutHint label="Quick Open" combo={["Ctrl", "P"]} />
          <ShortcutHint label="Show Commands" combo={["Ctrl", "Shift", "P"]} />
          <ShortcutHint label="Open Settings" combo={["Ctrl", ","]} />
          <ShortcutHint label="Open Folder" combo={["Ctrl", "Shift", "O"]} />
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

function getRelativeSegments(path: string, root: string) {
  const relativePath = path.startsWith(root) ? path.slice(root.length).replace(/^[\\/]/, "") : path;
  return relativePath.split(/[\\/]/).filter(Boolean);
}
