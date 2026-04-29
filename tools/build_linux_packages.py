#!/usr/bin/env python3
"""Build Linux packages for Zenith compiler, ZPM, and LSP using fpm."""

from __future__ import annotations

import argparse
import hashlib
import shlex
import shutil
import stat
import subprocess
from pathlib import Path

DEFAULT_VERSION = "0.4.1-alpha.1"
DEFAULT_OUTPUT_DIR = "dist/linux"
DEFAULT_STAGE_DIR = ".artifacts/linux/stage"
DEFAULT_MAINTAINER = "Zenith Team <maintainers@zenithlang.dev>"


def _repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def _resolve_repo_path(repo_root: Path, value: str) -> Path:
    path = Path(value)
    if path.is_absolute():
        return path
    return (repo_root / path).resolve()


def _safe_rmtree(path: Path) -> None:
    if path.exists():
        shutil.rmtree(path)


def _write_text(path: Path, content: str, mode: int | None = None) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")
    if mode is not None:
        path.chmod(mode)


def _copy_file(src: Path, dst: Path) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)


def _copy_tree(src: Path, dst: Path) -> None:
    if dst.exists():
        shutil.rmtree(dst)
    shutil.copytree(src, dst)


def _chmod_executable(path: Path) -> None:
    try:
        mode = path.stat().st_mode
        path.chmod(mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
    except OSError:
        pass


def _stage_files(args: argparse.Namespace, repo_root: Path, stage_root: Path) -> None:
    rootfs = stage_root / "rootfs"
    install_prefix = Path("usr/local")
    zenith_root = rootfs / install_prefix / "lib" / "zenith"
    zenith_bin = rootfs / install_prefix / "bin"
    profile_file = rootfs / "etc" / "profile.d" / "zenith.sh"
    doc_root = rootfs / install_prefix / "share" / "doc" / "zenith"

    if stage_root.exists():
        _safe_rmtree(stage_root)
    rootfs.mkdir(parents=True, exist_ok=True)

    zt_src = _resolve_repo_path(repo_root, args.zt_binary)
    zpm_src = _resolve_repo_path(repo_root, args.zpm_binary)
    lsp_src = _resolve_repo_path(repo_root, args.lsp_binary)
    stdlib_src = repo_root / "stdlib"
    runtime_src = repo_root / "runtime"

    required = [
        (zt_src, "compiler binary"),
        (zpm_src, "package manager binary"),
        (lsp_src, "language server binary"),
        (stdlib_src, "stdlib folder"),
        (runtime_src, "runtime folder"),
        (repo_root / "README.md", "README"),
        (repo_root / "CHANGELOG.md", "CHANGELOG"),
        (repo_root / "LICENSE", "LICENSE"),
        (repo_root / "LICENSE-APACHE", "LICENSE-APACHE"),
        (repo_root / "LICENSE-MIT", "LICENSE-MIT"),
    ]

    for path, label in required:
        if not path.exists():
            raise FileNotFoundError(f"{label} not found: {path}")

    _copy_file(zt_src, zenith_root / "zt")
    _copy_file(zpm_src, zenith_root / "zpm")
    _copy_file(lsp_src, zenith_root / "zt-lsp")
    _copy_tree(stdlib_src, zenith_root / "stdlib")
    _copy_tree(runtime_src, zenith_root / "runtime")

    _copy_file(repo_root / "README.md", doc_root / "README.md")
    _copy_file(repo_root / "CHANGELOG.md", doc_root / "CHANGELOG.md")
    _copy_file(repo_root / "LICENSE", doc_root / "LICENSE")
    _copy_file(repo_root / "LICENSE-APACHE", doc_root / "LICENSE-APACHE")
    _copy_file(repo_root / "LICENSE-MIT", doc_root / "LICENSE-MIT")

    wrapper_template = """#!/usr/bin/env sh
set -eu

if [ -z \"${{ZENITH_HOME:-}}\" ]; then
  export ZENITH_HOME=\"/usr/local/lib/zenith\"
fi

exec \"$ZENITH_HOME/{tool}\" \"$@\"
"""
    for tool in ("zt", "zpm", "zt-lsp"):
        _write_text(zenith_bin / tool, wrapper_template.format(tool=tool), 0o755)

    profile_script = """# Zenith global environment
export ZENITH_HOME=\"/usr/local/lib/zenith\"
"""
    _write_text(profile_file, profile_script, 0o644)

    _chmod_executable(zenith_root / "zt")
    _chmod_executable(zenith_root / "zpm")
    _chmod_executable(zenith_root / "zt-lsp")


def _build_commands(args: argparse.Namespace, rootfs: Path, output_dir: Path) -> list[list[str]]:
    base = [
        args.fpm,
        "-s",
        "dir",
        "-f",
        "-n",
        "zenith",
        "-v",
        args.version,
        "--iteration",
        args.iteration,
        "--license",
        "Apache-2.0 OR MIT",
        "--maintainer",
        args.maintainer,
        "--url",
        "https://github.com/raillen/zenithlang",
        "--description",
        "Zenith compiler, ZPM package manager, LSP, stdlib, and runtime",
        "-C",
        str(rootfs),
    ]

    targets = [
        (
            "deb",
            "amd64",
            output_dir / f"zenith-{args.version}-linux-amd64.deb",
            ["gcc", "libc6"],
        ),
        (
            "rpm",
            "x86_64",
            output_dir / f"zenith-{args.version}-linux-x86_64.rpm",
            ["gcc", "glibc"],
        ),
        (
            "pacman",
            "x86_64",
            output_dir / f"zenith-{args.version}-linux-x86_64.pkg.tar.zst",
            ["gcc", "glibc"],
        ),
    ]

    commands: list[list[str]] = []
    for pkg_type, arch, output_path, dependencies in targets:
        cmd = list(base)
        for dependency in dependencies:
            cmd.extend(["--depends", dependency])
        cmd.extend([
            "-t",
            pkg_type,
            "-a",
            arch,
            "-p",
            str(output_path),
            ".",
        ])
        commands.append(cmd)

    return commands


def _write_checksums(output_dir: Path) -> Path:
    files = sorted(p for p in output_dir.iterdir() if p.is_file() and p.suffix in {".deb", ".rpm", ".zst"})
    checksum_path = output_dir / "checksums.txt"
    lines: list[str] = []

    for file_path in files:
        data = file_path.read_bytes()
        sha256 = hashlib.sha256(data).hexdigest()
        sha512 = hashlib.sha512(data).hexdigest()
        lines.append(f"SHA256  {sha256}  {file_path.name}")
        lines.append(f"SHA512  {sha512}  {file_path.name}")

    checksum_path.write_text("\n".join(lines) + ("\n" if lines else ""), encoding="utf-8", newline="\n")
    return checksum_path


def _run_commands(commands: list[list[str]], dry_run: bool) -> None:
    for cmd in commands:
        print("$", " ".join(shlex.quote(part) for part in cmd))
        if not dry_run:
            subprocess.run(cmd, check=True)


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build Zenith Linux packages (.deb/.rpm/.pkg.tar.zst) via fpm.")
    parser.add_argument("--version", default=DEFAULT_VERSION, help="Package version")
    parser.add_argument("--zt-binary", "--binary", dest="zt_binary", default="zt", help="Path to compiled Linux zt binary")
    parser.add_argument("--zpm-binary", default="zpm", help="Path to compiled Linux zpm binary")
    parser.add_argument("--lsp-binary", default="zt-lsp", help="Path to compiled Linux zt-lsp binary")
    parser.add_argument("--fpm", default="fpm", help="fpm executable path")
    parser.add_argument("--output-dir", default=DEFAULT_OUTPUT_DIR, help="Output directory for packages")
    parser.add_argument("--stage-dir", default=DEFAULT_STAGE_DIR, help="Temporary staging directory")
    parser.add_argument("--maintainer", default=DEFAULT_MAINTAINER, help="Maintainer metadata")
    parser.add_argument("--iteration", default="1", help="Package iteration/revision")
    parser.add_argument("--skip-staging", action="store_true", help="Do not recreate staged rootfs")
    parser.add_argument("--dry-run", action="store_true", help="Print fpm commands only")
    parser.add_argument("--skip-checksums", action="store_true", help="Do not generate checksum file")
    return parser.parse_args()


def main() -> int:
    args = _parse_args()
    repo_root = _repo_root()
    stage_root = _resolve_repo_path(repo_root, args.stage_dir)
    output_dir = _resolve_repo_path(repo_root, args.output_dir)
    rootfs = stage_root / "rootfs"

    if not args.skip_staging:
        _stage_files(args, repo_root, stage_root)

    if not rootfs.exists():
        raise FileNotFoundError(f"Staged rootfs not found: {rootfs}")

    output_dir.mkdir(parents=True, exist_ok=True)
    commands = _build_commands(args, rootfs, output_dir)
    _run_commands(commands, args.dry_run)

    if not args.dry_run and not args.skip_checksums:
        checksum_path = _write_checksums(output_dir)
        print(f"checksums: {checksum_path}")

    print("done")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
