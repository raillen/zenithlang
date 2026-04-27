use serde_json::Value;

use crate::models::history::EditCommand;
use crate::models::scene::{SceneComponent, SceneDocument, SceneEntity, Transform3d};

pub struct AddEntityCommand {
    pub entity: SceneEntity,
}

impl EditCommand for AddEntityCommand {
    fn execute(&self, scene: &mut SceneDocument) -> Result<(), String> {
        if scene.entities.iter().any(|e| e.id == self.entity.id) {
            return Err(format!("entity {} already exists", self.entity.id));
        }
        scene.entities.push(self.entity.clone());
        Ok(())
    }
    fn undo(&self, scene: &mut SceneDocument) -> Result<(), String> {
        scene.entities.retain(|e| e.id != self.entity.id);
        Ok(())
    }
    fn description(&self) -> String {
        format!("Add {}", self.entity.name)
    }
}

pub struct RemoveEntityCommand {
    pub entity: SceneEntity,
    pub index: usize,
}

impl EditCommand for RemoveEntityCommand {
    fn execute(&self, scene: &mut SceneDocument) -> Result<(), String> {
        scene.entities.retain(|e| e.id != self.entity.id);
        Ok(())
    }
    fn undo(&self, scene: &mut SceneDocument) -> Result<(), String> {
        let insert_at = self.index.min(scene.entities.len());
        scene.entities.insert(insert_at, self.entity.clone());
        Ok(())
    }
    fn description(&self) -> String {
        format!("Remove {}", self.entity.name)
    }
}

pub struct UpdateEntityCommand {
    pub entity_id: String,
    pub old_name: Option<String>,
    pub new_name: Option<String>,
    pub old_layer: Option<String>,
    pub new_layer: Option<String>,
    pub old_tags: Option<Vec<String>>,
    pub new_tags: Option<Vec<String>>,
}

impl EditCommand for UpdateEntityCommand {
    fn execute(&self, scene: &mut SceneDocument) -> Result<(), String> {
        let entity = scene
            .entities
            .iter_mut()
            .find(|e| e.id == self.entity_id)
            .ok_or_else(|| format!("entity not found: {}", self.entity_id))?;
        if let Some(name) = &self.new_name {
            entity.name = name.clone();
        }
        if let Some(layer) = &self.new_layer {
            entity.layer = layer.clone();
        }
        if let Some(tags) = &self.new_tags {
            entity.tags = tags.clone();
        }
        Ok(())
    }
    fn undo(&self, scene: &mut SceneDocument) -> Result<(), String> {
        let entity = scene
            .entities
            .iter_mut()
            .find(|e| e.id == self.entity_id)
            .ok_or_else(|| format!("entity not found: {}", self.entity_id))?;
        if let Some(name) = &self.old_name {
            entity.name = name.clone();
        }
        if let Some(layer) = &self.old_layer {
            entity.layer = layer.clone();
        }
        if let Some(tags) = &self.old_tags {
            entity.tags = tags.clone();
        }
        Ok(())
    }
    fn description(&self) -> String {
        format!("Update entity {}", self.entity_id)
    }
}

pub struct UpdateTransformCommand {
    pub entity_id: String,
    pub old_transform: Transform3d,
    pub new_transform: Transform3d,
}

impl EditCommand for UpdateTransformCommand {
    fn execute(&self, scene: &mut SceneDocument) -> Result<(), String> {
        let entity = scene
            .entities
            .iter_mut()
            .find(|e| e.id == self.entity_id)
            .ok_or_else(|| format!("entity not found: {}", self.entity_id))?;
        entity.transform = self.new_transform.clone();
        Ok(())
    }
    fn undo(&self, scene: &mut SceneDocument) -> Result<(), String> {
        let entity = scene
            .entities
            .iter_mut()
            .find(|e| e.id == self.entity_id)
            .ok_or_else(|| format!("entity not found: {}", self.entity_id))?;
        entity.transform = self.old_transform.clone();
        Ok(())
    }
    fn description(&self) -> String {
        format!("Move {}", self.entity_id)
    }
}

pub struct AddComponentCommand {
    pub entity_id: String,
    pub component: SceneComponent,
}

impl EditCommand for AddComponentCommand {
    fn execute(&self, scene: &mut SceneDocument) -> Result<(), String> {
        let entity = scene
            .entities
            .iter_mut()
            .find(|e| e.id == self.entity_id)
            .ok_or_else(|| format!("entity not found: {}", self.entity_id))?;
        entity.components.push(self.component.clone());
        Ok(())
    }
    fn undo(&self, scene: &mut SceneDocument) -> Result<(), String> {
        let entity = scene
            .entities
            .iter_mut()
            .find(|e| e.id == self.entity_id)
            .ok_or_else(|| format!("entity not found: {}", self.entity_id))?;
        if let Some(pos) = entity
            .components
            .iter()
            .rposition(|c| c.kind == self.component.kind)
        {
            entity.components.remove(pos);
        }
        Ok(())
    }
    fn description(&self) -> String {
        format!("Add {} to {}", self.component.kind, self.entity_id)
    }
}

pub struct RemoveComponentCommand {
    pub entity_id: String,
    pub component: SceneComponent,
    pub index: usize,
}

impl EditCommand for RemoveComponentCommand {
    fn execute(&self, scene: &mut SceneDocument) -> Result<(), String> {
        let entity = scene
            .entities
            .iter_mut()
            .find(|e| e.id == self.entity_id)
            .ok_or_else(|| format!("entity not found: {}", self.entity_id))?;
        if self.index < entity.components.len() {
            entity.components.remove(self.index);
        }
        Ok(())
    }
    fn undo(&self, scene: &mut SceneDocument) -> Result<(), String> {
        let entity = scene
            .entities
            .iter_mut()
            .find(|e| e.id == self.entity_id)
            .ok_or_else(|| format!("entity not found: {}", self.entity_id))?;
        let insert_at = self.index.min(entity.components.len());
        entity.components.insert(insert_at, self.component.clone());
        Ok(())
    }
    fn description(&self) -> String {
        format!("Remove {} from {}", self.component.kind, self.entity_id)
    }
}

pub struct UpdateComponentCommand {
    pub entity_id: String,
    pub component_index: usize,
    pub old_component: SceneComponent,
    pub new_component: SceneComponent,
}

impl EditCommand for UpdateComponentCommand {
    fn execute(&self, scene: &mut SceneDocument) -> Result<(), String> {
        let entity = scene
            .entities
            .iter_mut()
            .find(|e| e.id == self.entity_id)
            .ok_or_else(|| format!("entity not found: {}", self.entity_id))?;
        if let Some(c) = entity.components.get_mut(self.component_index) {
            *c = self.new_component.clone();
        }
        Ok(())
    }
    fn undo(&self, scene: &mut SceneDocument) -> Result<(), String> {
        let entity = scene
            .entities
            .iter_mut()
            .find(|e| e.id == self.entity_id)
            .ok_or_else(|| format!("entity not found: {}", self.entity_id))?;
        if let Some(c) = entity.components.get_mut(self.component_index) {
            *c = self.old_component.clone();
        }
        Ok(())
    }
    fn description(&self) -> String {
        format!("Update {} on {}", self.new_component.kind, self.entity_id)
    }
}

pub struct ReparentEntityCommand {
    pub entity_id: String,
    pub old_parent: Option<String>,
    pub new_parent: Option<String>,
}

impl EditCommand for ReparentEntityCommand {
    fn execute(&self, scene: &mut SceneDocument) -> Result<(), String> {
        let entity = scene
            .entities
            .iter_mut()
            .find(|e| e.id == self.entity_id)
            .ok_or_else(|| format!("entity not found: {}", self.entity_id))?;
        entity.parent = self.new_parent.clone();
        Ok(())
    }
    fn undo(&self, scene: &mut SceneDocument) -> Result<(), String> {
        let entity = scene
            .entities
            .iter_mut()
            .find(|e| e.id == self.entity_id)
            .ok_or_else(|| format!("entity not found: {}", self.entity_id))?;
        entity.parent = self.old_parent.clone();
        Ok(())
    }
    fn description(&self) -> String {
        format!("Reparent {}", self.entity_id)
    }
}

pub struct UpdateSceneSettingsCommand {
    pub section: String,
    pub old_values: Value,
    pub new_values: Value,
}

impl EditCommand for UpdateSceneSettingsCommand {
    fn execute(&self, scene: &mut SceneDocument) -> Result<(), String> {
        apply_scene_section(scene, &self.section, &self.new_values)
    }
    fn undo(&self, scene: &mut SceneDocument) -> Result<(), String> {
        apply_scene_section(scene, &self.section, &self.old_values)
    }
    fn description(&self) -> String {
        format!("Update scene {}", self.section)
    }
}

pub struct BatchCommand {
    pub commands: Vec<Box<dyn EditCommand>>,
    pub label: String,
}

impl EditCommand for BatchCommand {
    fn execute(&self, scene: &mut SceneDocument) -> Result<(), String> {
        for cmd in &self.commands {
            cmd.execute(scene)?;
        }
        Ok(())
    }
    fn undo(&self, scene: &mut SceneDocument) -> Result<(), String> {
        for cmd in self.commands.iter().rev() {
            cmd.undo(scene)?;
        }
        Ok(())
    }
    fn description(&self) -> String {
        self.label.clone()
    }
}

fn apply_scene_section(scene: &mut SceneDocument, section: &str, values: &Value) -> Result<(), String> {
    match section {
        "environment" => scene.environment = values.clone(),
        "render" => scene.render = values.clone(),
        "audio" => scene.audio = values.clone(),
        other => return Err(format!("unknown scene section: {other}")),
    }
    Ok(())
}
