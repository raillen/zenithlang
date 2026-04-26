use crate::models::asset::ProjectAsset;
use crate::models::console::ConsoleLine;
use crate::models::history::HistoryStack;
use crate::models::preview::PreviewSession;
use crate::models::project::StudioLayout;
use crate::models::scene::SceneDocument;
use crate::models::script::ScriptDocument;
use crate::models::selection::SelectionState;

pub struct AppState {
    pub layout: StudioLayout,
    pub project: Option<ProjectState>,
    pub preview: Option<PreviewSession>,
}

pub struct ProjectState {
    pub name: String,
    pub path: String,
    pub root: String,
    pub scene: SceneDocument,
    pub assets: Vec<ProjectAsset>,
    pub scripts: Vec<ScriptDocument>,
    pub selection: SelectionState,
    pub history: HistoryStack,
    pub console: Vec<ConsoleLine>,
    pub dirty: bool,
}

impl AppState {
    pub fn new(layout: StudioLayout) -> Self {
        Self {
            layout,
            project: None,
            preview: None,
        }
    }
}
