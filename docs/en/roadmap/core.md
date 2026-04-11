# Consolidation Roadmap: Zenith Core

This document tracks the technical consolidation of the Zenith language core and records the path that led to Ascension Final.

## 1. Overview

I maintain Zenith as a language focused on cognitive accessibility, syntactic clarity, and technical maturity, without sacrificing performance or hospitable reading.

## 2. Phases 1 to 3: The Heart (v0.2.5) - Completed

Focus: establishing the grammar, type system, and basic infrastructure.

- [X] ADTs and `match`
- [X] Type system with `trait`, generics, and `where`
- [X] Integrated async/await
- [X] Interoperability with `native lua`
- [X] Lambdas, spread, slicing, and surface ergonomics

## 3. Phase 4: Tooling and Foundation (v0.2.5) - Completed

Focus: allowing Zenith to interact with the real world.

- [X] Stable CLI with build and run support
- [X] Project system via `.ztproj`
- [X] Base libraries such as `std.os`, `std.fs`, and `std.json`

## 4. Phase 5: Utilities and Ecosystem (v0.2.6 - v0.2.7) - Completed

Focus: expanding tooling for practical use.

- [X] `std.time` and `std.text`
- [X] ZPM for external dependencies
- [X] Interactive manual and support resources

## 5. Phase 6: Shielding and Quality (v0.2.8 - v0.2.9) - Completed

Focus: shielding the platform before self-hosting.

- [X] ZTest integrated into the CLI
- [X] Visual reports and data export
- [X] Parallel execution and suite isolation
- [X] Snapshot testing and broad global coverage

## 6. Phase 7: Zenith Ascension (v0.3.0 - v0.3.5) - Open

Focus: self-hosting, optimization, and total pipeline sovereignty.

- [X] **v0.3.0 (Sovereign)**: Ascension Engine Finalized (FFI, Multi-target, Bundle).
- [X] **v0.3.1 (ZDoc)**: .zdoc file support and consolidated CLI purity.
- [ ] **v0.3.2 (Lexer)**: Lexical analyzer rewritten in pure Zenith.
- [ ] **v0.3.3 (Parser)**: Pratt Parser algorithm ported to Zenith.
- [ ] **v0.3.4 (Binder)**: Symbol table and native semantic resolution.
- [ ] **v0.3.5 (Emitter)**: Code generator (Codegen) operating in Zenith.

---
## 7. Studies that continue after Ascension

These items record the transition to the Zenith v0.3.1 ecosystem:

- [X] **Zenith Doc-Linking (ZDoc)**: Support for external `.zdoc` files for decoupled documentation, using the `@target` tag to associate prose with symbols without polluting the source code.
- [X] **Native Regex Module**: Fluent Builder implementation for constructing rhythmic and readable patterns.
- [X] **Interactive Manual (ZMan)**: Refactoring to pure Zenith, integrating the Regex system for colorization without reliance on native blocks.
- [X] **CLI Purity Expansion**: Drastic reduction of `native lua` in central tools. ZPM, ZTest, and ZMan are now orchestrated in sovereign Zenith.

## 8. Next Steps (Transcendence v1.0)

With the Ascension Engine Finalized and the CLI Purified, focus shifts to the visual and industrial foundation:

- [ ] **Zenith Intersect (UI)**: Declarative and reactive framework.
- [ ] **Borealis (Games)**: Industrial game engine over Raylib.
- [ ] **Atlas (Data)**: Type-safe ORM for SQLite/Postgres.

*Updated: 2026-04-11 (Zenith v0.3.1)*
