import { useEffect } from "react";
import { MainLayout } from "./components/MainLayout.tsx";
import { EditorPanel } from "./components/EditorPanel";
import {
  clearPersistedWorkbenchSession,
  persistWorkbenchSession,
  useWorkspaceStore,
} from "./store/useWorkspaceStore";
import { useCommandStore } from "./store/useCommandStore";
import { invoke } from "./utils/tauri";
import {
  executePipelineAction,
  stopActivePipelineTask,
} from "./utils/pipelineActions";

function requestExplorerCreate(type: "file" | "folder") {
  const workspace = useWorkspaceStore.getState();
  workspace.setSidebarTab("navigator");
  workspace.setSidebarOpen(true);
  window.setTimeout(() => {
    window.dispatchEvent(new CustomEvent("zenith://explorer-create", {
      detail: { type },
    }));
  }, 0);
}

function App() {
  const {
    loadSettings,
    hasLoadedSettings,
    settings,
    currentProjectRoot,
    openFiles,
    activeFile,
    primaryFile,
    secondaryFile,
    previewFile,
    isBottomPanelOpen,
    isSidebarOpen,
    activeSidebarTab,
    activeBottomTab,
    focusedPane,
    splitMode,
    isInspectorOpen,
    activeInspectorTab,
    isZenMode,
    gitStatus,
    breakpoints,
  } = useWorkspaceStore();
  const registerCommand = useCommandStore((s) => s.registerCommand);

  useEffect(() => {
    registerCommand({
      id: "zenith.workbench.action.showCommands",
      title: "Show All Commands",
      category: "View",
      action: () => useCommandStore.getState().setPaletteOpen(true, ">"),
    });

    registerCommand({
      id: "zenith.workbench.action.quickOpen",
      title: "Go to File...",
      category: "Search",
      action: () => useCommandStore.getState().setPaletteOpen(true, ""),
    });

    registerCommand({
      id: "zenith.workbench.action.openSettings",
      title: "Open Settings",
      category: "System",
      action: () => useCommandStore.getState().setSettingsOpen(true),
    });

    registerCommand({
      id: "zenith.workbench.action.findInFiles",
      title: "Find in Files",
      category: "Search",
      action: () => {
        const st = useWorkspaceStore.getState();
        st.setSidebarTab("search");
        if (!st.isSidebarOpen) st.setSidebarOpen(true);
      },
    });

    registerCommand({
      id: "zenith.workbench.action.openFolder",
      title: "Open Folder...",
      category: "File",
      action: async () => {
        try {
          const path = await invoke<string | null>("pick_folder");
          if (path) useWorkspaceStore.getState().setProjectRoot(path);
        } catch (e) {
          console.error("Failed to pick folder", e);
        }
      },
    });

    registerCommand({
      id: "zenith.workbench.action.openFile",
      title: "Open File...",
      category: "File",
      action: async () => {
        try {
          const path = await invoke<string | null>("pick_file");
          if (path) {
            const name = path.split(/[\/\\]/).pop() || "file";
            useWorkspaceStore.getState().openFile({ name, path, is_directory: false }, true);
          }
        } catch (e) {
          console.error("Failed to pick file", e);
        }
      },
    });

    registerCommand({
      id: "zenith.files.newFile",
      title: "New File",
      category: "File",
      action: () => requestExplorerCreate("file"),
    });

    registerCommand({
      id: "zenith.files.newFolder",
      title: "New Folder",
      category: "File",
      action: () => requestExplorerCreate("folder"),
    });

    registerCommand({
      id: "zenith.pipeline.run",
      title: "Run Active Target",
      category: "Zenith",
      action: () => {
        void executePipelineAction("run");
      },
    });

    registerCommand({
      id: "zenith.pipeline.build",
      title: "Build Active Target",
      category: "Zenith",
      action: () => {
        void executePipelineAction("build");
      },
    });

    registerCommand({
      id: "zenith.pipeline.stop",
      title: "Stop Active Task",
      category: "Zenith",
      action: () => {
        void stopActivePipelineTask();
      },
    });

    registerCommand({
      id: "zenith.workbench.action.splitEditorRight",
      title: "Split Editor Right",
      category: "View",
      action: () => {
        const st = useWorkspaceStore.getState();
        st.setSplitMode("vertical");
        st.setFocusedPane("secondary");
      },
    });

    registerCommand({
      id: "zenith.workbench.action.splitEditorDown",
      title: "Split Editor Down",
      category: "View",
      action: () => {
        const st = useWorkspaceStore.getState();
        st.setSplitMode("horizontal");
        st.setFocusedPane("secondary");
      },
    });

    registerCommand({
      id: "zenith.workbench.action.singleEditor",
      title: "Single Editor Layout",
      category: "View",
      action: () => {
        useWorkspaceStore.getState().setSplitMode("single");
      },
    });

    registerCommand({
      id: "zenith.workbench.action.focusPrimaryEditorGroup",
      title: "Focus Primary Editor",
      category: "View",
      action: () => {
        useWorkspaceStore.getState().setFocusedPane("primary");
      },
    });

    registerCommand({
      id: "zenith.workbench.action.focusSecondaryEditorGroup",
      title: "Focus Secondary Editor",
      category: "View",
      action: () => {
        const st = useWorkspaceStore.getState();
        if (st.splitMode === "single") {
          st.setSplitMode("vertical");
        }
        st.setFocusedPane("secondary");
      },
    });

    registerCommand({
      id: "zenith.workbench.action.toggleInspector",
      title: "Toggle Inspector",
      category: "View",
      action: () => {
        const st = useWorkspaceStore.getState();
        st.setInspectorOpen(!st.isInspectorOpen);
      },
    });
  }, [registerCommand]);

  useEffect(() => {
    void loadSettings();

    const handleKeyDown = (e: KeyboardEvent) => {
      const activeElement = document.activeElement;
      const isTyping =
        activeElement?.tagName === "INPUT" ||
        activeElement?.tagName === "TEXTAREA" ||
        activeElement instanceof HTMLElement &&
          (activeElement.isContentEditable ||
            !!activeElement.closest(".monaco-editor") ||
            !!activeElement.closest(".xterm"));

      // 1. Determine key combination
      const keys = [];
      if (e.ctrlKey || e.metaKey) keys.push("ctrl");
      if (e.shiftKey) keys.push("shift");
      if (e.altKey) keys.push("alt");

      const keyChar = e.key.toLowerCase();
      if (["control", "shift", "alt", "meta"].includes(keyChar)) return;

      keys.push(keyChar);
      const combo = keys.join("+");

      // 2. Identify if this combo is an IDE command
      const keymap = useWorkspaceStore.getState().settings.keymap || {};
      const commandId = keymap[combo];

      if (commandId) {
        // If it's a typing context, only prevent default if it's a real shortcut combo (ctrl/alt/meta)
        // This ensures Space, Tab, Enter work in the editor unless they are explicitly mapped as IDE shortcuts
        const isModifierCombo = e.ctrlKey || e.metaKey || e.altKey || combo.startsWith("f");
        
        if (!isTyping || isModifierCombo) {
          e.preventDefault();
          e.stopPropagation();
          useCommandStore.getState().executeCommand(commandId);
        }
      }
    };

    window.addEventListener("keydown", handleKeyDown);
    return () => window.removeEventListener("keydown", handleKeyDown);
  }, [loadSettings]);

  useEffect(() => {
    if (!hasLoadedSettings) return;

    if (!settings.restoreSession) {
      clearPersistedWorkbenchSession();
      return;
    }

    persistWorkbenchSession({
      currentProjectRoot,
      openFiles,
      activeFile,
      primaryFile,
      secondaryFile,
      previewFile,
      isBottomPanelOpen,
      isSidebarOpen,
      activeSidebarTab,
      activeBottomTab,
      focusedPane,
      splitMode,
      isInspectorOpen,
      activeInspectorTab,
      isZenMode,
      gitStatus,
      breakpoints,
    });
  }, [
    activeBottomTab,
    activeFile,
    activeInspectorTab,
    breakpoints,
    activeSidebarTab,
    currentProjectRoot,
    focusedPane,
    gitStatus,
    hasLoadedSettings,
    isBottomPanelOpen,
    isInspectorOpen,
    isSidebarOpen,
    isZenMode,
    openFiles,
    previewFile,
    primaryFile,
    secondaryFile,
    settings.restoreSession,
    splitMode,
  ]);

  return (
    <MainLayout>
      <EditorPanel />
    </MainLayout>
  );
}

export default App;
