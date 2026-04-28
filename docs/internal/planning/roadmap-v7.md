# Zenith Unified Roadmap v7

> Replaces: roadmap-v1 through v6, manual-memory-roadmap-v1,
> production-readiness-roadmap-v1, self-hosting-roadmap-v1,
> language-maturity-host-async-roadmap-v1, documentation-roadmap-v1,
> stabilization-roadmap, m9-feature-pack-a-design, m10-stdlib-priorities.
>
> Status: active
> Created: 2026-04-27
> Origin: block-by-block language design session

---

## Principles

1. ARC is the default. Manual memory is opt-in and future.
2. Explicit over implicit. The reader should never guess.
3. Reading-first, neurodivergent-friendly. Words over symbols.
4. No feature enters without a real use case.
5. Diálogo futuro items are recorded but carry zero commitment.

---

## Phase 0 — Stabilization (0.4)

Priority: **critical**. Nothing else ships until this is green.

| ID | Item | Status |
|----|-------|--------|
| S.01 | Repo hygiene: clean clone builds on Windows and Linux | pending |
| S.02 | CI pipeline: Linux + Windows, `build.py` + smoke suite | pending |
| S.03 | Surface freeze: mark every public feature as stable / experimental / internal | pending |
| S.04 | Diagnostic audit: every error is action-first, no raw compiler internals | pending |

---

## Phase 1 — Language Features (0.5)

### 1A — `using` statement (resource cleanup)

| ID | Item | Status |
|----|-------|--------|
| L.01 | Add `using` keyword to lexer | done |
| L.02 | Parser: `using x = expr ... end` (block form) | done |
| L.03 | Parser: `using x = expr` (flat form, lives to end of function) | done |
| L.04 | Parser: `using x = expr then cleanup_expr` (custom cleanup) | done |
| L.05 | Binder/checker: validate `using` bindings | done |
| L.06 | HIR/ZIR: lower `using` to internal `defer` with LIFO order | done |
| L.07 | C emitter: emit cleanup calls on all exit paths (return, ?, panic) | done |
| L.08 | Tests: basic, early return, error propagation, LIFO order, loop per-iteration | done |
| L.09 | Documentation: usage guide with examples | done |

### 1B — Destructuring in `match`

| ID | Item | Status |
|----|-------|--------|
| L.10 | Parser: `case variant(binding)` for enum/result/optional | pending |
| L.11 | Parser: `case variant(a, b)` for multi-field payloads | pending |
| L.12 | Binder: resolve destructured bindings in match arms | pending |
| L.13 | Checker: validate payload types against enum definition | pending |
| L.14 | HIR/ZIR: lower destructured match | pending |
| L.15 | C emitter: emit extraction from tagged union | pending |
| L.16 | Tests: optional, result, custom enum, nested | pending |

### 1C — `f"..."` string interpolation

| ID | Item | Status |
|----|-------|--------|
| L.17 | Parser: accept `f` (length 1) as interpolation prefix | pending |
| L.18 | Keep `fmt` as deprecated alias with diagnostic | pending |
| L.19 | Formatter: output `f"..."` instead of `fmt "..."` | pending |
| L.20 | Update all tests, examples, and docs | pending |

### 1D — `range()` builtin

| ID | Item | Status |
|----|-------|--------|
| L.21 | Define `range(start, end)` and `range(start, end, step)` as builtins | pending |
| L.22 | Only inclusive: `range(0, 10)` = 0 to 10 inclusive | pending |
| L.23 | Integrate with `for i in range(...)` | pending |
| L.24 | Tests: basic, step, negative step, empty range | pending |

### 1E — Closures with mutable capture

| ID | Item | Status |
|----|-------|--------|
| L.25 | Parser: `func(...) [mut var1, mut var2]` capture list syntax | pending |
| L.26 | Checker: validate mutable captures, prevent aliasing | pending |
| L.27 | C emitter: emit captured-by-reference with proper ARC | pending |
| L.28 | Tests: mutable capture, multiple captures, nested closures | pending |

### 1F — Trait default implementations

| ID | Item | Status |
|----|-------|--------|
| L.29 | Parser: allow function bodies inside `trait ... end` | pending |
| L.30 | Binder: distinguish required vs default methods | pending |
| L.31 | Checker: use default when `apply` omits a method | pending |
| L.32 | C emitter: emit default method as fallback | pending |
| L.33 | Tests: default used, default overridden, default calling required | pending |

---

## Phase 2 — Builtins and Runtime (0.5)

| ID | Item | Status |
|----|-------|--------|
| B.01 | `print(value)` — stdout + newline, no result | pending |
| B.02 | `read()` — stdin line, returns text | pending |
| B.03 | `len(value)` — polymorphic: list, map, set, text, bytes | pending |
| B.04 | `debug(value)` — print file, line, variable name, type, value | pending |
| B.05 | `type_name(value)` — returns type as text | pending |
| B.06 | `size_of<T>()` — returns byte size of type | pending |
| B.07 | `range(start, end)` / `range(start, end, step)` — inclusive range | pending |

---

## Phase 3 — Collections and Specializations (0.5-0.6)

### 3A — `set<T>` type

| ID | Item | Status |
|----|-------|--------|
| C.01 | Add `set` keyword to lexer | pending |
| C.02 | Runtime: hash-based set implementation | pending |
| C.03 | API: `set.of()`, `set.empty()`, `set.add()`, `set.remove()`, `set.has()`, `set.len()` | pending |
| C.04 | API: `set.union()`, `set.intersect()`, `set.difference()` | pending |
| C.05 | Iteration: `for item in my_set` | pending |
| C.06 | Tests and docs | pending |

### 3B — List/collection helpers

| ID | Item | Status |
|----|-------|--------|
| C.07 | `list.first(items)` -> `optional<T>` | pending |
| C.08 | `list.last(items)` -> `optional<T>` | pending |
| C.09 | `list.rest(items)` -> `list<T>` | pending |
| C.10 | `list.skip(items, count)` -> `list<T>` | pending |

### 3C — Primitive type specializations

| ID | Item | Status |
|----|-------|--------|
| C.11 | Emitter rule: if T is primitive, emit contiguous array (no ARC per element) | pending |
| C.12 | Covers: list<int>, list<float>, list<bool>, list<i8-i64>, list<u8-u64> | pending |
| C.13 | Covers: optional<int>, optional<float>, optional<i8-i64>, optional<u8-u64> | done |
| C.14 | Benchmarks: before/after for numeric lists | done |

---

## Phase 4 — Standard Library Expansion (0.6)

### 4A — `std.fs`

| ID | Item | Status |
|----|-------|--------|
| X.01 | `fs.exists`, `fs.is_file`, `fs.is_dir` | done |
| X.02 | `fs.create_dir`, `fs.create_dir_all` | done |
| X.03 | `fs.remove_file`, `fs.remove_dir` | done |
| X.04 | `fs.rename`, `fs.copy` | done |
| X.05 | `fs.list_dir`, `fs.file_size` | done |

### 4B — `std.math`

| ID | Item | Status |
|----|-------|--------|
| X.06 | `math.abs`, `math.pow`, `math.sqrt` | done |
| X.07 | `math.min`, `math.max`, `math.clamp` | done |
| X.08 | `math.floor`, `math.ceil`, `math.round` | done |
| X.09 | `math.sin`, `math.cos`, `math.tan` | done |
| X.10 | `math.pi`, `math.e` | done |

### 4C — `std.time`

| ID | Item | Status |
|----|-------|--------|
| X.11 | `time.now()` -> `time.Instant`; `time.now_ms()` -> int (ms timestamp) | done |
| X.12 | `time.elapsed(start, finish)` -> int (ms) | done |
| X.13 | `time.sleep(duration)` and `time.sleep_ms(ms)` | done |

### 4D — `std.regex` (portable subset; PCRE2/RE2 later if needed)

| ID | Item | Status |
|----|-------|--------|
| X.14 | `regex.compile(pattern)` -> `result<Regex, Error>` | done |
| X.15 | `regex.is_match(pattern, input)` -> bool | done |
| X.16 | `regex.find_all(pattern, input)` -> `list<text>` | done |

### 4E — Error helpers

| ID | Item | Status |
|----|-------|--------|
| X.17 | `.or_return(value)` on optional — early return pattern | done |
| X.18 | `.or_wrap(context)` on result — error context chaining | done |

---

## Phase 5 — Tooling and Ecosystem (0.7)

| ID | Item | Status |
|----|-------|--------|
| T.01 | VSCode extension on Marketplace (LSP + syntax highlighting) | pending |
| T.02 | `zt test --filter <name>` — test filtering | pending |
| T.03 | Test runner DX: test name, duration, stacktrace in output | pending |
| T.04 | `test.throws(func)` for expected panics | pending |
| T.05 | C-Binding Pack: sqlite3 and/or libcurl working example | pending |
| T.06 | REPL: `zt repl` via compile-and-run per expression | pending |
| T.07 | ZPM: lock file for reproducible builds | pending |
| T.08 | ZPM: semantic versioning for dependencies | pending |
| T.09 | Neovim/Zed/Helix LSP config snippets published | pending |

---

## Phase 5B — Learning and Examples (0.7)

| ID | Item | Status |
|----|-------|--------|
| E.01 | "Learn Zenith in 30 min" tutorial (English) | pending |
| E.02 | Example: CLI calculator | pending |
| E.03 | Example: file processor | pending |
| E.04 | Example: TODO app | pending |
| E.05 | Example: simple game (Borealis) | pending |
| E.06 | Example: data parser | pending |
| E.07 | Cookbook: grow organically with stdlib expansion | pending |

---

## Phase 5C — Benchmarks (0.7, internal only)

| ID | Item | Status |
|----|-------|--------|
| K.01 | Create `benchmarks/` directory with infrastructure | pending |
| K.02 | Micro: fibonacci, sort, string ops, allocation | pending |
| K.03 | Compilation time: `zt build` for N-line projects | pending |
| K.04 | Runtime: compare emitted C vs hand-written C | pending |
| K.05 | Cross-language: same problems in Go, Python, Kotlin | pending |
| K.06 | Use for regression detection, NOT for marketing | pending |

---

## Phase 6 — Self-hosting Bootstrap (0.8)

| ID | Item | Status |
|----|-------|--------|
| H.01 | Write 3 real tools in Zenith (link checker, manifest validator, fixture index) | pending |
| H.02 | Validate language gaps found during dogfooding | pending |
| H.03 | Fix gaps and iterate on stdlib based on real usage | pending |

---

## Phase 7 — Release Engineering (0.9)

| ID | Item | Status |
|----|-------|--------|
| R.01 | SemVer policy: define breaking change rules | pending |
| R.02 | Deprecation workflow: old syntax emits warning for 1 release | pending |
| R.03 | Borealis clean clone dogfood | pending |
| R.04 | English docs as canonical, other languages best-effort | pending |
| R.05 | License audit: verify all dependencies compatible with Apache-2.0 OR MIT | done |
| R.06 | Release candidate freeze | pending |

---

## Phase 8 — 1.0

| ID | Item | Status |
|----|-------|--------|
| V.01 | All stable features documented and tested | pending |
| V.02 | No known P0/P1 bugs | pending |
| V.03 | Clean install experience on Windows, Linux, macOS | pending |
| V.04 | Public announcement | pending |

---

## Diálogo Futuro (recorded, zero commitment)

These items were discussed and intentionally deferred. They require a new
design session before any work begins.

| Topic | Notes |
|-------|-------|
| `std.net` (HTTP, TCP) | Needs async/blocking decision |
| `std.mem.Allocator`, arenas | Only if real case (Borealis/self-hosting) proves need |
| Debug allocator | Depends on Allocator |
| Containers with allocator | Depends on Allocator |
| `std.unsafe`, raw pointers | Only if FFI 1.0 proves insufficient |
| `owned<T>`, `borrow<T>`, lifetimes | Changes language identity; requires full RFC |
| `async/await`, `task`, `channel`, `Shared<T>` | Concurrency deferred; host runtimes handle this |
| Compiler optimizations (ARC elision, incremental) | When benchmarks show need |
| JS transpilation (ZIR -> JS) | After stdlib + production readiness |
| LLVM backend | Post-1.0 |
| WASM via Emscripten | Low-cost exploration possible |
| Web playground | Needs backend exec or WASM |
| ZPM registry web UI | Nice-to-have, not blocking |
| Self-hosting lexer/parser (SH2-SH6) | After SH1 dogfooding validates gaps |
| `Disposable` trait for `using` auto-cleanup | After `using` is validated |

---

## Decisions Not Entering the Language

These were explicitly discussed and rejected.

| Feature | Reason |
|---------|--------|
| `char` type | `text` with helpers suffices; no real use case |
| `uint` standalone | `u64` exists; avoids casting hell |
| Tuples | Structs are the Zenith answer |
| `?.` safe navigation | Dense symbol, hides control flow |
| `??` null coalescing | `.or(default)` already solves this |
| Implicit return | Anti-explicit, minimal gain |
| Rest operator (`...`) | `list.rest()` / `list.skip()` solve this |
| `unpack` destructuring on `const` | `pos.x` is already explicit |
| `try/catch` | Zenith uses `result<T,E>` + `?` |
| `optional<bool>` specialization | Rare case, not worth effort |
