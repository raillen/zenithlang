# Decision 013 - Borealis Editor Architecture

- Status: accepted
- Date: 2026-04-22
- Type: architecture / editor / process model
- Scope: Borealis editor modules, preview process, data boundaries, play mode

## Summary

The Borealis editor will be split into clear layers.

The main rule is simple:

1. authoring happens in the editor;
2. execution happens in the runtime preview;
3. both use the same Borealis data model.

## Decision

Adopt this architecture:

1. one desktop editor process;
2. one separate preview/runtime process;
3. shared project model and scene documents;
4. clear separation between edit mode and play mode;
5. IPC bridge between editor and preview.

## High-Level Model

### Editor process

Responsibilities:

- window and panels;
- scene tree;
- inspector;
- asset browser;
- project open/save;
- command stack for undo/redo;
- editor metadata;
- Flow authoring;
- preview orchestration.

### Preview process

Responsibilities:

- load the same project data;
- run Borealis runtime code;
- render the playable preview;
- report runtime events and diagnostics;
- reset cleanly when play mode stops.

## Why separate processes

This gives us important advantages:

1. preview crashes do not take down the editor;
2. editor state stays clean while the game runs;
3. runtime stays close to real shipping behavior;
4. easier to support restart, reload, and debugging.

## IPC Direction

### First phase

Start with simple local IPC:

- `stdio` messages if launch flow is simple;
- local socket if continuous bidirectional messaging becomes easier that way.

### Message style

Messages should be:

- readable;
- versionable;
- easy to log.

The first recommended payload format is JSON.

## Main Editor Modules

### `editor.core`

Purpose:
- app lifecycle, modes, panels, global state.

### `editor.project`

Purpose:
- open/create project;
- read `project.toml`;
- manage workspace state.

### `editor.documents`

Purpose:
- scenes, prefabs, Flow graphs, unsaved changes, recent files.

### `editor.selection`

Purpose:
- current selection, multiselect, focus, hover state.

### `editor.inspector`

Purpose:
- edit properties from the runtime-aligned data model.

### `editor.viewport`

Purpose:
- scene view, gizmos, camera tools, overlays.

### `editor.assets`

Purpose:
- asset scanning, import, indexing, browser, cache.

### `editor.commands`

Purpose:
- undo/redo via explicit operations.

### `editor.preview`

Purpose:
- start/stop preview, IPC, diagnostics, runtime sync.

### `editor.flow`

Purpose:
- Borealis Flow graph editing and integration.

## Data Boundaries

### Runtime documents

These belong to the game/runtime model:

- scenes;
- entities;
- components;
- asset references;
- prefabs;
- Flow graph data that affects the game.

### Editor-only metadata

These belong only to authoring:

- panel layout;
- open tabs;
- viewport options;
- selection history;
- editor notes;
- editor bookmarks.

These must never be mixed into gameplay save data by accident.

## Edit Mode and Play Mode

### Edit mode

- changes document data;
- updates authoring metadata;
- does not mutate live runtime state.

### Play mode

- launches preview/runtime;
- may mirror selected data from editor into preview;
- must be able to stop and return safely to authoring state.

Rule:

Stopping play mode must restore the editor document state, not keep accidental runtime mutations.

## Viewport Strategy

The first viewport model should be simple:

1. editor owns overlays and selection tools;
2. preview owns runtime rendering;
3. scene interactions are passed through the bridge when needed.

This avoids coupling editor UI code to runtime render internals too early.

## Undo/Redo Model

Undo/redo should use explicit command operations such as:

- create entity;
- delete entity;
- rename entity;
- move entity;
- change property;
- assign asset;
- reorder hierarchy.

This keeps authoring predictable and editor-ready.

## Rules

1. The editor is a tool, not the runtime.
2. The preview is real execution, not a mock.
3. Shared documents must round-trip cleanly between editor and runtime.
4. Editor metadata must stay separate from game data.
5. Play mode must be disposable and restartable.

## Relationship to Earlier Decisions

This decision builds on:

- `005-editor-ready-architecture.md`
- `006-borealis-stack.md`
- `011-borealis-editor-stack.md`
- `012-borealis-editor-crates.md`

## Out Of Scope

This decision does not yet finalize:

1. the exact file layout inside the future editor repository;
2. the exact message schema of the preview bridge;
3. the final docking layout;
4. collaborative or networked editing.
