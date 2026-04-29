import os
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT = ROOT / ("zt.exe" if os.name == "nt" else "zt")
PROJECT = ROOT / "tests" / "behavior" / "check_intrinsic_basic"


def run_zt(*args):
    completed = subprocess.run(
        [str(ZT), *args],
        cwd=str(ROOT),
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    return completed.returncode, (completed.stdout or "") + (completed.stderr or "")


def expect_error(args, expected):
    rc, out = run_zt(*args)
    if rc == 0:
        raise AssertionError(f"expected non-zero exit for: {' '.join(args)}\n--- output ---\n{out}")
    if expected not in out:
        raise AssertionError(f"expected {expected!r} for: {' '.join(args)}\n--- output ---\n{out}")


def main():
    expect_error(["--lang"], "option --lang requires a language")
    expect_error(["check", str(PROJECT), "--lang", "--all"], "option --lang requires a language")
    expect_error(["build", str(PROJECT), "-o"], "option -o requires an output path")
    expect_error(["build", str(PROJECT), "--profile"], "option --profile requires a profile")
    expect_error(["check", str(PROJECT), "--focus"], "option --focus requires a path")
    expect_error(["check", str(PROJECT), "--since"], "option --since requires a git ref")
    expect_error(["test", str(PROJECT), "--filter", "--ci"], "option --filter requires a test name")
    print("cli option value tests passed")


if __name__ == "__main__":
    main()
