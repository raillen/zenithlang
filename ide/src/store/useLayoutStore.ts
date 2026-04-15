import { StateCreator } from 'zustand';
import { WorkspaceState, LayoutSlice } from '../types/ide';

export const createLayoutSlice: StateCreator<
  WorkspaceState,
  [],
  [],
  LayoutSlice
> = (set) => ({
  isSidebarOpen: true,
  isInspectorOpen: false,
  isTerminalOpen: false,
  isZenMode: false,
  isBottomPanelOpen: true,
  activeSidebarTab: 'explorer',
  activeInspectorTab: 'details',
  activeBottomTab: 'console',
  panelSizes: {},

  toggleSidebar: () => set((state) => ({ isSidebarOpen: !state.isSidebarOpen })),
  toggleInspector: () => set((state) => ({ isInspectorOpen: !state.isInspectorOpen })),
  toggleTerminal: () => set((state) => ({ isTerminalOpen: !state.isTerminalOpen })),
  toggleZenMode: () => set((state) => ({ isZenMode: !state.isZenMode })),
  toggleBottomPanel: () => set((state) => ({ isBottomPanelOpen: !state.isBottomPanelOpen })),
  
  setSidebarOpen: (open) => set({ isSidebarOpen: open }),
  setInspectorOpen: (open) => set({ isInspectorOpen: open }),
  setBottomPanelOpen: (open) => set({ isBottomPanelOpen: open }),
  setSidebarTab: (tab) => set({ activeSidebarTab: tab }),
  setInspectorTab: (tab) => set({ activeInspectorTab: tab }),
  setBottomTab: (tab) => set({ activeBottomTab: tab }),
  setPanelSize: (key, size) =>
    set((state) => ({
      panelSizes: { ...state.panelSizes, [key]: size },
    })),
});
