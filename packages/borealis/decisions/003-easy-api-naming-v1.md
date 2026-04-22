# Decision 003 - Easy API Naming V1

- Status: proposed
- Date: 2026-04-22
- Type: API naming / accessibility
- Scope: `borealis.game` easy layer function naming

## Summary

Define naming rules for the easy API so beginners can read and remember functions quickly.

Principle:

- Game Maker ease + Unity organization.

## Decision

For `borealis.game`, use these naming rules:

1. `snake_case` only.
2. short verb-first names (`draw_rect`, `move_towards`, `key_down`).
3. explicit frame semantics (`frame_begin`, `frame_end`).
4. avoid abbreviations unless common (`fps`, `id`).
5. keep family consistency:
   - input: `key_down`, `key_pressed`, `key_released`
   - animation: `anim_create`, `anim_play`, `anim_update`
   - camera: `camera_follow`, `camera_shake`, `camera_set_zoom`

Canonical loop naming in easy layer:

1. `start(config)` -> opens game context/window.
2. `running(ctx)` -> true while game should continue.
3. `frame_begin(ctx, clear)`.
4. `frame_end(ctx)`.
5. `close(ctx)`.

Engine layer remains explicit:

1. `open_window`;
2. `window_should_close`;
3. backend-oriented functions.

## Rationale

This split keeps onboarding simple while preserving technical precision in engine APIs.

Short names help with dyslexia/attention load when combined with consistent prefixes.

## Naming Quality Checklist

Before accepting a new function name in `borealis.game`, validate:

1. can a beginner guess what it does from the name?
2. does it match an existing family pattern?
3. is frame timing clear (instant, per-frame, state)?
4. can autocomplete group it with similar functions?

If any answer is "no", rename before merge.

## Open Naming Items (pending final decision)

These names are intentionally kept as proposal only:

1. collision `sweep`:
   - preferred candidate: `shape_sweep`
   - alternatives: `sweep_cast`, `motion_cast`
2. collision `query`:
   - preferred candidate: `find_overlaps`
   - alternatives: `query_overlaps`, `filter_overlaps`

Decision policy:

- keep one canonical name;
- keep temporary aliases only during migration window;
- remove deprecated alias after documented cutoff.

## Non-Canonical Forms

1. mixed camelCase and snake_case in easy layer;
2. one-off naming styles that break family patterns;
3. hidden frame behavior in generic names (example: `update` without scope).

## Out of Scope

This decision does not define:

1. every final function signature;
2. type layout for scene/entity/component;
3. advanced engine naming.
