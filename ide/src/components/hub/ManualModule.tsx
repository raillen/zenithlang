import { useState } from "react";
import { Book, Search, ChevronRight, ExternalLink, Bookmark } from "lucide-react";
import { AnimatePresence, motion } from "framer-motion";
import { ZENITH_HUB_SECTIONS, ZenithHubSectionId } from "../../content/zenithHubContent";

export function ManualModule() {
  const [activeDocId, setActiveDocId] = useState<ZenithHubSectionId>("manual");
  const [searchQuery, setSearchQuery] = useState("");

  const docSections = ZENITH_HUB_SECTIONS.filter((section) => 
    section.id !== "overview" && section.id !== "marketplace" && section.id !== "theme-lab"
  );
  const activeDoc = docSections.find((section) => section.id === activeDocId) || docSections[0];

  const filteredSections = docSections.filter((section) => {
    const query = searchQuery.toLowerCase();
    return (
      section.title.toLowerCase().includes(query) ||
      section.navLabel.toLowerCase().includes(query) ||
      section.summary.toLowerCase().includes(query)
    );
  });

  return (
    <div className="animate-in fade-in slide-in-from-bottom-4 duration-1000 h-full flex flex-col lg:flex-row gap-12">
      {/* Sovereign Sidebar */}
      <aside className="w-full lg:w-[300px] shrink-0 space-y-10">
        <div className="space-y-6">
          <div className="flex items-center justify-between">
            <div className="text-[10px] font-bold uppercase tracking-[0.4em] text-primary">Navegação</div>
            <Book size={14} className="text-primary/40" />
          </div>
          
          <div className="relative group">
            <Search size={14} className="pointer-events-none absolute left-4 top-1/2 -translate-y-1/2 text-ide-text-dim/40 transition-colors group-focus-within:text-primary" />
            <input
              type="search"
              placeholder="Pesquisar guias..."
              value={searchQuery}
              onChange={(event) => setSearchQuery(event.target.value)}
              className="w-full h-10 rounded-xl border border-ide-border bg-ide-panel/10 pl-11 pr-4 text-[13px] text-ide-text outline-none transition-all placeholder:text-ide-text-dim/30 focus:border-primary/40 focus:bg-ide-bg focus:ring-4 focus:ring-primary/5"
            />
          </div>
        </div>

        <nav className="space-y-1.5">
          {filteredSections.map((section) => {
            const isActive = activeDocId === section.id;

            return (
              <button
                key={section.id}
                type="button"
                onClick={() => setActiveDocId(section.id)}
                className={`group relative flex w-full items-center gap-4 rounded-xl px-4 py-3 text-left transition-all duration-300 ${
                  isActive
                    ? "bg-ide-bg text-ide-text shadow-md ring-1 ring-ide-border"
                    : "text-ide-text-dim/70 hover:bg-ide-panel/20 hover:text-ide-text"
                }`}
              >
                {isActive && (
                   <motion.div 
                     layoutId="active-indicator"
                     className="absolute left-1.5 h-6 w-1 rounded-full bg-primary"
                   />
                )}
                <span className={`text-[12px] font-bold uppercase tracking-[0.15em] transition-colors ${isActive ? "text-primary" : ""}`}>
                  {section.navLabel}
                </span>
                {isActive && <ChevronRight size={14} className="ml-auto text-primary/60" />}
              </button>
            );
          })}
        </nav>
        
        {/* Quick Help Card */}
        <div className="rounded-2xl border border-ide-border bg-gradient-to-br from-ide-panel/10 to-transparent p-6 space-y-4">
           <div className="text-[9px] font-bold uppercase tracking-[0.2em] text-ide-text-dim/50">Recursos Externos</div>
           <div className="space-y-3">
              <button className="flex items-center justify-between w-full text-[11px] font-bold text-ide-text-dim hover:text-primary transition-colors">
                 Zenith Wiki
                 <ExternalLink size={12} />
              </button>
              <button className="flex items-center justify-between w-full text-[11px] font-bold text-ide-text-dim hover:text-primary transition-colors">
                 GitHub Community
                 <ExternalLink size={12} />
              </button>
           </div>
        </div>
      </aside>

      {/* Sovereign Content Area */}
      <main className="flex-1 min-w-0 pr-4 overflow-y-auto no-scrollbar pb-20">
        <AnimatePresence mode="wait">
          <motion.div
            key={activeDocId}
            initial={{ opacity: 0, scale: 0.98 }}
            animate={{ opacity: 1, scale: 1 }}
            exit={{ opacity: 0, scale: 1.02 }}
            transition={{ duration: 0.5, ease: "circOut" }}
            className="space-y-16"
          >
            {/* Minimalist Header */}
            <header className="space-y-8">
              <div className="flex items-center gap-3">
                <div className="h-2 w-10 bg-primary/20 rounded-full" />
                <div className="text-[11px] font-bold uppercase tracking-[0.4em] text-primary/60">
                   {activeDoc.eyebrow || "Guia de Engenharia"}
                </div>
              </div>

              <h1 className="font-display text-[44px] lg:text-[60px] font-bold tracking-tight text-ide-text leading-tight">
                {activeDoc.title}
              </h1>

              <p className="max-w-3xl text-[18px] leading-relaxed text-ide-text-dim/80">
                {activeDoc.summary}
              </p>
            </header>

            {/* Structured Content */}
            <div className="space-y-24">
              {activeDoc.blocks?.map((block, index) => (
                <section key={index} className="grid lg:grid-cols-[200px_1fr] gap-12 group">
                  <aside className="space-y-4">
                     <div className="flex items-center gap-2 text-primary/30 group-hover:text-primary transition-colors duration-500">
                        <Bookmark size={14} />
                        <span className="text-[10px] font-bold uppercase tracking-widest italic">{String(index + 1).padStart(2, '0')}</span>
                     </div>
                     <h2 className="text-[16px] font-bold text-ide-text leading-snug group-hover:text-primary transition-colors duration-500">{block.title}</h2>
                  </aside>

                  <div className="space-y-8">
                     <p className="text-[16px] leading-[1.8] text-ide-text-dim/90">{block.body}</p>

                     {block.code && (
                        <div className="group/code overflow-hidden rounded-2xl border border-ide-border bg-ide-panel/10 shadow-xl transition-all hover:bg-ide-panel/20">
                           <div className="flex items-center justify-between border-b border-ide-border/50 bg-black/5 px-6 py-3">
                              <span className="text-[10px] font-bold uppercase tracking-[0.3em] text-ide-text-dim/40">
                                 {block.codeLang || "syntax.zt"}
                              </span>
                              <div className="flex gap-2">
                                 <div className="h-1.5 w-1.5 rounded-full bg-primary/20" />
                                 <div className="h-1.5 w-1.5 rounded-full bg-primary/20" />
                              </div>
                           </div>
                           <pre className="p-8 text-[13px] leading-relaxed text-ide-text font-mono overflow-x-auto no-scrollbar">
                              <code>{block.code}</code>
                           </pre>
                        </div>
                     )}

                     {block.bullets && (
                        <div className="grid gap-4">
                           {block.bullets.map((bullet, idx) => (
                              <div key={idx} className="flex gap-4 p-5 rounded-2xl border border-ide-border/50 bg-ide-panel/5 hover:bg-primary/[0.02] transition-colors">
                                 <div className="mt-1 h-1.5 w-1.5 shrink-0 rounded-full bg-primary" />
                                 <div className="text-[14px] leading-relaxed text-ide-text-dim/80">{bullet}</div>
                              </div>
                           ))}
                        </div>
                     )}
                  </div>
                </section>
              ))}
            </div>
          </motion.div>
        </AnimatePresence>
      </main>
    </div>
  );
}

