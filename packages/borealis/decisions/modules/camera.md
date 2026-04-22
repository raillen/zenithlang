# Borealis Module Decision - Camera

- Status: proposed
- Date: 2026-04-22
- Type: module / view control
- Scope: `borealis.game.camera`

## Summary

Camera handles world view, framing, following, and screen-space conversion.

## Implementation

- keep camera movement separate from player movement;
- prefer explicit helpers for follow, bounds, and framing;
- support shake and look-ahead as optional quality helpers.

## Proposed API

- `camera_set_position(position)`: sets the camera position.
- `camera_set_zoom(value)`: sets the zoom.
- `world_to_screen(position) -> Vector2`: converts world to screen.
- `screen_to_world(position) -> Vector2`: converts screen to world.
- `camera_follow(target, smooth)`: follows a target smoothly.
- `camera_set_bounds(rect)`: clamps the camera to an area.
- `camera_lerp_to(target, speed)`: smoothly moves toward a target.
- `camera_center_on(target)`: centers on a target.
- `camera_set_offset(offset)`: applies a fixed offset.
- `camera_lock_to(target)`: locks the camera to a target.
- `camera_rig_create(config) -> CameraRig`: creates a full camera rig.
- `camera_shake(intensity, duration)`: shakes the camera.
- `camera_set_deadzone(rect)`: sets a dead zone.
- `camera_set_look_ahead(offset)`: adds look-ahead.
- `camera_frame_targets(entities[])`: frames multiple targets.
- `camera_set_focus(target)`: changes focus target.
- `camera_reset()`: resets camera state.

## Notes

- camera is a strong candidate for an easy-layer module.
- framing helpers are very useful for polished gameplay.
