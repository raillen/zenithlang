use serde::Serialize;
use std::collections::HashSet;

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct SelectionState {
    pub selected_ids: HashSet<String>,
    pub primary_id: Option<String>,
    pub hover_id: Option<String>,
}

impl Default for SelectionState {
    fn default() -> Self {
        Self {
            selected_ids: HashSet::new(),
            primary_id: None,
            hover_id: None,
        }
    }
}

impl SelectionState {
    pub fn select_single(&mut self, id: String) {
        self.selected_ids.clear();
        self.selected_ids.insert(id.clone());
        self.primary_id = Some(id);
    }

    pub fn toggle(&mut self, id: String) {
        if self.selected_ids.contains(&id) {
            self.selected_ids.remove(&id);
            if self.primary_id.as_deref() == Some(&id) {
                self.primary_id = self.selected_ids.iter().next().cloned();
            }
        } else {
            self.selected_ids.insert(id.clone());
            self.primary_id = Some(id);
        }
    }

    pub fn clear(&mut self) {
        self.selected_ids.clear();
        self.primary_id = None;
        self.hover_id = None;
    }

    pub fn is_selected(&self, id: &str) -> bool {
        self.selected_ids.contains(id)
    }
}
