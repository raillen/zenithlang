import os
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT = ROOT / ("zt.exe" if os.name == "nt" else "zt")
ZPM = ROOT / ("zpm.exe" if os.name == "nt" else "zpm")
EXPECTED = "Zenith 0.4.1-alpha.1"


def run_tool(tool, *args):
    completed = subprocess.run(
        [str(tool), *args],
        cwd=str(ROOT),
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    return completed.returncode, (completed.stdout or "") + (completed.stderr or "")


def require(condition, message, output=""):
    if condition:
        return
    raise AssertionError(f"{message}\n--- output ---\n{output}")


def main():
    for flag in ("--version", "-v", "version"):
        rc, out = run_tool(ZT, flag)
        require(rc == 0, f"zt {flag} should succeed", out)
        require(out.strip() == EXPECTED, f"zt {flag} should print current version", out)

    for flag in ("--version", "version"):
        rc, out = run_tool(ZPM, flag)
        require(rc == 0, f"zpm {flag} should succeed", out)
        require(out.strip() == EXPECTED, f"zpm {flag} should print current version", out)

    rc, out = run_tool(ZPM, "-v", "help")
    require(rc == 0, "zpm -v help should still mean verbose help, not version", out)
    require("zpm" in out.lower(), "zpm -v help should print help", out)

    print("cli version tests passed")


if __name__ == "__main__":
    main()
