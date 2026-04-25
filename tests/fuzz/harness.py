"""
R2.M2 - Fuzz harness core.

Responsabilidades:
- Montar um projeto temporario minimo (zenith.ztproj + src/app/main.zt)
- Executar `zt.exe check` sobre ele com timeout
- Classificar a saida em: ok | diag (erro de usuario) | crash | timeout
- Persistir seeds que quebram em tests/fuzz/seeds/crashes/

Nao e um fuzz agressivo de C API - opera em cima do binario `zt.exe`
o que e suficiente para detectar ICEs, crashes nativos e travamentos.
"""

from __future__ import annotations

import hashlib
import os
import subprocess
import sys
import tempfile
import time
from pathlib import Path
from typing import NamedTuple

ROOT = Path(__file__).resolve().parents[2]
ZT_EXE = ROOT / "zt.exe"
CRASHES_DIR = ROOT / "tests" / "fuzz" / "seeds" / "crashes"

PROJECT_TEMPLATE = """\
[project]
name = "fuzz-sample"
kind = "app"
version = "0.0.0"
[source]
root = "src"
[app]
entry = "app.main"
[build]
target = "native"
output = "build"
profile = "debug"
"""

# ---------------------------------------------------------------------------
# Crash detection patterns
# ---------------------------------------------------------------------------
CRASH_SUBSTRINGS = (
    "internal compiler error",
    "assertion failed",
    "assert failed",
    "segmentation fault",
    "segfault",
    "access violation",
    "stack overflow",
    " ICE ",
    "abort()",
    "panic:",
    "double free",
    "heap corruption",
)


class RunResult(NamedTuple):
    kind: str       # "ok" | "diag" | "crash" | "timeout"
    rc: int
    elapsed_ms: int
    output: str
    reason: str     # short human label


def _classify(rc: int, output: str, timed_out: bool) -> tuple[str, str]:
    if timed_out:
        return "timeout", "timeout"
    low = output.lower()
    for s in CRASH_SUBSTRINGS:
        if s.lower() in low:
            return "crash", f"pattern:{s.strip()}"
    # zt.exe: 0 ok; 1 erro de usuario; anything else is suspicious
    if rc == 0:
        return "ok", "clean"
    if rc == 1:
        return "diag", "user_error"
    if rc < 0 or rc > 2:
        return "crash", f"rc={rc}"
    return "diag", f"rc={rc}"


def run_check(source: str, timeout_sec: float = 8.0) -> RunResult:
    """Run `zt.exe check` on a one-file project containing `source`."""
    if not ZT_EXE.exists():
        return RunResult("crash", -1, 0, f"zt.exe not found at {ZT_EXE}", "no_compiler")

    with tempfile.TemporaryDirectory(prefix="zt_fuzz_") as td:
        tdp = Path(td)
        (tdp / "src" / "app").mkdir(parents=True, exist_ok=True)
        (tdp / "zenith.ztproj").write_text(PROJECT_TEMPLATE, encoding="utf-8")
        # ensure file always has a namespace header so parser is exercised,
        # even when the fuzzed body is malformed
        main_zt = "namespace app.main\n" + source + "\n"
        (tdp / "src" / "app" / "main.zt").write_text(main_zt, encoding="utf-8")

        t0 = time.time()
        timed_out = False
        try:
            proc = subprocess.run(
                [str(ZT_EXE), "check", str(tdp / "zenith.ztproj"), "--ci"],
                capture_output=True,
                text=True,
                timeout=timeout_sec,
                cwd=str(tdp),
            )
            rc = proc.returncode
            output = (proc.stdout or "") + (proc.stderr or "")
        except subprocess.TimeoutExpired as e:
            rc = -9
            output = (e.stdout or "") + (e.stderr or "") if hasattr(e, "stdout") else ""
            if isinstance(output, bytes):
                output = output.decode("utf-8", errors="replace")
            timed_out = True
        elapsed_ms = int((time.time() - t0) * 1000)

        kind, reason = _classify(rc, output, timed_out)
        return RunResult(kind, rc, elapsed_ms, output[:4000], reason)


def persist_crash(source: str, label: str) -> Path:
    """Save a crashing input to tests/fuzz/seeds/crashes/."""
    CRASHES_DIR.mkdir(parents=True, exist_ok=True)
    h = hashlib.sha1(source.encode("utf-8", errors="replace")).hexdigest()[:12]
    path = CRASHES_DIR / f"{label}__{h}.zt"
    path.write_text(source, encoding="utf-8")
    return path
