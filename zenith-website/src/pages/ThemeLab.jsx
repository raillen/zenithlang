import React, { useState, useRef } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { 
  Palette, Download, Upload, Eye, Code, Layout, 
  Trash, Sparkle, MagicWand, Info, Check, X,
  Monitor, Browser, DeviceMobile
} from '@phosphor-icons/react';
import { useLanguage } from '../contexts/LanguageContext';
import IdePreview from '../components/ThemeLab/IdePreview';
import { DEFAULT_THEME_DARK, DEFAULT_THEME_LIGHT, harmonizeTheme, downloadTheme, buildFullTheme } from '../utils/themeUtils';

const ThemeLab = () => {
  const { lang } = useLanguage();
  const [theme, setTheme] = useState(DEFAULT_THEME_DARK);
  const [activeTab, setActiveTab] = useState('interface');
  const [previewMode, setPreviewMode] = useState('editor'); // 'editor' or 'hub'
  const [showJson, setShowJson] = useState(false);
  const fileInputRef = useRef(null);

  const updateUi = (key, value) => {
    setTheme(prev => ({
      ...prev,
      ui: { ...prev.ui, [key]: value }
    }));
  };

  const updateSyntax = (key, value) => {
    setTheme(prev => ({
      ...prev,
      syntax: { ...prev.syntax, [key]: value }
    }));
  };

  const handleImport = (e) => {
    const file = e.target.files[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = (event) => {
      try {
        const imported = JSON.parse(event.target.result);
        // Simple mapping from full theme to quick theme for web lab
        setTheme({
          ...DEFAULT_THEME_DARK,
          id: imported.id || theme.id,
          name: imported.name || theme.name,
          ui: { ...DEFAULT_THEME_DARK.ui, ...imported.ui },
          syntax: { ...DEFAULT_THEME_DARK.syntax, ...imported.syntax }
        });
      } catch (err) {
        alert("Erro ao importar tema: Formato inválido.");
      }
    };
    reader.readAsText(file);
  };

  return (
    <div className="page-theme-lab min-h-screen bg-[#F8F9FA] py-12 px-6">
      <div className="max-w-[1700px] mx-auto">
        
        {/* SOVEREIGN HEADER */}
        <header className="flex flex-col lg:flex-row justify-between items-start lg:items-end gap-8 mb-12 border-b border-black/[0.03] pb-12">
          <div className="space-y-4">
            <div className="flex items-center gap-3">
              <div className="p-2 rounded-2xl bg-white shadow-xl shadow-black/5 border border-black/[0.03]">
                <img src="/logo-only.svg" alt="Zenith" className="w-8 h-8 object-contain" />
              </div>
              <div className="flex flex-col">
                <span className="text-[10px] font-black uppercase tracking-[0.3em] text-neutral/30">Theme Engine v0.3.0</span>
                <div className="flex items-center gap-2">
                   <h1 className="text-4xl font-bold tracking-tight text-neutral">Theme Lab Studio</h1>
                   <div className="px-2 py-0.5 rounded-md bg-emerald-500/10 text-emerald-600 text-[10px] font-black uppercase tracking-widest border border-emerald-500/20">Active</div>
                </div>
              </div>
            </div>
            <p className="text-neutral/50 font-medium text-lg max-w-[60ch] leading-relaxed">
                Projete identidades visuais soberanas para a Zenith IDE. Ajuste tokens, visualize em tempo real e exporte para sua estação de trabalho.
            </p>
          </div>

          <div className="flex items-center gap-4">
             <button 
                onClick={() => fileInputRef.current.click()}
                className="group flex items-center gap-2 px-6 py-3 rounded-2xl border border-black/5 bg-white text-sm font-bold text-neutral hover:bg-black hover:text-white transition-all shadow-sm"
             >
                <Upload weight="bold" className="group-hover:scale-110 transition-transform" /> 
                Importar JSON
             </button>
             <button 
                onClick={() => downloadTheme(theme)}
                className="group flex items-center gap-3 px-8 py-3 rounded-2xl bg-primary text-white text-sm font-bold hover:bg-primary-hover transition-all shadow-xl shadow-primary/20"
             >
                <Download weight="bold" className="group-hover:translate-y-0.5 transition-transform" /> 
                Exportar para IDE
             </button>
             <input type="file" ref={fileInputRef} className="hidden" accept=".json" onChange={handleImport} />
          </div>
        </header>

        <div className="grid grid-cols-1 xl:grid-cols-12 gap-10">
            
            {/* INSPECTOR PANEL */}
            <aside className="xl:col-span-3 space-y-6">
                <div className="p-6 rounded-2xl bg-white border border-black/[0.03] shadow-xl shadow-black/[0.02]">
                    <div className="flex items-center justify-between mb-8">
                       <h3 className="text-[10px] font-black uppercase tracking-[0.2em] text-neutral/30">Quick Presets</h3>
                       <div className="flex gap-2">
                          <button 
                             onClick={() => setTheme(DEFAULT_THEME_DARK)}
                             className="px-3 py-1.5 rounded-lg bg-neutral-900 text-white text-[9px] font-bold uppercase transition-all hover:scale-105"
                          >
                             Dark Base
                          </button>
                          <button 
                             onClick={() => setTheme(DEFAULT_THEME_LIGHT)}
                             className="px-3 py-1.5 rounded-lg bg-neutral-100 border text-neutral text-[9px] font-bold uppercase transition-all hover:scale-105"
                          >
                             Light Base
                          </button>
                       </div>
                    </div>

                    <div className="flex p-1 bg-neutral-100 rounded-xl mb-8">
                       <button 
                          onClick={() => setActiveTab('interface')}
                          className={`flex-1 flex items-center justify-center gap-2 py-2.5 rounded-xl text-xs font-black uppercase tracking-widest transition-all ${activeTab === 'interface' ? 'bg-white text-neutral shadow-sm font-bold' : 'text-neutral/30 hover:text-neutral/60'}`}
                       >
                          UI & Chrome
                       </button>
                       <button 
                          onClick={() => setActiveTab('syntax')}
                          className={`flex-1 flex items-center justify-center gap-2 py-2.5 rounded-xl text-xs font-black uppercase tracking-widest transition-all ${activeTab === 'syntax' ? 'bg-white text-neutral shadow-sm font-bold' : 'text-neutral/30 hover:text-neutral/60'}`}
                       >
                          Syntax Tokens
                       </button>
                    </div>

                    <div className="space-y-10">
                        {activeTab === 'interface' ? (
                          <div className="space-y-8 animate-in fade-in slide-in-from-left-4 duration-500">
                             <TokenSection title="Ambiente Base">
                                <ColorPicker label="Background" value={theme.ui.background} onChange={v => updateUi('background', v)} />
                                <ColorPicker label="Painéis (Glass)" value={theme.ui.panel} onChange={v => updateUi('panel', v)} />
                                <ColorPicker label="Bordas" value={theme.ui.border} onChange={v => updateUi('border', v)} />
                             </TokenSection>
                             <TokenSection title="Identidade">
                                <ColorPicker label="Core Primary" value={theme.ui.primary} onChange={v => updateUi('primary', v)} />
                                <ColorPicker label="Texto Principal" value={theme.ui.textMain} onChange={v => updateUi('textMain', v)} />
                                <ColorPicker label="Texto Mudo" value={theme.ui.textMuted} onChange={v => updateUi('textMuted', v)} />
                             </TokenSection>
                          </div>
                        ) : (
                          <div className="space-y-8 animate-in fade-in slide-in-from-right-4 duration-500">
                             <TokenSection title="Código & Semântica">
                                <ColorPicker label="Keywords" value={theme.syntax.keyword} onChange={v => updateSyntax('keyword', v)} />
                                <ColorPicker label="Strings" value={theme.syntax.string} onChange={v => updateSyntax('string', v)} />
                                <ColorPicker label="Functions" value={theme.syntax.function} onChange={v => updateSyntax('function', v)} />
                                <ColorPicker label="Numbers" value={theme.syntax.number} onChange={v => updateSyntax('number', v)} />
                                <ColorPicker label="Types" value={theme.syntax.type} onChange={v => updateSyntax('type', v)} />
                                <ColorPicker label="Comments" value={theme.syntax.comment} onChange={v => updateSyntax('comment', v)} />
                             </TokenSection>
                          </div>
                        )}
                    </div>
                </div>

                <div className="p-6 rounded-2xl bg-neutral-900 text-white shadow-2xl relative overflow-hidden group">
                    <div className="absolute top-0 right-0 p-6 opacity-10 group-hover:rotate-12 transition-transform">
                       <Sparkle size={100} weight="fill" />
                    </div>
                    <div className="relative z-10">
                       <MagicWand weight="bold" size={24} className="text-primary mb-4" />
                       <h3 className="text-lg font-bold">Assistente Soberano</h3>
                       <p className="text-[11px] text-white/50 mt-1 leading-relaxed max-w-[28ch]">
                          Gere uma paleta harmônica baseada na sua cor primária.
                       </p>
                       <button 
                          onClick={() => setTheme(prev => ({ ...prev, ...harmonizeTheme(prev.ui.primary, prev.type) }))}
                          className="mt-6 px-4 py-2 rounded-lg bg-white/10 border border-white/10 text-[9px] font-black uppercase tracking-widest hover:bg-white/20 transition-all"
                       >
                          Auto-Harmonize
                       </button>
                    </div>
                </div>
            </aside>

            {/* PREVIEW STAGE */}
            <main className="xl:col-span-9 space-y-6">
                <div className="flex items-center justify-between">
                   <div className="flex bg-white rounded-2xl border border-black/[0.03] p-1.5 shadow-sm">
                       <button 
                           onClick={() => setPreviewMode('editor')}
                           className={`flex items-center gap-2 px-6 py-2 rounded-xl text-[10px] font-black uppercase tracking-widest transition-all ${previewMode === 'editor' ? 'bg-neutral text-white shadow-lg shadow-black/10' : 'text-neutral/30 hover:text-neutral/60'}`}
                       >
                           <Browser weight="bold" /> Editor View
                       </button>
                       <button 
                           onClick={() => setPreviewMode('hub')}
                           className={`flex items-center gap-2 px-6 py-2 rounded-xl text-[10px] font-black uppercase tracking-widest transition-all ${previewMode === 'hub' ? 'bg-neutral text-white shadow-lg shadow-black/10' : 'text-neutral/30 hover:text-neutral/60'}`}
                       >
                           <Monitor weight="bold" /> Zenith Hub Mode
                       </button>
                   </div>

                   <button 
                      onClick={() => setShowJson(!showJson)}
                      className={`flex items-center gap-2 px-4 py-2 rounded-xl text-[10px] font-black uppercase tracking-[0.1em] transition-all border ${showJson ? 'border-primary/40 text-primary bg-primary/5' : 'border-black/5 text-neutral/40 hover:text-neutral'}`}
                   >
                      <Code weight="bold" /> {showJson ? "Fechar JSON" : "Ver JSON Output"}
                   </button>
                </div>

                <div className="relative aspect-video xl:aspect-auto xl:h-[750px] group">
                   {/* DYNAMIC IDE MOCKUP */}
                   <IdePreview theme={theme} showHub={previewMode === 'hub'} />
                   
                   <AnimatePresence>
                     {showJson && (
                        <motion.div 
                           initial={{ x: '100%', opacity: 0 }}
                           animate={{ x: 0, opacity: 1 }}
                           exit={{ x: '100%', opacity: 0 }}
                           className="absolute top-4 right-4 bottom-4 w-96 rounded-2xl bg-neutral-900/95 backdrop-blur-xl border border-white/10 p-8 shadow-2xl z-[60] overflow-hidden flex flex-col"
                        >
                           <div className="flex items-center justify-between mb-6">
                              <h3 className="text-xs font-black uppercase tracking-widest text-white/40">JSON Schema</h3>
                              <X className="text-white/20 cursor-pointer hover:text-white" onClick={() => setShowJson(false)} />
                           </div>
                           <div className="flex-1 overflow-y-auto font-mono text-[11px] text-emerald-400 no-scrollbar">
                              <pre className="whitespace-pre-wrap">
                                 {JSON.stringify(buildFullTheme(theme), null, 2)}
                              </pre>
                           </div>
                        </motion.div>
                     )}
                   </AnimatePresence>
                </div>

                <div className="flex items-center gap-4 px-2">
                   <div className="flex items-center gap-2 text-[10px] font-bold text-neutral/40 uppercase tracking-widest">
                      <Info size={14} />
                      <span>Preview 1:1 com a Engine Sovereign v0.3</span>
                   </div>
                   <div className="h-px flex-1 bg-black/[0.03]" />
                </div>
            </main>
        </div>
      </div>
    </div>
  );
};

// Internal Components
const TokenSection = ({ title, children }) => (
  <div className="space-y-4">
    <h3 className="text-[10px] font-black uppercase tracking-[.2em] text-neutral/30 px-2">{title}</h3>
    <div className="grid gap-2">
      {children}
    </div>
  </div>
);

const ColorPicker = ({ label, value, onChange }) => {
  const isHex = value.startsWith('#');
  const colorInputRef = useRef(null);
  
  return (
    <div className="group flex items-center justify-between p-3 rounded-xl border border-transparent hover:border-black/[0.03] hover:bg-neutral-50 transition-all">
      <div className="flex flex-col flex-1">
        <span className="text-[11px] font-bold text-neutral group-hover:text-primary transition-colors">{label}</span>
        <input 
          type="text" 
          value={value} 
          onChange={(e) => onChange(e.target.value)}
          className="text-[9px] font-mono text-neutral/40 uppercase tracking-tight bg-transparent border-none p-0 focus:ring-0 focus:text-neutral focus:font-bold transition-all w-24"
        />
      </div>
      <div className="relative flex items-center shrink-0">
        <input 
          ref={colorInputRef}
          type="color" 
          value={isHex ? value : '#000000'}
          onChange={(e) => onChange(e.target.value)}
          className="opacity-0 absolute pointer-events-none"
          style={{ width: 0, height: 0 }}
        />
        <div 
          onClick={() => colorInputRef.current?.click()}
          className="w-8 h-8 rounded-lg border border-black/10 shadow-sm transition-transform cursor-pointer group-hover:scale-110 active:scale-95" 
          style={{ backgroundColor: value }} 
        />
      </div>
    </div>
  );
};

export default ThemeLab;
