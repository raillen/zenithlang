import os
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT = ROOT / ("zt.exe" if os.name == "nt" else "zt")
SINGLE_FILE = ROOT / "tests" / "behavior" / "check_intrinsic_basic" / "src" / "app" / "main.zt"


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


def main():
    rc, out = run_zt("run", str(SINGLE_FILE))
    if rc != 0:
        raise AssertionError(f"zt run <file.zt> should exit 0\n--- output ---\n{out}")
    if "project.invalid_input" in out:
        raise AssertionError(f"zt run <file.zt> should not print project.invalid_input\n--- output ---\n{out}")
    noisy_fragments = ["compiling:", "built:", "running:", "exit code:", "warning[native.", ".ztc-tmp"]
    for fragment in noisy_fragments:
        if fragment in out:
            raise AssertionError(f"zt run <file.zt> should keep normal output clean: {fragment}\n--- output ---\n{out}")
    print("single-file run test passed")


if __name__ == "__main__":
    main()
