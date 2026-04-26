use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct ProjectAsset {
    pub id: String,
    pub name: String,
    pub path: String,
    pub kind: String,
}
