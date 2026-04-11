import { create } from 'zustand';
import { RuntimeProfileId } from '../utils/runtimeProfiles';

export type PipelineAction = 'run' | 'build';
export type PipelineConsoleEntryLevel = 'info' | 'output' | 'success' | 'warning' | 'error';

export interface ActivePipelineTask {
  taskId: string;
  sessionId: number;
  action: PipelineAction;
  runtimeId: RuntimeProfileId;
  targetId: string;
  label: string;
  command: string;
}

export interface PipelineConsoleEntry {
  id: string;
  taskId?: string;
  level: PipelineConsoleEntryLevel;
  text: string;
  timestamp: number;
}

export interface TerminalSessionMeta {
  id: number;
  shell: string;
  label: string;
  cwd: string;
  isReady: boolean;
  groupId: string;
}

interface ExecutionState {
  sessions: TerminalSessionMeta[];
  activeSessionId: number | null; // Keep for backward compat, but we'll use focusedSessionId mostly
  activeGroupId: string | null;
  focusedSessionId: number | null;
  pendingCommands: Record<number, string[]>; // Queued commands per session
  terminalOutput: Record<number, string>;
  activeTask: ActivePipelineTask | null;
  selectedTargets: Partial<Record<RuntimeProfileId, string>>;
  lastExitCode: number | null;
  consoleEntries: PipelineConsoleEntry[];

  addSession: (id: number, shell: string, label?: string, groupId?: string, cwd?: string) => void;
  renameSession: (id: number, label: string) => void;
  removeSession: (id: number) => void;
  resetSessions: () => void;
  setActiveSession: (id: number | null) => void;
  setActiveGroup: (groupId: string | null) => void;
  setFocusedSession: (sessionId: number | null) => void;
  setSessionReady: (id: number, ready: boolean) => void;
  enqueueCommand: (sessionId: number, command: string) => void;
  consumeNextCommand: (sessionId: number) => string | null;
  clearPendingCommands: (sessionId: number) => void;
  appendTerminalOutput: (sessionId: number, text: string) => void;
  clearTerminalOutput: (sessionId: number) => void;
  closeActiveSession: () => void;
  startTask: (task: ActivePipelineTask) => void;
  finishTask: (taskId: string, exitCode: number | null) => void;
  finishTaskForSession: (sessionId: number, exitCode: number | null) => void;
  clearActiveTask: () => void;
  setSelectedTarget: (runtimeId: RuntimeProfileId, targetId: string) => void;
  appendConsoleEntry: (entry: Omit<PipelineConsoleEntry, 'id' | 'timestamp'>) => void;
  appendConsoleChunk: (taskId: string | undefined, text: string) => void;
  appendConsoleChunkForSession: (sessionId: number, text: string) => void;
  clearConsole: () => void;
}

const SELECTED_TARGETS_STORAGE_KEY = 'zenith:pipeline-targets:v1';
const MAX_CONSOLE_ENTRIES = 500;

function createConsoleEntry(
  entry: Omit<PipelineConsoleEntry, 'id' | 'timestamp'>
): PipelineConsoleEntry {
  return {
    ...entry,
    id: `${Date.now()}-${Math.random().toString(36).slice(2, 8)}`,
    timestamp: Date.now(),
  };
}

function pushConsoleEntry(
  entries: PipelineConsoleEntry[],
  entry: Omit<PipelineConsoleEntry, 'id' | 'timestamp'>
) {
  return [...entries, createConsoleEntry(entry)].slice(-MAX_CONSOLE_ENTRIES);
}

function getStorage() {
  return typeof window === 'undefined' ? null : window.localStorage;
}

function loadSelectedTargets(): Partial<Record<RuntimeProfileId, string>> {
  const storage = getStorage();
  if (!storage) return {};

  try {
    const raw = storage.getItem(SELECTED_TARGETS_STORAGE_KEY);
    if (!raw) return {};

    const parsed = JSON.parse(raw);
    if (!parsed || typeof parsed !== 'object') return {};

    return parsed as Partial<Record<RuntimeProfileId, string>>;
  } catch (error) {
    console.error('Failed to restore pipeline targets', error);
    return {};
  }
}

function persistSelectedTargets(targets: Partial<Record<RuntimeProfileId, string>>) {
  const storage = getStorage();
  if (!storage) return;

  try {
    storage.setItem(SELECTED_TARGETS_STORAGE_KEY, JSON.stringify(targets));
  } catch (error) {
    console.error('Failed to persist pipeline targets', error);
  }
}

function describeExitCode(exitCode: number | null) {
  if (exitCode === 0) return 'completed successfully';
  if (exitCode === 130) return 'stopped by user';
  if (typeof exitCode === 'number') return `failed with exit code ${exitCode}`;
  return 'finished with unknown status';
}

export const useExecutionStore = create<ExecutionState>((set, get) => ({
  sessions: [],
  activeSessionId: null,
  activeGroupId: null,
  focusedSessionId: null,
  pendingCommands: {},
  terminalOutput: {},
  activeTask: null,
  selectedTargets: loadSelectedTargets(),
  lastExitCode: null,
  consoleEntries: [],

  addSession: (id, shell, label, groupId, cwd) => set((state) => {
    const finalGroupId = groupId || crypto.randomUUID();
    const derivedLabel =
      label ||
      cwd?.split(/[\\/]/).filter(Boolean).pop() ||
      `Terminal ${state.sessions.length + 1}`;
    const newSession = {
      id,
      shell,
      label: derivedLabel,
      cwd: cwd || "",
      isReady: true,
      groupId: finalGroupId
    };

    return {
      sessions: [...state.sessions, newSession],
      activeSessionId: id,
      activeGroupId: finalGroupId,
      focusedSessionId: id,
      pendingCommands: { ...state.pendingCommands, [id]: [] },
      terminalOutput: { ...state.terminalOutput, [id]: "" }
    };
  }),

  renameSession: (id, label) => set((state) => ({
    sessions: state.sessions.map(s => s.id === id ? { ...s, label } : s)
  })),

  removeSession: (id) => set((state) => {
    const nextSessions = state.sessions.filter(s => s.id !== id);

    let nextActiveGroupId = state.activeGroupId;
    let nextFocusedSessionId = state.focusedSessionId;
    let nextActiveSessionId = state.activeSessionId;

    if (id === state.focusedSessionId) {
        // Try to find another session in the same group
        const groupSessions = nextSessions.filter(s => s.groupId === state.activeGroupId);
        if (groupSessions.length > 0) {
            nextFocusedSessionId = groupSessions[groupSessions.length - 1].id;
            nextActiveSessionId = nextFocusedSessionId;
        } else {
            // Group is empty, find another group
            const otherSessions = nextSessions;
            if (otherSessions.length > 0) {
                const fallback = otherSessions[otherSessions.length - 1];
                nextActiveGroupId = fallback.groupId;
                nextFocusedSessionId = fallback.id;
                nextActiveSessionId = fallback.id;
            } else {
                nextActiveGroupId = null;
                nextFocusedSessionId = null;
                nextActiveSessionId = null;
            }
        }
    }

    return {
      sessions: nextSessions,
      activeSessionId: nextActiveSessionId,
      activeGroupId: nextActiveGroupId,
      focusedSessionId: nextFocusedSessionId,
      pendingCommands: Object.fromEntries(
        Object.entries(state.pendingCommands).filter(([sessionId]) => Number(sessionId) !== id)
      ),
      terminalOutput: Object.fromEntries(
        Object.entries(state.terminalOutput).filter(([sessionId]) => Number(sessionId) !== id)
      ),
    };
  }),

  resetSessions: () => set({
    sessions: [],
    activeSessionId: null,
    activeGroupId: null,
    focusedSessionId: null,
    pendingCommands: {},
    terminalOutput: {},
    activeTask: null,
    lastExitCode: null,
  }),

  setActiveSession: (id) => set((state) => {
    const session = state.sessions.find(s => s.id === id);
    return {
      activeSessionId: id,
      focusedSessionId: id,
      activeGroupId: session ? session.groupId : state.activeGroupId
    };
  }),

  setActiveGroup: (groupId) => set((state) => {
    if (!groupId) return { activeGroupId: null, focusedSessionId: null, activeSessionId: null };

    // When switching group, focus the last session in that group
    const groupSessions = state.sessions.filter(s => s.groupId === groupId);
    const lastSession = groupSessions.length > 0 ? groupSessions[groupSessions.length - 1] : null;

    return {
      activeGroupId: groupId,
      focusedSessionId: lastSession ? lastSession.id : state.focusedSessionId,
      activeSessionId: lastSession ? lastSession.id : state.activeSessionId
    };
  }),

  setFocusedSession: (id) => set({ focusedSessionId: id, activeSessionId: id }),

  setSessionReady: (id, ready) => set((state) => ({
    sessions: state.sessions.map(s => s.id === id ? { ...s, isReady: ready } : s)
  })),

  closeActiveSession: () => {
    const focusedId = get().focusedSessionId;
    if (focusedId !== null) {
      get().removeSession(focusedId);
    }
  },

  enqueueCommand: (sessionId, command) =>
    set((state) => ({
      pendingCommands: {
        ...state.pendingCommands,
        [sessionId]: [...(state.pendingCommands[sessionId] || []), command]
      },
    })),

  consumeNextCommand: (sessionId) => {
    const sessionQueue = get().pendingCommands[sessionId] || [];
    if (sessionQueue.length === 0) return null;

    const [next, ...rest] = sessionQueue;
    set((state) => ({
      pendingCommands: {
        ...state.pendingCommands,
        [sessionId]: rest
      }
    }));
    return next;
  },

  clearPendingCommands: (sessionId) => set((state) => {
    const nextPending = { ...state.pendingCommands };
    delete nextPending[sessionId];
    return { pendingCommands: nextPending };
  }),

  appendTerminalOutput: (sessionId, text) =>
    set((state) => ({
      terminalOutput: {
        ...state.terminalOutput,
        [sessionId]: `${state.terminalOutput[sessionId] || ""}${text}`.slice(-200000),
      },
    })),

  clearTerminalOutput: (sessionId) =>
    set((state) => {
      const nextOutput = { ...state.terminalOutput };
      delete nextOutput[sessionId];
      return { terminalOutput: nextOutput };
    }),

  startTask: (task) =>
    set((state) => ({
      activeTask: task,
      lastExitCode: null,
      consoleEntries: pushConsoleEntry(state.consoleEntries, {
        taskId: task.taskId,
        level: 'info',
        text: `${task.action === 'build' ? 'Build' : 'Run'} started: ${task.label}\n$ ${task.command}`,
      }),
    })),

  finishTask: (taskId, exitCode) =>
    set((state) => {
      if (!state.activeTask || state.activeTask.taskId !== taskId) {
        return state;
      }

      const level =
        exitCode === 0 ? 'success' : exitCode === 130 ? 'warning' : 'error';

      return {
        activeTask: null,
        lastExitCode: exitCode,
        consoleEntries: pushConsoleEntry(state.consoleEntries, {
          taskId,
          level,
          text: `${state.activeTask.label} ${describeExitCode(exitCode)}.`,
        }),
      };
    }),

  finishTaskForSession: (sessionId, exitCode) =>
    set((state) => {
      if (!state.activeTask || state.activeTask.sessionId !== sessionId) {
        return state;
      }

      const level =
        exitCode === 0 ? 'success' : exitCode === 130 ? 'warning' : 'error';

      return {
        activeTask: null,
        lastExitCode: exitCode,
        consoleEntries: pushConsoleEntry(state.consoleEntries, {
          taskId: state.activeTask.taskId,
          level,
          text: `${state.activeTask.label} ${describeExitCode(exitCode)}.`,
        }),
      };
    }),

  clearActiveTask: () => set({ activeTask: null }),

  setSelectedTarget: (runtimeId, targetId) =>
    set((state) => {
      const selectedTargets = {
        ...state.selectedTargets,
        [runtimeId]: targetId,
      };

      persistSelectedTargets(selectedTargets);
      return { selectedTargets };
    }),

  appendConsoleEntry: (entry) =>
    set((state) => ({
      consoleEntries: pushConsoleEntry(state.consoleEntries, entry),
    })),

  appendConsoleChunk: (taskId, text) => {
    const normalized = text.replace(/\u001b\[[0-9;?]*[ -/]*[@-~]/g, '').trimEnd();
    if (!normalized.trim()) return;

    set((state) => ({
      consoleEntries: pushConsoleEntry(state.consoleEntries, {
        taskId,
        level: 'output',
        text: normalized,
      }),
    }));
  },

  appendConsoleChunkForSession: (sessionId, text) => {
    const activeTask = get().activeTask;
    get().appendConsoleChunk(
      activeTask?.sessionId === sessionId ? activeTask.taskId : undefined,
      text
    );
  },

  clearConsole: () => set({ consoleEntries: [] }),
}));
