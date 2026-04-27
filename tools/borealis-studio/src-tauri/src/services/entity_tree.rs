use std::collections::HashMap;

use uuid::Uuid;

use crate::models::entity::{AddEntityRequest, EntityId, EntityPatch, FlatTreeNode};
use crate::models::scene::{SceneDocument, SceneEntity, Transform3d};

pub struct EntityTree {
    entities: HashMap<EntityId, SceneEntity>,
    root_order: Vec<EntityId>,
    children: HashMap<EntityId, Vec<EntityId>>,
}

impl EntityTree {
    pub fn from_scene(scene: &SceneDocument) -> Self {
        let mut tree = Self {
            entities: HashMap::new(),
            root_order: Vec::new(),
            children: HashMap::new(),
        };
        for entity in &scene.entities {
            tree.entities.insert(entity.id.clone(), entity.clone());
            if let Some(parent_id) = &entity.parent {
                tree.children
                    .entry(parent_id.clone())
                    .or_default()
                    .push(entity.id.clone());
            } else {
                tree.root_order.push(entity.id.clone());
            }
        }
        tree
    }

    pub fn to_entities(&self) -> Vec<SceneEntity> {
        let mut result = Vec::new();
        for id in &self.root_order {
            self.collect_depth_first(id, &mut result);
        }
        result
    }

    fn collect_depth_first(&self, id: &str, out: &mut Vec<SceneEntity>) {
        if let Some(entity) = self.entities.get(id) {
            out.push(entity.clone());
            if let Some(child_ids) = self.children.get(id) {
                for child_id in child_ids {
                    self.collect_depth_first(child_id, out);
                }
            }
        }
    }

    pub fn flatten(&self) -> Vec<FlatTreeNode> {
        let mut nodes = Vec::new();
        for id in &self.root_order {
            self.flatten_recursive(id, 0, None, &mut nodes);
        }
        nodes
    }

    fn flatten_recursive(
        &self,
        id: &str,
        depth: usize,
        parent_id: Option<&str>,
        out: &mut Vec<FlatTreeNode>,
    ) {
        let Some(entity) = self.entities.get(id) else {
            return;
        };
        let child_ids = self.children.get(id);
        let child_count = child_ids.map_or(0, |c| c.len());
        out.push(FlatTreeNode {
            id: id.to_string(),
            name: entity.name.clone(),
            layer: entity.layer.clone(),
            depth,
            parent_id: parent_id.map(str::to_string),
            child_count,
            has_children: child_count > 0,
        });
        if let Some(child_ids) = child_ids {
            for child_id in child_ids {
                self.flatten_recursive(child_id, depth + 1, Some(id), out);
            }
        }
    }

    pub fn get(&self, id: &str) -> Option<&SceneEntity> {
        self.entities.get(id)
    }

    pub fn get_mut(&mut self, id: &str) -> Option<&mut SceneEntity> {
        self.entities.get_mut(id)
    }

    pub fn entity_count(&self) -> usize {
        self.entities.len()
    }

    pub fn add_entity(
        &mut self,
        request: &AddEntityRequest,
        transform: Transform3d,
    ) -> SceneEntity {
        let id = Uuid::new_v4().to_string();
        let entity = SceneEntity {
            id: id.clone(),
            name: request.name.clone(),
            layer: request.layer.clone(),
            parent: request.parent_id.clone(),
            tags: request.tags.clone(),
            components: Vec::new(),
            transform,
        };
        self.entities.insert(id.clone(), entity.clone());
        if let Some(parent_id) = &request.parent_id {
            self.children
                .entry(parent_id.clone())
                .or_default()
                .push(id);
        } else {
            self.root_order.push(id);
        }
        entity
    }

    pub fn remove_entity(&mut self, id: &str) -> Vec<SceneEntity> {
        let mut removed = Vec::new();
        self.remove_recursive(id, &mut removed);
        self.root_order.retain(|root_id| root_id != id);
        if let Some(entity) = self.entities.get(id) {
            if let Some(parent_id) = &entity.parent {
                if let Some(siblings) = self.children.get_mut(parent_id) {
                    siblings.retain(|sibling_id| sibling_id != id);
                }
            }
        }
        if let Some(entity) = self.entities.remove(id) {
            removed.insert(0, entity);
        }
        self.children.remove(id);
        removed
    }

    fn remove_recursive(&mut self, id: &str, removed: &mut Vec<SceneEntity>) {
        if let Some(child_ids) = self.children.get(id).cloned() {
            for child_id in &child_ids {
                self.remove_recursive(child_id, removed);
                if let Some(child) = self.entities.remove(child_id) {
                    removed.push(child);
                }
                self.children.remove(child_id);
            }
        }
    }

    pub fn update_entity(&mut self, id: &str, patch: &EntityPatch) -> Result<(), String> {
        let entity = self
            .entities
            .get_mut(id)
            .ok_or_else(|| format!("entity not found: {id}"))?;
        if let Some(name) = &patch.name {
            entity.name = name.clone();
        }
        if let Some(layer) = &patch.layer {
            entity.layer = layer.clone();
        }
        if let Some(tags) = &patch.tags {
            entity.tags = tags.clone();
        }
        Ok(())
    }

    pub fn update_transform(&mut self, id: &str, transform: Transform3d) -> Result<(), String> {
        let entity = self
            .entities
            .get_mut(id)
            .ok_or_else(|| format!("entity not found: {id}"))?;
        entity.transform = transform;
        Ok(())
    }

    pub fn duplicate_entity(&mut self, id: &str) -> Result<SceneEntity, String> {
        let source = self
            .entities
            .get(id)
            .ok_or_else(|| format!("entity not found: {id}"))?
            .clone();
        let new_id = Uuid::new_v4().to_string();
        let mut duplicate = source.clone();
        duplicate.id = new_id.clone();
        duplicate.name = format!("{} Copy", source.name);
        self.entities.insert(new_id.clone(), duplicate.clone());
        if let Some(parent_id) = &source.parent {
            self.children
                .entry(parent_id.clone())
                .or_default()
                .push(new_id);
        } else {
            let index = self
                .root_order
                .iter()
                .position(|root_id| root_id == id)
                .map(|i| i + 1)
                .unwrap_or(self.root_order.len());
            self.root_order.insert(index, new_id);
        }
        Ok(duplicate)
    }

    pub fn reparent(
        &mut self,
        id: &str,
        new_parent_id: Option<&str>,
        index: Option<usize>,
    ) -> Result<(), String> {
        if !self.entities.contains_key(id) {
            return Err(format!("entity not found: {id}"));
        }
        if let Some(parent_id) = new_parent_id {
            if !self.entities.contains_key(parent_id) {
                return Err(format!("parent entity not found: {parent_id}"));
            }
            if self.is_descendant_of(parent_id, id) {
                return Err("cannot reparent to a descendant".to_string());
            }
        }

        let old_parent = self.entities.get(id).and_then(|e| e.parent.clone());
        if let Some(old_parent_id) = &old_parent {
            if let Some(siblings) = self.children.get_mut(old_parent_id) {
                siblings.retain(|sibling_id| sibling_id != id);
            }
        } else {
            self.root_order.retain(|root_id| root_id != id);
        }

        if let Some(entity) = self.entities.get_mut(id) {
            entity.parent = new_parent_id.map(str::to_string);
        }

        if let Some(parent_id) = new_parent_id {
            let children = self.children.entry(parent_id.to_string()).or_default();
            let insert_at = index.unwrap_or(children.len()).min(children.len());
            children.insert(insert_at, id.to_string());
        } else {
            let insert_at = index.unwrap_or(self.root_order.len()).min(self.root_order.len());
            self.root_order.insert(insert_at, id.to_string());
        }

        Ok(())
    }

    fn is_descendant_of(&self, candidate: &str, ancestor: &str) -> bool {
        if candidate == ancestor {
            return true;
        }
        if let Some(child_ids) = self.children.get(ancestor) {
            for child_id in child_ids {
                if self.is_descendant_of(candidate, child_id) {
                    return true;
                }
            }
        }
        false
    }

    pub fn apply_to_scene(&self, scene: &mut SceneDocument) {
        scene.entities = self.to_entities();
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::models::scene::{SceneComponent, SceneDocument};
    use serde_json::json;

    fn make_scene() -> SceneDocument {
        SceneDocument {
            name: "test".to_string(),
            path: "test.scene.json".to_string(),
            document_id: "test:scene".to_string(),
            environment: json!({}),
            render: json!({}),
            audio: json!({}),
            entities: vec![
                SceneEntity {
                    id: "root-a".to_string(),
                    name: "Root A".to_string(),
                    layer: "default".to_string(),
                    parent: None,
                    tags: vec![],
                    components: vec![],
                    transform: Transform3d::default(),
                },
                SceneEntity {
                    id: "child-1".to_string(),
                    name: "Child 1".to_string(),
                    layer: "default".to_string(),
                    parent: Some("root-a".to_string()),
                    tags: vec![],
                    components: vec![],
                    transform: Transform3d::default(),
                },
                SceneEntity {
                    id: "root-b".to_string(),
                    name: "Root B".to_string(),
                    layer: "default".to_string(),
                    parent: None,
                    tags: vec![],
                    components: vec![],
                    transform: Transform3d::default(),
                },
            ],
        }
    }

    #[test]
    fn flatten_produces_correct_depth() {
        let scene = make_scene();
        let tree = EntityTree::from_scene(&scene);
        let flat = tree.flatten();
        assert_eq!(flat.len(), 3);
        assert_eq!(flat[0].id, "root-a");
        assert_eq!(flat[0].depth, 0);
        assert!(flat[0].has_children);
        assert_eq!(flat[1].id, "child-1");
        assert_eq!(flat[1].depth, 1);
        assert_eq!(flat[2].id, "root-b");
        assert_eq!(flat[2].depth, 0);
    }

    #[test]
    fn add_entity_generates_uuid() {
        let scene = make_scene();
        let mut tree = EntityTree::from_scene(&scene);
        let request = AddEntityRequest {
            name: "New Entity".to_string(),
            layer: "default".to_string(),
            tags: vec![],
            parent_id: None,
        };
        let entity = tree.add_entity(&request, Transform3d::default());
        assert!(uuid::Uuid::parse_str(&entity.id).is_ok());
        assert_eq!(tree.entity_count(), 4);
    }

    #[test]
    fn remove_entity_removes_children() {
        let scene = make_scene();
        let mut tree = EntityTree::from_scene(&scene);
        let removed = tree.remove_entity("root-a");
        assert_eq!(removed.len(), 2);
        assert_eq!(tree.entity_count(), 1);
    }

    #[test]
    fn duplicate_creates_copy() {
        let scene = make_scene();
        let mut tree = EntityTree::from_scene(&scene);
        let dup = tree.duplicate_entity("root-a").unwrap();
        assert_ne!(dup.id, "root-a");
        assert_eq!(dup.name, "Root A Copy");
        assert_eq!(tree.entity_count(), 4);
    }

    #[test]
    fn reparent_prevents_circular() {
        let scene = make_scene();
        let mut tree = EntityTree::from_scene(&scene);
        let result = tree.reparent("root-a", Some("child-1"), None);
        assert!(result.is_err());
    }
}
