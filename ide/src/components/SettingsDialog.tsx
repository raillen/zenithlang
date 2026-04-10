import { useState } from "react";
import { createPortal } from "react-dom";
import { X, Globe, Cpu, Palette, Code, Check, Blocks, ChevronRight, ChevronDown } from "lucide-react";
import { useWorkspaceStore } from "../store/useWorkspaceStore";
import { useTranslation } from "../utils/i18n";
import { THEMES } from "../themes";
import { SearchableDropdown, DropdownOption } from "./ui/SearchableDropdown";
import { ToggleSwitch } from "./ui/ToggleSwitch";

interface SettingsDialogProps {
  onClose: () => void;
}

export function SettingsDialog({ onClose }: SettingsDialogProps) {
  const [activeTab, setActiveTab] = useState<'general' | 'editor' | 'extensions'>('general');
  const { settings, updateSettings } = useWorkspaceStore();
  const { t } = useTranslation();

  const themeOptions: DropdownOption[] = Object.values(THEMES).map(t => ({
    value: t.id,
    label: t.name
  }));

  const handleLanguageChange = (lang: 'pt' | 'en' | 'es') => {
    updateSettings({ language: lang });
  };

  const modalContent = (
    <div className="fixed inset-0 z-[9999] flex items-center justify-center bg-black/20 backdrop-blur-sm animate-in fade-in duration-200">
      <div className="w-[600px] h-[450px] border border-ide-border bg-ide-bg rounded-2xl shadow-2xl flex overflow-hidden animate-in zoom-in-95 duration-300">
        
        {/* Sidebar */}
        <div className="w-48 bg-ide-panel border-r border-ide-border p-4 flex flex-col gap-1">
          <h2 className="text-[14px] font-bold text-ide-text mb-4 px-2">{t('settings.title')}</h2>
          
          <TabButton 
            active={activeTab === 'general'} 
            onClick={() => setActiveTab('general')} 
            icon={<Globe size={14} />} 
            label={t('settings.tabs.general')} 
          />
          <TabButton 
            active={activeTab === 'editor'} 
            onClick={() => setActiveTab('editor')} 
            icon={<Palette size={14} />} 
            label={t('settings.tabs.editor')} 
          />
          <TabButton 
            active={activeTab === 'extensions'} 
            onClick={() => setActiveTab('extensions')} 
            icon={<Blocks size={14} />} 
            label="Extensões" 
          />
        </div>

        {/* Content */}
        <div className="flex-1 flex flex-col min-w-0 bg-ide-bg text-ide-text">
          <div className="h-12 flex items-center justify-end px-4 border-b border-ide-border">
            <button onClick={onClose} className="p-1.5 hover:bg-black/5 rounded-full text-ide-text-dim hover:text-ide-text transition-colors">
              <X size={18} />
            </button>
          </div>

          <div className="p-8 overflow-y-auto flex-1">
            {activeTab === 'general' && (
              <div className="flex flex-col gap-8">
                <section>
                  <label className="text-[11px] font-bold text-ide-text-dim uppercase tracking-widest block mb-4">
                    {t('settings.labels.language')}
                  </label>
                  <div className="flex flex-col gap-2">
                    <LangOption 
                        label="Português (Brasil)" 
                        active={settings.language === 'pt'} 
                        onClick={() => handleLanguageChange('pt')} 
                    />
                    <LangOption 
                        label="English (US)" 
                        active={settings.language === 'en'} 
                        onClick={() => handleLanguageChange('en')} 
                    />
                    <LangOption 
                        label="Español (ES)" 
                        active={settings.language === 'es'} 
                        onClick={() => handleLanguageChange('es')} 
                    />
                  </div>
                </section>

                <section className="flex flex-col gap-2">
                   <label className="text-[11px] font-bold text-ide-text-dim uppercase tracking-widest mb-1">
                     Workspace Behavior
                   </label>

                   <div className="flex items-center justify-between py-2 border-b border-ide-border">
                     <div className="flex flex-col">
                       <span className="text-[12px] text-ide-text">Restore Previous Session</span>
                       <span className="text-[10px] text-ide-text-dim opacity-70">Reopen files from your last session.</span>
                     </div>
                     <ToggleSwitch checked={settings.restoreSession} onChange={(val) => updateSettings({ restoreSession: val })} />
                   </div>

                   <div className="flex items-center justify-between py-2 border-b border-ide-border">
                     <div className="flex flex-col">
                       <span className="text-[12px] text-ide-text">Auto-Save</span>
                       <span className="text-[10px] text-ide-text-dim opacity-70">Automatically save dirty files.</span>
                     </div>
                     <select 
                       value={settings.autoSave}
                       onChange={(e) => updateSettings({ autoSave: e.target.value as 'off' | 'onFocusChange' | 'afterDelay' })}
                       className="bg-ide-panel border border-ide-border rounded-lg px-2 py-1 text-[11px] text-ide-text focus:outline-none"
                     >
                       <option value="off">Off</option>
                       <option value="onFocusChange">On Focus Change</option>
                       <option value="afterDelay">After Delay</option>
                     </select>
                   </div>
                </section>
              </div>
            )}

            {activeTab === 'editor' && (
               <div className="flex flex-col gap-8">
                 {/* Theme */}
                 <section className="flex flex-col gap-2">
                   <label className="text-[11px] font-bold text-ide-text-dim uppercase tracking-widest">
                     Color Theme
                   </label>
                   <p className="text-[11px] text-ide-text-dim opacity-70 mb-2">Select the active color and layout profile.</p>
                   <SearchableDropdown 
                     options={themeOptions}
                     value={settings.theme}
                     onChange={(val) => updateSettings({ theme: val })}
                   />
                 </section>

                 {/* Typography */}
                 <section className="flex flex-col gap-2">
                   <label className="text-[11px] font-bold text-ide-text-dim uppercase tracking-widest">
                     Typography
                   </label>
                   
                   <div className="flex items-center justify-between py-2 border-b border-ide-border">
                     <span className="text-[12px] text-ide-text">Font Family</span>
                     <input 
                        type="text" 
                        value={settings.editorFontFamily}
                        onChange={(e) => updateSettings({ editorFontFamily: e.target.value })}
                        className="w-48 bg-ide-panel border border-ide-border rounded-lg px-2 py-1 text-[11px] focus:outline-none"
                     />
                   </div>
                   
                   <div className="flex items-center justify-between py-2 border-b border-ide-border">
                     <span className="text-[12px] text-ide-text">Font Size</span>
                     <input 
                        type="number" 
                        value={settings.editorFontSize}
                        onChange={(e) => updateSettings({ editorFontSize: Number(e.target.value) })}
                        className="w-20 bg-ide-panel border border-ide-border rounded-lg px-2 py-1 text-[11px] focus:outline-none"
                     />
                   </div>

                   <div className="flex items-center justify-between py-2 border-b border-ide-border">
                     <span className="text-[12px] text-ide-text">Font Ligatures</span>
                     <ToggleSwitch checked={settings.editorFontLigatures} onChange={(val) => updateSettings({ editorFontLigatures: val })} />
                   </div>
                 </section>

                 {/* Ergonomics */}
                 <section className="flex flex-col gap-2">
                   <label className="text-[11px] font-bold text-ide-text-dim uppercase tracking-widest mb-1">
                     View & Ergonomics
                   </label>

                   <div className="flex items-center justify-between py-2 border-b border-ide-border">
                     <div className="flex flex-col">
                       <span className="text-[12px] text-ide-text">Word Wrap</span>
                       <span className="text-[10px] text-ide-text-dim opacity-70">Wrap text to the viewport boundary.</span>
                     </div>
                     <ToggleSwitch checked={settings.editorWordWrap} onChange={(val) => updateSettings({ editorWordWrap: val })} />
                   </div>

                   <div className="flex items-center justify-between py-2 border-b border-ide-border">
                     <div className="flex flex-col">
                       <span className="text-[12px] text-ide-text">Minimap</span>
                       <span className="text-[10px] text-ide-text-dim opacity-70">Show code overview. (Disable for focus).</span>
                     </div>
                     <ToggleSwitch checked={settings.editorMinimap} onChange={(val) => updateSettings({ editorMinimap: val })} />
                   </div>
                 </section>
               </div>
            )}

            {activeTab === 'extensions' && (
              <div className="flex flex-col gap-6">
                <section>
                    <label className="text-[11px] font-bold text-ide-text-dim uppercase tracking-widest block mb-4">
                      Toolchains Ativos
                    </label>
                    <p className="text-[11px] text-ide-text-dim opacity-70 mb-4">
                      Configure binários, LSP e builds das linguagens ativas no seu workspace.
                    </p>

                    <div className="flex flex-col gap-2">
                       {Object.entries(settings.extensions || {}).map(([key, ext]) => (
                          <ExtensionAccordion 
                             key={key}
                             id={key}
                             config={ext}
                             onUpdate={(newCfg) => {
                                updateSettings({
                                   extensions: {
                                      ...settings.extensions,
                                      [key]: { ...ext, ...newCfg }
                                   }
                                });
                             }}
                          />
                       ))}
                    </div>
                </section>
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );

  return createPortal(modalContent, document.body);
}

function TabButton({ active, icon, label, onClick }: { active: boolean, icon: React.ReactNode, label: string, onClick: () => void }) {
  return (
    <button 
      onClick={onClick}
      className={`flex items-center gap-3 px-3 py-2 rounded-lg text-[11px] transition-all ${active ? 'bg-primary text-white shadow-md font-medium' : 'text-ide-text-dim hover:bg-ide-panel hover:text-ide-text'}`}
    >
      {icon}
      {label}
    </button>
  );
}

function ExtensionAccordion({ id, config, onUpdate }: { id: string, config: any, onUpdate: (cfg: any) => void }) {
  const [open, setOpen] = useState(id === 'zenith'); // Open zenith by default

  const headerLabel = id.charAt(0).toUpperCase() + id.slice(1) + " Language";

  return (
    <div className="border border-ide-border rounded-xl overflow-hidden bg-ide-bg transition-all">
      <button 
         onClick={() => setOpen(!open)}
         className={`w-full flex items-center justify-between px-4 py-3 hover:bg-ide-panel transition-colors ${open ? 'border-b border-ide-border bg-ide-panel/50' : ''}`}
      >
         <div className="flex items-center gap-3">
            <span className="text-[12px] font-bold text-ide-text">{headerLabel}</span>
         </div>
         <div className="flex items-center gap-4">
            <ToggleSwitch 
               checked={config.isEnabled} 
               onChange={(val) => {
                  onUpdate({ isEnabled: val });
               }} 
            />
            {open ? <ChevronDown size={16} className="text-ide-text-dim" /> : <ChevronRight size={16} className="text-ide-text-dim" />}
         </div>
      </button>

      {open && (
         <div className="p-4 flex flex-col gap-4 animate-in fade-in slide-in-from-top-2 duration-200">
            <div>
               <label className="text-[10px] font-bold text-ide-text-dim uppercase mb-1 block">Root Path / Binary</label>
               <input 
                 type="text" 
                 value={config.compilerPath || ""}
                 onChange={(e) => onUpdate({ compilerPath: e.target.value })}
                 className="w-full bg-ide-panel border border-ide-border rounded-lg px-3 py-2 text-[11px] font-mono text-ide-text focus:outline-none focus:border-ide-text-dim"
                 placeholder="e.g. ztc.lua"
               />
            </div>
            
            <div>
               <label className="text-[10px] font-bold text-ide-text-dim uppercase mb-1 block">LSP Server Path</label>
               <input 
                 type="text" 
                 value={config.lspPath || ""}
                 onChange={(e) => onUpdate({ lspPath: e.target.value })}
                 className="w-full bg-ide-panel border border-ide-border rounded-lg px-3 py-2 text-[11px] font-mono text-ide-text focus:outline-none focus:border-ide-text-dim opacity-70 cursor-not-allowed"
                 placeholder="Language Server Path (Coming Soon)"
                 disabled
               />
            </div>

            <div>
               <label className="text-[10px] font-bold text-ide-text-dim uppercase mb-1 block">Default Build Task</label>
               <input 
                 type="text" 
                 value={config.buildCommand || ""}
                 onChange={(e) => onUpdate({ buildCommand: e.target.value })}
                 className="w-full bg-ide-panel border border-ide-border rounded-lg px-3 py-2 text-[11px] font-mono text-ide-text focus:outline-none focus:border-ide-text-dim"
                 placeholder="e.g. lua ztc.lua build"
               />
            </div>
         </div>
      )}
    </div>
  );
}

function LangOption({ label, active, onClick }: { label: string, active: boolean, onClick: () => void }) {
  return (
    <button 
      onClick={onClick}
      className={`flex items-center justify-between px-4 py-3 rounded-xl border transition-all ${active ? 'border-primary bg-primary/5 text-primary' : 'border-ide-border hover:border-ide-text-dim text-ide-text'}`}
    >
      <span className="text-[12px] font-medium">{label}</span>
      {active && <Check size={14} />}
    </button>
  );
}

