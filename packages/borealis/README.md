# Borealis (skeleton)

Local package skeleton for a future ZPM-published 2D framework.

Status:

- API contract drafted;
- backend boundary drafted (`borealis.backend`);
- minimal usage example drafted (`examples/minimal_loop.zt`).

## Why This Is A Package (Not stdlib)

- graphics/game API evolves fast in early cycles;
- stdlib should stay smaller and more stable;
- this package can iterate fast and migrate to ZPM with less churn.

## Folder Layout

```text
packages/borealis/
  zenith.ztproj
  src/
    borealis.zt
    borealis/
      backend.zt
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

Current note:

- this repo still needs linker extensibility for external native libs;
- this package is ready at API level while build/link integration lands.

## Example Dependency (Future ZPM / Local Path)

```toml
[dependencies]
borealis = { path = "../packages/borealis" }
```

Then import:

```zt
import borealis
```
