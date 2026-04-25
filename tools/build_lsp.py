import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / ("zt-lsp.exe" if os.name == "nt" else "zt-lsp")

SOURCES = [
    "compiler/driver/lsp.c",
    "compiler/frontend/ast/model.c",
    "compiler/frontend/lexer/token.c",
    "compiler/frontend/lexer/lexer.c",
    "compiler/frontend/parser/parser.c",
    "compiler/semantic/diagnostics/diagnostics.c",
    "compiler/semantic/symbols/symbols.c",
    "compiler/semantic/binder/binder.c",
    "compiler/semantic/types/types.c",
    "compiler/semantic/types/checker.c",
    "compiler/semantic/parameter_validation.c",
    "compiler/tooling/formatter.c",
    "compiler/utils/arena.c",
    "compiler/utils/string_pool.c",
    "compiler/utils/l10n.c",
]


def find_cc():
    env_cc = os.environ.get("CC")
    if env_cc:
        return env_cc
    for candidate in ["gcc", "clang", "cc"]:
        try:
            completed = subprocess.run(
                [candidate, "--version"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            if completed.returncode == 0:
                return candidate
        except FileNotFoundError:
            continue
    return None


def main():
    cc = find_cc()
    if cc is None:
        print("FAIL")
        print("No C compiler found. Install gcc or clang, or set CC.")
        return 1

    missing = [src for src in SOURCES if not (ROOT / src).exists()]
    if missing:
        print("FAIL")
        print("Missing LSP source files:")
        for src in missing:
            print(f"  - {src}")
        return 1

    cmd = [cc, "-O0", "-Wall", "-Wextra", "-I.", "-o", str(OUT)] + SOURCES
    print("Building LSP with:", " ".join(cmd))
    completed = subprocess.run(cmd, cwd=ROOT)
    print("Exit code:", completed.returncode)
    if completed.returncode == 0:
        print("SUCCESS")
    else:
        print("FAIL")
    return completed.returncode


if __name__ == "__main__":
    sys.exit(main())
