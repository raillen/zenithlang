"""
R2.M1 - Zenith Test Suite Runner 2.0

Usage:
  python run_suite.py smoke
  python run_suite.py pr_gate
  python run_suite.py nightly
  python run_suite.py stress

Options:
  --no-perf        Skip performance benchmarks
  --report-dir     Override report output directory (default: reports/suites)
  --artifacts-dir  Override failure artifacts directory (default: reports/suites/artifacts)
  --seed           Override global seed for stress/fuzz tests

Exit codes:
  0  all tests passed
  1  one or more tests failed
  2  suite time limit exceeded
"""

import argparse
import json
import os
import re
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT / "tests" / "suites"))

from suite_definitions import (
    SUITE_SMOKE, SUITE_PR_GATE, SUITE_NIGHTLY, SUITE_STRESS,
    SUITE_TIME_LIMITS, STABLE_SEEDS,
    BEHAVIOR_CHECK_FAIL, BEHAVIOR_BUILD_FAIL, BEHAVIOR_RUN_FAIL, BEHAVIOR_RUN_PASS,
    BEHAVIOR_DIAGNOSTIC_FRAGMENT_FILES,
    BEHAVIOR_SMOKE_SUBSET,
    behavior_suites, behavior_timeout,
    LAYER_FRONTEND, LAYER_SEMANTIC, LAYER_HIR_ZIR, LAYER_BACKEND,
    LAYER_RUNTIME, LAYER_STDLIB, LAYER_TOOLING, LAYER_PERF,
)

ZT_EXE = ROOT / "zt.exe"
BEHAVIOR_DIR = ROOT / "tests" / "behavior"
DEFAULT_REPORT_DIR = ROOT / "reports" / "suites"
DEFAULT_ARTIFACTS_DIR = ROOT / "reports" / "suites" / "artifacts"
DEFAULT_PERF_OVERRIDE = ROOT / "docs" / "governance" / "perf-overrides" / "R3.M9-release-baseline-acceptance.md"

VALID_SUITES = [SUITE_SMOKE, SUITE_PR_GATE, SUITE_NIGHTLY, SUITE_STRESS]

# ---------------------------------------------------------------------------
# Failure classes for per-class counting
# ---------------------------------------------------------------------------
FAIL_CLASS_TIMEOUT       = "timeout"
FAIL_CLASS_CRASH         = "crash"
FAIL_CLASS_WRONG_EXIT    = "wrong_exit"
FAIL_CLASS_WRONG_OUTPUT  = "wrong_output"
FAIL_CLASS_INFRA         = "infra"
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def safe_print(msg=""):
    try:
        print(msg)
    except UnicodeEncodeError:
        enc = getattr(sys.stdout, "encoding", None) or "utf-8"
        print(str(msg).encode(enc, errors="replace").decode(enc, errors="replace"))


def section(title):
    print("\n" + "=" * 64)
    print(f"  {title}")
    print("=" * 64)


def run_cmd(cmd, cwd=".", timeout=60):
    """Run a command, return (returncode, output, elapsed_ms, fail_class)."""
    t0 = time.monotonic()
    try:
        completed = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=timeout,
            cwd=str(cwd),
        )
        elapsed_ms = int((time.monotonic() - t0) * 1000)
        output = ((completed.stdout or "") + (completed.stderr or "")).strip()
        fail_class = None
        if completed.returncode < 0:
            fail_class = FAIL_CLASS_CRASH
        return completed.returncode, output, elapsed_ms, fail_class
    except subprocess.TimeoutExpired:
        elapsed_ms = int((time.monotonic() - t0) * 1000)
        return 124, "TIMEOUT", elapsed_ms, FAIL_CLASS_TIMEOUT
    except Exception as exc:
        elapsed_ms = int((time.monotonic() - t0) * 1000)
        return 125, str(exc), elapsed_ms, FAIL_CLASS_INFRA


def save_artifact(artifacts_dir, test_id, stage, output):
    """Save failure output to artifacts directory."""
    artifacts_dir = Path(artifacts_dir)
    artifacts_dir.mkdir(parents=True, exist_ok=True)
    ts = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    fname = artifacts_dir / f"{test_id}__{stage}__{ts}.txt"
    try:
        fname.write_text(output, encoding="utf-8", errors="replace")
    except Exception:
        pass
    return str(fname)


def normalize_for_match(text: str) -> str:
    clean = ANSI_RE.sub("", text or "")
    clean = clean.replace("\r\n", "\n")
    clean = clean.replace("\\", "/")
    return clean


def behavior_diag_expectation(name: str, output: str):
    rel_path = BEHAVIOR_DIAGNOSTIC_FRAGMENT_FILES.get(name)
    if not rel_path:
        return True, []

    frag_path = ROOT / rel_path
    if not frag_path.exists():
        return False, [f"diagnostic fragment file not found: {rel_path}"]

    hay = normalize_for_match(output)
    missing = []
    for raw in frag_path.read_text(encoding="utf-8", errors="replace").splitlines():
        frag = raw.strip()
        if not frag:
            continue
        if frag.replace("\\", "/") not in hay:
            missing.append(frag)

    return len(missing) == 0, missing


# ---------------------------------------------------------------------------
# Result accumulator
# ---------------------------------------------------------------------------

class Results:
    def __init__(self):
        self.entries = []  # list of dicts

    def add(self, test_id, layer, kind, suite, stage, passed, elapsed_ms,
            fail_class=None, detail=None, artifact=None):
        self.entries.append({
            "test_id":    test_id,
            "layer":      layer,
            "kind":       kind,
            "suite":      suite,
            "stage":      stage,
            "passed":     passed,
            "elapsed_ms": elapsed_ms,
            "fail_class": fail_class,
            "detail":     detail,
            "artifact":   artifact,
        })

    def passed(self):
        return [e for e in self.entries if e["passed"]]

    def failed(self):
        return [e for e in self.entries if not e["passed"]]

    def by_layer(self):
        layers = {}
        for e in self.entries:
            layers.setdefault(e["layer"], []).append(e)
        return layers

    def by_fail_class(self):
        classes = {}
        for e in self.failed():
            fc = e["fail_class"] or FAIL_CLASS_WRONG_EXIT
            classes.setdefault(fc, []).append(e)
        return classes


# ---------------------------------------------------------------------------
# Behavior test runner
# ---------------------------------------------------------------------------

def run_behavior_test(name, path, suite, results, artifacts_dir):
    timeout_check = behavior_timeout(name, suite)
    timeout_build = 120
    timeout_run   = 45

    rc, out, ms, fc = run_cmd([str(ZT_EXE), "check", str(path)], timeout=timeout_check)

    if name in BEHAVIOR_CHECK_FAIL:
        ok = rc != 0
        if ok:
            diag_ok, missing = behavior_diag_expectation(name, out)
            if not diag_ok:
                ok = False
                out = out + "\n\nmissing diagnostic fragments:\n" + "\n".join(missing)
        fc2 = None if ok else (fc or FAIL_CLASS_WRONG_OUTPUT)
        artifact = save_artifact(artifacts_dir, f"behavior__{name}", "check", out) if not ok else None
        results.add(f"behavior/{name}", LAYER_FRONTEND, "behavior", suite, "check-fail",
                    ok, ms, fc2, out[:200] if not ok else None, artifact)
        status = "OK  " if ok else "FAIL"
        print(f"  [{status}] {name:<50} [check-fail] {ms}ms")
        return

    if rc != 0:
        fc2 = fc or FAIL_CLASS_WRONG_EXIT
        artifact = save_artifact(artifacts_dir, f"behavior__{name}", "check", out)
        results.add(f"behavior/{name}", LAYER_FRONTEND, "behavior", suite, "check-pass",
                    False, ms, fc2, out[:200], artifact)
        print(f"  [FAIL] {name:<50} [check-pass] {ms}ms")
        return

    if name in BEHAVIOR_BUILD_FAIL:
        rc2, out2, ms2, fc2 = run_cmd([str(ZT_EXE), "build", str(path)], timeout=timeout_build)
        ok = rc2 != 0
        if ok:
            diag_ok, missing = behavior_diag_expectation(name, out2)
            if not diag_ok:
                ok = False
                out2 = out2 + "\n\nmissing diagnostic fragments:\n" + "\n".join(missing)
        fc3 = None if ok else (fc2 or FAIL_CLASS_WRONG_EXIT)
        artifact = save_artifact(artifacts_dir, f"behavior__{name}", "build", out2) if not ok else None
        results.add(f"behavior/{name}", LAYER_BACKEND, "behavior", suite, "build-fail",
                    ok, ms + ms2, fc3, out2[:200] if not ok else None, artifact)
        status = "OK  " if ok else "FAIL"
        print(f"  [{status}] {name:<50} [build-fail] {ms+ms2}ms")
        return

    if name in BEHAVIOR_RUN_FAIL:
        rc2, out2, ms2, fc2 = run_cmd([str(ZT_EXE), "build", str(path)], timeout=timeout_build)
        if rc2 != 0:
            artifact = save_artifact(artifacts_dir, f"behavior__{name}", "build", out2)
            results.add(f"behavior/{name}", LAYER_BACKEND, "behavior", suite, "run-fail",
                        False, ms + ms2, fc2 or FAIL_CLASS_WRONG_EXIT,
                        f"build failed unexpectedly: {out2[:120]}", artifact)
            print(f"  [FAIL] {name:<50} [run-fail/build] {ms+ms2}ms")
            return
        rc3, out3, ms3, fc3 = run_cmd([str(ZT_EXE), "run", str(path)], timeout=timeout_run)
        ok = rc3 != 0
        if ok:
            diag_ok, missing = behavior_diag_expectation(name, out3)
            if not diag_ok:
                ok = False
                out3 = out3 + "\n\nmissing diagnostic fragments:\n" + "\n".join(missing)
        fc4 = None if ok else (fc3 or FAIL_CLASS_WRONG_EXIT)
        artifact = save_artifact(artifacts_dir, f"behavior__{name}", "run", out3) if not ok else None
        results.add(f"behavior/{name}", LAYER_RUNTIME, "behavior", suite, "run-fail",
                    ok, ms + ms2 + ms3, fc4, out3[:200] if not ok else None, artifact)
        status = "OK  " if ok else "FAIL"
        print(f"  [{status}] {name:<50} [run-fail] {ms+ms2+ms3}ms")
        return

    if name in BEHAVIOR_RUN_PASS:
        ok_check = "verification ok" in out or "check ok" in out
        if not ok_check:
            artifact = save_artifact(artifacts_dir, f"behavior__{name}", "check", out)
            results.add(f"behavior/{name}", LAYER_FRONTEND, "behavior", suite, "run-pass",
                        False, ms, FAIL_CLASS_WRONG_OUTPUT, out[:200], artifact)
            print(f"  [FAIL] {name:<50} [run-pass/check] {ms}ms")
            return
        rc2, out2, ms2, fc2 = run_cmd([str(ZT_EXE), "build", str(path)], timeout=timeout_build)
        if rc2 != 0:
            artifact = save_artifact(artifacts_dir, f"behavior__{name}", "build", out2)
            results.add(f"behavior/{name}", LAYER_BACKEND, "behavior", suite, "run-pass",
                        False, ms + ms2, fc2 or FAIL_CLASS_WRONG_EXIT,
                        f"build failed: {out2[:120]}", artifact)
            print(f"  [FAIL] {name:<50} [run-pass/build] {ms+ms2}ms")
            return
        rc3, out3, ms3, fc3 = run_cmd([str(ZT_EXE), "run", str(path)], timeout=timeout_run)
        ok = rc3 == 0
        fc4 = None if ok else (fc3 or FAIL_CLASS_WRONG_EXIT)
        artifact = save_artifact(artifacts_dir, f"behavior__{name}", "run", out3) if not ok else None
        results.add(f"behavior/{name}", LAYER_RUNTIME, "behavior", suite, "run-pass",
                    ok, ms + ms2 + ms3, fc4, out3[:200] if not ok else None, artifact)
        status = "OK  " if ok else "FAIL"
        print(f"  [{status}] {name:<50} [run-pass] {ms+ms2+ms3}ms")
        return

    ok = "verification ok" in out or "check ok" in out
    fc2 = None if ok else (fc or FAIL_CLASS_WRONG_OUTPUT)
    artifact = save_artifact(artifacts_dir, f"behavior__{name}", "check", out) if not ok else None
    results.add(f"behavior/{name}", LAYER_FRONTEND, "behavior", suite, "check-pass",
                ok, ms, fc2, out[:200] if not ok else None, artifact)
    status = "OK  " if ok else "FAIL"
    print(f"  [{status}] {name:<50} [check-pass] {ms}ms")


# ---------------------------------------------------------------------------
# Suite runners
# ---------------------------------------------------------------------------

def run_behavior_section(suite, results, artifacts_dir):
    section(f"Behavior Tests [{suite}]")
    if not BEHAVIOR_DIR.is_dir():
        print("  [SKIP] tests/behavior not found")
        return

    for name in sorted(os.listdir(BEHAVIOR_DIR)):
        path = BEHAVIOR_DIR / name
        if not path.is_dir():
            continue
        if not (path / "zenith.ztproj").exists():
            continue
        if suite not in behavior_suites(name):
            continue
        run_behavior_test(name, path, suite, results, artifacts_dir)


def run_unit_section(suite, results, artifacts_dir):
    if suite == SUITE_SMOKE:
        return
    section(f"Unit Test Binaries [{suite}]")
    skip_bins = {"test_m16.exe"}
    found = False
    for root, _dirs, files in os.walk(ROOT / "tests"):
        for filename in sorted(files):
            if filename.endswith(".exe") and filename.startswith("test_"):
                found = True
                if filename in skip_bins:
                    print(f"  [SKIP] {filename} (legacy harness)")
                    continue
                exe = Path(root) / filename
                rc, out, ms, fc = run_cmd([str(exe)], timeout=60)
                ok = rc == 0
                fc2 = None if ok else (fc or FAIL_CLASS_WRONG_EXIT)
                artifact = save_artifact(artifacts_dir, f"unit__{filename}", "run", out) if not ok else None
                results.add(f"unit/{filename}", _infer_layer(filename), "unit", suite, "run",
                            ok, ms, fc2, out[:200] if not ok else None, artifact)
                status = "OK  " if ok else "FAIL"
                print(f"  [{status}] {filename:<50} {ms}ms")
    if not found:
        print("  [SKIP] no compiled unit test binaries found")


def run_formatter_section(suite, results, artifacts_dir):
    if suite == SUITE_SMOKE:
        return
    section(f"Formatter Golden Tests [{suite}]")
    runner = ROOT / "tests" / "formatter" / "run_formatter_golden.py"
    if not runner.exists():
        print("  [SKIP] formatter golden runner not found")
    else:
        rc, out, ms, fc = run_cmd([sys.executable, str(runner)], timeout=120)
        ok = rc == 0
        fc2 = None if ok else (fc or FAIL_CLASS_WRONG_OUTPUT)
        artifact = save_artifact(artifacts_dir, "formatter__golden", "run", out) if not ok else None
        results.add("formatter/golden", LAYER_TOOLING, "formatter", suite, "run",
                    ok, ms, fc2, out[:400] if not ok else None, artifact)
        status = "OK  " if ok else "FAIL"
        print(f"  [{status}] formatter golden {ms}ms")

    idem_runner = ROOT / "tests" / "formatter" / "run_formatter_idempotence.py"
    if not idem_runner.exists():
        print("  [SKIP] formatter idempotence runner not found")
        return
    rc, out, ms, fc = run_cmd([sys.executable, str(idem_runner)], timeout=180)
    ok = rc == 0
    fc2 = None if ok else (fc or FAIL_CLASS_WRONG_OUTPUT)
    artifact = save_artifact(artifacts_dir, "formatter__idempotence", "run", out) if not ok else None
    results.add("formatter/idempotence", LAYER_TOOLING, "formatter", suite, "run",
                ok, ms, fc2, out[:400] if not ok else None, artifact)
    status = "OK  " if ok else "FAIL"
    print(f"  [{status}] formatter idempotence {ms}ms")


def run_perf_section(suite, results, artifacts_dir, no_perf=False):
    if no_perf or suite == SUITE_SMOKE:
        return
    perf_suite = "nightly" if suite in (SUITE_NIGHTLY, SUITE_STRESS) else "quick"
    section(f"Performance Gate [{suite} -> perf:{perf_suite}]")
    runner = ROOT / "tests" / "perf" / "run_perf.py"
    if not runner.exists():
        print("  [SKIP] tests/perf/run_perf.py not found")
        return
    cmd = [sys.executable, str(runner), "--suite", perf_suite, "--release-gate"]
    if DEFAULT_PERF_OVERRIDE.exists():
        cmd.extend(["--override-file", str(DEFAULT_PERF_OVERRIDE)])
    timeout = 900 if perf_suite == "nightly" else 180
    rc, out, ms, fc = run_cmd(cmd, timeout=timeout)
    ok = rc == 0
    fc2 = None if ok else (fc or FAIL_CLASS_WRONG_EXIT)
    artifact = save_artifact(artifacts_dir, f"perf__{perf_suite}", "run", out) if not ok else None
    results.add(f"perf/{perf_suite}", LAYER_PERF, "perf", suite, "run",
                ok, ms, fc2, out[:400] if not ok else None, artifact)
    status = "OK  " if ok else "FAIL"
    print(f"  [{status}] perf/{perf_suite} {ms}ms")


def _run_fuzz_driver(driver_rel, test_id, suite, results, artifacts_dir,
                     iters, seed, timeout_sec, layer=LAYER_FRONTEND):
    """Invoke a fuzz driver with stable seed/iterations and record result."""
    runner = driver_rel if isinstance(driver_rel, Path) else (ROOT / "tests" / "fuzz" / driver_rel)
    if not runner.exists():
        print(f"  [SKIP] {driver_rel} not found")
        results.add(test_id, layer, "fuzz", suite, "fuzz",
                    True, 0, None, f"missing {driver_rel}", None)
        return
    cmd = [sys.executable, str(runner),
           "--iters", str(iters), "--seed", str(seed)]
    rc, out, ms, fc = run_cmd(cmd, timeout=timeout_sec)
    ok = rc == 0
    fc2 = None if ok else (fc or FAIL_CLASS_WRONG_EXIT)
    artifact = save_artifact(artifacts_dir, test_id.replace("/", "__"), "fuzz", out) if not ok else None
    results.add(test_id, layer, "fuzz", suite, "fuzz",
                ok, ms, fc2, out[:400] if not ok else None, artifact)
    status = "OK  " if ok else "FAIL"
    print(f"  [{status}] {test_id} iters={iters} seed={seed} {ms}ms")


def _run_fuzz_replay(test_id, suite, results, artifacts_dir, timeout_sec):
    """Replay persisted crash seeds (must stay green on every relevant run)."""
    runner = ROOT / "tests" / "fuzz" / "replay.py"
    if not runner.exists():
        print(f"  [SKIP] tests/fuzz/replay.py not found")
        results.add(test_id, LAYER_FRONTEND, "fuzz", suite, "replay",
                    True, 0, None, "missing replay.py", None)
        return
    rc, out, ms, fc = run_cmd([sys.executable, str(runner)], timeout=timeout_sec)
    ok = rc == 0
    fc2 = None if ok else (fc or FAIL_CLASS_WRONG_EXIT)
    artifact = save_artifact(artifacts_dir, test_id.replace("/", "__"), "replay", out) if not ok else None
    results.add(test_id, LAYER_FRONTEND, "fuzz", suite, "replay",
                ok, ms, fc2, out[:400] if not ok else None, artifact)
    status = "OK  " if ok else "FAIL"
    print(f"  [{status}] {test_id} {ms}ms")


def run_fuzz_replay_section(suite, results, artifacts_dir):
    """R2.M2: replay persisted crash seeds in every relevant suite."""
    if suite == SUITE_SMOKE:
        return
    section(f"Fuzz Replay [{suite}]")
    _run_fuzz_replay("fuzz/replay", suite, results, artifacts_dir, timeout_sec=120)


def run_stress_section(suite, results, artifacts_dir, seed=None):
    if suite not in (SUITE_NIGHTLY, SUITE_STRESS):
        return
    section(f"Stress / Fuzz Campaign [{suite}]")
    seed_l = seed or STABLE_SEEDS["fuzz_lexer"]
    seed_p = seed or STABLE_SEEDS["fuzz_parser"]
    seed_s = seed or STABLE_SEEDS["fuzz_semantic"]

    # Campaign sizes per suite (short for nightly, longer for stress)
    if suite == SUITE_STRESS:
        iters_l, iters_p, iters_s, to = 500, 500, 80, 1200
    else:  # nightly
        iters_l, iters_p, iters_s, to = 120, 120, 20, 300

    print(f"  [INFO] seeds lexer={seed_l} parser={seed_p} semantic={seed_s} (stable, override via --seed)")
    _run_fuzz_driver("fuzz_lexer.py",  "stress/fuzz_lexer",
                     suite, results, artifacts_dir, iters_l, seed_l, to)
    _run_fuzz_driver("fuzz_parser.py", "stress/fuzz_parser",
                     suite, results, artifacts_dir, iters_p, seed_p, to)
    _run_fuzz_driver(ROOT / "tests" / "heavy" / "fuzz" / "semantic" / "fuzz_semantic.py",
                     "stress/fuzz_semantic", suite, results, artifacts_dir,
                     iters_s, seed_s, to, layer=LAYER_SEMANTIC)


def run_hardening_section(suite, results, artifacts_dir):
    if suite == SUITE_SMOKE:
        return

    section(f"Hardening Tests [{suite}]")

    tasks = [
        ("driver/explain_cli", ROOT / "tests" / "driver" / "test_explain_cli.py", LAYER_TOOLING, 60, {SUITE_PR_GATE, SUITE_NIGHTLY, SUITE_STRESS}),
        ("hardening/determinism", ROOT / "tests" / "hardening" / "test_determinism.py", LAYER_TOOLING, 120, {SUITE_PR_GATE, SUITE_NIGHTLY, SUITE_STRESS}),
        ("hardening/roundtrip_emit_c", ROOT / "tests" / "hardening" / "test_roundtrip_emit_c.py", LAYER_BACKEND, 180, {SUITE_PR_GATE, SUITE_NIGHTLY, SUITE_STRESS}),
        ("hardening/differential_validate_between", ROOT / "tests" / "hardening" / "test_differential_validate_between.py", LAYER_RUNTIME, 240, {SUITE_PR_GATE, SUITE_NIGHTLY, SUITE_STRESS}),
        ("hardening/heavy_semantic_curated", ROOT / "tests" / "hardening" / "test_heavy_semantic_curated.py", LAYER_SEMANTIC, 240, {SUITE_NIGHTLY, SUITE_STRESS}),
        ("hardening/concurrent_compilation", ROOT / "tests" / "hardening" / "test_concurrent_compilation.py", LAYER_TOOLING, 300, {SUITE_PR_GATE, SUITE_NIGHTLY, SUITE_STRESS}),
        ("hardening/coverage_snapshot", ROOT / "tests" / "hardening" / "test_coverage_snapshot.py", LAYER_TOOLING, 900, {SUITE_NIGHTLY, SUITE_STRESS}),
        ("hardening/emitter_memory_profile", ROOT / "tests" / "hardening" / "test_emitter_memory_profile.py", LAYER_BACKEND, 900, {SUITE_NIGHTLY, SUITE_STRESS}),
        ("hardening/runtime_sanitizers", ROOT / "tests" / "hardening" / "test_runtime_sanitizers.py", LAYER_RUNTIME, 600, {SUITE_NIGHTLY, SUITE_STRESS}),
    ]

    for test_id, script_path, layer, timeout_sec, suites in tasks:
        if suite not in suites:
            continue
        if not script_path.exists():
            print(f"  [SKIP] {test_id} (missing {script_path})")
            continue
        rc, out, ms, fc = run_cmd([sys.executable, str(script_path)], timeout=timeout_sec)
        ok = rc == 0
        fc2 = None if ok else (fc or FAIL_CLASS_WRONG_EXIT)
        artifact = save_artifact(artifacts_dir, test_id.replace("/", "__"), "run", out) if not ok else None
        results.add(test_id, layer, "hardening", suite, "run",
                    ok, ms, fc2, out[:400] if not ok else None, artifact)
        status = "OK  " if ok else "FAIL"
        print(f"  [{status}] {test_id} {ms}ms")


# ---------------------------------------------------------------------------
# Reporting
# ---------------------------------------------------------------------------

def _infer_layer(filename):
    name = filename.lower()
    if "lexer" in name or "parser" in name or "ast" in name:
        return LAYER_FRONTEND
    if "binder" in name or "types" in name or "constraint" in name or "semantic" in name:
        return LAYER_SEMANTIC
    if "hir" in name or "zir" in name or "lowering" in name or "verifier" in name:
        return LAYER_HIR_ZIR
    if "emitter" in name or "legalization" in name or "backend" in name:
        return LAYER_BACKEND
    if "runtime" in name or "shared_text" in name:
        return LAYER_RUNTIME
    if "stdlib" in name or "format" in name or "validate" in name:
        return LAYER_STDLIB
    if "formatter" in name or "driver" in name or "project" in name:
        return LAYER_TOOLING
    return "other"


def print_report(suite, results, elapsed_total_ms):
    section("Summary")
    total   = len(results.entries)
    n_pass  = len(results.passed())
    n_fail  = len(results.failed())
    print(f"  Suite        : {suite}")
    print(f"  Total        : {total}")
    print(f"  Pass         : {n_pass}")
    print(f"  Fail         : {n_fail}")
    print(f"  Elapsed      : {elapsed_total_ms}ms")

    section("Per-Layer Report")
    for layer, entries in sorted(results.by_layer().items()):
        lp = sum(1 for e in entries if e["passed"])
        lf = sum(1 for e in entries if not e["passed"])
        bar = "OK  " if lf == 0 else "FAIL"
        print(f"  [{bar}] {layer:<20} pass={lp} fail={lf}")

    if results.failed():
        section("Failure Class Counts")
        for fc, entries in sorted(results.by_fail_class().items()):
            print(f"  {fc:<20} : {len(entries)}")

        section("Failures")
        for e in results.failed():
            safe_print(f"  - {e['test_id']} [{e['stage']}] class={e['fail_class'] or 'wrong_exit'}")
            if e["detail"]:
                safe_print(f"      {e['detail'][:120]}")
            if e["artifact"]:
                safe_print(f"      artifact: {e['artifact']}")


def save_report(suite, results, elapsed_total_ms, report_dir):
    report_dir = Path(report_dir)
    report_dir.mkdir(parents=True, exist_ok=True)
    ts = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    report = {
        "suite":       suite,
        "timestamp":   ts,
        "elapsed_ms":  elapsed_total_ms,
        "total":       len(results.entries),
        "pass":        len(results.passed()),
        "fail":        len(results.failed()),
        "by_layer":    {
            layer: {
                "pass": sum(1 for e in entries if e["passed"]),
                "fail": sum(1 for e in entries if not e["passed"]),
            }
            for layer, entries in results.by_layer().items()
        },
        "by_fail_class": {
            fc: len(entries)
            for fc, entries in results.by_fail_class().items()
        },
        "entries": results.entries,
    }
    path = report_dir / f"{suite}__{ts}.json"
    path.write_text(json.dumps(report, indent=2, default=str), encoding="utf-8")
    latest = report_dir / f"{suite}__latest.json"
    latest.write_text(json.dumps(report, indent=2, default=str), encoding="utf-8")
    print(f"\n  Report saved: {path}")
    print(f"  Latest link : {latest}")
    return path


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Zenith Suite Runner 2.0 (R2.M1)")
    parser.add_argument("suite", choices=VALID_SUITES,
                        help="Suite to run: smoke | pr_gate | nightly | stress")
    parser.add_argument("--no-perf", action="store_true",
                        help="Skip performance benchmarks")
    parser.add_argument("--report-dir", default=str(DEFAULT_REPORT_DIR),
                        help="Directory for JSON reports")
    parser.add_argument("--artifacts-dir", default=str(DEFAULT_ARTIFACTS_DIR),
                        help="Directory for failure artifacts")
    parser.add_argument("--seed", type=int, default=None,
                        help="Override global seed for stress/fuzz tests")
    args = parser.parse_args()

    suite        = args.suite
    time_limit   = SUITE_TIME_LIMITS[suite]
    report_dir   = Path(args.report_dir)
    artifacts_dir = Path(args.artifacts_dir)

    print(f"\nZenith Suite Runner 2.0  |  suite={suite}  |  time_limit={time_limit}s")
    print(f"  report_dir   : {report_dir}")
    print(f"  artifacts_dir: {artifacts_dir}")

    if not ZT_EXE.exists():
        print(f"\n[FATAL] zt.exe not found at {ZT_EXE}")
        sys.exit(2)

    results = Results()
    t_start = time.monotonic()

    run_behavior_section(suite, results, artifacts_dir)
    run_unit_section(suite, results, artifacts_dir)
    run_formatter_section(suite, results, artifacts_dir)
    run_hardening_section(suite, results, artifacts_dir)
    run_perf_section(suite, results, artifacts_dir, no_perf=args.no_perf)
    run_fuzz_replay_section(suite, results, artifacts_dir)
    run_stress_section(suite, results, artifacts_dir, seed=args.seed)

    elapsed_ms = int((time.monotonic() - t_start) * 1000)
    elapsed_s  = elapsed_ms / 1000

    print_report(suite, results, elapsed_ms)
    save_report(suite, results, elapsed_ms, report_dir)

    if elapsed_s > time_limit:
        print(f"\n[WARN] Suite exceeded time limit: {elapsed_s:.1f}s > {time_limit}s")

    if results.failed():
        sys.exit(1)
    print("\n  All checks passed.")
    sys.exit(0)


if __name__ == "__main__":
    main()
