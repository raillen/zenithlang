# Borealis Module Decision - Audio

- Status: proposed
- Date: 2026-04-22
- Type: module / sound API
- Scope: `borealis.game.audio`

## Summary

Audio handles sound effects, music, volume control, and optional grouping.

## Implementation

- keep sound and music separate in behavior;
- avoid mixing audio rules into movement or UI;
- keep simple one-shot helpers available.

## Proposed API

- `sound_load(path) -> Sound`: loads a short sound effect.
- `sound_play(sound)`: plays a sound.
- `sound_stop(sound)`: stops a sound.
- `sound_set_volume(sound, volume)`: changes sound volume.
- `sound_set_pitch(sound, pitch)`: changes sound pitch.
- `music_load(path) -> Music`: loads music.
- `music_play(music)`: starts music playback.
- `music_stop(music)`: stops music.
- `music_pause(music)`: pauses music.
- `music_resume(music)`: resumes music.
- `music_set_volume(music, volume)`: changes music volume.
- `music_set_loop(music, loop)`: enables or disables looping.
- `audio_set_master_volume(volume)`: changes global volume.
- `audio_is_playing(sound) -> bool`: checks playback state.
- `audio_play_one_shot(path)`: plays a quick sound without setup.
- `audio_group_create(name) -> AudioGroup`: creates a sound group.
- `audio_group_set_volume(group, volume)`: changes group volume.
- `audio_group_pause(group)`: pauses the group.
- `audio_group_resume(group)`: resumes the group.

## Notes

- one-shot helpers are good for gameplay feedback.
- audio groups are useful for menus, combat, and ambience.
