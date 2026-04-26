use std::fs;
use std::path::Path;

use serde_json::Value;

use crate::models::scene::{SceneComponent, SceneDocument, SceneEntity, Transform3d};
use crate::utils::json::{number_value, text_value};
use crate::utils::paths::normalize_path;

pub fn read_scene_document(project_root: &Path, path: &Path) -> Result<SceneDocument, String> {
    let raw = fs::read_to_string(path)
        .map_err(|error| format!("failed to read scene {}: {error}", normalize_path(path)))?;
    let value: Value =
        serde_json::from_str(&raw).map_err(|error| format!("invalid scene json: {error}"))?;
    let name = value
        .get("name")
        .and_then(Value::as_str)
        .unwrap_or("scene")
        .to_string();
    let document_id = value
        .get("document_id")
        .and_then(Value::as_str)
        .unwrap_or("")
        .to_string();
    let mut entities = Vec::new();

    if let Some(items) = value.get("entities").and_then(Value::as_array) {
        for (index, item) in items.iter().enumerate() {
            entities.push(read_entity(item, index));
        }
    }

    Ok(SceneDocument {
        name,
        path: normalize_path(path.strip_prefix(project_root).unwrap_or(path)),
        document_id,
        environment: scene_object_or_default(
            value.get("environment"),
            serde_json::json!({
                "skybox": { "mode": "solid", "color": "#1c1f26" },
                "ambient": { "color": "#ffffff", "intensity": 0.4 },
                "fog": { "enabled": false, "color": "#9ca3af", "density": 0.05 },
                "weather": { "preset": "clear" }
            }),
        ),
        render: scene_object_or_default(
            value.get("render"),
            serde_json::json!({
                "quality": { "profile": "medium" },
                "postfx": { "fxaa": false, "bloom": 0.0, "vignette": 0.0 },
                "camera": {}
            }),
        ),
        audio: scene_object_or_default(
            value.get("audio"),
            serde_json::json!({
                "listener": {},
                "mix": { "master": 1.0, "music": 0.8, "sfx": 1.0 }
            }),
        ),
        entities,
    })
}

fn scene_object_or_default(value: Option<&Value>, fallback: Value) -> Value {
    match value {
        Some(Value::Object(_)) => value.cloned().unwrap_or(fallback),
        _ => fallback,
    }
}

fn read_entity(value: &Value, index: usize) -> SceneEntity {
    let transform = value.get("transform").unwrap_or(&Value::Null);
    let components = value
        .get("components")
        .and_then(Value::as_array)
        .map(|items| items.iter().map(read_component).collect())
        .unwrap_or_default();

    SceneEntity {
        id: text_value(value, &["stable_id", "id"])
            .unwrap_or_else(|| format!("entity-{index}")),
        name: text_value(value, &["name"]).unwrap_or_else(|| format!("Entity {index}")),
        layer: text_value(value, &["layer"]).unwrap_or_else(|| "default".to_string()),
        parent: text_value(value, &["parent", "parent_id"]),
        tags: value
            .get("tags")
            .and_then(Value::as_array)
            .map(|items| {
                items
                    .iter()
                    .filter_map(Value::as_str)
                    .map(str::to_string)
                    .collect()
            })
            .unwrap_or_default(),
        components,
        transform: Transform3d {
            x: number_value(transform, "x", 0.0),
            y: number_value(transform, "y", 0.0),
            z: number_value(transform, "z", 0.0),
            rotation_x: number_value(transform, "rotation_x", 0.0),
            rotation_y: number_value(transform, "rotation_y", 0.0),
            rotation_z: number_value(
                transform,
                "rotation_z",
                number_value(transform, "rotation", 0.0),
            ),
            scale_x: number_value(transform, "scale_x", 1.0),
            scale_y: number_value(transform, "scale_y", 1.0),
            scale_z: number_value(transform, "scale_z", 1.0),
        },
    }
}

fn read_component(value: &Value) -> SceneComponent {
    let mut properties = serde_json::Map::new();
    if let Some(object) = value.as_object() {
        for (key, item) in object {
            if !matches!(key.as_str(), "kind" | "type") {
                properties.insert(key.clone(), item.clone());
            }
        }
    }

    SceneComponent {
        kind: text_value(value, &["kind", "type"])
            .unwrap_or_else(|| "component".to_string()),
        asset: text_value(value, &["asset", "model", "path"]),
        script: text_value(value, &["script", "source"]),
        profile: text_value(value, &["profile", "projection"]),
        properties: Value::Object(properties),
    }
}
