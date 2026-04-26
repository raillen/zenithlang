use std::fs;
use std::path::PathBuf;

use serde_json::{json, Value};

use crate::models::project::{DocumentationLink, StudioLayout};
use crate::utils::paths::normalize_path;

pub use crate::services::studio_layout::*;
pub use crate::services::templates::*;

pub fn read_project_name(project_path: &std::path::Path) -> Option<String> {
    let raw = fs::read_to_string(project_path).ok()?;
    for line in raw.lines() {
        let trimmed = line.trim();
        if let Some((key, value)) = trimmed.split_once('=') {
            if key.trim() == "name" {
                return Some(value.trim().trim_matches('"').to_string());
            }
        }
    }
    None
}

pub fn project_manifest(slug: &str) -> String {
    format!(
        "[project]\nname = \"{}\"\nversion = \"0.1.0\"\nkind = \"app\"\n\n[source]\nroot = \"src\"\n\n[app]\nentry = \"app.main\"\n\n[build]\ntarget = \"native\"\nprofile = \"debug\"\n",
        slug
    )
}

pub fn documentation_links(layout: &StudioLayout) -> Vec<DocumentationLink> {
    let docs_root = layout.repo_root.as_ref().unwrap_or(&layout.app_root);
    vec![
        DocumentationLink {
            title: "Studio README".to_string(),
            path: normalize_path(&docs_root.join("tools/borealis-studio/README.md")),
            summary: "Como rodar o editor Tauri + React e a ponte de preview.".to_string(),
        },
        DocumentationLink {
            title: "Stack Decision".to_string(),
            path: normalize_path(
                &docs_root
                    .join("packages/borealis/decisions/015-borealis-studio-tauri-react-stack.md"),
            ),
            summary: "Stack atual aceita e limites de responsabilidade.".to_string(),
        },
        DocumentationLink {
            title: "Preview IPC".to_string(),
            path: normalize_path(
                &docs_root.join("packages/borealis/decisions/014-borealis-editor-preview-ipc.md"),
            ),
            summary: "Protocolo JSONL usado pelo Play mode.".to_string(),
        },
    ]
}

pub fn load_editor_manifest(layout: &StudioLayout) -> Value {
    for candidate in editor_manifest_candidates(layout) {
        if !candidate.exists() {
            continue;
        }
        match fs::read_to_string(&candidate)
            .ok()
            .and_then(|raw| serde_json::from_str::<Value>(&raw).ok())
        {
            Some(Value::Object(manifest)) => {
                let mut manifest = manifest;
                manifest.insert(
                    "source".to_string(),
                    Value::String(normalize_path(&candidate)),
                );
                return Value::Object(manifest);
            }
            _ => continue,
        }
    }

    json!({
        "version": 1,
        "source": "fallback:frontend-catalog",
        "components": {},
        "sceneSettings": {},
        "assetKinds": {},
        "actions": {},
        "templates": {}
    })
}

fn editor_manifest_candidates(layout: &StudioLayout) -> Vec<PathBuf> {
    let mut candidates = Vec::new();
    if let Some(sdk_root) = &layout.sdk_root {
        candidates.push(sdk_root.join("borealis.editor.json"));
        candidates.push(sdk_root.join("editor/borealis.editor.json"));
    }
    if let Some(repo_root) = &layout.repo_root {
        candidates.push(repo_root.join("packages/borealis/borealis.editor.json"));
    }
    candidates.push(layout.app_root.join("runtime/sdk/borealis.editor.json"));
    candidates.push(layout.app_root.join("sdk/borealis.editor.json"));
    candidates
}
