# Zenith Language Coherence Closure Plan

- Status: active closure plan
- Date: 2026-04-18
- Scope: work required for Zenith to match its manifesto at production quality

## Purpose

This plan turns the remaining language-coherence concerns into concrete implementation tracks.

A feature is not considered complete just because it parses or emits C. It must preserve the reading-first philosophy, have clear diagnostics, stable formatting, runtime safety and conformance coverage.

## Closure Tracks

### C1. Final Specs Without Ambiguity

Goal: keep a small set of canonical implementation-facing specs.

Canonical specs:

- `language/spec/surface-syntax.md`
- `language/spec/stdlib-model.md`
- `language/spec/runtime-model.md`
- `language/spec/backend-scalability-risk-model.md`
- `language/spec/diagnostics-model.md`
- `language/spec/formatter-model.md`
- `language/spec/project-model.md`
- `language/spec/compiler-model.md`
- `language/spec/tooling-model.md`
- `language/spec/implementation-status.md`

Definition of done:

- each user-visible rule has exactly one canonical home
- older decisions remain historical rationale, not competing specs
- known conflicts are resolved in the spec, not left as reader interpretation
- implementation status uses stable labels

### C2. Mandatory Formatter

Goal: make reading-first source style enforceable.

The formatter is mandatory for canonical Zenith code because `where`, named args, attrs, imports, structs, match and long blocks need consistent layout.

Definition of done:

- `zt fmt` formats a project deterministically
- formatter is idempotent
- conformance includes formatter golden tests
- formatter rules are documented in `formatter-model.md`

### C3. Excellent Diagnostics

Goal: make diagnostics part of the language UX, not afterthought tooling.

Definition of done:

- parser, semantic, lowering, project, runtime and formatter diagnostics share one structured model
- terminal renderer implements the detailed format from the spec
- diagnostic codes are stable
- common beginner mistakes have direct help
- conformance includes golden diagnostic fragments

### C4. Solid C Runtime

Goal: make the simple source model true at runtime.

Definition of done:

- RC, COW and cleanup rules are documented and tested
- RC cycle limitation is documented as a risk
- weak/handle/cycle-collection policy is required before cycle-prone APIs become stable
- optional/result wrappers use stack/in-place representation where practical
- `result<void, E>` success does not allocate in the final runtime model
- normal return, early return, `?` propagation and panic paths have defined cleanup behavior
- text, bytes, list, map, optional, result, structs and enums follow value semantics
- bounds checks, map-key checks and contract checks report structured runtime diagnostics

### C5. Value Semantics Proven By Tests

Goal: prove that managed values do not leak shared mutable aliasing.

Definition of done:

- behavior tests cover copy and mutation of list, map, struct, optional, result and enum payloads
- const collections reject observable mutation
- var collections preserve copy-on-write/value semantics
- runtime tests cover retain/release/refcount edge cases where visible to implementation

### C6. Runtime `where` Contracts Complete

Goal: make `where` real, not only parsed/checked.

Definition of done:

- field contracts run on construction
- field contracts run on field assignment when the field changes
- parameter contracts run at call boundaries
- contract failures use `runtime.contract`
- diagnostics show predicate, value and source location when available
- constructors with `where` fields do not secretly become `result`
- recoverable validation is expressed through explicit result-returning APIs

### C7. Enums With Payload And Strong Match

Goal: make state modeling viable without classical OOP.

Definition of done:

- payload enum layout is stable in HIR/ZIR/C
- qualified enum constructors work
- match over enum cases works
- payload binding works
- default cases work
- match without `case default` is exhaustive when enum type information is available
- missing enum cases produce compile-time diagnostics

### C8. Bytes, UTF-8 And Stdlib Base

Goal: make text/binary boundaries reliable before filesystem, process, JSON and network work expand.

Definition of done:

- `bytes` runtime is complete
- `hex bytes "..."` is fully covered by parser, typechecker, HIR/ZIR, C and tests
- `std.bytes` exists
- `std.text.to_utf8` and `std.text.from_utf8` exist
- invalid UTF-8 returns typed errors, not panic

### C9. Safe Collections API

Goal: keep strict indexing while giving users a safe default for expected absence.

Definition of done:

- direct list/map indexing remains strict and may panic on invalid access
- safe APIs return `optional<T>`
- examples prefer safe lookup when absence is ordinary
- panic-oriented access is documented as assertion-like direct access

### C10. Final CLI

Goal: make the toolchain predictable.

Required commands:

- `zt check`
- `zt build`
- `zt run`
- `zt test`
- `zt fmt`
- `zt doc check`

Definition of done:

- commands use project roots and `zenith.ztproj`
- errors are rendered through the same diagnostic renderer
- generated C output stays an implementation artifact
- CLI names match `tooling-model.md`

### C11. Functional ZDoc

Goal: make external docs useful enough that source files can stay clean.

Definition of done:

- `.zdoc` parser exists
- paired docs resolve `@target`
- guide docs resolve `@page`
- `@link` warnings exist
- public symbols missing docs produce warnings
- private docs are allowed but excluded from public output by default

### C12. Conformance Matrix

Goal: prevent "implemented" from becoming subjective.

Definition of done:

- every feature has parser, semantic, behavior, diagnostic and runtime coverage as applicable
- matrix status is updated when implementation status changes
- M16-style executable conformance remains green
- new tracks add their own conformance harnesses
- backend scalability risks are represented in the matrix

### C13. Backend Scalability Risk Gate

Goal: keep C target scalability risks visible before stable release.

Definition of done:

- monomorphization uses canonical type keys and instance caching
- excessive monomorphization can be reported or diagnosed
- optional/result representation is stack-first where practical
- heap-first wrappers are tracked as performance debt
- RC cycle policy is documented and gated before callback/UI/game APIs
- match exhaustiveness and runtime `where` risks are covered by conformance

## Execution Order

Recommended order:

1. C1 specs
2. C13 backend scalability risk gate
3. C3 diagnostics model and renderer
4. C4 runtime ownership hardening
5. C5 value semantics tests
6. C6 runtime `where`
7. C7 enums and match
8. C8 bytes, UTF-8 and stdlib base
9. C9 safe collections API
10. C2 formatter
11. C10 CLI final
12. C11 ZDoc
13. C12 final conformance matrix

Diagnostics and runtime come early because they affect every later feature.

