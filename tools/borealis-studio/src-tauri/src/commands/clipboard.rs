use std::sync::Mutex;

use uuid::Uuid;

use crate::models::scene::SceneEntity;
use crate::state::AppState;

static CLIPBOARD: std::sync::OnceLock<Mutex<Vec<SceneEntity>>> = std::sync::OnceLock::new();

fn clipboard() -> &'static Mutex<Vec<SceneEntity>> {
    CLIPBOARD.get_or_init(|| Mutex::new(Vec::new()))
}

#[tauri::command]
pub fn copy_entities(
    state: tauri::State<'_, Mutex<AppState>>,
) -> Result<usize, String> {
    let guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_ref().ok_or("no project open")?;
    let selected: Vec<SceneEntity> = project
        .scene
        .entities
        .iter()
        .filter(|e| project.selection.is_selected(&e.id))
        .cloned()
        .collect();
    let count = selected.len();
    let mut cb = clipboard().lock().map_err(|e| e.to_string())?;
    *cb = selected;
    Ok(count)
}

#[tauri::command]
pub fn paste_entities(
    state: tauri::State<'_, Mutex<AppState>>,
) -> Result<Vec<SceneEntity>, String> {
    let cb = clipboard().lock().map_err(|e| e.to_string())?;
    if cb.is_empty() {
        return Err("clipboard is empty".to_string());
    }
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    let mut pasted = Vec::new();
    project.selection.clear();
    for source in cb.iter() {
        let new_id = Uuid::new_v4().to_string();
        let mut entity = source.clone();
        entity.id = new_id.clone();
        entity.name = format!("{} Copy", source.name);
        project.scene.entities.push(entity.clone());
        project.selection.selected_ids.insert(new_id.clone());
        project.selection.primary_id = Some(new_id);
        pasted.push(entity);
    }
    project.dirty = true;
    Ok(pasted)
}

#[tauri::command]
pub fn cut_entities(
    state: tauri::State<'_, Mutex<AppState>>,
) -> Result<usize, String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    let selected: Vec<SceneEntity> = project
        .scene
        .entities
        .iter()
        .filter(|e| project.selection.is_selected(&e.id))
        .cloned()
        .collect();
    let count = selected.len();
    let ids_to_remove: Vec<String> = selected.iter().map(|e| e.id.clone()).collect();
    project
        .scene
        .entities
        .retain(|e| !ids_to_remove.contains(&e.id));
    project.selection.clear();
    project.dirty = true;
    let mut cb = clipboard().lock().map_err(|e| e.to_string())?;
    *cb = selected;
    Ok(count)
}

#[tauri::command]
pub fn delete_selected(
    state: tauri::State<'_, Mutex<AppState>>,
) -> Result<usize, String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    let before = project.scene.entities.len();
    let selected = project.selection.selected_ids.clone();
    project
        .scene
        .entities
        .retain(|e| !selected.contains(&e.id));
    let removed = before - project.scene.entities.len();
    project.selection.clear();
    if removed > 0 {
        project.dirty = true;
    }
    Ok(removed)
}

#[tauri::command]
pub fn duplicate_selected(
    state: tauri::State<'_, Mutex<AppState>>,
) -> Result<Vec<SceneEntity>, String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    let sources: Vec<SceneEntity> = project
        .scene
        .entities
        .iter()
        .filter(|e| project.selection.is_selected(&e.id))
        .cloned()
        .collect();
    if sources.is_empty() {
        return Err("nothing selected to duplicate".to_string());
    }
    let mut duplicated = Vec::new();
    project.selection.clear();
    for source in &sources {
        let new_id = Uuid::new_v4().to_string();
        let mut entity = source.clone();
        entity.id = new_id.clone();
        entity.name = format!("{} Copy", source.name);
        project.scene.entities.push(entity.clone());
        project.selection.selected_ids.insert(new_id.clone());
        project.selection.primary_id = Some(new_id);
        duplicated.push(entity);
    }
    project.dirty = true;
    Ok(duplicated)
}
