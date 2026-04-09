import { create } from 'zustand';
import { persist } from 'zustand/middleware';
import type { EditorFontFamily } from '../utils/editorPreferences';

export interface FileEntry {
  name: string;
  path: string;
  is_directory: boolean;
  children?: FileEntry[];
}

export type SidebarView = 'explorer' | 'search' | 'sourceControl' | 'run' | 'extensions';
export type BottomPanelTab = 'output' | 'problems' | 'terminal';
export type PanelDock = 'bottom' | 'left' | 'right';
export type TerminalSessionState = 'idle' | 'starting' | 'ready' | 'error' | 'exited';

export interface EditorStatus {
  line: number;
  column: number;
  language: string;
  selection: string;
}

const allowedDocks: PanelDock[] = ['bottom', 'left', 'right'];
const allowedTabs: BottomPanelTab[] = ['output', 'problems', 'terminal'];
const allowedThemes: Array<ProjectState['theme']> = ['zenith', 'nord', 'neon', 'dracula', 'neuro'];
const allowedFonts: Array<ProjectState['fontFamily']> = ['SF Mono', 'OpenDyslexic'];

function clamp(value: number, min: number, max: number) {
  return Math.min(max, Math.max(min, value));
}

function asFiniteNumber(value: unknown) {
  return typeof value === 'number' && Number.isFinite(value) ? value : null;
}

function normalizeSplit(primary: number, secondary: number) {
  const safePrimary = Number.isFinite(primary) ? Math.max(0.1, primary) : 50;
  const safeSecondary = Number.isFinite(secondary) ? Math.max(0.1, secondary) : 50;
  const total = safePrimary + safeSecondary;

  return {
    primary: (safePrimary / total) * 100,
    secondary: (safeSecondary / total) * 100,
  };
}

function normalizePair({
  rawPrimary,
  rawSecondary,
  fallbackPrimary,
  fallbackSecondary,
  primaryMin,
  primaryMax,
  secondaryMin,
  secondaryMax,
}: {
  rawPrimary: unknown;
  rawSecondary: unknown;
  fallbackPrimary: number;
  fallbackSecondary: number;
  primaryMin: number;
  primaryMax: number;
  secondaryMin: number;
  secondaryMax: number;
}) {
  const primaryValue = asFiniteNumber(rawPrimary) ?? fallbackPrimary;
  const secondaryValue = asFiniteNumber(rawSecondary) ?? fallbackSecondary;
  const normalized = normalizeSplit(primaryValue, secondaryValue);

  return {
    primary: clamp(normalized.primary, primaryMin, primaryMax),
    secondary: clamp(normalized.secondary, secondaryMin, secondaryMax),
  };
}

function sanitizePersistedState(
  persistedState: Partial<ProjectState> | undefined,
  currentState: ProjectState
): ProjectState {
  const persisted = persistedState ?? {};

  const panel = normalizePair({
    rawPrimary: persisted.panelLayout?.sidebar,
    rawSecondary: persisted.panelLayout?.editor,
    fallbackPrimary: currentState.panelLayout.sidebar,
    fallbackSecondary: currentState.panelLayout.editor,
    primaryMin: 16,
    primaryMax: 46,
    secondaryMin: 54,
    secondaryMax: 84,
  });

  const vertical = normalizePair({
    rawPrimary: persisted.mainVerticalLayout?.editor,
    rawSecondary: persisted.mainVerticalLayout?.bottom,
    fallbackPrimary: currentState.mainVerticalLayout.editor,
    fallbackSecondary: currentState.mainVerticalLayout.bottom,
    primaryMin: 55,
    primaryMax: 88,
    secondaryMin: 12,
    secondaryMax: 45,
  });

  const side = normalizePair({
    rawPrimary: persisted.sidePanelLayout?.editor,
    rawSecondary: persisted.sidePanelLayout?.utility,
    fallbackPrimary: currentState.sidePanelLayout.editor,
    fallbackSecondary: currentState.sidePanelLayout.utility,
    primaryMin: 54,
    primaryMax: 82,
    secondaryMin: 18,
    secondaryMax: 46,
  });

  return {
    ...currentState,
    ...persisted,
    workspaceRoot:
      typeof persisted.workspaceRoot === 'string'
        ? persisted.workspaceRoot
        : currentState.workspaceRoot,
    fileTree: Array.isArray(persisted.fileTree) ? persisted.fileTree : currentState.fileTree,
    openFiles: Array.isArray(persisted.openFiles)
      ? persisted.openFiles.filter((item): item is string => typeof item === 'string')
      : currentState.openFiles,
    recentFiles: Array.isArray(persisted.recentFiles)
      ? persisted.recentFiles.filter((item): item is string => typeof item === 'string')
      : currentState.recentFiles,
    activeFilePath:
      persisted.activeFilePath === null || typeof persisted.activeFilePath === 'string'
        ? persisted.activeFilePath
        : currentState.activeFilePath,
    panelLayout: {
      sidebar: panel.primary,
      editor: panel.secondary,
    },
    mainVerticalLayout: {
      editor: vertical.primary,
      bottom: vertical.secondary,
    },
    sidePanelLayout: {
      editor: side.primary,
      utility: side.secondary,
    },
    activeSidebarView:
      persisted.activeSidebarView === 'explorer' ||
      persisted.activeSidebarView === 'search' ||
      persisted.activeSidebarView === 'sourceControl' ||
      persisted.activeSidebarView === 'run' ||
      persisted.activeSidebarView === 'extensions'
        ? persisted.activeSidebarView
        : currentState.activeSidebarView,
    activeBottomPanelTab: allowedTabs.includes(
      persisted.activeBottomPanelTab as BottomPanelTab
    )
      ? (persisted.activeBottomPanelTab as BottomPanelTab)
      : currentState.activeBottomPanelTab,
    isBottomPanelOpen:
      typeof persisted.isBottomPanelOpen === 'boolean'
        ? persisted.isBottomPanelOpen
        : currentState.isBottomPanelOpen,
    utilityPanelDock: allowedDocks.includes(persisted.utilityPanelDock as PanelDock)
      ? (persisted.utilityPanelDock as PanelDock)
      : currentState.utilityPanelDock,
    theme: allowedThemes.includes(persisted.theme as ProjectState['theme'])
      ? (persisted.theme as ProjectState['theme'])
      : currentState.theme,
    fontFamily: allowedFonts.includes(persisted.fontFamily as ProjectState['fontFamily'])
      ? (persisted.fontFamily as ProjectState['fontFamily'])
      : currentState.fontFamily,
    editorFontFamily: persisted.editorFontFamily ?? currentState.editorFontFamily,
    editorFontSize: clamp(
      Math.round(asFiniteNumber(persisted.editorFontSize) ?? currentState.editorFontSize),
      11,
      24
    ),
    acrylicOpacity: clamp(
      Math.round(asFiniteNumber(persisted.acrylicOpacity) ?? currentState.acrylicOpacity),
      20,
      100
    ),
    acrylicBlur: clamp(
      Math.round(asFiniteNumber(persisted.acrylicBlur) ?? currentState.acrylicBlur),
      0,
      40
    ),
    focusMode:
      typeof persisted.focusMode === 'boolean' ? persisted.focusMode : currentState.focusMode,
  };
}

interface ProjectState {
  workspaceRoot: string;
  fileTree: FileEntry[];
  openFiles: string[];
  recentFiles: string[];
  dirtyFiles: string[];
  activeFilePath: string | null;
  panelLayout: {
    sidebar: number;
    editor: number;
  };
  mainVerticalLayout: {
    editor: number;
    bottom: number;
  };
  sidePanelLayout: {
    editor: number;
    utility: number;
  };
  activeSidebarView: SidebarView;
  activeBottomPanelTab: BottomPanelTab;
  isBottomPanelOpen: boolean;
  utilityPanelDock: PanelDock;
  editorStatus: EditorStatus;
  theme: 'zenith' | 'nord' | 'neon' | 'dracula' | 'neuro';
  fontFamily: 'SF Mono' | 'OpenDyslexic';
  editorFontFamily: EditorFontFamily;
  editorFontSize: number;
  acrylicOpacity: number;
  acrylicBlur: number;
  focusMode: boolean;
  isBuilding: boolean;
  buildLogs: string;
  terminalSessionId: number | null;
  terminalSessionState: TerminalSessionState;
  terminalSnapshot: string;
  terminalError: string | null;
  terminalResetNonce: number;
  
  // Actions
  setWorkspaceRoot: (root: string) => void;
  setFileTree: (tree: FileEntry[]) => void;
  openFile: (path: string) => void;
  closeFile: (path: string) => void;
  setActiveFile: (path: string | null) => void;
  setPanelLayout: (layout: Partial<ProjectState['panelLayout']>) => void;
  setMainVerticalLayout: (layout: Partial<ProjectState['mainVerticalLayout']>) => void;
  setSidePanelLayout: (layout: Partial<ProjectState['sidePanelLayout']>) => void;
  setActiveSidebarView: (view: SidebarView) => void;
  setActiveBottomPanelTab: (tab: BottomPanelTab) => void;
  setBottomPanelOpen: (isOpen: boolean) => void;
  setUtilityPanelDock: (dock: PanelDock) => void;
  setEditorStatus: (status: Partial<EditorStatus>) => void;
  setFileDirtyState: (path: string, isDirty: boolean) => void;
  setTheme: (theme: 'zenith' | 'nord' | 'neon' | 'dracula' | 'neuro') => void;
  setFontFamily: (font: 'SF Mono' | 'OpenDyslexic') => void;
  setEditorFontFamily: (font: EditorFontFamily) => void;
  setEditorFontSize: (size: number) => void;
  setAcrylicOpacity: (opacity: number) => void;
  setAcrylicBlur: (blur: number) => void;
  setFocusMode: (focusMode: boolean) => void;
  setIsBuilding: (isBuilding: boolean) => void;
  setBuildLogs: (logs: string) => void;
  setTerminalSession: (sessionId: number | null, sessionState?: TerminalSessionState) => void;
  setTerminalSessionState: (sessionState: TerminalSessionState) => void;
  appendTerminalSnapshot: (data: string) => void;
  clearTerminalSnapshot: () => void;
  setTerminalError: (error: string | null) => void;
  requestTerminalReset: () => void;
}

export const useProjectStore = create<ProjectState>()(
  persist(
    (set) => ({
      workspaceRoot: '..',
      fileTree: [],
      openFiles: [],
      recentFiles: [],
      dirtyFiles: [],
      activeFilePath: null,
      panelLayout: {
        sidebar: 24,
        editor: 76,
      },
      mainVerticalLayout: {
        editor: 72,
        bottom: 28,
      },
      sidePanelLayout: {
        editor: 74,
        utility: 26,
      },
      activeSidebarView: 'explorer',
      activeBottomPanelTab: 'terminal',
      isBottomPanelOpen: true,
      utilityPanelDock: 'bottom',
      editorStatus: {
        line: 1,
        column: 1,
        language: 'plaintext',
        selection: 'Ln 1, Col 1',
      },
      theme: 'zenith',
      fontFamily: 'SF Mono',
      editorFontFamily: 'SF Mono',
      editorFontSize: 13,
      acrylicOpacity: 76,
      acrylicBlur: 22,
      focusMode: true,
      isBuilding: false,
      buildLogs: '',
      terminalSessionId: null,
      terminalSessionState: 'idle',
      terminalSnapshot: '',
      terminalError: null,
      terminalResetNonce: 0,

      setWorkspaceRoot: (workspaceRoot) =>
        set({
          workspaceRoot,
          fileTree: [],
          openFiles: [],
          dirtyFiles: [],
          activeFilePath: null,
        }),
      setFileTree: (tree) => set({ fileTree: tree }),
      setMainVerticalLayout: (layout) =>
        set((state) => ({
          mainVerticalLayout: (() => {
            const rawEditor = layout.editor ?? state.mainVerticalLayout.editor;
            const rawBottom = layout.bottom ?? state.mainVerticalLayout.bottom;
            const normalized = normalizeSplit(rawEditor, rawBottom);

            return {
              editor: clamp(normalized.primary, 55, 88),
              bottom: clamp(normalized.secondary, 12, 45),
            };
          })(),
        })),
      setSidePanelLayout: (layout) =>
        set((state) => ({
          sidePanelLayout: (() => {
            const rawEditor = layout.editor ?? state.sidePanelLayout.editor;
            const rawUtility = layout.utility ?? state.sidePanelLayout.utility;
            const normalized = normalizeSplit(rawEditor, rawUtility);

            return {
              editor: clamp(normalized.primary, 54, 82),
              utility: clamp(normalized.secondary, 18, 46),
            };
          })(),
        })),
      setPanelLayout: (layout) =>
        set((state) => ({
          panelLayout: (() => {
            const rawSidebar = layout.sidebar ?? state.panelLayout.sidebar;
            const rawEditor = layout.editor ?? state.panelLayout.editor;
            const normalized = normalizeSplit(rawSidebar, rawEditor);

            return {
              sidebar: clamp(normalized.primary, 16, 46),
              editor: clamp(normalized.secondary, 54, 84),
            };
          })(),
        })),
      setActiveSidebarView: (activeSidebarView) => set({ activeSidebarView }),
      setActiveBottomPanelTab: (activeBottomPanelTab) => set({ activeBottomPanelTab }),
      setBottomPanelOpen: (isBottomPanelOpen) => set({ isBottomPanelOpen }),
      setUtilityPanelDock: (utilityPanelDock) =>
        set({
          utilityPanelDock,
          isBottomPanelOpen: true,
        }),
      setEditorStatus: (status) =>
        set((state) => ({
          editorStatus: {
            ...state.editorStatus,
            ...status,
          },
        })),
      setFileDirtyState: (path, isDirty) =>
        set((state) => ({
          dirtyFiles: isDirty
            ? [path, ...state.dirtyFiles.filter((item) => item !== path)]
            : state.dirtyFiles.filter((item) => item !== path),
        })),
      setTheme: (theme) => set({ theme }),
      setFontFamily: (fontFamily) => set({ fontFamily }),
      setEditorFontFamily: (editorFontFamily) => set({ editorFontFamily }),
      setEditorFontSize: (editorFontSize) =>
        set({ editorFontSize: Math.min(24, Math.max(11, Math.round(editorFontSize))) }),
      setAcrylicOpacity: (acrylicOpacity) =>
        set({ acrylicOpacity: Math.min(100, Math.max(20, Math.round(acrylicOpacity))) }),
      setAcrylicBlur: (acrylicBlur) =>
        set({ acrylicBlur: Math.min(40, Math.max(0, Math.round(acrylicBlur))) }),
      setFocusMode: (focusMode) => set({ focusMode }),
      setIsBuilding: (isBuilding) => set({ isBuilding }),
      setBuildLogs: (buildLogs) => set({ buildLogs }),
      setTerminalSession: (terminalSessionId, terminalSessionState) =>
        set({
          terminalSessionId,
          terminalSessionState:
            terminalSessionState ?? (terminalSessionId === null ? 'idle' : 'ready'),
        }),
      setTerminalSessionState: (terminalSessionState) => set({ terminalSessionState }),
      appendTerminalSnapshot: (data) =>
        set((state) => {
          const nextSnapshot = `${state.terminalSnapshot}${data}`;
          return {
            terminalSnapshot:
              nextSnapshot.length > 250_000
                ? nextSnapshot.slice(-250_000)
                : nextSnapshot,
          };
        }),
      clearTerminalSnapshot: () => set({ terminalSnapshot: '' }),
      setTerminalError: (terminalError) => set({ terminalError }),
      requestTerminalReset: () =>
        set((state) => ({
          terminalSessionId: null,
          terminalSessionState: 'idle',
          terminalSnapshot: '',
          terminalError: null,
          terminalResetNonce: state.terminalResetNonce + 1,
        })),
      
      openFile: (path) => set((state) => {
        const recentFiles = [path, ...state.recentFiles.filter((item) => item !== path)].slice(0, 8);

        if (state.openFiles.includes(path)) {
          return { activeFilePath: path, recentFiles };
        }

        return {
          openFiles: [...state.openFiles, path],
          recentFiles,
          activeFilePath: path
        };
      }),

      closeFile: (path) => set((state) => {
        const newOpenFiles = state.openFiles.filter(f => f !== path);
        let newActive = state.activeFilePath;
        if (state.activeFilePath === path) {
          newActive = newOpenFiles.length > 0 ? newOpenFiles[newOpenFiles.length - 1] : null;
        }
        return { openFiles: newOpenFiles, activeFilePath: newActive };
      }),

      setActiveFile: (path) => set((state) => ({
        activeFilePath: path,
        recentFiles:
          path === null
            ? state.recentFiles
            : [path, ...state.recentFiles.filter((item) => item !== path)].slice(0, 8),
      })),
    }),
    {
      name: 'zenith-ide-storage-v2',
      partialize: (state) => ({
        workspaceRoot: state.workspaceRoot,
        fileTree: state.fileTree,
        openFiles: state.openFiles,
        recentFiles: state.recentFiles,
        activeFilePath: state.activeFilePath,
        panelLayout: state.panelLayout,
        mainVerticalLayout: state.mainVerticalLayout,
        sidePanelLayout: state.sidePanelLayout,
        activeSidebarView: state.activeSidebarView,
        activeBottomPanelTab: state.activeBottomPanelTab,
        isBottomPanelOpen: state.isBottomPanelOpen,
        utilityPanelDock: state.utilityPanelDock,
        theme: state.theme,
        fontFamily: state.fontFamily,
        editorFontFamily: state.editorFontFamily,
        editorFontSize: state.editorFontSize,
        acrylicOpacity: state.acrylicOpacity,
        acrylicBlur: state.acrylicBlur,
        focusMode: state.focusMode,
      }),
      merge: (persistedState, currentState) =>
        sanitizePersistedState(persistedState as Partial<ProjectState> | undefined, currentState),
    }
  )
);
