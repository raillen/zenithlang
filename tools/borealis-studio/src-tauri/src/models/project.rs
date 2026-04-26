use serde::{Deserialize, Serialize};
use serde_json::Value;
use std::path::PathBuf;

use super::asset::ProjectAsset;
use super::console::ConsoleLine;
use super::scene::SceneDocument;
use super::script::ScriptDocument;

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct StudioSnapshot {
    pub project_name: String,
    pub project_path: String,
    pub project_root: String,
    pub scene: SceneDocument,
    pub assets: Vec<ProjectAsset>,
    pub scripts: Vec<ScriptDocument>,
    pub console: Vec<ConsoleLine>,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct StudioHome {
    pub workspace_root: String,
    pub app_root: String,
    pub repo_root: Option<String>,
    pub sdk_root: Option<String>,
    pub runtime_mode: String,
    pub runtime_status: String,
    pub default_project_path: String,
    pub default_projects_dir: String,
    pub editor_manifest: Value,
    pub templates: Vec<ProjectTemplate>,
    pub docs: Vec<DocumentationLink>,
}

#[derive(Debug, Clone)]
pub struct StudioLayout {
    pub app_root: PathBuf,
    pub base_root: PathBuf,
    pub repo_root: Option<PathBuf>,
    pub sdk_root: Option<PathBuf>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct ProjectTemplate {
    pub id: String,
    pub name: String,
    pub summary: String,
    pub default_name: String,
    pub tags: Vec<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub scene: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub main_script: Option<String>,
    #[serde(default)]
    pub assets: Vec<ProjectTemplateAsset>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub source: Option<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct ProjectTemplateAsset {
    pub source: String,
    pub target: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct DocumentationLink {
    pub title: String,
    pub path: String,
    pub summary: String,
}

#[derive(Debug, Clone, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct NewProjectRequest {
    pub project_name: String,
    pub parent_dir: String,
    pub template_id: String,
}
