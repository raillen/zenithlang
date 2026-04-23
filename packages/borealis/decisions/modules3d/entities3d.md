# Borealis Module Decision - Entities3D

- Status: proposed
- Date: 2026-04-22
- Type: helper submodule / 3D entity conveniences
- Scope: `borealis.game.entities3d`

## Summary

Entities3D is not a separate entity system.
It is a helper layer built on top of `borealis.game.entities` for 3D-specific workflows.

## Implementation

- keep `borealis.game.entities` as the single canonical entity model;
- keep `entities3d` as an optional convenience layer for 3D games;
- avoid duplicating save, editor, hierarchy, tags, and stable IDs;
- bind 3D-facing runtime systems through helpers instead of a second entity core.

## What stays in `entities`

- create/destroy entity;
- stable id;
- name;
- tags;
- parent/child;
- active/visible;
- snapshot/clone/inspect.

## What belongs to `entities3d`

- 3D transform helpers;
- model and material binding;
- physics3d binding;
- animation3d binding;
- spawn/look-at helpers.

## Proposed API

- `entity3d_create(name: text) -> result<entities.Entity, core.Error>`: creates an entity prepared for 3D use.
- `entity3d_spawn_model(name: text, model_id: assets3d.ModelHandle) -> result<entities.Entity, core.Error>`: creates a 3D entity with a bound model.
- `entity3d_set_position(entity: entities.Entity, position: game.Vector3) -> result<void, core.Error>`: sets 3D position.
- `entity3d_get_position(entity: entities.Entity) -> game.Vector3`: reads 3D position.
- `entity3d_set_rotation(entity: entities.Entity, rotation: game.Vector3) -> result<void, core.Error>`: sets 3D rotation.
- `entity3d_get_rotation(entity: entities.Entity) -> game.Vector3`: reads 3D rotation.
- `entity3d_set_scale(entity: entities.Entity, scale: game.Vector3) -> result<void, core.Error>`: sets 3D scale.
- `entity3d_get_scale(entity: entities.Entity) -> game.Vector3`: reads 3D scale.
- `entity3d_translate(entity: entities.Entity, delta: game.Vector3) -> result<void, core.Error>`: moves entity by delta.
- `entity3d_rotate(entity: entities.Entity, delta: game.Vector3) -> result<void, core.Error>`: rotates entity by delta.
- `entity3d_look_at(entity: entities.Entity, target: game.Vector3) -> result<void, core.Error>`: makes entity face a world point.
- `entity3d_set_model(entity: entities.Entity, model_id: assets3d.ModelHandle) -> result<void, core.Error>`: binds a model to the entity.
- `entity3d_get_model(entity: entities.Entity) -> optional<assets3d.ModelHandle>`: reads bound model.
- `entity3d_set_material_override(entity: entities.Entity, slot: int, material_id: assets3d.MaterialHandle) -> result<void, core.Error>`: overrides one model slot material.
- `entity3d_get_material_override(entity: entities.Entity, slot: int) -> optional<assets3d.MaterialHandle>`: reads material override.
- `entity3d_bind_body(entity: entities.Entity, body_id: physics3d.BodyId) -> result<void, core.Error>`: binds a physics body.
- `entity3d_get_body(entity: entities.Entity) -> optional<physics3d.BodyId>`: reads bound physics body.
- `entity3d_bind_animator(entity: entities.Entity, animator_id: animation3d.AnimatorId) -> result<void, core.Error>`: binds an animator.
- `entity3d_get_animator(entity: entities.Entity) -> optional<animation3d.AnimatorId>`: reads bound animator.
- `entity3d_set_static(entity: entities.Entity, enabled: bool) -> result<void, core.Error>`: marks entity as static.
- `entity3d_is_static(entity: entities.Entity) -> bool`: reads static flag.
- `entity3d_debug_info(entity: entities.Entity) -> entities3d.Entity3DDebugInfo`: returns debug/editor-facing 3D info.

## Notes

- 2D games do not need `entities3d`.
- a future `entities2d` helper can follow the same pattern.
- quaternions can remain hidden in `borealis.engine` if needed later.

