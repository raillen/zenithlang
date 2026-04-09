import { create } from 'zustand';
import { persist } from 'zustand/middleware';

export interface FileEntry {
  name: string;
  path: string;
  is_directory: boolean;
  children?: FileEntry[];
}

interface ProjectState {
  fileTree: FileEntry[];
  openFiles: string[];
  activeFilePath: string | null;
  theme: 'zenith' | 'nord' | 'neon' | 'dracula' | 'neuro';
  fontFamily: 'SF Mono' | 'OpenDyslexic';
  isBuilding: boolean;
  buildLogs: string;
  
  // Actions
  setFileTree: (tree: FileEntry[]) => void;
  openFile: (path: string) => void;
  closeFile: (path: string) => void;
  setActiveFile: (path: string | null) => void;
  setTheme: (theme: 'zenith' | 'nord' | 'neon' | 'dracula' | 'neuro') => void;
  setFontFamily: (font: 'SF Mono' | 'OpenDyslexic') => void;
  setIsBuilding: (isBuilding: boolean) => void;
  setBuildLogs: (logs: string) => void;
}

export const useProjectStore = create<ProjectState>()(
  persist(
    (set) => ({
      fileTree: [],
      openFiles: [],
      activeFilePath: null,
      theme: 'zenith',
      fontFamily: 'SF Mono',
      isBuilding: false,
      buildLogs: '',

      setFileTree: (tree) => set({ fileTree: tree }),
      setTheme: (theme) => set({ theme }),
      setFontFamily: (fontFamily) => set({ fontFamily }),
      setIsBuilding: (isBuilding) => set({ isBuilding }),
      setBuildLogs: (buildLogs) => set({ buildLogs }),
      
      openFile: (path) => set((state) => {
        if (state.openFiles.includes(path)) {
          return { activeFilePath: path };
        }
        return { 
          openFiles: [...state.openFiles, path],
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

      setActiveFile: (path) => set({ activeFilePath: path }),
    }),
    {
      name: 'zenith-ide-storage',
    }
  )
);
