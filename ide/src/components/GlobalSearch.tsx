import { useState, useEffect } from "react";
import { invoke } from "@tauri-apps/api/core";
import { 
  Search, 
  Loader2, 
  ChevronRight, 
  ChevronDown, 
  FileText, 
  CaseSensitive, 
  WholeWord, 
  Regex, 
  Replace, 
  ReplaceAll, 
  X,
  FolderSearch
} from "lucide-react";
import { useWorkspaceStore } from "../store/useWorkspaceStore";
import { motion, AnimatePresence } from "framer-motion";
import { clsx } from 'clsx';

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
  const [replaceValue, setReplaceValue] = useState("");
  const [showReplace, setShowReplace] = useState(false);
  const [results, setResults] = useState<FileResult[]>([]);
  const [isSearching, setIsSearching] = useState(false);
  const [error, setError] = useState<string | null>(null);
  
  // Search Options
  const [matchCase, setMatchCase] = useState(false);
  const [wholeWord, setWholeWord] = useState(false);
  const [isRegex, setIsRegex] = useState(false);

  const { openFile, searchFilterPath, setSearchFilterPath } = useWorkspaceStore();

  const handleSearch = async (currentQuery = query) => {
    if (!currentQuery.trim()) {
      setResults([]);
      return;
    }

    setIsSearching(true);
    setError(null);
    try {
      const res: FileResult[] = await invoke("search_in_files", {
        query: currentQuery,
        isRegex,
        matchCase,
        wholeWord,
        pathFilter: searchFilterPath
      });
      setResults(res);
    } catch (err: any) {
      setError(err.toString());
    } finally {
      setIsSearching(false);
    }
  };

  const handleReplaceAll = async () => {
    if (!query.trim() || results.length === 0) return;
    
    const targetFiles = results.map(r => r.file_path);
    setIsSearching(true);
    try {
      const count = await invoke<number>("replace_in_files", {
        query,
        replacement: replaceValue,
        isRegex,
        matchCase,
        wholeWord,
        targetFiles
      });
      console.log(`Replaced in ${count} files`);
      await handleSearch(); // Refresh results
    } catch (err: any) {
      setError(err.toString());
    } finally {
      setIsSearching(false);
    }
  };

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter') handleSearch();
  };

  // Trigger search on options change if query exists
  useEffect(() => {
    if (query.trim()) handleSearch();
  }, [matchCase, wholeWord, isRegex, searchFilterPath]);

  return (
    <div className="flex flex-col h-full bg-ide-panel/40 backdrop-blur-md select-none overflow-hidden">
      {/* Search Header */}
      <div className="px-4 pt-4 pb-2 border-b border-white/5 bg-white/[0.02]">
        <div className="flex items-center gap-2.5 mb-3">
          <div className="w-6 h-6 rounded-lg bg-primary/20 flex items-center justify-center">
            <Search size={14} className="text-primary" />
          </div>
          <h3 className="text-[10px] font-black uppercase tracking-[0.25em] text-ide-text opacity-90">Search</h3>
        </div>

        <div className="space-y-2">
          {/* Search Input Area */}
          <div className="relative group flex items-center gap-1">
            <div 
              className="p-1 rounded-md hover:bg-white/5 text-ide-text-dim transition-colors cursor-pointer"
              onClick={() => setShowReplace(!showReplace)}
            >
              {showReplace ? <ChevronDown size={14} /> : <ChevronRight size={14} />}
            </div>
            <div className="relative flex-1 group">
              <input 
                autoFocus
                type="text" 
                value={query}
                onChange={(e) => setQuery(e.target.value)}
                onKeyDown={handleKeyDown}
                placeholder="Search..."
                className="w-full bg-black/20 border border-white/5 rounded-xl pl-8 pr-20 py-2 text-[11px] text-ide-text placeholder:text-ide-text-dim/30 focus:outline-none focus:ring-1 focus:ring-primary/40 transition-all font-mono"
              />
              <Search size={12} className="absolute left-3 top-1/2 -translate-y-1/2 text-ide-text-dim/40 group-focus-within:text-primary transition-colors" />
              
              <div className="absolute right-2 top-1/2 -translate-y-1/2 flex items-center gap-0.5">
                <OptionToggle active={matchCase} onClick={() => setMatchCase(!matchCase)} icon={<CaseSensitive size={14} />} title="Match Case" />
                <OptionToggle active={wholeWord} onClick={() => setWholeWord(!wholeWord)} icon={<WholeWord size={14} />} title="Match Whole Word" />
                <OptionToggle active={isRegex} onClick={() => setIsRegex(!isRegex)} icon={<Regex size={14} />} title="Use Regular Expression" />
              </div>
            </div>
          </div>

          {/* Replace Area */}
          <AnimatePresence>
            {showReplace && (
              <motion.div 
                initial={{ height: 0, opacity: 0 }}
                animate={{ height: "auto", opacity: 1 }}
                exit={{ height: 0, opacity: 0 }}
                className="overflow-hidden flex items-center gap-1 pl-5"
              >
                <div className="relative flex-1 group">
                  <input 
                    type="text" 
                    value={replaceValue}
                    onChange={(e) => setReplaceValue(e.target.value)}
                    placeholder="Replace with..."
                    className="w-full bg-black/20 border border-white/5 rounded-xl pl-8 pr-12 py-1.5 text-[11px] text-ide-text placeholder:text-ide-text-dim/30 focus:outline-none focus:ring-1 focus:ring-rose-500/40 transition-all font-mono"
                  />
                  <Replace size={12} className="absolute left-3 top-1/2 -translate-y-1/2 text-ide-text-dim/40 group-focus-within:text-rose-400 transition-colors" />
                  
                  <button 
                    onClick={handleReplaceAll}
                    disabled={!query.trim() || results.length === 0}
                    className="absolute right-2 top-1/2 -translate-y-1/2 p-1 rounded-md hover:bg-rose-500/20 text-ide-text-dim hover:text-rose-400 transition-all disabled:opacity-20"
                    title="Replace All"
                  >
                    <ReplaceAll size={14} />
                  </button>
                </div>
              </motion.div>
            )}
          </AnimatePresence>

          {/* Filter Path Area */}
          <AnimatePresence>
            {searchFilterPath && (
              <motion.div 
                initial={{ opacity: 0, scale: 0.95 }}
                animate={{ opacity: 1, scale: 1 }}
                exit={{ opacity: 0, scale: 0.95 }}
                className="flex items-center gap-2 px-3 py-1 bg-primary/10 border border-primary/20 rounded-lg text-[10px] text-primary font-bold overflow-hidden"
              >
                <FolderSearch size={12} className="shrink-0" />
                <span className="truncate flex-1">in {searchFilterPath}</span>
                <button 
                  onClick={() => setSearchFilterPath(null)}
                  className="p-0.5 hover:bg-primary/20 rounded transition-colors"
                >
                  <X size={12} />
                </button>
              </motion.div>
            )}
          </AnimatePresence>
        </div>
      </div>

      {/* Results List */}
      <div className="flex-1 overflow-y-auto no-scrollbar py-2">
        {isSearching && results.length === 0 && (
          <div className="py-20 flex flex-col items-center justify-center text-ide-text-dim opacity-40 gap-3">
             <Loader2 size={24} className="animate-spin text-primary" />
             <span className="text-[10px] font-black uppercase tracking-widest">Searching...</span>
          </div>
        )}

        {error && (
           <div className="mx-4 my-2 p-3 bg-rose-500/10 border border-rose-500/20 rounded-xl text-rose-400 text-[10px] font-medium leading-relaxed">
              <div className="font-black uppercase tracking-widest mb-1 opacity-60">Search Error</div>
              {error}
           </div>
        )}

        {!isSearching && results.length === 0 && query.trim() !== "" && !error && (
            <div className="py-20 flex flex-col items-center justify-center text-center px-8 opacity-40">
              <Search size={32} className="mb-4 text-ide-text-dim" />
              <p className="text-[11px] font-bold text-ide-text mb-1">No results found</p>
              <p className="text-[9px] text-ide-text-dim leading-relaxed font-medium">Try different keywords or check your filters.</p>
            </div>
        )}

        <div className="px-1 space-y-1">
          <AnimatePresence mode="popLayout">
            {results.map((file) => (
              <FileResultItem 
                 key={file.file_path} 
                 file={file} 
                 onMatchClick={(_line) => {
                    const filename = file.file_path.split('/').pop() || file.file_path;
                    openFile({
                        name: filename,
                        path: file.file_path,
                        is_directory: false
                    }, false); 
                    
                    setTimeout(() => {
                       window.dispatchEvent(new CustomEvent('zenith://editor-jump', {
                           detail: { line: _line, col: 1 }
                       }));
                    }, 100);
                 }} 
                 query={query}
                 isRegex={isRegex}
                 matchCase={matchCase}
              />
            ))}
          </AnimatePresence>
        </div>
      </div>

      {/* Footer Info */}
      {!isSearching && results.length > 0 && (
        <div className="px-4 py-2 bg-black/20 border-t border-white/5 flex items-center justify-between">
           <div className="flex items-center gap-1.5 text-[9px] font-bold text-primary opacity-80 uppercase tracking-widest">
              Found {results.reduce((acc, r) => acc + r.matches.length, 0)} results 
              <span className="opacity-30">/</span>
              {results.length} files
           </div>
        </div>
      )}
    </div>
  );
}

function OptionToggle({ active, onClick, icon, title }: { active: boolean, onClick: () => void, icon: React.ReactNode, title: string }) {
  return (
    <button 
      onClick={onClick}
      title={title}
      className={clsx(
        "p-1 rounded transition-all",
        active ? "bg-primary text-white shadow-sm" : "hover:bg-white/5 text-ide-text-dim opacity-40 hover:opacity-100"
      )}
    >
      {icon}
    </button>
  );
}

function FileResultItem({ file, onMatchClick, query, isRegex, matchCase }: { 
  file: FileResult, 
  onMatchClick: (line: number) => void, 
  query: string,
  isRegex: boolean,
  matchCase: boolean
}) {
  const [expanded, setExpanded] = useState(true);

  const filename = file.file_path.split('/').pop() || file.file_path;
  const dirpath = file.file_path.substring(0, file.file_path.length - filename.length);

  return (
    <motion.div 
      layout
      initial={{ opacity: 0, y: 5 }}
      animate={{ opacity: 1, y: 0 }}
      exit={{ opacity: 0, scale: 0.95 }}
      className="flex flex-col group/file"
    >
       <div 
          className="flex items-center gap-2 py-1.5 px-3 hover:bg-white/[0.04] cursor-pointer rounded-lg group transition-all"
          onClick={() => setExpanded(!expanded)}
       >
          <div className="w-4 h-4 flex items-center justify-center text-ide-text-dim/40 group-hover:text-primary transition-colors">
            {expanded ? <ChevronDown size={14} /> : <ChevronRight size={14} />}
          </div>
          <FileText size={12} className="text-primary/70 opacity-70 group-hover:opacity-100 transition-all" />
          <div className="flex items-baseline gap-1.5 overflow-hidden">
             <span className="text-[11px] font-bold text-ide-text truncate opacity-90">{filename}</span>
             <span className="text-[9px] text-ide-text-dim opacity-30 truncate group-hover:opacity-50 font-mono">{dirpath}</span>
          </div>
          <span className="ml-auto text-[9px] bg-white/5 px-2 py-0.5 rounded-full text-ide-text-dim font-black font-mono group-hover:bg-primary/10 group-hover:text-primary transition-all">
             {file.matches.length}
          </span>
       </div>

       <AnimatePresence>
          {expanded && (
            <motion.div 
              initial={{ height: 0, opacity: 0 }}
              animate={{ height: "auto", opacity: 1 }}
              exit={{ height: 0, opacity: 0 }}
              className="flex flex-col mt-0.5"
            >
               {file.matches.map((m, i) => (
                  <MatchLine 
                    key={i} 
                    match={m} 
                    query={query} 
                    isRegex={isRegex} 
                    matchCase={matchCase}
                    onClick={() => onMatchClick(m.line_number)} 
                  />
               ))}
            </motion.div>
          )}
       </AnimatePresence>
    </motion.div>
  );
}

function MatchLine({ match, query, isRegex, matchCase, onClick }: { 
  match: SearchMatch, 
  query: string, 
  isRegex: boolean, 
  matchCase: boolean,
  onClick: () => void 
}) {
  const highlightMatch = () => {
    const text = match.line_content;
    if (!query) return text;

    try {
      const pattern = isRegex ? query : regexEscape(query);
      const re = new RegExp(`(${pattern})`, matchCase ? "g" : "gi");
      const parts = text.split(re);
      
      return parts.map((part, i) => 
        re.test(part) ? (
          <span key={i} className="bg-primary/20 text-primary border-b border-primary/40 rounded-[1px] font-bold">{part}</span>
        ) : part
      );
    } catch (e) {
      return text;
    }
  };

  return (
    <div 
      className="flex items-start gap-4 py-1.5 pl-10 pr-3 hover:bg-primary/10 cursor-pointer text-[11px] group transition-all relative overflow-hidden"
      onClick={onClick}
    >
      <span className="text-[9px] text-ide-text-dim opacity-20 w-8 text-right shrink-0 group-hover:text-primary group-hover:opacity-60 font-mono transition-all font-black">{match.line_number}</span>
      <div className="truncate font-mono text-[10px] opacity-70 group-hover:opacity-100 transition-opacity whitespace-pre">
        {highlightMatch()}
      </div>
    </div>
  );
}

function regexEscape(string: string) {
  return string.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

