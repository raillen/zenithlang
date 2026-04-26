import argparse
import json
import math
import os
import platform
import shutil
import statistics
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
REPORT_DIR = ROOT / "reports" / "perf"
BASELINE_DIR = ROOT / "tests" / "perf" / "baselines"

SUITES = {
    "quick": {"warmup": 1, "iters": 3, "retry": 1, "cv": 0.35},
    "nightly": {"warmup": 2, "iters": 7, "retry": 2, "cv": 0.25},
}


def bench_specs():
    return [
        # micro frontend
        {"id": "micro_frontend_small_check", "scenario": "micro_frontend", "cat": "frontend", "kind": "cmd", "cmd": "check", "project": ROOT / "tests" / "behavior" / "simple_app", "suites": ["quick", "nightly"]},
        {"id": "micro_frontend_medium_check", "scenario": "micro_frontend", "cat": "frontend", "kind": "cmd", "cmd": "check", "project": ROOT / "tests" / "behavior" / "multifile_import_alias", "suites": ["nightly"]},
        {"id": "micro_frontend_large_check", "scenario": "micro_frontend", "cat": "frontend", "kind": "cmd", "cmd": "check", "project": ROOT / "tests" / "perf" / "m37_result_generic", "suites": ["nightly"]},
        # micro lowering/backend
        {"id": "micro_lowering_small_emit_c", "scenario": "micro_lowering_backend", "cat": "lowering_backend", "kind": "cmd", "cmd": "emit-c", "project": ROOT / "tests" / "behavior" / "simple_app", "suites": ["quick", "nightly"]},
        {"id": "micro_lowering_large_emit_c", "scenario": "micro_lowering_backend", "cat": "lowering_backend", "kind": "cmd", "cmd": "emit-c", "project": ROOT / "tests" / "perf" / "m37_result_generic", "suites": ["nightly"]},
        # micro runtime + stdlib
        {"id": "micro_runtime_core", "scenario": "micro_runtime", "cat": "runtime", "kind": "bin", "project": ROOT / "tests" / "perf" / "m36_runtime_core", "expect": "m36-runtime-core-ok", "suites": ["quick", "nightly"]},
        {"id": "micro_stdlib_core", "scenario": "micro_stdlib", "cat": "stdlib", "kind": "bin", "project": ROOT / "tests" / "perf" / "m36_stdlib_core", "expect": "m36-stdlib-core-ok", "suites": ["quick", "nightly"]},
        {"id": "micro_lambda_hof_run", "scenario": "micro_lambda_hof", "cat": "stdlib", "kind": "cmd", "cmd": "run", "project": ROOT / "tests" / "behavior" / "lambda_hof_basic", "suites": ["quick", "nightly"]},
        # macro small
        {"id": "macro_small_check", "scenario": "macro_small", "cat": "macro", "kind": "cmd", "cmd": "check", "project": ROOT / "tests" / "behavior" / "simple_app", "suites": ["quick", "nightly"]},
        {"id": "macro_small_build_cold", "scenario": "macro_small", "cat": "macro", "kind": "cmd", "cmd": "build", "project": ROOT / "tests" / "behavior" / "simple_app", "cold": True, "suites": ["quick", "nightly"]},
        {"id": "macro_small_build_warm", "scenario": "macro_small", "cat": "macro", "kind": "cmd", "cmd": "build", "project": ROOT / "tests" / "behavior" / "simple_app", "suites": ["quick", "nightly"]},
        {"id": "macro_small_run", "scenario": "macro_small", "cat": "macro", "kind": "cmd", "cmd": "run", "project": ROOT / "tests" / "behavior" / "std_io_basic", "expect": "Hello from std.io!", "suites": ["quick", "nightly"]},
        {"id": "macro_small_test", "scenario": "macro_small", "cat": "macro", "kind": "cmd", "cmd": "test", "project": ROOT / "tests" / "behavior" / "std_test_basic", "suites": ["quick", "nightly"]},
        # macro medium
        {"id": "macro_medium_check", "scenario": "macro_medium", "cat": "macro", "kind": "cmd", "cmd": "check", "project": ROOT / "tests" / "perf" / "std_validate_vs_c", "suites": ["quick", "nightly"]},
        {"id": "macro_medium_build_cold", "scenario": "macro_medium", "cat": "macro", "kind": "cmd", "cmd": "build", "project": ROOT / "tests" / "perf" / "std_validate_vs_c", "cold": True, "timeout": 900, "suites": ["nightly"]},
        {"id": "macro_medium_build_warm", "scenario": "macro_medium", "cat": "macro", "kind": "cmd", "cmd": "build", "project": ROOT / "tests" / "perf" / "std_validate_vs_c", "timeout": 900, "suites": ["nightly"]},
        {"id": "macro_medium_run", "scenario": "macro_medium", "cat": "macro", "kind": "cmd", "cmd": "run", "project": ROOT / "tests" / "perf" / "std_validate_vs_c", "expect": "benchmark std.validate.between vs c helper", "iters": {"quick": 2, "nightly": 4}, "suites": ["nightly"]},
        {"id": "macro_medium_test", "scenario": "macro_medium", "cat": "macro", "kind": "cmd", "cmd": "test", "project": ROOT / "tests" / "behavior" / "std_test_attr_pass_skip", "suites": ["nightly"]},
        # macro large
        {"id": "macro_large_check", "scenario": "macro_large", "cat": "macro", "kind": "cmd", "cmd": "check", "project": ROOT / "tests" / "perf" / "m37_result_generic", "iters": {"nightly": 11}, "baseline_cmp": {"lat_median_ms": {"warn_pct": 16.0, "fail_pct": 32.0}, "lat_p95_ms": {"warn_pct": 20.0, "fail_pct": 40.0}, "throughput_ops_per_sec": {"warn_pct": 16.0, "fail_pct": 32.0}}, "suites": ["nightly"]},
        {"id": "macro_large_build_cold", "scenario": "macro_large", "cat": "macro", "kind": "cmd", "cmd": "build", "project": ROOT / "tests" / "perf" / "m37_result_generic", "cold": True, "timeout": 900, "iters": {"quick": 2, "nightly": 5}, "suites": ["nightly"]},
        {"id": "macro_large_build_warm", "scenario": "macro_large", "cat": "macro", "kind": "cmd", "cmd": "build", "project": ROOT / "tests" / "perf" / "m37_result_generic", "timeout": 900, "iters": {"quick": 2, "nightly": 5}, "suites": ["nightly"]},
        {"id": "macro_large_run", "scenario": "macro_large", "cat": "macro", "kind": "cmd", "cmd": "run", "project": ROOT / "tests" / "perf" / "m37_result_generic", "expect": "m37-result-generic-ok", "iters": {"quick": 2, "nightly": 4}, "suites": ["nightly"]},
        {"id": "macro_large_test", "scenario": "macro_large", "cat": "macro", "kind": "cmd", "cmd": "test", "project": ROOT / "tests" / "perf" / "m36_test_large", "iters": {"quick": 2, "nightly": 4}, "budget": {"warn": {"lat_median_ms": 45000, "lat_p95_ms": 60000, "peak_ws": 520_000_000, "binary_size": 22_000_000}, "fail": {"lat_median_ms": 70000, "lat_p95_ms": 90000, "peak_ws": 800_000_000, "binary_size": 44_000_000}}, "suites": ["nightly"]},
        # legacy gate alias
        {"id": "m37_result_generic", "scenario": "m37_result_generic", "cat": "macro", "kind": "bin", "project": ROOT / "tests" / "perf" / "m37_result_generic", "expect": "m37-result-generic-ok", "iters": {"quick": 2, "nightly": 11}, "baseline_cmp": {"lat_median_ms": {"warn_pct": 16.0, "fail_pct": 32.0}, "lat_p95_ms": {"warn_pct": 20.0, "fail_pct": 40.0}, "throughput_ops_per_sec": {"warn_pct": 16.0, "fail_pct": 32.0}, "startup_ms": {"warn_pct": 16.0, "fail_pct": 32.0}}, "suites": ["nightly"]},
    ]


def default_budget(cat):
    return {
        "frontend": {"warn": {"lat_median_ms": 3000, "lat_p95_ms": 5000, "peak_ws": 420_000_000}, "fail": {"lat_median_ms": 7000, "lat_p95_ms": 11000, "peak_ws": 700_000_000}},
        "lowering_backend": {"warn": {"lat_median_ms": 3500, "lat_p95_ms": 6000, "peak_ws": 460_000_000}, "fail": {"lat_median_ms": 9000, "lat_p95_ms": 14000, "peak_ws": 760_000_000}},
        "runtime": {"warn": {"lat_median_ms": 2200, "lat_p95_ms": 3500, "startup_ms": 2200, "peak_ws": 260_000_000}, "fail": {"lat_median_ms": 5000, "lat_p95_ms": 8000, "startup_ms": 5000, "peak_ws": 520_000_000}},
        "stdlib": {"warn": {"lat_median_ms": 2400, "lat_p95_ms": 3800, "peak_ws": 280_000_000}, "fail": {"lat_median_ms": 5600, "lat_p95_ms": 8500, "peak_ws": 560_000_000}},
        "macro": {"warn": {"lat_median_ms": 4200, "lat_p95_ms": 7000, "peak_ws": 520_000_000, "binary_size": 22_000_000}, "fail": {"lat_median_ms": 9800, "lat_p95_ms": 16000, "peak_ws": 800_000_000, "binary_size": 44_000_000}},
    }[cat]


def host_meta():
    return {
        "host": platform.node(),
        "platform": platform.platform(),
        "system": platform.system().lower(),
        "release": platform.release(),
        "machine": platform.machine(),
        "processor": platform.processor(),
        "python_version": platform.python_version(),
        "cpu_count": os.cpu_count() or 1,
    }


def now_iso():
    return datetime.now(timezone.utc).isoformat()


def pkey():
    m = host_meta()
    return f"{m['system']}-{(m['machine'] or 'unknown').replace(' ', '_')}"


def run_cmd(args, cwd, timeout=None):
    return subprocess.run(args, cwd=str(cwd), text=True, capture_output=True, check=False, timeout=timeout)


def ensure_driver():
    name = "zt.exe" if os.name == "nt" else "zt"
    path = ROOT / name
    if path.exists():
        return path
    c = run_cmd([sys.executable, "build.py"], ROOT)
    if c.returncode != 0:
        raise RuntimeError(f"falha build.py\nstdout:\n{c.stdout}\nstderr:\n{c.stderr}")
    if not path.exists():
        raise RuntimeError("driver nao gerado")
    return path


def parse_manifest(project_dir):
    p = project_dir / "zenith.ztproj"
    if not p.exists():
        raise RuntimeError(f"manifesto ausente: {p}")
    section = ""
    name = project_dir.name
    out = "build"
    for raw in p.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        if line.startswith("[") and line.endswith("]"):
            section = line[1:-1].strip().lower()
            continue
        if "=" not in line:
            continue
        k, v = line.split("=", 1)
        k = k.strip()
        v = v.strip()
        if v.startswith('"') and v.endswith('"'):
            v = v[1:-1]
        if section == "project" and k == "name" and v:
            name = v
        if section == "build" and k == "output" and v:
            out = v
    return name, out


def exe_path(project_dir):
    name, out = parse_manifest(project_dir)
    suf = ".exe" if os.name == "nt" else ""
    return project_dir / out / f"{name}{suf}"


def clean_build(project_dir):
    _, out = parse_manifest(project_dir)
    shutil.rmtree(project_dir / out, ignore_errors=True)


def psq(s):
    return "'" + str(s).replace("'", "''") + "'"


def normalize_out(s):
    return (s or "").replace("\r\n", "\n")


def trim(s, limit=12000):
    s = s or ""
    return s if len(s) <= limit else s[:limit] + "\n...[truncated]..."


def measure(exe, args, cwd, timeout_sec):
    # Use the same fast Python+ctypes path on Windows so working-set measurement
    # is consistent across kind=bin and kind=cmd benchmarks. The legacy
    # PowerShell wrapper is kept below as a fallback (force via env var).
    if os.name == "nt" and os.environ.get("ZT_PERF_LEGACY_WRAPPER") != "1":
        return measure_cmd_fast(exe, args, cwd, timeout_sec)

    if os.name != "nt":
        t0 = time.perf_counter()
        p = subprocess.run([str(exe), *args], cwd=str(cwd), text=True, capture_output=True, check=False, timeout=timeout_sec)
        return {"exit": p.returncode, "ws": 0, "alloc": 0, "stdout": normalize_out(p.stdout), "stderr": normalize_out(p.stderr), "ms": (time.perf_counter() - t0) * 1000.0, "timeout": False}

    arg_line = subprocess.list2cmdline(args)
    cmd = "\n".join([
        "$psi = New-Object System.Diagnostics.ProcessStartInfo",
        f"$psi.FileName = {psq(exe)}",
        f"$psi.Arguments = {psq(arg_line)}",
        f"$psi.WorkingDirectory = {psq(cwd)}",
        "$psi.UseShellExecute = $false",
        "$psi.RedirectStandardOutput = $true",
        "$psi.RedirectStandardError = $true",
        "$p = New-Object System.Diagnostics.Process",
        "$p.StartInfo = $psi",
        "$sw = [System.Diagnostics.Stopwatch]::StartNew()",
        "$null = $p.Start()",
        "$peak_ws = 0",
        "$peak_alloc = 0",
        "$to = $false",
        "$stdout = ''",
        "$stderr = ''",
        f"$limit = {max(1000, int(timeout_sec * 1000))}",
        "while (-not $p.HasExited) {",
        "  try {",
        "    $p.Refresh()",
        "    if ($p.WorkingSet64 -gt $peak_ws) { $peak_ws = $p.WorkingSet64 }",
        "    if ($p.PrivateMemorySize64 -gt $peak_alloc) { $peak_alloc = $p.PrivateMemorySize64 }",
        "    while ($p.StandardOutput.Peek() -ne -1) { $stdout += [char]$p.StandardOutput.Read() }",
        "    while ($p.StandardError.Peek() -ne -1) { $stderr += [char]$p.StandardError.Read() }",
        "  } catch { }",
        "  if ($sw.ElapsedMilliseconds -gt $limit) { try { $p.Kill() } catch { }; $to = $true; break }",
        "  Start-Sleep -Milliseconds 1",
        "}",
        "while ($p.StandardOutput.Peek() -ne -1) { $stdout += [char]$p.StandardOutput.Read() }",
        "while ($p.StandardError.Peek() -ne -1) { $stderr += [char]$p.StandardError.Read() }",
        "$p.WaitForExit()",
        "$p.Refresh()",
        "if ($p.PeakWorkingSet64 -gt $peak_ws) { $peak_ws = $p.PeakWorkingSet64 }",
        "if ($p.PrivateMemorySize64 -gt $peak_alloc) { $peak_alloc = $p.PrivateMemorySize64 }",
        "$r = [ordered]@{ exit = $p.ExitCode; ws = $peak_ws; alloc = $peak_alloc; stdout = $stdout; stderr = $stderr; ms = $sw.Elapsed.TotalMilliseconds; timeout = $to }",
        "$r | ConvertTo-Json -Compress",
    ])
    c = run_cmd(["powershell", "-NoProfile", "-Command", cmd], ROOT, timeout=max(timeout_sec + 10, 30))
    if c.returncode != 0:
        raise RuntimeError(f"falha medicao\nstdout:\n{c.stdout}\nstderr:\n{c.stderr}")
    d = json.loads(c.stdout.strip())
    return {"exit": int(d.get("exit", -1)), "ws": int(d.get("ws", 0)), "alloc": int(d.get("alloc", 0)), "stdout": normalize_out(d.get("stdout", "")), "stderr": normalize_out(d.get("stderr", "")), "ms": float(d.get("ms", 0.0)), "timeout": bool(d.get("timeout", False))}


if os.name == "nt":
    import ctypes
    from ctypes import wintypes

    class _PROCESS_MEMORY_COUNTERS(ctypes.Structure):
        _fields_ = [
            ("cb", wintypes.DWORD),
            ("PageFaultCount", wintypes.DWORD),
            ("PeakWorkingSetSize", ctypes.c_size_t),
            ("WorkingSetSize", ctypes.c_size_t),
            ("QuotaPeakPagedPoolUsage", ctypes.c_size_t),
            ("QuotaPagedPoolUsage", ctypes.c_size_t),
            ("QuotaPeakNonPagedPoolUsage", ctypes.c_size_t),
            ("QuotaNonPagedPoolUsage", ctypes.c_size_t),
            ("PagefileUsage", ctypes.c_size_t),
            ("PeakPagefileUsage", ctypes.c_size_t),
        ]

    try:
        _psapi_GetProcessMemoryInfo = ctypes.windll.psapi.GetProcessMemoryInfo
        _psapi_GetProcessMemoryInfo.argtypes = [wintypes.HANDLE, ctypes.POINTER(_PROCESS_MEMORY_COUNTERS), wintypes.DWORD]
        _psapi_GetProcessMemoryInfo.restype = wintypes.BOOL
    except (AttributeError, OSError):
        _psapi_GetProcessMemoryInfo = None

    def _read_process_memory(handle):
        if _psapi_GetProcessMemoryInfo is None or not handle:
            return 0, 0
        pmc = _PROCESS_MEMORY_COUNTERS()
        pmc.cb = ctypes.sizeof(pmc)
        ok = _psapi_GetProcessMemoryInfo(int(handle), ctypes.byref(pmc), ctypes.sizeof(pmc))
        if not ok:
            return 0, 0
        return int(pmc.PeakWorkingSetSize), int(pmc.PeakPagefileUsage)


def measure_cmd_fast(exe, args, cwd, timeout_sec):
    """Fast subprocess-based measurement.

    On Windows, polls the live process handle with Win32 GetProcessMemoryInfo so
    peak working set is captured even for sub-100ms processes (without the
    PowerShell-wrapper overhead used by `measure`). On POSIX, working set is
    not collected here (same behavior as the legacy fast path).
    """
    if os.name != "nt":
        t0 = time.perf_counter()
        p = subprocess.run([str(exe), *args], cwd=str(cwd), text=True, capture_output=True, check=False, timeout=timeout_sec)
        return {
            "exit": p.returncode,
            "ws": 0,
            "alloc": 0,
            "stdout": normalize_out(p.stdout),
            "stderr": normalize_out(p.stderr),
            "ms": (time.perf_counter() - t0) * 1000.0,
            "timeout": False,
        }

    t0 = time.perf_counter()
    proc = subprocess.Popen(
        [str(exe), *args],
        cwd=str(cwd),
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    peak_ws = 0
    peak_alloc = 0
    timed_out = False
    handle = getattr(proc, "_handle", None)
    deadline = t0 + max(1.0, float(timeout_sec))
    try:
        while True:
            cur_ws, cur_alloc = _read_process_memory(handle)
            if cur_ws > peak_ws:
                peak_ws = cur_ws
            if cur_alloc > peak_alloc:
                peak_alloc = cur_alloc
            if proc.poll() is not None:
                break
            if time.perf_counter() > deadline:
                proc.kill()
                timed_out = True
                break
            time.sleep(0.001)
        # Final snapshot while handle is still valid (after process exit but
        # before communicate() closes it).
        cur_ws, cur_alloc = _read_process_memory(handle)
        if cur_ws > peak_ws:
            peak_ws = cur_ws
        if cur_alloc > peak_alloc:
            peak_alloc = cur_alloc
        try:
            stdout, stderr = proc.communicate(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()
            stdout, stderr = proc.communicate()
            timed_out = True
    except Exception:
        try:
            proc.kill()
        except Exception:
            pass
        raise

    return {
        "exit": proc.returncode if proc.returncode is not None else -1,
        "ws": peak_ws,
        "alloc": peak_alloc,
        "stdout": normalize_out(stdout or ""),
        "stderr": normalize_out(stderr or ""),
        "ms": (time.perf_counter() - t0) * 1000.0,
        "timeout": timed_out,
    }

def pct(values, p):
    if not values:
        return 0.0
    if len(values) == 1:
        return float(values[0])
    pos = (len(values) - 1) * p
    lo = int(math.floor(pos))
    hi = int(math.ceil(pos))
    if lo == hi:
        return float(values[lo])
    w = pos - lo
    return float(values[lo]) * (1.0 - w) + float(values[hi]) * w


def outliers_iqr(values):
    if len(values) < 4:
        return set()
    v = sorted(float(x) for x in values)
    q1 = pct(v, 0.25)
    q3 = pct(v, 0.75)
    iqr = q3 - q1
    lo = q1 - (1.5 * iqr)
    hi = q3 + (1.5 * iqr)
    bad = set()
    for i, x in enumerate(values):
        fx = float(x)
        if fx < lo or fx > hi:
            bad.add(i)
    return bad


def metric_status(value, warn, fail, higher=False):
    if warn is None or fail is None:
        return "pass"
    if higher:
        if value <= fail:
            return "fail"
        if value <= warn:
            return "warn"
        return "pass"
    if value >= fail:
        return "fail"
    if value >= warn:
        return "warn"
    return "pass"


def baseline_cmp(current, baseline, higher=False, warn_pct=12.0, fail_pct=25.0):
    if baseline is None:
        return {"status": "pass", "pct": 0.0}
    b = float(baseline)
    c = float(current)
    if b == 0.0:
        return {"status": "pass", "pct": 0.0}
    reg = ((b - c) / b) * 100.0 if higher else ((c - b) / b) * 100.0
    if reg >= fail_pct:
        return {"status": "fail", "pct": reg}
    if reg >= warn_pct:
        return {"status": "warn", "pct": reg}
    return {"status": "pass", "pct": reg}



def cmp_thresholds(task, metric_name, key, default_warn, default_fail):
    cfg = task.get(key)
    if not isinstance(cfg, dict):
        return float(default_warn), float(default_fail)
    metric_cfg = cfg.get(metric_name, {})
    if not isinstance(metric_cfg, dict):
        metric_cfg = {}
    warn = metric_cfg.get("warn_pct", cfg.get("warn_pct", default_warn))
    fail = metric_cfg.get("fail_pct", cfg.get("fail_pct", default_fail))
    return float(warn), float(fail)

def load_baseline(platform_id, bid):
    p = BASELINE_DIR / platform_id / f"{bid}.json"
    if not p.exists():
        return None
    return json.loads(p.read_text(encoding="utf-8"))


def save_baseline(platform_id, bid, metrics):
    p = BASELINE_DIR / platform_id / f"{bid}.json"
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(json.dumps({"benchmark_id": bid, "generated_at_utc": now_iso(), "platform_key": platform_id, "host": host_meta(), "metric_values": metrics}, indent=2) + "\n", encoding="utf-8")


def load_base_report(path):
    if not path:
        return {}
    p = Path(path)
    if not p.exists():
        raise RuntimeError(f"base report ausente: {p}")
    data = json.loads(p.read_text(encoding="utf-8"))
    out = {}
    for b in data.get("benchmarks", []):
        m = {}
        for k, v in b.get("metrics", {}).items():
            m[k] = v.get("value") if isinstance(v, dict) and "value" in v else v
        out[b["benchmark_id"]] = m
    return out


def run_one(task, driver, suite_name, suite_cfg, platform_id, base_metrics):
    warmup = suite_cfg["warmup"]
    iters = suite_cfg["iters"]
    if isinstance(task.get("iters"), dict):
        iters = int(task["iters"].get(suite_name, iters))

    timeout = int(task.get("timeout", 180))

    if task["kind"] == "bin":
        c = run_cmd([str(driver), "build", str(task["project"]), "--ci"], ROOT, timeout=240)
        if c.returncode != 0 and os.name == "nt":
            c = run_cmd([str(driver), "build", str(task["project"]), "--ci"], ROOT, timeout=240)
        if c.returncode != 0:
            raise RuntimeError(f"falha build de preparo {task['id']}\nstdout:\n{c.stdout}\nstderr:\n{c.stderr}")
        task["_exe"] = exe_path(task["project"])
        if not task["_exe"].exists():
            raise RuntimeError(f"binario nao encontrado: {task['_exe']}")

    for _ in range(warmup):
        _ = exec_sample(task, driver, timeout)

    samples = []
    retries = 0
    target = iters
    while True:
        while len(samples) < target:
            samples.append(exec_sample(task, driver, timeout))
        bad = outliers_iqr([s["ms"] for s in samples])
        med = statistics.median([s["ms"] for s in samples]) if samples else 0.0
        std = statistics.pstdev([s["ms"] for s in samples]) if len(samples) > 1 else 0.0
        cv = (std / med) if med > 0 else 0.0
        if (not bad and cv <= suite_cfg["cv"]) or retries >= suite_cfg["retry"]:
            break
        retries += 1
        target += max(1, suite_cfg["iters"] // 2)

    bad = outliers_iqr([s["ms"] for s in samples])
    lat_values = [s["ms"] for i, s in enumerate(samples) if i not in bad]
    if len(lat_values) < 2:
        lat_values = [s["ms"] for s in samples]

    lat_sorted = sorted(float(x) for x in lat_values)
    lat_median = float(statistics.median(lat_sorted)) if lat_sorted else 0.0
    lat_p95 = pct(lat_sorted, 0.95)
    lat_std = float(statistics.pstdev(lat_sorted)) if len(lat_sorted) > 1 else 0.0
    throughput = (1000.0 / lat_median) if lat_median > 0 else 0.0
    peak_ws = max(int(s["ws"]) for s in samples) if samples else 0
    alloc = max(int(s["alloc"]) for s in samples) if samples else 0
    bsize = int(task.get("_exe", exe_path(task["project"])).stat().st_size) if exe_path(task["project"]).exists() else 0

    metrics = {
        "lat_median_ms": round(lat_median, 3),
        "lat_p95_ms": round(lat_p95, 3),
        "lat_std_ms": round(lat_std, 3),
        "throughput_ops_per_sec": round(throughput, 4),
        "peak_ws": peak_ws,
        "alloc_proxy": alloc,
        "binary_size": bsize,
    }
    if task["kind"] == "bin":
        metrics["startup_ms"] = metrics["lat_median_ms"]

    budget = task.get("budget", default_budget(task["cat"]))
    bl = load_baseline(platform_id, task["id"])
    blm = None if bl is None else bl.get("metric_values", {})
    brm = base_metrics.get(task["id"], {})

    compared = {}
    status = "pass"
    for k, v in metrics.items():
        higher = k == "throughput_ops_per_sec"
        b_status = metric_status(float(v), budget["warn"].get(k), budget["fail"].get(k), higher=higher)
        # Keep jitter visibility without turning it into a flaky gate.
        if k == "lat_std_ms":
            bl_status = {"status": "pass", "pct": 0.0}
            br_status = {"status": "pass", "pct": 0.0}
        else:
            bl_warn_pct, bl_fail_pct = cmp_thresholds(task, k, "baseline_cmp", 12.0, 25.0)
            br_warn_pct, br_fail_pct = cmp_thresholds(task, k, "base_report_cmp", 8.0, 18.0)
            bl_status = baseline_cmp(float(v), None if blm is None else blm.get(k), higher=higher, warn_pct=bl_warn_pct, fail_pct=bl_fail_pct)
            br_status = baseline_cmp(float(v), brm.get(k), higher=higher, warn_pct=br_warn_pct, fail_pct=br_fail_pct)
        final = b_status
        for st in (bl_status["status"], br_status["status"]):
            if st == "fail":
                final = "fail"
                break
            if st == "warn" and final != "fail":
                final = "warn"
        compared[k] = {"value": v, "budget": b_status, "baseline": bl_status, "base_report": br_status, "status": final}
        if final == "fail":
            status = "fail"
        elif final == "warn" and status != "fail":
            status = "warn"

    rep = {
        "benchmark_id": task["id"],
        "scenario": task["scenario"],
        "category": task["cat"],
        "kind": task["kind"],
        "suite": suite_name,
        "status": status,
        "generated_at_utc": now_iso(),
        "host": host_meta(),
        "metrics": compared,
        "metric_values": metrics,
        "budget": budget,
        "method": {"warmup": warmup, "runs": len(samples), "retries": retries, "outliers": len(bad)},
        "samples": samples,
        "last": samples[-1] if samples else {"exit": 0, "ms": 0, "ws": 0, "alloc": 0, "stdout": "", "stderr": ""},
    }
    return rep


def exec_sample(task, driver, timeout):
    expected_exit = int(task.get("exit", 0))

    def run_once():
        if task["kind"] == "bin":
            return measure(task["_exe"], [], task["project"], timeout)

        if task.get("cold"):
            clean_build(task["project"])
        args = [task["cmd"], str(task["project"]), "--ci"]
        if os.name == "nt":
            return measure_cmd_fast(driver, args, ROOT, timeout)
        return measure(driver, args, ROOT, timeout)

    sample = run_once()

    # Windows command benches can be flaky due transient file locks/process races; retry once.
    if sample["exit"] != expected_exit and os.name == "nt" and task["kind"] == "cmd":
        sample = run_once()

    sample["stdout"] = trim(sample["stdout"])
    sample["stderr"] = trim(sample["stderr"])
    if sample["timeout"]:
        raise RuntimeError(f"timeout em {task['id']}")
    if sample["exit"] != expected_exit:
        raise RuntimeError(f"exit invalido em {task['id']}: {sample['exit']}\nstdout:\n{sample['stdout']}\nstderr:\n{sample['stderr']}")
    if task.get("expect") and task["expect"] not in sample["stdout"]:
        raise RuntimeError(f"stdout invalido em {task['id']}: faltou {task['expect']!r}\nstdout:\n{sample['stdout']}")
    return sample

def write_benchmark_report(rep, suite_name):
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    jp = REPORT_DIR / f"{suite_name}-{rep['benchmark_id']}.json"
    mp = REPORT_DIR / f"{suite_name}-{rep['benchmark_id']}.md"
    jp.write_text(json.dumps(rep, indent=2) + "\n", encoding="utf-8")

    lines = [
        f"# Perf Benchmark: {rep['benchmark_id']}",
        "",
        f"- scenario: `{rep['scenario']}`",
        f"- suite: `{rep['suite']}`",
        f"- status: `{rep['status']}`",
        "",
        "| metric | value | budget | baseline | base-report | final |",
        "| --- | ---: | --- | --- | --- | --- |",
    ]
    for k, v in rep["metrics"].items():
        lines.append(f"| `{k}` | `{v['value']}` | `{v['budget']}` | `{v['baseline']['status']}` | `{v['base_report']['status']}` | `{v['status']}` |")
    lines += [
        "",
        "## Method",
        "",
        f"- warmup: `{rep['method']['warmup']}`",
        f"- runs: `{rep['method']['runs']}`",
        f"- retries: `{rep['method']['retries']}`",
        f"- outliers: `{rep['method']['outliers']}`",
        "",
        "## Last Sample",
        "",
        f"- exit: `{rep['last']['exit']}`",
        f"- ms: `{rep['last']['ms']}`",
        f"- ws: `{rep['last']['ws']}`",
        f"- alloc_proxy: `{rep['last']['alloc']}`",
    ]
    if rep["last"]["stdout"]:
        lines += ["", "```text", rep["last"]["stdout"].rstrip(), "```"]
    if rep["last"]["stderr"]:
        lines += ["", "```text", rep["last"]["stderr"].rstrip(), "```"]

    mp.write_text("\n".join(lines) + "\n", encoding="utf-8")


def select_benches(args, specs):
    by_id = {b["id"]: b for b in specs}
    by_scenario = {}
    for b in specs:
        by_scenario.setdefault(b["scenario"], []).append(b)

    if args.list:
        print("Suites:")
        for s in SUITES:
            print(f"  - {s}")
        print("Scenarios:")
        for s in sorted(by_scenario):
            print(f"  - {s}")
        print("Benchmarks:")
        for b in specs:
            print(f"  - {b['id']} ({b['scenario']})")
        raise SystemExit(0)

    targets = []
    targets.extend(args.targets or [])
    targets.extend(args.scenario or [])
    targets.extend(args.benchmark or [])

    selected = []
    if targets:
        added = set()
        for raw in targets:
            t = "m37_result_generic" if raw == "m37" else raw
            if t in by_id:
                if t not in added:
                    selected.append(by_id[t])
                    added.add(t)
                continue
            if t in by_scenario:
                for b in by_scenario[t]:
                    if b["id"] not in added:
                        selected.append(b)
                        added.add(b["id"])
                continue
            raise RuntimeError(f"alvo desconhecido: {raw}")
    else:
        selected = [b for b in specs if args.suite in b["suites"]]

    if not selected:
        raise RuntimeError("nenhum benchmark selecionado")
    return selected


def parse_args(argv):
    ap = argparse.ArgumentParser(prog="python tests/perf/run_perf.py", description="M36 perf suite")
    ap.add_argument("targets", nargs="*", help="benchmark id ou scenario")
    ap.add_argument("--suite", choices=sorted(SUITES.keys()), default="quick")
    ap.add_argument("--scenario", action="append", default=[])
    ap.add_argument("--benchmark", action="append", default=[])
    ap.add_argument("--update-baseline", action="store_true")
    ap.add_argument("--platform-key", default="")
    ap.add_argument("--base-report", default="")
    ap.add_argument("--release-gate", action="store_true")
    ap.add_argument("--override-file", default="")
    ap.add_argument("--list", action="store_true")
    return ap.parse_args(argv)


def run_suite(args):
    suite_name = args.suite
    if suite_name not in SUITES:
        raise RuntimeError(f"suite invalida: {suite_name}")
    suite_cfg = SUITES[suite_name]

    specs = bench_specs()
    selected = select_benches(args, specs)
    base_metrics = load_base_report(args.base_report)
    platform_id = args.platform_key or pkey()
    driver = ensure_driver()

    reports = []
    scen = {}

    for b in selected:
        r = run_one(b, driver, suite_name, suite_cfg, platform_id, base_metrics)
        reports.append(r)
        write_benchmark_report(r, suite_name)
        print(f"{b['id']}: {r['status']}")

        s = scen.get(b["scenario"], {"status": "pass", "count": 0})
        s["count"] += 1
        if r["status"] == "fail":
            s["status"] = "fail"
        elif r["status"] == "warn" and s["status"] != "fail":
            s["status"] = "warn"
        scen[b["scenario"]] = s

        if args.update_baseline:
            save_baseline(platform_id, b["id"], r["metric_values"])

    overall = "pass"
    for r in reports:
        if r["status"] == "fail":
            overall = "fail"
            break
        if r["status"] == "warn":
            overall = "warn"

    summary = {
        "suite": suite_name,
        "generated_at_utc": now_iso(),
        "platform_key": platform_id,
        "host": host_meta(),
        "status": overall,
        "benchmark_count": len(reports),
        "scenarios": scen,
        "benchmarks": reports,
        "release_gate": bool(args.release_gate),
        "override_file": args.override_file or "",
    }

    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    sj = REPORT_DIR / f"summary-{suite_name}.json"
    sm = REPORT_DIR / f"summary-{suite_name}.md"
    sj.write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")

    lines = [
        f"# Perf Summary ({suite_name})",
        "",
        f"- generated_at_utc: `{summary['generated_at_utc']}`",
        f"- platform_key: `{summary['platform_key']}`",
        f"- status: `{summary['status']}`",
        f"- benchmark_count: `{summary['benchmark_count']}`",
        "",
        "| scenario | status | benchmarks |",
        "| --- | --- | ---: |",
    ]
    for k in sorted(scen):
        lines.append(f"| `{k}` | `{scen[k]['status']}` | `{scen[k]['count']}` |")
    lines += ["", "| benchmark | scenario | status | lat(ms) | peak_ws |", "| --- | --- | --- | ---: | ---: |"]
    for r in reports:
        lines.append(f"| `{r['benchmark_id']}` | `{r['scenario']}` | `{r['status']}` | `{r['metrics']['lat_median_ms']['value']}` | `{r['metrics']['peak_ws']['value']}` |")
    sm.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(f"summary: {overall}")
    print(f"json: {sj}")
    print(f"md: {sm}")

    if overall == "fail":
        if args.override_file:
            p = Path(args.override_file)
            if not p.exists():
                raise RuntimeError(f"override ausente: {p}")
            if not p.read_text(encoding="utf-8").strip():
                raise RuntimeError(f"override vazio: {p}")
            print(f"override aplicado: {p}")
            return 0
        return 1
    return 0


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    args = parse_args(argv)
    try:
        return run_suite(args)
    except subprocess.TimeoutExpired as exc:
        print(f"perf timeout: {exc}", file=sys.stderr)
        return 1
    except Exception as exc:
        print(f"perf error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())





