#!/usr/bin/env python3
"""
Aligned heavy runner for the current Zenith repository.

This runner is intentionally small and honest:
- it only exposes suites that exist today;
- it reuses the same stable drivers used by the official test runner;
- it writes JSON + Markdown summaries under tests/heavy/reports/.
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
import time
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
REPORT_DIR = ROOT / "tests" / "heavy" / "reports"


@dataclass(frozen=True)
class HeavyTask:
    task_id: str
    command: tuple[str, ...]
    timeout: int
    note: str


SUITES = {
    "curated": (
        HeavyTask(
            task_id="heavy/semantic_curated",
            command=(sys.executable, str(ROOT / "tests" / "hardening" / "test_heavy_semantic_curated.py")),
            timeout=240,
            note="Replay curado sobre fixtures pesados modernizados.",
        ),
    ),
    "fuzz": (
        HeavyTask(
            task_id="heavy/fuzz_semantic",
            command=(
                sys.executable,
                str(ROOT / "tests" / "heavy" / "fuzz" / "semantic" / "fuzz_semantic.py"),
                "--iterations",
                "80",
                "--seed",
                "20260423",
                "--timeout",
                "10",
            ),
            timeout=1200,
            note="Campanha semantica reproducivel.",
        ),
    ),
}
SUITES["all"] = SUITES["curated"] + SUITES["fuzz"]


def run_cmd(args: tuple[str, ...], timeout: int) -> tuple[int, str, float]:
    start = time.time()
    completed = subprocess.run(
        list(args),
        cwd=str(ROOT),
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        timeout=timeout,
    )
    elapsed_ms = (time.time() - start) * 1000.0
    output = (completed.stdout or "") + (completed.stderr or "")
    return completed.returncode, output, elapsed_ms


def save_reports(suite_name: str, results: list[dict[str, object]]) -> tuple[Path, Path]:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    json_path = REPORT_DIR / f"heavy-tests-{suite_name}-{stamp}.json"
    md_path = REPORT_DIR / "heavy-tests-summary.md"

    payload = {
        "suite": suite_name,
        "generated_at_utc": stamp,
        "results": results,
    }
    json_path.write_text(json.dumps(payload, indent=2, ensure_ascii=True) + "\n", encoding="utf-8")

    passed = sum(1 for item in results if item["passed"])
    failed = len(results) - passed
    lines = [
        "# Heavy Tests Summary",
        "",
        f"Suite: `{suite_name}`",
        f"Pass: `{passed}`",
        f"Fail: `{failed}`",
        "",
        "| Task | Status | Duration | Note |",
        "|---|---|---:|---|",
    ]
    for item in results:
        status = "OK" if item["passed"] else "FAIL"
        lines.append(
            f"| `{item['task_id']}` | `{status}` | `{item['duration_ms']:.2f}ms` | {item['note']} |"
        )
    md_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return json_path, md_path


def main() -> int:
    parser = argparse.ArgumentParser(description="Run aligned heavy tests.")
    parser.add_argument(
        "--suite",
        choices=sorted(SUITES.keys()),
        default="all",
        help="Heavy suite to run.",
    )
    args = parser.parse_args()

    tasks = SUITES[args.suite]
    results: list[dict[str, object]] = []
    overall_ok = True

    for task in tasks:
        rc, out, elapsed_ms = run_cmd(task.command, task.timeout)
        passed = rc == 0
        overall_ok = overall_ok and passed
        results.append(
            {
                "task_id": task.task_id,
                "passed": passed,
                "duration_ms": elapsed_ms,
                "note": task.note,
                "output": out[:1200],
            }
        )
        status = "OK" if passed else "FAIL"
        print(f"[{status}] {task.task_id} {elapsed_ms:.2f}ms")
        if not passed:
            print(out[:1200], file=sys.stderr)

    json_path, md_path = save_reports(args.suite, results)
    print(f"json report: {json_path}")
    print(f"summary: {md_path}")
    return 0 if overall_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
