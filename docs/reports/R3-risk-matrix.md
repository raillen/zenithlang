# R3 Risk Matrix

- Date: 2026-04-23
- Scope: Zenith cycle R3 (`R3.P1` through `R3.M9`)
- Status: published (initial snapshot for kickoff)
- Upstream: `docs/planning/roadmap-v3.md`, `docs/planning/checklist-v3.md`
- Downstream: `docs/reports/release/R3.M0-kickoff-report.md`, milestone reports

## Legend

- **Severity**:
  - `P0` - blocks release or corrupts core invariants
  - `P1` - degrades milestone quality or delays scope
  - `P2` - contained risk, watchlist only
- **Status**: `open` / `monitoring` / `closed`
- **Owner**: the milestone (or role) accountable for tracking

## 1) Language And Semantics

| ID | Severity | Title | Owner | Deadline | Status | Notes |
|---|---|---|---|---|---|---|
| R3-RISK-001 | P1 | `public var` used without mitigation in stdlib/packages | `R3.P1` follow-up | until `R3.M2` start | monitoring | adoption must stay pontual; guideline in `R3.P1.A` analysis |
| R3-RISK-002 | P1 | namespace state semantics drifts under concurrency before `R3.M2` lands | `R3.M2` | before `R3.M2` merge | open | blocked by `R3.P1.A` analysis already published |
| R3-RISK-003 | P0 | `global` keyword reintroduced by accident in formatter/diagnostic text | `R3.M1` | each milestone gate | monitoring | formatter/diagnostics must canonicalize to `public` only |
| R3-RISK-004 | P1 | `trait` vs `interface` conflict reappears (alias surfaces in docs/tests) | `R3.M4` / `R3.M5` | before `R3.M5` merge | monitoring | per cycle directive: `interface` is not a new concept |
| R3-RISK-005 | P1 | `mut func` contract weakens because of namespace mutation shortcuts | `R3.M2` | before `R3.M2` merge | monitoring | `mut func` still required for `self` mutation; doc already covered in Decision 086 |

## 2) Concurrency (`R3.M2`)

`R3.M2` Phase 1 closed 2026-04-23 (see `docs/reports/release/R3.M2-concurrency-base-report.md`). Phases 3-5 are tracked below with updated owners and deadlines.

| ID | Severity | Title | Owner | Deadline | Status | Notes |
|---|---|---|---|---|---|---|
| R3-RISK-010 | P0 | data races via `public var` accessed from multiple tasks | `R3.M2` follow-up (Phase 3) | before `R3.M5` checkpoint | monitoring | single-owner default locked in Decision 087 + `language/spec/concurrency.md`; `task`/`channel` surface not yet shipped, so no runtime exposure in this cut |
| R3-RISK-011 | P1 | `Shared<T>` surface leaks into beginner path | `R3.M2` follow-up (Phase 5) | before `R3.M9` | monitoring | spec explicitly marks `Shared<T>`/`atomic<T>` as Phase 5 opt-in only |
| R3-RISK-012 | P1 | cancellation and determinism tests flaky on Windows runner | `R3.M2` follow-up (Phase 3) | with Phase 3 | open | determinism for copy helpers delivered (`std_concurrent_boundary_copy_determinism`); cancellation fixtures await Phase 3 runtime surface |

## 3) FFI (`R3.M3`)

`R3.M3` Phase 1 closed 2026-04-23 (see `docs/reports/release/R3.M3-ffi-1.0-report.md`). Remaining items are Phase 2-4 per `language/spec/ffi.md`.

| ID | Severity | Title | Owner | Deadline | Status | Notes |
|---|---|---|---|---|---|---|
| R3-RISK-020 | P1 | ABI drift between toolchains (MSVC vs MinGW vs gcc-linux); arity / invalid-return negatives not yet pinned | `R3.M3` follow-up (Phase 2) | before `R3.M9` | monitoring | spec published; positive + struct-as-arg negative delivered; arity/return negatives deferred |
| R3-RISK-021 | P1 | long-lived C pointer caching violates ARC shielding semantics | `R3.M3` follow-up (Phase 2) | before `R3.M9` | monitoring | spec forbids it; no static analysis yet |
| R3-RISK-022 | P1 | callback/delegate interop with FFI blocks certain use cases | `R3.M5` (Phase 3 FFI) | `R3.M5` merge | open | depends on callables/delegates landing |

## 4) Dyn Dispatch (`R3.M4`)

| ID | Severity | Title | Owner | Deadline | Status | Notes |
|---|---|---|---|---|---|---|
| R3-RISK-030 | P1 | dyn dispatch perf cost exceeds budget | `R3.M4` | `R3.M4` merge | open | measure + publish budget as acceptance criterion |
| R3-RISK-031 | P1 | heterogeneous collection surface overshoots committed subset | `R3.M4` | `R3.M4` merge | open | keep subset documented and enforced |

## 5) Callables, Closures, Lambdas (`R3.M5`-`R3.M7`)

| ID | Severity | Title | Owner | Deadline | Status | Notes |
|---|---|---|---|---|---|---|
| R3-RISK-040 | P1 | closure capture leaks lifetimes outside owner scope | `R3.M6` | `R3.M6` merge | open | immutable-capture-only in first cut |
| R3-RISK-041 | P1 | lambda syntax degrades readability in canonical formatter | `R3.M7` | `R3.M7` merge | open | formatter guideline + readability review |
| R3-RISK-042 | P2 | HOF hot-path perf regression on `map`/`filter`/`reduce` | `R3.M7` | `R3.M7` merge | monitoring | benchmark required at merge |

## 6) Lazy (`R3.M8`)

| ID | Severity | Title | Owner | Deadline | Status | Notes |
|---|---|---|---|---|---|---|
| R3-RISK-050 | P1 | implicit lazy leaks via common expressions | `R3.M8` | `R3.M8` merge | open | explicit `lazy<T>` only; block implicit |
| R3-RISK-051 | P2 | single-consumption invariant not enforced at runtime | `R3.M8` | `R3.M8` merge | monitoring | tests on reuse-invalid required |

## 7) Release (`R3.M9`)

| ID | Severity | Title | Owner | Deadline | Status | Notes |
|---|---|---|---|---|---|---|
| R3-RISK-060 | P0 | open P0 without formal acceptance at release cut | `R3.M9` | `R3.M9` merge | monitoring | gate rule already enforced by release gates |
| R3-RISK-061 | P1 | clean-install validation fails on packaged artifact | `R3.M9` | `R3.M9` merge | open | reuse validation workflow from `R2.M12` |
| R3-RISK-062 | P1 | perf nightly exceeds budget at release gate | `R3.M9` | `R3.M9` merge | open | reference `docs/governance/baselines/perf-baseline.json` |

## 8) Borealis Alignment

| ID | Severity | Title | Owner | Deadline | Status | Notes |
|---|---|---|---|---|---|---|
| R3-RISK-070 | P1 | Borealis depends on language feature not shipped in target R3 milestone | `R3.M0`/`R3.M5`/`R3.M9` checkpoints | each checkpoint | monitoring | checkpoint at `R3.M0` initial, `R3.M5`, `R3.M9` |
| R3-RISK-071 | P1 | Borealis regression caused by compiler/runtime change lands without local test | Borealis trilha | each milestone | monitoring | keep `./zt.exe check packages/borealis/zenith.ztproj --all` in CI |

## 9) Cross-Cutting Continuous Risks

| ID | Severity | Title | Owner | Deadline | Status | Notes |
|---|---|---|---|---|---|---|
| R3-RISK-080 | P1 | new bug without regression test | all milestones | each PR | monitoring | continuous rule already in checklist |
| R3-RISK-081 | P1 | spec vs implementation drift without P0/P1/P2 classification | all milestones | each PR | monitoring | continuous rule already in checklist |
| R3-RISK-082 | P2 | critical perf regression merged | all milestones | each PR | monitoring | gate `tests/perf/gate_pr.ps1` must block |

## Review Cadence

- revise this matrix at each milestone closeout;
- any new risk raised during the cycle receives an id `R3-RISK-NNN` and an owner;
- `P0` items without owner block the milestone closeout.

## Initial Acceptance Criterion For `R3.M0`

- no `P0` in this matrix without owner and deadline at cycle start.
- satisfied at publication time (every `P0` above has an owner milestone and a deadline gate).
