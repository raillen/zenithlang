import { useEffect, useState, useRef } from "react";
import Editor from "@monaco-editor/react";
import { useWorkspaceStore, FileEntry, Diagnostic } from "../store/useWorkspaceStore";
import { invoke } from "../utils/tauri";
import { TabManager } from "./TabManager";
import { registerZenithLanguage } from "../utils/zenithLang";

export function EditorPanel() {
  const { activeFile, setFileDirty, setDiagnostics, diagnosticsMap } = useWorkspaceStore();
  const [content, setContent] = useState("");
  const editorRef = useRef<any>(null);
  const monacoRef = useRef<any>(null);

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

  if (!activeFile) {
    return (
      <div className="flex-1 flex flex-col bg-white overflow-hidden">
        <TabManager />
        <div className="flex-1 flex flex-col items-center justify-center text-zinc-300 select-none">
          <div className="w-20 h-20 rounded-3xl bg-zinc-50 border border-zinc-100 flex items-center justify-center mb-6">
              <span className="text-5xl opacity-20 italic font-serif">Z</span>
          </div>
          <h2 className="text-lg font-bold tracking-tight text-zinc-400">Zenith IDE Retina</h2>
          <p className="text-[11px] mt-2 opacity-50 uppercase tracking-widest font-medium">Select a file to begin crafting</p>
        </div>
      </div>
    );
  }

  const file = activeFile as FileEntry;

  return (
    <div className="flex-1 flex flex-col h-full overflow-hidden bg-white">
      <TabManager />
      
      {/* Breadcrumbs */}
      <div className="h-7 px-4 flex items-center bg-zinc-50 border-b border-black/[0.03] text-[10px] text-zinc-400 font-medium">
        <span className="opacity-50 hover:text-zinc-600 cursor-default">Project</span>
        <span className="mx-1.5 opacity-20">/</span>
        <span className="opacity-50 hover:text-zinc-600 cursor-default">src</span>
        <span className="mx-1.5 opacity-20">/</span>
        <span className="text-zinc-600">{file.name}</span>
      </div>

      <div className="flex-1 overflow-hidden relative">
        <Editor
          height="100%"
          defaultLanguage="zenith"
          theme="vs-light" 
          value={content}
          onMount={handleEditorDidMount}
          onChange={handleEditorChange}
          options={{
            minimap: { enabled: false },
            fontSize: 13,
            fontFamily: "JetBrains Mono, Menlo, Monaco, 'Courier New', monospace",
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
