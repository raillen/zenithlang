import { type ThemePillar, type PillarOption } from "./types";
import { FlaskConical, Sparkles, Box, Settings2, Palette } from "lucide-react";

const PILLARS: PillarOption[] = [
  {
    id: "soul",
    label: "Soul",
    description: "Vibe & core mood",
    icon: <Sparkles size={18} />,
  },
  {
    id: "anatomy",
    label: "Anatomy",
    description: "Regional functional areas",
    icon: <Box size={18} />,
  },
  {
    id: "engine",
    label: "Engine",
    description: "Deep semantic tokens",
    icon: <Settings2 size={18} />,
  },
  {
    id: "gallery",
    label: "Gallery",
    description: "Presets & history",
    icon: <Palette size={18} />,
  },
];

interface PillarsProps {
  activePillar: ThemePillar;
  onChange: (pillar: ThemePillar) => void;
}

export function Pillars({ activePillar, onChange }: PillarsProps) {
  return (
    <div className="flex w-full flex-col gap-3">
      <div className="mb-4 px-2">
         <div className="inline-flex items-center gap-2 rounded-full border border-primary/20 bg-primary/10 px-3 py-1 text-[10px] font-bold uppercase tracking-[0.2em] text-primary">
            <FlaskConical size={12} />
            Lab Engine
         </div>
      </div>
      
      {PILLARS.map((pillar) => (
        <button
          key={pillar.id}
          onClick={() => onChange(pillar.id)}
          className={`group flex w-full items-center gap-4 rounded-2xl border p-4 text-left transition-all duration-300 ${
            activePillar === pillar.id
              ? "border-primary/40 bg-primary/10 shadow-lg shadow-primary/10"
              : "border-ide-border bg-ide-bg hover:border-primary/20 hover:bg-ide-panel/30"
          }`}
        >
          <div
            className={`flex h-12 w-12 items-center justify-center rounded-xl border transition-all duration-300 ${
              activePillar === pillar.id
                ? "border-primary/30 bg-primary text-white"
                : "border-ide-border bg-ide-panel text-ide-text-dim group-hover:border-primary/30 group-hover:text-primary"
            }`}
          >
            {pillar.icon}
          </div>
          <div className="flex-1">
            <div
              className={`text-[12px] font-bold uppercase tracking-[0.16em] transition-colors ${
                activePillar === pillar.id ? "text-primary" : "text-ide-text group-hover:text-primary"
              }`}
            >
              {pillar.label}
            </div>
            <div className="mt-1 text-[11px] font-medium leading-relaxed text-ide-text-dim opacity-70">
              {pillar.description}
            </div>
          </div>
          {activePillar === pillar.id && (
            <div className="h-1.5 w-1.5 rounded-full bg-primary animate-pulse" />
          )}
        </button>
      ))}
    </div>
  );
}
