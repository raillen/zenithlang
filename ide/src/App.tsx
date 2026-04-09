import { MainLayout } from "./components/MainLayout";
import { useProjectStore } from "./store/useProjectStore";
import { Play } from "lucide-react";
import { motion, AnimatePresence } from "framer-motion";
import { EditorPanel } from "./components/EditorPanel";
import { CommandPalette } from "./components/CommandPalette";
import { MarkdownPreview } from "./components/MarkdownPreview";
import { useTranslation } from "react-i18next";
import { safeInvoke as invoke } from "./utils/tauri";

function App() {
  const { activeFilePath, isBuilding, setIsBuilding, setBuildLogs } = useProjectStore();
  const { t } = useTranslation();

  const runBuild = async () => {
    if (!activeFilePath || isBuilding) return;
    
    setIsBuilding(true);
    setBuildLogs(t('loading'));
    
    try {
      const output = await invoke<string>('run_compiler', { inputPath: activeFilePath });
      setBuildLogs(output);
    } catch (err: any) {
      setBuildLogs(err.toString());
    } finally {
      setIsBuilding(false);
    }
  };

  const isMarkdown = activeFilePath?.endsWith('.md');

  return (
    <MainLayout>
      <CommandPalette />
      <div className="flex-1 flex flex-col">
        {/* Sub-toolbar (Breadcrumbs / Run info) */}
        <div className="h-10 flex items-center justify-between px-4 border-b border-white/5 bg-white/[0.02]">
          <div className="flex items-center gap-4">
            <button 
              onClick={runBuild}
              disabled={isBuilding}
              className={`flex items-center gap-1 bg-white/5 px-2 py-0.5 rounded-full border border-white/10 hover:bg-white/10 transition-colors ${isBuilding ? 'opacity-50 cursor-not-allowed' : ''}`}
            >
              <Play size={10} className={`${isBuilding ? 'text-white/20' : 'text-[#28C840] fill-[#28C840]'}`} />
              <span className="text-[10px] font-medium px-1">Build {activeFilePath?.split(/[/\\]/).pop()}</span>
              <div className="h-2 w-[1px] bg-white/10 mx-0.5" />
              <span className="text-[10px] opacity-30">v0.2</span>
            </button>
            
            {activeFilePath && (
              <div className="text-[11px] text-white/40 flex items-center gap-1">
                <span className="opacity-50">{t('project')}</span>
                <span className="px-1">/</span>
                <span className="text-white/60">{activeFilePath.replace('..\\', '').replace('..', '')}</span>
              </div>
            )}
          </div>

          <div className={`
            px-4 py-0.5 rounded-full border border-white/5 text-[10px] font-medium flex items-center gap-2 transition-all
            ${isBuilding ? 'bg-xcode-accent/20 border-xcode-accent/40' : 'bg-black/20'}
          `}>
            <div className={`w-1.5 h-1.5 rounded-full ${isBuilding ? 'bg-xcode-accent animate-ping' : 'bg-[#28C840]'} `} />
            <span className="opacity-60">{isBuilding ? 'Building...' : t('ready_status')}</span>
          </div>
        </div>

        {/* Editor or Preview Area */}
        <div className="flex-1 relative bg-black/5">
          <AnimatePresence mode="wait">
            {!activeFilePath ? (
              <motion.div 
                key="empty"
                initial={{ opacity: 0, y: 10 }}
                animate={{ opacity: 1, y: 0 }}
                exit={{ opacity: 0, y: -10 }}
                className="absolute inset-0 flex flex-col items-center justify-center pointer-events-none"
              >
                 <h1 className="text-3xl font-light tracking-tighter text-white/10 mb-2">Zenith Retina</h1>
                 <p className="text-xs text-white/5">{t('no_file_selected')}</p>
              </motion.div>
            ) : isMarkdown ? (
              <motion.div 
                key="markdown"
                initial={{ opacity: 0 }}
                animate={{ opacity: 1 }}
                exit={{ opacity: 0 }}
                className="absolute inset-0"
              >
                <MarkdownPreview path={activeFilePath} />
              </motion.div>
            ) : (
              <motion.div 
                key="editor"
                initial={{ opacity: 0 }}
                animate={{ opacity: 1 }}
                exit={{ opacity: 0 }}
                className="absolute inset-0 select-text"
              >
                <EditorPanel path={activeFilePath} />
              </motion.div>
            )}
          </AnimatePresence>
        </div>
      </div>
    </MainLayout>
  );
}

export default App;
