"""
Focused coverage snapshot for core language modules.

This is intentionally small and reproducible:
- it does not try to compute whole-project union coverage;
- it reports per-scenario coverage for a few critical files;
- it writes a markdown summary under reports/coverage/.
"""

from __future__ import annotations

import json
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
TMP_DIR = ROOT / ".ztc-tmp" / "coverage"
REPORT_DIR = ROOT / "reports" / "coverage"
REPORT_MD = REPORT_DIR / "coverage-snapshot.md"
REPORT_JSON = REPORT_DIR / "coverage-snapshot.json"


def normalize_path_text(path: Path | str) -> str:
    return str(path).replace("\\", "/").lower()


def compiler_sources() -> list[Path]:
    compiler_dir = ROOT / "compiler"
    files = []
    for path in compiler_dir.rglob("*.c"):
        text = str(path).replace("\\", "/")
        if "/compiler/driver/" in text or "/compiler/tooling/" in text:
            continue
        files.append(path)
    return sorted(files)


COMPILER_SOURCES = compiler_sources()
COMPILER_SOURCES_NO_ZIR_PARSER = [
    path for path in COMPILER_SOURCES
    if normalize_path_text(path) != normalize_path_text(ROOT / "compiler" / "zir" / "parser.c")
]


@dataclass(frozen=True)
class CoverageCase:
    case_id: str
    source: Path
    compile_sources: tuple[Path, ...]
    link_args: tuple[str, ...] = ()
    timeout_compile: int = 420
    timeout_run: int = 240
    note: str = ""


CASES = (
    CoverageCase(
        case_id="frontend/parser_depth",
        source=ROOT / "compiler" / "frontend" / "parser" / "parser.c",
        compile_sources=tuple(COMPILER_SOURCES_NO_ZIR_PARSER + [ROOT / "tests" / "frontend" / "test_parser_depth_guard.c"]),
        note="Guarda de profundidade do parser.",
    ),
    CoverageCase(
        case_id="semantic/numeric_guardrails",
        source=ROOT / "compiler" / "semantic" / "types" / "checker.c",
        compile_sources=tuple(COMPILER_SOURCES + [ROOT / "tests" / "semantic" / "test_numeric_literal_guardrails.c"]),
        note="Overflow inteiro e faixa de float no checker.",
    ),
    CoverageCase(
        case_id="zir/lowering_depth",
        source=ROOT / "compiler" / "zir" / "lowering" / "from_hir.c",
        compile_sources=tuple(COMPILER_SOURCES + [ROOT / "tests" / "zir" / "test_lowering_depth_guard.c"]),
        note="Guarda estrutural do lowering HIR -> ZIR.",
    ),
    CoverageCase(
        case_id="backend/emitter_stream",
        source=ROOT / "compiler" / "targets" / "c" / "emitter.c",
        compile_sources=(
            ROOT / "compiler" / "zir" / "model.c",
            ROOT / "compiler" / "zir" / "verifier.c",
            ROOT / "compiler" / "targets" / "c" / "legalization.c",
            ROOT / "compiler" / "targets" / "c" / "emitter.c",
            ROOT / "tests" / "targets" / "c" / "test_emitter_stream.c",
        ),
        note="Caminho spill + stream do emitter C.",
    ),
    CoverageCase(
        case_id="runtime/process_run",
        source=ROOT / "runtime" / "c" / "zenith_rt.c",
        compile_sources=(
            ROOT / "runtime" / "c" / "zenith_rt.c",
            ROOT / "tests" / "runtime" / "c" / "test_process_run.c",
        ),
        link_args=("-lws2_32",),
        note="Execucao de processo segura, cwd e overflows fatais.",
    ),
    CoverageCase(
        case_id="runtime/text_utf8_guardrails",
        source=ROOT / "runtime" / "c" / "zenith_rt.c",
        compile_sources=(
            ROOT / "runtime" / "c" / "zenith_rt.c",
            ROOT / "tests" / "runtime" / "c" / "test_text_utf8_guardrails.c",
        ),
        link_args=("-lws2_32",),
        note="Construtor UTF-8 validado e falha em entrada invalida.",
    ),
)


def run_cmd(args: list[str], cwd: Path | None = None, timeout: int = 240) -> tuple[int, str]:
    completed = subprocess.run(
        args,
        cwd=str(cwd or ROOT),
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


def ensure_tool(name: str) -> str | None:
    return shutil.which(name)


def slug(case_id: str) -> str:
    return case_id.replace("/", "__").replace("\\", "__")


def run_gcov_for_source(build_dir: Path, exe_stem: str, source: Path) -> str:
    candidates = []
    for candidate in sorted(build_dir.glob(f"{exe_stem}-*.gcno")):
        if candidate.stem.endswith(f"-{source.stem}"):
            candidates.append(candidate)

    if not candidates:
        raise RuntimeError(f"missing gcno for {source.name} in {build_dir}")

    expected_source = normalize_path_text(source)
    gcov_name = build_dir / f"{source.name}.gcov"
    attempted = []

    for candidate in candidates:
        if gcov_name.exists():
            gcov_name.unlink()

        rc_gcov, out_gcov = run_cmd(["gcov", "-b", "-c", str(candidate)], cwd=build_dir, timeout=120)
        attempted.append((candidate.name, rc_gcov, out_gcov[:400]))
        if rc_gcov != 0 or not gcov_name.exists():
            continue

        first_line = gcov_name.read_text(encoding="utf-8", errors="replace").splitlines()[0]
        if "Source:" not in first_line:
            continue

        actual_source = normalize_path_text(first_line.split("Source:", 1)[1].strip())
        if actual_source == expected_source:
            return out_gcov

    summary = "\n".join(f"- {name}: rc={rc}" for name, rc, _out in attempted)
    raise RuntimeError(f"gcov could not resolve source {source}\n{summary}")


def parse_gcov_metrics(output: str) -> dict[str, object]:
    patterns = {
        "lines": re.search(r"Lines executed:([0-9.]+)% of (\d+)", output),
        "branches": re.search(r"Branches executed:([0-9.]+)% of (\d+)", output),
        "calls": re.search(r"Calls executed:([0-9.]+)% of (\d+)", output),
    }
    if patterns["lines"] is None:
        raise ValueError(f"gcov output without line metrics:\n{output}")

    result: dict[str, object] = {}
    for key, match in patterns.items():
        if match is None:
            result[key] = None
            continue
        result[key] = {
            "percent": float(match.group(1)),
            "count": int(match.group(2)),
        }
    return result


def metric_cell(metric: object) -> str:
    if not isinstance(metric, dict):
        return "-"
    return f"{metric['percent']:.2f}% ({metric['count']})"


def build_case(case: CoverageCase) -> dict[str, object]:
    build_dir = TMP_DIR / slug(case.case_id)
    exe_stem = slug(case.case_id)
    exe_path = build_dir / f"{exe_stem}.exe"

    shutil.rmtree(build_dir, ignore_errors=True)
    build_dir.mkdir(parents=True, exist_ok=True)

    compile_cmd = [
        "gcc",
        "-std=c11",
        "-Wall",
        "-Wextra",
        "-pedantic",
        "--coverage",
        "-I.",
        *[str(path) for path in case.compile_sources],
        "-o",
        str(exe_path),
        *case.link_args,
    ]
    rc_compile, out_compile = run_cmd(compile_cmd, timeout=case.timeout_compile)
    if rc_compile != 0:
        raise RuntimeError(f"compile failed for {case.case_id}\n{out_compile[:1200]}")

    rc_run, out_run = run_cmd([str(exe_path)], timeout=case.timeout_run)
    if rc_run != 0:
        raise RuntimeError(f"run failed for {case.case_id}\n{out_run[:1200]}")

    out_gcov = run_gcov_for_source(build_dir, exe_stem, case.source)
    metrics = parse_gcov_metrics(out_gcov)
    return {
        "case_id": case.case_id,
        "source": str(case.source.relative_to(ROOT)).replace("\\", "/"),
        "note": case.note,
        "lines": metrics["lines"],
        "branches": metrics["branches"],
        "calls": metrics["calls"],
    }


def write_reports(results: list[dict[str, object]], skipped_reason: str | None = None) -> None:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)

    payload = {
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
        "kind": "focused-coverage-snapshot",
        "note": "Scenario coverage only; this is not whole-project union coverage.",
        "skipped_reason": skipped_reason,
        "results": results,
    }
    REPORT_JSON.write_text(json.dumps(payload, indent=2, ensure_ascii=True) + "\n", encoding="utf-8")

    lines = [
        "# Coverage Snapshot",
        "",
        f"Gerado em: `{payload['generated_at_utc']}`",
        "",
        "Escopo:",
        "- cobertura focada por cenario;",
        "- nao e cobertura unificada do projeto inteiro;",
        "- serve para dar visibilidade objetiva por modulo critico.",
        "",
    ]

    if skipped_reason is not None:
        lines.extend([
            f"Status: `SKIP`",
            "",
            f"Motivo: {skipped_reason}",
            "",
        ])
    else:
        lines.extend([
            "| Cenario | Arquivo | Linhas | Branches | Calls | Nota |",
            "|---|---|---:|---:|---:|---|",
        ])
        for item in results:
            lines.append(
                f"| `{item['case_id']}` | `{item['source']}` | "
                f"{metric_cell(item['lines'])} | {metric_cell(item['branches'])} | "
                f"{metric_cell(item['calls'])} | {item['note']} |"
            )
        lines.extend([
            "",
            "Leitura:",
            "- percentuais baixos aqui nao significam regressao por si so;",
            "- eles mostram quanto cada cenario focado toca no arquivo alvo;",
            "- o objetivo e acompanhar tendencia e descobrir buracos evidentes.",
            "",
        ])

    REPORT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    if ensure_tool("gcc") is None:
        reason = "gcc nao encontrado no ambiente"
        write_reports([], skipped_reason=reason)
        print(f"coverage snapshot skipped: {reason}")
        return 0

    if ensure_tool("gcov") is None:
        reason = "gcov nao encontrado no ambiente"
        write_reports([], skipped_reason=reason)
        print(f"coverage snapshot skipped: {reason}")
        return 0

    TMP_DIR.mkdir(parents=True, exist_ok=True)

    results = []
    for case in CASES:
        results.append(build_case(case))

    write_reports(results)
    print(f"coverage snapshot ok -> {REPORT_MD}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
