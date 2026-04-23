# Borealis Module Decision - Procedural3D

- Status: proposed
- Date: 2026-04-22
- Type: module / 3D procedural generation helpers
- Scope: `borealis.game.procedural3d`

## Summary

Procedural3D is the 3D helper layer for procedural generation in Borealis.
It focuses on terrain, heightfields, chunk generation, scatter, and simple spatial layouts.

## Implementation

- keep `borealis.game.procedural` as the canonical base for generic seeds, noise, and generation rules;
- keep `procedural3d` focused on 3D-ready outputs;
- generate data and recipes, while `world3d` consumes the results;
- avoid heavy systems such as marching cubes, voxel destruction, or realistic city generation in v1.

## What belongs to `procedural3d`

- terrain generation;
- heightfield generation;
- chunk generation;
- scatter of props and entities;
- spawn points and patrol layouts;
- generation recipes for editor/runtime reuse.

## Proposed API

### Base generator state

- `proc3d_create(seed: int) -> result<procedural3d.Proc3DHandle, core.Error>`: creates a 3D procedural generator.
- `proc3d_set_seed(id: procedural3d.Proc3DHandle, seed: int) -> result<void, core.Error>`: changes generator seed.
- `proc3d_get_seed(id: procedural3d.Proc3DHandle) -> int`: reads current seed.
- `proc3d_reset(id: procedural3d.Proc3DHandle) -> result<void, core.Error>`: resets internal generation state.

### Noise

- `noise3d_create(config: procedural.NoiseConfig) -> result<procedural3d.Noise3DHandle, core.Error>`: creates a 3D noise configuration.
- `noise3d_sample(id: procedural3d.Noise3DHandle, position: game.Vector3) -> float`: samples 3D noise at a point.
- `noise2d_sample_height(id: procedural3d.Noise3DHandle, x: float, z: float) -> float`: samples height-style noise.
- `noise3d_set_scale(id: procedural3d.Noise3DHandle, value: float) -> result<void, core.Error>`: sets noise scale.
- `noise3d_set_octaves(id: procedural3d.Noise3DHandle, value: int) -> result<void, core.Error>`: sets octave count.
- `noise3d_set_persistence(id: procedural3d.Noise3DHandle, value: float) -> result<void, core.Error>`: sets persistence.
- `noise3d_set_lacunarity(id: procedural3d.Noise3DHandle, value: float) -> result<void, core.Error>`: sets lacunarity.

### Terrain and heightfield

- `terrain_generate_flat(id: procedural3d.Proc3DHandle, size: game.Vector2) -> procedural3d.HeightField`: generates flat terrain.
- `terrain_generate_hills(id: procedural3d.Proc3DHandle, size: game.Vector2, noise: procedural3d.Noise3DHandle) -> procedural3d.HeightField`: generates hills.
- `terrain_generate_mountains(id: procedural3d.Proc3DHandle, size: game.Vector2, noise: procedural3d.Noise3DHandle) -> procedural3d.HeightField`: generates mountains.
- `terrain_generate_island(id: procedural3d.Proc3DHandle, size: game.Vector2, noise: procedural3d.Noise3DHandle) -> procedural3d.HeightField`: generates a simple island.
- `terrain_generate_plateau(id: procedural3d.Proc3DHandle, size: game.Vector2, noise: procedural3d.Noise3DHandle) -> procedural3d.HeightField`: generates a plateau.
- `terrain_smooth(field: procedural3d.HeightField, iterations: int) -> procedural3d.HeightField`: smooths terrain.
- `terrain_clamp_height(field: procedural3d.HeightField, min: float, max: float) -> procedural3d.HeightField`: clamps terrain heights.
- `terrain_apply_erosion_light(field: procedural3d.HeightField, iterations: int) -> procedural3d.HeightField`: applies light/simple erosion.
- `terrain_to_world3d(field: procedural3d.HeightField, world: world3d.World3D) -> result<void, core.Error>`: applies terrain result to `world3d`.

### Chunk generation

- `chunk_generate_height(id: procedural3d.Proc3DHandle, chunk_coord: world3d.ChunkCoord, noise: procedural3d.Noise3DHandle) -> procedural3d.HeightFieldChunk`: generates terrain data for one chunk.
- `chunk_generate_props(id: procedural3d.Proc3DHandle, chunk_coord: world3d.ChunkCoord, rules: procedural3d.ScatterRules) -> procedural3d.ScatterResult`: generates props for one chunk.
- `chunk_generate_spawn_points(id: procedural3d.Proc3DHandle, chunk_coord: world3d.ChunkCoord, rules: procedural3d.SpawnRules) -> list<procedural3d.SpawnPoint>`: generates spawn points for one chunk.

### Scatter

- `scatter_points(area: game.Bounds3D, count: int, rules: procedural3d.ScatterRules) -> list<game.Vector3>`: distributes points in space.
- `scatter_models(area: game.Bounds3D, model_id: assets3d.ModelHandle, count: int, rules: procedural3d.ScatterRules) -> procedural3d.ScatterResult`: places model instances procedurally.
- `scatter_entities(area: game.Bounds3D, prefab: text, count: int, rules: procedural3d.ScatterRules) -> list<entities.Entity>`: spawns entities procedurally.
- `scatter_on_terrain(field: procedural3d.HeightField, count: int, rules: procedural3d.ScatterRules) -> list<game.Vector3>`: scatters points over terrain.
- `scatter_avoid_radius(points: list<game.Vector3>, radius: float) -> list<game.Vector3>`: filters points to keep distance apart.
- `scatter_by_slope(field: procedural3d.HeightField, max_slope: float, rules: procedural3d.ScatterRules) -> list<game.Vector3>`: selects points by slope.
- `scatter_by_height(field: procedural3d.HeightField, min: float, max: float, rules: procedural3d.ScatterRules) -> list<game.Vector3>`: selects points by height range.

### Layout and routes

- `layout_generate_navpoints(area: game.Bounds3D, count: int) -> list<game.Vector3>`: creates simple navigation points.
- `layout_generate_patrol_route(area: game.Bounds3D, count: int) -> list<game.Vector3>`: creates a patrol route.
- `layout_generate_ring(center: game.Vector3, radius: float, count: int) -> list<game.Vector3>`: creates a ring of points.
- `layout_generate_grid(area: game.Bounds3D, spacing: float) -> list<game.Vector3>`: creates a regular point grid.
- `layout_generate_spawn_ring(center: game.Vector3, radius: float, count: int) -> list<procedural3d.SpawnPoint>`: creates spawn points around a center.

### Recipes and helpers

- `proc3d_preview_height(field: procedural3d.HeightField, x: float, z: float) -> float`: previews terrain height before world apply.
- `proc3d_debug_info(id: procedural3d.Proc3DHandle) -> procedural3d.Proc3DDebugState`: returns debug/editor-facing information.
- `proc3d_bake_recipe(id: procedural3d.Proc3DHandle, recipe: procedural3d.Proc3dRecipe) -> procedural3d.Proc3dResult`: runs a full generation recipe.
- `proc3d_save_recipe(path: text, recipe: procedural3d.Proc3dRecipe) -> result<void, core.Error>`: saves a recipe.
- `proc3d_load_recipe(path: text) -> result<procedural3d.Proc3dRecipe, core.Error>`: loads a recipe.

## Notes

- `procedural3d` should generate data and reusable results.
- `world3d` consumes terrain/chunk output.
- `assets3d` provides models for scatter.
- `ai3d` can consume generated navpoints and patrol routes.

