use portable_pty::{native_pty_system, Child, ChildKiller, CommandBuilder, MasterPty, PtySize};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::fs;
use std::io::{Read, Write};
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicU32, Ordering};
use std::sync::{Arc, Mutex};
use tauri::{AppHandle, Emitter, Manager};

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct FileEntry {
    name: String,
    path: String,
    is_directory: bool,
    children: Option<Vec<FileEntry>>,
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

            if name == ".git"
                || name == "node_modules"
                || name == "target"
                || name == "dist"
                || name == ".tauri"
            {
                continue;
            }

            let is_directory = path.is_dir();
            let children = if is_directory {
                Some(scan_dir(&path)?)
            } else {
                None
            };

            entries.push(FileEntry {
                name,
                path: path.to_string_lossy().to_string(),
                is_directory,
                children,
            });
        }
    }

    entries.sort_by(|a, b| {
        if a.is_directory != b.is_directory {
            b.is_directory.cmp(&a.is_directory)
        } else {
            a.name.to_lowercase().cmp(&b.name.to_lowercase())
        }
    });

    Ok(entries)
}

#[tauri::command]
fn read_file(path: String) -> Result<String, String> {
    let resolved_path = resolve_path(&path)?;
    let bytes = fs::read(&resolved_path).map_err(|e| format!("{} ({})", e, resolved_path.display()))?;
    Ok(String::from_utf8_lossy(&bytes).into_owned())
}

#[tauri::command]
fn write_file(path: String, content: String) -> Result<(), String> {
    let resolved_path = resolve_path(&path)?;
    fs::write(&resolved_path, content).map_err(|e| format!("{} ({})", e, resolved_path.display()))
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
fn get_git_status() -> Result<HashMap<String, String>, String> {
    use std::process::Command;
    
    let output = Command::new("git")
        .arg("status")
        .arg("--porcelain")
        .current_dir(workspace_root())
        .output()
        .map_err(|e| format!("Failed to execute git: {}", e))?;

    let stdout = String::from_utf8_lossy(&output.stdout);
    let mut status_map = HashMap::new();

    for line in stdout.lines() {
        if line.len() > 3 {
            let status = line[..2].trim().to_string();
            let path = line[3..].trim().to_string();
            // Resolve relative path to absolute to match file tree
            let abs_path = workspace_root().join(path).to_string_lossy().to_string();
            status_map.insert(abs_path, status);
        }
    }

    Ok(status_map)
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
    root_path: Option<String>,
    query: String, 
    is_regex: bool, 
    match_case: bool
) -> Result<Vec<FileResult>, String> {
    let mut all_results = Vec::new();
    let root = match root_path {
        Some(p) if !p.is_empty() => resolve_path(&p)?,
        _ => workspace_root(),
    };

    let query_lower = if !match_case { query.to_lowercase() } else { query.clone() };

    let mut dirs_to_visit = vec![root.clone()];
    
    let re = if is_regex {
        // We reuse the regex dependency
        match regex::RegexBuilder::new(&query).case_insensitive(!match_case).build() {
            Ok(r) => Some(r),
            Err(e) => return Err(format!("Invalid regex: {}", e)),
        }
    } else {
        None
    };

    while let Some(current_dir) = dirs_to_visit.pop() {
        if let Ok(entries) = fs::read_dir(&current_dir) {
            for entry in entries.flatten() {
                let path = entry.path();
                let name = entry.file_name().to_string_lossy().to_string();

                if name == ".git" || name == "node_modules" || name == "target" || name == "dist" || name == ".tauri" || name.ends_with(".db") || name.ends_with(".png") || name.ends_with(".jpg") || name.ends_with(".woff2") {
                    continue; 
                }

                if path.is_dir() {
                    dirs_to_visit.push(path);
                } else {
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
                                    line_content: line.trim().to_string(), // Trimming whitespace limits frontend rendering mess
                                });
                            }
                        }

                        if !file_matches.is_empty() {
                            let clean_path = path.strip_prefix(&root).unwrap_or(&path).to_string_lossy().to_string();
                            all_results.push(FileResult {
                                file_path: clean_path.replace("\\", "/"),
                                matches: file_matches,
                            });
                        }
                    }
                }
            }
        }
    }

    all_results.sort_by(|a, b| a.file_path.cmp(&b.file_path));
    
    Ok(all_results)
}

#[derive(Serialize)]
struct FileNameResult {
    name: String,
    path: String,
}

#[tauri::command]
fn search_file_names(root_path: Option<String>, query: String) -> Result<Vec<FileNameResult>, String> {
    let mut all_results = Vec::new();
    let root = match root_path {
        Some(p) if !p.is_empty() => resolve_path(&p)?,
        _ => workspace_root(),
    };

    let query_lower = query.to_lowercase();
    let mut dirs_to_visit = vec![root.clone()];
    
    while let Some(current_dir) = dirs_to_visit.pop() {
        if let Ok(entries) = fs::read_dir(&current_dir) {
            for entry in entries.flatten() {
                let path = entry.path();
                let name = entry.file_name().to_string_lossy().to_string();

                if name == ".git" || name == "node_modules" || name == "target" || name == "dist" || name == ".tauri" || name.ends_with(".db") || name.ends_with(".png") || name.ends_with(".jpg") || name.ends_with(".woff2") {
                    continue; 
                }

                if path.is_dir() {
                    dirs_to_visit.push(path);
                } else {
                    if query_lower.is_empty() || name.to_lowercase().contains(&query_lower) {
                         let clean_path = path.strip_prefix(&root).unwrap_or(&path).to_string_lossy().to_string();
                         all_results.push(FileNameResult {
                             name,
                             path: clean_path.replace("\\", "/"),
                         });
                    }
                }
            }
        }
    }

    all_results.sort_by(|a, b| a.name.cmp(&b.name));
    all_results.truncate(100);
    Ok(all_results)
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

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(
            tauri_plugin_sql::Builder::default()
                .add_migrations(
                    "sqlite:zenith.db",
                    vec![tauri_plugin_sql::Migration {
                        version: 1,
                        description: "create initial tables",
                        sql: "CREATE TABLE IF NOT EXISTS settings (key TEXT PRIMARY KEY, value TEXT NOT NULL);
                              CREATE TABLE IF NOT EXISTS extension_data (ext_id TEXT, key TEXT, value TEXT, PRIMARY KEY(ext_id, key));",
                        kind: tauri_plugin_sql::MigrationKind::Up,
                    }],
                )
                .build(),
        )
        .manage(TerminalState::default())
        .invoke_handler(tauri::generate_handler![
            greet,
            get_file_tree,
            read_file,
            write_file,
            run_compiler,
            pick_file,
            pick_folder,
            pick_save_path,
            run_diagnostics,
            get_git_status,
            search_in_files,
            search_file_names,
            get_file_symbols,
            terminal_create,
            terminal_write,
            terminal_resize,
            terminal_kill
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
