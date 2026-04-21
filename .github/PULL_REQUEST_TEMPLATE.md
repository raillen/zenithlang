## Summary

- issue: <link or n/a>
- severity: <P0|P1|P2|P3 or n/a>
- layer: <frontend|semantic|hir/zir|backend|runtime|stdlib|tooling|docs>
- spec divergence class (if any): <P0|P1|P2 or n/a>

## Regression flow

- [ ] If this is a bugfix, a regression test was added first.
- [ ] Regression test path is listed below.
- [ ] If this is a new feature, positive and negative tests were added.
- [ ] If behavior changed, docs were updated in this same PR.
- [ ] `python tools/triage_cycle.py` ran locally.
- [ ] No open P0 is being ignored.

Regression test path:

`<path or n/a>`

Feature test paths (positive/negative):

`<path-positive or n/a>`

`<path-negative or n/a>`

## Validation

- [ ] `python run_suite.py pr_gate` (required)
- [ ] `python run_all_tests.py` (cross-check)
- [ ] `python tests/perf/run_perf.py --suite quick --release-gate` (perf gate)
- [ ] If critical perf regression was found, merge is blocked (or override is documented).

## Evidencia minima

- command + result:
  - `<command>`
  - `<result>`
- test file added/changed:
  - `<path>`
- closing commit/PR:
  - `<sha or link>`
- residual risk (if any):
  - `<risk or none>`

## Notes

- Risk:
- Follow-up (if any):
