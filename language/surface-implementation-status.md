# Zenith Surface Implementation Status

- Status: current compiler cut snapshot
- Date: 2026-04-22
- Labels: `Spec`, `Parsed`, `Semantic`, `Lowered`, `Emitted`, `Runtime`, `Executable`, `Conformant`, `Deferred`, `Risk`, `Rejected`

## Language Surface

| Feature | Status | Notes |
| --- | --- | --- |
| Namespaces/imports/multifile | `Conformant` | covered in `multifile_*` behavior projects |
| Functions/control flow (`if/while/for/repeat/match`) | `Conformant` | covered in `control_flow_*` |
| Structs/traits/apply/methods | `Conformant` | covered in `structs_*` and `methods_*` |
| Collections (`list`, `map`, index/slice/len/get`) | `Conformant` | covered in `list_*` and `map_*` |
| `optional<T>` / `result<T,E>` | `Conformant` | includes `optional_match_value` and `result_question_basic` |
| `?` propagation (`result` + `optional`) | `Conformant` | covered in `result_question_basic` and `optional_question_basic` |
| `fmt "..."` interpolation | `Conformant` | covered in `fmt_interpolation_basic` + type error case |
| `panic(...)` and `check(...)` | `Conformant` | covered in `panic_*` and `check_intrinsic_*` |
| `core.Error(...)` qualified | `Conformant` | covered in `core_error_construction` |
| Unsigned aliases (`u8/u16/u32/u64`) | `Conformant` | covered in `u_alias_basic` |
| Namespace `public var` (read public, write owner namespace) | `Conformant` | covered in `public_var_module`, `public_var_module_state`, `public_var_cross_namespace_write_error` |
| `std.random` public state (`seeded`, `last_seed`, `draw_count`, `stats`) | `Conformant` | covered in `std_random_basic`, `std_random_state_observability`, `std_random_between_branches`, `std_random_cross_namespace_write_error` |

## Tooling And Runtime

| Area | Status | Notes |
| --- | --- | --- |
| `zt fmt` / `zt fmt --check` | `Conformant` | gate project `tooling_gate_smoke` |
| `zt doc check` | `Conformant` | gate project `tooling_gate_smoke` |
| Runtime contracts (`where`) | `Conformant` | positive and negative behavior tests |
| Runtime diagnostic codes | `Conformant` | canonical alpha contract is `language/spec/diagnostic-code-catalog.md` plus `language/spec/diagnostics-model.md` |
| Perf nightly gate | `Conformant` | `reports/perf/summary-nightly.json` status `pass` |

## Open Items

| Item | Label | Notes |
| --- | --- | --- |
| Default runtime thread-safety boundary | `Risk` | default runtime path is single-isolate with non-atomic ARC; cross-thread work must stay behind isolate/message-passing boundaries or future explicit shared wrappers |
| RC cycles ownership strategy | `Risk` | default runtime ships without cycle collection; cycle-prone APIs remain gated by `language/spec/runtime-model.md` |
