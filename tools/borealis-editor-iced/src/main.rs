mod modern_theme;
#[allow(dead_code)]
#[path = "../../borealis-editor/src/project.rs"]
mod project;
#[allow(dead_code)]
#[path = "../../borealis-editor/src/scene_document.rs"]
mod scene_document;

use iced::widget::canvas::{
    self, Action, Canvas, Event as CanvasEvent, Frame, Path as CanvasPath, Stroke,
};
use iced::widget::scrollable::{Direction as ScrollDirection, Scrollbar};
use iced::widget::{button, column, container, row, rule, scrollable, text, text_input, Space};
use iced::{
    application, border, Color, Element, Font, Length, Padding, Pixels, Point, Rectangle, Renderer,
    Size, Subscription, Task, Theme, Vector,
};
use iced::{keyboard, mouse};
use iced_aw::{number_input, TabBar, TabLabel, ICED_AW_FONT_BYTES};
use iced_code_editor::{theme as code_theme, CodeEditor, Message as CodeEditorMessage};
use lucide_icons::Icon;
use modern_theme::{ButtonKind, ContainerKind, Modern};
use project::{
    load_project_summary, ProjectSummary, ProjectTree, ProjectTreeItem, ProjectTreeItemKind,
};
use scene_document::{
    load_scene_document, save_scene_document, SceneDocument, SceneEntity, Transform2D,
};
use std::path::{Path as StdPath, PathBuf};

const DEFAULT_PROJECT: &str = "packages/borealis/zenith.ztproj";
const DEFAULT_SCRIPT: &str = "packages/borealis/src/borealis/game/scene.zt";
const SNAP_SIZE: f32 = 16.0;
const VIEWPORT_FRAME_WIDTH: f32 = 900.0;
const VIEWPORT_FRAME_HEIGHT: f32 = 560.0;
const CONSOLE_LIMIT: usize = 160;
const BOTTOM_PANEL_HEIGHT: f32 = 210.0;

fn main() -> iced::Result {
    tracing_subscriber::fmt()
        .with_env_filter(
            std::env::var("BOREALIS_EDITOR_LOG")
                .unwrap_or_else(|_| "info,borealis_editor_iced=debug".to_owned()),
        )
        .with_target(false)
        .compact()
        .init();

    application(
        BorealisIcedApp::new,
        BorealisIcedApp::update,
        BorealisIcedApp::view,
    )
    .title("Borealis Editor Iced")
    .theme(BorealisIcedApp::theme)
    .subscription(BorealisIcedApp::subscription)
    .font(ICED_AW_FONT_BYTES)
    .font(lucide_icons::LUCIDE_FONT_BYTES)
    .window_size(Size::new(1440.0, 900.0))
    .run()
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum WorkspaceTab {
    Scene,
    Scene3D,
    Script,
    Terminal,
    Console,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum TransformNumberField {
    PosX,
    PosY,
    PosZ,
    RotationX,
    RotationY,
    RotationZ,
    ScaleX,
    ScaleY,
    ScaleZ,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum ViewportTool {
    Select,
    Move,
    Rotate,
    Scale,
}

impl ViewportTool {
    fn label(self) -> &'static str {
        match self {
            Self::Select => "Select",
            Self::Move => "Move",
            Self::Rotate => "Rotate",
            Self::Scale => "Scale",
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum ViewPreset {
    Front,
    Right,
    Top,
    Reset,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum MainMenu {
    File,
    Edit,
    View,
    Window,
    Help,
}

impl MainMenu {
    fn label(self) -> &'static str {
        match self {
            Self::File => "Arquivo",
            Self::Edit => "Editar",
            Self::View => "Exibir",
            Self::Window => "Janela",
            Self::Help => "Ajuda",
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum GizmoHandle {
    MoveFree,
    MoveX,
    MoveY,
    MoveZ,
    RotateX,
    RotateY,
    RotateZ,
    ScaleUniform,
    ScaleX,
    ScaleY,
    ScaleZ,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum ProjectViewMode {
    List,
    Grid,
    Details,
}

impl ProjectViewMode {
    fn label(self) -> &'static str {
        match self {
            Self::List => "List",
            Self::Grid => "Grid",
            Self::Details => "Details",
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum OpenFileKind {
    Text,
    AssetPreview,
}

#[derive(Debug, Clone)]
struct OpenFile {
    path: String,
    label: String,
    kind: OpenFileKind,
    content: String,
    dirty: bool,
}

impl OpenFile {
    fn new(path: String, content: String, kind: OpenFileKind) -> Self {
        let label = StdPath::new(&path)
            .file_name()
            .and_then(|value| value.to_str())
            .unwrap_or(&path)
            .to_string();

        Self {
            path,
            label,
            kind,
            content,
            dirty: false,
        }
    }
}

#[derive(Debug, Clone)]
enum Message {
    MainMenuSelected(MainMenu),
    WorkspaceTabSelected(WorkspaceTab),
    ProjectItemSelected(String),
    ProjectViewModeSelected(ProjectViewMode),
    FileTabSelected(String),
    CloseFileTab(String),
    EntitySelected(String),
    CreateEntity,
    DuplicateEntity,
    DeleteSelectedEntity,
    SaveScene,
    SaveScript,
    SaveAll,
    RunProjectCheck,
    ProjectCheckFinished(CheckReport),
    RunEditorCommand(EditorCommand),
    ScriptEditorEvent(CodeEditorMessage),
    KeyboardEvent(keyboard::Event),
    #[cfg(feature = "terminal")]
    TerminalEvent(iced_term::Event),
    FocusSelectedEntity,
    FrameScene,
    ViewportToolSelected(ViewportTool),
    ToggleSnap,
    ToggleGrid,
    ToggleGizmos,
    ToggleViewCube,
    ToggleSettingsMenu,
    GizmoSizeChanged(f32),
    ViewPresetSelected(ViewPreset),
    Noop,
    EntityNameChanged(String),
    EntityLayerChanged(String),
    EntityTagsChanged(String),
    EntityModelAssetChanged(String),
    ComponentChanged(usize, String),
    ComponentDraftChanged(String),
    AddComponent,
    RemoveComponent(usize),
    TransformNumberChanged(TransformNumberField, f32),
    ViewportPanned {
        dx: f32,
        dy: f32,
    },
    ViewportZoomed {
        delta: f32,
    },
    Viewport3DOrbited {
        dx: f32,
        dy: f32,
    },
    ViewportGizmoDragged {
        stable_id: String,
        handle: GizmoHandle,
        dx: f32,
        dy: f32,
    },
    ViewportEntityDragged {
        stable_id: String,
        dx: f32,
        dy: f32,
    },
}

#[derive(Debug, Clone, Copy)]
enum EditorCommand {
    Check,
    Build,
    Run,
    Summary,
}

impl EditorCommand {
    fn label(self) -> &'static str {
        match self {
            Self::Check => "check",
            Self::Build => "build",
            Self::Run => "run",
            Self::Summary => "summary",
        }
    }

    fn zt_args(self) -> &'static [&'static str] {
        match self {
            Self::Check => &["check"],
            Self::Build => &["build"],
            Self::Run => &["run"],
            Self::Summary => &["summary"],
        }
    }
}

#[derive(Debug, Clone)]
struct CheckReport {
    ok: bool,
    command: String,
    output: String,
}

struct BorealisIcedApp {
    project_path: String,
    project: Option<ProjectSummary>,
    project_tree: ProjectTree,
    scene_path: String,
    scene: Option<SceneDocument>,
    selected_entity_id: String,
    active_tab: WorkspaceTab,
    viewport_pan: Vector,
    viewport_zoom: f32,
    viewport_3d_yaw: f32,
    viewport_3d_pitch: f32,
    active_viewport_tool: ViewportTool,
    snap_enabled: bool,
    grid_enabled: bool,
    gizmos_enabled: bool,
    view_cube_enabled: bool,
    settings_menu_open: bool,
    active_main_menu: Option<MainMenu>,
    gizmo_size: f32,
    status: String,
    console: Vec<String>,
    script_path: String,
    script_editor: CodeEditor,
    script_dirty: bool,
    open_files: Vec<OpenFile>,
    active_file_path: String,
    ignore_next_editor_dirty: bool,
    bottom_view_mode: ProjectViewMode,
    scene_dirty: bool,
    component_draft: String,
    #[cfg(feature = "terminal")]
    terminal: Option<iced_term::Terminal>,
}

impl BorealisIcedApp {
    fn new() -> Self {
        let project_path = workspace_path(DEFAULT_PROJECT);
        let (script_path, script_source) = load_script_document();
        let mut script_editor = CodeEditor::new(&script_source, "rs")
            .with_line_numbers_enabled(true)
            .with_wrap_enabled(false);
        script_editor.set_theme(code_theme::from_iced_theme(&Theme::TokyoNightStorm));
        #[cfg(feature = "terminal")]
        let (terminal, terminal_status) = create_terminal();

        let script_path = normalize_path(&script_path);
        let open_files = vec![OpenFile::new(
            script_path.clone(),
            script_source.clone(),
            OpenFileKind::Text,
        )];

        let mut app = Self {
            project_path: normalize_path(&project_path),
            project: None,
            project_tree: ProjectTree::default(),
            scene_path: String::new(),
            scene: None,
            selected_entity_id: String::new(),
            active_tab: WorkspaceTab::Scene,
            viewport_pan: Vector::new(0.0, 0.0),
            viewport_zoom: 1.0,
            viewport_3d_yaw: 0.0,
            viewport_3d_pitch: 0.0,
            active_viewport_tool: ViewportTool::Select,
            snap_enabled: true,
            grid_enabled: true,
            gizmos_enabled: true,
            view_cube_enabled: true,
            settings_menu_open: false,
            active_main_menu: None,
            gizmo_size: 1.0,
            status: "Loading project".to_string(),
            console: Vec::new(),
            script_path: script_path.clone(),
            script_editor,
            script_dirty: false,
            open_files,
            active_file_path: script_path,
            ignore_next_editor_dirty: false,
            bottom_view_mode: ProjectViewMode::Grid,
            scene_dirty: false,
            component_draft: String::new(),
            #[cfg(feature = "terminal")]
            terminal,
        };

        #[cfg(feature = "terminal")]
        if let Some(message) = terminal_status {
            app.push_console(message);
        }

        app.load_project();
        app
    }

    fn theme(&self) -> Theme {
        Modern::light_theme()
    }

    fn subscription(&self) -> Subscription<Message> {
        let keyboard = keyboard::listen().map(Message::KeyboardEvent);

        #[cfg(feature = "terminal")]
        {
            let terminal = self
                .terminal
                .as_ref()
                .map(|terminal| terminal.subscription().map(Message::TerminalEvent))
                .unwrap_or_else(Subscription::none);

            Subscription::batch([keyboard, terminal])
        }

        #[cfg(not(feature = "terminal"))]
        {
            keyboard
        }
    }

    fn update(&mut self, message: Message) -> Task<Message> {
        match message {
            Message::MainMenuSelected(menu) => {
                self.active_main_menu = if self.active_main_menu == Some(menu) {
                    None
                } else {
                    Some(menu)
                };
            }
            Message::WorkspaceTabSelected(tab) => {
                self.active_tab = tab;
                self.active_main_menu = None;
            }
            Message::ProjectItemSelected(id) => {
                return self.open_project_item(&id);
            }
            Message::ProjectViewModeSelected(mode) => {
                self.bottom_view_mode = mode;
            }
            Message::FileTabSelected(path) => {
                return self.activate_file_tab(&path);
            }
            Message::CloseFileTab(path) => {
                return self.close_file_tab(&path);
            }
            Message::EntitySelected(stable_id) => {
                self.selected_entity_id = stable_id;
            }
            Message::CreateEntity => {
                self.create_entity();
            }
            Message::DuplicateEntity => {
                self.duplicate_selected_entity();
            }
            Message::DeleteSelectedEntity => {
                self.delete_selected_entity();
            }
            Message::SaveScene => match &self.scene {
                Some(scene) => match save_scene_document(scene) {
                    Ok(()) => {
                        self.scene_dirty = false;
                        self.push_console("Scene saved");
                    }
                    Err(error) => self.push_console(format!("Save failed: {error}")),
                },
                None => self.push_console("No scene loaded"),
            },
            Message::SaveScript => {
                self.save_script_to_disk();
            }
            Message::SaveAll => {
                self.save_all_open_files();
                if let Some(scene) = &self.scene {
                    match save_scene_document(scene) {
                        Ok(()) => {
                            self.scene_dirty = false;
                            self.push_console("Scene saved");
                        }
                        Err(error) => self.push_console(format!("Save failed: {error}")),
                    }
                }
            }
            Message::RunProjectCheck => {
                if self.save_script_to_disk() {
                    self.active_tab = WorkspaceTab::Console;
                    self.push_console("Running project check");
                    return Task::perform(
                        run_project_command(EditorCommand::Check, self.project_path.clone()),
                        Message::ProjectCheckFinished,
                    );
                }
            }
            Message::RunEditorCommand(command) => {
                if matches!(
                    command,
                    EditorCommand::Check | EditorCommand::Build | EditorCommand::Run
                ) {
                    self.save_script_to_disk();
                }
                self.active_tab = WorkspaceTab::Console;
                self.push_console(format!("Running {}", command.label()));
                return Task::perform(
                    run_project_command(command, self.project_path.clone()),
                    Message::ProjectCheckFinished,
                );
            }
            Message::ProjectCheckFinished(report) => {
                self.active_tab = WorkspaceTab::Console;
                if report.ok {
                    self.push_console(format!("Check passed: {}", report.command));
                } else {
                    self.push_console(format!("Check failed: {}", report.command));
                }

                for line in tail_lines(&report.output, 24) {
                    self.push_console(line);
                }
            }
            Message::ScriptEditorEvent(event) => {
                let task = self
                    .script_editor
                    .update(&event)
                    .map(Message::ScriptEditorEvent);
                if self.ignore_next_editor_dirty {
                    self.ignore_next_editor_dirty = false;
                } else {
                    self.mark_active_file_dirty();
                }
                return task;
            }
            Message::KeyboardEvent(event) => {
                self.handle_keyboard_event(event);
            }
            #[cfg(feature = "terminal")]
            Message::TerminalEvent(event) => {
                self.handle_terminal_event(event);
            }
            Message::FocusSelectedEntity => {
                self.focus_selected_entity();
            }
            Message::FrameScene => {
                self.frame_scene();
            }
            Message::ViewportToolSelected(tool) => {
                self.active_viewport_tool = tool;
                self.push_console(format!("Viewport tool: {}", tool.label()));
            }
            Message::ToggleSnap => {
                self.snap_enabled = !self.snap_enabled;
            }
            Message::ToggleGrid => {
                self.grid_enabled = !self.grid_enabled;
            }
            Message::ToggleGizmos => {
                self.gizmos_enabled = !self.gizmos_enabled;
            }
            Message::ToggleViewCube => {
                self.view_cube_enabled = !self.view_cube_enabled;
            }
            Message::ToggleSettingsMenu => {
                self.settings_menu_open = !self.settings_menu_open;
            }
            Message::GizmoSizeChanged(value) => {
                self.gizmo_size = value.clamp(0.6, 2.0);
            }
            Message::ViewPresetSelected(preset) => {
                self.apply_view_preset(preset);
            }
            Message::Noop => {}
            Message::EntityNameChanged(value) => {
                if let Some(entity) = self.selected_entity_mut() {
                    entity.name = value;
                    self.scene_dirty = true;
                }
            }
            Message::EntityLayerChanged(value) => {
                if let Some(entity) = self.selected_entity_mut() {
                    entity.layer = value;
                    self.scene_dirty = true;
                }
            }
            Message::EntityTagsChanged(value) => {
                if let Some(entity) = self.selected_entity_mut() {
                    entity.tags = value
                        .split(',')
                        .map(str::trim)
                        .filter(|tag| !tag.is_empty())
                        .map(str::to_string)
                        .collect();
                    self.scene_dirty = true;
                }
            }
            Message::EntityModelAssetChanged(value) => {
                if let Some(entity) = self.selected_entity_mut() {
                    entity.model_asset = value;
                    self.scene_dirty = true;
                }
            }
            Message::ComponentChanged(index, value) => {
                if let Some(entity) = self.selected_entity_mut() {
                    if let Some(component) = entity.components.get_mut(index) {
                        *component = value.trim().to_string();
                        self.scene_dirty = true;
                    }
                }
            }
            Message::ComponentDraftChanged(value) => {
                self.component_draft = value;
            }
            Message::AddComponent => {
                let component = self.component_draft.trim().to_string();
                if !component.is_empty() {
                    if let Some(entity) = self.selected_entity_mut() {
                        entity.components.push(component);
                        self.component_draft.clear();
                        self.scene_dirty = true;
                    }
                }
            }
            Message::RemoveComponent(index) => {
                if let Some(entity) = self.selected_entity_mut() {
                    if index < entity.components.len() {
                        entity.components.remove(index);
                        self.scene_dirty = true;
                    }
                }
            }
            Message::TransformNumberChanged(field, value) => {
                if let Some(entity) = self.selected_entity_mut() {
                    match field {
                        TransformNumberField::PosX => entity.transform.x = value,
                        TransformNumberField::PosY => entity.transform.y = value,
                        TransformNumberField::PosZ => entity.transform.z = value,
                        TransformNumberField::RotationX => entity.transform.rotation_x = value,
                        TransformNumberField::RotationY => entity.transform.rotation_y = value,
                        TransformNumberField::RotationZ => entity.transform.rotation = value,
                        TransformNumberField::ScaleX => entity.transform.scale_x = value.max(0.01),
                        TransformNumberField::ScaleY => entity.transform.scale_y = value.max(0.01),
                        TransformNumberField::ScaleZ => entity.transform.scale_z = value.max(0.01),
                    }
                    self.scene_dirty = true;
                }
            }
            Message::ViewportPanned { dx, dy } => {
                self.viewport_pan = self.viewport_pan + Vector::new(dx, dy);
            }
            Message::ViewportZoomed { delta } => {
                let factor = if delta > 0.0 { 1.1 } else { 0.9 };
                self.viewport_zoom = (self.viewport_zoom * factor).clamp(0.35, 3.0);
            }
            Message::Viewport3DOrbited { dx, dy } => {
                self.viewport_3d_yaw += dx * 0.01;
                self.viewport_3d_pitch = (self.viewport_3d_pitch + dy * 0.006).clamp(-0.8, 0.8);
            }
            Message::ViewportGizmoDragged {
                stable_id,
                handle,
                dx,
                dy,
            } => {
                self.apply_gizmo_drag(&stable_id, handle, dx, dy);
            }
            Message::ViewportEntityDragged { stable_id, dx, dy } => {
                if let Some(scene) = &mut self.scene {
                    if let Some(entity) = scene
                        .entities
                        .iter_mut()
                        .find(|entity| entity.stable_id == stable_id)
                    {
                        entity.transform.x += dx;
                        entity.transform.y += dy;
                        if self.snap_enabled {
                            entity.transform.x = snap_to_grid(entity.transform.x);
                            entity.transform.y = snap_to_grid(entity.transform.y);
                        }
                        self.scene_dirty = true;
                    }
                }
            }
        }

        Task::none()
    }

    fn view(&self) -> Element<'_, Message> {
        let top_bar = self.top_bar();
        let body = row![
            self.left_panel(),
            rule::vertical(1),
            self.center_panel(),
            rule::vertical(1),
            self.inspector_panel(),
        ]
        .height(Length::Fill);

        container(
            column![
                self.menu_bar(),
                self.menu_panel(),
                rule::horizontal(1),
                top_bar,
                rule::horizontal(1),
                body,
            ]
            .width(Length::Fill)
            .height(Length::Fill),
        )
        .style(Modern::container(ContainerKind::App))
        .width(Length::Fill)
        .height(Length::Fill)
        .into()
    }

    fn load_project(&mut self) {
        match load_project_summary(&self.project_path) {
            Ok(project) => {
                self.project_tree = ProjectTree::from_summary(&project);
                self.status = format!("Project {}", project.name);
                self.project = Some(project);

                let first_scene = self
                    .project
                    .as_ref()
                    .and_then(|project| project.scene_candidates.first())
                    .cloned();
                if let Some(scene_path) = first_scene {
                    self.load_scene(&scene_path);
                }
            }
            Err(error) => {
                self.status = "Project load failed".to_string();
                self.push_console(format!("Project load failed: {error}"));
            }
        }
    }

    fn load_scene(&mut self, scene_path: &str) {
        let Some(project) = &self.project else {
            self.push_console("Project must load before a scene");
            return;
        };

        match load_scene_document(project, scene_path) {
            Ok(scene) => {
                self.scene_path = scene_path.to_string();
                self.selected_entity_id = scene
                    .entities
                    .first()
                    .map(|entity| entity.stable_id.clone())
                    .unwrap_or_default();
                self.status = format!("Scene {}", scene.name);
                self.push_console(format!("Opened scene {scene_path}"));
                self.scene = Some(scene);
                self.scene_dirty = false;
            }
            Err(error) => {
                self.push_console(format!("Scene load failed: {error}"));
            }
        }
    }

    fn open_project_item(&mut self, id: &str) -> Task<Message> {
        let Some(item) = self.project_tree.find_item(id) else {
            self.push_console(format!("Project item not found: {id}"));
            return Task::none();
        };

        match item.kind {
            project::ProjectTreeItemKind::SceneCandidate => {
                self.load_scene(&item.path);
                self.active_tab = WorkspaceTab::Scene;
                Task::none()
            }
            project::ProjectTreeItemKind::Module | project::ProjectTreeItemKind::Example => {
                self.open_script_path(&item.path)
            }
            project::ProjectTreeItemKind::Asset => {
                if item.path.ends_with(".json") && item.path.contains("scene") {
                    self.load_scene(&item.path);
                    self.active_tab = WorkspaceTab::Scene;
                    Task::none()
                } else {
                    self.open_script_path(&item.path)
                }
            }
        }
    }

    fn open_script_path(&mut self, path: &str) -> Task<Message> {
        let Some(project) = &self.project else {
            self.push_console("Project must load before opening a file");
            return Task::none();
        };

        let resolved = resolve_project_file(project, path);
        let resolved_path = normalize_path(&resolved);

        if self
            .open_files
            .iter()
            .any(|file| file.path == resolved_path)
        {
            return self.activate_file_tab(&resolved_path);
        }

        let kind = classify_open_file(&resolved);
        let content = match std::fs::read_to_string(&resolved) {
            Ok(source) => source,
            Err(error) if kind == OpenFileKind::AssetPreview => {
                asset_preview_text(&resolved_path, &error.to_string())
            }
            Err(error) => {
                self.push_console(format!("Open failed: {error}"));
                return Task::none();
            }
        };

        self.sync_active_file_from_editor();
        self.open_files
            .push(OpenFile::new(resolved_path.clone(), content, kind));
        self.push_console(format!("Opened file {resolved_path}"));
        self.activate_file_tab(&resolved_path)
    }

    fn selected_entity(&self) -> Option<&SceneEntity> {
        let scene = self.scene.as_ref()?;
        scene
            .entities
            .iter()
            .find(|entity| entity.stable_id == self.selected_entity_id)
    }

    fn selected_entity_mut(&mut self) -> Option<&mut SceneEntity> {
        let scene = self.scene.as_mut()?;
        scene
            .entities
            .iter_mut()
            .find(|entity| entity.stable_id == self.selected_entity_id)
    }

    fn push_console(&mut self, message: impl Into<String>) {
        self.console.push(message.into());
        if self.console.len() > CONSOLE_LIMIT {
            self.console.remove(0);
        }
    }

    fn save_script_to_disk(&mut self) -> bool {
        self.sync_active_file_from_editor();

        let Some(index) = self.active_file_index() else {
            self.push_console("No open file to save");
            return false;
        };

        if self.open_files[index].kind == OpenFileKind::AssetPreview {
            self.push_console("Asset preview is read-only");
            return true;
        }

        let path = self.open_files[index].path.clone();
        let content = self.open_files[index].content.clone();

        match std::fs::write(&path, content) {
            Ok(()) => {
                self.script_editor.mark_saved();
                self.script_dirty = false;
                if let Some(file) = self.open_files.get_mut(index) {
                    file.dirty = false;
                }
                self.push_console(format!("File saved: {path}"));
                true
            }
            Err(error) => {
                self.push_console(format!("File save failed: {error}"));
                false
            }
        }
    }

    fn save_all_open_files(&mut self) -> bool {
        self.sync_active_file_from_editor();

        let mut ok = true;
        let mut saved_count = 0usize;
        for file in &mut self.open_files {
            if !file.dirty || file.kind == OpenFileKind::AssetPreview {
                continue;
            }

            match std::fs::write(&file.path, &file.content) {
                Ok(()) => {
                    file.dirty = false;
                    saved_count += 1;
                }
                Err(error) => {
                    ok = false;
                    self.console
                        .push(format!("File save failed: {}: {error}", file.path));
                }
            }
        }

        if let Some(index) = self.active_file_index() {
            self.script_dirty = self.open_files[index].dirty;
            if !self.script_dirty {
                self.script_editor.mark_saved();
            }
        }

        if saved_count > 0 {
            self.push_console(format!("Saved {saved_count} open file(s)"));
        }

        ok
    }

    fn activate_file_tab(&mut self, path: &str) -> Task<Message> {
        self.sync_active_file_from_editor();

        let Some(index) = self.open_files.iter().position(|file| file.path == path) else {
            self.push_console(format!("File tab not found: {path}"));
            return Task::none();
        };

        let file = &self.open_files[index];
        self.active_file_path = file.path.clone();
        self.script_path = file.path.clone();
        self.script_dirty = file.dirty;
        self.active_tab = WorkspaceTab::Script;
        self.ignore_next_editor_dirty = true;

        self.script_editor
            .reset(&file.content)
            .map(Message::ScriptEditorEvent)
    }

    fn close_file_tab(&mut self, path: &str) -> Task<Message> {
        let Some(index) = self.open_files.iter().position(|file| file.path == path) else {
            return Task::none();
        };

        if self.open_files[index].dirty {
            self.push_console(format!(
                "Save or discard changes before closing {}",
                self.open_files[index].label
            ));
            return Task::none();
        }

        self.open_files.remove(index);

        if self.active_file_path == path {
            let next_path = self
                .open_files
                .get(
                    index
                        .saturating_sub(1)
                        .min(self.open_files.len().saturating_sub(1)),
                )
                .map(|file| file.path.clone());
            if let Some(next_path) = next_path {
                return self.activate_file_tab(&next_path);
            }

            self.active_file_path.clear();
            self.script_path.clear();
            self.script_dirty = false;
            self.ignore_next_editor_dirty = true;
            return self.script_editor.reset("").map(Message::ScriptEditorEvent);
        }

        Task::none()
    }

    fn active_file_index(&self) -> Option<usize> {
        self.open_files
            .iter()
            .position(|file| file.path == self.active_file_path)
    }

    fn sync_active_file_from_editor(&mut self) {
        let Some(index) = self.active_file_index() else {
            return;
        };

        if let Some(file) = self.open_files.get_mut(index) {
            file.content = self.script_editor.content();
            file.dirty = self.script_dirty;
        }
    }

    fn mark_active_file_dirty(&mut self) {
        self.script_dirty = true;
        if let Some(index) = self.active_file_index() {
            if let Some(file) = self.open_files.get_mut(index) {
                file.dirty = true;
            }
        }
    }

    fn has_dirty_files(&self) -> bool {
        self.open_files.iter().any(|file| file.dirty)
    }

    fn focus_selected_entity(&mut self) {
        let Some(entity) = self.selected_entity() else {
            self.push_console("Select an entity before Focus");
            return;
        };
        let entity_name = entity.name.clone();
        let entity_x = entity.transform.x;
        let entity_y = entity.transform.y;

        self.viewport_pan = Vector::new(
            VIEWPORT_FRAME_WIDTH * 0.5 - 80.0 - entity_x * self.viewport_zoom,
            VIEWPORT_FRAME_HEIGHT * 0.5 - 72.0 - entity_y * self.viewport_zoom,
        );
        self.push_console(format!("Focused {entity_name}"));
    }

    fn frame_scene(&mut self) {
        let Some(scene) = &self.scene else {
            self.push_console("No scene loaded");
            return;
        };

        let Some(bounds) = scene_bounds(scene) else {
            self.push_console("Scene has no entities");
            return;
        };

        let width = (bounds.2 - bounds.0).max(96.0);
        let height = (bounds.3 - bounds.1).max(96.0);
        self.viewport_zoom = (VIEWPORT_FRAME_WIDTH * 0.72 / width)
            .min(VIEWPORT_FRAME_HEIGHT * 0.72 / height)
            .clamp(0.35, 3.0);

        let center_x = (bounds.0 + bounds.2) * 0.5;
        let center_y = (bounds.1 + bounds.3) * 0.5;
        self.viewport_pan = Vector::new(
            VIEWPORT_FRAME_WIDTH * 0.5 - 80.0 - center_x * self.viewport_zoom,
            VIEWPORT_FRAME_HEIGHT * 0.5 - 72.0 - center_y * self.viewport_zoom,
        );
        self.push_console("Framed scene");
    }

    fn handle_keyboard_event(&mut self, event: keyboard::Event) {
        let keyboard::Event::KeyPressed {
            key,
            physical_key,
            repeat,
            ..
        } = event
        else {
            return;
        };

        if repeat || !matches!(self.active_tab, WorkspaceTab::Scene | WorkspaceTab::Scene3D) {
            return;
        }

        if let Some(key) = key
            .to_latin(physical_key)
            .map(|key| key.to_ascii_lowercase())
        {
            match key {
                'w' => self.active_viewport_tool = ViewportTool::Select,
                'g' => self.active_viewport_tool = ViewportTool::Move,
                'r' => self.active_viewport_tool = ViewportTool::Rotate,
                's' => self.active_viewport_tool = ViewportTool::Scale,
                'f' => self.frame_scene(),
                '1' => self.apply_view_preset(ViewPreset::Front),
                '3' => self.apply_view_preset(ViewPreset::Right),
                '5' => self.apply_view_preset(ViewPreset::Reset),
                '7' => self.apply_view_preset(ViewPreset::Top),
                _ => {}
            }
            return;
        }

        if matches!(key, keyboard::Key::Named(keyboard::key::Named::Escape)) {
            self.active_viewport_tool = ViewportTool::Select;
        }
    }

    fn apply_view_preset(&mut self, preset: ViewPreset) {
        match preset {
            ViewPreset::Front => {
                self.active_tab = WorkspaceTab::Scene3D;
                self.viewport_3d_yaw = 0.0;
                self.viewport_3d_pitch = 0.0;
            }
            ViewPreset::Right => {
                self.active_tab = WorkspaceTab::Scene3D;
                self.viewport_3d_yaw = std::f32::consts::FRAC_PI_2;
                self.viewport_3d_pitch = 0.0;
            }
            ViewPreset::Top => {
                self.active_tab = WorkspaceTab::Scene3D;
                self.viewport_3d_yaw = 0.0;
                self.viewport_3d_pitch = -0.8;
            }
            ViewPreset::Reset => {
                self.viewport_3d_yaw = 0.0;
                self.viewport_3d_pitch = 0.0;
                self.viewport_zoom = 1.0;
                self.viewport_pan = Vector::new(0.0, 0.0);
            }
        }
    }

    fn apply_gizmo_drag(&mut self, stable_id: &str, handle: GizmoHandle, dx: f32, dy: f32) {
        let snap_enabled = self.snap_enabled;
        let Some(scene) = &mut self.scene else {
            return;
        };
        let Some(entity) = scene
            .entities
            .iter_mut()
            .find(|entity| entity.stable_id == stable_id)
        else {
            return;
        };

        match handle {
            GizmoHandle::MoveFree => {
                entity.transform.x += dx;
                entity.transform.y += dy;
                if snap_enabled {
                    entity.transform.x = snap_to_grid(entity.transform.x);
                    entity.transform.y = snap_to_grid(entity.transform.y);
                }
            }
            GizmoHandle::MoveX => {
                entity.transform.x += dx;
                if snap_enabled {
                    entity.transform.x = snap_to_grid(entity.transform.x);
                }
            }
            GizmoHandle::MoveY => {
                entity.transform.y += dy;
                if snap_enabled {
                    entity.transform.y = snap_to_grid(entity.transform.y);
                }
            }
            GizmoHandle::MoveZ => {
                entity.transform.z = (entity.transform.z - dy).max(-100000.0);
                if snap_enabled {
                    entity.transform.z = snap_to_grid(entity.transform.z);
                }
            }
            GizmoHandle::RotateX => {
                entity.transform.rotation_x = normalize_degrees(entity.transform.rotation_x + dy);
            }
            GizmoHandle::RotateY => {
                entity.transform.rotation_y = normalize_degrees(entity.transform.rotation_y + dx);
            }
            GizmoHandle::RotateZ => {
                entity.transform.rotation = normalize_degrees(entity.transform.rotation + dx - dy);
            }
            GizmoHandle::ScaleUniform => {
                let amount = 1.0 + (dx - dy) * 0.01;
                entity.transform.scale_x = (entity.transform.scale_x * amount).max(0.01);
                entity.transform.scale_y = (entity.transform.scale_y * amount).max(0.01);
                entity.transform.scale_z = (entity.transform.scale_z * amount).max(0.01);
            }
            GizmoHandle::ScaleX => {
                entity.transform.scale_x = (entity.transform.scale_x + dx * 0.02).max(0.01);
            }
            GizmoHandle::ScaleY => {
                entity.transform.scale_y = (entity.transform.scale_y + dy * 0.02).max(0.01);
            }
            GizmoHandle::ScaleZ => {
                entity.transform.scale_z = (entity.transform.scale_z - dy * 0.02).max(0.01);
            }
        }

        self.scene_dirty = true;
    }

    fn create_entity(&mut self) {
        let Some(scene) = &mut self.scene else {
            self.push_console("No scene loaded");
            return;
        };

        let next = scene.entities.len() + 1;
        let stable_id = unique_entity_id(scene, "entity");
        scene.entities.push(SceneEntity {
            stable_id: stable_id.clone(),
            name: format!("Entity {next}"),
            layer: "default".to_string(),
            parent: String::new(),
            tags: Vec::new(),
            components: Vec::new(),
            model_asset: String::new(),
            transform: Transform2D {
                x: 64.0 * next as f32,
                y: 48.0 * next as f32,
                ..Transform2D::default()
            },
        });
        self.selected_entity_id = stable_id;
        self.scene_dirty = true;
    }

    fn duplicate_selected_entity(&mut self) {
        let Some(scene) = &mut self.scene else {
            self.push_console("No scene loaded");
            return;
        };

        let Some(source) = scene
            .entities
            .iter()
            .find(|entity| entity.stable_id == self.selected_entity_id)
            .cloned()
        else {
            self.push_console("Select an entity before Duplicate");
            return;
        };

        let mut duplicate = source;
        duplicate.stable_id = unique_entity_id(scene, &duplicate.stable_id);
        duplicate.name = format!("{} Copy", duplicate.name);
        duplicate.transform.x += SNAP_SIZE;
        duplicate.transform.y += SNAP_SIZE;
        self.selected_entity_id = duplicate.stable_id.clone();
        scene.entities.push(duplicate);
        self.scene_dirty = true;
    }

    fn delete_selected_entity(&mut self) {
        let Some(scene) = &mut self.scene else {
            self.push_console("No scene loaded");
            return;
        };

        let original_len = scene.entities.len();
        scene
            .entities
            .retain(|entity| entity.stable_id != self.selected_entity_id);
        if scene.entities.len() == original_len {
            self.push_console("Select an entity before Delete");
            return;
        }

        self.selected_entity_id = scene
            .entities
            .first()
            .map(|entity| entity.stable_id.clone())
            .unwrap_or_default();
        self.scene_dirty = true;
    }

    fn menu_bar(&self) -> Element<'_, Message> {
        let menus = row![
            main_menu_button(MainMenu::File, self.active_main_menu),
            main_menu_button(MainMenu::Edit, self.active_main_menu),
            main_menu_button(MainMenu::View, self.active_main_menu),
            main_menu_button(MainMenu::Window, self.active_main_menu),
            main_menu_button(MainMenu::Help, self.active_main_menu),
            Space::new().width(Length::Fill),
            text("Borealis Editor")
                .size(12)
                .style(Modern::secondary_text()),
        ]
        .spacing(10)
        .align_y(iced::Alignment::Center);

        container(menus)
            .padding([6, 12])
            .height(34)
            .width(Length::Fill)
            .style(Modern::container(ContainerKind::Toolbar))
            .into()
    }

    fn menu_panel(&self) -> Element<'_, Message> {
        let Some(menu) = self.active_main_menu else {
            return Space::new().height(0).into();
        };

        let content: Element<'_, Message> = match menu {
            MainMenu::File => row![
                menu_command_button("Nova entidade", Message::CreateEntity),
                menu_command_button("Salvar cena", Message::SaveScene),
                menu_command_button("Salvar tudo", Message::SaveAll),
                menu_command_button("Check", Message::RunEditorCommand(EditorCommand::Check)),
                menu_command_button("Build", Message::RunEditorCommand(EditorCommand::Build)),
            ]
            .spacing(8)
            .align_y(iced::Alignment::Center)
            .into(),
            MainMenu::Edit => row![
                menu_command_button(
                    "Select",
                    Message::ViewportToolSelected(ViewportTool::Select)
                ),
                menu_command_button("Move", Message::ViewportToolSelected(ViewportTool::Move)),
                menu_command_button(
                    "Rotate",
                    Message::ViewportToolSelected(ViewportTool::Rotate)
                ),
                menu_command_button("Scale", Message::ViewportToolSelected(ViewportTool::Scale)),
                rule::vertical(1),
                menu_command_button("Duplicar", Message::DuplicateEntity),
                menu_command_button("Excluir", Message::DeleteSelectedEntity),
            ]
            .spacing(8)
            .align_y(iced::Alignment::Center)
            .into(),
            MainMenu::View => row![
                menu_command_button(
                    "Scene 2D",
                    Message::WorkspaceTabSelected(WorkspaceTab::Scene)
                ),
                menu_command_button(
                    "Scene 3D",
                    Message::WorkspaceTabSelected(WorkspaceTab::Scene3D)
                ),
                menu_command_button(
                    "Script",
                    Message::WorkspaceTabSelected(WorkspaceTab::Script)
                ),
                menu_command_button(
                    "Terminal",
                    Message::WorkspaceTabSelected(WorkspaceTab::Terminal)
                ),
                menu_command_button(
                    "Console",
                    Message::WorkspaceTabSelected(WorkspaceTab::Console)
                ),
                rule::vertical(1),
                menu_command_button("Grid", Message::ToggleGrid),
                menu_command_button("Gizmos", Message::ToggleGizmos),
                menu_command_button("Cube", Message::ToggleViewCube),
                rule::vertical(1),
                menu_command_button("Front", Message::ViewPresetSelected(ViewPreset::Front)),
                menu_command_button("Right", Message::ViewPresetSelected(ViewPreset::Right)),
                menu_command_button("Top", Message::ViewPresetSelected(ViewPreset::Top)),
                menu_command_button("Reset", Message::ViewPresetSelected(ViewPreset::Reset)),
            ]
            .spacing(8)
            .align_y(iced::Alignment::Center)
            .into(),
            MainMenu::Window => row![
                menu_command_button("Settings", Message::ToggleSettingsMenu),
                menu_command_button("Focus", Message::FocusSelectedEntity),
                menu_command_button("Frame", Message::FrameScene),
            ]
            .spacing(8)
            .align_y(iced::Alignment::Center)
            .into(),
            MainMenu::Help => row![
                text("W Select").size(12),
                text("G Move").size(12),
                text("R Rotate").size(12),
                text("S Scale").size(12),
                text("F Frame").size(12),
                text("1/3/7 Views").size(12),
                text("5 Reset").size(12),
            ]
            .spacing(14)
            .align_y(iced::Alignment::Center)
            .into(),
        };

        container(scrollable(content).direction(ScrollDirection::Horizontal(Scrollbar::default())))
            .padding([6, 12])
            .height(40)
            .width(Length::Fill)
            .style(Modern::container(ContainerKind::Panel))
            .into()
    }

    fn settings_menu(&self) -> Element<'_, Message> {
        if !self.settings_menu_open {
            return Space::new().height(0).into();
        }

        let viewport = column![
            text("Viewport").size(13),
            row![
                settings_toggle_button("Snap", self.snap_enabled, Message::ToggleSnap),
                settings_toggle_button("Grid", self.grid_enabled, Message::ToggleGrid),
                settings_toggle_button("Gizmos", self.gizmos_enabled, Message::ToggleGizmos),
                settings_toggle_button(
                    "View cube",
                    self.view_cube_enabled,
                    Message::ToggleViewCube
                ),
            ]
            .spacing(8),
            row![
                text("Gizmo size").size(12).width(88),
                number_input(&self.gizmo_size, 0.6..=2.0, Message::GizmoSizeChanged)
                    .step(0.1)
                    .set_size(12.0)
                    .padding(6.0),
            ]
            .spacing(8)
            .align_y(iced::Alignment::Center),
            row![
                button(text("Front 1").size(12))
                    .style(Modern::system_button())
                    .on_press(Message::ViewPresetSelected(ViewPreset::Front)),
                button(text("Right 3").size(12))
                    .style(Modern::system_button())
                    .on_press(Message::ViewPresetSelected(ViewPreset::Right)),
                button(text("Top 7").size(12))
                    .style(Modern::system_button())
                    .on_press(Message::ViewPresetSelected(ViewPreset::Top)),
                button(text("Reset 5").size(12))
                    .style(Modern::system_button())
                    .on_press(Message::ViewPresetSelected(ViewPreset::Reset)),
            ]
            .spacing(8),
        ]
        .spacing(8);

        let keys = column![
            text("Keybinding").size(13),
            text("W Select | G Move | R Rotate | S Scale").size(12),
            text("Middle drag Pan | Right drag Orbit 3D | Wheel Zoom").size(12),
            text("F Frame | 1 Front | 3 Right | 7 Top | 5 Reset").size(12),
        ]
        .spacing(6);

        let content = row![viewport, rule::vertical(1), keys]
            .spacing(18)
            .align_y(iced::Alignment::Start);

        container(scrollable(content).direction(ScrollDirection::Horizontal(Scrollbar::default())))
            .padding([10, 12])
            .height(128)
            .width(Length::Fill)
            .style(Modern::container(ContainerKind::Toolbar))
            .into()
    }

    fn top_bar(&self) -> Element<'_, Message> {
        let dirty = match (self.scene_dirty, self.has_dirty_files()) {
            (true, true) => "Scene + Script modified",
            (true, false) => "Scene modified",
            (false, true) => "Script modified",
            (false, false) => "Saved",
        };

        let scene_name = self
            .scene
            .as_ref()
            .map(|scene| scene.name.as_str())
            .unwrap_or("No scene");

        let left = row![
            text("Borealis").size(13),
            text(scene_name).size(12).style(Modern::secondary_text()),
        ]
        .spacing(10)
        .align_y(iced::Alignment::Center)
        .width(Length::FillPortion(1));

        let transport = row![
            icon_button(
                Icon::Play,
                Message::RunEditorCommand(EditorCommand::Run),
                ButtonKind::Success
            ),
            icon_button(Icon::Pause, Message::Noop, ButtonKind::System),
            icon_button(Icon::Square, Message::Noop, ButtonKind::Danger),
        ]
        .spacing(8)
        .align_y(iced::Alignment::Center);

        let right = row![
            Space::new().width(Length::Fill),
            icon_button(Icon::Save, Message::SaveAll, ButtonKind::Primary),
            icon_button(
                Icon::CheckCircle,
                Message::RunEditorCommand(EditorCommand::Check),
                ButtonKind::Secondary
            ),
            icon_button(
                Icon::Hammer,
                Message::RunEditorCommand(EditorCommand::Build),
                ButtonKind::Secondary
            ),
            icon_button(
                Icon::RefreshCw,
                Message::RunEditorCommand(EditorCommand::Summary),
                ButtonKind::System
            ),
            text(dirty).size(12).style(Modern::secondary_text()),
            text(format!("{:.0}%", self.viewport_zoom * 100.0)).size(12),
        ]
        .spacing(8)
        .align_y(iced::Alignment::Center)
        .width(Length::FillPortion(1));

        container(
            row![left, container(transport).width(Length::Shrink), right,]
                .spacing(12)
                .align_y(iced::Alignment::Center),
        )
        .padding([6, 12])
        .height(42)
        .width(Length::Fill)
        .style(Modern::container(ContainerKind::Toolbar))
        .into()
    }

    fn left_panel(&self) -> Element<'_, Message> {
        let mut scenes = column![text("Hierarchy").size(15)].spacing(8);

        if let Some(scene) = &self.scene {
            scenes = scenes.push(meta_row("Scene", &scene.name));
            scenes = scenes.push(meta_row("Entities", &scene.entities.len().to_string()));
            scenes = scenes.push(rule::horizontal(1));

            for entity in &scene.entities {
                let label = if entity.stable_id == self.selected_entity_id {
                    format!("> {}", entity.name)
                } else {
                    entity.name.clone()
                };
                scenes = scenes.push(
                    button(text(label).size(13))
                        .style(Modern::button(
                            if entity.stable_id == self.selected_entity_id {
                                ButtonKind::Primary
                            } else {
                                ButtonKind::Plain
                            },
                        ))
                        .width(Length::Fill)
                        .on_press(Message::EntitySelected(entity.stable_id.clone())),
                );
            }
        } else {
            scenes = scenes.push(text("No scene loaded").size(12));
        }

        container(scrollable(scenes))
            .width(276)
            .height(Length::Fill)
            .padding(12)
            .style(Modern::container(ContainerKind::Sidebar))
            .into()
    }

    fn center_panel(&self) -> Element<'_, Message> {
        let header = row![
            self.workspace_tabs(),
            Space::new().width(Length::Fill),
            text(format!("Tool: {}", self.active_viewport_tool.label()))
                .size(12)
                .style(Modern::secondary_text()),
            button(text("Settings").size(12))
                .style(Modern::system_button())
                .on_press(Message::ToggleSettingsMenu),
        ]
        .spacing(8)
        .align_y(iced::Alignment::Center);

        let content = match self.active_tab {
            WorkspaceTab::Scene | WorkspaceTab::Scene3D => self.viewport_panel(),
            WorkspaceTab::Script => self.script_panel(),
            WorkspaceTab::Terminal => self.terminal_panel(),
            WorkspaceTab::Console => self.console_panel(),
        };

        let mut layout = column![container(header).padding([8, 10])].width(Length::Fill);

        if matches!(self.active_tab, WorkspaceTab::Scene | WorkspaceTab::Scene3D) {
            layout = layout.push(self.viewport_toolbar());
        }

        if self.settings_menu_open {
            layout = layout.push(self.settings_menu());
        }

        layout = layout
            .push(content)
            .push(rule::horizontal(1))
            .push(self.bottom_project_panel());

        layout.width(Length::Fill).height(Length::Fill).into()
    }

    fn viewport_toolbar(&self) -> Element<'_, Message> {
        let tools = row![
            text("Tools").size(12).style(Modern::secondary_text()),
            viewport_tool_button(
                Icon::MousePointer2,
                ViewportTool::Select,
                self.active_viewport_tool
            ),
            viewport_tool_button(Icon::Move, ViewportTool::Move, self.active_viewport_tool),
            viewport_tool_button(
                Icon::RotateCw,
                ViewportTool::Rotate,
                self.active_viewport_tool
            ),
            viewport_tool_button(
                Icon::Scaling,
                ViewportTool::Scale,
                self.active_viewport_tool
            ),
            rule::vertical(1),
            compact_toggle_button(Icon::Magnet, self.snap_enabled, Message::ToggleSnap),
            compact_toggle_button(Icon::Grid, self.grid_enabled, Message::ToggleGrid),
            compact_toggle_button(Icon::Crosshair, self.gizmos_enabled, Message::ToggleGizmos),
            compact_toggle_button(Icon::Box, self.view_cube_enabled, Message::ToggleViewCube),
            rule::vertical(1),
            icon_button(
                Icon::Focus,
                Message::FocusSelectedEntity,
                ButtonKind::System
            ),
            icon_button(Icon::Maximize2, Message::FrameScene, ButtonKind::System),
        ]
        .spacing(6)
        .align_y(iced::Alignment::Center);

        container(scrollable(tools).direction(ScrollDirection::Horizontal(Scrollbar::default())))
            .padding([6, 10])
            .height(40)
            .width(Length::Fill)
            .style(Modern::container(ContainerKind::Toolbar))
            .into()
    }

    fn viewport_panel(&self) -> Element<'_, Message> {
        let entities = self
            .scene
            .as_ref()
            .map(|scene| {
                scene
                    .entities
                    .iter()
                    .map(ViewportEntity::from)
                    .collect::<Vec<_>>()
            })
            .unwrap_or_default();

        let viewport = ViewportCanvas {
            entities,
            selected_entity_id: self.selected_entity_id.clone(),
            mode: self.active_tab,
            pan: self.viewport_pan,
            zoom: self.viewport_zoom,
            snap_enabled: self.snap_enabled,
            yaw: self.viewport_3d_yaw,
            pitch: self.viewport_3d_pitch,
            active_tool: self.active_viewport_tool,
            show_grid: self.grid_enabled,
            show_gizmos: self.gizmos_enabled,
            show_view_cube: self.view_cube_enabled,
            gizmo_size: self.gizmo_size,
        };

        container(
            Canvas::new(viewport)
                .width(Length::Fill)
                .height(Length::Fill),
        )
        .width(Length::Fill)
        .height(Length::Fill)
        .padding(Padding::default().left(10).right(10).bottom(10))
        .style(Modern::container(ContainerKind::Viewport))
        .into()
    }

    fn script_panel(&self) -> Element<'_, Message> {
        let active_label = self
            .open_files
            .iter()
            .find(|file| file.path == self.active_file_path)
            .map(|file| file.label.as_str())
            .unwrap_or("No file");

        let header = row![
            text(active_label).size(15),
            text(&self.script_path)
                .size(12)
                .style(Modern::secondary_text()),
            Space::new().width(Length::Fill),
            button(text("Run Check").size(12))
                .style(Modern::system_button())
                .on_press(Message::RunProjectCheck),
            button(text("Save").size(12))
                .style(Modern::primary_button())
                .on_press(Message::SaveScript),
        ]
        .spacing(8)
        .align_y(iced::Alignment::Center);

        let code = container(self.script_editor.view().map(Message::ScriptEditorEvent))
            .padding(8)
            .width(Length::Fill)
            .height(Length::Fill)
            .style(Modern::container(ContainerKind::Code));

        column![
            container(header).padding([8, 10]),
            self.file_tab_strip(),
            code,
        ]
        .width(Length::Fill)
        .height(Length::Fill)
        .into()
    }

    fn file_tab_strip(&self) -> Element<'_, Message> {
        let mut tabs = row![].spacing(4).align_y(iced::Alignment::Center);

        for file in &self.open_files {
            let label = if file.dirty {
                format!("* {}", file.label)
            } else {
                file.label.clone()
            };
            let selected = file.path == self.active_file_path;

            tabs = tabs
                .push(
                    button(text(label).size(12))
                        .style(Modern::button(if selected {
                            ButtonKind::Primary
                        } else {
                            ButtonKind::System
                        }))
                        .on_press(Message::FileTabSelected(file.path.clone())),
                )
                .push(
                    button(text("x").size(12))
                        .style(Modern::system_button())
                        .on_press(Message::CloseFileTab(file.path.clone())),
                );
        }

        container(scrollable(tabs).direction(ScrollDirection::Horizontal(Scrollbar::default())))
            .padding([4, 10])
            .width(Length::Fill)
            .style(Modern::container(ContainerKind::Toolbar))
            .into()
    }

    fn bottom_project_panel(&self) -> Element<'_, Message> {
        let item_count = self.project_items().len();
        let controls = row![
            text("Project").size(15),
            text(format!("{item_count} items"))
                .size(12)
                .style(Modern::secondary_text()),
            Space::new().width(Length::Fill),
            view_mode_button(ProjectViewMode::List, self.bottom_view_mode),
            view_mode_button(ProjectViewMode::Grid, self.bottom_view_mode),
            view_mode_button(ProjectViewMode::Details, self.bottom_view_mode),
        ]
        .spacing(8)
        .align_y(iced::Alignment::Center);

        let content = match self.bottom_view_mode {
            ProjectViewMode::List => self.project_list_view(),
            ProjectViewMode::Grid => self.project_grid_view(),
            ProjectViewMode::Details => self.project_details_view(),
        };

        container(column![controls, content].spacing(8))
            .height(BOTTOM_PANEL_HEIGHT)
            .width(Length::Fill)
            .padding([10, 12])
            .style(Modern::container(ContainerKind::Panel))
            .into()
    }

    fn project_list_view(&self) -> Element<'_, Message> {
        let mut rows = column![].spacing(4);
        for item in self.project_items() {
            rows = rows.push(
                button(
                    row![
                        text(item.label.clone()).size(12),
                        Space::new().width(Length::Fill),
                        text(project_item_kind_label(&item.kind))
                            .size(11)
                            .style(Modern::secondary_text()),
                    ]
                    .spacing(8),
                )
                .style(Modern::system_button())
                .width(Length::Fill)
                .on_press(Message::ProjectItemSelected(item.id.clone())),
            );
        }

        container(scrollable(rows))
            .height(Length::Fill)
            .width(Length::Fill)
            .into()
    }

    fn project_grid_view(&self) -> Element<'_, Message> {
        let items = self.project_items();
        let mut grid = column![].spacing(6);

        for chunk in items.chunks(4) {
            let mut line = row![].spacing(6);
            for item in chunk {
                line = line.push(
                    button(
                        column![
                            text(item.label.clone()).size(12),
                            text(project_item_kind_label(&item.kind))
                                .size(11)
                                .style(Modern::secondary_text()),
                        ]
                        .spacing(2),
                    )
                    .style(Modern::system_button())
                    .width(Length::Fill)
                    .height(48)
                    .on_press(Message::ProjectItemSelected(item.id.clone())),
                );
            }

            for _ in chunk.len()..4 {
                line = line.push(Space::new().width(Length::Fill).height(48));
            }

            grid = grid.push(line);
        }

        container(scrollable(grid))
            .height(Length::Fill)
            .width(Length::Fill)
            .into()
    }

    fn project_details_view(&self) -> Element<'_, Message> {
        let mut rows = column![].spacing(4).push(
            row![
                text("Name").size(11).style(Modern::secondary_text()),
                text("Type").size(11).style(Modern::secondary_text()),
                text("Path").size(11).style(Modern::secondary_text()),
            ]
            .spacing(8),
        );

        for item in self.project_items() {
            rows = rows.push(
                button(
                    row![
                        text(item.label.clone())
                            .size(12)
                            .width(Length::FillPortion(2)),
                        text(project_item_kind_label(&item.kind))
                            .size(12)
                            .width(Length::FillPortion(1)),
                        text(item.path.clone())
                            .size(12)
                            .width(Length::FillPortion(4)),
                    ]
                    .spacing(8),
                )
                .style(Modern::plain_button())
                .width(Length::Fill)
                .on_press(Message::ProjectItemSelected(item.id.clone())),
            );
        }

        container(scrollable(rows))
            .height(Length::Fill)
            .width(Length::Fill)
            .into()
    }

    fn project_items(&self) -> Vec<ProjectTreeItem> {
        self.project_tree
            .sections
            .iter()
            .flat_map(|section| section.items.iter().cloned())
            .collect()
    }

    fn inspector_panel(&self) -> Element<'_, Message> {
        let content = if let Some(entity) = self.selected_entity() {
            let tags = entity.tags.join(", ");
            let position = column![
                text("Transform").size(15),
                number_row("X", entity.transform.x, TransformNumberField::PosX, 1.0),
                number_row("Y", entity.transform.y, TransformNumberField::PosY, 1.0),
                number_row("Z", entity.transform.z, TransformNumberField::PosZ, 1.0),
                rule::horizontal(1),
                number_row(
                    "Rot X",
                    entity.transform.rotation_x,
                    TransformNumberField::RotationX,
                    1.0
                ),
                number_row(
                    "Rot Y",
                    entity.transform.rotation_y,
                    TransformNumberField::RotationY,
                    1.0
                ),
                number_row(
                    "Rot Z",
                    entity.transform.rotation,
                    TransformNumberField::RotationZ,
                    1.0
                ),
                rule::horizontal(1),
                number_row(
                    "Scale X",
                    entity.transform.scale_x,
                    TransformNumberField::ScaleX,
                    0.1
                ),
                number_row(
                    "Scale Y",
                    entity.transform.scale_y,
                    TransformNumberField::ScaleY,
                    0.1
                ),
                number_row(
                    "Scale Z",
                    entity.transform.scale_z,
                    TransformNumberField::ScaleZ,
                    0.1
                ),
            ]
            .spacing(8);

            column![
                text("Inspector").size(15),
                field_row("Name", entity.name.clone(), Message::EntityNameChanged),
                read_only_row("Stable ID", &entity.stable_id),
                field_row("Layer", entity.layer.clone(), Message::EntityLayerChanged),
                read_only_row("Parent", &entity.parent),
                field_row("Tags", tags, Message::EntityTagsChanged),
                field_row(
                    "Model",
                    entity.model_asset.clone(),
                    Message::EntityModelAssetChanged
                ),
                rule::horizontal(1),
                position,
                rule::horizontal(1),
                text("Components").size(15),
                component_editor_list(&entity.components, &self.component_draft),
            ]
            .spacing(10)
        } else {
            column![
                text("Inspector").size(15),
                text("Select an entity in the hierarchy or viewport.").size(12),
            ]
            .spacing(10)
        };

        container(scrollable(content))
            .width(336)
            .height(Length::Fill)
            .padding(Padding::default().top(12).right(24).bottom(12).left(12))
            .style(Modern::container(ContainerKind::Panel))
            .into()
    }

    fn workspace_tabs(&self) -> Element<'_, Message> {
        TabBar::new(Message::WorkspaceTabSelected)
            .push(WorkspaceTab::Scene, TabLabel::Text("Scene".to_string()))
            .push(
                WorkspaceTab::Scene3D,
                TabLabel::Text("Scene 3D".to_string()),
            )
            .push(WorkspaceTab::Script, TabLabel::Text("Script".to_string()))
            .push(
                WorkspaceTab::Terminal,
                TabLabel::Text("Terminal".to_string()),
            )
            .push(WorkspaceTab::Console, TabLabel::Text("Console".to_string()))
            .set_active_tab(&self.active_tab)
            .tab_width(Length::Shrink)
            .spacing(4.0)
            .padding(Padding::from([6.0, 10.0]))
            .text_size(12.0)
            .into()
    }

    fn terminal_panel(&self) -> Element<'_, Message> {
        #[cfg(feature = "terminal")]
        {
            if let Some(terminal) = &self.terminal {
                return container(
                    iced_term::TerminalView::show(terminal).map(Message::TerminalEvent),
                )
                .padding(10)
                .width(Length::Fill)
                .height(Length::Fill)
                .style(Modern::container(ContainerKind::Viewport))
                .into();
            }

            return container(
                column![
                    text("Terminal").size(15),
                    text("Terminal could not start. Check the Console tab.").size(12),
                ]
                .spacing(8),
            )
            .padding(12)
            .width(Length::Fill)
            .height(Length::Fill)
            .style(Modern::container(ContainerKind::Code))
            .into();
        }

        #[cfg(not(feature = "terminal"))]
        {
            container(
                column![
                    text("Terminal").size(15),
                    text("Enable it with: cargo run --features terminal").size(12),
                ]
                .spacing(8),
            )
            .padding(12)
            .width(Length::Fill)
            .height(Length::Fill)
            .style(Modern::container(ContainerKind::Code))
            .into()
        }
    }

    fn console_panel(&self) -> Element<'_, Message> {
        let mut lines = column![text("Console").size(15)].spacing(8);
        for item in &self.console {
            lines = lines.push(text(item).size(12));
        }

        container(scrollable(lines))
            .padding(12)
            .width(Length::Fill)
            .height(Length::Fill)
            .style(Modern::container(ContainerKind::Code))
            .into()
    }

    #[cfg(feature = "terminal")]
    fn handle_terminal_event(&mut self, event: iced_term::Event) {
        let Some(terminal) = &mut self.terminal else {
            return;
        };

        match event {
            iced_term::Event::BackendCall(_, command) => {
                let action = terminal.handle(iced_term::Command::ProxyToBackend(command));
                match action {
                    iced_term::actions::Action::Shutdown => {
                        self.push_console("Terminal backend stopped");
                    }
                    iced_term::actions::Action::ChangeTitle(title) => {
                        self.push_console(format!("Terminal: {title}"));
                    }
                    iced_term::actions::Action::Ignore => {}
                }
            }
        }
    }
}

#[derive(Debug, Clone)]
struct ViewportEntity {
    stable_id: String,
    name: String,
    x: f32,
    y: f32,
    z: f32,
    rotation: f32,
    rotation_x: f32,
    rotation_y: f32,
    scale_x: f32,
    scale_y: f32,
    scale_z: f32,
}

impl From<&SceneEntity> for ViewportEntity {
    fn from(entity: &SceneEntity) -> Self {
        Self {
            stable_id: entity.stable_id.clone(),
            name: entity.name.clone(),
            x: entity.transform.x,
            y: entity.transform.y,
            z: entity.transform.z,
            rotation: entity.transform.rotation,
            rotation_x: entity.transform.rotation_x,
            rotation_y: entity.transform.rotation_y,
            scale_x: entity.transform.scale_x,
            scale_y: entity.transform.scale_y,
            scale_z: entity.transform.scale_z,
        }
    }
}

struct ViewportCanvas {
    entities: Vec<ViewportEntity>,
    selected_entity_id: String,
    mode: WorkspaceTab,
    pan: Vector,
    zoom: f32,
    snap_enabled: bool,
    yaw: f32,
    pitch: f32,
    active_tool: ViewportTool,
    show_grid: bool,
    show_gizmos: bool,
    show_view_cube: bool,
    gizmo_size: f32,
}

#[derive(Debug, Clone, Default)]
struct ViewportState {
    panning: bool,
    orbiting: bool,
    dragging_entity: Option<String>,
    active_gizmo: Option<(String, GizmoHandle)>,
    last_cursor: Option<Point>,
}

impl canvas::Program<Message> for ViewportCanvas {
    type State = ViewportState;

    fn update(
        &self,
        state: &mut Self::State,
        event: &CanvasEvent,
        bounds: Rectangle,
        cursor: mouse::Cursor,
    ) -> Option<Action<Message>> {
        let position = cursor.position_in(bounds);

        match event {
            CanvasEvent::Mouse(mouse::Event::ButtonPressed(mouse::Button::Left)) => {
                let position = position?;
                if self.show_view_cube {
                    if let Some(preset) = self.hit_test_view_cube(position, bounds.size()) {
                        return Some(
                            Action::publish(Message::ViewPresetSelected(preset)).and_capture(),
                        );
                    }
                }

                if self.show_gizmos {
                    if let Some(handle) = self.hit_test_gizmo(position, bounds.size()) {
                        state.active_gizmo = Some((self.selected_entity_id.clone(), handle));
                        state.last_cursor = Some(position);
                        return Some(Action::capture());
                    }
                }

                if let Some(entity_id) = self.hit_test(position, bounds.size()) {
                    if self.active_tool == ViewportTool::Move {
                        state.dragging_entity = Some(entity_id.clone());
                    }
                    state.last_cursor = Some(position);
                    return Some(Action::publish(Message::EntitySelected(entity_id)).and_capture());
                }
            }
            CanvasEvent::Mouse(mouse::Event::ButtonPressed(mouse::Button::Middle)) => {
                state.panning = true;
                state.last_cursor = position;
                return Some(Action::capture());
            }
            CanvasEvent::Mouse(mouse::Event::ButtonPressed(mouse::Button::Right)) => {
                if self.mode == WorkspaceTab::Scene3D {
                    state.orbiting = true;
                } else {
                    state.panning = true;
                }
                state.last_cursor = position;
                return Some(Action::capture());
            }
            CanvasEvent::Mouse(mouse::Event::ButtonReleased(_)) => {
                state.panning = false;
                state.orbiting = false;
                state.dragging_entity = None;
                state.active_gizmo = None;
                state.last_cursor = None;
                return Some(Action::capture());
            }
            CanvasEvent::Mouse(mouse::Event::CursorMoved { .. }) => {
                let Some(position) = position else {
                    return None;
                };
                let Some(previous) = state.last_cursor.replace(position) else {
                    return None;
                };
                let delta = Vector::new(position.x - previous.x, position.y - previous.y);

                if state.panning {
                    return Some(
                        Action::publish(Message::ViewportPanned {
                            dx: delta.x,
                            dy: delta.y,
                        })
                        .and_capture(),
                    );
                }

                if state.orbiting {
                    return Some(
                        Action::publish(Message::Viewport3DOrbited {
                            dx: delta.x,
                            dy: delta.y,
                        })
                        .and_capture(),
                    );
                }

                if let Some((stable_id, handle)) = &state.active_gizmo {
                    let (dx, dy) = self.screen_delta_to_scene_delta(delta);
                    return Some(
                        Action::publish(Message::ViewportGizmoDragged {
                            stable_id: stable_id.clone(),
                            handle: *handle,
                            dx,
                            dy,
                        })
                        .and_capture(),
                    );
                }

                if let Some(stable_id) = &state.dragging_entity {
                    let (dx, dy) = self.screen_delta_to_scene_delta(delta);
                    return Some(
                        Action::publish(Message::ViewportEntityDragged {
                            stable_id: stable_id.clone(),
                            dx,
                            dy,
                        })
                        .and_capture(),
                    );
                }
            }
            CanvasEvent::Mouse(mouse::Event::WheelScrolled { delta }) => {
                let amount = match delta {
                    mouse::ScrollDelta::Lines { y, .. } => *y,
                    mouse::ScrollDelta::Pixels { y, .. } => *y / 24.0,
                };
                if amount.abs() > f32::EPSILON {
                    return Some(
                        Action::publish(Message::ViewportZoomed { delta: amount }).and_capture(),
                    );
                }
            }
            _ => {}
        }

        None
    }

    fn draw(
        &self,
        _state: &Self::State,
        renderer: &Renderer,
        _theme: &Theme,
        bounds: Rectangle,
        _cursor: mouse::Cursor,
    ) -> Vec<canvas::Geometry> {
        let mut frame = Frame::new(renderer, bounds.size());
        let background = CanvasPath::rectangle(Point::ORIGIN, bounds.size());
        frame.fill(&background, color(0x24282f));

        match self.mode {
            WorkspaceTab::Scene => self.draw_scene_2d(&mut frame, bounds.size()),
            WorkspaceTab::Scene3D => self.draw_scene_3d(&mut frame, bounds.size()),
            WorkspaceTab::Script | WorkspaceTab::Terminal | WorkspaceTab::Console => {}
        }

        if self.show_view_cube && matches!(self.mode, WorkspaceTab::Scene | WorkspaceTab::Scene3D) {
            self.draw_view_cube(&mut frame, bounds.size());
        }

        vec![frame.into_geometry()]
    }

    fn mouse_interaction(
        &self,
        state: &Self::State,
        bounds: Rectangle,
        cursor: mouse::Cursor,
    ) -> mouse::Interaction {
        if state.panning
            || state.orbiting
            || state.dragging_entity.is_some()
            || state.active_gizmo.is_some()
        {
            return mouse::Interaction::Grabbing;
        }

        if let Some(position) = cursor.position_in(bounds) {
            if self.show_view_cube && self.hit_test_view_cube(position, bounds.size()).is_some() {
                return mouse::Interaction::Pointer;
            }
            if self.show_gizmos && self.hit_test_gizmo(position, bounds.size()).is_some() {
                return mouse::Interaction::Grab;
            }
            if self.hit_test(position, bounds.size()).is_some() {
                return mouse::Interaction::Grab;
            }
        }

        mouse::Interaction::Crosshair
    }
}

impl ViewportCanvas {
    fn draw_scene_2d(&self, frame: &mut Frame, size: Size) {
        if self.show_grid {
            draw_grid(frame, size, 32.0 * self.zoom, self.pan, color(0x343943));
        }

        for entity in &self.entities {
            let selected = entity.stable_id == self.selected_entity_id;
            let width = (56.0 * entity.scale_x * self.zoom).max(24.0);
            let height = (34.0 * entity.scale_y * self.zoom).max(20.0);
            let point = self.scene_to_viewport_2d(entity.x, entity.y);
            let center = Point::new(point.x + width * 0.5, point.y + height * 0.5);
            frame.with_save(|frame| {
                frame.translate(Vector::new(center.x, center.y));
                frame.rotate(entity.rotation.to_radians());
                let rect = CanvasPath::rectangle(
                    Point::new(-width * 0.5, -height * 0.5),
                    Size::new(width, height),
                );
                frame.fill(
                    &rect,
                    if selected {
                        color(0x4c88d9)
                    } else {
                        color(0x68717f)
                    },
                );
                frame.stroke(
                    &rect,
                    Stroke::default()
                        .with_color(if selected {
                            color(0xb8d7ff)
                        } else {
                            color(0x8d98a7)
                        })
                        .with_width(1.0),
                );
            });
            frame.fill_text(canvas::Text {
                content: entity.name.clone(),
                position: Point::new(point.x, point.y - 8.0),
                color: color(0xd8dee9),
                size: Pixels(12.0),
                ..Default::default()
            });

            if selected && self.show_gizmos {
                self.draw_gizmo_2d(frame, point, Size::new(width, height));
            }
        }
    }

    fn draw_scene_3d(&self, frame: &mut Frame, size: Size) {
        if self.show_grid {
            draw_iso_grid(frame, size);
        }

        for entity in &self.entities {
            let selected = entity.stable_id == self.selected_entity_id;
            let point = self.scene_to_viewport_3d(size, entity.x, entity.y, entity.z);
            let radius = ((if selected { 7.0 } else { 5.0 }) * entity.scale_z.max(0.4)).min(18.0);
            let marker = CanvasPath::circle(point, radius);
            frame.fill(
                &marker,
                if selected {
                    color(0x58a6ff)
                } else {
                    color(0x9aa4b2)
                },
            );
            frame.stroke(
                &marker,
                Stroke::default()
                    .with_color(if selected {
                        color(0xd7ecff)
                    } else {
                        color(0x6f7784)
                    })
                    .with_width(1.0),
            );
            frame.fill_text(canvas::Text {
                content: entity.name.clone(),
                position: Point::new(point.x + 10.0, point.y - 4.0),
                color: color(0xd8dee9),
                size: Pixels(12.0),
                ..Default::default()
            });

            if selected && self.show_gizmos {
                frame.fill_text(canvas::Text {
                    content: format!(
                        "R {:.0}/{:.0}/{:.0}",
                        entity.rotation_x, entity.rotation_y, entity.rotation
                    ),
                    position: Point::new(point.x + 10.0, point.y + 12.0),
                    color: color(0x9aa4b2),
                    size: Pixels(10.0),
                    ..Default::default()
                });
                self.draw_gizmo_3d(frame, point);
            }
        }
    }

    fn hit_test(&self, position: Point, size: Size) -> Option<String> {
        for entity in self.entities.iter().rev() {
            match self.mode {
                WorkspaceTab::Scene => {
                    let point = self.scene_to_viewport_2d(entity.x, entity.y);
                    let width = (56.0 * entity.scale_x * self.zoom).max(24.0);
                    let height = (34.0 * entity.scale_y * self.zoom).max(20.0);
                    let inside_x = position.x >= point.x && position.x <= point.x + width;
                    let inside_y = position.y >= point.y && position.y <= point.y + height;
                    if inside_x && inside_y {
                        return Some(entity.stable_id.clone());
                    }
                }
                WorkspaceTab::Scene3D => {
                    let point = self.scene_to_viewport_3d(size, entity.x, entity.y, entity.z);
                    let dx = position.x - point.x;
                    let dy = position.y - point.y;
                    if (dx * dx + dy * dy).sqrt() <= 14.0 {
                        return Some(entity.stable_id.clone());
                    }
                }
                WorkspaceTab::Script | WorkspaceTab::Terminal | WorkspaceTab::Console => {}
            }
        }

        None
    }

    fn selected_entity_screen_origin(&self, size: Size) -> Option<Point> {
        let entity = self
            .entities
            .iter()
            .find(|entity| entity.stable_id == self.selected_entity_id)?;

        match self.mode {
            WorkspaceTab::Scene => {
                let width = (56.0 * entity.scale_x * self.zoom).max(24.0);
                let height = (34.0 * entity.scale_y * self.zoom).max(20.0);
                let point = self.scene_to_viewport_2d(entity.x, entity.y);
                Some(Point::new(point.x + width * 0.5, point.y + height * 0.5))
            }
            WorkspaceTab::Scene3D => {
                Some(self.scene_to_viewport_3d(size, entity.x, entity.y, entity.z))
            }
            WorkspaceTab::Script | WorkspaceTab::Terminal | WorkspaceTab::Console => None,
        }
    }

    fn hit_test_view_cube(&self, position: Point, size: Size) -> Option<ViewPreset> {
        let origin = view_cube_origin(size);
        if point_in_rect(position, origin.x, origin.y + 62.0, 58.0, 20.0) {
            return Some(ViewPreset::Reset);
        }
        if point_in_rect(position, origin.x + 54.0, origin.y - 12.0, 16.0, 66.0) {
            return Some(ViewPreset::Right);
        }
        if point_in_rect(position, origin.x, origin.y - 14.0, 68.0, 16.0) {
            return Some(ViewPreset::Top);
        }
        if point_in_rect(position, origin.x, origin.y, 54.0, 54.0) {
            return Some(ViewPreset::Front);
        }

        None
    }

    fn hit_test_gizmo(&self, position: Point, size: Size) -> Option<GizmoHandle> {
        if self.selected_entity_id.is_empty() {
            return None;
        }

        let origin = self.selected_entity_screen_origin(size)?;
        let scale = self.gizmo_size.clamp(0.6, 2.0);
        match (self.mode, self.active_tool) {
            (_, ViewportTool::Select) => None,
            (WorkspaceTab::Scene, ViewportTool::Move) => hit_move_gizmo_2d(origin, position, scale),
            (WorkspaceTab::Scene3D, ViewportTool::Move) => {
                hit_move_gizmo_3d(origin, position, scale)
            }
            (WorkspaceTab::Scene, ViewportTool::Rotate) => {
                hit_rotate_gizmo_2d(origin, position, scale)
            }
            (WorkspaceTab::Scene3D, ViewportTool::Rotate) => {
                hit_rotate_gizmo_3d(origin, position, scale)
            }
            (WorkspaceTab::Scene, ViewportTool::Scale) => {
                hit_scale_gizmo_2d(origin, position, scale)
            }
            (WorkspaceTab::Scene3D, ViewportTool::Scale) => {
                hit_scale_gizmo_3d(origin, position, scale)
            }
            (WorkspaceTab::Script | WorkspaceTab::Terminal | WorkspaceTab::Console, _) => None,
        }
    }

    fn scene_to_viewport_2d(&self, x: f32, y: f32) -> Point {
        Point::new(
            80.0 + self.pan.x + x * self.zoom,
            72.0 + self.pan.y + y * self.zoom,
        )
    }

    fn scene_to_viewport_3d(&self, size: Size, x: f32, y: f32, z: f32) -> Point {
        let origin = Point::new(
            size.width * 0.5 + self.pan.x,
            size.height * 0.42 + self.pan.y,
        );
        let yaw_cos = self.yaw.cos();
        let yaw_sin = self.yaw.sin();
        let rotated_x = x * yaw_cos - y * yaw_sin;
        let rotated_y = x * yaw_sin + y * yaw_cos;
        let lifted_z = z + rotated_y * self.pitch.sin() * 0.45;
        project_iso(
            origin,
            rotated_x * self.zoom,
            rotated_y * self.zoom,
            lifted_z * self.zoom,
        )
    }

    fn screen_delta_to_scene_delta(&self, delta: Vector) -> (f32, f32) {
        match self.mode {
            WorkspaceTab::Scene => (delta.x / self.zoom, delta.y / self.zoom),
            WorkspaceTab::Scene3D => {
                let a = delta.x / (0.72 * self.zoom);
                let b = delta.y / (0.36 * self.zoom);
                ((a + b) * 0.5, (b - a) * 0.5)
            }
            WorkspaceTab::Script | WorkspaceTab::Terminal | WorkspaceTab::Console => (0.0, 0.0),
        }
    }

    fn draw_gizmo_2d(&self, frame: &mut Frame, point: Point, size: Size) {
        let origin = Point::new(point.x + size.width * 0.5, point.y + size.height * 0.5);
        let scale = self.gizmo_size.clamp(0.6, 2.0);
        match self.active_tool {
            ViewportTool::Select => {}
            ViewportTool::Move => {
                draw_axis(
                    frame,
                    origin,
                    scale_vector(Vector::new(58.0, 0.0), scale),
                    color(0xff5a52),
                );
                draw_axis(
                    frame,
                    origin,
                    scale_vector(Vector::new(0.0, 58.0), scale),
                    color(0x42d67c),
                );
                frame.fill(&CanvasPath::circle(origin, 5.0 * scale), color(0xf3f6fb));
                if self.snap_enabled {
                    draw_rotation_ring(frame, origin, SNAP_SIZE * 0.5 * scale, color(0x9fc5ff));
                }
            }
            ViewportTool::Rotate => {
                draw_rotation_ring(frame, origin, 45.0 * scale, color(0x5b8ee6));
                draw_axis(
                    frame,
                    origin,
                    scale_vector(Vector::new(0.0, -52.0), scale),
                    color(0x5b8ee6),
                );
            }
            ViewportTool::Scale => {
                draw_scale_handle(
                    frame,
                    origin,
                    scale_vector(Vector::new(54.0, 0.0), scale),
                    color(0xff5a52),
                    scale,
                );
                draw_scale_handle(
                    frame,
                    origin,
                    scale_vector(Vector::new(0.0, 54.0), scale),
                    color(0x42d67c),
                    scale,
                );
                draw_scale_square(frame, origin, 10.0 * scale, color(0xf3f6fb));
            }
        }
    }

    fn draw_gizmo_3d(&self, frame: &mut Frame, origin: Point) {
        let scale = self.gizmo_size.clamp(0.6, 2.0);
        match self.active_tool {
            ViewportTool::Select => {}
            ViewportTool::Move => {
                draw_axis(
                    frame,
                    origin,
                    scale_vector(Vector::new(58.0, 29.0), scale),
                    color(0xff5a52),
                );
                draw_axis(
                    frame,
                    origin,
                    scale_vector(Vector::new(-58.0, 29.0), scale),
                    color(0x42d67c),
                );
                draw_axis(
                    frame,
                    origin,
                    scale_vector(Vector::new(0.0, -64.0), scale),
                    color(0x58a6ff),
                );
                if self.snap_enabled {
                    draw_rotation_ring(frame, origin, SNAP_SIZE * 0.5 * scale, color(0x9fc5ff));
                }
            }
            ViewportTool::Rotate => {
                draw_ellipse_ring(
                    frame,
                    origin,
                    Vector::new(54.0 * scale, 17.0 * scale),
                    0.45,
                    color(0xff5a52),
                );
                draw_ellipse_ring(
                    frame,
                    origin,
                    Vector::new(54.0 * scale, 17.0 * scale),
                    -0.45,
                    color(0x42d67c),
                );
                draw_rotation_ring(frame, origin, 48.0 * scale, color(0x58a6ff));
            }
            ViewportTool::Scale => {
                draw_scale_handle(
                    frame,
                    origin,
                    scale_vector(Vector::new(54.0, 27.0), scale),
                    color(0xff5a52),
                    scale,
                );
                draw_scale_handle(
                    frame,
                    origin,
                    scale_vector(Vector::new(-54.0, 27.0), scale),
                    color(0x42d67c),
                    scale,
                );
                draw_scale_handle(
                    frame,
                    origin,
                    scale_vector(Vector::new(0.0, -60.0), scale),
                    color(0x58a6ff),
                    scale,
                );
                draw_scale_square(frame, origin, 10.0 * scale, color(0xf3f6fb));
            }
        }
    }

    fn draw_view_cube(&self, frame: &mut Frame, size: Size) {
        let origin = view_cube_origin(size);
        let front = CanvasPath::rectangle(origin, Size::new(54.0, 54.0));
        let top = CanvasPath::new(|path| {
            path.move_to(origin);
            path.line_to(Point::new(origin.x + 12.0, origin.y - 12.0));
            path.line_to(Point::new(origin.x + 66.0, origin.y - 12.0));
            path.line_to(Point::new(origin.x + 54.0, origin.y));
            path.close();
        });
        let side = CanvasPath::new(|path| {
            path.move_to(Point::new(origin.x + 54.0, origin.y));
            path.line_to(Point::new(origin.x + 66.0, origin.y - 12.0));
            path.line_to(Point::new(origin.x + 66.0, origin.y + 42.0));
            path.line_to(Point::new(origin.x + 54.0, origin.y + 54.0));
            path.close();
        });

        frame.fill(&top, color(0x3a414d));
        frame.fill(&side, color(0x303743));
        frame.fill(&front, color(0x252b35));
        for path in [&top, &side, &front] {
            frame.stroke(
                path,
                Stroke::default()
                    .with_color(color(0x9aa4b2))
                    .with_width(1.0),
            );
        }

        let center = Point::new(origin.x + 27.0, origin.y + 27.0);
        draw_axis(frame, center, Vector::new(28.0, 14.0), color(0xff5a52));
        draw_axis(frame, center, Vector::new(-28.0, 14.0), color(0x42d67c));
        draw_axis(frame, center, Vector::new(0.0, -32.0), color(0x58a6ff));
        frame.fill_text(canvas::Text {
            content: format!(
                "Y {:.0} P {:.0}",
                self.yaw.to_degrees(),
                self.pitch.to_degrees()
            ),
            position: Point::new(origin.x - 8.0, origin.y + 72.0),
            color: color(0xd8dee9),
            size: Pixels(10.0),
            ..Default::default()
        });
    }
}

fn draw_grid(frame: &mut Frame, size: Size, step: f32, pan: Vector, line_color: Color) {
    let step = step.max(8.0);
    let mut x = pan.x.rem_euclid(step);
    while x < size.width {
        let path = CanvasPath::line(Point::new(x, 0.0), Point::new(x, size.height));
        frame.stroke(
            &path,
            Stroke::default().with_color(line_color).with_width(1.0),
        );
        x += step;
    }

    let mut y = pan.y.rem_euclid(step);
    while y < size.height {
        let path = CanvasPath::line(Point::new(0.0, y), Point::new(size.width, y));
        frame.stroke(
            &path,
            Stroke::default().with_color(line_color).with_width(1.0),
        );
        y += step;
    }
}

fn draw_iso_grid(frame: &mut Frame, size: Size) {
    let origin = Point::new(size.width * 0.5, size.height * 0.42);
    for value in (-10..=10).map(|item| item as f32 * 48.0) {
        let a = project_iso(origin, value, -480.0, 0.0);
        let b = project_iso(origin, value, 480.0, 0.0);
        let c = project_iso(origin, -480.0, value, 0.0);
        let d = project_iso(origin, 480.0, value, 0.0);
        frame.stroke(
            &CanvasPath::line(a, b),
            Stroke::default()
                .with_color(color(0x363c46))
                .with_width(1.0),
        );
        frame.stroke(
            &CanvasPath::line(c, d),
            Stroke::default()
                .with_color(color(0x363c46))
                .with_width(1.0),
        );
    }

    frame.stroke(
        &CanvasPath::line(
            project_iso(origin, -520.0, 0.0, 0.0),
            project_iso(origin, 520.0, 0.0, 0.0),
        ),
        Stroke::default()
            .with_color(color(0xc05252))
            .with_width(2.0),
    );
    frame.stroke(
        &CanvasPath::line(
            project_iso(origin, 0.0, -520.0, 0.0),
            project_iso(origin, 0.0, 520.0, 0.0),
        ),
        Stroke::default()
            .with_color(color(0x5ab56f))
            .with_width(2.0),
    );
    frame.stroke(
        &CanvasPath::line(origin, project_iso(origin, 0.0, 0.0, 180.0)),
        Stroke::default()
            .with_color(color(0x5b8ee6))
            .with_width(2.0),
    );
}

fn project_iso(origin: Point, x: f32, y: f32, z: f32) -> Point {
    Point::new(origin.x + (x - y) * 0.72, origin.y + (x + y) * 0.36 - z)
}

fn draw_axis(frame: &mut Frame, origin: Point, vector: Vector, axis_color: Color) {
    let end = Point::new(origin.x + vector.x, origin.y + vector.y);
    frame.stroke(
        &CanvasPath::line(origin, end),
        Stroke::default().with_color(axis_color).with_width(2.0),
    );
    frame.fill(&CanvasPath::circle(end, 5.0), axis_color);
}

fn draw_rotation_ring(frame: &mut Frame, origin: Point, radius: f32, ring_color: Color) {
    frame.stroke(
        &CanvasPath::circle(origin, radius),
        Stroke::default()
            .with_color(ring_color.scale_alpha(0.78))
            .with_width(2.0),
    );
    frame.fill(&CanvasPath::circle(origin, 4.0), color(0xf3f6fb));
}

fn draw_ellipse_ring(
    frame: &mut Frame,
    origin: Point,
    radii: Vector,
    rotation: f32,
    ring_color: Color,
) {
    let path = CanvasPath::new(|path| {
        for index in 0..=72 {
            let angle = index as f32 / 72.0 * std::f32::consts::TAU;
            let local = Vector::new(angle.cos() * radii.x, angle.sin() * radii.y);
            let rotated = rotate_vector(local, rotation);
            let point = add_vector(origin, rotated);
            if index == 0 {
                path.move_to(point);
            } else {
                path.line_to(point);
            }
        }
    });

    frame.stroke(
        &path,
        Stroke::default()
            .with_color(ring_color.scale_alpha(0.78))
            .with_width(2.0),
    );
}

fn draw_scale_handle(
    frame: &mut Frame,
    origin: Point,
    vector: Vector,
    handle_color: Color,
    scale: f32,
) {
    let end = Point::new(origin.x + vector.x, origin.y + vector.y);
    frame.stroke(
        &CanvasPath::line(origin, end),
        Stroke::default().with_color(handle_color).with_width(2.0),
    );
    draw_scale_square(frame, end, 9.0 * scale, handle_color);
}

fn draw_scale_square(frame: &mut Frame, center: Point, size: f32, handle_color: Color) {
    let half = size * 0.5;
    let rect = CanvasPath::rectangle(
        Point::new(center.x - half, center.y - half),
        Size::new(size, size),
    );
    frame.fill(&rect, handle_color);
    frame.stroke(
        &rect,
        Stroke::default()
            .with_color(color(0xf3f6fb).scale_alpha(0.75))
            .with_width(1.0),
    );
}

fn hit_move_gizmo_2d(origin: Point, position: Point, scale: f32) -> Option<GizmoHandle> {
    if distance(origin, position) <= 10.0 * scale {
        return Some(GizmoHandle::MoveFree);
    }
    if distance_to_segment(
        position,
        origin,
        add_vector(origin, scale_vector(Vector::new(64.0, 0.0), scale)),
    ) <= 8.0 * scale
    {
        return Some(GizmoHandle::MoveX);
    }
    if distance_to_segment(
        position,
        origin,
        add_vector(origin, scale_vector(Vector::new(0.0, 64.0), scale)),
    ) <= 8.0 * scale
    {
        return Some(GizmoHandle::MoveY);
    }
    None
}

fn hit_move_gizmo_3d(origin: Point, position: Point, scale: f32) -> Option<GizmoHandle> {
    if distance(origin, position) <= 10.0 * scale {
        return Some(GizmoHandle::MoveFree);
    }
    if distance_to_segment(
        position,
        origin,
        add_vector(origin, scale_vector(Vector::new(66.0, 33.0), scale)),
    ) <= 8.0 * scale
    {
        return Some(GizmoHandle::MoveX);
    }
    if distance_to_segment(
        position,
        origin,
        add_vector(origin, scale_vector(Vector::new(-66.0, 33.0), scale)),
    ) <= 8.0 * scale
    {
        return Some(GizmoHandle::MoveY);
    }
    if distance_to_segment(
        position,
        origin,
        add_vector(origin, scale_vector(Vector::new(0.0, -70.0), scale)),
    ) <= 8.0 * scale
    {
        return Some(GizmoHandle::MoveZ);
    }
    None
}

fn hit_rotate_gizmo_2d(origin: Point, position: Point, scale: f32) -> Option<GizmoHandle> {
    let radius = distance(origin, position);
    if (radius - 45.0 * scale).abs() <= 8.0 * scale {
        return Some(GizmoHandle::RotateZ);
    }
    None
}

fn hit_rotate_gizmo_3d(origin: Point, position: Point, scale: f32) -> Option<GizmoHandle> {
    if distance_to_rotated_ellipse(
        position,
        origin,
        Vector::new(54.0 * scale, 17.0 * scale),
        0.45,
    ) <= 8.0 * scale
    {
        return Some(GizmoHandle::RotateX);
    }
    if distance_to_rotated_ellipse(
        position,
        origin,
        Vector::new(54.0 * scale, 17.0 * scale),
        -0.45,
    ) <= 8.0 * scale
    {
        return Some(GizmoHandle::RotateY);
    }
    let radius = distance(origin, position);
    if (radius - 48.0 * scale).abs() <= 8.0 * scale {
        return Some(GizmoHandle::RotateZ);
    }
    None
}

fn hit_scale_gizmo_2d(origin: Point, position: Point, scale: f32) -> Option<GizmoHandle> {
    if distance(origin, position) <= 10.0 * scale {
        return Some(GizmoHandle::ScaleUniform);
    }
    if distance(
        position,
        add_vector(origin, scale_vector(Vector::new(54.0, 0.0), scale)),
    ) <= 10.0 * scale
    {
        return Some(GizmoHandle::ScaleX);
    }
    if distance(
        position,
        add_vector(origin, scale_vector(Vector::new(0.0, 54.0), scale)),
    ) <= 10.0 * scale
    {
        return Some(GizmoHandle::ScaleY);
    }
    None
}

fn hit_scale_gizmo_3d(origin: Point, position: Point, scale: f32) -> Option<GizmoHandle> {
    if distance(origin, position) <= 10.0 * scale {
        return Some(GizmoHandle::ScaleUniform);
    }
    if distance(
        position,
        add_vector(origin, scale_vector(Vector::new(54.0, 27.0), scale)),
    ) <= 10.0 * scale
    {
        return Some(GizmoHandle::ScaleX);
    }
    if distance(
        position,
        add_vector(origin, scale_vector(Vector::new(-54.0, 27.0), scale)),
    ) <= 10.0 * scale
    {
        return Some(GizmoHandle::ScaleY);
    }
    if distance(
        position,
        add_vector(origin, scale_vector(Vector::new(0.0, -60.0), scale)),
    ) <= 10.0 * scale
    {
        return Some(GizmoHandle::ScaleZ);
    }
    None
}

fn distance(a: Point, b: Point) -> f32 {
    ((a.x - b.x).powi(2) + (a.y - b.y).powi(2)).sqrt()
}

fn distance_to_segment(point: Point, a: Point, b: Point) -> f32 {
    let ab = Vector::new(b.x - a.x, b.y - a.y);
    let ap = Vector::new(point.x - a.x, point.y - a.y);
    let length_sq = ab.x * ab.x + ab.y * ab.y;
    if length_sq <= f32::EPSILON {
        return distance(point, a);
    }

    let t = ((ap.x * ab.x + ap.y * ab.y) / length_sq).clamp(0.0, 1.0);
    let closest = Point::new(a.x + ab.x * t, a.y + ab.y * t);
    distance(point, closest)
}

fn distance_to_rotated_ellipse(point: Point, origin: Point, radii: Vector, rotation: f32) -> f32 {
    let offset = Vector::new(point.x - origin.x, point.y - origin.y);
    let local = rotate_vector(offset, -rotation);
    let angle = (local.y / radii.y.max(1.0)).atan2(local.x / radii.x.max(1.0));
    let closest = rotate_vector(
        Vector::new(angle.cos() * radii.x, angle.sin() * radii.y),
        rotation,
    );
    distance(point, add_vector(origin, closest))
}

fn add_vector(point: Point, vector: Vector) -> Point {
    Point::new(point.x + vector.x, point.y + vector.y)
}

fn scale_vector(vector: Vector, scale: f32) -> Vector {
    Vector::new(vector.x * scale, vector.y * scale)
}

fn rotate_vector(vector: Vector, angle: f32) -> Vector {
    let cos = angle.cos();
    let sin = angle.sin();
    Vector::new(
        vector.x * cos - vector.y * sin,
        vector.x * sin + vector.y * cos,
    )
}

fn point_in_rect(point: Point, x: f32, y: f32, width: f32, height: f32) -> bool {
    point.x >= x && point.x <= x + width && point.y >= y && point.y <= y + height
}

fn view_cube_origin(size: Size) -> Point {
    Point::new(size.width - 84.0, 28.0)
}

fn normalize_degrees(value: f32) -> f32 {
    let mut value = value % 360.0;
    if value < 0.0 {
        value += 360.0;
    }
    value
}

fn meta_row(label: &str, value: &str) -> Element<'static, Message> {
    row![
        text(label.to_string()).size(12).width(96),
        text(value.to_string()).size(12).width(Length::Fill),
    ]
    .spacing(8)
    .into()
}

fn field_row(
    label: &str,
    value: String,
    message: impl Fn(String) -> Message + 'static,
) -> Element<'static, Message> {
    row![
        text(label.to_string()).size(12).width(72),
        text_input("", &value)
            .on_input(message)
            .style(Modern::text_input())
            .padding(6)
            .size(12)
            .width(Length::Fill),
    ]
    .spacing(8)
    .align_y(iced::Alignment::Center)
    .into()
}

fn number_row(
    label: &str,
    value: f32,
    field: TransformNumberField,
    step: f32,
) -> Element<'static, Message> {
    row![
        text(label.to_string()).size(12).width(72),
        number_input(&value, -100000.0..=100000.0, move |next| {
            Message::TransformNumberChanged(field, next)
        })
        .step(step)
        .set_size(12.0)
        .padding(6.0),
    ]
    .spacing(8)
    .align_y(iced::Alignment::Center)
    .into()
}

fn read_only_row(label: &str, value: &str) -> Element<'static, Message> {
    row![
        text(label.to_string()).size(12).width(72),
        container(text(value.to_string()).size(12))
            .padding(6)
            .width(Length::Fill)
            .style(|_theme| container::Style {
                background: Some(color(0xf3f4f6).into()),
                border: border::rounded(3),
                ..Default::default()
            }),
    ]
    .spacing(8)
    .align_y(iced::Alignment::Center)
    .into()
}

fn component_editor_list(items: &[String], draft: &str) -> Element<'static, Message> {
    let mut list = column![].spacing(6);
    if items.is_empty() {
        list = list.push(text("No components").size(12));
    } else {
        for (index, item) in items.iter().enumerate() {
            list = list.push(
                row![
                    text_input("component", item)
                        .on_input(move |value| Message::ComponentChanged(index, value))
                        .style(Modern::text_input())
                        .padding(6)
                        .size(12)
                        .width(Length::Fill),
                    button(text("Remove").size(12))
                        .style(Modern::danger_button())
                        .on_press(Message::RemoveComponent(index)),
                ]
                .spacing(8)
                .align_y(iced::Alignment::Center),
            );
        }
    }

    list = list.push(
        row![
            text_input("new component", draft)
                .on_input(Message::ComponentDraftChanged)
                .style(Modern::text_input())
                .padding(6)
                .size(12)
                .width(Length::Fill),
            button(text("Add").size(12))
                .style(Modern::primary_button())
                .on_press(Message::AddComponent),
        ]
        .spacing(8)
        .align_y(iced::Alignment::Center),
    );

    list.into()
}

fn view_mode_button(mode: ProjectViewMode, active: ProjectViewMode) -> Element<'static, Message> {
    button(text(mode.label()).size(12))
        .style(Modern::button(if mode == active {
            ButtonKind::Primary
        } else {
            ButtonKind::System
        }))
        .on_press(Message::ProjectViewModeSelected(mode))
        .into()
}

fn main_menu_button(menu: MainMenu, active: Option<MainMenu>) -> Element<'static, Message> {
    button(text(menu.label()).size(12))
        .style(Modern::button(if active == Some(menu) {
            ButtonKind::Primary
        } else {
            ButtonKind::Plain
        }))
        .on_press(Message::MainMenuSelected(menu))
        .into()
}

fn menu_command_button(label: &'static str, message: Message) -> Element<'static, Message> {
    button(text(label).size(12))
        .style(Modern::system_button())
        .on_press(message)
        .into()
}

fn icon_view(icon: Icon) -> Element<'static, Message> {
    text(char::from(icon).to_string())
        .font(Font::with_name("lucide"))
        .size(16)
        .into()
}

fn icon_button(icon: Icon, message: Message, kind: ButtonKind) -> Element<'static, Message> {
    button(icon_view(icon))
        .style(Modern::button(kind))
        .padding(6)
        .width(34)
        .height(30)
        .on_press(message)
        .into()
}

fn viewport_tool_button(
    icon: Icon,
    tool: ViewportTool,
    active: ViewportTool,
) -> Element<'static, Message> {
    button(icon_view(icon))
        .style(Modern::button(if tool == active {
            ButtonKind::Primary
        } else {
            ButtonKind::System
        }))
        .padding(6)
        .width(34)
        .height(30)
        .on_press(Message::ViewportToolSelected(tool))
        .into()
}

fn compact_toggle_button(icon: Icon, enabled: bool, message: Message) -> Element<'static, Message> {
    button(icon_view(icon))
        .style(Modern::button(if enabled {
            ButtonKind::Primary
        } else {
            ButtonKind::System
        }))
        .padding(6)
        .width(34)
        .height(30)
        .on_press(message)
        .into()
}

fn settings_toggle_button(
    label: &'static str,
    enabled: bool,
    message: Message,
) -> Element<'static, Message> {
    let label = if enabled {
        format!("{label}: on")
    } else {
        format!("{label}: off")
    };

    button(text(label).size(12))
        .style(Modern::button(if enabled {
            ButtonKind::Primary
        } else {
            ButtonKind::System
        }))
        .on_press(message)
        .into()
}

fn project_item_kind_label(kind: &ProjectTreeItemKind) -> &'static str {
    match kind {
        ProjectTreeItemKind::SceneCandidate => "Scene",
        ProjectTreeItemKind::Example => "Example",
        ProjectTreeItemKind::Asset => "Asset",
        ProjectTreeItemKind::Module => "Script",
    }
}

fn classify_open_file(path: &StdPath) -> OpenFileKind {
    let extension = path
        .extension()
        .and_then(|value| value.to_str())
        .unwrap_or_default()
        .to_ascii_lowercase();

    match extension.as_str() {
        "zt" | "ztproj" | "json" | "toml" | "ron" | "txt" | "md" | "rs" | "wgsl" => {
            OpenFileKind::Text
        }
        _ => OpenFileKind::AssetPreview,
    }
}

fn asset_preview_text(path: &str, error: &str) -> String {
    format!(
        "Asset preview\n\nPath: {path}\n\nThis file is not editable as text here.\nRead note: {error}\n"
    )
}

fn load_script_document() -> (PathBuf, String) {
    let path = workspace_path(DEFAULT_SCRIPT);
    let content = std::fs::read_to_string(&path).unwrap_or_else(|_| {
        "scene main {\n    // Script source will appear here when a file is selected.\n}\n"
            .to_string()
    });
    (path, content)
}

fn scene_bounds(scene: &SceneDocument) -> Option<(f32, f32, f32, f32)> {
    let first = scene.entities.first()?;
    let mut min_x = first.transform.x;
    let mut min_y = first.transform.y;
    let mut max_x = first.transform.x;
    let mut max_y = first.transform.y;

    for entity in &scene.entities {
        min_x = min_x.min(entity.transform.x);
        min_y = min_y.min(entity.transform.y);
        max_x = max_x.max(entity.transform.x);
        max_y = max_y.max(entity.transform.y);
    }

    Some((min_x, min_y, max_x, max_y))
}

fn resolve_project_file(project: &ProjectSummary, path: &str) -> PathBuf {
    let raw = PathBuf::from(path);
    if raw.exists() {
        return raw;
    }

    let root = PathBuf::from(&project.project_dir);
    let candidates = [
        root.join(path),
        root.join(&project.source_root).join(path),
        root.join("examples").join(path),
        root.join("assets").join(path),
        root.join("scenes").join(path),
    ];

    candidates
        .into_iter()
        .find(|candidate| candidate.exists())
        .unwrap_or_else(|| root.join(path))
}

fn unique_entity_id(scene: &SceneDocument, prefix: &str) -> String {
    let clean_prefix = prefix
        .chars()
        .map(|ch| {
            if ch.is_ascii_alphanumeric() || ch == '-' || ch == '_' {
                ch
            } else {
                '-'
            }
        })
        .collect::<String>()
        .trim_matches('-')
        .to_string();
    let base = if clean_prefix.is_empty() {
        "entity".to_string()
    } else {
        clean_prefix
    };

    for index in 1..10_000 {
        let candidate = format!("{base}-{index:02}");
        if scene
            .entities
            .iter()
            .all(|entity| entity.stable_id != candidate)
        {
            return candidate;
        }
    }

    format!("{base}-overflow")
}

fn snap_to_grid(value: f32) -> f32 {
    (value / SNAP_SIZE).round() * SNAP_SIZE
}

async fn run_project_command(editor_command: EditorCommand, project_path: String) -> CheckReport {
    let zt_exe = workspace_path("zt.exe");
    let mut command = if zt_exe.exists() {
        std::process::Command::new(&zt_exe)
    } else {
        std::process::Command::new("zt")
    };

    command.current_dir(workspace_path("."));
    for arg in editor_command.zt_args() {
        command.arg(arg);
    }
    command.arg(&project_path);
    if matches!(editor_command, EditorCommand::Check) {
        command.arg("--all");
    }

    let command_label = if matches!(editor_command, EditorCommand::Check) {
        format!("zt {} {} --all", editor_command.label(), project_path)
    } else {
        format!("zt {} {}", editor_command.label(), project_path)
    };
    match command.output() {
        Ok(output) => {
            let stdout = String::from_utf8_lossy(&output.stdout);
            let stderr = String::from_utf8_lossy(&output.stderr);
            let mut combined = String::new();
            if !stdout.trim().is_empty() {
                combined.push_str(stdout.trim());
            }
            if !stderr.trim().is_empty() {
                if !combined.is_empty() {
                    combined.push('\n');
                }
                combined.push_str(stderr.trim());
            }
            if combined.is_empty() {
                combined.push_str("No compiler output.");
            }

            CheckReport {
                ok: output.status.success(),
                command: command_label,
                output: combined,
            }
        }
        Err(error) => CheckReport {
            ok: false,
            command: command_label,
            output: format!("Could not start zt: {error}"),
        },
    }
}

fn tail_lines(value: &str, limit: usize) -> Vec<String> {
    let lines = value.lines().map(str::to_string).collect::<Vec<_>>();
    if lines.len() <= limit {
        return lines;
    }

    let start = lines.len() - limit;
    let mut visible = vec![format!("... {} earlier lines hidden", start)];
    visible.extend(lines.into_iter().skip(start));
    visible
}

#[cfg(feature = "terminal")]
fn create_terminal() -> (Option<iced_term::Terminal>, Option<String>) {
    let mut settings = iced_term::settings::Settings::default();
    settings.backend.program = "powershell.exe".to_string();
    settings.backend.args = vec!["-NoLogo".to_string()];
    settings.backend.working_directory = Some(workspace_path("."));

    match iced_term::Terminal::new(1, settings) {
        Ok(terminal) => (Some(terminal), Some("Terminal ready".to_string())),
        Err(error) => (None, Some(format!("Terminal start failed: {error}"))),
    }
}

fn workspace_path(path: &str) -> PathBuf {
    StdPath::new(env!("CARGO_MANIFEST_DIR"))
        .join("../..")
        .join(path)
}

fn normalize_path(path: &StdPath) -> String {
    path.to_string_lossy().replace('\\', "/")
}

fn color(hex: u32) -> Color {
    let r = ((hex >> 16) & 0xff) as f32 / 255.0;
    let g = ((hex >> 8) & 0xff) as f32 / 255.0;
    let b = (hex & 0xff) as f32 / 255.0;
    Color::from_rgb(r, g, b)
}
