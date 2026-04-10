import { useState, useRef, useEffect } from "react";
import { Menu, X, FilePlus, FolderOpen, Save, Hammer, Play, Settings, HelpCircle, Info, Zap } from "lucide-react";
import { useWorkspaceStore, FileEntry } from "../store/useWorkspaceStore";
import { invoke } from "../utils/tauri";
import { useTranslation } from "../utils/i18n";
import { SettingsDialog } from "./SettingsDialog";

export function ZenithMenu() {
  const [isOpen, setIsOpen] = useState(false);
  const [isSettingsOpen, setIsSettingsOpen] = useState(false);
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
        className={`p-2 rounded-lg transition-all ${isOpen ? 'bg-primary text-white shadow-lg' : 'hover:bg-black/5 text-zinc-600'}`}
      >
        {isOpen ? <X size={20} /> : <Menu size={20} />}
      </button>

      {isOpen && (
        <div className="absolute top-12 left-0 w-[500px] bg-white/98 backdrop-blur-2xl border border-ide-border rounded-2xl shadow-2xl p-6 z-[100] animate-in fade-in slide-in-from-top-4 duration-300">
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
                setIsSettingsOpen(true);
                setIsOpen(false);
              }} />
              <MenuItem icon={<HelpCircle size={14} />} label={t('hub.actions.docs')} onClick={() => {}} />
              <MenuItem icon={<Info size={14} />} label={t('hub.actions.about')} onClick={() => {}} />
            </MenuSection>
          </div>

          <div className="mt-6 pt-4 border-t border-ide-border flex items-center justify-between">
            <div className="flex items-center gap-2 text-primary font-bold tracking-tighter italic scale-110">
              <div className="w-5 h-5 bg-primary rounded-md flex items-center justify-center">
                 <div className="w-2 h-2 bg-white rotate-45" />
              </div>
              {t('hub.title')}
            </div>
            <div className="text-[10px] text-zinc-400">{t('hub.version')}</div>
          </div>
        </div>
      )}

      {isSettingsOpen && <SettingsDialog onClose={() => setIsSettingsOpen(false)} />}
    </div>
  );
}

function MenuSection({ title, children }: { title: string, children: React.ReactNode }) {
  return (
    <div className="flex-1 flex flex-col gap-3">
      <h3 className="text-[10px] font-bold uppercase tracking-widest text-zinc-400 mb-1">{title}</h3>
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
      className="flex items-center gap-3 px-2 py-1.5 rounded-md text-[11px] text-zinc-600 hover:bg-primary/10 hover:text-primary transition-all text-left group"
    >
      <span className="opacity-50 group-hover:opacity-100 transition-opacity">{icon}</span>
      <span className="font-medium">{label}</span>
    </button>
  );
}
