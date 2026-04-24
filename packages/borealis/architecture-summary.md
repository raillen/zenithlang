# Borealis Architecture Summary

## Goal

One compact reference for the Borealis runtime, module boundaries, and editor-ready structure.

## Summary Table

| Module | Purpose | Includes | Excludes / Notes |
|---|---|---|---|
| `game` | Easy layer entrypoint | simple game API, defaults | no backend/linker burden |
| `engine` | Technical layer | advanced control, backend-facing APIs | not the beginner path |
| `contracts` | Shared gameplay contracts | frame/input/body data structs and helpers | no rendering/audio logic |
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
| `ui` | General interface widgets | buttons, panels, inputs, focus, hover/click, layout | `hud` stays as namespace here |
| `hud` | In-game overlay namespace | health, ammo, score, text/value widgets, position, visibility | lives inside `ui` |
| `assets` | Resource access | load/get/unload/cache, typed loaders, stable ids, source metadata | not render or gameplay behavior |
| `save` | Game progress snapshots | slots, autosave, snapshots, restore | can use `storage` under the hood |
| `storage` | Generic persistence | text/json/binary IO, copy/move/list | base for `save` and cache-like flows |
| `database` | Future DB backend | SQLite-style open/query/transaction | future-facing, not blocking |
| `services` | Remote capabilities | network, cloud save, APIs, matchmaking | umbrella for remote features |
| `settings` | User configuration | resolution, audio, language, accessibility | can persist via `save` or `storage` |
| `scene` | Flow and screen organization | enter/exit/update/draw/transitions, phase, document id, stack | does not replace gameplay model |
| `events` | Decoupled messaging | emit/on/off/queue/dispatch | lightweight, not a huge bus |
| `debug` | Diagnostics and overlays | bounds, hitboxes, FPS, watches | optional and easy to strip |
| `editor` | Editor metadata bridge | labels, notes, grouping, lock/hidden flags by stable id | tool-facing metadata only |

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

## 3D Design Track

The current Borealis 1.0 cycle also has a documented 3D design track.
This track is architectural only for now and does not change the runtime scope of Borealis 1.0.

Core 3D modules:

- `core3d`
- `render3d`
- `camera3d`
- `physics3d`
- `world3d`

Gameplay-facing 3D helpers:

- `assets3d`
- `animation3d`
- `audio3d`
- `entities3d`
- `controllers3d`
- `ai3d`
- `ui3d`
- `procedural3d`
- `debug3d`
- `postfx`
- `settings.video`

Shared modules remain shared across 2D and 3D:

- `scene`
- `save`
- `events`
- `services`
- `storage`
- `database`
- `input`
- `settings`

## Backend Hook (B7)

Desktop backend integration now follows an adapter contract in runtime C:

1. `zt_borealis_desktop_api` defines desktop operations.
2. `zt_borealis_set_desktop_api(...)` registers a desktop adapter.
3. Runtime now ships an initial Raylib adapter loaded dynamically when the library exists.
4. If no desktop adapter is registered/available, `backend_id=1` safely falls back to stub.

Canonical profile doc:

- `packages/borealis/backend-desktop-linker-profile-v1.md`.

## Dependency Map (B6)

Allowed direct dependencies in easy layer:

1. `contracts` can depend only on `game` primitives.
2. `movement`, `controllers`, `ai`, `camera`, `input` can depend on `contracts`.
3. `ui.hud` depends on `ui` only by namespace convention (light coupling).
4. `save` can use `storage` later, but each module remains usable standalone.
5. `editor` remains isolated from runtime draw/audio/backend modules.

Canonical file: `packages/borealis/dependency-map-v1.md`.

## ECS Hybrid Guidance (B5)

Use `borealis.game` first when:

1. You are prototyping quickly.
2. You only need entity lifecycle, tags, hierarchy, and simple scene flow.
3. You want the lowest cognitive load.

Move to `borealis.engine.ecs` when:

1. You need explicit component payload ownership.
2. You need system lifecycle tracking (`update`/`draw`) and run counters.
3. You are preparing editor tooling, automation, or larger project organization.

Recommended path:

1. Start entities in `borealis.game.entities`.
2. Add components through the facade (`entity_add_component` and related).
3. Promote orchestration to `borealis.engine.ecs` systems as complexity grows.
