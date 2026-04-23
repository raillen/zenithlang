# Borealis Module Decision - Assets3D

- Status: proposed
- Date: 2026-04-22
- Type: module / 3D asset management API
- Scope: `borealis.game.assets3d`

## Summary

Assets3D manages loading, caching, streaming, and releasing 3D assets.
It keeps the public API simple while handling reuse and memory budget internally.

## Implementation

- keep IDs in the public API for consistency and editor readiness;
- deduplicate assets automatically behind the scenes;
- support both loaded and procedural assets in the same module;
- keep animation clips as data here, with playback owned by `animation3d`.

## Proposed API

### Meshes

- `mesh_load(path: text) -> result<assets3d.MeshId, core.Error>`: loads a mesh from disk.
- `mesh_get(id: assets3d.MeshId) -> optional<assets3d.Mesh>`: reads a loaded mesh.
- `mesh_unload(id: assets3d.MeshId) -> result<void, core.Error>`: unloads a mesh.
- `mesh_create_cube(size: game.Vector3) -> result<assets3d.MeshId, core.Error>`: creates a procedural cube mesh.
- `mesh_create_plane(size: game.Vector2) -> result<assets3d.MeshId, core.Error>`: creates a procedural plane mesh.
- `mesh_create_sphere(radius: float, rings: int, slices: int) -> result<assets3d.MeshId, core.Error>`: creates a procedural sphere mesh.
- `mesh_get_bounds(id: assets3d.MeshId) -> result<game.Bounds3D, core.Error>`: returns mesh bounds.

### Models

- `model_load(path: text) -> result<assets3d.ModelHandle, core.Error>`: loads a full model.
- `model_get(id: assets3d.ModelHandle) -> optional<assets3d.Model>`: reads a loaded model.
- `model_unload(id: assets3d.ModelHandle) -> result<void, core.Error>`: unloads a model.
- `model_set_material(model_id: assets3d.ModelHandle, slot: int, material_id: assets3d.MaterialHandle) -> result<void, core.Error>`: swaps model material slot.
- `model_get_material(model_id: assets3d.ModelHandle, slot: int) -> optional<assets3d.MaterialHandle>`: reads material from a slot.
- `model_mesh_count(model_id: assets3d.ModelHandle) -> int`: returns number of meshes in the model.

### Materials

- `material_create() -> result<assets3d.MaterialHandle, core.Error>`: creates a basic material.
- `material_clone(id: assets3d.MaterialHandle) -> result<assets3d.MaterialHandle, core.Error>`: duplicates a material.
- `material_set_color(id: assets3d.MaterialHandle, color: game.Color) -> result<void, core.Error>`: sets base color.
- `material_set_texture(id: assets3d.MaterialHandle, texture_id: assets3d.TextureId) -> result<void, core.Error>`: sets base texture.
- `material_set_normal_map(id: assets3d.MaterialHandle, texture_id: assets3d.TextureId) -> result<void, core.Error>`: sets a simple normal map.
- `material_set_metallic(id: assets3d.MaterialHandle, value: float) -> result<void, core.Error>`: sets metallic value.
- `material_set_roughness(id: assets3d.MaterialHandle, value: float) -> result<void, core.Error>`: sets roughness value.
- `material_get(id: assets3d.MaterialHandle) -> optional<assets3d.Material>`: reads material state.
- `material_unload(id: assets3d.MaterialHandle) -> result<void, core.Error>`: unloads a material.

### Raw images, textures, and cubemaps

- `image_load(path: text) -> result<assets3d.ImageId, core.Error>`: loads a raw image for heightmaps or texture preprocessing.
- `image_get(id: assets3d.ImageId) -> optional<assets3d.Image>`: reads a raw loaded image.
- `image_unload(id: assets3d.ImageId) -> result<void, core.Error>`: unloads a raw image.

- `texture_load(path: text) -> result<assets3d.TextureId, core.Error>`: loads a 2D texture.
- `texture_get(id: assets3d.TextureId) -> optional<assets3d.Texture>`: reads a loaded texture.
- `texture_unload(id: assets3d.TextureId) -> result<void, core.Error>`: unloads a texture.
- `cubemap_load(path_or_layout: text) -> result<assets3d.CubemapId, core.Error>`: loads a cubemap for skybox or simple reflection.
- `cubemap_get(id: assets3d.CubemapId) -> optional<assets3d.Cubemap>`: reads a loaded cubemap.
- `cubemap_unload(id: assets3d.CubemapId) -> result<void, core.Error>`: unloads a cubemap.

### Animation data

- `anim_clip_load(path: text) -> result<assets3d.AnimClipHandle, core.Error>`: loads a model animation clip.
- `anim_clip_get(id: assets3d.AnimClipHandle) -> optional<assets3d.AnimClip>`: reads a loaded animation clip.
- `anim_clip_unload(id: assets3d.AnimClipHandle) -> result<void, core.Error>`: unloads an animation clip.
- `model_has_skeleton(model_id: assets3d.ModelHandle) -> bool`: checks if model has a skeleton.

### Cache, streaming, and memory

- `asset_exists(id: assets3d.AssetId) -> bool`: checks if an asset exists.
- `asset_ref_inc(id: assets3d.AssetId) -> result<void, core.Error>`: increments asset reference count.
- `asset_ref_dec(id: assets3d.AssetId) -> result<void, core.Error>`: decrements asset reference count.
- `asset_preload(path: text) -> result<void, core.Error>`: schedules early asset loading.
- `asset_stream_request(path: text) -> result<assets3d.StreamRequestId, core.Error>`: requests on-demand streaming.
- `asset_stream_poll(request_id: assets3d.StreamRequestId) -> assets3d.StreamState`: checks streaming state.
- `asset_release_unused() -> result<void, core.Error>`: releases unused assets.
- `asset_memory_stats() -> assets3d.AssetMemoryStats`: returns current asset memory usage.
- `asset_set_memory_budget(mb: int) -> result<void, core.Error>`: sets asset memory budget.
- `asset_get_memory_budget() -> int`: reads asset memory budget.

## Notes

- optimization and streaming policy should follow `packages/borealis/decisions/009-borealis-3d-performance-phasing.md`.
- `render3d` owns drawing, while `assets3d` owns asset lifetime and reuse.
- raw `ImageId` assets are useful for heightmaps and import/preprocessing flows even when the final runtime asset is a texture.

