export type StudioMode = "select" | "move" | "rotate" | "scale";

export type AssetKind = "scene" | "script" | "model" | "texture" | "audio" | "shader" | "cubemap" | "data";

export type BottomTab = "assets" | "console" | "problems";

export type ProjectTemplateId = string;

export type ComponentFieldKind = "asset" | "number" | "text" | "select" | "boolean" | "color";

export interface ComponentFieldSchema {
  key: string;
  label: string;
  kind: ComponentFieldKind;
  options?: string[];
  assetKind?: AssetKind | string;
  min?: number;
  max?: number;
  step?: number;
  default?: unknown;
}

export interface ComponentSchema {
  description: string;
  fields: ComponentFieldSchema[];
  module: string;
  label?: string;
  category?: string;
  viewport?: string;
}

export interface SceneSettingSchema {
  description?: string;
  fields: ComponentFieldSchema[];
  module: string;
  label?: string;
  category?: string;
  viewport?: string;
}

export interface BorealisEditorManifest {
  version?: number;
  source?: string;
  components?: Record<string, ComponentSchema>;
  sceneSettings?: Record<string, SceneSettingSchema>;
  assetKinds?: Record<string, unknown>;
  actions?: Record<string, unknown>;
  templates?: Record<string, unknown>;
}

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

export interface SceneEnvironment {
  skybox: Record<string, unknown>;
  ambient: Record<string, unknown>;
  fog: Record<string, unknown>;
  weather: Record<string, unknown>;
}

export interface SceneRenderSettings {
  quality: Record<string, unknown>;
  postfx: Record<string, unknown>;
  camera: Record<string, unknown>;
}

export interface SceneAudioSettings {
  listener: Record<string, unknown>;
  mix: Record<string, unknown>;
}

export interface SceneDocument {
  name: string;
  path: string;
  documentId: string;
  environment: SceneEnvironment;
  render: SceneRenderSettings;
  audio: SceneAudioSettings;
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
  cameraCount?: number;
  lightCount?: number;
  audioCount?: number;
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
  scene?: string;
  mainScript?: string;
  source?: string;
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
  editorManifest: BorealisEditorManifest;
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

export type BridgeStatus = "loading" | "tauri" | "browser" | "error";

export type ViewMode = "2d" | "3d";
export type ViewProjection = "perspective" | "orthographic" | "isometric";
export type ViewOrientation = "free" | "top" | "bottom" | "left" | "right" | "front" | "back";
export type RenderMode = "wireframe" | "color" | "texture" | "light";
export type ShortcutTemplate = "blender" | "3dsmax" | "maya";
export type ThemeMode = "codex" | "xcode" | "unity-dark";
export type SnapMode = "grid" | "object" | "grid-object";

export interface SnapSettings {
  gridSize: number;
  snapToGrid: boolean;
  snapToObject: boolean;
}

export interface StudioPreferences {
  gizmoSize: number;
  ptzSpeed: number;
  shortcutTemplate: ShortcutTemplate;
  snapMode: SnapMode;
  theme: ThemeMode;
}

export interface LayoutState {
  left: number;
  right: number;
  bottom: number;
}

export interface DragState {
  id: string;
  startX: number;
  startY: number;
  originX: number;
  originY: number;
}

export interface AssetDragState {
  asset: ProjectAsset;
  x: number;
  y: number;
}

export interface ViewportCamera {
  panX: number;
  panY: number;
  zoom: number;
}

export interface ViewportPanState {
  pointerId: number;
  startX: number;
  startY: number;
  originPanX: number;
  originPanY: number;
}

export interface ViewportRendererProps {
  camera: ViewportCamera;
  dragState: DragState | null;
  entities: SceneEntity[];
  mode: StudioMode;
  preferences: StudioPreferences;
  selectedEntityId: string;
  onDragStateChange: (state: DragState | null) => void;
  onEntityPointerDown: (entity: SceneEntity, event: React.PointerEvent<HTMLButtonElement>) => void;
  onEntityPointerMove: (entity: SceneEntity, event: React.PointerEvent<HTMLButtonElement>) => void;
  onResetCamera: () => void;
  onClearSelection: () => void;
  onSelectEntity: (entity: SceneEntity) => void;
  onUpdateTransform: (id: string, transform: Transform3D) => void;
}
