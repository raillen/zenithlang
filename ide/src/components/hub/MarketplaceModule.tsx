import { useMemo, useState } from "react";
import { CheckCircle2, Download, LayoutGrid, List, Search, ShoppingBag, Box, Palette, Globe, ShieldCheck } from "lucide-react";
import { useWorkspaceStore } from "../../store/useWorkspaceStore";
import { resolveThemes, type ZenithTheme } from "../../themes";
import { AnimatePresence, motion } from "framer-motion";

type MarketplaceCategoryId = "themes" | "languages" | "tooling" | "accessibility" | "ui";
type MarketplaceViewMode = "list" | "cards";
type ThemeSourceFilter = "all" | "official" | "local";
type ThemeToneFilter = "all" | "light" | "dark";

interface MarketplaceCategory {
  id: MarketplaceCategoryId;
  label: string;
  icon: any;
}

interface ThemeCardViewModel {
  theme: ZenithTheme;
  isActive: boolean;
  sourceLabel: string;
  authorLabel: string;
  tags: string[];
}

const MARKETPLACE_CATEGORIES: MarketplaceCategory[] = [
  { id: "themes", label: "Temas", icon: Palette },
  { id: "languages", label: "Languages", icon: Globe },
  { id: "tooling", label: "Tooling", icon: Box },
  { id: "accessibility", label: "Acessibilidade", icon: ShieldCheck },
  { id: "ui", label: "Interfaces", icon: LayoutGrid },
];

function buildThemeTags(theme: ZenithTheme) {
  const tags = [theme.type, theme.meta?.builtin ? "oficial" : "local"];
  if (theme.ui.glass) tags.push("glass");
  return tags;
}

export function MarketplaceModule() {
  const settings = useWorkspaceStore((state) => state.settings);
  const updateSettings = useWorkspaceStore((state) => state.updateSettings);

  const [activeCategory, setActiveCategory] = useState<MarketplaceCategoryId>("themes");
  const [viewMode, setViewMode] = useState<MarketplaceViewMode>("list");
  const [searchQuery, setSearchQuery] = useState("");
  const [sourceFilter, setSourceFilter] = useState<ThemeSourceFilter>("all");
  const [toneFilter] = useState<ThemeToneFilter>("all");

  const themeCards = useMemo<ThemeCardViewModel[]>(() => {
    const registry = resolveThemes(settings.customThemes);

    return Object.values(registry)
      .sort((left, right) => {
        const leftWeight = left.id === settings.theme ? -2 : left.meta?.builtin ? -1 : 0;
        const rightWeight = right.id === settings.theme ? -2 : right.meta?.builtin ? -1 : 0;

        if (leftWeight !== rightWeight) return leftWeight - rightWeight;
        return left.name.localeCompare(right.name);
      })
      .map((theme) => ({
        theme,
        isActive: theme.id === settings.theme,
        sourceLabel: theme.meta?.builtin ? "Oficial" : "Local",
        authorLabel: theme.meta?.author || (theme.meta?.builtin ? "Zenith Core" : "Custom Theme"),
        tags: buildThemeTags(theme),
      }));
  }, [settings.customThemes, settings.theme]);

  const filteredThemes = useMemo(() => {
    const query = searchQuery.trim().toLowerCase();

    return themeCards.filter((entry) => {
      if (sourceFilter !== "all" && entry.sourceLabel.toLowerCase() !== (sourceFilter === "official" ? "oficial" : "local")) {
        return false;
      }

      if (toneFilter !== "all" && entry.theme.type !== toneFilter) {
        return false;
      }

      if (!query) return true;

      const searchable = [
        entry.theme.name,
        entry.theme.id,
        entry.theme.meta?.description || "",
        entry.authorLabel,
        ...entry.tags,
      ]
        .join(" ")
        .toLowerCase();

      return searchable.includes(query);
    });
  }, [searchQuery, sourceFilter, themeCards, toneFilter]);

  const categoryCounts = useMemo(
    () => ({
      themes: themeCards.length,
      languages: 0,
      tooling: 0,
      accessibility: themeCards.filter((entry) => entry.tags.includes("glass")).length,
      ui: 0,
    }),
    [themeCards]
  );

  const isThemeCategory = activeCategory === "themes";
  const visibleThemes = isThemeCategory ? filteredThemes : [];
  const applyTheme = (themeId: string) => {
    void updateSettings({ theme: themeId });
  };

  return (
    <div className="space-y-12 animate-in fade-in slide-in-from-bottom-4 duration-1000 h-full flex flex-col">
      {/* Sovereign Header */}
      <header className="space-y-8 shrink-0">
        <div className="flex items-center gap-3">
          <div className="h-2 w-8 bg-primary/30 rounded-full" />
          <div className="text-[10px] font-bold uppercase tracking-[0.4em] text-primary/60">Ecosistema Soberano</div>
        </div>
        
        <div className="grid grid-cols-2 md:grid-cols-5 gap-4">
          {MARKETPLACE_CATEGORIES.map((category) => {
            const isActive = activeCategory === category.id;
            const count = categoryCounts[category.id];
            const Icon = category.icon;

            return (
              <button
                key={category.id}
                type="button"
                onClick={() => setActiveCategory(category.id)}
                className={`group flex flex-col items-center justify-center p-6 rounded-2xl border transition-all duration-300 ${
                  isActive
                    ? "bg-primary/5 border-primary/40 shadow-xl shadow-primary/5"
                    : "bg-ide-panel/5 border-ide-border hover:border-ide-text-dim/30 hover:bg-ide-panel/10"
                }`}
              >
                <div className={`mb-2.5 transition-transform duration-300 group-hover:scale-105 ${isActive ? "text-primary" : "text-ide-text-dim/40 group-hover:text-ide-text-dim"}`}>
                  <Icon size={24} />
                </div>
                <div className={`text-[10px] font-bold uppercase tracking-[0.2em] transition-colors ${isActive ? "text-primary" : "text-ide-text-dim/60 group-hover:text-ide-text-dim"}`}>
                  {category.label}
                </div>
                {count > 0 && (
                  <div className={`mt-2 text-[16px] font-display font-bold ${isActive ? "text-ide-text" : "text-ide-text-dim/30"}`}>
                    {count}
                  </div>
                )}
              </button>
            );
          })}
        </div>
      </header>

      {/* Control Bar */}
      <section className="space-y-8 flex-1 flex flex-col min-h-0">
        <div className="flex flex-col gap-6 lg:flex-row lg:items-center shrink-0">
          <div className="relative group flex-1">
            <Search size={16} className="pointer-events-none absolute left-4 top-1/2 -translate-y-1/2 text-ide-text-dim/40 transition-colors group-focus-within:text-primary" />
            <input
              type="search"
              value={searchQuery}
              onChange={(event) => setSearchQuery(event.target.value)}
              placeholder="Pesquisar no ecossistema..."
              className="w-full h-11 rounded-xl border border-ide-border bg-ide-panel/10 pl-11 pr-4 text-[12px] text-ide-text outline-none transition-all placeholder:text-ide-text-dim/30 focus:border-primary/40 focus:bg-ide-bg focus:ring-4 focus:ring-primary/5 shadow-inner shadow-white/[0.02]"
            />
          </div>

          <div className="flex items-center gap-4">
            <div className="flex items-center gap-1 rounded-xl border border-ide-border bg-ide-panel/10 p-0.5">
              <ViewToggleButton active={viewMode === "list"} icon={<List size={14} />} onClick={() => setViewMode("list")} />
              <ViewToggleButton active={viewMode === "cards"} icon={<LayoutGrid size={14} />} onClick={() => setViewMode("cards")} />
            </div>

            <div className="h-6 w-px bg-ide-border/40" />

            <div className="flex gap-1.5">
              <FilterPill active={sourceFilter === "all"} label="Todos" onClick={() => setSourceFilter("all")} />
              <FilterPill active={sourceFilter === "official"} label="Oficiais" onClick={() => setSourceFilter("official")} />
              <FilterPill active={sourceFilter === "local"} label="Locais" onClick={() => setSourceFilter("local")} />
            </div>
          </div>
        </div>

        {/* Content Container */}
        <div className="flex-1 overflow-y-auto no-scrollbar pb-20">
          <AnimatePresence mode="wait">
            {isThemeCategory ? (
              visibleThemes.length > 0 ? (
                <motion.div
                  key={`results-${viewMode}`}
                  initial={{ opacity: 0, scale: 0.98 }}
                  animate={{ opacity: 1, scale: 1 }}
                  exit={{ opacity: 0, scale: 1.02 }}
                  transition={{ duration: 0.5, ease: "circOut" }}
                  className={viewMode === "list" ? "space-y-3" : "grid gap-6 md:grid-cols-2 lg:grid-cols-3"}
                >
                  {visibleThemes.map((entry) => (
                    viewMode === "list" ? (
                      <ThemeListRow key={entry.theme.id} entry={entry} onApplyTheme={applyTheme} />
                    ) : (
                      <ThemeMiniCard key={entry.theme.id} entry={entry} onApplyTheme={applyTheme} />
                    )
                  ))}
                </motion.div>
              ) : (
                <EmptyResultsState searchQuery={searchQuery} />
              )
            ) : (
              <motion.div
                key="empty-cat"
                initial={{ opacity: 0 }}
                animate={{ opacity: 1 }}
                className="rounded-2xl border-2 border-dashed border-ide-border bg-ide-panel/5 p-24 text-center"
              >
                <div className="inline-flex h-16 w-16 items-center justify-center rounded-xl bg-ide-panel/20 text-ide-text-dim/20 mb-6">
                  <ShoppingBag size={40} />
                </div>
                <h4 className="text-[18px] font-bold text-ide-text/60">Em Breve</h4>
                <p className="mt-4 text-[14px] text-ide-text-dim/60 max-w-sm mx-auto leading-relaxed">
                  Estamos construindo um repositório centralizado para extensões e módulos Zenith v2-compatíveis.
                </p>
              </motion.div>
            )}
          </AnimatePresence>
        </div>
      </section>
    </div>
  );
}

function ThemeListRow({
  entry,
  onApplyTheme,
}: {
  entry: ThemeCardViewModel;
  onApplyTheme: (themeId: string) => void;
}) {
  return (
    <article
      className={`group relative flex items-center gap-6 p-4 rounded-2xl border transition-all duration-300 ${
        entry.isActive 
          ? "bg-primary/[0.04] border-primary/30 shadow-xl shadow-primary/5" 
          : "bg-ide-panel/5 border-ide-border hover:border-ide-text-dim/30 hover:bg-ide-panel/10"
      }`}
    >
      <CompactThemePreview theme={entry.theme} />

      <div className="flex-1 min-w-0">
        <div className="flex items-center gap-3 mb-1.5">
          <h5 className="text-[16px] font-bold tracking-tight text-ide-text">{entry.theme.name}</h5>
          {entry.isActive && (
            <span className="px-2 py-0.5 rounded-full bg-primary/10 text-[9px] font-bold uppercase tracking-widest text-primary border border-primary/20">
              Ativo
            </span>
          )}
        </div>
        <p className="text-[12px] text-ide-text-dim/60 line-clamp-1">
          {entry.theme.meta?.description || `Schema visual ${entry.theme.type === "dark" ? "Dark Onyx" : "Light Frost"} para Taurus IDE.`}
        </p>
      </div>

       <div className="flex items-center gap-4">
         <div className="hidden lg:flex flex-col items-end">
            <span className="text-[8px] font-bold uppercase tracking-widest text-ide-text-dim/30">Author</span>
            <span className="text-[10px] font-bold text-ide-text-dim/70">{entry.authorLabel}</span>
         </div>
         <button
           type="button"
           onClick={() => onApplyTheme(entry.theme.id)}
           disabled={entry.isActive}
           className={`h-9 px-4 rounded-xl text-[10px] font-bold uppercase tracking-[0.2em] transition-all duration-300 ${
             entry.isActive
               ? "cursor-default text-emerald-500 bg-emerald-500/10 border border-emerald-500/20"
               : "bg-ide-text text-ide-bg hover:shadow-lg hover:shadow-ide-text/20 hover:scale-105 active:scale-95"
           }`}
         >
           {entry.isActive ? "Instalado" : "Ativar"}
         </button>
      </div>
    </article>
  );
}

function ThemeMiniCard({
  entry,
  onApplyTheme,
}: {
  entry: ThemeCardViewModel;
  onApplyTheme: (themeId: string) => void;
}) {
  return (
    <article
      className={`relative overflow-hidden rounded-2xl border transition-all duration-300 group ${
        entry.isActive
          ? "bg-primary/[0.04] border-primary/30 shadow-xl shadow-primary/5"
          : "bg-ide-panel/30 border-ide-border hover:border-ide-text-dim/30 hover:bg-ide-panel/50"
      }`}
    >
      <div className="aspect-[1.8/1] relative overflow-hidden p-8 flex flex-col justify-end transition-transform duration-700 group-hover:scale-[1.02]" style={{ background: entry.theme.ui.background }}>
        <div className="absolute top-6 left-6 flex gap-2">
           <ColorDot color={entry.theme.ui.primary} />
           <ColorDot color={entry.theme.tokens.surface.panel} />
        </div>
        <div className="absolute top-6 right-6">
           <span className="text-[9px] font-bold uppercase tracking-widest opacity-30" style={{ color: entry.theme.ui.textMain }}>{entry.theme.type} mode</span>
        </div>
        <h5 className="text-[20px] font-display font-bold tracking-tight" style={{ color: entry.theme.ui.textMain }}>{entry.theme.name}</h5>
      </div>

      <div className="p-8 space-y-6">
        <div className="flex items-center justify-between">
           <div className="flex -space-x-1">
              {[entry.theme.ui.primary, entry.theme.ui.background, entry.theme.tokens.surface.panel].map((c, i) => (
                 <div key={i} className="h-6 w-6 rounded-full border-2 border-ide-bg" style={{ background: c }} />
              ))}
           </div>
           <span className="text-[10px] font-bold uppercase tracking-widest text-ide-text-dim/40">{entry.authorLabel}</span>
        </div>

        <button
          type="button"
          onClick={() => onApplyTheme(entry.theme.id)}
          disabled={entry.isActive}
          className={`h-12 w-full flex items-center justify-center gap-3 rounded-2xl text-[11px] font-bold uppercase tracking-[0.2em] transition-all duration-500 ${
            entry.isActive
              ? "cursor-default text-emerald-500 bg-emerald-500/10 border border-emerald-500/20"
              : "bg-ide-text text-ide-bg hover:shadow-xl hover:shadow-ide-text/20 group-hover:scale-[1.02]"
          }`}
        >
          {entry.isActive ? <CheckCircle2 size={16} /> : <Download size={16} />}
          {entry.isActive ? "Ativado" : "Instalar Tema"}
        </button>
      </div>
    </article>
  );
}

function CompactThemePreview({ theme }: { theme: ZenithTheme }) {
  return (
    <div className="grid h-14 w-20 shrink-0 grid-cols-2 overflow-hidden rounded-xl border border-ide-border bg-ide-panel/40 shadow-xl transition-transform duration-300 group-hover:scale-105">
      <div style={{ background: theme.ui.background }} />
      <div className="grid grid-rows-3">
        <div style={{ background: theme.tokens.surface.panel }} />
        <div style={{ background: theme.ui.primary }} />
        <div style={{ background: theme.ui.textMain }} />
      </div>
    </div>
  );
}

function ViewToggleButton({ active, icon, onClick }: { active: boolean; icon: React.ReactNode; onClick: () => void }) {
  return (
    <button
      type="button"
      onClick={onClick}
      className={`inline-flex h-10 w-11 items-center justify-center rounded-xl transition-all duration-500 ${
        active ? "bg-ide-bg text-primary shadow-xl ring-1 ring-ide-border/50 scale-105" : "text-ide-text-dim/40 hover:text-ide-text-dim"
      }`}
    >
      {icon}
    </button>
  );
}

function FilterPill({ active, label, onClick }: { active: boolean; label: string; onClick: () => void }) {
  return (
    <button
      type="button"
      onClick={onClick}
      className={`h-9 px-5 rounded-xl text-[10px] font-bold uppercase tracking-[0.1em] transition-all duration-300 border ${
        active
          ? "border-primary/40 bg-primary/10 text-primary shadow-lg shadow-primary/5"
          : "border-ide-border bg-ide-panel/5 text-ide-text-dim/60 hover:border-ide-text-dim/30 hover:text-ide-text-dim"
      }`}
    >
      {label}
    </button>
  );
}

function EmptyResultsState({ searchQuery }: { searchQuery: string }) {
  return (
    <div className="rounded-[48px] border-2 border-dashed border-ide-border bg-ide-panel/5 p-32 text-center">
      <h4 className="text-[16px] font-bold text-ide-text/60">Sem resultados</h4>
      <p className="mt-4 text-[13px] text-ide-text-dim/60 max-w-sm mx-auto leading-relaxed">
        {searchQuery ? `Não encontramos correspondências para \"${searchQuery}\".` : "Tente ajustar os filtros de busca."}
      </p>
    </div>
  );
}

function ColorDot({ color }: { color: string }) {
  return <div className="h-3 w-3 rounded-full border border-black/10 shadow-sm" style={{ background: color }} />;
}


