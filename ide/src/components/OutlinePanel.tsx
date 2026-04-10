import { useEffect, useState } from "react";
import { invoke } from "@tauri-apps/api/core";
import { 
  Code2, 
  Box, 
  Variable, 
  Hash, 
  Layers, 
  Search,
  MousePointer2 
} from "lucide-react";
import { useWorkspaceStore, Symbol } from "../store/useWorkspaceStore";

export function OutlinePanel() {
  const { activeFile, activeFileSymbols, setActiveFileSymbols } = useWorkspaceStore();
  const [filter, setFilter] = useState("");

  // Update symbols whenever active file changes or user is typing (debounced)
  useEffect(() => {
    if (!activeFile) {
      setActiveFileSymbols([]);
      return;
    }

    const updateSymbols = async () => {
      try {
        // We'll read the file content from the filesystem or editor state if available
        const content = await invoke<string>("read_file", { path: activeFile.path });
        const symbols = await invoke<Symbol[]>("get_file_symbols", { content });
        setActiveFileSymbols(symbols);
      } catch (err) {
        console.error("Failed to fetch symbols", err);
      }
    };

    updateSymbols();
  }, [activeFile, setActiveFileSymbols]);

  const filteredSymbols = activeFileSymbols.filter(s => 
    s.name.toLowerCase().includes(filter.toLowerCase())
  );

  if (!activeFile) {
    return (
      <div className="flex-1 flex flex-col items-center justify-center p-8 text-center text-ide-text-dim opacity-50 select-none">
        <MousePointer2 size={32} className="mb-4 opacity-20" />
        <p className="text-[11px]">Nenhum arquivo aberto para mapear símbolos.</p>
      </div>
    );
  }

  return (
    <div className="flex flex-col h-full bg-ide-bg text-ide-text select-none">
      {/* Mini Search for Symbols */}
      <div className="p-3 border-b border-ide-border bg-ide-panel/20">
        <div className="relative group">
          <input 
            type="text" 
            value={filter}
            onChange={(e) => setFilter(e.target.value)}
            placeholder="Filtrar símbolos..."
            className="w-full bg-ide-bg border border-ide-border rounded-md px-7 py-1.5 text-[11px] focus:outline-none focus:border-primary transition-all font-mono"
          />
          <Search size={12} className="absolute left-2.5 top-2.5 text-ide-text-dim group-focus-within:text-primary transition-colors" />
        </div>
      </div>

      <div className="flex-1 overflow-y-auto py-2 custom-scrollbar">
        {filteredSymbols.length === 0 ? (
          <div className="py-8 text-center text-[11px] text-ide-text-dim italic">
            Nenhum símbolo encontrado
          </div>
        ) : (
          <div className="flex flex-col gap-0.5 px-2">
            {filteredSymbols.map((symbol, idx) => (
              <SymbolItem key={`${symbol.name}-${idx}`} symbol={symbol} />
            ))}
          </div>
        )}
      </div>
    </div>
  );
}

function SymbolItem({ symbol }: { symbol: Symbol }) {
  const jumpToLine = () => {
    window.dispatchEvent(new CustomEvent("zenith://editor-jump", { 
        detail: { line: symbol.line, col: symbol.col } 
    }));
  };

  const getIcon = () => {
    switch (symbol.kind) {
      case 'Function': return <Code2 size={13} className="text-primary" />;
      case 'Struct': return <Box size={13} className="text-amber-500" />;
      case 'Variable': return <Variable size={13} className="text-blue-500" />;
      case 'Constant': return <Hash size={13} className="text-purple-500" />;
      case 'Interface': return <Layers size={13} className="text-emerald-500" />;
      default: return <Hash size={13} />;
    }
  };

  const getKindLabel = () => {
    switch (symbol.kind) {
      case 'Function': return 'func';
      case 'Struct': return 'struct';
      case 'Variable': return 'var';
      case 'Constant': return 'const';
      case 'Interface': return 'interface';
      default: return '';
    }
  };

  return (
    <div 
      className="group flex items-center gap-2.5 py-1 px-2 rounded-md hover:bg-primary/10 cursor-pointer transition-all border border-transparent hover:border-primary/20"
      onClick={jumpToLine}
    >
      <div className="shrink-0 opacity-80 group-hover:opacity-100 transition-opacity">
        {getIcon()}
      </div>
      
      <div className="flex items-baseline gap-2 truncate">
        <span className="text-[12px] font-mono text-ide-text truncate">{symbol.name}</span>
        <span className="text-[9px] font-bold uppercase tracking-tighter text-ide-text-dim opacity-40 group-hover:opacity-60 transition-opacity">
          {getKindLabel()}
        </span>
      </div>

      <span className="ml-auto text-[9px] font-mono text-ide-text-dim opacity-0 group-hover:opacity-100 transition-opacity pr-1">
        L{symbol.line}
      </span>
    </div>
  );
}
