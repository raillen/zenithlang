import json
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


SCENARIOS = {
    "m37_result_generic": {
        "project_dir": ROOT / "tests" / "perf" / "m37_result_generic",
        "budget_file": ROOT / "tests" / "perf" / "budgets" / "m37_result_generic.json",
    },
}


def run_command(args, cwd):
    completed = subprocess.run(
        args,
        cwd=str(cwd),
        text=True,
        capture_output=True,
        check=False,
    )
    return completed


def ensure_driver():
    zt_path = ROOT / "zt.exe"
    if zt_path.exists():
        return zt_path

    completed = run_command([sys.executable, "build.py"], ROOT)
    if completed.returncode != 0:
        raise RuntimeError(
            "falha ao compilar zt.exe\nstdout:\n%s\nstderr:\n%s"
            % (completed.stdout, completed.stderr)
        )
    if not zt_path.exists():
        raise RuntimeError("build.py terminou sem gerar zt.exe")
    return zt_path


def ps_quote(text):
    return "'" + text.replace("'", "''") + "'"


def run_with_peak_memory(exe_path, working_dir):
    command = "\n".join(
        [
            "$psi = New-Object System.Diagnostics.ProcessStartInfo",
            f"$psi.FileName = {ps_quote(str(exe_path))}",
            f"$psi.WorkingDirectory = {ps_quote(str(working_dir))}",
            "$psi.UseShellExecute = $false",
            "$psi.RedirectStandardOutput = $true",
            "$psi.RedirectStandardError = $true",
            "$process = New-Object System.Diagnostics.Process",
            "$process.StartInfo = $psi",
            "$null = $process.Start()",
            "$peak_working_set64 = 0",
            "while (-not $process.HasExited) {",
            "  try {",
            "    $process.Refresh()",
            "    if ($process.WorkingSet64 -gt $peak_working_set64) { $peak_working_set64 = $process.WorkingSet64 }",
            "  } catch { }",
            "  Start-Sleep -Milliseconds 10",
            "}",
            "$stdout = $process.StandardOutput.ReadToEnd()",
            "$stderr = $process.StandardError.ReadToEnd()",
            "$process.WaitForExit()",
            "$process.Refresh()",
            "if ($process.PeakWorkingSet64 -gt $peak_working_set64) {",
            "  $peak_working_set64 = $process.PeakWorkingSet64",
            "}",
            "$result = [ordered]@{",
            "  exit_code = $process.ExitCode",
            "  peak_working_set64 = $peak_working_set64",
            "  stdout = $stdout",
            "  stderr = $stderr",
            "}",
            "$result | ConvertTo-Json -Compress",
        ]
    )
    completed = run_command(
        [
            "powershell",
            "-NoProfile",
            "-Command",
            command,
        ],
        ROOT,
    )
    if completed.returncode != 0:
        raise RuntimeError(
            "falha ao medir memoria\nstdout:\n%s\nstderr:\n%s"
            % (completed.stdout, completed.stderr)
        )
    return json.loads(completed.stdout.strip())


def metric_status(value, warn_limit, fail_limit):
    if value >= fail_limit:
        return "fail"
    if value >= warn_limit:
        return "warn"
    return "pass"


def render_markdown(report):
    metrics = report["metrics"]
    lines = [
        f"# Perf Report: {report['scenario']}",
        "",
        f"- generated_at_utc: `{report['generated_at_utc']}`",
        f"- host: `{report['host']}`",
        f"- platform: `{report['platform']}`",
        f"- status: `{report['status']}`",
        "",
        "## Metrics",
        "",
        "| metric | value | status |",
        "| --- | ---: | --- |",
    ]
    for key in [
        "build_median_ms",
        "binary_size_bytes",
        "peak_working_set_bytes",
        "peak_working_set_spread_bytes",
    ]:
        metric = metrics[key]
        lines.append(f"| `{key}` | `{metric['value']}` | `{metric['status']}` |")
    lines.extend(
        [
            "",
            "## Samples",
            "",
            f"- build_ms: `{report['samples']['build_ms']}`",
            f"- peak_working_set_bytes: `{report['samples']['peak_working_set_bytes']}`",
            "",
            "## Output Check",
            "",
            f"- expected_stdout_ok: `{report['stdout_ok']}`",
            f"- last_stdout: `{report['last_stdout'].rstrip()}`",
            f"- last_exit_code: `{report['last_exit_code']}`",
        ]
    )
    return "\n".join(lines) + "\n"


def run_scenario(name):
    if name not in SCENARIOS:
        raise RuntimeError(f"cenario desconhecido: {name}")

    scenario = SCENARIOS[name]
    budget = json.loads(scenario["budget_file"].read_text(encoding="utf-8"))
    zt_path = ensure_driver()
    project_dir = scenario["project_dir"]
    build_dir = project_dir / "build"
    exe_path = build_dir / f"{project_dir.name}.exe"

    build_samples = []
    run_samples = []
    last_stdout = ""
    last_exit_code = 0

    for _ in range(int(budget["build_samples"])):
        shutil.rmtree(build_dir, ignore_errors=True)
        started = time.perf_counter()
        completed = run_command([str(zt_path), "build", str(project_dir), "--ci"], ROOT)
        elapsed_ms = (time.perf_counter() - started) * 1000.0
        if completed.returncode != 0:
            raise RuntimeError(
                "falha no build do cenario %s\nstdout:\n%s\nstderr:\n%s"
                % (name, completed.stdout, completed.stderr)
            )
        build_samples.append(round(elapsed_ms, 3))

    if not exe_path.exists():
        raise RuntimeError(f"executavel esperado nao encontrado: {exe_path}")

    binary_size_bytes = exe_path.stat().st_size

    for _ in range(int(budget["run_samples"])):
        sample = run_with_peak_memory(exe_path, project_dir)
        if int(sample["exit_code"]) != 0:
            raise RuntimeError(
                "falha ao executar cenario %s\nstdout:\n%s\nstderr:\n%s"
                % (name, sample["stdout"], sample["stderr"])
            )
        run_samples.append(int(sample["peak_working_set64"]))
        last_stdout = sample["stdout"]
        last_exit_code = int(sample["exit_code"])

    expected_stdout = budget["expected_stdout"]
    normalized_stdout = last_stdout.replace("\r\n", "\n")
    stdout_ok = normalized_stdout == expected_stdout
    if not stdout_ok:
        raise RuntimeError(
            "saida inesperada no cenario %s\nesperado: %r\nrecebido: %r"
            % (name, expected_stdout, normalized_stdout)
        )

    build_median_ms = round(statistics.median(build_samples), 3)
    peak_working_set_bytes = max(run_samples)
    peak_working_set_spread_bytes = max(run_samples) - min(run_samples)

    metrics = {
        "build_median_ms": {
            "value": build_median_ms,
            "status": metric_status(
                build_median_ms,
                float(budget["warn"]["build_median_ms"]),
                float(budget["fail"]["build_median_ms"]),
            ),
        },
        "binary_size_bytes": {
            "value": binary_size_bytes,
            "status": metric_status(
                binary_size_bytes,
                int(budget["warn"]["binary_size_bytes"]),
                int(budget["fail"]["binary_size_bytes"]),
            ),
        },
        "peak_working_set_bytes": {
            "value": peak_working_set_bytes,
            "status": metric_status(
                peak_working_set_bytes,
                int(budget["warn"]["peak_working_set_bytes"]),
                int(budget["fail"]["peak_working_set_bytes"]),
            ),
        },
        "peak_working_set_spread_bytes": {
            "value": peak_working_set_spread_bytes,
            "status": metric_status(
                peak_working_set_spread_bytes,
                int(budget["warn"]["peak_working_set_spread_bytes"]),
                int(budget["fail"]["peak_working_set_spread_bytes"]),
            ),
        },
    }

    overall_status = "pass"
    for metric in metrics.values():
        if metric["status"] == "fail":
            overall_status = "fail"
            break
        if metric["status"] == "warn":
            overall_status = "warn"

    report = {
        "scenario": name,
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
        "host": platform.node(),
        "platform": platform.platform(),
        "status": overall_status,
        "stdout_ok": stdout_ok,
        "last_stdout": last_stdout,
        "last_exit_code": last_exit_code,
        "samples": {
            "build_ms": build_samples,
            "peak_working_set_bytes": run_samples,
        },
        "metrics": metrics,
        "budgets": budget,
    }

    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    json_path = REPORT_DIR / f"{name}.json"
    md_path = REPORT_DIR / f"{name}.md"
    json_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    md_path.write_text(render_markdown(report), encoding="utf-8")

    return report


def main():
    if len(sys.argv) > 2:
        raise SystemExit("uso: python tests/perf/run_perf.py [cenario]")

    scenario_names = [sys.argv[1]] if len(sys.argv) == 2 else list(SCENARIOS.keys())
    failed = []

    for name in scenario_names:
        report = run_scenario(name)
        print(f"{name}: {report['status']}")
        if report["status"] == "fail":
            failed.append(name)

    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()