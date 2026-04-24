mod app;
mod ipc;
mod messages;
mod project;
mod scene_document;

use app::BorealisEditorApp;
use eframe::egui;

fn main() -> eframe::Result<()> {
    tracing_subscriber::fmt()
        .with_env_filter(
            std::env::var("BOREALIS_EDITOR_LOG")
                .unwrap_or_else(|_| "info,borealis_editor=debug".to_owned()),
        )
        .with_target(false)
        .compact()
        .init();

    let native_options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size([1440.0, 900.0])
            .with_min_inner_size([1120.0, 700.0]),
        ..Default::default()
    };

    eframe::run_native(
        "Borealis Editor",
        native_options,
        Box::new(|cc| Ok(Box::new(BorealisEditorApp::new(cc)))),
    )
}
