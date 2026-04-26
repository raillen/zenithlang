use std::path::{Path, PathBuf};

pub fn normalize_path(path: &Path) -> String {
    path.to_string_lossy().replace('\\', "/")
}

pub fn clean_project_name(name: &str) -> String {
    let trimmed = name.trim();
    if trimmed.is_empty() {
        "Borealis Project".to_string()
    } else {
        trimmed.to_string()
    }
}

pub fn slugify_project_name(name: &str) -> String {
    let mut out = String::new();
    let mut last_dash = false;

    for ch in name.chars() {
        if ch.is_ascii_alphanumeric() {
            out.push(ch.to_ascii_lowercase());
            last_dash = false;
        } else if !last_dash {
            out.push('-');
            last_dash = true;
        }
    }

    let slug = out.trim_matches('-').to_string();
    if slug.is_empty() {
        "borealis-project".to_string()
    } else {
        slug
    }
}

pub fn normalize_project_file(path: PathBuf) -> Result<PathBuf, String> {
    let path = if path.is_dir() {
        path.join("zenith.ztproj")
    } else {
        path
    };

    if !path.exists() {
        return Err(format!(
            "project file not found: {}",
            normalize_path(&path)
        ));
    }

    path.canonicalize().map_err(|error| {
        format!(
            "failed to resolve project path {}: {error}",
            normalize_path(&path)
        )
    })
}

pub fn resolve_project_path(project_root: &Path, path: &str) -> PathBuf {
    let raw = PathBuf::from(path);
    if raw.is_absolute() {
        raw
    } else {
        project_root.join(raw)
    }
}
