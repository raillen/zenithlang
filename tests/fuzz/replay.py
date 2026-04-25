"""
R2.M2 - Replay persisted crash seeds.

Runs every `.zt` under `tests/fuzz/seeds/crashes/` through `zt.exe check`
and fails if any seed still produces a crash/timeout. This is what gets
wired into the `pr_gate`, `nightly` and `stress` suites to guarantee
"reexecucao de seeds em toda rodada relevante".
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from harness import run_check   # noqa: E402

SEEDS_DIR = HERE / "seeds" / "crashes"


def replay(timeout_sec: float, verbose: bool) -> int:
    if not SEEDS_DIR.exists():
        print(json.dumps({"driver": "replay", "seeds": 0, "failures": 0}))
        return 0

    seeds = sorted(SEEDS_DIR.glob("*.zt"))
    failures: list[str] = []
    for p in seeds:
        src = p.read_text(encoding="utf-8", errors="replace")
        res = run_check(src, timeout_sec=timeout_sec)
        if res.kind in ("crash", "timeout"):
            failures.append(p.name)
            print(f"  [FAIL] {p.name} kind={res.kind} reason={res.reason}")
        elif verbose:
            print(f"  [OK  ] {p.name} kind={res.kind} rc={res.rc}")

    print(json.dumps({
        "driver": "replay", "seeds": len(seeds), "failures": len(failures),
        "failed_names": failures,
    }))
    return 1 if failures else 0


def main() -> int:
    ap = argparse.ArgumentParser(description="Replay persisted fuzz crash seeds")
    ap.add_argument("--timeout", type=float, default=8.0)
    ap.add_argument("--verbose", "-v", action="store_true")
    args = ap.parse_args()
    return replay(args.timeout, args.verbose)


if __name__ == "__main__":
    raise SystemExit(main())
