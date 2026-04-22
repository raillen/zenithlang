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
- `world3d.md`
- `world3d.atmosphere.md`
- `world3d.weather.md`

## Notes

- 3D decisions follow the same process used in Borealis 2D:
  - discuss module;
  - user chooses final direction;
  - decision file is saved.
- performance phasing for 3D is defined in:
  - `packages/borealis/decisions/009-borealis-3d-performance-phasing.md`
- next planned module discussions:
  - `assets3d`
  - `postfx`
