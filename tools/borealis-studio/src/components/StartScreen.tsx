import { BookOpen, FilePlus2, FolderOpen, Home, LayoutTemplate } from "lucide-react";
import React, { useEffect, useState } from "react";
import { ICON_STROKE } from "../constants";
import type {
  BridgeStatus,
  DocumentationLink,
  NewProjectRequest,
  ProjectTemplate,
  ProjectTemplateId,
  StudioHome,
} from "../types";
import { StatusPill } from "./shared";

export function StartScreen({
  bridgeStatus,
  busy,
  error,
  home,
  onCreateProject,
  onOpenDefaultProject,
  onOpenProject,
}: {
  bridgeStatus: BridgeStatus;
  busy: boolean;
  error: string | null;
  home: StudioHome;
  onCreateProject: (request: NewProjectRequest) => void;
  onOpenDefaultProject: () => void;
  onOpenProject: (projectPath: string) => void;
}) {
  const firstTemplate: ProjectTemplate = home.templates[0] ?? {
    id: "empty3d",
    name: "Empty 3D",
    summary: "Cena 3D limpa com camera e cubo.",
    defaultName: "Projeto Borealis 3D",
    tags: ["3D", "starter"],
  };
  const [projectPath, setProjectPath] = useState(home.defaultProjectPath);
  const [projectName, setProjectName] = useState(firstTemplate.defaultName);
  const [parentDir, setParentDir] = useState(home.defaultProjectsDir);
  const [templateId, setTemplateId] = useState<ProjectTemplateId>(firstTemplate.id);
  const selectedTemplate = home.templates.find((template) => template.id === templateId) ?? firstTemplate;
  const runtimeTone = home.runtimeMode === "missing" ? "warn" : "good";
  const runtimeLabel = home.runtimeMode === "sdk" ? "SDK" : home.runtimeMode === "repo-dev" ? "Dev repo" : "Runtime";

  useEffect(() => {
    setProjectPath(home.defaultProjectPath);
    setParentDir(home.defaultProjectsDir);
  }, [home.defaultProjectPath, home.defaultProjectsDir]);

  useEffect(() => {
    if (!home.templates.some((template) => template.id === templateId)) {
      setTemplateId(firstTemplate.id);
      setProjectName((current) => (current.trim() ? current : firstTemplate.defaultName));
    }
  }, [firstTemplate.defaultName, firstTemplate.id, home.templates, templateId]);

  function chooseTemplate(template: ProjectTemplate) {
    setTemplateId(template.id);
    setProjectName((current) => (current.trim() ? current : template.defaultName));
  }

  return (
    <main className="start-screen">
      <section className="start-hero">
        <div className="start-kicker">
          <Home size={15} strokeWidth={ICON_STROKE} />
          Borealis Studio
        </div>
        <h1>Comece por um projeto real.</h1>
        <p>
          Abra um zenith.ztproj, crie um projeto novo ou use o pacote Borealis como amostra. O preview
          precisa desse caminho correto para iniciar o runner.
        </p>
        <div className="start-actions">
          <button className="start-primary" disabled={busy} onClick={() => void onOpenDefaultProject()} type="button">
            <FolderOpen size={15} strokeWidth={ICON_STROKE} />
            Abrir Borealis
          </button>
          <button className="start-secondary" onClick={() => setProjectPath(home.defaultProjectPath)} type="button">
            Usar caminho padrao
          </button>
        </div>
        <div className="start-meta">
          <StatusPill tone={bridgeStatus === "tauri" ? "good" : "warn"}>
            {bridgeStatus === "tauri" ? "Ponte desktop pronta" : "Fallback browser"}
          </StatusPill>
          <StatusPill tone={runtimeTone}>{runtimeLabel}</StatusPill>
          <span>{home.workspaceRoot}</span>
        </div>
        <div className="start-runtime">
          <span>{home.runtimeStatus}</span>
          {home.sdkRoot ? <small>SDK: {home.sdkRoot}</small> : null}
          {!home.sdkRoot && home.repoRoot ? <small>Repo: {home.repoRoot}</small> : null}
          {home.editorManifest.source ? <small>Editor manifest: {home.editorManifest.source}</small> : null}
        </div>
      </section>

      <section className="start-grid">
        <form
          className="start-panel start-open-panel"
          onSubmit={(event) => {
            event.preventDefault();
            void onOpenProject(projectPath);
          }}
        >
          <div className="start-panel-title">
            <FolderOpen size={16} strokeWidth={ICON_STROKE} />
            <div>
              <strong>Abrir projeto</strong>
              <span>Arquivo zenith.ztproj ou pasta de projeto.</span>
            </div>
          </div>
          <label className="start-field">
            <span>Caminho</span>
            <input value={projectPath} onChange={(event) => setProjectPath(event.target.value)} />
          </label>
          <button className="start-panel-action" disabled={busy} type="submit">
            Abrir
          </button>
        </form>

        <form
          className="start-panel start-new-panel"
          onSubmit={(event) => {
            event.preventDefault();
            void onCreateProject({ projectName, parentDir, templateId });
          }}
        >
          <div className="start-panel-title">
            <FilePlus2 size={16} strokeWidth={ICON_STROKE} />
            <div>
              <strong>Novo projeto</strong>
              <span>Cria pastas, manifesto, cena e script inicial.</span>
            </div>
          </div>
          <label className="start-field">
            <span>Nome</span>
            <input value={projectName} onChange={(event) => setProjectName(event.target.value)} />
          </label>
          <label className="start-field">
            <span>Pasta</span>
            <input value={parentDir} onChange={(event) => setParentDir(event.target.value)} />
          </label>
          <label className="start-field">
            <span>Template</span>
            <select value={templateId} onChange={(event) => setTemplateId(event.target.value as ProjectTemplateId)}>
              {home.templates.map((template) => (
                <option key={template.id} value={template.id}>
                  {template.name}
                </option>
              ))}
            </select>
          </label>
          <p>{selectedTemplate.summary}</p>
          <button className="start-panel-action" disabled={busy} type="submit">
            Criar e abrir
          </button>
        </form>

        <section className="start-panel start-template-panel">
          <div className="start-panel-title">
            <LayoutTemplate size={16} strokeWidth={ICON_STROKE} />
            <div>
              <strong>Templates</strong>
              <span>Escolha a base antes de criar.</span>
            </div>
          </div>
          <div className="template-list">
            {home.templates.map((template) => (
              <button
                className={template.id === templateId ? "selected" : ""}
                key={template.id}
                onClick={() => chooseTemplate(template)}
                type="button"
              >
                <strong>{template.name}</strong>
                <span>{template.summary}</span>
                <small>{template.tags.join(" / ")}</small>
              </button>
            ))}
          </div>
        </section>

        <section className="start-panel start-docs-panel">
          <div className="start-panel-title">
            <BookOpen size={16} strokeWidth={ICON_STROKE} />
            <div>
              <strong>Documentacao</strong>
              <span>Arquivos locais para consulta rapida.</span>
            </div>
          </div>
          <div className="docs-list">
            {home.docs.map((doc) => (
              <DocumentationRow doc={doc} key={doc.path} />
            ))}
          </div>
        </section>
      </section>

      {error ? (
        <div className="start-error" role="alert">
          {error}
        </div>
      ) : null}
    </main>
  );
}

function DocumentationRow({ doc }: { doc: DocumentationLink }) {
  return (
    <div className="doc-row">
      <strong>{doc.title}</strong>
      <span>{doc.summary}</span>
      <code>{doc.path}</code>
    </div>
  );
}
