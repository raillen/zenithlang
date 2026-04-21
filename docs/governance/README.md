# Governance 2.0 (R2.M0)

This folder stores the governance baseline for cycle 2.0.

## Fast start

Run one command for cycle triage:

```powershell
python tools/triage_cycle.py
```

Freeze baseline (quality + perf):

```powershell
python tools/triage_cycle.py --freeze-baseline
```

## Main files

- `docs/governance/r2-m0-governance.md`: owners, SLA, bug flow, P0 block rule.
- `docs/governance/baselines/quality-baseline.json`: frozen quality baseline.
- `docs/governance/baselines/perf-baseline.json`: frozen perf baseline snapshot.
- `tests/perf/baselines/<platform>/*.json`: per-benchmark perf baselines.
- `.github/ISSUE_TEMPLATE/bug-report.yml`: mandatory bug template.
- `.github/PULL_REQUEST_TEMPLATE.md`: mandatory regression checklist.
- `tools/triage_cycle.py`: single triage command.

## Triage artifacts

Each run writes:

- `docs/reports/triage/latest.md`
- `docs/reports/triage/latest.json`
- `docs/reports/triage/triage-<timestamp>.md`
- `docs/reports/triage/triage-<timestamp>.json`
