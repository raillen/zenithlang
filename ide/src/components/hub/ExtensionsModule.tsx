import { useState, useMemo } from "react";
import { ChevronDown, Terminal, Zap, ShieldCheck, Box, Search } from "lucide-react";
import { useWorkspaceStore } from "../../store/useWorkspaceStore";
import { ToggleSwitch } from "../ui/ToggleSwitch";
import { getRuntimeProfileForSettingsKey } from "../../utils/runtimeProfiles";
import { AnimatePresence, motion } from "framer-motion";

type ExtensionCategory = "all" | "runtime" | "lsp" | "tool";

export function ExtensionsModule() {
  const { settings, updateSettings } = useWorkspaceStore();
  const [searchQuery, setSearchQuery] = useState("");
  const [activeCategory, setActiveCategory] = useState<ExtensionCategory>("all");

  const handleUpdate = (key: string, newCfg: any) => {
    updateSettings({
      extensions: {
        ...settings.extensions,
        [key]: { ...settings.extensions[key], ...newCfg }
      }
    });
  };

  const filteredExtensions = useMemo(() => {
    return Object.entries(settings.extensions || {}).filter(([key, _ext]) => {
      const profile = getRuntimeProfileForSettingsKey(key);
      const matchesSearch = 
        profile.settingsLabel.toLowerCase().includes(searchQuery.toLowerCase()) ||
        key.toLowerCase().includes(searchQuery.toLowerCase());
      
      if (!matchesSearch) return false;

      if (activeCategory === "all") return true;
      // Simple heuristic for demo: zenith is runtime, others tools/lsp
      if (activeCategory === "runtime") return ["zenith"].includes(key);
      if (activeCategory === "lsp") return key.includes("lsp") || key === "zenith"; // Zenith is both
      if (activeCategory === "tool") return !["zenith"].includes(key);
      
      return true;
    });
  }, [settings.extensions, searchQuery, activeCategory]);

  return (
    <div className="space-y-10 animate-in fade-in slide-in-from-bottom-4 duration-1000 h-full flex flex-col">
      {/* Sovereign Header */}
      <header className="space-y-6 shrink-0">
        <div className="flex items-center justify-between">
          <div className="space-y-1">
            <div className="flex items-center gap-3">
              <div className="h-1.5 w-6 bg-primary/30 rounded-full" />
              <div className="text-[9px] font-bold uppercase tracking-[0.3em] text-primary/60">Sovereign Ecosystem</div>
            </div>
            <h2 className="text-[28px] font-bold tracking-tight text-ide-text">Extensões & Toolchains</h2>
          </div>
          
          <div className="flex items-center gap-2 px-3 py-1.5 rounded-lg border border-ide-border bg-ide-panel/20 text-[10px] font-bold text-ide-text-dim">
             <Box size={14} className="text-primary/60" />
             <span>{Object.keys(settings.extensions || {}).length} Carregadas</span>
          </div>
        </div>

        {/* Search & Filter Bar */}
        <div className="flex flex-col sm:flex-row gap-3">
          <div className="relative flex-1 group">
            <Search size={14} className="absolute left-4 top-1/2 -translate-y-1/2 text-ide-text-dim/40 group-focus-within:text-primary transition-colors" />
            <input 
              type="text"
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              placeholder="Pesquisar runtimes, compiladores ou LSPs..."
              className="w-full bg-ide-panel/10 border border-ide-border rounded-xl pl-11 pr-4 py-2.5 text-[12px] text-ide-text focus:outline-none focus:border-primary/40 focus:ring-4 focus:ring-primary/5 transition-all"
            />
          </div>
          <div className="flex items-center gap-1 p-1 rounded-xl border border-ide-border bg-ide-panel/5">
             {(["all", "runtime", "lsp", "tool"] as ExtensionCategory[]).map((cat) => (
                <button
                  key={cat}
                  onClick={() => setActiveCategory(cat)}
                  className={`px-3 py-1.5 rounded-lg text-[9px] font-bold uppercase tracking-wider transition-all ${
                    activeCategory === cat 
                      ? "bg-ide-bg text-primary shadow-sm border border-ide-border" 
                      : "text-ide-text-dim/60 hover:text-ide-text"
                  }`}
                >
                  {cat}
                </button>
             ))}
          </div>
        </div>
      </header>

      <div className="space-y-3 flex-1 overflow-y-auto no-scrollbar pb-10">
        {filteredExtensions.length > 0 ? (
          filteredExtensions.map(([key, ext]) => (
            <ExtensionRow
              key={key}
              id={key}
              config={ext}
              onUpdate={(newCfg) => handleUpdate(key, newCfg)}
            />
          ))
        ) : (
          <div className="py-20 text-center space-y-4">
             <div className="inline-flex h-12 w-12 items-center justify-center rounded-2xl bg-ide-panel/10 text-ide-text-dim/20">
                <Search size={24} />
             </div>
             <p className="text-[13px] text-ide-text-dim/60">Nenhuma extensão encontrada para "{searchQuery}"</p>
          </div>
        )}
      </div>

      <div className="p-6 rounded-2xl border border-primary/20 bg-primary/5 flex items-start gap-4 shrink-0">
        <div className="h-8 w-8 shrink-0 flex items-center justify-center rounded-lg bg-primary text-white shadow-lg shadow-primary/20">
           <Zap size={14} />
        </div>
        <div className="space-y-1">
          <div className="text-[11px] font-bold text-primary uppercase tracking-wider">Compass Protocol</div>
          <p className="text-[11px] leading-relaxed text-ide-text-dim/80">
            Comunicação isolada via JSON-RPC. A performance da UI é preservada independentemente da carga do backend.
          </p>
        </div>
      </div>
    </div>
  );
}

function ExtensionRow({ id, config, onUpdate }: { id: string; config: any; onUpdate: (cfg: any) => void }) {
  const [open, setOpen] = useState(id === "zenith");
  const profile = getRuntimeProfileForSettingsKey(id);
  const headerLabel = profile.settingsLabel;

  return (
    <div className={`group rounded-xl border transition-all duration-300 overflow-hidden ${
      open 
        ? 'border-primary/40 bg-ide-panel/10 shadow-lg shadow-primary/5' 
        : 'border-ide-border bg-ide-panel/5 hover:border-ide-text-dim/30 hover:bg-ide-panel/10'
    }`}>
      <div
        className={`w-full flex items-center justify-between px-5 py-4 transition-all duration-300 cursor-pointer ${
          open ? "bg-primary/[0.02]" : ""
        }`}
        onClick={() => setOpen(!open)}
      >
        <div className="flex items-center gap-4">
           <div className={`flex h-10 w-10 items-center justify-center rounded-xl transition-all duration-300 ${
              config.isEnabled ? 'bg-primary/10 text-primary' : 'bg-ide-panel/20 text-ide-text-dim/40'
           }`}>
              <Box size={18} className={config.isEnabled ? "animate-pulse" : ""} />
           </div>
           <div className="text-left">
              <span className={`block text-[14px] font-bold tracking-tight transition-colors ${config.isEnabled ? 'text-ide-text' : 'text-ide-text-dim/60'}`}>
                {headerLabel}
              </span>
              <div className="flex items-center gap-2">
                <span className="text-[9px] font-bold uppercase tracking-widest text-ide-text-dim/30">{id}</span>
                {config.isEnabled && <div className="h-1 w-1 rounded-full bg-emerald-500 shadow-[0_0_5px_rgba(16,185,129,0.5)]" />}
              </div>
           </div>
        </div>
        <div className="flex items-center gap-6" onClick={(e) => e.stopPropagation()}>
          <div className="flex items-center gap-3 border-r border-ide-border/50 pr-6 mr-1 lg:flex hidden">
             <span className="text-[9px] font-bold uppercase tracking-widest text-ide-text-dim/40">{config.isEnabled ? "Ativo" : "Off"}</span>
             <ToggleSwitch
                checked={config.isEnabled}
                onChange={(val) => onUpdate({ isEnabled: val })}
             />
          </div>
          <motion.div 
            animate={{ rotate: open ? 180 : 0 }}
            className="h-8 w-8 flex items-center justify-center rounded-lg border border-ide-border/50 bg-ide-panel/20 text-ide-text-dim group-hover:text-ide-text transition-colors"
          >
             <ChevronDown size={14} />
          </motion.div>
        </div>
      </div>

      <AnimatePresence>
        {open && (
          <motion.div 
            initial={{ height: 0, opacity: 0 }}
            animate={{ height: "auto", opacity: 1 }}
            exit={{ height: 0, opacity: 0 }}
            transition={{ duration: 0.3, ease: "circOut" }}
            className="overflow-hidden"
          >
            <div className="p-6 pt-2 grid gap-6 border-t border-ide-border/20 bg-black/[0.01]">
              <div className="space-y-3">
                <div className="flex items-center gap-2 text-[10px] font-bold text-ide-text-dim/50 uppercase tracking-[0.2em]">
                  <Terminal size={12} className="text-primary/40" />
                  <span>Caminho do Binário</span>
                </div>
                <input
                  type="text"
                  value={config.compilerPath || ""}
                  onChange={(e) => onUpdate({ compilerPath: e.target.value })}
                  className="w-full bg-ide-panel/10 border border-ide-border rounded-xl px-4 py-2.5 text-[12px] font-mono text-ide-text focus:outline-none focus:border-primary/40 focus:bg-ide-bg transition-all"
                  placeholder={`e.g. ${profile.compilerPlaceholder}`}
                />
              </div>

              <div className="grid lg:grid-cols-2 gap-6">
                 <div className="space-y-3">
                    <div className="flex items-center gap-2 text-[10px] font-bold text-ide-text-dim/50 uppercase tracking-[0.2em]">
                      <ShieldCheck size={12} className="text-emerald-500/40" />
                      <span>LSP Server</span>
                    </div>
                    <input
                      type="text"
                      value={config.lspPath || ""}
                      onChange={(e) => onUpdate({ lspPath: e.target.value })}
                      className="w-full bg-ide-panel/10 border border-ide-border rounded-xl px-4 py-2.5 text-[12px] font-mono text-ide-text focus:outline-none focus:border-primary/40 focus:bg-ide-bg transition-all"
                      placeholder={profile.lspPlaceholder || "Caminho do LSP"}
                    />
                 </div>

                 <div className="space-y-3">
                    <div className="flex items-center gap-2 text-[10px] font-bold text-ide-text-dim/50 uppercase tracking-[0.2em]">
                      <Zap size={12} className="text-orange-500/40" />
                      <span>Build Task</span>
                    </div>
                    <input
                      type="text"
                      value={config.buildCommand || ""}
                      onChange={(e) => onUpdate({ buildCommand: e.target.value })}
                      className="w-full bg-ide-panel/10 border border-ide-border rounded-xl px-4 py-2.5 text-[12px] font-mono text-ide-text focus:outline-none focus:border-primary/40 focus:bg-ide-bg transition-all"
                      placeholder={`e.g. ${profile.buildPlaceholder}`}
                    />
                 </div>
              </div>
            </div>
          </motion.div>
        )}
      </AnimatePresence>
    </div>
  );
}
