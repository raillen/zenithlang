use std::io::{BufRead, BufReader, Write};
use std::path::PathBuf;
use std::process::{Command, Stdio};
use std::sync::mpsc;
use std::thread;
use std::time::{Duration, Instant};

use serde_json::{json, Value};

use crate::models::preview::{PreviewCommandResult, PreviewEvent, PreviewSession};
use crate::models::project::StudioLayout;
use crate::utils::paths::normalize_path;

pub fn preview_scene_target(layout: &StudioLayout) -> (PathBuf, String) {
    if let Some(sdk_root) = &layout.sdk_root {
        let relative = PathBuf::from(".ztc-tmp")
            .join("borealis-studio")
            .join("preview.scene.json");
        return (sdk_root.join(&relative), normalize_path(&relative));
    }

    let path = std::env::temp_dir()
        .join("borealis-studio")
        .join("preview.scene.json");
    let ipc_path = normalize_path(path.strip_prefix(&layout.base_root).unwrap_or(&path));
    (path, ipc_path)
}

pub fn spawn_preview_process(layout: &StudioLayout) -> Result<PreviewSession, String> {
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
                let mut command = Command::new(&candidate);
                command.env("ZENITH_HOME", sdk_root);
                return Ok((command, normalize_path(&candidate), sdk_root.clone()));
            }
        }

        if let (Some(zt), Some(project)) = (
            sdk_zt_path(sdk_root),
            sdk_preview_project(sdk_root),
        ) {
            let mut command = Command::new(&zt);
            command.arg("run").arg(&project);
            command.env("ZENITH_HOME", sdk_root);
            return Ok((
                command,
                format!("{} run {}", normalize_path(&zt), normalize_path(&project)),
                sdk_root.clone(),
            ));
        }
    }

    if let Some(repo_root) = &layout.repo_root {
        return Err(format!(
            "preview runtime unavailable: no repo-local preview harness is bundled under {}. Configure BOREALIS_SDK_ROOT or bundle runtime/sdk.",
            normalize_path(repo_root)
        ));
    }

    Err(
        "preview runtime unavailable: configure BOREALIS_SDK_ROOT or bundle runtime/sdk"
            .to_string(),
    )
}

pub fn preview_is_alive(session: &mut PreviewSession) -> bool {
    match session.child.try_wait() {
        Ok(Some(_)) => false,
        Ok(None) => true,
        Err(_) => false,
    }
}

pub fn send_preview_command(
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

pub fn collect_preview_events(
    session: &mut PreviewSession,
    wait: Duration,
) -> Vec<PreviewEvent> {
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
            Err(std::sync::mpsc::TryRecvError::Empty) if Instant::now() < deadline => {
                thread::sleep(Duration::from_millis(20));
            }
            Err(std::sync::mpsc::TryRecvError::Empty) => break,
            Err(std::sync::mpsc::TryRecvError::Disconnected) => {
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

pub fn parse_preview_event(raw: String) -> PreviewEvent {
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
        camera_count: payload
            .and_then(|value| value.get("camera_count"))
            .and_then(Value::as_i64),
        light_count: payload
            .and_then(|value| value.get("light_count"))
            .and_then(Value::as_i64),
        audio_count: payload
            .and_then(|value| value.get("audio_count"))
            .and_then(Value::as_i64),
        raw,
    }
}

pub fn preview_result(
    session: &PreviewSession,
    events: Vec<PreviewEvent>,
) -> PreviewCommandResult {
    PreviewCommandResult {
        status: session.status.clone(),
        runner: Some(session.runner.clone()),
        events,
    }
}

fn sdk_zt_path(sdk_root: &PathBuf) -> Option<PathBuf> {
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

fn sdk_preview_project(sdk_root: &PathBuf) -> Option<PathBuf> {
    let candidate = sdk_root.join("preview/zenith.ztproj");
    if candidate.exists() {
        Some(candidate)
    } else {
        None
    }
}
