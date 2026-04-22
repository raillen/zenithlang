# Decision 004 - Module Map And Secondary Modules

- Status: proposed
- Date: 2026-04-22
- Type: architecture / module design / API organization
- Scope: `borealis.game` module map, secondary modules, `ui` and `hud`

## Summary

Define a clearer module map for Borealis so the easy layer stays organized as it grows.

This decision discusses which concerns should live in their own modules and which ones should remain as namespaces or helpers.

Module-specific decisions live under:

- `packages/borealis/decisions/modules/README.md`

## Decision

Adopt this module map as the current proposal for `borealis.game`:

1. `movement` - generic movement helpers.
2. `controllers` - ready-made control schemes.
3. `vehicles` - vehicle-specific control and behavior.
4. `animation` - sprite animation and visual state.
5. `audio` - sounds, music, volume, playback.
6. `ai` - decision making, pursuit, evasion, perception.
7. `camera` - camera control and framing.
8. `input` - raw input, actions, contexts, device state.
9. `entities` - gameplay-facing entity layer over ECS.
10. `world` - map, tiles, collision, pathing, tile metadata.
11. `procedural` - map/content generation algorithms.
12. `ui` - widgets, layout, panels, text input, interaction.
13. `debug` - debug overlays, helpers, diagnostics.
14. `scene` - scene/layer flow and screen organization.
15. `events` - event dispatch and gameplay notifications.
16. `assets` - asset loading, references, caches.
17. `save` - persistence of game progress and serializable snapshots.
18. `storage` - generic persistence for files, blobs, and structured data.
19. `database` - future database integrations such as SQLite.
20. `services` - network, cloud save, API calls, matchmaking, remote sessions.
21. `settings` - user/game configuration.

Namespace rule:

- `hud` lives under `ui` as a namespace, not as a top-level module.
- Example: `borealis.game.ui.hud`.

## Rationale

This split keeps the easy layer readable and reduces the chance of a single giant module.

It also makes the module list closer to how developers think about game work:

- movement and control;
- visuals and audio;
- world and generation;
- UI and HUD;
- persistence and settings;
- scene flow and debug.

## UI And HUD Boundary

Use `ui` for all general interface elements:

- buttons;
- labels;
- sliders;
- panels;
- inputs;
- popups;
- menus;
- inventories.

Use `hud` only for in-game overlay elements:

- health bars;
- ammo counters;
- minimap;
- score;
- timers;
- status indicators.

## Secondary Module Notes

1. `assets` should stay about acquisition and access, not gameplay logic.
2. `save` should focus on game progress and state snapshots, not general storage.
3. `storage` should handle generic persistence and file/blob helpers.
4. `database` should stay reserved for actual database backends.
5. `services` should cover remote capabilities without becoming a giant SDK wrapper.
6. `settings` should keep user preferences small and predictable.
7. `debug` should stay optional and easy to strip from release builds.
8. `scene` should help structure flow, not replace the gameplay model.
9. `events` should keep decoupling simple, not become a full enterprise event system.

## Non-Canonical Forms

1. turning `hud` into a top-level public module instead of a `ui` namespace;
2. putting generation algorithms inside `world` instead of `procedural`;
3. merging UI widgets, save data, and assets into one catch-all module;
4. putting animation or audio inside movement.

## Out Of Scope

This decision does not finalize:

1. the exact public names of every function in each module;
2. whether `scene` becomes `scene` or `scenes`;
3. whether `events` stays public or becomes an internal helper layer later.
