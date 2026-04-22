# Borealis (skeleton)

Local package skeleton for a future ZPM-published 2D framework.

Status:

- API contract drafted;
- backend boundary drafted (`borealis.backend`);
- minimal usage example drafted (`examples/minimal_loop.zt`);
- runtime stub implemented (`zt_borealis_*` in `zenith_rt.c`);
- linker hook implemented via `build.linker_flags`;
- public layering decision accepted: `borealis.game` + `borealis.engine`.

## Why This Is A Package (Not stdlib)

- graphics/game API evolves fast in early cycles;
- stdlib should stay smaller and more stable;
- this package can iterate fast and migrate to ZPM with less churn.

## Folder Layout

```text
packages/borealis/
  decisions/
    001-module-layering-and-naming.md
  zenith.ztproj
  src/
    borealis.zt
    borealis/
      backend.zt
      engine.zt
      game.zt
  examples/
    minimal_loop.zt
```

## Public Surface

- `borealis`:
  - open/close window;
  - frame begin/end;
  - draw rect/text;
  - key input helper.
- `borealis` core types:
  - `WindowConfig`, `Window`, `Color`, `Rect`, `Key`, `Backend`.
- `borealis.backend`:
  - C ABI contract (`zt_borealis_*`) for backend adapters (stub/raylib/etc).

## Layering (Easy vs Advanced)

Accepted decision:

- `packages/borealis/decisions/001-module-layering-and-naming.md`

Module map discussion:

- `packages/borealis/decisions/004-module-map-and-secondary-modules.md`
- `packages/borealis/decisions/005-editor-ready-architecture.md`
- `packages/borealis/decisions/006-borealis-stack.md`
- `packages/borealis/decisions/007-borealis-flow.md`
- `packages/borealis/architecture-summary.md`

Module decisions:

- `packages/borealis/decisions/modules/README.md`

Public layers:

- `borealis.game`: easy-first entrypoint (scaffold in this cut).
- `borealis.engine`: advanced/technical entrypoint (scaffold in this cut).
- `borealis`: current compatibility root.

Imports:

```zt
import borealis.game as game
import borealis.engine as engine
```

## Backend Contract (C ABI)

Expected symbols:

- `zt_borealis_open_window`
- `zt_borealis_close_window`
- `zt_borealis_window_should_close`
- `zt_borealis_begin_frame`
- `zt_borealis_end_frame`
- `zt_borealis_draw_rect`
- `zt_borealis_draw_text`
- `zt_borealis_is_key_down`

Current behavior:

- `Backend.Stub` works as no-op runtime backend (safe for bootstrap);
- non-stub backends return `core.Error` with a clear message if not linked.

## Linker Hook (`zenith.ztproj`)

Use `[build].linker_flags` to inject native linker args.

```toml
[build]
linker_flags = "-lraylib -lopengl32 -lgdi32 -lwinmm"
```

Accepted aliases:

- `linker_flags`
- `link_flags`
- `link`

## Example Dependency (Future ZPM / Local Path)

```toml
[dependencies]
borealis = { path = "../packages/borealis" }
```

Then import:

```zt
import borealis
```
