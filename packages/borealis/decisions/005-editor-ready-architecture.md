# Decision 005 - Editor Ready Architecture

- Status: proposed
- Date: 2026-04-22
- Type: architecture / editor readiness / data model
- Scope: Borealis runtime, data model, editor compatibility

## Summary

Borealis should be organized so the future editor can read, edit, inspect, and save the same data model used by the runtime.

The editor must not invent a separate project format that diverges from the game runtime.

## Decision

Adopt these architecture rules for Borealis:

1. Entities must have stable IDs.
2. Scenes must be treated as documents.
3. Components must stay serializable and version-friendly.
4. Runtime logic must stay separate from authoring metadata.
5. Editor-only state must not leak into gameplay saves.
6. Undo/redo should be represented as command operations.
7. Hot reload should work for assets, scenes, and configuration when possible.
8. The editor should operate on the same data model the runtime consumes.

## Rationale

This keeps the runtime and editor aligned and reduces rewrite risk later.

The structure also helps with:

- scene editing;
- hierarchy editing;
- inspector workflows;
- prefab-like reuse;
- save/load of authored content;
- future asset management.

## Data Boundaries

Runtime data:

- entity state;
- scene composition;
- component data;
- asset references;
- gameplay snapshots.

Editor metadata:

- selection state;
- window layout;
- panel docking;
- gizmo visibility;
- preview toggles;
- last open documents.

The two must remain separate.

## Required Characteristics

1. Stable IDs for entities and scenes.
2. Snapshot support for entities and scenes.
3. Serializable component payloads.
4. Explicit asset references instead of hidden paths.
5. Clear command-based editing for undo/redo.
6. Readable hierarchy structures for editor views.

## Non-Canonical Forms

1. building a separate editor-only scene format that cannot round-trip to runtime;
2. using transient pointer identity as the only object identity;
3. storing editor window state inside gameplay save files;
4. mixing runtime code paths with editor tool state.

## Out Of Scope

This decision does not finalize:

1. the editor UI framework;
2. the exact serialization format for scenes and prefabs;
3. the command model implementation details;
4. the rendering backend for the editor.
