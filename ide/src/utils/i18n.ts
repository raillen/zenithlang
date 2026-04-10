import pt from "../i18n/pt.json";
import en from "../i18n/en.json";
import es from "../i18n/es.json";
import { useWorkspaceStore } from "../store/useWorkspaceStore";

const dictionaries: Record<string, any> = { pt, en, es };

/**
 * Translates a key based on the current language in the store.
 * Supports nested keys like 'hub.actions.new_script'
 */
export function t(key: string): string {
  // Use a reactive approach outside but for now we'll pull from store
  const language = useWorkspaceStore.getState().settings?.language || "pt";
  const dictionary = dictionaries[language] || dictionaries.pt;

  const parts = key.split(".");
  let current = dictionary;

  for (const part of parts) {
    if (current && typeof current === "object" && part in current) {
      current = current[part];
    } else {
      return key; // Fallback to the key itself
    }
  }

  return typeof current === "string" ? current : key;
}

/**
 * React hook version for components that need to re-render on language change
 */
export function useTranslation() {
  const language = useWorkspaceStore((state) => state.settings?.language || "pt");
  
  return {
    t,
    language
  };
}
