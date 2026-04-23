# Borealis Module Decision - Audio3D

- Status: proposed
- Date: 2026-04-22
- Type: module / spatial audio API
- Scope: `borealis.game.audio3d`

## Summary

Audio3D provides simple spatial audio for classic 3D games.
It focuses on positional playback, listener control, attenuation, and easy attachment to entities.

## Implementation

- keep sound and music behavior separated, like in 2D audio;
- keep spatial sound simple and gameplay-oriented, not studio-grade simulation;
- support one-shot spatial playback and bound audio emitters;
- use entity binding and listener helpers for camera/player-driven audio.

## Proposed API

### Listener

- `audio3d_set_listener_position(position: game.Vector3) -> result<void, core.Error>`: sets listener position.
- `audio3d_get_listener_position() -> game.Vector3`: reads listener position.
- `audio3d_set_listener_forward(direction: game.Vector3) -> result<void, core.Error>`: sets listener facing direction.
- `audio3d_get_listener_forward() -> game.Vector3`: reads listener facing direction.
- `audio3d_set_listener_up(direction: game.Vector3) -> result<void, core.Error>`: sets listener up vector.
- `audio3d_get_listener_up() -> game.Vector3`: reads listener up vector.
- `audio3d_bind_listener_entity(entity: entities.Entity, offset: game.Vector3) -> result<void, core.Error>`: binds listener to an entity.
- `audio3d_unbind_listener_entity() -> result<void, core.Error>`: clears listener entity binding.

### Spatial sounds

- `sound3d_load(path: text) -> result<audio3d.Sound3dId, core.Error>`: loads a spatial sound effect.
- `sound3d_unload(id: audio3d.Sound3dId) -> result<void, core.Error>`: unloads a spatial sound effect.
- `sound3d_play(id: audio3d.Sound3dId, position: game.Vector3) -> result<audio3d.InstanceId, core.Error>`: plays a sound at a world position.
- `sound3d_play_one_shot(path: text, position: game.Vector3) -> result<audio3d.InstanceId, core.Error>`: plays a quick one-shot spatial sound.
- `sound3d_stop(instance_id: audio3d.InstanceId) -> result<void, core.Error>`: stops one playing sound instance.
- `sound3d_pause(instance_id: audio3d.InstanceId) -> result<void, core.Error>`: pauses one instance.
- `sound3d_resume(instance_id: audio3d.InstanceId) -> result<void, core.Error>`: resumes one instance.
- `sound3d_is_playing(instance_id: audio3d.InstanceId) -> bool`: checks one instance playback state.

### Spatial properties

- `sound3d_set_position(instance_id: audio3d.InstanceId, position: game.Vector3) -> result<void, core.Error>`: moves a playing sound.
- `sound3d_get_position(instance_id: audio3d.InstanceId) -> result<game.Vector3, core.Error>`: reads sound position.
- `sound3d_set_volume(instance_id: audio3d.InstanceId, volume: float) -> result<void, core.Error>`: sets instance volume.
- `sound3d_get_volume(instance_id: audio3d.InstanceId) -> result<float, core.Error>`: reads instance volume.
- `sound3d_set_pitch(instance_id: audio3d.InstanceId, pitch: float) -> result<void, core.Error>`: sets instance pitch.
- `sound3d_get_pitch(instance_id: audio3d.InstanceId) -> result<float, core.Error>`: reads instance pitch.
- `sound3d_set_min_distance(instance_id: audio3d.InstanceId, value: float) -> result<void, core.Error>`: sets the near attenuation distance.
- `sound3d_get_min_distance(instance_id: audio3d.InstanceId) -> result<float, core.Error>`: reads near attenuation distance.
- `sound3d_set_max_distance(instance_id: audio3d.InstanceId, value: float) -> result<void, core.Error>`: sets the far attenuation distance.
- `sound3d_get_max_distance(instance_id: audio3d.InstanceId) -> result<float, core.Error>`: reads far attenuation distance.
- `sound3d_set_rolloff(instance_id: audio3d.InstanceId, value: float) -> result<void, core.Error>`: sets falloff strength.
- `sound3d_get_rolloff(instance_id: audio3d.InstanceId) -> result<float, core.Error>`: reads falloff strength.

### Entity binding

- `sound3d_bind_entity(instance_id: audio3d.InstanceId, entity: entities.Entity, offset: game.Vector3) -> result<void, core.Error>`: binds a playing sound to an entity.
- `sound3d_get_entity(instance_id: audio3d.InstanceId) -> optional<entities.Entity>`: reads the bound entity.
- `sound3d_unbind_entity(instance_id: audio3d.InstanceId) -> result<void, core.Error>`: clears entity binding.

### Spatial music and ambience

- `music3d_load(path: text) -> result<audio3d.Music3dId, core.Error>`: loads spatial ambience or music.
- `music3d_unload(id: audio3d.Music3dId) -> result<void, core.Error>`: unloads spatial music.
- `music3d_play(id: audio3d.Music3dId, position: game.Vector3) -> result<audio3d.InstanceId, core.Error>`: plays spatial ambience/music at a point.
- `music3d_set_loop(id: audio3d.Music3dId, enabled: bool) -> result<void, core.Error>`: configures looping.

### Groups and helpers

- `audio3d_group_create(name: text) -> result<audio3d.Audio3dGroupId, core.Error>`: creates a spatial audio group.
- `audio3d_group_set_volume(group_id: audio3d.Audio3dGroupId, volume: float) -> result<void, core.Error>`: sets group volume.
- `audio3d_group_pause(group_id: audio3d.Audio3dGroupId) -> result<void, core.Error>`: pauses a group.
- `audio3d_group_resume(group_id: audio3d.Audio3dGroupId) -> result<void, core.Error>`: resumes a group.
- `audio3d_update(dt: float) -> result<void, core.Error>`: updates listener and bound spatial sounds each frame.
- `audio3d_debug_info(instance_id: audio3d.InstanceId) -> audio3d.Audio3dDebugInfo`: returns debug/editor-friendly state.

## Notes

- `audio3d` should stay simple and useful for gameplay first.
- advanced reverb zones and complex sound propagation can remain future work or engine-level features.

