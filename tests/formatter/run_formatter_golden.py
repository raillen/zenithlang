import os
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT = ROOT / ("zt.exe" if os.name == "nt" else "zt")
TMP = ROOT / ".ztc-tmp" / "tests" / "formatter-golden"


CASES = [
    {
        "id": "case_all",
        "input_project": ROOT / "tests" / "formatter" / "cases" / "case_all" / "input",
        "expected_file": ROOT / "tests" / "formatter" / "cases" / "case_all" / "expected" / "src" / "app" / "main.zt",
        "target_file": Path("src") / "app" / "main.zt",
    }
]


def run_cmd(args, cwd, timeout=60):
    completed = subprocess.run(
        args,
        cwd=str(cwd),
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        timeout=timeout,
        check=False,
    )
    return completed.returncode, (completed.stdout or "") + (completed.stderr or "")


def run_case(case):
    case_tmp = TMP / case["id"]
    shutil.rmtree(case_tmp, ignore_errors=True)
    case_tmp.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(case["input_project"], case_tmp)

    rc, out = run_cmd([str(ZT), "fmt", str(case_tmp)], ROOT, timeout=90)
    if rc != 0:
        return False, f"fmt failed ({case['id']}):\n{out}"

    actual = case_tmp / case["target_file"]
    expected = case["expected_file"]
    if not actual.exists():
        return False, f"formatted file missing: {actual}"
    if not expected.exists():
        return False, f"golden expected missing: {expected}"

    actual_text = actual.read_text(encoding="utf-8")
    expected_text = expected.read_text(encoding="utf-8")
    if actual_text.replace("\r\n", "\n") == expected_text.replace("\r\n", "\n"):
        return True, ""

    return (
        False,
        f"golden mismatch ({case['id']})\n--- expected ---\n{expected_text}\n--- actual ---\n{actual_text}",
    )


def main():
    if not ZT.exists():
        print(f"missing driver: {ZT}")
        return 1

    failures = []
    for case in CASES:
        ok, detail = run_case(case)
        print(f"[{'OK' if ok else 'FAIL'}] formatter/{case['id']}")
        if not ok:
            failures.append(detail)

    if failures:
        print("\n".join(failures))
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
