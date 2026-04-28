# Surface Freeze

> Audience: maintainer
> Surface: release engineering
> Status: Phase 0 stabilization artifact

This file classifies public surface before the 1.0 push.

Labels:

- `stable`: user-facing contract for the current release line.
- `experimental`: available, but may still change with release notes.
- `internal`: implementation detail; not promised to users.
- `deferred`: documented direction, not shipped as a supported surface.

## Stable

- Core syntax listed as `Conformant` in `language/surface-implementation-status.md`.
- `zt` commands used by gates: `check`, `build`, `run`, `emit-c`, `fmt`, `doc check`, `test`, `repl`.
- `zenith.ztproj` project model documented in `language/spec/project-model.md`.
- `zenith.lock` schema documented in `language/spec/lockfile-schema.md`.
- Runtime diagnostics model and stable diagnostic codes under `language/spec/diagnostics-model.md` and `language/spec/diagnostic-code-catalog.md`.
- Standard library modules documented under `docs/public/en/stdlib/`.
- Public examples under `examples/`.

## Experimental

- `packages/borealis` public package surface.
- Borealis editor metadata and Studio integration files.
- `std.console` terminal controls beyond basic line output and prompt helpers.
- Performance benchmark comparisons. They are regression signals, not marketing claims.
- Translated docs under `docs/public/es/` and `docs/public/jp/`.

## Internal

- Compiler C internals under `compiler/`.
- HIR, ZIR and generated C representation details, except documented debug fixtures.
- Runtime C implementation details under `runtime/c/`.
- Test harness internals under `tests/`.
- Reports, roadmaps and planning material under `docs/internal/`.

## Deferred

- Manual memory APIs.
- LLVM backend.
- Full implicit type inference.
- Generic argument inference.
- Cycle collection or a stable cycle ownership strategy.
- Async tasks, workers, channels and broad concurrency surface.

## Promotion Rule

To promote a surface to `stable`:

- the English public docs must describe it;
- positive and negative tests must exist when behavior can fail;
- specs or decisions must match the implementation;
- `pr_gate --no-perf` must pass;
- release notes must mention user-visible behavior.

