# Borealis Module Decision - Procedural

- Status: proposed
- Date: 2026-04-22
- Type: module / generation
- Scope: `borealis.game.procedural`

## Summary

Procedural contains generation algorithms for maps and content.

## Implementation

- keep generation separate from map structure;
- let the world module consume generated data;
- use this module for algorithms, not storage.

## Proposed API

- `map_generate_empty(size) -> Map`: creates an empty map.
- `map_generate_room(size) -> Map`: creates a room layout.
- `map_generate_cave(size) -> Map`: creates a cave-like map.
- `map_generate_dungeon(size) -> Map`: creates a dungeon layout.
- `map_generate_platforms(size) -> Map`: creates platform-style geometry.
- `noise_create(config) -> Noise`: creates noise configuration.
- `noise_sample(noise, position) -> float`: samples noise.
- `random_walk_generate(size, steps) -> Map`: generates with random walk.
- `cellular_automata_generate(size, iterations) -> Map`: generates with automata.

## Notes

- this module is intentionally separate from `world`.
- use it for content creation, not for runtime map editing.
