# Borealis Module Decision - Render3D

- Status: proposed
- Date: 2026-04-22
- Type: module / 3D rendering API
- Scope: `borealis.game.render3d`

## Summary

Render3D is the drawing layer for Borealis 3D.
It provides a simple API for meshes, models, primitives, lighting, depth, and billboards.

## Implementation

- keep beginner-first API for fast prototypes;
- keep frame flow explicit (`render3d_begin` and `render3d_end`);
- keep lighting state queryable (`set/get`) for debug, save, and editor use;
- expose light IDs in public API, map to internal slots in backend.

## Proposed API

- `render3d_begin(camera: camera3d.Camera3D) -> result<void, core.Error>`: starts 3D rendering for the frame.
- `render3d_end() -> result<void, core.Error>`: ends 3D rendering for the frame.
- `draw_mesh(mesh: assets3d.MeshId, material: assets3d.MaterialHandle, transform: game.Transform3D) -> result<void, core.Error>`: draws a 3D mesh.
- `draw_model(model: assets3d.ModelHandle, transform: game.Transform3D) -> result<void, core.Error>`: draws a full model.
- `draw_cube(position: game.Vector3, size: game.Vector3, color: game.Color) -> result<void, core.Error>`: draws a quick cube for prototyping.
- `draw_plane(center: game.Vector3, size: game.Vector2, material: assets3d.MaterialHandle) -> result<void, core.Error>`: draws a plane/floor.
- `draw_billboard(texture: assets3d.TextureId, position: game.Vector3, size: game.Vector2, tint: game.Color) -> result<void, core.Error>`: draws a 2D sprite facing the camera.

### Render state

- `set_depth_test(enabled: bool) -> result<void, core.Error>`: enables/disables depth test.
- `get_depth_test() -> bool`: returns whether depth test is enabled.
- `set_wireframe(enabled: bool) -> result<void, core.Error>`: enables/disables wireframe mode.
- `get_wireframe() -> bool`: returns whether wireframe mode is enabled.
- `render3d_set_quality_profile(level: render3d.QualityProfile) -> result<void, core.Error>`: applies `low`, `medium`, or `high` profile.
- `render3d_get_quality_profile() -> render3d.QualityProfile`: reads active quality profile.

### Ambient and directional light

- `set_ambient_light(color: game.Color, intensity: float) -> result<void, core.Error>`: sets ambient global light.
- `get_ambient_light() -> render3d.AmbientLightState`: reads ambient light config.
- `set_directional_light(direction: game.Vector3, color: game.Color, intensity: float) -> result<void, core.Error>`: sets directional light.
- `get_directional_light() -> render3d.DirectionalLightState`: reads directional light config.

### Point lights (ID-based public API)

- `point_light_create(position: game.Vector3, color: game.Color, intensity: float, range: float) -> result<render3d.LightId, core.Error>`: creates a point light and returns its ID.
- `point_light_destroy(id: render3d.LightId) -> result<void, core.Error>`: removes a point light.
- `point_light_exists(id: render3d.LightId) -> bool`: checks if a point light exists.
- `point_light_set_enabled(id: render3d.LightId, enabled: bool) -> result<void, core.Error>`: enables/disables a point light.
- `point_light_is_enabled(id: render3d.LightId) -> bool`: checks if a point light is enabled.
- `point_light_toggle(id: render3d.LightId) -> result<bool, core.Error>`: toggles enabled state and returns new state.
- `point_light_set_position(id: render3d.LightId, position: game.Vector3) -> result<void, core.Error>`: updates light position.
- `point_light_get_position(id: render3d.LightId) -> result<game.Vector3, core.Error>`: reads light position.
- `point_light_set_color(id: render3d.LightId, color: game.Color) -> result<void, core.Error>`: updates light color.
- `point_light_get_color(id: render3d.LightId) -> result<game.Color, core.Error>`: reads light color.
- `point_light_set_intensity(id: render3d.LightId, value: float) -> result<void, core.Error>`: updates light intensity.
- `point_light_get_intensity(id: render3d.LightId) -> result<float, core.Error>`: reads light intensity.
- `point_light_get(id: render3d.LightId) -> optional<render3d.PointLightState>`: reads full point light data.

### Light effects (easy layer)

- `light_fade_to(id: render3d.LightId, target_intensity: float, duration: float) -> result<void, core.Error>`: smooth transition to target intensity.
- `light_pulse(id: render3d.LightId, min_intensity: float, max_intensity: float, speed: float) -> result<void, core.Error>`: continuous pulse effect.
- `light_flicker(id: render3d.LightId, base_intensity: float, variance: float, speed: float, seed: int) -> result<void, core.Error>`: random-like lamp flicker effect.
- `light_follow_entity(id: render3d.LightId, entity: entities.Entity, offset: game.Vector3) -> result<void, core.Error>`: makes light follow an entity.
- `lights_update_effects(dt: float) -> result<void, core.Error>`: updates active light effects each frame.

## Notes

- public API uses IDs for ergonomics and editor-readiness.
- asset-facing draw calls consume `assets3d` IDs to stay consistent with the resource layer.
- backend may use fixed-size slots internally (for example, 4 active point lights in v1).
- the ID-to-slot mapping stays internal and transparent to game code.
- optimization phasing follows `packages/borealis/decisions/009-borealis-3d-performance-phasing.md`.

