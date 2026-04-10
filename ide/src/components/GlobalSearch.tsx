import { useState } from "react";
import { invoke } from "@tauri-apps/api/core";
import { Search, Loader2, ChevronRight, ChevronDown, FileText } from "lucide-react";
import { useWorkspaceStore } from "../store/useWorkspaceStore";

interface SearchMatch {
    line_number: number;
    line_content: string;
}

interface FileResult {
    file_path: string;
    matches: SearchMatch[];
}

export function GlobalSearch() {
  const [query, setQuery] = useState("");
  const [results, setResults] = useState<FileResult[]>([]);
  const [isSearching, setIsSearching] = useState(false);
  const [error, setError] = useState<string | null>(null);
  
  // NOTE: openFileAndGoToLine -> For now we use the existing global openFile which defaults to tab mapping.
  // The actual scroll-to-line support will be added deeply in EditorPanel later.
  const { openFile } = useWorkspaceStore();

  const handleSearch = async () => {
    if (!query.trim()) {
      setResults([]);
      return;
    }

    setIsSearching(true);
    setError(null);
    try {
      const res: FileResult[] = await invoke("search_in_files", {
        rootPath: useWorkspaceStore.getState().currentProjectRoot,
        query,
        isRegex: false,
        matchCase: false
      });
      setResults(res);
    } catch (err: any) {
      setError(err.toString());
    } finally {
      setIsSearching(false);
    }
  };

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter') handleSearch();
  };

  return (
    <div className="flex flex-col h-full bg-ide-bg text-ide-text">
       <div className="p-4 border-b border-ide-border flex flex-col gap-2 shrink-0 bg-ide-panel/30">
          <div className="relative group">
             <input 
               autoFocus
               type="text" 
               value={query}
               onChange={(e) => setQuery(e.target.value)}
               onKeyDown={handleKeyDown}
               placeholder="Search (Enter)..."
               className="w-full bg-ide-bg border border-ide-border rounded-lg pl-8 pr-3 py-1.5 text-[11px] focus:outline-none focus:border-primary focus:ring-1 focus:ring-primary/20 transition-all font-mono"
             />
             <Search size={12} className="absolute left-2.5 top-2 text-ide-text-dim group-focus-within:text-primary transition-colors" />
          </div>
       </div>

       <div className="flex-1 overflow-y-auto p-2">
          {isSearching && (
             <div className="flex items-center justify-center p-8 text-ide-text-dim">
                <Loader2 size={16} className="animate-spin text-primary" />
             </div>
          )}

          {error && (
             <div className="p-4 text-[11px] text-red-500 break-words border border-red-500/20 bg-red-500/5 rounded-lg m-2">
                <span className="font-bold block mb-1">Search Error</span>
                {error}
             </div>
          )}

          {!isSearching && results.length === 0 && query.trim() !== "" && !error && (
             <div className="p-8 text-[11px] text-ide-text-dim text-center">
                Nenhum resultado encontrado para <span className="text-ide-text">"{query}"</span>.
             </div>
          )}

          {!isSearching && results.length > 0 && (
             <div className="flex flex-col gap-2">
                {results.map((file, i) => (
                   <FileResultItem 
                      key={i} 
                      file={file} 
                      onMatchClick={(_line) => {
                         const filename = file.file_path.split('/').pop() || file.file_path;
                         openFile({
                             name: filename,
                             path: file.file_path,
                             is_directory: false
                         }, false); 
                      }} 
                      query={query}
                   />
                ))}
             </div>
          )}
       </div>
    </div>
  );
}

function FileResultItem({ file, onMatchClick, query }: { file: FileResult, onMatchClick: (line: number) => void, query: string }) {
  const [expanded, setExpanded] = useState(true);

  const filename = file.file_path.split('/').pop() || file.file_path;
  const dirpath = file.file_path.substring(0, file.file_path.length - filename.length);

  return (
    <div className="flex flex-col">
       <div 
          className="flex items-center gap-2 py-1 px-2 hover:bg-ide-panel cursor-pointer rounded-md group transition-colors"
          onClick={() => setExpanded(!expanded)}
       >
          <div className="w-4 h-4 flex items-center justify-center text-ide-text-dim">
            {expanded ? <ChevronDown size={14} /> : <ChevronRight size={14} />}
          </div>
          <FileText size={12} className="text-ide-text-dim opacity-70 group-hover:text-primary transition-colors" />
          <div className="flex items-baseline gap-1 overflow-hidden">
             <span className="text-[11px] font-bold text-ide-text truncate">{filename}</span>
             <span className="text-[10px] text-ide-text-dim opacity-50 truncate">{dirpath}</span>
          </div>
          <span className="ml-auto text-[9px] bg-ide-panel group-hover:bg-ide-border px-1.5 py-0.5 rounded-full text-ide-text-dim font-mono">
             {file.matches.length}
          </span>
       </div>

       {expanded && (
          <div className="flex flex-col mt-0.5">
             {file.matches.map((m, i) => {
                 const normalizedContent = m.line_content;
                 const idx = normalizedContent.toLowerCase().indexOf(query.toLowerCase());
                 
                 let prefix = normalizedContent;
                 let matchStr = "";
                 let suffix = "";

                 if (idx !== -1) {
                     prefix = normalizedContent.substring(0, idx);
                     matchStr = normalizedContent.substring(idx, idx + query.length);
                     suffix = normalizedContent.substring(idx + query.length);
                 }

                 return (
                    <div 
                       key={i} 
                       className="flex items-start gap-3 py-1 pl-8 pr-2 hover:bg-primary/10 cursor-pointer text-[11px] group transition-colors"
                       onClick={() => onMatchClick(m.line_number)}
                    >
                       <span className="text-[10px] text-ide-text-dim opacity-50 w-6 text-right shrink-0 group-hover:text-primary group-hover:opacity-100 font-mono mt-[1px]">{m.line_number}</span>
                       <div className="truncate font-mono opacity-80 group-hover:opacity-100 whitespace-pre">
                          {idx !== -1 ? (
                             <>
                               {prefix}<span className="bg-primary/20 text-primary rounded-[2px]">{matchStr}</span>{suffix}
                             </>
                          ) : (
                             normalizedContent
                          )}
                       </div>
                    </div>
                 );
             })}
          </div>
       )}
    </div>
  );
}
