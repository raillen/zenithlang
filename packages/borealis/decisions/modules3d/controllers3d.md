# Borealis Module Decision - Controllers3D

- Status: proposed
- Date: 2026-04-22
- Type: module / 3D gameplay controller helpers
- Scope: `borealis.game.controllers3d`

## Summary

Controllers3D is a convenience layer for common 3D gameplay control styles.
It does not replace the base systems.

## Implementation

- keep the foundations in `entities3d`, `physics3d`, `input`, `camera3d`, and `animation3d`;
- keep `controllers3d` as an easy glue layer for common control schemes;
- keep animation automation optional;
- keep `vehicles` as a separate module.

## Recommended structure

- `borealis.game.controllers3d`
- conceptual areas:
  - `character`
  - `flight`
  - `camera_bridge`

## Presets that should exist

- `fps`: first-person character controller.
- `tps`: third-person follow controller.
- `platformer3d`: 3D jumping and grounded movement.
- `topdown3d`: top-down world movement.
- `tank`: rotate body and move forward/backward.
- `free_fly`: free flight for ship, editor, or debug.

## Proposed API

### Controller lifecycle

- `controller3d_create_fps(entity: entities.Entity, camera: camera3d.Camera3D, config: controllers3d.FpsConfig) -> result<controllers3d.Controller3DHandle, core.Error>`: creates a first-person controller.
- `controller3d_create_tps(entity: entities.Entity, camera: camera3d.Camera3D, config: controllers3d.TpsConfig) -> result<controllers3d.Controller3DHandle, core.Error>`: creates a third-person controller.
- `controller3d_create_platformer(entity: entities.Entity, camera: camera3d.Camera3D, config: controllers3d.Platformer3dConfig) -> result<controllers3d.Controller3DHandle, core.Error>`: creates a 3D platformer controller.
- `controller3d_create_topdown(entity: entities.Entity, camera: camera3d.Camera3D, config: controllers3d.TopDown3dConfig) -> result<controllers3d.Controller3DHandle, core.Error>`: creates a top-down controller.
- `controller3d_create_tank(entity: entities.Entity, camera: camera3d.Camera3D, config: controllers3d.TankConfig) -> result<controllers3d.Controller3DHandle, core.Error>`: creates a tank-style controller.
- `controller3d_create_free_fly(entity: entities.Entity, camera: camera3d.Camera3D, config: controllers3d.FreeFlyConfig) -> result<controllers3d.Controller3DHandle, core.Error>`: creates a free-fly controller.
- `controller3d_destroy(id: controllers3d.Controller3DHandle) -> result<void, core.Error>`: removes a controller.
- `controller3d_update(id: controllers3d.Controller3DHandle, dt: float) -> result<void, core.Error>`: updates controller state by frame.
- `controller3d_set_enabled(id: controllers3d.Controller3DHandle, enabled: bool) -> result<void, core.Error>`: toggles controller.
- `controller3d_is_enabled(id: controllers3d.Controller3DHandle) -> bool`: reads enabled state.

### Binding helpers

- `controller3d_bind_animator(id: controllers3d.Controller3DHandle, animator: animation3d.AnimatorId, profile: controllers3d.AnimationProfile) -> result<void, core.Error>`: binds animation transitions such as `idle`, `walk`, `run`, and `jump`.
- `controller3d_bind_body(id: controllers3d.Controller3DHandle, body: physics3d.BodyId) -> result<void, core.Error>`: binds a physics body explicitly.
- `controller3d_bind_input_context(id: controllers3d.Controller3DHandle, context: input.InputContextId) -> result<void, core.Error>`: binds an input context.

### Movement toggles and direct actions

- `controller3d_set_move_enabled(id: controllers3d.Controller3DHandle, enabled: bool) -> result<void, core.Error>`: toggles movement updates.
- `controller3d_set_look_enabled(id: controllers3d.Controller3DHandle, enabled: bool) -> result<void, core.Error>`: toggles look/camera updates.
- `controller3d_set_jump_enabled(id: controllers3d.Controller3DHandle, enabled: bool) -> result<void, core.Error>`: toggles jumping.
- `controller3d_jump(id: controllers3d.Controller3DHandle) -> result<void, core.Error>`: forces a jump through code.
- `controller3d_dash(id: controllers3d.Controller3DHandle) -> result<void, core.Error>`: applies a short dash impulse.
- `controller3d_teleport(id: controllers3d.Controller3DHandle, position: game.Vector3) -> result<void, core.Error>`: teleports the controlled entity.
- `controller3d_face_move_direction(id: controllers3d.Controller3DHandle, enabled: bool) -> result<void, core.Error>`: rotates character toward movement direction.

### Debug and inspection

- `controller3d_get_entity(id: controllers3d.Controller3DHandle) -> optional<entities.Entity>`: reads controlled entity.
- `controller3d_get_camera(id: controllers3d.Controller3DHandle) -> optional<camera3d.Camera3D>`: reads bound camera.
- `controller3d_debug_info(id: controllers3d.Controller3DHandle) -> controllers3d.Controller3DDebugState`: returns debug/editor-facing state.

## Notes

- `fps`, `tps`, and `platformer3d` should be treated as the most valuable early presets.
- `tank`, `topdown3d`, and `free_fly` are still useful and should remain in scope.
- `vehicles` remains a separate module on purpose.

