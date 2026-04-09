fn main() {
    tauri_build::try_build(tauri_build::Attributes::new().app_manifest(
        tauri_build::AppManifest::new().commands(&[
            "get_file_tree",
            "read_file",
            "write_file",
            "run_compiler",
            "pick_file",
            "pick_folder",
            "pick_save_path",
            "terminal_create",
            "terminal_write",
            "terminal_resize",
            "terminal_kill",
            "greet",
        ]),
    ))
    .expect("failed to build Tauri application");
}
