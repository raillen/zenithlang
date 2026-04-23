# Borealis Module Decision - Core3D

- Status: proposed
- Date: 2026-04-22
- Type: module / runtime foundation
- Scope: `borealis.game.core3d`

## Summary

Core3D is the runtime base for 3D games in Borealis.
It controls loop, frame timing, and game state flow.

## Implementation

- keep onboarding simple: one clear game loop path;
- keep frame semantics explicit (`frame_begin` and `frame_end`);
- separate core runtime from render, physics, and assets;
- keep compatibility with future editor integration.

## Proposed API

- `core3d_start(config) -> result<void, core.Error>`: starts the 3D runtime.
- `core3d_close() -> result<void, core.Error>`: closes the runtime safely.
- `core3d_running() -> bool`: tells if the game loop should continue.
- `core3d_frame_begin() -> result<void, core.Error>`: marks frame start.
- `core3d_frame_end() -> result<void, core.Error>`: marks frame end.
- `core3d_delta() -> float`: returns delta time of the last frame.
- `core3d_fps() -> int`: returns current FPS.
- `core3d_set_target_fps(fps: int) -> result<void, core.Error>`: sets target FPS.
- `state_set(name: text) -> result<void, core.Error>`: switches to a named game state.
- `state_get() -> text`: returns current state name.
- `state_push(name: text) -> result<void, core.Error>`: pushes a state to the stack.
- `state_pop() -> optional<text>`: pops top state and returns its name.

## Notes

- `core3d` owns only runtime flow and time.
- rendering belongs to `render3d`.
- camera belongs to `camera3d`.
- collisions and movement belong to `physics3d`.

