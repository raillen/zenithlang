mod app;
mod ipc;
mod messages;

use app::BorealisEditorApp;

fn main() -> eframe::Result<()> {
    tracing_subscriber::fmt()
        .with_env_filter(
            std::env::var("BOREALIS_EDITOR_LOG")
                .unwrap_or_else(|_| "info,borealis_editor=debug".to_owned()),
        )
        .with_target(false)
        .compact()
        .init();

    let native_options = eframe::NativeOptions::default();

    eframe::run_native(
        "Borealis Editor",
        native_options,
        Box::new(|cc| Ok(Box::new(BorealisEditorApp::new(cc)))),
    )
}
