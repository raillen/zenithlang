#!/usr/bin/env python3
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ZT_EXE = ROOT / ("zt.exe" if sys.platform.startswith("win") else "zt")


TOOLS = [
    ("link-checker", ROOT / "tools" / "selfhost" / "link-checker" / "zenith.ztproj", "selfhost-link-checker ok"),
    ("manifest-validator", ROOT / "tools" / "selfhost" / "manifest-validator" / "zenith.ztproj", "selfhost-manifest-validator ok"),
    ("fixture-index", ROOT / "tools" / "selfhost" / "fixture-index" / "zenith.ztproj", "selfhost-fixture-index ok"),
]


def fail(message):
    print(f"FAIL: {message}", file=sys.stderr)
    return 1


def run_tool(name, project, expected):
    result = subprocess.run(
        [str(ZT_EXE), "run", str(project)],
        cwd=str(ROOT),
        text=True,
        encoding="utf-8",
        errors="replace",
        capture_output=True,
        timeout=120,
        check=False,
    )
    output = (result.stdout or "") + (result.stderr or "")
    if result.returncode != 0:
        raise AssertionError(f"{name} failed with exit {result.returncode}\n{output}")
    if expected not in output:
        raise AssertionError(f"{name} did not print expected marker: {expected}\n{output}")


def main():
    if not ZT_EXE.exists():
        return fail("zt.exe must exist before running Phase 6 selfhost tools")

    try:
        for name, project, expected in TOOLS:
            run_tool(name, project, expected)

        generated = ROOT / ".ztc-tmp" / "selfhost" / "fixture-index.txt"
        if not generated.exists():
            raise AssertionError("fixture index tool did not write .ztc-tmp/selfhost/fixture-index.txt")
        if "behavior fixtures indexed" not in generated.read_text(encoding="utf-8"):
            raise AssertionError("fixture index output is missing expected content")
    except Exception as exc:
        return fail(str(exc))

    print("phase6 selfhost tools ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
