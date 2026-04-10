import { useState, useRef, useEffect } from "react";
import { FilePlus, FolderOpen, Save, Hammer, Play, Settings, HelpCircle, Info, Zap } from "lucide-react";
import { useWorkspaceStore, FileEntry } from "../store/useWorkspaceStore";
import { invoke } from "../utils/tauri";
import { useCommandStore } from "../store/useCommandStore";
import { useTranslation } from "../utils/i18n";
import { BrandLogo } from "./BrandLogo";

export function ZenithMenu() {
  const [isOpen, setIsOpen] = useState(false);
  const { setSettingsOpen } = useCommandStore();
  const menuRef = useRef<HTMLDivElement>(null);
  const { setProjectRoot, openFile } = useWorkspaceStore();
  const { t } = useTranslation();

  useEffect(() => {
    function handleClickOutside(event: MouseEvent) {
      if (menuRef.current && !menuRef.current.contains(event.target as Node)) {
        setIsOpen(false);
      }
    }
    document.addEventListener("mousedown", handleClickOutside);
    return () => document.removeEventListener("mousedown", handleClickOutside);
  }, []);

  const handleOpenFolder = async () => {
    try {
      const path = await invoke<string | null>("pick_folder");
      if (path) {
        setProjectRoot(path);
        setIsOpen(false);
      }
    } catch (err) {
      console.error("Failed to pick folder", err);
    }
  };

  const handleOpenFile = async () => {
    try {
        const path = await invoke<string | null>("pick_file");
        if (path) {
            const name = path.split(/[\\/]/).pop() || "file";
            const fileStub: FileEntry = { name, path, is_directory: false };
            openFile(fileStub, true);
            setIsOpen(false);
        }
    } catch (err) {
        console.error("Failed to pick file", err);
    }
  };

  return (
    <div className="relative" ref={menuRef}>
      <button 
        onClick={() => setIsOpen(!isOpen)}
        className={`flex h-8 w-8 items-center justify-center rounded-lg transition-all ${
          isOpen
            ? 'bg-primary/10 text-primary shadow-sm ring-1 ring-primary/20'
            : 'text-ide-text-dim hover:bg-black/5 hover:text-ide-text active:scale-95'
        }`}
        title={t('hub.title')}
      >
        <BrandLogo variant="icon" className={`h-5 w-5 transition-transform duration-300 ${isOpen ? 'rotate-90 scale-90' : ''}`} />
      </button>

      {isOpen && (
        <div className="absolute top-10 left-0 w-[500px] rounded-2xl border border-ide-border bg-ide-panel p-6 shadow-2xl z-[100] animate-in fade-in slide-in-from-top-2 duration-300 ring-1 ring-black/5">
          <div className="flex gap-8">
            {/* Category: File */}
            <MenuSection title={t('hub.categories.file')}>
              <MenuItem icon={<FilePlus size={14} />} label={t('hub.actions.new_script')} onClick={() => {}} />
              <MenuItem icon={<FolderOpen size={14} />} label={t('hub.actions.open_folder')} onClick={handleOpenFolder} />
              <MenuItem icon={<FilePlus size={14} />} label={t('hub.actions.open_file')} onClick={handleOpenFile} />
              <MenuItem icon={<Save size={14} />} label={t('hub.actions.save_as')} onClick={() => {}} />
            </MenuSection>

            {/* Category: Zenith Tools */}
            <MenuSection title={t('hub.categories.zenith')}>
              <MenuItem icon={<Hammer size={14} />} label={t('hub.actions.compile')} onClick={() => {}} />
              <MenuItem icon={<Play size={14} />} label={t('hub.actions.run')} onClick={() => {}} />
              <MenuItem icon={<Zap size={14} />} label={t('hub.actions.check')} onClick={() => {}} />
            </MenuSection>

            {/* Category: Help & Settings */}
            <MenuSection title={t('hub.categories.system')}>
              <MenuItem icon={<Settings size={14} />} label={t('hub.actions.preferences')} onClick={() => {
                setSettingsOpen(true);
                setIsOpen(false);
              }} />
              <MenuItem icon={<HelpCircle size={14} />} label={t('hub.actions.docs')} onClick={() => {}} />
              <MenuItem icon={<Info size={14} />} label={t('hub.actions.about')} onClick={() => {}} />
            </MenuSection>
          </div>

          <div className="mt-6 pt-4 border-t border-ide-border flex items-center justify-between">
            <BrandLogo variant="lockup" className="h-6 w-auto" alt={t('hub.title')} />
            <div className="text-[10px] text-ide-text-dim/50">{t('hub.version')}</div>
          </div>
        </div>
      )}
    </div>
  );
}

function MenuSection({ title, children }: { title: string, children: React.ReactNode }) {
  return (
    <div className="flex-1 flex flex-col gap-3">
      <h3 className="text-[10px] font-bold uppercase tracking-widest text-ide-text-dim/60 mb-1">{title}</h3>
      <div className="flex flex-col gap-1">
        {children}
      </div>
    </div>
  );
}

function MenuItem({ icon, label, onClick }: { icon: React.ReactNode, label: string, onClick: () => void }) {
  return (
    <button 
      onClick={onClick}
      className="flex items-center gap-3 px-2 py-1.5 rounded-md text-[11px] text-ide-text-dim hover:bg-primary/10 hover:text-primary transition-all text-left group"
    >
      <span className="opacity-50 group-hover:opacity-100 transition-opacity">{icon}</span>
      <span className="font-medium">{label}</span>
    </button>
  );
}
