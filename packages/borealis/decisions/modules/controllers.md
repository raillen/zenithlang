# Borealis Module Decision - Controllers

- Status: proposed
- Date: 2026-04-22
- Type: module / gameplay API / input-to-motion
- Scope: `borealis.game.controllers`

## Summary

Controllers are ready-made control schemes that turn input into gameplay motion.

## Implementation

- controllers should own state and per-frame behavior;
- visual syncing with sprites is optional and belongs here;
- controllers must stay separate from raw movement and from animation.

## Proposed API

- `controller_create(config) -> Controller`: creates a generic controller.
- `controller_update(controller, input, dt)`: updates the controller each frame.
- `controller_set_enabled(controller, bool)`: enables or disables the controller.
- `controller_reset(controller)`: resets controller state.
- `controller_bind_sprite(controller, sprite)`: optionally links a sprite.
- `controller_unbind_sprite(controller)`: removes the sprite link.
- `controller_sync_sprite(controller)`: syncs state to the visual layer.

## Controller Families

- `platform_create(config) -> Controller`: platform-style control.
- `topdown_create(config) -> Controller`: top-down control.
- `four_dir_create(config) -> Controller`: 4-direction movement.
- `eight_dir_create(config) -> Controller`: 8-direction movement.
- `grid_create(config) -> Controller`: tile/grid step control.
- `twin_stick_create(config) -> Controller`: move and aim separately.
- `flight_create(config) -> Controller`: free flying or hovering.
- `rail_create(config) -> Controller`: path/rail guided control.

## Notes

- `controllers` are a high-level convenience layer.
- sprite binding is optional, not mandatory.
- `animation` should consume controller state, not own the controller bridge.
