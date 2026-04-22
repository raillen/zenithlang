# Borealis Module Decision - Debug

- Status: proposed
- Date: 2026-04-22
- Type: module / diagnostics
- Scope: `borealis.game.debug`

## Summary

Debug contains optional overlays, visual helpers, and diagnostics.

## Implementation

- keep it easy to disable in release builds;
- keep the helpers readable;
- avoid mixing debug visuals into production gameplay modules.

## Proposed API

- `debug_draw_grid()`: draws a grid overlay.
- `debug_draw_bounds(entity)`: draws entity bounds.
- `debug_draw_hitbox(entity)`: draws hitboxes.
- `debug_draw_fps()`: draws FPS.
- `debug_log(message)`: prints a debug message.
- `debug_watch(name, value)`: tracks a value.
- `debug_toggle(name)`: toggles a debug flag.

## Notes

- debug should stay optional and cheap to remove.
