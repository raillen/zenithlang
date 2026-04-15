import { StateCreator } from 'zustand';
import { WorkspaceState, EditorSlice } from '../types/ide';

export const createEditorSlice: StateCreator<
  WorkspaceState,
  [],
  [],
  EditorSlice
> = (set, get) => ({
  openFiles: [],
  activeFile: null,
  primaryFile: null,
  secondaryFile: null,
  focusedPane: 'primary',
  splitMode: 'single',
  diagnosticsMap: {},
  symbols: {},
  cursorPosition: { line: 1, col: 1 },
  breakpoints: [],
  previewFile: null,
  activeFileSymbols: [],
  activeFileHistory: [],
  diffFile: null,
  hasLoadedSettings: false,

  openFile: (file, isFixed = true, pane) => {
    const targetPane = pane || get().focusedPane;
    set((state) => {
      // Check if file is already open
      const isAlreadyOpen = state.openFiles.some((f) => f.path === file.path);
      const nextOpenFiles = isAlreadyOpen ? state.openFiles : [...state.openFiles, file];

      const update: Partial<WorkspaceState> = {
        openFiles: nextOpenFiles,
        activeFile: file,
      };

      if (targetPane === 'primary') update.primaryFile = file;
      else update.secondaryFile = file;

      if (!isFixed) update.previewFile = file;
      else if (state.previewFile?.path === file.path) update.previewFile = null;

      return update;
    });
  },

  closeFile: (path) =>
    set((state) => {
      const nextOpenFiles = state.openFiles.filter((f) => f.path !== path);
      const update: Partial<WorkspaceState> = { openFiles: nextOpenFiles };

      if (state.activeFile?.path === path) {
        update.activeFile = nextOpenFiles[nextOpenFiles.length - 1] || null;
      }
      if (state.primaryFile?.path === path) update.primaryFile = null;
      if (state.secondaryFile?.path === path) update.secondaryFile = null;
      if (state.previewFile?.path === path) update.previewFile = null;

      return update;
    }),

  setFocusedPane: (pane) => set({ focusedPane: pane }),
  
  setSplitMode: (mode) => set({ splitMode: mode }),

  setDiagnostics: (path, diagnostics) =>
    set((state) => ({
      diagnosticsMap: { ...state.diagnosticsMap, [path]: diagnostics },
    })),

  setCursorPosition: (pos) => set({ cursorPosition: pos }),

  toggleBreakpoint: (path, line) =>
    set((state) => {
      const exists = state.breakpoints.some((b) => b.path === path && b.line === line);
      const nextBreakpoints = exists
        ? state.breakpoints.filter((b) => !(b.path === path && b.line === line))
        : [...state.breakpoints, { path, line }];
      return { breakpoints: nextBreakpoints };
    }),

  clearBreakpoints: () => set({ breakpoints: [] }),

  setDiffFile: (path) => set({ diffFile: path }),

  closeDiff: () => set({ diffFile: null }),

  setActiveFileSymbols: (symbols) => set({ activeFileSymbols: symbols }),

  fetchHistory: async (_path) => {
    // Placeholder - history logic usually involves invoke('git_history', { path })
    set({ activeFileHistory: [] });
  },

  loadSettings: async () => {
    // Logic moved to settings slice or hydrated from workspace context
    set({ hasLoadedSettings: true });
  },
});
