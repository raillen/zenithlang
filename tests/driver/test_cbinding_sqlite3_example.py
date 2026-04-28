import os
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
EXAMPLE = ROOT / "examples" / "c-bindings-sqlite3"
BUILD = EXAMPLE / "build"
ZT = ROOT / ("zt.exe" if os.name == "nt" else "zt")


def run(cmd, *, env=None):
    completed = subprocess.run(
        [str(part) for part in cmd],
        cwd=str(ROOT),
        env=env,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    output = (completed.stdout or "") + (completed.stderr or "")
    if completed.returncode != 0:
        raise AssertionError(
            f"command failed with {completed.returncode}: {cmd}\n--- output ---\n{output}"
        )
    return output


def first_existing(paths):
    for path in paths:
        if path and Path(path).exists():
            return Path(path)
    return None


def find_sqlite():
    include_dir = first_existing([
        os.environ.get("SQLITE3_INCLUDE"),
        r"C:\ProgramData\mingw64\mingw64\opt\include",
        "/usr/include",
        "/usr/local/include",
    ])
    lib_dir = first_existing([
        os.environ.get("SQLITE3_LIB_DIR"),
        r"C:\ProgramData\mingw64\mingw64\opt\lib",
        "/usr/lib",
        "/usr/local/lib",
        "/usr/lib/x86_64-linux-gnu",
    ])

    if include_dir is None or not (include_dir / "sqlite3.h").exists():
        return None
    if lib_dir is None:
        return None

    lib_file = first_existing([
        lib_dir / "libsqlite3.a",
        lib_dir / "libsqlite3.dll.a",
        lib_dir / "libsqlite3.so",
        lib_dir / "sqlite3.lib",
    ])
    if lib_file is None:
        return None

    dll_dir = first_existing([
        os.environ.get("SQLITE3_DLL_DIR"),
        lib_dir / "sqlite3.34.0",
        lib_dir,
    ])

    return include_dir, lib_file, dll_dir


def copy_runtime_dlls(dll_dir):
    if dll_dir is None:
        return
    for dll_name in ("sqlite3.dll", "sqlite3340.dll"):
        dll_path = dll_dir / dll_name
        if dll_path.exists():
            shutil.copy2(dll_path, BUILD / dll_name)


def main():
    sqlite = find_sqlite()
    if sqlite is None:
        print("sqlite3 binding example skipped: sqlite3 headers/libs not found")
        return

    include_dir, lib_file, dll_dir = sqlite
    BUILD.mkdir(parents=True, exist_ok=True)
    shutil.copy2(lib_file, BUILD / "libsqlite3.a")
    copy_runtime_dlls(dll_dir)

    run([
        "gcc",
        "-Wall",
        "-Wextra",
        "-I.",
        f"-I{include_dir}",
        "-c",
        EXAMPLE / "native" / "sqlite3_smoke.c",
        "-o",
        BUILD / "sqlite3_smoke.o",
    ])

    env = os.environ.copy()
    env["PATH"] = str(BUILD) + os.pathsep + env.get("PATH", "")
    output = run([ZT, "run", EXAMPLE / "zenith.ztproj", "--native-raw"], env=env)
    if "exit code: 0" not in output:
        raise AssertionError(f"expected sqlite example to exit 0\n--- output ---\n{output}")

    print("sqlite3 c-binding example: passed")


if __name__ == "__main__":
    main()
