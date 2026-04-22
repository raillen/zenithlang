# Decision 001 - Borealis Module Layering And Naming

- Status: accepted
- Date: 2026-04-22
- Type: framework / architecture / API naming
- Scope: `borealis.game`, `borealis.engine`, boundary between easy and advanced APIs

## Summary

Borealis will use two explicit public module layers:

- `borealis.game` for the easy-first developer experience.
- `borealis.engine` for advanced and technical control.

The root module `borealis` remains available for compatibility during this phase.

## Decision

Normative rules for this cut:

- `borealis.game` is the recommended entrypoint for beginners and rapid prototyping.
- `borealis.engine` is the advanced layer for teams that need deeper control and explicit architecture.
- `borealis` root remains valid during transition and should not be broken.
- backend and linker concerns are not exposed as first-step concepts in `borealis.game`.
- detailed ergonomic naming for the easy API will be finalized in a later decision.

## Rationale

This split balances two goals that can conflict:

- easy onboarding (Game Maker style flow).
- scalable code organization (Unity style structure).

Keeping two layers avoids forcing beginners into engine-level complexity while preserving growth paths for larger projects.

## Initial Boundary

- `borealis.game`: simple usage path and defaults.
- `borealis.engine`: explicit window/frame/render/input control and backend-oriented details.

Both layers are scaffold modules in this cut while the easy API naming RFC remains open.

## Non-Canonical Forms

Treating `borealis.engine` as the default recommendation for first-contact tutorials.

Treating linker/backend setup as required first-step knowledge in `borealis.game`.

## Out of Scope

This decision does not define:

- final naming of every easy-layer function.
- scene/entity/system API details.
- editor, map tooling, or asset pipeline.
