mod lsp;

use portable_pty::{native_pty_system, Child, ChildKiller, CommandBuilder, MasterPty, PtySize};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::fs;
use std::io::{Read, Write};
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicU32, Ordering};
use std::sync::{Arc, Mutex, RwLock};
use tauri::{AppHandle, Emitter, Manager};
use notify::{Watcher, RecursiveMode, Config};
use sysinfo::System;
use crate::lsp::{
    LspState,
    lsp_completion,
    lsp_definition,
    lsp_hover,
    lsp_latest_diagnostics,
    lsp_references,
    lsp_rename,
    lsp_shutdown,
    lsp_sync_document,
};

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct FileEntry {
    name: String,
    path: String,
    is_directory: bool,
    children: Option<Vec<FileEntry>>,
}

#[derive(Serialize, Clone)]
struct HistorySnapshot {
    id: i32,
    timestamp: u64,
    content: String,
}

pub struct WorkspaceIndex {
    pub root: RwLock<PathBuf>,
    pub files: RwLock<Vec<String>>,
    pub watcher: Mutex<Option<notify::RecommendedWatcher>>,
    history: RwLock<HashMap<String, Vec<HistorySnapshot>>>,
    next_history_id: AtomicU32,
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

type PtyHandler = u32;

const TERMINAL_DATA_EVENT: &str = "zenith://terminal-data";
const TERMINAL_EXIT_EVENT: &str = "zenith://terminal-exit";

struct TerminalSession {
    master: Mutex<Box<dyn MasterPty + Send>>,
    child: Mutex<Box<dyn Child + Send + Sync>>,
    child_killer: Mutex<Box<dyn ChildKiller + Send + Sync>>,
    writer: Mutex<Box<dyn Write + Send>>,
    reader: Mutex<Box<dyn Read + Send>>,
}

#[derive(Default)]
struct TerminalState {
    next_session_id: AtomicU32,
    sessions: Mutex<HashMap<PtyHandler, Arc<TerminalSession>>>,
}

impl TerminalState {
    fn allocate_session_id(&self) -> PtyHandler {
        self.next_session_id.fetch_add(1, Ordering::Relaxed) + 1
    }

    fn get_session(&self, session_id: PtyHandler) -> Result<Arc<TerminalSession>, String> {
        self.sessions
            .lock()
            .map_err(|_| "Unable to access terminal sessions".to_string())?
            .get(&session_id)
            .cloned()
            .ok_or_else(|| format!("Terminal session {} is not available", session_id))
    }

    fn remove_session(&self, session_id: PtyHandler) {
        if let Ok(mut sessions) = self.sessions.lock() {
            sessions.remove(&session_id);
        }
    }
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct TerminalSessionInfo {
    session_id: PtyHandler,
    cwd: String,
    shell: String,
}

#[derive(Serialize, Clone)]
#[serde(rename_all = "camelCase")]
struct TerminalDataPayload {
    session_id: PtyHandler,
    data: String,
}

#[derive(Serialize, Clone)]
#[serde(rename_all = "camelCase")]
struct TerminalExitPayload {
    session_id: PtyHandler,
    exit_code: Option<u32>,
}

#[tauri::command]
fn get_file_tree(root_path: String) -> Result<Vec<FileEntry>, String> {
    let base_path = resolve_path(&root_path)?;

    if !base_path.exists() {
        return Err(format!("Path does not exist: {}", base_path.display()));
    }

    scan_dir(&base_path)
}

fn scan_dir(path: &Path) -> Result<Vec<FileEntry>, String> {
    let mut entries = Vec::new();

    if let Ok(read_dir) = fs::read_dir(path) {
        for entry in read_dir.flatten() {
            let path = entry.path();
            let name = entry.file_name().to_string_lossy().to_string();

            if should_skip_workspace_entry(&name) {
                continue;
            }

            entries.push(file_entry_from_path(&path)?);
        }
    }

    sort_file_entries(&mut entries);

    Ok(entries)
}

fn should_skip_workspace_entry(name: &str) -> bool {
    matches!(
        name,
        ".git" | "node_modules" | "target" | "dist" | ".tauri"
    )
}

fn sort_file_entries(entries: &mut [FileEntry]) {
    entries.sort_by(|a, b| {
        if a.is_directory != b.is_directory {
            b.is_directory.cmp(&a.is_directory)
        } else {
            a.name.to_lowercase().cmp(&b.name.to_lowercase())
        }
    });
}

fn file_entry_from_path(path: &Path) -> Result<FileEntry, String> {
    let metadata = fs::metadata(path).map_err(|e| format!("{} ({})", e, path.display()))?;
    let name = path
        .file_name()
        .map(|value| value.to_string_lossy().to_string())
        .unwrap_or_else(|| path.to_string_lossy().to_string());
    let is_directory = metadata.is_dir();
    let children = if is_directory {
        Some(scan_dir(path)?)
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

fn child_path(parent: &Path, name: &str, allow_nested: bool) -> Result<PathBuf, String> {
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

fn copy_dir_recursive(source: &Path, destination: &Path) -> Result<(), String> {
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

fn split_file_name(name: &str) -> (String, String) {
    if let Some((stem, extension)) = name.rsplit_once('.') {
        if !stem.is_empty() && !extension.is_empty() {
            return (stem.to_string(), format!(".{extension}"));
        }
    }

    (name.to_string(), String::new())
}

fn next_available_child_path(parent: &Path, preferred_name: &str) -> PathBuf {
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

fn is_same_or_child_path(path: &Path, root: &Path) -> bool {
    path == root || path.starts_with(root)
}

#[tauri::command]
fn read_file(path: String) -> Result<String, String> {
    let resolved_path = resolve_path(&path)?;
    let bytes = fs::read(&resolved_path).map_err(|e| format!("{} ({})", e, resolved_path.display()))?;
    Ok(String::from_utf8_lossy(&bytes).into_owned())
}

#[tauri::command]
fn write_file(
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
fn create_file(
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
    let entry = file_entry_from_path(&path)?;
    refresh_active_index(&state);
    Ok(entry)
}

#[tauri::command]
fn create_folder(
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
    let entry = file_entry_from_path(&path)?;
    refresh_active_index(&state);
    Ok(entry)
}

#[tauri::command]
fn copy_path(
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

    let entry = file_entry_from_path(&destination)?;
    refresh_active_index(&state);
    Ok(entry)
}

#[tauri::command]
fn move_path(
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

    let entry = file_entry_from_path(&destination)?;
    refresh_active_index(&state);
    Ok(entry)
}

#[tauri::command]
fn rename_path(
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
    let entry = file_entry_from_path(&destination)?;
    refresh_active_index(&state);
    Ok(entry)
}

#[tauri::command]
fn delete_path(
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
fn reveal_in_system_explorer(path: String) -> Result<(), String> {
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
fn open_in_system_explorer(path: String) -> Result<(), String> {
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
fn run_compiler(input_path: String) -> Result<String, String> {
    use std::process::Command;

    let resolved_input_path = resolve_path(&input_path)?;
    let project_root = workspace_root();
    let compiler_path = project_root.join("ztc.lua");

    if !compiler_path.exists() {
        return Err(format!(
            "Compiler script not found at {}",
            compiler_path.display()
        ));
    }

    let output = Command::new("lua")
        .arg(&compiler_path)
        .arg(&resolved_input_path)
        .current_dir(&project_root)
        .output()
        .map_err(|e| format!("Failed to execute lua: {}", e))?;

    let stdout = String::from_utf8_lossy(&output.stdout).to_string();
    let stderr = String::from_utf8_lossy(&output.stderr).to_string();

    if output.status.success() {
        Ok(stdout)
    } else {
        Err(format!("Error: {}\n{}", stdout, stderr))
    }
}

#[tauri::command]
fn pick_file() -> Result<Option<String>, String> {
    Ok(rfd::FileDialog::new()
        .pick_file()
        .map(|path| path.to_string_lossy().to_string()))
}

#[tauri::command]
fn pick_folder() -> Result<Option<String>, String> {
    Ok(rfd::FileDialog::new()
        .pick_folder()
        .map(|path| path.to_string_lossy().to_string()))
}

#[tauri::command]
fn pick_save_path(default_name: Option<String>) -> Result<Option<String>, String> {
    let mut dialog = rfd::FileDialog::new();
    if let Some(name) = default_name {
        dialog = dialog.set_file_name(&name);
    }

    Ok(dialog
        .save_file()
        .map(|path| path.to_string_lossy().to_string()))
}

#[derive(serde::Serialize, Debug, Clone)]
struct Diagnostic {
    line: u32,
    col: u32,
    message: String,
    severity: String,
    code: String,
}

#[tauri::command]
fn run_diagnostics(_path: String, content: String) -> Result<Vec<Diagnostic>, String> {
    use std::process::Command;
    use regex::Regex;
    use std::fs;

    // Create a temporary file for checking (to support as-you-type without saving)
    let temp_path = workspace_root().join(".check.zt");
    fs::write(&temp_path, content).map_err(|e| format!("Failed to write temp file: {}", e))?;

    let output = Command::new("lua")
        .arg("ztc.lua")
        .arg(&temp_path)
        .current_dir(workspace_root())
        .output()
        .map_err(|e| format!("Failed to execute ztc.lua: {}", e))?;

    let stderr = String::from_utf8_lossy(&output.stderr);
    let stdout = String::from_utf8_lossy(&output.stdout);
    let combined = format!("{}{}", stdout, stderr);

    let mut diagnostics = Vec::new();

    // Regex for: ÔØî Erro [ZT-P002]: express├úo esperada, encontrado RPAREN
    // and --> test_error.zt:1:13
    let error_re = Regex::new(r"Erro \[(?P<code>[^\]]+)\]:\s+(?P<msg>.+?)\r?\n\s+-->\s+.+:(?P<line>\d+):(?P<col>\d+)").unwrap();

    // Clean-up temp file
    let _ = fs::remove_file(&temp_path);

    for cap in error_re.captures_iter(&combined) {
        diagnostics.push(Diagnostic {
            line: cap["line"].parse().unwrap_or(1),
            col: cap["col"].parse().unwrap_or(1),
            message: cap["msg"].to_string(),
            severity: "error".to_string(),
            code: cap["code"].to_string(),
        });
    }

    Ok(diagnostics)
}

#[tauri::command]
fn get_git_status(state: tauri::State<'_, WorkspaceIndex>) -> Result<HashMap<String, String>, String> {
    use std::process::Command;

    let root = active_workspace_root(&state)?;
    let output = Command::new("git")
        .arg("status")
        .arg("--porcelain")
        .current_dir(&root)
        .output()
        .map_err(|e| format!("Failed to execute git status: {}", e))?;

    if !output.status.success() {
        // Might not be a git repo
        return Ok(HashMap::new());
    }

    let stdout = String::from_utf8_lossy(&output.stdout);
    let mut status_map = HashMap::new();

    for line in stdout.lines() {
        if line.len() > 3 {
            let status = line[..2].to_string(); // Keep Both XY status
            let path = line[3..].trim().trim_matches('"').to_string();
            let abs_path = root.join(path).to_string_lossy().to_string();
            status_map.insert(abs_path, status);
        }
    }

    Ok(status_map)
}

#[tauri::command]
fn git_stage(state: tauri::State<'_, WorkspaceIndex>, path: String) -> Result<(), String> {
    use std::process::Command;
    let root = active_workspace_root(&state)?;
    let output = Command::new("git")
        .arg("add")
        .arg(path)
        .current_dir(&root)
        .output()
        .map_err(|e| format!("Failed to stage file: {}", e))?;

    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }
    Ok(())
}

#[tauri::command]
fn git_unstage(state: tauri::State<'_, WorkspaceIndex>, path: String) -> Result<(), String> {
    use std::process::Command;
    let root = active_workspace_root(&state)?;
    let output = Command::new("git")
        .arg("reset")
        .arg("HEAD")
        .arg(path)
        .current_dir(&root)
        .output()
        .map_err(|e| format!("Failed to unstage file: {}", e))?;

    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }
    Ok(())
}

#[tauri::command]
fn git_commit(state: tauri::State<'_, WorkspaceIndex>, message: String) -> Result<(), String> {
    use std::process::Command;
    let root = active_workspace_root(&state)?;
    let output = Command::new("git")
        .arg("commit")
        .arg("-m")
        .arg(message)
        .current_dir(&root)
        .output()
        .map_err(|e| format!("Failed to commit: {}", e))?;

    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }
    Ok(())
}

#[tauri::command]
fn git_stage_all(state: tauri::State<'_, WorkspaceIndex>) -> Result<(), String> {
    use std::process::Command;
    let root = active_workspace_root(&state)?;
    let output = Command::new("git")
        .arg("add")
        .arg(".")
        .current_dir(&root)
        .output()
        .map_err(|e| format!("Failed to stage all: {}", e))?;

    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }
    Ok(())
}

#[tauri::command]
fn git_unstage_all(state: tauri::State<'_, WorkspaceIndex>) -> Result<(), String> {
    use std::process::Command;
    let root = active_workspace_root(&state)?;
    let output = Command::new("git")
        .arg("reset")
        .arg("HEAD")
        .arg(".")
        .current_dir(&root)
        .output()
        .map_err(|e| format!("Failed to unstage all: {}", e))?;

    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }
    Ok(())
}

#[tauri::command]
fn git_discard_changes(state: tauri::State<'_, WorkspaceIndex>, path: String) -> Result<(), String> {
    use std::process::Command;
    let root = active_workspace_root(&state)?;
    let output = Command::new("git")
        .arg("checkout")
        .arg("--")
        .arg(path)
        .current_dir(&root)
        .output()
        .map_err(|e| format!("Failed to discard changes: {}", e))?;

    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }
    Ok(())
}

#[tauri::command]
fn git_read_original(state: tauri::State<'_, WorkspaceIndex>, path: String) -> Result<String, String> {
    use std::process::Command;
    let root = active_workspace_root(&state)?;
    
    // Resolve relative path for git show
    let abs_path = PathBuf::from(&path);
    let relative_path = abs_path.strip_prefix(&root)
        .map_err(|_| "Path is outside workspace root".to_string())?
        .to_string_lossy()
        .replace("\\", "/");

    let output = Command::new("git")
        .arg("show")
        .arg(format!("HEAD:{}", relative_path))
        .current_dir(&root)
        .output()
        .map_err(|e| format!("Failed to read original file: {}", e))?;

    if !output.status.success() {
        // If file is new, return empty string instead of error
        return Ok(String::new());
    }

    Ok(String::from_utf8_lossy(&output.stdout).to_string())
}

#[tauri::command]
async fn history_save_snapshot(
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
        id: state.next_history_id.fetch_add(1, Ordering::Relaxed) as i32,
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
async fn history_get_snapshots(
    state: tauri::State<'_, WorkspaceIndex>,
    path: String,
) -> Result<Vec<HistorySnapshot>, String> {
    let history = state
        .history
        .read()
        .map_err(|_| "Failed to read file history".to_string())?;

    Ok(history.get(&path).cloned().unwrap_or_default())
}

#[derive(Serialize)]
struct SearchMatch {
    line_number: usize,
    line_content: String,
}

#[derive(Serialize)]
struct FileResult {
    file_path: String,
    matches: Vec<SearchMatch>,
}

#[tauri::command]
fn search_in_files(
    state: tauri::State<'_, WorkspaceIndex>,
    query: String,
    is_regex: bool,
    match_case: bool,
    whole_word: bool,
    path_filter: Option<String>,
) -> Result<Vec<FileResult>, String> {
    let mut all_results = Vec::new();
    let root = state.root.read().map_err(|_| "Failed to lock root")?;
    let files = state.files.read().map_err(|_| "Failed to lock index")?;

    let filter_path = path_filter.map(|p| root.join(p.replace("\\", "/")));

    let query_lower = if !match_case { query.to_lowercase() } else { query.clone() };

    // Prepare Regex for either explicit regex search or whole word search
    let re = if is_regex || whole_word {
        let pattern = if is_regex {
            query.clone()
        } else {
            // Escape literal for safe inclusion in regex
            format!(r"\b{}\b", regex::escape(&query))
        };

        match regex::RegexBuilder::new(&pattern).case_insensitive(!match_case).build() {
            Ok(r) => Some(r),
            Err(e) => return Err(format!("Invalid search pattern: {}", e)),
        }
    } else {
        None
    };

    for path_str in files.iter() {
        let path = Path::new(path_str);

        // Path filtering
        if let Some(ref filter) = filter_path {
            if !path.starts_with(filter) {
                continue;
            }
        }

        // Skip common binary types or very large files for now
        let name = path.file_name().map(|n| n.to_string_lossy().to_string()).unwrap_or_default();
        if name.ends_with(".db") || name.ends_with(".png") || name.ends_with(".jpg") || name.ends_with(".woff2") || name.ends_with(".exe") {
            continue;
        }

        if let Ok(content) = fs::read_to_string(&path) {
            let mut file_matches = Vec::new();
            for (i, line) in content.lines().enumerate() {
                let matched = if let Some(ref r) = re {
                    r.is_match(line)
                } else if match_case {
                    line.contains(&query)
                } else {
                    line.to_lowercase().contains(&query_lower)
                };

                if matched {
                    file_matches.push(SearchMatch {
                        line_number: i + 1,
                        line_content: line.trim().to_string(),
                    });
                }

                if file_matches.len() >= 100 { break; } // Limit per file
            }

            if !file_matches.is_empty() {
                let clean_path = path.strip_prefix(&*root).unwrap_or(&path).to_string_lossy().to_string();
                all_results.push(FileResult {
                    file_path: clean_path.replace("\\", "/"),
                    matches: file_matches,
                });
            }
        }

        if all_results.len() >= 200 { break; } // Total result limit
    }

    all_results.sort_by(|a, b| a.file_path.cmp(&b.file_path));
    Ok(all_results)
}

#[tauri::command]
fn replace_in_files(
    state: tauri::State<'_, WorkspaceIndex>,
    query: String,
    replacement: String,
    is_regex: bool,
    match_case: bool,
    whole_word: bool,
    target_files: Vec<String>,
) -> Result<usize, String> {
    let root = state.root.read().map_err(|_| "Failed to lock root")?;
    let mut replaced_count = 0;

    let re = if is_regex || whole_word {
        let pattern = if is_regex {
            query.clone()
        } else {
            format!(r"\b{}\b", regex::escape(&query))
        };

        match regex::RegexBuilder::new(&pattern).case_insensitive(!match_case).build() {
            Ok(r) => Some(r),
            Err(e) => return Err(format!("Invalid search pattern: {}", e)),
        }
    } else {
        None
    };

    for rel_path in target_files {
        let abs_path = root.join(rel_path.replace("\\", "/"));
        if !abs_path.exists() { continue; }

        if let Ok(content) = fs::read_to_string(&abs_path) {
            let new_content = if let Some(ref r) = re {
                r.replace_all(&content, &*replacement).to_string()
            } else if match_case {
                content.replace(&query, &replacement)
            } else {
                // For case-insensitive literal replace, we still use regex but escaped
                let r_lite = regex::RegexBuilder::new(&regex::escape(&query))
                    .case_insensitive(true)
                    .build()
                    .map_err(|e| e.to_string())?;
                r_lite.replace_all(&content, &*replacement).to_string()
            };

            if new_content != content {
                fs::write(&abs_path, new_content).map_err(|e| format!("Failed to write to file: {}", e))?;
                replaced_count += 1;
            }
        }
    }

    Ok(replaced_count)
}

#[derive(Serialize)]
struct FileNameResult {
    name: String,
    path: String,
}

#[tauri::command]
fn search_file_names(
    state: tauri::State<'_, WorkspaceIndex>,
    query: String
) -> Result<Vec<FileNameResult>, String> {
    let query_lower = query.to_lowercase();
    let files = state.files.read().map_err(|_| "Failed to lock index")?;
    let root = state.root.read().map_err(|_| "Failed to lock root")?;

    let mut results = Vec::new();

    for path_str in files.iter() {
        let path = Path::new(path_str);
        let name = path.file_name().map(|n| n.to_string_lossy().to_string()).unwrap_or_default();

        if query_lower.is_empty() || name.to_lowercase().contains(&query_lower) {
            let relative_path = path.strip_prefix(&*root).unwrap_or(path).to_string_lossy().to_string();
            results.push(FileNameResult {
                name,
                path: relative_path.replace("\\", "/"),
            });
        }

        if results.len() >= 100 {
            break;
        }
    }

    results.sort_by(|a, b| a.name.cmp(&b.name));
    Ok(results)
}


#[derive(Serialize, Clone, Debug)]
pub enum SymbolKind {
    Function,
    Struct,
    Variable,
    Constant,
    Interface,
}

#[derive(Serialize, Clone, Debug)]
pub struct Symbol {
    name: String,
    kind: SymbolKind,
    line: usize,
    col: usize,
}

#[tauri::command]
fn get_file_symbols(content: String) -> Result<Vec<Symbol>, String> {
    use regex::Regex;

    let mut symbols = Vec::new();

    // Patterns for Zenith v0.2
    let func_re = Regex::new(r"(?m)^func\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\(").unwrap();
    let struct_re = Regex::new(r"(?m)^struct\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*").unwrap();
    let const_re = Regex::new(r"(?m)^const\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*[:=]").unwrap();
    let var_re = Regex::new(r"(?m)^var\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*[:=]").unwrap();
    let interface_re = Regex::new(r"(?m)^interface\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\{").unwrap();

    // Mapping patterns to kinds
    let patterns = [
        (&func_re, SymbolKind::Function),
        (&struct_re, SymbolKind::Struct),
        (&const_re, SymbolKind::Constant),
        (&var_re, SymbolKind::Variable),
        (&interface_re, SymbolKind::Interface),
    ];

    for (re, kind) in patterns {
        for cap in re.captures_iter(&content) {
            let name = cap[1].to_string();
            let pos = cap.get(1).unwrap().start();

            // Calculate line and col from byte position
            let line = content[..pos].lines().count();
            let col = content[..pos].lines().last().map(|l| l.len() + 1).unwrap_or(1);

            symbols.push(Symbol {
                name,
                kind: kind.clone(),
                line,
                col,
            });
        }
    }

    // Sort by line number
    symbols.sort_by_key(|s| s.line);

    Ok(symbols)
}

#[tauri::command]
fn greet(name: &str) -> String {
    format!("Hello, {}! You've been greeted from Zenith App!", name)
}

#[tauri::command]
fn terminal_create(
    app_handle: AppHandle,
    state: tauri::State<'_, TerminalState>,
    cwd: Option<String>,
    shell: Option<String>,
    cols: Option<u16>,
    rows: Option<u16>,
) -> Result<TerminalSessionInfo, String> {
    let resolved_cwd = cwd
        .as_deref()
        .map(resolve_path)
        .transpose()?
        .unwrap_or_else(workspace_root);
    let shell_path = resolve_shell(shell);
    let size = PtySize {
        rows: rows.unwrap_or(32).max(1),
        cols: cols.unwrap_or(120).max(20),
        pixel_width: 0,
        pixel_height: 0,
    };

    let pty_system = native_pty_system();
    let portable_pty::PtyPair { master, slave } = pty_system
        .openpty(size)
        .map_err(|error| format!("Unable to open pty: {error}"))?;

    let command = build_shell_command(&shell_path, &resolved_cwd);
    let child = slave
        .spawn_command(command)
        .map_err(|error| format!("Unable to spawn shell: {error}"))?;
    let child_killer = child.clone_killer();
    let reader = master
        .try_clone_reader()
        .map_err(|error| format!("Unable to attach terminal reader: {error}"))?;
    let writer = master
        .take_writer()
        .map_err(|error| format!("Unable to attach terminal writer: {error}"))?;

    let session_id = state.allocate_session_id();
    let session = Arc::new(TerminalSession {
        master: Mutex::new(master),
        child: Mutex::new(child),
        child_killer: Mutex::new(child_killer),
        writer: Mutex::new(writer),
        reader: Mutex::new(reader),
    });

    state
        .sessions
        .lock()
        .map_err(|_| "Unable to register terminal session".to_string())?
        .insert(session_id, session.clone());

    spawn_terminal_reader(app_handle.clone(), session_id, session.clone());
    spawn_terminal_waiter(app_handle, session_id, session);

    Ok(TerminalSessionInfo {
        session_id,
        cwd: resolved_cwd.to_string_lossy().to_string(),
        shell: shell_path,
    })
}

#[tauri::command]
fn terminal_write(
    session_id: PtyHandler,
    data: String,
    state: tauri::State<'_, TerminalState>,
) -> Result<(), String> {
    let session = state.get_session(session_id)?;
    let mut writer = session
        .writer
        .lock()
        .map_err(|_| "Unable to write to the terminal session".to_string())?;

    writer
        .write_all(data.as_bytes())
        .map_err(|error| format!("Unable to write to the shell: {error}"))?;
    writer
        .flush()
        .map_err(|error| format!("Unable to flush terminal input: {error}"))?;

    Ok(())
}

#[tauri::command]
fn workspace_index_bootstrap(
    app_handle: AppHandle,
    state: tauri::State<'_, WorkspaceIndex>,
    root_path: String,
) -> Result<(), String> {
    let root = resolve_path(&root_path)?;

    // Update root in state
    {
        let mut root_lock = state.root.write().map_err(|_| "Failed to lock root")?;
        *root_lock = root.clone();
    }

    // Initialize Watcher
    let app_handle_inner = app_handle.clone();
    let mut watcher = notify::RecommendedWatcher::new(move |res: notify::Result<notify::Event>| {
        if let Ok(event) = res {
            // Only emit if it's a data change or structural change
            if event.kind.is_modify() || event.kind.is_create() || event.kind.is_remove() {
                let _ = app_handle_inner.emit("zenith://fs-changed", ());
            }
        }
    }, Config::default()).map_err(|e| e.to_string())?;

    watcher.watch(&root, RecursiveMode::Recursive).map_err(|e| e.to_string())?;

    // Store watcher
    {
        let mut watcher_lock = state.watcher.lock().map_err(|_| "Failed to lock watcher")?;
        *watcher_lock = Some(watcher);
    }

    // Perform initial index
    refresh_index_internal(&state, &root)?;

    let _ = app_handle.emit("zenith://index-ready", ());
    Ok(())
}

fn refresh_index_internal(state: &WorkspaceIndex, root: &Path) -> Result<(), String> {
    use walkdir::WalkDir;

    let mut new_files = Vec::new();

    // We use a simple walkdir for now, but we could use 'ignore' crate for .gitignore awareness
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

    let mut files_lock = state.files.write().map_err(|_| "Failed to lock index files")?;
    *files_lock = new_files;

    Ok(())
}

#[tauri::command]
fn terminal_resize(
    session_id: PtyHandler,
    cols: u16,
    rows: u16,
    state: tauri::State<'_, TerminalState>,
) -> Result<(), String> {
    let session = state.get_session(session_id)?;
    let master = session
        .master
        .lock()
        .map_err(|_| "Unable to resize the terminal session".to_string())?;

    master
        .resize(PtySize {
            rows: rows.max(1),
            cols: cols.max(20),
            pixel_width: 0,
            pixel_height: 0,
        })
        .map_err(|error| format!("Unable to apply terminal size: {error}"))?;

    Ok(())
}

#[tauri::command]
fn terminal_kill(
    session_id: PtyHandler,
    state: tauri::State<'_, TerminalState>,
) -> Result<(), String> {
    let session = state.get_session(session_id)?;
    let mut killer = session
        .child_killer
        .lock()
        .map_err(|_| "Unable to stop the terminal session".to_string())?;

    killer
        .kill()
        .map_err(|error| format!("Unable to stop the shell process: {error}"))?;

    state.remove_session(session_id);
    Ok(())
}

fn ide_root() -> PathBuf {
    PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .parent()
        .map(Path::to_path_buf)
        .unwrap_or_else(|| PathBuf::from(env!("CARGO_MANIFEST_DIR")))
}

fn workspace_root() -> PathBuf {
    let ide_root = ide_root();
    ide_root.parent().map(Path::to_path_buf).unwrap_or(ide_root)
}

fn active_workspace_root(state: &WorkspaceIndex) -> Result<PathBuf, String> {
    let root = state.root.read().map_err(|_| "Failed to lock root")?;
    if root.as_os_str().is_empty() {
        Ok(workspace_root())
    } else {
        Ok(root.clone())
    }
}

fn refresh_active_index(state: &WorkspaceIndex) {
    if let Ok(root) = active_workspace_root(state) {
        let _ = refresh_index_internal(state, &root);
    }
}

fn resolve_shell(shell: Option<String>) -> String {
    if let Some(shell) = shell.filter(|value| !value.trim().is_empty()) {
        return shell;
    }

    #[cfg(windows)]
    {
        if let Ok(system_root) = std::env::var("SystemRoot") {
            let powershell = PathBuf::from(system_root)
                .join("System32")
                .join("WindowsPowerShell")
                .join("v1.0")
                .join("powershell.exe");

            if powershell.exists() {
                return powershell.to_string_lossy().to_string();
            }
        }

        return std::env::var("COMSPEC").unwrap_or_else(|_| "cmd.exe".to_string());
    }

    #[cfg(not(windows))]
    {
        std::env::var("SHELL").unwrap_or_else(|_| "/bin/bash".to_string())
    }
}

fn build_shell_command(shell: &str, cwd: &Path) -> CommandBuilder {
    let mut command = CommandBuilder::new(shell);
    let shell_lower = shell.to_ascii_lowercase();

    if shell_lower.ends_with("powershell.exe") || shell_lower.ends_with("pwsh.exe") {
        command.arg("-NoLogo");
        command.arg("-NoProfile");
    } else if shell_lower.ends_with("/bash")
        || shell_lower.ends_with("/zsh")
        || shell_lower.ends_with("/fish")
    {
        command.arg("-l");
    }

    command.cwd(cwd);
    command.env("TERM", "xterm-256color");
    command.env("COLORTERM", "truecolor");
    command
}

fn resolve_path(raw_path: &str) -> Result<PathBuf, String> {
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

fn spawn_terminal_reader(app_handle: AppHandle, session_id: PtyHandler, session: Arc<TerminalSession>) {
    std::thread::spawn(move || {
        let mut buffer = [0u8; 8192];

        loop {
            let bytes_read = {
                let mut reader = match session.reader.lock() {
                    Ok(reader) => reader,
                    Err(_) => break,
                };

                match reader.read(&mut buffer) {
                    Ok(bytes_read) => bytes_read,
                    Err(error) if error.kind() == std::io::ErrorKind::Interrupted => continue,
                    Err(_) => break,
                }
            };

            if bytes_read == 0 {
                break;
            }

            let payload = TerminalDataPayload {
                session_id,
                data: String::from_utf8_lossy(&buffer[..bytes_read]).into_owned(),
            };

            let _ = app_handle.emit(TERMINAL_DATA_EVENT, payload);
        }
    });
}

fn spawn_terminal_waiter(app_handle: AppHandle, session_id: PtyHandler, session: Arc<TerminalSession>) {
    std::thread::spawn(move || {
        let exit_code = {
            let mut child = match session.child.lock() {
                Ok(child) => child,
                Err(_) => return,
            };

            child.wait().ok().map(|status| status.exit_code())
        };

        if let Some(terminal_state) = app_handle.try_state::<TerminalState>() {
            terminal_state.remove_session(session_id);
        }

        let _ = app_handle.emit(
            TERMINAL_EXIT_EVENT,
            TerminalExitPayload {
                session_id,
                exit_code,
            },
        );
    });
}
#[derive(Serialize)]
struct SysInfo {
    cpu: f32,
    memory: u64,      // total bytes
    memory_used: u64, // used bytes
}

#[tauri::command]
fn get_sys_info() -> Result<SysInfo, String> {
    let mut sys = System::new_all();
    sys.refresh_all();
    
    // Average CPU usage across all cores
    let cpu_sum: f32 = sys.cpus().iter().map(|cpu| cpu.cpu_usage()).sum();
    let cpu_avg = if sys.cpus().len() > 0 { cpu_sum / sys.cpus().len() as f32 } else { 0.0 };
    
    Ok(SysInfo {
        cpu: cpu_avg,
        memory: sys.total_memory(),
        memory_used: sys.used_memory(),
    })
}

#[tauri::command]
fn get_git_branch(state: tauri::State<'_, WorkspaceIndex>) -> Result<String, String> {
    use std::process::Command;
    let root = active_workspace_root(&state)?;
    
    let output = Command::new("git")
        .arg("branch")
        .arg("--show-current")
        .current_dir(&root)
        .output()
        .map_err(|e| format!("Failed to execute git: {}", e))?;

    if !output.status.success() {
        return Ok(String::new());
    }

    Ok(String::from_utf8_lossy(&output.stdout).trim().to_string())
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(
            tauri_plugin_sql::Builder::default()
                .add_migrations(
                    "sqlite:zenith.db",
                    vec![
                        tauri_plugin_sql::Migration {
                            version: 1,
                            description: "create initial tables",
                            sql: "CREATE TABLE IF NOT EXISTS settings (key TEXT PRIMARY KEY, value TEXT NOT NULL);
                                  CREATE TABLE IF NOT EXISTS extension_data (ext_id TEXT, key TEXT, value TEXT, PRIMARY KEY(ext_id, key));",
                            kind: tauri_plugin_sql::MigrationKind::Up,
                        },
                        tauri_plugin_sql::Migration {
                            version: 2,
                            description: "add workspaces and file history",
                            sql: "CREATE TABLE IF NOT EXISTS workspaces (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE NOT NULL, snapshot TEXT NOT NULL, updated_at DATETIME DEFAULT CURRENT_TIMESTAMP);
                                  CREATE TABLE IF NOT EXISTS file_history (id INTEGER PRIMARY KEY AUTOINCREMENT, path TEXT NOT NULL, content_blob BLOB NOT NULL, hash TEXT NOT NULL, created_at DATETIME DEFAULT CURRENT_TIMESTAMP);
                                  CREATE INDEX IF NOT EXISTS idx_file_history_path ON file_history(path);",
                            kind: tauri_plugin_sql::MigrationKind::Up,
                        }
                    ],
                )
                .build(),
        )
        .manage(TerminalState::default())
        .manage(WorkspaceIndex::default())
        .manage(LspState::default())
        .invoke_handler(tauri::generate_handler![
            greet,
            get_file_tree,
            read_file,
            write_file,
            create_file,
            create_folder,
            rename_path,
            delete_path,
            copy_path,
            move_path,
            reveal_in_system_explorer,
            open_in_system_explorer,
            run_compiler,
            pick_file,
            pick_folder,
            pick_save_path,
            run_diagnostics,
            get_git_status,
            search_in_files,
            replace_in_files,
            search_file_names,
            get_file_symbols,
            workspace_index_bootstrap,
            terminal_create,
            terminal_write,
            terminal_resize,
            terminal_kill,
            git_stage,
            git_unstage,
            git_commit,
            git_read_original,
            history_save_snapshot,
            history_get_snapshots,
            get_git_branch,
            get_sys_info,
            git_stage_all,
            git_unstage_all,
            git_discard_changes,
            lsp_sync_document,
            lsp_completion,
            lsp_hover,
            lsp_definition,
            lsp_references,
            lsp_rename,
            lsp_latest_diagnostics,
            lsp_shutdown
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
