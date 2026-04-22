# Decision 002 - Raylib Feasibility By Topic

- Status: proposed
- Date: 2026-04-22
- Type: feasibility / backend strategy
- Scope: `borealis.game`, `borealis.engine`, Raylib backend planning

## Summary

This decision classifies Borealis function groups by implementation effort with Raylib:

1. direct in Raylib (`raylib-direct`);
2. possible with Raylib + Borealis logic (`raylib-plus-borealis`);
3. needs external library for production quality (`needs-external`);
4. postpone to later milestone (`postpone`).

Goal: reduce risk and keep an easy-first delivery path.

## Decision

Adopt this rule for Borealis 1.0 planning:

1. Start with `raylib-direct` and `raylib-plus-borealis` items.
2. Mark `needs-external` features as optional integrations, not hard requirements.
3. Keep `postpone` features out of initial promises.

Per topic status:

1. Collision: `raylib-plus-borealis` (partial `raylib-direct` for primitive checks).
2. Physics: `raylib-plus-borealis` for lightweight kinematic model; robust rigid body is `needs-external`.
3. Sprites: mostly `raylib-direct`, advanced animation in `raylib-plus-borealis`.
4. Shaders/FX: mostly `raylib-direct` + `raylib-plus-borealis`.
5. Movement: `raylib-plus-borealis`.
6. AI/Behavior: `raylib-plus-borealis` (algorithms are Borealis-side).
7. Camera: base `raylib-direct`, advanced rig in `raylib-plus-borealis`.
8. Input: base `raylib-direct`, actions/contexts in `raylib-plus-borealis`.
9. World/Tilemap: `raylib-plus-borealis` for basics, `needs-external` for advanced generation.
10. Particles: `raylib-plus-borealis`.

## Rationale

Raylib is excellent for:

- window, frame loop, drawing, textures, shaders, camera, basic input.

Raylib does not provide a full gameplay framework for:

- robust physics stack;
- behavior tree/navmesh stack;
- turnkey state-driven gameplay systems.

So Borealis should use Raylib as rendering/input backend and add gameplay systems in Borealis modules.

## Delivery Guardrails

Do first (low risk / high value):

1. draw primitives + textures + text;
2. frame lifecycle + input down/pressed/released;
3. animation player v1;
4. camera follow + bounds + shake;
5. collision primitives + move-and-collide v1.

Defer from first cut:

1. full rigid-body simulation parity;
2. navmesh generation;
3. full behavior tree editor-grade tooling;
4. rollback/rewind physics for netcode scenarios.

## Non-Canonical Forms

1. Promise every idea from `ideia_funcs.md` in Borealis 1.0.
2. Couple first release to mandatory external physics library.
3. Block easy API release waiting for advanced AI/navmesh stack.

## Out of Scope

This decision does not freeze:

1. final function-by-function naming;
2. exact milestone dates;
3. which external physics library (if any) will be official first.
