# Borealis Module Decision - Debug3D

- Status: proposed
- Date: 2026-04-22
- Type: module / 3D diagnostics and overlays
- Scope: `borealis.game.debug3d`

## Summary

Debug3D provides simple visualization helpers for 3D development.
It is meant for gameplay diagnostics, editor support, and fast iteration.

## Implementation

- keep it opt-in and easy to strip from shipping builds;
- focus on the highest-value visuals first: axes, bounds, colliders, rays, paths, and chunk guides;
- let it inspect existing runtime systems instead of creating duplicate state.

## Recommended structure

- `borealis.game.debug3d`
- conceptual areas:
  - `gizmos`
  - `physics`
  - `world`
  - `ai`
  - `ui`

## Proposed API

### Global control

- `debug3d_set_enabled(enabled: bool) -> result<void, core.Error>`: toggles 3D debug rendering.
- `debug3d_is_enabled() -> bool`: reads global debug state.
- `debug3d_clear() -> result<void, core.Error>`: clears queued debug draw requests.

### Basic gizmos

- `debug3d_draw_axes(position: game.Vector3, size: float) -> result<void, core.Error>`: draws local axes at a point.
- `debug3d_draw_grid(center: game.Vector3, size: game.Vector2, spacing: float) -> result<void, core.Error>`: draws a world debug grid.
- `debug3d_draw_bounds(bounds: game.Bounds3D, color: game.Color) -> result<void, core.Error>`: draws a bounds box.
- `debug3d_draw_text(position: game.Vector3, text: text, color: game.Color) -> result<void, core.Error>`: draws simple world debug text.

### Physics and rays

- `debug3d_draw_body(body: physics3d.BodyId, color: game.Color) -> result<void, core.Error>`: draws one physics body.
- `debug3d_draw_colliders(enabled: bool) -> result<void, core.Error>`: toggles collider overlays.
- `debug3d_draw_ray(ray: game.Ray3, color: game.Color, length: float) -> result<void, core.Error>`: draws a debug ray.
- `debug3d_draw_contacts(body: physics3d.BodyId, color: game.Color) -> result<void, core.Error>`: draws contact points and normals.

### World and streaming

- `debug3d_draw_chunk_bounds(world: world3d.World3D, color: game.Color) -> result<void, core.Error>`: draws chunk boundaries.
- `debug3d_draw_navpoints(points: list<game.Vector3>, color: game.Color) -> result<void, core.Error>`: draws navigation points.
- `debug3d_draw_path(path: ai3d.Path3D, color: game.Color) -> result<void, core.Error>`: draws an AI path.
- `debug3d_draw_spawn_points(points: list<procedural3d.SpawnPoint>, color: game.Color) -> result<void, core.Error>`: draws generated spawn points.

### Runtime watches

- `debug3d_watch_entity(entity: entities.Entity) -> result<void, core.Error>`: adds an entity to the debug watch list.
- `debug3d_watch_body(body: physics3d.BodyId) -> result<void, core.Error>`: adds a body to the debug watch list.
- `debug3d_watch_ai(agent: ai3d.Ai3dId) -> result<void, core.Error>`: adds an AI controller to the debug watch list.
- `debug3d_watch_camera(camera: camera3d.Camera3D) -> result<void, core.Error>`: adds a camera to the debug watch list.
- `debug3d_unwatch_all() -> result<void, core.Error>`: clears the watch list.

### UI and audio helpers

- `debug3d_draw_ui_anchor(ui_id: ui3d.Ui3dId, color: game.Color) -> result<void, core.Error>`: draws a UI3D anchor/debug rect.
- `debug3d_draw_audio_range(instance_id: audio3d.InstanceId, color: game.Color) -> result<void, core.Error>`: draws spatial audio range.

### Inspection

- `debug3d_get_overlay_state() -> debug3d.Debug3dOverlayState`: reads current debug overlay state.

## Notes

- `debug3d` is the one extra 3D-specific module worth keeping even though generic `debug` already exists.
- generic `debug`, `scene`, `save`, `events`, `services`, `storage`, `database`, and `input` remain shared modules instead of getting `*3d` duplicates.

