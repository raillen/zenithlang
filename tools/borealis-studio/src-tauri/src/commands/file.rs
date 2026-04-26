use std::fs;

use crate::services::file_service::write_text_at_path;
use crate::services::project_manager::resolve_studio_path;
use crate::utils::paths::{normalize_path, resolve_project_path};

#[tauri::command]
pub fn read_text_file(path: String) -> Result<String, String> {
    let path = resolve_studio_path(&path);
    fs::read_to_string(&path)
        .map_err(|error| format!("failed to read {}: {error}", normalize_path(&path)))
}

#[tauri::command]
pub fn write_text_file(path: String, content: String) -> Result<(), String> {
    let path = resolve_studio_path(&path);
    write_text_at_path(&path, content)
}

#[tauri::command]
pub fn write_project_text_file(
    project_root: String,
    path: String,
    content: String,
) -> Result<(), String> {
    let project_root = resolve_studio_path(&project_root);
    let path = resolve_project_path(&project_root, &path);
    write_text_at_path(&path, content)
}
