import os
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT = ROOT / ("zt.exe" if os.name == "nt" else "zt")
SOURCE_FILE = ROOT / "tests" / "behavior" / "check_intrinsic_basic" / "src" / "app" / "main.zt"


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


def expect_project_only_error(args, expected):
    rc, out = run_zt(*args, str(SOURCE_FILE))
    if rc == 0:
        raise AssertionError(f"expected non-zero exit for: {' '.join(args)} <file.zt>\n--- output ---\n{out}")
    if expected not in out:
        raise AssertionError(f"expected {expected!r} for: {' '.join(args)} <file.zt>\n--- output ---\n{out}")
    if "project.invalid_input" in out:
        raise AssertionError(f"expected command-specific error, not project.invalid_input\n--- output ---\n{out}")


def main():
    expect_project_only_error(["fmt"], "fmt expects a project path, not a source file")
    expect_project_only_error(["test"], "test expects a project path, not a source file")
    expect_project_only_error(["doc", "check"], "doc-check expects a project path, not a source file")
    expect_project_only_error(["project-info"], "project-info expects a project path, not a source file")
    print("project-only input tests passed")


if __name__ == "__main__":
    main()
