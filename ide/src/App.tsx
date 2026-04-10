import { useEffect } from "react";
import { MainLayout } from "./components/MainLayout.tsx";
import { EditorPanel } from "./components/EditorPanel";
import { useWorkspaceStore } from "./store/useWorkspaceStore";
import { useCommandStore } from "./store/useCommandStore";
import { invoke } from "./utils/tauri";

function App() {
  const { loadSettings } = useWorkspaceStore();
  const registerCommand = useCommandStore(s => s.registerCommand);

  // Default IDE App-level Commands registration
  useEffect(() => {
     registerCommand({
         id: "zenith.workbench.action.showCommands",
         title: "Show All Commands",
         category: "View",
         action: () => useCommandStore.getState().setPaletteOpen(true, ">")
     });
     
     registerCommand({
         id: "zenith.workbench.action.quickOpen",
         title: "Go to File...",
         category: "Search",
         action: () => useCommandStore.getState().setPaletteOpen(true, "")
     });

     registerCommand({
         id: "zenith.workbench.action.openSettings",
         title: "Open Settings",
         category: "System",
         action: () => useCommandStore.getState().setSettingsOpen(true)
     });
     
     registerCommand({
         id: "zenith.workbench.action.findInFiles",
         title: "Find in Files",
         category: "Search",
         action: () => {
             const st = useWorkspaceStore.getState();
             st.setSidebarTab('search');
             if (!st.isSidebarOpen) st.setSidebarOpen(true);
         }
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
         }
     });

     registerCommand({
         id: "zenith.workbench.action.openFile",
         title: "Open File...",
         category: "File",
         action: async () => {
             try {
               const path = await invoke<string | null>("pick_file");
               if (path) {
                 const name = path.split(/[\\/]/).pop() || "file";
                 useWorkspaceStore.getState().openFile({ name, path, is_directory: false }, true);
               }
             } catch (e) {
               console.error("Failed to pick file", e);
             }
         }
     });
  }, [registerCommand]);

  useEffect(() => {
    loadSettings();
    
    const handleKeyDown = async (e: KeyboardEvent) => {
      if (e.ctrlKey) {
         const k = e.key.toLowerCase();
         // Explicitly block common browser overrides that we want to take over
         if (['p', 's', 'f', 'h', 'o', 'g', ','].includes(k)) {
            e.preventDefault();
         }
      }

      // Dynamic Keybind Dispatcher
      const keys = [];
      if (e.ctrlKey || e.metaKey) keys.push('ctrl');
      if (e.shiftKey) keys.push('shift');
      if (e.altKey) keys.push('alt');
      
      const keyChar = e.key.toLowerCase();
      
      // Ignore modifier keys by themselves
      if (!['control', 'shift', 'alt', 'meta'].includes(keyChar)) {
         keys.push(keyChar);
         const combo = keys.join('+');
         
         const keymap = useWorkspaceStore.getState().settings.keymap || {};
         const commandId = keymap[combo];
         
         if (commandId) {
             e.preventDefault();
             useCommandStore.getState().executeCommand(commandId);
         }
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [loadSettings]);

  return (
    <MainLayout>
      <EditorPanel />
    </MainLayout>
  );
}

export default App;
