# Borealis Architecture Summary

## Goal

One compact reference for the Borealis runtime, module boundaries, and editor-ready structure.

## Summary Table

| Module | Purpose | Includes | Excludes / Notes |
|---|---|---|---|
| `game` | Easy layer entrypoint | simple game API, defaults | no backend/linker burden |
| `engine` | Technical layer | advanced control, backend-facing APIs | not the beginner path |
| `entities` | Gameplay-facing ECS bridge | create/destroy, tags, hierarchy, snapshot, inspect | logic lives in systems/controllers |
| `movement` | Generic motion helpers | move, push, teleport, basic motion math | not controllers, not animation |
| `controllers` | Ready-made control schemes | platformer, topdown, grid, twin-stick, visual bridge | owns optional sprite sync |
| `vehicles` | Vehicle-specific controllers | car controls and future vehicle families | not generic movement |
| `animation` | Visual state and frames | play/update/mapping/flip | controller bridge stays in `controllers` |
| `audio` | Sound and music | sound/music playback, volume, groups | no gameplay logic |
| `ai` | Behavior and perception | chase, evade, patrol, sensors, FSM/BT | not movement core |
| `camera` | View control | follow, bounds, shake, framing | not player movement |
| `input` | Raw input and actions | keys, mouse, gamepad, contexts, hover | not gameplay behavior |
| `world` | Tile/world structure | tiles, collision data, pathing, tilesets | generation stays in `procedural` |
| `procedural` | Generic generation | noise, seeds, pipelines, map/content generation | not runtime storage |
| `ui` | General interface widgets | buttons, panels, inputs, layout | `hud` stays as namespace here |
| `hud` | In-game overlay namespace | health, ammo, score, timers | lives inside `ui` |
| `assets` | Resource access | load/get/unload/cache, textures, fonts, sounds | not render or gameplay behavior |
| `save` | Game progress snapshots | slots, autosave, snapshots, restore | can use `storage` under the hood |
| `storage` | Generic persistence | text/json/binary IO, copy/move/list | base for `save` and cache-like flows |
| `database` | Future DB backend | SQLite-style open/query/transaction | future-facing, not blocking |
| `services` | Remote capabilities | network, cloud save, APIs, matchmaking | umbrella for remote features |
| `settings` | User configuration | resolution, audio, language, accessibility | can persist via `save` or `storage` |
| `scene` | Flow and screen organization | enter/exit/update/draw/transitions | does not replace gameplay model |
| `events` | Decoupled messaging | emit/on/off/queue/dispatch | lightweight, not a huge bus |
| `debug` | Diagnostics and overlays | bounds, hitboxes, FPS, watches | optional and easy to strip |

## Editor Rules

1. Entities need stable IDs.
2. Scenes are documents.
3. Editor metadata must stay outside gameplay save data.
4. The editor reads and edits the same runtime model.
5. Undo/redo should be command-based.

## Notes

- `save` is for game state snapshots.
- `storage` is for generic persistence.
- `database` is for future SQLite-like backends.
- `services` is for network and remote services.
- `procedural` is separate from `world`.
- `hud` is a namespace under `ui`.
