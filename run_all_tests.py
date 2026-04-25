"""
Zenith consolidated test runner.

Goals:
- Keep output ASCII-only (Windows CP1252 friendly).
- Separate expected failures by stage (check/build/run).
- Avoid false negatives caused by mixed expectation modes.
"""

import os
import re
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT / "tests" / "suites"))
from suite_definitions import (  # noqa: E402
    BEHAVIOR_CHECK_FAIL,
    BEHAVIOR_BUILD_FAIL,
    BEHAVIOR_RUN_FAIL,
    BEHAVIOR_RUN_PASS,
    BEHAVIOR_DIAGNOSTIC_FRAGMENT_FILES,
)


ZT_EXE = os.path.abspath("zt.exe")
ZT_OLD = os.path.abspath(os.path.join("compiler", "driver", "zt-next-v2.exe"))
BEHAVIOR_DIR = os.path.join("tests", "behavior")
TOOLING_GATE_PROJECT = os.path.join("tests", "behavior", "tooling_gate_smoke")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")

RESULTS = {"pass": [], "fail": [], "skip": []}


def safe_print(message=""):
    try:
        print(message)
    except UnicodeEncodeError:
        encoding = getattr(sys.stdout, "encoding", None) or "utf-8"
        sanitized = str(message).encode(encoding, errors="replace").decode(encoding, errors="replace")
        print(sanitized)


def section(title):
    print("\n" + "=" * 60)
    print(f"  {title}")
    print("=" * 60)


def run_cmd(name, cmd, cwd=".", timeout=30):
    try:
        completed = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=timeout,
            cwd=cwd,
        )
        stdout = completed.stdout or ""
        stderr = completed.stderr or ""
        output = (stdout + stderr).strip()
        return completed.returncode, output
    except subprocess.TimeoutExpired:
        return 124, "TIMEOUT"
    except Exception as exc:  # pragma: no cover - defensive
        return 125, str(exc)


def mark(passed, key, detail=None):
    if passed:
        RESULTS["pass"].append(key)
    else:
        entry = key if detail is None else f"{key}: {detail}"
        RESULTS["fail"].append(entry)


def normalize_for_match(text):
    clean = ANSI_RE.sub("", text or "")
    clean = clean.replace("\r\n", "\n")
    clean = clean.replace("\\", "/")
    return clean


def behavior_diag_expectation(name, output):
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


def print_behavior_status(name, mode, ok):
    status = "OK" if ok else "FAIL"
    print(f"  [{status:<4}] {name.ljust(45)} [{mode}]")


def print_contributor_next_steps():
    if not RESULTS["fail"]:
        return

    print("\n  Next steps (contributor-friendly):")
    emitted = set()

    for failure in RESULTS["fail"]:
        key = failure.split(": ", 1)[0]

        if key in emitted:
            continue

        if key == "compiler_rebuild":
            print("    - Rebuild compiler: python build.py")
            emitted.add(key)
            continue

        if key == "tooling/fmt_check":
            print(f"    - Fix formatting gate: {ZT_EXE} fmt {TOOLING_GATE_PROJECT} --check")
            print(f"      If failing: {ZT_EXE} fmt {TOOLING_GATE_PROJECT}")
            emitted.add(key)
            continue

        if key == "tooling/doc_check":
            print(f"    - Fix docs gate: {ZT_EXE} doc check {TOOLING_GATE_PROJECT}")
            emitted.add(key)
            continue

        if key == "tooling/lsp_smoke":
            print("    - Re-run LSP smoke: python tests/lsp/test_lsp_smoke.py")
            print("      If build fails: python tools/build_lsp.py")
            emitted.add(key)
            continue

        if key == "tooling/vscode_extension_syntax":
            print("    - Recheck VSCode extension syntax: node --check tools/vscode-zenith/extension.js")
            emitted.add(key)
            continue

        if key.startswith("behavior/"):
            behavior_name = key.split("/", 1)[1]
            project_path = os.path.join(BEHAVIOR_DIR, behavior_name)
            print(f"    - Recheck behavior: {ZT_EXE} check {project_path}")
            print(f"      Rerun behavior:  {ZT_EXE} run {project_path}")
            emitted.add(key)
            continue

        if key == "formatter/golden":
            print("    - Re-run formatter golden: python tests/formatter/run_formatter_golden.py")
            emitted.add(key)
            continue

        if key.startswith("unit/"):
            print(f"    - Re-run unit binary: {key.split('/', 1)[1]}")
            emitted.add(key)
            continue

        if key.startswith("hardening/"):
            script = key.split("/", 1)[1]
            print(f"    - Re-run hardening check: python tests/hardening/test_{script}.py")
            emitted.add(key)
            continue


def test_behavior_project(name, path):
    """
    Modes:
    - check-pass: check must pass.
    - check-fail: check must fail.
    - build-fail: check must pass, build must fail.
    - run-fail: check/build must pass, run must fail.
    - run-pass: check/build/run must pass.
    """
    rc, out = run_cmd(f"check:{name}", [ZT_EXE, "check", path], timeout=45)

    if name in BEHAVIOR_CHECK_FAIL:
        ok = rc != 0
        if ok:
            diag_ok, missing = behavior_diag_expectation(name, out)
            if not diag_ok:
                ok = False
                out = out + "\n\nmissing diagnostic fragments:\n" + "\n".join(missing)
        print_behavior_status(name, "check-fail", ok)
        mark(ok, f"behavior/{name}", out[:160] if not ok else None)
        return

    if rc != 0:
        print_behavior_status(name, "check-pass", False)
        mark(False, f"behavior/{name}", out[:160])
        return

    if name in BEHAVIOR_BUILD_FAIL:
        rc_build, out_build = run_cmd(f"build:{name}", [ZT_EXE, "build", path], timeout=120)
        ok = rc_build != 0
        if ok:
            diag_ok, missing = behavior_diag_expectation(name, out_build)
            if not diag_ok:
                ok = False
                out_build = out_build + "\n\nmissing diagnostic fragments:\n" + "\n".join(missing)
        print_behavior_status(name, "build-fail", ok)
        mark(ok, f"behavior/{name}", out_build[:160] if not ok else None)
        return

    if name in BEHAVIOR_RUN_FAIL:
        rc_build, out_build = run_cmd(f"build:{name}", [ZT_EXE, "build", path], timeout=120)
        if rc_build != 0:
            print_behavior_status(name, "run-fail", False)
            mark(False, f"behavior/{name}", f"build failed unexpectedly: {out_build[:120]}")
            return
        rc_run, out_run = run_cmd(f"run:{name}", [ZT_EXE, "run", path], timeout=45)
        ok = rc_run != 0
        if ok:
            diag_ok, missing = behavior_diag_expectation(name, out_run)
            if not diag_ok:
                ok = False
                out_run = out_run + "\n\nmissing diagnostic fragments:\n" + "\n".join(missing)
        print_behavior_status(name, "run-fail", ok)
        mark(ok, f"behavior/{name}", out_run[:160] if not ok else None)
        return

    if name in BEHAVIOR_RUN_PASS:
        ok_check = "verification ok" in out or "check ok" in out
        if not ok_check:
            print_behavior_status(name, "run-pass", False)
            mark(False, f"behavior/{name}", out[:160])
            return

        rc_build, out_build = run_cmd(f"build:{name}", [ZT_EXE, "build", path], timeout=120)
        if rc_build != 0:
            print_behavior_status(name, "run-pass", False)
            mark(False, f"behavior/{name}", f"build failed: {out_build[:120]}")
            return

        rc_run, out_run = run_cmd(f"run:{name}", [ZT_EXE, "run", path], timeout=45)
        ok = rc_run == 0
        print_behavior_status(name, "run-pass", ok)
        mark(ok, f"behavior/{name}", out_run[:160] if not ok else None)
        return

    ok = "verification ok" in out or "check ok" in out
    print_behavior_status(name, "check-pass", ok)
    mark(ok, f"behavior/{name}", out[:160] if not ok else None)


def main():
    section("1. Compiler Build Verification")

    if os.path.exists(ZT_EXE):
        print(f"  [OK  ] zt.exe found ({os.path.getsize(ZT_EXE)} bytes)")
        RESULTS["pass"].append("compiler_binary_exists")
    else:
        print("  [FAIL] zt.exe not found")
        RESULTS["fail"].append("compiler_binary_exists")

    print("  [....] rebuilding compiler")
    rc, out = run_cmd("rebuild", [sys.executable, "build.py"], timeout=120)
    rebuild_ok = rc == 0
    print(f"  [{'OK' if rebuild_ok else 'FAIL':<4}] compiler rebuild")
    mark(rebuild_ok, "compiler_rebuild", out[:200] if not rebuild_ok else None)

    section("2. Tooling Gates (fmt/doc)")
    if os.path.isdir(TOOLING_GATE_PROJECT):
        rc_fmt, out_fmt = run_cmd(
            "fmt-check",
            [ZT_EXE, "fmt", TOOLING_GATE_PROJECT, "--check"],
            timeout=60,
        )
        fmt_ok = rc_fmt == 0 and ("fmt check ok" in out_fmt or "fmt ok" in out_fmt)
        print(f"  [{'OK' if fmt_ok else 'FAIL':<4}] fmt --check ({TOOLING_GATE_PROJECT})")
        mark(fmt_ok, "tooling/fmt_check", out_fmt[:200] if not fmt_ok else None)

        rc_doc, out_doc = run_cmd(
            "doc-check",
            [ZT_EXE, "doc", "check", TOOLING_GATE_PROJECT],
            timeout=60,
        )
        doc_ok = rc_doc == 0 and "doc check ok" in out_doc
        print(f"  [{'OK' if doc_ok else 'FAIL':<4}] doc check ({TOOLING_GATE_PROJECT})")
        mark(doc_ok, "tooling/doc_check", out_doc[:200] if not doc_ok else None)
    else:
        print("  [SKIP] tooling gate project not found")
        RESULTS["skip"].append("tooling/gate_project_missing")

    section("3. LSP Smoke")
    lsp_smoke = os.path.join("tests", "lsp", "test_lsp_smoke.py")
    if os.path.exists(lsp_smoke):
        rc_lsp, out_lsp = run_cmd(
            "lsp-smoke",
            [sys.executable, lsp_smoke],
            timeout=60,
        )
        lsp_ok = rc_lsp == 0 and "lsp smoke ok" in out_lsp
        print(f"  [{'OK' if lsp_ok else 'FAIL':<4}] Compass LSP smoke")
        mark(lsp_ok, "tooling/lsp_smoke", out_lsp[:400] if not lsp_ok else None)
    else:
        print("  [SKIP] LSP smoke runner not found")
        RESULTS["skip"].append("tooling/lsp_smoke_missing")

    rc_node, _out_node = run_cmd("node-version", ["node", "--version"], timeout=10)
    if rc_node == 0 and os.path.exists(os.path.join("tools", "vscode-zenith", "extension.js")):
        rc_ext, out_ext = run_cmd(
            "vscode-extension-syntax",
            ["node", "--check", os.path.join("tools", "vscode-zenith", "extension.js")],
            timeout=30,
        )
        ext_ok = rc_ext == 0
        print(f"  [{'OK' if ext_ok else 'FAIL':<4}] VSCode extension syntax")
        mark(ext_ok, "tooling/vscode_extension_syntax", out_ext[:400] if not ext_ok else None)
    else:
        print("  [SKIP] VSCode extension syntax (node not available or extension missing)")
        RESULTS["skip"].append("tooling/vscode_extension_syntax")

    section("4. Behavior Tests")
    if not os.path.isdir(BEHAVIOR_DIR):
        print("  [FAIL] tests/behavior not found")
        RESULTS["fail"].append("behavior_root_missing")
    else:
        for test_name in sorted(os.listdir(BEHAVIOR_DIR)):
            test_path = os.path.join(BEHAVIOR_DIR, test_name)
            if not os.path.isdir(test_path):
                continue
            if not os.path.exists(os.path.join(test_path, "zenith.ztproj")):
                RESULTS["skip"].append(f"behavior/{test_name}")
                continue
            test_behavior_project(test_name, test_path)

    section("5. Cross Validation (new vs old driver)")
    if os.path.exists(ZT_OLD):
        for test_name in ["std_io_basic", "simple_app", "result_question_basic", "optional_result_basic"]:
            test_path = os.path.join(BEHAVIOR_DIR, test_name)
            if not os.path.isdir(test_path):
                continue
            rc_new, out_new = run_cmd(f"new:{test_name}", [ZT_EXE, "check", test_path], timeout=45)
            rc_old, out_old = run_cmd(f"old:{test_name}", [ZT_OLD, "check", test_path], timeout=45)
            new_ok = rc_new == 0 and ("verification ok" in out_new or "check ok" in out_new)
            old_ok = rc_old == 0 and ("verification ok" in out_old or "check ok" in out_old)
            print(f"  {test_name.ljust(40)} new={'OK' if new_ok else 'FAIL'} old={'OK' if old_ok else 'FAIL'}")
            mark(new_ok, f"crossval/{test_name}")
    else:
        print("  [SKIP] old driver not found (compiler/driver/zt-next-v2.exe)")
        RESULTS["skip"].append("crossval/old_driver_missing")

    section("6. Unit Test Binaries")
    unit_bins = []
    for root, _dirs, files in os.walk("tests"):
        for filename in files:
            if filename.endswith(".exe") and filename.startswith("test_"):
                unit_bins.append(os.path.join(root, filename))

    if not unit_bins:
        print("  [SKIP] no compiled unit test binaries found")
        RESULTS["skip"].append("unit/no_binaries")
    else:
        skip_bins = {"test_m16.exe"}
        for test_exe in sorted(unit_bins):
            name = os.path.basename(test_exe)
            if name in skip_bins:
                print(f"  [SKIP] {name} (legacy harness)")
                RESULTS["skip"].append(f"unit/{name}")
                continue
            rc, out = run_cmd(name, [test_exe], timeout=30)
            ok = rc == 0
            print(f"  [{'OK' if ok else 'FAIL':<4}] {name}")
            mark(ok, f"unit/{name}", out[:160] if not ok else None)

    section("7. Formatter Golden Tests")
    if os.path.exists(os.path.join("tests", "formatter", "run_formatter_golden.py")):
        rc, out = run_cmd(
            "formatter-golden",
            [sys.executable, os.path.join("tests", "formatter", "run_formatter_golden.py")],
            timeout=120,
        )
        ok = rc == 0
        print(f"  [{'OK' if ok else 'FAIL':<4}] formatter golden")
        mark(ok, "formatter/golden", out[:400] if not ok else None)
    else:
        print("  [SKIP] formatter golden runner not found")
        RESULTS["skip"].append("formatter/golden_missing")

    section("8. Hardening Tests")
    hardening_scripts = [
        ("hardening/determinism", os.path.join("tests", "hardening", "test_determinism.py"), 120),
        ("hardening/roundtrip_emit_c", os.path.join("tests", "hardening", "test_roundtrip_emit_c.py"), 180),
        ("hardening/differential_validate_between", os.path.join("tests", "hardening", "test_differential_validate_between.py"), 240),
        ("hardening/runtime_sanitizers", os.path.join("tests", "hardening", "test_runtime_sanitizers.py"), 600),
    ]
    for key, script_path, timeout in hardening_scripts:
        if not os.path.exists(script_path):
            print(f"  [SKIP] {key} (missing)")
            RESULTS["skip"].append(key)
            continue
        rc, out = run_cmd(key, [sys.executable, script_path], timeout=timeout)
        ok = rc == 0
        print(f"  [{'OK' if ok else 'FAIL':<4}] {key}")
        mark(ok, key, out[:400] if not ok else None)

    section("9. Stdlib Modules")
    if os.path.isdir("stdlib"):
        for root, _dirs, files in os.walk("stdlib"):
            for filename in sorted(files):
                if filename.endswith(".zt"):
                    full_path = os.path.join(root, filename)
                    print(f"  [INFO] {full_path} ({os.path.getsize(full_path)} bytes)")
                    RESULTS["pass"].append(f"stdlib/{filename}")
    else:
        print("  [SKIP] stdlib folder not found")
        RESULTS["skip"].append("stdlib/missing")

    section("Summary")
    total = len(RESULTS["pass"]) + len(RESULTS["fail"]) + len(RESULTS["skip"])
    print(f"  Total: {total}")
    print(f"  Pass : {len(RESULTS['pass'])}")
    print(f"  Fail : {len(RESULTS['fail'])}")
    print(f"  Skip : {len(RESULTS['skip'])}")

    if RESULTS["fail"]:
        safe_print("\n  Failures:")
        for failure in RESULTS["fail"]:
            safe_print(f"    - {failure}")
        print_contributor_next_steps()
        return 1

    print("\n  All checks passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
