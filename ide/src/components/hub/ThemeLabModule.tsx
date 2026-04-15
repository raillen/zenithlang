import { useState, useRef } from "react";
import { 
  ArrowUpRight, Sparkles, Layers3, Wand2, FlaskConical, Palette, 
  Share2, Zap, FileJson, Download, Upload, Edit3, Check, X
} from "lucide-react";
import { invoke } from "../../utils/tauri";
import { HubSectionProps } from "./HubTypes";
import { useWorkspaceStore } from "../../store/useWorkspaceStore";
import { serializeTheme, parseThemeJson } from "../../themes";

export function ThemeLabModule({ activeSection, onClose }: HubSectionProps) {
  const { settings, addCustomTheme, updateSettings } = useWorkspaceStore();
  const [isRenaming, setIsRenaming] = useState(false);
  const [newName, setNewName] = useState("");
  const fileInputRef = useRef<HTMLInputElement>(null);

  // Identify active theme object
  const activeThemeId = settings.theme;
  const activeThemeName = activeThemeId.includes('custom') 
    ? (settings.customThemes[activeThemeId]?.name || activeThemeId) 
    : activeThemeId;

  const launchThemeLab = async () => {
    await invoke("open_theme_lab");
    onClose();
  };

  const handleExport = () => {
    const currentTheme = settings.customThemes[activeThemeId];
    if (!currentTheme) {
      alert("Por favor, selecione um tema customizado para exportar.");
      return;
    }
    const data = serializeTheme(currentTheme);
    const blob = new Blob([data], { type: "application/json" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = `${currentTheme.id}.json`;
    link.click();
    URL.revokeObjectURL(url);
  };

  const handleImportClick = () => {
    fileInputRef.current?.click();
  };

  const handleFileChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = (event) => {
      try {
        const json = event.target?.result as string;
        const theme = parseThemeJson(json);
        addCustomTheme(theme);
        updateSettings({ theme: theme.id });
      } catch (err) {
        console.error("Failed to parse theme:", err);
        alert("Erro ao importar tema. Verifique se o arquivo JSON é válido.");
      }
    };
    reader.readAsText(file);
  };

  const handleRename = () => {
    const currentTheme = settings.customThemes[activeThemeId];
    if (!currentTheme || !newName.trim()) return;

    const updatedTheme = { ...currentTheme, name: newName.trim() };
    addCustomTheme(updatedTheme);
    setIsRenaming(false);
  };

  return (
    <div className="space-y-10 animate-in fade-in slide-in-from-bottom-4 duration-1000">
      {/* Sovereign Utility Bar */}
      <div className="flex flex-col md:flex-row md:items-center justify-between gap-4 p-6 rounded-2xl border border-ide-border bg-ide-panel/10">
        <div className="flex items-center gap-5">
          <div className="h-12 w-12 rounded-xl bg-primary/10 flex items-center justify-center text-primary shadow-inner">
             <Palette size={24} />
          </div>
          <div className="space-y-1">
             <div className="text-[10px] font-bold uppercase tracking-[0.2em] text-primary/60">Tema Ativo</div>
             {isRenaming ? (
               <div className="flex items-center gap-2">
                 <input 
                   type="text" 
                   value={newName}
                   autoFocus
                   onChange={(e) => setNewName(e.target.value)}
                   className="bg-ide-bg border border-primary/40 rounded-lg px-2 py-1 text-[16px] font-bold text-ide-text focus:outline-none focus:ring-4 focus:ring-primary/5"
                 />
                 <button onClick={handleRename} className="p-1.5 rounded-lg bg-emerald-500/20 text-emerald-500 hover:bg-emerald-500/30">
                   <Check size={14} />
                 </button>
                 <button onClick={() => setIsRenaming(false)} className="p-1.5 rounded-lg bg-red-500/20 text-red-500 hover:bg-red-500/30">
                   <X size={14} />
                 </button>
               </div>
             ) : (
               <div className="flex items-center gap-3">
                 <h2 className="text-[20px] font-bold text-ide-text tracking-tight">{activeThemeName}</h2>
                 <button 
                   onClick={() => { setNewName(activeThemeName); setIsRenaming(true); }}
                   className="p-1 text-ide-text-dim/40 hover:text-primary transition-colors"
                 >
                   <Edit3 size={14} />
                 </button>
               </div>
             )}
          </div>
        </div>

        <div className="flex items-center gap-2">
           <button 
             onClick={handleExport}
             className="flex items-center gap-2 px-4 py-2.5 rounded-xl border border-ide-border bg-ide-bg text-[11px] font-bold uppercase tracking-widest text-ide-text-dim hover:text-primary hover:border-primary/30 transition-all active:scale-95"
           >
              <Download size={14} />
              Exportar
           </button>
           <button 
             onClick={handleImportClick}
             className="flex items-center gap-2 px-4 py-2.5 rounded-xl border border-ide-border bg-ide-bg text-[11px] font-bold uppercase tracking-widest text-ide-text-dim hover:text-primary hover:border-primary/30 transition-all active:scale-95"
           >
              <Upload size={14} />
              Importar
           </button>
           <input 
             type="file" 
             ref={fileInputRef} 
             onChange={handleFileChange} 
             accept=".json" 
             className="hidden" 
           />
           <button 
             onClick={launchThemeLab}
             className="flex items-center gap-2 px-5 py-2.5 rounded-xl bg-primary shadow-lg shadow-primary/20 text-[11px] font-bold uppercase tracking-widest text-white hover:scale-[1.02] active:scale-[0.98] transition-all"
           >
              <Wand2 size={14} />
              Abrir Lab
           </button>
        </div>
      </div>

      <div className="grid gap-8 lg:grid-cols-[1fr_360px]">
        <div className="space-y-10">
          {/* Immersive Hero Section */}
          <section className="relative overflow-hidden rounded-2xl border border-ide-border bg-gradient-to-br from-primary/5 via-transparent to-transparent p-10 lg:p-12">
            <div className="relative z-10 space-y-8">
              <div className="inline-flex items-center gap-3 rounded-full border border-primary/20 bg-primary/5 px-4 py-1.5 text-[9px] font-bold uppercase tracking-[0.25em] text-primary">
                <FlaskConical size={14} className="animate-pulse" />
                Sovereign Design Lab
              </div>

              <h1 className="font-display text-[36px] font-bold tracking-tight text-white lg:text-[44px] leading-tight">
                A alquimia da sua <br />
                <span className="text-primary italic">Interface Ideal</span>.
              </h1>

              <p className="text-[16px] leading-relaxed text-ide-text-dim/80 max-w-xl">
                {activeSection.summary} O Theme Lab permite que você sculpe cada átomo visual do seu ambiente de trabalho.
              </p>

              <div className="flex flex-wrap gap-2.5">
                {activeSection.highlights.map((highlight) => (
                  <div
                    key={highlight.label}
                    className="group flex items-center gap-3 rounded-xl border border-ide-border bg-ide-bg/40 px-4 py-2 transition-all hover:border-primary/30"
                  >
                    <div className="text-[12px] font-mono font-bold text-primary">{highlight.value}</div>
                    <div className="h-3 w-px bg-ide-border/60" />
                    <div className="text-[9px] font-bold uppercase tracking-[0.1em] text-ide-text-dim/60">
                      {highlight.label}
                    </div>
                  </div>
                ))}
              </div>
            </div>
          </section>

          {/* Feature Grid */}
          <section className="grid gap-4 md:grid-cols-3">
            {[
              {
                title: "Atmosfera Zenith",
                body: "Defina tons base antes de entrar em detalhes finos.",
                icon: <Sparkles size={16} />,
              },
              {
                title: "Arquitetura Visual",
                body: "Refine editor, chrome e terminal de forma coerente.",
                icon: <Layers3 size={16} />,
              },
              {
                title: "JSON Workflow",
                body: "Importe e exporte temas para backup ou portabilidade.",
                icon: <FileJson size={16} />,
              },
            ].map((tip) => (
              <div
                key={tip.title}
                className="group p-6 rounded-2xl border border-ide-border bg-ide-panel/5 transition-all hover:bg-ide-panel/10"
              >
                <div className="flex h-10 w-10 items-center justify-center rounded-xl bg-ide-bg border border-ide-border text-primary transition-all group-hover:scale-110">
                  {tip.icon}
                </div>
                <h3 className="mt-5 text-[15px] font-bold text-ide-text tracking-tight">{tip.title}</h3>
                <p className="mt-2 text-[13px] leading-relaxed text-ide-text-dim/60">{tip.body}</p>
              </div>
            ))}
          </section>
        </div>

        {/* Sidebar Context */}
        <aside className="space-y-8">
           <div className="rounded-2xl border border-ide-border bg-ide-bg p-8 space-y-6">
              <div className="flex items-center gap-3">
                <Zap size={18} className="text-primary" />
                <h3 className="text-[16px] font-bold text-ide-text">Consistência</h3>
              </div>
              <p className="text-[13px] leading-relaxed text-ide-text-dim/70">
                O Theme Lab valida automaticamente as relações de contraste entre o editor e o workbench original.
              </p>
              <ul className="space-y-4 pt-2">
                {[
                  "Modo síncrono de cores",
                  "Syntax Zenith nativa",
                  "Profundidade adaptativa"
                ].map((item) => (
                  <li key={item} className="flex items-center gap-3 text-[12px] text-ide-text-dim/80">
                     <div className="h-1 w-1 rounded-full bg-primary" />
                     {item}
                  </li>
                ))}
              </ul>
           </div>

           <div className="rounded-2xl border border-ide-border bg-gradient-to-br from-ide-panel/10 to-transparent p-8 space-y-5">
              <div className="text-[10px] font-bold uppercase tracking-[0.3em] text-primary">Comunidade</div>
              <h3 className="text-[18px] font-bold text-ide-text leading-tight">Distribua seu trabalho.</h3>
              <p className="text-[13px] leading-relaxed text-ide-text-dim/70">
                Seus temas são pacotes `.json` compactos. Compartilhe no Marketplace ou GitHub.
              </p>
              <button className="flex items-center gap-2 text-[11px] font-bold uppercase tracking-widest text-primary hover:scale-[1.02] transition-all">
                 <Share2 size={14} />
                 Guia de Distribuição
                 <ArrowUpRight size={14} />
              </button>
           </div>
        </aside>
      </div>
    </div>
  );
}
