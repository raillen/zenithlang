use serde::Serialize;
use std::process::{Child, ChildStdin};
use std::sync::mpsc::Receiver;

pub struct PreviewSession {
    pub child: Child,
    pub stdin: ChildStdin,
    pub receiver: Receiver<String>,
    pub runner: String,
    pub seq: u64,
    pub status: String,
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
