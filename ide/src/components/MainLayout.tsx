import { Panel, Group as PanelGroup, Separator as PanelResizeHandle } from "react-resizable-panels";
import { FileTree } from "./FileTree";
import { FolderTree, Search, Code2, Box, Info, Settings, X } from "lucide-react";
import { useProjectStore } from "../store/useProjectStore";
import { motion } from "framer-motion";

interface MainLayoutProps {
  children: React.ReactNode;
}

export function MainLayout({ children }: MainLayoutProps) {
  const { openFiles, activeFilePath, setActiveFile, closeFile, theme, setTheme, fontFamily, setFontFamily } = useProjectStore();

  return (
    <motion.div 
      initial={{ opacity: 0 }}
      animate={{ opacity: 1 }}
      className="flex h-screen w-full select-none overflow-hidden bg-transparent transition-colors duration-500"
      data-theme={theme}
      data-font={fontFamily}
    >
      <PanelGroup orientation="horizontal" className="h-full w-full">
        {/* Navigator Panel */}
        <Panel 
          id="sidebar-navigator"
          defaultSize={25} 
          minSize={20} 
          maxSize={45} 
          className="xcode-sidebar-glass border-r border-white/10 flex flex-col"
        >
          {/* Traffic Lights Area */}
          <div data-tauri-drag-region className="h-10 flex items-center px-4 gap-2 shrink-0">
            <div className="w-3 h-3 rounded-full bg-[#FF5F57]" />
            <div className="w-3 h-3 rounded-full bg-[#FFBD2E]" />
            <div className="w-3 h-3 rounded-full bg-[#28C840]" />
          </div>

          <div className="flex justify-around py-2 border-b border-white/5 opacity-60 shrink-0">
            <FolderTree size={16} className="text-xcode-accent" />
            <Search size={16} />
            <Code2 size={16} />
            <Box size={16} />
            <Info size={16} />
          </div>

          <div className="flex-1 overflow-hidden">
            <FileTree />
          </div>

          {/* Theme & Font Switcher Area */}
          <div className="p-4 border-t border-white/5 flex flex-col gap-4">
            <div className="flex flex-col gap-2">
              <span className="text-[9px] uppercase tracking-widest opacity-30 font-bold">Theme</span>
              <div className="flex gap-2">
                {(['zenith', 'nord', 'neon', 'dracula', 'neuro'] as const).map((t) => (
                   <button 
                     key={t}
                     onClick={() => setTheme(t)}
                     title={t}
                     className={`w-4 h-4 rounded-full border border-white/20 transition-transform hover:scale-125 ${theme === t ? 'ring-2 ring-xcode-accent' : ''}`}
                     style={{ 
                       backgroundColor: t === 'zenith' ? '#212121' : 
                                        t === 'nord' ? '#88c0d0' : 
                                        t === 'neon' ? '#ff00ff' : 
                                        t === 'dracula' ? '#bd93f9' : '#f5f5dc' 
                     }}
                   />
                ))}
              </div>
            </div>

            <div className="flex items-center justify-between">
              <span className="text-[9px] uppercase tracking-widest opacity-30 font-bold">Dyslexic Font</span>
              <button 
                onClick={() => setFontFamily(fontFamily === 'SF Mono' ? 'OpenDyslexic' : 'SF Mono')}
                className={`w-8 h-4 rounded-full p-0.5 transition-colors ${fontFamily === 'OpenDyslexic' ? 'bg-xcode-accent' : 'bg-white/10'}`}
              >
                <div className={`w-3 h-3 bg-white rounded-full transition-transform ${fontFamily === 'OpenDyslexic' ? 'translate-x-4' : ''}`} />
              </button>
            </div>
            
            <div className="opacity-50">
               <Settings size={18} />
            </div>
          </div>
        </Panel>

        <PanelResizeHandle className="z-50 relative w-[2px] bg-white/5 hover:bg-xcode-accent/50 transition-colors after:content-[''] after:absolute after:left-[-10px] after:right-[-10px] after:top-0 after:bottom-0 after:cursor-col-resize" />

        {/* Editor Main Area */}
        <Panel 
          id="main-editor"
          className="flex flex-col xcode-editor-glass" 
          minSize={30}
        >
          {/* Header & Tabs */}
          <header data-tauri-drag-region className="h-10 flex flex-col border-b border-white/5">
            <div className="flex-1 flex items-center overflow-x-auto no-scrollbar pt-1">
              {openFiles.map((path) => {
                const fileName = path.split(/[/\\]/).pop() || path;
                const isActive = activeFilePath === path;
                return (
                  <div 
                    key={path}
                    onClick={() => setActiveFile(path)}
                    className={`
                      h-full px-4 flex items-center gap-2 border-r border-white/5 cursor-pointer text-xs transition-colors group
                      ${isActive ? 'bg-white/5 border-b border-b-xcode-accent' : 'opacity-50 hover:opacity-100'}
                    `}
                  >
                    <Code2 size={12} className={isActive ? 'text-xcode-accent' : ''} />
                    <span className="max-w-[120px] truncate">{fileName}</span>
                    <X 
                      size={10} 
                      className="opacity-0 group-hover:opacity-100 p-0.5 hover:bg-white/10 rounded-sm" 
                      onClick={(e) => {
                        e.stopPropagation();
                        closeFile(path);
                      }}
                    />
                  </div>
                );
              })}
            </div>
          </header>

          <div className="flex-1 flex flex-col relative overflow-hidden">
            {children}
          </div>

          {/* Footer Area */}
          <footer className="h-8 border-t border-white/5 bg-black/10 flex items-center px-4 text-[10px] text-white/40 uppercase tracking-widest justify-between shrink-0">
            <div>{activeFilePath ? `UTF-8  |  ${activeFilePath.split('.').pop()?.toUpperCase()}` : 'No file selected'}</div>
            {activeFilePath && <div>Line 1, Column 1</div>}
          </footer>
        </Panel>
      </PanelGroup>
    </motion.div>
  );
}
