# Decision 012 - Borealis Editor Crates

- Status: accepted
- Date: 2026-04-22
- Type: architecture / editor / crates
- Scope: Borealis editor dependencies and support libraries

## Summary

Define the first recommended crate set for the Borealis editor.

The goal is simple:

1. small core;
2. clear responsibilities;
3. easy evolution;
4. no unnecessary dependencies too early.

## Decision

Adopt these crates as the first recommended editor stack.

## Core UI

### Required

- `egui`
  - base immediate-mode UI.
- `eframe`
  - desktop app shell and native rendering integration.
- `egui_extras`
  - useful extras such as images and table helpers.

### Why

These three give us the full base for:

- windows;
- panels;
- inspector;
- asset browser;
- hierarchy tree;
- property editors;
- viewport overlays.

## File dialogs and system integration

### Required

- `rfd`
  - native open/save dialogs.
- `walkdir`
  - recursive project and asset scanning.
- `notify`
  - file watching for hot reload and refresh.

### Why

These solve three constant editor tasks:

1. choose files;
2. scan projects;
3. react to changes on disk.

## Serialization and formats

### Required

- `serde`
  - common serialization layer.
- `serde_json`
  - scenes, prefabs, Flow graphs, runtime messages.
- `toml`
  - project manifests and editor configuration.

### Why

This matches the current Borealis direction:

- `TOML` for project/config;
- `JSON` for scene-like and graph-like documents.

## Logging and diagnostics

### Required

- `tracing`
  - structured logs and spans.
- `tracing-subscriber`
  - log formatting and sinks.

### Why

The editor will need:

- readable logs;
- debug output;
- preview diagnostics;
- internal telemetry for tooling.

## Local cache and indexing

### Required

- `rusqlite`
  - local cache/index database.

### Why

Use SQLite only for:

- asset indexing;
- search cache;
- thumbnails metadata;
- editor-local lookup tables.

Do not use it as the main format for scenes or game documents.

## Accessibility

### Included by stack

- `AccessKit`
  - through the `egui` ecosystem when available.

### Why

Accessibility must not be an afterthought.

This is especially important because the project already has an accessibility direction.

## Phase 2 crates

These are useful, but not mandatory in the first cut.

### Recommended later

- `egui_dock`
  - docking and tabbed editor layout.
- `egui_plot`
  - metrics, profiler panels, curves, graphs.
- `petgraph`
  - backend graph model for Borealis Flow.

## Phase 3 candidates

These are promising, but should enter only when there is a concrete need.

- `transform-gizmo`
  - transform gizmos for scene editing.
- a dedicated node editor crate
  - only if Flow really needs one before we build our own layer.

## Canonical Core Set

If we had to freeze the minimal editor crate list today, it would be:

1. `egui`
2. `eframe`
3. `egui_extras`
4. `rfd`
5. `serde`
6. `serde_json`
7. `toml`
8. `notify`
9. `walkdir`
10. `tracing`
11. `tracing-subscriber`
12. `rusqlite`

## Rules

1. Every new crate must solve a clear problem.
2. The first cut should prefer boring and proven crates.
3. The editor core must stay readable and maintainable.
4. The runtime preview must not inherit the editor's crate graph by accident.

## Out Of Scope

This decision does not yet freeze:

1. the exact docking crate version;
2. the exact graph editor crate;
3. thumbnail/image processing crates;
4. networking crates for future remote collaboration.
