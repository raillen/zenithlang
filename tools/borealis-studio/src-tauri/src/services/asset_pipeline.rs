use std::fs;
use std::path::Path;

use crate::models::asset::ProjectAsset;
use crate::models::script::ScriptDocument;
use crate::services::file_service::collect_files;
use crate::utils::paths::normalize_path;

pub fn collect_assets(root: &Path, current: &Path) -> Result<Vec<ProjectAsset>, String> {
    let mut assets = Vec::new();
    collect_files(root, current, &mut |path| {
        if let Some(kind) = asset_kind(path) {
            assets.push(ProjectAsset {
                id: normalize_path(path),
                name: path
                    .file_name()
                    .and_then(|name| name.to_str())
                    .unwrap_or("asset")
                    .to_string(),
                path: normalize_path(path.strip_prefix(root).unwrap_or(path)),
                kind,
            });
        }
    })?;
    assets.sort_by(|left, right| left.path.cmp(&right.path));
    assets.truncate(160);
    Ok(assets)
}

pub fn collect_scripts(root: &Path, current: &Path) -> Result<Vec<ScriptDocument>, String> {
    let mut scripts = Vec::new();
    collect_files(root, current, &mut |path| {
        if path.extension().and_then(|value| value.to_str()) == Some("zt") {
            let content = fs::read_to_string(path).unwrap_or_default();
            scripts.push(ScriptDocument {
                path: normalize_path(path.strip_prefix(root).unwrap_or(path)),
                name: path
                    .file_name()
                    .and_then(|name| name.to_str())
                    .unwrap_or("script.zt")
                    .to_string(),
                content,
                dirty: Some(false),
            });
        }
    })?;
    scripts.sort_by(|left, right| left.path.cmp(&right.path));
    scripts.truncate(40);
    Ok(scripts)
}

pub fn asset_kind(path: &Path) -> Option<String> {
    match path
        .extension()
        .and_then(|value| value.to_str())
        .unwrap_or("")
        .to_ascii_lowercase()
        .as_str()
    {
        "scene" | "json" if path.to_string_lossy().contains(".scene.") => {
            Some("scene".to_string())
        }
        "zt" => Some("script".to_string()),
        "obj" | "glb" | "gltf" | "fbx" | "iqm" | "m3d" => Some("model".to_string()),
        "png" | "jpg" | "jpeg" | "webp" => Some("texture".to_string()),
        "wav" | "ogg" | "mp3" => Some("audio".to_string()),
        "glsl" | "vs" | "fs" => Some("shader".to_string()),
        "hdr" if path.to_string_lossy().contains("sky") => Some("cubemap".to_string()),
        _ => None,
    }
}
