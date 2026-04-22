# Borealis Module Decision - Render3D

- Status: proposed
- Date: 2026-04-22
- Type: module / 3D rendering API
- Scope: `borealis.game.render3d`

## Summary

Render3D is the drawing layer for Borealis 3D.
It provides a simple API for meshes, primitives, lighting, depth, and billboards.

## Implementation

- keep beginner-first API for fast prototypes;
- keep frame flow explicit (`render3d_begin` and `render3d_end`);
- keep lighting state queryable (`set/get`) for debug, save, and editor use;
- expose light IDs in public API, map to internal slots in backend.

## Proposed API

- `render3d_begin(camera: camera3d.Camera3D) -> result<void, core.Error>`: starts 3D rendering for the frame.
- `render3d_end() -> result<void, core.Error>`: ends 3D rendering for the frame.
- `draw_mesh(mesh: assets.Mesh, material: assets.Material, transform: math.Transform3D) -> result<void, core.Error>`: draws a 3D mesh.
- `draw_cube(position: math.Vector3, size: math.Vector3, color: core.Color) -> result<void, core.Error>`: draws a quick cube for prototyping.
- `draw_plane(center: math.Vector3, size: math.Vector2, material: assets.Material) -> result<void, core.Error>`: draws a plane/floor.
- `draw_billboard(texture: assets.Texture, position: math.Vector3, size: math.Vector2, tint: core.Color) -> result<void, core.Error>`: draws a 2D sprite facing the camera.

### Render state

- `set_depth_test(enabled: bool) -> result<void, core.Error>`: enables/disables depth test.
- `get_depth_test() -> bool`: returns whether depth test is enabled.
- `set_wireframe(enabled: bool) -> result<void, core.Error>`: enables/disables wireframe mode.
- `get_wireframe() -> bool`: returns whether wireframe mode is enabled.
- `render3d_set_quality_profile(level: render3d.QualityProfile) -> result<void, core.Error>`: applies `low`, `medium`, or `high` profile.
- `render3d_get_quality_profile() -> render3d.QualityProfile`: reads active quality profile.

### Ambient and directional light

- `set_ambient_light(color: core.Color, intensity: float) -> result<void, core.Error>`: sets ambient global light.
- `get_ambient_light() -> lighting.AmbientLight`: reads ambient light config.
- `set_directional_light(direction: math.Vector3, color: core.Color, intensity: float) -> result<void, core.Error>`: sets directional light.
- `get_directional_light() -> lighting.DirectionalLight`: reads directional light config.

### Point lights (ID-based public API)

- `point_light_create(position: math.Vector3, color: core.Color, intensity: float, range: float) -> result<lighting.LightId, core.Error>`: creates a point light and returns its ID.
- `point_light_destroy(id: lighting.LightId) -> result<void, core.Error>`: removes a point light.
- `point_light_exists(id: lighting.LightId) -> bool`: checks if a point light exists.
- `point_light_set_enabled(id: lighting.LightId, enabled: bool) -> result<void, core.Error>`: enables/disables a point light.
- `point_light_is_enabled(id: lighting.LightId) -> bool`: checks if a point light is enabled.
- `point_light_toggle(id: lighting.LightId) -> result<bool, core.Error>`: toggles enabled state and returns new state.
- `point_light_set_position(id: lighting.LightId, position: math.Vector3) -> result<void, core.Error>`: updates light position.
- `point_light_get_position(id: lighting.LightId) -> result<math.Vector3, core.Error>`: reads light position.
- `point_light_set_color(id: lighting.LightId, color: core.Color) -> result<void, core.Error>`: updates light color.
- `point_light_get_color(id: lighting.LightId) -> result<core.Color, core.Error>`: reads light color.
- `point_light_set_intensity(id: lighting.LightId, value: float) -> result<void, core.Error>`: updates light intensity.
- `point_light_get_intensity(id: lighting.LightId) -> result<float, core.Error>`: reads light intensity.
- `point_light_get(id: lighting.LightId) -> optional<lighting.PointLight>`: reads full point light data.

### Light effects (easy layer)

- `light_fade_to(id: lighting.LightId, target_intensity: float, duration: float) -> result<void, core.Error>`: smooth transition to target intensity.
- `light_pulse(id: lighting.LightId, min_intensity: float, max_intensity: float, speed: float) -> result<void, core.Error>`: continuous pulse effect.
- `light_flicker(id: lighting.LightId, base_intensity: float, variance: float, speed: float, seed: int) -> result<void, core.Error>`: random-like lamp flicker effect.
- `light_follow_entity(id: lighting.LightId, entity: entities.Entity, offset: math.Vector3) -> result<void, core.Error>`: makes light follow an entity.
- `lights_update_effects(dt: float) -> result<void, core.Error>`: updates active light effects each frame.

## Notes

- public API uses IDs for ergonomics and editor-readiness.
- backend may use fixed-size slots internally (for example, 4 active point lights in v1).
- the ID-to-slot mapping stays internal and transparent to game code.
- optimization phasing follows `packages/borealis/decisions/009-borealis-3d-performance-phasing.md`.
