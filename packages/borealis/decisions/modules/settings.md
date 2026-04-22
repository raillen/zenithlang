# Borealis Module Decision - Settings

- Status: proposed
- Date: 2026-04-22
- Type: module / configuration
- Scope: `borealis.game.settings`

## Summary

Settings handles user preferences and basic game configuration.

## Implementation

- keep settings small and predictable;
- separate defaults from runtime state;
- keep user-facing options easy to change.

## Proposed API

- `settings_get(name) -> value`: reads a setting.
- `settings_set(name, value)`: changes a setting.
- `settings_reset(name)`: resets a setting.
- `settings_load(path)`: loads settings.
- `settings_save(path)`: saves settings.
- `settings_apply_defaults()`: applies defaults.
- `settings_list() -> text[]`: lists setting names.

## Notes

- this module should remain simple enough for beginners.
- settings can persist through storage or save, depending on the use case.
