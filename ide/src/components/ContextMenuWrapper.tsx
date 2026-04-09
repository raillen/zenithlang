import * as ContextMenu from '@radix-ui/react-context-menu';
import { FilePlus, FolderPlus, Trash2, Edit3, Share, Copy } from 'lucide-react';

interface ContextMenuWrapperProps {
  children: React.ReactNode;
  onAction?: (action: string) => void;
}

export function ContextMenuWrapper({ children, onAction }: ContextMenuWrapperProps) {
  return (
    <ContextMenu.Root>
      <ContextMenu.Trigger>
        {children}
      </ContextMenu.Trigger>
      
      <ContextMenu.Portal>
        <ContextMenu.Content 
          className="min-w-[200px] bg-[#1E1E1E]/95 backdrop-blur-xl border border-white/10 rounded-lg p-1 shadow-2xl z-[100] animate-in fade-in zoom-in-95 duration-100"
        >
          <ContextMenuItem 
            icon={<FilePlus size={14} />} 
            label="New File" 
            shortcut="⌘N" 
            onClick={() => onAction?.('new-file')} 
          />
          <ContextMenuItem 
            icon={<FolderPlus size={14} />} 
            label="New Folder" 
            onClick={() => onAction?.('new-folder')} 
          />
          
          <ContextMenu.Separator className="h-[1px] bg-white/5 my-1" />
          
          <ContextMenuItem 
            icon={<Copy size={14} />} 
            label="Duplicate" 
            shortcut="⌘D" 
            onClick={() => onAction?.('duplicate')} 
          />
          <ContextMenuItem 
            icon={<Edit3 size={14} />} 
            label="Rename..." 
            onClick={() => onAction?.('rename')} 
          />
          
          <ContextMenu.Separator className="h-[1px] bg-white/5 my-1" />
          
          <ContextMenuItem 
            icon={<Share size={14} />} 
            label="Share" 
            onClick={() => onAction?.('share')} 
          />
          
          <ContextMenu.Separator className="h-[1px] bg-white/5 my-1" />
          
          <ContextMenuItem 
            icon={<Trash2 size={14} />} 
            label="Delete" 
            shortcut="⌫" 
            danger 
            onClick={() => onAction?.('delete')} 
          />
        </ContextMenu.Content>
      </ContextMenu.Portal>
    </ContextMenu.Root>
  );
}

function ContextMenuItem({ icon, label, shortcut, onClick, danger }: { 
  icon: React.ReactNode, 
  label: string, 
  shortcut?: string, 
  onClick?: () => void,
  danger?: boolean 
}) {
  return (
    <ContextMenu.Item 
      onClick={onClick}
      className={`
        flex items-center gap-3 px-2 py-1.5 text-xs rounded-md outline-none cursor-default select-none transition-colors
        ${danger ? 'text-red-400 hover:bg-red-500/20' : 'text-white/80 hover:bg-xcode-accent/80 hover:text-white'}
      `}
    >
      <span className="opacity-70">{icon}</span>
      <span className="flex-1">{label}</span>
      {shortcut && <span className="opacity-30 text-[10px] ml-4">{shortcut}</span>}
    </ContextMenu.Item>
  );
}
