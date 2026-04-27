use std::sync::Mutex;

use serde_json::Value;

use crate::models::component::ComponentSchema;
use crate::models::scene::SceneComponent;
use crate::services::component_registry::ComponentRegistry;
use crate::state::AppState;

#[tauri::command]
pub fn get_component_schemas() -> Vec<ComponentSchema> {
    let registry = ComponentRegistry::new();
    registry.all_schemas().into_iter().cloned().collect()
}

#[tauri::command]
pub fn get_component_schema(kind: String) -> Result<ComponentSchema, String> {
    let registry = ComponentRegistry::new();
    registry
        .get_schema(&kind)
        .cloned()
        .ok_or_else(|| format!("unknown component kind: {kind}"))
}

#[tauri::command]
pub fn add_component(
    state: tauri::State<'_, Mutex<AppState>>,
    entity_id: String,
    kind: String,
) -> Result<SceneComponent, String> {
    let registry = ComponentRegistry::new();
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    let entity = project
        .scene
        .entities
        .iter_mut()
        .find(|e| e.id == entity_id)
        .ok_or_else(|| format!("entity not found: {entity_id}"))?;

    registry.validate_add(&kind, &entity.components)?;

    let missing = registry.missing_requirements(&kind, &entity.components);
    for required_kind in &missing {
        let dep = registry.create_default_component(required_kind)?;
        entity.components.push(dep);
    }

    let component = registry.create_default_component(&kind)?;
    entity.components.push(component.clone());
    project.dirty = true;
    Ok(component)
}

#[tauri::command]
pub fn remove_component(
    state: tauri::State<'_, Mutex<AppState>>,
    entity_id: String,
    component_index: usize,
) -> Result<(), String> {
    let registry = ComponentRegistry::new();
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    let entity = project
        .scene
        .entities
        .iter_mut()
        .find(|e| e.id == entity_id)
        .ok_or_else(|| format!("entity not found: {entity_id}"))?;

    if component_index >= entity.components.len() {
        return Err(format!(
            "component index {component_index} out of range (entity has {} components)",
            entity.components.len()
        ));
    }

    let kind = entity.components[component_index].kind.clone();
    let dependents = registry.dependents_of(&kind, &entity.components);
    if !dependents.is_empty() {
        return Err(format!(
            "cannot remove {kind}: required by {}",
            dependents.join(", ")
        ));
    }

    entity.components.remove(component_index);
    project.dirty = true;
    Ok(())
}

#[tauri::command]
pub fn update_component(
    state: tauri::State<'_, Mutex<AppState>>,
    entity_id: String,
    component_index: usize,
    field_key: String,
    value: Value,
) -> Result<(), String> {
    let mut guard = state.lock().map_err(|e| e.to_string())?;
    let project = guard.project.as_mut().ok_or("no project open")?;
    let entity = project
        .scene
        .entities
        .iter_mut()
        .find(|e| e.id == entity_id)
        .ok_or_else(|| format!("entity not found: {entity_id}"))?;

    let component = entity
        .components
        .get_mut(component_index)
        .ok_or_else(|| format!("component index {component_index} out of range"))?;

    match field_key.as_str() {
        "asset" => component.asset = value.as_str().map(str::to_string),
        "script" => component.script = value.as_str().map(str::to_string),
        "profile" => component.profile = value.as_str().map(str::to_string),
        key => {
            if let Value::Object(ref mut props) = component.properties {
                props.insert(key.to_string(), value);
            }
        }
    }

    project.dirty = true;
    Ok(())
}
