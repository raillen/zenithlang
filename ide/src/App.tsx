import { useEffect } from "react";
import { MainLayout } from "./components/MainLayout.tsx";
import { EditorPanel } from "./components/EditorPanel";
import { useWorkspaceStore } from "./store/useWorkspaceStore";
import { invoke } from "./utils/tauri";

function App() {
  const { setProjectRoot, openFile } = useWorkspaceStore();

  useEffect(() => {
    const handleKeyDown = async (e: KeyboardEvent) => {
      // Ctrl + Shift + O: Open Folder
      if (e.ctrlKey && e.shiftKey && e.key.toLowerCase() === 'o') {
        e.preventDefault();
        try {
          const path = await invoke<string | null>("pick_folder");
          if (path) setProjectRoot(path);
        } catch (err) {
          console.error("Failed to pick folder via shortcut", err);
        }
      }
      // Ctrl + O: Open File
      else if (e.ctrlKey && e.key.toLowerCase() === 'o') {
        e.preventDefault();
        try {
          const path = await invoke<string | null>("pick_file");
          if (path) {
            const name = path.split(/[\\/]/).pop() || "file";
            openFile({ name, path, is_directory: false }, true);
          }
        } catch (err) {
          console.error("Failed to pick file via shortcut", err);
        }
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [setProjectRoot, openFile]);

  return (
    <MainLayout>
      <EditorPanel />
    </MainLayout>
  );
}

export default App;
