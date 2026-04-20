import { useEffect } from "react";
import { useWorkspaceStore } from "../store/useWorkspaceStore";
import {
  GitBranch,
  Cpu,
  HardDrive,
} from "lucide-react";
import { BrandLogo } from "./BrandLogo";

export function StatusBar() {
  const {
    cursorPosition,
    gitCurrentBranch,
    systemStats,
    refreshGitBranch,
    refreshSystemStats,
  } = useWorkspaceStore();

  useEffect(() => {
    refreshGitBranch();
    refreshSystemStats();

    const statsInterval = setInterval(() => {
      refreshSystemStats();
    }, 3000);

    const gitInterval = setInterval(() => {
      refreshGitBranch();
    }, 10000);

    return () => {
      clearInterval(statsInterval);
      clearInterval(gitInterval);
    };
  }, [refreshGitBranch, refreshSystemStats]);

  const formatBytes = (bytes: number) => {
    if (!Number.isFinite(bytes) || bytes <= 0) return "0B";
    const units = ["B", "KB", "MB", "GB", "TB"];
    const unitIndex = Math.min(
      Math.floor(Math.log(bytes) / Math.log(1024)),
      units.length - 1,
    );
    return `${(bytes / Math.pow(1024, unitIndex)).toFixed(1)}${units[unitIndex]}`;
  };

  const cpuUsage = typeof systemStats?.cpu === "number" ? systemStats.cpu : 0;
  const totalMemory = typeof systemStats?.memory === "number" ? systemStats.memory : 0;
  const usedMemory = typeof systemStats?.memoryUsed === "number" ? systemStats.memoryUsed : 0;
  const memUsagePercent = totalMemory > 0 ? Math.round((usedMemory / totalMemory) * 100) : 0;
  const cpuUsageLabel = systemStats ? `CPU Usage: ${cpuUsage.toFixed(1)}%` : "CPU Usage unavailable";
  const memoryUsageLabel = systemStats
    ? `Memory: ${formatBytes(usedMemory)} / ${formatBytes(totalMemory)}`
    : "Memory usage unavailable";

  return (
    <footer className="h-7 px-3 bg-ide-panel/80 backdrop-blur-md border-t border-ide-border flex items-center justify-between text-[10px] text-ide-text-dim select-none z-50">
      <div className="flex items-center gap-4 h-full">
        <div className="flex items-center gap-1.5 hover:text-primary transition-colors cursor-pointer group px-1">
          <GitBranch size={12} className="group-hover:rotate-12 transition-transform" />
          <span className="font-medium tracking-wide">{gitCurrentBranch || "main"}</span>
        </div>

        <div className="h-3 w-px bg-ide-border mx-1" />

        <div className="flex items-center gap-3">
          {cursorPosition ? (
            <div className="flex items-center gap-1.5 opacity-80 decoration-primary/30 underline-offset-2 hover:underline cursor-default">
              <span>Ln {cursorPosition.line},</span>
              <span>Col {cursorPosition.col}</span>
            </div>
          ) : (
            <span className="opacity-40 italic">No line selected</span>
          )}
        </div>
      </div>

      <div className="absolute left-1/2 -translate-x-1/2 flex items-center gap-4 opacity-60">
        <div className="flex items-center gap-1.5">
          <div className="w-1.5 h-1.5 rounded-full bg-green-500 animate-pulse" />
          <span className="font-bold tracking-widest uppercase text-[9px]">Zenith Runtime</span>
        </div>
      </div>

      <div className="flex items-center gap-5 h-full">
        <div className="flex items-center gap-4">
          <div className="flex items-center gap-1.5 group cursor-help transition-all hover:text-ide-text" title={cpuUsageLabel}>
            <Cpu size={12} className={cpuUsage > 80 ? "text-orange-500" : ""} />
            <div className="w-12 h-1 bg-ide-border rounded-full overflow-hidden relative">
              <div
                className="absolute left-0 top-0 h-full bg-primary transition-all duration-1000"
                style={{ width: `${cpuUsage}%` }}
              />
            </div>
          </div>

          <div className="flex items-center gap-1.5 group cursor-help transition-all hover:text-ide-text" title={memoryUsageLabel}>
            <HardDrive size={12} className={memUsagePercent > 90 ? "text-red-500" : ""} />
            <div className="w-12 h-1 bg-ide-border rounded-full overflow-hidden relative">
              <div
                className="absolute left-0 top-0 h-full bg-secondary transition-all duration-1000"
                style={{ width: `${memUsagePercent}%` }}
              />
            </div>
          </div>
        </div>

        <div className="h-3 w-px bg-ide-border mx-1" />

        <div className="flex items-center gap-4">
          <span className="font-medium tracking-tighter opacity-80 uppercase">UTF-8</span>

          <div className="flex items-center gap-1.5 hover:text-primary cursor-pointer transition-colors group">
            <BrandLogo variant="icon" className="h-3 w-3 opacity-90 group-hover:scale-110 transition-transform" alt="Zenith" />
            <span className="font-bold tracking-tight">v0.2-alpha</span>
          </div>
        </div>
      </div>
    </footer>
  );
}