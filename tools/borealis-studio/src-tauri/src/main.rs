use serde::Serialize;
use serde_json::json;
use serde_json::Value;
use std::fs;
use std::io::{BufRead, BufReader, Write};
use std::path::{Path, PathBuf};
use std::process::{Child, ChildStdin, Command, Stdio};
use std::sync::mpsc::{self, Receiver, TryRecvError};
use std::sync::{Mutex, OnceLock};
use std::thread;
use std::time::{Duration, Instant};

static PREVIEW_SESSION: OnceLock<Mutex<Option<PreviewSession>>> = OnceLock::new();

struct PreviewSession {
    child: Child,
    stdin: ChildStdin,
    receiver: Receiver<String>,
    runner: String,
    seq: u64,
    status: String,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct StudioSnapshot {
    project_name: String,
    project_path: String,
    project_root: String,
    scene: SceneDocument,
    assets: Vec<ProjectAsset>,
    scripts: Vec<ScriptDocument>,
    console: Vec<ConsoleLine>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct StudioHome {
    workspace_root: String,
    app_root: String,
    repo_root: Option<String>,
    sdk_root: Option<String>,
    runtime_mode: String,
    runtime_status: String,
    default_project_path: String,
    default_projects_dir: String,
    templates: Vec<ProjectTemplate>,
    docs: Vec<DocumentationLink>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct ProjectTemplate {
    id: String,
    name: String,
    summary: String,
    default_name: String,
    tags: Vec<String>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct DocumentationLink {
    title: String,
    path: String,
    summary: String,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct SceneDocument {
    name: String,
    path: String,
    document_id: String,
    entities: Vec<SceneEntity>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct SceneEntity {
    id: String,
    name: String,
    layer: String,
    parent: Option<String>,
    tags: Vec<String>,
    components: Vec<SceneComponent>,
    transform: Transform3d,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct SceneComponent {
    kind: String,
    asset: Option<String>,
    script: Option<String>,
    profile: Option<String>,
    properties: Value,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct Transform3d {
    x: f64,
    y: f64,
    z: f64,
    rotation_x: f64,
    rotation_y: f64,
    rotation_z: f64,
    scale_x: f64,
    scale_y: f64,
    scale_z: f64,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct ProjectAsset {
    id: String,
    name: String,
    path: String,
    kind: String,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct ScriptDocument {
    path: String,
    name: String,
    content: String,
    dirty: Option<bool>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct ConsoleLine {
    id: String,
    level: String,
    source: String,
    message: String,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct PreviewCommandResult {
    status: String,
    runner: Option<String>,
    events: Vec<PreviewEvent>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct PreviewEvent {
    raw: String,
    channel: Option<String>,
    kind: Option<String>,
    status: Option<String>,
    message: Option<String>,
    loaded: Option<bool>,
    entity_count: Option<i64>,
}

#[derive(Debug, Clone)]
struct StudioLayout {
    app_root: PathBuf,
    base_root: PathBuf,
    repo_root: Option<PathBuf>,
    sdk_root: Option<PathBuf>,
}

#[tauri::command]
fn load_studio_snapshot(project_path: Option<String>) -> Result<StudioSnapshot, String> {
    let layout = studio_layout();
    let project_path = normalize_project_file(
        project_path
            .map(|path| resolve_studio_path(&path))
            .unwrap_or_else(|| default_project_path(&layout)),
    )?;
    let project_dir = project_path
        .parent()
        .ok_or_else(|| "project path has no parent".to_string())?
        .to_path_buf();
    let project_name = read_project_name(&project_path).unwrap_or_else(|| "Borealis".to_string());
    let scene_path = if project_dir.join("scenes/sample_3d.scene.json").exists() {
        project_dir.join("scenes/sample_3d.scene.json")
    } else if project_dir.join("scenes/sample.scene.json").exists() {
        project_dir.join("scenes/sample.scene.json")
    } else {
        project_dir.join("scenes/main.scene.json")
    };
    let scene = read_scene_document(&project_dir, &scene_path)?;
    let assets = collect_assets(&project_dir, &project_dir)?;
    let scripts = collect_scripts(&project_dir, &project_dir)?;

    Ok(StudioSnapshot {
        project_name,
        project_path: normalize_path(&project_path),
        project_root: normalize_path(&project_dir),
        scene,
        assets,
        scripts,
        console: vec![ConsoleLine {
            id: "studio-ready".to_string(),
            level: "info".to_string(),
            source: "studio".to_string(),
            message: format!("Loaded {} through Tauri.", normalize_path(&project_path)),
        }],
    })
}

#[tauri::command]
fn load_studio_home() -> StudioHome {
    let layout = studio_layout();
    let (runtime_mode, runtime_status) = runtime_status(&layout);
    StudioHome {
        workspace_root: normalize_path(&layout.base_root),
        app_root: normalize_path(&layout.app_root),
        repo_root: layout.repo_root.as_ref().map(|path| normalize_path(path)),
        sdk_root: layout.sdk_root.as_ref().map(|path| normalize_path(path)),
        runtime_mode,
        runtime_status,
        default_project_path: normalize_path(&default_project_path(&layout)),
        default_projects_dir: normalize_path(&default_projects_dir(&layout.base_root)),
        templates: project_templates(),
        docs: documentation_links(&layout),
    }
}

#[tauri::command]
fn read_text_file(path: String) -> Result<String, String> {
    let path = resolve_studio_path(&path);
    fs::read_to_string(&path)
        .map_err(|error| format!("failed to read {}: {error}", normalize_path(&path)))
}

#[tauri::command]
fn write_text_file(path: String, content: String) -> Result<(), String> {
    let path = resolve_studio_path(&path);
    write_text_at_path(&path, content)
}

#[tauri::command]
fn write_project_text_file(
    project_root: String,
    path: String,
    content: String,
) -> Result<(), String> {
    let project_root = resolve_studio_path(&project_root);
    let path = resolve_project_path(&project_root, &path);
    write_text_at_path(&path, content)
}

#[tauri::command]
fn create_borealis_project(
    project_name: String,
    parent_dir: String,
    template_id: String,
) -> Result<StudioSnapshot, String> {
    let layout = studio_layout();
    let clean_name = clean_project_name(&project_name);
    let slug = slugify_project_name(&clean_name);
    let parent = if parent_dir.trim().is_empty() {
        default_projects_dir(&layout.base_root)
    } else {
        resolve_studio_path(parent_dir.trim())
    };
    let project_dir = parent.join(&slug);

    if project_dir.exists() {
        return Err(format!(
            "project folder already exists: {}",
            normalize_path(&project_dir)
        ));
    }

    fs::create_dir_all(project_dir.join("scenes")).map_err(|error| {
        format!(
            "failed to create project scenes folder {}: {error}",
            normalize_path(&project_dir.join("scenes"))
        )
    })?;
    fs::create_dir_all(project_dir.join("src/app")).map_err(|error| {
        format!(
            "failed to create project source folder {}: {error}",
            normalize_path(&project_dir.join("src/app"))
        )
    })?;
    fs::create_dir_all(project_dir.join("assets")).map_err(|error| {
        format!(
            "failed to create project assets folder {}: {error}",
            normalize_path(&project_dir.join("assets"))
        )
    })?;

    let project_file = project_dir.join("zenith.ztproj");
    write_text_at_path(&project_file, project_manifest(&slug))?;
    write_text_at_path(
        &project_dir.join("scenes/main.scene.json"),
        template_scene_json(&clean_name, &template_id),
    )?;
    write_text_at_path(
        &project_dir.join("src/app/main.zt"),
        template_main_script(&slug, &template_id),
    )?;
    write_text_at_path(
        &project_dir.join("assets/triangle.obj"),
        template_triangle_obj(),
    )?;

    let mut snapshot = load_studio_snapshot(Some(normalize_path(&project_file)))?;
    snapshot.console.push(ConsoleLine {
        id: "project-created".to_string(),
        level: "info".to_string(),
        source: "studio".to_string(),
        message: format!("Created project {}.", normalize_path(&project_file)),
    });
    Ok(snapshot)
}

fn write_text_at_path(path: &Path, content: String) -> Result<(), String> {
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent)
            .map_err(|error| format!("failed to create {}: {error}", normalize_path(parent)))?;
    }
    fs::write(&path, content)
        .map_err(|error| format!("failed to write {}: {error}", normalize_path(&path)))
}

#[tauri::command]
fn start_preview(
    project_path: String,
    scene_path: String,
    scene_json: String,
) -> Result<PreviewCommandResult, String> {
    let layout = studio_layout();
    let preview_scene_path = std::env::temp_dir()
        .join("borealis-studio")
        .join("preview.scene.json");
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

    let preview_scene_for_ipc = normalize_path(
        preview_scene_path
            .strip_prefix(&layout.base_root)
            .unwrap_or(&preview_scene_path),
    );
    let resolved_project_path = resolve_studio_path(&project_path);
    let project_for_ipc = normalize_path(
        resolved_project_path
            .strip_prefix(&layout.base_root)
            .unwrap_or_else(|_| Path::new(&project_path)),
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
fn pause_preview() -> Result<PreviewCommandResult, String> {
    with_preview_command("pause_play_mode", json!({}), Duration::from_millis(240))
}

#[tauri::command]
fn stop_preview() -> Result<PreviewCommandResult, String> {
    with_preview_command("stop_play_mode", json!({}), Duration::from_millis(240))
}

#[tauri::command]
fn poll_preview() -> Result<PreviewCommandResult, String> {
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
    payload: Value,
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

fn spawn_preview_process(layout: &StudioLayout) -> Result<PreviewSession, String> {
    let (mut command, runner, current_dir) = preview_command(layout)?;

    let mut child = command
        .current_dir(current_dir)
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .map_err(|error| format!("failed to spawn preview runner {runner}: {error}"))?;

    let stdin = child
        .stdin
        .take()
        .ok_or_else(|| "preview stdin is not available".to_string())?;
    let stdout = child
        .stdout
        .take()
        .ok_or_else(|| "preview stdout is not available".to_string())?;
    let stderr = child
        .stderr
        .take()
        .ok_or_else(|| "preview stderr is not available".to_string())?;
    let (sender, receiver) = mpsc::channel::<String>();
    let stdout_sender = sender.clone();

    thread::spawn(move || {
        let reader = BufReader::new(stdout);
        for line in reader.lines().flatten() {
            if stdout_sender.send(line).is_err() {
                break;
            }
        }
    });

    thread::spawn(move || {
        let reader = BufReader::new(stderr);
        for line in reader.lines().flatten() {
            if sender.send(format!("[stderr] {line}")).is_err() {
                break;
            }
        }
    });

    Ok(PreviewSession {
        child,
        stdin,
        receiver,
        runner,
        seq: 1,
        status: "starting".to_string(),
    })
}

fn preview_command(layout: &StudioLayout) -> Result<(Command, String, PathBuf), String> {
    if let Some(sdk_root) = &layout.sdk_root {
        for candidate in [
            sdk_root.join("bin").join(if cfg!(windows) {
                "borealis-preview.exe"
            } else {
                "borealis-preview"
            }),
            sdk_root.join("preview").join(if cfg!(windows) {
                "borealis-preview.exe"
            } else {
                "borealis-preview"
            }),
        ] {
            if candidate.exists() {
                return Ok((
                    Command::new(&candidate),
                    normalize_path(&candidate),
                    sdk_root.clone(),
                ));
            }
        }

        if let (Some(zt), Some(project)) = (sdk_zt_path(sdk_root), sdk_preview_project(sdk_root)) {
            let mut command = Command::new(&zt);
            command.arg("run").arg(&project);
            return Ok((
                command,
                format!("{} run {}", normalize_path(&zt), normalize_path(&project)),
                sdk_root.clone(),
            ));
        }
    }

    if let Some(repo_root) = &layout.repo_root {
        let compiled =
            repo_root
                .join("tools/borealis-editor/preview/build")
                .join(if cfg!(windows) {
                    "borealis-preview.exe"
                } else {
                    "borealis-preview"
                });
        if compiled.exists() {
            return Ok((
                Command::new(&compiled),
                normalize_path(&compiled),
                repo_root.clone(),
            ));
        }

        let zt = repo_root.join(if cfg!(windows) { "zt.exe" } else { "zt-linux" });
        let preview_project = repo_root.join("tools/borealis-editor/preview/zenith.ztproj");
        let mut command = Command::new(&zt);
        command.arg("run").arg(&preview_project);
        return Ok((
            command,
            format!(
                "{} run {}",
                normalize_path(&zt),
                normalize_path(&preview_project)
            ),
            repo_root.clone(),
        ));
    }

    Err(
        "preview runtime unavailable: configure BOREALIS_SDK_ROOT or bundle runtime/sdk"
            .to_string(),
    )
}

fn send_preview_command(
    session: &mut PreviewSession,
    kind: &str,
    payload: Value,
) -> Result<(), String> {
    let message = json!({
        "protocol": 1,
        "seq": session.seq,
        "channel": "command",
        "kind": kind,
        "payload": payload
    });
    session.seq += 1;
    writeln!(session.stdin, "{message}")
        .map_err(|error| format!("failed to write preview command {kind}: {error}"))?;
    session
        .stdin
        .flush()
        .map_err(|error| format!("failed to flush preview command {kind}: {error}"))
}

fn collect_preview_events(session: &mut PreviewSession, wait: Duration) -> Vec<PreviewEvent> {
    let deadline = Instant::now() + wait;
    let mut events = Vec::new();

    loop {
        match session.receiver.try_recv() {
            Ok(line) => {
                let event = parse_preview_event(line);
                if let Some(status) = event.status.as_ref() {
                    session.status = status.clone();
                }
                events.push(event);
            }
            Err(TryRecvError::Empty) if Instant::now() < deadline => {
                thread::sleep(Duration::from_millis(20));
            }
            Err(TryRecvError::Empty) => break,
            Err(TryRecvError::Disconnected) => {
                session.status = "exited".to_string();
                break;
            }
        }
    }

    if !preview_is_alive(session) && session.status != "stopped" {
        session.status = "exited".to_string();
    }

    events
}

fn parse_preview_event(raw: String) -> PreviewEvent {
    let parsed = serde_json::from_str::<Value>(&raw).ok();
    let payload = parsed.as_ref().and_then(|value| value.get("payload"));

    PreviewEvent {
        channel: parsed
            .as_ref()
            .and_then(|value| value.get("channel"))
            .and_then(Value::as_str)
            .map(str::to_string),
        kind: parsed
            .as_ref()
            .and_then(|value| value.get("kind"))
            .and_then(Value::as_str)
            .map(str::to_string),
        status: payload
            .and_then(|value| value.get("status"))
            .and_then(Value::as_str)
            .map(str::to_string),
        message: payload
            .and_then(|value| value.get("message"))
            .and_then(Value::as_str)
            .map(str::to_string)
            .or_else(|| raw.strip_prefix("[stderr] ").map(str::to_string)),
        loaded: payload
            .and_then(|value| value.get("loaded"))
            .and_then(Value::as_bool),
        entity_count: payload
            .and_then(|value| value.get("entity_count"))
            .and_then(Value::as_i64),
        raw,
    }
}

fn preview_result(session: &PreviewSession, events: Vec<PreviewEvent>) -> PreviewCommandResult {
    PreviewCommandResult {
        status: session.status.clone(),
        runner: Some(session.runner.clone()),
        events,
    }
}

fn preview_is_alive(session: &mut PreviewSession) -> bool {
    match session.child.try_wait() {
        Ok(Some(_)) => false,
        Ok(None) => true,
        Err(_) => false,
    }
}

fn read_scene_document(project_root: &Path, path: &Path) -> Result<SceneDocument, String> {
    let raw = fs::read_to_string(path)
        .map_err(|error| format!("failed to read scene {}: {error}", normalize_path(path)))?;
    let value: Value =
        serde_json::from_str(&raw).map_err(|error| format!("invalid scene json: {error}"))?;
    let name = value
        .get("name")
        .and_then(Value::as_str)
        .unwrap_or("scene")
        .to_string();
    let document_id = value
        .get("document_id")
        .and_then(Value::as_str)
        .unwrap_or("")
        .to_string();
    let mut entities = Vec::new();

    if let Some(items) = value.get("entities").and_then(Value::as_array) {
        for (index, item) in items.iter().enumerate() {
            entities.push(read_entity(item, index));
        }
    }

    Ok(SceneDocument {
        name,
        path: normalize_path(path.strip_prefix(project_root).unwrap_or(path)),
        document_id,
        entities,
    })
}

fn read_entity(value: &Value, index: usize) -> SceneEntity {
    let transform = value.get("transform").unwrap_or(&Value::Null);
    let components = value
        .get("components")
        .and_then(Value::as_array)
        .map(|items| items.iter().map(read_component).collect())
        .unwrap_or_default();

    SceneEntity {
        id: text_value(value, &["stable_id", "id"]).unwrap_or_else(|| format!("entity-{index}")),
        name: text_value(value, &["name"]).unwrap_or_else(|| format!("Entity {index}")),
        layer: text_value(value, &["layer"]).unwrap_or_else(|| "default".to_string()),
        parent: text_value(value, &["parent", "parent_id"]),
        tags: value
            .get("tags")
            .and_then(Value::as_array)
            .map(|items| {
                items
                    .iter()
                    .filter_map(Value::as_str)
                    .map(str::to_string)
                    .collect()
            })
            .unwrap_or_default(),
        components,
        transform: Transform3d {
            x: number_value(transform, "x", 0.0),
            y: number_value(transform, "y", 0.0),
            z: number_value(transform, "z", 0.0),
            rotation_x: number_value(transform, "rotation_x", 0.0),
            rotation_y: number_value(transform, "rotation_y", 0.0),
            rotation_z: number_value(
                transform,
                "rotation_z",
                number_value(transform, "rotation", 0.0),
            ),
            scale_x: number_value(transform, "scale_x", 1.0),
            scale_y: number_value(transform, "scale_y", 1.0),
            scale_z: number_value(transform, "scale_z", 1.0),
        },
    }
}

fn read_component(value: &Value) -> SceneComponent {
    let mut properties = serde_json::Map::new();
    if let Some(object) = value.as_object() {
        for (key, item) in object {
            if !matches!(key.as_str(), "kind" | "type") {
                properties.insert(key.clone(), item.clone());
            }
        }
    }

    SceneComponent {
        kind: text_value(value, &["kind", "type"]).unwrap_or_else(|| "component".to_string()),
        asset: text_value(value, &["asset", "model", "path"]),
        script: text_value(value, &["script", "source"]),
        profile: text_value(value, &["profile", "projection"]),
        properties: Value::Object(properties),
    }
}

fn read_project_name(project_path: &Path) -> Option<String> {
    let raw = fs::read_to_string(project_path).ok()?;
    for line in raw.lines() {
        let trimmed = line.trim();
        if let Some((key, value)) = trimmed.split_once('=') {
            if key.trim() == "name" {
                return Some(value.trim().trim_matches('"').to_string());
            }
        }
    }
    None
}

fn collect_assets(root: &Path, current: &Path) -> Result<Vec<ProjectAsset>, String> {
    let mut assets = Vec::new();
    collect_files(root, current, &mut |path| {
        if let Some(kind) = asset_kind(path) {
            assets.push(ProjectAsset {
                id: normalize_path(path),
                name: path
                    .file_name()
                    .and_then(|name| name.to_str())
                    .unwrap_or("asset")
                    .to_string(),
                path: normalize_path(path.strip_prefix(root).unwrap_or(path)),
                kind,
            });
        }
    })?;
    assets.sort_by(|left, right| left.path.cmp(&right.path));
    assets.truncate(160);
    Ok(assets)
}

fn collect_scripts(root: &Path, current: &Path) -> Result<Vec<ScriptDocument>, String> {
    let mut scripts = Vec::new();
    collect_files(root, current, &mut |path| {
        if path.extension().and_then(|value| value.to_str()) == Some("zt") {
            let content = fs::read_to_string(path).unwrap_or_default();
            scripts.push(ScriptDocument {
                path: normalize_path(path.strip_prefix(root).unwrap_or(path)),
                name: path
                    .file_name()
                    .and_then(|name| name.to_str())
                    .unwrap_or("script.zt")
                    .to_string(),
                content,
                dirty: Some(false),
            });
        }
    })?;
    scripts.sort_by(|left, right| left.path.cmp(&right.path));
    scripts.truncate(40);
    Ok(scripts)
}

fn collect_files(
    root: &Path,
    current: &Path,
    visitor: &mut dyn FnMut(&Path),
) -> Result<(), String> {
    let entries = fs::read_dir(current)
        .map_err(|error| format!("failed to read {}: {error}", normalize_path(current)))?;
    for entry in entries {
        let entry = entry.map_err(|error| format!("failed to read directory entry: {error}"))?;
        let path = entry.path();
        if path.is_dir() {
            let name = path
                .file_name()
                .and_then(|name| name.to_str())
                .unwrap_or("");
            if matches!(name, "target" | "build" | ".ztc-tmp" | "native") {
                continue;
            }
            collect_files(root, &path, visitor)?;
        } else {
            visitor(&path);
        }
    }
    Ok(())
}

fn project_templates() -> Vec<ProjectTemplate> {
    vec![
        ProjectTemplate {
            id: "empty3d".to_string(),
            name: "Empty 3D".to_string(),
            summary: "Cena 3D limpa com camera, cubo e espaco para os primeiros sistemas."
                .to_string(),
            default_name: "Projeto Borealis 3D".to_string(),
            tags: vec!["3D".to_string(), "starter".to_string()],
        },
        ProjectTemplate {
            id: "topdown2d".to_string(),
            name: "Top-down 2D".to_string(),
            summary: "Cena 2D pequena para movimento, sprites e testes iniciais de gameplay."
                .to_string(),
            default_name: "Projeto Borealis 2D".to_string(),
            tags: vec!["2D".to_string(), "gameplay".to_string()],
        },
        ProjectTemplate {
            id: "scripted3d".to_string(),
            name: "Scripted 3D".to_string(),
            summary: "Amostra 3D com script anexado para iterar no preview.".to_string(),
            default_name: "Projeto Borealis Scripted".to_string(),
            tags: vec!["3D".to_string(), "scripts".to_string()],
        },
    ]
}

fn documentation_links(layout: &StudioLayout) -> Vec<DocumentationLink> {
    let docs_root = layout.repo_root.as_ref().unwrap_or(&layout.app_root);
    vec![
        DocumentationLink {
            title: "Studio README".to_string(),
            path: normalize_path(&docs_root.join("tools/borealis-studio/README.md")),
            summary: "Como rodar o editor Tauri + React e a ponte de preview.".to_string(),
        },
        DocumentationLink {
            title: "Stack Decision".to_string(),
            path: normalize_path(
                &docs_root
                    .join("packages/borealis/decisions/015-borealis-studio-tauri-react-stack.md"),
            ),
            summary: "Stack atual aceita e limites de responsabilidade.".to_string(),
        },
        DocumentationLink {
            title: "Preview IPC".to_string(),
            path: normalize_path(
                &docs_root.join("packages/borealis/decisions/014-borealis-editor-preview-ipc.md"),
            ),
            summary: "Protocolo JSONL usado pelo Play mode.".to_string(),
        },
    ]
}

fn default_projects_dir(workspace: &Path) -> PathBuf {
    if let Ok(user_profile) = std::env::var("USERPROFILE") {
        return Path::new(&user_profile)
            .join("Documents")
            .join("Projetos")
            .join("Borealis Projects");
    }

    workspace.join("Borealis Projects")
}

fn normalize_project_file(path: PathBuf) -> Result<PathBuf, String> {
    let path = if path.is_dir() {
        path.join("zenith.ztproj")
    } else {
        path
    };

    if !path.exists() {
        return Err(format!("project file not found: {}", normalize_path(&path)));
    }

    path.canonicalize().map_err(|error| {
        format!(
            "failed to resolve project path {}: {error}",
            normalize_path(&path)
        )
    })
}

fn resolve_project_path(project_root: &Path, path: &str) -> PathBuf {
    let raw = PathBuf::from(path);
    if raw.is_absolute() {
        raw
    } else {
        project_root.join(raw)
    }
}

fn clean_project_name(name: &str) -> String {
    let trimmed = name.trim();
    if trimmed.is_empty() {
        "Borealis Project".to_string()
    } else {
        trimmed.to_string()
    }
}

fn slugify_project_name(name: &str) -> String {
    let mut out = String::new();
    let mut last_dash = false;

    for ch in name.chars() {
        if ch.is_ascii_alphanumeric() {
            out.push(ch.to_ascii_lowercase());
            last_dash = false;
        } else if !last_dash {
            out.push('-');
            last_dash = true;
        }
    }

    let slug = out.trim_matches('-').to_string();
    if slug.is_empty() {
        "borealis-project".to_string()
    } else {
        slug
    }
}

fn project_manifest(slug: &str) -> String {
    format!(
        "[project]\nname = \"{}\"\nversion = \"0.1.0\"\nkind = \"app\"\n\n[source]\nroot = \"src\"\n\n[app]\nentry = \"app.main\"\n\n[build]\ntarget = \"native\"\nprofile = \"debug\"\n",
        slug
    )
}

fn template_scene_json(project_name: &str, template_id: &str) -> String {
    let (document_id, entities) = match template_id {
        "topdown2d" => (
            "scene:main_2d",
            json!([
                {
                    "stable_id": "camera-main",
                    "name": "Camera 2D",
                    "layer": "camera",
                    "tags": ["camera", "2d"],
                    "transform": { "x": 0, "y": 0, "z": 120, "rotation": 0, "scale_x": 1, "scale_y": 1, "scale_z": 1 },
                    "components": [{ "kind": "camera2d", "projection": "orthographic" }]
                },
                {
                    "stable_id": "player",
                    "name": "Player",
                    "layer": "actors",
                    "tags": ["player", "controllable"],
                    "transform": { "x": 0, "y": 0, "z": 0, "rotation": 0, "scale_x": 1, "scale_y": 1, "scale_z": 1 },
                    "components": [{ "kind": "script", "script": "src/app/main.zt" }]
                }
            ]),
        ),
        "scripted3d" => (
            "scene:main_scripted_3d",
            json!([
                {
                    "stable_id": "camera-main",
                    "name": "Camera 3D",
                    "layer": "world3d",
                    "tags": ["camera", "3d"],
                    "transform": { "x": 120, "y": -120, "z": 120, "rotation_x": -35, "rotation_y": 35, "rotation": 0, "scale_x": 1, "scale_y": 1, "scale_z": 1 },
                    "components": [{ "kind": "camera3d", "projection": "perspective" }]
                },
                {
                    "stable_id": "scripted-cube",
                    "name": "Scripted Cube",
                    "layer": "world3d",
                    "tags": ["3d", "scripted"],
                    "transform": { "x": 0, "y": 0, "z": 24, "rotation": 0, "scale_x": 1, "scale_y": 1, "scale_z": 1 },
                    "components": [
                        { "kind": "cube3d", "size": 48 },
                        { "kind": "script", "script": "src/app/main.zt" }
                    ]
                }
            ]),
        ),
        _ => (
            "scene:main_3d",
            json!([
                {
                    "stable_id": "camera-main",
                    "name": "Camera 3D",
                    "layer": "world3d",
                    "tags": ["camera", "3d"],
                    "transform": { "x": 120, "y": -120, "z": 120, "rotation_x": -35, "rotation_y": 35, "rotation": 0, "scale_x": 1, "scale_y": 1, "scale_z": 1 },
                    "components": [{ "kind": "camera3d", "projection": "perspective" }]
                },
                {
                    "stable_id": "cube-01",
                    "name": "Cube 01",
                    "layer": "world3d",
                    "tags": ["3d"],
                    "transform": { "x": 0, "y": 0, "z": 24, "rotation": 0, "scale_x": 1, "scale_y": 1, "scale_z": 1 },
                    "components": [{ "kind": "cube3d", "size": 48 }]
                }
            ]),
        ),
    };

    let scene = json!({
        "version": 1,
        "name": project_name,
        "document_id": document_id,
        "entities": entities,
    });

    format!(
        "{}\n",
        serde_json::to_string_pretty(&scene).unwrap_or_else(|_| "{}".to_string())
    )
}

fn template_main_script(slug: &str, template_id: &str) -> String {
    if template_id == "topdown2d" {
        return format!(
            "namespace app.main\n\npublic func update(entity_id: text, dt: float) -> result<void, core.Error>\n    return success()\nend\n\n-- Project: {}\n",
            slug
        );
    }

    format!(
        "namespace app.main\n\npublic func update(entity_id: text, dt: float) -> result<void, core.Error>\n    return success()\nend\n\n-- Project: {}\n",
        slug
    )
}

fn template_triangle_obj() -> String {
    "o Triangle\nv 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n".to_string()
}

fn asset_kind(path: &Path) -> Option<String> {
    match path
        .extension()
        .and_then(|value| value.to_str())
        .unwrap_or("")
        .to_ascii_lowercase()
        .as_str()
    {
        "scene" | "json" if path.to_string_lossy().contains(".scene.") => Some("scene".to_string()),
        "zt" => Some("script".to_string()),
        "obj" | "glb" | "gltf" | "fbx" | "iqm" | "m3d" => Some("model".to_string()),
        "png" | "jpg" | "jpeg" | "webp" => Some("texture".to_string()),
        "wav" | "ogg" | "mp3" => Some("audio".to_string()),
        _ => None,
    }
}

fn text_value(value: &Value, keys: &[&str]) -> Option<String> {
    keys.iter()
        .find_map(|key| value.get(key).and_then(Value::as_str).map(str::to_string))
}

fn number_value(value: &Value, key: &str, fallback: f64) -> f64 {
    value.get(key).and_then(Value::as_f64).unwrap_or(fallback)
}

fn resolve_studio_path(path: &str) -> PathBuf {
    let raw = PathBuf::from(path);
    if raw.is_absolute() {
        raw
    } else {
        studio_layout().base_root.join(path)
    }
}

fn default_project_path(layout: &StudioLayout) -> PathBuf {
    if let Some(repo_root) = &layout.repo_root {
        return repo_root.join("packages/borealis/zenith.ztproj");
    }

    layout.base_root.join("zenith.ztproj")
}

fn runtime_status(layout: &StudioLayout) -> (String, String) {
    if let Some(sdk_root) = &layout.sdk_root {
        if sdk_zt_path(sdk_root).is_some() {
            return (
                "sdk".to_string(),
                format!("SDK local pronto: {}", normalize_path(sdk_root)),
            );
        }

        return (
            "sdk".to_string(),
            format!("SDK encontrado sem zt runner: {}", normalize_path(sdk_root)),
        );
    }

    if let Some(repo_root) = &layout.repo_root {
        return (
            "repo-dev".to_string(),
            format!("Modo dev pelo repositorio: {}", normalize_path(repo_root)),
        );
    }

    (
        "missing".to_string(),
        "Runtime ausente. Configure BOREALIS_SDK_ROOT ou bundle runtime/sdk.".to_string(),
    )
}

fn studio_layout() -> StudioLayout {
    let app_root = studio_app_root();
    let repo_root = find_repo_root(&app_root);
    let sdk_root = find_sdk_root(&app_root);
    let base_root = repo_root.clone().unwrap_or_else(|| app_root.clone());

    StudioLayout {
        app_root,
        base_root,
        repo_root,
        sdk_root,
    }
}

fn studio_app_root() -> PathBuf {
    let manifest_dir = Path::new(env!("CARGO_MANIFEST_DIR"));
    manifest_dir
        .parent()
        .map(Path::to_path_buf)
        .unwrap_or_else(|| manifest_dir.to_path_buf())
        .canonicalize()
        .unwrap_or_else(|_| manifest_dir.parent().unwrap_or(manifest_dir).to_path_buf())
}

fn find_repo_root(start: &Path) -> Option<PathBuf> {
    for candidate in start.ancestors() {
        if candidate.join("zt.exe").exists()
            && candidate.join("packages/borealis/zenith.ztproj").exists()
        {
            return Some(
                candidate
                    .canonicalize()
                    .unwrap_or_else(|_| candidate.to_path_buf()),
            );
        }
    }

    None
}

fn find_sdk_root(app_root: &Path) -> Option<PathBuf> {
    if let Ok(value) = std::env::var("BOREALIS_SDK_ROOT") {
        let path = PathBuf::from(value);
        if is_sdk_root(&path) {
            return Some(path.canonicalize().unwrap_or(path));
        }
    }

    for candidate in [
        app_root.join("runtime/sdk"),
        app_root.join("sdk"),
        app_root.join("../runtime/sdk"),
        app_root.join("../sdk"),
    ] {
        if is_sdk_root(&candidate) {
            return Some(candidate.canonicalize().unwrap_or(candidate));
        }
    }

    None
}

fn is_sdk_root(path: &Path) -> bool {
    sdk_zt_path(path).is_some()
        || sdk_preview_project(path).is_some()
        || path.join("stdlib").exists()
        || path.join("packages/borealis/zenith.ztproj").exists()
}

fn sdk_zt_path(sdk_root: &Path) -> Option<PathBuf> {
    for candidate in [
        sdk_root
            .join("bin")
            .join(if cfg!(windows) { "zt.exe" } else { "zt" }),
        sdk_root.join(if cfg!(windows) { "zt.exe" } else { "zt" }),
    ] {
        if candidate.exists() {
            return Some(candidate);
        }
    }

    None
}

fn sdk_preview_project(sdk_root: &Path) -> Option<PathBuf> {
    for candidate in [
        sdk_root.join("preview/zenith.ztproj"),
        sdk_root.join("tools/borealis-editor/preview/zenith.ztproj"),
    ] {
        if candidate.exists() {
            return Some(candidate);
        }
    }

    None
}

#[cfg(test)]
fn workspace_root() -> PathBuf {
    studio_layout().base_root
}

fn normalize_path(path: &Path) -> String {
    path.to_string_lossy().replace('\\', "/")
}

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![
            create_borealis_project,
            load_studio_home,
            load_studio_snapshot,
            pause_preview,
            poll_preview,
            read_text_file,
            start_preview,
            stop_preview,
            write_project_text_file,
            write_text_file
        ])
        .run(tauri::generate_context!())
        .expect("failed to run Borealis Studio");
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn workspace_root_finds_repo_root() {
        let root = workspace_root();
        assert!(root.join("zt.exe").exists());
        assert!(root.join("packages/borealis/zenith.ztproj").exists());
    }

    #[test]
    fn project_file_accepts_project_directory() {
        let root = workspace_root();
        let project_file = normalize_project_file(root.join("packages/borealis")).unwrap();
        assert_eq!(
            normalize_path(&project_file),
            normalize_path(&root.join("packages/borealis/zenith.ztproj"))
        );
    }

    #[test]
    fn generated_template_scene_is_valid_json() {
        let scene = template_scene_json("Projeto Teste", "scripted3d");
        let parsed: Value = serde_json::from_str(&scene).unwrap();
        assert_eq!(parsed.get("version").and_then(Value::as_i64), Some(1));
        assert!(parsed.get("entities").and_then(Value::as_array).is_some());
    }
}
