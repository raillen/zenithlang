# Borealis Module Decisions

This folder stores module-by-module decisions for Borealis.

Rule of the folder:

- one module per file;
- each file explains the module, implementation approach, and proposed API surface;
- descriptions stay short and easy to read.

## Current Modules

- `movement.md`
- `controllers.md`
- `vehicles.md`
- `animation.md`
- `audio.md`
- `ai.md`
- `camera.md`
- `input.md`
- `entities.md`
- `world.md`
- `procedural.md`
- `ui.md`
- `assets.md`
- `save.md`
- `storage.md`
- `database.md`
- `services.md`
- `settings.md`
- `debug.md`
- `scene.md`
- `events.md`

## Notes

- `hud` lives inside `ui` as `borealis.game.ui.hud`.
- `save` is for game progress and snapshots.
- `storage` is for generic persistence.
- `database` is for future SQLite-style backends.
- `services` is for network, cloud save, APIs, and remote sessions.
- editor-ready architecture guidance lives in `packages/borealis/decisions/005-editor-ready-architecture.md`.
- module decisions here are proposals until explicitly accepted.
