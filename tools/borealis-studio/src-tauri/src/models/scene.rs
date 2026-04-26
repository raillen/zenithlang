use serde::{Deserialize, Serialize};
use serde_json::Value;

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct SceneDocument {
    pub name: String,
    pub path: String,
    pub document_id: String,
    pub environment: Value,
    pub render: Value,
    pub audio: Value,
    pub entities: Vec<SceneEntity>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct SceneEntity {
    pub id: String,
    pub name: String,
    pub layer: String,
    pub parent: Option<String>,
    pub tags: Vec<String>,
    pub components: Vec<SceneComponent>,
    pub transform: Transform3d,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct SceneComponent {
    pub kind: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub asset: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub script: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub profile: Option<String>,
    #[serde(default)]
    pub properties: Value,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Transform3d {
    pub x: f64,
    pub y: f64,
    pub z: f64,
    pub rotation_x: f64,
    pub rotation_y: f64,
    pub rotation_z: f64,
    pub scale_x: f64,
    pub scale_y: f64,
    pub scale_z: f64,
}

impl Default for Transform3d {
    fn default() -> Self {
        Self {
            x: 0.0,
            y: 0.0,
            z: 0.0,
            rotation_x: 0.0,
            rotation_y: 0.0,
            rotation_z: 0.0,
            scale_x: 1.0,
            scale_y: 1.0,
            scale_z: 1.0,
        }
    }
}
