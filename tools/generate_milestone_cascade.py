from __future__ import annotations

import argparse
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

from generate_code_maps import MODULE_SPECS, ROOT, normalize, summarize_file


ROADMAP_PATH = ROOT / "IMPLEMENTATION_ROADMAP.md"
CHECKLIST_PATH = ROOT / "IMPLEMENTATION_CHECKLIST.md"
OUTPUT_PATH = ROOT / "IMPLEMENTATION_CASCADE.md"


SECTION_RE = re.compile(r"^## (M\d+)\. (.+)$", re.M)
CHECKBOX_RE = re.compile(r"^\s*-\s*\[([ xX])\]\s+(.*)$")
BACKTICK_RE = re.compile(r"`([^`]+)`")


KEYWORD_RULES = (
    {
        "tokens": ("lexer", "token"),
        "maps": ("compiler/frontend/lexer/LEXER_MAP.md",),
        "files": (),
        "tests": ("tests/frontend/test_lexer.c",),
    },
    {
        "tokens": ("parser", "syntax", "match", "interpol", "param where"),
        "maps": ("compiler/frontend/parser/PARSER_MAP.md", "compiler/frontend/ast/AST_MAP.md"),
        "files": (
            "compiler/frontend/parser/parser.c",
            "compiler/frontend/parser/parser.h",
            "compiler/frontend/ast/model.c",
            "compiler/frontend/ast/model.h",
        ),
        "tests": ("tests/frontend/test_parser.c", "tests/frontend/test_parser_error_recovery.c"),
    },
    {
        "tokens": ("ast",),
        "maps": ("compiler/frontend/ast/AST_MAP.md",),
        "files": (),
        "tests": (),
    },
    {
        "tokens": ("binder", "scope", "namespace", "import", "shadow", "public"),
        "maps": ("compiler/semantic/binder/BINDER_MAP.md", "compiler/semantic/symbols/SYMBOLS_MAP.md", "compiler/project/PROJECT_MAP.md"),
        "files": (
            "compiler/semantic/binder/binder.c",
            "compiler/semantic/binder/binder.h",
            "compiler/semantic/symbols/symbols.c",
            "compiler/semantic/symbols/symbols.h",
        ),
        "tests": ("tests/semantic/test_binder.c",),
    },
    {
        "tokens": ("diagnostic", "renderer", "action", "why", "next", "beginner", "balanced", "full", "parameter_validation"),
        "maps": ("compiler/semantic/diagnostics/DIAGNOSTICS_MAP.md", "compiler/driver/DRIVER_MAP.md"),
        "files": (
            "compiler/semantic/diagnostics/diagnostics.c",
            "compiler/semantic/diagnostics/diagnostics.h",
            "compiler/semantic/parameter_validation.c",
            "compiler/driver/main.c",
        ),
        "tests": ("tests/test_diagnostics.zt",),
    },
    {
        "tokens": ("type", "checker", "optional", "result", "trait", "where", "default"),
        "maps": ("compiler/semantic/types/TYPE_SYSTEM_MAP.md", "compiler/semantic/binder/BINDER_MAP.md"),
        "files": (
            "compiler/semantic/types/checker.c",
            "compiler/semantic/types/checker.h",
            "compiler/semantic/types/types.c",
            "compiler/semantic/types/types.h",
            "compiler/semantic/parameter_validation.c",
        ),
        "tests": ("tests/semantic/test_types.c", "tests/semantic/test_constraints.c"),
    },
    {
        "tokens": ("hir", "desugar"),
        "maps": ("compiler/hir/lowering/HIR_LOWERING_MAP.md",),
        "files": ("compiler/hir/nodes/model.h", "compiler/hir/nodes/model.c", "compiler/zir/lowering/from_hir.h", "compiler/zir/lowering/from_hir.c"),
        "tests": ("tests/semantic/test_hir_lowering.c",),
    },
    {
        "tokens": ("zir", "lowering", "verifier"),
        "maps": ("compiler/zir/ZIR_MODEL_MAP.md", "compiler/zir/ZIR_PARSER_MAP.md", "compiler/zir/ZIR_VERIFIER_MAP.md"),
        "files": ("compiler/zir/lowering/from_hir.h", "compiler/zir/lowering/from_hir.c"),
        "tests": ("tests/zir/test_lowering.c", "tests/zir/test_printer.c", "tests/zir/test_verifier.c"),
    },
    {
        "tokens": ("backend", "emitter", "legalization", "mangling", "ffi", "extern c"),
        "maps": ("compiler/targets/c/EMITTER_MAP.md",),
        "files": (
            "compiler/targets/c/emitter.c",
            "compiler/targets/c/emitter.h",
            "compiler/targets/c/legalization.c",
            "compiler/targets/c/legalization.h",
        ),
        "tests": ("tests/targets/c/test_emitter.c", "tests/targets/c/test_legalization.c"),
    },
    {
        "tokens": ("driver", "cli", "summary", "resume", "focus", "since", "ztproj", "manifest", "project", "zdoc"),
        "maps": ("compiler/driver/DRIVER_MAP.md", "compiler/project/PROJECT_MAP.md"),
        "files": (
            "compiler/driver/main.c",
            "compiler/project/ztproj.c",
            "compiler/project/ztproj.h",
            "compiler/project/zdoc.c",
            "compiler/project/zdoc.h",
        ),
        "tests": ("tests/driver/test_project.c", "tests/driver/test_zdoc.c"),
    },
    {
        "tokens": ("formatter", "fmt"),
        "maps": ("compiler/tooling/FORMATTER_MAP.md",),
        "files": ("compiler/tooling/formatter.c", "compiler/tooling/formatter.h"),
        "tests": ("tests/formatter/run_formatter_golden.py",),
    },
    {
        "tokens": ("runtime", "ownership", "arc", "cow", "contract", "shared", "thread", "concurrency"),
        "maps": ("runtime/c/RUNTIME_MAP.md",),
        "files": (),
        "tests": ("tests/runtime/c/test_runtime.c", "tests/runtime/c/test_shared_text.c", "tests/runtime/test_fase11_safety.zt"),
    },
    {
        "tokens": ("stdlib", "bytes", "utf-8", "utf8", "collections", "json", "math", "random", "validate", "time", "os", "net", "test harness"),
        "maps": ("stdlib/STDLIB_MAP.md", "runtime/c/RUNTIME_MAP.md"),
        "files": (),
        "tests": ("tests/stdlib/*.zt", "tests/behavior/std_*/*"),
    },
    {
        "tokens": ("performance", "benchmark", "budget", "nightly", "perf"),
        "maps": (),
        "files": ("tests/perf/run_perf.py", "tests/perf/README.md"),
        "tests": ("tests/perf/*",),
    },
    {
        "tokens": ("cognitive accessibility", "accessibility"),
        "maps": ("compiler/driver/DRIVER_MAP.md", "compiler/semantic/diagnostics/DIAGNOSTICS_MAP.md", "compiler/utils/UTILS_MAP.md"),
        "files": ("language/spec/cognitive-accessibility.md",),
        "tests": (),
    },
)


@dataclass(frozen=True)
class Section:
    milestone: str
    title: str
    start_line: int
    end_line: int
    body: str


@dataclass(frozen=True)
class MilestoneContext:
    milestone: str
    title: str
    roadmap: Section
    checklist: Section


def parse_sections(path: Path) -> dict[str, Section]:
    text = path.read_text(encoding="utf-8")
    lines = text.splitlines()
    milestone_positions: list[tuple[int, str, str]] = []
    all_h2_lines: list[int] = []

    for index, line in enumerate(lines, start=1):
        if line.startswith("## "):
            all_h2_lines.append(index)
        match = re.match(r"^## (M\d+)\. (.+)$", line)
        if match is None:
            continue
        milestone_positions.append((index, match.group(1), match.group(2).strip()))

    sections: dict[str, Section] = {}
    for start_line, milestone, title in milestone_positions:
        next_candidates = [line_no for line_no in all_h2_lines if line_no > start_line]
        next_start = next_candidates[0] if next_candidates else len(lines) + 1
        end_line = next_start - 1
        body = "\n".join(lines[start_line:end_line])
        sections[milestone] = Section(
            milestone=milestone,
            title=title,
            start_line=start_line,
            end_line=end_line,
            body=body,
        )
    return sections


def section_field(section: Section, name: str) -> str | None:
    pattern = re.compile(rf"(?m)^{re.escape(name)}:\s*(.+)$")
    match = pattern.search(section.body)
    if match is None:
        return None
    return match.group(1).strip()


def section_list(section: Section, name: str) -> list[str]:
    lines = section.body.splitlines()
    captured: list[str] = []
    label = f"{name}:"
    active = False

    for line in lines:
        stripped = line.strip()
        if not active:
            if stripped.startswith(label):
                active = True
                tail = stripped[len(label):].strip()
                if tail:
                    captured.append(tail)
            continue

        if not stripped:
            if captured:
                break
            continue

        if re.match(r"^[A-ZA-Za-z0-9_][^:]{0,40}:$", stripped):
            break

        if stripped.startswith("- ") or stripped.startswith("* ") or stripped.startswith("1."):
            captured.append(stripped)
            continue

        if captured:
            captured.append(stripped)
            continue

    return captured


def extract_existing_paths(text: str) -> list[str]:
    discovered: list[str] = []
    for match in BACKTICK_RE.finditer(text):
        candidate = match.group(1).strip()
        if "/" not in candidate and "\\" not in candidate:
            continue
        normalized_candidate = candidate.replace("\\", "/")
        full_path = ROOT / normalized_candidate
        if full_path.exists():
            discovered.append(normalize(full_path.resolve()))
    return sorted(dict.fromkeys(discovered))


def parse_checklist_status(section: Section) -> tuple[list[str], list[str]]:
    done: list[str] = []
    open_items: list[str] = []
    for line in section.body.splitlines():
        match = CHECKBOX_RE.match(line)
        if match is None:
            continue
        text = match.group(2).strip()
        if match.group(1).lower() == "x":
            done.append(text)
        else:
            open_items.append(text)
    return done, open_items


def build_file_map() -> dict[str, str]:
    mapping: dict[str, str] = {}
    for spec in MODULE_SPECS:
        for pattern in spec.source_globs:
            for path in sorted(ROOT.glob(pattern)):
                if not path.is_file():
                    continue
                mapping[normalize(path.resolve())] = spec.map_path
    mapping["compiler/semantic/parameter_validation.c"] = "compiler/semantic/diagnostics/DIAGNOSTICS_MAP.md"
    return mapping


def token_blob(*chunks: str) -> str:
    return " ".join(chunks).lower()


def heuristic_matches(blob: str, tokens: Iterable[str]) -> bool:
    raw_blob = blob.lower()
    normalized_blob = f" {re.sub(r'[^a-z0-9_]+', ' ', raw_blob)} "
    for token in tokens:
        lowered = token.lower()
        if " " in lowered:
            if lowered in raw_blob:
                return True
            continue
        if "_" in lowered:
            if lowered in raw_blob:
                return True
            continue
        if f" {lowered} " in normalized_blob:
            return True
    return False


def expand_test_patterns(patterns: Iterable[str]) -> list[str]:
    matched: list[str] = []
    for pattern in patterns:
        for path in sorted(ROOT.glob(pattern)):
            if path.is_file():
                matched.append(normalize(path.resolve()))
    return sorted(dict.fromkeys(matched))


def relevant_maps_and_files(roadmap: Section, checklist: Section, file_map: dict[str, str]) -> tuple[list[str], list[str], list[str], list[str]]:
    direct_paths = extract_existing_paths(roadmap.body) + extract_existing_paths(checklist.body)
    blob = token_blob(roadmap.title, roadmap.body, checklist.body)

    candidate_files = list(direct_paths)
    candidate_maps = [file_map[path] for path in direct_paths if path in file_map]
    candidate_tests = [path for path in direct_paths if path.startswith("tests/")]
    candidate_docs = [path for path in direct_paths if path.startswith("language/")]

    for rule in KEYWORD_RULES:
        if not heuristic_matches(blob, rule["tokens"]):
            continue
        candidate_maps.extend(rule["maps"])
        candidate_files.extend(rule["files"])
        candidate_tests.extend(expand_test_patterns(rule["tests"]))

    files = []
    for path in candidate_files:
        full_path = ROOT / path
        if not full_path.exists():
            continue
        files.append(normalize(full_path.resolve()))

    maps = []
    for map_path in candidate_maps:
        full_path = ROOT / map_path
        if full_path.exists():
            maps.append(normalize(full_path.resolve()))

    for path in files:
        if path in file_map:
            map_path = file_map[path]
            full_path = ROOT / map_path
            if full_path.exists():
                maps.append(normalize(full_path.resolve()))

    doc_paths = []
    for path in candidate_docs:
        full_path = ROOT / path
        if full_path.exists():
            doc_paths.append(normalize(full_path.resolve()))

    source_files = []
    for path in files:
        if path.startswith("tests/") or path.endswith(".md"):
            continue
        source_files.append(path)

    return (
        sorted(dict.fromkeys(maps)),
        sorted(dict.fromkeys(source_files)),
        sorted(dict.fromkeys(candidate_tests)),
        sorted(dict.fromkeys(doc_paths)),
    )


def choose_symbol_preview(path: str, milestone_text: str) -> list[str]:
    summary = summarize_file(ROOT / path)
    symbols = [
        symbol
        for symbol in summary.symbols
        if symbol.kind not in {"macro", "func_decl", "doc_target", "namespace"}
    ]
    if not symbols:
        symbols = summary.symbols
    if not symbols:
        return []

    keywords = [token for token in re.findall(r"[a-z_]{4,}", milestone_text.lower()) if token not in {"status", "objetivo", "escopo", "dependencias"}]
    ranked: list[str] = []
    selected: list[tuple[int, str, str]] = []

    for symbol in symbols:
        if any(keyword in symbol.name.lower() for keyword in keywords):
            selected.append((symbol.line, symbol.kind, symbol.name))

    for symbol in symbols[:8]:
        selected.append((symbol.line, symbol.kind, symbol.name))

    seen: set[tuple[int, str]] = set()
    for line, kind, name in selected:
        key = (line, name)
        if key in seen:
            continue
        seen.add(key)
        ranked.append(f"`{name}` ({kind}, line {line})")
        if len(ranked) >= 8:
            break
    return ranked


def roadmap_summary(section: Section) -> list[str]:
    summary: list[str] = []
    status = section_field(section, "Status")
    objective = section_field(section, "Objetivo")
    dependencies = section_field(section, "Dependencias")
    parallel = section_list(section, "Pode rodar em paralelo")

    if status:
        summary.append(f"- Status roadmap: {status}")
    if objective:
        summary.append(f"- Objetivo: {objective}")
    if dependencies:
        summary.append(f"- Dependencias: {dependencies}")
    if parallel:
        summary.append("- Paralelo possivel:")
        for item in parallel:
            summary.append(f"  - {item.lstrip('- ').strip()}")
    return summary


def build_document(contexts: list[MilestoneContext]) -> str:
    file_map = build_file_map()
    lines = [
        "# Implementation Cascade",
        "",
        "Este arquivo liga `IMPLEMENTATION_CHECKLIST.md`, `IMPLEMENTATION_ROADMAP.md` e os `*_MAP.md`.",
        "",
        "Uso recomendado:",
        "",
        "1. Ache o milestone no checklist ou no roadmap.",
        "2. Abra a secao correspondente aqui.",
        "3. Siga a ordem checklist -> roadmap -> mapas -> codigo -> testes.",
        "4. Use os simbolos e linhas sugeridos como janela inicial, nao como verdade absoluta.",
        "",
        "Regra pratica:",
        "",
        "- checklist = item exato e status",
        "- roadmap = objetivo e escopo",
        "- code maps = arquivo, simbolo e linha de entrada",
        "",
    ]

    for context in contexts:
        done_items, open_items = parse_checklist_status(context.checklist)
        maps, source_files, tests, docs = relevant_maps_and_files(context.roadmap, context.checklist, file_map)
        milestone_blob = token_blob(context.title, context.roadmap.body, context.checklist.body)

        lines.extend(
            [
                f"## {context.milestone}. {context.title}",
                "",
                "### Cascade Order",
                "",
                f"1. Checklist: `IMPLEMENTATION_CHECKLIST.md` lines {context.checklist.start_line}-{context.checklist.end_line}",
                f"2. Roadmap: `IMPLEMENTATION_ROADMAP.md` lines {context.roadmap.start_line}-{context.roadmap.end_line}",
            ]
        )

        if maps:
            for index, map_path in enumerate(maps, start=3):
                lines.append(f"{index}. Map: `{map_path}`")
        else:
            lines.append("3. Map: no dedicated code map matched this milestone yet")

        lines.extend(
            [
                "",
                "### Checklist Status",
                "",
                f"- Done items: {len(done_items)}",
                f"- Open items: {len(open_items)}",
            ]
        )

        if open_items:
            lines.append("- Pending work:")
            for item in open_items:
                lines.append(f"  - {item}")

        lines.extend(
            [
                "",
                "### Roadmap Summary",
                "",
            ]
        )
        roadmap_bits = roadmap_summary(context.roadmap)
        if roadmap_bits:
            lines.extend(roadmap_bits)
        else:
            lines.append("- No structured roadmap fields found beyond the section text.")

        lines.extend(
            [
                "",
                "### Entry Files",
                "",
            ]
        )

        if source_files:
            for path in source_files:
                preview = choose_symbol_preview(path, milestone_blob)
                map_path = file_map.get(path, "-")
                lines.append(f"- `{path}`")
                lines.append(f"  - Closest map: `{map_path}`")
                if preview:
                    lines.append(f"  - Start near: {', '.join(preview)}")
                else:
                    lines.append("  - Start near: no extracted symbols")
        else:
            lines.append("- No explicit source files were detected. Use the maps and tests as the first anchors.")

        lines.extend(
            [
                "",
                "### Tests And Docs",
                "",
            ]
        )

        if tests:
            lines.append("- Tests:")
            for path in tests[:16]:
                lines.append(f"  - `{path}`")
            if len(tests) > 16:
                lines.append(f"  - ... +{len(tests) - 16} more")

        if docs:
            lines.append("- Supporting docs:")
            for path in docs:
                lines.append(f"  - `{path}`")

        if not tests and not docs:
            lines.append("- No extra tests/docs were inferred.")

        lines.extend(
            [
                "",
                "### Prompt Seed",
                "",
                "```text",
                f"Implement {context.milestone} ({context.title}).",
                f"Start from IMPLEMENTATION_CHECKLIST.md lines {context.checklist.start_line}-{context.checklist.end_line}.",
                f"Then read IMPLEMENTATION_ROADMAP.md lines {context.roadmap.start_line}-{context.roadmap.end_line}.",
            ]
        )

        if maps:
            lines.append("Then open these maps:")
            for path in maps:
                lines.append(f"- {path}")
        if source_files:
            lines.append("Then inspect these source files first:")
            for path in source_files[:6]:
                lines.append(f"- {path}")
        if tests:
            lines.append("Validate with these tests first:")
            for path in tests[:6]:
                lines.append(f"- {path}")
        lines.extend(
            [
                "Return only: files to edit, first symbols to inspect, validation plan, and main regression risk.",
                "```",
                "",
            ]
        )

    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate milestone cascade documentation.")
    parser.add_argument("--match", help="Only include milestones whose id or title contains this text.")
    args = parser.parse_args()

    roadmap_sections = parse_sections(ROADMAP_PATH)
    checklist_sections = parse_sections(CHECKLIST_PATH)
    common_ids = sorted(set(roadmap_sections) & set(checklist_sections), key=lambda value: int(value[1:]))

    contexts: list[MilestoneContext] = []
    for milestone in common_ids:
        roadmap = roadmap_sections[milestone]
        checklist = checklist_sections[milestone]
        context = MilestoneContext(
            milestone=milestone,
            title=roadmap.title,
            roadmap=roadmap,
            checklist=checklist,
        )
        if args.match:
            blob = token_blob(context.milestone, context.title)
            if args.match.lower() not in blob:
                continue
        contexts.append(context)

    document = build_document(contexts)
    OUTPUT_PATH.write_text(document, encoding="utf-8", newline="\n")
    print(f"Updated {normalize(OUTPUT_PATH)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
