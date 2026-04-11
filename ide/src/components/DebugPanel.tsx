import { useEffect, useMemo, useState } from "react";
import {
  Bug,
  ChevronDown,
  Circle,
  Database,
  Hash,
  Layers,
  Play,
  Plus,
  Square,
  Trash2,
  X,
} from "lucide-react";
import { invoke } from "../utils/tauri";
import { useWorkspaceStore, Symbol } from "../store/useWorkspaceStore";
import { useExecutionStore } from "../store/useExecutionStore";
import { executePipelineAction, stopActivePipelineTask } from "../utils/pipelineActions";

const WATCH_STORAGE_KEY = "zenith:debug-watches:v1";

type StackFrame = {
  label: string;
  path: string;
  line: number;
  col: number;
};

export function DebugPanel() {
  const {
    breakpoints,
    clearBreakpoints,
    openFile,
    activeFile,
    fileBuffers,
    cursorPosition,
    diagnosticsMap,
  } = useWorkspaceStore();
  const { activeTask, consoleEntries, lastExitCode } = useExecutionStore();

  const [symbols, setSymbols] = useState<Symbol[]>([]);
  const [watchInput, setWatchInput] = useState("");
  const [watches, setWatches] = useState<string[]>(() => {
    try {
      const raw = window.localStorage.getItem(WATCH_STORAGE_KEY);
      return raw ? JSON.parse(raw) : [];
    } catch {
      return [];
    }
  });

  const currentContent = activeFile ? fileBuffers[activeFile.path] || "" : "";
  const diagnostics = activeFile ? diagnosticsMap[activeFile.path] || [] : [];

  useEffect(() => {
    try {
      window.localStorage.setItem(WATCH_STORAGE_KEY, JSON.stringify(watches));
    } catch {
      // Ignore persistence errors for local-only debug preferences.
    }
  }, [watches]);

  useEffect(() => {
    let cancelled = false;

    async function loadSymbols() {
      if (!activeFile || !currentContent) {
        setSymbols([]);
        return;
      }

      try {
        const nextSymbols = await invoke<Symbol[]>("get_file_symbols", { content: currentContent });
        if (!cancelled) setSymbols(nextSymbols);
      } catch (error) {
        if (!cancelled) {
          console.error("Failed to read symbols for debug panel", error);
          setSymbols([]);
        }
      }
    }

    void loadSymbols();
    return () => {
      cancelled = true;
    };
  }, [activeFile?.path, currentContent]);

  const stackFrames = useMemo(
    () => parseStackFrames(consoleEntries, activeTask?.taskId),
    [consoleEntries, activeTask?.taskId]
  );

  const variablePreview = useMemo(
    () =>
      symbols
        .filter((symbol) => symbol.kind === "Variable" || symbol.kind === "Constant")
        .slice(0, 10),
    [symbols]
  );

  const watchResults = useMemo(
    () => watches.map((watch) => ({ watch, value: evaluateWatch(watch, currentContent, symbols) })),
    [currentContent, symbols, watches]
  );

  const runLabel = activeTask
    ? activeTask.action === "build"
      ? "Build running"
      : "Run session active"
    : lastExitCode === 0
      ? "Last session succeeded"
      : typeof lastExitCode === "number"
        ? "Last session finished"
        : "Ready";

  return (
    <div className="flex h-full flex-col overflow-hidden bg-ide-panel select-none">
      <div className="px-4 py-3 border-b border-ide-border bg-ide-bg/20">
        <div className="flex items-center justify-between mb-3">
          <div className="flex items-center gap-2">
            <Bug size={14} className="text-primary" />
            <h3 className="text-[11px] font-bold uppercase tracking-[0.2em] text-ide-text">Debug Center</h3>
          </div>
          <div className="flex items-center gap-1">
            <DebugControlIcon
              icon={activeTask ? <Square size={12} fill="currentColor" /> : <Play size={12} fill="currentColor" />}
              className={activeTask ? "text-red-500 bg-red-500/10" : "text-emerald-500 bg-emerald-500/10"}
              onClick={activeTask ? () => void stopActivePipelineTask() : () => void executePipelineAction("run")}
            />
          </div>
        </div>

        <div className="rounded-xl border border-ide-border bg-ide-bg/60 px-3 py-2">
          <div className="text-[9px] font-bold uppercase tracking-[0.18em] text-ide-text-dim">Session</div>
          <div className="mt-1 text-[11px] font-semibold text-ide-text">{runLabel}</div>
          <div className="mt-1 text-[10px] text-ide-text-dim">
            {activeTask
              ? `${activeTask.runtimeId} / ${activeTask.targetId}`
              : activeFile
                ? activeFile.name
                : "Open a file to start a run session"}
          </div>
        </div>
      </div>

      <div className="flex-1 overflow-y-auto no-scrollbar py-2">
        <DebugSection title="Variables" icon={<Database size={12} />}>
          <div className="px-4 py-2 space-y-2">
            <DebugMetaRow label="Cursor" value={cursorPosition ? `L${cursorPosition.line}:${cursorPosition.col}` : "No cursor"} />
            <DebugMetaRow label="Diagnostics" value={diagnostics.length ? `${diagnostics.length} issue(s)` : "Clean"} />
            <DebugMetaRow label="Symbols" value={symbols.length ? `${symbols.length} discovered` : "No symbols"} />

            {variablePreview.length === 0 ? (
              <EmptyDebugState label="No variables found in the active buffer." />
            ) : (
              variablePreview.map((symbol) => (
                <button
                  key={`${symbol.name}-${symbol.line}`}
                  onClick={() => jumpToLocation(openFile, activeFile, symbol.line, symbol.col)}
                  className="w-full rounded-lg border border-ide-border bg-ide-bg/50 px-3 py-2 text-left hover:bg-black/5 transition-colors"
                >
                  <div className="flex items-center justify-between gap-3">
                    <span className="text-[11px] font-semibold text-ide-text">{symbol.name}</span>
                    <span className="text-[9px] font-mono text-ide-text-dim">
                      {symbol.kind} / L{symbol.line}
                    </span>
                  </div>
                </button>
              ))
            )}
          </div>
        </DebugSection>

        <DebugSection title="Watch" icon={<Hash size={12} />}>
          <div className="px-4 py-2 space-y-2">
            <form
              onSubmit={(event) => {
                event.preventDefault();
                const value = watchInput.trim();
                if (!value) return;
                setWatches((current) => (current.includes(value) ? current : [...current, value]));
                setWatchInput("");
              }}
              className="flex items-center gap-2"
            >
              <input
                value={watchInput}
                onChange={(event) => setWatchInput(event.target.value)}
                placeholder="Add watch expression"
                className="flex-1 rounded-lg border border-ide-border bg-ide-bg px-2 py-1.5 text-[11px] text-ide-text outline-none"
              />
              <button
                type="submit"
                className="flex h-8 w-8 items-center justify-center rounded-lg bg-primary/10 text-primary transition-colors hover:bg-primary/15"
              >
                <Plus size={12} />
              </button>
            </form>

            {watchResults.length === 0 ? (
              <EmptyDebugState label="Add expressions to keep an eye on declarations in the current buffer." />
            ) : (
              watchResults.map(({ watch, value }) => (
                <div key={watch} className="rounded-lg border border-ide-border bg-ide-bg/50 px-3 py-2">
                  <div className="flex items-center justify-between gap-3">
                    <span className="text-[11px] font-semibold text-ide-text">{watch}</span>
                    <button
                      onClick={() => setWatches((current) => current.filter((item) => item !== watch))}
                      className="text-ide-text-dim hover:text-red-500 transition-colors"
                    >
                      <X size={12} />
                    </button>
                  </div>
                  <div className="mt-1 text-[10px] font-mono text-ide-text-dim">{value}</div>
                </div>
              ))
            )}
          </div>
        </DebugSection>

        <DebugSection title="Call Stack" icon={<Layers size={12} />}>
          <div className="px-4 py-2 space-y-2">
            {stackFrames.length === 0 ? (
              <EmptyDebugState label="No stack frames captured yet. Run something and Zenith will mine the console for trace locations." />
            ) : (
              stackFrames.map((frame, index) => (
                <button
                  key={`${frame.path}-${frame.line}-${index}`}
                  onClick={() => openStackFrame(openFile, frame)}
                  className="w-full rounded-lg border border-ide-border bg-ide-bg/50 px-3 py-2 text-left hover:bg-black/5 transition-colors"
                >
                  <div className="text-[11px] font-semibold text-ide-text truncate">{frame.label}</div>
                  <div className="mt-1 text-[9px] font-mono text-ide-text-dim truncate">
                    {frame.path} : {frame.line}:{frame.col}
                  </div>
                </button>
              ))
            )}
          </div>
        </DebugSection>

        <DebugSection
          title="Breakpoints"
          icon={<Circle size={12} className="fill-red-500 text-red-500" />}
          action={
            <button
              onClick={() => clearBreakpoints()}
              className="hover:text-red-500 transition-colors"
            >
              <Trash2 size={12} />
            </button>
          }
        >
          {breakpoints.length === 0 ? (
            <div className="px-4 py-2 opacity-40 italic text-[10px] text-ide-text-dim">
              No breakpoints set.
            </div>
          ) : (
            <div className="pt-1">
              {breakpoints.map((bp, index) => (
                <div
                  key={`${bp.path}-${bp.line}-${index}`}
                  onClick={() => {
                    const name = bp.path.split(/[\\/]/).pop() || "file";
                    openFile({ name, path: bp.path, is_directory: false }, true);
                    setTimeout(() => {
                      window.dispatchEvent(
                        new CustomEvent("zenith://editor-jump", {
                          detail: { line: bp.line, col: 1 },
                        })
                      );
                    }, 80);
                  }}
                  className="flex items-center gap-3 px-4 py-1.5 hover:bg-black/5 cursor-pointer group"
                >
                  <Circle size={8} className="fill-red-500 text-red-500 shrink-0" />
                  <div className="flex-1 min-w-0">
                    <div className="text-[11px] text-ide-text truncate">
                      {bp.path.split(/[\\/]/).pop()}
                    </div>
                    <div className="text-[9px] text-ide-text-dim/60 font-mono tracking-tighter">
                      Line {bp.line}
                    </div>
                  </div>
                </div>
              ))}
            </div>
          )}
        </DebugSection>
      </div>
    </div>
  );
}

function DebugSection({
  title,
  icon,
  children,
  action,
}: {
  title: string;
  icon: React.ReactNode;
  children: React.ReactNode;
  action?: React.ReactNode;
}) {
  const [isOpen, setIsOpen] = useState(true);

  return (
    <div className="border-b border-ide-border/30 last:border-0 pb-1">
      <div
        className="flex items-center justify-between px-2 py-1.5 hover:bg-black/5 cursor-pointer group"
        onClick={() => setIsOpen(!isOpen)}
      >
        <div className="flex items-center gap-2">
          <div className="text-ide-text-dim/40">{isOpen ? <ChevronDown size={14} /> : <Layers size={14} />}</div>
          <span className="text-[10px] font-bold uppercase tracking-wider text-ide-text-dim/80">
            {title}
          </span>
        </div>
        <div className="flex items-center gap-2 pr-2">
          {action}
          <div className="text-ide-text-dim/30">{icon}</div>
        </div>
      </div>
      {isOpen && children}
    </div>
  );
}

function DebugControlIcon({
  icon,
  className,
  onClick,
}: {
  icon: React.ReactNode;
  className?: string;
  onClick: () => void;
}) {
  return (
    <button
      onClick={onClick}
      className={`w-6 h-6 flex items-center justify-center rounded-lg transition-all active:scale-95 ${className}`}
    >
      {icon}
    </button>
  );
}

function DebugMetaRow({ label, value }: { label: string; value: string }) {
  return (
    <div className="flex items-center justify-between rounded-lg border border-ide-border bg-ide-bg/50 px-3 py-2">
      <span className="text-[10px] font-bold uppercase tracking-wider text-ide-text-dim">{label}</span>
      <span className="text-[10px] text-ide-text">{value}</span>
    </div>
  );
}

function EmptyDebugState({ label }: { label: string }) {
  return <div className="rounded-lg border border-dashed border-ide-border px-3 py-2 text-[10px] italic text-ide-text-dim/70">{label}</div>;
}

function evaluateWatch(watch: string, content: string, symbols: Symbol[]) {
  if (!content) return "No active buffer";

  const symbol = symbols.find((item) => item.name === watch);
  if (symbol) {
    return `${symbol.kind} declared on line ${symbol.line}`;
  }

  const escaped = watch.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
  const patterns = [
    new RegExp(`\\b(?:const|let|var|local)\\s+${escaped}\\b[^=\\n]*=\\s*(.+)$`, "m"),
    new RegExp(`\\b${escaped}\\s*[:=]\\s*(.+)$`, "m"),
  ];

  for (const pattern of patterns) {
    const match = content.match(pattern);
    if (match?.[1]) {
      return match[1].trim().slice(0, 120);
    }
  }

  return "Not found in current buffer";
}

function parseStackFrames(entries: Array<{ taskId?: string; text: string }>, taskId?: string) {
  const patterns = [
    /^\s*at\s+(.+?)\s+\((.+?):(\d+):(\d+)\)$/m,
    /^\s*at\s+(.+?):(\d+):(\d+)$/m,
    /-->\s+(.+?):(\d+):(\d+)/m,
    /\b(.+?):(\d+):(\d+)\b/m,
  ];

  const frames: StackFrame[] = [];
  const seen = new Set<string>();
  const relevantEntries = taskId ? entries.filter((entry) => entry.taskId === taskId || entry.taskId === undefined) : entries;

  for (const entry of relevantEntries) {
    const lines = entry.text.split(/\r?\n/);
    for (const rawLine of lines) {
      for (const pattern of patterns) {
        const match = rawLine.match(pattern);
        if (!match) continue;

        const [, maybeLabel, maybePath, maybeLine, maybeCol] = match;
        const path = maybeCol ? maybePath : maybeLabel;
        const line = Number(maybeCol ? maybeLine : maybePath);
        const col = Number(maybeCol || maybeLine);
        const label = maybeCol ? maybeLabel : rawLine.trim();
        const key = `${path}:${line}:${col}`;
        if (seen.has(key)) continue;

        seen.add(key);
        frames.push({ label, path, line, col });
        break;
      }
    }
  }

  return frames.slice(0, 12);
}

function jumpToLocation(
  openFile: (file: { name: string; path: string; is_directory: false }, isFixed?: boolean) => void,
  activeFile: { path: string; name: string } | null,
  line: number,
  col: number
) {
  if (!activeFile) return;
  openFile({ name: activeFile.name, path: activeFile.path, is_directory: false }, true);
  setTimeout(() => {
    window.dispatchEvent(new CustomEvent("zenith://editor-jump", { detail: { line, col } }));
  }, 80);
}

function openStackFrame(
  openFile: (file: { name: string; path: string; is_directory: false }, isFixed?: boolean) => void,
  frame: StackFrame
) {
  const name = frame.path.split(/[\\/]/).pop() || "file";
  openFile({ name, path: frame.path, is_directory: false }, true);
  setTimeout(() => {
    window.dispatchEvent(
      new CustomEvent("zenith://editor-jump", {
        detail: { line: frame.line, col: frame.col },
      })
    );
  }, 80);
}
