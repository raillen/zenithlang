use std::fs;
use std::sync::{Mutex, OnceLock};
use std::time::Duration;

use serde_json::json;

use crate::models::preview::{PreviewCommandResult, PreviewSession};
use crate::services::preview_runner::{
    collect_preview_events, preview_is_alive, preview_result, preview_scene_target,
    send_preview_command, spawn_preview_process,
};
use crate::services::project_manager::{resolve_studio_path, studio_layout};
use crate::utils::paths::normalize_path;

static PREVIEW_SESSION: OnceLock<Mutex<Option<PreviewSession>>> = OnceLock::new();

#[tauri::command]
pub fn start_preview(
    project_path: String,
    scene_path: String,
    scene_json: String,
) -> Result<PreviewCommandResult, String> {
    let layout = studio_layout();
    let (preview_scene_path, preview_scene_for_ipc) = preview_scene_target(&layout);
    if let Some(parent) = preview_scene_path.parent() {
        fs::create_dir_all(parent)
            .map_err(|error| format!("failed to create {}: {error}", normalize_path(parent)))?;
    }
    fs::write(&preview_scene_path, scene_json).map_err(|error| {
        format!(
            "failed to write {}: {error}",
            normalize_path(&preview_scene_path)
        )
    })?;

    let resolved_project_path = resolve_studio_path(&project_path);
    let project_for_ipc = normalize_path(
        resolved_project_path
            .strip_prefix(&layout.base_root)
            .unwrap_or_else(|_| std::path::Path::new(&project_path)),
    );
    let source_scene = if scene_path.is_empty() {
        preview_scene_for_ipc.clone()
    } else {
        scene_path
    };

    let lock = PREVIEW_SESSION.get_or_init(|| Mutex::new(None));
    let mut guard = lock
        .lock()
        .map_err(|_| "preview session lock poisoned".to_string())?;
    let needs_spawn = match guard.as_mut() {
        Some(session) => !preview_is_alive(session),
        None => true,
    };

    if needs_spawn {
        *guard = Some(spawn_preview_process(&layout)?);
    }

    let session = guard
        .as_mut()
        .ok_or_else(|| "preview session was not initialized".to_string())?;

    if needs_spawn {
        send_preview_command(session, "hello", json!({ "role": "studio" }))?;
    }
    send_preview_command(
        session,
        "open_project",
        json!({
            "path": project_for_ipc,
            "sourceScene": source_scene
        }),
    )?;
    send_preview_command(
        session,
        "open_scene",
        json!({ "path": preview_scene_for_ipc }),
    )?;
    send_preview_command(
        session,
        "enter_play_mode",
        json!({ "scene": preview_scene_for_ipc }),
    )?;

    let events = collect_preview_events(session, Duration::from_millis(600));
    Ok(preview_result(session, events))
}

#[tauri::command]
pub fn pause_preview() -> Result<PreviewCommandResult, String> {
    with_preview_command("pause_play_mode", json!({}), Duration::from_millis(240))
}

#[tauri::command]
pub fn stop_preview() -> Result<PreviewCommandResult, String> {
    with_preview_command("stop_play_mode", json!({}), Duration::from_millis(240))
}

#[tauri::command]
pub fn poll_preview() -> Result<PreviewCommandResult, String> {
    let lock = PREVIEW_SESSION.get_or_init(|| Mutex::new(None));
    let mut guard = lock
        .lock()
        .map_err(|_| "preview session lock poisoned".to_string())?;
    if let Some(session) = guard.as_mut() {
        let events = collect_preview_events(session, Duration::from_millis(20));
        return Ok(preview_result(session, events));
    }

    Ok(PreviewCommandResult {
        status: "idle".to_string(),
        runner: None,
        events: Vec::new(),
    })
}

fn with_preview_command(
    kind: &str,
    payload: serde_json::Value,
    wait: Duration,
) -> Result<PreviewCommandResult, String> {
    let lock = PREVIEW_SESSION.get_or_init(|| Mutex::new(None));
    let mut guard = lock
        .lock()
        .map_err(|_| "preview session lock poisoned".to_string())?;
    let Some(session) = guard.as_mut() else {
        return Ok(PreviewCommandResult {
            status: "idle".to_string(),
            runner: None,
            events: Vec::new(),
        });
    };

    if !preview_is_alive(session) {
        session.status = "exited".to_string();
        let events = collect_preview_events(session, Duration::from_millis(20));
        return Ok(preview_result(session, events));
    }

    send_preview_command(session, kind, payload)?;
    let events = collect_preview_events(session, wait);
    Ok(preview_result(session, events))
}
