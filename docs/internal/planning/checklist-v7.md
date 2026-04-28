# Zenith Unified Checklist v7

> Replaces: checklist-v1 through v6, manual-memory-checklist-v1,
> production-readiness-checklist-v1, self-hosting-checklist-v1,
> language-maturity-host-async-checklist-v1, documentation-checklist-v1.
>
> Status: active
> Created: 2026-04-27
> Derived from: roadmap-v7.md

## How to use

1. Do not mark an item without evidence in code, docs, or tests.
2. Each phase has gate criteria. Do not start the next phase until gates are met.
3. If a feature hurts readability of common code, return to design.
4. Preserve the rule: manual memory is optional and future.

---

## Gate criteria (apply to every phase)

- [ ] `python build.py` green.
- [ ] `./zt.exe check zenith.ztproj --all --ci` green.
- [ ] `python run_suite.py smoke --no-perf` green.
- [ ] Behavior tests for each new public surface.
- [ ] Negative tests for each new error.
- [ ] Spec or decision updated before changing semantics.

---

## Phase 0 — Stabilization (0.4)

- [ ] S.01 — Repo hygiene: clean clone builds on Windows and Linux.
- [ ] S.02 — CI pipeline: Linux + Windows green.
- [ ] S.03 — Surface freeze: every feature marked stable / experimental / internal.
- [ ] S.04 — Diagnostic audit: every error is action-first.

Gate: clean clone + CI green + surface freeze doc exists.

---

## Phase 1 — Language Features (0.5)

### 1A — `using` statement

- [x] L.01 — `using` keyword added to lexer (`ZT_TOKEN_USING`).
- [x] L.02 — Parser: block form `using x = expr ... end`.
- [x] L.03 — Parser: flat form `using x = expr` (scope = enclosing function).
- [x] L.04 — Parser: `using x = expr then cleanup_expr`.
- [x] L.05 — Binder/checker validates `using` bindings.
- [x] L.06 — HIR/ZIR: lowered to internal `defer`, LIFO order.
- [x] L.07 — C emitter: cleanup on all exit paths (return, ?, panic).
- [x] L.08 — Tests: basic, early return, error propagation, LIFO, loop iteration.
- [x] L.09 — Docs: usage guide with examples.

### 1B — Destructuring in `match`

- [x] L.10 — Parser: `case variant(binding)`.
- [x] L.11 — Parser: `case variant(a, b)` multi-field.
- [x] L.12 — Binder: resolve destructured bindings.
- [x] L.13 — Checker: validate payload types.
- [x] L.14 — HIR/ZIR: lower destructured match.
- [x] L.15 — C emitter: extract from tagged union.
- [x] L.16 — Tests: optional, result, custom enum, nested.

### 1C — `f"..."` interpolation

- [x] L.17 — Parser: `f` (length 1) as interpolation prefix.
- [x] L.18 — `fmt` accepted with deprecation diagnostic.
- [x] L.19 — Formatter outputs `f"..."`.
- [x] L.20 — Tests, examples, and docs updated.

### 1D — `range()` builtin

- [x] L.21 — `range(start, end)` and `range(start, end, step)` defined.
- [x] L.22 — Inclusive only.
- [x] L.23 — Works with `for i in range(...)`.
- [x] L.24 — Tests: basic, step, negative step, empty.

### 1E — Closures with mutable capture

- [x] L.25 — Parser: `[mut var]` capture list.
- [x] L.26 — Checker: validate mutable captures.
- [x] L.27 — C emitter: captured-by-reference with ARC.
- [x] L.28 — Tests: mutable, multiple, nested.

### 1F — Trait default implementations

- [x] L.29 — Parser: function bodies in `trait ... end`.
- [x] L.30 — Binder: required vs default methods.
- [x] L.31 — Checker: fallback to default when `apply` omits.
- [x] L.32 — C emitter: default method fallback.
- [x] L.33 — Tests: default used, overridden, default calls required.

Gate: all L.* items green + gate criteria.

---

## Phase 2 — Builtins and Runtime (0.5)

- [x] B.01 — `print(value)` builtin (stdout + newline, no result).
- [x] B.02 — `read()` builtin (stdin line -> text).
- [x] B.03 — `len(value)` builtin (polymorphic).
- [x] B.04 — `debug(value)` builtin (file, line, name, type, value).
- [x] B.05 — `type_name(value)` builtin (-> text).
- [x] B.06 — `size_of<T>()` builtin (-> int, bytes).
- [x] B.07 — `range()` registered as builtin function.

Gate: all B.* items green + gate criteria.

---

## Phase 3 — Collections and Specializations (0.5-0.6)

### 3A — `set<T>`

- [x] C.01 — `set` keyword in lexer.
- [x] C.02 — Hash-based set in runtime.
- [x] C.03 — API: `set.of`, `set.empty`, `set.add`, `set.remove`, `set.has`, `set.len`.
- [x] C.04 — API: `set.union`, `set.intersect`, `set.difference`.
- [x] C.05 — `for item in my_set` iteration.
- [x] C.06 — Tests and docs.

### 3B — List helpers

- [x] C.07 — `list.first(items)` -> `optional<T>`.
- [x] C.08 — `list.last(items)` -> `optional<T>`.
- [x] C.09 — `list.rest(items)` -> `list<T>`.
- [x] C.10 — `list.skip(items, count)` -> `list<T>`.

### 3C — Primitive specializations

- [x] C.11 — Emitter rule: primitive T -> contiguous array, no ARC per element.
- [x] C.12 — list<int/float/bool/i8-i64/u8-u64> specialized.
- [x] C.13 — optional<int/float/i8-i64/u8-u64> specialized.
- [x] C.14 — Benchmarks: before/after.

Gate: all C.* items green + gate criteria.

---

## Phase 4 — Standard Library Expansion (0.6)

### 4A — `std.fs`

- [x] X.01 — `fs.exists`, `fs.is_file`, `fs.is_dir`.
- [x] X.02 — `fs.create_dir`, `fs.create_dir_all`.
- [x] X.03 — `fs.remove_file`, `fs.remove_dir`.
- [x] X.04 — `fs.rename`, `fs.copy`.
- [x] X.05 — `fs.list_dir`, `fs.file_size`.

### 4B — `std.math`

- [x] X.06 — `math.abs`, `math.pow`, `math.sqrt`.
- [x] X.07 — `math.min`, `math.max`, `math.clamp`.
- [x] X.08 — `math.floor`, `math.ceil`, `math.round`.
- [x] X.09 — `math.sin`, `math.cos`, `math.tan`.
- [x] X.10 — `math.pi`, `math.e`.

### 4C — `std.time`

- [x] X.11 — `time.now()`, `time.now_ms()`.
- [x] X.12 — `time.elapsed(start, finish)`.
- [x] X.13 — `time.sleep(duration)`, `time.sleep_ms(ms)`.

### 4D — `std.regex`

- [x] X.14 — `regex.compile(pattern)`.
- [x] X.15 — `regex.is_match(pattern, input)`.
- [x] X.16 — `regex.find_all(pattern, input)`.

### 4E — Error helpers

- [x] X.17 — `.or_return(value)` on optional.
- [x] X.18 — `.or_wrap(context)` on result.

Gate: all X.* items green + gate criteria.

---

## Phase 5 — Tooling and Ecosystem (0.7)

- [ ] T.01 — VSCode extension published on Marketplace.
- [ ] T.02 — `zt test --filter <name>`.
- [ ] T.03 — Test runner: name, duration, stacktrace.
- [ ] T.04 — `test.throws(func)`.
- [ ] T.05 — C-Binding Pack (sqlite3 / libcurl).
- [ ] T.06 — `zt repl` (compile-and-run).
- [ ] T.07 — ZPM lock file.
- [ ] T.08 — ZPM semver for dependencies.
- [ ] T.09 — Neovim/Zed/Helix LSP config snippets.

Gate: all T.* items green + gate criteria.

---

## Phase 5B — Learning and Examples (0.7)

- [ ] E.01 — "Learn Zenith in 30 min" tutorial (English).
- [ ] E.02 — Example: CLI calculator.
- [ ] E.03 — Example: file processor.
- [ ] E.04 — Example: TODO app.
- [ ] E.05 — Example: simple game (Borealis).
- [ ] E.06 — Example: data parser.
- [ ] E.07 — Cookbook: grows with stdlib.

Gate: tutorial reviewed, examples compile and run.

---

## Phase 5C — Benchmarks (0.7, internal)

- [ ] K.01 — `benchmarks/` directory with infrastructure.
- [ ] K.02 — Micro: fibonacci, sort, string ops, allocation.
- [ ] K.03 — Compilation time for N-line projects.
- [ ] K.04 — Runtime: emitted C vs hand-written C.
- [ ] K.05 — Cross-language: Go, Python, Kotlin.
- [ ] K.06 — Regression detection only, no publishing.

Gate: benchmarks run, results recorded.

---

## Phase 6 — Self-hosting Bootstrap (0.8)

- [ ] H.01 — 3 real tools written in Zenith.
- [ ] H.02 — Language gaps identified and documented.
- [ ] H.03 — Gaps fixed, stdlib iterated.

Gate: tools compile, run, and pass their own test suites.

---

## Phase 7 — Release Engineering (0.9)

- [ ] R.01 — SemVer policy documented.
- [ ] R.02 — Deprecation workflow: warning for 1 release.
- [ ] R.03 — Borealis clean clone dogfood.
- [ ] R.04 — English docs canonical, others best-effort.
- [x] R.05 — License audit: all deps compatible with Apache-2.0 OR MIT.
- [ ] R.06 — Release candidate freeze.

Gate: no known P0/P1 bugs, clean install on 3 platforms.

---

## Phase 8 — 1.0

- [ ] V.01 — All stable features documented and tested.
- [ ] V.02 — No known P0/P1 bugs.
- [ ] V.03 — Clean install: Windows, Linux, macOS.
- [ ] V.04 — Public announcement.

Gate: ship it.
