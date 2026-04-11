import { useEffect, useRef } from "react";
import { listen, UnlistenFn } from "@tauri-apps/api/event";
import { invoke } from "../utils/tauri";
import { useWorkspaceStore } from "../store/useWorkspaceStore";
import { useExecutionStore } from "../store/useExecutionStore";
import { THEMES, defaultLight } from "../themes";
import { Plus, Trash2, Terminal as TerminalIcon, X, Columns } from "lucide-react";
import { TerminalInstance } from "./TerminalInstance";

interface TerminalDataPayload {
  sessionId: number;
  data: string;
}

interface TerminalExitPayload {
  sessionId: number;
  exitCode?: number | null;
}

interface TerminalSessionInfo {
  sessionId: number;
  cwd: string;
  shell: string;
}

function getPathLeaf(path: string) {
  return path.split(/[\\/]/).filter(Boolean).pop() || "Workspace";
}

export function TerminalPanel() {
  const {
    sessions,
    activeGroupId,
    focusedSessionId,
    terminalOutput,
    setActiveGroup,
    setFocusedSession,
    addSession,
    removeSession,
    appendTerminalOutput,
    clearTerminalOutput,
    finishTaskForSession,
    appendConsoleChunkForSession,
    resetSessions,
  } = useExecutionStore();

  const { settings, currentProjectRoot } = useWorkspaceStore();
  const currentThemeObj = THEMES[settings.theme] || defaultLight;
  const previousProjectRootRef = useRef(currentProjectRoot);
  const isWorkspaceResettingRef = useRef(false);
  const workspaceResetTokenRef = useRef(0);

  // Grouping sessions for the tab bar
  const groupsRaw = sessions.reduce<Record<string, { id: string; sessions: typeof sessions; primaryLabel: string; primaryCwd: string }>>((acc, s) => {
    if (!acc[s.groupId]) {
      acc[s.groupId] = { id: s.groupId, sessions: [], primaryLabel: s.label, primaryCwd: s.cwd };
    }
    acc[s.groupId].sessions.push(s);
    return acc;
  }, {});

  const groups = Object.values(groupsRaw);

  const activeGroupSessions = sessions.filter(s => s.groupId === activeGroupId);

  // Global Listeners for Terminal Output
  useEffect(() => {
    let unlistenData: Promise<UnlistenFn>;
    let unlistenExit: Promise<UnlistenFn>;

    unlistenData = listen<TerminalDataPayload>("zenith://terminal-data", (event) => {
      const { sessionId, data } = event.payload;
      appendTerminalOutput(sessionId, data);
      appendConsoleChunkForSession(sessionId, data);
    });

    unlistenExit = listen<TerminalExitPayload>("zenith://terminal-exit", (event) => {
      const { sessionId, exitCode } = event.payload;
      removeSession(sessionId);
      clearTerminalOutput(sessionId);
      finishTaskForSession(sessionId, exitCode ?? 0);
    });

    return () => {
      unlistenData.then(f => f());
      unlistenExit.then(f => f());
    };
  }, []);

  const createNewSession = async (path?: string, label?: string, groupId?: string) => {
    const cwd = path || currentProjectRoot;
    if (!cwd || cwd === ".") return;

    try {
      const session = await invoke<TerminalSessionInfo>("terminal_create", {
        cwd,
      });

      addSession(
        session.sessionId,
        session.shell,
        label || getPathLeaf(cwd),
        groupId,
        session.cwd
      );
    } catch (error) {
      console.error("Failed to create terminal session", error);
    }
  };

  useEffect(() => {
    if (isWorkspaceResettingRef.current) return;
    if (!currentProjectRoot || currentProjectRoot === "." || sessions.length > 0) {
      return;
    }

    void createNewSession(currentProjectRoot, getPathLeaf(currentProjectRoot));
  }, [currentProjectRoot, sessions.length]);

  useEffect(() => {
    const previousRoot = previousProjectRootRef.current;
    previousProjectRootRef.current = currentProjectRoot;

    if (!currentProjectRoot || currentProjectRoot === "." || previousRoot === currentProjectRoot) {
      return;
    }

    if (sessions.length === 0) {
      return;
    }

    const token = workspaceResetTokenRef.current + 1;
    workspaceResetTokenRef.current = token;
    isWorkspaceResettingRef.current = true;

    const sessionIds = sessions.map((session) => session.id);
    resetSessions();

    void (async () => {
      await Promise.allSettled(
        sessionIds.map((sessionId) => invoke("terminal_kill", { sessionId }))
      );

      if (workspaceResetTokenRef.current !== token) return;

      isWorkspaceResettingRef.current = false;

      if (useWorkspaceStore.getState().currentProjectRoot !== currentProjectRoot) {
        return;
      }

      await createNewSession(currentProjectRoot, getPathLeaf(currentProjectRoot));
    })();
  }, [currentProjectRoot]);

  const handleKill = () => {
    if (focusedSessionId !== null) {
      void invoke("terminal_kill", { sessionId: focusedSessionId });
      removeSession(focusedSessionId);
    }
  };

  const handleSplit = () => {
    if (activeGroupId) {
        void createNewSession(undefined, undefined, activeGroupId);
    }
  };

  return (
    <section data-testid="terminal-panel" className="flex h-full min-h-0 flex-col bg-ide-panel">
      <div className="flex h-9 shrink-0 items-center justify-between border-b border-ide-border/70 bg-ide-panel">
        <div className="flex h-full min-w-0 overflow-x-auto no-scrollbar">
          {groups.map((g) => (
            <div
              key={g.id}
              onClick={() => setActiveGroup(g.id)}
              className={`
                group relative flex h-full min-w-[148px] max-w-[240px] cursor-pointer items-center gap-2 border-r border-ide-border px-4 text-[11px] transition-colors
                ${activeGroupId === g.id
                  ? "bg-ide-bg text-ide-text"
                  : "text-ide-text-dim hover:bg-black/5"
                }
              `}
              title={g.primaryCwd || g.primaryLabel}
            >
              {activeGroupId === g.id && <div className="absolute top-0 left-0 right-0 h-[2px] bg-primary" aria-hidden="true" />}

              <TerminalIcon size={12} className={`shrink-0 ${activeGroupId === g.id ? "text-primary" : "text-ide-text-dim/60"}`} />

              <div className="min-w-0 flex-1">
                <div className={`truncate ${activeGroupId === g.id ? "font-medium text-ide-text" : ""}`} title={g.primaryLabel}>
                  {g.primaryLabel}
                </div>
              </div>

              {g.sessions.length > 1 && (
                <span className="inline-flex h-4 min-w-4 items-center justify-center rounded-md bg-ide-panel px-1 text-[9px] font-bold text-ide-text-dim">
                  {g.sessions.length}
                </span>
              )}

              <div className={`flex items-center gap-0.5 transition-opacity ${activeGroupId === g.id ? "opacity-100" : "opacity-0 group-hover:opacity-100"}`}>
                <button
                  onClick={(e) => {
                    e.stopPropagation();
                    // Close all sessions in group
                    g.sessions.forEach(s => {
                        void invoke("terminal_kill", { sessionId: s.id });
                        removeSession(s.id);
                    });
                  }}
                  className="rounded-md p-1 text-ide-text-dim transition-colors hover:bg-red-500/10 hover:text-red-500"
                >
                  <X size={9} />
                </button>
              </div>
            </div>
          ))}

          <button
            data-testid="terminal-new-group"
            onClick={() => void createNewSession(currentProjectRoot)}
            className="flex h-full shrink-0 items-center justify-center border-r border-ide-border px-3 text-ide-text-dim transition-colors hover:bg-black/5 hover:text-primary"
            title="New Terminal Group"
          >
            <Plus size={14} />
          </button>
        </div>

        <div className="flex h-full items-center gap-1 px-2">
          <button
            data-testid="terminal-split"
            onClick={handleSplit}
            disabled={!activeGroupId}
            className="rounded-md p-1.5 text-primary/60 transition-colors hover:bg-primary/10 hover:text-primary disabled:opacity-30"
            title="Split Terminal"
          >
            <Columns size={14} />
          </button>
          <button
            onClick={handleKill}
            disabled={focusedSessionId === null}
            className="rounded-md p-1.5 text-red-500/60 transition-colors hover:bg-red-500/10 hover:text-red-500 disabled:opacity-30"
            title="Kill Focused Session"
          >
            <Trash2 size={14} />
          </button>
        </div>
      </div>

      <div className="flex-1 min-h-0 overflow-hidden bg-ide-bg">
        <div className="flex h-full min-h-0 overflow-hidden">
        {activeGroupSessions.map((s) => (
          <div
            key={s.id}
            className="relative h-full min-w-[220px] flex-1 border-r border-ide-border/60 last:border-r-0"
            onClick={() => setFocusedSession(s.id)}
          >
             <TerminalInstance
                session={s}
                output={terminalOutput[s.id] || ""}
                active={true} // In grouped view, all in active group are "rendered"
                focused={focusedSessionId === s.id}
                theme={currentThemeObj}
                onMount={() => {}}
             />
             {activeGroupSessions.length > 1 && (
                <div className="absolute top-2 right-2 z-20 flex items-center gap-2">
                    <span className={`rounded-md border px-1.5 py-0.5 text-[9px] ${
                        focusedSessionId === s.id ? "border-primary/30 bg-primary/10 text-primary" : "border-ide-border bg-ide-panel/90 text-ide-text-dim"
                    }`}>
                        {s.label}
                    </span>
                    <button
                         onClick={(e) => {
                            e.stopPropagation();
                            void invoke("terminal_kill", { sessionId: s.id });
                            removeSession(s.id);
                        }}
                        className="rounded-md p-1 text-ide-text-dim transition-colors hover:bg-red-500/10 hover:text-red-500"
                    >
                        <X size={10} />
                    </button>
                </div>
             )}
          </div>
        ))}

        {!activeGroupId && (
             <div className="flex flex-1 items-center justify-center text-ide-text-dim/20">
                <TerminalIcon size={48} strokeWidth={1} />
             </div>
        )}
        </div>
      </div>
    </section>
  );
}
