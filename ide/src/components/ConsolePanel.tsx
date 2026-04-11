import { useEffect, useRef } from "react";
import {
  CheckCircle2,
  CircleStop,
  Hammer,
  Play,
  TerminalSquare,
  Trash2,
  XCircle,
} from "lucide-react";
import {
  PipelineConsoleEntry,
  PipelineConsoleEntryLevel,
  useExecutionStore,
} from "../store/useExecutionStore";

const LEVEL_STYLES: Record<PipelineConsoleEntryLevel, string> = {
  info: "text-sky-600 border-sky-500/20 bg-sky-500/5",
  output: "text-ide-text border-transparent bg-transparent",
  success: "text-emerald-600 border-emerald-500/20 bg-emerald-500/5",
  warning: "text-amber-600 border-amber-500/20 bg-amber-500/5",
  error: "text-red-500 border-red-500/20 bg-red-500/5",
};

export function ConsolePanel() {
  const { activeTask, consoleEntries, clearConsole } = useExecutionStore();
  const scrollRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const frame = window.requestAnimationFrame(() => {
      scrollRef.current?.scrollTo({
        top: scrollRef.current.scrollHeight,
        behavior: "smooth",
      });
    });

    return () => window.cancelAnimationFrame(frame);
  }, [consoleEntries.length]);

  return (
    <section className="flex h-full min-h-0 flex-col bg-ide-panel" aria-label="Task console">
      <div className="flex h-9 shrink-0 items-center justify-between border-b border-ide-border/70 px-4">
        <div className="flex items-center gap-2 min-w-0">
          <div
            className={`flex h-5 w-5 items-center justify-center rounded-md border ${
              activeTask
                ? "border-amber-500/20 bg-amber-500/10 text-amber-600"
                : "border-ide-border bg-ide-bg text-ide-text-dim"
            }`}
          >
            {activeTask?.action === "build" ? (
              <Hammer size={12} />
            ) : activeTask ? (
              <Play size={12} fill="currentColor" />
            ) : (
              <TerminalSquare size={12} />
            )}
          </div>

          <div className="min-w-0">
            <div className="truncate text-[10px] font-bold uppercase tracking-[0.18em] text-ide-text">
              {activeTask ? activeTask.label : "Task Console"}
            </div>
            <div className="truncate text-[10px] text-ide-text-dim">
              {activeTask
                ? `${activeTask.runtimeId} / ${activeTask.targetId}`
                : "Run and build output appears here"}
            </div>
          </div>
        </div>

        <button
          onClick={clearConsole}
          className="flex items-center gap-1.5 rounded-md px-2 py-1 text-[10px] font-semibold text-ide-text-dim transition-colors hover:bg-black/5 hover:text-ide-text active:scale-[0.98]"
          title="Clear console"
        >
          <Trash2 size={12} />
          Clear
        </button>
      </div>

      <div
        ref={scrollRef}
        role="log"
        aria-live="polite"
        className="flex-1 overflow-auto px-4 py-3"
      >
        {consoleEntries.length === 0 ? (
          <ConsoleEmptyState />
        ) : (
          <div className="flex flex-col gap-2">
            {consoleEntries.map((entry) => (
              <ConsoleEntryRow key={entry.id} entry={entry} />
            ))}
          </div>
        )}
      </div>
    </section>
  );
}

function ConsoleEmptyState() {
  return (
    <div className="flex h-full items-center justify-center">
      <div className="max-w-sm rounded-xl border border-dashed border-ide-border bg-ide-bg/40 px-6 py-5 text-center">
        <div className="mx-auto mb-3 flex h-9 w-9 items-center justify-center rounded-lg border border-ide-border bg-ide-panel text-ide-text-dim">
          <TerminalSquare size={16} />
        </div>
        <div className="text-[11px] font-bold uppercase tracking-[0.18em] text-ide-text">
          No task output yet
        </div>
        <p className="mt-2 text-[11px] leading-5 text-ide-text-dim">
          Use Run or Build from the toolbar. This panel will show a clean task log while the Terminal tab stays available as an interactive shell.
        </p>
      </div>
    </div>
  );
}

function ConsoleEntryRow({ entry }: { entry: PipelineConsoleEntry }) {
  const isOutput = entry.level === "output";

  return (
    <div
      className={`grid grid-cols-[72px_1fr] gap-3 rounded-lg border px-3 py-2 text-[11px] ${LEVEL_STYLES[entry.level]}`}
    >
      <div className="flex items-start gap-2 text-[10px] font-mono uppercase tracking-tight opacity-70">
        <ConsoleEntryIcon level={entry.level} />
        <span>{formatTime(entry.timestamp)}</span>
      </div>

      {isOutput ? (
        <pre className="min-w-0 whitespace-pre-wrap break-words font-mono text-[11px] leading-5 text-ide-text">
          {entry.text}
        </pre>
      ) : (
        <div className="min-w-0 whitespace-pre-wrap break-words font-medium leading-5">
          {entry.text}
        </div>
      )}
    </div>
  );
}

function ConsoleEntryIcon({ level }: { level: PipelineConsoleEntryLevel }) {
  if (level === "success") return <CheckCircle2 size={12} />;
  if (level === "warning") return <CircleStop size={12} />;
  if (level === "error") return <XCircle size={12} />;
  if (level === "output") return <TerminalSquare size={12} />;

  return <Play size={12} fill="currentColor" />;
}

function formatTime(timestamp: number) {
  return new Date(timestamp).toLocaleTimeString([], {
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
  });
}
