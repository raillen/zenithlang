import { Info } from "lucide-react";
import { HubSectionProps } from "./HubTypes";

export function DocSection({ activeSection }: HubSectionProps) {
  return (
    <div className="space-y-6 animate-in fade-in duration-500 h-full flex flex-col">
      <header className="flex items-center justify-between gap-4 sticky top-0 bg-ide-bg/80 backdrop-blur-md pb-6 z-10 border-b border-ide-border shrink-0">
        <div className="flex items-center gap-3">
          <div className="p-2 rounded-lg bg-primary/10 text-primary">
            <Info size={18} />
          </div>
          <div>
            <h3 className="text-[14px] font-bold text-ide-text">{activeSection.title}</h3>
            <p className="text-[11px] text-ide-text-dim text-nowrap">{activeSection.summary || "Documentação do sistema"}</p>
          </div>
        </div>
      </header>

      <div className="bg-ide-panel/30 border border-ide-border rounded-2xl overflow-hidden divide-y divide-ide-border/50 overflow-y-auto min-h-0">
        {activeSection.blocks.map((block) => (
          <article key={block.title} className="p-6 transition-colors hover:bg-black/5">
            <h3 className="text-[14px] font-bold text-ide-text flex items-center gap-2">
              {block.title}
            </h3>
            <p className="mt-2 max-w-3xl text-[12px] leading-relaxed text-ide-text-dim">
              {block.body}
            </p>
            {block.bullets?.length ? (
              <ul className="mt-4 space-y-2 pl-2 text-[12px] leading-relaxed text-ide-text-dim">
                {block.bullets.map((item) => (
                  <li key={item} className="flex gap-2 items-start">
                    <span className="mt-1.5 h-1.5 w-1.5 shrink-0 rounded-full bg-primary/50" />
                    <span>{item}</span>
                  </li>
                ))}
              </ul>
            ) : null}
          </article>
        ))}
      </div>
    </div>
  );
}
