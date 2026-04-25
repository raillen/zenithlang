from __future__ import annotations

import ctypes
import hashlib
import json
import os
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT_EXE = ROOT / "zt.exe"
TMP_DIR = ROOT / ".ztc-tmp" / "emitter-memory-profile"
PROJECT_DIR = TMP_DIR / "synthetic_app"
MANIFEST = PROJECT_DIR / "zenith.ztproj"
REPORT_DIR = ROOT / "reports" / "hardening"
REPORT_MD = REPORT_DIR / "emitter-memory-profile.md"
REPORT_JSON = REPORT_DIR / "emitter-memory-profile.json"

HIGH_THRESHOLD = "268435456"
LOW_THRESHOLD = "512"
FUNCTION_COUNT = 7000
class PROCESS_MEMORY_COUNTERS(ctypes.Structure):
    _fields_ = [
        ("cb", ctypes.c_ulong),
        ("PageFaultCount", ctypes.c_ulong),
        ("PeakWorkingSetSize", ctypes.c_size_t),
        ("WorkingSetSize", ctypes.c_size_t),
        ("QuotaPeakPagedPoolUsage", ctypes.c_size_t),
        ("QuotaPagedPoolUsage", ctypes.c_size_t),
        ("QuotaPeakNonPagedPoolUsage", ctypes.c_size_t),
        ("QuotaNonPagedPoolUsage", ctypes.c_size_t),
        ("PagefileUsage", ctypes.c_size_t),
        ("PeakPagefileUsage", ctypes.c_size_t),
    ]


@dataclass(frozen=True)
class ProfileRun:
    mode: str
    spill_threshold_bytes: int
    output_path: Path
    sha256: str
    output_bytes: int
    elapsed_ms: int
    peak_working_set_bytes: int
    stderr_tail: str


kernel32 = None
psapi = None
if os.name == "nt":
    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    psapi = ctypes.WinDLL("psapi", use_last_error=True)
    psapi.GetProcessMemoryInfo.argtypes = (
        ctypes.c_void_p,
        ctypes.POINTER(PROCESS_MEMORY_COUNTERS),
        ctypes.c_ulong,
    )
    psapi.GetProcessMemoryInfo.restype = ctypes.c_int


def fail(message: str) -> int:
    print(message, file=sys.stderr)
    return 1


def safe_symbol(index: int) -> str:
    alphabet = "abcdefghjkmnpqrstuvwxyz"
    value = index
    out = []
    while True:
        out.append(alphabet[value % len(alphabet)])
        value = (value // len(alphabet)) - 1
        if value < 0:
            break
    return "fn_" + "".join(reversed(out))


def write_synthetic_project() -> None:
    shutil.rmtree(PROJECT_DIR, ignore_errors=True)
    (PROJECT_DIR / "src" / "app").mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(
        "\n".join(
            [
                "[project]",
                'name = "emitter-memory-profile"',
                'kind = "app"',
                'version = "0.1.0"',
                "",
                "[source]",
                'root = "src"',
                "",
                "[app]",
                'entry = "app.main"',
                "",
                "[build]",
                'target = "native"',
                'output = "build"',
                'profile = "debug"',
                "",
            ]
        ),
        encoding="utf-8",
    )

    parts = ["namespace app.main", ""]
    for i in range(FUNCTION_COUNT):
        helper_name = safe_symbol(i)
        parts.extend(
            [
                f"func {helper_name}(seed: int) -> int",
                f"    const a: int = seed + {i}",
                f"    const b: int = a + {(i % 11) + 1}",
                f"    const c: int = b + {(i % 17) + 3}",
                "    return c",
                "end",
                "",
            ]
        )

    parts.extend(
        [
            "func main() -> int",
            "    var total: int = 0",
            f"    total = total + {safe_symbol(0)}({FUNCTION_COUNT})",
            f"    total = total + {safe_symbol(FUNCTION_COUNT // 2)}({FUNCTION_COUNT // 3})",
            f"    total = total + {safe_symbol(FUNCTION_COUNT - 1)}(1)",
            "    return total",
            "end",
            "",
        ]
    )

    (PROJECT_DIR / "src" / "app" / "main.zt").write_text("\n".join(parts), encoding="utf-8")


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as handle:
        while True:
            chunk = handle.read(1024 * 1024)
            if not chunk:
                break
            h.update(chunk)
    return h.hexdigest()


def query_peak_working_set(process_handle: int) -> int:
    if os.name != "nt":
        raise RuntimeError("peak working set probe only implemented for Windows")
    counters = PROCESS_MEMORY_COUNTERS()
    counters.cb = ctypes.sizeof(PROCESS_MEMORY_COUNTERS)
    ok = psapi.GetProcessMemoryInfo(ctypes.c_void_p(process_handle), ctypes.byref(counters), counters.cb)
    if not ok:
        raise OSError(ctypes.get_last_error(), "GetProcessMemoryInfo failed for child process handle")
    return int(counters.PeakWorkingSetSize)


def profile_emit(mode: str, spill_threshold: str) -> ProfileRun:
    output_path = TMP_DIR / f"emit-{mode}.c"
    error_path = TMP_DIR / f"emit-{mode}.stderr.txt"
    output_path.parent.mkdir(parents=True, exist_ok=True)

    env = dict(os.environ)
    env["ZT_EMITTER_SPILL_THRESHOLD_BYTES"] = spill_threshold

    t0 = time.monotonic()
    with output_path.open("wb") as stdout_handle, error_path.open("wb") as stderr_handle:
        process = subprocess.Popen(
            [str(ZT_EXE), "emit-c", str(MANIFEST)],
            cwd=str(ROOT),
            stdout=stdout_handle,
            stderr=stderr_handle,
            env=env,
        )
        rc = process.wait(timeout=600)
        elapsed_ms = int((time.monotonic() - t0) * 1000)
        peak_bytes = query_peak_working_set(process._handle)

    stderr_text = error_path.read_text(encoding="utf-8", errors="replace")
    if rc != 0:
        raise RuntimeError(
            f"emit-c failed for mode={mode}, rc={rc}\n{stderr_text[:1200]}"
        )

    return ProfileRun(
        mode=mode,
        spill_threshold_bytes=int(spill_threshold),
        output_path=output_path,
        sha256=sha256_file(output_path),
        output_bytes=output_path.stat().st_size,
        elapsed_ms=elapsed_ms,
        peak_working_set_bytes=peak_bytes,
        stderr_tail=stderr_text[-400:],
    )


def mib(value: int) -> float:
    return value / (1024.0 * 1024.0)


def write_reports(runs: list[ProfileRun], skipped_reason: str | None = None) -> None:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)

    payload = {
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
        "kind": "emitter-memory-profile",
        "function_count": FUNCTION_COUNT,
        "project": str(MANIFEST.relative_to(ROOT)).replace("\\", "/") if MANIFEST.exists() else None,
        "skipped_reason": skipped_reason,
        "runs": [
            {
                "mode": run.mode,
                "spill_threshold_bytes": run.spill_threshold_bytes,
                "output": str(run.output_path.relative_to(ROOT)).replace("\\", "/"),
                "sha256": run.sha256,
                "output_bytes": run.output_bytes,
                "elapsed_ms": run.elapsed_ms,
                "peak_working_set_bytes": run.peak_working_set_bytes,
                "stderr_tail": run.stderr_tail,
            }
            for run in runs
        ],
    }

    if len(runs) == 2:
        buffered, streamed = runs
        payload["delta"] = {
            "peak_working_set_bytes": streamed.peak_working_set_bytes - buffered.peak_working_set_bytes,
            "peak_working_set_mib": round(mib(streamed.peak_working_set_bytes - buffered.peak_working_set_bytes), 2),
            "elapsed_ms": streamed.elapsed_ms - buffered.elapsed_ms,
        }

    REPORT_JSON.write_text(json.dumps(payload, indent=2, ensure_ascii=True) + "\n", encoding="utf-8")

    lines = [
        "# Emitter Memory Profile",
        "",
        f"Gerado em: `{payload['generated_at_utc']}`",
        "",
        "Escopo:",
        "- mede o pico de working set do processo `zt.exe` no caminho `emit-c`;",
        "- compara modo buffered (spill alto) com modo streamed/spill (spill baixo);",
        "- valida equivalencia por `sha256` do C gerado.",
        "",
    ]

    if skipped_reason is not None:
        lines.extend([
            "Status: `SKIP`",
            "",
            f"Motivo: {skipped_reason}",
            "",
        ])
    else:
        lines.extend([
            f"Projeto sintetico: `{payload['project']}`",
            f"Funcoes geradas: `{FUNCTION_COUNT}`",
            "",
            "| Modo | Spill threshold | Saida | Tempo | Pico de memoria | SHA256 |",
            "|---|---:|---:|---:|---:|---|",
        ])
        for run in runs:
            lines.append(
                f"| `{run.mode}` | `{run.spill_threshold_bytes}` | `{run.output_bytes}` bytes | "
                f"`{run.elapsed_ms}` ms | `{mib(run.peak_working_set_bytes):.2f}` MiB | `{run.sha256[:16]}` |"
            )
        if len(runs) == 2:
            delta = payload["delta"]
            lines.extend([
                "",
                f"Delta de pico (`streamed - buffered`): `{delta['peak_working_set_mib']}` MiB",
                f"Delta de tempo (`streamed - buffered`): `{delta['elapsed_ms']}` ms",
                "",
                "Leitura:",
                "- este teste nao falha por um threshold arbitrario de memoria;",
                "- ele falha se o emit-c quebrar, se a medicao quebrar ou se a saida mudar;",
                "- o snapshot serve para acompanhar tendencia real no `nightly/stress`.",
                "",
            ])

    REPORT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    if not ZT_EXE.exists():
        return fail(f"missing compiler binary: {ZT_EXE}")

    if os.name != "nt":
        reason = "medicao de pico do processo ainda esta implementada apenas para Windows"
        write_reports([], skipped_reason=reason)
        print(f"emitter memory profile skipped: {reason}")
        return 0

    TMP_DIR.mkdir(parents=True, exist_ok=True)
    write_synthetic_project()

    buffered = profile_emit("buffered", HIGH_THRESHOLD)
    streamed = profile_emit("streamed", LOW_THRESHOLD)

    if buffered.sha256 != streamed.sha256:
        return fail(
            "emitter memory profile mismatch: buffered/streamed outputs diverged\n"
            f"buffered={buffered.sha256}\nstreamed={streamed.sha256}"
        )

    write_reports([buffered, streamed])
    print(f"emitter memory profile ok -> {REPORT_MD}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
