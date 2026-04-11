import { FileEntry } from "../store/useWorkspaceStore";
import { detectRuntimeProfileFromFile } from "./runtimeProfiles";

export function getEditorLanguageForFile(file: FileEntry | null) {
  if (!file) return "plaintext";

  const name = file.name.toLowerCase();
  const runtimeId = detectRuntimeProfileFromFile(file);

  if (runtimeId === "zenith") return "zenith";
  if (runtimeId === "dotnet") return "csharp";
  if (runtimeId === "rust") return "rust";
  if (runtimeId === "lua") return "lua";
  if (runtimeId === "ruby") return "ruby";

  if (runtimeId === "node") {
    if (
      name.endsWith(".ts") ||
      name.endsWith(".tsx") ||
      name.endsWith(".mts") ||
      name.endsWith(".cts")
    ) {
      return "typescript";
    }

    return "javascript";
  }

  if (name.endsWith(".json")) return "json";
  if (name.endsWith(".md") || name.endsWith(".markdown")) return "markdown";
  if (name.endsWith(".html")) return "html";
  if (name.endsWith(".css")) return "css";
  if (name.endsWith(".toml")) return "toml";
  if (name.endsWith(".yaml") || name.endsWith(".yml")) return "yaml";

  return "plaintext";
}
