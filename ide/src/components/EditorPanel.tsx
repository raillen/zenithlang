import { useEffect, useState, useRef } from "react";
import Editor from "@monaco-editor/react";
import { useWorkspaceStore, FileEntry, Diagnostic } from "../store/useWorkspaceStore";
import { invoke } from "../utils/tauri";
import { TabManager } from "./TabManager";
import { registerZenithLanguage } from "../utils/zenithLang";
import { THEMES, defaultLight } from "../themes";
import { getMonacoTheme } from "../utils/themeEngine";
import { BrandLogo } from "./BrandLogo";

export function EditorPanel() {
  const { activeFile, setFileDirty, setDiagnostics, diagnosticsMap } = useWorkspaceStore();
  const [content, setContent] = useState("");
  const editorRef = useRef<any>(null);
  const monacoRef = useRef<any>(null);
  const { settings } = useWorkspaceStore();
  const currentThemeObj = THEMES[settings.theme] || defaultLight;

  useEffect(() => {
    if (monacoRef.current) {
      monacoRef.current.editor.defineTheme('zenith-dynamic', getMonacoTheme(currentThemeObj));
      monacoRef.current.editor.setTheme('zenith-dynamic');
    }
  }, [currentThemeObj]);

  useEffect(() => {
    async function loadFile() {
      if (activeFile) {
        const file = activeFile as FileEntry;
        try {
          const fileContent = await invoke<string>("read_file", { path: file.path });
          setContent(fileContent);
          setFileDirty(file.path, false);
        } catch (err) {
          console.error("Failed to read file:", err);
          setContent(`// Error loading file: ${err}`);
        }
      }
    }
    loadFile();
  }, [activeFile, setFileDirty]);

  // Diagnostics Debounce Effect
  useEffect(() => {
    if (!activeFile || !content || activeFile.is_directory) return;

    const timer = setTimeout(async () => {
      try {
        const results = await invoke<Diagnostic[]>("run_diagnostics", { 
          path: activeFile.path, 
          content 
        });
        setDiagnostics(activeFile.path, results);
      } catch (err) {
        console.error("Diagnostics error:", err);
      }
    }, 800); // 800ms debounce

    return () => clearTimeout(timer);
  }, [content, activeFile?.path, setDiagnostics]);

  // Update Monaco Markers when diagnostics for this file change
  useEffect(() => {
    if (editorRef.current && monacoRef.current && activeFile) {
      const markers = (diagnosticsMap[activeFile.path] || []).map(d => ({
        message: `${d.code}: ${d.message}`,
        severity: d.severity === 'error' 
          ? monacoRef.current.MarkerSeverity.Error 
          : d.severity === 'warning' 
            ? monacoRef.current.MarkerSeverity.Warning 
            : monacoRef.current.MarkerSeverity.Info,
        startLineNumber: d.line,
        startColumn: d.col,
        endLineNumber: d.line,
        endColumn: d.col + 1
      }));

      const model = editorRef.current.getModel();
      if (model) {
        monacoRef.current.editor.setModelMarkers(model, "zenith", markers);
      }
    }
  }, [diagnosticsMap, activeFile?.path]);

  const handleEditorDidMount = (editor: any, monaco: any) => {
    editorRef.current = editor;
    monacoRef.current = monaco;
    registerZenithLanguage(monaco);
    
    monaco.editor.defineTheme('zenith-dynamic', getMonacoTheme(currentThemeObj));
    monaco.editor.setTheme('zenith-dynamic');

    // Save Shortcut
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyS, async () => {
      const value = editor.getValue();
      const file = activeFile as FileEntry;
      if (file) {
        try {
          await invoke("write_file", { path: file.path, content: value });
          setFileDirty(file.path, false);
        } catch (err) {
          console.error("[Zenith IDE] Save Error:", err);
        }
      }
    });
  };

  const handleEditorChange = (value: string | undefined) => {
    if (activeFile) {
      setFileDirty(activeFile.path, true);
    }
    setContent(value || "");
  };

  // Jump to line logic
  useEffect(() => {
    const handleJump = (e: any) => {
        const { line, col } = e.detail;
        if (editorRef.current) {
            editorRef.current.revealLineInCenter(line);
            editorRef.current.setPosition({ lineNumber: line, column: col });
            editorRef.current.focus();
        }
    };

    window.addEventListener("zenith://editor-jump", handleJump);
    return () => window.removeEventListener("zenith://editor-jump", handleJump);
  }, []);

  if (!activeFile) {

    return (
      <div className="h-full flex flex-col bg-ide-bg transition-colors duration-200 overflow-hidden relative">
        <TabManager />
        
        {/* Background Gradient / Pattern */}
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
            <p className="text-[10px] uppercase tracking-[0.3em] font-bold text-primary/60">Crafting at the speed of thought</p>
          </div>

          {/* Shortcut Hints Grid */}
          <div className="grid grid-cols-1 md:grid-cols-2 gap-x-12 gap-y-4 max-w-lg w-full px-8 opacity-60 hover:opacity-100 transition-opacity duration-300">
            <ShortcutHint label="Quick Open" combo={["Ctrl", "P"]} />
            <ShortcutHint label="Show Commands" combo={["Ctrl", "Shift", "P"]} />
            <ShortcutHint label="Open Settings" combo={["Ctrl", ","]} />
            <ShortcutHint label="Open Folder" combo={["Ctrl", "Shift", "O"]} />
          </div>

          <div className="mt-16 text-[9px] text-ide-text-dim/40 border-t border-ide-border/30 pt-4 flex items-center gap-4">
            <span className="flex items-center gap-1.5"><div className="w-1.5 h-1.5 rounded-full bg-primary/40" /> v0.2-ALPHA</span>
            <span>OS: {navigator.platform}</span>
          </div>
        </div>
      </div>
    );
  }

  const file = activeFile as FileEntry;

  return (
    <div className="flex-1 flex flex-col h-full overflow-hidden bg-ide-bg transition-colors duration-200">
      <TabManager />
      
      {/* Breadcrumbs */}
      <div className="h-7 px-4 flex items-center bg-ide-panel border-b border-ide-border text-[10px] text-ide-text-dim font-medium transition-colors duration-200">
        <span className="opacity-50 hover:text-ide-text cursor-default">Project</span>
        <span className="mx-1.5 opacity-20">/</span>
        <span className="opacity-50 hover:text-ide-text cursor-default">src</span>
        <span className="mx-1.5 opacity-20">/</span>
        <span className="text-ide-text">{file.name}</span>
      </div>

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
              vertical: 'visible',
              horizontal: 'visible',
              useShadows: false,
              verticalSliderSize: 8,
              horizontalSliderSize: 8,
            }
          }}
        />
      </div>
    </div>
  );
}

function ShortcutHint({ label, combo }: { label: string, combo: string[] }) {
  return (
    <div className="flex items-center justify-between text-[11px] group cursor-default">
      <span className="text-ide-text-dim group-hover:text-ide-text transition-colors">{label}</span>
      <div className="flex items-center gap-1 translate-y-[1px]">
        {combo.map((key, i) => (
          <span key={i} className="min-w-[1.4em] h-[1.4em] flex items-center justify-center px-1 rounded border border-ide-border bg-ide-panel/50 text-ide-text-dim font-mono text-[9px] shadow-[0_1px_0_rgba(255,255,255,0.05)]">
            {key}
          </span>
        ))}
      </div>
    </div>
  );
}
