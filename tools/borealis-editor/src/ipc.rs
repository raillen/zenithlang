use crate::messages::{PeerRole, PreviewChannel, PreviewEnvelope, PreviewPayload, PreviewStatus};
use std::io::{BufRead, BufReader, Write};
use std::path::Path;
use std::process::{Child, Command, Stdio};
use std::sync::mpsc::{self, Receiver, TryRecvError};
use std::thread;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TransportKind {
    JsonlStdio,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ConnectionState {
    Disconnected,
    Starting,
    Connected,
}

pub struct PreviewBridge {
    transport: TransportKind,
    state: ConnectionState,
    next_seq: u64,
    current_status: PreviewStatus,
    outgoing_lines: Vec<String>,
    sent_lines: Vec<String>,
    incoming: Vec<PreviewEnvelope>,
    child: Option<Child>,
    rx: Option<Receiver<String>>,
    preview_path: String,
    mock: bool,
}

impl PreviewBridge {
    pub fn new() -> Self {
        Self {
            transport: TransportKind::JsonlStdio,
            state: ConnectionState::Disconnected,
            next_seq: 1,
            current_status: PreviewStatus::Idle,
            outgoing_lines: Vec::new(),
            sent_lines: Vec::new(),
            incoming: Vec::new(),
            child: None,
            rx: None,
            preview_path: String::new(),
            mock: true,
        }
    }

    pub fn set_mock(&mut self, enabled: bool) {
        self.mock = enabled;
    }

    pub fn is_mock(&self) -> bool {
        self.mock
    }

    pub fn set_preview_path(&mut self, path: String) {
        self.preview_path = path;
    }

    fn spawn_preview_process(&mut self, _project_path: &str) -> Result<(), String> {
        let (cmd, args, desc) = self.resolve_preview_binary()?;

        tracing::info!("Spawning preview process: {} {:?}", cmd, args);

        let mut child = Command::new(&cmd)
            .args(&args)
            .stdin(Stdio::piped())
            .stdout(Stdio::piped())
            .stderr(Stdio::piped())
            .spawn()
            .map_err(|e| format!("failed to spawn preview: {e}"))?;

        let stdout = child
            .stdout
            .take()
            .ok_or_else(|| "failed to open stdout".to_string())?;
        let (tx, rx) = mpsc::channel();

        thread::spawn(move || {
            let mut reader = BufReader::new(stdout);
            let mut line = String::new();
            loop {
                line.clear();
                match reader.read_line(&mut line) {
                    Ok(0) => break,
                    Ok(_) => {
                        let _ = tx.send(line.trim().to_string());
                    }
                    Err(_) => break,
                }
            }
        });

        self.child = Some(child);
        self.rx = Some(rx);
        tracing::info!("Preview process spawned: {}", desc);
        Ok(())
    }

    fn resolve_preview_binary(&self) -> Result<(String, Vec<String>, String), String> {
        if !self.preview_path.is_empty() && Path::new(&self.preview_path).exists() {
            if is_zenith_runner(&self.preview_path) {
                let project_path = preview_project_path();
                return Ok((
                    self.preview_path.clone(),
                    vec!["run".to_string(), project_path.clone()],
                    format!("{} (via zt runner)", self.preview_path),
                ));
            }
            return Ok((
                self.preview_path.clone(),
                Vec::new(),
                self.preview_path.clone(),
            ));
        }

        let exe_name = if cfg!(windows) {
            "borealis-preview.exe"
        } else {
            "borealis-preview"
        };

        let candidates = [
            format!("preview/build/{}", exe_name),
            format!("build/{}", exe_name),
            format!("target/{}", exe_name),
        ];

        for candidate in &candidates {
            if Path::new(candidate).exists() {
                return Ok((
                    candidate.clone(),
                    Vec::new(),
                    format!("{} (compiled)", candidate),
                ));
            }
        }

        let bundled_zenith = if cfg!(windows) { "zt.exe" } else { "zt" };
        if Path::new(bundled_zenith).exists() {
            let project_path = preview_project_path();
            return Ok((
                bundled_zenith.to_string(),
                vec!["run".to_string(), project_path.clone()],
                format!("{bundled_zenith} (workspace runner)"),
            ));
        }

        if !self.preview_path.is_empty() {
            let project_path = preview_project_path();
            return Ok((
                self.preview_path.clone(),
                vec!["run".to_string(), project_path.clone()],
                format!("{} (via zt.exe)", self.preview_path),
            ));
        }

        Err("preview binary not found. build with: zt run preview/zenith.ztproj".to_string())
    }

    fn read_incoming_messages(&mut self) {
        if let Some(rx) = &self.rx {
            loop {
                match rx.try_recv() {
                    Ok(line) => {
                        if let Ok(envelope) = PreviewEnvelope::from_json_line(&line) {
                            self.incoming.push(envelope);
                        }
                    }
                    Err(TryRecvError::Empty) => break,
                    Err(TryRecvError::Disconnected) => {
                        self.state = ConnectionState::Disconnected;
                        self.current_status = PreviewStatus::Stopped;
                        break;
                    }
                }
            }
        }
    }

    fn write_outgoing_lines(&mut self, lines: &[String]) -> Result<(), String> {
        if self.mock || lines.is_empty() {
            return Ok(());
        }

        let child = self
            .child
            .as_mut()
            .ok_or_else(|| "preview process is not running".to_string())?;
        let stdin = child
            .stdin
            .as_mut()
            .ok_or_else(|| "failed to open preview stdin".to_string())?;
        for line in lines {
            writeln!(stdin, "{}", line).map_err(|e| format!("stdin write error: {e}"))?;
        }
        stdin
            .flush()
            .map_err(|e| format!("stdin flush error: {e}"))?;
        Ok(())
    }

    pub fn start_preview(&mut self, project_path: Option<String>) -> Result<String, String> {
        let project = project_path.unwrap_or_else(|| "packages/borealis/zenith.ztproj".to_string());
        self.state = ConnectionState::Starting;
        self.current_status = PreviewStatus::Starting;

        if self.mock {
            tracing::info!("Starting preview in MOCK mode (no process spawned)");
            let hello = self.send(
                PreviewChannel::Command,
                PreviewPayload::Hello {
                    role: PeerRole::Editor,
                    project_path: Some(project.clone()),
                },
            )?;
            self.flush_outgoing()?;
            self.state = ConnectionState::Connected;
            self.current_status = PreviewStatus::Ready;
            Ok(hello)
        } else {
            self.spawn_preview_process(&project)?;

            let hello = self.send(
                PreviewChannel::Command,
                PreviewPayload::Hello {
                    role: PeerRole::Editor,
                    project_path: Some(project.clone()),
                },
            )?;

            self.flush_outgoing()?;
            self.read_incoming_messages();
            Ok(hello)
        }
    }

    pub fn stop_preview(&mut self) -> Result<String, String> {
        let line = self.send(PreviewChannel::Command, PreviewPayload::StopPlayMode)?;
        self.flush_outgoing()?;

        if !self.mock {
            if let Some(mut child) = self.child.take() {
                let _ = child.kill();
                let _ = child.wait();
            }
        } else {
            self.child = None;
        }

        self.current_status = PreviewStatus::Stopped;
        self.state = ConnectionState::Disconnected;
        Ok(line)
    }

    pub fn update(&mut self) {
        self.read_incoming_messages();
        for message in &self.incoming {
            if let PreviewPayload::Status { status } = &message.payload {
                self.current_status = *status;
                if matches!(status, PreviewStatus::Ready | PreviewStatus::Playing) {
                    self.state = ConnectionState::Connected;
                }
            }
        }
    }

    pub fn flush_outgoing(&mut self) -> Result<(), String> {
        let lines = self.outgoing_lines.clone();
        if self.mock {
            for line in &lines {
                self.simulate_mock_response(line);
            }
        }
        self.write_outgoing_lines(&lines)?;
        self.sent_lines.extend(lines);
        self.outgoing_lines.clear();
        Ok(())
    }

    pub fn open_scene(&mut self, path: String) -> Result<String, String> {
        self.current_status = PreviewStatus::Loading;
        let line = self.send(PreviewChannel::Command, PreviewPayload::OpenScene { path })?;
        self.flush_outgoing()?;
        Ok(line)
    }

    pub fn open_project(&mut self, path: String) -> Result<String, String> {
        self.current_status = PreviewStatus::Loading;
        let line = self.send(
            PreviewChannel::Command,
            PreviewPayload::OpenProject { path },
        )?;
        self.flush_outgoing()?;
        Ok(line)
    }

    pub fn enter_play_mode(&mut self) -> Result<String, String> {
        self.current_status = PreviewStatus::Playing;
        let line = self.send(PreviewChannel::Command, PreviewPayload::EnterPlayMode)?;
        self.flush_outgoing()?;
        Ok(line)
    }

    pub fn select_entity(&mut self, stable_id: String) -> Result<String, String> {
        let line = self.send(
            PreviewChannel::Command,
            PreviewPayload::SelectEntity { stable_id },
        )?;
        self.flush_outgoing()?;
        Ok(line)
    }

    pub fn ping(&mut self, token: String) -> Result<String, String> {
        let line = self.send(
            PreviewChannel::Heartbeat,
            PreviewPayload::Ping {
                token: token.clone(),
            },
        )?;
        self.flush_outgoing()?;
        Ok(line)
    }

    pub fn drain_incoming(&mut self) -> Vec<PreviewEnvelope> {
        let messages = self.incoming.clone();
        self.incoming.clear();
        messages
    }

    pub fn drain_outgoing_lines(&mut self) -> Vec<String> {
        let lines = self.sent_lines.clone();
        self.sent_lines.clear();
        lines
    }

    pub fn transport(&self) -> TransportKind {
        self.transport
    }
    pub fn state(&self) -> ConnectionState {
        self.state
    }
    pub fn current_status(&self) -> PreviewStatus {
        self.current_status
    }

    fn send(&mut self, channel: PreviewChannel, payload: PreviewPayload) -> Result<String, String> {
        let envelope = PreviewEnvelope::new(self.alloc_seq(), channel, payload);
        let line = envelope
            .to_json_line()
            .map_err(|e| format!("encode error: {e}"))?;
        self.outgoing_lines.push(line.clone());
        Ok(line)
    }

    fn alloc_seq(&mut self) -> u64 {
        let seq = self.next_seq;
        self.next_seq += 1;
        seq
    }

    fn simulate_mock_response(&mut self, outgoing_line: &str) {
        if let Ok(envelope) = PreviewEnvelope::from_json_line(outgoing_line) {
            match &envelope.payload {
                PreviewPayload::Hello { .. } => {
                    let seq = self.alloc_seq();
                    let resp = PreviewEnvelope::new(
                        seq,
                        PreviewChannel::Event,
                        PreviewPayload::Status {
                            status: PreviewStatus::Ready,
                        },
                    );
                    if let Ok(line) = resp.to_json_line() {
                        if let Ok(mock_env) = PreviewEnvelope::from_json_line(&line) {
                            self.incoming.push(mock_env);
                        }
                    }
                }
                PreviewPayload::OpenProject { .. } | PreviewPayload::OpenScene { .. } => {
                    let seq = self.alloc_seq();
                    let resp = PreviewEnvelope::new(
                        seq,
                        PreviewChannel::Event,
                        PreviewPayload::Status {
                            status: PreviewStatus::Ready,
                        },
                    );
                    if let Ok(line) = resp.to_json_line() {
                        if let Ok(mock_env) = PreviewEnvelope::from_json_line(&line) {
                            self.incoming.push(mock_env);
                        }
                    }
                }
                PreviewPayload::EnterPlayMode => {
                    let seq = self.alloc_seq();
                    let resp = PreviewEnvelope::new(
                        seq,
                        PreviewChannel::Event,
                        PreviewPayload::Status {
                            status: PreviewStatus::Playing,
                        },
                    );
                    if let Ok(line) = resp.to_json_line() {
                        if let Ok(mock_env) = PreviewEnvelope::from_json_line(&line) {
                            self.incoming.push(mock_env);
                        }
                    }
                }
                PreviewPayload::StopPlayMode => {
                    let seq = self.alloc_seq();
                    let resp = PreviewEnvelope::new(
                        seq,
                        PreviewChannel::Event,
                        PreviewPayload::Status {
                            status: PreviewStatus::Stopped,
                        },
                    );
                    if let Ok(line) = resp.to_json_line() {
                        if let Ok(mock_env) = PreviewEnvelope::from_json_line(&line) {
                            self.incoming.push(mock_env);
                        }
                    }
                }
                PreviewPayload::Ping { token } => {
                    let seq = self.alloc_seq();
                    let resp = PreviewEnvelope::new(
                        seq,
                        PreviewChannel::Heartbeat,
                        PreviewPayload::Pong {
                            token: token.clone(),
                        },
                    );
                    if let Ok(line) = resp.to_json_line() {
                        if let Ok(mock_env) = PreviewEnvelope::from_json_line(&line) {
                            self.incoming.push(mock_env);
                        }
                    }
                }
                _ => {}
            }
        }
    }
}

fn is_zenith_runner(path: &str) -> bool {
    Path::new(path)
        .file_name()
        .and_then(|name| name.to_str())
        .map(|name| name.eq_ignore_ascii_case("zt.exe") || name.eq_ignore_ascii_case("zt"))
        .unwrap_or(false)
}

fn preview_project_path() -> String {
    Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("preview")
        .join("zenith.ztproj")
        .to_string_lossy()
        .replace('\\', "/")
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs::File;
    use std::time::{SystemTime, UNIX_EPOCH};

    #[test]
    fn resolves_existing_zenith_runner_with_preview_args() {
        let unique = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .expect("system time should be valid")
            .as_nanos();
        let temp_dir = std::env::temp_dir().join(format!("codex-preview-{unique}"));
        std::fs::create_dir_all(&temp_dir).expect("temp test dir should be created");
        let temp_path = temp_dir.join("zt.exe");
        File::create(&temp_path).expect("temp runner should be created");

        let mut bridge = PreviewBridge::new();
        bridge.set_mock(false);
        bridge.set_preview_path(temp_path.to_string_lossy().to_string());

        let (_cmd, args, desc) = bridge
            .resolve_preview_binary()
            .expect("preview runner should resolve");

        assert_eq!(args.first().map(String::as_str), Some("run"));
        assert!(args
            .get(1)
            .map(|value| value.ends_with("tools/borealis-editor/preview/zenith.ztproj"))
            .unwrap_or(false));
        assert!(desc.contains("via zt runner"));

        let _ = std::fs::remove_file(&temp_path);
        let _ = std::fs::remove_dir(&temp_dir);
    }
}
