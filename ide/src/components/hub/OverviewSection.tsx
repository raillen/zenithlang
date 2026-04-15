import { useState, useEffect } from "react";
import { Sparkles, FolderOpen, Search, Settings, ArrowUpRight, Cpu, HardDrive, Activity, Bookmark } from "lucide-react";
import { useWorkspaceStore } from "../../store/useWorkspaceStore";
import { HubSectionProps, QuickAction } from "./HubTypes";
import { invoke } from "../../utils/tauri";

interface SysInfo {
  cpu: number;
  memory: number;
  memory_used: number;
}

export function OverviewSection({ setActiveSectionId, onClose }: HubSectionProps) {
  const executeCommand = useWorkspaceStore((state) => state.executeCommand);
  const setPaletteOpen = useWorkspaceStore((state) => state.setPaletteOpen);
  const { currentProjectRoot } = useWorkspaceStore();
  const [sysInfo, setSysInfo] = useState<SysInfo | null>(null);

  useEffect(() => {
    const updateStats = async () => {
      try {
        const info = await invoke<SysInfo>("get_sys_info");
        setSysInfo(info);
      } catch (err) {
        console.error("Failed to fetch system info", err);
      }
    };

    updateStats();
    const interval = setInterval(updateStats, 3000);
    return () => clearInterval(interval);
  }, []);

  const getWorkspaceLabel = (rootPath: string) => {
    if (!rootPath || rootPath === ".") return "Sem workspace";
    const parts = rootPath.split(/[\\/]/).filter(Boolean);
    return parts[parts.length - 1] || rootPath;
  };

  const memUsagePercent = sysInfo ? (sysInfo.memory_used / sysInfo.memory) * 100 : 0;

  const quickActions: QuickAction[] = [
    {
      title: "Explorador Nativo",
      body: "Abrir a raiz do projeto no sistema operacional.",
      icon: <FolderOpen size={20} />,
      onClick: () => {
        invoke("reveal_in_system_explorer", { path: currentProjectRoot });
      },
    },
    {
      title: "Busca Global",
      body: "Localize símbolos ou arquivos em todo o projeto.",
      icon: <Search size={20} />,
      onClick: () => {
        onClose();
        setPaletteOpen(true, ">");
      },
    },
    {
      title: "Atalhos Rápidos",
      body: "Veja todos os comandos e bindings configurados.",
      icon: <Settings size={20} />,
      onClick: () => setActiveSectionId("keymap"),
    },
    {
      title: "Novo Script",
      body: "Crie um novo arquivo Zenith [.zt] agora.",
      icon: <Sparkles size={20} />,
      onClick: () => {
        onClose();
        executeCommand("zenith.files.newFile");
      },
    },
  ];

  return (
    <div className="space-y-12 animate-in fade-in slide-in-from-bottom-2 duration-500">
      {/* Project Pulse Header - Technical and Crisp */}
      <section className="flex flex-col gap-8">
        <div className="flex items-center justify-between">
          <div className="space-y-1">
            <h2 className="text-2xl font-bold tracking-tight text-white flex items-center gap-3">
              Project Pulse
              <div className="h-2 w-2 rounded-full bg-emerald-500 shadow-[0_0_8px_rgba(16,185,129,0.4)]" />
            </h2>
            <p className="text-[12px] text-ide-text-dim/60 font-medium italic">
              Monitoring sovereign environment: {getWorkspaceLabel(currentProjectRoot)}
            </p>
          </div>
          
          <div className="flex gap-4">
            {/* System Mini-Widgets */}
            <div className="flex gap-3">
              <div className="rounded-xl border border-ide-border bg-ide-panel/40 px-4 py-2 min-w-[120px]">
                <div className="flex items-center justify-between mb-1.5">
                  <span className="text-[8px] font-bold uppercase tracking-widest text-ide-text-dim/40">CPU</span>
                  <Cpu size={10} className="text-primary/60" />
                </div>
                <div className="flex items-center gap-3">
                  <span className="text-[14px] font-mono font-bold text-white leading-none">
                    {sysInfo?.cpu.toFixed(0) || "0"}%
                  </span>
                  <div className="flex-1 h-1 bg-white/5 rounded-full overflow-hidden">
                    <div 
                      className="h-full bg-primary transition-all duration-700"
                      style={{ width: `${Math.min(100, sysInfo?.cpu || 0)}%` }}
                    />
                  </div>
                </div>
              </div>

              <div className="rounded-xl border border-ide-border bg-ide-panel/40 px-4 py-2 min-w-[120px]">
                <div className="flex items-center justify-between mb-1.5">
                  <span className="text-[8px] font-bold uppercase tracking-widest text-ide-text-dim/40">RAM</span>
                  <HardDrive size={10} className="text-primary/60" />
                </div>
                <div className="flex items-center gap-3">
                  <span className="text-[14px] font-mono font-bold text-white leading-none">
                    {memUsagePercent.toFixed(0)}%
                  </span>
                  <div className="flex-1 h-1 bg-white/5 rounded-full overflow-hidden">
                    <div 
                      className="h-full bg-primary transition-all duration-700"
                      style={{ width: `${Math.min(100, memUsagePercent)}%` }}
                    />
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>

        <div className="h-px w-full bg-gradient-to-r from-ide-border/50 via-ide-border to-transparent" />
      </section>

      {/* Quick Actions Grid - Higher Density */}
      <section className="grid gap-4 md:grid-cols-2 lg:grid-cols-4">
        {quickActions.map((action, i) => (
          <button
            key={action.title}
            type="button"
            onClick={action.onClick}
            className="group relative flex flex-col rounded-2xl border border-ide-border bg-ide-panel/30 p-6 text-left transition-all duration-300 hover:border-primary/40 hover:bg-ide-panel/50 hover:-translate-y-1 shadow-inner shadow-white/[0.02]"
            style={{ animationDelay: `${i * 50}ms` }}
          >
            <div className="flex h-10 w-10 items-center justify-center rounded-xl border border-ide-border bg-ide-bg text-primary group-hover:bg-primary group-hover:text-white transition-all duration-300">
              {action.icon}
            </div>
            
            <div className="mt-6">
              <h3 className="text-[14px] font-bold text-ide-text flex items-center justify-between group-hover:text-primary transition-colors">
                {action.title}
                <ArrowUpRight size={12} className="opacity-0 group-hover:opacity-100 transition-opacity" />
              </h3>
              <p className="mt-1.5 text-[11px] leading-relaxed text-ide-text-dim/50">
                {action.body}
              </p>
            </div>
          </button>
        ))}
      </section>

      {/* Insight Strip */}
      <section className="rounded-2xl border border-ide-border bg-gradient-to-br from-ide-panel/40 to-transparent p-6 flex items-center justify-between">
        <div className="flex items-center gap-6">
           <div className="flex items-center gap-3">
              <div className="h-8 w-8 rounded-lg bg-emerald-500/10 border border-emerald-500/20 flex items-center justify-center text-emerald-500">
                <Bookmark size={16} />
              </div>
              <div className="flex flex-col">
                <span className="text-[8px] font-bold uppercase tracking-[0.2em] text-ide-text-dim/40">Active Path</span>
                <span className="text-[13px] font-semibold text-ide-text">{getWorkspaceLabel(currentProjectRoot)}</span>
              </div>
           </div>

           <div className="w-px h-8 bg-ide-border" />

           <div className="flex items-center gap-3">
              <div className="h-8 w-8 rounded-lg bg-primary/10 border border-primary/20 flex items-center justify-center text-primary">
                <Activity size={16} />
              </div>
              <div className="flex flex-col">
                <span className="text-[8px] font-bold uppercase tracking-[0.2em] text-ide-text-dim/40">Health</span>
                <span className="text-[13px] font-semibold text-ide-text">Stable Optimization</span>
              </div>
           </div>
        </div>

        <button 
          onClick={() => executeCommand("zenith.workbench.action.openFolder")}
          className="px-5 py-2 rounded-xl border border-ide-border bg-ide-bg text-[10px] font-bold uppercase tracking-widest text-ide-text-dim hover:text-white hover:border-primary/40 transition-all active:scale-95"
        >
          Mudar Raiz
        </button>
      </section>
    </div>
  );
}
