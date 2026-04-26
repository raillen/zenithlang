# Self-Hosting Roadmap v1

> Audience: maintainer
> Status: draft
> Surface: internal
> Source of truth: planning

## Decision

Zenith should not become fully self-hosted now.

The current goal is to prepare the language for self-hosting through small, verifiable tools written in Zenith.

Self-hosting is a maturity test, not an immediate compiler rewrite.

## Why This Exists

A self-hosted compiler is only useful when the language is stable enough to maintain itself.

If Zenith moves too early, every language change can break both:

- the user-facing language;
- the compiler written in that same language.

That would slow the project down and create avoidable risk.

The safer path is gradual.

## Guiding Principles

- Keep the current compiler as the stable implementation base.
- Write small Zenith tools before writing compiler subsystems.
- Treat every self-hosting slice as a test of language maturity.
- Do not add language features only because self-hosting would be easier.
- Prefer explicit data structures and readable diagnostics.
- Keep docs, tests and fixtures in sync with each slice.

## Non-Goals For v1

- Do not rewrite the current compiler in Zenith.
- Do not replace the C backend because of this roadmap.
- Do not make Zenith depend on itself to build the official toolchain.
- Do not introduce async, raw pointers or advanced metaprogramming only for self-hosting.
- Do not publish experimental self-hosted tools as stable user-facing tooling.

## Readiness Gates

Zenith is ready for real self-hosting only when these areas are stable:

| Area | Requirement |
| --- | --- |
| Syntax | File shape, declarations, functions, match and blocks are stable. |
| Modules | Namespace/import rules are stable and documented. |
| Visibility | `public`, `public const`, `public var` and private-by-default are stable. |
| Types | `optional`, `result`, `void`, `mut`, `dyn` and `lazy` are documented and tested. |
| Stdlib | Text, bytes, collections, filesystem, JSON, CLI and diagnostics support are usable. |
| Tooling | `zt check`, package checks and LSP basics are reliable. |
| Tests | Behavior, semantic and stdlib fixtures catch regressions. |
| Diagnostics | Errors are precise enough to debug code written in Zenith itself. |

## Phase SH1: Zenith Tools First

Goal: prove Zenith can build small useful tools without changing the compiler architecture.

Candidate tools:

- docs link checker subset;
- manifest validator for packages;
- simple project metadata linter;
- fixture index generator;
- small CLI that reads files and prints structured diagnostics.

Validation:

- tool runs through `zt run`;
- tool has behavior fixture;
- failure output is readable;
- no new language feature is required.

Exit criteria:

- at least 3 small internal tools written in Zenith;
- each tool has tests or golden output;
- stdlib gaps are documented before adding features.

## Phase SH2: Lexer Prototype

Goal: write a non-authoritative lexer in Zenith.

Scope:

- read `.zt` text;
- split tokens;
- preserve line and column;
- report invalid characters;
- support comments and strings.

Non-scope:

- no official compiler replacement;
- no full parser yet;
- no semantic analysis.

Why this matters:

- tests text handling;
- tests list/struct ergonomics;
- tests diagnostic data models;
- exposes missing stdlib helpers.

Exit criteria:

- can tokenize a curated subset of fixtures;
- diagnostics include file, line, column and message;
- output is deterministic.

## Phase SH3: Parser Prototype

Goal: parse a small Zenith subset into an AST-like data model.

Scope:

- namespace-first rule;
- imports;
- const/var declarations;
- function signatures;
- simple blocks;
- `match` with multiple case values;
- optional/result patterns.

Non-scope:

- no full language grammar;
- no official parser swap;
- no code generation.

Exit criteria:

- parser handles curated syntax fixtures;
- parser rejects malformed namespace placement;
- AST printing is stable enough for golden tests.

## Phase SH4: Diagnostics Prototype

Goal: prove Zenith can express compiler-quality diagnostics.

Scope:

- diagnostic struct;
- severity;
- code;
- span;
- message;
- hint;
- rendering for terminal output.

Exit criteria:

- invalid fixtures produce stable diagnostic text;
- docs explain the diagnostic model;
- output is accessible and concise.

## Phase SH5: Semantic Toy Checker

Goal: check a very small subset of types and names.

Scope:

- duplicate declaration detection;
- unknown symbol detection;
- simple type mismatch;
- namespace/import consistency;
- private/public boundary checks.

Non-scope:

- no full generics;
- no full trait solving;
- no official checker replacement.

Exit criteria:

- checker finds real mistakes in curated fixtures;
- false positives are tracked;
- no hidden compiler dependency on the prototype.

## Phase SH6: Bootstrap Feasibility Report

Goal: decide whether a real self-hosted compiler stage is justified.

Inputs:

- SH1 to SH5 results;
- stdlib gap list;
- performance observations;
- diagnostics quality;
- language stability review;
- package system maturity.

Possible outcomes:

| Outcome | Meaning |
| --- | --- |
| Continue prototypes | Zenith is improving, but not ready for compiler migration. |
| Start bootstrap compiler | The language is stable enough for a separate experimental compiler. |
| Stop self-hosting track | The cost is too high for current project goals. |

## Maturity Risks

| Risk | Mitigation |
| --- | --- |
| The language grows just to support self-hosting | Require a language decision for any new feature. |
| Experimental tools become mistaken for official tooling | Mark all self-hosted tools as experimental. |
| Stdlib gaps turn into rushed language features | Fill stdlib first when possible. |
| The official compiler becomes harder to change | Keep the current compiler authoritative until a formal bootstrap decision. |
| Docs drift from implementation | Update reference docs, checklist and tests together. |

## Minimum Language Features Needed

These features should be mature before real self-hosting:

- structs and enums;
- lists and maps;
- `optional<T>`;
- `result<T,E>`;
- `match`, including multiple values in one case;
- public/private namespace semantics;
- readable diagnostics;
- filesystem and text APIs;
- deterministic tests.

## Features That Should Stay Deferred

Do not block this roadmap on:

- full async/await;
- raw pointers;
- advanced macro system;
- language-level channels;
- language-level task runtime;
- full reflection.

If these become necessary, the prototype should document why.

They should not be added by assumption.

## Success Definition

This roadmap succeeds if Zenith becomes better at writing real tools.

It does not need to produce a production self-hosted compiler in v1.

The first win is evidence:

- which parts of the language are solid;
- which stdlib APIs are missing;
- which diagnostics are weak;
- which syntax choices help or hurt implementation work.
