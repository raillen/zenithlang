# Zenith Final Conformance Matrix (M32)

- Status: implemented, with explicit open gates
- Date: 2026-04-20
- Scope: final matrix using the canonical status labels from `language/spec/implementation-status.md`

## Inputs Used In This Snapshot

- `language/spec/implementation-status.md` (label vocabulary and closure rules)
- `language/surface-implementation-status.md` (current implementation cut)
- `tests/behavior/MATRIX.md` (M16 behavior map)
- `docs/planning/checklist-v1.md` (M32 required coverage)
- `tests/perf/README.md` and latest local `perf quick` run output
- latest local runs on this date:
  - `python run_all_tests.py` (failed: rebuild output parsing/encoding + behavior failures in `std_fs_path_basic` and `std_math_basic`)
  - `python tests/perf/run_perf.py --suite quick` (failed at `micro_runtime_core`)
  - `zt fmt --check tests/behavior/simple_app` (passed)
  - `zt doc check tests/behavior/simple_app` (passed)
  - `zt test tests/behavior/std_test_attr_pass_skip` (passed: `pass=1 skip=1 fail=0`)
  - `.ztc-tmp/tests/conformance/test_m16.exe` (failed: `29/122`)

## Canonical Labels

This matrix uses only canonical labels:

- `Spec`
- `Parsed`
- `Semantic`
- `Lowered`
- `Emitted`
- `Runtime`
- `Executable`
- `Conformant`
- `Deferred`
- `Risk`
- `Rejected`

## Layer Matrix By Feature

Notes:

- `-` means the layer is not applicable for that row.
- `Lowering+Backend` uses `Lowered`/`Emitted` depending on current evidence.

| Feature | Parser | Semantic | Lowering+Backend | Runtime | Diagnostics | Formatter | Overall | Evidence |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Project model (`zenith.ztproj`, namespace/imports, multifile) | Parsed | Semantic | Emitted | Executable | Executable | Executable | Executable | behavior: `multifile_*`, `public_const_module`; CLI `check/build` |
| Functions and control flow (`if/while/repeat/for/match`) | Parsed | Semantic | Emitted | Executable | Executable | Executable | Executable | behavior: `control_flow_*`, `functions_*` |
| Structs, traits, `apply`, mutating methods | Parsed | Semantic | Emitted | Executable | Executable | Executable | Executable | behavior: `structs_*`, `methods_*` |
| Collections, `optional/result`, value semantics | Parsed | Semantic | Emitted | Executable | Executable | Executable | Executable | behavior: `list_*`, `map_*`, `optional_result_*`, `value_semantics_*` |
| Runtime `where` contracts | Parsed | Semantic | Emitted | Executable | Executable | Executable | Executable | behavior: `where_contracts_ok`, `where_contract_*_error` |
| Enum payload and exhaustive `match` | Parsed | Semantic | Lowered | Deferred | Semantic | Executable | Semantic | `enum_match` and `enum_match_non_exhaustive_error` validate semantic path; no stable full E2E conformance gate yet |
| Diagnostics renderer (cross-stage) | - | - | - | - | Executable | - | Executable | golden fragments in `tests/fixtures/diagnostics/*` + CLI checks |
| Formatter (`zt fmt`, check mode) | - | - | - | - | - | Executable | Executable | `zt fmt --check tests/behavior/simple_app` pass |
| Stdlib MVP modules (`std.*`) | Parsed | Semantic | Emitted | Executable | Executable | Executable | Executable | behavior: `std_*` projects + module files in `stdlib/std/` |
| CLI final (`check/build/run/test/fmt/doc check`) | - | - | - | - | Executable | Executable | Executable | `run_all_tests.py` exercises command set; `zt test/fmt/doc check` executed |
| ZDoc (`zt doc check`, links/targets/pages) | - | - | - | - | Executable | - | Executable | `zt doc check tests/behavior/simple_app` pass; driver tests include `test_zdoc.exe` |

## Risk Matrix (Explicit `Risk` Coverage)

| Risk Item | Label | Gate State | Current Mitigation / Evidence |
| --- | --- | --- | --- |
| RC cycles without full cycle strategy | Risk | Open | Tracked in `language/spec/backend-scalability-risk-model.md`; planned closure in `M35` |
| Monomorphization code bloat | Risk | Open (partially mitigated) | `build.monomorphization_limit` + behavior `monomorphization_limit_error`; still needs long-scale closure |
| Heap-vs-stack wrappers overhead | Risk | Open (partially mitigated) | stack/in-place work exists for `optional/result`, but perf gate still failing in current snapshot |
| Runtime `where` completeness | Executable | Closed for current subset | runtime errors validated via `where_contract_*` behavior tests |
| Exhaustive enum match in emitted C path | Risk | Open | semantic diagnostics present, but missing stable E2E conformance closure in current gate snapshot |

## M36 Performance Integration (By Critical Benchmark)

Snapshot from latest local `python tests/perf/run_perf.py --suite quick`:

| Benchmark | Status | Evidence |
| --- | --- | --- |
| `micro_frontend_small_check` | Executable | pass |
| `micro_lowering_small_emit_c` | Executable | pass |
| `micro_runtime_core` | Risk | fail (`backend.c.emit`: missing `std.bytes` wrapper symbols in generated C) |
| `micro_stdlib_core` | Deferred | not executed (suite stopped after previous fail) |
| `macro_small_check` | Deferred | not executed in latest failed run |
| `macro_small_build_cold` | Deferred | not executed in latest failed run |
| `macro_small_build_warm` | Deferred | not executed in latest failed run |
| `macro_small_run` | Deferred | not executed in latest failed run |
| `macro_small_test` | Deferred | not executed in latest failed run |
| `macro_medium_check` | Deferred | not executed in latest failed run |
| `nightly` gate (`tests/perf/gate_nightly.ps1`) | Deferred | still pending dedicated long window |

## Full-Suite Gate Snapshot (M32)

| Gate | Status | Evidence |
| --- | --- | --- |
| Consolidated suite run before stable declaration | Executable | `python run_all_tests.py` was executed; result failed with actionable blockers |
| Stable conformance gate (`M16`) | Deferred | `.ztc-tmp/tests/conformance/test_m16.exe` returned `29/122` |
| Performance quick gate | Risk | failed at `micro_runtime_core` |
| Performance nightly gate | Deferred | not fully executed in dedicated long window |

## M32 Closure Statement

M32 is implemented as a real, auditable matrix with canonical labels, layer coverage, explicit risks and integrated performance status by benchmark.

Stable MVP release remains blocked until the open gates above are resolved.
