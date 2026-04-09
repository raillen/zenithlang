import * as ContextMenu from '@radix-ui/react-context-menu';
import { FilePlus, FolderPlus, Trash2, Edit3, Share, Copy } from 'lucide-react';

interface ContextMenuWrapperProps {
  children: React.ReactNode;
  onAction?: (action: string) => void;
}

export function ContextMenuWrapper({ children, onAction }: ContextMenuWrapperProps) {
  return (
    <ContextMenu.Root>
      <ContextMenu.Trigger asChild>
        {children}
      </ContextMenu.Trigger>
      
      <ContextMenu.Portal>
        <ContextMenu.Content 
          className="z-[100] min-w-[200px] rounded-[1.1rem] border p-1 shadow-2xl animate-in fade-in zoom-in-95 duration-100"
          style={{
            background: 'color-mix(in srgb, var(--ide-panel) 88%, white 12%)',
            borderColor: 'var(--border)',
            backdropFilter: 'blur(24px)',
            boxShadow: 'var(--panel-shadow-strong)',
          }}
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
          
          <ContextMenu.Separator className="my-1 h-[1px]" style={{ background: 'var(--border)' }} />
          
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
          
          <ContextMenu.Separator className="my-1 h-[1px]" style={{ background: 'var(--border)' }} />
          
          <ContextMenuItem 
            icon={<Share size={14} />} 
            label="Share" 
            onClick={() => onAction?.('share')} 
          />
          
          <ContextMenu.Separator className="my-1 h-[1px]" style={{ background: 'var(--border)' }} />
          
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
        ${danger ? 'hover:bg-red-500/12' : 'hover:bg-xcode-accent/10'}
      `}
      style={{
        color: danger ? 'var(--danger)' : 'var(--text-primary)',
      }}
    >
      <span className="opacity-70">{icon}</span>
      <span className="flex-1">{label}</span>
      {shortcut && <span className="ml-4 text-[10px] opacity-50">{shortcut}</span>}
    </ContextMenu.Item>
  );
}
