import { useEffect, useState } from 'react';
import { Command } from 'cmdk';
import { Search, File as FileIcon, Settings } from 'lucide-react';
import { useProjectStore } from '../store/useProjectStore';
import { motion, AnimatePresence } from 'framer-motion';

export function CommandPalette() {
  const [isOpen, setIsOpen] = useState(false);
  const { fileTree, openFile, setTheme } = useProjectStore();

  useEffect(() => {
    const down = (e: KeyboardEvent) => {
      if (e.key === 'p' && (e.metaKey || e.ctrlKey)) {
        e.preventDefault();
        setIsOpen((open) => !open);
      }
    };

    document.addEventListener('keydown', down);
    return () => document.removeEventListener('keydown', down);
  }, []);

  const flatFiles: {name: string, path: string}[] = [];
  const flatten = (items: any[]) => {
    items.forEach(item => {
      if (item.is_directory) flatten(item.children || []);
      else flatFiles.push({ name: item.name, path: item.path });
    });
  };
  flatten(fileTree);

  return (
    <AnimatePresence>
      {isOpen && (
        <>
          <motion.div 
            initial={{ opacity: 0 }}
            animate={{ opacity: 1 }}
            exit={{ opacity: 0 }}
            className="fixed inset-0 bg-black/40 backdrop-blur-sm z-50"
            onClick={() => setIsOpen(false)}
          />
          <motion.div 
            initial={{ opacity: 0, scale: 0.95, y: -20 }}
            animate={{ opacity: 1, scale: 1, y: 0 }}
            exit={{ opacity: 0, scale: 0.95, y: -20 }}
            className="fixed top-[20%] left-1/2 -translate-x-1/2 w-[600px] max-w-[90vw] z-[51]"
          >
            <Command className="bg-[#1E1E1E]/90 backdrop-blur-2xl border border-white/10 rounded-xl shadow-2xl overflow-hidden text-white font-sans">
              <div className="flex items-center px-4 border-b border-white/10">
                <Search className="w-4 h-4 opacity-50 mr-3" />
                <Command.Input 
                  placeholder="Seach files or commands..." 
                  className="w-full h-12 bg-transparent outline-none text-sm placeholder:text-white/20"
                />
              </div>

              <Command.List className="max-h-[300px] overflow-y-auto p-2 scrollbar-thin">
                <Command.Empty className="p-4 text-center text-xs opacity-30 italic">
                  No results found.
                </Command.Empty>

                <Command.Group heading="Files" className="px-2 mb-2 text-[10px] uppercase tracking-widest opacity-30 font-bold">
                  {flatFiles.map(file => (
                    <Command.Item 
                      key={file.path}
                      onSelect={() => {
                        openFile(file.path);
                        setIsOpen(false);
                      }}
                      className="flex items-center gap-3 px-3 py-2 rounded-lg cursor-pointer aria-selected:bg-xcode-accent/80 transition-colors text-sm"
                    >
                      <FileIcon size={14} className="opacity-50" />
                      <span>{file.name}</span>
                      <span className="ml-auto text-[10px] opacity-30 truncate max-w-[200px]">{file.path}</span>
                    </Command.Item>
                  ))}
                </Command.Group>

                <Command.Group heading="Actions" className="px-2 text-[10px] uppercase tracking-widest opacity-30 font-bold mt-4">
                  <Command.Item 
                     onSelect={() => setTheme('zenith')}
                     className="flex items-center gap-3 px-3 py-2 rounded-lg cursor-pointer aria-selected:bg-xcode-accent/80 transition-colors text-sm"
                  >
                    <Settings size={14} />
                    <span>Switch to Zenith Theme</span>
                  </Command.Item>
                  <Command.Item 
                     onSelect={() => setTheme('nord')}
                     className="flex items-center gap-3 px-3 py-2 rounded-lg cursor-pointer aria-selected:bg-xcode-accent/80 transition-colors text-sm"
                  >
                    <Settings size={14} />
                    <span>Switch to Nord Theme</span>
                  </Command.Item>
                </Command.Group>
              </Command.List>
              
              <div className="p-2 bg-black/20 text-[9px] opacity-30 flex gap-4 justify-end border-t border-white/5">
                <div className="flex items-center gap-1"><kbd className="bg-white/10 px-1 rounded">↵</kbd> select</div>
                <div className="flex items-center gap-1"><kbd className="bg-white/10 px-1 rounded">↑↓</kbd> navigate</div>
                <div className="flex items-center gap-1"><kbd className="bg-white/10 px-1 rounded">esc</kbd> close</div>
              </div>
            </Command>
          </motion.div>
        </>
      )}
    </AnimatePresence>
  );
}
