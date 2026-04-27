use serde::{Deserialize, Serialize};
use serde_json::Value;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum ComponentCategory {
    Rendering,
    Physics,
    Audio,
    Camera,
    Light,
    Script,
    #[serde(rename = "ai")]
    AI,
    #[serde(rename = "ui")]
    UI,
    Custom,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct ComponentSchema {
    pub kind: String,
    pub label: String,
    pub description: String,
    pub category: ComponentCategory,
    pub fields: Vec<ComponentFieldSchema>,
    pub icon: String,
    #[serde(default)]
    pub singleton: bool,
    #[serde(default)]
    pub requires: Vec<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct ComponentFieldSchema {
    pub key: String,
    pub label: String,
    pub field_type: ComponentFieldType,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub default_value: Option<Value>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub min: Option<f64>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub max: Option<f64>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub options: Option<Vec<String>>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub asset_kind: Option<String>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum ComponentFieldType {
    Float,
    Int,
    Bool,
    Text,
    Color,
    Vector2,
    Vector3,
    Select,
    Asset,
}
