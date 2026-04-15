import { type ReactNode } from "react";
import { type ThemeLabModuleProps } from "./types";
import { 
  applyQuickTokens, 
  applyRegionTokens, 
  getQuickTokens, 
  getRegionTokens
} from "../../themes";

// Reusable Components
export function ColorField({ label, value, onChange, onHover }: { label: string; value: string; onChange: (v: string) => void; onHover?: (region: string | undefined) => void; regionId?: string }) {
  return (
    <div 
      onMouseEnter={() => onHover?.(label.toLowerCase())} 
      onMouseLeave={() => onHover?.(undefined)}
      className="group relative flex items-center justify-between rounded-xl border border-ide-border bg-ide-panel/30 p-2 pl-3 transition-all hover:border-primary/20 hover:bg-ide-panel/50"
    >
      <span className="text-[11px] font-medium text-ide-text-dim group-hover:text-ide-text overflow-hidden text-ellipsis whitespace-nowrap pr-2">{label}</span>
      <div className="relative flex h-7 w-12 shrink-0 items-center justify-center overflow-hidden rounded-lg border border-ide-border group-hover:border-primary/30">
        <input 
          type="color" 
          value={toColorValue(value)} 
          onChange={e => onChange(e.target.value)}
          className="absolute h-[150%] w-[150%] cursor-pointer border-none bg-none p-0 outline-none"
        />
      </div>
    </div>
  );
}

function Section({ title, eyebrow, children }: { title: string; eyebrow: string; children: ReactNode }) {
  return (
    <div className="space-y-4 rounded-3xl border border-ide-border bg-ide-bg/20 p-6">
      <div className="flex flex-col gap-1">
        <div className="text-[10px] font-bold uppercase tracking-[0.2em] text-primary/60">{eyebrow}</div>
        <div className="text-lg font-bold text-ide-text">{title}</div>
      </div>
      <div className="grid gap-3 sm:grid-cols-2 lg:grid-cols-2">{children}</div>
    </div>
  );
}

// Pillar Editors
export function SoulEditor({ theme, onChange, onHover }: ThemeLabModuleProps) {
  const tokens = getQuickTokens(theme);
  return (
    <div className="space-y-8 animate-in fade-in slide-in-from-right-4 duration-500">
      <Section eyebrow="Foundation" title="Dominant Surfaces">
        <ColorField label="Background" value={tokens.surface.background} onChange={v => onChange(c => applyQuickTokens(c, { surface: { background: v } }))} onHover={onHover} />
        <ColorField label="Primary Accent" value={tokens.surface.primary} onChange={v => onChange(c => applyQuickTokens(c, { surface: { primary: v } }))} onHover={onHover} />
        <ColorField label="Main Typography" value={tokens.text.main} onChange={v => onChange(c => applyQuickTokens(c, { text: { main: v } }))} onHover={onHover} />
      </Section>
      
      <Section eyebrow="Code Mood" title="Syntax Essentials">
        <ColorField label="Keywords" value={tokens.syntax.keyword} onChange={v => onChange(c => applyQuickTokens(c, { syntax: { keyword: v } }))} onHover={onHover} />
        <ColorField label="Functions" value={tokens.syntax.function} onChange={v => onChange(c => applyQuickTokens(c, { syntax: { function: v } }))} onHover={onHover} />
        <ColorField label="Strings" value={tokens.syntax.string} onChange={v => onChange(c => applyQuickTokens(c, { syntax: { string: v } }))} onHover={onHover} />
        <ColorField label="Comments" value={tokens.syntax.comment} onChange={v => onChange(c => applyQuickTokens(c, { syntax: { comment: v } }))} onHover={onHover} />
      </Section>
    </div>
  );
}

export function AnatomyEditor({ theme, onChange, onHover }: ThemeLabModuleProps) {
  const tokens = getRegionTokens(theme);
  return (
    <div className="space-y-8 animate-in fade-in slide-in-from-right-4 duration-500 pb-12">
      <Section eyebrow="Chrome" title="Workspace Frames">
        <ColorField label="Toolbar" value={tokens.chrome.toolbarBackground} onChange={v => onChange(c => applyRegionTokens(c, { chrome: { toolbarBackground: v } }))} onHover={onHover} />
        <ColorField label="Sidebar" value={tokens.chrome.sidebarBackground} onChange={v => onChange(c => applyRegionTokens(c, { chrome: { sidebarBackground: v } }))} onHover={onHover} />
        <ColorField label="Inspector" value={tokens.chrome.inspectorBackground} onChange={v => onChange(c => applyRegionTokens(c, { chrome: { inspectorBackground: v } }))} onHover={onHover} />
        <ColorField label="Status Bar" value={tokens.chrome.statusBarBackground} onChange={v => onChange(c => applyRegionTokens(c, { chrome: { statusBarBackground: v } }))} onHover={onHover} />
      </Section>

      <Section eyebrow="Editor" title="Writing Surface">
        <ColorField label="Line Background" value={tokens.editor.activeLineBackground} onChange={v => onChange(c => applyRegionTokens(c, { editor: { activeLineBackground: v } }))} onHover={onHover} />
        <ColorField label="Line Border" value={tokens.editor.activeLineBorder} onChange={v => onChange(c => applyRegionTokens(c, { editor: { activeLineBorder: v } }))} onHover={onHover} />
        <ColorField label="Active Gutter" value={tokens.editor.gutterActiveForeground} onChange={v => onChange(c => applyRegionTokens(c, { editor: { gutterActiveForeground: v } }))} onHover={onHover} />
      </Section>

      <Section eyebrow="Navigation" title="Tabs & Lists">
        <ColorField label="Active Tab" value={tokens.tab.activeBackground} onChange={v => onChange(c => applyRegionTokens(c, { tab: { activeBackground: v } }))} onHover={onHover} />
        <ColorField label="Inactive Tab" value={tokens.tab.inactiveBackground} onChange={v => onChange(c => applyRegionTokens(c, { tab: { inactiveBackground: v } }))} onHover={onHover} />
        <ColorField label="List Selected" value={tokens.list.activeBackground} onChange={v => onChange(c => applyRegionTokens(c, { list: { activeBackground: v } }))} onHover={onHover} />
      </Section>

      <Section eyebrow="Console" title="Terminal Base">
        <ColorField label="Term Background" value={tokens.terminal.background} onChange={v => onChange(c => applyRegionTokens(c, { terminal: { background: v } }))} onHover={onHover} />
        <ColorField label="Term Foreground" value={tokens.terminal.foreground} onChange={v => onChange(c => applyRegionTokens(c, { terminal: { foreground: v } }))} onHover={onHover} />
        <ColorField label="Term Cursor" value={tokens.terminal.cursor} onChange={v => onChange(c => applyRegionTokens(c, { terminal: { cursor: v } }))} onHover={onHover} />
      </Section>
    </div>
  );
}

export function EngineEditor({ theme, onChange, onHover }: ThemeLabModuleProps) {
  return (
    <div className="space-y-8 animate-in fade-in slide-in-from-right-4 duration-500 pb-12">
      <Section eyebrow="Semantic Engine" title="Abstract Surfaces">
        <ColorField label="Canvas" value={theme.tokens.surface.canvas} onChange={v => onChange(c => ({...c, tokens: {...c.tokens, surface: {...c.tokens.surface, canvas: v}}}) )} onHover={onHover} />
        <ColorField label="Panel" value={theme.tokens.surface.panel} onChange={v => onChange(c => ({...c, tokens: {...c.tokens, surface: {...c.tokens.surface, panel: v}}}) )} onHover={onHover} />
        <ColorField label="Elevated" value={theme.tokens.surface.elevated} onChange={v => onChange(c => ({...c, tokens: {...c.tokens, surface: {...c.tokens.surface, elevated: v}}}) )} onHover={onHover} />
        <ColorField label="Subtle" value={theme.tokens.surface.subtle} onChange={v => onChange(c => ({...c, tokens: {...c.tokens, surface: {...c.tokens.surface, subtle: v}}}) )} onHover={onHover} />
      </Section>

      <Section eyebrow="Interactions" title="Borders & Accents">
        <ColorField label="Border Subtle" value={theme.tokens.border.subtle} onChange={v => onChange(c => ({...c, tokens: {...c.tokens, border: {...c.tokens.border, subtle: v}}}) )} onHover={onHover} />
        <ColorField label="Border Strong" value={theme.tokens.border.strong} onChange={v => onChange(c => ({...c, tokens: {...c.tokens, border: {...c.tokens.border, strong: v}}}) )} onHover={onHover} />
        <ColorField label="Focus" value={theme.tokens.border.focus} onChange={v => onChange(c => ({...c, tokens: {...c.tokens, border: {...c.tokens.border, focus: v}}}) )} onHover={onHover} />
        <ColorField label="Accent Base" value={theme.tokens.accent.base} onChange={v => onChange(c => ({...c, tokens: {...c.tokens, accent: {...c.tokens.accent, base: v}}}) )} onHover={onHover} />
      </Section>
    </div>
  );
}

function toColorValue(value: string) {
  if (value.startsWith("#") && (value.length === 4 || value.length === 7)) return value;
  if (value.startsWith("rgba") || value.startsWith("rgb")) {
    const match = value.match(/\d+/g);
    if (match && match.length >= 3) {
      const r = parseInt(match[0]).toString(16).padStart(2, "0");
      const g = parseInt(match[1]).toString(16).padStart(2, "0");
      const b = parseInt(match[2]).toString(16).padStart(2, "0");
      return `#${r}${g}${b}`;
    }
  }
  return "#000000";
}
