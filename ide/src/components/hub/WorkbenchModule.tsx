import { Globe, History, Save, Check, Zap } from "lucide-react";
import { useWorkspaceStore } from "../../store/useWorkspaceStore";
import { ToggleSwitch } from "../ui/ToggleSwitch";

export function WorkbenchModule() {
  const { settings, updateSettings } = useWorkspaceStore();

  const handleLanguageChange = (lang: 'pt' | 'en' | 'es') => {
    updateSettings({ language: lang });
  };

  return (
    <div className="space-y-12 animate-in fade-in slide-in-from-bottom-4 duration-1000">
      {/* Header Context */}
      <header className="space-y-4">
        <div className="flex items-center gap-3">
          <div className="h-2 w-8 bg-primary/30 rounded-full" />
          <div className="text-[10px] font-bold uppercase tracking-[0.4em] text-primary/60">Configurações Globais</div>
        </div>
        <h2 className="text-[32px] font-bold tracking-tight text-ide-text">Estação Geral</h2>
        <p className="text-[14px] leading-relaxed text-ide-text-dim/70 max-w-2xl">
          Ajuste o comportamento fundamental da sua IDE. Estas configurações são aplicadas globalmente em todos os seus projetos Sovereign.
        </p>
      </header>

      {/* Idioma Select */}
      <section className="space-y-8">
        <div className="flex items-center gap-4">
          <div className="flex h-10 w-10 items-center justify-center rounded-xl bg-primary/10 text-primary">
            <Globe size={18} />
          </div>
          <div>
            <h3 className="text-[15px] font-bold text-ide-text">Idioma e Localização</h3>
            <p className="text-[11px] text-ide-text-dim/60">Define a linguagem da interface e diagnósticos do compilador.</p>
          </div>
        </div>

        <div className="grid grid-cols-1 sm:grid-cols-3 gap-4">
          <LangCard 
            label="Português" 
            sub="Brasil [PT-BR]"
            active={settings.language === 'pt'} 
            onClick={() => handleLanguageChange('pt')} 
          />
          <LangCard 
            label="English" 
            sub="United States [US]"
            active={settings.language === 'en'} 
            onClick={() => handleLanguageChange('en')} 
          />
          <LangCard 
            label="Español" 
            sub="España [ES]"
            active={settings.language === 'es'} 
            onClick={() => handleLanguageChange('es')} 
          />
        </div>
      </section>

      {/* Comportamento de Sessão */}
      <section className="space-y-8">
        <div className="flex items-center gap-4">
          <div className="flex h-10 w-10 items-center justify-center rounded-xl bg-orange-500/10 text-orange-500">
            <History size={18} />
          </div>
          <div>
            <h3 className="text-[15px] font-bold text-ide-text">Persistência e Sessão</h3>
            <p className="text-[11px] text-ide-text-dim/60">Controle a continuidade do trabalho entre reinicializações.</p>
          </div>
        </div>

        <div className="bg-ide-panel/10 border border-ide-border rounded-2xl overflow-hidden">
          <SettingRow 
            label="Restaurar Sessão Anterior" 
            description="Reabre todos os arquivos, painéis e contextos ativos na última vez que o Zenith foi fechado."
          >
            <ToggleSwitch 
              checked={settings.restoreSession} 
              onChange={(val) => updateSettings({ restoreSession: val })} 
            />
          </SettingRow>

          <SettingRow 
            label="Sincronização Cloud [Experimental]" 
            description="Mantém o estado do workbench sincronizado entre diferentes máquinas (requer conta Zenith Hub)."
          >
            <ToggleSwitch checked={false} onChange={() => {}} />
          </SettingRow>
        </div>
      </section>

      {/* Auto-Save Strategy */}
      <section className="space-y-8">
        <div className="flex items-center gap-4">
          <div className="flex h-10 w-10 items-center justify-center rounded-xl bg-emerald-500/10 text-emerald-500">
            <Save size={18} />
          </div>
          <div>
            <h3 className="text-[15px] font-bold text-ide-text">Estratégia de Auto-Save</h3>
            <p className="text-[11px] text-ide-text-dim/60">Garante a integridade do seu código em tempo real.</p>
          </div>
        </div>

        <div className="grid grid-cols-1 sm:grid-cols-3 gap-4">
          <StrategyCard 
            label="Manual" 
            desc="Control+S"
            active={settings.autoSave === 'off'} 
            onClick={() => updateSettings({ autoSave: 'off' })} 
          />
          <StrategyCard 
            label="Foco" 
            desc="Ao desfocar janela"
            active={settings.autoSave === 'onFocusChange'} 
            onClick={() => updateSettings({ autoSave: 'onFocusChange' })} 
          />
          <StrategyCard 
            label="Atrasado" 
            desc="Após 1000ms"
            active={settings.autoSave === 'afterDelay'} 
            onClick={() => updateSettings({ autoSave: 'afterDelay' })} 
          />
        </div>
      </section>

      {/* Beta Features */}
      <div className="p-6 rounded-2xl border border-primary/20 bg-primary/5 flex items-center justify-between gap-6 group">
         <div className="flex items-center gap-5">
            <div className="h-10 w-10 flex items-center justify-center rounded-xl bg-primary text-white shadow-lg shadow-primary/20">
               <Zap size={20} />
            </div>
            <div>
               <div className="text-[13px] font-bold text-ide-text">Laboratório de Performance</div>
               <p className="text-[11px] text-ide-text-dim/70">Ative otimizações experimentais para máquinas de baixa latência.</p>
            </div>
         </div>
         <button className="px-5 py-2 rounded-lg border border-primary/30 text-[10px] font-bold uppercase tracking-widest text-primary hover:bg-primary hover:text-white transition-all duration-300">
            Configurar
         </button>
      </div>
    </div>
  );
}

function LangCard({ label, sub, active, onClick }: { label: string, sub: string, active: boolean, onClick: () => void }) {
  return (
    <button 
      onClick={onClick}
      className={`group flex flex-col items-start p-5 rounded-2xl border transition-all duration-300 text-left ${
        active 
          ? 'border-primary/40 bg-primary/5 shadow-lg shadow-primary/5' 
          : 'border-ide-border bg-ide-panel/5 hover:border-ide-text-dim/30 hover:bg-ide-panel/10'
      }`}
    >
      <div className="flex items-center justify-between w-full mb-3">
        <span className={`text-[13px] font-bold transition-colors ${active ? 'text-primary' : 'text-ide-text'}`}>{label}</span>
        {active && (
           <div className="h-5 w-5 rounded-full bg-primary flex items-center justify-center">
              <Check size={12} className="text-white" />
           </div>
        )}
      </div>
      <span className="text-[10px] text-ide-text-dim/60 font-mono uppercase tracking-widest group-hover:text-ide-text-dim transition-colors">{sub}</span>
    </button>
  );
}

function StrategyCard({ label, desc, active, onClick }: { label: string, desc: string, active: boolean, onClick: () => void }) {
  return (
    <button 
      onClick={onClick}
      className={`relative overflow-hidden flex flex-col p-5 rounded-2xl border transition-all duration-300 text-left ${
        active 
          ? 'border-emerald-500/40 bg-emerald-500/5 shadow-lg shadow-emerald-500/5' 
          : 'border-ide-border bg-ide-panel/5 hover:border-ide-text-dim/30 hover:bg-ide-panel/10'
      }`}
    >
      <div className="flex items-center justify-between w-full mb-2">
         <span className={`text-[13px] font-bold transition-colors ${active ? 'text-emerald-500' : 'text-ide-text'}`}>{label}</span>
         {active && <div className="h-1.5 w-1.5 rounded-full bg-emerald-500 animate-pulse" />}
      </div>
      <span className="text-[10px] text-ide-text-dim/60 font-bold uppercase tracking-[0.2em]">{desc}</span>
    </button>
  );
}

function SettingRow({ label, description, children }: { label: string, description: string, children: React.ReactNode }) {
  return (
    <div className="flex items-center justify-between px-6 py-5 border-b border-ide-border/40 last:border-0 hover:bg-ide-panel/5 transition-all">
      <div className="flex flex-col gap-1.5 max-w-[75%]">
        <span className="text-[14px] font-bold text-ide-text">{label}</span>
        <span className="text-[12px] text-ide-text-dim/70 leading-relaxed">{description}</span>
      </div>
      <div className="flex shrink-0 items-center justify-center">
        {children}
      </div>
    </div>
  );
}

