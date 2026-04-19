# Zenith Next Surface Implementation Status

- Status: M19 in progress
- Date: 2026-04-18
- Scope: current C-backed compiler implementation under ``

This document records how the canonical specs map to the current compiler. It is intentionally stricter than `language/spec/`: the specs say what the language accepts, while this file says what the implementation can parse, validate and execute through the C backend today.

## Status Labels

This document follows the vocabulary defined in `language/spec/implementation-status.md`.

Primary labels used in this snapshot:

- `Executable`: parsed, semantically checked, lowered through HIR/ZIR and emitted to C behavior tests.
- `Semantic`: parsed and semantically checked, but not yet part of the executable C subset.
- `Parsed`: represented in AST/HIR shape, but not semantically closed or executable.
- `Deferred`: accepted language direction, but not part of the current implementation cut.
- `Risk`: architecture risk requiring explicit mitigation before stable release.

Additional canonical labels (`Spec`, `Lowered`, `Emitted`, `Runtime`, `Conformant`, `Rejected`) may be used as this document evolves.

## Parser Coverage

The parser currently accepts the MVP surface forms below:

- Files with mandatory `namespace` and imports after namespace.
- `import path` and `import path as alias`.
- Top-level `func`, `public func`, `struct`, `public struct`, `trait`, `public trait`, `apply Type`, `apply Trait to Type`, `enum` and `extern`.
- Function parameters, named call arguments and default parameter expressions.
- Generic parameter lists and `where TypeParam is TraitType` constraints on `func`, `struct`, `trait`, `enum` and `apply`.
- `const`, `var`, assignment, field assignment and index assignment.
- `if`, `else if`, `else`, `while`, `for`, `repeat N times`, `break`, `continue`, `match`, `case`, `case default` and comma-separated match patterns.
- Calls, method calls, field access, indexing, slice expressions, postfix `?`, grouping, unary operators, arithmetic, comparison and logical operators.
- Text, multiline text, `hex bytes "..."`, bool, integer, float, list literal, map literal, `none`, `success(...)` and `error(...)`.
- Qualified names such as `user_types.User` in type positions and call/member positions.

## Executable C Subset

The behavior matrix in `tests/behavior/MATRIX.md` defines the executable subset validated by M16.

Current executable features:

- Project manifest through the canonical `[project]`, `[source] root`, `[app] entry`, `[lib] root_namespace`, `[build]`, `[test]` and `[zdoc]` sections, with namespace/path validation and multi-file aggregation.
- Imports by fully qualified namespace and alias-qualified access.
- Import cycles rejected with project-level diagnostics.
- `func main() -> int` as the current C emitter entrypoint.
- Direct function calls, positional args, named args in declaration order and trailing defaults.
- `int`, `bool`, `float`, `text`, `bytes` and `void` returns in user functions for the supported backend paths.
- `const`, `var`, reassignment rules and mutation diagnostics.
- `if`, `else if`, `else`, `while`, `repeat`, `match`, `break`, `continue`, `for item in list/text` and `for key, value in map` for the behavior-tested combinations.
- `struct` construction, field defaults, field reads and field updates.
- Inherent methods through `apply Type`.
- Trait methods through `apply Trait to Type`.
- Mutating methods are declared with `mut func`; legacy suffix syntax is not accepted.
- `list<int>`, `list<text>`, `map<text,text>`, `optional<T>` and `result<T,E>` for the behavior-tested combinations.
- Indexing, assignment, slicing and `len(...)` for supported text/list/map combinations; `hex bytes "..."`, `len(bytes)`, bytes indexing and bytes slicing are covered by `bytes_hex_literal`.
- Safe lookup `map.get(key)` for `map<text,text>` returning `optional<text>` in the current backend subset.
- Safe lookup `list.get(index)` for `list<int>` and `list<text>` returning `optional<T>` in the current backend subset.
- `none`, `success(...)`, `error(...)` and `result<T,E>` `?` propagation in const/var initialization inside result-returning functions.
- Core trait constraints for `Equatable<Item>`, `Hashable<Item>` and `TextRepresentable<Item>` in semantic checks.
- Explicit numeric conversions and checked integer overflow diagnostics in semantic tests.

## Semantic But Not Executable

These forms are intentionally not treated as C-backend-complete in the current cut:

- Generic collection and map combinations beyond the current C-supported monomorphic paths.
- Generic declarations beyond the currently behavior-tested monomorphic lowering path.
- Generic constraints beyond compile-time validation; they do not yet produce runtime contract machinery.
- Value-level `where` refinements as generated runtime boundary checks.
- Optional `?` propagation and expression-level unwrap outside the current `result<T,E>` const/var initialization subset.
- `enum` declaration and exhaustive enum-oriented pattern matching in generated C.
- Collection APIs beyond `len(...)`, strict indexing (`list[index]`/`map[key]`) and behavior-tested safe lookups (`map.get` and `list.get`) in the current subset.

## Deferred Out Of The Compiler Cut

The following are not compiler frontend/backend blockers for M16:

- CLI process API and command-line arguments.
- Final stdlib module implementation and public package surface.
- `std.bytes` implementation and UTF-8 conversion APIs in `std.text`.
- `zpm`, dependency resolution and package version solving.
- ZDoc parser/linker integration.
- Host API beyond the current runtime hooks.
- Platform-specific targets beyond C/native build through `gcc`.

## Architecture Risks

These items are tracked as `Risk` in the implementation status model. They are not ordinary deferred features because they can affect correctness, performance or the language's predictability promise.

- RC cycles: the runtime currently relies on RC without a completed cycle strategy. Before rich callbacks, UI graphs, game object graphs or stored reference-like APIs become stable, the project must choose `weak<T>`, handles/arenas, constrained ownership graphs or cycle collection.
- Monomorphization code bloat: generics for the C target require canonical type keys, instance caching, deduplication and diagnostics or build reports for excessive instantiation.
- Heap-first wrappers: `optional<T>` and `result<T,E>` should become stack/in-place where practical. Heap-first wrappers are performance debt unless required by managed payloads, escape, oversized payloads or bootstrap constraints.
- Runtime `where`: value-level contracts must be generated as runtime checks for construction, field assignment and parameter boundaries. Constructors must not secretly become `result<T,E>`.
- Enum match exhaustiveness: enum `match` without `case default` must diagnose missing cases when enum definitions are known.

Mitigation details live in `language/spec/backend-scalability-risk-model.md`, `language/decisions/078-backend-scalability-and-runtime-risk-policy.md` and roadmap milestones M24, M26, M27 and M32.

## Diagnostics Contract

The diagnostics contract is considered closed for the current compiler cut when:

- Parser diagnostics include source name, line and column when available.
- Semantic diagnostics include stage, diagnostic code and span for common user mistakes.
- Lowering diagnostics use the same diagnostic list infrastructure and preserve source spans where the AST/HIR node has one.
- Conformance checks assert essential fragments rather than byte-for-byte full diagnostic output, because file separators and compiler toolchain details can vary across hosts.

Essential diagnostic fixtures live under `tests/fixtures/diagnostics/`.

## Notes

This file should be updated whenever `language/spec/` changes or when a parsed-but-deferred form becomes executable in the C backend.


