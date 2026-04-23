# Borealis Module Decision - Animation3D

- Status: proposed
- Date: 2026-04-22
- Type: module / 3D animation playback API
- Scope: `borealis.game.animation3d`

## Summary

Animation3D plays model animations in a simple and predictable way.
It separates animation data loading from runtime playback.

## Implementation

- keep `assets3d` responsible for loading clips and skeleton data;
- keep `animation3d` responsible for playback, blending, and events;
- use `AnimatorId` in the public API for consistency and editor readiness;
- start with simple clip blending and clear runtime errors.

## Proposed API

### Animator lifecycle

- `animator_create(model_id: assets3d.ModelHandle) -> result<animation3d.AnimatorId, core.Error>`: creates an animator for a model.
- `animator_destroy(id: animation3d.AnimatorId) -> result<void, core.Error>`: removes an animator.
- `animator_exists(id: animation3d.AnimatorId) -> bool`: checks if an animator exists.
- `animator_update(id: animation3d.AnimatorId, dt: float) -> result<void, core.Error>`: updates playback for the frame.

### Playback

- `animator_play(id: animation3d.AnimatorId, clip_id: assets3d.AnimClipHandle) -> result<void, core.Error>`: starts playing a clip.
- `animator_stop(id: animation3d.AnimatorId) -> result<void, core.Error>`: stops the current clip.
- `animator_pause(id: animation3d.AnimatorId) -> result<void, core.Error>`: pauses playback.
- `animator_resume(id: animation3d.AnimatorId) -> result<void, core.Error>`: resumes playback.
- `animator_is_playing(id: animation3d.AnimatorId) -> bool`: checks whether a clip is playing.
- `animator_get_clip(id: animation3d.AnimatorId) -> optional<assets3d.AnimClipHandle>`: returns the current clip.

### Time and looping

- `animator_set_speed(id: animation3d.AnimatorId, value: float) -> result<void, core.Error>`: sets playback speed.
- `animator_get_speed(id: animation3d.AnimatorId) -> float`: reads playback speed.
- `animator_set_loop(id: animation3d.AnimatorId, enabled: bool) -> result<void, core.Error>`: toggles loop mode.
- `animator_get_loop(id: animation3d.AnimatorId) -> bool`: reads loop mode.
- `animator_seek(id: animation3d.AnimatorId, time: float) -> result<void, core.Error>`: jumps to a playback time.
- `animator_get_time(id: animation3d.AnimatorId) -> float`: reads current playback time.
- `animator_get_length(id: animation3d.AnimatorId) -> float`: returns clip duration.

### Blending and state

- `animator_blend_to(id: animation3d.AnimatorId, clip_id: assets3d.AnimClipHandle, duration: float) -> result<void, core.Error>`: blends to another clip.
- `animator_set_default(id: animation3d.AnimatorId, clip_id: assets3d.AnimClipHandle) -> result<void, core.Error>`: sets default clip.
- `animator_get_default(id: animation3d.AnimatorId) -> optional<assets3d.AnimClipHandle>`: reads default clip.
- `animator_queue(id: animation3d.AnimatorId, clip_id: assets3d.AnimClipHandle) -> result<void, core.Error>`: queues the next clip.
- `animator_clear_queue(id: animation3d.AnimatorId) -> result<void, core.Error>`: clears playback queue.

### Events

- `animator_add_event(id: animation3d.AnimatorId, clip_id: assets3d.AnimClipHandle, time: float, name: text) -> result<void, core.Error>`: adds an event to a clip timeline.
- `animator_poll_event(id: animation3d.AnimatorId) -> optional<animation3d.AnimEvent>`: returns the next fired event.
- `animator_clear_events(id: animation3d.AnimatorId) -> result<void, core.Error>`: clears registered events.

### Helpers

- `animator_play_if_not_current(id: animation3d.AnimatorId, clip_id: assets3d.AnimClipHandle) -> result<void, core.Error>`: plays a clip only if it is not already active.
- `animator_restart(id: animation3d.AnimatorId) -> result<void, core.Error>`: restarts the current clip.
- `animator_is_finished(id: animation3d.AnimatorId) -> bool`: checks whether the current clip finished.
- `animator_bind_entity(id: animation3d.AnimatorId, entity: entities.Entity) -> result<void, core.Error>`: binds animator ownership to an entity.
- `animator_debug_info(id: animation3d.AnimatorId) -> animation3d.AnimatorDebugInfo`: returns editor/debug-friendly state.

## Notes

- models without skeleton support should return a clear runtime error.
- advanced animation trees and state machines can stay in `borealis.engine` or a future higher-level module.

