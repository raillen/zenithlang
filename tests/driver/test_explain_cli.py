import os
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT = ROOT / ("zt.exe" if os.name == "nt" else "zt")


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


def require_contains(text, needle):
    if needle not in text:
        raise AssertionError(f"expected to find {needle!r}\n--- output ---\n{text}")


def test_known_code():
    rc, out = run_zt("explain", "type.mismatch")
    assert rc == 0, out
    require_contains(out, "code: type.mismatch")
    require_contains(out, "meaning:")
    require_contains(out, "invalid:")
    require_contains(out, "fixed:")
    require_contains(out, "next:")
    require_contains(out, "doc:")
    require_contains(out, 'const count: int = "five"')
    require_contains(out, "const count: int = 5")


def test_unknown_code():
    rc, out = run_zt("explain", "does.not_exist")
    assert rc != 0, out
    require_contains(out, "unknown diagnostic code: does.not_exist")
    require_contains(out, "zt explain type.mismatch")
    require_contains(out, "known examples:")


def main():
    test_known_code()
    test_unknown_code()
    print("explain cli tests: 2/2 passed")


if __name__ == "__main__":
    main()
