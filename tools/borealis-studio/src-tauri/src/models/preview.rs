use serde::{Deserialize, Serialize};
use std::process::{Child, ChildStdin};
use std::sync::mpsc::Receiver;

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
    Error,
    Exited,
}

impl PreviewStatus {
    pub fn as_str(&self) -> &'static str {
        match self {
            Self::Idle => "idle",
            Self::Starting => "starting",
            Self::Loading => "loading",
            Self::Ready => "ready",
            Self::Playing => "playing",
            Self::Paused => "paused",
            Self::Stopped => "stopped",
            Self::Error => "error",
            Self::Exited => "exited",
        }
    }

    pub fn from_str(s: &str) -> Self {
        match s {
            "idle" => Self::Idle,
            "starting" => Self::Starting,
            "loading" => Self::Loading,
            "ready" => Self::Ready,
            "playing" => Self::Playing,
            "paused" => Self::Paused,
            "stopped" => Self::Stopped,
            "error" => Self::Error,
            "exited" => Self::Exited,
            _ => Self::Idle,
        }
    }
}

impl std::fmt::Display for PreviewStatus {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(self.as_str())
    }
}

pub struct PreviewSession {
    pub child: Child,
    pub stdin: ChildStdin,
    pub receiver: Receiver<String>,
    pub runner: String,
    pub seq: u64,
    pub status: String,
    pub machine: PreviewStatus,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct PreviewCommandResult {
    pub status: String,
    pub runner: Option<String>,
    pub events: Vec<PreviewEvent>,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct PreviewEvent {
    pub raw: String,
    pub channel: Option<String>,
    pub kind: Option<String>,
    pub status: Option<String>,
    pub message: Option<String>,
    pub loaded: Option<bool>,
    pub entity_count: Option<i64>,
    pub camera_count: Option<i64>,
    pub light_count: Option<i64>,
    pub audio_count: Option<i64>,
}
