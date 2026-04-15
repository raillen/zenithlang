import { StateCreator } from 'zustand';
import { SettingsSlice, WorkspaceState } from '../types/ide';
import { DEFAULT_RUNTIME_EXTENSION_CONFIGS } from '../utils/runtimeProfiles';
import { ZenithTheme } from '../themes';

export const createSettingsSlice: StateCreator<
  WorkspaceState,
  [],
  [],
  SettingsSlice
> = (set) => ({
  settings: {
    theme: 'zenith-dark',
    customThemes: {},
    extensions: DEFAULT_RUNTIME_EXTENSION_CONFIGS,
    editorFontSize: 13,
    editorFontFamily: "'JetBrains Mono', 'Fira Code', monospace",
    editorFontLigatures: true,
    editorMinimap: false,
    editorWordWrap: true,
    editorCursorBlinking: 'smooth',
    editorFormatOnSave: true,
    historyMaxSnapshots: 50,
    historyRetentionDays: 7,
    
    // Restoration
    keymap: {},
    restoreSession: true,
    language: 'pt',
    autoSave: 'onFocusChange',
    uiScale: 1.0,
    toolbar: {
      showWorkspaceSelector: true,
      showGitBranch: true,
      showQuickOpen: true,
      showDiagnostics: true,
      showRuntimeTarget: true,
      showPanelToggles: true,
      showLayoutControls: true,
      showFocusHints: true,
      showSettingsButton: true,
    },
  },
  updateSettings: (update) =>
    set((state) => ({
      settings: { ...state.settings, ...update },
    })),
  addCustomTheme: (theme: ZenithTheme) =>
    set((state) => ({
      settings: {
        ...state.settings,
        customThemes: { ...state.settings.customThemes, [theme.id]: theme },
      },
    })),
});
