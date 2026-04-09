import { useEffect, useState } from "react";
import { ChevronRight, ChevronDown, FileCode, File as FileIcon, Folder, FolderOpen } from "lucide-react";
import { useWorkspaceStore, FileEntry } from "../store/useWorkspaceStore";
import { invoke } from "../utils/tauri";

export function FileNavigator() {
  const { fileTree, setFileTree, openFile, activeFile } = useWorkspaceStore();
  const [gitStatus, setGitStatus] = useState<Record<string, string>>({});

  useEffect(() => {
    async function fetchData() {
      try {
        const tree = await invoke<FileEntry[]>("get_file_tree", { rootPath: "." });
        setFileTree(tree);
        
        const git = await invoke<Record<string, string>>("get_git_status");
        setGitStatus(git);
      } catch (err) {
        console.error("Failed to fetch navigator data", err);
      }
    }
    fetchData();
    // Refresh git status every 10 seconds
    const interval = setInterval(fetchData, 10000);
    return () => clearInterval(interval);
  }, [setFileTree]);

  return (
    <div className="py-2 select-none overflow-x-hidden">
      {fileTree.map((entry) => (
        <FileNode 
          key={entry.path} 
          entry={entry} 
          depth={0} 
          activeFile={activeFile} 
          onOpen={openFile} 
          gitStatusMap={gitStatus}
        />
      ))}
    </div>
  );
}

function FileNode({ 
  entry, 
  depth, 
  activeFile, 
  onOpen,
  gitStatusMap
}: { 
  entry: FileEntry, 
  depth: number, 
  activeFile: FileEntry | null, 
  onOpen: (file: FileEntry, isFixed?: boolean) => void,
  gitStatusMap: Record<string, string>
}) {
  const [isOpen, setIsOpen] = useState(false);
  const isSelected = activeFile?.path === entry.path;
  const currentStatus = gitStatusMap[entry.path];

  const handleClick = (e: React.MouseEvent) => {
    e.stopPropagation();
    if (entry.is_directory) {
      setIsOpen(!isOpen);
    } else {
      onOpen(entry, false); // Single click = Preview
    }
  };

  const handleDoubleClick = (e: React.MouseEvent) => {
    e.stopPropagation();
    if (!entry.is_directory) {
      onOpen(entry, true); // Double click = Fixed
    }
  };

  // Status Colors: M (Modified) -> Blue, ?? or A (Untracked/Added) -> Green
  const getStatusColor = () => {
    if (!currentStatus) return '';
    if (currentStatus === 'M') return 'text-blue-500';
    if (currentStatus === '??' || currentStatus === 'A') return 'text-green-500';
    return '';
  };

  return (
    <div className="flex flex-col">
      <div 
        onClick={handleClick}
        onDoubleClick={handleDoubleClick}
        className={`group flex items-center h-6 px-2 cursor-pointer transition-colors hover:bg-black/5 ${isSelected ? 'bg-primary/10 text-primary' : 'text-zinc-600'}`}
        style={{ paddingLeft: `${(depth * 12) + 8}px` }}
      >
        <div className="w-4 h-4 flex items-center justify-center mr-1">
          {entry.is_directory && (
            isOpen ? <ChevronDown size={14} /> : <ChevronRight size={14} className="opacity-40 group-hover:opacity-100" />
          )}
        </div>
        
        <div className="mr-2 text-primary/70">
          {entry.is_directory ? (
            isOpen ? <FolderOpen size={14} /> : <Folder size={14} />
          ) : (
            entry.name.endsWith('.zt') ? <FileCode size={14} className="text-primary" /> : <FileIcon size={14} />
          )}
        </div>

        <span className={`text-[11px] truncate ${isSelected ? 'font-medium' : ''} ${getStatusColor()}`}>
          {entry.name}
        </span>
      </div>

      {entry.is_directory && isOpen && entry.children && (
        <div className="flex flex-col">
          {entry.children.map((child) => (
            <FileNode 
              key={child.path} 
              entry={child} 
              depth={depth + 1} 
              activeFile={activeFile} 
              onOpen={onOpen} 
              gitStatusMap={gitStatusMap} 
            />
          ))}
        </div>
      )}
    </div>
  );
}
