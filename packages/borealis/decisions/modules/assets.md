# Borealis Module Decision - Assets

- Status: proposed
- Date: 2026-04-22
- Type: module / asset loading
- Scope: `borealis.game.assets`

## Summary

Assets handles loading and access to textures, sounds, fonts, and other resources.

## Implementation

- keep loading and lookup simple;
- avoid gameplay logic here;
- expose clear asset handles.

## Proposed API

- `asset_load(path) -> Asset`: loads an asset.
- `asset_get(name) -> Asset`: fetches a loaded asset.
- `asset_unload(asset)`: unloads an asset.
- `asset_cache_clear()`: clears cached assets.
- `texture_load(path) -> Texture`: loads a texture.
- `font_load(path) -> Font`: loads a font.
- `shader_load(vs, fs) -> Shader`: loads a shader.
- `sound_load(path) -> Sound`: loads a sound.

## Notes

- this module is about resource access, not render behavior.
- assets can build on storage and the runtime backend under the hood.
