import { useState, useEffect, useRef } from "react";
import { createPortal } from "react-dom";
import { Search, FileText } from "lucide-react";
import { invoke } from "@tauri-apps/api/core";
import { useCommandStore } from "../store/useCommandStore";
import { useWorkspaceStore } from "../store/useWorkspaceStore";

interface FileNameResult {
    name: string;
    path: string;
}

export function CommandPalette() {
   const { isPaletteOpen, setPaletteOpen, commands, executeCommand, initialQuery } = useCommandStore();
   const { settings, openFile } = useWorkspaceStore();
   
   const [query, setQuery] = useState("");
   const [selectedIndex, setSelectedIndex] = useState(0);
   const [fileResults, setFileResults] = useState<FileNameResult[]>([]);

   const inputRef = useRef<HTMLInputElement>(null);
   const listRef = useRef<HTMLDivElement>(null);

   const mode = query.startsWith(">") ? 'command' : 'file';
   const cleanQuery = mode === 'command' ? query.substring(1).trimStart() : query;

   // 1. COMMANDS LOGIC
   const commandsArray = Object.values(commands).filter(cmd => 
       cmd.title.toLowerCase().includes(cleanQuery.toLowerCase()) || 
       (cmd.category && cmd.category.toLowerCase().includes(cleanQuery.toLowerCase()))
   );
   commandsArray.sort((a, b) => {
       const catA = a.category || "Z";
       const catB = b.category || "Z";
       if (catA < catB) return -1;
       if (catA > catB) return 1;
       return a.title.localeCompare(b.title);
   });

   // Utility to find the reverse mapping (Command ID -> Keybinding)
   const getShortcutForCommand = (id: string) => {
       for (const [combo, cmdId] of Object.entries(settings.keymap || {})) {
           if (cmdId === id) return combo;
       }
       return null;
   };

   // 2. FILES LOGIC
   useEffect(() => {
       if (mode === 'file' && isPaletteOpen) {
           // Debounce search loosely
           const timer = setTimeout(async () => {
               try {
                   const res: FileNameResult[] = await invoke("search_file_names", { query: cleanQuery });
                   setFileResults(res);
                   setSelectedIndex(0);
               } catch(e) {
                   console.error("Quick Open file scan failed", e);
               }
           }, 100);
           return () => clearTimeout(timer);
       }
   }, [cleanQuery, mode, isPaletteOpen]);

   // 3. UI STATE
   useEffect(() => {
       if (isPaletteOpen) {
           setQuery(initialQuery);
           setSelectedIndex(0);
           setFileResults([]);
           setTimeout(() => inputRef.current?.focus(), 50);
       }
   }, [isPaletteOpen, initialQuery]);

   useEffect(() => {
       setSelectedIndex(0);
   }, [query]);

   useEffect(() => {
       // Scroll selected into view
       if (listRef.current) {
           const activeElement = listRef.current.children[selectedIndex] as HTMLElement;
           if (activeElement) {
                activeElement.scrollIntoView({ block: "nearest" });
           }
       }
   }, [selectedIndex]);

   if (!isPaletteOpen) return null;

   const activeListLength = mode === 'command' ? commandsArray.length : fileResults.length;

   const handleKeyDown = (e: React.KeyboardEvent) => {
       if (e.key === "Escape") {
           e.preventDefault();
           setPaletteOpen(false);
       } else if (e.key === "ArrowDown") {
           e.preventDefault();
           setSelectedIndex((prev) => (prev + 1) % activeListLength);
       } else if (e.key === "ArrowUp") {
           e.preventDefault();
           setSelectedIndex((prev) => (prev - 1 + activeListLength) % activeListLength);
       } else if (e.key === "Enter") {
           e.preventDefault();
           if (mode === 'command') {
               const cmd = commandsArray[selectedIndex];
               if (cmd) executeCommand(cmd.id);
           } else {
               const file = fileResults[selectedIndex];
               if (file) {
                   setPaletteOpen(false);
                   openFile({ name: file.name, path: file.path, is_directory: false }, false);
               }
           }
       }
   };

   const formatShortcut = (combo: string) => {
       return combo.split('+').map(k => {
           if (k === 'ctrl') return 'Ctrl';
           if (k === 'shift') return '⇧';
           if (k === 'alt') return 'Alt';
           return k.length === 1 ? k.toUpperCase() : k;
       }).join(' + ');
   };

   const renderList = () => {
       if (mode === 'command') {
           if (commandsArray.length === 0) {
               return <div className="py-8 text-center text-[12px] text-ide-text-dim">No commands matching '{cleanQuery}'</div>;
           }
           return commandsArray.map((cmd, i) => {
               const isActive = i === selectedIndex;
               const shortcut = getShortcutForCommand(cmd.id);

               return (
                   <div 
                      key={cmd.id}
                      onClick={() => executeCommand(cmd.id)}
                      onMouseEnter={() => setSelectedIndex(i)}
                      className={`flex items-center justify-between px-4 py-2 mx-2 rounded-lg cursor-pointer transition-colors ${isActive ? 'bg-primary text-white' : 'text-ide-text hover:bg-ide-panel hover:text-white'}`}
                   >
                       <div className="flex items-center gap-3 truncate">
                          <span className={`text-[10px] font-bold uppercase tracking-wider w-16 text-right shrink-0 ${isActive ? 'text-white/70' : 'text-primary'}`}>
                              {cmd.category || 'System'}
                          </span>
                          <span className="text-[12px] truncate">{cmd.title}</span>
                       </div>
                       
                       {shortcut && (
                           <div className={`flex items-center gap-1 text-[10px] font-mono px-2 py-0.5 rounded shrink-0 ${isActive ? 'bg-white/20 text-white' : 'bg-ide-bg text-ide-text-dim'}`}>
                               {formatShortcut(shortcut)}
                           </div>
                       )}
                   </div>
               )
           });
       } else {
           if (fileResults.length === 0) {
               return <div className="py-8 text-center text-[12px] text-ide-text-dim">No files matching '{cleanQuery}'</div>;
           }
           return fileResults.map((file, i) => {
               const isActive = i === selectedIndex;
               // Extrac dir path
               const dirpath = file.path.substring(0, file.path.length - file.name.length);

               return (
                   <div 
                      key={file.path}
                      onClick={() => {
                          setPaletteOpen(false);
                          openFile({ name: file.name, path: file.path, is_directory: false }, false);
                      }}
                      onMouseEnter={() => setSelectedIndex(i)}
                      className={`flex items-center justify-between px-4 py-2 mx-2 rounded-lg cursor-pointer transition-colors ${isActive ? 'bg-primary text-white' : 'text-ide-text hover:bg-ide-panel hover:text-white'}`}
                   >
                       <div className="flex items-center gap-2 truncate">
                          <FileText size={14} className={`${isActive ? 'text-white/70' : 'text-ide-text-dim'}`} />
                          <div className="flex items-baseline gap-1.5 overflow-hidden">
                             <span className="text-[12px] font-bold truncate">{file.name}</span>
                             <span className={`text-[10px] truncate ${isActive ? 'text-white/70' : 'text-ide-text-dim opacity-50'}`}>{dirpath}</span>
                          </div>
                       </div>
                   </div>
               )
           });
       }
   };

   const modalContent = (
       <div className="fixed inset-0 z-[99999] flex items-start justify-center pt-[15vh]" onClick={() => setPaletteOpen(false)}>
          <div className="absolute inset-0 bg-black/40 backdrop-blur-[2px] animate-in fade-in duration-200" />
          
          <div 
             className="relative w-[600px] max-w-[90vw] bg-ide-bg border border-ide-border rounded-xl shadow-2xl overflow-hidden flex flex-col animate-in slide-in-from-top-4 fade-in duration-200"
             onClick={e => e.stopPropagation()}
          >
              <div className="flex items-center px-4 py-3 border-b border-ide-border gap-3 bg-ide-panel/30">
                 <Search size={16} className={`${mode === 'command' ? 'text-primary' : 'text-ide-text-dim'} transition-colors`} />
                 <input 
                    ref={inputRef}
                    type="text"
                    value={query}
                    onChange={(e) => setQuery(e.target.value)}
                    onKeyDown={handleKeyDown}
                    placeholder={mode === 'command' ? "Type a command..." : "Search files by name..."}
                    className="flex-1 bg-transparent border-none text-[13px] text-ide-text font-mono focus:outline-none focus:ring-0 placeholder:text-ide-text-dim"
                 />
              </div>

              <div ref={listRef} className="max-h-[350px] overflow-y-auto py-2 flex flex-col gap-0.5">
                 {renderList()}
              </div>

              <div className="px-4 py-2 bg-ide-panel/50 border-t border-ide-border flex items-center justify-between text-[10px] text-ide-text-dim">
                 <div className="flex items-center gap-4">
                    <span className="flex items-center gap-1"><kbd className="bg-ide-bg border border-ide-border rounded px-1 pb-[1px]">↑</kbd> <kbd className="bg-ide-bg border border-ide-border rounded px-1 pb-[1px]">↓</kbd> navigate</span>
                    <span className="flex items-center gap-1"><kbd className="bg-ide-bg border border-ide-border rounded px-1 pb-[1px]">Enter</kbd> {mode === 'command' ? 'execute' : 'open'}</span>
                 </div>
                 <div className="flex items-center gap-4">
                    <span>{mode === 'command' ? "Command Mode" : "File Search Mode"}</span>
                    <span className="flex items-center gap-1"><kbd className="bg-ide-bg border border-ide-border rounded px-1 pb-[1px]">Esc</kbd> dismiss</span>
                 </div>
              </div>
          </div>
       </div>
   );

   return createPortal(modalContent, document.body);
}
