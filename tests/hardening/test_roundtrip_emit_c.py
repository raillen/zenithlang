"""
Round-trip checks: emit C -> compile with host C compiler -> run.
"""

from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT_EXE = ROOT / "zt.exe"
TMP_DIR = ROOT / ".ztc-tmp" / "hardening-roundtrip"
RUNTIME_OBJ = ROOT / ".ztc-tmp" / "runtime" / "zenith_rt.o"


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


def compile_emitted_c(cc: str, source_path: Path, exe_path: Path) -> tuple[int, str]:
    cmd = [
        cc,
        "-Wall",
        "-Wextra",
        "-Wno-unused-function",
        "-I.",
        "-o",
        str(exe_path),
        str(source_path),
        str(RUNTIME_OBJ),
    ]
    if os.name == "nt":
        cmd.append("-lws2_32")
    return run_cmd(cmd, timeout=240)


def main() -> int:
    if not ZT_EXE.exists():
        return fail(f"missing compiler binary: {ZT_EXE}")

    cc = os.environ.get("CC", "gcc")
    if shutil.which(cc) is None:
        return fail(f"C compiler not found: {cc}")

    # Ensure runtime object exists.
    rc_boot, out_boot = run_cmd([str(ZT_EXE), "build", "tests/behavior/simple_app"], timeout=240)
    if rc_boot != 0:
        return fail(f"failed to bootstrap runtime object via build:\n{out_boot[:400]}")
    if not RUNTIME_OBJ.exists():
        return fail(f"runtime object not found: {RUNTIME_OBJ}")

    TMP_DIR.mkdir(parents=True, exist_ok=True)
    cases = [
        ("simple_app", 42),
        ("std_bytes_ops", 7),
        ("edge_boundaries_empty", 0),
    ]

    for name, expected_exit in cases:
        project = f"tests/behavior/{name}"
        rc_emit, out_emit = run_cmd([str(ZT_EXE), "emit-c", project], timeout=180)
        if rc_emit != 0:
            return fail(f"emit-c failed for {name}")
        if '#include "runtime/c/zenith_rt.h"' not in out_emit:
            return fail(f"emit-c output missing runtime include for {name}")

        c_path = TMP_DIR / f"{name}.c"
        exe_path = TMP_DIR / f"{name}.roundtrip.exe"
        c_path.write_text(out_emit, encoding="utf-8")

        rc_cc, out_cc = compile_emitted_c(cc, c_path, exe_path)
        if rc_cc != 0:
            return fail(f"host C compile failed for {name}:\n{out_cc[:400]}")

        rc_host, out_host = run_cmd([str(exe_path)], timeout=120)
        if rc_host != expected_exit:
            return fail(
                f"host C executable exit mismatch for {name}: expected {expected_exit}, got {rc_host}\n"
                f"{out_host[:300]}"
            )

        rc_zt, out_zt = run_cmd([str(ZT_EXE), "run", project], timeout=180)
        if rc_zt != expected_exit:
            return fail(
                f"zt run exit mismatch for {name}: expected {expected_exit}, got {rc_zt}\n"
                f"{out_zt[:300]}"
            )

    print("roundtrip emit-c checks ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
