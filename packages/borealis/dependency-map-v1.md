# Borealis Dependency Map v1

## Goal

Define safe dependency directions for `borealis.game` modules.

Rule: keep dependencies predictable to support editor integration and low coupling.

## Allowed Dependencies

| Module | Allowed to depend on |
|---|---|
| `game` | `backend` only |
| `contracts` | `game` primitives only |
| `entities` | `game`, `engine.ecs` |
| `scene` | `entities` |
| `movement` | `game`, `contracts` |
| `controllers` | `game`, `contracts`, `movement` |
| `vehicles` | none (pure logic) |
| `animation` | none (pure state) |
| `audio` | none (stub + state) |
| `ai` | `game`, `contracts`, `movement` |
| `camera` | `game`, `contracts` |
| `input` | `game`, `contracts` |
| `world` | none (tile flags/state) |
| `procedural` | none (seed/random utilities) |
| `ui` | `game` |
| `ui.hud` | none (pure state) |
| `assets` | none (handles + stubs) |
| `save` | none (slot state) |
| `storage` | none (key-value state) |
| `database` | none (stub contract) |
| `services` | none (stub contract) |
| `settings` | none (state contract) |
| `events` | none (message state) |
| `debug` | `game` |
| `editor` | none (tool metadata) |

## Forbidden Directions (v1)

1. `game` should not depend on gameplay subsystems.
2. `editor` should not depend on rendering/input/backend modules.
3. `entities` should not depend on `ui`, `audio`, `services`, or `database`.
4. `world` and `procedural` should stay separate.
5. `services` should not be required by core offline gameplay flow.
