import { create } from 'zustand';
import Database from '@tauri-apps/plugin-sql';
import { THEMES, defaultLight } from '../themes';
import { applyThemeToDOM } from '../utils/themeEngine';

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

export interface ExtensionConfig {
  compilerPath: string;
  lspPath: string;
  buildCommand: string;
  isEnabled: boolean;
}

export interface IDESettings {
  language: 'pt' | 'en' | 'es';
  theme: string;
  extensions: Record<string, ExtensionConfig>;
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
}

interface WorkspaceState {
  fileTree: FileEntry[];
  openFiles: FileEntry[];     // Files currently pinned/fixed as tabs
  activeFile: FileEntry | null;
  primaryFile: FileEntry | null;
  secondaryFile: FileEntry | null;
  previewFile: FileEntry | null; // The temporary "single-click" file
  dirtyFiles: Set<string>;      // Set of paths that are unsaved
  diagnosticsMap: Record<string, Diagnostic[]>; // Errors/Warnings per file
  isBottomPanelOpen: boolean;
  isSidebarOpen: boolean;
  activeSidebarTab: string;
  activeBottomTab: 'console' | 'terminal' | 'problems';
  currentProjectRoot: string;
  settings: IDESettings;
  activeFileSymbols: Symbol[];
  focusedPane: EditorPane;
  splitMode: EditorSplitMode;

  
  setFileTree: (tree: FileEntry[]) => void;
  openFile: (file: FileEntry, isFixed?: boolean) => void;
  closeFile: (path: string) => void;
  setFileDirty: (path: string, dirty: boolean) => void;
  setDiagnostics: (path: string, diagnostics: Diagnostic[]) => void;
  setBottomPanelOpen: (open: boolean) => void;
  setSidebarOpen: (open: boolean) => void;
  setSidebarTab: (tab: string) => void;
  setBottomTab: (tab: 'console' | 'terminal' | 'problems') => void;
  setProjectRoot: (path: string) => void;
  setActiveFileSymbols: (symbols: Symbol[]) => void;
  setFocusedPane: (pane: EditorPane) => void;
  setSplitMode: (mode: EditorSplitMode) => void;

  updateSettings: (newSettings: Partial<IDESettings>) => Promise<void>;
  loadSettings: () => Promise<void>;
}

const DEFAULT_SETTINGS: IDESettings = {
  language: 'pt',
  theme: 'light-retina',
  extensions: {
    "zenith": {
      compilerPath: "ztc.lua",
      lspPath: "",
      buildCommand: "lua ztc.lua build",
      isEnabled: true
    }
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
    "ctrl+shift+p": "zenith.workbench.action.showCommands",
    "ctrl+shift+f": "zenith.workbench.action.findInFiles",
    "ctrl+shift+o": "zenith.workbench.action.openFolder",
    "ctrl+o": "zenith.workbench.action.openFile",
    "ctrl+p": "zenith.workbench.action.quickOpen",
    "ctrl+,": "zenith.workbench.action.openSettings"
  }
};

const appendUniqueFile = (files: FileEntry[], file: FileEntry) =>
  files.some((entry) => entry.path === file.path) ? files : [...files, file];

const getFallbackFile = (files: FileEntry[], excludedPaths: string[] = []) =>
  files.find((entry) => !excludedPaths.includes(entry.path)) ?? null;

export const useWorkspaceStore = create<WorkspaceState>((set, get) => ({
  fileTree: [],
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


  setFileTree: (tree) => set({ fileTree: tree }),
  
  setDiagnostics: (path, diagnostics) => set((state) => ({
    diagnosticsMap: { ...state.diagnosticsMap, [path]: diagnostics }
  })),
  
  setBottomTab: (tab) => set({ activeBottomTab: tab }),

  updateSettings: async (newSettings) => {
    const updated = { ...get().settings, ...newSettings };
    set({ settings: updated });
    
    if (newSettings.theme) {
      applyThemeToDOM(THEMES[newSettings.theme] || defaultLight);
    }
    
    // Persist to SQLite
    try {
      const db = await Database.load("sqlite:zenith.db");
      for (const [key, value] of Object.entries(newSettings)) {
        const valToSave = typeof value === 'object' && value !== null ? JSON.stringify(value) : value;
        await db.execute(
          "INSERT OR REPLACE INTO settings (key, value) VALUES ($1, $2)",
          [key, valToSave]
        );
      }
    } catch (err) {
      console.error("Failed to persist settings", err);
    }
  },

  loadSettings: async () => {
    try {
      const db = await Database.load("sqlite:zenith.db");
      const rows = await db.select<{ key: string, value: string }[]>("SELECT key, value FROM settings");
      
      const loadedSettings: Partial<IDESettings> = {};
      rows.forEach(row => {
          if (row.key in DEFAULT_SETTINGS) {
              const defaultVal = (DEFAULT_SETTINGS as any)[row.key];
              let val: any = row.value;
              if (typeof defaultVal === 'boolean') val = val === 'true';
              else if (typeof defaultVal === 'number') val = Number(val);
              else if (typeof defaultVal === 'object' && defaultVal !== null) {
                  try {
                      val = JSON.parse(val);
                  } catch (e) {
                      val = defaultVal;
                  }
              }
              (loadedSettings as any)[row.key] = val;
          }
      });

      const finalSettings = { 
          ...DEFAULT_SETTINGS, 
          ...loadedSettings,
          keymap: { ...DEFAULT_SETTINGS.keymap, ...(loadedSettings.keymap || {}) }
      };
      set({ settings: finalSettings });
      
      // Bootstrap theme
      applyThemeToDOM(THEMES[finalSettings.theme] || defaultLight);
    } catch (err) {
      console.error("Failed to load settings from SQLite", err);
    }
  },

  setProjectRoot: (path) => set({ 
    currentProjectRoot: path,
    openFiles: [],
    activeFile: null,
    primaryFile: null,
    secondaryFile: null,
    previewFile: null,
    dirtyFiles: new Set(),
    diagnosticsMap: {},
    activeFileSymbols: [],
    focusedPane: 'primary',
    splitMode: 'single'
  }),

  setActiveFileSymbols: (symbols) => set({ activeFileSymbols: symbols }),

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
          ? (state.secondaryFile ?? state.primaryFile ?? state.activeFile)
          : (state.primaryFile ?? state.activeFile ?? state.secondaryFile);

      return {
        splitMode: 'single',
        focusedPane: 'primary',
        primaryFile: nextPrimaryFile,
        secondaryFile: null,
        activeFile: nextPrimaryFile,
        previewFile: state.previewFile?.path === nextPrimaryFile?.path ? state.previewFile : null,
      };
    }

    const primaryFile = state.primaryFile ?? state.activeFile ?? state.previewFile ?? state.openFiles[0] ?? null;

    return {
      splitMode: mode,
      primaryFile,
      activeFile: state.focusedPane === 'secondary' && state.secondaryFile
        ? state.secondaryFile
        : primaryFile,
      previewFile: state.previewFile?.path === primaryFile?.path ? state.previewFile : null,
    };
  }),

  
  openFile: (file, isFixed = false) => set((state) => {
    // If it's a folder, do nothing
    if (file.is_directory) return state;

    if (state.splitMode !== 'single') {
      const paneWithFile =
        state.primaryFile?.path === file.path
          ? 'primary'
          : state.secondaryFile?.path === file.path
            ? 'secondary'
            : null;

      if (paneWithFile) {
        return {
          focusedPane: paneWithFile,
          activeFile: paneWithFile === 'secondary' ? state.secondaryFile : state.primaryFile,
        };
      }

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
      // Pinning the file: Add to openFiles, clear preview if it was this file
      const isOpened = state.openFiles.some(f => f.path === file.path);
      return {
        activeFile: file,
        primaryFile: file,
        previewFile: state.previewFile?.path === file.path ? null : state.previewFile,
        openFiles: isOpened ? state.openFiles : [...state.openFiles, file],
      };
    } else {
      // Preview mode: Replaces the current previewFile
      const isAlreadyInOpenFiles = state.openFiles.some(f => f.path === file.path);
      
      return {
        activeFile: file,
        primaryFile: file,
        previewFile: isAlreadyInOpenFiles ? null : file,
      };
    }
  }),

  closeFile: (path) => set((state) => {
    const newOpenFiles = state.openFiles.filter(f => f.path !== path);
    const newDirtyFiles = new Set(state.dirtyFiles);
    newDirtyFiles.delete(path);
    const nextPreviewFile = state.previewFile?.path === path ? null : state.previewFile;

    if (state.splitMode === 'single') {
      const nextPrimaryFile =
        state.primaryFile?.path === path
          ? (nextPreviewFile ?? newOpenFiles[0] ?? null)
          : state.primaryFile;

      const nextActiveFile =
        state.activeFile?.path === path
          ? (nextPreviewFile ?? newOpenFiles[0] ?? null)
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
      nextPrimaryFile = nextPreviewFile ?? getFallbackFile(
        newOpenFiles,
        nextSecondaryFile ? [nextSecondaryFile.path] : []
      );
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

    const nextActiveFile = nextFocusedPane === 'secondary' ? nextSecondaryFile : nextPrimaryFile;

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
      // Auto-fix if it was a preview file
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
  
  setBottomPanelOpen: (open) => set({ isBottomPanelOpen: open }),
  setSidebarOpen: (open) => set({ isSidebarOpen: open }),
  
  setSidebarTab: (tab) => set({ activeSidebarTab: tab }),
}));
