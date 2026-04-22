# Borealis Module Decision - World

- Status: proposed
- Date: 2026-04-22
- Type: module / world and tiles
- Scope: `borealis.game.world`

## Summary

World handles tilemaps, tilesets, collision data, pathing, and map editing helpers.

## Implementation

- keep world structure separate from procedural generation;
- allow edit-friendly setters for tiles and metadata;
- keep tile indexing simple and predictable.

## Proposed API

- `map_load(file) -> Map`: loads a map.
- `map_draw(map)`: draws a map.
- `map_set_tile(map, x, y, tile)`: sets a tile.
- `map_get_tile(map, x, y) -> Tile`: reads a tile.
- `map_set_tile_index(map, x, y, index)`: sets a tile by index.
- `map_get_tile_index(map, x, y) -> int`: reads a tile index.
- `map_is_solid(map, x, y) -> bool`: checks blocking state.
- `map_is_walkable(map, x, y) -> bool`: checks walkable state.
- `map_region(map, rect) -> Tile[]`: returns tiles in a region.
- `tile_at(map, x, y)`: gets tile by grid coordinate.
- `world_to_tile(position)`: converts world to tile coordinate.
- `tile_to_world(tile)`: converts tile coordinate to world position.
- `tile_neighbors(map, x, y) -> Tile[]`: gets neighbor tiles.
- `tile_set_solid(tile, bool)`: sets solidity.
- `tile_set_walkable(tile, bool)`: sets walkability.
- `tile_set_index(tile, index)`: sets tile index.
- `tileset_load(path) -> Tileset`: loads a tileset.
- `tileset_get_tile(tileset, index) -> Tile`: reads a tile from a tileset.
- `tileset_tile_count(tileset) -> int`: counts tiles.
- `map_auto_tile(map)`: auto-tiles a map.
- `map_build_collision(map)`: builds collision data.
- `map_find_path(map, start, goal) -> Path`: finds a path.
- `map_spawn_entities(map)`: spawns entities from map markers.
- `map_set_spawn_point(map, point)`: sets spawn point.
- `map_get_spawn_point(map) -> Point2`: reads spawn point.
- `map_fill_rect(map, rect, tile)`: fills a region.
- `map_clear(map)`: clears a map.
- `map_clone(map) -> Map`: clones a map.
- `tile_is_empty(tile) -> bool`: checks empty state.
- `tile_is_blocking(tile) -> bool`: checks blocking state.
- `map_debug_draw(map)`: draws debug info.

## Notes

- pathing belongs here, not in procedural.
- generation algorithms must live elsewhere.
