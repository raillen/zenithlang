use std::path::{Path, PathBuf};

use crate::models::project::StudioLayout;
use crate::utils::paths::normalize_path;

pub fn studio_layout() -> StudioLayout {
    let app_root = studio_app_root();
    let repo_root = find_repo_root(&app_root);
    let sdk_root = find_sdk_root(&app_root);
    let base_root = repo_root.clone().unwrap_or_else(|| app_root.clone());

    StudioLayout {
        app_root,
        base_root,
        repo_root,
        sdk_root,
    }
}

pub fn studio_app_root() -> PathBuf {
    let manifest_dir = Path::new(env!("CARGO_MANIFEST_DIR"));
    manifest_dir
        .parent()
        .map(Path::to_path_buf)
        .unwrap_or_else(|| manifest_dir.to_path_buf())
        .canonicalize()
        .unwrap_or_else(|_| manifest_dir.parent().unwrap_or(manifest_dir).to_path_buf())
}

pub fn find_repo_root(start: &Path) -> Option<PathBuf> {
    for candidate in start.ancestors() {
        if candidate.join("packages/borealis/zenith.ztproj").exists() {
            return Some(
                candidate
                    .canonicalize()
                    .unwrap_or_else(|_| candidate.to_path_buf()),
            );
        }
    }
    None
}

pub fn find_sdk_root(app_root: &Path) -> Option<PathBuf> {
    if let Ok(value) = std::env::var("BOREALIS_SDK_ROOT") {
        let path = PathBuf::from(value);
        if is_sdk_root(&path) {
            return Some(path.canonicalize().unwrap_or(path));
        }
    }

    for candidate in [
        app_root.join("runtime/sdk"),
        app_root.join("sdk"),
        app_root.join("../runtime/sdk"),
        app_root.join("../sdk"),
    ] {
        if is_sdk_root(&candidate) {
            return Some(candidate.canonicalize().unwrap_or(candidate));
        }
    }

    None
}

pub fn is_sdk_root(path: &Path) -> bool {
    sdk_zt_path(path).is_some()
        || sdk_preview_project(path).is_some()
        || path.join("stdlib").exists()
        || path.join("packages/borealis/zenith.ztproj").exists()
}

pub fn sdk_zt_path(sdk_root: &Path) -> Option<PathBuf> {
    for candidate in [
        sdk_root
            .join("bin")
            .join(if cfg!(windows) { "zt.exe" } else { "zt" }),
        sdk_root.join(if cfg!(windows) { "zt.exe" } else { "zt" }),
    ] {
        if candidate.exists() {
            return Some(candidate);
        }
    }
    None
}

pub fn sdk_preview_project(sdk_root: &Path) -> Option<PathBuf> {
    let candidate = sdk_root.join("preview/zenith.ztproj");
    if candidate.exists() {
        Some(candidate)
    } else {
        None
    }
}

pub fn resolve_studio_path(path: &str) -> PathBuf {
    let raw = PathBuf::from(path);
    if raw.is_absolute() {
        raw
    } else {
        studio_layout().base_root.join(path)
    }
}

pub fn default_project_path(layout: &StudioLayout) -> PathBuf {
    if let Some(repo_root) = &layout.repo_root {
        return repo_root.join("packages/borealis/zenith.ztproj");
    }
    layout.base_root.join("zenith.ztproj")
}

pub fn default_projects_dir(workspace: &Path) -> PathBuf {
    if let Ok(user_profile) = std::env::var("USERPROFILE") {
        return Path::new(&user_profile)
            .join("Documents")
            .join("Projetos")
            .join("Borealis Projects");
    }
    workspace.join("Borealis Projects")
}

pub fn runtime_status(layout: &StudioLayout) -> (String, String) {
    if let Some(sdk_root) = &layout.sdk_root {
        if sdk_zt_path(sdk_root).is_some() {
            return (
                "sdk".to_string(),
                format!("SDK local pronto: {}", normalize_path(sdk_root)),
            );
        }
        return (
            "sdk".to_string(),
            format!(
                "SDK encontrado sem zt runner: {}",
                normalize_path(sdk_root)
            ),
        );
    }

    if let Some(repo_root) = &layout.repo_root {
        return (
            "repo-dev".to_string(),
            format!("Modo dev pelo repositorio: {}", normalize_path(repo_root)),
        );
    }

    (
        "missing".to_string(),
        "Runtime ausente. Configure BOREALIS_SDK_ROOT ou bundle runtime/sdk.".to_string(),
    )
}
