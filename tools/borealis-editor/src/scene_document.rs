use crate::project::ProjectSummary;
use serde_json::{Map, Number, Value};
use std::fs;
use std::path::{Path, PathBuf};

#[derive(Debug, Clone, Default)]
pub struct SceneDocument {
    pub version: i64,
    pub name: String,
    pub document_id: String,
    pub source_path: String,
    pub format: SceneDocumentFormat,
    pub raw_json: Option<Value>,
    pub entities: Vec<SceneEntity>,
}

#[derive(Debug, Clone, Default)]
pub enum SceneDocumentFormat {
    Json,
    ZenithExample,
    #[default]
    Unknown,
}

#[derive(Debug, Clone, Default)]
pub struct SceneEntity {
    pub stable_id: String,
    pub name: String,
    pub layer: String,
    pub parent: String,
    pub tags: Vec<String>,
    pub components: Vec<String>,
    pub transform: Transform2D,
}

#[derive(Debug, Clone)]
pub struct Transform2D {
    pub x: f32,
    pub y: f32,
    pub rotation: f32,
    pub scale_x: f32,
    pub scale_y: f32,
}

impl Default for Transform2D {
    fn default() -> Self {
        Self {
            x: 0.0,
            y: 0.0,
            rotation: 0.0,
            scale_x: 1.0,
            scale_y: 1.0,
        }
    }
}

pub fn load_scene_document(
    summary: &ProjectSummary,
    scene_path: &str,
) -> Result<SceneDocument, String> {
    let path = resolve_scene_path(summary, scene_path)
        .ok_or_else(|| format!("cena nao encontrada: {scene_path}"))?;
    let raw = fs::read_to_string(&path)
        .map_err(|error| format!("falha ao ler cena {}: {error}", normalize_path(&path)))?;

    match path.extension().and_then(|value| value.to_str()) {
        Some("json") => load_json_scene(&path, &raw),
        Some("zt") => Ok(load_zt_scene(&path, &raw)),
        _ => Ok(SceneDocument {
            version: 1,
            name: path
                .file_stem()
                .and_then(|value| value.to_str())
                .unwrap_or("scene")
                .to_string(),
            document_id: String::new(),
            source_path: normalize_path(&path),
            format: SceneDocumentFormat::Unknown,
            raw_json: None,
            entities: Vec::new(),
        }),
    }
}

pub fn save_scene_document(document: &SceneDocument) -> Result<(), String> {
    if !matches!(document.format, SceneDocumentFormat::Json) {
        return Err("salvamento suportado apenas para scene documents json".to_string());
    }

    let mut root = document
        .raw_json
        .clone()
        .unwrap_or_else(|| Value::Object(Map::new()));
    if !root.is_object() {
        root = Value::Object(Map::new());
    }

    let previous_entities = root
        .get("entities")
        .and_then(Value::as_array)
        .cloned()
        .unwrap_or_default();

    set_root_string(&mut root, "name", &document.name);
    set_root_string(&mut root, "document_id", &document.document_id);
    set_root_number(&mut root, "version", document.version as f64);

    let entities_json = document
        .entities
        .iter()
        .map(|entity| build_json_entity(entity, &previous_entities))
        .collect();
    set_root_value(&mut root, "entities", Value::Array(entities_json));

    let encoded = serde_json::to_string_pretty(&root)
        .map_err(|error| format!("falha ao serializar cena json: {error}"))?;
    fs::write(&document.source_path, format!("{encoded}\n"))
        .map_err(|error| format!("falha ao salvar cena {}: {error}", document.source_path))?;
    Ok(())
}

fn resolve_scene_path(summary: &ProjectSummary, scene_path: &str) -> Option<PathBuf> {
    let root = PathBuf::from(&summary.project_dir);
    let raw = PathBuf::from(scene_path);
    if raw.exists() {
        return Some(raw);
    }

    let candidates = [
        root.join(scene_path),
        root.join("examples").join(scene_path),
        root.join(&summary.source_root).join(scene_path),
    ];

    candidates.into_iter().find(|path| path.exists())
}

fn load_json_scene(path: &Path, raw: &str) -> Result<SceneDocument, String> {
    let value: Value =
        serde_json::from_str(raw).map_err(|error| format!("json de cena invalido: {error}"))?;
    let name = value
        .get("name")
        .and_then(Value::as_str)
        .unwrap_or_else(|| fallback_scene_name(path))
        .to_string();
    let document_id = value
        .get("document_id")
        .and_then(Value::as_str)
        .unwrap_or("")
        .to_string();
    let version = value.get("version").and_then(Value::as_i64).unwrap_or(1);

    let mut entities = Vec::new();
    if let Some(items) = value.get("entities").and_then(Value::as_array) {
        for (index, entity) in items.iter().enumerate() {
            entities.push(SceneEntity {
                stable_id: json_text(entity, &["stable_id", "id"])
                    .unwrap_or_else(|| format!("entity-{index}")),
                name: json_text(entity, &["name"]).unwrap_or_else(|| format!("Entity {index}")),
                layer: json_text(entity, &["layer"]).unwrap_or_else(|| "default".to_string()),
                parent: json_text(entity, &["parent", "parent_id"]).unwrap_or_default(),
                tags: json_tags(entity),
                components: json_components(entity),
                transform: json_transform(entity),
            });
        }
    }

    Ok(SceneDocument {
        version,
        name,
        document_id,
        source_path: normalize_path(path),
        format: SceneDocumentFormat::Json,
        raw_json: Some(value),
        entities,
    })
}

fn load_zt_scene(path: &Path, raw: &str) -> SceneDocument {
    let mut scene = SceneDocument {
        version: 1,
        name: first_call_arg(raw, "scene_create")
            .unwrap_or_else(|| fallback_scene_name(path).to_string()),
        document_id: String::new(),
        source_path: normalize_path(path),
        format: SceneDocumentFormat::ZenithExample,
        raw_json: None,
        entities: Vec::new(),
    };

    for line in raw.lines() {
        let trimmed = line.trim();
        if !trimmed.contains("entity_create(") {
            continue;
        }

        let var_name = trimmed
            .split(':')
            .next()
            .and_then(|left| left.split_whitespace().last())
            .unwrap_or("entity");
        let name = first_call_arg(trimmed, "entity_create").unwrap_or_else(|| var_name.to_string());

        scene.entities.push(SceneEntity {
            stable_id: var_name.to_string(),
            name,
            layer: "default".to_string(),
            parent: String::new(),
            tags: Vec::new(),
            components: Vec::new(),
            transform: Transform2D::default(),
        });
    }

    for line in raw.lines() {
        let trimmed = line.trim();
        if trimmed.contains("entity_add_tag(") {
            apply_tag(&mut scene.entities, trimmed);
        } else if trimmed.contains("entity_set_parent(") {
            apply_parent(&mut scene.entities, trimmed);
        }
    }

    scene
}

fn apply_tag(entities: &mut [SceneEntity], line: &str) {
    let Some(args) = call_args(line, "entity_add_tag") else {
        return;
    };
    let Some((entity_ref, tag_raw)) = args.split_once(',') else {
        return;
    };
    let tag = tag_raw.trim().trim_matches('"').trim_end_matches(")?");

    if let Some(entity) = entities
        .iter_mut()
        .find(|entity| entity.stable_id == entity_ref.trim())
    {
        entity.tags.push(tag.to_string());
    }
}

fn apply_parent(entities: &mut [SceneEntity], line: &str) {
    let Some(args) = call_args(line, "entity_set_parent") else {
        return;
    };
    let Some((child_ref, parent_ref)) = args.split_once(',') else {
        return;
    };
    let child = child_ref.trim();
    let parent = parent_ref.trim().trim_end_matches(")?").to_string();

    if let Some(entity) = entities.iter_mut().find(|entity| entity.stable_id == child) {
        entity.parent = parent;
    }
}

fn first_call_arg(raw: &str, call: &str) -> Option<String> {
    for line in raw.lines() {
        if let Some(args) = call_args(line, call) {
            return args
                .split(',')
                .next()
                .map(|value| value.trim().trim_matches('"').to_string());
        }
    }

    None
}

fn call_args(line: &str, call: &str) -> Option<String> {
    let needle = format!("{call}(");
    let start = line.find(&needle)? + needle.len();
    let rest = &line[start..];
    let end = rest.find(')')?;
    Some(rest[..end].trim().to_string())
}

fn json_text(value: &Value, keys: &[&str]) -> Option<String> {
    for key in keys {
        if let Some(text) = value.get(key).and_then(Value::as_str) {
            return Some(text.to_string());
        }
    }

    None
}

fn json_tags(value: &Value) -> Vec<String> {
    value
        .get("tags")
        .and_then(Value::as_array)
        .map(|items| {
            items
                .iter()
                .filter_map(Value::as_str)
                .map(str::to_string)
                .collect()
        })
        .unwrap_or_default()
}

fn json_components(value: &Value) -> Vec<String> {
    value
        .get("components")
        .and_then(Value::as_array)
        .map(|items| {
            items
                .iter()
                .enumerate()
                .map(|(index, item)| {
                    json_text(item, &["kind", "type"])
                        .unwrap_or_else(|| format!("component-{index}"))
                })
                .collect()
        })
        .unwrap_or_default()
}

fn json_transform(value: &Value) -> Transform2D {
    let Some(transform) = value.get("transform") else {
        return Transform2D::default();
    };

    Transform2D {
        x: json_number(transform, "x").unwrap_or(0.0),
        y: json_number(transform, "y").unwrap_or(0.0),
        rotation: json_number(transform, "rotation").unwrap_or(0.0),
        scale_x: json_number(transform, "scale_x").unwrap_or(1.0),
        scale_y: json_number(transform, "scale_y").unwrap_or(1.0),
    }
}

fn json_number(value: &Value, key: &str) -> Option<f32> {
    value
        .get(key)
        .and_then(Value::as_f64)
        .map(|number| number as f32)
}

fn build_json_entity(entity: &SceneEntity, previous_entities: &[Value]) -> Value {
    let mut value = previous_entities
        .iter()
        .find(|candidate| {
            json_text(candidate, &["stable_id", "id"]).as_deref() == Some(entity.stable_id.as_str())
        })
        .cloned()
        .unwrap_or_else(|| Value::Object(Map::new()));
    if !value.is_object() {
        value = Value::Object(Map::new());
    }

    set_value_string(&mut value, "stable_id", &entity.stable_id);
    set_value_string(&mut value, "name", &entity.name);
    set_value_string(&mut value, "layer", &entity.layer);
    if entity.parent.is_empty() {
        remove_value_key(&mut value, "parent");
    } else {
        set_value_string(&mut value, "parent", &entity.parent);
    }
    set_value_value(
        &mut value,
        "tags",
        Value::Array(entity.tags.iter().cloned().map(Value::String).collect()),
    );
    set_value_value(
        &mut value,
        "transform",
        Value::Object(Map::from_iter([
            ("x".to_string(), json_number_value(entity.transform.x)),
            ("y".to_string(), json_number_value(entity.transform.y)),
            (
                "rotation".to_string(),
                json_number_value(entity.transform.rotation),
            ),
            (
                "scale_x".to_string(),
                json_number_value(entity.transform.scale_x),
            ),
            (
                "scale_y".to_string(),
                json_number_value(entity.transform.scale_y),
            ),
        ])),
    );
    let previous_components = value
        .get("components")
        .and_then(Value::as_array)
        .cloned()
        .unwrap_or_default();
    let rebuilt_components = build_json_components(&entity.components, &previous_components);
    if rebuilt_components.is_empty() {
        remove_value_key(&mut value, "components");
    } else {
        set_value_value(&mut value, "components", Value::Array(rebuilt_components));
    }

    value
}

fn build_json_components(components: &[String], previous_components: &[Value]) -> Vec<Value> {
    let mut reused = vec![false; previous_components.len()];
    let mut rebuilt = Vec::with_capacity(components.len());

    for component in components {
        let mut value = previous_components
            .iter()
            .enumerate()
            .find(|(index, item)| {
                !reused[*index]
                    && json_text(item, &["kind", "type"]).as_deref() == Some(component.as_str())
            })
            .map(|(index, item)| {
                reused[index] = true;
                item.clone()
            })
            .unwrap_or_else(|| Value::Object(Map::new()));

        if !value.is_object() {
            value = Value::Object(Map::new());
        }

        if value.get("kind").is_some() {
            set_value_string(&mut value, "kind", component);
        } else if value.get("type").is_some() {
            set_value_string(&mut value, "type", component);
        } else {
            set_value_string(&mut value, "kind", component);
        }

        rebuilt.push(value);
    }

    rebuilt
}

fn json_number_value(value: f32) -> Value {
    Value::Number(Number::from_f64(value as f64).unwrap_or_else(|| Number::from(0)))
}

fn set_root_string(root: &mut Value, key: &str, value: &str) {
    set_value_string(root, key, value);
}

fn set_root_number(root: &mut Value, key: &str, value: f64) {
    set_value_value(
        root,
        key,
        Value::Number(Number::from_f64(value).unwrap_or_else(|| Number::from(0))),
    );
}

fn set_root_value(root: &mut Value, key: &str, value: Value) {
    set_value_value(root, key, value);
}

fn set_value_string(target: &mut Value, key: &str, value: &str) {
    set_value_value(target, key, Value::String(value.to_string()));
}

fn set_value_value(target: &mut Value, key: &str, value: Value) {
    if let Some(object) = target.as_object_mut() {
        object.insert(key.to_string(), value);
    }
}

fn remove_value_key(target: &mut Value, key: &str) {
    if let Some(object) = target.as_object_mut() {
        object.remove(key);
    }
}

fn fallback_scene_name(path: &Path) -> &str {
    path.file_stem()
        .and_then(|value| value.to_str())
        .unwrap_or("scene")
}

fn normalize_path(path: &Path) -> String {
    path.to_string_lossy().replace('\\', "/")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn loads_borealis_sample_scene_json() {
        let project_dir = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
            .join("../..")
            .join("packages/borealis");
        let summary = ProjectSummary {
            project_dir: normalize_path(&project_dir),
            source_root: "src".to_string(),
            ..ProjectSummary::default()
        };

        let document = load_scene_document(&summary, "scenes/sample.scene.json")
            .expect("sample scene should load");

        assert_eq!(document.name, "sample");
        assert_eq!(document.entities.len(), 3);
        assert_eq!(document.entities[0].stable_id, "player");
        assert_eq!(
            document.entities[0].components,
            vec!["sprite", "controller"]
        );
        assert_eq!(document.entities[0].transform.x, 120.0);
        assert_eq!(document.entities[0].transform.y, 96.0);
        assert_eq!(document.entities[1].parent, "player");
    }

    #[test]
    fn saves_scene_document_roundtrip_with_edits() {
        let scene_source = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
            .join("../..")
            .join("packages/borealis/scenes/sample.scene.json");
        let temp_dir = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("target/test-output");
        let _ = fs::create_dir_all(&temp_dir);
        let scene_copy = temp_dir.join("sample-roundtrip.scene.json");
        fs::copy(&scene_source, &scene_copy).expect("sample scene copy should succeed");

        let summary = ProjectSummary {
            project_dir: normalize_path(&temp_dir),
            source_root: "src".to_string(),
            ..ProjectSummary::default()
        };

        let mut document =
            load_scene_document(&summary, &normalize_path(&scene_copy)).expect("scene should load");
        document.name = "sample-edited".to_string();
        document.document_id = "scene:sample-edited".to_string();
        document.entities[0].name = "Player Edited".to_string();
        document.entities[0].layer = "foreground".to_string();
        document.entities[0].tags = vec!["hero".to_string(), "controllable".to_string()];
        document.entities[0].components = vec!["sprite".to_string(), "collider2d".to_string()];
        document.entities[0].transform.x = 180.0;
        document.entities[0].transform.y = 140.0;
        save_scene_document(&document).expect("scene should save");

        let reloaded = load_scene_document(&summary, &normalize_path(&scene_copy))
            .expect("scene should reload");
        assert_eq!(reloaded.name, "sample-edited");
        assert_eq!(reloaded.document_id, "scene:sample-edited");
        assert_eq!(reloaded.entities[0].name, "Player Edited");
        assert_eq!(reloaded.entities[0].layer, "foreground");
        assert_eq!(reloaded.entities[0].tags, vec!["hero", "controllable"]);
        assert_eq!(reloaded.entities[0].transform.x, 180.0);
        assert_eq!(reloaded.entities[0].transform.y, 140.0);
        assert_eq!(
            reloaded.entities[0].components,
            vec!["sprite", "collider2d"]
        );
    }

    #[test]
    fn saves_scene_document_with_added_and_removed_entities() {
        let scene_source = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
            .join("../..")
            .join("packages/borealis/scenes/sample.scene.json");
        let temp_dir = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("target/test-output");
        let _ = fs::create_dir_all(&temp_dir);
        let scene_copy = temp_dir.join("sample-entities-roundtrip.scene.json");
        fs::copy(&scene_source, &scene_copy).expect("sample scene copy should succeed");

        let summary = ProjectSummary {
            project_dir: normalize_path(&temp_dir),
            source_root: "src".to_string(),
            ..ProjectSummary::default()
        };

        let mut document =
            load_scene_document(&summary, &normalize_path(&scene_copy)).expect("scene should load");
        document
            .entities
            .retain(|entity| entity.stable_id != "camera-main");
        document.entities.push(SceneEntity {
            stable_id: "npc-01".to_string(),
            name: "NPC 01".to_string(),
            layer: "actors".to_string(),
            parent: String::new(),
            tags: vec!["npc".to_string()],
            components: vec!["dialog".to_string()],
            transform: Transform2D {
                x: 320.0,
                y: 180.0,
                rotation: 0.0,
                scale_x: 1.0,
                scale_y: 1.0,
            },
        });
        save_scene_document(&document).expect("scene should save");

        let reloaded = load_scene_document(&summary, &normalize_path(&scene_copy))
            .expect("scene should reload");
        assert_eq!(reloaded.entities.len(), 3);
        assert!(reloaded
            .entities
            .iter()
            .all(|entity| entity.stable_id != "camera-main"));
        let npc = reloaded
            .entities
            .iter()
            .find(|entity| entity.stable_id == "npc-01")
            .expect("npc should exist after reload");
        assert_eq!(npc.name, "NPC 01");
        assert_eq!(npc.components, vec!["dialog"]);
        assert_eq!(npc.transform.x, 320.0);
        assert_eq!(npc.transform.y, 180.0);
    }
}
