import { useEffect, useMemo } from "react";
import { emit, listen } from "@tauri-apps/api/event";
import { getCurrentWindow } from "@tauri-apps/api/window";
import { useWorkspaceStore } from "../store/useWorkspaceStore";
import { resolveThemeById, type ZenithTheme } from "../themes";
import { applyThemeToDOM } from "../utils/themeEngine";

interface ThemeSyncPayload {
  theme: ZenithTheme;
  id: string;
  source: string;
}

export function useThemeSync() {
  const currentLabel = useMemo(() => {
    try {
      return getCurrentWindow().label;
    } catch {
      return "browser";
    }
  }, []);

  useEffect(() => {
    const unlistenPromise = listen<ThemeSyncPayload>("zenith://theme-updated", async (event) => {
      const { theme, id, source } = event.payload;
      if (source === currentLabel) return;

      const current = useWorkspaceStore.getState().settings;
      const nextSettings = {
        ...current,
        customThemes: {
          ...current.customThemes,
          [id]: theme,
        },
        theme: id,
      };

      useWorkspaceStore.setState({ settings: nextSettings });
      applyThemeToDOM(resolveThemeById(id, nextSettings.customThemes));
    });

    return () => {
      void unlistenPromise.then((unlisten) => unlisten());
    };
  }, [currentLabel]);

  const broadcastThemeUpdate = async (theme: ZenithTheme, id: string) => {
    await emit("zenith://theme-updated", {
      theme,
      id,
      source: currentLabel,
    } satisfies ThemeSyncPayload);
  };

  return { broadcastThemeUpdate, currentLabel };
}
