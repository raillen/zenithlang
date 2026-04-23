# Borealis Module Decision - Settings Video

- Status: proposed
- Date: 2026-04-22
- Type: submodule / video configuration
- Scope: `borealis.game.settings.video`

## Summary

Settings Video controls display and render output options.
It owns resolution, fullscreen, VSync, anti-aliasing mode, render scale, and quality profile.

## Implementation

- keep video settings separate from artistic post-processing;
- keep VSync and MSAA here, not in `postfx`;
- allow some settings to be applied immediately and others through a pending/apply flow;
- keep configuration readable for save, debug, and future editor tooling.

## Proposed API

### Window and display

- `video_set_resolution(width: int, height: int) -> result<void, core.Error>`: sets internal render resolution.
- `video_get_resolution() -> game.Size2`: reads render resolution.
- `video_set_window_size(width: int, height: int) -> result<void, core.Error>`: sets window size.
- `video_get_window_size() -> game.Size2`: reads window size.
- `video_set_fullscreen(enabled: bool) -> result<void, core.Error>`: toggles fullscreen.
- `video_get_fullscreen() -> bool`: reads fullscreen state.
- `video_set_borderless(enabled: bool) -> result<void, core.Error>`: toggles borderless mode.
- `video_get_borderless() -> bool`: reads borderless state.
- `video_set_resizable(enabled: bool) -> result<void, core.Error>`: toggles window resize permission.
- `video_get_resizable() -> bool`: reads resizable state.

### Sync and frame target

- `video_set_vsync(enabled: bool) -> result<void, core.Error>`: toggles vertical sync.
- `video_get_vsync() -> bool`: reads vertical sync state.
- `video_set_target_fps(value: int) -> result<void, core.Error>`: sets target FPS.
- `video_get_target_fps() -> int`: reads target FPS.

### Anti-aliasing

- `video_set_antialiasing(mode: settings.AntiAliasingMode) -> result<void, core.Error>`: sets anti-aliasing mode (`none`, `msaa2`, `msaa4`, `msaa8`, `fxaa`).
- `video_get_antialiasing() -> settings.AntiAliasingMode`: reads anti-aliasing mode.
- `video_requires_restart(name: text) -> bool`: tells whether a setting change needs backend/window recreation.
- `video_apply_pending() -> result<void, core.Error>`: applies pending video changes.

### Quality and output

- `video_set_quality_profile(level: settings.QualityProfile) -> result<void, core.Error>`: sets quality profile (`low`, `medium`, `high`).
- `video_get_quality_profile() -> settings.QualityProfile`: reads quality profile.
- `video_set_render_scale(value: float) -> result<void, core.Error>`: sets internal render scale.
- `video_get_render_scale() -> float`: reads render scale.
- `video_set_brightness(value: float) -> result<void, core.Error>`: sets output brightness.
- `video_get_brightness() -> float`: reads brightness.
- `video_set_gamma(value: float) -> result<void, core.Error>`: sets output gamma.
- `video_get_gamma() -> float`: reads gamma.

### Presets and state

- `video_reset_defaults() -> result<void, core.Error>`: restores default video configuration.
- `video_get_state() -> settings.VideoSettings`: returns full video settings state.
- `video_save_preset(name: text) -> result<void, core.Error>`: saves a named video preset.
- `video_load_preset(name: text) -> result<void, core.Error>`: loads a named video preset.

## Notes

- `FXAA` can be selected here as a mode, but its effect belongs to `borealis.game.postfx`.
- `MSAA` belongs to render/video setup, not to `postfx`.
- some modes may require recreating the window or framebuffer, so `video_requires_restart` and `video_apply_pending` are part of the design.

