# Borealis Module Decisions (3D)

This folder stores module-by-module decisions for Borealis 3D.

Rules:

- one module per file;
- each file explains purpose, implementation approach, and API surface;
- descriptions should stay short, clear, and beginner-friendly.

## Current Modules

- `core3d.md`
- `render3d.md`
- `camera3d.md`
- `physics3d.md`
- `assets3d.md`
- `animation3d.md`
- `audio3d.md`
- `ai3d.md`
- `controllers3d.md`
- `debug3d.md`
- `procedural3d.md`
- `entities3d.md`
- `ui3d.md`
- `settings.video.md`
- `world3d.md`
- `world3d.atmosphere.md`
- `world3d.weather.md`
- `postfx.md`

## Notes

- 3D decisions follow the same process used in Borealis 2D:
  - discuss module;
  - user chooses final direction;
  - decision file is saved.
- performance phasing for 3D is defined in:
  - `packages/borealis/decisions/009-borealis-3d-performance-phasing.md`
- shared modules stay shared:
  - `scene`
  - `save`
  - `events`
  - `services`
  - `storage`
  - `database`
  - `input`
  - `settings` (with `settings.video` as the 3D-facing video submodule)
- no separate `scene3d`, `save3d`, `input3d`, or `services3d` is planned in this design pass.

