import { useState, useMemo } from 'react';
import { useWorkspaceStore } from '../store/useWorkspaceStore';
import { 
  GitCommit, 
  RefreshCw, 
  Plus, 
  Minus, 
  CheckCircle2,
  Trash2,
  Undo2,
  Check,
  ChevronRight,
  GitBranch
} from 'lucide-react';
import { motion, AnimatePresence } from 'framer-motion';
import { clsx } from 'clsx';

export function SourceControlPanel() {
  const { 
    gitStatus, 
    refreshGitStatus, 
    stageFile, 
    unstageFile, 
    stageAll,
    unstageAll,
    discardChanges,
    commitChanges,
    openDiff,
    gitCurrentBranch
  } = useWorkspaceStore();
  
  const [commitMessage, setCommitMessage] = useState('');
  const [isRefreshing, setIsRefreshing] = useState(false);

  const handleRefresh = async () => {
    setIsRefreshing(true);
    await refreshGitStatus();
    setTimeout(() => setIsRefreshing(false), 600);
  };

  const handleCommit = async () => {
    if (!commitMessage.trim()) return;
    await commitChanges(commitMessage);
    setCommitMessage('');
  };

  const { staged, unstaged } = useMemo(() => {
    const s: [string, string][] = [];
    const u: [string, string][] = [];

    Object.entries(gitStatus).forEach(([path, status]) => {
      const x = status[0];
      const y = status[1];

      if (x !== ' ' && x !== '?') {
        s.push([path, x]);
      }
      if (y !== ' ' || x === '?') {
        u.push([path, y === ' ' ? x : y]);
      }
    });

    return { staged: s, unstaged: u };
  }, [gitStatus]);

  return (
    <div className="flex flex-col h-full bg-ide-panel/40 backdrop-blur-md select-none overflow-hidden">
      {/* Header & Commit Area */}
      <div className="px-4 pt-4 pb-3 border-b border-white/5 bg-white/[0.02]">
        <div className="flex items-center justify-between mb-4">
          <div className="flex items-center gap-2.5">
            <div className="w-6 h-6 rounded-lg bg-primary/20 flex items-center justify-center">
              <GitCommit size={14} className="text-primary" />
            </div>
            <h3 className="text-[10px] font-black uppercase tracking-[0.25em] text-ide-text opacity-90">Source Control</h3>
          </div>
          <button 
            onClick={handleRefresh}
            className={clsx(
              "p-1.5 rounded-lg hover:bg-white/5 text-ide-text-dim transition-all",
              isRefreshing && "animate-spin text-primary"
            )}
          >
            <RefreshCw size={14} />
          </button>
        </div>

        <div className="space-y-2.5">
          <div className="group relative">
            <textarea
              value={commitMessage}
              onChange={(e) => setCommitMessage(e.target.value)}
              placeholder="What did you change?"
              className="w-full h-20 bg-black/20 border border-white/5 rounded-xl p-3 text-[11px] text-ide-text placeholder:text-ide-text-dim/30 focus:outline-none focus:ring-1 focus:ring-primary/40 transition-all resize-none font-medium leading-relaxed"
              onKeyDown={(e) => (e.metaKey || e.ctrlKey) && e.key === 'Enter' && handleCommit()}
            />
            <div className="absolute bottom-2 right-2 opacity-0 group-hover:opacity-40 transition-opacity pointer-events-none text-[9px] font-bold">
               {navigator.platform.includes('Mac') ? '⌘' : 'Ctrl'} + Enter
            </div>
          </div>
          
          <button 
            onClick={handleCommit}
            disabled={!commitMessage.trim() || staged.length === 0}
            className="w-full py-2 bg-primary text-white rounded-xl text-[10px] font-black uppercase tracking-[0.15em] shadow-lg shadow-primary/20 hover:shadow-primary/40 hover:-translate-y-0.5 active:translate-y-0 disabled:opacity-20 disabled:translate-y-0 disabled:shadow-none transition-all flex items-center justify-center gap-2 group"
          >
            <Check size={14} className="group-hover:scale-110 transition-transform" />
            Commit to {gitCurrentBranch || 'main'}
          </button>
        </div>
      </div>

      <div className="flex-1 overflow-y-auto no-scrollbar pt-2 px-1">
        <AnimatePresence mode="popLayout">
          {/* Staged Changes */}
          {staged.length > 0 && (
            <motion.div 
              layout
              initial={{ opacity: 0, y: 10 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, scale: 0.95 }}
              className="mb-4"
            >
              <div className="px-3 py-1.5 flex items-center justify-between group cursor-default">
                <div className="flex items-center gap-2">
                   <span className="text-[9px] font-black uppercase tracking-widest text-primary/80">Staged</span>
                   <div className="px-1.5 py-0.5 rounded-full bg-primary/10 text-primary text-[9px] font-bold font-mono">
                      {staged.length}
                   </div>
                </div>
                <button 
                  onClick={unstageAll}
                  title="Unstage All Changes"
                  className="opacity-0 group-hover:opacity-100 p-1 hover:bg-white/5 rounded-md text-ide-text-dim hover:text-red-400 transition-all"
                >
                  <Minus size={12} />
                </button>
              </div>
              <div className="space-y-0.5">
                {staged.map(([path, status]) => (
                  <ChangeItem 
                    key={`staged-${path}`} 
                    path={path} 
                    status={status} 
                    isStaged={true}
                    onAction={() => unstageFile(path)}
                    onClick={() => openDiff(path)}
                    onDiscard={() => discardChanges(path)}
                  />
                ))}
              </div>
            </motion.div>
          )}

          {/* Unstaged Changes */}
          <motion.div layout className="mb-6">
            <div className="px-3 py-1.5 flex items-center justify-between group cursor-default">
                <div className="flex items-center gap-2">
                   <span className="text-[9px] font-black uppercase tracking-widest text-ide-text-dim opacity-60">Changes</span>
                   <div className="px-1.5 py-0.5 rounded-full bg-white/5 text-ide-text-dim text-[9px] font-bold font-mono">
                      {unstaged.length}
                   </div>
                </div>
                {unstaged.length > 0 && (
                  <button 
                    onClick={stageAll}
                    title="Stage All Changes"
                    className="opacity-0 group-hover:opacity-100 p-1 hover:bg-white/5 rounded-md text-ide-text-dim hover:text-primary transition-all"
                  >
                    <Plus size={12} />
                  </button>
                )}
            </div>

            {staged.length === 0 && unstaged.length === 0 ? (
              <motion.div 
                initial={{ opacity: 0 }}
                animate={{ opacity: 1 }}
                className="py-12 flex flex-col items-center justify-center text-center px-6"
              >
                <div className="w-12 h-12 rounded-full bg-emerald-500/10 flex items-center justify-center mb-4 border border-emerald-500/20">
                  <CheckCircle2 size={24} className="text-emerald-500 opacity-80" />
                </div>
                <p className="text-[11px] font-bold text-ide-text opacity-80 mb-1">Your workspace is clean</p>
                <p className="text-[9px] text-ide-text-dim opacity-40 leading-relaxed font-medium">All changes have been committed or discarded.</p>
              </motion.div>
            ) : (
              <div className="space-y-0.5">
                {unstaged.map(([path, status]) => (
                  <ChangeItem 
                    key={`unstaged-${path}`} 
                    path={path} 
                    status={status} 
                    isStaged={false}
                    onAction={() => stageFile(path)}
                    onClick={() => openDiff(path)}
                    onDiscard={() => discardChanges(path)}
                  />
                ))}
              </div>
            )}
          </motion.div>
        </AnimatePresence>
      </div>

      {/* Footer info Bar */}
      <div className="px-4 py-2 border-t border-white/5 bg-black/20 flex items-center justify-between mt-auto">
        <div className="flex items-center gap-2 text-[9px] font-black uppercase tracking-tighter text-ide-text-dim opacity-50">
          <GitBranch size={10} />
          <span>{gitCurrentBranch || 'main'}</span>
        </div>
        <div className="flex items-center gap-1.5 text-[8px] font-bold text-primary opacity-80 uppercase tracking-widest">
           Synced
        </div>
      </div>
    </div>
  );
}

function ChangeItem({ 
  path, 
  status, 
  isStaged, 
  onAction, 
  onClick,
  onDiscard
}: { 
  path: string; 
  status: string; 
  isStaged: boolean; 
  onAction: () => void;
  onClick: () => void;
  onDiscard: () => void;
}) {
  const [showConfirmDiscard, setShowConfirmDiscard] = useState(false);
  const fileName = path.split(/[\\/]/).pop() || path;
  
  const getStatusInfo = (s: string) => {
    const cleanStatus = s.trim() || 'M';
    switch(cleanStatus) {
      case 'M': return { icon: 'M', color: 'text-amber-400', bg: 'bg-amber-400/10', label: 'Modified' };
      case 'A': return { icon: 'A', color: 'text-emerald-400', bg: 'bg-emerald-400/10', label: 'Added' };
      case 'D': return { icon: 'D', color: 'text-rose-400', bg: 'bg-rose-400/10', label: 'Deleted' };
      case '?': return { icon: 'U', color: 'text-sky-400', bg: 'bg-sky-400/10', label: 'Untracked' };
      default: return { icon: 'M', color: 'text-amber-400', bg: 'bg-amber-400/10', label: 'Modified' };
    }
  };

  const info = getStatusInfo(status);

  return (
    <motion.div 
      layout
      initial={{ opacity: 0, x: -5 }}
      animate={{ opacity: 1, x: 0 }}
      exit={{ opacity: 0, x: 5 }}
      className="group relative flex items-center gap-3 px-3 py-1.5 hover:bg-white/[0.04] cursor-pointer transition-all border-l-2 border-transparent hover:border-primary/20"
    >
      <div className="flex-1 min-w-0 flex items-center gap-2" onClick={onClick}>
        <ChevronRight size={10} className="text-ide-text-dim opacity-0 group-hover:opacity-40 transition-opacity -ml-1" />
        <div className="flex flex-col min-w-0 flex-1">
          <span className="text-[11px] font-bold text-ide-text truncate opacity-90">{fileName}</span>
          <span className="text-[8px] font-mono text-ide-text-dim opacity-30 truncate group-hover:opacity-50 transition-opacity">
            {path.length > 40 ? '...' + path.slice(-40) : path}
          </span>
        </div>
      </div>
      
      <div className="flex items-center gap-1.5 pr-1">
        <AnimatePresence mode="wait">
          {!showConfirmDiscard ? (
            <motion.div 
               key="actions"
               initial={{ opacity: 0 }}
               animate={{ opacity: 1 }}
               exit={{ opacity: 0 }}
               className="flex items-center gap-1"
            >
               <button 
                onClick={(e) => { e.stopPropagation(); onAction(); }}
                title={isStaged ? "Unstage" : "Stage"}
                className="opacity-0 group-hover:opacity-100 p-1 hover:bg-white/10 rounded-md transition-all text-ide-text-dim hover:text-white"
              >
                {isStaged ? <Minus size={12} /> : <Plus size={12} />}
              </button>
              
              {!isStaged && (
                <button 
                  onClick={(e) => { e.stopPropagation(); setShowConfirmDiscard(true); }}
                  title="Discard Changes"
                  className="opacity-0 group-hover:opacity-100 p-1 hover:bg-rose-500/20 rounded-md transition-all text-ide-text-dim hover:text-rose-400"
                >
                  <Trash2 size={12} />
                </button>
              )}
            </motion.div>
          ) : (
            <motion.div 
               key="confirm"
               initial={{ scale: 0.8, opacity: 0 }}
               animate={{ scale: 1, opacity: 1 }}
               exit={{ scale: 0.8, opacity: 0 }}
               className="flex items-center gap-1 bg-rose-500/20 rounded-md px-1 py-0.5 border border-rose-500/30"
            >
              <button 
                onClick={(e) => { e.stopPropagation(); onDiscard(); setShowConfirmDiscard(false); }}
                className="p-1 hover:bg-rose-500/40 rounded text-rose-300 transition-colors"
                title="Confirm Discard"
              >
                <Check size={10} />
              </button>
              <button 
                onClick={(e) => { e.stopPropagation(); setShowConfirmDiscard(false); }}
                className="p-1 hover:bg-white/10 rounded text-ide-text-dim transition-colors"
                title="Cancel"
              >
                <Undo2 size={10} />
              </button>
            </motion.div>
          )}
        </AnimatePresence>

        <div 
          className={clsx(
            "shrink-0 flex items-center justify-center w-5 h-5 rounded font-black font-mono text-[9px] shadow-sm",
            info.bg, info.color
          )}
          title={info.label}
        >
          {info.icon}
        </div>
      </div>
    </motion.div>
  );
}
