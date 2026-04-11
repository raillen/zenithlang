import { useEffect } from "react";
import { Menu } from "lucide-react";
import { useCommandStore } from "../store/useCommandStore";
import { BrandLogo } from "./BrandLogo";
import { useTranslation } from "../utils/i18n";

export function ZenithMenu() {
  const isHubOpen = useCommandStore((state) => state.isHubOpen);
  const setHubOpen = useCommandStore((state) => state.setHubOpen);
  const { t } = useTranslation();

  useEffect(() => {
    const handleToggleEvent = () => setHubOpen(!useCommandStore.getState().isHubOpen);
    window.addEventListener("zenith://toggle-menu", handleToggleEvent);
    return () => window.removeEventListener("zenith://toggle-menu", handleToggleEvent);
  }, [setHubOpen]);

  return (
    <button
      type="button"
      onClick={() => setHubOpen(!isHubOpen)}
      className={`group flex h-8 items-center gap-2 rounded-lg border px-2.5 transition-all ${
        isHubOpen
          ? "border-primary/20 bg-primary/10 text-primary shadow-sm ring-1 ring-primary/20"
          : "border-transparent text-ide-text-dim hover:bg-black/5 hover:text-ide-text active:scale-95"
      }`}
      title={`${t("hub.title")} Hub`}
      aria-label={`${t("hub.title")} Hub`}
      aria-expanded={isHubOpen}
      aria-haspopup="dialog"
    >
      <BrandLogo
        variant="wordmark"
        className="h-4 w-auto shrink-0 transition-opacity duration-200 group-hover:opacity-100"
        alt={t("hub.title")}
      />
      <span
        className={`h-4 w-px transition-colors ${
          isHubOpen ? "bg-primary/20" : "bg-ide-border/70 group-hover:bg-ide-border"
        }`}
      />
      <span
        className={`flex h-5 w-5 items-center justify-center rounded-md transition-colors ${
          isHubOpen
            ? "bg-primary/10 text-primary"
            : "text-ide-text-dim/80 group-hover:bg-black/5 group-hover:text-ide-text"
        }`}
      >
        <Menu size={13} />
      </span>
    </button>
  );
}
