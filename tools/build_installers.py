#!/usr/bin/env python3
"""Build Zenith release installers/packages.

This script builds the three shipped tools:

- zt: compiler/driver
- zpm: package manager
- zt-lsp: language server

Then it delegates to the platform package builders:

- Windows: Inno Setup script at installer/zenith.iss
- Linux: fpm packages for .deb, .rpm, and Arch/pacman
"""

from __future__ import annotations

import argparse
import os
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

DEFAULT_VERSION = "0.4.1-alpha.1"
ROOT = Path(__file__).resolve().parents[1]

LSP_SOURCES = [
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


def host_exe(name: str) -> str:
    return f"{name}.exe" if os.name == "nt" else name


def print_cmd(cmd: list[str]) -> None:
    print("$", " ".join(shlex.quote(part) for part in cmd))


def run(cmd: list[str], *, dry_run: bool = False) -> None:
    print_cmd(cmd)
    if dry_run:
        return
    subprocess.run(cmd, cwd=ROOT, check=True)


def find_cc() -> str:
    env_cc = os.environ.get("CC")
    if env_cc:
        return env_cc
    for candidate in ("gcc", "clang", "cc"):
        if shutil.which(candidate):
            return candidate
    raise RuntimeError("No C compiler found. Install gcc or clang, or set CC.")


def compiler_common_sources() -> list[str]:
    excluded = {"main.c", "zpm_main.c", "lsp.c"}
    sources = [
        str(path.relative_to(ROOT)).replace("\\", "/")
        for path in (ROOT / "compiler").rglob("*.c")
        if path.name not in excluded
    ]
    if not sources:
        raise RuntimeError("No compiler C sources found.")
    return sorted(sources)


def assert_sources_exist(paths: list[str]) -> None:
    missing = [path for path in paths if not (ROOT / path).exists()]
    if missing:
        raise FileNotFoundError("Missing source files: " + ", ".join(missing))


def build_binaries(dry_run: bool = False) -> None:
    cc = find_cc()
    common = compiler_common_sources()
    assert_sources_exist(common)
    assert_sources_exist(LSP_SOURCES)

    builds = [
        (host_exe("zt"), "compiler/driver/main.c", common),
        (host_exe("zpm"), "compiler/driver/zpm_main.c", common),
        (host_exe("zt-lsp"), "compiler/driver/lsp.c", LSP_SOURCES[1:]),
    ]

    for output, entrypoint, extra_sources in builds:
        cmd = [cc, "-O0", "-Wall", "-Wextra", "-I.", "-o", output, entrypoint] + extra_sources
        run(cmd, dry_run=dry_run)


def ensure_host(target: str) -> None:
    if target == "windows" and os.name != "nt":
        raise RuntimeError("Windows ISS installer must be built on Windows with Inno Setup installed.")
    if target == "linux" and os.name == "nt":
        raise RuntimeError("Linux packages must be built on Linux or WSL with Linux binaries.")


def build_windows_installer(args: argparse.Namespace) -> None:
    ensure_host("windows")
    script = ROOT / "tools" / "build_installer.ps1"
    cmd = [
        "powershell",
        "-ExecutionPolicy",
        "Bypass",
        "-File",
        str(script),
        "-Version",
        args.version,
        "-IsccPath",
        args.iscc,
        "-OutputDir",
        str(Path(args.output_root) / "installer"),
    ]
    run(cmd, dry_run=args.dry_run)


def build_linux_packages(args: argparse.Namespace) -> None:
    ensure_host("linux")
    script = ROOT / "tools" / "build_linux_packages.py"
    cmd = [
        sys.executable,
        str(script),
        "--version",
        args.version,
        "--zt-binary",
        host_exe("zt"),
        "--zpm-binary",
        host_exe("zpm"),
        "--lsp-binary",
        host_exe("zt-lsp"),
        "--fpm",
        args.fpm,
        "--output-dir",
        str(Path(args.output_root) / "linux"),
        "--iteration",
        args.iteration,
        "--maintainer",
        args.maintainer,
    ]
    if args.dry_run:
        cmd.append("--dry-run")
    run(cmd, dry_run=False)


def resolve_targets(target: str) -> list[str]:
    if target == "auto":
        return ["windows"] if os.name == "nt" else ["linux"]
    if target == "all":
        return ["windows", "linux"]
    return [target]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build Zenith installers/packages for release.")
    parser.add_argument("--version", default=DEFAULT_VERSION, help="Release/package version.")
    parser.add_argument("--target", choices=("auto", "windows", "linux", "all"), default="auto")
    parser.add_argument("--output-root", default="dist", help="Output root for generated artifacts.")
    parser.add_argument("--skip-build", action="store_true", help="Reuse existing zt/zpm/zt-lsp binaries.")
    parser.add_argument("--dry-run", action="store_true", help="Print commands without building binaries or installers.")
    parser.add_argument("--iscc", default="iscc", help="Inno Setup compiler path.")
    parser.add_argument("--fpm", default="fpm", help="fpm executable path for Linux packages.")
    parser.add_argument("--iteration", default="1", help="Linux package iteration/revision.")
    parser.add_argument("--maintainer", default="Zenith Team <maintainers@zenithlang.dev>")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    targets = resolve_targets(args.target)

    if not args.skip_build:
        build_binaries(dry_run=args.dry_run)

    for target in targets:
        if target == "windows":
            build_windows_installer(args)
        elif target == "linux":
            build_linux_packages(args)
        else:
            raise RuntimeError(f"Unknown target: {target}")

    print("done")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
