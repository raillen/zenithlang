# Borealis Decision 009 - 3D Performance Phasing

- Status: proposed
- Date: 2026-04-22
- Type: architecture / performance policy
- Scope: Borealis 3D (`borealis.game` + `borealis.engine`)

## Summary

Borealis 3D will adopt performance in phases.
The easy layer stays simple.
Advanced optimization stays mostly internal to the engine.

## Goal

- keep onboarding simple;
- avoid premature complexity;
- guarantee a clear path to scale when scenes grow.

## Phases

### Phase 1 (MVP default)

Required:

- frustum culling (simple);
- backface culling;
- depth buffer enabled by default;
- asset deduplication and cache reuse;
- broad phase in physics;
- sleep for inactive rigid bodies;
- update only active/visible entities.

### Phase 2 (after stable gameplay loop)

Planned:

- instancing;
- texture atlas support;
- material sorting;
- manual LOD by distance;
- asset streaming;
- object pooling.

### Phase 3 (advanced scaling)

Planned:

- simple occlusion heuristics;
- logic LOD (AI/system simplification when far);
- multithreaded loading/jobs (as opt-in).

## Module Integration

### `render3d`

- should expose only simple controls to gameplay code;
- should keep heavy optimization mostly automatic.

Recommended easy controls:

- `render3d_set_quality_profile(level)` with `low | medium | high`;
- `render3d_get_quality_profile()`.

### `assets3d` (next module decision)

Must include hooks for:

- deduplicated asset cache;
- streaming on demand;
- memory budget stats.

### `physics3d`

- broad phase and body sleep are default behavior in runtime;
- gameplay API remains simple in `borealis.game.physics3d`.

## Notes

- this policy is for classic/non-realistic 3D too.
- optimization is not optional in architecture, but complexity is phased.
