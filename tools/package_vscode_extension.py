import json
import os
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
EXTENSION_DIR = ROOT / "tools" / "vscode-zenith"
DIST_DIR = ROOT / "dist"


def fail(message: str) -> int:
    print("FAIL")
    print(message)
    return 1


def copy_tree(src: Path, dst: Path) -> None:
    if dst.exists():
        shutil.rmtree(dst)
    shutil.copytree(src, dst)


def copy_file(src: Path, dst: Path) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)


def find_command(name: str) -> str | None:
    if os.name == "nt":
        for candidate in (f"{name}.cmd", f"{name}.ps1", f"{name}.exe", name):
            found = shutil.which(candidate)
            if found:
                return found
    return shutil.which(name)


def prepare_payload() -> int:
    bin_dir = EXTENSION_DIR / "bin"
    bin_dir.mkdir(parents=True, exist_ok=True)

    tool_names = ["zt", "zt-lsp"]
    if os.name == "nt":
        tool_names = [f"{name}.exe" for name in tool_names]

    missing = [name for name in tool_names if not (ROOT / name).exists()]
    if missing:
        return fail(
            "Missing toolchain binaries: "
            + ", ".join(missing)
            + ". Run python build.py and python tools/build_lsp.py first."
        )

    for name in tool_names:
        copy_file(ROOT / name, bin_dir / name)
    for name in ["LICENSE", "LICENSE-APACHE", "LICENSE-MIT"]:
        license_path = ROOT / name
        if license_path.exists():
            copy_file(license_path, EXTENSION_DIR / name)

    stdlib_src = ROOT / "stdlib"
    runtime_src = ROOT / "runtime"
    if not stdlib_src.exists():
        return fail("Missing stdlib folder.")
    if not runtime_src.exists():
        return fail("Missing runtime folder.")

    copy_tree(stdlib_src, EXTENSION_DIR / "stdlib")
    copy_tree(runtime_src, EXTENSION_DIR / "runtime")
    return 0


def package_extension() -> int:
    vsce = find_command("vsce")
    if vsce is None:
        return fail("vsce was not found on PATH.")

    package_json = json.loads((EXTENSION_DIR / "package.json").read_text(encoding="utf-8"))
    name = package_json["name"]
    version = package_json["version"]
    DIST_DIR.mkdir(parents=True, exist_ok=True)
    output = DIST_DIR / f"{name}-{version}.vsix"

    cmd = [vsce, "package", "--no-dependencies", "--out", str(output)]
    print("Packaging VSCode extension with:", " ".join(cmd))
    completed = subprocess.run(cmd, cwd=EXTENSION_DIR)
    if completed.returncode != 0:
        return completed.returncode
    print("SUCCESS")
    print(output)
    return 0


def main() -> int:
    prepared = prepare_payload()
    if prepared != 0:
        return prepared
    return package_extension()


if __name__ == "__main__":
    sys.exit(main())
