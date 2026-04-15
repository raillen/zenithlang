import { 
  cloneTheme,
  resolveThemeById,
  resolveThemes,
  isBuiltinThemeId,
  parseThemeJson,
  serializeTheme,
  type ZenithTheme,
} from "../../themes";
import { 
  useState, 
  useMemo, 
  useRef, 
  useEffect,
  type ChangeEvent
} from "react";
import { motion, AnimatePresence } from "framer-motion";
import { 
  FlaskConical, 
  RefreshCw, 
  Save, 
  Download, 
  Upload, 
  Zap, 
  Globe 
} from "lucide-react";
import { useWorkspaceStore } from "../../store/useWorkspaceStore";
import { useThemeSync } from "../../hooks/useThemeSync";
import { applyThemeToDOM } from "../../utils/themeEngine";
import { SearchableDropdown } from "../ui/SearchableDropdown";
import { type ThemePillar } from "./types";
import { Pillars } from "./Pillars";
import { HeroWorkbench } from "./HeroWorkbench";
import { SoulEditor, AnatomyEditor, EngineEditor } from "./Editors";

const PREVIEW_THEME_ID = "__theme_lab_preview__";

export function ThemeLab() {
  const { settings, updateSettings } = useWorkspaceStore();
  const { broadcastThemeUpdate } = useThemeSync();
  const themeRegistry = useMemo(() => resolveThemes(settings.customThemes), [settings.customThemes]);
  
  const activeTheme = useMemo(
    () => resolveThemeById(settings.theme, settings.customThemes),
    [settings.customThemes, settings.theme]
  );

  const [pillar, setPillar] = useState<ThemePillar>("soul");
  const [draft, setDraft] = useState(() => forkTheme(activeTheme, false));
  const [feedback, setFeedback] = useState<string | null>(null);
  const [hoveredRegion, setHoveredRegion] = useState<string | undefined>(undefined);
  
  const fileInputRef = useRef<HTMLInputElement>(null);
  const persistedThemeRef = useRef(activeTheme);

  // Sync effect
  useEffect(() => {
    applyThemeToDOM(draft);
    const previewTheme = { ...cloneTheme(draft), id: PREVIEW_THEME_ID };
    void broadcastThemeUpdate(previewTheme, PREVIEW_THEME_ID);
  }, [broadcastThemeUpdate, draft]);

  // Restore logic on unmount
  useEffect(() => {
    return () => {
      const saved = persistedThemeRef.current;
      applyThemeToDOM(saved);
      void broadcastThemeUpdate(saved, saved.id);
    };
  }, [broadcastThemeUpdate]);

  // Handle preset selection
  const selectPreset = (id: string) => {
    const next = resolveThemeById(id, settings.customThemes);
    setDraft(forkTheme(next, false));
    setFeedback(`Enzo carregou: ${next.name}`);
  };

  const applyDraft = async () => {
    const themeToSave = ensureSavableTheme(draft, themeRegistry);
    await updateSettings({
      customThemes: { ...settings.customThemes, [themeToSave.id]: themeToSave },
      theme: themeToSave.id,
    });
    persistedThemeRef.current = themeToSave;
    setDraft(cloneTheme(themeToSave));
    await broadcastThemeUpdate(themeToSave, themeToSave.id);
    setFeedback("Sincronia soberana completa.");
  };

  const resetDraft = () => {
    setDraft(forkTheme(persistedThemeRef.current, false));
    setFeedback("Rascunho descartado.");
  };

  const handleExport = () => {
    const data = serializeTheme(draft);
    const blob = new Blob([data], { type: "application/json" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    a.download = `${draft.name.toLowerCase().replace(/\s+/g, "-")}.zenith.json`;
    a.click();
    URL.revokeObjectURL(url);
    setFeedback("Artefato de alma exportado.");
  };

  const handleFileImport = async (event: ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file) return;
    try {
      const parsed = parseThemeJson(await file.text());
      const forked = forkTheme(parsed, true); // Always fork on import to prevent overwrites
      setDraft(forked);
      setFeedback(`Importado: ${forked.name}`);
    } catch (error) {
      setFeedback("Falha na alquimia de importação.");
    } finally {
      event.target.value = "";
    }
  };

  return (
    <div className="flex h-full w-full flex-col bg-ide-bg overflow-hidden">
      <header className="z-20 flex shrink-0 items-center justify-between border-b border-ide-border bg-ide-panel/80 px-8 py-5 backdrop-blur-2xl">
        <div className="flex items-center gap-4">
          <div className="flex h-12 w-12 items-center justify-center rounded-2xl border border-primary/20 bg-primary/10 text-primary shadow-inner">
            <FlaskConical size={22} className="animate-pulse" />
          </div>
          <div className="flex flex-col">
            <div className="flex items-center gap-2">
               <span className="text-[10px] font-bold uppercase tracking-[0.25em] text-primary">Zenith Lab</span>
               <div className="h-1 w-1 rounded-full bg-primary/40" />
               <span className="text-[10px] font-bold uppercase tracking-widest text-ide-text-dim">Engine v1.0</span>
            </div>
            <input 
               value={draft.name}
               onChange={e => setDraft(d => ({ ...d, name: e.target.value }))}
               placeholder="Rename your soul..."
               className="w-64 bg-transparent text-xl font-bold tracking-tight text-white outline-none focus:ring-1 focus:ring-primary/20 rounded-md -ml-1 pl-1 transition-all"
            />
          </div>
        </div>

        <div className="flex items-center gap-3">
          <button 
             onClick={resetDraft}
             className="flex h-11 items-center gap-3 rounded-xl border border-ide-border bg-ide-panel px-5 text-[11px] font-bold uppercase tracking-widest text-ide-text-dim hover:border-primary/20 hover:text-primary transition-all active:scale-95"
          >
            <RefreshCw size={14} />
            Reset
          </button>
          
          <button 
             onClick={applyDraft}
             title="Save current theme"
             className="group relative flex h-11 items-center gap-3 overflow-hidden rounded-xl bg-primary px-6 text-[11px] font-bold uppercase tracking-widest text-white shadow-lg shadow-primary/20 transition-all hover:scale-[1.02] active:scale-95"
          >
            <div className="absolute inset-0 bg-gradient-to-r from-white/0 via-white/10 to-white/0 -translate-x-full group-hover:translate-x-full transition-transform duration-700" />
            <Save size={14} />
            Keep Evolution
          </button>

          <button 
             onClick={() => {
               const newTheme = forkTheme(draft, true);
               setDraft(newTheme);
               setFeedback(`${newTheme.name} criado. Pronto para evoluir.`);
             }}
             title="Save as a new theme"
             className="flex h-11 w-11 items-center justify-center rounded-xl border border-ide-border bg-ide-panel text-ide-text-dim hover:border-primary/20 hover:text-primary transition-all active:scale-95"
          >
            <Zap size={16} />
          </button>
        </div>
      </header>

      {/* Lab Surface */}
      <div className="grid min-h-0 flex-1 grid-cols-[380px_1fr] overflow-hidden">
        {/* Navigation & Controls Sidebar */}
        <aside className="relative z-10 flex flex-col border-r border-ide-border bg-ide-panel/40 p-8 overflow-y-auto no-scrollbar">
           <Pillars activePillar={pillar} onChange={setPillar} />
           
           <div className="mt-8 flex-1">
              <AnimatePresence mode="wait">
                <motion.div
                  key={pillar}
                  initial={{ opacity: 0, x: -10 }}
                  animate={{ opacity: 1, x: 0 }}
                  exit={{ opacity: 0, x: 10 }}
                  transition={{ duration: 0.3 }}
                >
                  {pillar === "soul" && <SoulEditor theme={draft} onChange={setDraft} onHover={setHoveredRegion} />}
                  {pillar === "anatomy" && <AnatomyEditor theme={draft} onChange={setDraft} onHover={setHoveredRegion} />}
                  {pillar === "engine" && <EngineEditor theme={draft} onChange={setDraft} onHover={setHoveredRegion} />}
                  {pillar === "gallery" && (
                    <div className="space-y-6">
                       <SectionTitle eyebrow="Collection" title="Base Presets" />
                       <SearchableDropdown 
                          options={Object.values(themeRegistry).map(t => ({ value: t.id, label: t.name }))}
                          value={draft.id}
                          onChange={selectPreset}
                       />
                       <div className="grid grid-cols-2 gap-3">
                          <ActionButton icon={<Download size={14} />} label="Export" onClick={handleExport} />
                          <ActionButton icon={<Upload size={14} />} label="Import" onClick={() => fileInputRef.current?.click()} />
                          <input ref={fileInputRef} type="file" className="hidden" onChange={handleFileImport} />
                       </div>
                    </div>
                  )}
                </motion.div>
              </AnimatePresence>
           </div>

           {/* Feedback Toast (Integrated) */}
           {feedback && (
             <motion.div 
               initial={{ opacity: 0, y: 10 }} animate={{ opacity: 1, y: 0 }}
               className="mt-6 rounded-2xl border border-primary/20 bg-primary/5 p-4 text-center text-[10px] font-bold uppercase tracking-widest text-primary"
             >
               {feedback}
             </motion.div>
           )}
        </aside>

        {/* Hero Workbench Surface */}
        <main className="relative flex items-center justify-center bg-ide-bg p-12 lg:p-20 overflow-hidden">
           {/* Decorative Background Elements */}
           <div className="absolute top-0 right-0 h-[600px] w-[600px] translate-x-1/2 -translate-y-1/2 rounded-full bg-primary/5 blur-[120px] pointer-events-none" />
           <div className="absolute bottom-0 left-0 h-[600px] w-[600px] -translate-x-1/2 translate-y-1/2 rounded-full bg-blue-500/5 blur-[120px] pointer-events-none" />
           
           <div className="relative h-full w-full max-w-[1200px] max-h-[800px]">
              <HeroWorkbench 
                theme={draft} 
                highlightedRegion={hoveredRegion}
                onRegionClick={(r) => {
                  if (r === "editor" || r === "terminal") setPillar("anatomy");
                  if (r === "chrome") setPillar("anatomy");
                }}
              />
           </div>

           {/* Quick Stats Overlay */}
           <div className="absolute bottom-10 right-10 flex gap-4">
              <StatCard icon={<Zap size={14} />} label="Sync" value="Real-time" />
              <StatCard icon={<Globe size={14} />} label="Type" value={draft.type} />
           </div>
        </main>
      </div>
    </div>
  );
}

function forkTheme(theme: ZenithTheme, randomizeId = true): ZenithTheme {
  return {
    ...cloneTheme(theme),
    id: randomizeId ? `theme-${Math.random().toString(36).substring(2, 7)}` : theme.id,
    name: randomizeId ? `${theme.name} (Copy)` : theme.name,
    meta: { ...theme.meta, builtin: false },
  };
}

function ensureSavableTheme(theme: ZenithTheme, registry: Record<string, ZenithTheme>): ZenithTheme {
  const next = cloneTheme(theme);
  next.id = next.id.toLowerCase().replace(/[^a-z0-9]/g, "-");
  if (registry[next.id] && registry[next.id].meta?.builtin) next.id = `${next.id}-custom`;
  if (!next.id || isBuiltinThemeId(next.id)) next.id = `custom-${Date.now()}`;
  next.meta = { ...next.meta, builtin: false };
  return next;
}

function SectionTitle({ eyebrow, title }: { eyebrow: string; title: string }) {
  return (
    <div className="space-y-1">
      <div className="text-[10px] font-bold uppercase tracking-[0.2em] text-primary/60">{eyebrow}</div>
      <div className="text-lg font-bold text-ide-text">{title}</div>
    </div>
  );
}

function ActionButton({ icon, label, onClick }: { icon: any; label: string; onClick: () => void }) {
  return (
    <button 
      onClick={onClick}
      className="flex items-center justify-center gap-3 rounded-xl border border-ide-border bg-ide-panel/40 py-3 text-[10px] font-bold uppercase tracking-widest text-ide-text-dim hover:text-primary transition-all"
    >
      {icon}
      {label}
    </button>
  );
}

function StatCard({ icon, label, value }: { icon: any; label: string; value: string }) {
  return (
    <div className="flex items-center gap-3 rounded-2xl border border-ide-border bg-ide-panel/60 px-4 py-2 backdrop-blur-xl">
      <div className="text-primary">{icon}</div>
      <div>
        <div className="text-[9px] font-bold uppercase tracking-widest text-ide-text-dim opacity-60">{label}</div>
        <div className="text-[11px] font-bold text-ide-text">{value}</div>
      </div>
    </div>
  );
}
