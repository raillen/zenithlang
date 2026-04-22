# Borealis Module Decision - Camera3D

- Status: proposed
- Date: 2026-04-22
- Type: module / camera gameplay API
- Scope: `borealis.game.camera3d`

## Summary

Camera3D defines how the player sees the world.
The API is split into core camera controls and ready-to-use presets.

## Implementation

- keep a clean beginner path with preset-based cameras;
- keep full control available with low-level camera functions;
- make camera state readable for debug, save, and future editor tools;
- keep per-frame update explicit (`camera_update_mode`).

## Proposed API

### Core camera

- `camera3d_create(position: math.Vector3, target: math.Vector3, fov: float) -> result<camera3d.Camera3D, core.Error>`: creates a camera.
- `camera3d_set_position(camera: camera3d.Camera3D, position: math.Vector3) -> result<void, core.Error>`: moves the camera.
- `camera3d_get_position(camera: camera3d.Camera3D) -> math.Vector3`: reads camera position.
- `camera3d_set_target(camera: camera3d.Camera3D, target: math.Vector3) -> result<void, core.Error>`: sets look target.
- `camera3d_get_target(camera: camera3d.Camera3D) -> math.Vector3`: reads look target.
- `camera3d_set_fov(camera: camera3d.Camera3D, fov: float) -> result<void, core.Error>`: sets field of view.
- `camera3d_get_fov(camera: camera3d.Camera3D) -> float`: reads field of view.
- `camera3d_zoom(camera: camera3d.Camera3D, amount: float) -> result<void, core.Error>`: zooms in or out.
- `camera3d_shake(camera: camera3d.Camera3D, intensity: float, duration: float) -> result<void, core.Error>`: applies camera shake.
- `camera3d_update_effects(camera: camera3d.Camera3D, dt: float) -> result<void, core.Error>`: updates shake and other effects.

### Presets (`camera.presets`)

- `camera_use_fps(camera: camera3d.Camera3D, config: camera3d.FpsConfig) -> result<void, core.Error>`: first-person preset.
- `camera_use_tps(camera: camera3d.Camera3D, config: camera3d.TpsConfig) -> result<void, core.Error>`: third-person preset.
- `camera_use_topdown(camera: camera3d.Camera3D, config: camera3d.TopDownConfig) -> result<void, core.Error>`: top-down preset.
- `camera_use_isometric(camera: camera3d.Camera3D, config: camera3d.IsometricConfig) -> result<void, core.Error>`: isometric preset.
- `camera_use_over_shoulder(camera: camera3d.Camera3D, config: camera3d.OverShoulderConfig) -> result<void, core.Error>`: shoulder preset.
- `camera_use_fixed(camera: camera3d.Camera3D, config: camera3d.FixedConfig) -> result<void, core.Error>`: fixed camera preset.
- `camera_mode_get(camera: camera3d.Camera3D) -> camera3d.CameraMode`: returns active preset mode.
- `camera_update_mode(camera: camera3d.Camera3D, dt: float) -> result<void, core.Error>`: updates active preset behavior.
- `camera_blend_to_mode(camera: camera3d.Camera3D, mode: camera3d.CameraMode, duration: float) -> result<void, core.Error>`: smooth transition between presets.

### Auxiliary helpers

- `camera_look_at_point(camera: camera3d.Camera3D, point: math.Vector3) -> result<void, core.Error>`: points camera at a world position.
- `camera_look_at_entity(camera: camera3d.Camera3D, entity: entities.Entity, offset: math.Vector3) -> result<void, core.Error>`: points camera at an entity.
- `camera_set_bounds(camera: camera3d.Camera3D, min: math.Vector3, max: math.Vector3) -> result<void, core.Error>`: clamps camera movement to an area.
- `camera_lock_axis(camera: camera3d.Camera3D, lock_x: bool, lock_y: bool, lock_z: bool) -> result<void, core.Error>`: locks movement axes.
- `camera_world_to_screen(camera: camera3d.Camera3D, world_pos: math.Vector3) -> math.Point2`: converts world point to screen point.
- `camera_screen_to_world_ray(camera: camera3d.Camera3D, screen_pos: math.Point2) -> physics3d.Ray`: builds a world ray from screen coordinates.
- `camera_is_visible(camera: camera3d.Camera3D, world_pos: math.Vector3, radius: float) -> bool`: basic visibility test.
- `camera_set_deadzone(camera: camera3d.Camera3D, x: float, y: float, w: float, h: float) -> result<void, core.Error>`: configures follow deadzone.
- `camera_set_collision(camera: camera3d.Camera3D, enabled: bool, radius: float) -> result<void, core.Error>`: prevents camera clipping through walls.
- `camera_reset(camera: camera3d.Camera3D) -> result<void, core.Error>`: resets to default state of current preset.
- `camera_debug_info(camera: camera3d.Camera3D) -> camera3d.CameraDebugInfo`: returns debug/editor-oriented state.

## Notes

- presets are the default path for most games.
- core camera functions stay available for advanced control.
- API keeps the same accessibility principle used in Borealis 2D.
