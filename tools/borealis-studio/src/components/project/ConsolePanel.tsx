import { AlertTriangle, CheckCircle2, Terminal, Trash2 } from "lucide-react";
import React, { useMemo, useState } from "react";
import { ICON_STROKE } from "../../constants";
import type { ConsoleLine } from "../../types";

export function ConsolePanel({ lines, onClear }: { lines: ConsoleLine[]; onClear: () => void }) {
  const [filter, setFilter] = useState<"all" | "info" | "warn" | "error">("all");
  const [source, setSource] = useState<"all" | "studio" | "preview" | "compiler" | "script">("all");

  const filtered = useMemo(
    () =>
      lines.filter((line) => {
        if (filter !== "all" && line.level !== filter) return false;
        if (source !== "all" && line.source !== source) return false;
        return true;
      }),
    [filter, lines, source],
  );

  const warningCount = lines.filter((line) => line.level === "warn").length;
  const errorCount = lines.filter((line) => line.level === "error").length;

  return (
    <div className="console-shell">
      <div className="console-toolbar-unity">
        <div className="console-toolbar-left">
          <span className="console-toolbar-chip">
            <Terminal size={14} strokeWidth={ICON_STROKE} /> Console
          </span>
          <span className="console-toolbar-chip warn">
            <AlertTriangle size={14} strokeWidth={ICON_STROKE} /> {warningCount} warnings
          </span>
          <span className="console-toolbar-chip error">
            <AlertTriangle size={14} strokeWidth={ICON_STROKE} /> {errorCount} errors
          </span>
        </div>
        <div className="console-toolbar-right">
          <select onChange={(event) => setFilter(event.target.value as typeof filter)} value={filter}>
            <option value="all">All levels</option>
            <option value="info">Info</option>
            <option value="warn">Warnings</option>
            <option value="error">Errors</option>
          </select>
          <select onChange={(event) => setSource(event.target.value as typeof source)} value={source}>
            <option value="all">All sources</option>
            <option value="studio">Studio</option>
            <option value="preview">Preview</option>
            <option value="compiler">Compiler</option>
            <option value="script">Script</option>
          </select>
          <button onClick={onClear} type="button">
            <Trash2 size={14} strokeWidth={ICON_STROKE} />
          </button>
        </div>
      </div>
      <div className="console-list-unity">
        {filtered.length === 0 ? (
          <div className="empty-state inline">
            <CheckCircle2 size={16} strokeWidth={ICON_STROKE} />
            No console lines for this filter.
          </div>
        ) : (
          filtered.map((line) => (
            <div className={`console-line-unity ${line.level}`} key={line.id}>
              <span className="console-line-level">{line.level.toUpperCase()}</span>
              <span className="console-line-source">{line.source}</span>
              <p>{line.message}</p>
            </div>
          ))
        )}
      </div>
    </div>
  );
}
