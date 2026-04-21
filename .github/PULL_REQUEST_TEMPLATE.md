## Summary

- issue: <link or n/a>
- severity: <P0|P1|P2|P3 or n/a>
- layer: <frontend|semantic|hir/zir|backend|runtime|stdlib|tooling|docs>

## Regression flow

- [ ] If this is a bugfix, a regression test was added first.
- [ ] Regression test path is listed below.
- [ ] `python tools/triage_cycle.py` ran locally.
- [ ] No open P0 is being ignored.

Regression test path:

`<path or n/a>`

## Validation
- [ ] `python run_suite.py pr_gate`  ← comando único obrigatório (R2.M1)
- [ ] `python run_all_tests.py`  ← fallback / referência cruzada
- [ ] `python tests/perf/run_perf.py --suite quick --release-gate`  ← perf isolado (opcional)

## Notes

- Risk:
- Follow-up (if any):
