use serde::Serialize;

use super::scene::SceneDocument;

pub trait EditCommand: Send + Sync {
    fn execute(&self, scene: &mut SceneDocument) -> Result<(), String>;
    fn undo(&self, scene: &mut SceneDocument) -> Result<(), String>;
    fn description(&self) -> String;
}

pub struct HistoryStack {
    undo_stack: Vec<Box<dyn EditCommand>>,
    redo_stack: Vec<Box<dyn EditCommand>>,
    max_size: usize,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct HistoryState {
    pub can_undo: bool,
    pub can_redo: bool,
    pub undo_description: String,
    pub redo_description: String,
}

impl Default for HistoryStack {
    fn default() -> Self {
        Self {
            undo_stack: Vec::new(),
            redo_stack: Vec::new(),
            max_size: 100,
        }
    }
}

impl HistoryStack {
    pub fn push(&mut self, command: Box<dyn EditCommand>) {
        self.redo_stack.clear();
        self.undo_stack.push(command);
        if self.undo_stack.len() > self.max_size {
            self.undo_stack.remove(0);
        }
    }

    pub fn undo(&mut self, scene: &mut SceneDocument) -> Result<(), String> {
        let command = self
            .undo_stack
            .pop()
            .ok_or_else(|| "nothing to undo".to_string())?;
        command.undo(scene)?;
        self.redo_stack.push(command);
        Ok(())
    }

    pub fn redo(&mut self, scene: &mut SceneDocument) -> Result<(), String> {
        let command = self
            .redo_stack
            .pop()
            .ok_or_else(|| "nothing to redo".to_string())?;
        command.execute(scene)?;
        self.undo_stack.push(command);
        Ok(())
    }

    pub fn state(&self) -> HistoryState {
        HistoryState {
            can_undo: !self.undo_stack.is_empty(),
            can_redo: !self.redo_stack.is_empty(),
            undo_description: self
                .undo_stack
                .last()
                .map(|cmd| cmd.description())
                .unwrap_or_default(),
            redo_description: self
                .redo_stack
                .last()
                .map(|cmd| cmd.description())
                .unwrap_or_default(),
        }
    }

    pub fn clear(&mut self) {
        self.undo_stack.clear();
        self.redo_stack.clear();
    }
}
