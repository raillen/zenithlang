"""
Cycle triage runner for R2.M0 governance.

Single command:
    python tools/triage_cycle.py

Freeze baseline:
    python tools/triage_cycle.py --freeze-baseline
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REPORT_DIR = ROOT / "docs" / "reports" / "triage"
QUALITY_BASELINE_PATH = ROOT / "docs" / "governance" / "baselines" / "quality-baseline.json"
PERF_SUMMARY_PATH = ROOT / "reports" / "perf" / "summary-quick.json"


def now_iso() -> str:
    return datetime.now(timezone.utc).isoformat()


def timestamp_tag() -> str:
    return datetime.now(timezone.utc).strftime("%Y%m%d-%H%M%SZ")


def git_sha() -> str:
    completed = subprocess.run(
        ["git", "rev-parse", "HEAD"],
        cwd=ROOT,
        text=True,
        capture_output=True,
        check=False,
    )
    if completed.returncode == 0:
        return completed.stdout.strip()
    return "unknown"


def tail_text(value: str, max_lines: int = 60, max_chars: int = 12000) -> str:
    if not value:
        return ""
    lines = value.splitlines()
    if len(lines) > max_lines:
        lines = lines[-max_lines:]
    text = "\n".join(lines)
    if len(text) > max_chars:
        text = text[-max_chars:]
    return text


def run_cmd(args: list[str]) -> dict:
    started = now_iso()
    t0 = time.perf_counter()
    env = dict(os.environ)
    env["PYTHONIOENCODING"] = "utf-8"
    env["PYTHONUTF8"] = "1"
    completed = subprocess.run(
        args,
        cwd=ROOT,
        env=env,
        text=True,
        capture_output=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    duration_ms = round((time.perf_counter() - t0) * 1000.0, 3)
    return {
        "args": args,
        "started_at_utc": started,
        "duration_ms": duration_ms,
        "returncode": completed.returncode,
        "stdout_tail": tail_text(completed.stdout),
        "stderr_tail": tail_text(completed.stderr),
    }


def parse_quality_summary(stdout_tail: str) -> dict:
    patterns = {
        "total": r"Total\s*:\s*(\d+)",
        "pass": r"Pass\s*:\s*(\d+)",
        "fail": r"Fail\s*:\s*(\d+)",
        "skip": r"Skip\s*:\s*(\d+)",
    }

    summary: dict[str, int] = {}
    for key, pattern in patterns.items():
        match = re.search(pattern, stdout_tail)
        summary[key] = int(match.group(1)) if match else -1

    failures: list[str] = []
    in_fail_block = False
    for line in stdout_tail.splitlines():
        stripped = line.strip()
        if stripped == "Failures:":
            in_fail_block = True
            continue
        if not in_fail_block:
            continue
        if stripped.startswith("- "):
            failures.append(stripped[2:].strip())
            continue
        if stripped == "":
            continue
        if not line.startswith(" ") and not line.startswith("\t"):
            in_fail_block = False

    return {
        "summary": summary,
        "failures": failures,
    }


def load_json(path: Path) -> dict | None:
    if not path.exists():
        return None
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError:
        return None


def compare_quality_with_baseline(current: dict, baseline: dict | None) -> dict:
    current_summary = current.get("summary", {})
    if baseline is None:
        return {
            "status": "no-baseline",
            "delta": {},
        }

    base_summary = baseline.get("summary", {})
    delta = {
        "pass": current_summary.get("pass", -1) - int(base_summary.get("pass", 0)),
        "fail": current_summary.get("fail", -1) - int(base_summary.get("fail", 0)),
        "skip": current_summary.get("skip", -1) - int(base_summary.get("skip", 0)),
    }

    status = "pass"
    if delta["fail"] > 0:
        status = "fail"
    elif delta["pass"] < 0:
        status = "warn"

    return {
        "status": status,
        "delta": delta,
        "baseline_sha": baseline.get("git_sha", "unknown"),
    }


def write_quality_baseline(quality: dict, git_head: str) -> None:
    QUALITY_BASELINE_PATH.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "generated_at_utc": now_iso(),
        "git_sha": git_head,
        "command": "python run_all_tests.py",
        "summary": quality.get("summary", {}),
        "notes": "Frozen by tools/triage_cycle.py --freeze-baseline",
    }
    QUALITY_BASELINE_PATH.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def build_markdown(report: dict) -> str:
    q = report["quality"]
    p = report["performance"]
    qc = report["commands"]["quality"]
    pc = report["commands"]["performance"]

    lines = [
        "# Cycle Triage Report",
        "",
        "## Summary",
        "",
        f"- status: `{report['overall_status']}`",
        f"- generated_at_utc: `{report['generated_at_utc']}`",
        f"- git_sha: `{report['git_sha']}`",
        f"- freeze_baseline: `{report['freeze_baseline']}`",
        "",
        "## Quality",
        "",
        f"- command: `{report['quality_command']}`",
        f"- return_code: `{qc['returncode']}`",
        f"- pass: `{q['summary'].get('pass', -1)}`",
        f"- fail: `{q['summary'].get('fail', -1)}`",
        f"- skip: `{q['summary'].get('skip', -1)}`",
        f"- baseline_compare: `{q['baseline_compare']['status']}`",
    ]

    deltas = q["baseline_compare"].get("delta", {})
    if deltas:
        lines.append(
            f"- baseline_delta(pass/fail/skip): `{deltas.get('pass', 0)}/{deltas.get('fail', 0)}/{deltas.get('skip', 0)}`"
        )

    failures = q.get("failures", [])
    if failures:
        lines.extend([
            "",
            "### Quality failures",
            "",
        ])
        for item in failures:
            lines.append(f"- {item}")

    lines.extend([
        "",
        "## Performance",
        "",
        f"- command: `{report['performance_command']}`",
        f"- return_code: `{pc['returncode']}`",
        f"- suite_status: `{p.get('status', 'missing')}`",
        f"- benchmark_count: `{p.get('benchmark_count', -1)}`",
        f"- platform_key: `{p.get('platform_key', 'unknown')}`",
        f"- summary_json: `{PERF_SUMMARY_PATH.as_posix()}`",
        "",
        "## Artifacts",
        "",
        f"- latest_md: `docs/internal/reports/triage/latest.md`",
        f"- latest_json: `docs/internal/reports/triage/latest.json`",
        f"- quality_baseline: `{QUALITY_BASELINE_PATH.as_posix()}`",
        "",
    ])

    return "\n".join(lines)


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="python tools/triage_cycle.py",
        description="Run quality + perf quick triage in one command.",
    )
    parser.add_argument(
        "--freeze-baseline",
        action="store_true",
        help="Update quality baseline JSON and perf benchmark baselines.",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    REPORT_DIR.mkdir(parents=True, exist_ok=True)

    quality_cmd = [sys.executable, "run_all_tests.py"]
    perf_cmd = [
        sys.executable,
        "tests/perf/run_perf.py",
        "--suite",
        "quick",
        "--release-gate",
    ]
    if args.freeze_baseline:
        perf_cmd.append("--update-baseline")

    quality_run = run_cmd(quality_cmd)
    perf_run = run_cmd(perf_cmd)

    quality_data = parse_quality_summary(quality_run["stdout_tail"])
    perf_summary = load_json(PERF_SUMMARY_PATH) or {
        "status": "missing",
        "benchmark_count": -1,
        "platform_key": "unknown",
    }
    perf_status = str(perf_summary.get("status", "missing"))
    perf_benchmark_count = int(perf_summary.get("benchmark_count", -1))
    perf_platform_key = str(perf_summary.get("platform_key", "unknown"))
    if perf_run["returncode"] != 0:
        perf_status = "fail"
        perf_benchmark_count = -1

    existing_quality_baseline = load_json(QUALITY_BASELINE_PATH)
    baseline_compare = compare_quality_with_baseline(quality_data, existing_quality_baseline)
    quality_data["baseline_compare"] = baseline_compare

    git_head = git_sha()
    if args.freeze_baseline:
        write_quality_baseline(quality_data, git_head)

    overall = "pass"
    quality_fail_count = quality_data["summary"].get("fail", -1)

    if quality_run["returncode"] != 0 or quality_fail_count > 0:
        overall = "fail"

    if perf_status in {"fail", "missing"}:
        overall = "fail"
    elif perf_status == "warn" and overall != "fail":
        overall = "warn"

    cmp_status = baseline_compare.get("status")
    if cmp_status == "fail" and overall != "fail":
        overall = "fail"
    elif cmp_status == "warn" and overall == "pass":
        overall = "warn"

    report = {
        "generated_at_utc": now_iso(),
        "git_sha": git_head,
        "freeze_baseline": bool(args.freeze_baseline),
        "quality_command": "python run_all_tests.py",
        "performance_command": "python tests/perf/run_perf.py --suite quick --release-gate",
        "overall_status": overall,
        "commands": {
            "quality": quality_run,
            "performance": perf_run,
        },
        "quality": quality_data,
        "performance": {
            "status": perf_status,
            "benchmark_count": perf_benchmark_count,
            "platform_key": perf_platform_key,
            "summary_path": PERF_SUMMARY_PATH.as_posix(),
        },
    }

    tag = timestamp_tag()
    report_json = REPORT_DIR / f"triage-{tag}.json"
    report_md = REPORT_DIR / f"triage-{tag}.md"
    latest_json = REPORT_DIR / "latest.json"
    latest_md = REPORT_DIR / "latest.md"

    json_text = json.dumps(report, indent=2) + "\n"
    md_text = build_markdown(report) + "\n"

    report_json.write_text(json_text, encoding="utf-8")
    report_md.write_text(md_text, encoding="utf-8")
    latest_json.write_text(json_text, encoding="utf-8")
    latest_md.write_text(md_text, encoding="utf-8")

    print(f"overall_status: {overall}")
    print(f"report_md: {report_md}")
    print(f"report_json: {report_json}")
    print(f"latest_md: {latest_md}")
    print(f"latest_json: {latest_json}")

    if overall == "fail":
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())







