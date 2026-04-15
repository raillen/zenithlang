import { Maximize, Layers } from "lucide-react";
import { useWorkspaceStore } from "../../store/useWorkspaceStore";
import { ToggleSwitch } from "../ui/ToggleSwitch";

export function InterfaceModule() {
  const { settings, updateSettings } = useWorkspaceStore();

  const handleToggleToolbar = (key: keyof typeof settings.toolbar) => {
    void updateSettings({
      toolbar: {
        ...settings.toolbar,
        [key]: !settings.toolbar[key],
      },
    });
  };

  const handleUiScale = (scale: number) => {
    void updateSettings({ uiScale: scale });
  };

  return (
    <div className="space-y-12 animate-in fade-in slide-in-from-bottom-4 duration-1000">
      {/* Header Context */}
      <header className="space-y-4">
        <div className="flex items-center gap-3">
          <div className="h-2 w-8 bg-primary/30 rounded-full" />
          <div className="text-[10px] font-bold uppercase tracking-[0.4em] text-primary/60">Controle de Layout</div>
        </div>
        <h2 className="text-[32px] font-bold tracking-tight text-ide-text">Interface & Painéis</h2>
        <p className="text-[14px] leading-relaxed text-ide-text-dim/70 max-w-2xl">
          Personalize a densidade de informações e a escala visual da Taurus. Elimine o ruído para focar no que importa: seu código.
        </p>
      </header>

      {/* Toolbar Grid */}
      <section className="space-y-8">
        <div className="flex items-center gap-4">
          <div className="flex h-10 w-10 items-center justify-center rounded-xl bg-primary/10 text-primary">
            <Layers size={18} />
          </div>
          <div>
            <h3 className="text-[15px] font-bold text-ide-text">Módulos da Toolbar</h3>
            <p className="text-[11px] text-ide-text-dim/60">Ligue ou desligue elementos da barra superior.</p>
          </div>
        </div>

        <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-3">
          <ToolbarToggle
            label="Workspace selector"
            checked={settings.toolbar.showWorkspaceSelector}
            onChange={() => handleToggleToolbar("showWorkspaceSelector")}
          />
          <ToolbarToggle
            label="Git branch"
            checked={settings.toolbar.showGitBranch}
            onChange={() => handleToggleToolbar("showGitBranch")}
          />
          <ToolbarToggle
            label="Quick open"
            checked={settings.toolbar.showQuickOpen}
            onChange={() => handleToggleToolbar("showQuickOpen")}
          />
          <ToolbarToggle
            label="Diagnostics"
            checked={settings.toolbar.showDiagnostics}
            onChange={() => handleToggleToolbar("showDiagnostics")}
          />
          <ToolbarToggle
            label="Runtime target"
            checked={settings.toolbar.showRuntimeTarget}
            onChange={() => handleToggleToolbar("showRuntimeTarget")}
          />
          <ToolbarToggle
            label="Panel toggles"
            checked={settings.toolbar.showPanelToggles}
            onChange={() => handleToggleToolbar("showPanelToggles")}
          />
          <ToolbarToggle
            label="Layout controls"
            checked={settings.toolbar.showLayoutControls}
            onChange={() => handleToggleToolbar("showLayoutControls")}
          />
          <ToolbarToggle
            label="Focus hints"
            checked={settings.toolbar.showFocusHints}
            onChange={() => handleToggleToolbar("showFocusHints")}
          />
          <ToolbarToggle
            label="Settings button"
            checked={settings.toolbar.showSettingsButton}
            onChange={() => handleToggleToolbar("showSettingsButton")}
          />
        </div>
      </section>

      {/* Global Scale */}
      <section className="space-y-8">
        <div className="flex items-center gap-4">
          <div className="flex h-10 w-10 items-center justify-center rounded-xl bg-orange-500/10 text-orange-500">
            <Maximize size={18} />
          </div>
          <div>
            <h3 className="text-[15px] font-bold text-ide-text">Escala Global de UI</h3>
            <p className="text-[11px] text-ide-text-dim/60">Ajuste o zoom da aplicação preservando a nitidez.</p>
          </div>
        </div>

        <div className="group relative flex flex-col gap-6 rounded-2xl border border-ide-border bg-ide-panel/10 p-8 transition-all hover:bg-ide-panel/15">
          <div className="flex items-center justify-between mb-4">
            <span className="text-[11px] font-bold uppercase tracking-[0.2em] text-ide-text/40">Nível de Zoom</span>
            <div className="flex items-center gap-4">
               <span className="text-[24px] font-display font-bold text-primary">{Math.round(settings.uiScale * 100)}%</span>
               <button
                  type="button"
                  onClick={() => handleUiScale(1)}
                  className="px-4 py-1.5 rounded-full border border-ide-border bg-ide-bg text-[10px] font-bold uppercase tracking-widest text-ide-text-dim transition-all hover:border-primary/50 hover:text-primary"
               >
                  Reset
               </button>
            </div>
          </div>
          
          <div className="relative">
            <input
              type="range"
              min="0.5"
              max="2.0"
              step="0.05"
              value={settings.uiScale}
              onChange={(event) => handleUiScale(parseFloat(event.target.value))}
              className="h-1.5 w-full cursor-pointer appearance-none rounded-full bg-ide-border accent-primary"
            />
            <div className="mt-6 flex justify-between text-[10px] font-bold uppercase tracking-[0.3em] text-ide-text-dim/40">
               <span>Extremo Compacto (50%)</span>
               <span>Amplo (200%)</span>
            </div>
          </div>
        </div>
      </section>
    </div>
  );
}

function ToolbarToggle({
  label,
  checked,
  onChange,
}: {
  label: string;
  checked: boolean;
  onChange: () => void;
}) {
  return (
    <button 
      onClick={onChange}
      className={`group flex items-center justify-between px-5 py-4 rounded-xl border transition-all duration-300 text-left ${
        checked 
          ? "border-primary/30 bg-primary/5 shadow-lg shadow-primary/5" 
          : "border-ide-border bg-ide-panel/5 hover:border-ide-text-dim/30 hover:bg-ide-panel/10"
      }`}
    >
      <div className="flex flex-col gap-0.5">
         <span className={`text-[12px] font-bold transition-all ${checked ? "text-ide-text" : "text-ide-text-dim/70 group-hover:text-ide-text"}`}>
           {label}
         </span>
         {checked && <span className="text-[9px] font-bold uppercase tracking-widest text-primary/60">Ativo</span>}
      </div>
      <ToggleSwitch checked={checked} onChange={onChange} />
    </button>
  );
}

