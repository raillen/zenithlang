import { useState } from "react";
import { createPortal } from "react-dom";
import { X, Globe, Cpu, Palette, Code, Check } from "lucide-react";
import { useWorkspaceStore } from "../store/useWorkspaceStore";
import { useTranslation } from "../utils/i18n";

interface SettingsDialogProps {
  onClose: () => void;
}

export function SettingsDialog({ onClose }: SettingsDialogProps) {
  const [activeTab, setActiveTab] = useState<'general' | 'editor' | 'zenith'>('general');
  const { settings, updateSettings } = useWorkspaceStore();
  const { t } = useTranslation();

  const handleLanguageChange = (lang: 'pt' | 'en' | 'es') => {
    updateSettings({ language: lang });
  };

  const modalContent = (
    <div className="fixed inset-0 z-[9999] flex items-center justify-center bg-black/20 backdrop-blur-sm animate-in fade-in duration-200">
      <div className="w-[600px] h-[450px] bg-white/95 backdrop-blur-2xl border border-ide-border rounded-2xl shadow-2xl flex overflow-hidden animate-in zoom-in-95 duration-300">
        
        {/* Sidebar */}
        <div className="w-48 bg-black/[0.02] border-r border-ide-border p-4 flex flex-col gap-1">
          <h2 className="text-[14px] font-bold text-zinc-800 mb-4 px-2">{t('settings.title')}</h2>
          
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
            active={activeTab === 'zenith'} 
            onClick={() => setActiveTab('zenith')} 
            icon={<Cpu size={14} />} 
            label={t('settings.tabs.zenith')} 
          />
        </div>

        {/* Content */}
        <div className="flex-1 flex flex-col min-w-0">
          <div className="h-12 flex items-center justify-end px-4 border-b border-black/[0.02]">
            <button onClick={onClose} className="p-1.5 hover:bg-black/5 rounded-full text-zinc-400 transition-colors">
              <X size={18} />
            </button>
          </div>

          <div className="p-8 overflow-y-auto flex-1">
            {activeTab === 'general' && (
              <div className="flex flex-col gap-6">
                <section>
                  <label className="text-[11px] font-bold text-zinc-400 uppercase tracking-widest block mb-4">
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

                <section>
                  <label className="text-[11px] font-bold text-zinc-400 uppercase tracking-widest block mb-4">
                    {t('settings.labels.theme')}
                  </label>
                  <div className="flex gap-2">
                    <button 
                      onClick={() => updateSettings({ theme: 'light' })}
                      className={`flex-1 p-3 rounded-xl border text-[11px] font-medium transition-all ${settings.theme === 'light' ? 'border-primary bg-primary/5 text-primary' : 'border-ide-border hover:border-zinc-300 text-zinc-600'}`}
                    >
                      Standard Light
                    </button>
                    <button 
                      onClick={() => updateSettings({ theme: 'dark' })}
                      className={`flex-1 p-3 rounded-xl border text-[11px] font-medium transition-all ${settings.theme === 'dark' ? 'border-primary bg-primary/5 text-primary' : 'border-ide-border hover:border-zinc-300 text-zinc-600'}`}
                    >
                      Modern Dark
                    </button>
                  </div>
                </section>
              </div>
            )}

            {activeTab === 'editor' && (
               <div className="flex flex-col gap-6">
                  <div className="p-12 border-2 border-dashed border-zinc-100 rounded-2xl flex flex-col items-center justify-center text-center">
                      <Code size={32} className="text-zinc-200 mb-4" />
                      <p className="text-[12px] text-zinc-400">Editor fonts and styling are currently locked to Retina defaults.</p>
                  </div>
               </div>
            )}

            {activeTab === 'zenith' && (
              <div className="flex flex-col gap-6">
                <section>
                    <label className="text-[11px] font-bold text-zinc-400 uppercase tracking-widest block mb-2">
                      {t('settings.labels.compiler_path')}
                    </label>
                    <input 
                      type="text" 
                      value={settings.compilerPath}
                      onChange={(e) => updateSettings({ compilerPath: e.target.value })}
                      className="w-full bg-black/[0.03] border border-ide-border rounded-lg px-3 py-2 text-[12px] font-mono text-zinc-600 focus:outline-none focus:ring-1 focus:ring-primary/30"
                      placeholder="e.g. ztc.lua"
                    />
                    <p className="mt-2 text-[10px] text-zinc-400 italic">Relative to your project root or absolute path.</p>
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
      className={`flex items-center gap-3 px-3 py-2 rounded-lg text-[11px] transition-all ${active ? 'bg-primary text-white shadow-md font-medium' : 'text-zinc-500 hover:bg-black/5 hover:text-zinc-800'}`}
    >
      {icon}
      {label}
    </button>
  );
}

function LangOption({ label, active, onClick }: { label: string, active: boolean, onClick: () => void }) {
  return (
    <button 
      onClick={onClick}
      className={`flex items-center justify-between px-4 py-3 rounded-xl border transition-all ${active ? 'border-primary bg-primary/5 text-primary' : 'border-ide-border hover:border-zinc-300 text-zinc-600'}`}
    >
      <span className="text-[12px] font-medium">{label}</span>
      {active && <Check size={14} />}
    </button>
  );
}
