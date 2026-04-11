import { useEffect, useMemo, useState } from "react";
import { createPortal } from "react-dom";
import {
  ArrowUpRight,
  BookOpen,
  FolderOpen,
  Keyboard,
  LayoutGrid,
  Palette,
  Search,
  Settings,
  Sparkles,
  Wrench,
  X,
} from "lucide-react";
import { BrandLogo } from "./BrandLogo";
import { useCommandStore } from "../store/useCommandStore";
import { useWorkspaceStore } from "../store/useWorkspaceStore";
import {
  buildZenithHubBrowserDocument,
  getZenithHubSection,
  type ZenithHubSectionId,
  ZENITH_HUB_SECTIONS,
} from "../content/zenithHubContent";

const HUB_DEFAULT_SECTION: ZenithHubSectionId = "overview";

export function ZenithHub() {
  const isHubOpen = useCommandStore((state) => state.isHubOpen);
  const setHubOpen = useCommandStore((state) => state.setHubOpen);
  const setPaletteOpen = useCommandStore((state) => state.setPaletteOpen);
  const setSettingsOpen = useCommandStore((state) => state.setSettingsOpen);
  const executeCommand = useCommandStore((state) => state.executeCommand);
  const { currentProjectRoot, activeFile, setSidebarOpen, setSidebarTab } = useWorkspaceStore();
  const [activeSectionId, setActiveSectionId] = useState<ZenithHubSectionId>(HUB_DEFAULT_SECTION);

  useEffect(() => {
    if (!isHubOpen) return;

    const handleKeyDown = (event: KeyboardEvent) => {
      if (event.key === "Escape") {
        event.preventDefault();
        setHubOpen(false);
      }
    };

    window.addEventListener("keydown", handleKeyDown);
    return () => window.removeEventListener("keydown", handleKeyDown);
  }, [isHubOpen, setHubOpen]);

  useEffect(() => {
    if (!isHubOpen) return;
    setActiveSectionId((current) => current || HUB_DEFAULT_SECTION);
  }, [isHubOpen]);

  const activeSection = useMemo(
    () => getZenithHubSection(activeSectionId),
    [activeSectionId]
  );

  if (!isHubOpen) return null;

  const closeHub = () => setHubOpen(false);

  const openQuickOpen = () => {
    setHubOpen(false);
    setPaletteOpen(true, "");
  };

  const openCommands = () => {
    setHubOpen(false);
    setPaletteOpen(true, ">");
  };

  const openSettings = (tab: "general" | "editor" | "extensions") => {
    setHubOpen(false);
    setSettingsOpen(true, tab);
  };

  const focusWorkspaceContexts = () => {
    setHubOpen(false);
    setSidebarTab("contexts");
    setSidebarOpen(true);
  };

  const openBrowserManual = () => {
    const html = buildZenithHubBrowserDocument(activeSectionId);
    const blob = new Blob([html], { type: "text/html" });
    const url = URL.createObjectURL(blob);
    window.open(`${url}#${activeSectionId}`, "_blank", "noopener,noreferrer");
    window.setTimeout(() => URL.revokeObjectURL(url), 60_000);
  };

  const quickActions = [
    {
      title: "Novo arquivo",
      body: "Abre o fluxo de criacao direto no explorer.",
      icon: <Sparkles size={16} />,
      onClick: () => {
        setHubOpen(false);
        executeCommand("zenith.files.newFile");
      },
    },
    {
      title: "Abrir pasta",
      body: "Troca a raiz do workspace e reindexa o projeto.",
      icon: <FolderOpen size={16} />,
      onClick: () => {
        setHubOpen(false);
        executeCommand("zenith.workbench.action.openFolder");
      },
    },
    {
      title: "Quick Open",
      body: "Vai direto para arquivos pelo nome.",
      icon: <Search size={16} />,
      onClick: openQuickOpen,
    },
    {
      title: "Preferencias",
      body: "Abre configuracoes gerais da IDE.",
      icon: <Settings size={16} />,
      onClick: () => openSettings("general"),
    },
    {
      title: "Temas",
      body: "Ajusta tema, tipografia e escala da interface.",
      icon: <Palette size={16} />,
      onClick: () => openSettings("editor"),
    },
    {
      title: "Extensoes",
      body: "Configura toolchains, build e LSP por runtime.",
      icon: <Wrench size={16} />,
      onClick: () => openSettings("extensions"),
    },
    {
      title: "Contextos",
      body: "Abre a area de workspaces e snapshots salvos.",
      icon: <LayoutGrid size={16} />,
      onClick: focusWorkspaceContexts,
    },
    {
      title: "Comandos",
      body: "Abre a command palette no modo de comandos.",
      icon: <Keyboard size={16} />,
      onClick: openCommands,
    },
  ];

  const primaryQuickActions = quickActions.slice(0, 4);
  const secondaryQuickActions = quickActions.slice(4);

  const modalContent = (
    <div
      className="fixed inset-0 z-[99998] flex items-center justify-center bg-black/25 backdrop-blur-sm"
      onClick={closeHub}
    >
      <div
        className="grid h-[min(920px,calc(100vh-28px))] w-[min(1400px,calc(100vw-28px))] grid-cols-[220px_minmax(0,1fr)] overflow-hidden rounded-2xl border border-ide-border bg-ide-bg shadow-[0_20px_60px_-30px_rgba(0,0,0,0.35)]"
        onClick={(event) => event.stopPropagation()}
      >
        <aside className="flex min-h-0 flex-col border-r border-ide-border bg-ide-panel/90 px-4 py-4 backdrop-blur-xl">
          <div className="border-b border-ide-border px-2 pb-4">
            <BrandLogo variant="wordmark" className="h-5 w-auto" alt="ZENITH KETER" />
            <div className="mt-4 text-[10px] font-bold uppercase tracking-[0.2em] text-ide-text-dim">
              Zenith Hub
            </div>
            <p className="mt-2 text-[11px] leading-relaxed text-ide-text-dim">
              Manual offline e atalhos de contexto.
            </p>
          </div>

          <nav className="flex-1 overflow-y-auto py-4">
            <div className="space-y-1">
              {ZENITH_HUB_SECTIONS.map((section) => (
                <button
                  key={section.id}
                  type="button"
                  onClick={() => setActiveSectionId(section.id)}
                  className={`flex w-full items-center justify-between rounded-lg border px-3 py-2 text-left transition-all ${
                    activeSectionId === section.id
                      ? "border-primary/20 bg-primary/10 text-primary"
                      : "border-transparent text-ide-text-dim hover:bg-black/5 hover:text-ide-text"
                  }`}
                >
                  <span className="text-[10px] font-bold uppercase tracking-[0.18em]">
                    {section.navLabel}
                  </span>
                  {activeSectionId === section.id && <span className="h-1.5 w-1.5 rounded-full bg-current" />}
                </button>
              ))}
            </div>
          </nav>

          <div className="border-t border-ide-border px-2 pt-4">
            <button
              type="button"
              onClick={openBrowserManual}
              className="flex w-full items-center justify-between rounded-lg border border-ide-border bg-ide-bg/80 px-3 py-2 text-left text-[10px] font-bold uppercase tracking-[0.18em] text-ide-text-dim transition-colors hover:border-ide-text/20 hover:text-ide-text"
            >
              <span>Ver no navegador</span>
              <ArrowUpRight size={13} />
            </button>
          </div>
        </aside>

        <div className="flex min-h-0 flex-col bg-ide-bg">
          <header className="flex h-14 items-center justify-between border-b border-ide-border px-6">
            <div className="min-w-0">
              <div className="text-[10px] font-bold uppercase tracking-[0.18em] text-primary">
                {activeSection.eyebrow}
              </div>
              <div className="mt-1 text-[16px] font-semibold text-ide-text">
                {activeSection.title}
              </div>
            </div>

            <div className="flex items-center gap-2">
              <button
                type="button"
                onClick={openBrowserManual}
                className="hidden md:flex items-center gap-2 rounded-lg border border-ide-border bg-ide-panel/60 px-3 py-2 text-[10px] font-bold uppercase tracking-[0.18em] text-ide-text-dim transition-colors hover:border-ide-text/20 hover:text-ide-text"
              >
                <BookOpen size={13} />
                Ver no navegador
              </button>
              <button
                type="button"
                onClick={closeHub}
                className="flex h-8 w-8 items-center justify-center rounded-lg text-ide-text-dim transition-colors hover:bg-black/5 hover:text-ide-text"
                title="Fechar hub"
              >
                <X size={16} />
              </button>
            </div>
          </header>

          <div className="min-h-0 flex-1 overflow-y-auto px-6 py-6">
            <div className="mx-auto max-w-4xl">
            <section className="border-b border-ide-border pb-5">
              <div className="max-w-2xl">
                <p className="text-[15px] leading-relaxed text-ide-text-dim">
                  {activeSection.summary}
                </p>
              </div>

              <div className="mt-5 flex flex-wrap gap-2">
                {activeSection.highlights.map((highlight) => (
                  <div
                    key={highlight.label}
                    className="rounded-full border border-ide-border bg-ide-panel/45 px-3 py-1.5"
                  >
                    <span className="text-[9px] font-bold uppercase tracking-[0.16em] text-ide-text-dim">
                      {highlight.label}
                    </span>
                    <span className="mx-2 text-ide-text-dim/35">/</span>
                    <span className="text-[11px] font-semibold text-ide-text">{highlight.value}</span>
                  </div>
                ))}
              </div>
            </section>

            {activeSection.id === "overview" && (
              <section className="border-b border-ide-border py-6">
                <div className="flex items-center justify-between gap-4">
                  <div>
                    <div className="text-[10px] font-bold uppercase tracking-[0.18em] text-primary">
                      Actions
                    </div>
                    <h3 className="mt-2 text-[18px] font-semibold text-ide-text">
                      Acessos rapidos do workbench
                    </h3>
                  </div>
                  <div className="text-[10px] uppercase tracking-[0.18em] text-ide-text-dim">
                    {getWorkspaceLabel(currentProjectRoot)}
                    {activeFile ? ` / ${activeFile.name}` : ""}
                  </div>
                </div>

                <div className="mt-5 flex flex-wrap items-center gap-x-4 gap-y-2 rounded-lg border border-ide-border bg-ide-panel/35 px-4 py-3 text-[11px] text-ide-text-dim">
                  <span className="font-semibold text-ide-text">
                    {currentProjectRoot === "." ? "No workspace loaded" : currentProjectRoot}
                  </span>
                  <span className="text-ide-text-dim/35">/</span>
                  <span>{activeFile?.path || "No file selected"}</span>
                  <span className="text-ide-text-dim/35">/</span>
                  <span>Offline inside the IDE</span>
                </div>

                <div className="mt-5 grid gap-2 md:grid-cols-2 xl:grid-cols-4">
                  {primaryQuickActions.map((action) => (
                    <button
                      key={action.title}
                      type="button"
                      onClick={action.onClick}
                      className="rounded-lg border border-ide-border bg-ide-panel/35 px-4 py-3 text-left transition-colors hover:border-primary/20 hover:bg-primary/5"
                    >
                      <div className="flex h-7 w-7 items-center justify-center rounded-lg border border-ide-border bg-ide-bg/80 text-primary">
                        {action.icon}
                      </div>
                      <div className="mt-3 text-[12px] font-semibold text-ide-text">
                        {action.title}
                      </div>
                      <p className="mt-1.5 text-[10px] leading-relaxed text-ide-text-dim">
                        {action.body}
                      </p>
                    </button>
                  ))}
                </div>

                <div className="mt-4 flex flex-wrap gap-2">
                  {secondaryQuickActions.map((action) => (
                    <button
                      key={action.title}
                      type="button"
                      onClick={action.onClick}
                      className="rounded-full border border-ide-border bg-ide-bg/80 px-3 py-1.5 text-[10px] font-bold uppercase tracking-[0.16em] text-ide-text-dim transition-colors hover:border-ide-text/20 hover:text-ide-text"
                    >
                      {action.title}
                    </button>
                  ))}
                </div>
              </section>
            )}

            <section className="py-6">
              <div className="text-[10px] font-bold uppercase tracking-[0.18em] text-primary">
                Manual offline
              </div>
              <div className="mt-4 divide-y divide-ide-border">
                {activeSection.blocks.map((block) => (
                  <article
                    key={block.title}
                    className="py-4 first:pt-0 last:pb-0"
                  >
                    <h3 className="text-[14px] font-semibold text-ide-text">
                      {block.title}
                    </h3>
                    <p className="mt-2 max-w-3xl text-[13px] leading-relaxed text-ide-text-dim">
                      {block.body}
                    </p>
                    {block.bullets?.length ? (
                      <ul className="mt-3 space-y-1.5 pl-4 text-[12px] leading-relaxed text-ide-text-dim">
                        {block.bullets.map((item) => (
                          <li key={item}>{item}</li>
                        ))}
                      </ul>
                    ) : null}
                  </article>
                ))}
              </div>
            </section>

            {activeSection.id !== "overview" && (
              <section className="border-t border-ide-border pt-5">
                <div className="text-[10px] font-bold uppercase tracking-[0.18em] text-ide-text-dim">
                  Outras secoes
                </div>
                <div className="mt-3 flex flex-wrap gap-2">
                  {ZENITH_HUB_SECTIONS.filter((section) => section.id !== activeSection.id)
                    .slice(0, 3)
                    .map((section) => (
                      <button
                        key={section.id}
                        type="button"
                        onClick={() => setActiveSectionId(section.id)}
                        className="rounded-full border border-ide-border bg-ide-bg/80 px-3 py-1.5 text-[10px] font-bold uppercase tracking-[0.16em] text-ide-text-dim transition-colors hover:border-ide-text/20 hover:text-ide-text"
                      >
                        {section.navLabel}
                      </button>
                    ))}
                </div>
              </section>
            )}
            </div>
          </div>
        </div>
      </div>
    </div>
  );

  return createPortal(modalContent, document.body);
}

function getWorkspaceLabel(rootPath: string) {
  if (!rootPath || rootPath === ".") return "No workspace";
  const parts = rootPath.split(/[\\/]/).filter(Boolean);
  return parts[parts.length - 1] || rootPath;
}
