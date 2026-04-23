# Borealis Module Decision - PostFX

- Status: proposed
- Date: 2026-04-22
- Type: module / screen-space visual effects API
- Scope: `borealis.game.postfx`

## Summary

PostFX owns full-screen visual finishing effects.
It is separate from world-space atmosphere and weather.

## Implementation

- keep post-processing focused on screen-space effects only;
- keep beginner-friendly presets for common looks;
- include FXAA here, but keep VSync and MSAA out of this module.

## Proposed API

### Global control

- `postfx_set_enabled(enabled: bool) -> result<void, core.Error>`: enables/disables post-processing.
- `postfx_is_enabled() -> bool`: reads post-processing state.
- `postfx_reset() -> result<void, core.Error>`: resets all effects to default values.
- `postfx_apply_preset(name: text) -> result<void, core.Error>`: applies a named preset.
- `postfx_get_state() -> postfx.PostFxState`: returns full post-processing state.

### Color controls

- `color_set_tint(color: game.Color) -> result<void, core.Error>`: applies a global tint.
- `color_get_tint() -> game.Color`: reads current tint.
- `color_set_brightness(value: float) -> result<void, core.Error>`: adjusts brightness.
- `color_get_brightness() -> float`: reads brightness.
- `color_set_contrast(value: float) -> result<void, core.Error>`: adjusts contrast.
- `color_get_contrast() -> float`: reads contrast.
- `color_set_saturation(value: float) -> result<void, core.Error>`: adjusts saturation.
- `color_get_saturation() -> float`: reads saturation.

### Vignette

- `vignette_set_enabled(enabled: bool) -> result<void, core.Error>`: toggles vignette.
- `vignette_set_intensity(value: float) -> result<void, core.Error>`: sets vignette strength.
- `vignette_set_smoothness(value: float) -> result<void, core.Error>`: sets vignette softness.
- `vignette_get_state() -> postfx.VignetteState`: reads vignette settings.

### Grain

- `grain_set_enabled(enabled: bool) -> result<void, core.Error>`: toggles film grain.
- `grain_set_intensity(value: float) -> result<void, core.Error>`: sets grain strength.
- `grain_set_speed(value: float) -> result<void, core.Error>`: sets grain animation speed.
- `grain_get_state() -> postfx.GrainState`: reads grain settings.

### Scanlines

- `scanlines_set_enabled(enabled: bool) -> result<void, core.Error>`: toggles scanlines.
- `scanlines_set_intensity(value: float) -> result<void, core.Error>`: sets line strength.
- `scanlines_set_spacing(value: float) -> result<void, core.Error>`: sets line spacing.
- `scanlines_get_state() -> postfx.ScanlinesState`: reads scanline settings.

### Pixelation and blur

- `pixelate_set_enabled(enabled: bool) -> result<void, core.Error>`: toggles pixelation.
- `pixelate_set_size(value: float) -> result<void, core.Error>`: sets pixel size.
- `pixelate_get_state() -> postfx.PixelateState`: reads pixelation settings.
- `blur_set_enabled(enabled: bool) -> result<void, core.Error>`: toggles blur.
- `blur_set_strength(value: float) -> result<void, core.Error>`: sets blur strength.
- `blur_get_state() -> postfx.BlurState`: reads blur settings.

### Bloom and chromatic aberration

- `bloom_set_enabled(enabled: bool) -> result<void, core.Error>`: toggles bloom.
- `bloom_set_threshold(value: float) -> result<void, core.Error>`: sets bloom threshold.
- `bloom_set_intensity(value: float) -> result<void, core.Error>`: sets bloom intensity.
- `bloom_get_state() -> postfx.BloomState`: reads bloom settings.
- `chromatic_set_enabled(enabled: bool) -> result<void, core.Error>`: toggles chromatic aberration.
- `chromatic_set_intensity(value: float) -> result<void, core.Error>`: sets chromatic intensity.
- `chromatic_get_state() -> postfx.ChromaticState`: reads chromatic settings.

### Retro and palette

- `palette_set_enabled(enabled: bool) -> result<void, core.Error>`: toggles palette reduction.
- `palette_set_steps(value: int) -> result<void, core.Error>`: sets number of color steps.
- `posterize_set_enabled(enabled: bool) -> result<void, core.Error>`: toggles posterization.
- `posterize_set_levels(value: int) -> result<void, core.Error>`: sets posterize levels.

### Anti-aliasing in post-processing

- `fxaa_set_enabled(enabled: bool) -> result<void, core.Error>`: toggles FXAA.
- `fxaa_get_enabled() -> bool`: reads FXAA state.

### Presets

- `postfx_preset_ps1() -> result<void, core.Error>`: applies retro PS1-style look.
- `postfx_preset_dream() -> result<void, core.Error>`: applies soft dream-like look.
- `postfx_preset_horror() -> result<void, core.Error>`: applies contrast-heavy horror look.
- `postfx_preset_clean() -> result<void, core.Error>`: applies minimal-clean look.

## Notes

- `fog`, `sky`, `weather`, and `time-of-day` belong to `world3d`.
- `VSync` belongs to `settings.video`.
- `MSAA` belongs to `settings.video` or render setup, not to `postfx`.

