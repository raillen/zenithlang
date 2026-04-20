from __future__ import annotations

import argparse
import bisect
import datetime as dt
import fnmatch
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


ROOT = Path(__file__).resolve().parent.parent
GENERATED_BEGIN = "<!-- CODEMAP:GENERATED:BEGIN -->"
GENERATED_END = "<!-- CODEMAP:GENERATED:END -->"


@dataclass(frozen=True)
class Symbol:
    line: int
    kind: str
    name: str


@dataclass(frozen=True)
class FileSummary:
    path: str
    line_count: int
    dependencies: list[str]
    symbols: list[Symbol]


@dataclass(frozen=True)
class ModuleSpec:
    map_path: str
    title: str
    summary: str
    priority: str
    source_globs: tuple[str, ...]
    related_tests_globs: tuple[str, ...] = ()


MODULE_SPECS: tuple[ModuleSpec, ...] = (
    ModuleSpec(
        map_path="compiler/driver/DRIVER_MAP.md",
        title="Driver / CLI Code Map",
        summary="Entry points, CLI commands, orchestration, and documentation commands.",
        priority="critical",
        source_globs=("compiler/driver/*.c", "compiler/driver/*.h"),
        related_tests_globs=("tests/driver/*",),
    ),
    ModuleSpec(
        map_path="compiler/frontend/lexer/LEXER_MAP.md",
        title="Lexer Code Map",
        summary="Tokenization, source spans, keywords, and literal scanning.",
        priority="critical",
        source_globs=("compiler/frontend/lexer/*.c", "compiler/frontend/lexer/*.h"),
        related_tests_globs=("tests/frontend/test_lexer.c",),
    ),
    ModuleSpec(
        map_path="compiler/frontend/parser/PARSER_MAP.md",
        title="Parser Code Map",
        summary="AST construction, parse recovery, and user-facing syntax entry points.",
        priority="critical",
        source_globs=("compiler/frontend/parser/*.c", "compiler/frontend/parser/*.h"),
        related_tests_globs=("tests/frontend/test_parser.c", "tests/frontend/test_parser_error_recovery.c"),
    ),
    ModuleSpec(
        map_path="compiler/frontend/ast/AST_MAP.md",
        title="AST Code Map",
        summary="AST node structures, allocation helpers, and source model utilities.",
        priority="medium",
        source_globs=("compiler/frontend/ast/*.c", "compiler/frontend/ast/*.h"),
        related_tests_globs=("tests/frontend/test_parser.c", "tests/frontend/test_parser_error_recovery.c"),
    ),
    ModuleSpec(
        map_path="compiler/zir/ZIR_MODEL_MAP.md",
        title="ZIR Model Code Map",
        summary="ZIR data structures, instruction model, and serialization-facing helpers.",
        priority="critical",
        source_globs=("compiler/zir/model.c", "compiler/zir/model.h"),
        related_tests_globs=("tests/zir/test_printer.c", "tests/zir/test_lowering.c", "tests/zir/test_enum_lowering.c"),
    ),
    ModuleSpec(
        map_path="compiler/zir/ZIR_PARSER_MAP.md",
        title="ZIR Parser Code Map",
        summary="Parsing of textual ZIR fixtures into the internal ZIR model.",
        priority="medium",
        source_globs=("compiler/zir/parser.c", "compiler/zir/parser.h"),
        related_tests_globs=("tests/zir/test_lowering.c", "tests/zir/test_enum_lowering.c"),
    ),
    ModuleSpec(
        map_path="compiler/zir/ZIR_VERIFIER_MAP.md",
        title="ZIR Verifier Code Map",
        summary="Validation of labels, symbol definitions, and instruction-level consistency in ZIR.",
        priority="medium",
        source_globs=("compiler/zir/verifier.c", "compiler/zir/verifier.h"),
        related_tests_globs=("tests/zir/test_verifier.c",),
    ),
    ModuleSpec(
        map_path="compiler/hir/lowering/HIR_LOWERING_MAP.md",
        title="HIR Lowering Code Map",
        summary="Lowering from AST into HIR symbols, declarations, and executable forms.",
        priority="medium",
        source_globs=("compiler/hir/lowering/*.c", "compiler/hir/lowering/*.h"),
        related_tests_globs=("tests/semantic/test_hir_lowering.c", "tests/semantic/test_binder.c"),
    ),
    ModuleSpec(
        map_path="compiler/semantic/binder/BINDER_MAP.md",
        title="Binder Code Map",
        summary="Scope creation, symbol declaration, and name resolution across AST nodes.",
        priority="critical",
        source_globs=("compiler/semantic/binder/*.c", "compiler/semantic/binder/*.h"),
        related_tests_globs=("tests/semantic/test_binder.c", "tests/behavior/multifile_*", "tests/fixtures/diagnostics/multifile_*.contains.txt"),
    ),
    ModuleSpec(
        map_path="compiler/semantic/types/TYPE_SYSTEM_MAP.md",
        title="Type System Code Map",
        summary="Type representation, type checking, and constraint validation.",
        priority="critical",
        source_globs=("compiler/semantic/types/*.c", "compiler/semantic/types/*.h"),
        related_tests_globs=("tests/semantic/test_constraints.c", "tests/core/test_types_pro.zt", "tests/semantic_tests/test_types_advanced.zt"),
    ),
    ModuleSpec(
        map_path="compiler/semantic/symbols/SYMBOLS_MAP.md",
        title="Symbols Code Map",
        summary="Symbol kinds, scope lookup, and declaration storage.",
        priority="medium",
        source_globs=("compiler/semantic/symbols/*.c", "compiler/semantic/symbols/*.h"),
        related_tests_globs=("tests/semantic/test_binder.c", "tests/behavior/multifile_*"),
    ),
    ModuleSpec(
        map_path="compiler/semantic/diagnostics/DIAGNOSTICS_MAP.md",
        title="Diagnostics Code Map",
        summary="Semantic diagnostic metadata, user messages, and fix guidance.",
        priority="low",
        source_globs=("compiler/semantic/diagnostics/*.c", "compiler/semantic/diagnostics/*.h", "compiler/semantic/parameter_validation.c"),
        related_tests_globs=("tests/test_diagnostics.zt", "tests/fixtures/diagnostics/*"),
    ),
    ModuleSpec(
        map_path="compiler/targets/c/EMITTER_MAP.md",
        title="C Emitter Code Map",
        summary="C backend emission, symbol mangling, and C legalization handoff.",
        priority="critical",
        source_globs=("compiler/targets/c/*.c", "compiler/targets/c/*.h"),
        related_tests_globs=("tests/targets/c/*", "tests/conformance/test_m*.c"),
    ),
    ModuleSpec(
        map_path="compiler/project/PROJECT_MAP.md",
        title="Project / ZDoc Code Map",
        summary="Manifest parsing, source discovery, and ZDoc validation/generation.",
        priority="medium",
        source_globs=("compiler/project/*.c", "compiler/project/*.h"),
        related_tests_globs=("tests/driver/*", "tests/behavior/project_unknown_key_manifest/*"),
    ),
    ModuleSpec(
        map_path="compiler/tooling/FORMATTER_MAP.md",
        title="Formatter Code Map",
        summary="Formatting rules, canonical whitespace, and tooling-facing output helpers.",
        priority="low",
        source_globs=("compiler/tooling/*.c", "compiler/tooling/*.h"),
        related_tests_globs=("tests/formatter/*",),
    ),
    ModuleSpec(
        map_path="compiler/utils/UTILS_MAP.md",
        title="Utils Code Map",
        summary="Memory helpers, string pooling, shared diagnostics primitives, and localization.",
        priority="low",
        source_globs=("compiler/utils/*.c", "compiler/utils/*.h"),
        related_tests_globs=("tests/frontend/*", "tests/semantic/*", "tests/zir/*"),
    ),
    ModuleSpec(
        map_path="runtime/c/RUNTIME_MAP.md",
        title="Runtime Code Map",
        summary="Managed values, refcount/COW behavior, host API adapters, and runtime diagnostics.",
        priority="critical",
        source_globs=("runtime/c/*.c", "runtime/c/*.h"),
        related_tests_globs=("tests/runtime/c/*", "tests/runtime/*", "tests/behavior/where_contract*/*"),
    ),
    ModuleSpec(
        map_path="stdlib/STDLIB_MAP.md",
        title="Stdlib Code Map",
        summary="Public Zenith modules, extern bindings, and paired ZDoc targets.",
        priority="medium",
        source_globs=("stdlib/std/**/*.zt", "stdlib/zdoc/std/**/*.zdoc"),
        related_tests_globs=("tests/behavior/std_*/*", "tests/stdlib/*"),
    ),
)


PRIORITY_LABELS = {
    "critical": "Critical",
    "medium": "Medium",
    "low": "Low",
}


CONTROL_KEYWORDS = {
    "if",
    "for",
    "while",
    "switch",
    "return",
    "sizeof",
}

TEXT_TEST_EXTENSIONS = {
    ".c",
    ".h",
    ".md",
    ".ps1",
    ".py",
    ".txt",
    ".zt",
}


ZT_SYMBOL_PATTERNS: tuple[tuple[str, re.Pattern[str]], ...] = (
    ("namespace", re.compile(r"^\s*namespace\s+([A-Za-z_][\w.]*)")),
    ("const", re.compile(r"^\s*(?:public\s+)?const\s+([A-Za-z_]\w*)\b")),
    ("func", re.compile(r"^\s*(?:public\s+)?func\s+([A-Za-z_]\w*)\b")),
    ("struct", re.compile(r"^\s*(?:public\s+)?struct\s+([A-Za-z_]\w*)\b")),
    ("enum", re.compile(r"^\s*(?:public\s+)?enum\s+([A-Za-z_]\w*)\b")),
    ("trait", re.compile(r"^\s*(?:public\s+)?trait\s+([A-Za-z_]\w*)\b")),
    ("apply", re.compile(r"^\s*apply\s+(.+?)\s*$")),
)


def normalize(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def iter_unique_files(patterns: Iterable[str]) -> list[Path]:
    seen: set[Path] = set()
    files: list[Path] = []
    for pattern in patterns:
        for path in ROOT.glob(pattern):
            if not path.is_file():
                continue
            if path.name.endswith("_MAP.md") or path.name == "CODE_MAP.md":
                continue
            if path.suffix.lower() not in {".c", ".h", ".zt", ".zdoc"}:
                continue
            resolved = path.resolve()
            if resolved in seen:
                continue
            seen.add(resolved)
            files.append(resolved)
    return sorted(files, key=lambda item: normalize(item))


def preserve_layout(match: re.Match[str]) -> str:
    return "".join("\n" if char == "\n" else " " for char in match.group(0))


def strip_c_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", preserve_layout, text, flags=re.S)
    text = re.sub(r"//[^\n]*", preserve_layout, text)
    return text


def line_lookup(text: str, offset: int) -> int:
    starts = [0]
    for index, char in enumerate(text):
        if char == "\n":
            starts.append(index + 1)
    return bisect.bisect_right(starts, offset)


def build_brace_depth_map(text: str) -> list[int]:
    depths = [0] * (len(text) + 1)
    depth = 0
    for index, char in enumerate(text):
        depths[index] = depth
        if char == "{":
            depth += 1
        elif char == "}":
            depth = max(0, depth - 1)
    depths[len(text)] = depth
    return depths


def extract_c_symbols(text: str) -> list[Symbol]:
    clean = strip_c_comments(text)
    depths = build_brace_depth_map(clean)
    symbols: list[Symbol] = []

    for match in re.finditer(r"(?m)^[ \t]*#define[ \t]+([A-Za-z_]\w*)\b", clean):
        if depths[match.start()] == 0:
            symbols.append(Symbol(line_lookup(clean, match.start()), "macro", match.group(1)))

    typedef_re = re.compile(
        r"(?ms)^[ \t]*typedef[ \t]+(struct|enum|union)\b(?:[ \t]+([A-Za-z_]\w*))?[ \t]*\{.*?^[ \t]*\}[ \t]*([A-Za-z_]\w*)[ \t]*;"
    )
    for match in typedef_re.finditer(clean):
        if depths[match.start()] != 0:
            continue
        kind = match.group(1)
        name = match.group(3)
        symbols.append(Symbol(line_lookup(clean, match.start()), kind, name))

    func_re = re.compile(
        r"""(?ms)
        ^[ \t]*
        (?P<signature>
            (?P<prefix>(?:[A-Za-z_]\w*[\w\s\*\(\),\[\]]*?))
            \b(?P<name>[A-Za-z_]\w*)
            [ \t]*\(
                (?P<params>(?:[^(){};]|\([^()]*\)|\n)*?)
            \)
            [ \t]*(?P<trailer>\{|;)
        )
        """,
        re.X,
    )
    for match in func_re.finditer(clean):
        if depths[match.start()] != 0:
            continue
        name = match.group("name")
        signature = match.group("signature")
        prefix = match.group("prefix").strip()
        if name in CONTROL_KEYWORDS:
            continue
        if prefix.startswith("typedef"):
            continue
        if re.search(r"\(\s*\*\s*" + re.escape(name) + r"\s*\)", signature):
            continue
        kind = "func_def" if match.group("trailer") == "{" else "func_decl"
        symbols.append(Symbol(line_lookup(clean, match.start()), kind, name))

    return dedupe_symbols(symbols)


def extract_zt_symbols(text: str) -> list[Symbol]:
    symbols: list[Symbol] = []
    for line_no, line in enumerate(text.splitlines(), start=1):
        stripped = line.strip()
        if stripped.startswith("--"):
            continue
        for kind, pattern in ZT_SYMBOL_PATTERNS:
            match = pattern.match(line)
            if match is None:
                continue
            symbols.append(Symbol(line_no, kind, match.group(1).strip()))
            break
    return dedupe_symbols(symbols)


def extract_zdoc_symbols(text: str) -> list[Symbol]:
    symbols: list[Symbol] = []
    pattern = re.compile(r"^\s*---\s*@target:\s*(.+?)\s*$")
    for line_no, line in enumerate(text.splitlines(), start=1):
        match = pattern.match(line)
        if match is None:
            continue
        symbols.append(Symbol(line_no, "doc_target", match.group(1).strip()))
    return dedupe_symbols(symbols)


def dedupe_symbols(symbols: Iterable[Symbol]) -> list[Symbol]:
    seen: set[tuple[int, str, str]] = set()
    unique: list[Symbol] = []
    for symbol in sorted(symbols, key=lambda item: (item.line, item.kind, item.name)):
        key = (symbol.line, symbol.kind, symbol.name)
        if key in seen:
            continue
        seen.add(key)
        unique.append(symbol)
    return unique


def extract_dependencies(path: Path, text: str) -> list[str]:
    dependencies: list[str] = []
    if path.suffix.lower() in {".c", ".h"}:
        for match in re.finditer(r'(?m)^[ \t]*#include[ \t]+"([^"]+)"', text):
            dependencies.append(match.group(1))
    elif path.suffix.lower() == ".zt":
        for match in re.finditer(r"(?m)^[ \t]*import[ \t]+([A-Za-z_][\w.]*)", text):
            dependencies.append(match.group(1))
    return sorted(dict.fromkeys(dependencies))


def summarize_file(path: Path) -> FileSummary:
    text = path.read_text(encoding="utf-8")
    if path.suffix.lower() in {".c", ".h"}:
        symbols = extract_c_symbols(text)
    elif path.suffix.lower() == ".zt":
        symbols = extract_zt_symbols(text)
    elif path.suffix.lower() == ".zdoc":
        symbols = extract_zdoc_symbols(text)
    else:
        symbols = []
    dependencies = extract_dependencies(path, text)
    return FileSummary(
        path=normalize(path),
        line_count=text.count("\n") + (0 if text == "" else 1),
        dependencies=dependencies,
        symbols=symbols,
    )


def collect_tests(patterns: Iterable[str]) -> list[str]:
    matched: list[str] = []
    all_test_files = [
        normalize(path.resolve())
        for path in ROOT.glob("tests/**/*")
        if path.is_file()
        and path.suffix.lower() in TEXT_TEST_EXTENSIONS
        and "/build/" not in normalize(path.resolve())
        and "/.ztc-tmp/" not in normalize(path.resolve())
        and "__pycache__" not in normalize(path.resolve())
    ]
    for pattern in patterns:
        normalized_pattern = pattern.replace("\\", "/")
        for file_path in all_test_files:
            if fnmatch.fnmatch(file_path, normalized_pattern):
                matched.append(file_path)
    return sorted(dict.fromkeys(matched))


def module_generated_block(spec: ModuleSpec, files: list[FileSummary]) -> str:
    symbol_total = sum(len(file.symbols) for file in files)
    dependencies = sorted({dependency for file in files for dependency in file.dependencies})
    related_tests = collect_tests(spec.related_tests_globs)
    lines = [
        GENERATED_BEGIN,
        "## Generated Index",
        "",
        f"- Priority: {PRIORITY_LABELS[spec.priority]}",
        f"- Source files: {len(files)}",
        f"- Extracted symbols: {symbol_total}",
        "",
        "Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.",
        "",
        "### File Summary",
        "",
        "| File | Lines | Symbols | Local deps |",
        "| --- | ---: | ---: | ---: |",
    ]
    for file in files:
        lines.append(f"| `{file.path}` | {file.line_count} | {len(file.symbols)} | {len(file.dependencies)} |")

    lines.extend(
        [
            "",
            "### Local Dependencies",
            "",
        ]
    )
    if dependencies:
        for dependency in dependencies:
            lines.append(f"- `{dependency}`")
    else:
        lines.append("- None detected")

    lines.extend(
        [
            "",
            "### Related Tests",
            "",
        ]
    )
    if related_tests:
        for test_path in related_tests:
            lines.append(f"- `{test_path}`")
    else:
        lines.append("- Add test links here if this module gets dedicated coverage.")

    lines.extend(
        [
            "",
            "### Symbol Index",
            "",
        ]
    )
    for file in files:
        lines.append(f"#### `{file.path}`")
        lines.append("")
        lines.append("| Line | Kind | Symbol |")
        lines.append("| ---: | --- | --- |")
        if file.symbols:
            for symbol in file.symbols:
                lines.append(f"| {symbol.line} | `{symbol.kind}` | `{symbol.name}` |")
        else:
            lines.append("| - | - | No symbols extracted |")
        lines.append("")

    lines.extend(
        [
            "### Manual Notes",
            "",
            "- Critical flow:",
            "- Break conditions:",
            "- Related docs or decisions:",
            "- Extra test cases worth adding:",
            GENERATED_END,
            "",
        ]
    )
    return "\n".join(lines)


def master_generated_block(specs: Iterable[ModuleSpec], module_files: dict[str, list[FileSummary]]) -> str:
    lines = [
        GENERATED_BEGIN,
        "## Generated Module Catalog",
        "",
        "| Module | Priority | Source files | Symbols | Map |",
        "| --- | --- | ---: | ---: | --- |",
    ]
    total_symbols = 0
    total_sources = 0
    for spec in specs:
        files = module_files[spec.map_path]
        source_count = len(files)
        symbol_count = sum(len(file.symbols) for file in files)
        total_symbols += symbol_count
        total_sources += source_count
        lines.append(
            f"| {spec.title} | {PRIORITY_LABELS[spec.priority]} | {source_count} | {symbol_count} | [{Path(spec.map_path).name}]({Path(spec.map_path).relative_to('compiler').as_posix() if spec.map_path.startswith('compiler/') else '../' + spec.map_path}) |"
        )
    lines.extend(
        [
            "",
            "### Totals",
            "",
            f"- Source files indexed: {total_sources}",
            f"- Symbols indexed: {total_symbols}",
            "",
            "### Workflow",
            "",
            "1. Open the closest module map.",
            "2. Find the file and top-level symbol.",
            "3. Jump to the line in the source file.",
            "4. Add manual notes only for real risks or recurring bugs.",
            GENERATED_END,
            "",
        ]
    )
    return "\n".join(lines)


def build_module_files(spec: ModuleSpec) -> list[FileSummary]:
    return [summarize_file(path) for path in iter_unique_files(spec.source_globs)]


def upsert_generated_block(path: Path, block: str) -> tuple[bool, str]:
    if path.exists():
        current = path.read_text(encoding="utf-8")
    else:
        current = ""

    if GENERATED_BEGIN in current and GENERATED_END in current:
        updated = re.sub(
            re.escape(GENERATED_BEGIN) + r".*?" + re.escape(GENERATED_END),
            block.rstrip(),
            current,
            flags=re.S,
        )
    elif current.strip():
        updated = current.rstrip() + "\n\n" + block
    else:
        updated = block

    changed = updated != current
    return changed, updated


def filter_specs(specs: Iterable[ModuleSpec], match: str | None) -> list[ModuleSpec]:
    if not match:
        return list(specs)
    lowered = match.lower()
    return [
        spec
        for spec in specs
        if lowered in spec.map_path.lower() or lowered in spec.title.lower()
    ]


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate code map indexes for compiler, runtime, and stdlib modules.")
    parser.add_argument("--match", help="Only update modules whose title or map path contains this text.")
    parser.add_argument("--check", action="store_true", help="Do not write files. Exit non-zero if any file would change.")
    args = parser.parse_args()

    selected_specs = filter_specs(MODULE_SPECS, args.match)
    if not selected_specs:
        print("No matching code map specs found.", file=sys.stderr)
        return 1

    module_files = {spec.map_path: build_module_files(spec) for spec in selected_specs}
    pending_writes: list[tuple[Path, str]] = []

    for spec in selected_specs:
        block = module_generated_block(spec, module_files[spec.map_path])
        path = ROOT / spec.map_path
        changed, updated = upsert_generated_block(path, block)
        if changed:
            pending_writes.append((path, updated))

    if args.match is None:
        master_path = ROOT / "compiler/CODE_MAP.md"
        master_block = master_generated_block(MODULE_SPECS, module_files)
        changed, updated = upsert_generated_block(master_path, master_block)
        if changed:
            pending_writes.append((master_path, updated))

    if args.check:
        if pending_writes:
            for path, _ in pending_writes:
                print(f"Would update {normalize(path)}")
            return 1
        return 0

    for path, updated in pending_writes:
        path.write_text(updated, encoding="utf-8", newline="\n")
        print(f"Updated {normalize(path)}")

    if not pending_writes:
        print("Code maps are already up to date.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
