use std::fs;

use crate::models::console::ConsoleLine;
use crate::models::project::{StudioHome, StudioSnapshot};
use crate::services::asset_pipeline::{collect_assets, collect_scripts};
use crate::services::file_service::write_text_at_path;
use crate::services::project_manager::{
    copy_template_assets, default_project_path, default_projects_dir, documentation_links,
    load_editor_manifest, project_main_script, project_manifest, project_scene_json,
    project_templates, read_project_name, resolve_project_template, resolve_studio_path,
    runtime_status, studio_layout, template_triangle_obj,
};
use crate::services::scene_manager::read_scene_document;
use crate::utils::paths::{clean_project_name, normalize_path, normalize_project_file, slugify_project_name};

#[tauri::command]
pub fn load_studio_snapshot(project_path: Option<String>) -> Result<StudioSnapshot, String> {
    let layout = studio_layout();
    let project_path = normalize_project_file(
        project_path
            .map(|path| resolve_studio_path(&path))
            .unwrap_or_else(|| default_project_path(&layout)),
    )?;
    let project_dir = project_path
        .parent()
        .ok_or_else(|| "project path has no parent".to_string())?
        .to_path_buf();
    let project_name =
        read_project_name(&project_path).unwrap_or_else(|| "Borealis".to_string());
    let scene_path = if project_dir.join("scenes/sample_3d.scene.json").exists() {
        project_dir.join("scenes/sample_3d.scene.json")
    } else if project_dir.join("scenes/sample.scene.json").exists() {
        project_dir.join("scenes/sample.scene.json")
    } else {
        project_dir.join("scenes/main.scene.json")
    };
    let scene = read_scene_document(&project_dir, &scene_path)?;
    let assets = collect_assets(&project_dir, &project_dir)?;
    let scripts = collect_scripts(&project_dir, &project_dir)?;

    Ok(StudioSnapshot {
        project_name,
        project_path: normalize_path(&project_path),
        project_root: normalize_path(&project_dir),
        scene,
        assets,
        scripts,
        console: vec![ConsoleLine {
            id: "studio-ready".to_string(),
            level: "info".to_string(),
            source: "studio".to_string(),
            message: format!(
                "Loaded {} through Tauri.",
                normalize_path(&project_path)
            ),
        }],
    })
}

#[tauri::command]
pub fn load_studio_home() -> StudioHome {
    let layout = studio_layout();
    let (runtime_mode, runtime_status) = runtime_status(&layout);
    StudioHome {
        workspace_root: normalize_path(&layout.base_root),
        app_root: normalize_path(&layout.app_root),
        repo_root: layout
            .repo_root
            .as_ref()
            .map(|path| normalize_path(path)),
        sdk_root: layout.sdk_root.as_ref().map(|path| normalize_path(path)),
        runtime_mode,
        runtime_status,
        default_project_path: normalize_path(&default_project_path(&layout)),
        default_projects_dir: normalize_path(&default_projects_dir(&layout.base_root)),
        editor_manifest: load_editor_manifest(&layout),
        templates: project_templates(&layout),
        docs: documentation_links(&layout),
    }
}

#[tauri::command]
pub fn create_borealis_project(
    project_name: String,
    parent_dir: String,
    template_id: String,
) -> Result<StudioSnapshot, String> {
    let layout = studio_layout();
    let clean_name = clean_project_name(&project_name);
    let slug = slugify_project_name(&clean_name);
    let parent = if parent_dir.trim().is_empty() {
        default_projects_dir(&layout.base_root)
    } else {
        resolve_studio_path(parent_dir.trim())
    };
    let project_dir = parent.join(&slug);

    if project_dir.exists() {
        return Err(format!(
            "project folder already exists: {}",
            normalize_path(&project_dir)
        ));
    }

    fs::create_dir_all(project_dir.join("scenes")).map_err(|error| {
        format!(
            "failed to create project scenes folder {}: {error}",
            normalize_path(&project_dir.join("scenes"))
        )
    })?;
    fs::create_dir_all(project_dir.join("src/app")).map_err(|error| {
        format!(
            "failed to create project source folder {}: {error}",
            normalize_path(&project_dir.join("src/app"))
        )
    })?;
    fs::create_dir_all(project_dir.join("assets")).map_err(|error| {
        format!(
            "failed to create project assets folder {}: {error}",
            normalize_path(&project_dir.join("assets"))
        )
    })?;

    let project_file = project_dir.join("zenith.ztproj");
    write_text_at_path(&project_file, project_manifest(&slug))?;
    let template = resolve_project_template(&layout, &template_id);
    write_text_at_path(
        &project_dir.join("scenes/main.scene.json"),
        project_scene_json(&layout, &template, &clean_name, &template_id),
    )?;
    write_text_at_path(
        &project_dir.join("src/app/main.zt"),
        project_main_script(&layout, &template, &slug, &template_id),
    )?;
    copy_template_assets(&layout, &template, &project_dir)?;
    if !project_dir.join("assets/triangle.obj").exists() {
        write_text_at_path(
            &project_dir.join("assets/triangle.obj"),
            template_triangle_obj(),
        )?;
    }

    let mut snapshot = load_studio_snapshot(Some(normalize_path(&project_file)))?;
    snapshot.console.push(ConsoleLine {
        id: "project-created".to_string(),
        level: "info".to_string(),
        source: "studio".to_string(),
        message: format!("Created project {}.", normalize_path(&project_file)),
    });
    Ok(snapshot)
}
