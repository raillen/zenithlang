use crate::ipc::{ConnectionState, PreviewBridge};
use crate::messages::PreviewStatus;
use crate::project::{
    load_project_summary, ProjectSummary, ProjectTree, ProjectTreeItem, ProjectTreeItemKind,
};
use crate::scene_document::SceneEntity;
use crate::scene_document::{
    load_scene_document, save_scene_document, SceneDocument, SceneDocumentFormat, Transform2D,
};
use eframe::egui;
use std::collections::BTreeMap;
use std::path::PathBuf;

const TOP_BAR_ID: &str = "borealis_editor.top_bar";
const HIERARCHY_PROJECT_DOCK_ID: &str = "borealis_editor.hierarchy_project_dock";
const INSPECTOR_DOCK_ID: &str = "borealis_editor.inspector_dock";
const CONSOLE_DOCK_ID: &str = "borealis_editor.console_dock";
const VIEWPORT_ID: &str = "borealis_editor.scene_view";

const LABEL_APP_TITLE: &str = "Borealis";
const LABEL_HIERARCHY: &str = "Hierarchy";
const LABEL_PROJECT: &str = "Project";
const LABEL_INSPECTOR: &str = "Inspector";
const LABEL_SCENE_VIEW: &str = "Scene";
const LABEL_SCENE_3D: &str = "Scene 3D";
const LABEL_CONSOLE: &str = "Console";
const LABEL_GAME: &str = "Game";
const LABEL_TOOLBAR: &str = "Toolbar";

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum BottomDockTab {
    Project,
    Console,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum AssetViewMode {
    Icon,
    List,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum ViewportTab {
    Scene,
    Scene3D,
    Game,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum TransformTool {
    Hand,
    Move,
    Rotate,
    Scale,
}

#[derive(Debug, Clone, Copy, PartialEq)]
enum ViewportFocusRequest {
    None,
    Selected,
    All,
}

#[derive(Debug, Clone)]
struct ProjectDragPayload {
    label: String,
    path: String,
    kind: ProjectTreeItemKind,
}

pub struct BorealisEditorApp {
    preview: PreviewBridge,
    project_path: String,
    preview_path: String,
    scene_path: String,
    selected_entity_id: String,
    last_sent_line: String,
    transcript: Vec<String>,
    project_summary: Option<ProjectSummary>,
    project_tree: Option<ProjectTree>,
    project_error: Option<String>,
    selected_tree_item: String,
    scene_document: Option<SceneDocument>,
    scene_error: Option<String>,
    selected_scene_entity: String,
    viewport_drag_entity: Option<String>,
    viewport_drag_grab_offset: Option<(f32, f32)>,
    viewport_drag_moved: bool,
    scene_dirty: bool,
    auto_sync_preview: bool,
    bottom_dock_tab: BottomDockTab,
    asset_view_mode: AssetViewMode,
    hierarchy_filter: String,
    viewport_tab: ViewportTab,
    transform_tool: TransformTool,
    viewport_pan: egui::Vec2,
    viewport_zoom: f32,
    viewport_focus_request: ViewportFocusRequest,
}

#[cfg(test)]
#[derive(Debug, Clone, PartialEq, Eq)]
struct EditorSmokeSnapshot {
    project_path: String,
    scene_path: String,
    selected_entity_id: String,
    has_project_summary: bool,
    loaded_entity_count: usize,
    selected_entity_name: String,
    selected_entity_position: (i32, i32),
    scene_candidate_count: usize,
    dock_ids: Vec<&'static str>,
    auto_sync_preview: bool,
}

impl BorealisEditorApp {
    pub fn new(cc: &eframe::CreationContext<'_>) -> Self {
        Self::new_with_context(&cc.egui_ctx)
    }

    fn new_with_context(ctx: &egui::Context) -> Self {
        configure_editor_style(ctx);

        let mut app = Self {
            preview: PreviewBridge::new(),
            project_path: initial_project_path(),
            preview_path: String::new(),
            scene_path: "scenes/sample.scene.json".to_owned(),
            selected_entity_id: "player".to_owned(),
            last_sent_line: String::new(),
            transcript: vec![
                "Borealis Editor scaffold inicializado.".to_owned(),
                "Preview IPC v1: jsonl-stdio.".to_owned(),
                "Mock mode: ATIVO (simulando respostas do preview)".to_owned(),
                "Compile o preview: zt run preview/zenith.ztproj".to_owned(),
            ],
            project_summary: None,
            project_tree: None,
            project_error: None,
            selected_tree_item: String::new(),
            scene_document: None,
            scene_error: None,
            selected_scene_entity: String::new(),
            viewport_drag_entity: None,
            viewport_drag_grab_offset: None,
            viewport_drag_moved: false,
            scene_dirty: false,
            auto_sync_preview: true,
            bottom_dock_tab: BottomDockTab::Project,
            asset_view_mode: AssetViewMode::List,
            hierarchy_filter: String::new(),
            viewport_tab: ViewportTab::Scene,
            transform_tool: TransformTool::Move,
            viewport_pan: egui::vec2(80.0, 80.0),
            viewport_zoom: 1.0,
            viewport_focus_request: ViewportFocusRequest::All,
        };
        app.refresh_project_summary();
        app.load_current_scene_document();
        app
    }

    #[cfg(test)]
    fn smoke_snapshot(&self) -> EditorSmokeSnapshot {
        EditorSmokeSnapshot {
            project_path: self.project_path.clone(),
            scene_path: self.scene_path.clone(),
            selected_entity_id: self.selected_entity_id.clone(),
            has_project_summary: self.project_summary.is_some(),
            loaded_entity_count: self
                .scene_document
                .as_ref()
                .map(|document| document.entities.len())
                .unwrap_or_default(),
            selected_entity_name: self
                .selected_scene_entity_info()
                .map(|entity| entity.name.clone())
                .unwrap_or_default(),
            selected_entity_position: self
                .selected_scene_entity_info()
                .map(|entity| (entity.transform.x as i32, entity.transform.y as i32))
                .unwrap_or_default(),
            scene_candidate_count: self
                .project_summary
                .as_ref()
                .map(|summary| summary.scene_candidates.len())
                .unwrap_or_default(),
            dock_ids: vec![
                TOP_BAR_ID,
                HIERARCHY_PROJECT_DOCK_ID,
                INSPECTOR_DOCK_ID,
                CONSOLE_DOCK_ID,
                VIEWPORT_ID,
            ],
            auto_sync_preview: self.auto_sync_preview,
        }
    }

    fn refresh_project_summary(&mut self) {
        match load_project_summary(&self.project_path) {
            Ok(summary) => {
                self.scene_path = summary
                    .scene_candidates
                    .first()
                    .cloned()
                    .unwrap_or_else(|| self.scene_path.clone());
                self.project_error = None;
                self.project_tree = Some(ProjectTree::from_summary(&summary));
                self.project_summary = Some(summary.clone());
                self.push_transcript(format!(
                    "Projeto lido: {} ({}) com {} modulos",
                    summary.name, summary.kind, summary.module_count
                ));
            }
            Err(error) => {
                self.project_summary = None;
                self.project_tree = None;
                self.scene_document = None;
                self.project_error = Some(error.clone());
                self.push_transcript(format!("falha ao ler projeto: {error}"));
            }
        }
    }

    fn load_current_scene_document(&mut self) {
        let Some(summary) = &self.project_summary else {
            self.scene_error = Some("inspecione um projeto antes de carregar cena".to_string());
            return;
        };

        match load_scene_document(summary, &self.scene_path) {
            Ok(document) => {
                let first_entity = document.entities.first().cloned();
                if let Some(entity) = first_entity {
                    self.set_selected_scene_entity(&entity.stable_id, &entity.name, false);
                } else {
                    self.selected_scene_entity.clear();
                    self.selected_entity_id.clear();
                }
                self.scene_error = None;
                self.scene_dirty = false;
                self.push_transcript(format!(
                    "Cena lida: {} com {} entidades",
                    document.name,
                    document.entities.len()
                ));
                self.viewport_focus_request = ViewportFocusRequest::All;
                self.scene_document = Some(document);
            }
            Err(error) => {
                self.scene_document = None;
                self.scene_error = Some(error.clone());
                self.scene_dirty = false;
                self.push_transcript(format!("falha ao ler cena: {error}"));
            }
        }
    }

    fn set_selected_scene_entity(&mut self, stable_id: &str, entity_name: &str, log: bool) {
        self.selected_scene_entity = stable_id.to_string();
        self.selected_entity_id = stable_id.to_string();
        if log {
            self.push_transcript(format!(
                "Entidade selecionada: {} ({})",
                entity_name, stable_id
            ));
        }
    }

    fn render_project_summary(&self, ui: &mut egui::Ui) {
        section_caption(ui, "Project summary");

        if let Some(error) = &self.project_error {
            ui.colored_label(ui.visuals().warn_fg_color, error);
            return;
        }

        let Some(summary) = &self.project_summary else {
            ui.label("Nenhum resumo carregado ainda.");
            return;
        };

        kv_row(ui, "Name", &summary.name);
        kv_row(ui, "Version", &summary.version);
        kv_row(ui, "Type", &summary.kind);
        kv_row(ui, "File", &summary.project_file);
        kv_row(ui, "Folder", &summary.project_dir);
        kv_row(ui, "Namespace", &summary.root_namespace);
        kv_row(ui, "Source", &summary.source_root);
        kv_row(ui, "A11y", &summary.accessibility_profile);
        kv_row(
            ui,
            "Telemetry",
            match summary.telemetry {
                Some(true) => "ativa",
                Some(false) => "desligada",
                None => "nao informada",
            },
        );
        kv_row(ui, "Modules", summary.module_count.to_string());
        kv_row(ui, "Examples", summary.example_count.to_string());
        kv_row(ui, "Assets", summary.asset_count.to_string());
        kv_row(
            ui,
            "Game API",
            format!(
                "{}/{}/{}",
                yes_no(summary.has_scene_module),
                yes_no(summary.has_editor_module),
                yes_no(summary.has_assets_module)
            ),
        );

        render_string_list(ui, "Arquivos de cena candidatos", &summary.scene_candidates);
        render_string_list(ui, "Exemplos candidatos", &summary.example_candidates);
        render_string_list(ui, "Assets candidatos", &summary.asset_candidates);
    }

    fn render_project_tree(&mut self, ui: &mut egui::Ui) {
        ui.push_id("project_tree", |ui| {
            let Some(tree) = self.project_tree.clone() else {
                ui.label("Inspecione um projeto para montar a arvore.");
                return;
            };

            for section in tree.sections {
                let section_title = section.title.clone();
                egui::CollapsingHeader::new(
                    egui::RichText::new(section_title.clone())
                        .size(11.5)
                        .strong()
                        .color(color_text()),
                )
                .default_open(true)
                .show(ui, |ui| {
                    if section.items.is_empty() {
                        ui.label(
                            egui::RichText::new("(empty)")
                                .size(11.0)
                                .monospace()
                                .color(color_text_faint()),
                        );
                        return;
                    }

                    if self.asset_view_mode == AssetViewMode::Icon && section_title == "Assets" {
                        ui.horizontal_wrapped(|ui| {
                            for item in section.items {
                                let selected = self.selected_tree_item == item.id;
                                let response = ui.add_sized(
                                    [118.0, 36.0],
                                    egui::Button::new(
                                        egui::RichText::new(format!("asset\n{}", item.label))
                                            .size(10.5)
                                            .monospace()
                                            .color(if selected {
                                                rgb(238, 245, 255)
                                            } else {
                                                color_text()
                                            }),
                                    )
                                    .fill(if selected {
                                        color_accent()
                                    } else {
                                        color_field()
                                    })
                                    .corner_radius(egui::CornerRadius::same(2)),
                                );
                                self.handle_project_item_response(response, &item);
                            }
                        });
                        return;
                    }

                    for item in section.items {
                        let selected = self.selected_tree_item == item.id;
                        let icon = match item.kind {
                            ProjectTreeItemKind::SceneCandidate => "scene",
                            ProjectTreeItemKind::Example => "ex",
                            ProjectTreeItemKind::Asset => "asset",
                            ProjectTreeItemKind::Module => "mod",
                        };
                        let response = ui.selectable_label(
                            selected,
                            egui::RichText::new(format!("{icon:<5} {}", item.label))
                                .size(11.0)
                                .monospace()
                                .color(if selected {
                                    rgb(238, 245, 255)
                                } else {
                                    color_text()
                                }),
                        );
                        self.handle_project_item_response(response, &item);
                    }
                });
            }
        });
    }

    fn handle_project_item_response(&mut self, response: egui::Response, item: &ProjectTreeItem) {
        if response.clicked() {
            self.selected_tree_item = item.id.clone();
            match &item.kind {
                ProjectTreeItemKind::SceneCandidate
                | ProjectTreeItemKind::Example
                | ProjectTreeItemKind::Module => {
                    self.scene_path = item.path.clone();
                }
                ProjectTreeItemKind::Asset => {}
            }
            self.push_transcript(format!("Selecionado na arvore: {}", item.path));
        }
        response.dnd_set_drag_payload(ProjectDragPayload {
            label: item.label.clone(),
            path: item.path.clone(),
            kind: item.kind.clone(),
        });
        response.on_hover_text(&item.path);
    }

    fn selected_project_item(&self) -> Option<ProjectTreeItem> {
        self.project_tree
            .as_ref()
            .and_then(|tree| tree.find_item(&self.selected_tree_item))
    }

    fn selected_scene_entity_info(&self) -> Option<&SceneEntity> {
        let document = self.scene_document.as_ref()?;
        document
            .entities
            .iter()
            .find(|entity| entity.stable_id == self.selected_scene_entity)
    }

    fn scene_entity_info(&self, stable_id: &str) -> Option<&SceneEntity> {
        let document = self.scene_document.as_ref()?;
        document
            .entities
            .iter()
            .find(|entity| entity.stable_id == stable_id)
    }

    fn scene_entity_info_mut(&mut self, stable_id: &str) -> Option<&mut SceneEntity> {
        let document = self.scene_document.as_mut()?;
        document
            .entities
            .iter_mut()
            .find(|entity| entity.stable_id == stable_id)
    }

    fn scene_document_mut(&mut self) -> Option<&mut SceneDocument> {
        self.scene_document.as_mut()
    }

    fn preview_connected(&self) -> bool {
        self.preview.state() != ConnectionState::Disconnected
    }

    fn ensure_scene_change_is_safe(&mut self, action: &str) -> bool {
        if !self.scene_dirty {
            return true;
        }

        self.push_transcript(format!(
            "Cena atual tem alteracoes nao salvas. Use Save Scene antes de {action}."
        ));
        false
    }

    fn clear_scene_selection(&mut self) {
        self.selected_scene_entity.clear();
        self.selected_entity_id.clear();
    }

    fn request_focus_selected_in_viewport(&mut self) {
        self.viewport_focus_request = ViewportFocusRequest::Selected;
    }

    fn request_frame_all_in_viewport(&mut self) {
        self.viewport_focus_request = ViewportFocusRequest::All;
    }

    fn center_viewport_on_scene_point(&mut self, rect: egui::Rect, x: f32, y: f32) {
        let center = rect.center();
        self.viewport_pan = egui::vec2(
            center.x - rect.left() - (x * self.viewport_zoom),
            center.y - rect.top() - (y * self.viewport_zoom),
        );
    }

    fn frame_scene_entities_in_viewport(&mut self, rect: egui::Rect) {
        let Some(document) = self.scene_document.as_ref() else {
            return;
        };
        if document.entities.is_empty() {
            return;
        }

        let mut min_x = f32::INFINITY;
        let mut min_y = f32::INFINITY;
        let mut max_x = f32::NEG_INFINITY;
        let mut max_y = f32::NEG_INFINITY;

        for entity in &document.entities {
            min_x = min_x.min(entity.transform.x);
            min_y = min_y.min(entity.transform.y);
            max_x = max_x.max(entity.transform.x);
            max_y = max_y.max(entity.transform.y);
        }

        let scene_width = (max_x - min_x).max(160.0);
        let scene_height = (max_y - min_y).max(120.0);
        let available_width = (rect.width() - 180.0).max(120.0);
        let available_height = (rect.height() - 140.0).max(120.0);

        self.viewport_zoom = (available_width / scene_width)
            .min(available_height / scene_height)
            .clamp(0.35, 2.75);
        self.center_viewport_on_scene_point(rect, (min_x + max_x) * 0.5, (min_y + max_y) * 0.5);
    }

    fn apply_viewport_focus_request(&mut self, rect: egui::Rect) {
        match self.viewport_focus_request {
            ViewportFocusRequest::None => {}
            ViewportFocusRequest::Selected => {
                if let Some((x, y)) = self
                    .selected_scene_entity_info()
                    .map(|entity| (entity.transform.x, entity.transform.y))
                {
                    self.viewport_zoom = self.viewport_zoom.max(1.0);
                    self.center_viewport_on_scene_point(rect, x, y);
                }
            }
            ViewportFocusRequest::All => self.frame_scene_entities_in_viewport(rect),
        }

        self.viewport_focus_request = ViewportFocusRequest::None;
    }

    fn update_scene_entity_position(&mut self, stable_id: &str, x: f32, y: f32) {
        if let Some(entity) = self.scene_entity_info_mut(stable_id) {
            entity.transform.x = x;
            entity.transform.y = y;
        }
        self.scene_dirty = true;
    }

    fn add_component_to_entity(&mut self, stable_id: &str, component: &str) {
        let mut entity_name = None;
        let mut changed = false;

        if let Some(entity) = self.scene_entity_info_mut(stable_id) {
            entity_name = Some(entity.name.clone());
            if !entity
                .components
                .iter()
                .any(|existing| existing == component)
            {
                entity.components.push(component.to_string());
                changed = true;
            }
        }

        if changed {
            self.scene_dirty = true;
            self.push_transcript(format!(
                "Component adicionado: {} -> {}",
                entity_name.unwrap_or_else(|| stable_id.to_string()),
                component
            ));
        } else {
            self.push_transcript(format!(
                "Componente ja presente em {}: {}",
                entity_name.unwrap_or_else(|| stable_id.to_string()),
                component
            ));
        }
    }

    fn remove_component_from_entity(&mut self, stable_id: &str, component_index: usize) {
        let mut removed_component = None;
        let mut entity_name = None;

        if let Some(entity) = self.scene_entity_info_mut(stable_id) {
            entity_name = Some(entity.name.clone());
            if component_index < entity.components.len() {
                removed_component = Some(entity.components.remove(component_index));
            }
        }

        if let Some(component) = removed_component {
            self.scene_dirty = true;
            self.push_transcript(format!(
                "Component removido: {} -> {}",
                entity_name.unwrap_or_else(|| stable_id.to_string()),
                component
            ));
        }
    }

    fn apply_scene_document_edit(&mut self, name: String, document_id: String) {
        if let Some(document) = self.scene_document_mut() {
            document.name = name;
            document.document_id = document_id;
        }
        self.scene_dirty = true;
    }

    fn apply_scene_entity_edit(
        &mut self,
        stable_id: &str,
        name: String,
        layer: String,
        parent: String,
        tags: Vec<String>,
        transform: Transform2D,
        model_asset: String,
    ) {
        if let Some(entity) = self.scene_entity_info_mut(stable_id) {
            entity.name = name;
            entity.layer = layer;
            entity.parent = parent;
            entity.tags = tags;
            entity.transform = transform;
            entity.model_asset = model_asset;
        }
        self.scene_dirty = true;
    }

    fn next_scene_entity_index(&self) -> usize {
        let Some(document) = self.scene_document.as_ref() else {
            return 1;
        };

        let mut index = document.entities.len() + 1;
        loop {
            let stable_id = format!("entity-{index:02}");
            if document
                .entities
                .iter()
                .all(|entity| entity.stable_id != stable_id)
            {
                return index;
            }
            index += 1;
        }
    }

    fn create_scene_entity_in_model(&mut self) {
        let index = self.next_scene_entity_index();
        let stable_id = format!("entity-{index:02}");
        let name = format!("Entity {index}");

        let Some(document) = self.scene_document.as_mut() else {
            self.push_transcript("nenhuma cena carregada para criar entidade");
            return;
        };
        let entity = SceneEntity {
            stable_id: stable_id.clone(),
            name: name.clone(),
            layer: "default".to_string(),
            parent: String::new(),
            tags: Vec::new(),
            components: Vec::new(),
            model_asset: String::new(),
            transform: Transform2D {
                x: 96.0 + (index as f32 * 24.0),
                y: 96.0 + (index as f32 * 16.0),
                ..Transform2D::default()
            },
        };

        document.entities.push(entity);
        self.set_selected_scene_entity(&stable_id, &name, true);
        self.scene_dirty = true;
        self.request_focus_selected_in_viewport();
        self.push_transcript(format!("Entidade criada: {} ({})", name, stable_id));
    }

    fn create_scene_primitive_in_model(&mut self, component: &str, name: &str) {
        let index = self.next_scene_entity_index();
        let stable_id = format!("entity-{index:02}");
        let x = 96.0 + (index as f32 * 28.0);
        let y = 96.0 + (index as f32 * 18.0);

        let Some(document) = self.scene_document.as_mut() else {
            self.push_transcript("nenhuma cena carregada para criar objeto 3D");
            return;
        };

        let entity = SceneEntity {
            stable_id: stable_id.clone(),
            name: name.to_string(),
            layer: "world3d".to_string(),
            parent: String::new(),
            tags: vec!["3d".to_string()],
            components: vec![component.to_string()],
            model_asset: String::new(),
            transform: Transform2D {
                x,
                y,
                z: if component == "camera3d" { 96.0 } else { 0.0 },
                ..Transform2D::default()
            },
        };

        document.entities.push(entity);
        self.viewport_tab = ViewportTab::Scene3D;
        self.set_selected_scene_entity(&stable_id, name, true);
        self.scene_dirty = true;
        self.request_focus_selected_in_viewport();
        self.push_transcript(format!("Objeto 3D criado: {} ({})", name, stable_id));
    }

    fn create_scene_entity_from_asset(
        &mut self,
        payload: &ProjectDragPayload,
        x: f32,
        y: f32,
        z: f32,
    ) {
        let index = self.next_scene_entity_index();
        let stable_id = format!("entity-{index:02}");
        let name = asset_entity_name(&payload.label);
        let is_model = asset_is_model3d(&payload.path);
        let component = if is_model { "model3d" } else { "sprite" };

        let Some(document) = self.scene_document.as_mut() else {
            self.push_transcript("nenhuma cena carregada para receber asset");
            return;
        };

        let entity = SceneEntity {
            stable_id: stable_id.clone(),
            name: name.clone(),
            layer: if is_model {
                "world3d".to_string()
            } else {
                "default".to_string()
            },
            parent: String::new(),
            tags: if is_model {
                vec!["3d".to_string(), "asset".to_string()]
            } else {
                vec!["asset".to_string()]
            },
            components: vec![component.to_string()],
            model_asset: if is_model {
                payload.path.clone()
            } else {
                String::new()
            },
            transform: Transform2D {
                x,
                y,
                z,
                ..Transform2D::default()
            },
        };

        document.entities.push(entity);
        if is_model {
            self.viewport_tab = ViewportTab::Scene3D;
        }
        self.set_selected_scene_entity(&stable_id, &name, true);
        self.scene_dirty = true;
        self.push_transcript(format!(
            "Asset solto na cena: {} -> {}",
            payload.path, stable_id
        ));
    }

    fn remove_selected_scene_entity_from_model(&mut self) {
        if self.selected_scene_entity.is_empty() {
            self.push_transcript("nenhuma entidade selecionada para remover");
            return;
        }

        let removed_id = self.selected_scene_entity.clone();
        let Some(document) = self.scene_document.as_mut() else {
            self.push_transcript("nenhuma cena carregada para remover entidade");
            return;
        };

        let Some(index) = document
            .entities
            .iter()
            .position(|entity| entity.stable_id == removed_id)
        else {
            self.push_transcript("entidade selecionada nao foi encontrada na cena");
            return;
        };

        let removed = document.entities.remove(index);
        for entity in &mut document.entities {
            if entity.parent == removed.stable_id {
                entity.parent.clear();
            }
        }

        let next_selection = document
            .entities
            .get(index)
            .or_else(|| document.entities.last())
            .map(|entity| (entity.stable_id.clone(), entity.name.clone()));

        self.scene_dirty = true;
        self.push_transcript(format!(
            "Entidade removida: {} ({})",
            removed.name, removed.stable_id
        ));

        if let Some((stable_id, name)) = next_selection {
            self.set_selected_scene_entity(&stable_id, &name, false);
        } else {
            self.selected_scene_entity.clear();
            self.selected_entity_id.clear();
        }
    }

    fn save_scene_document_internal(&mut self) -> bool {
        let Some(document) = self.scene_document.clone() else {
            self.push_transcript("nenhuma cena carregada para salvar");
            return false;
        };

        match save_scene_document(&document) {
            Ok(()) => {
                self.scene_dirty = false;
                self.scene_error = None;
                self.push_transcript(format!("Cena salva: {}", document.source_path));
                true
            }
            Err(error) => {
                self.scene_error = Some(error.clone());
                self.push_transcript(format!("falha ao salvar cena: {error}"));
                false
            }
        }
    }

    fn save_scene_document_from_ui(&mut self) {
        if self.save_scene_document_internal() && self.auto_sync_preview && self.preview_connected()
        {
            self.sync_scene_to_preview_from_ui();
        }
    }

    fn sync_scene_to_preview_from_ui(&mut self) {
        if self.scene_dirty {
            self.push_transcript(
                "Cena local esta dirty. Salvando antes de sincronizar com o preview.",
            );
            if !self.save_scene_document_internal() {
                return;
            }
        }

        if !self.preview_connected() {
            self.push_transcript("preview ainda nao foi iniciado");
            return;
        }

        match self.preview.open_scene(self.scene_path.clone()) {
            Ok(_) => {
                self.flush_preview_messages();
                self.check_preview_status();
                self.push_transcript(format!(
                    "Cena sincronizada com preview: {}",
                    self.scene_path
                ));
            }
            Err(error) => self.push_transcript(format!("erro ao sincronizar cena: {error}")),
        }
    }

    fn activate_scene_from_ui(&mut self, scene_path: String, open_in_preview: bool) {
        let action = if open_in_preview {
            "abrir outra cena"
        } else {
            "trocar de cena"
        };
        if !self.ensure_scene_change_is_safe(action) {
            return;
        }

        self.scene_path = scene_path;
        self.load_current_scene_document();
        self.push_transcript(format!("Cena ativa no editor: {}", self.scene_path));

        if open_in_preview {
            self.sync_scene_to_preview_from_ui();
            if !self.selected_entity_id.is_empty() {
                self.select_entity_from_ui();
            }
        }
    }

    fn finish_viewport_drag_if_needed(&mut self, ui: &egui::Ui) {
        if self.viewport_drag_entity.is_none() || ui.input(|input| input.pointer.primary_down()) {
            return;
        }

        if self.viewport_drag_moved {
            if let Some(entity_id) = self.viewport_drag_entity.clone() {
                if let Some(entity) = self.scene_entity_info(&entity_id) {
                    self.push_transcript(format!(
                        "Entidade movida: {} -> ({:.1}, {:.1})",
                        entity.name, entity.transform.x, entity.transform.y
                    ));
                }
            }
        }

        self.viewport_drag_entity = None;
        self.viewport_drag_grab_offset = None;
        self.viewport_drag_moved = false;
    }

    fn render_inspector(&mut self, ui: &mut egui::Ui) {
        ui.push_id("inspector", |ui| {
            self.render_entity_inspector(ui);
            ui.separator();

            let Some(item) = self.selected_project_item() else {
                section_caption(ui, "Project selection");
                ui.label(
                    egui::RichText::new("Selecione um item na arvore do projeto.")
                        .size(11.0)
                        .color(color_text_dim()),
                );
                return;
            };

            section_caption(ui, "Project selection");
            kv_row(ui, "Name", &item.label);
            kv_row(ui, "Type", tree_item_kind_label(&item.kind));
            kv_row(ui, "Path", &item.path);

            ui.add_space(8.0);
            match item.kind {
                ProjectTreeItemKind::SceneCandidate
                | ProjectTreeItemKind::Example
                | ProjectTreeItemKind::Module => {
                    ui.horizontal(|ui| {
                        if mini_button(ui, "Use", "Usar como cena ativa").clicked() {
                            self.activate_scene_from_ui(item.path.clone(), false);
                        }

                        if mini_button(ui, "Open", "Abrir cena no preview").clicked() {
                            self.activate_scene_from_ui(item.path.clone(), true);
                        }
                    });
                }
                ProjectTreeItemKind::Asset => {
                    ui.label(
                        egui::RichText::new("Asset detectado pelo projeto.")
                            .size(11.0)
                            .color(color_text_dim()),
                    );
                    if mini_button(ui, "Log", "Registrar asset no transcript").clicked() {
                        self.push_transcript(format!("Asset selecionado: {}", item.path));
                    }
                }
            }
        });
    }

    fn render_entity_inspector(&mut self, ui: &mut egui::Ui) {
        section_caption(ui, "Entity");

        let Some(entity) = self.selected_scene_entity_info().cloned() else {
            ui.label(
                egui::RichText::new("Nenhuma entidade selecionada.")
                    .size(11.0)
                    .color(color_text_dim()),
            );
            return;
        };

        let mut name = entity.name.clone();
        let mut layer = entity.layer.clone();
        let mut parent = entity.parent.clone();
        let mut tags = entity.tags.join(", ");
        let mut x = entity.transform.x;
        let mut y = entity.transform.y;
        let mut z = entity.transform.z;
        let mut rotation = entity.transform.rotation;
        let mut rotation_x = entity.transform.rotation_x;
        let mut rotation_y = entity.transform.rotation_y;
        let mut scale_x = entity.transform.scale_x;
        let mut scale_y = entity.transform.scale_y;
        let mut scale_z = entity.transform.scale_z;
        let mut model_asset = entity.model_asset.clone();
        let mut changed = false;

        changed |= inspector_text_field(ui, "Name", &mut name);
        kv_row(ui, "Stable ID", &entity.stable_id);
        changed |= inspector_text_field(ui, "Layer", &mut layer);
        changed |= inspector_text_field(ui, "Parent", &mut parent);
        changed |= inspector_text_field(ui, "Tags", &mut tags);

        section_caption(ui, "Transform");
        ui.horizontal(|ui| {
            ui.add_sized(
                [68.0, 20.0],
                egui::Label::new(
                    egui::RichText::new("Position")
                        .size(11.0)
                        .color(color_text_dim()),
                ),
            );
            changed |= ui
                .add(egui::DragValue::new(&mut x).speed(1.0).prefix("x "))
                .changed();
            changed |= ui
                .add(egui::DragValue::new(&mut y).speed(1.0).prefix("y "))
                .changed();
            changed |= ui
                .add(egui::DragValue::new(&mut z).speed(1.0).prefix("z "))
                .changed();
        });

        ui.horizontal(|ui| {
            ui.add_sized(
                [68.0, 20.0],
                egui::Label::new(
                    egui::RichText::new("Scale")
                        .size(11.0)
                        .color(color_text_dim()),
                ),
            );
            changed |= ui
                .add(egui::DragValue::new(&mut scale_x).speed(0.05).prefix("x "))
                .changed();
            changed |= ui
                .add(egui::DragValue::new(&mut scale_y).speed(0.05).prefix("y "))
                .changed();
            changed |= ui
                .add(egui::DragValue::new(&mut scale_z).speed(0.05).prefix("z "))
                .changed();
        });

        ui.horizontal(|ui| {
            ui.add_sized(
                [68.0, 20.0],
                egui::Label::new(
                    egui::RichText::new("Rotation")
                        .size(11.0)
                        .color(color_text_dim()),
                ),
            );
            changed |= ui
                .add(
                    egui::DragValue::new(&mut rotation_x)
                        .speed(1.0)
                        .prefix("x "),
                )
                .changed();
            changed |= ui
                .add(
                    egui::DragValue::new(&mut rotation_y)
                        .speed(1.0)
                        .prefix("y "),
                )
                .changed();
            changed |= ui
                .add(egui::DragValue::new(&mut rotation).speed(1.0).prefix("z "))
                .changed();
        });

        if entity_is_3d(&entity) {
            section_caption(ui, "3D Asset");
            changed |= inspector_text_field(ui, "Model", &mut model_asset);
        }

        let mut remove_component = None;
        let mut add_component = None;

        section_caption(ui, "Components");
        if entity.components.is_empty() {
            ui.label(
                egui::RichText::new("Nenhum component adicionado.")
                    .size(11.0)
                    .color(color_text_dim()),
            );
        } else {
            for (index, component) in entity.components.iter().enumerate() {
                egui::Frame::new()
                    .fill(color_field())
                    .inner_margin(egui::Margin::symmetric(6, 3))
                    .stroke(egui::Stroke::new(1.0, color_border()))
                    .show(ui, |ui| {
                        ui.horizontal(|ui| {
                            ui.label(
                                egui::RichText::new(component)
                                    .size(11.0)
                                    .monospace()
                                    .color(color_text()),
                            );
                            ui.with_layout(
                                egui::Layout::right_to_left(egui::Align::Center),
                                |ui| {
                                    if mini_button(ui, "X", "Remover component").clicked() {
                                        remove_component = Some(index);
                                    }
                                },
                            );
                        });
                    });
            }
        }

        ui.menu_button("Add Component", |ui| {
            for component in [
                "sprite",
                "controller",
                "camera2d",
                "ai",
                "collider2d",
                "script",
                "model3d",
                "cube3d",
                "camera3d",
                "light3d",
            ] {
                if ui.button(component).clicked() {
                    add_component = Some(component.to_string());
                    ui.close();
                }
            }
        });

        if changed {
            self.apply_scene_entity_edit(
                &entity.stable_id,
                name,
                layer,
                parent,
                parse_tags_input(&tags),
                Transform2D {
                    x,
                    y,
                    z,
                    rotation,
                    rotation_x,
                    rotation_y,
                    scale_x,
                    scale_y,
                    scale_z,
                },
                model_asset,
            );
        }

        if let Some(index) = remove_component {
            self.remove_component_from_entity(&entity.stable_id, index);
        }

        if let Some(component) = add_component {
            self.add_component_to_entity(&entity.stable_id, &component);
        }

        ui.horizontal(|ui| {
            if mini_button(ui, "Select", "Selecionar no preview").clicked() {
                self.selected_entity_id = entity.stable_id.clone();
                self.select_entity_from_ui();
            }

            if mini_button(ui, "Delete", "Remover entidade da cena").clicked() {
                self.remove_selected_scene_entity_from_model();
            }
        });
    }

    fn render_scene_settings(&mut self, ui: &mut egui::Ui) {
        section_caption(ui, "Scene");

        let Some(document) = self.scene_document.clone() else {
            ui.label(
                egui::RichText::new("Nenhuma cena carregada.")
                    .size(11.0)
                    .color(color_text_dim()),
            );
            return;
        };

        let mut scene_name = document.name.clone();
        let mut document_id = document.document_id.clone();
        let mut changed = false;

        ui.horizontal(|ui| {
            status_chip(
                ui,
                if self.scene_dirty { "dirty" } else { "saved" },
                if self.scene_dirty {
                    color_warning()
                } else {
                    color_play()
                },
            );
            status_chip(ui, scene_format_label(&document.format), color_text_dim());
        });
        changed |= inspector_text_field(ui, "Name", &mut scene_name);
        changed |= inspector_text_field(ui, "Document ID", &mut document_id);
        kv_row(ui, "File", &document.source_path);

        if changed {
            self.apply_scene_document_edit(scene_name, document_id);
        }

        ui.horizontal(|ui| {
            if mini_button(ui, "Save", "Salvar cena").clicked() {
                self.save_scene_document_from_ui();
            }
            if mini_button(ui, "Save+Sync", "Salvar e sincronizar com preview").clicked() {
                if self.save_scene_document_internal() {
                    self.sync_scene_to_preview_from_ui();
                }
            }
            if mini_button(ui, "Reload", "Recarregar cena do disco").clicked() {
                if self.ensure_scene_change_is_safe("recarregar a cena") {
                    self.load_current_scene_document();
                }
            }
        });

        ui.checkbox(
            &mut self.auto_sync_preview,
            "Auto Sync preview depois de salvar",
        );
    }

    fn render_scene_document(&mut self, ui: &mut egui::Ui) {
        if let Some(error) = &self.scene_error {
            ui.colored_label(ui.visuals().warn_fg_color, error);
        }

        let Some(document) = self.scene_document.clone() else {
            ui.label(
                egui::RichText::new("Carregue uma cena para listar entidades.")
                    .size(11.0)
                    .color(color_text_dim()),
            );
            return;
        };

        kv_row(ui, "Scene", &document.name);
        kv_row(ui, "Format", scene_format_label(&document.format));
        kv_row(ui, "Entities", document.entities.len().to_string());
        ui.add_space(8.0);

        ui.horizontal(|ui| {
            if mini_button(ui, "Create", "Criar entidade").clicked() {
                self.create_scene_entity_in_model();
            }
            if mini_button(ui, "Delete", "Remover entidade selecionada").clicked() {
                self.remove_selected_scene_entity_from_model();
            }
            if mini_button(ui, "Focus", "Centralizar entidade selecionada").clicked() {
                self.request_focus_selected_in_viewport();
            }
            if mini_button(ui, "Frame", "Enquadrar toda a cena").clicked() {
                self.request_frame_all_in_viewport();
            }
        });
        ui.add_space(8.0);

        inspector_text_field(ui, "Filter", &mut self.hierarchy_filter);
        ui.horizontal(|ui| {
            let filter = self.hierarchy_filter.trim();
            if filter.is_empty() {
                status_chip(
                    ui,
                    format!("{} visible", document.entities.len()),
                    color_text_dim(),
                );
            } else {
                let visible = document
                    .entities
                    .iter()
                    .filter(|entity| entity_matches_filter(entity, filter))
                    .count();
                status_chip(ui, format!("{} matches", visible), color_text_dim());
                if mini_button(ui, "Clear", "Limpar busca").clicked() {
                    self.hierarchy_filter.clear();
                }
            }
        });
        ui.add_space(8.0);

        if document.entities.is_empty() {
            ui.label(
                egui::RichText::new("(sem entidades detectadas)")
                    .size(11.0)
                    .monospace()
                    .color(color_text_faint()),
            );
            return;
        }

        egui::CollapsingHeader::new(
            egui::RichText::new("Entities")
                .size(11.5)
                .strong()
                .color(color_text()),
        )
        .default_open(true)
        .show(ui, |ui| {
            let filter = self.hierarchy_filter.trim().to_ascii_lowercase();
            if filter.is_empty() {
                let mut roots: Vec<_> = document
                    .entities
                    .iter()
                    .filter(|entity| {
                        entity.parent.is_empty()
                            || !document
                                .entities
                                .iter()
                                .any(|candidate| candidate.stable_id == entity.parent)
                    })
                    .cloned()
                    .collect();
                roots.sort_by(|left, right| left.name.cmp(&right.name));
                for entity in roots {
                    self.render_scene_hierarchy_node(ui, &document, entity);
                }
            } else {
                let mut matches: Vec<_> = document
                    .entities
                    .into_iter()
                    .filter(|entity| entity_matches_filter(entity, &filter))
                    .collect();
                matches.sort_by(|left, right| left.name.cmp(&right.name));

                if matches.is_empty() {
                    ui.label(
                        egui::RichText::new("Nenhuma entidade combina com o filtro.")
                            .size(11.0)
                            .color(color_text_dim()),
                    );
                } else {
                    for entity in matches {
                        self.render_scene_hierarchy_leaf(ui, &entity);
                    }
                }
            }
        });
    }

    fn render_scene_hierarchy_node(
        &mut self,
        ui: &mut egui::Ui,
        document: &SceneDocument,
        entity: SceneEntity,
    ) {
        let mut children: Vec<_> = document
            .entities
            .iter()
            .filter(|candidate| candidate.parent == entity.stable_id)
            .cloned()
            .collect();
        children.sort_by(|left, right| left.name.cmp(&right.name));

        if children.is_empty() {
            self.render_scene_hierarchy_leaf(ui, &entity);
            return;
        }

        let label = scene_hierarchy_label(&entity, self.selected_scene_entity == entity.stable_id);
        let response = egui::CollapsingHeader::new(label)
            .id_salt(("scene_hierarchy", entity.stable_id.clone()))
            .default_open(true)
            .show(ui, |ui| {
                for child in children {
                    self.render_scene_hierarchy_node(ui, document, child);
                }
            });

        if response.header_response.clicked() {
            self.set_selected_scene_entity(&entity.stable_id, &entity.name, true);
        }
        response.header_response.on_hover_text(format!(
            "id: {}\nlayer: {}\nparent: {}\ntags: {}\ncomponents: {}",
            entity.stable_id,
            entity.layer,
            empty_as_dash(&entity.parent),
            list_label(&entity.tags),
            list_label(&entity.components)
        ));
    }

    fn render_scene_hierarchy_leaf(&mut self, ui: &mut egui::Ui, entity: &SceneEntity) {
        let response = ui.selectable_label(
            self.selected_scene_entity == entity.stable_id,
            scene_hierarchy_label(entity, self.selected_scene_entity == entity.stable_id),
        );
        if response.clicked() {
            self.set_selected_scene_entity(&entity.stable_id, &entity.name, true);
        }
        response.on_hover_text(format!(
            "id: {}\nlayer: {}\nparent: {}\ntags: {}\ncomponents: {}",
            entity.stable_id,
            entity.layer,
            empty_as_dash(&entity.parent),
            list_label(&entity.tags),
            list_label(&entity.components)
        ));
    }

    fn open_project_in_preview(&mut self) {
        match self.preview.open_project(self.project_path.clone()) {
            Ok(_) => {
                self.flush_preview_messages();
                self.check_preview_status();
            }
            Err(error) => self.push_transcript(format!("erro ao abrir projeto: {error}")),
        }
    }

    fn push_transcript(&mut self, line: impl Into<String>) {
        self.transcript.push(line.into());
        if self.transcript.len() > 200 {
            let excess = self.transcript.len() - 200;
            self.transcript.drain(0..excess);
        }
    }

    fn flush_preview_messages(&mut self) {
        for line in self.preview.drain_outgoing_lines() {
            self.last_sent_line = line.trim().to_owned();
            self.push_transcript(format!("editor -> preview | {}", line.trim()));
        }

        for message in self.preview.drain_incoming() {
            self.push_transcript(format!(
                "preview -> editor | seq={} channel={:?} {}",
                message.seq,
                message.channel,
                message.summary()
            ));
        }
    }

    fn check_preview_status(&mut self) {
        let status = self.preview.current_status();
        let state = self.preview.state();
        if state != ConnectionState::Disconnected {
            self.push_transcript(format!("Preview status: {:?} (state: {:?})", status, state));
        }
    }

    fn start_preview_from_ui(&mut self) {
        self.preview.set_preview_path(self.preview_path.clone());
        match self.preview.start_preview(Some(self.project_path.clone())) {
            Ok(_) => {
                self.flush_preview_messages();
                self.open_project_in_preview();
                self.sync_scene_to_preview_from_ui();
                if !self.selected_entity_id.is_empty() {
                    self.select_entity_from_ui();
                }
                self.check_preview_status();
            }
            Err(error) => self.push_transcript(format!("erro ao iniciar preview: {error}")),
        }
    }

    fn open_scene_from_ui(&mut self) {
        let scene_path = self.scene_path.clone();
        self.activate_scene_from_ui(scene_path, true);
    }

    fn enter_play_mode_from_ui(&mut self) {
        match self.preview.enter_play_mode() {
            Ok(_) => {
                self.flush_preview_messages();
                self.check_preview_status();
            }
            Err(error) => self.push_transcript(format!("erro ao entrar em play mode: {error}")),
        }
    }

    fn pause_play_mode_from_ui(&mut self) {
        match self.preview.pause_play_mode() {
            Ok(_) => {
                self.flush_preview_messages();
                self.push_transcript("Play mode paused".to_string());
            }
            Err(error) => self.push_transcript(format!("erro ao pausar: {error}")),
        }
    }

    fn stop_preview_from_ui(&mut self) {
        match self.preview.stop_preview() {
            Ok(_) => {
                self.flush_preview_messages();
                self.push_transcript("Preview stopped".to_string());
            }
            Err(error) => self.push_transcript(format!("erro ao parar preview: {error}")),
        }
    }

    fn select_entity_from_ui(&mut self) {
        match self.preview.select_entity(self.selected_entity_id.clone()) {
            Ok(_) => {
                self.flush_preview_messages();
            }
            Err(error) => self.push_transcript(format!("erro ao selecionar entidade: {error}")),
        }
    }

    fn ping_preview_from_ui(&mut self) {
        match self.preview.ping("editor-ping".to_owned()) {
            Ok(_) => {
                self.flush_preview_messages();
            }
            Err(error) => self.push_transcript(format!("erro no ping: {error}")),
        }
    }

    fn render_top_toolbar(&mut self, ui: &mut egui::Ui) {
        ui.vertical(|ui| {
            ui.horizontal(|ui| {
                ui.label(
                    egui::RichText::new(LABEL_APP_TITLE)
                        .size(13.0)
                        .strong()
                        .color(color_text()),
                );
                ui.separator();

                egui::MenuBar::new().ui(ui, |ui| {
                    ui.menu_button("File", |ui| {
                        if ui.button("Save Scene").clicked() {
                            self.save_scene_document_from_ui();
                        }
                        if ui.button("Reload Scene").clicked()
                            && self.ensure_scene_change_is_safe("recarregar a cena")
                        {
                            self.load_current_scene_document();
                        }
                        if ui.button("Open Project in Preview").clicked() {
                            self.open_project_in_preview();
                        }
                    });
                    ui.menu_button("Edit", |ui| {
                        if ui.button("Select Entity").clicked() {
                            self.select_entity_from_ui();
                        }
                        if ui.button("Ping Preview").clicked() {
                            self.ping_preview_from_ui();
                        }
                    });
                    ui.menu_button("GameObject", |ui| {
                        if ui.button("Create Entity").clicked() {
                            self.create_scene_entity_in_model();
                        }
                        if ui.button("Create 3D Cube").clicked() {
                            self.create_scene_primitive_in_model("cube3d", "Cube 3D");
                        }
                        if ui.button("Create 3D Camera").clicked() {
                            self.create_scene_primitive_in_model("camera3d", "Camera 3D");
                        }
                        if ui.button("Delete Selected").clicked() {
                            self.remove_selected_scene_entity_from_model();
                        }
                    });
                    ui.menu_button("Window", |ui| {
                        if ui.button("Inspect Project").clicked() {
                            self.refresh_project_summary();
                        }
                    });
                    ui.menu_button("Help", |ui| {
                        ui.label("Borealis Editor");
                        ui.label("Unity-like layout preview");
                    });
                });

                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                    if self.preview.is_mock() {
                        status_chip(ui, "MOCK", color_warning());
                    }
                    status_chip(
                        ui,
                        if self.auto_sync_preview {
                            "AUTO SYNC"
                        } else {
                            "MANUAL SYNC"
                        },
                        color_text_dim(),
                    );
                    status_chip(
                        ui,
                        if self.scene_dirty { "DIRTY" } else { "SAVED" },
                        if self.scene_dirty {
                            color_warning()
                        } else {
                            color_play()
                        },
                    );
                });
            });

            ui.add_space(2.0);
            ui.horizontal(|ui| {
                ui.label(
                    egui::RichText::new(LABEL_TOOLBAR)
                        .size(10.5)
                        .monospace()
                        .color(color_text_faint()),
                );
                ui.separator();
                if mini_toggle(
                    ui,
                    self.transform_tool == TransformTool::Hand,
                    "Hand",
                    "Pan tool",
                )
                .clicked()
                {
                    self.transform_tool = TransformTool::Hand;
                }
                if mini_toggle(
                    ui,
                    self.transform_tool == TransformTool::Move,
                    "Move",
                    "Move tool",
                )
                .clicked()
                {
                    self.transform_tool = TransformTool::Move;
                }
                if mini_toggle(
                    ui,
                    self.transform_tool == TransformTool::Rotate,
                    "Rot",
                    "Rotate tool",
                )
                .clicked()
                {
                    self.transform_tool = TransformTool::Rotate;
                }
                if mini_toggle(
                    ui,
                    self.transform_tool == TransformTool::Scale,
                    "Scale",
                    "Scale tool",
                )
                .clicked()
                {
                    self.transform_tool = TransformTool::Scale;
                }
                ui.separator();

                let is_playing = self.preview.current_status() == PreviewStatus::Playing;
                let is_paused = self.preview.current_status() == PreviewStatus::Paused;

                if ui
                    .add(
                        egui::Button::new(
                            egui::RichText::new(if is_playing { "\u{25A0}" } else { "\u{25B6}" })
                                .size(12.0)
                                .strong()
                                .color(if is_playing {
                                    rgb(255, 255, 255)
                                } else {
                                    rgb(255, 255, 255)
                                }),
                        )
                        .fill(if is_playing {
                            color_play()
                        } else {
                            rgb(72, 145, 72)
                        })
                        .min_size(egui::vec2(28.0, 24.0))
                        .corner_radius(egui::CornerRadius::same(3)),
                    )
                    .on_hover_text(if is_playing { "Stop Play" } else { "Play" })
                    .clicked()
                {
                    if is_playing {
                        self.stop_preview_from_ui();
                    } else {
                        self.enter_play_mode_from_ui();
                    }
                }

                let pause_button = egui::Button::new(
                    egui::RichText::new("\u{23EA}")
                        .size(12.0)
                        .strong()
                        .color(rgb(255, 255, 255)),
                )
                .fill(if is_paused {
                    rgb(180, 140, 50)
                } else {
                    rgb(120, 120, 120)
                })
                .min_size(egui::vec2(28.0, 24.0))
                .corner_radius(egui::CornerRadius::same(3));
                if ui
                    .add_enabled(!is_playing || is_paused, pause_button)
                    .on_hover_text("Pause")
                    .clicked()
                {
                    if is_playing {
                        self.pause_play_mode_from_ui();
                    } else {
                        self.enter_play_mode_from_ui();
                        self.pause_play_mode_from_ui();
                    }
                }

                let step_button = egui::Button::new(
                    egui::RichText::new("\u{23E9}")
                        .size(12.0)
                        .strong()
                        .color(rgb(255, 255, 255)),
                )
                .fill(rgb(120, 120, 120))
                .min_size(egui::vec2(28.0, 24.0))
                .corner_radius(egui::CornerRadius::same(3));
                if ui
                    .add_enabled(is_playing || is_paused, step_button)
                    .on_hover_text("Step Forward")
                    .clicked()
                {
                    self.push_transcript("Step forward (frame by frame)".to_string());
                }

                ui.separator();

                if mini_button(ui, "Start", "Iniciar preview").clicked() {
                    self.start_preview_from_ui();
                }
                if mini_button(ui, "Save", "Salvar cena").clicked() {
                    self.save_scene_document_from_ui();
                }
                if mini_button(ui, "Sync", "Sincronizar cena com preview").clicked() {
                    self.sync_scene_to_preview_from_ui();
                }

                ui.separator();
                if mini_button(ui, "Ping", "Enviar ping ao preview").clicked() {
                    self.ping_preview_from_ui();
                }
                if mini_button(ui, "Focus", "Centralizar entidade selecionada").clicked() {
                    self.request_focus_selected_in_viewport();
                }
                if mini_button(ui, "Frame", "Enquadrar toda a cena").clicked() {
                    self.request_frame_all_in_viewport();
                }

                ui.separator();
                status_chip(
                    ui,
                    format!("State {:?}", self.preview.state()),
                    color_text_dim(),
                );
                status_chip(
                    ui,
                    format!("Preview {:?}", self.preview.current_status()),
                    color_text_dim(),
                );
                status_chip(
                    ui,
                    format!("Transport {:?}", self.preview.transport()),
                    color_text_dim(),
                );
                status_chip(
                    ui,
                    format!("Zoom {:.0}%", self.viewport_zoom * 100.0),
                    color_text_dim(),
                );
                ui.separator();
                ui.label(
                    egui::RichText::new(format!("Scene {}", empty_as_dash(&self.scene_path)))
                        .size(10.5)
                        .monospace()
                        .color(color_text_dim()),
                );
                ui.label(
                    egui::RichText::new(format!(
                        "Entity {}",
                        empty_as_dash(&self.selected_scene_entity)
                    ))
                    .size(10.5)
                    .monospace()
                    .color(color_text_dim()),
                );
            });
        });
    }

    fn render_left_dock(&mut self, ui: &mut egui::Ui) {
        egui::ScrollArea::vertical()
            .auto_shrink([false, false])
            .show(ui, |ui| {
                dock_title(ui, LABEL_HIERARCHY, "scene objects");
                self.render_scene_document(ui);

                ui.separator();
                dock_title(ui, "Connection", "preview bridge");
                egui::CollapsingHeader::new(
                    egui::RichText::new("Transport")
                        .size(11.5)
                        .strong()
                        .color(color_text()),
                )
                .default_open(true)
                .show(ui, |ui| {
                    ui.horizontal(|ui| {
                        if mini_button(
                            ui,
                            if self.preview.is_mock() {
                                "Use Real"
                            } else {
                                "Use Mock"
                            },
                            "Alternar entre preview real e mock",
                        )
                        .clicked()
                        {
                            let new_val = !self.preview.is_mock();
                            self.preview.set_mock(new_val);
                            if new_val {
                                self.push_transcript("Mock mode: ATIVO - simulando respostas");
                            } else {
                                self.push_transcript("Real mode: ATIVO - usando preview compilado");
                            }
                        }

                        if mini_button(ui, "Open", "Abrir projeto no preview").clicked() {
                            self.open_project_in_preview();
                        }
                    });

                    section_caption(ui, "Paths");
                    inspector_text_field(ui, "Project", &mut self.project_path);
                    inspector_text_field(ui, "Preview", &mut self.preview_path);
                    if mini_button(ui, "Inspect", "Inspecionar manifesto do projeto").clicked() {
                        self.refresh_project_summary();
                    }
                });
            });
    }

    fn render_right_dock(&mut self, ui: &mut egui::Ui) {
        egui::ScrollArea::vertical()
            .auto_shrink([false, false])
            .show(ui, |ui| {
                dock_title(ui, LABEL_INSPECTOR, "properties");
                self.render_inspector(ui);

                ui.separator();
                dock_title(ui, "Selection", "preview target");
                inspector_text_field(ui, "Scene", &mut self.scene_path);
                ui.horizontal(|ui| {
                    if mini_button(ui, "Load", "Carregar no editor").clicked() {
                        let scene_path = self.scene_path.clone();
                        self.activate_scene_from_ui(scene_path, false);
                    }
                    if mini_button(ui, "Open", "Abrir no preview").clicked() {
                        self.open_scene_from_ui();
                    }
                });

                inspector_text_field(ui, "Entity", &mut self.selected_entity_id);
                if mini_button(ui, "Select Entity", "Selecionar entidade no preview").clicked() {
                    self.select_entity_from_ui();
                }

                ui.separator();
                self.render_scene_settings(ui);

                ui.separator();
                egui::CollapsingHeader::new(
                    egui::RichText::new("Project Info")
                        .size(11.5)
                        .strong()
                        .color(color_text()),
                )
                .default_open(false)
                .show(ui, |ui| {
                    self.render_project_summary(ui);
                });

                ui.separator();
                section_caption(ui, "Last message");
                ui.add_sized(
                    [ui.available_width(), 18.0],
                    egui::Label::new(
                        egui::RichText::new(if self.last_sent_line.is_empty() {
                            "(nenhuma ainda)"
                        } else {
                            &self.last_sent_line
                        })
                        .size(11.0)
                        .monospace()
                        .color(color_text_dim()),
                    )
                    .truncate(),
                );
            });
    }

    fn render_viewport(&mut self, ui: &mut egui::Ui) {
        ui.push_id(VIEWPORT_ID, |ui| {
            egui::Frame::new()
                .fill(color_dock_header())
                .inner_margin(egui::Margin::symmetric(8, 4))
                .stroke(egui::Stroke::new(1.0, color_border_dark()))
                .show(ui, |ui| {
                    ui.horizontal(|ui| {
                        if mini_toggle(
                            ui,
                            self.viewport_tab == ViewportTab::Scene,
                            LABEL_SCENE_VIEW,
                            "Scene view",
                        )
                        .clicked()
                        {
                            self.viewport_tab = ViewportTab::Scene;
                        }
                        if mini_toggle(
                            ui,
                            self.viewport_tab == ViewportTab::Scene3D,
                            LABEL_SCENE_3D,
                            "Scene view 3D",
                        )
                        .clicked()
                        {
                            self.viewport_tab = ViewportTab::Scene3D;
                        }
                        if mini_toggle(
                            ui,
                            self.viewport_tab == ViewportTab::Game,
                            LABEL_GAME,
                            "Game view",
                        )
                        .clicked()
                        {
                            self.viewport_tab = ViewportTab::Game;
                        }
                        ui.separator();
                        if mini_button(ui, "Focus", "Centralizar entidade selecionada").clicked() {
                            self.request_focus_selected_in_viewport();
                        }
                        if mini_button(ui, "Frame", "Enquadrar toda a cena").clicked() {
                            self.request_frame_all_in_viewport();
                        }
                        ui.separator();
                        ui.label(
                            egui::RichText::new(format!("Active {}", self.scene_path))
                                .size(10.5)
                                .monospace()
                                .color(color_text_dim()),
                        );
                        ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                            status_chip(
                                ui,
                                if self.scene_dirty { "dirty" } else { "saved" },
                                if self.scene_dirty {
                                    color_warning()
                                } else {
                                    color_play()
                                },
                            );
                            status_chip(
                                ui,
                                format!("{:.0}%", self.viewport_zoom * 100.0),
                                color_text_dim(),
                            );
                            status_chip(
                                ui,
                                if self.viewport_tab == ViewportTab::Scene3D {
                                    "3D"
                                } else {
                                    "2D"
                                },
                                color_text_dim(),
                            );
                            status_chip(ui, "Shaded", color_text_dim());
                        });
                    });
                });
            ui.add_space(4.0);

            let available = ui.available_size();
            let desired = egui::vec2(available.x.max(360.0), available.y.max(240.0));
            let (rect, response) = ui.allocate_exact_size(desired, egui::Sense::click_and_drag());
            let placeholder_color = ui.visuals().strong_text_color();
            self.apply_viewport_focus_request(rect);

            let (hover_pos, pointer_delta, scroll_delta, pinch_zoom, middle_down, primary_down) =
                ui.input(|input| {
                    (
                        input.pointer.hover_pos(),
                        input.pointer.delta(),
                        input.smooth_scroll_delta.y,
                        input.zoom_delta(),
                        input.pointer.middle_down(),
                        input.pointer.primary_down(),
                    )
                });
            let pointer_inside = hover_pos
                .map(|pointer| rect.contains(pointer))
                .unwrap_or(false);

            if pointer_inside && middle_down {
                self.viewport_pan += pointer_delta;
                ui.ctx().request_repaint();
            }

            if pointer_inside
                && self.transform_tool == TransformTool::Hand
                && primary_down
                && self.viewport_drag_entity.is_none()
            {
                self.viewport_pan += pointer_delta;
                ui.ctx().request_repaint();
            }

            let zoom_factor = if pinch_zoom != 1.0 {
                pinch_zoom
            } else {
                (1.0 + (scroll_delta * 0.0015)).clamp(0.88, 1.14)
            };
            if pointer_inside && (zoom_factor - 1.0).abs() > f32::EPSILON {
                let anchor = hover_pos.unwrap_or(rect.center());
                let (scene_x, scene_y) =
                    viewport_to_scene(rect, self.viewport_pan, self.viewport_zoom, anchor);
                self.viewport_zoom = (self.viewport_zoom * zoom_factor).clamp(0.35, 2.75);
                self.viewport_pan = egui::vec2(
                    anchor.x - rect.left() - (scene_x * self.viewport_zoom),
                    anchor.y - rect.top() - (scene_y * self.viewport_zoom),
                );
                ui.ctx().request_repaint();
            }

            ui.painter().rect_filled(rect, 0.0, color_viewport());
            ui.painter().rect_stroke(
                rect,
                0.0,
                egui::Stroke::new(1.0, color_border_dark()),
                egui::StrokeKind::Inside,
            );
            match self.viewport_tab {
                ViewportTab::Scene => {
                    draw_viewport_grid(ui, rect, self.viewport_pan, self.viewport_zoom);
                    draw_viewport_gizmo(ui, rect);
                }
                ViewportTab::Scene3D => {
                    draw_viewport_grid_3d(ui, rect, self.viewport_pan, self.viewport_zoom);
                    draw_viewport_gizmo_3d(ui, rect);
                }
                ViewportTab::Game => {}
            }
            let entity_interaction = self.draw_scene_entities(ui, rect);

            if let Some(payload) = response.dnd_hover_payload::<ProjectDragPayload>() {
                let is_supported = payload.kind == ProjectTreeItemKind::Asset
                    && (asset_is_model3d(&payload.path) || asset_is_image2d(&payload.path));
                let text = if is_supported {
                    format!("Drop {}", payload.label)
                } else {
                    format!("Cannot drop {}", payload.label)
                };
                let overlay_rect =
                    egui::Rect::from_center_size(rect.center(), egui::vec2(220.0, 38.0));
                ui.painter().rect_filled(overlay_rect, 3.0, rgb(42, 42, 42));
                ui.painter().rect_stroke(
                    overlay_rect,
                    3.0,
                    egui::Stroke::new(
                        1.0,
                        if is_supported {
                            color_accent()
                        } else {
                            color_warning()
                        },
                    ),
                    egui::StrokeKind::Inside,
                );
                ui.painter().text(
                    overlay_rect.center(),
                    egui::Align2::CENTER_CENTER,
                    text,
                    egui::FontId::monospace(11.0),
                    color_text(),
                );
            }

            if let Some(payload) = response.dnd_release_payload::<ProjectDragPayload>() {
                if payload.kind == ProjectTreeItemKind::Asset
                    && (asset_is_model3d(&payload.path) || asset_is_image2d(&payload.path))
                {
                    let pointer = ui
                        .input(|input| input.pointer.interact_pos())
                        .unwrap_or(rect.center());
                    if self.viewport_tab == ViewportTab::Scene3D || asset_is_model3d(&payload.path)
                    {
                        let (x, y) = viewport_to_scene3d_ground(
                            rect,
                            self.viewport_pan,
                            self.viewport_zoom,
                            pointer,
                            0.0,
                        );
                        self.create_scene_entity_from_asset(&payload, x, y, 0.0);
                    } else {
                        let (x, y) =
                            viewport_to_scene(rect, self.viewport_pan, self.viewport_zoom, pointer);
                        self.create_scene_entity_from_asset(&payload, x, y, 0.0);
                    }
                } else {
                    self.push_transcript(format!(
                        "Drop ignorado: {} nao e asset visual suportado",
                        payload.path
                    ));
                }
            }

            match self.viewport_tab {
                ViewportTab::Scene => {
                    ui.painter().text(
                        rect.left_top() + egui::vec2(10.0, 8.0),
                        egui::Align2::LEFT_TOP,
                        "Scene | Pivot | Global",
                        egui::FontId::monospace(10.5),
                        color_text_dim(),
                    );
                    ui.painter().text(
                        rect.left_bottom() + egui::vec2(10.0, -10.0),
                        egui::Align2::LEFT_BOTTOM,
                        "Middle mouse pans. Wheel zooms. Move tool drags objects.",
                        egui::FontId::monospace(10.5),
                        color_text_faint(),
                    );
                }
                ViewportTab::Scene3D => {
                    ui.painter().text(
                        rect.left_top() + egui::vec2(10.0, 8.0),
                        egui::Align2::LEFT_TOP,
                        "Scene 3D | Pivot | Global",
                        egui::FontId::monospace(10.5),
                        color_text_dim(),
                    );
                    ui.painter().text(
                        rect.left_bottom() + egui::vec2(10.0, -10.0),
                        egui::Align2::LEFT_BOTTOM,
                        "Drag models from Project. Move tool drags on ground plane.",
                        egui::FontId::monospace(10.5),
                        color_text_faint(),
                    );
                }
                ViewportTab::Game => {
                    ui.painter().text(
                        rect.left_top() + egui::vec2(10.0, 8.0),
                        egui::Align2::LEFT_TOP,
                        "Game | Centered | Preview",
                        egui::FontId::monospace(10.5),
                        color_text_dim(),
                    );
                    ui.painter().text(
                        rect.left_bottom() + egui::vec2(10.0, -10.0),
                        egui::Align2::LEFT_BOTTOM,
                        "Runtime composition preview",
                        egui::FontId::monospace(10.5),
                        color_text_faint(),
                    );
                }
            }

            if matches!(self.viewport_tab, ViewportTab::Scene | ViewportTab::Scene3D)
                && response.clicked_by(egui::PointerButton::Primary)
                && !entity_interaction
            {
                self.clear_scene_selection();
            }

            if self.scene_document.is_none() {
                ui.painter().text(
                    rect.center(),
                    egui::Align2::CENTER_CENTER,
                    "Preview viewport",
                    egui::FontId::proportional(18.0),
                    placeholder_color,
                );
            }
        });
    }

    fn draw_scene_entities(&mut self, ui: &mut egui::Ui, rect: egui::Rect) -> bool {
        let Some(document) = self.scene_document.clone() else {
            return false;
        };
        let is_scene_view = matches!(self.viewport_tab, ViewportTab::Scene | ViewportTab::Scene3D);
        let is_scene_3d = self.viewport_tab == ViewportTab::Scene3D;
        let allow_transform = is_scene_view && self.transform_tool == TransformTool::Move;
        let allow_selection = is_scene_view;
        let mut entity_interaction = false;
        let mut selected_gizmo = None;

        for entity in document.entities {
            let id = ui.make_persistent_id(("scene_entity", entity.stable_id.clone()));
            let position = if is_scene_3d {
                scene3d_to_viewport(
                    rect,
                    self.viewport_pan,
                    self.viewport_zoom,
                    entity.transform.x,
                    entity.transform.y,
                    entity.transform.z,
                )
            } else {
                scene_to_viewport(
                    rect,
                    self.viewport_pan,
                    self.viewport_zoom,
                    entity.transform.x,
                    entity.transform.y,
                )
            };
            let size = egui::vec2(
                (54.0 * entity.transform.scale_x * self.viewport_zoom).max(18.0),
                ((if is_scene_3d { 46.0 } else { 34.0 })
                    * entity.transform.scale_y
                    * self.viewport_zoom)
                    .max(14.0),
            );
            let entity_rect = egui::Rect::from_center_size(position, size);
            let response = ui.interact(
                entity_rect,
                id,
                if allow_selection {
                    egui::Sense::click_and_drag()
                } else {
                    egui::Sense::hover()
                },
            );
            entity_interaction |= response.hovered() || response.dragged();

            if allow_selection && response.clicked() {
                self.set_selected_scene_entity(&entity.stable_id, &entity.name, true);
            }
            if allow_transform && response.drag_started_by(egui::PointerButton::Primary) {
                self.set_selected_scene_entity(&entity.stable_id, &entity.name, false);
                self.viewport_drag_entity = Some(entity.stable_id.clone());
                self.viewport_drag_grab_offset = ui
                    .input(|input| input.pointer.interact_pos())
                    .map(|pointer| {
                        let (scene_x, scene_y) = if is_scene_3d {
                            viewport_to_scene3d_ground(
                                rect,
                                self.viewport_pan,
                                self.viewport_zoom,
                                pointer,
                                entity.transform.z,
                            )
                        } else {
                            viewport_to_scene(rect, self.viewport_pan, self.viewport_zoom, pointer)
                        };
                        (entity.transform.x - scene_x, entity.transform.y - scene_y)
                    });
                self.viewport_drag_moved = false;
            }

            if allow_transform
                && self.viewport_drag_entity.as_deref() == Some(entity.stable_id.as_str())
                && ui.input(|input| input.pointer.primary_down())
            {
                if let (Some(pointer), Some((grab_x, grab_y))) = (
                    ui.input(|input| input.pointer.interact_pos()),
                    self.viewport_drag_grab_offset,
                ) {
                    let (scene_x, scene_y) = if is_scene_3d {
                        viewport_to_scene3d_ground(
                            rect,
                            self.viewport_pan,
                            self.viewport_zoom,
                            pointer,
                            entity.transform.z,
                        )
                    } else {
                        viewport_to_scene(rect, self.viewport_pan, self.viewport_zoom, pointer)
                    };
                    self.update_scene_entity_position(
                        &entity.stable_id,
                        scene_x + grab_x,
                        scene_y + grab_y,
                    );
                    self.viewport_drag_moved = true;
                    ui.ctx().request_repaint();
                }
            }
            let selected = allow_selection && self.selected_scene_entity == entity.stable_id;
            let fill = if response.hovered() {
                color_entity_hover()
            } else if selected {
                color_accent()
            } else if entity
                .components
                .iter()
                .any(|component| component == "camera2d" || component == "camera3d")
            {
                color_camera()
            } else if entity_is_3d(&entity) {
                rgb(86, 90, 96)
            } else {
                color_entity()
            };

            if is_scene_3d && entity_is_3d(&entity) {
                draw_entity_box_3d(ui, entity_rect, fill, selected);
            } else {
                ui.painter().rect_filled(entity_rect, 2.0, fill);
                ui.painter().rect_stroke(
                    entity_rect,
                    2.0,
                    egui::Stroke::new(
                        if selected { 2.0 } else { 1.0 },
                        if selected {
                            rgb(166, 203, 244)
                        } else {
                            rgb(118, 118, 118)
                        },
                    ),
                    egui::StrokeKind::Inside,
                );
            }
            if selected {
                selected_gizmo = Some((entity.clone(), entity_rect));
                ui.painter().rect_stroke(
                    entity_rect.expand(3.0),
                    2.0,
                    egui::Stroke::new(1.0, color_warning()),
                    egui::StrokeKind::Inside,
                );
            }
            ui.painter().text(
                entity_rect.center(),
                egui::Align2::CENTER_CENTER,
                entity.name.clone(),
                egui::FontId::monospace(11.0),
                rgb(236, 236, 236),
            );
            if is_scene_view {
                draw_entity_pivot_gizmo(ui, position, is_scene_3d);
            }
            response.on_hover_text(format!(
                "{}\nid: {}\nposition: {:.1}, {:.1}, {:.1}\ncomponents: {}\nmodel: {}",
                entity.name,
                entity.stable_id,
                entity.transform.x,
                entity.transform.y,
                entity.transform.z,
                list_label(&entity.components),
                empty_as_dash(&entity.model_asset)
            ));
        }

        if let Some((entity, entity_rect)) = selected_gizmo {
            draw_selected_entity_gizmo(
                ui,
                &entity,
                entity_rect,
                self.transform_tool,
                self.viewport_zoom,
                is_scene_3d,
            );
        }

        self.finish_viewport_drag_if_needed(ui);
        entity_interaction
    }

    fn render_console(&mut self, ui: &mut egui::Ui) {
        egui::ScrollArea::vertical()
            .stick_to_bottom(true)
            .show(ui, |ui| {
                for line in &self.transcript {
                    ui.label(
                        egui::RichText::new(line)
                            .size(11.0)
                            .monospace()
                            .color(color_text_dim()),
                    );
                }
            });
    }

    fn render_project_bottom_tab(&mut self, ui: &mut egui::Ui) {
        ui.horizontal(|ui| {
            ui.set_height(ui.available_height());

            ui.vertical(|ui| {
                ui.set_width((ui.available_width() * 0.42).clamp(260.0, 420.0));
                ui.horizontal(|ui| {
                    section_caption(ui, "Assets and scripts");
                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        if mini_toggle(
                            ui,
                            self.asset_view_mode == AssetViewMode::List,
                            "List",
                            "List view",
                        )
                        .clicked()
                        {
                            self.asset_view_mode = AssetViewMode::List;
                        }
                        if mini_toggle(
                            ui,
                            self.asset_view_mode == AssetViewMode::Icon,
                            "Grid",
                            "Grid view",
                        )
                        .clicked()
                        {
                            self.asset_view_mode = AssetViewMode::Icon;
                        }
                    });
                });
                egui::ScrollArea::vertical()
                    .auto_shrink([false, false])
                    .show(ui, |ui| {
                        self.render_project_tree(ui);
                    });
            });

            ui.separator();

            ui.vertical(|ui| {
                section_caption(ui, "Selection");
                let Some(item) = self.selected_project_item() else {
                    ui.label(
                        egui::RichText::new("Selecione um asset, script ou cena.")
                            .size(11.0)
                            .color(color_text_dim()),
                    );
                    ui.separator();
                    self.render_project_bottom_summary(ui);
                    return;
                };

                kv_row(ui, "Name", &item.label);
                kv_row(ui, "Type", tree_item_kind_label(&item.kind));
                kv_row(ui, "Path", &item.path);
                ui.add_space(6.0);

                match item.kind {
                    ProjectTreeItemKind::SceneCandidate
                    | ProjectTreeItemKind::Example
                    | ProjectTreeItemKind::Module => {
                        ui.horizontal(|ui| {
                            if mini_button(ui, "Use", "Usar como cena ativa").clicked() {
                                self.activate_scene_from_ui(item.path.clone(), false);
                            }
                            if mini_button(ui, "Open", "Abrir cena no preview").clicked() {
                                self.activate_scene_from_ui(item.path.clone(), true);
                            }
                        });
                    }
                    ProjectTreeItemKind::Asset => {
                        if mini_button(ui, "Log", "Registrar asset no transcript").clicked() {
                            self.push_transcript(format!("Asset selecionado: {}", item.path));
                        }
                    }
                }

                ui.separator();
                self.render_project_bottom_summary(ui);
            });
        });
    }

    fn render_project_bottom_summary(&self, ui: &mut egui::Ui) {
        section_caption(ui, "Project");
        let Some(summary) = &self.project_summary else {
            ui.label(
                egui::RichText::new("Nenhum projeto carregado.")
                    .size(11.0)
                    .color(color_text_dim()),
            );
            return;
        };

        ui.horizontal(|ui| {
            status_chip(
                ui,
                format!("{} modules", summary.module_count),
                color_text_dim(),
            );
            status_chip(
                ui,
                format!("{} examples", summary.example_count),
                color_text_dim(),
            );
            status_chip(
                ui,
                format!("{} assets", summary.asset_count),
                color_text_dim(),
            );
        });
        kv_row(ui, "Name", &summary.name);
        kv_row(ui, "Root", &summary.project_dir);
        kv_row(ui, "Source", &summary.source_root);
    }

    fn render_bottom_dock(&mut self, ui: &mut egui::Ui) {
        egui::Frame::new()
            .fill(color_dock_header())
            .inner_margin(egui::Margin::symmetric(8, 4))
            .stroke(egui::Stroke::new(1.0, color_border_dark()))
            .show(ui, |ui| {
                ui.horizontal(|ui| {
                    if mini_toggle(
                        ui,
                        self.bottom_dock_tab == BottomDockTab::Project,
                        LABEL_PROJECT,
                        "Assets and scripts",
                    )
                    .clicked()
                    {
                        self.bottom_dock_tab = BottomDockTab::Project;
                    }
                    if mini_toggle(
                        ui,
                        self.bottom_dock_tab == BottomDockTab::Console,
                        LABEL_CONSOLE,
                        "Editor transcript",
                    )
                    .clicked()
                    {
                        self.bottom_dock_tab = BottomDockTab::Console;
                    }

                    ui.separator();
                    match self.bottom_dock_tab {
                        BottomDockTab::Project => {
                            if let Some(summary) = &self.project_summary {
                                status_chip(
                                    ui,
                                    format!(
                                        "{} project items",
                                        summary.module_count + summary.asset_count
                                    ),
                                    color_text_dim(),
                                );
                            } else {
                                status_chip(ui, "no project", color_warning());
                            }
                        }
                        BottomDockTab::Console => {
                            status_chip(
                                ui,
                                format!("{} lines", self.transcript.len()),
                                color_text_dim(),
                            );
                        }
                    }

                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        status_chip(ui, "bottom dock", color_text_faint());
                    });
                });
            });
        ui.add_space(4.0);

        match self.bottom_dock_tab {
            BottomDockTab::Project => self.render_project_bottom_tab(ui),
            BottomDockTab::Console => self.render_console(ui),
        }
    }
}

impl eframe::App for BorealisEditorApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        self.preview.update();

        egui::TopBottomPanel::top(TOP_BAR_ID)
            .exact_height(58.0)
            .frame(top_bar_frame())
            .show(ctx, |ui| {
                self.render_top_toolbar(ui);
            });

        egui::SidePanel::left(HIERARCHY_PROJECT_DOCK_ID)
            .resizable(true)
            .default_width(275.0)
            .min_width(230.0)
            .frame(dock_frame())
            .show(ctx, |ui| {
                self.render_left_dock(ui);
            });

        egui::SidePanel::right(INSPECTOR_DOCK_ID)
            .resizable(true)
            .default_width(236.0)
            .width_range(198.0..=340.0)
            .frame(dock_frame())
            .show(ctx, |ui| {
                self.render_right_dock(ui);
            });

        egui::TopBottomPanel::bottom(CONSOLE_DOCK_ID)
            .resizable(true)
            .default_height(220.0)
            .min_height(150.0)
            .frame(console_frame())
            .show(ctx, |ui| {
                self.render_bottom_dock(ui);
            });

        egui::CentralPanel::default()
            .frame(viewport_frame())
            .show(ctx, |ui| {
                self.render_viewport(ui);
            });
    }
}

fn yes_no(value: bool) -> &'static str {
    if value {
        "sim"
    } else {
        "nao"
    }
}

fn render_string_list(ui: &mut egui::Ui, title: &str, items: &[String]) {
    section_caption(ui, title);
    if items.is_empty() {
        ui.label(
            egui::RichText::new("(nenhum)")
                .size(11.0)
                .monospace()
                .color(color_text_faint()),
        );
        return;
    }

    for item in items {
        ui.label(
            egui::RichText::new(item)
                .size(11.0)
                .monospace()
                .color(color_text()),
        );
    }
}

fn tree_item_kind_label(kind: &ProjectTreeItemKind) -> &'static str {
    match kind {
        ProjectTreeItemKind::SceneCandidate => "cena candidata",
        ProjectTreeItemKind::Example => "exemplo",
        ProjectTreeItemKind::Asset => "asset",
        ProjectTreeItemKind::Module => "modulo",
    }
}

fn scene_format_label(format: &SceneDocumentFormat) -> &'static str {
    match format {
        SceneDocumentFormat::Json => "json",
        SceneDocumentFormat::ZenithExample => "zenith example",
        SceneDocumentFormat::Unknown => "desconhecido",
    }
}

fn empty_as_dash(value: &str) -> &str {
    if value.is_empty() {
        "-"
    } else {
        value
    }
}

fn list_label(items: &[String]) -> String {
    if items.is_empty() {
        "-".to_string()
    } else {
        items.join(", ")
    }
}

fn entity_is_3d(entity: &SceneEntity) -> bool {
    !entity.model_asset.is_empty()
        || entity.transform.z.abs() > f32::EPSILON
        || entity.components.iter().any(|component| {
            matches!(
                component.as_str(),
                "model3d" | "mesh3d" | "cube3d" | "camera3d" | "light3d" | "static_model"
            )
        })
}

fn asset_is_model3d(path: &str) -> bool {
    matches!(
        path_extension(path).as_str(),
        "obj" | "glb" | "gltf" | "fbx" | "iqm" | "m3d"
    )
}

fn asset_is_image2d(path: &str) -> bool {
    matches!(
        path_extension(path).as_str(),
        "png" | "jpg" | "jpeg" | "webp"
    )
}

fn asset_entity_name(label: &str) -> String {
    let base = label.rsplit('/').next().unwrap_or(label);
    base.rsplit_once('.')
        .map(|(stem, _)| stem)
        .unwrap_or(base)
        .replace('_', " ")
        .replace('-', " ")
}

fn path_extension(path: &str) -> String {
    path.rsplit_once('.')
        .map(|(_, extension)| extension.to_ascii_lowercase())
        .unwrap_or_default()
}

fn parse_tags_input(value: &str) -> Vec<String> {
    value
        .split(',')
        .map(str::trim)
        .filter(|item| !item.is_empty())
        .map(str::to_string)
        .collect()
}

fn entity_matches_filter(entity: &SceneEntity, filter: &str) -> bool {
    let filter = filter.trim().to_ascii_lowercase();
    if filter.is_empty() {
        return true;
    }

    [
        entity.name.as_str(),
        entity.stable_id.as_str(),
        entity.layer.as_str(),
        entity.parent.as_str(),
    ]
    .iter()
    .any(|value| value.to_ascii_lowercase().contains(&filter))
        || entity
            .tags
            .iter()
            .any(|tag| tag.to_ascii_lowercase().contains(&filter))
        || entity
            .components
            .iter()
            .any(|component| component.to_ascii_lowercase().contains(&filter))
}

fn scene_hierarchy_label(entity: &SceneEntity, selected: bool) -> egui::RichText {
    let icon = if entity
        .components
        .iter()
        .any(|component| component == "camera3d")
    {
        "cam3"
    } else if entity
        .components
        .iter()
        .any(|component| component == "camera2d")
    {
        "cam"
    } else if entity_is_3d(entity) {
        "3d"
    } else if entity.parent.is_empty() {
        "obj"
    } else {
        "sub"
    };

    egui::RichText::new(format!("{icon:<3} {}", entity.name))
        .size(11.0)
        .monospace()
        .color(if selected {
            rgb(238, 245, 255)
        } else {
            color_text()
        })
}

fn scene_to_viewport(rect: egui::Rect, pan: egui::Vec2, zoom: f32, x: f32, y: f32) -> egui::Pos2 {
    egui::pos2(
        rect.left() + pan.x + (x * zoom),
        rect.top() + pan.y + (y * zoom),
    )
}

fn viewport_to_scene(
    rect: egui::Rect,
    pan: egui::Vec2,
    zoom: f32,
    position: egui::Pos2,
) -> (f32, f32) {
    (
        (position.x - rect.left() - pan.x) / zoom,
        (position.y - rect.top() - pan.y) / zoom,
    )
}

fn scene3d_to_viewport(
    rect: egui::Rect,
    pan: egui::Vec2,
    zoom: f32,
    x: f32,
    y: f32,
    z: f32,
) -> egui::Pos2 {
    let projected_x = x - (y * 0.72);
    let projected_y = (x * 0.18) + (y * 0.42) - z;
    egui::pos2(
        rect.left() + pan.x + (projected_x * zoom),
        rect.top() + pan.y + (projected_y * zoom),
    )
}

fn viewport_to_scene3d_ground(
    rect: egui::Rect,
    pan: egui::Vec2,
    zoom: f32,
    position: egui::Pos2,
    z: f32,
) -> (f32, f32) {
    let projected_x = (position.x - rect.left() - pan.x) / zoom;
    let projected_y = ((position.y - rect.top() - pan.y) / zoom) + z;
    let y = (projected_y - (0.18 * projected_x)) / 0.5496;
    let x = projected_x + (0.72 * y);
    (x, y)
}

fn draw_viewport_grid_3d(ui: &egui::Ui, rect: egui::Rect, pan: egui::Vec2, zoom: f32) {
    let minor = egui::Stroke::new(1.0, color_grid_minor());
    let major = egui::Stroke::new(1.0, color_grid_major());
    let axis_x = egui::Stroke::new(1.4, color_axis_x());
    let axis_y = egui::Stroke::new(1.4, color_axis_y());
    let axis_z = egui::Stroke::new(1.6, color_axis_z());
    let extent = 2048.0;
    let step = 64.0;

    for index in -24..=24 {
        let value = index as f32 * step;
        let stroke = if index % 4 == 0 { major } else { minor };
        ui.painter().line_segment(
            [
                scene3d_to_viewport(rect, pan, zoom, value, -extent, 0.0),
                scene3d_to_viewport(rect, pan, zoom, value, extent, 0.0),
            ],
            stroke,
        );
        ui.painter().line_segment(
            [
                scene3d_to_viewport(rect, pan, zoom, -extent, value, 0.0),
                scene3d_to_viewport(rect, pan, zoom, extent, value, 0.0),
            ],
            stroke,
        );
    }

    ui.painter().line_segment(
        [
            scene3d_to_viewport(rect, pan, zoom, -extent, 0.0, 0.0),
            scene3d_to_viewport(rect, pan, zoom, extent, 0.0, 0.0),
        ],
        axis_x,
    );
    ui.painter().line_segment(
        [
            scene3d_to_viewport(rect, pan, zoom, 0.0, -extent, 0.0),
            scene3d_to_viewport(rect, pan, zoom, 0.0, extent, 0.0),
        ],
        axis_y,
    );
    ui.painter().line_segment(
        [
            scene3d_to_viewport(rect, pan, zoom, 0.0, 0.0, 0.0),
            scene3d_to_viewport(rect, pan, zoom, 0.0, 0.0, 240.0),
        ],
        axis_z,
    );
}

fn draw_viewport_grid(ui: &egui::Ui, rect: egui::Rect, pan: egui::Vec2, zoom: f32) {
    let minor = egui::Stroke::new(1.0, color_grid_minor());
    let major = egui::Stroke::new(1.0, color_grid_major());
    let axis_x = egui::Stroke::new(1.0, color_axis_x());
    let axis_y = egui::Stroke::new(1.0, color_axis_y());
    let origin = scene_to_viewport(rect, pan, zoom, 0.0, 0.0);
    let minor_step = 32.0 * zoom;
    let major_step = 128.0 * zoom;

    if minor_step >= 12.0 {
        let start_x = rect.left() + pan.x.rem_euclid(minor_step);
        let mut x = start_x;
        while x <= rect.right() {
            ui.painter().line_segment(
                [egui::pos2(x, rect.top()), egui::pos2(x, rect.bottom())],
                minor,
            );
            x += minor_step;
        }

        let start_y = rect.top() + pan.y.rem_euclid(minor_step);
        let mut y = start_y;
        while y <= rect.bottom() {
            ui.painter().line_segment(
                [egui::pos2(rect.left(), y), egui::pos2(rect.right(), y)],
                minor,
            );
            y += minor_step;
        }
    }

    if major_step >= 10.0 {
        let start_x = rect.left() + pan.x.rem_euclid(major_step);
        let mut x = start_x;
        while x <= rect.right() {
            ui.painter().line_segment(
                [egui::pos2(x, rect.top()), egui::pos2(x, rect.bottom())],
                major,
            );
            x += major_step;
        }

        let start_y = rect.top() + pan.y.rem_euclid(major_step);
        let mut y = start_y;
        while y <= rect.bottom() {
            ui.painter().line_segment(
                [egui::pos2(rect.left(), y), egui::pos2(rect.right(), y)],
                major,
            );
            y += major_step;
        }
    }

    if rect.left() <= origin.x && origin.x <= rect.right() {
        ui.painter().line_segment(
            [
                egui::pos2(origin.x, rect.top()),
                egui::pos2(origin.x, rect.bottom()),
            ],
            axis_y,
        );
    }
    if rect.top() <= origin.y && origin.y <= rect.bottom() {
        ui.painter().line_segment(
            [
                egui::pos2(rect.left(), origin.y),
                egui::pos2(rect.right(), origin.y),
            ],
            axis_x,
        );
    }
}

fn draw_viewport_gizmo(ui: &egui::Ui, rect: egui::Rect) {
    let origin = rect.right_top() + egui::vec2(-58.0, 42.0);
    let x_end = origin + egui::vec2(32.0, 0.0);
    let y_end = origin + egui::vec2(0.0, -28.0);

    ui.painter()
        .line_segment([origin, x_end], egui::Stroke::new(2.0, color_axis_x()));
    ui.painter()
        .line_segment([origin, y_end], egui::Stroke::new(2.0, color_axis_y()));
    ui.painter().circle_filled(origin, 3.0, color_text_dim());
    ui.painter().text(
        x_end + egui::vec2(4.0, 0.0),
        egui::Align2::LEFT_CENTER,
        "X",
        egui::FontId::monospace(10.0),
        color_axis_x(),
    );
    ui.painter().text(
        y_end + egui::vec2(0.0, -4.0),
        egui::Align2::CENTER_BOTTOM,
        "Y",
        egui::FontId::monospace(10.0),
        color_axis_y(),
    );
}

fn draw_viewport_gizmo_3d(ui: &egui::Ui, rect: egui::Rect) {
    let origin = rect.right_top() + egui::vec2(-66.0, 54.0);
    let x_end = origin + egui::vec2(34.0, 8.0);
    let y_end = origin + egui::vec2(-28.0, 14.0);
    let z_end = origin + egui::vec2(0.0, -36.0);

    draw_gizmo_arrow(ui.painter(), origin, x_end, color_axis_x());
    draw_gizmo_arrow(ui.painter(), origin, y_end, color_axis_y());
    draw_gizmo_arrow(ui.painter(), origin, z_end, color_axis_z());
    ui.painter().circle_filled(origin, 3.0, color_text_dim());
    ui.painter().text(
        x_end + egui::vec2(4.0, 1.0),
        egui::Align2::LEFT_CENTER,
        "X",
        egui::FontId::monospace(10.0),
        color_axis_x(),
    );
    ui.painter().text(
        y_end + egui::vec2(-4.0, 2.0),
        egui::Align2::RIGHT_CENTER,
        "Y",
        egui::FontId::monospace(10.0),
        color_axis_y(),
    );
    ui.painter().text(
        z_end + egui::vec2(0.0, -4.0),
        egui::Align2::CENTER_BOTTOM,
        "Z",
        egui::FontId::monospace(10.0),
        color_axis_z(),
    );
}

fn draw_entity_box_3d(ui: &egui::Ui, rect: egui::Rect, fill: egui::Color32, selected: bool) {
    let top_offset = egui::vec2(-8.0, -8.0);
    let side_offset = egui::vec2(10.0, -5.0);
    let top = rect.translate(top_offset);
    let side = rect.translate(side_offset);
    let stroke = egui::Stroke::new(
        if selected { 2.0 } else { 1.0 },
        if selected {
            rgb(166, 203, 244)
        } else {
            rgb(104, 104, 104)
        },
    );

    ui.painter().rect_filled(side, 2.0, rgb(45, 47, 50));
    ui.painter().rect_filled(rect, 2.0, fill);
    ui.painter().rect_filled(top, 2.0, rgb(94, 98, 106));
    ui.painter()
        .rect_stroke(rect, 2.0, stroke, egui::StrokeKind::Inside);
    ui.painter()
        .rect_stroke(top, 2.0, stroke, egui::StrokeKind::Inside);
}

fn draw_entity_pivot_gizmo(ui: &egui::Ui, center: egui::Pos2, is_3d: bool) {
    let painter = ui.painter();
    painter.circle_filled(center, 2.5, rgb(236, 236, 236));
    painter.line_segment(
        [center, center + egui::vec2(14.0, 0.0)],
        egui::Stroke::new(1.4, color_axis_x()),
    );
    painter.line_segment(
        [center, center + egui::vec2(0.0, -14.0)],
        egui::Stroke::new(1.4, color_axis_y()),
    );
    if is_3d {
        painter.line_segment(
            [center, center + egui::vec2(-10.0, -9.0)],
            egui::Stroke::new(1.4, color_axis_z()),
        );
    }
}

fn draw_selected_entity_gizmo(
    ui: &egui::Ui,
    entity: &SceneEntity,
    entity_rect: egui::Rect,
    tool: TransformTool,
    zoom: f32,
    is_3d: bool,
) {
    let center = entity_rect.center();
    let arm = (38.0 * zoom).clamp(28.0, 78.0);
    let painter = ui.painter();

    painter.circle_filled(center, 3.5, rgb(238, 238, 238));

    match tool {
        TransformTool::Hand => {
            painter.circle_stroke(center, arm * 0.42, egui::Stroke::new(1.6, color_text_dim()));
            painter.line_segment(
                [
                    center + egui::vec2(-10.0, 0.0),
                    center + egui::vec2(10.0, 0.0),
                ],
                egui::Stroke::new(1.4, color_text_dim()),
            );
            painter.line_segment(
                [
                    center + egui::vec2(0.0, -10.0),
                    center + egui::vec2(0.0, 10.0),
                ],
                egui::Stroke::new(1.4, color_text_dim()),
            );
        }
        TransformTool::Move => {
            draw_gizmo_arrow(
                painter,
                center,
                center + egui::vec2(arm, 0.0),
                color_axis_x(),
            );
            draw_gizmo_arrow(
                painter,
                center,
                center + egui::vec2(0.0, -arm),
                color_axis_y(),
            );
            if is_3d {
                draw_gizmo_arrow(
                    painter,
                    center,
                    center + egui::vec2(-arm * 0.62, -arm * 0.48),
                    color_axis_z(),
                );
            }
            draw_gizmo_square_handle(painter, center + egui::vec2(12.0, -12.0), 9.0, color_text());
        }
        TransformTool::Rotate => {
            let radius = (entity_rect.width().max(entity_rect.height()) * 0.5 + arm * 0.35)
                .clamp(24.0, 84.0);
            let angle = entity.transform.rotation.to_radians() - std::f32::consts::FRAC_PI_2;
            let handle = center + egui::vec2(angle.cos() * radius, angle.sin() * radius);

            painter.circle_stroke(center, radius, egui::Stroke::new(1.8, rgb(214, 214, 214)));
            painter.circle_stroke(
                center,
                radius - 7.0,
                egui::Stroke::new(1.0, color_text_faint()),
            );
            painter.line_segment([center, handle], egui::Stroke::new(1.5, color_warning()));
            painter.circle_filled(handle, 5.0, color_warning());
        }
        TransformTool::Scale => {
            let x_end = center + egui::vec2(arm, 0.0);
            let y_end = center + egui::vec2(0.0, -arm);
            let corner = center + egui::vec2(arm * 0.72, -arm * 0.72);

            painter.line_segment([center, x_end], egui::Stroke::new(2.0, color_axis_x()));
            painter.line_segment([center, y_end], egui::Stroke::new(2.0, color_axis_y()));
            if is_3d {
                let z_end = center + egui::vec2(-arm * 0.62, -arm * 0.48);
                painter.line_segment([center, z_end], egui::Stroke::new(2.0, color_axis_z()));
                draw_gizmo_square_handle(painter, z_end, 10.0, color_axis_z());
            }
            painter.line_segment([x_end, corner], egui::Stroke::new(1.5, color_text_dim()));
            painter.line_segment([y_end, corner], egui::Stroke::new(1.5, color_text_dim()));
            draw_gizmo_square_handle(painter, x_end, 10.0, color_axis_x());
            draw_gizmo_square_handle(painter, y_end, 10.0, color_axis_y());
            draw_gizmo_square_handle(painter, corner, 10.0, color_text());
        }
    }

    painter.text(
        center + egui::vec2(arm + 10.0, -arm * 0.5),
        egui::Align2::LEFT_CENTER,
        match tool {
            TransformTool::Hand => "Hand",
            TransformTool::Move => "Move",
            TransformTool::Rotate => "Rotate",
            TransformTool::Scale => "Scale",
        },
        egui::FontId::monospace(10.5),
        color_text_dim(),
    );
}

fn draw_gizmo_arrow(
    painter: &egui::Painter,
    start: egui::Pos2,
    end: egui::Pos2,
    color: egui::Color32,
) {
    let delta = end - start;
    let length = delta.length();
    if length <= f32::EPSILON {
        return;
    }

    let direction = delta / length;
    let perpendicular = egui::vec2(-direction.y, direction.x);
    let head_length = 10.0;
    let head_width = 5.0;
    let base = end - direction * head_length;

    painter.line_segment([start, base], egui::Stroke::new(2.2, color));
    painter.add(egui::Shape::convex_polygon(
        vec![
            end,
            base + perpendicular * head_width,
            base - perpendicular * head_width,
        ],
        color,
        egui::Stroke::NONE,
    ));
}

fn draw_gizmo_square_handle(
    painter: &egui::Painter,
    center: egui::Pos2,
    size: f32,
    color: egui::Color32,
) {
    let rect = egui::Rect::from_center_size(center, egui::vec2(size, size));
    painter.rect_filled(rect, 1.0, color);
    painter.rect_stroke(
        rect,
        1.0,
        egui::Stroke::new(1.0, color_border_dark()),
        egui::StrokeKind::Inside,
    );
}

fn rgb(r: u8, g: u8, b: u8) -> egui::Color32 {
    egui::Color32::from_rgb(r, g, b)
}

fn color_app_bg() -> egui::Color32 {
    rgb(31, 31, 31)
}

fn color_top_bar() -> egui::Color32 {
    rgb(37, 37, 38)
}

fn color_dock() -> egui::Color32 {
    rgb(43, 43, 43)
}

fn color_dock_header() -> egui::Color32 {
    rgb(49, 49, 49)
}

fn color_field() -> egui::Color32 {
    rgb(33, 33, 33)
}

fn color_viewport() -> egui::Color32 {
    rgb(29, 30, 32)
}

fn color_border() -> egui::Color32 {
    rgb(58, 58, 58)
}

fn color_border_dark() -> egui::Color32 {
    rgb(24, 24, 24)
}

fn color_text() -> egui::Color32 {
    rgb(214, 214, 214)
}

fn color_text_dim() -> egui::Color32 {
    rgb(160, 160, 160)
}

fn color_text_faint() -> egui::Color32 {
    rgb(118, 118, 118)
}

fn color_accent() -> egui::Color32 {
    rgb(73, 133, 201)
}

fn color_play() -> egui::Color32 {
    rgb(88, 164, 93)
}

fn color_warning() -> egui::Color32 {
    rgb(214, 145, 66)
}

fn color_danger() -> egui::Color32 {
    rgb(190, 83, 73)
}

fn color_camera() -> egui::Color32 {
    rgb(92, 124, 82)
}

fn color_entity() -> egui::Color32 {
    rgb(76, 78, 82)
}

fn color_entity_hover() -> egui::Color32 {
    rgb(94, 96, 102)
}

fn color_grid_minor() -> egui::Color32 {
    rgb(39, 40, 43)
}

fn color_grid_major() -> egui::Color32 {
    rgb(50, 51, 55)
}

fn color_axis_x() -> egui::Color32 {
    rgb(196, 84, 80)
}

fn color_axis_y() -> egui::Color32 {
    rgb(98, 151, 86)
}

fn color_axis_z() -> egui::Color32 {
    rgb(80, 139, 206)
}

fn top_bar_frame() -> egui::Frame {
    egui::Frame::new()
        .fill(color_top_bar())
        .inner_margin(egui::Margin::symmetric(8, 4))
        .stroke(egui::Stroke::new(1.0, color_border_dark()))
}

fn dock_frame() -> egui::Frame {
    egui::Frame::new()
        .fill(color_dock())
        .inner_margin(egui::Margin::symmetric(8, 8))
        .stroke(egui::Stroke::new(1.0, color_border_dark()))
}

fn console_frame() -> egui::Frame {
    egui::Frame::new()
        .fill(rgb(36, 36, 36))
        .inner_margin(egui::Margin::symmetric(8, 6))
        .stroke(egui::Stroke::new(1.0, color_border_dark()))
}

fn viewport_frame() -> egui::Frame {
    egui::Frame::new()
        .fill(color_app_bg())
        .inner_margin(egui::Margin::symmetric(6, 6))
        .stroke(egui::Stroke::new(1.0, color_border_dark()))
}

fn dock_title(ui: &mut egui::Ui, title: &str, detail: &str) {
    egui::Frame::new()
        .fill(color_dock_header())
        .inner_margin(egui::Margin::symmetric(8, 4))
        .stroke(egui::Stroke::new(1.0, color_border_dark()))
        .show(ui, |ui| {
            ui.horizontal(|ui| {
                ui.label(
                    egui::RichText::new(title)
                        .size(12.0)
                        .strong()
                        .color(color_text()),
                );
                if !detail.is_empty() {
                    ui.label(
                        egui::RichText::new(detail)
                            .size(10.5)
                            .monospace()
                            .color(color_text_dim()),
                    );
                }
            });
        });
    ui.add_space(6.0);
}

fn section_caption(ui: &mut egui::Ui, title: &str) {
    ui.add_space(6.0);
    ui.label(
        egui::RichText::new(title)
            .size(10.5)
            .strong()
            .color(color_text_dim()),
    );
    ui.add_space(3.0);
}

fn inspector_text_field(ui: &mut egui::Ui, label: &str, value: &mut String) -> bool {
    ui.horizontal(|ui| {
        ui.add_sized(
            [68.0, 20.0],
            egui::Label::new(
                egui::RichText::new(label)
                    .size(11.0)
                    .color(color_text_dim()),
            ),
        );
        ui.add_sized(
            [ui.available_width(), 20.0],
            egui::TextEdit::singleline(value).desired_width(f32::INFINITY),
        )
        .changed()
    })
    .inner
}

fn kv_row(ui: &mut egui::Ui, label: &str, value: impl std::fmt::Display) {
    ui.horizontal(|ui| {
        ui.add_sized(
            [68.0, 18.0],
            egui::Label::new(
                egui::RichText::new(label)
                    .size(11.0)
                    .color(color_text_dim()),
            ),
        );
        ui.add_sized(
            [ui.available_width(), 18.0],
            egui::Label::new(
                egui::RichText::new(value.to_string())
                    .size(11.0)
                    .monospace()
                    .color(color_text()),
            )
            .truncate(),
        );
    });
}

fn mini_button(ui: &mut egui::Ui, label: &str, tooltip: &str) -> egui::Response {
    ui.add(
        egui::Button::new(egui::RichText::new(label).size(11.0))
            .min_size(egui::vec2(48.0, 20.0))
            .corner_radius(egui::CornerRadius::same(2)),
    )
    .on_hover_text(tooltip)
}

fn mini_toggle(ui: &mut egui::Ui, selected: bool, label: &str, tooltip: &str) -> egui::Response {
    ui.add(
        egui::Button::selectable(selected, egui::RichText::new(label).size(11.0))
            .min_size(egui::vec2(34.0, 20.0))
            .corner_radius(egui::CornerRadius::same(2)),
    )
    .on_hover_text(tooltip)
}

fn status_chip(ui: &mut egui::Ui, label: impl Into<String>, tone: egui::Color32) {
    egui::Frame::new()
        .fill(rgb(34, 34, 34))
        .inner_margin(egui::Margin::symmetric(6, 2))
        .corner_radius(egui::CornerRadius::same(2))
        .stroke(egui::Stroke::new(1.0, color_border()))
        .show(ui, |ui| {
            ui.label(
                egui::RichText::new(label.into())
                    .size(10.5)
                    .monospace()
                    .color(tone),
            );
        });
}

fn configure_editor_style(ctx: &egui::Context) {
    let mut visuals = egui::Visuals::dark();
    visuals.override_text_color = Some(color_text());
    visuals.weak_text_color = Some(color_text_dim());
    visuals.panel_fill = color_dock();
    visuals.window_fill = color_dock();
    visuals.window_stroke = egui::Stroke::new(1.0, color_border_dark());
    visuals.window_shadow = egui::Shadow::NONE;
    visuals.window_corner_radius = egui::CornerRadius::ZERO;
    visuals.menu_corner_radius = egui::CornerRadius::same(2);
    visuals.faint_bg_color = rgb(50, 50, 50);
    visuals.extreme_bg_color = color_field();
    visuals.text_edit_bg_color = Some(color_field());
    visuals.code_bg_color = color_field();
    visuals.warn_fg_color = color_warning();
    visuals.error_fg_color = color_danger();
    visuals.hyperlink_color = color_accent();
    visuals.button_frame = true;
    visuals.collapsing_header_frame = false;
    visuals.indent_has_left_vline = true;
    visuals.selection.bg_fill = color_accent();
    visuals.selection.stroke = egui::Stroke::new(1.0, rgb(235, 241, 248));
    visuals.widgets.noninteractive.bg_fill = color_dock();
    visuals.widgets.noninteractive.weak_bg_fill = color_app_bg();
    visuals.widgets.noninteractive.bg_stroke = egui::Stroke::new(1.0, color_border());
    visuals.widgets.noninteractive.fg_stroke = egui::Stroke::new(1.0, color_text());
    visuals.widgets.inactive.bg_fill = rgb(51, 51, 51);
    visuals.widgets.inactive.weak_bg_fill = rgb(46, 46, 46);
    visuals.widgets.inactive.bg_stroke = egui::Stroke::new(1.0, color_border());
    visuals.widgets.inactive.fg_stroke = egui::Stroke::new(1.0, color_text());
    visuals.widgets.hovered.bg_fill = rgb(61, 61, 61);
    visuals.widgets.hovered.weak_bg_fill = rgb(58, 58, 58);
    visuals.widgets.hovered.bg_stroke = egui::Stroke::new(1.0, rgb(82, 82, 82));
    visuals.widgets.hovered.fg_stroke = egui::Stroke::new(1.0, rgb(238, 238, 238));
    visuals.widgets.active.bg_fill = color_accent();
    visuals.widgets.active.weak_bg_fill = rgb(58, 96, 139);
    visuals.widgets.active.bg_stroke = egui::Stroke::new(1.0, rgb(93, 148, 210));
    visuals.widgets.active.fg_stroke = egui::Stroke::new(1.0, rgb(245, 247, 250));
    visuals.widgets.open = visuals.widgets.hovered;
    for widget in [
        &mut visuals.widgets.noninteractive,
        &mut visuals.widgets.inactive,
        &mut visuals.widgets.hovered,
        &mut visuals.widgets.active,
        &mut visuals.widgets.open,
    ] {
        widget.corner_radius = egui::CornerRadius::same(2);
        widget.expansion = 0.0;
    }
    ctx.set_visuals(visuals);

    let mut style = (*ctx.style()).clone();
    style.text_styles = BTreeMap::from([
        (
            egui::TextStyle::Heading,
            egui::FontId::new(15.0, egui::FontFamily::Proportional),
        ),
        (
            egui::TextStyle::Body,
            egui::FontId::new(12.0, egui::FontFamily::Proportional),
        ),
        (
            egui::TextStyle::Button,
            egui::FontId::new(11.5, egui::FontFamily::Proportional),
        ),
        (
            egui::TextStyle::Small,
            egui::FontId::new(10.0, egui::FontFamily::Proportional),
        ),
        (
            egui::TextStyle::Monospace,
            egui::FontId::new(11.0, egui::FontFamily::Monospace),
        ),
    ]);
    style.drag_value_text_style = egui::TextStyle::Monospace;
    style.animation_time = 0.08;
    style.spacing.item_spacing = egui::vec2(5.0, 4.0);
    style.spacing.button_padding = egui::vec2(7.0, 3.0);
    style.spacing.window_margin = egui::Margin::symmetric(6, 6);
    style.spacing.menu_margin = egui::Margin::symmetric(6, 4);
    style.spacing.indent = 14.0;
    style.spacing.interact_size = egui::vec2(18.0, 20.0);
    style.spacing.icon_width = 14.0;
    style.spacing.icon_width_inner = 8.0;
    style.spacing.icon_spacing = 4.0;
    style.spacing.text_edit_width = 160.0;
    ctx.set_style(style);
}

fn initial_project_path() -> String {
    let relative = PathBuf::from("packages/borealis/zenith.ztproj");
    if relative.exists() {
        return normalize_path(&relative);
    }

    normalize_path(
        &PathBuf::from(env!("CARGO_MANIFEST_DIR"))
            .join("../..")
            .join("packages/borealis/zenith.ztproj"),
    )
}

fn normalize_path(path: &PathBuf) -> String {
    path.to_string_lossy().replace('\\', "/")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn editor_starts_with_stable_layout_and_project_contract() {
        let ctx = egui::Context::default();
        let app = BorealisEditorApp::new_with_context(&ctx);
        let snapshot = app.smoke_snapshot();

        assert!(snapshot
            .project_path
            .ends_with("packages/borealis/zenith.ztproj"));
        assert_eq!(snapshot.scene_path, "scenes/sample.scene.json");
        assert_eq!(snapshot.selected_entity_id, "player");
        assert!(snapshot.has_project_summary);
        assert_eq!(snapshot.loaded_entity_count, 3);
        assert_eq!(snapshot.selected_entity_name, "Player");
        assert_eq!(snapshot.selected_entity_position, (120, 96));
        assert!(snapshot.scene_candidate_count > 0);
        assert!(snapshot.auto_sync_preview);
        assert_eq!(
            snapshot.dock_ids,
            vec![
                TOP_BAR_ID,
                HIERARCHY_PROJECT_DOCK_ID,
                INSPECTOR_DOCK_ID,
                CONSOLE_DOCK_ID,
                VIEWPORT_ID,
            ]
        );
    }

    #[test]
    fn moving_entity_updates_loaded_scene_model() {
        let ctx = egui::Context::default();
        let mut app = BorealisEditorApp::new_with_context(&ctx);

        app.update_scene_entity_position("player", 180.0, 140.0);

        let entity = app
            .scene_entity_info("player")
            .expect("player entity should exist");
        assert_eq!(entity.transform.x, 180.0);
        assert_eq!(entity.transform.y, 140.0);
    }

    #[test]
    fn viewport_scene_coordinates_roundtrip() {
        let rect = egui::Rect::from_min_size(egui::pos2(24.0, 48.0), egui::vec2(640.0, 360.0));
        let viewport = scene_to_viewport(rect, egui::vec2(92.0, 74.0), 1.35, 180.0, 140.0);
        let scene = viewport_to_scene(rect, egui::vec2(92.0, 74.0), 1.35, viewport);

        assert_eq!(scene, (180.0, 140.0));
    }

    #[test]
    fn editing_components_marks_scene_dirty_and_updates_entity() {
        let ctx = egui::Context::default();
        let mut app = BorealisEditorApp::new_with_context(&ctx);

        app.add_component_to_entity("player", "collider2d");
        app.remove_component_from_entity("player", 1);

        let entity = app
            .scene_entity_info("player")
            .expect("player entity should exist");
        assert_eq!(entity.components, vec!["sprite", "collider2d"]);
        assert!(app.scene_dirty);
    }

    #[test]
    fn creating_3d_primitive_switches_to_3d_scene_view() {
        let ctx = egui::Context::default();
        let mut app = BorealisEditorApp::new_with_context(&ctx);

        app.create_scene_primitive_in_model("cube3d", "Cube 3D");

        let entity = app
            .scene_entity_info(&app.selected_scene_entity)
            .expect("created 3d entity should exist");
        assert_eq!(app.viewport_tab, ViewportTab::Scene3D);
        assert!(entity
            .components
            .iter()
            .any(|component| component == "cube3d"));
        assert_eq!(entity.layer, "world3d");
        assert!(app.scene_dirty);
    }

    #[test]
    fn dropping_3d_asset_creates_model_entity() {
        let ctx = egui::Context::default();
        let mut app = BorealisEditorApp::new_with_context(&ctx);
        let payload = ProjectDragPayload {
            label: "triangle.obj".to_string(),
            path: "assets/triangle.obj".to_string(),
            kind: ProjectTreeItemKind::Asset,
        };

        app.create_scene_entity_from_asset(&payload, 16.0, 24.0, 8.0);

        let entity = app
            .scene_entity_info(&app.selected_scene_entity)
            .expect("dropped model entity should exist");
        assert_eq!(app.viewport_tab, ViewportTab::Scene3D);
        assert_eq!(entity.name, "triangle");
        assert_eq!(entity.model_asset, "assets/triangle.obj");
        assert_eq!(entity.transform.z, 8.0);
        assert!(entity
            .components
            .iter()
            .any(|component| component == "model3d"));
    }

    #[test]
    fn starting_preview_syncs_current_scene_in_mock_mode() {
        let ctx = egui::Context::default();
        let mut app = BorealisEditorApp::new_with_context(&ctx);

        app.start_preview_from_ui();

        assert_eq!(app.preview.state(), ConnectionState::Connected);
        assert!(app
            .transcript
            .iter()
            .any(|line| line.contains("open_scene") && line.contains("sample.scene.json")));
        assert!(app
            .transcript
            .iter()
            .any(|line| line.contains("Cena sincronizada com preview")));
    }

    #[test]
    fn editing_scene_metadata_marks_scene_dirty() {
        let ctx = egui::Context::default();
        let mut app = BorealisEditorApp::new_with_context(&ctx);

        app.apply_scene_document_edit(
            "sample-edited".to_string(),
            "scene:sample-edited".to_string(),
        );

        let document = app
            .scene_document
            .as_ref()
            .expect("scene document should stay loaded");
        assert_eq!(document.name, "sample-edited");
        assert_eq!(document.document_id, "scene:sample-edited");
        assert!(app.scene_dirty);
    }

    #[test]
    fn creating_and_removing_entity_updates_loaded_scene_model() {
        let ctx = egui::Context::default();
        let mut app = BorealisEditorApp::new_with_context(&ctx);

        let initial_count = app
            .scene_document
            .as_ref()
            .expect("scene should be loaded")
            .entities
            .len();

        app.create_scene_entity_in_model();

        let created_id = app.selected_scene_entity.clone();
        let created = app
            .scene_entity_info(&created_id)
            .expect("created entity should exist");
        assert_eq!(created.name, "Entity 4");
        assert_eq!(created.layer, "default");
        assert!(app.scene_dirty);
        assert_eq!(
            app.scene_document
                .as_ref()
                .expect("scene should stay loaded")
                .entities
                .len(),
            initial_count + 1
        );

        app.remove_selected_scene_entity_from_model();

        assert!(app.scene_entity_info(&created_id).is_none());
        assert_eq!(
            app.scene_document
                .as_ref()
                .expect("scene should stay loaded")
                .entities
                .len(),
            initial_count
        );
    }

    #[test]
    fn changing_scene_is_blocked_when_current_scene_is_dirty() {
        let ctx = egui::Context::default();
        let mut app = BorealisEditorApp::new_with_context(&ctx);

        app.apply_scene_document_edit(
            "sample-edited".to_string(),
            "scene:sample-edited".to_string(),
        );
        app.activate_scene_from_ui("examples/scene_entities_v1.zt".to_string(), false);

        let document = app
            .scene_document
            .as_ref()
            .expect("scene should remain loaded");
        assert_eq!(document.name, "sample-edited");
        assert_eq!(app.scene_path, "scenes/sample.scene.json");
        assert!(app
            .transcript
            .iter()
            .any(|line| line.contains("alteracoes nao salvas")));
    }
}
