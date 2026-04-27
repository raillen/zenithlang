use std::sync::Mutex;

use serde_json::Value;

use crate::models::entity::{AddEntityRequest, EntityPatch, FlatTreeNode, ReparentRequest};
use crate::models::scene::{SceneEntity, Transform3d};
use crate::services::entity_tree::EntityTree;
use crate::state::AppState;

#[tauri::command]
pub fn add_entity(
    state: tauri::State<'_, Mutex<AppState>>,
    request: AddEntityRequest,
    transform: Option<Transform3d>,
) -> Result<SceneEntity, String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    let mut tree = EntityTree::from_scene(&project.scene);
    let entity = tree.add_entity(&request, transform.unwrap_or_default());
    tree.apply_to_scene(&mut project.scene);
    project.dirty = true;
    Ok(entity)
}

#[tauri::command]
pub fn remove_entity(
    state: tauri::State<'_, Mutex<AppState>>,
    entity_id: String,
) -> Result<Vec<SceneEntity>, String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    let mut tree = EntityTree::from_scene(&project.scene);
    let removed = tree.remove_entity(&entity_id);
    if removed.is_empty() {
        return Err(format!("entity not found: {entity_id}"));
    }
    tree.apply_to_scene(&mut project.scene);
    project.selection.selected_ids.remove(&entity_id);
    if project.selection.primary_id.as_deref() == Some(&entity_id) {
        project.selection.primary_id = None;
    }
    project.dirty = true;
    Ok(removed)
}

#[tauri::command]
pub fn update_entity(
    state: tauri::State<'_, Mutex<AppState>>,
    entity_id: String,
    patch: EntityPatch,
) -> Result<SceneEntity, String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    let mut tree = EntityTree::from_scene(&project.scene);
    tree.update_entity(&entity_id, &patch)?;
    let updated = tree.get(&entity_id).cloned().ok_or("entity lost after update")?;
    tree.apply_to_scene(&mut project.scene);
    project.dirty = true;
    Ok(updated)
}

#[tauri::command]
pub fn update_entity_transform(
    state: tauri::State<'_, Mutex<AppState>>,
    entity_id: String,
    transform: Transform3d,
) -> Result<(), String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    let mut tree = EntityTree::from_scene(&project.scene);
    tree.update_transform(&entity_id, transform)?;
    tree.apply_to_scene(&mut project.scene);
    project.dirty = true;
    Ok(())
}

#[tauri::command]
pub fn duplicate_entity(
    state: tauri::State<'_, Mutex<AppState>>,
    entity_id: String,
) -> Result<SceneEntity, String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    let mut tree = EntityTree::from_scene(&project.scene);
    let duplicate = tree.duplicate_entity(&entity_id)?;
    tree.apply_to_scene(&mut project.scene);
    project.dirty = true;
    Ok(duplicate)
}

#[tauri::command]
pub fn reparent_entity(
    state: tauri::State<'_, Mutex<AppState>>,
    request: ReparentRequest,
) -> Result<(), String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    let mut tree = EntityTree::from_scene(&project.scene);
    tree.reparent(
        &request.entity_id,
        request.new_parent_id.as_deref(),
        request.index,
    )?;
    tree.apply_to_scene(&mut project.scene);
    project.dirty = true;
    Ok(())
}

#[tauri::command]
pub fn get_entity_tree(
    state: tauri::State<'_, Mutex<AppState>>,
) -> Result<Vec<FlatTreeNode>, String> {
    let guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_ref().ok_or("no project open")?;
    let tree = EntityTree::from_scene(&project.scene);
    Ok(tree.flatten())
}
