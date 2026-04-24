export type StudioMode = "select" | "move" | "rotate" | "scale";

export type AssetKind = "scene" | "script" | "model" | "texture" | "audio" | "data";

export type ViewTab = "scene" | "game" | "script";

export type BottomTab = "assets" | "console" | "problems";

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

export interface StudioSnapshot {
  projectName: string;
  projectPath: string;
  scene: SceneDocument;
  assets: ProjectAsset[];
  scripts: ScriptDocument[];
  console: ConsoleLine[];
}
