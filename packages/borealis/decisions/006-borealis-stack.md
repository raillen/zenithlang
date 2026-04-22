# Decision 006 - Borealis Stack

- Status: proposed
- Date: 2026-04-22
- Type: architecture / stack / platform direction
- Scope: Borealis runtime, data formats, persistence, editor foundation

## Summary

Define the foundational stack for Borealis so the runtime, data model, persistence, and future editor stay aligned.

## Decision

Adopt this stack direction for Borealis:

1. Language:
   - Zenith is the primary language.
2. Runtime:
   - the current C runtime remains the native execution foundation.
3. Graphics backend:
   - Raylib is the initial rendering backend.
   - backend access stays behind `borealis.backend`.
4. Project format:
   - `zenith.ztproj` in TOML remains the project manifest format.
5. Data formats:
   - JSON is the default format for saves, snapshots, and scene-like documents in early cuts.
   - binary storage is optional when needed for performance or transport.
6. Persistence layers:
   - `save` for gameplay snapshots and progress;
   - `storage` for generic persistence;
   - `database` reserved for future SQLite-style backends.
7. Remote layers:
   - `services` is the umbrella module for network, cloud save, APIs, and remote sessions.
8. Editor direction:
   - the future editor must consume the same runtime data model.

## Rationale

This stack keeps the project practical and incremental:

- Zenith stays at the center;
- C runtime keeps the native path stable;
- Raylib gives a fast rendering path;
- TOML and JSON match the existing ecosystem direction;
- persistence and remote concerns stay separate.

## Stack Layers

Core:

- Zenith
- C runtime
- Borealis package

Rendering:

- Raylib
- backend boundary in `borealis.backend`

Data:

- TOML for project/manifest
- JSON for saves, snapshots, and scene-style documents

Persistence:

- `save`
- `storage`
- `database`

Remote:

- `services`

Editor:

- same entity/scene/component/asset model as runtime

## Non-Canonical Forms

1. inventing a separate editor stack that ignores runtime data;
2. mixing save, storage, database, and remote services into one module;
3. making backend/linker concerns part of the beginner path.

## Out Of Scope

This decision does not finalize:

1. the future desktop shell or UI framework of the editor;
2. the exact scene/prefab file formats;
3. advanced backend choices beyond the initial Raylib direction.
