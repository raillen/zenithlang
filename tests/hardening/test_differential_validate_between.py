"""
Differential check against a standalone C reference implementation.
"""

from __future__ import annotations

import os
import re
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT_EXE = ROOT / "zt.exe"
TMP_DIR = ROOT / ".ztc-tmp" / "hardening-differential"

HITS_RE = re.compile(r"hits=0x([0-9a-fA-F]+)")


def run_cmd(args: list[str], timeout: int = 240) -> tuple[int, str]:
    completed = subprocess.run(
        args,
        cwd=str(ROOT),
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        timeout=timeout,
    )
    return completed.returncode, (completed.stdout or "") + (completed.stderr or "")


def fail(message: str) -> int:
    print(message, file=sys.stderr)
    return 1


def parse_hits(output: str) -> list[int]:
    values = []
    for match in HITS_RE.finditer(output):
        values.append(int(match.group(1), 16))
    return values


def main() -> int:
    if not ZT_EXE.exists():
        return fail(f"missing compiler binary: {ZT_EXE}")

    cc = os.environ.get("CC", "gcc")
    if shutil.which(cc) is None:
        return fail(f"C compiler not found: {cc}")

    rc_zt, out_zt = run_cmd([str(ZT_EXE), "run", "tests/perf/std_validate_vs_c"], timeout=300)
    if rc_zt != 0:
        return fail(f"zenith differential scenario failed:\n{out_zt[:400]}")

    hits = parse_hits(out_zt)
    if len(hits) < 3:
        return fail("could not parse hit counters from Zenith output")
    zenith_hits = hits[-3:]

    ref_c = r"""
    #include <stdio.h>

    static long long mixed_value(long long seed) {
        return ((seed * 73) % 301) - 100;
    }

    int main(void) {
        const long long iterations = 5000000;
        const long long min_value = -20;
        const long long max_value = 120;
        long long hits = 0;
        for (long long i = 0; i < iterations; i++) {
            long long value = mixed_value(i);
            if (value >= min_value && value <= max_value) {
                hits++;
            }
        }
        printf("%lld\n", hits);
        return 0;
    }
    """

    TMP_DIR.mkdir(parents=True, exist_ok=True)
    c_path = TMP_DIR / "validate_between_ref.c"
    exe_path = TMP_DIR / "validate_between_ref.exe"
    c_path.write_text(ref_c, encoding="utf-8")

    rc_cc, out_cc = run_cmd([cc, "-O2", "-std=c11", "-o", str(exe_path), str(c_path)], timeout=120)
    if rc_cc != 0:
        return fail(f"failed to compile C reference:\n{out_cc[:400]}")

    rc_ref, out_ref = run_cmd([str(exe_path)], timeout=120)
    if rc_ref != 0:
        return fail(f"C reference executable failed:\n{out_ref[:200]}")

    try:
        ref_hits = int((out_ref or "").strip().splitlines()[-1])
    except Exception:
        return fail(f"could not parse C reference output: {out_ref[:120]}")

    if not (zenith_hits[0] == zenith_hits[1] == zenith_hits[2] == ref_hits):
        return fail(
            "differential mismatch:\n"
            f"  zenith std={zenith_hits[0]}\n"
            f"  zenith c-helper={zenith_hits[1]}\n"
            f"  zenith inline={zenith_hits[2]}\n"
            f"  c-reference={ref_hits}"
        )

    print("differential validate-between checks ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
