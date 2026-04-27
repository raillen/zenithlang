use std::sync::Mutex;

use crate::models::history::HistoryState;
use crate::models::scene::SceneDocument;
use crate::state::AppState;

#[tauri::command]
pub fn undo(state: tauri::State<'_, Mutex<AppState>>) -> Result<SceneDocument, String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    project.history.undo(&mut project.scene)?;
    project.dirty = true;
    Ok(project.scene.clone())
}

#[tauri::command]
pub fn redo(state: tauri::State<'_, Mutex<AppState>>) -> Result<SceneDocument, String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    project.history.redo(&mut project.scene)?;
    project.dirty = true;
    Ok(project.scene.clone())
}

#[tauri::command]
pub fn get_history_state(
    state: tauri::State<'_, Mutex<AppState>>,
) -> Result<HistoryState, String> {
    let guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_ref().ok_or("no project open")?;
    Ok(project.history.state())
}

#[tauri::command]
pub fn clear_history(state: tauri::State<'_, Mutex<AppState>>) -> Result<(), String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    project.history.clear();
    Ok(())
}
