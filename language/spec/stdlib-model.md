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
- `std.list`
- `std.map`
- `std.random`
- `std.validate`
- `std.time`
- `std.format`
- `std.lazy`
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

## Test Helper Policy

`std.test` is a small helper module for tests.

It complements `attr test` and core `check(...)`.
It must not replace test discovery.

Public helper categories:

- explicit outcomes: `fail(...)`, `skip(...)`
- bool checks: `is_true(...)`, `is_false(...)`
- simple comparisons: `equal_int(...)`, `equal_text(...)`, `not_equal_int(...)`, `not_equal_text(...)`

Comparison failures should include expected and received values when that helps the user fix the test.

## Safe Collections API

Direct access remains strict:

```zt
const value: text = scores["Julia"]
```

If the key is missing, this is a runtime map-key panic.

Safe access must exist early:

```zt
const score_key: text = "Julia"
const maybe_score: optional<int> = scores.get(score_key)
const maybe_item: optional<int> = values.get(3)
const has_julia: bool = map.has_key(scores, score_key)
```

`std.list` and `std.map` also expose small compiler-known helpers:

- `list.is_empty(items)`
- `map.is_empty(values)`
- `map.has_key(values, key)`

The semantic rule is fixed: expected absence checks must not require panic.

## Higher-Order Collection Helpers

R3.M7 introduces a narrow higher-order subset in `std.collections`.

Current stable helpers:

- `map_int(values: list<int>, mapper: func(int) -> int) -> list<int>`
- `filter_int(values: list<int>, predicate: func(int) -> bool) -> list<int>`
- `reduce_int(values: list<int>, initial: int, reducer: func(int, int) -> int) -> int`

Rules:

- helpers are concrete for `int` in this cut
- generic HOFs are deferred
- callbacks use normal `func(...)` closure values
- callbacks may capture immutable values
- hot-path users should benchmark before replacing explicit loops

## Explicit Lazy Helpers

R3.M8 introduces `std.lazy` as a narrow explicit lazy module.

Current helpers:

- `once_int(thunk: func() -> int) -> lazy<int>`
- `force_int(value: lazy<int>) -> int`
- `is_consumed_int(value: lazy<int>) -> bool`

Rules:

- users must import the module explicitly
- `lazy<int>` is one-shot in this cut
- there is no implicit lazy in collection helpers
- reusable lazy and lazy iterators remain future work

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

