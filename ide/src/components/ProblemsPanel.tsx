import { useWorkspaceStore, FileEntry } from "../store/useWorkspaceStore";
import { AlertCircle, AlertTriangle, Info, ChevronRight } from "lucide-react";

export function ProblemsPanel() {
  const { diagnosticsMap, openFile } = useWorkspaceStore();

  const allDiagnostics = Object.entries(diagnosticsMap).flatMap(([path, diagnostics]) => 
    diagnostics.map(d => ({ ...d, path }))
  );

  if (allDiagnostics.length === 0) {
    return (
      <div className="flex flex-col items-center justify-center h-full text-zinc-300">
        <Info size={32} className="mb-2 opacity-20" />
        <p className="text-xs font-medium">No problems detected</p>
      </div>
    );
  }

  const handleDiagnosticClick = (path: string, line: number) => {
    console.log(`[Zenith IDE] Jumping to ${path}:${line}`);
    const fileStub: FileEntry = {
        name: path.split(/[\\/]/).pop() || "file",
        path: path,
        is_directory: false
    };
    openFile(fileStub, true);
  };

  return (
    <div className="flex flex-col h-full bg-white select-none overflow-y-auto">
      {allDiagnostics.map((d, i) => (
        <div 
          key={i}
          onClick={() => handleDiagnosticClick(d.path, d.line)}
          className="group flex items-start py-2 px-4 border-b border-zinc-50 hover:bg-zinc-50 cursor-pointer transition-colors"
        >
          <div className="mt-0.5 mr-3">
            {d.severity === 'error' && <AlertCircle size={14} className="text-red-500" />}
            {d.severity === 'warning' && <AlertTriangle size={14} className="text-amber-500" />}
            {d.severity === 'hint' && <Info size={14} className="text-blue-500" />}
          </div>
          
          <div className="flex-1 flex flex-col">
            <div className="flex items-center text-[11px] font-medium text-zinc-700">
              <span className="truncate max-w-[400px]">{d.message}</span>
              <span className="mx-2 text-zinc-300">·</span>
              <span className="text-zinc-400 font-mono text-[10px]">{d.code}</span>
            </div>
            
            <div className="flex items-center mt-0.5 text-[10px] text-zinc-400">
              <span className="truncate">{d.path.split(/[\\/]/).pop()}</span>
              <ChevronRight size={10} className="mx-1 opacity-50" />
              <span>Line {d.line}, Col {d.col}</span>
            </div>
          </div>
        </div>
      ))}
    </div>
  );
}
