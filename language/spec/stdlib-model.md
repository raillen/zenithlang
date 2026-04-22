# Zenith Stdlib Model Spec

- Status: canonical closure spec
- Date: 2026-04-22
- Scope: public standard library architecture and MVP module policy

## Purpose

The stdlib must extend Zenith without weakening the language philosophy.

Stdlib APIs should be explicit, typed, predictable and readable. Expected failure returns `result<T, E>`. Expected absence returns `optional<T>`. Panic is reserved for broken invariants and direct assertion-like operations.

## Module Set

The current MVP stdlib module set is:

- `std.io`
- `std.bytes`
- `std.fs`
- `std.fs.path`
- `std.json`
- `std.math`
- `std.text`
- `std.random`
- `std.validate`
- `std.time`
- `std.format`
- `std.os`
- `std.os.process`
- `std.test`

`std.net` is designed as a next-wave module after the core stdlib wave.

## Import Policy

Stdlib modules are imported explicitly.

Canonical:

```zt
import std.io as io
import std.text as text
```

Core language traits and compiler intrinsics may be implicit only when they are part of the language semantic contract.

## Error Policy

Each side-effecting or fallible module owns its error type.

Examples:

- `io.Error`
- `fs.Error`
- `json.Error`
- `time.Error`
- `os.Error`
- `process.Error`

Rules:

- expected failure returns `result<T, Module.Error>`
- absence returns `optional<T>`
- timeout is an error variant, not panic and not absence
- invalid direct indexing may panic
- safe lookup APIs should return `optional<T>`

## Type Policy

Modules may introduce own public types when raw `text`, `int`, `list` or `map` would hide important semantics.

Required module-owned types include:

- `io.Input`
- `io.Output`
- `io.Error`
- `json.Value`
- `json.Object`
- `json.Array`
- `json.Kind`
- `json.Error`
- `time.Instant`
- `time.Duration`
- `time.Error`
- `fs.Metadata`
- `fs.Error`
- `format.BytesStyle`
- `os.Platform`
- `os.Arch`
- `os.Error`
- `process.Output`
- `process.Error`

## Naming Policy

Names should prefer clear verbs and nouns over abbreviated cleverness.

Rules:

- mutating methods are declared with `mut func`
- safe lookup functions should include names such as `get`, `find` or `try_` only when the distinction is needed
- whole-file byte APIs may be deferred until binary/runtime support is complete
- path operations live in `std.fs.path`, not `std.fs`

## Namespace Mutable State Policy (`public var`)

`public var` may exist in stdlib only when shared module state is a clear part of the contract.

Rules:

- prefer `public const` by default
- use `public var` only at namespace top-level
- allow external read through qualified import
- block external write outside the owner namespace
- `public` is visibility only; it is not `global`
- mutation should be exposed by explicit `public func` APIs
- tests must reseed/reset state to avoid order-dependent flakes

Current first-slice adoption:

- `std.random` exposes read-only observable module state to callers:
  - `seeded`
  - `last_seed`
  - `draw_count`
  - `stats()`

## Safe Collections API

Direct access remains strict:

```zt
const value: text = scores["Julia"]
```

If the key is missing, this is a runtime map-key panic.

Safe access must exist early:

```zt
const maybe_score: optional<int> = scores.get("Julia")
const maybe_item: optional<int> = values.get(3)
```

The exact namespace may be finalized during stdlib implementation, but the semantic rule is fixed: expected absence must not require panic.

## Implementation Gate

A stdlib function is complete only when it has:

- public signature in docs
- runtime/backend implementation
- typed error behavior if fallible
- behavior tests
- diagnostic tests when misuse is likely
- examples using canonical import style

## Deferred

Deferred until after the MVP stdlib wave:

- public generic stream abstraction
- async IO
- TLS
- websocket
- package registry integration
- optional dependencies and feature flags

