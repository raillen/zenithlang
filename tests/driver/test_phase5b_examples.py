import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT_EXE = ROOT / "zt.exe"

EXAMPLES = [
    ("cli-calculator", "42"),
    ("file-processor", "processed"),
    ("todo-app", "2"),
    ("data-parser", "parsed"),
]


def run_zt(*args):
    completed = subprocess.run(
        [str(ZT_EXE), *map(str, args)],
        cwd=str(ROOT),
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    return completed.returncode, (completed.stdout or "") + (completed.stderr or "")


def expect(condition, message, output=""):
    if condition:
        return
    print(f"[FAIL] {message}")
    if output:
        safe = output.encode("ascii", errors="replace").decode("ascii", errors="replace")
        print(safe)
    sys.exit(1)


def main():
    expect(ZT_EXE.exists(), "zt.exe must exist before running Phase 5B example tests")

    for name, expected_line in EXAMPLES:
        project = ROOT / "examples" / name / "zenith.ztproj"
        expect(project.exists(), f"{name} project should exist")

        rc, out = run_zt("check", project, "--ci")
        expect(rc == 0, f"{name} should pass zt check", out)

        rc, out = run_zt("run", project, "--ci")
        expect(rc == 0, f"{name} should run successfully", out)
        expect(expected_line in out.splitlines(), f"{name} should print {expected_line}", out)

    print("[OK] phase 5B examples")


if __name__ == "__main__":
    main()
