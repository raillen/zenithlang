# Borealis Module Decision - Storage

- Status: proposed
- Date: 2026-04-22
- Type: module / generic persistence
- Scope: `borealis.game.storage`

## Summary

Storage handles generic persistence for files, blobs, text, JSON, and other structured data.

## Implementation

- keep it generic and reusable;
- separate it from game progress and from network services;
- expose clear helpers for common storage formats.

## Proposed API

- `storage_write_text(path, value)`: writes text.
- `storage_read_text(path) -> text`: reads text.
- `storage_write_json(path, value)`: writes JSON data.
- `storage_read_json(path) -> Data`: reads JSON data.
- `storage_write_binary(path, bytes)`: writes binary data.
- `storage_read_binary(path) -> Bytes`: reads binary data.
- `storage_exists(path) -> bool`: checks whether a path exists.
- `storage_delete(path)`: deletes a stored item.
- `storage_list(path) -> text[]`: lists items in a location.
- `storage_copy(from, to)`: copies a stored item.
- `storage_move(from, to)`: moves a stored item.

## Notes

- storage is the generic layer under `save`.
- storage is also the base for cache-like helpers.
- storage can build on stdlib file and JSON helpers under the hood.
