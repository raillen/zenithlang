use std::collections::HashMap;

use serde_json::Value;

use crate::models::component::{
    ComponentCategory, ComponentFieldSchema, ComponentFieldType, ComponentSchema,
};
use crate::models::scene::SceneComponent;

pub struct ComponentRegistry {
    schemas: HashMap<String, ComponentSchema>,
}

impl ComponentRegistry {
    pub fn new() -> Self {
        let mut registry = Self {
            schemas: HashMap::new(),
        };
        registry.register_builtins();
        registry
    }

    pub fn get_schema(&self, kind: &str) -> Option<&ComponentSchema> {
        self.schemas.get(kind)
    }

    pub fn all_schemas(&self) -> Vec<&ComponentSchema> {
        let mut list: Vec<_> = self.schemas.values().collect();
        list.sort_by_key(|s| (&s.category as *const _ as usize, s.label.clone()));
        list
    }

    pub fn schemas_by_category(&self) -> HashMap<ComponentCategory, Vec<&ComponentSchema>> {
        let mut map: HashMap<ComponentCategory, Vec<&ComponentSchema>> = HashMap::new();
        for schema in self.schemas.values() {
            map.entry(schema.category).or_default().push(schema);
        }
        for schemas in map.values_mut() {
            schemas.sort_by_key(|s| s.label.clone());
        }
        map
    }

    pub fn create_default_component(&self, kind: &str) -> Result<SceneComponent, String> {
        let schema = self
            .schemas
            .get(kind)
            .ok_or_else(|| format!("unknown component kind: {kind}"))?;
        let mut properties = serde_json::Map::new();
        for field in &schema.fields {
            if let Some(default) = &field.default_value {
                properties.insert(field.key.clone(), default.clone());
            }
        }
        Ok(SceneComponent {
            kind: kind.to_string(),
            asset: None,
            script: None,
            profile: None,
            properties: Value::Object(properties),
        })
    }

    pub fn validate_add(
        &self,
        kind: &str,
        existing_components: &[SceneComponent],
    ) -> Result<(), String> {
        let schema = self
            .schemas
            .get(kind)
            .ok_or_else(|| format!("unknown component kind: {kind}"))?;
        if schema.singleton && existing_components.iter().any(|c| c.kind == kind) {
            return Err(format!(
                "entity already has a {} component (singleton)",
                schema.label
            ));
        }
        Ok(())
    }

    pub fn missing_requirements(
        &self,
        kind: &str,
        existing_components: &[SceneComponent],
    ) -> Vec<String> {
        let Some(schema) = self.schemas.get(kind) else {
            return Vec::new();
        };
        schema
            .requires
            .iter()
            .filter(|req| !existing_components.iter().any(|c| &c.kind == *req))
            .cloned()
            .collect()
    }

    pub fn dependents_of(&self, kind: &str, existing_components: &[SceneComponent]) -> Vec<String> {
        existing_components
            .iter()
            .filter_map(|c| {
                let schema = self.schemas.get(&c.kind)?;
                if schema.requires.contains(&kind.to_string()) {
                    Some(c.kind.clone())
                } else {
                    None
                }
            })
            .collect()
    }

    pub fn load_manifest_overrides(&mut self, manifest: &Value) {
        let Some(components) = manifest.get("components").and_then(Value::as_object) else {
            return;
        };
        for (kind, schema_value) in components {
            if let Ok(schema) = serde_json::from_value::<ComponentSchema>(schema_value.clone()) {
                self.schemas.insert(kind.clone(), schema);
            }
        }
    }

    fn register(&mut self, schema: ComponentSchema) {
        self.schemas.insert(schema.kind.clone(), schema);
    }

    fn register_builtins(&mut self) {
        self.register(ComponentSchema {
            kind: "camera3d".to_string(),
            label: "Camera 3D".to_string(),
            description: "Perspective or orthographic 3D camera.".to_string(),
            category: ComponentCategory::Camera,
            icon: "camera".to_string(),
            singleton: true,
            requires: Vec::new(),
            fields: vec![ComponentFieldSchema {
                key: "projection".to_string(),
                label: "Projection".to_string(),
                field_type: ComponentFieldType::Select,
                default_value: Some(Value::String("perspective".to_string())),
                min: None,
                max: None,
                options: Some(vec!["perspective".to_string(), "orthographic".to_string()]),
                asset_kind: None,
            }],
        });

        self.register(ComponentSchema {
            kind: "camera2d".to_string(),
            label: "Camera 2D".to_string(),
            description: "Orthographic 2D camera.".to_string(),
            category: ComponentCategory::Camera,
            icon: "camera".to_string(),
            singleton: true,
            requires: Vec::new(),
            fields: vec![ComponentFieldSchema {
                key: "projection".to_string(),
                label: "Projection".to_string(),
                field_type: ComponentFieldType::Select,
                default_value: Some(Value::String("orthographic".to_string())),
                min: None,
                max: None,
                options: Some(vec!["orthographic".to_string()]),
                asset_kind: None,
            }],
        });

        self.register(ComponentSchema {
            kind: "cube3d".to_string(),
            label: "Cube".to_string(),
            description: "Primitive 3D cube mesh.".to_string(),
            category: ComponentCategory::Rendering,
            icon: "box".to_string(),
            singleton: false,
            requires: Vec::new(),
            fields: vec![float_field("size", "Size", 48.0, Some(0.1), Some(10000.0))],
        });

        self.register(ComponentSchema {
            kind: "sphere3d".to_string(),
            label: "Sphere".to_string(),
            description: "Primitive 3D sphere mesh.".to_string(),
            category: ComponentCategory::Rendering,
            icon: "circle".to_string(),
            singleton: false,
            requires: Vec::new(),
            fields: vec![float_field("radius", "Radius", 24.0, Some(0.1), Some(5000.0))],
        });

        self.register(ComponentSchema {
            kind: "plane3d".to_string(),
            label: "Plane".to_string(),
            description: "Flat 3D plane mesh.".to_string(),
            category: ComponentCategory::Rendering,
            icon: "square".to_string(),
            singleton: false,
            requires: Vec::new(),
            fields: vec![
                float_field("width", "Width", 100.0, Some(0.1), Some(10000.0)),
                float_field("height", "Height", 100.0, Some(0.1), Some(10000.0)),
            ],
        });

        self.register(ComponentSchema {
            kind: "model3d".to_string(),
            label: "3D Model".to_string(),
            description: "Imported 3D model asset (.glb, .obj, .fbx).".to_string(),
            category: ComponentCategory::Rendering,
            icon: "package".to_string(),
            singleton: false,
            requires: Vec::new(),
            fields: vec![asset_field("asset", "Model", "model")],
        });

        self.register(ComponentSchema {
            kind: "sprite".to_string(),
            label: "Sprite".to_string(),
            description: "2D sprite image.".to_string(),
            category: ComponentCategory::Rendering,
            icon: "image".to_string(),
            singleton: false,
            requires: Vec::new(),
            fields: vec![asset_field("asset", "Texture", "texture")],
        });

        self.register(ComponentSchema {
            kind: "directional_light".to_string(),
            label: "Directional Light".to_string(),
            description: "Infinite-distance light like the sun.".to_string(),
            category: ComponentCategory::Light,
            icon: "sun".to_string(),
            singleton: false,
            requires: Vec::new(),
            fields: vec![
                color_field("color", "Color", "#ffffff"),
                float_field("intensity", "Intensity", 1.0, Some(0.0), Some(10.0)),
            ],
        });

        self.register(ComponentSchema {
            kind: "point_light".to_string(),
            label: "Point Light".to_string(),
            description: "Omnidirectional point light.".to_string(),
            category: ComponentCategory::Light,
            icon: "lightbulb".to_string(),
            singleton: false,
            requires: Vec::new(),
            fields: vec![
                color_field("color", "Color", "#ffffff"),
                float_field("intensity", "Intensity", 1.0, Some(0.0), Some(10.0)),
                float_field("range", "Range", 200.0, Some(0.0), Some(5000.0)),
            ],
        });

        self.register(ComponentSchema {
            kind: "spot_light".to_string(),
            label: "Spot Light".to_string(),
            description: "Conical spot light.".to_string(),
            category: ComponentCategory::Light,
            icon: "flashlight".to_string(),
            singleton: false,
            requires: Vec::new(),
            fields: vec![
                color_field("color", "Color", "#ffffff"),
                float_field("intensity", "Intensity", 1.0, Some(0.0), Some(10.0)),
                float_field("range", "Range", 200.0, Some(0.0), Some(5000.0)),
                float_field("angle", "Cone Angle", 45.0, Some(1.0), Some(180.0)),
            ],
        });

        self.register(ComponentSchema {
            kind: "audio3d".to_string(),
            label: "Audio Source".to_string(),
            description: "3D positional audio emitter.".to_string(),
            category: ComponentCategory::Audio,
            icon: "volume-2".to_string(),
            singleton: false,
            requires: Vec::new(),
            fields: vec![
                asset_field("asset", "Audio Clip", "audio"),
                float_field("volume", "Volume", 1.0, Some(0.0), Some(2.0)),
                float_field("range", "Range", 300.0, Some(0.0), Some(5000.0)),
                bool_field("loop", "Loop", false),
            ],
        });

        self.register(ComponentSchema {
            kind: "script".to_string(),
            label: "Script".to_string(),
            description: "Zenith script attached to this entity.".to_string(),
            category: ComponentCategory::Script,
            icon: "file-code".to_string(),
            singleton: false,
            requires: Vec::new(),
            fields: vec![asset_field("script", "Script", "script")],
        });

        self.register(ComponentSchema {
            kind: "rigidbody3d".to_string(),
            label: "Rigidbody 3D".to_string(),
            description: "Physics body for 3D simulation.".to_string(),
            category: ComponentCategory::Physics,
            icon: "globe".to_string(),
            singleton: true,
            requires: Vec::new(),
            fields: vec![
                float_field("mass", "Mass", 1.0, Some(0.0), Some(10000.0)),
                float_field("drag", "Drag", 0.0, Some(0.0), Some(100.0)),
                bool_field("useGravity", "Use Gravity", true),
            ],
        });

        self.register(ComponentSchema {
            kind: "box_collider".to_string(),
            label: "Box Collider".to_string(),
            description: "Axis-aligned box collision shape.".to_string(),
            category: ComponentCategory::Physics,
            icon: "square".to_string(),
            singleton: false,
            requires: vec!["rigidbody3d".to_string()],
            fields: vec![
                float_field("sizeX", "Size X", 1.0, Some(0.01), Some(10000.0)),
                float_field("sizeY", "Size Y", 1.0, Some(0.01), Some(10000.0)),
                float_field("sizeZ", "Size Z", 1.0, Some(0.01), Some(10000.0)),
                bool_field("isTrigger", "Is Trigger", false),
            ],
        });

        self.register(ComponentSchema {
            kind: "sphere_collider".to_string(),
            label: "Sphere Collider".to_string(),
            description: "Spherical collision shape.".to_string(),
            category: ComponentCategory::Physics,
            icon: "circle".to_string(),
            singleton: false,
            requires: vec!["rigidbody3d".to_string()],
            fields: vec![
                float_field("radius", "Radius", 0.5, Some(0.01), Some(5000.0)),
                bool_field("isTrigger", "Is Trigger", false),
            ],
        });
    }
}

impl Default for ComponentRegistry {
    fn default() -> Self {
        Self::new()
    }
}

fn float_field(key: &str, label: &str, default: f64, min: Option<f64>, max: Option<f64>) -> ComponentFieldSchema {
    ComponentFieldSchema {
        key: key.to_string(),
        label: label.to_string(),
        field_type: ComponentFieldType::Float,
        default_value: Some(Value::from(default)),
        min,
        max,
        options: None,
        asset_kind: None,
    }
}

fn bool_field(key: &str, label: &str, default: bool) -> ComponentFieldSchema {
    ComponentFieldSchema {
        key: key.to_string(),
        label: label.to_string(),
        field_type: ComponentFieldType::Bool,
        default_value: Some(Value::Bool(default)),
        min: None,
        max: None,
        options: None,
        asset_kind: None,
    }
}

fn color_field(key: &str, label: &str, default: &str) -> ComponentFieldSchema {
    ComponentFieldSchema {
        key: key.to_string(),
        label: label.to_string(),
        field_type: ComponentFieldType::Color,
        default_value: Some(Value::String(default.to_string())),
        min: None,
        max: None,
        options: None,
        asset_kind: None,
    }
}

fn asset_field(key: &str, label: &str, asset_kind: &str) -> ComponentFieldSchema {
    ComponentFieldSchema {
        key: key.to_string(),
        label: label.to_string(),
        field_type: ComponentFieldType::Asset,
        default_value: None,
        min: None,
        max: None,
        options: None,
        asset_kind: Some(asset_kind.to_string()),
    }
}
