use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct ConsoleLine {
    pub id: String,
    pub level: String,
    pub source: String,
    pub message: String,
}
