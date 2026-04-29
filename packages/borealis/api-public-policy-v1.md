# Borealis Public API Policy v1

This file defines what may be public in `packages/borealis`.

The goal is simple: users should see game concepts first, not storage maps,
counters, cache cursors, or implementation helpers.

## Public API

Public API is allowed when the name is useful to a game developer:

- create, load, start, update, draw, play, stop, save, read, write;
- entity, scene, asset, input, audio, camera, world, render, debug;
- stable structs, enums, and handles used by examples or projects.

Public names should read as domain actions.

Good examples:

- `game.start`
- `entities.entity_create`
- `scene.scene_push`
- `assets.asset_load_image`
- `render3d.draw_cube`
- `camera3d.camera3d_create`

## Advanced API

Advanced API may be public when it exposes a real lower-level contract:

- `borealis.engine`
- `borealis.raylib`
- native availability probes;
- explicit handles needed by runtime-backed features.

Advanced API must still avoid leaking cache maps and counters.

## Internal Helpers

These must stay private unless there is a written exception:

- module state, maps, counters, cursors, and registries;
- helpers ending in `_or`;
- map-copy helpers ending in `_set` when the first argument is `values: map`;
- `require_*` validation helpers;
- `normalize_*` helpers;
- `*_internal` helpers;
- storage key builders such as `profile_value_key`.

Private helpers may still be called with a qualified self-prefix inside the same
module. Example: code inside `borealis.game.entities` may call
`entities.entities_bool_or(...)`, but external modules must not use it.

## Test-Only Surface

Large integration checks belong in fixtures, not in onboarding examples.

Use:

- `tests/behavior/*` for compiler/runtime gates;
- `packages/borealis/fixtures/*` for package-level integration samples;
- `packages/borealis/examples/*` for short learning examples.

## Validation

Run this before changing the package contract:

```text
python tools/validate_borealis_contracts.py
./zt.exe check packages/borealis/zenith.ztproj --all
```

The validator checks:

- no `public var` in `packages/borealis/src/borealis`;
- no public internal-helper patterns listed above;
- scene v2 files include `environment`, `render`, and `audio`;
- scene component kinds exist in `borealis.editor.json`.
