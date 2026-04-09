import { useState, useEffect } from 'react';
import { ChevronRight, ChevronDown, FileCode, File as FileIcon } from 'lucide-react';
import { useProjectStore, FileEntry } from '../store/useProjectStore';
import { safeInvoke as invoke } from '../utils/tauri';
import { clsx, type ClassValue } from 'clsx';
import { twMerge } from 'tailwind-merge';
import { ContextMenuWrapper } from './ContextMenuWrapper';

function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs));
}

export function FileTree() {
  const { fileTree, setFileTree, openFile, activeFilePath } = useProjectStore();

  useEffect(() => {
    // Carregar árvore de arquivos ao iniciar
    const loadFiles = async () => {
      try {
        // Tentamos carregar da raiz do projeto
        const tree = await invoke<FileEntry[]>('get_file_tree', { rootPath: '..' });
        setFileTree(tree);
      } catch (err) {
        console.error("Failed to load file tree:", err);
        // Fallback: mostrar uma estrutura básica se o Tauri falhar
        setFileTree([
          { name: 'src', path: 'src', is_directory: true, children: [
            { name: 'main.zt', path: 'src/main.zt', is_directory: false }
          ]},
          { name: 'zenith.project', path: 'zenith.project', is_directory: false }
        ]);
      }
    };
    loadFiles();
  }, [setFileTree]);

  return (
    <div className="py-2 h-full overflow-y-auto select-none">
      <div className="px-4 py-2 text-[10px] font-bold text-white/30 uppercase tracking-widest">
        Project Explorer
      </div>
      <div className="px-2">
        {fileTree.map((entry) => (
          <FileTreeNode 
            key={entry.path} 
            entry={entry} 
            depth={0} 
            onFileClick={openFile}
            activeFilePath={activeFilePath}
          />
        ))}
      </div>
    </div>
  );
}

interface FileTreeNodeProps {
  entry: FileEntry;
  depth: number;
  onFileClick: (path: string) => void;
  activeFilePath: string | null;
}

function FileTreeNode({ entry, depth, onFileClick, activeFilePath }: FileTreeNodeProps) {
  const [isOpen, setIsOpen] = useState(false);
  const isActive = activeFilePath === entry.path;

  const handleClick = () => {
    if (entry.is_directory) {
      setIsOpen(!isOpen);
    } else {
      onFileClick(entry.path);
    }
  };

  const getIcon = () => {
    if (entry.is_directory) {
      return isOpen ? <ChevronDown size={14} /> : <ChevronRight size={14} />;
    }
    if (entry.name.endsWith('.zt')) return <FileCode size={14} className="text-xcode-accent" />;
    return <FileIcon size={14} className="text-white/40" />;
  };

  return (
    <div className="flex flex-col">
      <ContextMenuWrapper onAction={(action) => console.log(`Action: ${action} on ${entry.path}`)}>
        <div 
          onClick={handleClick}
          className={cn(
            "flex items-center gap-2 px-2 py-1 rounded-md cursor-pointer transition-colors text-sm",
            isActive ? "bg-xcode-accent/20 border border-xcode-accent/30 text-white" : "text-white/60 hover:bg-white/5"
          )}
          style={{ paddingLeft: `${(depth + 1) * 8}px` }}
        >
          <span className="opacity-50">{getIcon()}</span>
          <span className="truncate">{entry.name}</span>
        </div>
      </ContextMenuWrapper>
      
      {entry.is_directory && isOpen && entry.children && (
        <div className="flex flex-col">
          {entry.children.map((child) => (
            <FileTreeNode 
              key={child.path} 
              entry={child} 
              depth={depth + 1} 
              onFileClick={onFileClick}
              activeFilePath={activeFilePath}
            />
          ))}
        </div>
      )}
    </div>
  );
}
