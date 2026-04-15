import { ArrowLeft, X } from "lucide-react";
import { BrandLogo } from "../BrandLogo";
import { ThemeLab } from "../ThemeLab";

interface ThemeLabSectionProps {
  title: string;
  summary: string;
  onBack: () => void;
  onClose: () => void;
  onOpenEditorSettings: () => void;
}

export function ThemeLabSection({
  title,
  summary,
  onBack,
  onClose,
  onOpenEditorSettings,
}: ThemeLabSectionProps) {
  return (
    <div className="flex h-[calc(100vh-20px)] w-[calc(100vw-20px)] min-h-0 flex-col bg-ide-bg rounded-2xl overflow-hidden border border-ide-border shadow-2xl transition-all duration-300">
      <header className="flex h-14 items-center justify-between border-b border-ide-border bg-ide-panel/70 px-5 backdrop-blur-xl">
        <div className="flex min-w-0 items-center gap-3">
          <button
            type="button"
            onClick={onBack}
            className="flex h-8 w-8 items-center justify-center rounded-lg border border-ide-border bg-ide-bg/80 text-ide-text-dim transition-colors hover:border-ide-text/20 hover:text-ide-text"
            title="Voltar ao Hub"
          >
            <ArrowLeft size={16} />
          </button>
          <BrandLogo variant="wordmark" className="h-4 w-auto shrink-0" alt="ZENITH KETER" />
          <div className="min-w-0">
            <div className="text-[10px] font-bold uppercase tracking-[0.18em] text-primary">
              Theme Lab Workspace
            </div>
            <div className="mt-0.5 truncate text-[13px] text-ide-text-dim">
              {title} / {summary}
            </div>
          </div>
        </div>

        <div className="flex items-center gap-2">
          <button
            type="button"
            onClick={onBack}
            className="rounded-lg border border-ide-border bg-ide-bg/80 px-3 py-2 text-[10px] font-bold uppercase tracking-[0.16em] text-ide-text-dim transition-colors hover:border-ide-text/20 hover:text-ide-text"
          >
            Sair do Lab
          </button>
          <button
            type="button"
            onClick={onClose}
            className="flex h-8 w-8 items-center justify-center rounded-lg text-ide-text-dim transition-colors hover:bg-black/5 hover:text-ide-text"
            title="Fechar Theme Lab"
          >
            <X size={16} />
          </button>
        </div>
      </header>

      <div className="min-h-0 flex-1 overflow-hidden px-5 py-5">
        <ThemeLab onOpenEditorSettings={onOpenEditorSettings} fullScreen />
      </div>
    </div>
  );
}
