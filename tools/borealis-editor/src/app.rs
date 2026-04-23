use crate::ipc::PreviewBridge;
use eframe::egui;

pub struct BorealisEditorApp {
    preview: PreviewBridge,
    project_path: String,
    scene_path: String,
    selected_entity_id: String,
    last_sent_line: String,
    transcript: Vec<String>,
}

impl BorealisEditorApp {
    pub fn new(_cc: &eframe::CreationContext<'_>) -> Self {
        Self {
            preview: PreviewBridge::new(),
            project_path: "packages/borealis/zenith.ztproj".to_owned(),
            scene_path: "scenes/sample.scene.json".to_owned(),
            selected_entity_id: "player".to_owned(),
            last_sent_line: String::new(),
            transcript: vec![
                "Borealis Editor scaffold inicializado.".to_owned(),
                "Preview IPC v1: jsonl-stdio.".to_owned(),
            ],
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
}

impl eframe::App for BorealisEditorApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        egui::TopBottomPanel::top("top_bar").show(ctx, |ui| {
            ui.horizontal_wrapped(|ui| {
                ui.heading("Borealis Editor");
                ui.separator();
                ui.label("Stack: Rust + egui");
                ui.separator();
                ui.label(format!("Transport: {:?}", self.preview.transport()));
                ui.separator();
                ui.label(format!("State: {:?}", self.preview.state()));
                ui.separator();
                ui.label(format!("Preview: {:?}", self.preview.current_status()));
            });
        });

        egui::SidePanel::left("left_panel")
            .resizable(true)
            .default_width(300.0)
            .show(ctx, |ui| {
                ui.heading("Preview Bridge");
                ui.label("Este painel controla o preview separado e mostra o protocolo inicial.");
                ui.add_space(8.0);

                ui.label("Projeto");
                ui.text_edit_singleline(&mut self.project_path);

                ui.label("Cena");
                ui.text_edit_singleline(&mut self.scene_path);

                ui.label("Entidade selecionada");
                ui.text_edit_singleline(&mut self.selected_entity_id);

                ui.add_space(8.0);

                if ui.button("Start Preview").clicked() {
                    match self.preview.start_preview(Some(self.project_path.clone())) {
                        Ok(_) => self.flush_preview_messages(),
                        Err(error) => {
                            self.push_transcript(format!("erro ao iniciar preview: {error}"))
                        }
                    }
                }

                if ui.button("Open Scene").clicked() {
                    match self.preview.open_scene(self.scene_path.clone()) {
                        Ok(_) => self.flush_preview_messages(),
                        Err(error) => self.push_transcript(format!("erro ao abrir cena: {error}")),
                    }
                }

                if ui.button("Enter Play Mode").clicked() {
                    match self.preview.enter_play_mode() {
                        Ok(_) => self.flush_preview_messages(),
                        Err(error) => {
                            self.push_transcript(format!("erro ao entrar em play mode: {error}"))
                        }
                    }
                }

                if ui.button("Select Entity").clicked() {
                    match self.preview.select_entity(self.selected_entity_id.clone()) {
                        Ok(_) => self.flush_preview_messages(),
                        Err(error) => {
                            self.push_transcript(format!("erro ao selecionar entidade: {error}"))
                        }
                    }
                }

                if ui.button("Ping Preview").clicked() {
                    match self.preview.ping("editor-ping".to_owned()) {
                        Ok(_) => self.flush_preview_messages(),
                        Err(error) => self.push_transcript(format!("erro no ping: {error}")),
                    }
                }

                if ui.button("Stop Preview").clicked() {
                    match self.preview.stop_preview() {
                        Ok(_) => self.flush_preview_messages(),
                        Err(error) => {
                            self.push_transcript(format!("erro ao parar preview: {error}"))
                        }
                    }
                }

                ui.add_space(12.0);
                ui.heading("Ultima linha enviada");
                ui.monospace(if self.last_sent_line.is_empty() {
                    "(nenhuma ainda)"
                } else {
                    &self.last_sent_line
                });
            });

        egui::CentralPanel::default().show(ctx, |ui| {
            ui.heading("Viewport Placeholder");
            ui.label("Por enquanto, o viewport real ainda nao existe aqui.");
            ui.label("Este scaffold valida a base do app e o bridge editor <-> preview.");
            ui.add_space(12.0);

            let available = ui.available_size();
            let desired = egui::vec2(available.x.max(200.0), (available.y * 0.45).max(140.0));
            let (rect, _) = ui.allocate_exact_size(desired, egui::Sense::hover());
            let visuals = ui.visuals();
            ui.painter().rect_filled(rect, 6.0, visuals.faint_bg_color);
            ui.painter().text(
                rect.center(),
                egui::Align2::CENTER_CENTER,
                "Preview sidecar will live here later",
                egui::FontId::proportional(18.0),
                visuals.strong_text_color(),
            );

            ui.add_space(16.0);
            ui.heading("Transcript");
            egui::ScrollArea::vertical()
                .stick_to_bottom(true)
                .show(ui, |ui| {
                    for line in &self.transcript {
                        ui.monospace(line);
                    }
                });
        });
    }
}
