use std::sync::Mutex;

use serde_json::Value;

use crate::models::scene::SceneDocument;
use crate::services::file_service::write_text_at_path;
use crate::state::AppState;
use crate::utils::paths::resolve_project_path;

#[tauri::command]
pub fn save_scene(state: tauri::State<'_, Mutex<AppState>>) -> Result<String, String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    let scene = &project.scene;
    let scene_json = serialize_scene(scene)?;
    let absolute_path = resolve_project_path(
        std::path::Path::new(&project.root),
        &scene.path,
    );
    write_text_at_path(&absolute_path, scene_json)?;
    project.dirty = false;
    Ok(scene.path.clone())
}

#[tauri::command]
pub fn update_scene_settings(
    state: tauri::State<'_, Mutex<AppState>>,
    section: String,
    values: Value,
) -> Result<(), String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    match section.as_str() {
        "environment" => project.scene.environment = values,
        "render" => project.scene.render = values,
        "audio" => project.scene.audio = values,
        other => return Err(format!("unknown scene section: {other}")),
    }
    project.dirty = true;
    Ok(())
}

#[tauri::command]
pub fn get_scene(state: tauri::State<'_, Mutex<AppState>>) -> Result<SceneDocument, String> {
    let guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_ref().ok_or("no project open")?;
    Ok(project.scene.clone())
}

fn serialize_scene(scene: &SceneDocument) -> Result<String, String> {
    let mut entities_json = Vec::new();
    for entity in &scene.entities {
        let mut obj = serde_json::Map::new();
        obj.insert("stable_id".to_string(), Value::String(entity.id.clone()));
        obj.insert("name".to_string(), Value::String(entity.name.clone()));
        obj.insert("layer".to_string(), Value::String(entity.layer.clone()));
        if let Some(parent) = &entity.parent {
            obj.insert("parent".to_string(), Value::String(parent.clone()));
        }
        if !entity.tags.is_empty() {
            obj.insert(
                "tags".to_string(),
                Value::Array(entity.tags.iter().map(|t| Value::String(t.clone())).collect()),
            );
        }
        let transform = serde_json::json!({
            "x": entity.transform.x,
            "y": entity.transform.y,
            "z": entity.transform.z,
            "rotation_x": entity.transform.rotation_x,
            "rotation_y": entity.transform.rotation_y,
            "rotation_z": entity.transform.rotation_z,
            "scale_x": entity.transform.scale_x,
            "scale_y": entity.transform.scale_y,
            "scale_z": entity.transform.scale_z,
        });
        obj.insert("transform".to_string(), transform);
        let components: Vec<Value> = entity
            .components
            .iter()
            .map(|c| {
                let mut cobj = serde_json::Map::new();
                cobj.insert("kind".to_string(), Value::String(c.kind.clone()));
                if let Some(asset) = &c.asset {
                    cobj.insert("asset".to_string(), Value::String(asset.clone()));
                }
                if let Some(script) = &c.script {
                    cobj.insert("script".to_string(), Value::String(script.clone()));
                }
                if let Some(profile) = &c.profile {
                    cobj.insert("profile".to_string(), Value::String(profile.clone()));
                }
                if let Value::Object(props) = &c.properties {
                    for (k, v) in props {
                        cobj.insert(k.clone(), v.clone());
                    }
                }
                Value::Object(cobj)
            })
            .collect();
        obj.insert("components".to_string(), Value::Array(components));
        entities_json.push(Value::Object(obj));
    }

    let document = serde_json::json!({
        "version": 2,
        "name": scene.name,
        "document_id": scene.document_id,
        "environment": scene.environment,
        "render": scene.render,
        "audio": scene.audio,
        "entities": entities_json,
    });

    serde_json::to_string_pretty(&document)
        .map(|s| format!("{s}\n"))
        .map_err(|e| format!("failed to serialize scene: {e}"))
}
