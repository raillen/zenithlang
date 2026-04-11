use serde::{Deserialize, Serialize};
use serde_json::{json, Value};
use std::collections::HashMap;
use std::io::{BufRead, BufReader, Read, Write};
use std::path::{Path, PathBuf};
use std::process::{Child, ChildStdin, ChildStdout, Command, Stdio};
use std::sync::atomic::{AtomicU32, Ordering};
use std::sync::{Arc, Condvar, Mutex, RwLock};
use std::time::Duration;
use tauri::{AppHandle, Emitter};

const LSP_DIAGNOSTICS_EVENT: &str = "zenith://lsp-diagnostics";

#[derive(Default)]
pub struct LspState {
    sessions: Mutex<HashMap<String, Arc<LspSession>>>,
}

struct LspSession {
    child: Mutex<Child>,
    stdin: Mutex<ChildStdin>,
    pending: Mutex<HashMap<u32, Arc<PendingResponse>>>,
    next_id: AtomicU32,
    document_versions: Mutex<HashMap<String, i32>>,
    latest_diagnostics: RwLock<HashMap<String, Vec<LspDiagnostic>>>,
}

struct PendingResponse {
    value: Mutex<Option<Result<Value, String>>>,
    notify: Condvar,
}

#[derive(Serialize, Clone)]
#[serde(rename_all = "camelCase")]
pub struct LspDiagnostic {
    pub line: u32,
    pub col: u32,
    pub end_line: u32,
    pub end_col: u32,
    pub message: String,
    pub severity: String,
    pub code: String,
}

#[derive(Serialize, Clone)]
#[serde(rename_all = "camelCase")]
pub struct LspCompletionItem {
    pub label: String,
    pub detail: Option<String>,
    pub documentation: Option<String>,
    pub insert_text: Option<String>,
    pub kind: Option<u32>,
}

#[derive(Serialize, Clone)]
#[serde(rename_all = "camelCase")]
pub struct LspHoverResult {
    pub contents: String,
}

#[derive(Serialize, Clone)]
#[serde(rename_all = "camelCase")]
pub struct LspLocation {
    pub file_path: String,
    pub line: u32,
    pub col: u32,
}

#[derive(Serialize, Clone)]
#[serde(rename_all = "camelCase")]
pub struct LspTextEdit {
    pub start_line: u32,
    pub start_col: u32,
    pub end_line: u32,
    pub end_col: u32,
    pub new_text: String,
}

#[derive(Serialize, Clone)]
#[serde(rename_all = "camelCase")]
pub struct LspWorkspaceEditFile {
    pub file_path: String,
    pub edits: Vec<LspTextEdit>,
}

#[derive(Serialize, Clone)]
#[serde(rename_all = "camelCase")]
pub struct LspWorkspaceEditResult {
    pub files: Vec<LspWorkspaceEditFile>,
}

#[derive(Serialize, Clone)]
#[serde(rename_all = "camelCase")]
struct LspDiagnosticsPayload {
    file_path: String,
    diagnostics: Vec<LspDiagnostic>,
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct LspDocumentRequest {
    runtime_id: String,
    root_path: String,
    lsp_command: String,
    file_path: String,
    language_id: String,
    text: String,
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct LspPositionRequest {
    runtime_id: String,
    root_path: String,
    lsp_command: String,
    file_path: String,
    language_id: String,
    text: String,
    line: u32,
    character: u32,
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct LspRenameRequest {
    runtime_id: String,
    root_path: String,
    lsp_command: String,
    file_path: String,
    language_id: String,
    text: String,
    line: u32,
    character: u32,
    new_name: String,
}

fn session_key(runtime_id: &str, root_path: &str, lsp_command: &str) -> String {
    format!("{}::{}::{}", runtime_id, root_path, lsp_command)
}

fn normalize_path(path: &str) -> PathBuf {
    let candidate = PathBuf::from(path);
    if candidate.is_absolute() {
        candidate
    } else {
        std::env::current_dir()
            .unwrap_or_else(|_| PathBuf::from("."))
            .join(candidate)
    }
}

fn path_to_uri(path: &Path) -> String {
    let normalized = path.to_string_lossy().replace('\\', "/");
    if normalized.starts_with('/') {
        format!("file://{}", normalized)
    } else {
        format!("file:///{}", normalized)
    }
}

fn uri_to_path(uri: &str) -> String {
    let trimmed = uri.strip_prefix("file://").unwrap_or(uri);
    #[cfg(windows)]
    {
        if trimmed.starts_with('/') && trimmed.as_bytes().get(2) == Some(&b':') {
            return trimmed[1..].replace('/', "\\");
        }
    }
    trimmed.replace('/', std::path::MAIN_SEPARATOR_STR)
}

fn parse_command_line(command_line: &str) -> Result<(String, Vec<String>), String> {
    let mut parts = Vec::new();
    let mut current = String::new();
    let mut quote: Option<char> = None;

    for ch in command_line.chars() {
        if let Some(active_quote) = quote {
            if ch == active_quote {
                quote = None;
            } else {
                current.push(ch);
            }
            continue;
        }

        if ch == '"' || ch == '\'' {
            quote = Some(ch);
            continue;
        }

        if ch.is_whitespace() {
            if !current.is_empty() {
                parts.push(std::mem::take(&mut current));
            }
            continue;
        }

        current.push(ch);
    }

    if !current.is_empty() {
        parts.push(current);
    }

    if parts.is_empty() {
        return Err("LSP command is empty".to_string());
    }

    let executable = parts.remove(0);
    Ok((executable, parts))
}

fn default_args(runtime_id: &str, executable: &str, args: &[String]) -> Vec<String> {
    if !args.is_empty() {
        return Vec::new();
    }

    let executable = executable.to_ascii_lowercase();
    match runtime_id {
        "node" if executable.contains("typescript-language-server") => vec!["--stdio".to_string()],
        "lua" if executable.contains("lua-language-server") => vec!["--stdio".to_string()],
        "ruby" if executable.contains("solargraph") => vec!["stdio".to_string()],
        _ => Vec::new(),
    }
}

fn write_lsp_message(stdin: &mut ChildStdin, payload: &Value) -> Result<(), String> {
    let bytes = serde_json::to_vec(payload).map_err(|e| e.to_string())?;
    write!(stdin, "Content-Length: {}\r\n\r\n", bytes.len()).map_err(|e| e.to_string())?;
    stdin.write_all(&bytes).map_err(|e| e.to_string())?;
    stdin.flush().map_err(|e| e.to_string())
}

fn read_lsp_message(reader: &mut BufReader<ChildStdout>) -> Result<Value, String> {
    let mut content_length = None;
    loop {
        let mut header = String::new();
        let bytes = reader.read_line(&mut header).map_err(|e| e.to_string())?;
        if bytes == 0 {
            return Err("LSP stream closed".to_string());
        }

        let trimmed = header.trim();
        if trimmed.is_empty() {
            break;
        }

        if let Some(value) = trimmed.strip_prefix("Content-Length:") {
            content_length = value.trim().parse::<usize>().ok();
        }
    }

    let length = content_length.ok_or_else(|| "Missing LSP content length".to_string())?;
    let mut body = vec![0u8; length];
    reader.read_exact(&mut body).map_err(|e| e.to_string())?;
    serde_json::from_slice(&body).map_err(|e| e.to_string())
}

fn send_notification(session: &LspSession, method: &str, params: Value) -> Result<(), String> {
    let mut stdin = session.stdin.lock().map_err(|_| "Failed to lock LSP stdin".to_string())?;
    write_lsp_message(
        &mut stdin,
        &json!({
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
        }),
    )
}

fn send_request(session: &Arc<LspSession>, method: &str, params: Value) -> Result<Value, String> {
    let id = session.next_id.fetch_add(1, Ordering::Relaxed) + 1;
    let pending = Arc::new(PendingResponse {
        value: Mutex::new(None),
        notify: Condvar::new(),
    });

    session
        .pending
        .lock()
        .map_err(|_| "Failed to lock pending LSP responses".to_string())?
        .insert(id, pending.clone());

    {
        let mut stdin = session.stdin.lock().map_err(|_| "Failed to lock LSP stdin".to_string())?;
        write_lsp_message(
            &mut stdin,
            &json!({
                "jsonrpc": "2.0",
                "id": id,
                "method": method,
                "params": params,
            }),
        )?;
    }

    let guard = pending
        .value
        .lock()
        .map_err(|_| "Failed to wait for LSP response".to_string())?;
    let (guard, _) = pending
        .notify
        .wait_timeout_while(guard, Duration::from_secs(8), |value| value.is_none())
        .map_err(|_| "Failed while waiting for LSP response".to_string())?;

    session
        .pending
        .lock()
        .map_err(|_| "Failed to lock pending LSP responses".to_string())?
        .remove(&id);

    guard
        .clone()
        .ok_or_else(|| format!("Timed out waiting for LSP method {method}"))?
}

fn spawn_reader(app_handle: AppHandle, stdout: ChildStdout, session: Arc<LspSession>) {
    std::thread::spawn(move || {
        let mut reader = BufReader::new(stdout);

        while let Ok(message) = read_lsp_message(&mut reader) {
            if let Some(id) = message.get("id").and_then(|value| value.as_u64()) {
                if let Ok(mut pending) = session.pending.lock() {
                    if let Some(waiter) = pending.remove(&(id as u32)) {
                        let result = if let Some(error) = message.get("error") {
                            Err(error.to_string())
                        } else {
                            Ok(message.get("result").cloned().unwrap_or(Value::Null))
                        };

                        if let Ok(mut value) = waiter.value.lock() {
                            *value = Some(result);
                            waiter.notify.notify_all();
                        }
                    }
                }

                continue;
            }

            if message.get("method").and_then(|value| value.as_str()) == Some("textDocument/publishDiagnostics") {
                let diagnostics = parse_publish_diagnostics(&message);
                if let Ok(mut store) = session.latest_diagnostics.write() {
                    store.insert(diagnostics.file_path.clone(), diagnostics.diagnostics.clone());
                }
                let _ = app_handle.emit(LSP_DIAGNOSTICS_EVENT, diagnostics);
            }
        }
    });
}

fn ensure_session(
    app_handle: &AppHandle,
    state: &LspState,
    runtime_id: &str,
    root_path: &str,
    lsp_command: &str,
) -> Result<Arc<LspSession>, String> {
    let key = session_key(runtime_id, root_path, lsp_command);

    if let Some(existing) = state
        .sessions
        .lock()
        .map_err(|_| "Failed to lock LSP sessions".to_string())?
        .get(&key)
        .cloned()
    {
        return Ok(existing);
    }

    let (executable, args) = parse_command_line(lsp_command)?;
    let mut command = Command::new(&executable);
    let root = normalize_path(root_path);
    let mut full_args = args.clone();
    full_args.extend(default_args(runtime_id, &executable, &args));

    command
        .args(&full_args)
        .current_dir(&root)
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .stderr(Stdio::null());

    let mut child = command
        .spawn()
        .map_err(|e| format!("Failed to spawn LSP process {}: {}", executable, e))?;

    let stdin = child.stdin.take().ok_or_else(|| "Failed to open LSP stdin".to_string())?;
    let stdout = child.stdout.take().ok_or_else(|| "Failed to open LSP stdout".to_string())?;

    let session = Arc::new(LspSession {
        child: Mutex::new(child),
        stdin: Mutex::new(stdin),
        pending: Mutex::new(HashMap::new()),
        next_id: AtomicU32::new(0),
        document_versions: Mutex::new(HashMap::new()),
        latest_diagnostics: RwLock::new(HashMap::new()),
    });

    spawn_reader(app_handle.clone(), stdout, session.clone());

    state
        .sessions
        .lock()
        .map_err(|_| "Failed to lock LSP sessions".to_string())?
        .insert(key, session.clone());

    let root_uri = path_to_uri(&root);
    let _ = send_request(
        &session,
        "initialize",
        json!({
            "processId": std::process::id(),
            "clientInfo": { "name": "Zenith Keter", "version": "0.2" },
            "rootUri": root_uri,
            "rootPath": root.to_string_lossy().to_string(),
            "capabilities": {
                "textDocument": {
                    "completion": { "completionItem": { "snippetSupport": true } },
                    "hover": {},
                    "definition": {},
                    "references": {},
                    "rename": {}
                }
            }
        }),
    )?;
    send_notification(&session, "initialized", json!({}))?;
    Ok(session)
}

fn sync_document(session: &Arc<LspSession>, request: &LspDocumentRequest) -> Result<(), String> {
    let path = normalize_path(&request.file_path);
    let uri = path_to_uri(&path);
    let version = {
        let mut versions = session
            .document_versions
            .lock()
            .map_err(|_| "Failed to lock LSP versions".to_string())?;
        let next = versions.get(&request.file_path).copied().unwrap_or(0) + 1;
        versions.insert(request.file_path.clone(), next);
        next
    };

    if version == 1 {
        send_notification(
            session,
            "textDocument/didOpen",
            json!({
                "textDocument": {
                    "uri": uri,
                    "languageId": request.language_id,
                    "version": version,
                    "text": request.text,
                }
            }),
        )
    } else {
        send_notification(
            session,
            "textDocument/didChange",
            json!({
                "textDocument": {
                    "uri": uri,
                    "version": version,
                },
                "contentChanges": [{ "text": request.text }],
            }),
        )
    }
}

fn parse_position_result(result: &Value) -> Vec<LspLocation> {
    let values = if result.is_array() {
        result.as_array().cloned().unwrap_or_default()
    } else if result.is_object() {
        vec![result.clone()]
    } else {
        Vec::new()
    };

    values
        .into_iter()
        .filter_map(|item| {
            let uri = item.get("uri")?.as_str()?;
            let start = item.get("range")?.get("start")?;
            Some(LspLocation {
                file_path: uri_to_path(uri),
                line: start.get("line")?.as_u64()? as u32 + 1,
                col: start.get("character")?.as_u64()? as u32 + 1,
            })
        })
        .collect()
}

fn markdown_from_value(value: &Value) -> Option<String> {
    if let Some(text) = value.as_str() {
        return Some(text.to_string());
    }

    if let Some(contents) = value.get("value").and_then(|item| item.as_str()) {
        return Some(contents.to_string());
    }

    if let Some(items) = value.as_array() {
        let merged = items
            .iter()
            .filter_map(markdown_from_value)
            .collect::<Vec<_>>()
            .join("\n\n");
        if !merged.is_empty() {
            return Some(merged);
        }
    }

    None
}

fn parse_publish_diagnostics(message: &Value) -> LspDiagnosticsPayload {
    let params = message.get("params").cloned().unwrap_or(Value::Null);
    let file_path = params
        .get("uri")
        .and_then(|value| value.as_str())
        .map(uri_to_path)
        .unwrap_or_default();

    let diagnostics = params
        .get("diagnostics")
        .and_then(|value| value.as_array())
        .map(|items| {
            items
                .iter()
                .filter_map(|item| {
                    let start = item.get("range")?.get("start")?;
                    let end = item.get("range")?.get("end")?;
                    Some(LspDiagnostic {
                        line: start.get("line")?.as_u64()? as u32 + 1,
                        col: start.get("character")?.as_u64()? as u32 + 1,
                        end_line: end.get("line")?.as_u64()? as u32 + 1,
                        end_col: end.get("character")?.as_u64()? as u32 + 1,
                        message: item.get("message")?.as_str()?.to_string(),
                        severity: match item.get("severity").and_then(|value| value.as_u64()) {
                            Some(1) => "error",
                            Some(2) => "warning",
                            _ => "hint",
                        }
                        .to_string(),
                        code: item
                            .get("code")
                            .and_then(|value| value.as_str().map(|s| s.to_string()).or_else(|| value.as_i64().map(|n| n.to_string())))
                            .unwrap_or_else(|| "LSP".to_string()),
                    })
                })
                .collect::<Vec<_>>()
        })
        .unwrap_or_default();

    LspDiagnosticsPayload { file_path, diagnostics }
}

#[tauri::command]
pub fn lsp_sync_document(
    app_handle: AppHandle,
    state: tauri::State<'_, LspState>,
    request: LspDocumentRequest,
) -> Result<(), String> {
    let session = ensure_session(&app_handle, &state, &request.runtime_id, &request.root_path, &request.lsp_command)?;
    sync_document(&session, &request)
}

#[tauri::command]
pub fn lsp_completion(
    app_handle: AppHandle,
    state: tauri::State<'_, LspState>,
    request: LspPositionRequest,
) -> Result<Vec<LspCompletionItem>, String> {
    let session = ensure_session(&app_handle, &state, &request.runtime_id, &request.root_path, &request.lsp_command)?;
    sync_document(&session, &LspDocumentRequest {
        runtime_id: request.runtime_id.clone(),
        root_path: request.root_path.clone(),
        lsp_command: request.lsp_command.clone(),
        file_path: request.file_path.clone(),
        language_id: request.language_id.clone(),
        text: request.text.clone(),
    })?;

    let uri = path_to_uri(&normalize_path(&request.file_path));
    let result = send_request(&session, "textDocument/completion", json!({
        "textDocument": { "uri": uri },
        "position": { "line": request.line, "character": request.character }
    }))?;

    let items = result.get("items").and_then(|value| value.as_array()).cloned().unwrap_or_else(|| result.as_array().cloned().unwrap_or_default());
    Ok(items.into_iter().filter_map(|item| {
        Some(LspCompletionItem {
            label: item.get("label")?.as_str()?.to_string(),
            detail: item.get("detail").and_then(|value| value.as_str()).map(|s| s.to_string()),
            documentation: item.get("documentation").and_then(markdown_from_value),
            insert_text: item.get("insertText").and_then(|value| value.as_str()).map(|s| s.to_string()),
            kind: item.get("kind").and_then(|value| value.as_u64()).map(|value| value as u32),
        })
    }).collect())
}

#[tauri::command]
pub fn lsp_hover(
    app_handle: AppHandle,
    state: tauri::State<'_, LspState>,
    request: LspPositionRequest,
) -> Result<Option<LspHoverResult>, String> {
    let session = ensure_session(&app_handle, &state, &request.runtime_id, &request.root_path, &request.lsp_command)?;
    sync_document(&session, &LspDocumentRequest {
        runtime_id: request.runtime_id.clone(),
        root_path: request.root_path.clone(),
        lsp_command: request.lsp_command.clone(),
        file_path: request.file_path.clone(),
        language_id: request.language_id.clone(),
        text: request.text.clone(),
    })?;

    let uri = path_to_uri(&normalize_path(&request.file_path));
    let result = send_request(&session, "textDocument/hover", json!({
        "textDocument": { "uri": uri },
        "position": { "line": request.line, "character": request.character }
    }))?;

    Ok(markdown_from_value(&result.get("contents").cloned().unwrap_or(Value::Null))
        .map(|contents| LspHoverResult { contents }))
}

#[tauri::command]
pub fn lsp_definition(
    app_handle: AppHandle,
    state: tauri::State<'_, LspState>,
    request: LspPositionRequest,
) -> Result<Vec<LspLocation>, String> {
    let session = ensure_session(&app_handle, &state, &request.runtime_id, &request.root_path, &request.lsp_command)?;
    sync_document(&session, &LspDocumentRequest {
        runtime_id: request.runtime_id.clone(),
        root_path: request.root_path.clone(),
        lsp_command: request.lsp_command.clone(),
        file_path: request.file_path.clone(),
        language_id: request.language_id.clone(),
        text: request.text.clone(),
    })?;

    let uri = path_to_uri(&normalize_path(&request.file_path));
    let result = send_request(&session, "textDocument/definition", json!({
        "textDocument": { "uri": uri },
        "position": { "line": request.line, "character": request.character }
    }))?;

    Ok(parse_position_result(&result))
}

#[tauri::command]
pub fn lsp_references(
    app_handle: AppHandle,
    state: tauri::State<'_, LspState>,
    request: LspPositionRequest,
) -> Result<Vec<LspLocation>, String> {
    let session = ensure_session(&app_handle, &state, &request.runtime_id, &request.root_path, &request.lsp_command)?;
    sync_document(&session, &LspDocumentRequest {
        runtime_id: request.runtime_id.clone(),
        root_path: request.root_path.clone(),
        lsp_command: request.lsp_command.clone(),
        file_path: request.file_path.clone(),
        language_id: request.language_id.clone(),
        text: request.text.clone(),
    })?;

    let uri = path_to_uri(&normalize_path(&request.file_path));
    let result = send_request(&session, "textDocument/references", json!({
        "textDocument": { "uri": uri },
        "position": { "line": request.line, "character": request.character },
        "context": { "includeDeclaration": true }
    }))?;

    Ok(parse_position_result(&result))
}

#[tauri::command]
pub fn lsp_rename(
    app_handle: AppHandle,
    state: tauri::State<'_, LspState>,
    request: LspRenameRequest,
) -> Result<LspWorkspaceEditResult, String> {
    let session = ensure_session(&app_handle, &state, &request.runtime_id, &request.root_path, &request.lsp_command)?;
    sync_document(&session, &LspDocumentRequest {
        runtime_id: request.runtime_id.clone(),
        root_path: request.root_path.clone(),
        lsp_command: request.lsp_command.clone(),
        file_path: request.file_path.clone(),
        language_id: request.language_id.clone(),
        text: request.text.clone(),
    })?;

    let uri = path_to_uri(&normalize_path(&request.file_path));
    let result = send_request(&session, "textDocument/rename", json!({
        "textDocument": { "uri": uri },
        "position": { "line": request.line, "character": request.character },
        "newName": request.new_name
    }))?;

    let changes = result
        .get("changes")
        .and_then(|value| value.as_object())
        .cloned()
        .unwrap_or_default();

    let files = changes
        .into_iter()
        .map(|(uri, edits)| LspWorkspaceEditFile {
            file_path: uri_to_path(&uri),
            edits: edits
                .as_array()
                .cloned()
                .unwrap_or_default()
                .into_iter()
                .filter_map(|edit| {
                    let range = edit.get("range")?;
                    let start = range.get("start")?;
                    let end = range.get("end")?;
                    Some(LspTextEdit {
                        start_line: start.get("line")?.as_u64()? as u32 + 1,
                        start_col: start.get("character")?.as_u64()? as u32 + 1,
                        end_line: end.get("line")?.as_u64()? as u32 + 1,
                        end_col: end.get("character")?.as_u64()? as u32 + 1,
                        new_text: edit.get("newText")?.as_str()?.to_string(),
                    })
                })
                .collect(),
        })
        .collect();

    Ok(LspWorkspaceEditResult { files })
}

#[tauri::command]
pub fn lsp_latest_diagnostics(
    state: tauri::State<'_, LspState>,
    runtime_id: String,
    root_path: String,
    lsp_command: String,
    file_path: String,
) -> Result<Vec<LspDiagnostic>, String> {
    let key = session_key(&runtime_id, &root_path, &lsp_command);
    let sessions = state.sessions.lock().map_err(|_| "Failed to lock LSP sessions".to_string())?;
    let session = sessions.get(&key).ok_or_else(|| "No LSP session available".to_string())?;
    let diagnostics = session
        .latest_diagnostics
        .read()
        .map_err(|_| "Failed to lock LSP diagnostics".to_string())?
        .get(&file_path)
        .cloned()
        .unwrap_or_default();
    Ok(diagnostics)
}

#[tauri::command]
pub fn lsp_shutdown(
    state: tauri::State<'_, LspState>,
    runtime_id: String,
    root_path: String,
    lsp_command: String,
) -> Result<(), String> {
    let key = session_key(&runtime_id, &root_path, &lsp_command);
    let session = state
        .sessions
        .lock()
        .map_err(|_| "Failed to lock LSP sessions".to_string())?
        .remove(&key);

    if let Some(session) = session {
        let _ = send_request(&session, "shutdown", json!(null));
        let _ = send_notification(&session, "exit", json!(null));
        if let Ok(mut child) = session.child.lock() {
            let _ = child.kill();
        }
    }

    Ok(())
}
