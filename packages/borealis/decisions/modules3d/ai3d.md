# Borealis Module Decision - AI3D

- Status: proposed
- Date: 2026-04-22
- Type: module / 3D AI helpers
- Scope: `borealis.game.ai3d`

## Summary

AI3D is the 3D helper layer for Borealis AI behavior.
It adds spatial navigation, perception, steering, and simple combat on top of the generic `borealis.game.ai` foundations.

## Implementation

- keep `borealis.game.ai` as the canonical base for generic AI concepts such as state machines, behavior trees, blackboards, timers, and cooldowns;
- keep `ai3d` focused on 3D-space helpers;
- avoid heavy systems such as crowd simulation or advanced tactical coordination in v1;
- keep names consistent with the accessible naming already accepted in Borealis.

## Recommended structure

- `borealis.game.ai3d`
- conceptual areas:
  - `navigation`
  - `perception`
  - `steering`
  - `combat`

## Proposed API

### Agent lifecycle

- `ai3d_create(entity: entities.Entity) -> result<ai3d.Ai3dId, core.Error>`: creates an AI controller for an entity.
- `ai3d_destroy(id: ai3d.Ai3dId) -> result<void, core.Error>`: removes an AI controller.
- `ai3d_exists(id: ai3d.Ai3dId) -> bool`: checks if the controller exists.
- `ai3d_update(id: ai3d.Ai3dId, dt: float) -> result<void, core.Error>`: updates AI state for the frame.
- `ai3d_set_enabled(id: ai3d.Ai3dId, enabled: bool) -> result<void, core.Error>`: toggles AI updates.
- `ai3d_is_enabled(id: ai3d.Ai3dId) -> bool`: reads enabled state.

### Steering and movement

- `ai3d_seek(id: ai3d.Ai3dId, position: game.Vector3) -> result<void, core.Error>`: moves toward a point.
- `ai3d_follow_entity(id: ai3d.Ai3dId, target: entities.Entity, distance: float) -> result<void, core.Error>`: follows an entity while keeping distance.
- `ai3d_chase_entity(id: ai3d.Ai3dId, target: entities.Entity) -> result<void, core.Error>`: chases an entity.
- `ai3d_evade_entity(id: ai3d.Ai3dId, target: entities.Entity) -> result<void, core.Error>`: moves away from an entity.
- `ai3d_move_randomly(id: ai3d.Ai3dId, center: game.Vector3, radius: float) -> result<void, core.Error>`: moves randomly inside an area.
- `ai3d_patrol_points(id: ai3d.Ai3dId, points: list<game.Vector3>) -> result<void, core.Error>`: patrols a list of points.
- `ai3d_stop(id: ai3d.Ai3dId) -> result<void, core.Error>`: stops current movement.
- `ai3d_look_at(id: ai3d.Ai3dId, position: game.Vector3) -> result<void, core.Error>`: faces a point.
- `ai3d_face_entity(id: ai3d.Ai3dId, target: entities.Entity) -> result<void, core.Error>`: faces another entity.
- `ai3d_has_arrived(id: ai3d.Ai3dId) -> bool`: checks whether the destination was reached.

### Navigation

- `ai3d_set_navigation_mode(id: ai3d.Ai3dId, mode: ai3d.NavigationMode) -> result<void, core.Error>`: sets navigation mode (`direct`, `grid`, `navpoints`, `navmesh`).
- `ai3d_get_navigation_mode(id: ai3d.Ai3dId) -> ai3d.NavigationMode`: reads navigation mode.
- `ai3d_set_destination(id: ai3d.Ai3dId, position: game.Vector3) -> result<void, core.Error>`: sets navigation destination.
- `ai3d_get_destination(id: ai3d.Ai3dId) -> optional<game.Vector3>`: reads current destination.
- `ai3d_path_find(id: ai3d.Ai3dId, from: game.Vector3, to: game.Vector3) -> result<void, core.Error>`: builds a path between two points.
- `ai3d_path_clear(id: ai3d.Ai3dId) -> result<void, core.Error>`: clears the current path.
- `ai3d_path_get(id: ai3d.Ai3dId) -> ai3d.Path3D`: reads the current path.
- `ai3d_path_next_point(id: ai3d.Ai3dId) -> optional<game.Vector3>`: reads the next point in the path.

### Perception

- `ai3d_set_view_distance(id: ai3d.Ai3dId, value: float) -> result<void, core.Error>`: sets view distance.
- `ai3d_get_view_distance(id: ai3d.Ai3dId) -> float`: reads view distance.
- `ai3d_set_view_angle(id: ai3d.Ai3dId, value: float) -> result<void, core.Error>`: sets field-of-view angle.
- `ai3d_get_view_angle(id: ai3d.Ai3dId) -> float`: reads field-of-view angle.
- `ai3d_can_see_entity(id: ai3d.Ai3dId, target: entities.Entity) -> bool`: checks if the target is visible.
- `ai3d_can_hear_entity(id: ai3d.Ai3dId, target: entities.Entity, radius: float) -> bool`: checks if the target can be heard.
- `ai3d_find_visible_entity(id: ai3d.Ai3dId, tag: text) -> optional<entities.Entity>`: finds a visible entity by tag.
- `ai3d_find_nearest_entity(id: ai3d.Ai3dId, tag: text, radius: float) -> optional<entities.Entity>`: finds the nearest entity by tag.

### Simple combat

- `ai3d_set_aggro_range(id: ai3d.Ai3dId, value: float) -> result<void, core.Error>`: sets aggro range.
- `ai3d_get_aggro_range(id: ai3d.Ai3dId) -> float`: reads aggro range.
- `ai3d_set_attack_range(id: ai3d.Ai3dId, value: float) -> result<void, core.Error>`: sets attack range.
- `ai3d_get_attack_range(id: ai3d.Ai3dId) -> float`: reads attack range.
- `ai3d_can_attack(id: ai3d.Ai3dId, target: entities.Entity) -> bool`: checks if the target can be attacked now.
- `ai3d_attack(id: ai3d.Ai3dId, target: entities.Entity) -> result<void, core.Error>`: performs a simple attack action.
- `ai3d_set_target(id: ai3d.Ai3dId, target: entities.Entity) -> result<void, core.Error>`: sets current target.
- `ai3d_get_target(id: ai3d.Ai3dId) -> optional<entities.Entity>`: reads current target.
- `ai3d_clear_target(id: ai3d.Ai3dId) -> result<void, core.Error>`: clears current target.

### Integration with generic AI and runtime

- `ai3d_bind_state_machine(id: ai3d.Ai3dId, machine: ai.StateMachineId) -> result<void, core.Error>`: binds a generic state machine.
- `ai3d_bind_behavior_tree(id: ai3d.Ai3dId, tree: ai.BehaviorTreeId) -> result<void, core.Error>`: binds a generic behavior tree.
- `ai3d_bind_body(id: ai3d.Ai3dId, body: physics3d.BodyId) -> result<void, core.Error>`: binds a physics body.
- `ai3d_bind_animator(id: ai3d.Ai3dId, animator: animation3d.AnimatorId) -> result<void, core.Error>`: binds an animator.

### Helpers

- `ai3d_set_home(id: ai3d.Ai3dId, position: game.Vector3) -> result<void, core.Error>`: sets a home/base position.
- `ai3d_go_home(id: ai3d.Ai3dId) -> result<void, core.Error>`: moves back to home.
- `ai3d_set_leash_distance(id: ai3d.Ai3dId, value: float) -> result<void, core.Error>`: limits how far the agent can move from home.
- `ai3d_is_idle(id: ai3d.Ai3dId) -> bool`: checks whether the AI has no active task.
- `ai3d_debug_info(id: ai3d.Ai3dId) -> ai3d.Ai3dDebugInfo`: returns debug/editor-facing state.

## Notes

- the accepted easy names remain preferred here:
  - `seek`
  - `evade`
  - `move_randomly`
- early value should focus on `seek`, `follow`, `chase`, `evade`, `patrol`, simple vision, target handling, and arrived checks.

