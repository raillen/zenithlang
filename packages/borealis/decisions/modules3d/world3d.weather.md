# Borealis Module Decision - World3D Weather

- Status: proposed
- Date: 2026-04-22
- Type: submodule / world-space weather API
- Scope: `borealis.game.world3d.weather`

## Summary

Weather provides simple, scalable climate effects:
rain, storm, lightning, and wind presets for gameplay.

## Implementation

- keep weather event-driven and lightweight;
- prioritize easy controls over realistic simulation;
- allow local weather zones and global weather state.

## Proposed API

### Weather state

- `weather_set_enabled(enabled: bool) -> result<void, core.Error>`: enables/disables weather system.
- `weather_is_enabled() -> bool`: reads weather system state.
- `weather_set_kind(kind: world3d.WeatherKind) -> result<void, core.Error>`: sets current weather (`clear`, `rain`, `storm`, `snow`, `foggy`).
- `weather_get_kind() -> world3d.WeatherKind`: reads current weather.
- `weather_set_intensity(value: float) -> result<void, core.Error>`: sets overall intensity (`0-1`).
- `weather_get_intensity() -> float`: reads intensity.
- `weather_set_transition(target: world3d.WeatherKind, duration: float) -> result<void, core.Error>`: smooth weather transition.
- `weather_update(dt: float) -> result<void, core.Error>`: updates weather behavior by frame.

### Rain and snow

- `rain_set_enabled(enabled: bool) -> result<void, core.Error>`: toggles rain emission.
- `rain_set_rate(rate: float) -> result<void, core.Error>`: sets particles/drops rate.
- `rain_get_rate() -> float`: reads rain rate.
- `rain_set_wind_influence(value: float) -> result<void, core.Error>`: applies wind effect on rain direction.
- `snow_set_enabled(enabled: bool) -> result<void, core.Error>`: toggles snow emission.
- `snow_set_rate(rate: float) -> result<void, core.Error>`: sets snow rate.
- `snow_get_rate() -> float`: reads snow rate.

### Storm and lightning

- `storm_set_enabled(enabled: bool) -> result<void, core.Error>`: toggles storm mode.
- `storm_set_strength(value: float) -> result<void, core.Error>`: sets storm strength (`0-1`).
- `storm_get_strength() -> float`: reads storm strength.
- `lightning_trigger() -> result<void, core.Error>`: triggers one lightning strike.
- `lightning_set_interval(min_seconds: float, max_seconds: float) -> result<void, core.Error>`: configures auto lightning interval.
- `lightning_get_interval() -> world3d.LightningInterval`: reads auto lightning interval.
- `lightning_set_flash(color: core.Color, intensity: float, duration: float) -> result<void, core.Error>`: configures flash effect.
- `lightning_get_last_strike_time() -> float`: reads last strike timestamp.

### Weather zones

- `weather_zone_create(bounds: world3d.Bounds3D, kind: world3d.WeatherKind, intensity: float) -> result<world3d.WeatherZoneId, core.Error>`: creates a local weather zone.
- `weather_zone_destroy(id: world3d.WeatherZoneId) -> result<void, core.Error>`: removes a weather zone.
- `weather_zone_set_enabled(id: world3d.WeatherZoneId, enabled: bool) -> result<void, core.Error>`: toggles a weather zone.
- `weather_zone_set_kind(id: world3d.WeatherZoneId, kind: world3d.WeatherKind) -> result<void, core.Error>`: sets zone weather kind.
- `weather_zone_set_intensity(id: world3d.WeatherZoneId, intensity: float) -> result<void, core.Error>`: sets zone intensity.
- `weather_zone_get(id: world3d.WeatherZoneId) -> optional<world3d.WeatherZone>`: reads zone data.

### Utility presets

- `weather_preset_clear() -> result<void, core.Error>`: applies clear weather quickly.
- `weather_preset_rain_light() -> result<void, core.Error>`: applies light rain.
- `weather_preset_storm_heavy() -> result<void, core.Error>`: applies heavy storm.

## Notes

- weather can drive audio, lights, and fog through module integration.
- realistic volumetric clouds are out of scope for v1.
- localized fog-like cloud behavior can be achieved with fog zones and weather zones.
