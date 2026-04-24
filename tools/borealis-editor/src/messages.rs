use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum PreviewChannel {
    Command,
    Event,
    Response,
    Error,
    Heartbeat,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum PeerRole {
    Editor,
    Preview,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum PreviewStatus {
    Idle,
    Starting,
    Loading,
    Ready,
    Playing,
    Paused,
    Stopped,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum DiagnosticLevel {
    Info,
    Warning,
    Error,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(tag = "kind", content = "payload", rename_all = "snake_case")]
pub enum PreviewPayload {
    Hello {
        role: PeerRole,
        project_path: Option<String>,
    },
    OpenProject {
        path: String,
    },
    OpenScene {
        path: String,
    },
    EnterPlayMode,
    PausePlayMode,
    StopPlayMode,
    SelectEntity {
        stable_id: String,
    },
    FocusEntity {
        stable_id: String,
    },
    Ping {
        token: String,
    },
    Pong {
        token: String,
    },
    Status {
        status: PreviewStatus,
    },
    Diagnostic {
        level: DiagnosticLevel,
        message: String,
    },
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PreviewEnvelope {
    pub protocol: u32,
    pub seq: u64,
    pub channel: PreviewChannel,
    #[serde(flatten)]
    pub payload: PreviewPayload,
}

impl PreviewEnvelope {
    pub const PROTOCOL_V1: u32 = 1;

    pub fn new(seq: u64, channel: PreviewChannel, payload: PreviewPayload) -> Self {
        Self {
            protocol: Self::PROTOCOL_V1,
            seq,
            channel,
            payload,
        }
    }

    pub fn to_json_line(&self) -> Result<String, serde_json::Error> {
        let mut line = serde_json::to_string(self)?;
        line.push('\n');
        Ok(line)
    }

    pub fn from_json_line(line: &str) -> Result<Self, serde_json::Error> {
        serde_json::from_str(line.trim())
    }

    pub fn summary(&self) -> String {
        match &self.payload {
            PreviewPayload::Hello { role, project_path } => {
                format!("hello role={role:?} project={project_path:?}")
            }
            PreviewPayload::OpenProject { path } => format!("open_project path={path}"),
            PreviewPayload::OpenScene { path } => format!("open_scene path={path}"),
            PreviewPayload::EnterPlayMode => "enter_play_mode".to_owned(),
            PreviewPayload::PausePlayMode => "pause_play_mode".to_owned(),
            PreviewPayload::StopPlayMode => "stop_play_mode".to_owned(),
            PreviewPayload::SelectEntity { stable_id } => {
                format!("select_entity stable_id={stable_id}")
            }
            PreviewPayload::FocusEntity { stable_id } => {
                format!("focus_entity stable_id={stable_id}")
            }
            PreviewPayload::Ping { token } => format!("ping token={token}"),
            PreviewPayload::Pong { token } => format!("pong token={token}"),
            PreviewPayload::Status { status } => format!("status {status:?}"),
            PreviewPayload::Diagnostic { level, message } => {
                format!("diagnostic {level:?}: {message}")
            }
        }
    }
}
