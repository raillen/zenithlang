# Zenith Surface Implementation Status

- Status: current compiler cut snapshot
- Date: 2026-04-21
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

## Tooling And Runtime

| Area | Status | Notes |
| --- | --- | --- |
| `zt fmt` / `zt fmt --check` | `Conformant` | gate project `tooling_gate_smoke` |
| `zt doc check` | `Conformant` | gate project `tooling_gate_smoke` |
| Runtime contracts (`where`) | `Conformant` | positive and negative behavior tests |
| Perf nightly gate | `Conformant` | `reports/perf/summary-nightly.json` status `pass` |

## Open Items

| Item | Label | Notes |
| --- | --- | --- |
| Runtime diagnostic taxonomy alignment (`runtime.bounds`, etc.) | `Risk` | tracked as non-blocking alignment work |
| RC cycles ownership strategy | `Risk` | tracked in scalability/runtime policy |
