# Zenith Surface Implementation Status

- Status: current compiler cut snapshot
- Date: 2026-04-28
- Labels: `Spec`, `Parsed`, `Semantic`, `Lowered`, `Emitted`, `Runtime`, `Executable`, `Conformant`, `Deferred`, `Risk`, `Rejected`

## Language Surface

| Feature | Status | Notes |
| --- | --- | --- |
| Namespaces/imports/multifile | `Conformant` | covered in `multifile_*` behavior projects |
| Functions/control flow (`if/while/for/repeat/match`) | `Conformant` | covered in `control_flow_*` |
| Structs/traits/apply/methods | `Conformant` | covered in `structs_*` and `methods_*` |
| Collections (`list`, `map`, index/slice/len/get`) | `Conformant` | covered in `list_*` and `map_*` |
| `optional<T>` / `result<T,E>` | `Conformant` | includes `optional_match_value`, `result_question_basic`, `optional_result_helpers_pass` and `optional_result_helpers_absence_error` |
| `?` propagation (`result` + `optional`) | `Conformant` | covered in `result_question_basic` and `optional_question_basic` |
| `fmt "..."` interpolation | `Conformant` | covered in `fmt_interpolation_basic` + type error case |
| `panic(...)`, `todo(...)`, `unreachable(...)` and `check(...)` | `Conformant` | covered in `panic_*`, `todo_builtin_fail`, `unreachable_builtin_fail` and `check_intrinsic_*` |
| `core.Error(...)` qualified | `Conformant` | covered in `core_error_construction` |
| Unsigned aliases (`u8/u16/u32/u64`) | `Conformant` | covered in `u_alias_basic` |
| Namespace `public var` (read public, write owner namespace) | `Conformant` | covered in `public_var_module`, `public_var_module_state`, `public_var_cross_namespace_write_error` |
| `std.random` public state (`seeded`, `last_seed`, `draw_count`, `stats`) | `Conformant` | covered in `std_random_basic`, `std_random_state_observability`, `std_random_between_branches`, `std_random_cross_namespace_write_error` |
| Closures v1 (`func ... end`, immutable capture) | `Conformant` | covered in `closure_capture_basic` and `closure_mut_capture_error`; mutable capture remains deferred |
| Lambdas v1 + int HOFs (`func(...) => expr`) | `Conformant` | covered in `lambda_hof_basic`; HOF subset is `std.collections.map_int/filter_int/reduce_int` |
| Explicit lazy v1 (`lazy<int>`) | `Conformant` | covered in `lazy_explicit_order_basic` and `lazy_reuse_error`; generic lazy and lazy iterators remain future work |

## Tooling And Runtime

| Area | Status | Notes |
| --- | --- | --- |
| Single-file execution (`zt run file.zt`) | `Conformant` | `check`, `build`, `run`, `emit-c` work on standalone `.zt` files without a project; synthetic manifest, stdlib auto-load, namespace-path validation skipped |
| `zt fmt` / `zt fmt --check` | `Conformant` | gate project `tooling_gate_smoke` |
| `zt fmt` idempotence (`fmt(fmt(x)) == fmt(x)`) | `Conformant` | gate runner `tests/formatter/run_formatter_idempotence.py`, integrado ao `pr_gate` tooling; cobre os 9 casos em `tests/formatter/cases/` |
| `zt doc check` | `Conformant` | gate project `tooling_gate_smoke` |
| Runtime contracts (`where`) | `Conformant` | positive and negative behavior tests |
| Runtime diagnostic codes | `Conformant` | canonical alpha contract is `language/spec/diagnostic-code-catalog.md` plus `language/spec/diagnostics-model.md` |
| Perf nightly gate | `Conformant` | `reports/perf/summary-nightly.json` status `pass` |
| `std.console` interactive helpers | `Conformant` | Phase 5D: `console.write_line`, `console.error_line`, `console.pause`, `console.prompt`, and `console.confirm(default_value: ...)`; `std.io` remains the stream layer |

## Open Items

| Item | Label | Notes |
| --- | --- | --- |
| Default runtime thread-safety boundary | `Risk` | default runtime path is single-isolate with non-atomic ARC; cross-thread work must stay behind isolate/message-passing boundaries; the first public transfer slice is `std.concurrent.copy_*`, while workers/jobs/channels remain future surface |
| RC cycles ownership strategy | `Risk` | default runtime ships without cycle collection; cycle-prone APIs remain gated by `language/spec/runtime-model.md` |
| Terminal controls in `std.console` | `Conformant` | Phase 5D includes detection, size, clear screen, foreground colors, basic styles, reset, and non-blocking key read; cursor movement remains future work |
