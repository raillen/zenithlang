import { Circle, Code2, FileCode2, Save, X } from "lucide-react";
import React, { useEffect } from "react";
import { ICON_STROKE } from "../constants";
import type { SceneEntity, ScriptDocument } from "../types";
import { PanelHeader } from "./shared";

export function ScriptEditorWindow({
  activeScript,
  scripts,
  selectedEntity,
  onAttachScript,
  onClose,
  onSaveScript,
  onScriptChange,
  onScriptSelect,
}: {
  activeScript?: ScriptDocument;
  scripts: ScriptDocument[];
  selectedEntity: SceneEntity | null;
  onAttachScript: (path: string) => void;
  onClose: () => void;
  onSaveScript: (script: ScriptDocument) => void;
  onScriptChange: (path: string, content: string) => void;
  onScriptSelect: (path: string) => void;
}) {
  useEffect(() => {
    function onKeyDown(event: KeyboardEvent) {
      if (event.key === "Escape") onClose();
    }

    window.addEventListener("keydown", onKeyDown);
    return () => window.removeEventListener("keydown", onKeyDown);
  }, [onClose]);

  return (
    <div className="script-window-layer" role="dialog" aria-modal="true" aria-label="Script editor">
      <section className="script-window">
        <button className="script-window-close" onClick={onClose} title="Close script editor">
          <X size={16} strokeWidth={ICON_STROKE} />
        </button>
        <CodePanel
          activeScript={activeScript}
          scripts={scripts}
          selectedEntity={selectedEntity}
          onAttachScript={onAttachScript}
          onSaveScript={onSaveScript}
          onScriptChange={onScriptChange}
          onScriptSelect={onScriptSelect}
        />
      </section>
    </div>
  );
}

export function CodePanel({
  activeScript,
  scripts,
  selectedEntity,
  onAttachScript,
  onSaveScript,
  onScriptChange,
  onScriptSelect,
}: {
  activeScript?: ScriptDocument;
  scripts: ScriptDocument[];
  selectedEntity: SceneEntity | null;
  onAttachScript: (path: string) => void;
  onSaveScript: (script: ScriptDocument) => void;
  onScriptChange: (path: string, content: string) => void;
  onScriptSelect: (path: string) => void;
}) {
  if (!activeScript) {
    return (
      <section className="code-panel">
        <PanelHeader icon={<Code2 size={15} strokeWidth={ICON_STROKE} />} title="Editor" meta="No script" />
        <div className="empty-state">No script document is open.</div>
      </section>
    );
  }

  const lines = activeScript.content.split("\n");

  return (
    <section className="code-panel">
      <PanelHeader
        icon={<Code2 size={15} strokeWidth={ICON_STROKE} />}
        title="Script Editor"
        meta={selectedEntity ? selectedEntity.name : "Detached"}
      >
        <button className="panel-action" onClick={() => onSaveScript(activeScript)} title="Save script">
          <Save size={14} strokeWidth={ICON_STROKE} />
          Save
        </button>
      </PanelHeader>
      <div className="script-tabs">
        {scripts.map((script) => (
          <button
            className={script.path === activeScript.path ? "selected" : ""}
            key={script.path}
            onClick={() => onScriptSelect(script.path)}
          >
            <FileCode2 size={13} strokeWidth={ICON_STROKE} />
            {script.name}
            {script.dirty ? <Circle size={7} fill="currentColor" strokeWidth={0} /> : null}
          </button>
        ))}
      </div>
      <div className="script-binding">
        <span>Attached to</span>
        <strong>{selectedEntity?.name ?? "No object"}</strong>
        <select value={activeScript.path} onChange={(event) => onAttachScript(event.target.value)}>
          {scripts.map((script) => (
            <option key={script.path} value={script.path}>
              {script.name}
            </option>
          ))}
        </select>
      </div>
      <div className="code-editor">
        <div className="line-gutter">
          {lines.map((_, index) => (
            <span key={index}>{index + 1}</span>
          ))}
        </div>
        <textarea
          aria-label="Zenith script editor"
          spellCheck={false}
          value={activeScript.content}
          onChange={(event) => onScriptChange(activeScript.path, event.target.value)}
        />
      </div>
    </section>
  );
}
