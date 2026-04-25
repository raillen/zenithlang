"""
Concurrent native build checks for runtime cache locking and driver isolation.
"""

from __future__ import annotations

import os
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT_EXE = ROOT / "zt.exe"
TMP_DIR = ROOT / ".ztc-tmp" / "hardening-concurrent"
RUNTIME_DIR = ROOT / ".ztc-tmp" / "runtime"
RUNTIME_OBJ = RUNTIME_DIR / "zenith_rt.o"
SPILL_THRESHOLD = "512"

BUILD_CASES = [
    ("simple_app", "tests/behavior/simple_app", 42),
    ("edge_boundaries_empty", "tests/behavior/edge_boundaries_empty", 0),
    ("fmt_interpolation_basic", "tests/behavior/fmt_interpolation_basic", 0),
    ("std_text_basic", "tests/behavior/std_text_basic", 0),
    ("std_collections_queue_stack_cow", "tests/behavior/std_collections_queue_stack_cow", 0),
    ("std_bytes_ops", "tests/behavior/std_bytes_ops", 7),
]


def run_cmd(args: list[str], timeout: int = 240, env_override: dict[str, str] | None = None) -> tuple[int, str]:
    env = None
    if env_override:
        env = dict(os.environ)
        env.update(env_override)

    completed = subprocess.run(
        args,
        cwd=str(ROOT),
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        timeout=timeout,
        env=env,
    )
    return completed.returncode, (completed.stdout or "") + (completed.stderr or "")


def fail(message: str) -> int:
    print(message, file=sys.stderr)
    return 1


def build_case(case_name: str, project: str, wave: str) -> tuple[str, int, str, Path]:
    exe_path = TMP_DIR / wave / f"{case_name}.exe"
    exe_path.parent.mkdir(parents=True, exist_ok=True)
    rc, out = run_cmd(
        [str(ZT_EXE), "build", project, "-o", str(exe_path)],
        timeout=240,
        env_override={"ZT_EMITTER_SPILL_THRESHOLD_BYTES": SPILL_THRESHOLD},
    )
    return case_name, rc, out, exe_path


def validate_built_exe(case_name: str, exe_path: Path, expected_exit: int) -> tuple[bool, str]:
    if not exe_path.exists():
        return False, f"missing built executable for {case_name}: {exe_path}"

    rc, out = run_cmd([str(exe_path)], timeout=120)
    if rc != expected_exit:
        return False, (
            f"unexpected exit for {case_name}: expected {expected_exit}, got {rc}\n"
            f"{out[:300]}"
        )

    return True, ""


def run_wave(wave: str) -> tuple[bool, str]:
    built = {}

    with ThreadPoolExecutor(max_workers=len(BUILD_CASES)) as pool:
        futures = {
            pool.submit(build_case, case_name, project, wave): (case_name, expected_exit)
            for case_name, project, expected_exit in BUILD_CASES
        }
        for future in as_completed(futures):
            case_name, expected_exit = futures[future]
            built_name, rc, out, exe_path = future.result()
            if rc != 0:
                return False, f"concurrent build failed for {built_name} ({wave})\n{out[:500]}"
            built[built_name] = (exe_path, expected_exit)

    for case_name, exe_path_expected in built.items():
        exe_path, expected_exit = exe_path_expected
        ok, detail = validate_built_exe(case_name, exe_path, expected_exit)
        if not ok:
            return False, f"{detail}\nwave={wave}"

    lock_files = sorted(RUNTIME_DIR.glob("*.lock"))
    if lock_files:
        names = ", ".join(str(path.name) for path in lock_files)
        return False, f"stale runtime cache lock(s) after concurrent builds: {names}"

    if not RUNTIME_OBJ.exists():
        return False, f"runtime object missing after concurrent builds: {RUNTIME_OBJ}"

    return True, ""


def main() -> int:
    if not ZT_EXE.exists():
        return fail(f"missing compiler binary: {ZT_EXE}")

    shutil.rmtree(TMP_DIR, ignore_errors=True)
    shutil.rmtree(RUNTIME_DIR, ignore_errors=True)

    cold_ok, cold_detail = run_wave("cold_cache")
    if not cold_ok:
        return fail(cold_detail)

    warm_ok, warm_detail = run_wave("warm_cache")
    if not warm_ok:
        return fail(warm_detail)

    print("concurrent compilation checks ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
