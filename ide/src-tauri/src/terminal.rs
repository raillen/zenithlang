use portable_pty::{native_pty_system, Child, ChildKiller, CommandBuilder, MasterPty, PtySize};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::io::{Read, Write};
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicU32, Ordering};
use std::sync::{Arc, Mutex};
use tauri::{AppHandle, Emitter, Manager};

pub type PtyHandler = u32;

pub const TERMINAL_DATA_EVENT: &str = "zenith://terminal-data";
pub const TERMINAL_EXIT_EVENT: &str = "zenith://terminal-exit";

pub struct TerminalSession {
    pub master: Mutex<Box<dyn MasterPty + Send>>,
    pub child: Mutex<Box<dyn Child + Send + Sync>>,
    pub child_killer: Mutex<Box<dyn ChildKiller + Send + Sync>>,
    pub writer: Mutex<Box<dyn Write + Send>>,
    pub reader: Mutex<Box<dyn Read + Send>>,
}

#[derive(Default)]
pub struct TerminalState {
    next_session_id: AtomicU32,
    pub sessions: Mutex<HashMap<PtyHandler, Arc<TerminalSession>>>,
}

impl TerminalState {
    pub fn allocate_session_id(&self) -> PtyHandler {
        self.next_session_id.fetch_add(1, Ordering::Relaxed) + 1
    }

    pub fn get_session(&self, session_id: PtyHandler) -> Result<Arc<TerminalSession>, String> {
        self.sessions
            .lock()
            .map_err(|_| "Unable to access terminal sessions".to_string())?
            .get(&session_id)
            .cloned()
            .ok_or_else(|| format!("Terminal session {} is not available", session_id))
    }

    pub fn remove_session(&self, session_id: PtyHandler) {
        if let Ok(mut sessions) = self.sessions.lock() {
            sessions.remove(&session_id);
        }
    }
}

#[derive(Serialize, Deserialize, Debug, Clone)]
#[serde(rename_all = "camelCase")]
pub struct TerminalSessionInfo {
    pub session_id: PtyHandler,
    pub cwd: String,
    pub shell: String,
}

#[derive(Serialize, Clone)]
#[serde(rename_all = "camelCase")]
pub struct TerminalDataPayload {
    pub session_id: PtyHandler,
    pub data: String,
}

#[derive(Serialize, Clone)]
#[serde(rename_all = "camelCase")]
pub struct TerminalExitPayload {
    pub session_id: PtyHandler,
    pub exit_code: Option<u32>,
}

#[tauri::command]
pub fn terminal_create(
    app_handle: AppHandle,
    state: tauri::State<'_, TerminalState>,
    cwd: Option<String>,
    shell: Option<String>,
    cols: Option<u16>,
    rows: Option<u16>,
) -> Result<TerminalSessionInfo, String> {
    // Note: resolve_path and workspace_root must be shared or reached via a common mod
    // For now we'll assume they will be in a common utility or accessible.
    // I will use a placeholder or import them if I move them to a 'utils' mod.
    let resolved_cwd = cwd
        .as_deref()
        .map(crate::fs_ops::resolve_path)
        .transpose()?
        .unwrap_or_else(crate::fs_ops::workspace_root);
        
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
pub fn terminal_write(
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
pub fn terminal_resize(
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
pub fn terminal_kill(
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
