use std::path::PathBuf;
use std::sync::Mutex;

use crate::services::asset_database::{AssetDatabase, AssetEntry};
use crate::state::AppState;

#[tauri::command]
pub fn scan_assets(
    state: tauri::State<'_, Mutex<AppState>>,
) -> Result<Vec<AssetEntry>, String> {
    let guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_ref().ok_or("no project open")?;
    let project_root = PathBuf::from(&project.root);
    drop(guard);
    let mut db = AssetDatabase::new();
    db.scan_project(&project_root)
}

#[tauri::command]
pub fn import_asset(
    state: tauri::State<'_, Mutex<AppState>>,
    source_path: String,
    target_dir: Option<String>,
) -> Result<AssetEntry, String> {
    let guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_ref().ok_or("no project open")?;
    let project_root = PathBuf::from(&project.root);
    drop(guard);

    let target = target_dir.as_deref().unwrap_or("assets");
    let mut db = AssetDatabase::new();
    db.scan_project(&project_root)?;
    db.import_asset(&PathBuf::from(source_path), &project_root, target)
}

#[tauri::command]
pub fn get_asset_metadata(
    state: tauri::State<'_, Mutex<AppState>>,
    asset_id: String,
) -> Result<AssetEntry, String> {
    let guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_ref().ok_or("no project open")?;
    let project_root = PathBuf::from(&project.root);
    drop(guard);

    let mut db = AssetDatabase::new();
    db.scan_project(&project_root)?;
    db.get_by_id(&asset_id)
        .cloned()
        .ok_or_else(|| format!("asset not found: {asset_id}"))
}

#[tauri::command]
pub fn delete_asset(
    state: tauri::State<'_, Mutex<AppState>>,
    asset_path: String,
) -> Result<(), String> {
    let guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_ref().ok_or("no project open")?;
    let project_root = PathBuf::from(&project.root);
    drop(guard);

    let absolute = project_root.join(&asset_path);
    if absolute.exists() {
        std::fs::remove_file(&absolute)
            .map_err(|e| format!("failed to delete asset: {e}"))?;
    }
    Ok(())
}
