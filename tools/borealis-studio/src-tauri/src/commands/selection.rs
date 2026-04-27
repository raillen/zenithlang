use std::sync::Mutex;

use crate::models::selection::SelectionState;
use crate::state::AppState;

#[tauri::command]
pub fn select_entity(
    state: tauri::State<'_, Mutex<AppState>>,
    entity_id: String,
) -> Result<SelectionState, String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    if !project.scene.entities.iter().any(|e| e.id == entity_id) {
        return Err(format!("entity not found: {entity_id}"));
    }
    project.selection.select_single(entity_id);
    Ok(project.selection.clone())
}

#[tauri::command]
pub fn toggle_selection(
    state: tauri::State<'_, Mutex<AppState>>,
    entity_id: String,
) -> Result<SelectionState, String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    if !project.scene.entities.iter().any(|e| e.id == entity_id) {
        return Err(format!("entity not found: {entity_id}"));
    }
    project.selection.toggle(entity_id);
    Ok(project.selection.clone())
}

#[tauri::command]
pub fn deselect_all(
    state: tauri::State<'_, Mutex<AppState>>,
) -> Result<SelectionState, String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    project.selection.clear();
    Ok(project.selection.clone())
}

#[tauri::command]
pub fn select_all(
    state: tauri::State<'_, Mutex<AppState>>,
) -> Result<SelectionState, String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    project.selection.selected_ids = project
        .scene
        .entities
        .iter()
        .map(|e| e.id.clone())
        .collect();
    project.selection.primary_id = project.scene.entities.last().map(|e| e.id.clone());
    Ok(project.selection.clone())
}

#[tauri::command]
pub fn get_selection(
    state: tauri::State<'_, Mutex<AppState>>,
) -> Result<SelectionState, String> {
    let guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_ref().ok_or("no project open")?;
    Ok(project.selection.clone())
}

#[tauri::command]
pub fn set_hover(
    state: tauri::State<'_, Mutex<AppState>>,
    entity_id: Option<String>,
) -> Result<(), String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    project.selection.hover_id = entity_id;
    Ok(())
}
