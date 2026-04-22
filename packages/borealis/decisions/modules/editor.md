# Module Decision: game.editor

## Purpose

`borealis.game.editor` stores editor-only metadata without polluting gameplay state.

Goal: future editor can attach labels/notes/groups/visibility/lock flags by stable id.

## Initial Scope (N1)

- Metadata by `stable_id` (`text`).
- Simple get/set APIs.
- Zero runtime dependency on renderer/backend.

## Types

- `EntityMeta`
  - `stable_id`
  - `label`
  - `note`
  - `group`
  - `locked`
  - `hidden`

## Functions

- `set_label`, `get_label`
- `set_note`, `get_note`
- `set_group`, `get_group`
- `set_locked`, `is_locked`
- `set_hidden`, `is_hidden`
- `inspect(stable_id)` -> `EntityMeta`

## Notes

- Metadata stays mutable and tool-facing.
- Save format integration comes later via `save/storage`.
