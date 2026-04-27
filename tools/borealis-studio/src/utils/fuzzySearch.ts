export interface SearchResult {
  id: string;
  label: string;
  category: "entity" | "asset" | "script" | "command";
  detail?: string;
  score: number;
}

export function fuzzyMatch(query: string, target: string): number {
  if (!query) return 1;
  const q = query.toLowerCase();
  const t = target.toLowerCase();
  if (t === q) return 100;
  if (t.startsWith(q)) return 80;
  if (t.includes(q)) return 60;

  let qi = 0;
  let consecutive = 0;
  let maxConsecutive = 0;
  let matchCount = 0;

  for (let ti = 0; ti < t.length && qi < q.length; ti++) {
    if (t[ti] === q[qi]) {
      qi++;
      matchCount++;
      consecutive++;
      maxConsecutive = Math.max(maxConsecutive, consecutive);
    } else {
      consecutive = 0;
    }
  }

  if (qi < q.length) return 0;
  return 20 + maxConsecutive * 10 + (matchCount / t.length) * 10;
}

export function searchAll(
  query: string,
  entities: { id: string; name: string }[],
  assets: { id: string; name: string; path: string; kind: string }[],
  scripts: { path: string; name: string }[],
  commands: { id: string; label: string }[],
): SearchResult[] {
  const results: SearchResult[] = [];

  for (const entity of entities) {
    const score = fuzzyMatch(query, entity.name);
    if (score > 0) {
      results.push({
        id: `entity:${entity.id}`,
        label: entity.name,
        category: "entity",
        detail: "Entity",
        score,
      });
    }
  }

  for (const asset of assets) {
    const score = Math.max(fuzzyMatch(query, asset.name), fuzzyMatch(query, asset.path) * 0.8);
    if (score > 0) {
      results.push({
        id: `asset:${asset.id}`,
        label: asset.name,
        category: "asset",
        detail: asset.kind,
        score,
      });
    }
  }

  for (const script of scripts) {
    const score = Math.max(fuzzyMatch(query, script.name), fuzzyMatch(query, script.path) * 0.8);
    if (score > 0) {
      results.push({
        id: `script:${script.path}`,
        label: script.name,
        category: "script",
        detail: script.path,
        score,
      });
    }
  }

  for (const cmd of commands) {
    const score = fuzzyMatch(query, cmd.label);
    if (score > 0) {
      results.push({
        id: `command:${cmd.id}`,
        label: cmd.label,
        category: "command",
        detail: "Command",
        score,
      });
    }
  }

  results.sort((a, b) => b.score - a.score);
  return results.slice(0, 20);
}

export const STUDIO_COMMANDS = [
  { id: "save", label: "Save Scene" },
  { id: "undo", label: "Undo" },
  { id: "redo", label: "Redo" },
  { id: "select-all", label: "Select All Entities" },
  { id: "deselect-all", label: "Deselect All" },
  { id: "delete-selected", label: "Delete Selected" },
  { id: "duplicate-selected", label: "Duplicate Selected" },
  { id: "add-entity", label: "Add Empty Entity" },
  { id: "toggle-2d", label: "Switch to 2D Viewport" },
  { id: "toggle-3d", label: "Switch to 3D Viewport" },
  { id: "start-preview", label: "Start Preview" },
  { id: "stop-preview", label: "Stop Preview" },
  { id: "open-scripts", label: "Open Script Editor" },
  { id: "toggle-settings", label: "Open Settings" },
];
