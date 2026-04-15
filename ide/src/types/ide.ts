export interface FileEntry {
  name: string;
  path: string;
  is_directory: boolean;
  children?: FileEntry[];
  isLoaded?: boolean;
}

export interface Diagnostic {
  line: number;
  col: number;
  message: string;
  severity: 'error' | 'warning' | 'hint';
  code: string;
}

export interface Symbol {
  name: string;
  kind: 'Function' | 'Struct' | 'Variable' | 'Constant' | 'Interface';
  line: number;
  col: number;
}

import { ZenithTheme } from '../themes';
import { RuntimeProfileId, RuntimeExtensionPreset } from '../utils/runtimeProfiles';
import { ZenithHubSectionId } from '../content/zenithHubContent';

export type ExtensionConfig = RuntimeExtensionPreset;

export type EditorPane = 'primary' | 'secondary';

export interface EditorSlice {
  openFiles: FileEntry[];
  activeFile: FileEntry | null;
  primaryFile: FileEntry | null;
  secondaryFile: FileEntry | null;
  focusedPane: EditorPane;
  splitMode: 'single' | 'vertical' | 'horizontal';
  diagnosticsMap: Record<string, Diagnostic[]>;
  symbols: Record<string, Symbol[]>;
  cursorPosition: { line: number; col: number };
  breakpoints: { path: string; line: number }[];
  
  // Restoration
  previewFile: FileEntry | null;
  activeFileSymbols: Symbol[];
  activeFileHistory: any[];
  diffFile: string | null;
  hasLoadedSettings: boolean;

  openFile: (file: FileEntry, isFixed?: boolean, pane?: EditorPane) => void;
  closeFile: (path: string) => void;
  setFocusedPane: (pane: EditorPane) => void;
  setSplitMode: (mode: 'single' | 'vertical' | 'horizontal') => void;
  setDiagnostics: (path: string, diagnostics: Diagnostic[]) => void;
  setCursorPosition: (pos: { line: number; col: number }) => void;
  toggleBreakpoint: (path: string, line: number) => void;
  clearBreakpoints: () => void;
  setDiffFile: (path: string | null) => void;
  closeDiff: () => void;
  setActiveFileSymbols: (symbols: Symbol[]) => void;
  fetchHistory: (path: string) => Promise<void>;
  loadSettings: () => Promise<void>;
}

export interface FileSystemSlice {
  currentProjectRoot: string;
  fileTree: FileEntry[];
  fileBuffers: Record<string, string>;
  dirtyFiles: Set<string>;
  gitStatus: Record<string, string>;
  gitCurrentBranch: string | null;
  
  // Restoration
  availableWorkspaces: string[];
  systemStats: { cpu: number; memory: number; memoryUsed: number; uptime: number } | null;
  searchFilterPath: string | null;

  setProjectRoot: (path: string) => Promise<void>;
  setFileTree: (tree: FileEntry[]) => void;
  updateFileTreeNode: (path: string, children: FileEntry[]) => void;
  refreshExplorerRoot: () => Promise<void>;
  loadDirectoryChildren: (path: string) => Promise<FileEntry[]>;
  setFileBuffer: (path: string, content: string) => void;
  removeFileBuffer: (path: string) => void;
  setFileDirty: (path: string, dirty: boolean) => void;
  
  scheduleGitRefresh: (delayMs?: number) => void;
  refreshGitStatus: () => Promise<void>;
  refreshGitBranch: () => Promise<void>;
  stageFile: (path: string) => Promise<void>;
  unstageFile: (path: string) => Promise<void>;
  stageAll: () => Promise<void>;
  unstageAll: () => Promise<void>;
  discardChanges: (path: string) => Promise<void>;
  commitChanges: (message: string) => Promise<void>;
  openDiff: (path: string) => void;
  
  loadAvailableWorkspaces: () => Promise<void>;
  loadWorkspaceContext: (name: string) => Promise<void>;
  saveWorkspaceContext: (name: string) => Promise<void>;
  deleteWorkspaceContext: (name: string) => Promise<void>;
  refreshSystemStats: () => Promise<void>;
  
  replacePathReference: (oldPath: string, newEntry: FileEntry) => void;
  removePathReferences: (path: string) => void;
  setSearchFilterPath: (path: string | null) => void;
}

export interface LayoutSlice {
  isSidebarOpen: boolean;
  isInspectorOpen: boolean;
  isTerminalOpen: boolean;
  isZenMode: boolean;
  isBottomPanelOpen: boolean;
  activeSidebarTab: string;
  activeInspectorTab: string;
  activeBottomTab: string;
  panelSizes: Record<string, number>;

  toggleSidebar: () => void;
  toggleInspector: () => void;
  toggleTerminal: () => void;
  toggleZenMode: () => void;
  toggleBottomPanel: () => void;
  
  setSidebarOpen: (open: boolean) => void;
  setInspectorOpen: (open: boolean) => void;
  setBottomPanelOpen: (open: boolean) => void;
  setSidebarTab: (tab: string) => void;
  setInspectorTab: (tab: string) => void;
  setBottomTab: (tab: string) => void;
  setPanelSize: (key: string, size: number) => void;
}

export interface SettingsSlice {
  settings: {
    theme: string;
    customThemes: Record<string, ZenithTheme>;
    extensions: Record<string, RuntimeExtensionPreset>;
    editorFontSize: number;
    editorFontFamily: string;
    editorFontLigatures: boolean;
    editorMinimap: boolean;
    editorWordWrap: boolean;
    editorCursorBlinking: 'blink' | 'smooth' | 'phase' | 'expand' | 'solid';
    editorFormatOnSave: boolean;
    historyMaxSnapshots: number;
    historyRetentionDays: number;
    
    // Restoration
    keymap: Record<string, string>;
    restoreSession: boolean;
    language: 'pt' | 'en' | 'es';
    autoSave: 'off' | 'onFocusChange' | 'afterDelay';
    uiScale: number;
    toolbar: {
      showWorkspaceSelector: boolean;
      showGitBranch: boolean;
      showQuickOpen: boolean;
      showDiagnostics: boolean;
      showRuntimeTarget: boolean;
      showPanelToggles: boolean;
      showLayoutControls: boolean;
      showFocusHints: boolean;
      showSettingsButton: boolean;
    };
  };
  updateSettings: (update: Partial<SettingsSlice['settings']>) => void;
  addCustomTheme: (theme: ZenithTheme) => void;
}

// Command Slice
export interface Command {
  id: string;
  title: string;
  category?: string;
  icon?: any;
  action: () => void;
}

export interface CommandSlice {
  commands: Record<string, Command>;
  isPaletteOpen: boolean;
  initialQuery: string;
  isHubOpen: boolean;
  hubInitialSection?: ZenithHubSectionId;

  registerCommand: (command: Command) => void;
  unregisterCommand: (id: string) => void;
  executeCommand: (id: string) => void;
  setPaletteOpen: (open: boolean, initialQuery?: string) => void;
  setHubOpen: (open: boolean, section?: ZenithHubSectionId) => void;
}

// Execution Slice
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

export interface ExecutionSlice {
  sessions: TerminalSessionMeta[];
  activeSessionId: number | null;
  activeGroupId: string | null;
  focusedSessionId: number | null;
  pendingCommands: Record<number, string[]>;
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

export type WorkspaceState = SettingsSlice &
  LayoutSlice &
  FileSystemSlice &
  EditorSlice &
  CommandSlice &
  ExecutionSlice;
