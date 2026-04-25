"""
Curated replay over modernized heavy semantic fixtures.

Purpose:
- keep a small, stable subset of tests/heavy in the official runner;
- validate representative failure and pass scenarios without depending on
  the legacy heavy runner;
- catch regressions where heavy fixtures fall back to project-format issues.
"""

from __future__ import annotations

import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT_EXE = ROOT / "zt.exe"
HEAVY_SEMANTIC = ROOT / "tests" / "heavy" / "fuzz" / "semantic"


@dataclass(frozen=True)
class CuratedCase:
    case_id: str
    should_pass: bool
    fragments: tuple[str, ...]


CASES = (
    CuratedCase(
        case_id="duplicate_symbol_17",
        should_pass=False,
        fragments=("error[name.duplicate]",),
    ),
    CuratedCase(
        case_id="result_propagation_12",
        should_pass=False,
        fragments=("error[type.mismatch]",),
    ),
    CuratedCase(
        case_id="trait_violation_23",
        should_pass=False,
        fragments=("error[type.invalid_map_key]",),
    ),
    CuratedCase(
        case_id="type_mismatch_10",
        should_pass=False,
        fragments=("error[syntax.unexpected_token]",),
    ),
    CuratedCase(
        case_id="unbound_type_0",
        should_pass=False,
        fragments=("error[syntax.error]",),
    ),
    CuratedCase(
        case_id="deep_if_24",
        should_pass=True,
        fragments=("check ok",),
    ),
    CuratedCase(
        case_id="generic_overflow_1",
        should_pass=True,
        fragments=("check ok",),
    ),
    CuratedCase(
        case_id="method_receiver_18",
        should_pass=False,
        fragments=("error[mutability.invalid_update]",),
    ),
)


def run_cmd(args: list[str], timeout: int = 240) -> tuple[int, str]:
    completed = subprocess.run(
        args,
        cwd=str(ROOT),
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        timeout=timeout,
    )
    return completed.returncode, (completed.stdout or "") + (completed.stderr or "")


def fail(message: str) -> int:
    print(message, file=sys.stderr)
    return 1


def main() -> int:
    if not ZT_EXE.exists():
        return fail(f"missing compiler binary: {ZT_EXE}")

    for case in CASES:
        project_dir = HEAVY_SEMANTIC / case.case_id
        if not project_dir.exists():
            return fail(f"missing heavy fixture directory: {project_dir}")

        rc, out = run_cmd([str(ZT_EXE), "check", str(project_dir), "--ci"], timeout=240)
        ok = rc == 0 if case.should_pass else rc != 0
        if not ok:
            expectation = "pass" if case.should_pass else "fail"
            return fail(
                f"heavy curated replay mismatch for {case.case_id}: expected {expectation}, rc={rc}\n"
                f"{out[:1200]}"
            )

        for fragment in case.fragments:
            if fragment not in out:
                return fail(
                    f"heavy curated replay missing fragment for {case.case_id}: {fragment}\n"
                    f"{out[:1200]}"
                )

        if "error[project." in out:
            return fail(
                f"heavy curated replay regressed to project-layer failure for {case.case_id}\n"
                f"{out[:1200]}"
            )

    print("heavy semantic curated replay ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
