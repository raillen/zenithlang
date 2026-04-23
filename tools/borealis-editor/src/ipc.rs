use crate::messages::{PeerRole, PreviewChannel, PreviewEnvelope, PreviewPayload, PreviewStatus};

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
    incoming: Vec<PreviewEnvelope>,
}

impl PreviewBridge {
    pub fn new() -> Self {
        Self {
            transport: TransportKind::JsonlStdio,
            state: ConnectionState::Disconnected,
            next_seq: 1,
            current_status: PreviewStatus::Idle,
            outgoing_lines: Vec::new(),
            incoming: Vec::new(),
        }
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

    pub fn start_preview(&mut self, project_path: Option<String>) -> Result<String, String> {
        self.state = ConnectionState::Starting;
        self.current_status = PreviewStatus::Starting;

        let hello = self.send(
            PreviewChannel::Command,
            PreviewPayload::Hello {
                role: PeerRole::Editor,
                project_path,
            },
        )?;

        let hello_seq = self.alloc_seq();
        self.mock_receive(PreviewEnvelope::new(
            hello_seq,
            PreviewChannel::Response,
            PreviewPayload::Hello {
                role: PeerRole::Preview,
                project_path: None,
            },
        ));
        let status_seq = self.alloc_seq();
        self.mock_receive(PreviewEnvelope::new(
            status_seq,
            PreviewChannel::Event,
            PreviewPayload::Status {
                status: PreviewStatus::Ready,
            },
        ));

        Ok(hello)
    }

    pub fn stop_preview(&mut self) -> Result<String, String> {
        let line = self.send(PreviewChannel::Command, PreviewPayload::StopPlayMode)?;
        self.current_status = PreviewStatus::Stopped;
        self.state = ConnectionState::Disconnected;
        Ok(line)
    }

    pub fn open_scene(&mut self, path: String) -> Result<String, String> {
        self.send(PreviewChannel::Command, PreviewPayload::OpenScene { path })
    }

    pub fn enter_play_mode(&mut self) -> Result<String, String> {
        self.current_status = PreviewStatus::Playing;
        self.send(PreviewChannel::Command, PreviewPayload::EnterPlayMode)
    }

    pub fn select_entity(&mut self, stable_id: String) -> Result<String, String> {
        self.send(
            PreviewChannel::Command,
            PreviewPayload::SelectEntity { stable_id },
        )
    }

    pub fn ping(&mut self, token: String) -> Result<String, String> {
        let line = self.send(
            PreviewChannel::Heartbeat,
            PreviewPayload::Ping {
                token: token.clone(),
            },
        )?;
        let pong_seq = self.alloc_seq();
        self.mock_receive(PreviewEnvelope::new(
            pong_seq,
            PreviewChannel::Response,
            PreviewPayload::Pong { token },
        ));
        Ok(line)
    }

    pub fn drain_incoming(&mut self) -> Vec<PreviewEnvelope> {
        let messages = self.incoming.clone();
        self.incoming.clear();

        for message in &messages {
            if let PreviewPayload::Status { status } = &message.payload {
                self.current_status = *status;
                if matches!(status, PreviewStatus::Ready | PreviewStatus::Playing) {
                    self.state = ConnectionState::Connected;
                }
            }
        }

        messages
    }

    pub fn drain_outgoing_lines(&mut self) -> Vec<String> {
        let lines = self.outgoing_lines.clone();
        self.outgoing_lines.clear();
        lines
    }

    fn send(&mut self, channel: PreviewChannel, payload: PreviewPayload) -> Result<String, String> {
        let envelope = PreviewEnvelope::new(self.alloc_seq(), channel, payload);
        let line = envelope
            .to_json_line()
            .map_err(|error| format!("failed to encode preview message: {error}"))?;
        self.outgoing_lines.push(line.clone());
        Ok(line)
    }

    fn mock_receive(&mut self, message: PreviewEnvelope) {
        let line = message
            .to_json_line()
            .expect("mock preview messages should always encode");
        let parsed = PreviewEnvelope::from_json_line(&line)
            .expect("mock preview messages should always decode");
        self.incoming.push(parsed);
    }

    fn alloc_seq(&mut self) -> u64 {
        let seq = self.next_seq;
        self.next_seq += 1;
        seq
    }
}
