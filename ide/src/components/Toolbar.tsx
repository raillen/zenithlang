import { useState } from "react";
import { Play, Square, Hammer, ChevronRight, Layout, Settings } from "lucide-react";
import { useWorkspaceStore, FileEntry } from "../store/useWorkspaceStore";
import { invoke } from "../utils/tauri";
import { useCommandStore } from "../store/useCommandStore";
import { useTranslation } from "../utils/i18n";
import { ZenithMenu } from "./ZenithMenu";

export function Toolbar() {
  const { dirtyFiles, activeFile, setBottomPanelOpen, isBottomPanelOpen } = useWorkspaceStore();
  const [isRunning, setIsRunning] = useState(false);
  const { setSettingsOpen } = useCommandStore();
  const { t } = useTranslation();

  const handleRun = async () => {
    const file = activeFile as FileEntry;
    if (!file) return;
    
    setIsRunning(true);
    setBottomPanelOpen(true);
    
    console.log(`[Zenith System] Compiling ${file.name}...`);
    
    try {
      const result = await invoke<string>("run_compiler", { inputPath: file.path });
      console.log("[Zenith System] Build Success:", result);
    } catch (err) {
      console.error("[Zenith System] Build Failed:", err);
    } finally {
      setIsRunning(false);
    }
  };

  return (
    <header className="xcode-toolbar justify-between select-none">
      {/* Left Area (Zenith Hub) */}
      <div className="flex items-center gap-2 w-1/4 px-4">
        <ZenithMenu />
      </div>

      {/* Project Status & Actions (Center-Right weighted) */}
      <div className="flex-1 max-w-2xl flex items-center gap-2">
        <div className="flex-1 bg-ide-bg rounded-md h-7 border border-ide-border flex items-center px-3 gap-2 overflow-hidden">
          <div className="flex items-center gap-1.5 text-[10px] font-medium text-ide-text-dim truncate">
            <span className="opacity-40">Zenith</span>
            <ChevronRight size={10} className="opacity-20" />
            <span className="opacity-40">v0.2</span>
            <ChevronRight size={10} className="opacity-20" />
            <span className={dirtyFiles.has(activeFile?.path || '') ? 'text-amber-600' : 'opacity-60'}>
              {activeFile ? (activeFile as FileEntry).name : t('toolbar.no_file')}
              {dirtyFiles.has(activeFile?.path || '') && " •"}
            </span>
          </div>
          
          <div className="ml-auto flex items-center gap-2">
            {isRunning ? (
              <div className="flex items-center gap-2 animate-pulse">
                  <span className="text-[9px] text-amber-600 font-bold uppercase tracking-wider">{t('toolbar.compiling')}</span>
              </div>
            ) : (
              <div className="flex items-center gap-2">
                  <div className="w-1.5 h-1.5 rounded-full bg-green-500" />
                  <span className="text-[9px] text-ide-text-dim font-bold uppercase tracking-wider">{t('toolbar.ready')}</span>
              </div>
            )}
          </div>
        </div>

        {/* Actions Group (Aligned to the Right of Status Bar) */}
        <div className="flex items-center gap-1">
          <button 
            onClick={handleRun}
            disabled={isRunning || !activeFile}
            title={t('toolbar.run_tooltip')}
            className={`p-1.5 rounded-md transition-all flex items-center justify-center ${isRunning ? 'bg-primary/20 text-primary' : 'hover:bg-black/5 text-ide-text-dim active:scale-95 disabled:opacity-30'}`}
          >
            {isRunning ? <Square size={14} fill="currentColor" /> : <Play size={14} fill="currentColor" />}
          </button>
          <button 
            title={t('toolbar.build_tooltip')}
            className="p-1.5 hover:bg-black/5 rounded-md transition-colors text-ide-text-dim disabled:opacity-30"
            disabled={!activeFile}
          >
            <Hammer size={14} />
          </button>
          
          <div className="h-4 w-[1px] bg-black/5 mx-1" />
          
          <button 
            title={t('toolbar.settings_tooltip')}
            onClick={() => setSettingsOpen(true)}
            className="p-1.5 hover:bg-black/5 rounded-md transition-colors text-ide-text-dim"
          >
            <Settings size={14} />
          </button>
        </div>
      </div>

      {/* Right Controls */}
      <div className="w-1/4 flex justify-end gap-1 px-4">
        <button 
          onClick={() => setBottomPanelOpen(!isBottomPanelOpen)}
          className={`p-1.5 rounded-md transition-colors ${isBottomPanelOpen ? 'bg-primary/10 text-primary' : 'hover:bg-black/5 text-ide-text-dim'}`}
        >
          <Layout size={16} />
        </button>
      </div>
    </header>
  );
}
