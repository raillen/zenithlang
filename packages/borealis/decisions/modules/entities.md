# Borealis Module Decision - Entities

- Status: proposed
- Date: 2026-04-22
- Type: module / ECS-facing gameplay API
- Scope: `borealis.game.entities`

## Summary

Entities are the visible gameplay-facing layer over the ECS and the future editor model.

## Implementation

- keep the public API simple and object-like;
- keep stable IDs for editor and save support;
- allow the engine to implement entities with ECS internally;
- separate entity state from behavior systems.

## Proposed API

- `entity_create(name) -> Entity`: creates an entity.
- `entity_destroy(entity)`: destroys an entity.
- `entity_exists(entity) -> bool`: checks whether the entity exists.
- `entity_set_active(entity, bool)`: enables or disables the entity.
- `entity_set_name(entity, name)`: sets the entity name.
- `entity_get_name(entity) -> text`: reads the entity name.
- `entity_set_position(entity, position)`: sets the position.
- `entity_get_position(entity) -> Vector2`: reads the position.
- `entity_set_rotation(entity, angle)`: sets the rotation.
- `entity_get_rotation(entity) -> float`: reads the rotation.
- `entity_set_scale(entity, scale)`: sets the scale.
- `entity_get_scale(entity) -> Vector2`: reads the scale.
- `entity_add_tag(entity, tag)`: adds a tag.
- `entity_remove_tag(entity, tag)`: removes a tag.
- `entity_has_tag(entity, tag) -> bool`: checks a tag.
- `entity_parent(entity, parent)`: sets the parent entity.
- `entity_children(entity) -> Entity[]`: lists child entities.
- `entity_remove_parent(entity)`: clears the parent.
- `entity_has_parent(entity) -> bool`: checks whether it has a parent.
- `entity_set_visible(entity, bool)`: shows or hides the entity.
- `entity_is_visible(entity) -> bool`: checks visibility.
- `entity_set_layer(entity, layer)`: sets the layer.
- `entity_get_layer(entity) -> text`: reads the layer.
- `entity_add_component(entity, component)`: adds a component.
- `entity_remove_component(entity, type)`: removes a component.
- `entity_has_component(entity, type) -> bool`: checks a component.
- `entity_get_component(entity, type) -> Component`: reads a component.
- `entity_list_components(entity) -> Component[]`: lists components.
- `entity_snapshot(entity) -> Data`: exports a serializable snapshot.
- `entity_restore_snapshot(entity, data)`: restores a snapshot.
- `entity_clone(entity) -> Entity`: clones the entity.
- `entity_mark_dirty(entity)`: marks the entity as changed.
- `entity_inspect(entity) -> EntityInfo`: returns editor/debug data.
- `entity_set_id(entity, id)`: sets a stable persistent id.

## Notes

- entities are the bridge between game code, save data, and editor tooling.
- components remain the technical ECS layer behind the scenes.
