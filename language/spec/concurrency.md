# Zenith Concurrency Model

- Status: authoritative spec (cut R3.M2)
- Date: 2026-04-23
- Scope: isolate model, transfer boundaries, determinism rule, namespace state under concurrency
- Upstream: `language/decisions/087-concurrency-workers-and-transfer-boundaries.md`
- Cross-reference: `language/spec/runtime-model.md` (sections `Concurrency Direction`, `Transferable Values`, `Implementation Phases`), `docs/internal/reports/R3.P1.A-namespace-shared-state-analysis.md`

## Purpose

Consolidate the official concurrency model of Zenith into a single authoritative document, so contributors do not need to cross-read `runtime-model.md`, `Decision 086`, `Decision 087`, and `R3.P1.A` to understand what is shipped today and what is deferred.

This document is normative. The implementation status section marks what is delivered vs deferred.

## Default Execution Model

1. Ordinary Zenith code executes on a single isolate (one logical owner of managed memory).
2. Managed values (`text`, `bytes`, `list<T>`, `map<K,V>`, structs, enums, `optional<T>`, `result<T,E>`) use non-atomic ARC.
3. There is no implicit cross-thread sharing of managed values.
4. The host (engine, runtime binding, FFI caller) may use threads internally, but crossing into Zenith managed memory must respect the boundary contract below.

Guarantees under this model:

- a function with identical input and identical reachable state produces identical output;
- a mutation inside a function cannot invisibly change state in unrelated code;
- an ordinary `public var` at namespace scope stays single-owner under concurrent execution (see `Namespace State` below).

## Boundary Contract

Crossing an isolate / worker / task boundary must use one of three modes:

1. **`copy`** - deep copy into the destination isolate.
   - This is the only currently delivered mode (Phase 1).
   - Surface: `std.concurrent.copy_*` helpers.
2. **`move`** - transfer of exclusive ownership when the compiler proves the source is no longer used.
   - Deferred to Phase 4.
3. **`shared wrapper`** - synchronized wrapper for narrow shared use cases.
   - Deferred to Phase 5 (`Shared<T>`).
   - `atomic<T>` for primitive scalars is a sibling primitive, also Phase 5.

### Transferable shapes

A value is transferable if its shape is one of:

- a scalar (`int`, `float`, `bool`);
- `text`;
- `bytes`;
- `optional<T>` where `T` is transferable;
- `result<T, E>` where both `T` and `E` are transferable;
- `list<T>` where `T` is transferable;
- `map<K, V>` where both `K` and `V` are transferable;
- a struct or enum whose fields/payloads are transferable.

Not transferable by default:

- live platform handles;
- network connections;
- raw `extern`/FFI resources;
- engine scene objects with shared mutable identity;
- any managed value that was merely "reachable" but not explicitly transferred.

### Determinism rule

For any transferable value `v`, repeated copy calls must produce observationally equal results:

```zt
const a = std.concurrent.copy_int(v)
const b = std.concurrent.copy_int(v)
-- a == b must hold
```

For any `list<T>` or `map<K,V>`, mutation of the copy after a boundary crossing must not affect the source:

```zt
var copy = std.concurrent.copy_list_int(source)
copy[0] = 99
-- source[0] remains unchanged
```

These rules are enforced as behavior tests (see Tests section).

## Namespace State Under Concurrency (`public var`)

This section integrates the `R3.P1.A` analysis.

Normative rules:

- a `public var` is **owned** by the declaring namespace;
- the default model is **single-owner**: the variable is read/written only by code executing inside the declaring namespace;
- crossing a worker/task boundary does **not** share a `public var` automatically; the copy contract applies to any value that crosses;
- there is no implicit promotion from bare `public var` to a shared/atomic wrapper;
- cross-task shared mutation requires one of the Phase 4/5 surfaces (`channels`, `Shared<T>`, `atomic<T>`).

These rules preserve the Decision 086 surface (read-public, write-owner) under concurrent execution.

## User-Facing API Direction

Delivered today (Phase 1):

- `std.concurrent.copy_int`
- `std.concurrent.copy_bool`
- `std.concurrent.copy_float`
- `std.concurrent.copy_text`
- `std.concurrent.copy_bytes`
- `std.concurrent.copy_list_int`
- `std.concurrent.copy_list_text`
- `std.concurrent.copy_map_text_text`

Deferred (Phase 3-5, canonical direction):

```zt
-- Phase 3
const job = jobs.spawn(build_navmesh, snapshot)
const mesh = jobs.join(job)?

-- Phase 4
const channel = channels.create<Chunk>()
channels.send(channel, chunk)

-- Phase 5
var counter: Shared<int> = Shared.create(0)
```

Explicitly **not** part of any planned surface:

- raw thread handles;
- mutex/condvar-first programming;
- implicit cross-thread sharing of managed values;
- reintroduction of `global`.

## Implementation Phases

| Phase | Scope | Status |
|---|---|---|
| 1 | Boundary contract + `std.concurrent.copy_*` + transferability groundwork | delivered (this cut) |
| 2 | Checker understands `transferable` | partial (groundwork in checker; surface still limited to copy helpers) |
| 3 | `jobs.spawn/join` on copy-based transfer | deferred |
| 4 | Move-based transfer + `channels` surface | deferred |
| 5 | Explicit shared wrappers (`Shared<T>`, `atomic<T>`) | deferred |

Deferral is tracked in `docs/internal/reports/R3-risk-matrix.md` as `R3-RISK-010`, `R3-RISK-011`, `R3-RISK-012`.

## Tests

Behavior fixtures covering the current cut:

- positive (copy helpers): `tests/behavior/std_concurrent_boundary_copy_basic`
- negative (non-transferable passed to `copy_text`): `tests/behavior/std_concurrent_boundary_copy_unsupported_error`
- determinism + boundary isolation + order: `tests/behavior/std_concurrent_boundary_copy_determinism`

Out of scope for this cut (waiting on Phase 3):

- race-condition fixtures;
- cancellation fixtures;
- runtime schedule nondeterminism fixtures.

## Non-Goals For `R3.M2`

- no shipping of `jobs`/`channels`/`Shared<T>` in this milestone;
- no promotion of the existing copy helpers to a general-purpose concurrent API;
- no changes to the `public var` surface delivered in `R3.P1`.

## Residual Risk

- Phase 2 surface in the checker is partial; a future cut must publish the explicit `transferable` predicate and unified diagnostic code for boundary violations.
- The copy helpers cover a curated set of shapes; additional shapes (for example `list<struct>`) will need individual helpers or a generic transferable-copy surface before Phase 3 lands.
- Spec remains stable only for Phase 1; each later phase requires its own spec revision in this document.
