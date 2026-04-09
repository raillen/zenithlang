import { create } from 'zustand';

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

interface WorkspaceState {
  fileTree: FileEntry[];
  openFiles: FileEntry[];     // Files currently pinned/fixed as tabs
  activeFile: FileEntry | null;
  previewFile: FileEntry | null; // The temporary "single-click" file
  dirtyFiles: Set<string>;      // Set of paths that are unsaved
  diagnosticsMap: Record<string, Diagnostic[]>; // Errors/Warnings per file
  isBottomPanelOpen: boolean;
  activeSidebarTab: string;
  activeBottomTab: 'console' | 'terminal' | 'problems';
  currentProjectRoot: string;
  
  setFileTree: (tree: FileEntry[]) => void;
  openFile: (file: FileEntry, isFixed?: boolean) => void;
  closeFile: (path: string) => void;
  setFileDirty: (path: string, dirty: boolean) => void;
  setDiagnostics: (path: string, diagnostics: Diagnostic[]) => void;
  setBottomPanelOpen: (open: boolean) => void;
  setSidebarTab: (tab: string) => void;
  setBottomTab: (tab: 'console' | 'terminal' | 'problems') => void;
  setProjectRoot: (path: string) => void;
}

export const useWorkspaceStore = create<WorkspaceState>((set) => ({
  fileTree: [],
  openFiles: [],
  activeFile: null,
  previewFile: null,
  dirtyFiles: new Set(),
  diagnosticsMap: {},
  isBottomPanelOpen: false,
  activeSidebarTab: 'navigator',
  activeBottomTab: 'console',
  currentProjectRoot: '.',

  setFileTree: (tree) => set({ fileTree: tree }),
  
  setDiagnostics: (path, diagnostics) => set((state) => ({
    diagnosticsMap: { ...state.diagnosticsMap, [path]: diagnostics }
  })),
  
  setBottomTab: (tab) => set({ activeBottomTab: tab }),

  setProjectRoot: (path) => set({ 
    currentProjectRoot: path,
    openFiles: [],
    activeFile: null,
    previewFile: null,
    dirtyFiles: new Set(),
    diagnosticsMap: {}
  }),
  
  openFile: (file, isFixed = false) => set((state) => {
    // If it's a folder, do nothing
    if (file.is_directory) return state;

    if (isFixed) {
      // Pinning the file: Add to openFiles, clear preview if it was this file
      const isOpened = state.openFiles.some(f => f.path === file.path);
      return {
        activeFile: file,
        previewFile: state.previewFile?.path === file.path ? null : state.previewFile,
        openFiles: isOpened ? state.openFiles : [...state.openFiles, file],
      };
    } else {
      // Preview mode: Replaces the current previewFile
      const isAlreadyInOpenFiles = state.openFiles.some(f => f.path === file.path);
      
      return {
        activeFile: file,
        previewFile: isAlreadyInOpenFiles ? null : file,
      };
    }
  }),

  closeFile: (path) => set((state) => {
    const newOpenFiles = state.openFiles.filter(f => f.path !== path);
    const newDirtyFiles = new Set(state.dirtyFiles);
    newDirtyFiles.delete(path);

    const isPathInPreview = state.previewFile?.path === path;
    const isPathActive = state.activeFile?.path === path;

    return {
      openFiles: newOpenFiles,
      dirtyFiles: newDirtyFiles,
      previewFile: isPathInPreview ? null : state.previewFile,
      activeFile: isPathActive 
        ? (isPathInPreview ? (newOpenFiles[0] || null) : (state.previewFile || newOpenFiles[0] || null))
        : state.activeFile
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
              openFiles: [...state.openFiles, file]
          };
      }
    } else {
      newDirtyFiles.delete(path);
    }
    return { dirtyFiles: newDirtyFiles };
  }),
  
  setBottomPanelOpen: (open) => set({ isBottomPanelOpen: open }),
  
  setSidebarTab: (tab) => set({ activeSidebarTab: tab }),
}));
