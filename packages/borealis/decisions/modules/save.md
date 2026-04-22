# Borealis Module Decision - Save

- Status: proposed
- Date: 2026-04-22
- Type: module / persistence
- Scope: `borealis.game.save`

## Summary

Save handles game progress, profile data, and serializable state snapshots for game elements.

## Implementation

- keep save format predictable;
- prefer small explicit helpers;
- avoid tying save logic to gameplay modules;
- allow any game element to expose a snapshot that can be saved and restored.

## Proposed API

- `save_write(name, data)`: writes save data.
- `save_read(name) -> Data`: reads save data.
- `save_delete(name)`: deletes save data.
- `save_exists(name) -> bool`: checks whether save exists.
- `save_list() -> text[]`: lists save slots.
- `save_backup(name)`: creates a backup.
- `save_restore(name)`: restores a backup.
- `save_snapshot(entity) -> Data`: exports the state of a game element.
- `save_restore_snapshot(entity, data)`: restores the state of a game element.
- `save_register_adapter(name, adapter)`: registers a serializer adapter.

## Notes

- default format should be JSON unless a case needs a different backend.
- save can build on storage and stdlib JSON helpers under the hood.
- persistence should stay separate from language serialization policy.
