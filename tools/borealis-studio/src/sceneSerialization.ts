import { componentSchema, componentValue } from "./borealisCatalog";
import type { SceneComponent, SceneDocument, SceneEntity, Transform3D } from "./types";

export function serializeSceneDocument(scene: SceneDocument): string {
  return `${JSON.stringify(toBorealisScene(scene), null, 2)}\n`;
}

function toBorealisScene(scene: SceneDocument) {
  return {
    version: 1,
    name: scene.name,
    document_id: scene.documentId,
    entities: scene.entities.map(toBorealisEntity),
  };
}

function toBorealisEntity(entity: SceneEntity) {
  return omitEmpty({
    stable_id: entity.id,
    name: entity.name,
    layer: entity.layer,
    parent: entity.parent,
    tags: entity.tags,
    transform: toBorealisTransform(entity.transform),
    components: entity.components.map(toBorealisComponent),
  });
}

function toBorealisTransform(transform: Transform3D) {
  return {
    x: transform.x,
    y: transform.y,
    z: transform.z,
    rotation: transform.rotationZ,
    rotation_x: transform.rotationX,
    rotation_y: transform.rotationY,
    rotation_z: transform.rotationZ,
    scale_x: transform.scaleX,
    scale_y: transform.scaleY,
    scale_z: transform.scaleZ,
  };
}

function toBorealisComponent(component: SceneComponent) {
  const result: Record<string, unknown> = { kind: component.kind };
  const properties = component.properties ?? {};

  for (const [key, value] of Object.entries(properties)) {
    if (key !== "kind" && key !== "type" && value !== undefined) {
      result[key] = value;
    }
  }

  for (const field of componentSchema(component).fields) {
    const value = componentValue(component, field.key);
    if (value === undefined || value === null || value === "") {
      delete result[field.key];
    } else {
      result[field.key] = value;
    }
  }

  return result;
}

function omitEmpty<T extends Record<string, unknown>>(value: T): T {
  const next = { ...value };
  for (const [key, item] of Object.entries(next)) {
    if (item === undefined || item === null || item === "") {
      delete next[key];
    }
  }
  return next;
}
