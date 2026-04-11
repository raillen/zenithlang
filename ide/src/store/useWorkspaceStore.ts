import { create } from 'zustand';
import Database from '@tauri-apps/plugin-sql';
import { THEMES, defaultLight } from '../themes';
import { applyThemeToDOM, applyUiScaleToDOM, clampUiScale } from '../utils/themeEngine';
import { DEFAULT_RUNTIME_EXTENSION_CONFIGS } from '../utils/runtimeProfiles';
import { invoke, isTauriEnvironment } from '../utils/tauri';
import { listen } from '@tauri-apps/api/event';
import {
  clearEditorBuffers,
  remapEditorBuffersInTree,
  removeEditorBuffersInTree,
} from '../utils/editorBuffers';

export interface FileEntry {
  name: string;
  path: string;
  is_directory: boolean;
  children?: FileEntry[];
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

export type EditorPane = 'primary' | 'secondary';
export type EditorSplitMode = 'single' | 'vertical' | 'horizontal';
export type InspectorTab = 'file' | 'context' | 'attributes' | 'history';

export interface ExtensionConfig {
  compilerPath: string;
  lspPath: string;
  buildCommand: string;
  isEnabled: boolean;
}

export interface ToolbarSettings {
  showWorkspaceSelector: boolean;
  showGitBranch: boolean;
  showQuickOpen: boolean;
  showDiagnostics: boolean;
  showRuntimeTarget: boolean;
  showPanelToggles: boolean;
  showLayoutControls: boolean;
  showFocusHints: boolean;
  showSettingsButton: boolean;
}

export interface IDESettings {
  language: 'pt' | 'en' | 'es';
  theme: string;
  extensions: Record<string, ExtensionConfig>;
  toolbar: ToolbarSettings;
  editorWordWrap: boolean;
  editorMinimap: boolean;
  editorFontSize: number;
  editorFontFamily: string;
  editorFontLigatures: boolean;
  editorCursorBlinking: 'blink' | 'smooth' | 'phase' | 'expand' | 'solid';
  editorFormatOnSave: boolean;
  autoSave: 'off' | 'onFocusChange' | 'afterDelay';
  restoreSession: boolean;
  uiScale: number;
  keymap: Record<string, string>;
  historyRetentionDays: number;
  historyMaxSnapshots: number;
}

export interface WorkbenchSessionSnapshot {
  openFiles: FileEntry[];
  activeFile: FileEntry | null;
  primaryFile: FileEntry | null;
  secondaryFile: FileEntry | null;
  previewFile: FileEntry | null;
  isBottomPanelOpen: boolean;
  isSidebarOpen: boolean;
  activeSidebarTab: string;
  activeBottomTab: 'console' | 'terminal' | 'problems';
  currentProjectRoot: string;
  focusedPane: EditorPane;
  splitMode: EditorSplitMode;
  isInspectorOpen: boolean;
  activeInspectorTab: InspectorTab;
  isZenMode: boolean;
  gitStatus: Record<string, string>;
  breakpoints: { path: string; line: number }[];
}

interface WorkspaceState extends WorkbenchSessionSnapshot {
  fileTree: FileEntry[];
  fileBuffers: Record<string, string>;
  dirtyFiles: Set<string>;
  diagnosticsMap: Record<string, Diagnostic[]>;
  settings: IDESettings;
  activeFileSymbols: Symbol[];
  activeFileHistory: any[];
  hasLoadedSettings: boolean;
  
  // Status Bar State
  cursorPosition: { line: number; col: number } | null;
  gitCurrentBranch: string | null;
  systemStats: { cpu: number; memory: number; memoryUsed: number } | null;
  searchFilterPath: string | null;

  setFileTree: (tree: FileEntry[]) => void;
  setFileBuffer: (path: string, content: string) => void;
  removeFileBuffer: (path: string) => void;
  openFile: (file: FileEntry, isFixed?: boolean) => void;
  closeFile: (path: string) => void;
  setFileDirty: (path: string, dirty: boolean) => void;
  setDiagnostics: (path: string, diagnostics: Diagnostic[]) => void;
  setBottomPanelOpen: (open: boolean) => void;
  setSidebarOpen: (open: boolean) => void;
  setSidebarTab: (tab: string) => void;
  setBottomTab: (tab: 'console' | 'terminal' | 'problems') => void;
  setProjectRoot: (path: string) => Promise<void>;
  setActiveFileSymbols: (symbols: Symbol[]) => void;
  replacePathReference: (oldPath: string, nextEntry: FileEntry) => void;
  removePathReferences: (path: string) => void;
  setFocusedPane: (pane: EditorPane) => void;
  setSplitMode: (mode: EditorSplitMode) => void;
  setInspectorOpen: (open: boolean) => void;
  setInspectorTab: (tab: InspectorTab) => void;
  toggleZenMode: () => void;
  toggleSidebar: () => void;
  toggleInspector: () => void;
  toggleBottomPanel: () => void;
  // Git & Debug Actions
  refreshGitStatus: () => Promise<void>;
  toggleBreakpoint: (path: string, line: number) => void;
  clearBreakpoints: (path?: string) => void;
  stageFile: (path: string) => Promise<void>;
  unstageFile: (path: string) => Promise<void>;
  stageAll: () => Promise<void>;
  unstageAll: () => Promise<void>;
  discardChanges: (path: string) => Promise<void>;
  commitChanges: (message: string) => Promise<void>;

  // Diff Mode
  diffFile: string | null;
  openDiff: (path: string) => void;
  closeDiff: () => void;

  updateSettings: (newSettings: Partial<IDESettings>) => Promise<void>;
  loadSettings: () => Promise<void>;

  // Context Workspaces
  availableWorkspaces: string[];
  loadAvailableWorkspaces: () => Promise<void>;
  saveWorkspaceContext: (name: string) => Promise<void>;
  loadWorkspaceContext: (name: string) => Promise<void>;
  deleteWorkspaceContext: (name: string) => Promise<void>;
  fetchHistory: (path: string) => Promise<void>;

  setCursorPosition: (pos: { line: number; col: number } | null) => void;
  refreshGitBranch: () => Promise<void>;
  refreshSystemStats: () => Promise<void>;
  setSearchFilterPath: (path: string | null) => void;
}

const SESSION_STORAGE_KEY = 'zenith:workbench-session:v1';

const DEFAULT_SETTINGS: IDESettings = {
  language: 'pt',
  theme: 'light-prism',
  extensions: {
    ...DEFAULT_RUNTIME_EXTENSION_CONFIGS,
  },
  toolbar: {
    showWorkspaceSelector: true,
    showGitBranch: true,
    showQuickOpen: true,
    showDiagnostics: true,
    showRuntimeTarget: true,
    showPanelToggles: true,
    showLayoutControls: true,
    showFocusHints: false,
    showSettingsButton: true,
  },
  editorWordWrap: true,
  editorMinimap: false,
  editorFontSize: 13,
  editorFontFamily: '"JetBrains Mono", Menlo, Monaco, "Courier New", monospace',
  editorFontLigatures: true,
  editorCursorBlinking: 'smooth',
  editorFormatOnSave: false,
  autoSave: 'off',
  restoreSession: true,
  uiScale: 1,
  keymap: {
    'ctrl+shift+p': 'zenith.workbench.action.showCommands',
    'ctrl+shift+f': 'zenith.workbench.action.findInFiles',
    'ctrl+shift+o': 'zenith.workbench.action.openFolder',
    'ctrl+o': 'zenith.workbench.action.openFile',
    'ctrl+n': 'zenith.files.newFile',
    'ctrl+shift+n': 'zenith.files.newFolder',
    'f5': 'zenith.pipeline.run',
    'shift+f5': 'zenith.pipeline.stop',
    'ctrl+shift+b': 'zenith.pipeline.build',
    'ctrl+p': 'zenith.workbench.action.quickOpen',
    'ctrl+,': 'zenith.workbench.action.openSettings',
    'ctrl+alt+v': 'zenith.workbench.action.splitEditorRight',
    'ctrl+alt+h': 'zenith.workbench.action.splitEditorDown',
    'ctrl+alt+1': 'zenith.workbench.action.singleEditor',
    'ctrl+alt+i': 'zenith.workbench.action.toggleInspector',
    'alt+2': 'zenith.workbench.action.focusSecondaryEditorGroup',
  },
  historyRetentionDays: 30,
  historyMaxSnapshots: 50,
};

const appendUniqueFile = (files: FileEntry[], file: FileEntry) =>
  files.some((entry) => entry.path === file.path) ? files : [...files, file];

const normalizePathForCompare = (path: string) =>
  path.replace(/\\/g, '/').replace(/\/+$/g, '').toLowerCase();

const isSameOrChildPath = (path: string, rootPath: string) => {
  const normalizedPath = normalizePathForCompare(path);
  const normalizedRoot = normalizePathForCompare(rootPath);
  return normalizedPath === normalizedRoot || normalizedPath.startsWith(`${normalizedRoot}/`);
};

const remapPathReference = (path: string, oldPath: string, newPath: string) => {
  const normalizedPath = normalizePathForCompare(path);
  const normalizedOldPath = normalizePathForCompare(oldPath);

  if (normalizedPath === normalizedOldPath) {
    return newPath;
  }

  const oldPrefix = normalizedOldPath.endsWith('/') ? normalizedOldPath : `${normalizedOldPath}/`;
  if (!normalizedPath.startsWith(oldPrefix)) {
    return path;
  }

  const normalizedSuffix = path.replace(/\\/g, '/').slice(oldPrefix.length);
  return `${newPath.replace(/[\\/]+$/g, '')}/${normalizedSuffix}`;
};

const remapFileEntry = (entry: FileEntry | null, oldPath: string, nextEntry: FileEntry) => {
  if (!entry || !isSameOrChildPath(entry.path, oldPath)) return entry;

  const isExactMatch = normalizePathForCompare(entry.path) === normalizePathForCompare(oldPath);
  return {
    ...entry,
    path: remapPathReference(entry.path, oldPath, nextEntry.path),
    name: isExactMatch ? nextEntry.name : entry.name,
    is_directory: isExactMatch ? nextEntry.is_directory : entry.is_directory,
  };
};

const getFallbackFile = (files: FileEntry[], excludedPaths: string[] = []) =>
  files.find((entry) => !excludedPaths.includes(entry.path)) ?? null;

const getSessionStorage = () =>
  typeof window === 'undefined' ? null : window.localStorage;

const normalizeFileEntry = (value: unknown): FileEntry | null => {
  if (!value || typeof value !== 'object') return null;

  const candidate = value as Partial<FileEntry>;
  if (typeof candidate.name !== 'string' || typeof candidate.path !== 'string') {
    return null;
  }

  return {
    name: candidate.name,
    path: candidate.path,
    is_directory: Boolean(candidate.is_directory),
  };
};

const normalizeFileList = (value: unknown) => {
  if (!Array.isArray(value)) return [];

  const seen = new Set<string>();
  return value.reduce<FileEntry[]>((acc, entry) => {
    const normalized = normalizeFileEntry(entry);
    if (!normalized || seen.has(normalized.path)) return acc;
    seen.add(normalized.path);
    acc.push(normalized);
    return acc;
  }, []);
};

const normalizeBottomTab = (value: unknown): WorkbenchSessionSnapshot['activeBottomTab'] =>
  value === 'terminal' || value === 'problems' ? value : 'console';

const normalizeFocusedPane = (value: unknown): EditorPane =>
  value === 'secondary' ? 'secondary' : 'primary';

const normalizeSplitMode = (value: unknown): EditorSplitMode =>
  value === 'vertical' || value === 'horizontal' ? value : 'single';

const normalizeInspectorTab = (value: unknown): InspectorTab =>
  value === 'context' || value === 'attributes' ? value : 'file';

const loadPersistedWorkbenchSession = (): WorkbenchSessionSnapshot | null => {
  const storage = getSessionStorage();
  if (!storage) return null;

  try {
    const raw = storage.getItem(SESSION_STORAGE_KEY);
    if (!raw) return null;

    const parsed = JSON.parse(raw);
    const openFiles = normalizeFileList(parsed.openFiles);
    const previewFile = normalizeFileEntry(parsed.previewFile);
    const splitMode = normalizeSplitMode(parsed.splitMode);
    const primaryFile = normalizeFileEntry(parsed.primaryFile) ?? previewFile ?? openFiles[0] ?? null;

    let secondaryFile =
      splitMode === 'single'
        ? null
        : normalizeFileEntry(parsed.secondaryFile);

    if (splitMode !== 'single' && !secondaryFile) {
      secondaryFile = getFallbackFile(
        openFiles,
        primaryFile ? [primaryFile.path] : []
      );
    }

    const focusedPane = splitMode === 'single' ? 'primary' : normalizeFocusedPane(parsed.focusedPane);
    const activeFile =
      normalizeFileEntry(parsed.activeFile) ??
      (focusedPane === 'secondary' ? secondaryFile : primaryFile) ??
      primaryFile ??
      secondaryFile ??
      previewFile ??
      openFiles[0] ??
      null;

    return {
      openFiles,
      activeFile,
      primaryFile,
      secondaryFile,
      previewFile,
      isBottomPanelOpen: typeof parsed.isBottomPanelOpen === 'boolean' ? parsed.isBottomPanelOpen : false,
      isSidebarOpen: typeof parsed.isSidebarOpen === 'boolean' ? parsed.isSidebarOpen : true,
      activeSidebarTab: typeof parsed.activeSidebarTab === 'string' ? parsed.activeSidebarTab : 'navigator',
      activeBottomTab: normalizeBottomTab(parsed.activeBottomTab),
      currentProjectRoot: typeof parsed.currentProjectRoot === 'string' && parsed.currentProjectRoot.length > 0
        ? parsed.currentProjectRoot
        : '.',
      focusedPane,
      splitMode,
      isInspectorOpen: typeof parsed.isInspectorOpen === 'boolean' ? parsed.isInspectorOpen : true,
      activeInspectorTab: normalizeInspectorTab(parsed.activeInspectorTab),
      isZenMode: typeof parsed.isZenMode === 'boolean' ? parsed.isZenMode : false,
      gitStatus: parsed.gitStatus || {},
      breakpoints: parsed.breakpoints || [],
    };
  } catch (error) {
    console.error('Failed to restore workbench session', error);
    return null;
  }
};

export function persistWorkbenchSession(snapshot: WorkbenchSessionSnapshot) {
  const storage = getSessionStorage();
  if (!storage) return;

  try {
    storage.setItem(SESSION_STORAGE_KEY, JSON.stringify({
      openFiles: snapshot.openFiles,
      activeFile: snapshot.activeFile,
      primaryFile: snapshot.primaryFile,
      secondaryFile: snapshot.secondaryFile,
      previewFile: snapshot.previewFile,
      isBottomPanelOpen: snapshot.isBottomPanelOpen,
      isSidebarOpen: snapshot.isSidebarOpen,
      activeSidebarTab: snapshot.activeSidebarTab,
      activeBottomTab: snapshot.activeBottomTab,
      currentProjectRoot: snapshot.currentProjectRoot,
      focusedPane: snapshot.focusedPane,
      splitMode: snapshot.splitMode,
      isInspectorOpen: snapshot.isInspectorOpen,
      activeInspectorTab: snapshot.activeInspectorTab,
      isZenMode: snapshot.isZenMode,
    }));
  } catch (error) {
    console.error('Failed to persist workbench session', error);
  }
}

export function clearPersistedWorkbenchSession() {
  const storage = getSessionStorage();
  if (!storage) return;

  try {
    storage.removeItem(SESSION_STORAGE_KEY);
  } catch (error) {
    console.error('Failed to clear workbench session', error);
  }
}

export const useWorkspaceStore = create<WorkspaceState>((set, get) => ({
  fileTree: [],
  fileBuffers: {},
  openFiles: [],
  activeFile: null,
  primaryFile: null,
  secondaryFile: null,
  previewFile: null,
  dirtyFiles: new Set(),
  diagnosticsMap: {},
  isBottomPanelOpen: false,
  isSidebarOpen: true,
  activeSidebarTab: 'navigator',
  activeBottomTab: 'console',
  currentProjectRoot: '.',
  settings: DEFAULT_SETTINGS,
  activeFileSymbols: [],
  focusedPane: 'primary',
  splitMode: 'single',
  isInspectorOpen: true,
  activeInspectorTab: 'file',
  isZenMode: false,
  gitStatus: {},
  breakpoints: [],
  hasLoadedSettings: false,
  availableWorkspaces: [],
  activeFileHistory: [],
  diffFile: null,
  cursorPosition: null,
  gitCurrentBranch: null,
  systemStats: null,
  searchFilterPath: null,

  setFileTree: (tree) => set({ fileTree: tree }),
  setFileBuffer: (path, content) =>
    set((state) => ({
      fileBuffers: {
        ...state.fileBuffers,
        [path]: content,
      },
    })),
  removeFileBuffer: (path) =>
    set((state) => {
      const nextBuffers = { ...state.fileBuffers };
      delete nextBuffers[path];
      return { fileBuffers: nextBuffers };
    }),

  setDiagnostics: (path, diagnostics) => set((state) => ({
    diagnosticsMap: { ...state.diagnosticsMap, [path]: diagnostics },
  })),

  setBottomTab: (tab) => set({ activeBottomTab: tab }),

  updateSettings: async (newSettings) => {
    const currentSettings = get().settings;
    const updatedSettings: IDESettings = {
      ...currentSettings,
      ...newSettings,
      extensions: newSettings.extensions
        ? { ...currentSettings.extensions, ...newSettings.extensions }
        : currentSettings.extensions,
      toolbar: newSettings.toolbar
        ? { ...currentSettings.toolbar, ...newSettings.toolbar }
        : currentSettings.toolbar,
      keymap: newSettings.keymap
        ? { ...DEFAULT_SETTINGS.keymap, ...newSettings.keymap }
        : currentSettings.keymap,
      uiScale:
        newSettings.uiScale !== undefined
          ? clampUiScale(newSettings.uiScale)
          : currentSettings.uiScale,
    };

    set({ settings: updatedSettings });

    if (newSettings.theme) {
      applyThemeToDOM(THEMES[newSettings.theme] || defaultLight);
    }

    if (newSettings.uiScale !== undefined) {
      applyUiScaleToDOM(updatedSettings.uiScale);
    }

    if (newSettings.restoreSession === false) {
      clearPersistedWorkbenchSession();
    }

    if (!isTauriEnvironment()) {
      return;
    }

    const settingsToPersist: Partial<IDESettings> = {
      ...newSettings,
      uiScale:
        newSettings.uiScale !== undefined ? updatedSettings.uiScale : newSettings.uiScale,
      keymap: newSettings.keymap ? updatedSettings.keymap : newSettings.keymap,
      extensions: newSettings.extensions ? updatedSettings.extensions : newSettings.extensions,
      toolbar: newSettings.toolbar ? updatedSettings.toolbar : newSettings.toolbar,
    };

    try {
      const db = await Database.load('sqlite:zenith.db');
      for (const [key, value] of Object.entries(settingsToPersist)) {
        const valToSave =
          typeof value === 'object' && value !== null ? JSON.stringify(value) : value;
        await db.execute(
          'INSERT OR REPLACE INTO settings (key, value) VALUES ($1, $2)',
          [key, valToSave]
        );
      }
    } catch (err) {
      console.error('Failed to persist settings', err);
    }
  },

  loadSettings: async () => {
    let finalSettings: IDESettings = DEFAULT_SETTINGS;

    if (isTauriEnvironment()) {
      try {
      const db = await Database.load('sqlite:zenith.db');
      const rows = await db.select<{ key: string; value: string }[]>(
        'SELECT key, value FROM settings'
      );

      const loadedSettings: Partial<IDESettings> = {};
      rows.forEach((row) => {
        if (row.key in DEFAULT_SETTINGS) {
          const defaultVal = (DEFAULT_SETTINGS as unknown as Record<string, unknown>)[row.key];
          let val: unknown = row.value;

          if (typeof defaultVal === 'boolean') {
            val = val === 'true';
          } else if (typeof defaultVal === 'number') {
            val = Number(val);
          } else if (typeof defaultVal === 'object' && defaultVal !== null) {
            try {
              val = JSON.parse(String(val));
            } catch {
              val = defaultVal;
            }
          }

          (loadedSettings as unknown as Record<string, unknown>)[row.key] = val;
        }
      });

      finalSettings = {
        ...DEFAULT_SETTINGS,
        ...loadedSettings,
        language:
          loadedSettings.language === 'en' || loadedSettings.language === 'es'
            ? loadedSettings.language
            : DEFAULT_SETTINGS.language,
        autoSave:
          loadedSettings.autoSave === 'afterDelay' || loadedSettings.autoSave === 'onFocusChange'
            ? loadedSettings.autoSave
            : DEFAULT_SETTINGS.autoSave,
        uiScale: clampUiScale(
          typeof loadedSettings.uiScale === 'number'
            ? loadedSettings.uiScale
            : DEFAULT_SETTINGS.uiScale
        ),
        extensions: {
          ...DEFAULT_SETTINGS.extensions,
          ...(loadedSettings.extensions || {}),
        },
        toolbar: {
          ...DEFAULT_SETTINGS.toolbar,
          ...(loadedSettings.toolbar || {}),
        },
        keymap: {
          ...DEFAULT_SETTINGS.keymap,
          ...(loadedSettings.keymap || {}),
        },
      };      await get().loadAvailableWorkspaces();
    } catch (err) {
      console.error('Failed to load settings from SQLite', err);
    }
    }

    const restoredSession = finalSettings.restoreSession
      ? loadPersistedWorkbenchSession()
      : null;

    set((state) => ({
      ...state,
      settings: finalSettings,
      ...(restoredSession ?? {}),
      dirtyFiles: new Set(),
      diagnosticsMap: {},
      activeFileSymbols: [],
      hasLoadedSettings: true,
    }));

    applyThemeToDOM(THEMES[finalSettings.theme] || defaultLight);
    applyUiScaleToDOM(finalSettings.uiScale);

    if (!finalSettings.restoreSession) {
      clearPersistedWorkbenchSession();
    }
  },

  setProjectRoot: async (path) => {
    clearEditorBuffers();
    set({
      currentProjectRoot: path,
      fileTree: [],
      fileBuffers: {},
      openFiles: [],
      activeFile: null,
      primaryFile: null,
      secondaryFile: null,
      previewFile: null,
      dirtyFiles: new Set(),
      diagnosticsMap: {},
      activeFileSymbols: [],
      focusedPane: 'primary',
      splitMode: 'single',
      activeInspectorTab: 'file',
    });

    // Bootstrap Indexer and Watcher on backend
    try {
      await invoke("workspace_index_bootstrap", { rootPath: path });
      console.log("Workspace indexer bootstrapped for:", path);
    } catch (err) {
      console.error("Failed to bootstrap workspace indexer", err);
    }
  },

  setActiveFileSymbols: (symbols) => set({ activeFileSymbols: symbols }),
  setInspectorOpen: (open) => set({ isInspectorOpen: open }),
  setInspectorTab: (tab) => set({ activeInspectorTab: tab }),

  setFocusedPane: (pane) => set((state) => {
    const nextPane = state.splitMode === 'single' ? 'primary' : pane;
    return {
      focusedPane: nextPane,
      activeFile: nextPane === 'secondary' ? state.secondaryFile : state.primaryFile,
    };
  }),

  setSplitMode: (mode) => set((state) => {
    if (mode === state.splitMode) return state;

    if (mode === 'single') {
      const nextPrimaryFile =
        state.focusedPane === 'secondary'
          ? state.secondaryFile ?? state.primaryFile ?? state.activeFile
          : state.primaryFile ?? state.activeFile ?? state.secondaryFile;

      return {
        splitMode: 'single',
        focusedPane: 'primary',
        primaryFile: nextPrimaryFile,
        secondaryFile: null,
        activeFile: nextPrimaryFile,
        previewFile: state.previewFile?.path === nextPrimaryFile?.path ? state.previewFile : null,
      };
    }

    const primaryFile =
      state.primaryFile ?? state.activeFile ?? state.previewFile ?? state.openFiles[0] ?? null;

    return {
      splitMode: mode,
      primaryFile,
      activeFile:
        state.focusedPane === 'secondary' && state.secondaryFile
          ? state.secondaryFile
          : primaryFile,
      previewFile: state.previewFile?.path === primaryFile?.path ? state.previewFile : null,
    };
  }),

  openFile: (file, isFixed = false) => set((state) => {
    if (file.is_directory) return state;

    if (state.splitMode !== 'single') {
      const nextOpenFiles = appendUniqueFile(state.openFiles, file);
      const nextState = {
        openFiles: nextOpenFiles,
        previewFile: null,
        activeFile: file,
      };

      if (state.focusedPane === 'secondary') {
        return {
          ...nextState,
          secondaryFile: file,
          focusedPane: 'secondary',
        };
      }

      return {
        ...nextState,
        primaryFile: file,
        focusedPane: 'primary',
      };
    }

    if (isFixed) {
      const isOpened = state.openFiles.some((entry) => entry.path === file.path);
      return {
        activeFile: file,
        primaryFile: file,
        previewFile: state.previewFile?.path === file.path ? null : state.previewFile,
        openFiles: isOpened ? state.openFiles : [...state.openFiles, file],
      };
    }

    const isAlreadyInOpenFiles = state.openFiles.some((entry) => entry.path === file.path);

    return {
      activeFile: file,
      primaryFile: file,
      previewFile: isAlreadyInOpenFiles ? null : file,
    };
  }),

  closeFile: (path) => set((state) => {
    const newOpenFiles = state.openFiles.filter((entry) => entry.path !== path);
    const newDirtyFiles = new Set(state.dirtyFiles);
    newDirtyFiles.delete(path);
    const nextPreviewFile = state.previewFile?.path === path ? null : state.previewFile;

    if (state.splitMode === 'single') {
      const nextPrimaryFile =
        state.primaryFile?.path === path
          ? nextPreviewFile ?? newOpenFiles[0] ?? null
          : state.primaryFile;

      const nextActiveFile =
        state.activeFile?.path === path
          ? nextPreviewFile ?? newOpenFiles[0] ?? null
          : state.activeFile;

      const visibleFile = nextActiveFile ?? nextPrimaryFile ?? null;

      return {
        openFiles: newOpenFiles,
        dirtyFiles: newDirtyFiles,
        previewFile: nextPreviewFile,
        primaryFile: visibleFile,
        activeFile: visibleFile,
      };
    }

    let nextPrimaryFile = state.primaryFile?.path === path ? null : state.primaryFile;
    let nextSecondaryFile = state.secondaryFile?.path === path ? null : state.secondaryFile;

    if (!nextPrimaryFile) {
      nextPrimaryFile =
        nextPreviewFile ??
        getFallbackFile(newOpenFiles, nextSecondaryFile ? [nextSecondaryFile.path] : []);
    }

    if (!nextSecondaryFile) {
      nextSecondaryFile = getFallbackFile(
        newOpenFiles,
        nextPrimaryFile ? [nextPrimaryFile.path] : []
      );
    }

    let nextFocusedPane = state.focusedPane;
    if (nextFocusedPane === 'primary' && !nextPrimaryFile && nextSecondaryFile) {
      nextFocusedPane = 'secondary';
    } else if (nextFocusedPane === 'secondary' && !nextSecondaryFile) {
      nextFocusedPane = 'primary';
    }

    const nextActiveFile =
      nextFocusedPane === 'secondary' ? nextSecondaryFile : nextPrimaryFile;

    return {
      openFiles: newOpenFiles,
      dirtyFiles: newDirtyFiles,
      previewFile: nextPreviewFile?.path === nextPrimaryFile?.path ? nextPreviewFile : null,
      primaryFile: nextPrimaryFile,
      secondaryFile: nextSecondaryFile,
      focusedPane: nextFocusedPane,
      activeFile: nextActiveFile ?? null,
    };
  }),

  setFileDirty: (path, dirty) => set((state) => {
    const newDirtyFiles = new Set(state.dirtyFiles);

    if (dirty) {
      newDirtyFiles.add(path);

      if (state.previewFile?.path === path) {
        const file = state.previewFile;
        return {
          dirtyFiles: newDirtyFiles,
          previewFile: null,
          openFiles: appendUniqueFile(state.openFiles, file),
          primaryFile: state.primaryFile?.path === path ? file : state.primaryFile,
          activeFile: state.activeFile?.path === path ? file : state.activeFile,
        };
      }
    } else {
      newDirtyFiles.delete(path);
    }

    return { dirtyFiles: newDirtyFiles };
  }),

  replacePathReference: (oldPath, nextEntry) => set((state) => {
    remapEditorBuffersInTree(oldPath, nextEntry.path);
    const dirtyFiles = new Set<string>();
    state.dirtyFiles.forEach((dirtyPath) => {
      dirtyFiles.add(remapPathReference(dirtyPath, oldPath, nextEntry.path));
    });

    const diagnosticsMap = Object.entries(state.diagnosticsMap).reduce<Record<string, Diagnostic[]>>(
      (acc, [path, diagnostics]) => {
        acc[remapPathReference(path, oldPath, nextEntry.path)] = diagnostics;
        return acc;
      },
      {}
    );
    const fileBuffers = Object.entries(state.fileBuffers).reduce<Record<string, string>>(
      (acc, [path, content]) => {
        acc[remapPathReference(path, oldPath, nextEntry.path)] = content;
        return acc;
      },
      {}
    );

    return {
      fileBuffers,
      openFiles: state.openFiles.map((entry) => remapFileEntry(entry, oldPath, nextEntry) ?? entry),
      activeFile: remapFileEntry(state.activeFile, oldPath, nextEntry),
      primaryFile: remapFileEntry(state.primaryFile, oldPath, nextEntry),
      secondaryFile: remapFileEntry(state.secondaryFile, oldPath, nextEntry),
      previewFile: remapFileEntry(state.previewFile, oldPath, nextEntry),
      dirtyFiles,
      diagnosticsMap,
      breakpoints: state.breakpoints.map((breakpoint) => ({
        ...breakpoint,
        path: remapPathReference(breakpoint.path, oldPath, nextEntry.path),
      })),
    };
  }),

  removePathReferences: (path) => set((state) => {
    removeEditorBuffersInTree(path);
    const openFiles = state.openFiles.filter((entry) => !isSameOrChildPath(entry.path, path));
    const dirtyFiles = new Set(
      Array.from(state.dirtyFiles).filter((dirtyPath) => !isSameOrChildPath(dirtyPath, path))
    );
    const diagnosticsMap = Object.entries(state.diagnosticsMap).reduce<Record<string, Diagnostic[]>>(
      (acc, [entryPath, diagnostics]) => {
        if (!isSameOrChildPath(entryPath, path)) {
          acc[entryPath] = diagnostics;
        }
        return acc;
      },
      {}
    );
    const fileBuffers = Object.entries(state.fileBuffers).reduce<Record<string, string>>(
      (acc, [entryPath, content]) => {
        if (!isSameOrChildPath(entryPath, path)) {
          acc[entryPath] = content;
        }
        return acc;
      },
      {}
    );

    const previewFile = state.previewFile && !isSameOrChildPath(state.previewFile.path, path)
      ? state.previewFile
      : null;
    let primaryFile = state.primaryFile && !isSameOrChildPath(state.primaryFile.path, path)
      ? state.primaryFile
      : null;
    let secondaryFile = state.secondaryFile && !isSameOrChildPath(state.secondaryFile.path, path)
      ? state.secondaryFile
      : null;
    let focusedPane = state.focusedPane;

    if (state.splitMode === 'single') {
      const activeFile = state.activeFile && !isSameOrChildPath(state.activeFile.path, path)
        ? state.activeFile
        : primaryFile ?? previewFile ?? openFiles[0] ?? null;

      return {
        openFiles,
        fileBuffers,
        dirtyFiles,
        diagnosticsMap,
        previewFile,
        primaryFile: activeFile,
        secondaryFile: null,
        activeFile,
        activeFileSymbols: activeFile ? state.activeFileSymbols : [],
        breakpoints: state.breakpoints.filter((breakpoint) => !isSameOrChildPath(breakpoint.path, path)),
      };
    }

    if (!primaryFile) {
      primaryFile = previewFile ?? getFallbackFile(openFiles, secondaryFile ? [secondaryFile.path] : []);
    }

    if (!secondaryFile) {
      secondaryFile = getFallbackFile(openFiles, primaryFile ? [primaryFile.path] : []);
    }

    if (focusedPane === 'primary' && !primaryFile && secondaryFile) {
      focusedPane = 'secondary';
    } else if (focusedPane === 'secondary' && !secondaryFile) {
      focusedPane = 'primary';
    }

    const activeFile = focusedPane === 'secondary' ? secondaryFile : primaryFile;

    return {
      openFiles,
      fileBuffers,
      dirtyFiles,
      diagnosticsMap,
      previewFile,
      primaryFile,
      secondaryFile,
      focusedPane,
      activeFile: activeFile ?? null,
      activeFileSymbols: activeFile ? state.activeFileSymbols : [],
      breakpoints: state.breakpoints.filter((breakpoint) => !isSameOrChildPath(breakpoint.path, path)),
    };
  }),

  setBottomPanelOpen: (open) => set({ isBottomPanelOpen: open }),
  setSidebarOpen: (open) => set({ isSidebarOpen: open }),
  setSidebarTab: (tab) => set({ activeSidebarTab: tab }),

  toggleZenMode: () => set((state) => ({ isZenMode: !state.isZenMode })),
  toggleSidebar: () => set((state) => ({ isSidebarOpen: !state.isSidebarOpen })),
  toggleInspector: () => set((state) => ({ isInspectorOpen: !state.isInspectorOpen })),
  toggleBottomPanel: () => set((state) => ({ isBottomPanelOpen: !state.isBottomPanelOpen })),

  stageFile: async (path) => {
    try {
      await invoke("git_stage", { path });
      await get().refreshGitStatus();
    } catch (err) {
      console.error("Failed to stage file", err);
    }
  },

  unstageFile: async (path) => {
    try {
      await invoke("git_unstage", { path });
      await get().refreshGitStatus();
    } catch (err) {
      console.error("Failed to unstage file", err);
    }
  },

  stageAll: async () => {
    try {
      await invoke("git_stage_all");
      await get().refreshGitStatus();
    } catch (err) {
      console.error("Failed to stage all files", err);
    }
  },

  unstageAll: async () => {
    try {
      await invoke("git_unstage_all");
      await get().refreshGitStatus();
    } catch (err) {
      console.error("Failed to unstage all files", err);
    }
  },

  discardChanges: async (path) => {
    try {
      await invoke("git_discard_changes", { path });
      await get().refreshGitStatus();
    } catch (err) {
      console.error("Failed to discard changes", err);
    }
  },

  commitChanges: async (message) => {
    try {
      await invoke("git_commit", { message });
      await get().refreshGitStatus();
    } catch (err) {
      console.error("Failed to commit changes", err);
    }
  },

  openDiff: (path) => set({ diffFile: path }),
  closeDiff: () => set({ diffFile: null }),

  refreshGitStatus: async () => {
    try {
      const status = await invoke<Record<string, string>>("get_git_status");
      set({ gitStatus: status });
    } catch (err) {
      console.warn("Failed to fetch git status", err);
    }
  },

  toggleBreakpoint: (path: string, line: number) => set((state) => {
    const exists = state.breakpoints.find(b => b.path === path && b.line === line);
    if (exists) {
      return { breakpoints: state.breakpoints.filter(b => !(b.path === path && b.line === line)) };
    }
    return { breakpoints: [...state.breakpoints, { path, line }] };
  }),

  clearBreakpoints: (path?: string) => set((state) => ({
    breakpoints: path
      ? state.breakpoints.filter(b => b.path !== path)
      : []
  })),

  loadAvailableWorkspaces: async () => {
    if (!isTauriEnvironment()) {
      set({ availableWorkspaces: [] });
      return;
    }

    try {
      const db = await Database.load('sqlite:zenith.db');
      const rows = await db.select<{ name: string }[]>('SELECT name FROM workspaces');
      set({ availableWorkspaces: rows.map(r => r.name) });
    } catch (err) {
      console.error('Failed to load workspaces', err);
    }
  },

  saveWorkspaceContext: async (name) => {
    if (!isTauriEnvironment()) {
      return;
    }

    try {
      const state = get();
      const snapshot: WorkbenchSessionSnapshot = {
        openFiles: state.openFiles,
        activeFile: state.activeFile,
        primaryFile: state.primaryFile,
        secondaryFile: state.secondaryFile,
        previewFile: state.previewFile,
        isBottomPanelOpen: state.isBottomPanelOpen,
        isSidebarOpen: state.isSidebarOpen,
        activeSidebarTab: state.activeSidebarTab,
        activeBottomTab: state.activeBottomTab,
        currentProjectRoot: state.currentProjectRoot,
        focusedPane: state.focusedPane,
        splitMode: state.splitMode,
        isInspectorOpen: state.isInspectorOpen,
        activeInspectorTab: state.activeInspectorTab,
        isZenMode: state.isZenMode,
        gitStatus: state.gitStatus,
        breakpoints: state.breakpoints,
      };

      const db = await Database.load('sqlite:zenith.db');
      await db.execute(
        'INSERT OR REPLACE INTO workspaces (name, snapshot, updated_at) VALUES ($1, $2, CURRENT_TIMESTAMP)',
        [name, JSON.stringify(snapshot)]
      );
      
      await get().loadAvailableWorkspaces();
    } catch (err) {
      console.error('Failed to save workspace', err);
    }
  },

  loadWorkspaceContext: async (name) => {
    if (!isTauriEnvironment()) {
      return;
    }

    try {
      const db = await Database.load('sqlite:zenith.db');
      const row = await db.select<{ snapshot: string }[]>(
        'SELECT snapshot FROM workspaces WHERE name = $1',
        [name]
      );

      if (row.length > 0) {
        const snapshot = JSON.parse(row[0].snapshot);
        set((state) => ({
          ...state,
          ...snapshot,
        }));
      }
    } catch (err) {
      console.error('Failed to load workspace', err);
    }
  },

  deleteWorkspaceContext: async (name) => {
    if (!isTauriEnvironment()) {
      return;
    }

    try {
      const db = await Database.load('sqlite:zenith.db');
      await db.execute('DELETE FROM workspaces WHERE name = $1', [name]);
      await get().loadAvailableWorkspaces();
    } catch (err) {
      console.error('Failed to delete workspace', err);
    }
  },

  fetchHistory: async (path: string) => {
    try {
      const history = await invoke<any[]>("history_get_snapshots", { path });
      set({ activeFileHistory: history });
    } catch (err) {
      console.error("Failed to fetch history:", err);
    }
  },

  setCursorPosition: (pos) => set({ cursorPosition: pos }),

  refreshGitBranch: async () => {
    try {
      const branch = await invoke<string>("get_git_branch");
      set({ gitCurrentBranch: branch || "main" });
    } catch (err) {
      console.error("Failed to refresh git branch:", err);
    }
  },

  refreshSystemStats: async () => {
    try {
      const stats = await invoke<any>("get_sys_info");
      set({ systemStats: {
        cpu: stats.cpu,
        memory: Number(stats.memory),
        memoryUsed: Number(stats.memory_used)
      }});
    } catch (err) {
      console.error("Failed to refresh system stats:", err);
    }
  },

  setSearchFilterPath: (path) => set({ searchFilterPath: path }),
}));

// Reactive Listeners
if (typeof window !== 'undefined' && (window as any).__TAURI_INTERNALS__) {
  // Listen for FS changes from Rust
  listen('zenith://fs-changed', () => {
    const state = useWorkspaceStore.getState();
    if (state.currentProjectRoot) {
      // Refresh tree and git status on change
      // Note: FileNavigator and Sidebar Navigator components already have their own refresh logic,
      // but we centralize here to ensure consistency.
      state.refreshGitStatus();

      // We can also trigger specific events for components to refresh their own recursive trees
      window.dispatchEvent(new CustomEvent('zenith://fs-changed'));
    }
  });

  // Initial Bootstrap for default root if needed
  setTimeout(() => {
    const root = useWorkspaceStore.getState().currentProjectRoot;
    if (root && root !== '.') {
      invoke("workspace_index_bootstrap", { rootPath: root }).catch(console.error);
    }
  }, 1000);
}

