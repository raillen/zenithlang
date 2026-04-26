mod commands;
mod models;
mod services;
mod state;
mod utils;

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![
            commands::project::create_borealis_project,
            commands::project::load_studio_home,
            commands::project::load_studio_snapshot,
            commands::preview::pause_preview,
            commands::preview::poll_preview,
            commands::preview::start_preview,
            commands::preview::stop_preview,
            commands::file::read_text_file,
            commands::file::write_text_file,
            commands::file::write_project_text_file,
        ])
        .run(tauri::generate_context!())
        .expect("failed to run Borealis Studio");
}

#[cfg(test)]
mod tests {
    use crate::models::project::StudioLayout;
    use crate::services::preview_runner::preview_scene_target;
    use crate::services::project_manager::{studio_layout, template_scene_json};
    use crate::utils::paths::{normalize_path, normalize_project_file};
    use serde_json::Value;

    fn workspace_root() -> std::path::PathBuf {
        studio_layout().base_root
    }

    #[test]
    fn workspace_root_finds_repo_root() {
        let root = workspace_root();
        assert!(root.join("packages/borealis/zenith.ztproj").exists());
    }

    #[test]
    fn project_file_accepts_project_directory() {
        let root = workspace_root();
        let project_file = normalize_project_file(root.join("packages/borealis")).unwrap();
        assert_eq!(
            normalize_path(&project_file),
            normalize_path(&root.join("packages/borealis/zenith.ztproj"))
        );
    }

    #[test]
    fn generated_template_scene_is_valid_json() {
        let scene = template_scene_json("Projeto Teste", "scripted3d");
        let parsed: Value = serde_json::from_str(&scene).unwrap();
        assert_eq!(parsed.get("version").and_then(Value::as_i64), Some(2));
        assert!(parsed
            .get("environment")
            .and_then(Value::as_object)
            .is_some());
        assert!(parsed.get("render").and_then(Value::as_object).is_some());
        assert!(parsed.get("audio").and_then(Value::as_object).is_some());
        assert!(parsed.get("entities").and_then(Value::as_array).is_some());
    }

    #[test]
    fn preview_scene_target_uses_sdk_relative_path() {
        let root = workspace_root();
        let sdk_root = root.join("tools/borealis-studio/runtime/sdk");
        let layout = StudioLayout {
            app_root: root.join("tools/borealis-studio"),
            base_root: root,
            repo_root: None,
            sdk_root: Some(sdk_root.clone()),
        };

        let (path, ipc_path) = preview_scene_target(&layout);
        assert!(path.starts_with(&sdk_root));
        assert_eq!(
            ipc_path,
            ".ztc-tmp/borealis-studio/preview.scene.json".to_string()
        );
    }
}
