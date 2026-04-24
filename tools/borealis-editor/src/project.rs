use std::fs;
use std::path::{Path, PathBuf};

#[derive(Debug, Clone, Default)]
pub struct ProjectSummary {
    pub project_file: String,
    pub project_dir: String,
    pub name: String,
    pub version: String,
    pub kind: String,
    pub root_namespace: String,
    pub source_root: String,
    pub zdoc_root: String,
    pub accessibility_profile: String,
    pub telemetry: Option<bool>,
    pub module_count: usize,
    pub example_count: usize,
    pub asset_count: usize,
    pub has_scene_module: bool,
    pub has_editor_module: bool,
    pub has_assets_module: bool,
    pub scene_candidates: Vec<String>,
    pub example_candidates: Vec<String>,
    pub asset_candidates: Vec<String>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ProjectTreeItemKind {
    SceneCandidate,
    Example,
    Asset,
    Module,
}

#[derive(Debug, Clone)]
pub struct ProjectTreeItem {
    pub id: String,
    pub label: String,
    pub path: String,
    pub kind: ProjectTreeItemKind,
}

#[derive(Debug, Clone)]
pub struct ProjectTreeSection {
    pub title: String,
    pub items: Vec<ProjectTreeItem>,
}

#[derive(Debug, Clone, Default)]
pub struct ProjectTree {
    pub sections: Vec<ProjectTreeSection>,
}

impl ProjectTree {
    pub fn from_summary(summary: &ProjectSummary) -> Self {
        let mut sections = Vec::new();

        let mut scene_items = items_from_paths(
            "scene",
            &summary.scene_candidates,
            ProjectTreeItemKind::SceneCandidate,
        );
        if summary.has_scene_module {
            scene_items.push(ProjectTreeItem {
                id: "module:src/borealis/game/scene.zt".to_string(),
                label: "scene.zt".to_string(),
                path: "src/borealis/game/scene.zt".to_string(),
                kind: ProjectTreeItemKind::Module,
            });
        }
        sections.push(ProjectTreeSection {
            title: "Cenas".to_string(),
            items: scene_items,
        });

        sections.push(ProjectTreeSection {
            title: "Exemplos".to_string(),
            items: items_from_paths(
                "example",
                &summary.example_candidates,
                ProjectTreeItemKind::Example,
            ),
        });

        sections.push(ProjectTreeSection {
            title: "Assets".to_string(),
            items: items_from_paths(
                "asset",
                &summary.asset_candidates,
                ProjectTreeItemKind::Asset,
            ),
        });

        Self { sections }
    }

    pub fn find_item(&self, id: &str) -> Option<ProjectTreeItem> {
        for section in &self.sections {
            for item in &section.items {
                if item.id == id {
                    return Some(item.clone());
                }
            }
        }

        None
    }
}

pub fn load_project_summary(project_path: &str) -> Result<ProjectSummary, String> {
    let project_file = PathBuf::from(project_path);
    if !project_file.exists() {
        return Err(format!("arquivo de projeto nao encontrado: {project_path}"));
    }

    let raw = fs::read_to_string(&project_file)
        .map_err(|error| format!("falha ao ler arquivo de projeto: {error}"))?;
    let project_dir = project_file
        .parent()
        .ok_or_else(|| "nao foi possivel descobrir a pasta do projeto".to_string())?;

    let mut current_section = String::new();
    let mut summary = ProjectSummary {
        project_file: normalize_path(&project_file),
        project_dir: normalize_path(project_dir),
        ..ProjectSummary::default()
    };

    for raw_line in raw.lines() {
        let line = raw_line.trim();
        if line.is_empty() || line.starts_with('#') || line.starts_with(';') {
            continue;
        }

        if line.starts_with('[') && line.ends_with(']') {
            current_section = line[1..line.len() - 1].trim().to_string();
            continue;
        }

        let Some((raw_key, raw_value)) = line.split_once('=') else {
            continue;
        };

        let key = raw_key.trim();
        let value = raw_value.trim().trim_matches('"');

        match (current_section.as_str(), key) {
            ("project", "name") => summary.name = value.to_string(),
            ("project", "version") => summary.version = value.to_string(),
            ("project", "kind") => summary.kind = value.to_string(),
            ("lib", "root_namespace") => summary.root_namespace = value.to_string(),
            ("source", "root") => summary.source_root = value.to_string(),
            ("zdoc", "root") => summary.zdoc_root = value.to_string(),
            ("accessibility", "profile") => summary.accessibility_profile = value.to_string(),
            ("accessibility", "telemetry") => {
                summary.telemetry = parse_bool(value);
            }
            _ => {}
        }
    }

    if summary.name.is_empty() {
        summary.name = project_file
            .file_stem()
            .and_then(|value| value.to_str())
            .unwrap_or("unknown")
            .to_string();
    }

    if summary.kind.is_empty() {
        summary.kind = "unknown".to_string();
    }

    if summary.source_root.is_empty() {
        summary.source_root = "src".to_string();
    }

    let source_dir = project_dir.join(&summary.source_root);
    let examples_dir = project_dir.join("examples");
    let scenes_dir = project_dir.join("scenes");

    if source_dir.exists() {
        let modules = collect_files_with_extensions(&source_dir, &["zt"], 256)?;
        summary.module_count = modules.len();
        summary.has_scene_module = modules.iter().any(|path| path.ends_with("scene.zt"));
        summary.has_editor_module = modules.iter().any(|path| path.ends_with("editor.zt"));
        summary.has_assets_module = modules.iter().any(|path| path.ends_with("assets.zt"));
        let module_scene_candidates = modules
            .iter()
            .filter(|path| path.contains("scene"))
            .take(6)
            .cloned()
            .collect::<Vec<_>>();
        append_unique_paths(&mut summary.scene_candidates, module_scene_candidates, 12);
    }

    if scenes_dir.exists() {
        let scene_files = collect_files_with_extensions(&scenes_dir, &["json"], 128)?
            .into_iter()
            .map(|path| format!("scenes/{path}"))
            .collect::<Vec<_>>();
        let mut combined = scene_files;
        combined.extend(summary.scene_candidates.clone());
        summary.scene_candidates.clear();
        append_unique_paths(&mut summary.scene_candidates, combined, 12);
    }

    if examples_dir.exists() {
        let examples = collect_files_with_extensions(&examples_dir, &["zt", "ztproj"], 128)?;
        summary.example_count = examples.len();
        summary.example_candidates = examples.into_iter().take(6).collect();
    }

    let assets = collect_files_with_extensions(
        project_dir,
        &[
            "png", "jpg", "jpeg", "wav", "ogg", "mp3", "glb", "gltf", "json",
        ],
        256,
    )?;
    summary.asset_count = assets.len();
    summary.asset_candidates = assets.into_iter().take(6).collect();

    Ok(summary)
}

fn collect_files_with_extensions(
    root: &Path,
    extensions: &[&str],
    limit: usize,
) -> Result<Vec<String>, String> {
    let mut files = Vec::new();
    collect_recursive(root, root, extensions, limit, &mut files)?;
    files.sort();
    Ok(files)
}

fn collect_recursive(
    root: &Path,
    current: &Path,
    extensions: &[&str],
    limit: usize,
    files: &mut Vec<String>,
) -> Result<(), String> {
    if files.len() >= limit {
        return Ok(());
    }

    let entries = fs::read_dir(current)
        .map_err(|error| format!("falha ao ler pasta {}: {error}", normalize_path(current)))?;

    for entry in entries {
        if files.len() >= limit {
            break;
        }

        let entry = entry.map_err(|error| format!("falha ao listar pasta: {error}"))?;
        let path = entry.path();
        if path.is_dir() {
            collect_recursive(root, &path, extensions, limit, files)?;
            continue;
        }

        let Some(extension) = path.extension().and_then(|value| value.to_str()) else {
            continue;
        };

        if extensions
            .iter()
            .any(|candidate| candidate.eq_ignore_ascii_case(extension))
        {
            let relative = path
                .strip_prefix(root)
                .unwrap_or(&path)
                .to_string_lossy()
                .replace('\\', "/");
            files.push(relative);
        }
    }

    Ok(())
}

fn parse_bool(value: &str) -> Option<bool> {
    match value {
        "true" => Some(true),
        "false" => Some(false),
        _ => None,
    }
}

fn append_unique_paths(target: &mut Vec<String>, paths: Vec<String>, limit: usize) {
    for path in paths {
        if target.len() >= limit {
            break;
        }

        if !target.iter().any(|existing| existing == &path) {
            target.push(path);
        }
    }
}

fn items_from_paths(
    prefix: &str,
    paths: &[String],
    kind: ProjectTreeItemKind,
) -> Vec<ProjectTreeItem> {
    paths
        .iter()
        .map(|path| ProjectTreeItem {
            id: format!("{prefix}:{path}"),
            label: path
                .rsplit('/')
                .next()
                .filter(|value| !value.is_empty())
                .unwrap_or(path)
                .to_string(),
            path: path.clone(),
            kind: kind.clone(),
        })
        .collect()
}

fn normalize_path(path: &Path) -> String {
    path.to_string_lossy().replace('\\', "/")
}
