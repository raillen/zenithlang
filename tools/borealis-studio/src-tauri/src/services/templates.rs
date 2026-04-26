use std::fs;
use std::path::{Path, PathBuf};

use serde_json::{json, Value};

use crate::models::project::{ProjectTemplate, ProjectTemplateAsset, StudioLayout};
use crate::utils::paths::{normalize_path, resolve_project_path};

pub fn project_templates(layout: &StudioLayout) -> Vec<ProjectTemplate> {
    if let Some(sdk_templates) = load_sdk_templates(layout) {
        if !sdk_templates.is_empty() {
            return sdk_templates;
        }
    }
    fallback_project_templates()
}

pub fn fallback_project_templates() -> Vec<ProjectTemplate> {
    vec![
        ProjectTemplate {
            id: "empty3d".to_string(),
            name: "Empty 3D".to_string(),
            summary: "Cena 3D limpa com camera, cubo e espaco para os primeiros sistemas."
                .to_string(),
            default_name: "Projeto Borealis 3D".to_string(),
            tags: vec!["3D".to_string(), "starter".to_string()],
            scene: None,
            main_script: None,
            assets: Vec::new(),
            source: Some("fallback".to_string()),
        },
        ProjectTemplate {
            id: "topdown2d".to_string(),
            name: "Top-down 2D".to_string(),
            summary: "Cena 2D pequena para movimento, sprites e testes iniciais de gameplay."
                .to_string(),
            default_name: "Projeto Borealis 2D".to_string(),
            tags: vec!["2D".to_string(), "gameplay".to_string()],
            scene: None,
            main_script: None,
            assets: Vec::new(),
            source: Some("fallback".to_string()),
        },
        ProjectTemplate {
            id: "scripted3d".to_string(),
            name: "Scripted 3D".to_string(),
            summary: "Amostra 3D com script anexado para iterar no preview.".to_string(),
            default_name: "Projeto Borealis Scripted".to_string(),
            tags: vec!["3D".to_string(), "scripts".to_string()],
            scene: None,
            main_script: None,
            assets: Vec::new(),
            source: Some("fallback".to_string()),
        },
    ]
}

fn load_sdk_templates(layout: &StudioLayout) -> Option<Vec<ProjectTemplate>> {
    let sdk_root = layout.sdk_root.as_ref()?;
    let templates_file = sdk_root.join("templates/templates.json");
    let raw = fs::read_to_string(&templates_file).ok()?;
    let value = serde_json::from_str::<Value>(&raw).ok()?;
    let items = value.as_array()?;
    let mut templates = Vec::new();

    for item in items {
        let Some(id) = item.get("id").and_then(Value::as_str) else {
            continue;
        };
        let name = item
            .get("name")
            .and_then(Value::as_str)
            .unwrap_or(id)
            .to_string();
        let summary = item
            .get("summary")
            .and_then(Value::as_str)
            .unwrap_or("Template do SDK Borealis.")
            .to_string();
        let default_name = item
            .get("defaultName")
            .or_else(|| item.get("default_name"))
            .and_then(Value::as_str)
            .unwrap_or(&name)
            .to_string();
        let tags = item
            .get("tags")
            .and_then(Value::as_array)
            .map(|items| {
                items
                    .iter()
                    .filter_map(Value::as_str)
                    .map(str::to_string)
                    .collect::<Vec<_>>()
            })
            .filter(|items| !items.is_empty())
            .unwrap_or_else(|| vec!["SDK".to_string()]);
        let assets = item
            .get("assets")
            .and_then(Value::as_array)
            .map(|items| {
                items
                    .iter()
                    .filter_map(|asset| {
                        let source = asset.get("source").and_then(Value::as_str)?;
                        let target = asset
                            .get("target")
                            .and_then(Value::as_str)
                            .unwrap_or(source);
                        Some(ProjectTemplateAsset {
                            source: source.to_string(),
                            target: target.to_string(),
                        })
                    })
                    .collect::<Vec<_>>()
            })
            .unwrap_or_default();

        templates.push(ProjectTemplate {
            id: id.to_string(),
            name,
            summary,
            default_name,
            tags,
            scene: item
                .get("scene")
                .and_then(Value::as_str)
                .map(str::to_string),
            main_script: item
                .get("mainScript")
                .or_else(|| item.get("main_script"))
                .and_then(Value::as_str)
                .map(str::to_string),
            assets,
            source: Some(normalize_path(&templates_file)),
        });
    }

    Some(templates)
}

pub fn resolve_project_template(
    layout: &StudioLayout,
    template_id: &str,
) -> ProjectTemplate {
    project_templates(layout)
        .into_iter()
        .find(|template| template.id == template_id)
        .unwrap_or_else(|| {
            fallback_project_templates()
                .into_iter()
                .next()
                .expect("fallback project templates must not be empty")
        })
}

pub fn project_scene_json(
    layout: &StudioLayout,
    template: &ProjectTemplate,
    project_name: &str,
    template_id: &str,
) -> String {
    if let Some(scene_path) = &template.scene {
        if let Some(mut value) = read_template_json(layout, scene_path) {
            if let Some(object) = value.as_object_mut() {
                object.insert("name".to_string(), Value::String(project_name.to_string()));
            }
            return format!(
                "{}\n",
                serde_json::to_string_pretty(&value).unwrap_or_else(|_| "{}".to_string())
            );
        }
    }
    template_scene_json(project_name, template_id)
}

pub fn template_scene_json(project_name: &str, template_id: &str) -> String {
    let (document_id, entities) = match template_id {
        "topdown2d" => (
            "scene:main_2d",
            json!([
                {
                    "stable_id": "camera-main",
                    "name": "Camera 2D",
                    "layer": "camera",
                    "tags": ["camera", "2d"],
                    "transform": { "x": 0, "y": 0, "z": 120, "rotation": 0, "scale_x": 1, "scale_y": 1, "scale_z": 1 },
                    "components": [{ "kind": "camera2d", "projection": "orthographic" }]
                },
                {
                    "stable_id": "player",
                    "name": "Player",
                    "layer": "actors",
                    "tags": ["player", "controllable"],
                    "transform": { "x": 0, "y": 0, "z": 0, "rotation": 0, "scale_x": 1, "scale_y": 1, "scale_z": 1 },
                    "components": [{ "kind": "script", "script": "src/app/main.zt" }]
                }
            ]),
        ),
        "scripted3d" => (
            "scene:main_scripted_3d",
            json!([
                {
                    "stable_id": "camera-main",
                    "name": "Camera 3D",
                    "layer": "world3d",
                    "tags": ["camera", "3d"],
                    "transform": { "x": 120, "y": -120, "z": 120, "rotation_x": -35, "rotation_y": 35, "rotation": 0, "scale_x": 1, "scale_y": 1, "scale_z": 1 },
                    "components": [{ "kind": "camera3d", "projection": "perspective" }]
                },
                {
                    "stable_id": "scripted-cube",
                    "name": "Scripted Cube",
                    "layer": "world3d",
                    "tags": ["3d", "scripted"],
                    "transform": { "x": 0, "y": 0, "z": 24, "rotation": 0, "scale_x": 1, "scale_y": 1, "scale_z": 1 },
                    "components": [
                        { "kind": "cube3d", "size": 48 },
                        { "kind": "script", "script": "src/app/main.zt" }
                    ]
                }
            ]),
        ),
        _ => (
            "scene:main_3d",
            json!([
                {
                    "stable_id": "camera-main",
                    "name": "Camera 3D",
                    "layer": "world3d",
                    "tags": ["camera", "3d"],
                    "transform": { "x": 120, "y": -120, "z": 120, "rotation_x": -35, "rotation_y": 35, "rotation": 0, "scale_x": 1, "scale_y": 1, "scale_z": 1 },
                    "components": [{ "kind": "camera3d", "projection": "perspective" }]
                },
                {
                    "stable_id": "cube-01",
                    "name": "Cube 01",
                    "layer": "world3d",
                    "tags": ["3d"],
                    "transform": { "x": 0, "y": 0, "z": 24, "rotation": 0, "scale_x": 1, "scale_y": 1, "scale_z": 1 },
                    "components": [{ "kind": "cube3d", "size": 48 }]
                }
            ]),
        ),
    };

    let scene = json!({
        "version": 2,
        "name": project_name,
        "document_id": document_id,
        "environment": {
            "skybox": { "mode": "solid", "color": "#1c1f26" },
            "ambient": { "color": "#ffffff", "intensity": 0.4 },
            "fog": { "enabled": false, "color": "#9ca3af", "density": 0.05 },
            "weather": { "preset": "clear" }
        },
        "render": {
            "quality": { "profile": "medium" },
            "postfx": { "fxaa": false, "bloom": 0.0, "vignette": 0.0 },
            "camera": {}
        },
        "audio": {
            "listener": {},
            "mix": { "master": 1.0, "music": 0.8, "sfx": 1.0 }
        },
        "entities": entities,
    });

    format!(
        "{}\n",
        serde_json::to_string_pretty(&scene).unwrap_or_else(|_| "{}".to_string())
    )
}

pub fn project_main_script(
    layout: &StudioLayout,
    template: &ProjectTemplate,
    slug: &str,
    template_id: &str,
) -> String {
    if let Some(script_path) = &template.main_script {
        if let Some(content) = read_template_text(layout, script_path) {
            return content;
        }
    }
    template_main_script(slug, template_id)
}

pub fn template_main_script(slug: &str, _template_id: &str) -> String {
    format!(
        "namespace app.main\n\npublic func update(entity_id: text, dt: float) -> result<void, core.Error>\n    return success()\nend\n\n-- Project: {}\n",
        slug
    )
}

pub fn template_triangle_obj() -> String {
    "o Triangle\nv 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n".to_string()
}

pub fn copy_template_assets(
    layout: &StudioLayout,
    template: &ProjectTemplate,
    project_dir: &Path,
) -> Result<(), String> {
    for asset in &template.assets {
        let Some(source) = template_path(layout, &asset.source) else {
            continue;
        };
        if !source.exists() || !source.is_file() {
            continue;
        }
        let target = resolve_project_path(project_dir, &asset.target);
        if let Some(parent) = target.parent() {
            fs::create_dir_all(parent).map_err(|error| {
                format!(
                    "failed to create template asset folder {}: {error}",
                    normalize_path(parent)
                )
            })?;
        }
        fs::copy(&source, &target).map_err(|error| {
            format!(
                "failed to copy template asset {} to {}: {error}",
                normalize_path(&source),
                normalize_path(&target)
            )
        })?;
    }
    Ok(())
}

pub fn read_template_json(layout: &StudioLayout, path: &str) -> Option<Value> {
    read_template_text(layout, path).and_then(|raw| serde_json::from_str::<Value>(&raw).ok())
}

pub fn read_template_text(layout: &StudioLayout, path: &str) -> Option<String> {
    let template_path = template_path(layout, path)?;
    fs::read_to_string(template_path).ok()
}

fn template_path(layout: &StudioLayout, path: &str) -> Option<PathBuf> {
    let raw = PathBuf::from(path);
    if raw.is_absolute() {
        return Some(raw);
    }
    if let Some(sdk_root) = &layout.sdk_root {
        return Some(sdk_root.join(path));
    }
    Some(layout.app_root.join(path))
}
