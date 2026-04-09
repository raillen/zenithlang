import { useState, useRef, useEffect } from "react";
import { Menu, X, FilePlus, FolderOpen, Save, Hammer, Play, Settings, HelpCircle, Info, Zap } from "lucide-react";
import { useWorkspaceStore, FileEntry } from "../store/useWorkspaceStore";
import { invoke } from "../utils/tauri";

export function ZenithMenu() {
  const [isOpen, setIsOpen] = useState(false);
  const menuRef = useRef<HTMLDivElement>(null);
  const { setProjectRoot, openFile } = useWorkspaceStore();

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
            <MenuSection title="Arquivo">
              <MenuItem icon={<FilePlus size={14} />} label="Novo Script (.zt)" onClick={() => {}} />
              <MenuItem icon={<FolderOpen size={14} />} label="Abrir Pasta..." onClick={handleOpenFolder} />
              <MenuItem icon={<FilePlus size={14} />} label="Abrir Arquivo..." onClick={handleOpenFile} />
              <MenuItem icon={<Save size={14} />} label="Salvar como..." onClick={() => {}} />
            </MenuSection>

            {/* Category: Zenith Tools */}
            <MenuSection title="Zenith">
              <MenuItem icon={<Hammer size={14} />} label="Compilar (Hammer)" onClick={() => {}} />
              <MenuItem icon={<Play size={14} />} label="Rodar em Zenith" onClick={() => {}} />
              <MenuItem icon={<Zap size={14} />} label="Check Diagnostics" onClick={() => {}} />
            </MenuSection>

            {/* Category: Help & Settings */}
            <MenuSection title="Sistema">
              <MenuItem icon={<Settings size={14} />} label="Preferências" onClick={() => {}} />
              <MenuItem icon={<HelpCircle size={14} />} label="Documentação" onClick={() => {}} />
              <MenuItem icon={<Info size={14} />} label="Sobre Zenith" onClick={() => {}} />
            </MenuSection>
          </div>

          <div className="mt-6 pt-4 border-t border-ide-border flex items-center justify-between">
            <div className="flex items-center gap-2 text-primary font-bold tracking-tighter italic scale-110">
              <div className="w-5 h-5 bg-primary rounded-md flex items-center justify-center">
                 <div className="w-2 h-2 bg-white rotate-45" />
              </div>
              ZENITH HUB
            </div>
            <div className="text-[10px] text-zinc-400">Version 0.2.0-Alpha</div>
          </div>
        </div>
      )}
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
