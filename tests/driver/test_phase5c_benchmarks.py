#!/usr/bin/env python3
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "benchmarks" / "manifest.json"


def fail(message):
    print(f"FAIL: {message}", file=sys.stderr)
    return 1


def load_manifest():
    if not MANIFEST.exists():
        raise AssertionError(f"missing manifest: {MANIFEST}")
    return json.loads(MANIFEST.read_text(encoding="utf-8"))


def check_manifest(manifest):
    if manifest.get("phase") != "5C":
        raise AssertionError("manifest phase must be 5C")
    if manifest.get("regression_detection_only") is not True:
        raise AssertionError("benchmarks must be marked as regression-only")
    if manifest.get("not_for_marketing") is not True:
        raise AssertionError("benchmarks must be marked as not for marketing")

    items = {item.get("id"): item for item in manifest.get("items", [])}
    expected = {f"K.0{i}" for i in range(1, 7)}
    missing = sorted(expected - set(items))
    if missing:
        raise AssertionError(f"missing Phase 5C items: {', '.join(missing)}")

    for item_id in sorted(expected):
        item = items[item_id]
        if item.get("status") != "done":
            raise AssertionError(f"{item_id} is not done in manifest")
        for raw in item.get("paths", []):
            path = ROOT / raw
            if not path.exists():
                raise AssertionError(f"{item_id} references missing path: {raw}")

    groups = manifest.get("benchmark_groups", {})
    required_benchmarks = {
        "micro_algorithm_core",
        "macro_small_build_cold",
        "macro_medium_check",
        "macro_large_check",
        "macro_medium_run",
    }
    listed = set()
    for values in groups.values():
        for value in values:
            if not value.endswith(".py") and "/" not in value:
                listed.add(value)
    missing_benchmarks = sorted(required_benchmarks - listed)
    if missing_benchmarks:
        raise AssertionError(f"manifest misses benchmarks: {', '.join(missing_benchmarks)}")


def check_runner_list(manifest):
    result = subprocess.run(
        [sys.executable, str(ROOT / "tests" / "perf" / "run_perf.py"), "--list"],
        cwd=str(ROOT),
        text=True,
        capture_output=True,
        timeout=60,
        check=False,
    )
    if result.returncode != 0:
        raise AssertionError(f"run_perf.py --list failed:\n{result.stdout}\n{result.stderr}")

    output = result.stdout
    for benchmark in manifest.get("benchmark_groups", {}).get("micro", []):
        if benchmark not in output:
            raise AssertionError(f"run_perf.py --list does not include {benchmark}")
    for benchmark in ["macro_small_build_cold", "macro_medium_check", "macro_large_check", "macro_medium_run"]:
        if benchmark not in output:
            raise AssertionError(f"run_perf.py --list does not include {benchmark}")


def check_cross_language_spec():
    path = ROOT / "benchmarks" / "cross-language" / "problems.json"
    data = json.loads(path.read_text(encoding="utf-8"))
    languages = {"zenith", "python", "go", "kotlin"}
    problems = data.get("problems", [])
    if len(problems) < 4:
        raise AssertionError("cross-language problem set must include at least four problems")
    for problem in problems:
        declared = set(problem.get("languages", {}))
        missing = languages - declared
        if missing:
            raise AssertionError(f"{problem.get('id')} misses languages: {', '.join(sorted(missing))}")


def main():
    try:
        manifest = load_manifest()
        check_manifest(manifest)
        check_runner_list(manifest)
        check_cross_language_spec()
    except Exception as exc:
        return fail(str(exc))

    print("phase5c benchmarks metadata ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
