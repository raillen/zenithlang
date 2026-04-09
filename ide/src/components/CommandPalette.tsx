import { useEffect, useState } from 'react';
import { Command } from 'cmdk';
import {
  BugBeetle,
  FolderOpen,
  Files,
  FileText,
  FloppyDisk,
  GearSix,
  GitBranch,
  MagnifyingGlass,
  TerminalWindow,
} from '@phosphor-icons/react';
import { useProjectStore } from '../store/useProjectStore';
import { motion, AnimatePresence } from 'framer-motion';
import { uiMarker } from '../utils/debugSelectors';

export function CommandPalette() {
  const [isOpen, setIsOpen] = useState(false);
  const {
    fileTree,
    activeFilePath,
    openFile,
    setTheme,
    setActiveSidebarView,
    setActiveBottomPanelTab,
    setBottomPanelOpen,
    setUtilityPanelDock,
  } = useProjectStore();

  useEffect(() => {
    const down = (event: KeyboardEvent) => {
      if ((event.metaKey || event.ctrlKey) && event.key.toLowerCase() === 'k') {
        event.preventDefault();
        setIsOpen((open) => !open);
      }
    };
    const openPalette = () => setIsOpen((open) => !open);

    document.addEventListener('keydown', down);
    window.addEventListener('ide:toggle-command-palette', openPalette);
    return () => {
      document.removeEventListener('keydown', down);
      window.removeEventListener('ide:toggle-command-palette', openPalette);
    };
  }, []);

  const flatFiles: Array<{ name: string; path: string }> = [];
  const flatten = (items: any[]) => {
    items.forEach((item) => {
      if (item.is_directory) {
        flatten(item.children || []);
      } else {
        flatFiles.push({ name: item.name, path: item.path });
      }
    });
  };
  flatten(fileTree);

  return (
    <AnimatePresence>
      {isOpen && (
        <>
          <motion.div
            initial={{ opacity: 0 }}
            animate={{ opacity: 1 }}
            exit={{ opacity: 0 }}
            className="fixed inset-0 z-50 backdrop-blur-md"
            style={{ background: 'color-mix(in srgb, var(--ide-overlay) 68%, black 32%)' }}
            onClick={() => setIsOpen(false)}
            {...uiMarker('command-palette-overlay')}
          />
          <motion.div
            initial={{ opacity: 0, scale: 0.97, y: -16 }}
            animate={{ opacity: 1, scale: 1, y: 0 }}
            exit={{ opacity: 0, scale: 0.97, y: -16 }}
            className="fixed left-1/2 top-[18%] z-[51] w-[620px] max-w-[92vw] -translate-x-1/2"
            {...uiMarker('command-palette-shell')}
          >
            <Command
              className="overflow-hidden rounded-[1.25rem] border font-sans shadow-2xl"
              style={{
                background: 'color-mix(in srgb, var(--ide-panel) 88%, white 12%)',
                borderColor: 'var(--border)',
                color: 'var(--text-primary)',
                backdropFilter: 'blur(28px)',
                boxShadow: 'var(--panel-shadow-strong)',
              }}
              {...uiMarker('command-palette')}
            >
              <div
                className="flex items-center gap-3 border-b px-4 py-3"
                style={{ borderColor: 'var(--border)', background: 'color-mix(in srgb, var(--ide-toolbar) 82%, white 18%)' }}
                {...uiMarker('command-palette-header')}
              >
                <img src="/logo-only.svg" alt="Zenith" className="h-4 w-4 object-contain" {...uiMarker('command-palette-logo-mark')} />
                <img src="/logo-with-text.svg" alt="Zenith" className="h-4 w-auto object-contain opacity-85" {...uiMarker('command-palette-logo-text')} />
              </div>
              <div className="flex items-center border-b px-4" style={{ borderColor: 'var(--border)' }} {...uiMarker('command-palette-search-row')}>
                <MagnifyingGlass size={16} style={{ color: 'var(--text-tertiary)' }} />
                <Command.Input
                  placeholder="Search files and commands"
                  className="h-12 w-full bg-transparent px-3 text-[15px] outline-none"
                  style={{ color: 'var(--text-primary)' }}
                  {...uiMarker('command-palette-input')}
                />
                <span
                  className="rounded-full px-2 py-0.5 text-[10px]"
                  style={{ background: 'var(--accent-faint)', color: 'var(--text-tertiary)' }}
                  {...uiMarker('command-palette-shortcut')}
                >
                  Ctrl/Cmd+K
                </span>
              </div>

              <Command.List className="max-h-[360px] overflow-y-auto p-2" {...uiMarker('command-palette-list')}>
                <Command.Empty className="p-4 text-center text-sm" style={{ color: 'var(--text-tertiary)' }} {...uiMarker('command-palette-empty')}>
                  No results found.
                </Command.Empty>

                <Command.Group heading="Files" className="mb-2 px-2 text-[12px] font-medium" style={{ color: 'var(--text-tertiary)' }} {...uiMarker('command-palette-group', { group: 'files' })}>
                  {flatFiles.map((file) => (
                    <Command.Item
                      key={file.path}
                      value={`${file.name} ${file.path}`}
                      onSelect={() => {
                        openFile(file.path);
                        setIsOpen(false);
                      }}
                      className="flex cursor-pointer items-center gap-3 rounded-xl px-3 py-2.5 text-sm transition-colors aria-selected:bg-xcode-accent/10"
                      {...uiMarker('command-palette-file-item', { filePath: file.path })}
                    >
                      <FileText size={16} style={{ color: 'var(--text-tertiary)' }} />
                      <span className="truncate">{file.name}</span>
                      <span className="ml-auto max-w-[220px] truncate text-[11px]" style={{ color: 'var(--text-tertiary)' }}>
                        {file.path}
                      </span>
                    </Command.Item>
                  ))}
                </Command.Group>

                <Command.Group heading="Actions" className="mt-4 px-2 text-[12px] font-medium" style={{ color: 'var(--text-tertiary)' }} {...uiMarker('command-palette-group', { group: 'actions' })}>
                  <Command.Item
                    value="open folder"
                    onSelect={() => {
                      window.dispatchEvent(new CustomEvent('ide:open-folder-dialog'));
                      setIsOpen(false);
                    }}
                    className="flex cursor-pointer items-center gap-3 rounded-xl px-3 py-2.5 text-sm transition-colors aria-selected:bg-xcode-accent/10"
                    {...uiMarker('command-palette-action', { action: 'open-folder' })}
                  >
                    <FolderOpen size={16} />
                    <span>Open Folder</span>
                  </Command.Item>
                  <Command.Item
                    value="open file"
                    onSelect={() => {
                      window.dispatchEvent(new CustomEvent('ide:open-file-dialog'));
                      setIsOpen(false);
                    }}
                    className="flex cursor-pointer items-center gap-3 rounded-xl px-3 py-2.5 text-sm transition-colors aria-selected:bg-xcode-accent/10"
                    {...uiMarker('command-palette-action', { action: 'open-file' })}
                  >
                    <FileText size={16} />
                    <span>Open File</span>
                  </Command.Item>
                  <Command.Item
                    value="save active file"
                    onSelect={() => {
                      window.dispatchEvent(new CustomEvent('ide:save-active-document'));
                      setIsOpen(false);
                    }}
                    className="flex cursor-pointer items-center gap-3 rounded-xl px-3 py-2.5 text-sm transition-colors aria-selected:bg-xcode-accent/10"
                    disabled={!activeFilePath}
                    {...uiMarker('command-palette-action', { action: 'save', disabled: !activeFilePath })}
                  >
                    <FloppyDisk size={16} />
                    <span>Save</span>
                  </Command.Item>
                  <Command.Item
                    value="save as active file"
                    onSelect={() => {
                      window.dispatchEvent(new CustomEvent('ide:save-active-document-as'));
                      setIsOpen(false);
                    }}
                    className="flex cursor-pointer items-center gap-3 rounded-xl px-3 py-2.5 text-sm transition-colors aria-selected:bg-xcode-accent/10"
                    disabled={!activeFilePath}
                    {...uiMarker('command-palette-action', { action: 'save-as', disabled: !activeFilePath })}
                  >
                    <FloppyDisk size={16} />
                    <span>Save As</span>
                  </Command.Item>
                  <Command.Item
                    value="explorer files"
                    onSelect={() => {
                      setActiveSidebarView('explorer');
                      setIsOpen(false);
                    }}
                    className="flex cursor-pointer items-center gap-3 rounded-xl px-3 py-2.5 text-sm transition-colors aria-selected:bg-xcode-accent/10"
                    {...uiMarker('command-palette-action', { action: 'focus-explorer' })}
                  >
                    <Files size={16} />
                    <span>Focus Explorer</span>
                  </Command.Item>
                  <Command.Item
                    value="search pane"
                    onSelect={() => {
                      setActiveSidebarView('search');
                      setIsOpen(false);
                    }}
                    className="flex cursor-pointer items-center gap-3 rounded-xl px-3 py-2.5 text-sm transition-colors aria-selected:bg-xcode-accent/10"
                    {...uiMarker('command-palette-action', { action: 'open-search' })}
                  >
                    <MagnifyingGlass size={16} />
                    <span>Open Search</span>
                  </Command.Item>
                  <Command.Item
                    value="source control git"
                    onSelect={() => {
                      setActiveSidebarView('sourceControl');
                      setIsOpen(false);
                    }}
                    className="flex cursor-pointer items-center gap-3 rounded-xl px-3 py-2.5 text-sm transition-colors aria-selected:bg-xcode-accent/10"
                    {...uiMarker('command-palette-action', { action: 'open-source-control' })}
                  >
                    <GitBranch size={16} />
                    <span>Open Source Control</span>
                  </Command.Item>
                  <Command.Item
                    value="run debug build"
                    onSelect={() => {
                      setActiveSidebarView('run');
                      setBottomPanelOpen(true);
                      setActiveBottomPanelTab('output');
                      setIsOpen(false);
                    }}
                    className="flex cursor-pointer items-center gap-3 rounded-xl px-3 py-2.5 text-sm transition-colors aria-selected:bg-xcode-accent/10"
                    {...uiMarker('command-palette-action', { action: 'open-run-debug' })}
                  >
                    <BugBeetle size={16} />
                    <span>Open Run and Debug</span>
                  </Command.Item>
                  <Command.Item
                    value="terminal panel"
                    onSelect={() => {
                      setBottomPanelOpen(true);
                      setActiveBottomPanelTab('terminal');
                      setIsOpen(false);
                    }}
                    className="flex cursor-pointer items-center gap-3 rounded-xl px-3 py-2.5 text-sm transition-colors aria-selected:bg-xcode-accent/10"
                    {...uiMarker('command-palette-action', { action: 'open-terminal-panel' })}
                  >
                    <TerminalWindow size={16} />
                    <span>Open Terminal Panel</span>
                  </Command.Item>
                  <Command.Item
                    value="terminal dock left"
                    onSelect={() => {
                      setUtilityPanelDock('left');
                      setBottomPanelOpen(true);
                      setActiveBottomPanelTab('terminal');
                      setIsOpen(false);
                    }}
                    className="flex cursor-pointer items-center gap-3 rounded-xl px-3 py-2.5 text-sm transition-colors aria-selected:bg-xcode-accent/10"
                    {...uiMarker('command-palette-action', { action: 'dock-terminal-left' })}
                  >
                    <TerminalWindow size={16} />
                    <span>Dock Panel Left</span>
                  </Command.Item>
                  <Command.Item
                    value="terminal dock bottom"
                    onSelect={() => {
                      setUtilityPanelDock('bottom');
                      setBottomPanelOpen(true);
                      setActiveBottomPanelTab('terminal');
                      setIsOpen(false);
                    }}
                    className="flex cursor-pointer items-center gap-3 rounded-xl px-3 py-2.5 text-sm transition-colors aria-selected:bg-xcode-accent/10"
                    {...uiMarker('command-palette-action', { action: 'dock-terminal-bottom' })}
                  >
                    <TerminalWindow size={16} />
                    <span>Dock Panel Bottom</span>
                  </Command.Item>
                  <Command.Item
                    value="terminal dock right"
                    onSelect={() => {
                      setUtilityPanelDock('right');
                      setBottomPanelOpen(true);
                      setActiveBottomPanelTab('terminal');
                      setIsOpen(false);
                    }}
                    className="flex cursor-pointer items-center gap-3 rounded-xl px-3 py-2.5 text-sm transition-colors aria-selected:bg-xcode-accent/10"
                    {...uiMarker('command-palette-action', { action: 'dock-terminal-right' })}
                  >
                    <TerminalWindow size={16} />
                    <span>Dock Panel Right</span>
                  </Command.Item>
                  <Command.Item
                    value="theme zenith"
                    onSelect={() => {
                      setTheme('zenith');
                      setIsOpen(false);
                    }}
                    className="flex cursor-pointer items-center gap-3 rounded-xl px-3 py-2.5 text-sm transition-colors aria-selected:bg-xcode-accent/10"
                    {...uiMarker('command-palette-action', { action: 'theme-zenith' })}
                  >
                    <GearSix size={16} />
                    <span>Switch to Zenith Theme</span>
                  </Command.Item>
                </Command.Group>
              </Command.List>
            </Command>
          </motion.div>
        </>
      )}
    </AnimatePresence>
  );
}
