use serde::Serialize;
use serde_json::Value;
use std::fs;
use std::path::{Path, PathBuf};

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct StudioSnapshot {
    project_name: String,
    project_path: String,
    scene: SceneDocument,
    assets: Vec<ProjectAsset>,
    scripts: Vec<ScriptDocument>,
    console: Vec<ConsoleLine>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct SceneDocument {
    name: String,
    path: String,
    document_id: String,
    entities: Vec<SceneEntity>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct SceneEntity {
    id: String,
    name: String,
    layer: String,
    parent: Option<String>,
    tags: Vec<String>,
    components: Vec<SceneComponent>,
    transform: Transform3d,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct SceneComponent {
    kind: String,
    asset: Option<String>,
    script: Option<String>,
    profile: Option<String>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct Transform3d {
    x: f64,
    y: f64,
    z: f64,
    rotation_x: f64,
    rotation_y: f64,
    rotation_z: f64,
    scale_x: f64,
    scale_y: f64,
    scale_z: f64,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct ProjectAsset {
    id: String,
    name: String,
    path: String,
    kind: String,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct ScriptDocument {
    path: String,
    name: String,
    content: String,
    dirty: Option<bool>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct ConsoleLine {
    id: String,
    level: String,
    source: String,
    message: String,
}

#[tauri::command]
fn load_studio_snapshot(project_path: Option<String>) -> Result<StudioSnapshot, String> {
    let workspace = workspace_root();
    let project_path = project_path
        .map(PathBuf::from)
        .unwrap_or_else(|| workspace.join("packages/borealis/zenith.ztproj"));
    let project_dir = project_path
        .parent()
        .ok_or_else(|| "project path has no parent".to_string())?
        .to_path_buf();
    let project_name = read_project_name(&project_path).unwrap_or_else(|| "Borealis".to_string());
    let scene_path = if project_dir.join("scenes/sample_3d.scene.json").exists() {
        project_dir.join("scenes/sample_3d.scene.json")
    } else {
        project_dir.join("scenes/sample.scene.json")
    };
    let scene = read_scene_document(&scene_path)?;
    let assets = collect_assets(&project_dir, &project_dir)?;
    let scripts = collect_scripts(&project_dir, &project_dir)?;

    Ok(StudioSnapshot {
        project_name,
        project_path: normalize_path(&project_path),
        scene,
        assets,
        scripts,
        console: vec![ConsoleLine {
            id: "studio-ready".to_string(),
            level: "info".to_string(),
            source: "studio".to_string(),
            message: "Loaded project snapshot through Tauri.".to_string(),
        }],
    })
}

#[tauri::command]
fn read_text_file(path: String) -> Result<String, String> {
    let path = resolve_workspace_path(&path);
    fs::read_to_string(&path).map_err(|error| format!("failed to read {}: {error}", normalize_path(&path)))
}

#[tauri::command]
fn write_text_file(path: String, content: String) -> Result<(), String> {
    let path = resolve_workspace_path(&path);
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent)
            .map_err(|error| format!("failed to create {}: {error}", normalize_path(parent)))?;
    }
    fs::write(&path, content).map_err(|error| format!("failed to write {}: {error}", normalize_path(&path)))
}

fn read_scene_document(path: &Path) -> Result<SceneDocument, String> {
    let raw = fs::read_to_string(path)
        .map_err(|error| format!("failed to read scene {}: {error}", normalize_path(path)))?;
    let value: Value = serde_json::from_str(&raw).map_err(|error| format!("invalid scene json: {error}"))?;
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
        path: normalize_path(path),
        document_id,
        entities,
    })
}

fn read_entity(value: &Value, index: usize) -> SceneEntity {
    let transform = value.get("transform").unwrap_or(&Value::Null);
    let components = value
        .get("components")
        .and_then(Value::as_array)
        .map(|items| items.iter().map(read_component).collect())
        .unwrap_or_default();

    SceneEntity {
        id: text_value(value, &["stable_id", "id"]).unwrap_or_else(|| format!("entity-{index}")),
        name: text_value(value, &["name"]).unwrap_or_else(|| format!("Entity {index}")),
        layer: text_value(value, &["layer"]).unwrap_or_else(|| "default".to_string()),
        parent: text_value(value, &["parent", "parent_id"]),
        tags: value
            .get("tags")
            .and_then(Value::as_array)
            .map(|items| items.iter().filter_map(Value::as_str).map(str::to_string).collect())
            .unwrap_or_default(),
        components,
        transform: Transform3d {
            x: number_value(transform, "x", 0.0),
            y: number_value(transform, "y", 0.0),
            z: number_value(transform, "z", 0.0),
            rotation_x: number_value(transform, "rotation_x", 0.0),
            rotation_y: number_value(transform, "rotation_y", 0.0),
            rotation_z: number_value(transform, "rotation_z", number_value(transform, "rotation", 0.0)),
            scale_x: number_value(transform, "scale_x", 1.0),
            scale_y: number_value(transform, "scale_y", 1.0),
            scale_z: number_value(transform, "scale_z", 1.0),
        },
    }
}

fn read_component(value: &Value) -> SceneComponent {
    SceneComponent {
        kind: text_value(value, &["kind", "type"]).unwrap_or_else(|| "component".to_string()),
        asset: text_value(value, &["asset", "model", "path"]),
        script: text_value(value, &["script", "source"]),
        profile: text_value(value, &["profile", "projection"]),
    }
}

fn read_project_name(project_path: &Path) -> Option<String> {
    let raw = fs::read_to_string(project_path).ok()?;
    for line in raw.lines() {
        let trimmed = line.trim();
        if let Some((key, value)) = trimmed.split_once('=') {
            if key.trim() == "name" {
                return Some(value.trim().trim_matches('"').to_string());
            }
        }
    }
    None
}

fn collect_assets(root: &Path, current: &Path) -> Result<Vec<ProjectAsset>, String> {
    let mut assets = Vec::new();
    collect_files(root, current, &mut |path| {
        if let Some(kind) = asset_kind(path) {
            assets.push(ProjectAsset {
                id: normalize_path(path),
                name: path.file_name().and_then(|name| name.to_str()).unwrap_or("asset").to_string(),
                path: normalize_path(path.strip_prefix(root).unwrap_or(path)),
                kind,
            });
        }
    })?;
    assets.sort_by(|left, right| left.path.cmp(&right.path));
    assets.truncate(160);
    Ok(assets)
}

fn collect_scripts(root: &Path, current: &Path) -> Result<Vec<ScriptDocument>, String> {
    let mut scripts = Vec::new();
    collect_files(root, current, &mut |path| {
        if path.extension().and_then(|value| value.to_str()) == Some("zt") {
            let content = fs::read_to_string(path).unwrap_or_default();
            scripts.push(ScriptDocument {
                path: normalize_path(path.strip_prefix(root).unwrap_or(path)),
                name: path.file_name().and_then(|name| name.to_str()).unwrap_or("script.zt").to_string(),
                content,
                dirty: Some(false),
            });
        }
    })?;
    scripts.sort_by(|left, right| left.path.cmp(&right.path));
    scripts.truncate(40);
    Ok(scripts)
}

fn collect_files(root: &Path, current: &Path, visitor: &mut dyn FnMut(&Path)) -> Result<(), String> {
    let entries = fs::read_dir(current).map_err(|error| format!("failed to read {}: {error}", normalize_path(current)))?;
    for entry in entries {
        let entry = entry.map_err(|error| format!("failed to read directory entry: {error}"))?;
        let path = entry.path();
        if path.is_dir() {
            let name = path.file_name().and_then(|name| name.to_str()).unwrap_or("");
            if matches!(name, "target" | "build" | ".ztc-tmp" | "native") {
                continue;
            }
            collect_files(root, &path, visitor)?;
        } else {
            visitor(&path);
        }
    }
    Ok(())
}

fn asset_kind(path: &Path) -> Option<String> {
    match path.extension().and_then(|value| value.to_str()).unwrap_or("").to_ascii_lowercase().as_str() {
        "scene" | "json" if path.to_string_lossy().contains(".scene.") => Some("scene".to_string()),
        "zt" => Some("script".to_string()),
        "obj" | "glb" | "gltf" | "fbx" | "iqm" | "m3d" => Some("model".to_string()),
        "png" | "jpg" | "jpeg" | "webp" => Some("texture".to_string()),
        "wav" | "ogg" | "mp3" => Some("audio".to_string()),
        _ => None,
    }
}

fn text_value(value: &Value, keys: &[&str]) -> Option<String> {
    keys.iter()
        .find_map(|key| value.get(key).and_then(Value::as_str).map(str::to_string))
}

fn number_value(value: &Value, key: &str, fallback: f64) -> f64 {
    value.get(key).and_then(Value::as_f64).unwrap_or(fallback)
}

fn resolve_workspace_path(path: &str) -> PathBuf {
    let raw = PathBuf::from(path);
    if raw.is_absolute() {
        raw
    } else {
        workspace_root().join(path)
    }
}

fn workspace_root() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../..")
        .canonicalize()
        .unwrap_or_else(|_| Path::new(env!("CARGO_MANIFEST_DIR")).join("../.."))
}

fn normalize_path(path: &Path) -> String {
    path.to_string_lossy().replace('\\', "/")
}

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![
            load_studio_snapshot,
            read_text_file,
            write_text_file
        ])
        .run(tauri::generate_context!())
        .expect("failed to run Borealis Studio");
}
