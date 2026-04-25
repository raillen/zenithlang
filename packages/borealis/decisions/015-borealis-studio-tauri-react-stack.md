# Decision 015 - Borealis Studio Tauri React Stack

- Status: accepted
- Date: 2026-04-25
- Type: architecture / editor / stack / supersession
- Scope: `tools/borealis-studio`, desktop shell, local project IO, preview integration

## Summary

Borealis Studio will proceed with `Tauri + React + TypeScript` as the active editor stack.

This supersedes Decision 011 for the current Studio product direction.

The chosen path is:

1. editor UI in React;
2. desktop shell and local system bridge in Tauri 2;
3. TypeScript for the frontend editor surface;
4. Rust for native commands, local files, and preview process control;
5. runtime preview kept as a separate sidecar process over JSONL stdio.

## Decision

Adopt this Studio stack:

1. Active editor:
   - `tools/borealis-studio` is the active editor application.
2. UI:
   - React + Vite + TypeScript are the main UI stack.
3. Desktop shell:
   - Tauri 2 owns local files, process spawning, and native bridge commands.
4. Runtime preview:
   - the preview process stays separate from the editor UI.
   - communication uses the JSONL stdio protocol from Decision 014.
5. Legacy prototype:
   - `tools/borealis-editor` remains useful as reference code and preview harness material.
   - it is not the active product stack unless a future decision revives it.

## Why

This direction matches the current implementation and the chosen product path.

It gives Borealis Studio:

- faster UI iteration;
- a working React prototype with real panels, viewport, inspector, assets, console, and script editing;
- easier integration with Three.js viewport work;
- a simple Rust bridge for local IO and preview process control;
- less duplication between product UI experiments and the active editor.

The user explicitly selected `Tauri + React` for the editor stack on 2026-04-25.

## What This Means In Practice

The editor has two layers:

1. React frontend:
   - layout;
   - panels;
   - viewport controls;
   - inspector editing;
   - script editing;
   - console and diagnostics display.
2. Tauri backend:
   - project snapshot loading;
   - file reads and writes;
   - scene serialization handoff;
   - preview process lifecycle;
   - JSONL command flushing and event polling.

The preview remains another process.

The editor sends commands like:

- open project;
- open scene;
- enter play mode;
- pause play mode;
- stop play mode.

The preview answers with data like:

- runtime status;
- loaded scene state;
- entity count;
- diagnostics;
- stderr messages.

## Relationship To Other Decisions

This decision:

- supersedes `011-borealis-editor-stack.md` for the active Studio stack;
- keeps the process separation from `013-borealis-editor-architecture.md`;
- keeps the preview IPC protocol from `014-borealis-editor-preview-ipc.md`.

## Rules

1. The editor stays desktop-first.
2. The Borealis runtime must not depend on the React UI.
3. Tauri commands should keep native responsibilities narrow and testable.
4. Preview communication should stay readable and line-oriented until a stronger transport is needed.
5. User-facing editor surfaces should stay dense, predictable, and low-noise.

## Out Of Scope

This decision does not finalize:

1. the final docking system;
2. the final command palette design;
3. a frame streaming protocol for embedding live Raylib output inside the viewport;
4. packaging and update distribution.
