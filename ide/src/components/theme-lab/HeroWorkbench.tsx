import { type CSSProperties } from "react";
import { type ZenithTheme } from "../../themes";
import { getThemeCssVariables } from "../../utils/themeEngine";
import { motion } from "framer-motion";

interface HeroWorkbenchProps {
  theme: ZenithTheme;
  highlightedRegion?: string;
  onRegionClick?: (region: string) => void;
}

export function HeroWorkbench({ theme, highlightedRegion, onRegionClick }: HeroWorkbenchProps) {
  const style = getThemeCssVariables(theme) as CSSProperties;
  const terminalColors = [
    theme.terminal.red,
    theme.terminal.green,
    theme.terminal.yellow,
    theme.terminal.blue,
    theme.terminal.magenta,
    theme.terminal.cyan,
  ];

  const Region = ({
    id,
    label,
    className,
    children,
  }: {
    id: string;
    label: string;
    className: string;
    children?: React.ReactNode;
  }) => (
    <div
      onClick={() => onRegionClick?.(id)}
      className={`group/region relative cursor-pointer transition-all duration-300 ${className} ${
        highlightedRegion === id ? "ring-2 ring-primary ring-inset z-10" : ""
      } hover:ring-1 hover:ring-primary/40`}
    >
      <div className="absolute left-2 top-2 z-20 rounded-md bg-primary px-2 py-0.5 text-[8px] font-bold uppercase tracking-widest text-white opacity-0 transition-opacity group-hover/region:opacity-100 pointer-events-none">
        {label}
      </div>
      {children}
    </div>
  );

  return (
    <div
      style={style}
      className="group relative h-full w-full overflow-hidden rounded-[32px] border border-[var(--z-border)] bg-[var(--z-bg)] shadow-[0_32px_64px_-16px_rgba(0,0,0,0.5)] transition-all duration-700 hover:shadow-[0_48px_96px_-24px_rgba(0,0,0,0.6)]"
    >
      {/* Decorative toolbar */}
      <Region
        id="chrome"
        label="Chrome / Toolbar"
        className="flex items-center justify-between border-b border-[var(--z-border)] bg-[var(--z-toolbar-bg)] px-5 py-3"
      >
        <div className="flex gap-2">
          <div className="h-2.5 w-2.5 rounded-full bg-red-500/30" />
          <div className="h-2.5 w-2.5 rounded-full bg-yellow-500/30" />
          <div className="h-2.5 w-2.5 rounded-full bg-green-500/30" />
        </div>
        <div className="h-2 w-32 rounded-full bg-[var(--z-text-muted)] opacity-20" />
        <div className="h-5 w-5 rounded bg-[var(--z-primary)] opacity-20" />
      </Region>

      <div className="flex h-[calc(100%-48px)]">
        {/* Sidebar / Rails */}
        <Region
          id="chrome"
          label="Sidebar"
          className="flex w-14 flex-col items-center gap-4 border-r border-[var(--z-border)] bg-[var(--z-sidebar-bg)] py-5"
        >
          <div className="h-8 w-8 rounded-lg bg-[var(--z-primary)] opacity-40" />
          <div className="h-8 w-8 rounded-lg bg-[var(--z-text-muted)] opacity-10" />
          <div className="h-8 w-8 rounded-lg bg-[var(--z-text-muted)] opacity-10" />
          <div className="mt-auto h-8 w-8 rounded-lg bg-[var(--z-text-muted)] opacity-10" />
        </Region>

        {/* Tree / Explorer */}
        <Region
          id="chrome"
          label="Explorer"
          className="w-48 border-r border-[var(--z-border)] bg-[var(--z-sidebar-bg)] p-4"
        >
          <div className="mb-4 h-3 w-20 rounded bg-[var(--z-text-muted)] opacity-30" />
          <div className="space-y-3">
            <div className="flex items-center gap-2">
              <div className="h-3 w-3 rounded bg-blue-400 opacity-40" />
              <div className="h-2 w-24 rounded bg-[var(--z-text-muted)] opacity-20" />
            </div>
            <div className="flex items-center gap-2 rounded-lg bg-[var(--z-list-active-bg)] p-1.5 ring-1 ring-[var(--z-primary)]/20">
              <div className="h-3 w-3 rounded bg-primary" />
              <div className="h-2 w-20 rounded bg-[var(--z-text-main)]" />
            </div>
            {[1, 2, 3].map((i) => (
              <div key={i} className="flex items-center gap-2 px-1.5">
                <div className="h-3 w-3 rounded bg-orange-400 opacity-40" />
                <div className="h-2 w-28 rounded bg-[var(--z-text-muted)] opacity-20" />
              </div>
            ))}
          </div>
        </Region>

        {/* Editor Main */}
        <div className="relative flex flex-1 flex-col overflow-hidden bg-[var(--z-bg)]">
          {/* Tabs */}
          <Region id="tabs" label="Tabs / Navigation" className="flex gap-px border-b border-[var(--z-border)] bg-[var(--z-toolbar-bg)] px-1">
            <div className="flex h-9 items-center gap-3 border-b border-[var(--z-primary)] bg-[var(--z-tab-active-bg)] px-4">
              <div className="h-2 w-16 rounded bg-[var(--z-text-main)]" />
              <div className="h-2 w-2 rounded-full bg-[var(--z-text-muted)] opacity-40" />
            </div>
            <div className="flex h-9 items-center gap-3 bg-[var(--z-tab-inactive-bg)] px-4 opacity-50">
              <div className="h-2 w-12 rounded bg-[var(--z-text-muted)] opacity-40" />
            </div>
          </Region>

          {/* Code Area */}
          <Region id="editor" label="Code Surface" className="relative flex-1 p-6 font-mono text-sm">
             <div className="flex h-full gap-6">
                <div className="w-8 space-y-4 text-right opacity-20">
                   {[10, 11, 12, 13, 14, 15].map(n => <div key={n}>{n}</div>)}
                </div>
                <div className="flex-1 space-y-4">
                   <div className="flex gap-2">
                      <span style={{color: theme.syntax.keyword}}>pub fn</span>
                      <span style={{color: theme.syntax.function}}>main</span>
                      <span>() {"{"}</span>
                   </div>
                   <div className="relative flex h-8 w-full items-center rounded border border-[var(--z-editor-active-line-border)] bg-[var(--z-editor-active-line-bg)] -mx-4 px-4">
                      <div className="ml-4 flex gap-2">
                         <span style={{color: theme.syntax.keyword}}>let</span>
                         <span>alchemy =</span>
                         <span style={{color: theme.syntax.string}}>"power"</span>
                         <span>;</span>
                      </div>
                      <motion.div 
                        initial={{ opacity: 0 }}
                        animate={{ opacity: [0, 1, 0] }}
                        transition={{ duration: 0.8, repeat: Infinity }}
                        className="ml-0.5 h-4 w-0.5 bg-[var(--z-primary)]" 
                      />
                   </div>
                   <div className="ml-4 flex gap-2 opacity-80">
                      <span style={{color: theme.syntax.function}}>apply_filter</span>
                      <span>(alchemy);</span>
                   </div>
                   <div className="opacity-20">{"}"}</div>
                </div>
             </div>
          </Region>

          {/* Terminal / Status Overlay */}
          <motion.div 
             initial={{ y: 20, opacity: 0 }}
             animate={{ y: 0, opacity: 1 }}
             className="absolute bottom-6 left-6 right-6"
          >
             <Region id="terminal" label="Terminal / Console" className="rounded-2xl border border-[var(--z-border)] bg-[var(--z-terminal-bg)] p-4 shadow-2xl">
                <div className="mb-2 flex items-center justify-between opacity-40">
                   <div className="text-[10px] font-bold uppercase tracking-widest text-[var(--z-text-muted)]">Zenith Terminal</div>
                   <div className="h-1.5 w-1.5 rounded-full bg-green-500" />
                </div>
                <div className="flex flex-wrap gap-2">
                   {terminalColors.map((c, i) => (
                      <div key={i} className="h-6 w-10 flex-1 rounded-lg" style={{ background: c }} />
                   ))}
                </div>
             </Region>
          </motion.div>
        </div>
      </div>
      
      {/* Bottom Status Bar */}
      <Region id="chrome" label="Status Bar" className="flex h-6 items-center border-t border-[var(--z-border)] bg-[var(--z-status-bar-bg)] px-3 text-[9px] font-bold uppercase tracking-widest text-white/40">
         <div className="flex flex-1 items-center gap-4">
            <div className="flex items-center gap-1.5">
               <div className="h-1.5 w-1.5 rounded-full bg-[var(--z-primary)]" />
               MAINLOBBY
            </div>
            <div className="opacity-40">UTF-8</div>
         </div>
         <div className="flex items-center gap-3">
            <div className="text-[var(--z-primary)]">ZENITH v0.2</div>
         </div>
      </Region>
    </div>
  );
}
