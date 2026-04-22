# Borealis Module Decision - Animation

- Status: proposed
- Date: 2026-04-22
- Type: module / visual API
- Scope: `borealis.game.animation`

## Summary

Animation handles visual state, frame switching, and optional synchronization with controller state.

## Implementation

- keep animation independent from movement;
- allow optional controller binding through `controllers`, not here;
- keep the first layer simple and frame-based.

## Proposed API

- `animation_create(frames, fps, loop) -> Animation`: creates a frame animation.
- `animation_play(sprite, animation)`: starts an animation.
- `animation_stop(sprite)`: stops the current animation.
- `animation_update(sprite, dt)`: advances the animation.
- `animation_set_speed(sprite, speed)`: changes playback speed.
- `animation_set_frame(sprite, frame)`: jumps to a specific frame.
- `animation_set_loop(sprite, loop)`: turns looping on or off.
- `animation_map_create() -> AnimationMap`: creates an animation map.
- `animation_map_set(map, state, animation)`: maps a state to an animation.
- `animation_sync(sprite, state)`: syncs the sprite with a state.
- `animation_flip(sprite, flip_x, flip_y)`: flips the animation visually.

## Notes

- controller-to-animation binding belongs to `controllers`.
- animation should stay readable and easy to wire.
