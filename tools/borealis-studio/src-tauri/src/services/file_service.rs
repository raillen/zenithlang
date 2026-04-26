use std::fs;
use std::path::Path;

use crate::utils::paths::normalize_path;

pub fn write_text_at_path(path: &Path, content: String) -> Result<(), String> {
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent)
            .map_err(|error| format!("failed to create {}: {error}", normalize_path(parent)))?;
    }
    fs::write(path, content)
        .map_err(|error| format!("failed to write {}: {error}", normalize_path(path)))
}

pub fn collect_files(
    root: &Path,
    current: &Path,
    visitor: &mut dyn FnMut(&Path),
) -> Result<(), String> {
    let entries = fs::read_dir(current)
        .map_err(|error| format!("failed to read {}: {error}", normalize_path(current)))?;
    for entry in entries {
        let entry = entry.map_err(|error| format!("failed to read directory entry: {error}"))?;
        let path = entry.path();
        if path.is_dir() {
            let name = path
                .file_name()
                .and_then(|name| name.to_str())
                .unwrap_or("");
            if matches!(name, "target" | "build" | ".ztc-tmp" | "native") {
                continue;
            }
            collect_files(root, &path, visitor)?;
        } else {
            visitor(&path);
        }
    }
    Ok(())
}
