import { X, ArrowUpRight, LayoutGrid, Settings2, Shield } from "lucide-react";
import { BrandLogo } from "../BrandLogo";
import { ZenithHubSection, ZenithHubSectionId } from "../../content/zenithHubContent";
import { HubRenderer } from "./HubRenderer";

interface HubShellProps {
  activeSection: ZenithHubSection;
  setActiveSectionId: (id: ZenithHubSectionId) => void;
  onClose: () => void;
  onOpenBrowserManual: () => void;
}

interface Pillar {
  id: string;
  label: string;
  icon: React.ReactNode;
  items: PillarItem[];
}

interface PillarItem {
  id: ZenithHubSectionId;
  label: string;
  kind: "section" | "configuration";
}

const PILLARS: Pillar[] = [
  {
    id: "portal",
    label: "Portal",
    icon: <LayoutGrid size={16} />,
    items: [
      { id: "overview", label: "Dashboard", kind: "section" },
      { id: "marketplace", label: "Marketplace", kind: "section" },
      { id: "about", label: "Sobre", kind: "section" },
    ],
  },
  {
    id: "workbench",
    label: "Workbench",
    icon: <Settings2 size={16} />,
    items: [
      { id: "workbench", label: "Geral", kind: "configuration" },
      { id: "editor-ui", label: "Editor", kind: "configuration" },
      { id: "interface", label: "Interface", kind: "configuration" },
      { id: "keymap", label: "Atalhos", kind: "configuration" },
    ],
  },
  {
    id: "ecosystem",
    label: "Ecossistema",
    icon: <Shield size={16} />,
    items: [
      { id: "manual", label: "Manual", kind: "section" },
      { id: "theme-lab", label: "Theme Lab", kind: "section" },
      { id: "extensions", label: "Extensões", kind: "section" },
    ],
  },
];

export function HubShell({
  activeSection,
  setActiveSectionId,
  onClose,
  onOpenBrowserManual,
}: HubShellProps) {
  return (
    <div className="grid h-[min(880px,calc(100vh-64px))] w-[min(1280px,calc(100vw-64px))] overflow-hidden rounded-2xl border border-ide-border bg-ide-bg shadow-2xl grid-cols-[240px_minmax(0,1fr)] animate-in zoom-in-95 fade-in duration-300">
      {/* Primary Sidebar - Sovereign Shell style */}
      <aside className="flex min-h-0 flex-col border-r border-ide-border bg-ide-panel/60 px-4 py-8 backdrop-blur-2xl">
        <div className="mb-10 px-2">
          <BrandLogo variant="lockup" className="h-4 w-auto opacity-80" alt="ZENITH" />
          <div className="mt-6 flex items-center gap-2">
             <div className="h-1 w-1 rounded-full bg-primary" />
             <div className="text-[9px] font-bold uppercase tracking-[0.2em] text-ide-text-dim/60">Sovereign Shell</div>
          </div>
        </div>

        <nav className="flex-1 space-y-7 overflow-y-auto no-scrollbar">
          {PILLARS.map((pillar) => (
            <div key={pillar.id} className="space-y-2">
              <div className="px-3 text-[8px] font-bold uppercase tracking-[0.25em] text-ide-text-dim/40">
                {pillar.label}
              </div>
              <div className="space-y-0.5">
                {pillar.items.map((item) => {
                  const isActive = activeSection.id === item.id;
                  return (
                    <button
                      key={item.id}
                      type="button"
                      onClick={() => setActiveSectionId(item.id)}
                      className={`group flex w-full items-center gap-3 rounded-xl px-3 py-2 text-left transition-all duration-200 ${
                        isActive
                          ? "bg-primary/10 text-primary ring-1 ring-primary/20"
                          : "text-ide-text-dim hover:text-ide-text hover:bg-white/5"
                      }`}
                    >
                      <div className={`h-1 w-1 rounded-full transition-all ${isActive ? "bg-primary" : "bg-transparent group-hover:bg-ide-text-dim/20"}`} />
                      <span className={`text-[11px] font-semibold tracking-wide ${isActive ? "text-primary" : ""}`}>
                        {item.label}
                      </span>
                    </button>
                  );
                })}
              </div>
            </div>
          ))}
        </nav>

        <div className="mt-8 border-t border-ide-border pt-6 space-y-2">
          <button 
             onClick={onOpenBrowserManual}
             className="flex w-full items-center justify-between rounded-xl border border-ide-border bg-ide-panel/40 px-4 py-2.5 hover:border-primary/30 transition-all active:scale-95"
          >
             <span className="text-[9px] font-bold uppercase tracking-widest text-ide-text-dim">Docs</span>
             <ArrowUpRight size={12} className="opacity-40" />
          </button>
          <div className="px-3 text-[8px] font-bold uppercase tracking-widest text-ide-text-dim/30">
            v0.2.0-alpha
          </div>
        </div>
      </aside>

      {/* Content Area */}
      <div className="flex min-h-0 flex-col bg-ide-bg">
        <header className="flex h-20 items-center justify-between border-b border-ide-border bg-ide-panel/20 px-10">
          <div className="flex flex-col">
            <span className="text-[9px] font-bold uppercase tracking-[0.3em] text-primary/80">{activeSection.eyebrow}</span>
            <h1 className="text-xl font-bold tracking-tight text-white">{activeSection.title}</h1>
          </div>

          <button
            onClick={onClose}
            className="flex h-9 w-9 items-center justify-center rounded-lg border border-ide-border bg-ide-panel/40 text-ide-text-dim hover:text-white transition-all active:scale-90"
          >
            <X size={16} />
          </button>
        </header>

        <main className="min-h-0 flex-1 overflow-y-auto p-10 no-scrollbar">
          <div className="mx-auto w-full max-w-[1000px]">
            <HubRenderer
              activeSection={activeSection}
              activeSectionId={activeSection.id}
              setActiveSectionId={setActiveSectionId}
              onClose={onClose}
              onOpenBrowserManual={onOpenBrowserManual}
            />
          </div>
        </main>
      </div>
    </div>
  );
}
