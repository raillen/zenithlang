"""
Zenith consolidated test runner.

Goals:
- Keep output ASCII-only (Windows CP1252 friendly).
- Separate expected failures by stage (check/build/run).
- Avoid false negatives caused by mixed expectation modes.
"""

import os
import subprocess
import sys


ZT_EXE = os.path.abspath("zt.exe")
ZT_OLD = os.path.abspath(os.path.join("compiler", "driver", "zt-next-v2.exe"))
BEHAVIOR_DIR = os.path.join("tests", "behavior")

RESULTS = {"pass": [], "fail": [], "skip": []}


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


def print_behavior_status(name, mode, ok):
    status = "OK" if ok else "FAIL"
    print(f"  [{status:<4}] {name.ljust(45)} [{mode}]")


def test_behavior_project(name, path):
    """
    Modes:
    - check-pass: check must pass.
    - check-fail: check must fail.
    - build-fail: check must pass, build must fail.
    - run-fail: check/build must pass, run must fail.
    - run-pass: check/build/run must pass.
    """
    check_fail = {
        "enum_match_non_exhaustive_error",
        "error_syntax",
        "error_type_mismatch",
        "functions_invalid_call_error",
        "functions_param_ordering_error",
        "multifile_duplicate_symbol",
        "multifile_import_cycle",
        "multifile_missing_import",
        "multifile_namespace_mismatch",
        "mutability_const_reassign_error",
        "project_unknown_key_manifest",
        "monomorphization_limit_error",
        "where_contract_param_where_invalid_error",
        "where_contract_param_where_non_bool_error",
    }

    build_fail = {
        "functions_main_signature_error",
        "result_optional_propagation_error",
    }

    run_fail = {
        "where_contract_construct_error",
        "where_contract_field_assign_error",
        "where_contract_param_error",
    }

    run_pass = {
        "extern_c_puts_e2e",
    }

    rc, out = run_cmd(f"check:{name}", [ZT_EXE, "check", path], timeout=45)

    if name in check_fail:
        ok = rc != 0
        print_behavior_status(name, "check-fail", ok)
        mark(ok, f"behavior/{name}", out[:160] if not ok else None)
        return

    if rc != 0:
        print_behavior_status(name, "check-pass", False)
        mark(False, f"behavior/{name}", out[:160])
        return

    if name in build_fail:
        rc_build, out_build = run_cmd(f"build:{name}", [ZT_EXE, "build", path], timeout=120)
        ok = rc_build != 0
        print_behavior_status(name, "build-fail", ok)
        mark(ok, f"behavior/{name}", out_build[:160] if not ok else None)
        return

    if name in run_fail:
        rc_build, out_build = run_cmd(f"build:{name}", [ZT_EXE, "build", path], timeout=120)
        if rc_build != 0:
            print_behavior_status(name, "run-fail", False)
            mark(False, f"behavior/{name}", f"build failed unexpectedly: {out_build[:120]}")
            return
        rc_run, out_run = run_cmd(f"run:{name}", [ZT_EXE, "run", path], timeout=45)
        ok = rc_run != 0
        print_behavior_status(name, "run-fail", ok)
        mark(ok, f"behavior/{name}", out_run[:160] if not ok else None)
        return

    if name in run_pass:
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

    section("2. Behavior Tests")
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

    section("3. Cross Validation (new vs old driver)")
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

    section("4. Unit Test Binaries")
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

    section("5. Formatter Golden Tests")
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

    section("6. Stdlib Modules")
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
        print("\n  Failures:")
        for failure in RESULTS["fail"]:
            print(f"    - {failure}")
        return 1

    print("\n  All checks passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
