"""
Formatter idempotence gate.

Verifies the property fmt(fmt(x)) == fmt(x) for a canonical subset of
cases. The formatter must converge to a stable fixed point after one
pass, so a second pass is expected to be a no-op on tree content.

Two case buckets exist:

- IDEMPOTENT_CASES: must converge. A divergence here fails the gate.
- XFAIL_CASES: known to not converge in the current alpha. They run but
  their result does not block the gate. They are tracked under PLI-10
  in reports/pending-language-issues-current.md.

New formatter features should add coverage to IDEMPOTENT_CASES. When an
XFAIL case starts converging, it should be promoted to IDEMPOTENT_CASES
and removed from XFAIL_CASES.
"""

import os
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT = ROOT / ("zt.exe" if os.name == "nt" else "zt")
TMP = ROOT / ".ztc-tmp" / "tests" / "formatter-idempotence"


CASES_ROOT = ROOT / "tests" / "formatter" / "cases"


def _case(case_id, target_file):
    return {
        "id": case_id,
        "input_project": CASES_ROOT / case_id / "input",
        "target_file": Path(target_file),
    }


# Cases that must remain idempotent. Breaking one here is a hard fail.
IDEMPOTENT_CASES = [
    _case("case_structs", "src/app/main.zt"),
    _case("case_triple_quoted", "src/app/main.zt"),
    _case("case_trailing_commas", "src/app/main.zt"),
    _case("case_manifest", "zenith.ztproj"),
    _case("case_all", "src/app/main.zt"),
    _case("case_imports", "src/app/main.zt"),
    _case("case_match", "src/app/main.zt"),
    _case("case_generics", "src/app/main.zt"),
    _case("case_comments", "src/app/main.zt"),
]


# Cases that do not yet converge. Tracked under PLI-10 until fixed.
XFAIL_CASES = []


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


def normalize_newlines(text):
    return text.replace("\r\n", "\n")


def run_case(case):
    case_tmp = TMP / case["id"]
    shutil.rmtree(case_tmp, ignore_errors=True)
    case_tmp.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(case["input_project"], case_tmp)

    target = case_tmp / case["target_file"]
    if not target.exists():
        return False, f"missing target before first fmt pass: {target}"

    rc1, out1 = run_cmd([str(ZT), "fmt", str(case_tmp)], ROOT, timeout=90)
    if rc1 != 0:
        return False, f"first fmt pass failed ({case['id']}):\n{out1}"
    if not target.exists():
        return False, f"target file missing after first fmt pass: {target}"
    first_pass = normalize_newlines(target.read_text(encoding="utf-8"))

    rc2, out2 = run_cmd([str(ZT), "fmt", str(case_tmp)], ROOT, timeout=90)
    if rc2 != 0:
        return False, (
            f"second fmt pass failed ({case['id']}):\n{out2}\n"
            f"--- first pass output ---\n{first_pass}"
        )
    second_pass = normalize_newlines(target.read_text(encoding="utf-8"))

    if first_pass == second_pass:
        return True, ""

    return (
        False,
        f"idempotence broken ({case['id']})\n"
        f"--- first pass ---\n{first_pass}\n"
        f"--- second pass ---\n{second_pass}",
    )


def main():
    if not ZT.exists():
        print(f"missing driver: {ZT}")
        return 1

    failures = []

    for case in IDEMPOTENT_CASES:
        ok, detail = run_case(case)
        print(f"[{'OK  ' if ok else 'FAIL'}] formatter_idempotence/{case['id']}")
        if not ok:
            failures.append(detail)

    for case in XFAIL_CASES:
        ok, detail = run_case(case)
        label = "XOK " if ok else "XFAIL"
        print(f"[{label}] formatter_idempotence/{case['id']} (expected to not converge yet)")
        if ok:
            failures.append(
                f"XFAIL case {case['id']} now converges; promote it to IDEMPOTENT_CASES."
            )

    if failures:
        print("\n".join(failures))
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
