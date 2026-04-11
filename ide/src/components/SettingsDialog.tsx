import { useState, useEffect } from "react";
import { createPortal } from "react-dom";
import { X, Globe, Palette, Check, Blocks, ChevronRight, ChevronDown, Keyboard, Pencil, Shield } from "lucide-react";
import { useWorkspaceStore } from "../store/useWorkspaceStore";
import { useCommandStore, type SettingsTab } from "../store/useCommandStore";
import { useTranslation } from "../utils/i18n";
import { THEMES } from "../themes";
import { SearchableDropdown, DropdownOption } from "./ui/SearchableDropdown";
import { ToggleSwitch } from "./ui/ToggleSwitch";
import { getRuntimeProfileForSettingsKey } from "../utils/runtimeProfiles";

interface SettingsDialogProps {
  onClose: () => void;
}

export function SettingsDialog({ onClose }: SettingsDialogProps) {
  const settingsTab = useCommandStore((state) => state.settingsTab);
  const [activeTab, setActiveTab] = useState<SettingsTab>(settingsTab);
  const { settings, updateSettings } = useWorkspaceStore();
  const { t } = useTranslation();

  const themeOptions: DropdownOption[] = Object.values(THEMES).map(t => ({
    value: t.id,
    label: t.name
  }));

  const handleLanguageChange = (lang: 'pt' | 'en' | 'es') => {
    updateSettings({ language: lang });
  };

  useEffect(() => {
    setActiveTab(settingsTab);
  }, [settingsTab]);

  const updateToolbarSetting = (key: keyof typeof settings.toolbar, value: boolean) => {
    updateSettings({
      toolbar: {
        ...settings.toolbar,
        [key]: value,
      },
    });
  };

  const modalContent = (
    <div className="fixed inset-0 z-[9999] flex items-center justify-center bg-black/20 backdrop-blur-sm animate-in fade-in duration-200">
      <div data-testid="settings-dialog" className="w-[850px] h-[600px] max-w-[90vw] max-h-[85vh] border border-ide-border bg-ide-bg rounded-2xl shadow-2xl flex overflow-hidden animate-in zoom-in-95 duration-300">
        
        {/* Sidebar */}
        <div className="w-56 bg-ide-panel border-r border-ide-border p-6 flex flex-col gap-2">
          <h2 className="text-[14px] font-bold text-ide-text mb-6 px-2">{t('settings.title')}</h2>
          
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
            active={activeTab === 'keymap'} 
            onClick={() => setActiveTab('keymap')} 
            icon={<Keyboard size={14} />} 
            label="Keybinds" 
          />
          <TabButton 
            active={activeTab === 'extensions'} 
            onClick={() => setActiveTab('extensions')} 
            icon={<Blocks size={14} />} 
            label="Extensões" 
          />
          <TabButton 
            active={activeTab === 'sanctuary'} 
            onClick={() => setActiveTab('sanctuary')} 
            icon={<Shield size={14} />} 
            label="Sanctuary" 
          />
        </div>

        {/* Content */}
        <div className="flex-1 flex flex-col min-w-0 bg-ide-bg text-ide-text">
          <div className="h-12 flex items-center justify-end px-4 border-b border-ide-border">
            <button data-testid="settings-close" onClick={onClose} className="p-1.5 hover:bg-black/5 rounded-full text-ide-text-dim hover:text-ide-text transition-colors">
              <X size={18} />
            </button>
          </div>

          <div className="px-12 py-10 overflow-y-auto flex-1">
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

                <section className="flex flex-col gap-2">
                  <label className="text-[11px] font-bold text-ide-text-dim uppercase tracking-widest mb-1">
                    Toolbar Modules
                  </label>
                  <p className="text-[11px] text-ide-text-dim opacity-70 mb-2">
                    Show or hide secondary toolbar functions without changing the core workbench flow.
                  </p>

                  <div className="flex items-center justify-between py-2 border-b border-ide-border">
                    <div className="flex flex-col">
                      <span className="text-[12px] text-ide-text">Workspace Selector</span>
                      <span className="text-[10px] text-ide-text-dim opacity-70">Show the current workspace and saved contexts switcher.</span>
                    </div>
                    <ToggleSwitch checked={settings.toolbar.showWorkspaceSelector} onChange={(val) => updateToolbarSetting('showWorkspaceSelector', val)} />
                  </div>

                  <div className="flex items-center justify-between py-2 border-b border-ide-border">
                    <div className="flex flex-col">
                      <span className="text-[12px] text-ide-text">Git Branch</span>
                      <span className="text-[10px] text-ide-text-dim opacity-70">Keep the active branch visible near the project context.</span>
                    </div>
                    <ToggleSwitch checked={settings.toolbar.showGitBranch} onChange={(val) => updateToolbarSetting('showGitBranch', val)} />
                  </div>

                  <div className="flex items-center justify-between py-2 border-b border-ide-border">
                    <div className="flex flex-col">
                      <span className="text-[12px] text-ide-text">Quick Open</span>
                      <span className="text-[10px] text-ide-text-dim opacity-70">Keep the command-style file launcher in the header.</span>
                    </div>
                    <ToggleSwitch checked={settings.toolbar.showQuickOpen} onChange={(val) => updateToolbarSetting('showQuickOpen', val)} />
                  </div>

                  <div className="flex items-center justify-between py-2 border-b border-ide-border">
                    <div className="flex flex-col">
                      <span className="text-[12px] text-ide-text">Problems Counter</span>
                      <span className="text-[10px] text-ide-text-dim opacity-70">Show diagnostics totals beside the run controls.</span>
                    </div>
                    <ToggleSwitch checked={settings.toolbar.showDiagnostics} onChange={(val) => updateToolbarSetting('showDiagnostics', val)} />
                  </div>

                  <div className="flex items-center justify-between py-2 border-b border-ide-border">
                    <div className="flex flex-col">
                      <span className="text-[12px] text-ide-text">Runtime Target</span>
                      <span className="text-[10px] text-ide-text-dim opacity-70">Display the active runtime profile and run target selector.</span>
                    </div>
                    <ToggleSwitch checked={settings.toolbar.showRuntimeTarget} onChange={(val) => updateToolbarSetting('showRuntimeTarget', val)} />
                  </div>

                  <div className="flex items-center justify-between py-2 border-b border-ide-border">
                    <div className="flex flex-col">
                      <span className="text-[12px] text-ide-text">Panel Toggles</span>
                      <span className="text-[10px] text-ide-text-dim opacity-70">Show controls for the left sidebar, inspector and bottom panel.</span>
                    </div>
                    <ToggleSwitch checked={settings.toolbar.showPanelToggles} onChange={(val) => updateToolbarSetting('showPanelToggles', val)} />
                  </div>

                  <div className="flex items-center justify-between py-2 border-b border-ide-border">
                    <div className="flex flex-col">
                      <span className="text-[12px] text-ide-text">Layout Controls</span>
                      <span className="text-[10px] text-ide-text-dim opacity-70">Keep split layout buttons visible in the header.</span>
                    </div>
                    <ToggleSwitch checked={settings.toolbar.showLayoutControls} onChange={(val) => updateToolbarSetting('showLayoutControls', val)} />
                  </div>

                  <div className="flex items-center justify-between py-2 border-b border-ide-border">
                    <div className="flex flex-col">
                      <span className="text-[12px] text-ide-text">Focus Hints</span>
                      <span className="text-[10px] text-ide-text-dim opacity-70">Show the editor focus shortcut chips in the toolbar.</span>
                    </div>
                    <ToggleSwitch checked={settings.toolbar.showFocusHints} onChange={(val) => updateToolbarSetting('showFocusHints', val)} />
                  </div>

                  <div className="flex items-center justify-between py-2 border-b border-ide-border">
                    <div className="flex flex-col">
                      <span className="text-[12px] text-ide-text">Settings Button</span>
                      <span className="text-[10px] text-ide-text-dim opacity-70">Keep a direct shortcut to preferences in the toolbar.</span>
                    </div>
                    <ToggleSwitch checked={settings.toolbar.showSettingsButton} onChange={(val) => updateToolbarSetting('showSettingsButton', val)} />
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

                   <div className="flex items-center justify-between py-2 border-b border-ide-border">
                     <div className="flex flex-col">
                       <span className="text-[12px] text-ide-text">Format on Save</span>
                       <span className="text-[10px] text-ide-text-dim opacity-70">Run the active formatter before every save.</span>
                     </div>
                     <ToggleSwitch checked={settings.editorFormatOnSave} onChange={(val) => updateSettings({ editorFormatOnSave: val })} />
                   </div>

                   <div className="flex items-center justify-between gap-4 py-2 border-b border-ide-border">
                     <div className="flex flex-col">
                       <span className="text-[12px] text-ide-text">UI Scale</span>
                       <span className="text-[10px] text-ide-text-dim opacity-70">Scale the entire workbench without changing your window size.</span>
                     </div>

                     <div className="flex items-center gap-3 w-56">
                       <input
                         type="range"
                         min="0.85"
                         max="1.35"
                         step="0.05"
                         value={settings.uiScale}
                         onChange={(e) => updateSettings({ uiScale: Number(e.target.value) })}
                         aria-label="User interface scale"
                         className="flex-1 accent-primary"
                       />
                       <span className="w-12 text-right text-[11px] font-semibold text-ide-text-dim">
                         {Math.round(settings.uiScale * 100)}%
                       </span>
                     </div>
                   </div>
                 </section>
               </div>
            )}

            {activeTab === 'keymap' && <KeybindsTab />}

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

            {activeTab === 'sanctuary' && (
              <div className="flex flex-col gap-8 animate-in fade-in duration-300">
                <section>
                  <div className="flex items-center gap-3 mb-4">
                    <div className="p-2 rounded-lg bg-primary/10 text-primary">
                      <Shield size={20} />
                    </div>
                    <div>
                      <h3 className="text-[14px] font-bold text-ide-text">History & Protection</h3>
                      <p className="text-[11px] text-ide-text-dim opacity-70">Configure local file versioning and database health.</p>
                    </div>
                  </div>
                  
                  <div className="flex flex-col gap-2 border-t border-ide-border pt-6">
                    <div className="flex items-center justify-between py-3 border-b border-ide-border/50">
                      <div className="flex flex-col">
                        <span className="text-[12px] text-ide-text">Retention Period</span>
                        <span className="text-[10px] text-ide-text-dim opacity-70">How many days should we keep local snapshots?</span>
                      </div>
                      <div className="flex items-center gap-3">
                        <input 
                          type="number" 
                          min="1"
                          max="365"
                          value={settings.historyRetentionDays}
                          onChange={(e) => updateSettings({ historyRetentionDays: Number(e.target.value) })}
                          className="w-20 bg-ide-panel border border-ide-border rounded-lg px-2 py-1 text-[11px] focus:outline-none focus:border-primary/50 text-center"
                        />
                        <span className="text-[11px] text-ide-text-dim w-10 text-center">days</span>
                      </div>
                    </div>

                    <div className="flex items-center justify-between py-3 border-b border-ide-border/50">
                      <div className="flex flex-col">
                        <span className="text-[12px] text-ide-text">Max Snapshots per File</span>
                        <span className="text-[10px] text-ide-text-dim opacity-70">Limit total versions stored for a single path.</span>
                      </div>
                      <div className="flex items-center gap-3">
                        <input 
                          type="number" 
                          min="5"
                          max="500"
                          value={settings.historyMaxSnapshots}
                          onChange={(e) => updateSettings({ historyMaxSnapshots: Number(e.target.value) })}
                          className="w-20 bg-ide-panel border border-ide-border rounded-lg px-2 py-1 text-[11px] focus:outline-none focus:border-primary/50 text-center"
                        />
                        <span className="text-[11px] text-ide-text-dim w-10 text-center">qty</span>
                      </div>
                    </div>

                    <div className="mt-6 p-4 rounded-xl bg-orange-500/5 border border-orange-500/10">
                      <p className="text-[10px] text-orange-500/80 leading-relaxed italic">
                        <strong>Note:</strong> Auto-cleanup runs on every file save. Setting very high values might increase the database size over time. Default values (30 days / 50 snapshots) are recommended for most projects.
                      </p>
                    </div>
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
  const [open, setOpen] = useState(id === 'zenith');
  const profile = getRuntimeProfileForSettingsKey(id);
  const headerLabel = profile.settingsLabel;

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
                 placeholder={`e.g. ${profile.compilerPlaceholder}`}
               />
            </div>
            
            <div>
               <div className="flex items-center justify-between gap-3 mb-1">
                 <label className="text-[10px] font-bold text-ide-text-dim uppercase block">LSP Server Path</label>
                 <span className={`rounded-full border px-2 py-0.5 text-[9px] font-bold uppercase tracking-[0.18em] ${config.lspPath ? 'border-emerald-500/20 bg-emerald-500/10 text-emerald-600' : 'border-ide-border bg-ide-panel text-ide-text-dim'}`}>
                   {config.lspPath ? 'Configured' : 'Local semantic fallback'}
                 </span>
               </div>
               <input 
                 type="text" 
                 value={config.lspPath || ""}
                 onChange={(e) => onUpdate({ lspPath: e.target.value })}
                 className="w-full bg-ide-panel border border-ide-border rounded-lg px-3 py-2 text-[11px] font-mono text-ide-text focus:outline-none focus:border-ide-text-dim"
                 placeholder={profile.lspPlaceholder || "Language Server Path"}
               />
               <p className="mt-1 text-[10px] leading-4 text-ide-text-dim opacity-70">
                 Used by semantic autocomplete context now; the external JSON-RPC LSP bridge can attach to this binary next.
               </p>
            </div>

            <div>
               <label className="text-[10px] font-bold text-ide-text-dim uppercase mb-1 block">Default Build Task</label>
               <input 
                 type="text" 
                 value={config.buildCommand || ""}
                 onChange={(e) => onUpdate({ buildCommand: e.target.value })}
                 className="w-full bg-ide-panel border border-ide-border rounded-lg px-3 py-2 text-[11px] font-mono text-ide-text focus:outline-none focus:border-ide-text-dim"
                 placeholder={`e.g. ${profile.buildPlaceholder}`}
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

function KeybindsTab() {
   const commands = useCommandStore(s => s.commands);
   const { settings, updateSettings } = useWorkspaceStore();
   
   const [editingCmdId, setEditingCmdId] = useState<string | null>(null);

   const getShortcutForCommand = (id: string) => {
       for (const [combo, cmdId] of Object.entries(settings.keymap || {})) {
           if (cmdId === id) return combo;
       }
       return null;
   };

   let commandList = Object.values(commands).sort((a,b) => {
       const catA = a.category || "Z";
       const catB = b.category || "Z";
       if (catA < catB) return -1;
       if (catA > catB) return 1;
       return a.title.localeCompare(b.title);
   });

   const handleSave = (cmdId: string, newCombo: string) => {
        const newKeymap = { ...settings.keymap };

        if (newKeymap[newCombo]) {
            delete newKeymap[newCombo];
        }

        const oldCombo = getShortcutForCommand(cmdId);
        if (oldCombo && oldCombo !== newCombo) {
            delete newKeymap[oldCombo];
        }

        newKeymap[newCombo] = cmdId;
        
        updateSettings({ keymap: newKeymap });
        setEditingCmdId(null);
   };

   return (
       <div className="flex flex-col gap-6 h-full pb-8">
           <section>
              <label className="text-[11px] font-bold text-ide-text-dim uppercase tracking-widest block mb-1">
                 Keyboard Shortcuts
              </label>
              <p className="text-[11px] text-ide-text-dim opacity-70 mb-4">
                 Customize IDE behavior by mapping key chords to Actions.
              </p>
              
              <div className="border border-ide-border rounded-xl bg-ide-panel/30 overflow-hidden flex flex-col">
                  {commandList.map((cmd, i) => {
                      const isEditing = editingCmdId === cmd.id;
                      const shortcut = getShortcutForCommand(cmd.id);
                      const isLast = i === commandList.length - 1;

                      return (
                          <div key={cmd.id} className={`flex items-center justify-between px-4 py-3 hover:bg-ide-panel/50 transition-colors ${!isLast ? 'border-b border-ide-border/50' : ''}`}>
                              <div className="flex flex-col">
                                 <span className="text-[12px] font-bold text-ide-text">{cmd.title}</span>
                                 <span className="text-[10px] text-ide-text-dim">{cmd.category || 'System'} • {cmd.id}</span>
                              </div>
                              
                              <div className="flex items-center gap-4">
                                  {isEditing ? (
                                      <KeybindCapture 
                                         currentCombo={shortcut} 
                                         onSave={(combo) => handleSave(cmd.id, combo)}
                                         onCancel={() => setEditingCmdId(null)}
                                      />
                                  ) : (
                                      <>
                                         {shortcut ? (
                                            <span className="text-[11px] font-mono bg-ide-bg border border-ide-border px-2 py-0.5 rounded text-ide-text-dim">
                                                {formatShortcutVisual(shortcut)}
                                            </span>
                                         ) : (
                                            <span className="text-[11px] text-ide-text-dim italic opacity-50">unassigned</span>
                                         )}
                                         
                                         <button 
                                            onClick={() => setEditingCmdId(cmd.id)}
                                            className="p-1.5 text-ide-text-dim hover:bg-ide-bg hover:text-white rounded transition-colors border border-transparent hover:border-ide-border"
                                         >
                                             <Pencil size={12} />
                                         </button>
                                      </>
                                  )}
                              </div>
                          </div>
                      )
                  })}
              </div>
           </section>
       </div>
   );
}

function KeybindCapture({ currentCombo, onSave, onCancel }: { currentCombo: string | null, onSave: (combo: string) => void, onCancel: () => void }) {
    const [combo, setCombo] = useState<string>(currentCombo || "");

    useEffect(() => {
        const handleKeyDown = (e: KeyboardEvent) => {
            e.preventDefault();
            e.stopPropagation();

            if (e.key === 'Escape') {
                onCancel();
                return;
            }
            
            // Allow storing only if a regular key is pressed
            if (e.key === 'Enter') {
                if (combo) {
                    onSave(combo);
                } else {
                    onCancel();
                }
                return;
            }

            // Exclude lone modifiers from closing the chord
            if (e.key === 'Control' || e.key === 'Shift' || e.key === 'Alt' || e.key === 'Meta') {
                return;
            }

            const keys = [];
            if (e.ctrlKey) keys.push('ctrl');
            if (e.shiftKey) keys.push('shift');
            if (e.altKey) keys.push('alt');
            
            // We just grab the raw key lowercase
            let keyName = e.key.toLowerCase();
            // Edge cases like space
            if (keyName === ' ') keyName = 'space';
            if (keyName === 'arrowup') keyName = 'up';
            if (keyName === 'arrowdown') keyName = 'down';
            if (keyName === 'arrowleft') keyName = 'left';
            if (keyName === 'arrowright') keyName = 'right';

            keys.push(keyName);

            setCombo(keys.join('+'));
        };

        window.addEventListener('keydown', handleKeyDown, { capture: true });
        return () => window.removeEventListener('keydown', handleKeyDown, { capture: true });
    }, [combo, onSave, onCancel]);

    return (
        <div className="flex items-center gap-3">
            <span className="text-[12px] bg-primary text-white px-3 py-1 rounded font-mono shadow-[0_0_10px_rgba(var(--color-primary-rgb),0.5)] animate-pulse">
                {combo ? formatShortcutVisual(combo) : "Aguardando teclas..."}
            </span>
            <div className="flex flex-col text-[9px] text-ide-text-dim leading-tight">
               <span><kbd className="opacity-70">Enter</kbd> salvar</span>
               <span><kbd className="opacity-70">Esc</kbd> cancelar</span>
            </div>
        </div>
    );
}

function formatShortcutVisual(combo: string) {
    return combo.split('+').map(k => {
        if (k === 'ctrl') return 'Ctrl';
        if (k === 'shift') return '⇧';
        if (k === 'alt') return 'Alt';
        if (k === 'up') return '↑';
        if (k === 'down') return '↓';
        if (k === 'left') return '←';
        if (k === 'right') return '→';
        if (k === 'space') return 'Space';
        return k.length === 1 ? k.toUpperCase() : k;
    }).join(' + ');
}
