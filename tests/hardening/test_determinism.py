"""
Determinism checks for compiler outputs and diagnostics.
"""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT_EXE = ROOT / "zt.exe"
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")


def run_cmd(args: list[str], timeout: int = 120) -> tuple[int, str]:
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


def normalize(text: str) -> str:
    clean = ANSI_RE.sub("", text or "")
    clean = clean.replace("\r\n", "\n").replace("\\", "/")
    lines = [line.rstrip() for line in clean.splitlines()]
    return "\n".join(lines).strip()


def fail(message: str) -> int:
    print(message, file=sys.stderr)
    return 1


def main() -> int:
    if not ZT_EXE.exists():
        return fail(f"missing compiler binary: {ZT_EXE}")

    # Emit-C output must be stable for the same input.
    rc1, out1 = run_cmd([str(ZT_EXE), "emit-c", "tests/behavior/simple_app"])
    rc2, out2 = run_cmd([str(ZT_EXE), "emit-c", "tests/behavior/simple_app"])
    if rc1 != 0 or rc2 != 0:
        return fail("emit-c failed while checking determinism")
    if normalize(out1) != normalize(out2):
        return fail("non-deterministic emit-c output for tests/behavior/simple_app")

    # Diagnostics text must be stable for the same invalid input.
    rc3, out3 = run_cmd([str(ZT_EXE), "check", "tests/behavior/error_type_mismatch"])
    rc4, out4 = run_cmd([str(ZT_EXE), "check", "tests/behavior/error_type_mismatch"])
    if rc3 == 0 or rc4 == 0:
        return fail("error_type_mismatch unexpectedly passed")
    if normalize(out3) != normalize(out4):
        return fail("non-deterministic diagnostics for error_type_mismatch")

    print("determinism checks ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
