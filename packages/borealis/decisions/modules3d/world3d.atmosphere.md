# Borealis Module Decision - World3D Atmosphere

- Status: proposed
- Date: 2026-04-22
- Type: submodule / world-space atmosphere API
- Scope: `borealis.game.world3d.atmosphere`

## Summary

Atmosphere controls world-space visual climate baseline:
fog, time-of-day, ambient profile, and global wind.

## Implementation

- keep setup simple with clear setters/getters;
- support retro/classic looks (including PS1-style fog);
- avoid expensive realism features in v1.

## Proposed API

### Fog

- `fog_set_enabled(enabled: bool) -> result<void, core.Error>`: turns fog on/off.
- `fog_is_enabled() -> bool`: reads fog state.
- `fog_set_color(color: core.Color) -> result<void, core.Error>`: sets fog color.
- `fog_get_color() -> core.Color`: reads fog color.
- `fog_set_range(start: float, end: float) -> result<void, core.Error>`: sets distance range.
- `fog_get_range() -> world3d.FogRange`: reads fog range.
- `fog_set_mode(mode: world3d.FogMode) -> result<void, core.Error>`: sets fog mode (`linear`, `exp`, `exp2`).
- `fog_get_mode() -> world3d.FogMode`: reads fog mode.
- `fog_set_density(value: float) -> result<void, core.Error>`: sets density for exponential fog.
- `fog_get_density() -> float`: reads fog density.
- `fog_set_height(enabled: bool, base: float, falloff: float) -> result<void, core.Error>`: enables height fog behavior.
- `fog_get_height() -> world3d.HeightFogState`: reads height fog settings.
- `fog_set_dither(enabled: bool) -> result<void, core.Error>`: enables retro dithering.
- `fog_get_dither() -> bool`: reads dithering state.
- `fog_preset_ps1() -> result<void, core.Error>`: applies quick PS1-like preset.
- `fog_get_state() -> world3d.FogState`: reads full fog state.

### Time and ambient profile

- `time_set(hour: float) -> result<void, core.Error>`: sets time of day (`0-24`).
- `time_get() -> float`: reads current time.
- `time_set_cycle(enabled: bool, day_seconds: float) -> result<void, core.Error>`: configures automatic day/night cycle.
- `time_get_cycle() -> world3d.TimeCycleState`: reads cycle config.
- `time_update(dt: float) -> result<void, core.Error>`: updates time progression.
- `atmosphere_set_ambient_profile(profile: world3d.AmbientProfile) -> result<void, core.Error>`: applies ambient profile.
- `atmosphere_get_ambient_profile() -> world3d.AmbientProfile`: reads active profile.
- `atmosphere_blend_to_profile(profile: world3d.AmbientProfile, duration: float) -> result<void, core.Error>`: smooth profile transition.

### Wind

- `wind_set(direction: math.Vector3, strength: float) -> result<void, core.Error>`: sets global wind.
- `wind_get() -> world3d.WindState`: reads global wind.

## Notes

- atmosphere values are world-space, not fullscreen post-processing.
- screen-space filters belong to `borealis.game.postfx`.
