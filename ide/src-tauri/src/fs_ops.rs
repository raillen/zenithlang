use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::fs;
use std::path::{Path, PathBuf};
use std::sync::atomic::AtomicU32;
use std::sync::{Mutex, RwLock};
use tauri::{AppHandle, Emitter, Manager};
use notify::{Watcher, RecursiveMode, Config};
use walkdir::WalkDir;

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct FileEntry {
    pub name: String,
    pub path: String,
    pub is_directory: bool,
    pub children: Option<Vec<FileEntry>>,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct HistorySnapshot {
    pub id: i32,
    pub timestamp: u64,
    pub content: String,
}

pub struct WorkspaceIndex {
    pub root: RwLock<PathBuf>,
    pub files: RwLock<Vec<String>>,
    pub watcher: Mutex<Option<notify::RecommendedWatcher>>,
    pub history: RwLock<HashMap<String, Vec<HistorySnapshot>>>,
    pub next_history_id: AtomicU32,
}

impl Default for WorkspaceIndex {
    fn default() -> Self {
        Self {
            root: RwLock::new(PathBuf::new()),
            files: RwLock::new(Vec::new()),
            watcher: Mutex::new(None),
            history: RwLock::new(HashMap::new()),
            next_history_id: AtomicU32::new(1),
        }
    }
}

#[tauri::command]
pub fn get_file_tree(root_path: String, recursive: Option<bool>) -> Result<Vec<FileEntry>, String> {
    let base_path = resolve_path(&root_path)?;

    if !base_path.exists() {
        return Err(format!("Path does not exist: {}", base_path.display()));
    }

    scan_dir(&base_path, recursive.unwrap_or(false))
}

pub fn scan_dir(path: &Path, recursive: bool) -> Result<Vec<FileEntry>, String> {
    let mut entries = Vec::new();

    if let Ok(read_dir) = fs::read_dir(path) {
        for entry in read_dir.flatten() {
            let path = entry.path();
            let name = entry.file_name().to_string_lossy().to_string();

            if should_skip_workspace_entry(&name) {
                continue;
            }

            entries.push(file_entry_from_path(&path, recursive)?);
        }
    }

    sort_file_entries(&mut entries);

    Ok(entries)
}

pub fn should_skip_workspace_entry(name: &str) -> bool {
    matches!(
        name,
        ".git" | "node_modules" | "target" | "dist" | ".tauri"
    )
}

pub fn sort_file_entries(entries: &mut [FileEntry]) {
    entries.sort_by(|a, b| {
        if a.is_directory != b.is_directory {
            b.is_directory.cmp(&a.is_directory)
        } else {
            a.name.to_lowercase().cmp(&b.name.to_lowercase())
        }
    });
}

pub fn file_entry_from_path(path: &Path, recursive: bool) -> Result<FileEntry, String> {
    let metadata = fs::metadata(path).map_err(|e| format!("{} ({})", e, path.display()))?;
    let name = path
        .file_name()
        .map(|value| value.to_string_lossy().to_string())
        .unwrap_or_else(|| path.to_string_lossy().to_string());
    let is_directory = metadata.is_dir();
    let children = if is_directory && recursive {
        Some(scan_dir(path, true)?)
    } else {
        None
    };

    Ok(FileEntry {
        name,
        path: path.to_string_lossy().to_string(),
        is_directory,
        children,
    })
}

pub fn ide_root() -> PathBuf {
    PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .parent()
        .map(Path::to_path_buf)
        .unwrap_or_else(|| PathBuf::from(env!("CARGO_MANIFEST_DIR")))
}

pub fn workspace_root() -> PathBuf {
    let ide_root = ide_root();
    ide_root.parent().map(Path::to_path_buf).unwrap_or(ide_root)
}

pub fn active_workspace_root(state: &WorkspaceIndex) -> Result<PathBuf, String> {
    let root = state.root.read().map_err(|_| "Failed to lock root")?;
    if root.as_os_str().is_empty() {
        Ok(workspace_root())
    } else {
        Ok(root.clone())
    }
}

pub fn resolve_path(raw_path: &str) -> Result<PathBuf, String> {
    if raw_path.is_empty() {
        return std::env::current_dir().map_err(|e| e.to_string());
    }

    let requested = PathBuf::from(raw_path);
    if requested.is_absolute() {
        return Ok(requested);
    }

    let current_dir = std::env::current_dir().unwrap_or_else(|_| ide_root());
    let current_candidate = current_dir.join(&requested);
    if current_candidate.exists() {
        return Ok(current_candidate);
    }

    let workspace_candidate = workspace_root().join(&requested);
    if workspace_candidate.exists() {
        return Ok(workspace_candidate);
    }

    match raw_path {
        "." => Ok(current_dir),
        ".." => Ok(workspace_root()),
        _ => Ok(current_candidate),
    }
}

pub fn refresh_active_index(state: &WorkspaceIndex) {
    if let Ok(root) = active_workspace_root(state) {
        let _ = refresh_index_internal(state, &root);
    }
}

pub fn refresh_index_internal(state: &WorkspaceIndex, root: &Path) -> Result<(), String> {
    let mut new_files = Vec::new();

    for entry in WalkDir::new(root)
        .into_iter()
        .filter_map(|e| e.ok())
        .filter(|e| {
            let name = e.file_name().to_string_lossy();
            !name.starts_with('.') && name != "node_modules" && name != "target" && name != "dist"
        })
    {
        if entry.file_type().is_file() {
            new_files.push(entry.path().to_string_lossy().to_string());
        }
    }

    let active_root = state
        .root
        .read()
        .map_err(|_| "Failed to lock root")?;
    if active_root.as_path() != root {
        return Ok(());
    }

    let mut files_lock = state.files.write().map_err(|_| "Failed to lock index files")?;
    *files_lock = new_files;

    Ok(())
}

#[tauri::command]
pub fn read_file(path: String) -> Result<String, String> {
    let resolved_path = resolve_path(&path)?;
    let bytes = fs::read(&resolved_path).map_err(|e| format!("{} ({})", e, resolved_path.display()))?;
    Ok(String::from_utf8_lossy(&bytes).into_owned())
}

#[tauri::command]
pub fn write_file(
    app_handle: AppHandle,
    state: tauri::State<'_, WorkspaceIndex>,
    path: String,
    content: String,
) -> Result<(), String> {
    let resolved_path = resolve_path(&path)?;
    if let Some(parent) = resolved_path.parent() {
        fs::create_dir_all(parent).map_err(|e| format!("{} ({})", e, parent.display()))?;
    }

    fs::write(&resolved_path, content).map_err(|e| format!("{} ({})", e, resolved_path.display()))?;
    refresh_active_index(&state);
    let _ = app_handle.emit("zenith://fs-changed", ());
    Ok(())
}

#[tauri::command]
pub fn delete_path(
    state: tauri::State<'_, WorkspaceIndex>,
    path: String,
    recursive: bool,
) -> Result<(), String> {
    let target = resolve_path(&path)?;
    if !target.exists() {
        return Ok(());
    }

    let metadata = fs::metadata(&target).map_err(|e| format!("{} ({})", e, target.display()))?;
    if metadata.is_dir() {
        if recursive {
            fs::remove_dir_all(&target).map_err(|e| format!("{} ({})", e, target.display()))?;
        } else {
            fs::remove_dir(&target).map_err(|e| format!("{} ({})", e, target.display()))?;
        }
    } else {
        fs::remove_file(&target).map_err(|e| format!("{} ({})", e, target.display()))?;
    }

    refresh_active_index(&state);
    Ok(())
}

#[tauri::command]
pub fn workspace_index_bootstrap(
    app_handle: AppHandle,
    state: tauri::State<'_, WorkspaceIndex>,
    root_path: String,
) -> Result<(), String> {
    let root = resolve_path(&root_path)?;

    {
        let mut root_lock = state.root.write().map_err(|_| "Failed to lock root")?;
        *root_lock = root.clone();
    }

    {
        let mut watcher_lock = state.watcher.lock().map_err(|_| "Failed to lock watcher")?;
        *watcher_lock = None;
    }

    let app_handle_inner = app_handle.clone();
    let root_for_bootstrap = root.clone();
    tauri::async_runtime::spawn_blocking(move || {
        let state = app_handle_inner.state::<WorkspaceIndex>();
        let watcher_app = app_handle_inner.clone();

        match notify::RecommendedWatcher::new(
            move |res: notify::Result<notify::Event>| {
                if let Ok(event) = res {
                    if event.kind.is_modify() || event.kind.is_create() || event.kind.is_remove() {
                        let _ = watcher_app.emit("zenith://fs-changed", ());
                    }
                }
            },
            Config::default(),
        ) {
            Ok(mut watcher) => {
                if watcher.watch(&root_for_bootstrap, RecursiveMode::Recursive).is_ok() {
                    if let Ok(active_root) = state.root.read() {
                        if active_root.as_path() == root_for_bootstrap {
                            if let Ok(mut watcher_lock) = state.watcher.lock() {
                                *watcher_lock = Some(watcher);
                            }
                        }
                    }
                }
            }
            Err(error) => {
                eprintln!("Failed to create workspace watcher: {error}");
            }
        }

        if let Err(error) = refresh_index_internal(&state, &root_for_bootstrap) {
            eprintln!("Failed to refresh workspace index: {error}");
            return;
        }

        let _ = app_handle_inner.emit("zenith://index-ready", ());
    });

    Ok(())
}

#[tauri::command]
pub fn reveal_in_system_explorer(path: String) -> Result<(), String> {
    let target = resolve_path(&path)?;
    if !target.exists() {
        return Err(format!("Path does not exist: {}", target.display()));
    }

    #[cfg(target_os = "windows")]
    {
        std::process::Command::new("explorer")
            .arg(format!("/select,{}", target.display()))
            .spawn()
            .map_err(|e| format!("Failed to reveal file in Explorer: {e}"))?;
    }

    #[cfg(target_os = "macos")]
    {
        std::process::Command::new("open")
            .arg("-R")
            .arg(&target)
            .spawn()
            .map_err(|e| format!("Failed to reveal file in Finder: {e}"))?;
    }

    #[cfg(all(unix, not(target_os = "macos")))]
    {
        let parent = target.parent().unwrap_or(&target);
        std::process::Command::new("xdg-open")
            .arg(parent)
            .spawn()
            .map_err(|e| format!("Failed to reveal file in file manager: {e}"))?;
    }

    Ok(())
}

#[tauri::command]
pub fn copy_path(
    state: tauri::State<'_, WorkspaceIndex>,
    source_path: String,
    destination_parent_path: String,
) -> Result<FileEntry, String> {
    let source = resolve_path(&source_path)?;
    if !source.exists() {
        return Err(format!("Path does not exist: {}", source.display()));
    }

    let destination_parent = resolve_path(&destination_parent_path)?;
    if !destination_parent.is_dir() {
        return Err(format!(
            "Destination is not a folder: {}",
            destination_parent.display()
        ));
    }

    if source.is_dir() && is_same_or_child_path(&destination_parent, &source) {
        return Err("Cannot copy a folder into itself or one of its children".to_string());
    }

    let source_name = source
        .file_name()
        .map(|value| value.to_string_lossy().to_string())
        .ok_or_else(|| "Unable to determine source name".to_string())?;
    let destination = next_available_child_path(&destination_parent, &source_name);

    if source.is_dir() {
        copy_dir_recursive(&source, &destination)?;
    } else {
        fs::copy(&source, &destination)
            .map_err(|e| format!("{} ({} -> {})", e, source.display(), destination.display()))?;
    }

    let entry = file_entry_from_path(&destination, false)?;
    refresh_active_index(&state);
    Ok(entry)
}

#[tauri::command]
pub fn move_path(
    state: tauri::State<'_, WorkspaceIndex>,
    source_path: String,
    destination_parent_path: String,
) -> Result<FileEntry, String> {
    let source = resolve_path(&source_path)?;
    if !source.exists() {
        return Err(format!("Path does not exist: {}", source.display()));
    }

    let destination_parent = resolve_path(&destination_parent_path)?;
    if !destination_parent.is_dir() {
        return Err(format!(
            "Destination is not a folder: {}",
            destination_parent.display()
        ));
    }

    if source.is_dir() && is_same_or_child_path(&destination_parent, &source) {
        return Err("Cannot move a folder into itself or one of its children".to_string());
    }

    let source_name = source
        .file_name()
        .map(|value| value.to_string_lossy().to_string())
        .ok_or_else(|| "Unable to determine source name".to_string())?;
    let destination = next_available_child_path(&destination_parent, &source_name);

    fs::rename(&source, &destination)
        .map_err(|e| format!("{} ({} -> {})", e, source.display(), destination.display()))?;

    let entry = file_entry_from_path(&destination, false)?;
    refresh_active_index(&state);
    Ok(entry)
}

#[tauri::command]
pub fn rename_path(
    state: tauri::State<'_, WorkspaceIndex>,
    path: String,
    new_name: String,
) -> Result<FileEntry, String> {
    let source = resolve_path(&path)?;
    if !source.exists() {
        return Err(format!("Path does not exist: {}", source.display()));
    }

    let parent = source
        .parent()
        .ok_or_else(|| "Cannot rename a filesystem root".to_string())?;
    let destination = child_path(parent, &new_name, false)?;

    if destination.exists() {
        return Err(format!("Destination already exists: {}", destination.display()));
    }

    fs::rename(&source, &destination)
        .map_err(|e| format!("{} ({} -> {})", e, source.display(), destination.display()))?;
    let entry = file_entry_from_path(&destination, false)?;
    refresh_active_index(&state);
    Ok(entry)
}

#[tauri::command]
pub fn create_file(
    state: tauri::State<'_, WorkspaceIndex>,
    parent_path: String,
    name: String,
    content: Option<String>,
) -> Result<FileEntry, String> {
    let parent = resolve_path(&parent_path)?;
    if !parent.is_dir() {
        return Err(format!("Parent is not a folder: {}", parent.display()));
    }

    let path = child_path(&parent, &name, true)?;
    if path.exists() {
        return Err(format!("File already exists: {}", path.display()));
    }

    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent).map_err(|e| format!("{} ({})", e, parent.display()))?;
    }

    fs::write(&path, content.unwrap_or_default()).map_err(|e| format!("{} ({})", e, path.display()))?;
    let entry = file_entry_from_path(&path, false)?;
    refresh_active_index(&state);
    Ok(entry)
}

#[tauri::command]
pub fn create_folder(
    state: tauri::State<'_, WorkspaceIndex>,
    parent_path: String,
    name: String,
) -> Result<FileEntry, String> {
    let parent = resolve_path(&parent_path)?;
    if !parent.is_dir() {
        return Err(format!("Parent is not a folder: {}", parent.display()));
    }

    let path = child_path(&parent, &name, true)?;
    if path.exists() {
        return Err(format!("Folder already exists: {}", path.display()));
    }

    fs::create_dir_all(&path).map_err(|e| format!("{} ({})", e, path.display()))?;
    let entry = file_entry_from_path(&path, false)?;
    refresh_active_index(&state);
    Ok(entry)
}

#[tauri::command]
pub fn open_in_system_explorer(path: String) -> Result<(), String> {
    let target = resolve_path(&path)?;
    if !target.exists() {
        return Err(format!("Path does not exist: {}", target.display()));
    }

    let folder = if target.is_dir() {
        target
    } else {
        target.parent().unwrap_or(&target).to_path_buf()
    };

    #[cfg(target_os = "windows")]
    {
        std::process::Command::new("explorer")
            .arg(&folder)
            .spawn()
            .map_err(|e| format!("Failed to open folder in Explorer: {e}"))?;
    }

    #[cfg(target_os = "macos")]
    {
        std::process::Command::new("open")
            .arg(&folder)
            .spawn()
            .map_err(|e| format!("Failed to open folder in Finder: {e}"))?;
    }

    #[cfg(all(unix, not(target_os = "macos")))]
    {
        std::process::Command::new("xdg-open")
            .arg(&folder)
            .spawn()
            .map_err(|e| format!("Failed to open folder in file manager: {e}"))?;
    }

    Ok(())
}

#[tauri::command]
pub async fn history_save_snapshot(
    state: tauri::State<'_, WorkspaceIndex>,
    path: String,
    content: String,
    max_snapshots: i32,
    retention_days: i32
) -> Result<(), String> {
    let now = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map_err(|e| e.to_string())?
        .as_millis() as u64;
    let max_entries = max_snapshots.max(1) as usize;
    let retention_ms = (retention_days.max(0) as u64)
        .saturating_mul(24)
        .saturating_mul(60)
        .saturating_mul(60)
        .saturating_mul(1000);
        
    let snapshot = HistorySnapshot {
        id: state.next_history_id.fetch_add(1, std::sync::atomic::Ordering::Relaxed) as i32,
        timestamp: now,
        content,
    };

    let mut history = state
        .history
        .write()
        .map_err(|_| "Failed to lock file history".to_string())?;
    let entries = history.entry(path).or_default();
    entries.insert(0, snapshot);

    if retention_ms > 0 {
        entries.retain(|entry| now.saturating_sub(entry.timestamp) <= retention_ms);
    }

    if entries.len() > max_entries {
        entries.truncate(max_entries);
    }

    Ok(())
}

#[tauri::command]
pub async fn history_get_snapshots(
    state: tauri::State<'_, WorkspaceIndex>,
    path: String,
) -> Result<Vec<HistorySnapshot>, String> {
    let history = state
        .history
        .read()
        .map_err(|_| "Failed to read file history".to_string())?;

    Ok(history.get(&path).cloned().unwrap_or_default())
}

pub fn is_same_or_child_path(path: &Path, root: &Path) -> bool {
    path == root || path.starts_with(root)
}

pub fn split_file_name(name: &str) -> (String, String) {
    if let Some((stem, extension)) = name.rsplit_once('.') {
        if !stem.is_empty() && !extension.is_empty() {
            return (stem.to_string(), format!(".{extension}"));
        }
    }

    (name.to_string(), String::new())
}

pub fn next_available_child_path(parent: &Path, preferred_name: &str) -> PathBuf {
    let candidate = parent.join(preferred_name);
    if !candidate.exists() {
        return candidate;
    }

    let (stem, extension) = split_file_name(preferred_name);
    for index in 2..1000 {
        let next_name = if extension.is_empty() {
            format!("{stem} {index}")
        } else {
            format!("{stem} {index}{extension}")
        };
        let next_path = parent.join(next_name);
        if !next_path.exists() {
            return next_path;
        }
    }

    candidate
}

pub fn child_path(parent: &Path, name: &str, allow_nested: bool) -> Result<PathBuf, String> {
    use std::path::Component;

    let trimmed = name.trim();
    if trimmed.is_empty() {
        return Err("Name cannot be empty".to_string());
    }

    let requested = Path::new(trimmed);
    if requested.is_absolute() {
        return Err("Use a relative name inside the workspace".to_string());
    }

    let mut destination = parent.to_path_buf();
    let mut segment_count = 0;

    for component in requested.components() {
        match component {
            Component::Normal(segment) => {
                if segment.to_string_lossy().trim().is_empty() {
                    return Err("Name cannot contain empty path segments".to_string());
                }

                destination.push(segment);
                segment_count += 1;
            }
            _ => return Err("Name cannot contain path traversal or root markers".to_string()),
        }
    }

    if segment_count == 0 {
        return Err("Name cannot be empty".to_string());
    }

    if !allow_nested && segment_count > 1 {
        return Err("Rename accepts only a single file or folder name".to_string());
    }

    Ok(destination)
}

pub fn copy_dir_recursive(source: &Path, destination: &Path) -> Result<(), String> {
    fs::create_dir_all(destination).map_err(|e| format!("{} ({})", e, destination.display()))?;

    for entry in fs::read_dir(source).map_err(|e| format!("{} ({})", e, source.display()))? {
        let entry = entry.map_err(|e| format!("{} ({})", e, source.display()))?;
        let entry_path = entry.path();
        let destination_path = destination.join(entry.file_name());

        if entry_path.is_dir() {
            copy_dir_recursive(&entry_path, &destination_path)?;
        } else {
            if let Some(parent) = destination_path.parent() {
                fs::create_dir_all(parent)
                    .map_err(|e| format!("{} ({})", e, parent.display()))?;
            }

            fs::copy(&entry_path, &destination_path).map_err(|e| {
                format!(
                    "{} ({} -> {})",
                    e,
                    entry_path.display(),
                    destination_path.display()
                )
            })?;
        }
    }

    Ok(())
}

#[derive(serde::Serialize, serde::Deserialize, Clone, Debug)]
pub struct WorkspaceContext {
    pub root_path: String,
    pub settings: serde_json::Value,
}

fn get_workspaces_file() -> PathBuf {
    workspace_root().join(".zenith_workspaces.json")
}

#[tauri::command]
pub fn workspace_list_contexts() -> Result<Vec<String>, String> {
    let path = get_workspaces_file();
    if !path.exists() {
        return Ok(Vec::new());
    }
    let content = fs::read_to_string(path).unwrap_or_default();
    let data: HashMap<String, WorkspaceContext> = serde_json::from_str(&content).unwrap_or_default();
    let mut names: Vec<String> = data.keys().cloned().collect();
    names.sort();
    Ok(names)
}

#[tauri::command]
pub fn workspace_load_context(name: String) -> Result<WorkspaceContext, String> {
    let path = get_workspaces_file();
    if !path.exists() {
        return Err("Context not found".to_string());
    }
    let content = fs::read_to_string(path).map_err(|e| e.to_string())?;
    let data: HashMap<String, WorkspaceContext> = serde_json::from_str(&content).unwrap_or_default();
    data.get(&name).cloned().ok_or_else(|| "Context not found".to_string())
}

#[tauri::command]
pub fn workspace_save_context(name: String, context: WorkspaceContext) -> Result<(), String> {
    let path = get_workspaces_file();
    let mut data: HashMap<String, WorkspaceContext> = if path.exists() {
        let content = fs::read_to_string(&path).unwrap_or_else(|_| "{}".to_string());
        serde_json::from_str(&content).unwrap_or_default()
    } else {
        HashMap::new()
    };
    data.insert(name, context);
    let new_content = serde_json::to_string_pretty(&data).map_err(|e| e.to_string())?;
    fs::write(path, new_content).map_err(|e| e.to_string())?;
    Ok(())
}

#[tauri::command]
pub fn workspace_delete_context(name: String) -> Result<(), String> {
    let path = get_workspaces_file();
    if !path.exists() {
        return Ok(());
    }
    let content = fs::read_to_string(&path).map_err(|e| e.to_string())?;
    let mut data: HashMap<String, WorkspaceContext> = serde_json::from_str(&content).unwrap_or_default();
    data.remove(&name);
    let new_content = serde_json::to_string_pretty(&data).map_err(|e| e.to_string())?;
    fs::write(path, new_content).map_err(|e| e.to_string())?;
    Ok(())
}
