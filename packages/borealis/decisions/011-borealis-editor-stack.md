# Decision 011 - Borealis Editor Stack

- Status: superseded by `015-borealis-studio-tauri-react-stack.md`
- Date: 2026-04-22
- Type: architecture / editor / stack
- Scope: Borealis editor application, desktop shell, UI toolkit, preview integration

> Superseded note, 2026-04-25:
> Decision 015 makes `Tauri + React + TypeScript` the active Borealis Studio stack.
> This file is kept as historical context for the Rust + egui path.

## Summary

The Borealis editor will use a Rust-first stack.

The chosen path is:

1. editor in Rust;
2. UI built with `egui` + `eframe`;
3. Borealis runtime preview kept as a separate native process;
4. communication between editor and preview done with simple IPC.

## Decision

Adopt this editor stack:

1. Language:
   - Rust is the primary language of the editor.
2. UI:
   - `egui` is the base UI library.
   - `eframe` is the desktop app framework.
3. Runtime preview:
   - the preview must run the real Borealis runtime, not a fake in-editor simulation.
4. Process model:
   - the editor process and the preview process stay separate.
5. Communication:
   - use simple IPC first:
     - local `stdio`, or
     - local socket,
     - with readable message payloads.
6. Web stack:
   - JavaScript/TypeScript is not part of the main editor stack.

## Why this stack

This stack gives the best balance for Borealis today:

- faster to build than a full C or full C++ editor;
- safer and easier to maintain than full C;
- more aligned with native tooling than a web-first editor;
- good for inspectors, trees, docking, gizmos, debug panels, and tools;
- keeps the Borealis runtime independent from the editor UI.

## What this means in practice

The editor is one application.

The preview is another process.

The editor sends commands like:

- open scene;
- reload asset;
- enter play mode;
- stop play mode;
- move selected entity;
- focus camera.

The preview answers with data like:

- runtime status;
- selected entity info;
- frame time;
- errors;
- warnings;
- debug events.

## Rejected directions

These were considered, but are not the primary path:

1. `full C` editor
   - too expensive for UI/tooling productivity.
2. `full C++ + Qt`
   - strong option, but heavier than needed for the current phase.
3. `Tauri + React`
   - productive, but not aligned with the desired non-web core stack.
4. `Rust + Slint`
   - good option, but `egui` is a better fit for a technical editor first.

## Rules

1. The editor must stay desktop-first.
2. The preview must stay close to the real game runtime.
3. The editor UI must not become a hard dependency of the Borealis runtime.
4. The stack must stay simple enough for incremental delivery.

## Relationship to Earlier Decisions

This decision complements:

- `005-editor-ready-architecture.md`
- `006-borealis-stack.md`

It resolves the previously open question of the editor UI stack.

## Out Of Scope

This decision does not yet finalize:

1. the exact docking crate;
2. the exact graph/node editor crate for Borealis Flow;
3. whether preview IPC starts on `stdio` or local socket first;
4. the visual design of the editor.
