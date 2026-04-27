use std::collections::HashMap;
use std::fs;
use std::path::Path;
use std::time::SystemTime;

use serde::{Deserialize, Serialize};
use serde_json::Value;
use uuid::Uuid;

use crate::services::asset_pipeline::asset_kind;
use crate::services::file_service::collect_files;
use crate::utils::paths::normalize_path;

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct AssetEntry {
    pub id: String,
    pub name: String,
    pub path: String,
    pub kind: String,
    pub size_bytes: u64,
    pub modified: u64,
    #[serde(default)]
    pub metadata: Value,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct AssetIndex {
    pub version: u32,
    pub entries: Vec<AssetEntry>,
}

impl Default for AssetIndex {
    fn default() -> Self {
        Self {
            version: 1,
            entries: Vec::new(),
        }
    }
}

pub struct AssetDatabase {
    entries: HashMap<String, AssetEntry>,
    path_to_id: HashMap<String, String>,
}

impl AssetDatabase {
    pub fn new() -> Self {
        Self {
            entries: HashMap::new(),
            path_to_id: HashMap::new(),
        }
    }

    pub fn scan_project(&mut self, project_root: &Path) -> Result<Vec<AssetEntry>, String> {
        let cached = self.load_cache(project_root);
        let mut cache_by_path: HashMap<String, AssetEntry> = cached
            .into_iter()
            .map(|e| (e.path.clone(), e))
            .collect();

        self.entries.clear();
        self.path_to_id.clear();

        collect_files(project_root, project_root, &mut |file_path| {
            let Some(kind) = asset_kind(file_path) else {
                return;
            };
            let relative = normalize_path(file_path.strip_prefix(project_root).unwrap_or(file_path));
            let name = file_path
                .file_name()
                .and_then(|n| n.to_str())
                .unwrap_or("asset")
                .to_string();
            let (size_bytes, modified) = file_stats(file_path);

            let entry = if let Some(cached) = cache_by_path.remove(&relative) {
                if cached.modified == modified && cached.size_bytes == size_bytes {
                    cached
                } else {
                    AssetEntry {
                        id: cached.id,
                        name,
                        path: relative.clone(),
                        kind,
                        size_bytes,
                        modified,
                        metadata: extract_metadata(file_path, &cached.kind),
                    }
                }
            } else {
                AssetEntry {
                    id: Uuid::new_v4().to_string(),
                    name,
                    path: relative.clone(),
                    kind: kind.clone(),
                    size_bytes,
                    modified,
                    metadata: extract_metadata(file_path, &kind),
                }
            };

            self.path_to_id.insert(relative, entry.id.clone());
            self.entries.insert(entry.id.clone(), entry);
        })?;

        self.save_cache(project_root);

        let mut list: Vec<AssetEntry> = self.entries.values().cloned().collect();
        list.sort_by(|a, b| a.path.cmp(&b.path));
        Ok(list)
    }

    pub fn get_by_id(&self, id: &str) -> Option<&AssetEntry> {
        self.entries.get(id)
    }

    pub fn get_by_path(&self, path: &str) -> Option<&AssetEntry> {
        self.path_to_id
            .get(path)
            .and_then(|id| self.entries.get(id))
    }

    pub fn all_entries(&self) -> Vec<&AssetEntry> {
        let mut list: Vec<_> = self.entries.values().collect();
        list.sort_by_key(|e| &e.path);
        list
    }

    pub fn import_asset(
        &mut self,
        source_path: &Path,
        project_root: &Path,
        target_dir: &str,
    ) -> Result<AssetEntry, String> {
        let file_name = source_path
            .file_name()
            .and_then(|n| n.to_str())
            .ok_or("invalid source filename")?;
        let target = project_root.join(target_dir).join(file_name);
        if let Some(parent) = target.parent() {
            fs::create_dir_all(parent).map_err(|e| format!("failed to create dir: {e}"))?;
        }
        fs::copy(source_path, &target).map_err(|e| format!("failed to copy asset: {e}"))?;

        let kind = asset_kind(&target).unwrap_or_else(|| "unknown".to_string());
        let relative = normalize_path(target.strip_prefix(project_root).unwrap_or(&target));
        let (size_bytes, modified) = file_stats(&target);

        let entry = AssetEntry {
            id: Uuid::new_v4().to_string(),
            name: file_name.to_string(),
            path: relative.clone(),
            kind: kind.clone(),
            size_bytes,
            modified,
            metadata: extract_metadata(&target, &kind),
        };

        self.path_to_id.insert(relative, entry.id.clone());
        self.entries.insert(entry.id.clone(), entry.clone());
        self.save_cache(project_root);
        Ok(entry)
    }

    pub fn remove_asset(&mut self, id: &str) -> Option<AssetEntry> {
        let entry = self.entries.remove(id)?;
        self.path_to_id.remove(&entry.path);
        Some(entry)
    }

    fn cache_path(project_root: &Path) -> std::path::PathBuf {
        project_root.join(".borealis/asset_index.json")
    }

    fn load_cache(&self, project_root: &Path) -> Vec<AssetEntry> {
        let path = Self::cache_path(project_root);
        fs::read_to_string(path)
            .ok()
            .and_then(|raw| serde_json::from_str::<AssetIndex>(&raw).ok())
            .map(|index| index.entries)
            .unwrap_or_default()
    }

    fn save_cache(&self, project_root: &Path) {
        let path = Self::cache_path(project_root);
        if let Some(parent) = path.parent() {
            let _ = fs::create_dir_all(parent);
        }
        let index = AssetIndex {
            version: 1,
            entries: self.entries.values().cloned().collect(),
        };
        let _ = fs::write(path, serde_json::to_string_pretty(&index).unwrap_or_default());
    }
}

impl Default for AssetDatabase {
    fn default() -> Self {
        Self::new()
    }
}

fn file_stats(path: &Path) -> (u64, u64) {
    let meta = fs::metadata(path).ok();
    let size = meta.as_ref().map(|m| m.len()).unwrap_or(0);
    let modified = meta
        .and_then(|m| m.modified().ok())
        .and_then(|t| t.duration_since(SystemTime::UNIX_EPOCH).ok())
        .map(|d| d.as_secs())
        .unwrap_or(0);
    (size, modified)
}

fn extract_metadata(path: &Path, kind: &str) -> Value {
    match kind {
        "script" => {
            let line_count = fs::read_to_string(path)
                .map(|c| c.lines().count())
                .unwrap_or(0);
            serde_json::json!({ "lineCount": line_count })
        }
        "model" => {
            let ext = path
                .extension()
                .and_then(|e| e.to_str())
                .unwrap_or("")
                .to_lowercase();
            serde_json::json!({ "format": ext })
        }
        "texture" => {
            let ext = path
                .extension()
                .and_then(|e| e.to_str())
                .unwrap_or("")
                .to_lowercase();
            serde_json::json!({ "format": ext })
        }
        "audio" => {
            let ext = path
                .extension()
                .and_then(|e| e.to_str())
                .unwrap_or("")
                .to_lowercase();
            serde_json::json!({ "format": ext })
        }
        _ => Value::Object(serde_json::Map::new()),
    }
}
