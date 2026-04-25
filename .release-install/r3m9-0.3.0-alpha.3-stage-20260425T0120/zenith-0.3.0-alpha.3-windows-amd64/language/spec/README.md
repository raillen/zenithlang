# Zenith Language Specs

- Status: canonical consolidated specs
- Date: 2026-04-22
- Source: accepted decisions in `language/decisions/`

## Purpose

This directory contains the readable canonical specs for Zenith Next.

Decision documents remain the historical source for rationale, tradeoffs and conversation context.

These spec files are the implementation-facing reference used after the design consolidation.

## Project context (AI-assisted)

- These specs are part of an early AI-assisted language development initiative.
- The repository is used as a study case, training ground, and validation target.
- Final architecture and release decisions remain human-owned.

## Canonical Documents

- `surface-syntax.md`: user-visible Zenith language syntax and semantic rules.
- `closures.md`: closure v1 syntax, capture rules, lifetime model and callable fat pointer ABI.
- `lambdas-hof-guidelines.md`: expression lambda syntax guidance and first HOF usage rules.
- `lazy.md`: explicit `lazy<T>` rules, one-shot consumption and no implicit lazy evaluation.
- `stdlib-model.md`: standard library architecture, module policy, error policy and safe API rules.
- `runtime-model.md`: C runtime, managed values, value semantics, cleanup, panic and contracts.
- `backend-scalability-risk-model.md`: RC cycles, monomorphization, stack/heap policy and backend scalability gates.
- `diagnostics-model.md`: structured diagnostics, stable codes and terminal rendering.
- `diagnostic-code-catalog.md`: initial stable code catalog for renderer/tests/tooling alignment.
- `formatter-model.md`: mandatory canonical formatting rules.
- `project-model.md`: `zenith.ztproj`, app/lib projects, file layout, ZDoc layout and ZPM package model.
- `lockfile-schema.md`: initial `zenith.lock` schema for reproducible dependency resolution.
- `compiler-model.md`: compiler pipeline, IR boundaries, C backend, runtime and artifact modes.
- `tooling-model.md`: `zt`, `zpm`, diagnostics, formatter, tests and documentation tooling.
- `implementation-status.md`: status vocabulary and closure rules for implementation tracking.
- `conformance-matrix.md`: final conformance snapshot by layer/feature/risk for M32.
- `decision-conflict-audit.md`: reconciled conflicts between historical decisions and canonical specs.

## Supporting Documents

- `legibility-evaluation.md`: protocol for validating reading-first legibility, metrics, tasks, approval criteria and release gates.
- `cognitive-accessibility.md`: design principles and tooling proposals for ADHD, Dyslexia, Autism and neurodiversity.

## Reading Order

1. `surface-syntax.md`
2. `closures.md`
3. `lambdas-hof-guidelines.md`
4. `lazy.md`
5. `stdlib-model.md`
6. `runtime-model.md`
7. `backend-scalability-risk-model.md`
8. `diagnostics-model.md`
9. `diagnostic-code-catalog.md`
10. `formatter-model.md`
11. `project-model.md`
12. `lockfile-schema.md`
13. `compiler-model.md`
14. `tooling-model.md`
15. `implementation-status.md`
16. `conformance-matrix.md`
17. `decision-conflict-audit.md`
18. `legibility-evaluation.md`
19. `cognitive-accessibility.md`

## Relationship To Decisions

The specs consolidate Decisions 001-086.

If a spec and an older decision conflict, the newer accepted decision and this consolidated spec take precedence.

Older decisions are not deleted because they preserve why the rule exists.

## Relationship To Implementation

Implementation status is tracked separately in `../surface-implementation-status.md` and `../../docs/planning/checklist-v1.md`.

The compiler may lag behind this spec during implementation milestones.

When that happens, roadmap/checklist items should describe the gap explicitly instead of weakening the spec.
