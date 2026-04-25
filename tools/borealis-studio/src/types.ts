export type StudioMode = "select" | "move" | "rotate" | "scale";

export type AssetKind = "scene" | "script" | "model" | "texture" | "audio" | "data";

export type BottomTab = "assets" | "console" | "problems";

export type ProjectTemplateId = "empty3d" | "topdown2d" | "scripted3d";

export type PreviewStatus =
  | "idle"
  | "starting"
  | "loading"
  | "ready"
  | "playing"
  | "paused"
  | "stopped"
  | "exited"
  | "error"
  | "unavailable";

export interface Transform3D {
  x: number;
  y: number;
  z: number;
  rotationX: number;
  rotationY: number;
  rotationZ: number;
  scaleX: number;
  scaleY: number;
  scaleZ: number;
}

export interface SceneComponent {
  kind: string;
  asset?: string;
  script?: string;
  profile?: string;
  properties?: Record<string, unknown>;
}

export interface SceneEntity {
  id: string;
  name: string;
  layer: string;
  parent?: string;
  tags: string[];
  components: SceneComponent[];
  transform: Transform3D;
}

export interface SceneDocument {
  name: string;
  path: string;
  documentId: string;
  entities: SceneEntity[];
}

export interface ProjectAsset {
  id: string;
  name: string;
  path: string;
  kind: AssetKind;
}

export interface ScriptDocument {
  path: string;
  name: string;
  content: string;
  dirty?: boolean;
}

export interface ConsoleLine {
  id: string;
  level: "info" | "warn" | "error";
  source: "studio" | "preview" | "compiler";
  message: string;
}

export interface PreviewEvent {
  raw: string;
  channel?: string;
  kind?: string;
  status?: string;
  message?: string;
  loaded?: boolean;
  entityCount?: number;
}

export interface PreviewCommandResult {
  status: PreviewStatus | string;
  runner?: string;
  events: PreviewEvent[];
}

export interface ProjectTemplate {
  id: ProjectTemplateId;
  name: string;
  summary: string;
  defaultName: string;
  tags: string[];
}

export interface DocumentationLink {
  title: string;
  path: string;
  summary: string;
}

export interface StudioHome {
  workspaceRoot: string;
  appRoot: string;
  repoRoot?: string;
  sdkRoot?: string;
  runtimeMode: "sdk" | "repo-dev" | "missing" | string;
  runtimeStatus: string;
  defaultProjectPath: string;
  defaultProjectsDir: string;
  templates: ProjectTemplate[];
  docs: DocumentationLink[];
}

export interface NewProjectRequest {
  projectName: string;
  parentDir: string;
  templateId: ProjectTemplateId;
}

export interface StudioSnapshot {
  projectName: string;
  projectPath: string;
  projectRoot: string;
  scene: SceneDocument;
  assets: ProjectAsset[];
  scripts: ScriptDocument[];
  console: ConsoleLine[];
}
