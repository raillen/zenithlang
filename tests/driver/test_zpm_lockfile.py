import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZPM_EXE = ROOT / "zpm.exe"
SANDBOX = ROOT / ".ztc-tmp" / "tests" / "zpm_lockfile"


def run_zpm(*args):
    completed = subprocess.run(
        [str(ZPM_EXE), *map(str, args)],
        cwd=str(ROOT),
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    return completed.returncode, (completed.stdout or "") + (completed.stderr or "")


def expect(condition, message, output=""):
    if condition:
        return
    print(f"[FAIL] {message}")
    if output:
        print(output.encode("utf-8", errors="replace").decode("utf-8", errors="replace"))
    sys.exit(1)


def write_manifest(project_dir, json_version="1.2.3"):
    (project_dir / "zenith.ztproj").write_text(
        f"""[project]
name = "lock-demo"
kind = "app"
version = "0.1.0"

[source]
root = "src"

[app]
entry = "app.main"

[dependencies]
json = "{json_version}"
local_helpers = {{ path = "../helpers" }}

[dev_dependencies]
test_helpers = "0.4.0"
""",
        encoding="utf-8",
    )


def main():
    expect(ZPM_EXE.exists(), "zpm.exe must exist before running zpm lockfile tests")

    if SANDBOX.exists():
        shutil.rmtree(SANDBOX)
    project_dir = SANDBOX / "project"
    project_dir.mkdir(parents=True)
    write_manifest(project_dir)

    rc, out = run_zpm("install", project_dir)
    expect(rc == 0, "zpm install should create zenith.lock", out)

    lock_path = project_dir / "zenith.lock"
    expect(lock_path.exists(), "zpm install should write zenith.lock", out)
    lock_text = lock_path.read_text(encoding="utf-8")
    expect("schema = 1" in lock_text, "lockfile should include schema", lock_text)
    expect('name = "json"' in lock_text and 'version = "1.2.3"' in lock_text, "lockfile should pin version dependency", lock_text)
    expect('name = "local_helpers"' in lock_text and 'path = "../helpers"' in lock_text, "lockfile should pin path dependency", lock_text)
    expect('name = "test_helpers"' in lock_text and 'version = "0.4.0"' in lock_text, "lockfile should include dev dependencies", lock_text)

    rc, out = run_zpm("install", "--locked", project_dir)
    expect(rc == 0, "zpm install --locked should accept current lockfile", out)
    expect("zenith.lock is up to date" in out, "--locked should report an up-to-date lockfile", out)

    write_manifest(project_dir, json_version="1.2.4")
    rc, out = run_zpm("install", "--locked", project_dir)
    expect(rc != 0, "zpm install --locked should reject stale lockfile", out)
    expect("out of date for dependency json" in out, "stale lockfile should name the dependency", out)

    lock_path.unlink()
    rc, out = run_zpm("install", "--locked", project_dir)
    expect(rc != 0, "zpm install --locked should require zenith.lock", out)
    expect("--locked requires" in out, "missing lockfile should explain the locked requirement", out)

    print("[OK] zpm lockfile")


if __name__ == "__main__":
    main()
