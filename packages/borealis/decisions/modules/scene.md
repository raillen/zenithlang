# Borealis Module Decision - Scene

- Status: proposed
- Date: 2026-04-22
- Type: module / flow control
- Scope: `borealis.game.scene`

## Summary

Scene handles screen flow and high-level organization of game states.

## Implementation

- keep scene transitions simple;
- use it to organize menus, gameplay, pause, and screens;
- do not let scene become a full engine inside the engine.

## Proposed API

- `scene_create(name) -> Scene`: creates a scene.
- `scene_add_entity(scene, entity)`: adds an entity.
- `scene_remove_entity(scene, entity)`: removes an entity.
- `scene_update(scene, dt)`: updates the scene.
- `scene_draw(scene)`: draws the scene.
- `scene_enter(scene)`: enters a scene.
- `scene_exit(scene)`: exits a scene.
- `scene_transition(from, to)`: changes between scenes.

## Notes

- scene should help structure flow, not replace gameplay architecture.
