"""
Runtime sanitizer checks (ASAN/UBSAN when compiler support is available).
"""

from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
TMP_DIR = ROOT / ".ztc-tmp" / "hardening-sanitizers"


def run_cmd(args: list[str], timeout: int = 240, env: dict[str, str] | None = None) -> tuple[int, str]:
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


def compiler_supports(cc: str, flags: list[str]) -> bool:
    TMP_DIR.mkdir(parents=True, exist_ok=True)
    src = TMP_DIR / "probe.c"
    exe = TMP_DIR / "probe.exe"
    src.write_text("int main(void) { return 0; }\n", encoding="utf-8")
    cmd = [cc, "-std=c11", str(src), "-o", str(exe)] + flags
    rc, _out = run_cmd(cmd, timeout=60)
    return rc == 0


def fail(message: str) -> int:
    print(message, file=sys.stderr)
    return 1


def main() -> int:
    cc = os.environ.get("CC", "gcc")
    if shutil.which(cc) is None:
        print(f"SKIP: C compiler not found: {cc}")
        return 0

    candidate_flags = [
        "-fsanitize=address",
        "-fsanitize=undefined",
    ]
    enabled_flags = [flag for flag in candidate_flags if compiler_supports(cc, [flag])]
    if not enabled_flags:
        print("SKIP: sanitizer flags are not supported by this compiler")
        return 0

    cmd = [
        cc,
        "-std=c11",
        "-Wall",
        "-Wextra",
        "-pedantic",
        "-fno-omit-frame-pointer",
        "-g",
        "-I.",
        "runtime/c/zenith_rt.c",
        "tests/runtime/c/test_runtime.c",
        "-o",
        str(TMP_DIR / "test_runtime_sanitized.exe"),
    ] + enabled_flags
    if os.name == "nt":
        cmd.append("-lws2_32")

    rc_build, out_build = run_cmd(cmd, timeout=300)
    if rc_build != 0:
        return fail(f"sanitized runtime build failed:\n{out_build[:400]}")

    run_env = os.environ.copy()
    if "-fsanitize=address" in enabled_flags:
        run_env.setdefault("ASAN_OPTIONS", "detect_leaks=0")
    rc_run, out_run = run_cmd([str(TMP_DIR / "test_runtime_sanitized.exe")], timeout=300, env=run_env)
    if rc_run != 0:
        return fail(f"sanitized runtime execution failed:\n{out_run[:500]}")

    print("runtime sanitizer checks ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
