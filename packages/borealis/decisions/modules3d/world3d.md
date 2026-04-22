# Borealis Module Decision - World3D

- Status: proposed
- Date: 2026-04-22
- Type: module / world and environment API
- Scope: `borealis.game.world3d`

## Summary

World3D defines static environment features for 3D games:
terrain, heightmap, skybox, bounds, and logical grid data.
It also owns world-space atmosphere and weather behavior.

## Implementation

- keep world setup simple for beginners;
- keep terrain streaming support via chunks;
- keep procedural generation separated from world runtime concerns;
- support layered skybox animation with explicit alpha behavior.
- keep atmosphere/weather lightweight and gameplay-focused (no realistic volumetric requirement).

## Proposed API

### World lifecycle and bounds

- `world3d_create(name: text) -> result<world3d.World3D, core.Error>`: creates a world.
- `world3d_set_active(world: world3d.World3D) -> result<void, core.Error>`: sets active world.
- `world3d_get_active() -> optional<world3d.World3D>`: gets active world.
- `world3d_set_bounds(world: world3d.World3D, min: math.Vector3, max: math.Vector3) -> result<void, core.Error>`: sets world bounds.
- `world3d_get_bounds(world: world3d.World3D) -> world3d.Bounds3D`: gets world bounds.
- `world3d_set_origin(world: world3d.World3D, origin: math.Vector3) -> result<void, core.Error>`: sets world origin.
- `world3d_get_origin(world: world3d.World3D) -> math.Vector3`: gets world origin.
- `world3d_clamp_to_bounds(world: world3d.World3D, position: math.Vector3) -> math.Vector3`: clamps a position to bounds.
- `world3d_debug_info(world: world3d.World3D) -> world3d.World3DDebugInfo`: returns debug/editor state.

### Terrain and height

- `terrain_create_plane(world: world3d.World3D, size: math.Vector2, material: assets.Material) -> result<void, core.Error>`: creates flat terrain.
- `terrain_create_grid(world: world3d.World3D, width: int, depth: int, cell_size: float, material: assets.Material) -> result<void, core.Error>`: creates grid terrain.
- `terrain_set_material(world: world3d.World3D, material: assets.Material) -> result<void, core.Error>`: sets terrain material.
- `terrain_get_material(world: world3d.World3D) -> optional<assets.Material>`: gets terrain material.
- `heightmap_load(world: world3d.World3D, image: assets.Image, height_scale: float) -> result<void, core.Error>`: builds terrain from heightmap.
- `heightmap_set_scale(world: world3d.World3D, scale: math.Vector3) -> result<void, core.Error>`: sets terrain height/space scale.
- `heightmap_sample(world: world3d.World3D, x: float, z: float) -> float`: samples heightmap value.
- `terrain_get_height(world: world3d.World3D, x: float, z: float) -> float`: gets final terrain height.
- `world3d_snap_position_to_ground(world: world3d.World3D, position: math.Vector3) -> math.Vector3`: snaps point to ground.

### Terrain chunks

- `terrain_chunk_set_size(world: world3d.World3D, chunk_size: int) -> result<void, core.Error>`: sets chunk dimensions.
- `terrain_chunk_load(world: world3d.World3D, chunk_coord: world3d.ChunkCoord) -> result<void, core.Error>`: loads a chunk.
- `terrain_chunk_unload(world: world3d.World3D, chunk_coord: world3d.ChunkCoord) -> result<void, core.Error>`: unloads a chunk.
- `terrain_chunk_is_loaded(world: world3d.World3D, chunk_coord: world3d.ChunkCoord) -> bool`: checks chunk state.
- `terrain_chunk_update_stream(world: world3d.World3D, camera_pos: math.Vector3, radius: int) -> result<void, core.Error>`: streams chunks around camera.

### Procedural bridge (world-consuming helpers)

- `terrain_apply_noise(world: world3d.World3D, noise_config: procedural.NoiseConfig) -> result<void, core.Error>`: applies noise-generated heights.
- `terrain_bake_chunk_height(world: world3d.World3D, chunk_coord: world3d.ChunkCoord, noise_config: procedural.NoiseConfig) -> result<void, core.Error>`: bakes one chunk from noise.

### Skybox and background

- `skybox_set(world: world3d.World3D, sky: assets.Skybox) -> result<void, core.Error>`: sets world skybox.
- `skybox_get(world: world3d.World3D) -> optional<assets.Skybox>`: gets current skybox.
- `skybox_clear(world: world3d.World3D) -> result<void, core.Error>`: clears skybox.
- `background_set_color(world: world3d.World3D, color: core.Color) -> result<void, core.Error>`: sets fallback clear color.
- `background_get_color(world: world3d.World3D) -> core.Color`: gets fallback clear color.

### Layered and animated skybox

- `skybox_layer_add(world: world3d.World3D, config: world3d.SkyLayerConfig) -> result<world3d.LayerId, core.Error>`: adds a sky layer.
- `skybox_layer_remove(world: world3d.World3D, layer_id: world3d.LayerId) -> result<void, core.Error>`: removes a sky layer.
- `skybox_layer_set_enabled(world: world3d.World3D, layer_id: world3d.LayerId, enabled: bool) -> result<void, core.Error>`: toggles layer.
- `skybox_layer_set_speed(world: world3d.World3D, layer_id: world3d.LayerId, speed: math.Vector2) -> result<void, core.Error>`: sets UV animation speed.
- `skybox_layer_set_offset(world: world3d.World3D, layer_id: world3d.LayerId, offset: math.Vector2) -> result<void, core.Error>`: sets UV offset.
- `skybox_layer_set_opacity(world: world3d.World3D, layer_id: world3d.LayerId, opacity: float) -> result<void, core.Error>`: sets alpha/opacity.
- `skybox_layer_set_blend_mode(world: world3d.World3D, layer_id: world3d.LayerId, mode: world3d.BlendMode) -> result<void, core.Error>`: sets layer blend mode.
- `skybox_layer_set_premultiplied_alpha(world: world3d.World3D, layer_id: world3d.LayerId, enabled: bool) -> result<void, core.Error>`: sets premultiplied alpha mode.
- `skybox_layer_get(world: world3d.World3D, layer_id: world3d.LayerId) -> optional<world3d.SkyLayerConfig>`: reads full layer config.
- `skybox_update(world: world3d.World3D, dt: float) -> result<void, core.Error>`: updates animated layers.

### Logical grid

- `world_to_cell(world: world3d.World3D, position: math.Vector3) -> world3d.Cell3`: converts world position to cell.
- `cell_to_world(world: world3d.World3D, cell: world3d.Cell3) -> math.Vector3`: converts cell to world position.
- `cell_set_walkable(world: world3d.World3D, cell: world3d.Cell3, value: bool) -> result<void, core.Error>`: sets walkable.
- `cell_is_walkable(world: world3d.World3D, cell: world3d.Cell3) -> bool`: reads walkable.
- `cell_set_solid(world: world3d.World3D, cell: world3d.Cell3, value: bool) -> result<void, core.Error>`: sets solid.
- `cell_is_solid(world: world3d.World3D, cell: world3d.Cell3) -> bool`: reads solid.
- `cell_set_tag(world: world3d.World3D, cell: world3d.Cell3, tag: text) -> result<void, core.Error>`: tags a cell.
- `cell_get_tag(world: world3d.World3D, cell: world3d.Cell3) -> text`: reads cell tag.

## Notes

- procedural generation stays owned by `borealis.game.procedural`.
- `world3d` can consume generated data via bridge helpers.
- layered skybox explicitly supports alpha behavior and blend configuration.
- recommended submodules:
  - `world3d.terrain`
  - `world3d.chunks`
  - `world3d.sky`
  - `world3d.atmosphere`
  - `world3d.weather`
  - `world3d.grid`
  - `world3d.streaming`
