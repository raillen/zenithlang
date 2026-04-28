import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZPM_EXE = ROOT / "zpm.exe"
SANDBOX = ROOT / ".ztc-tmp" / "tests" / "zpm_semver"


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


def main():
    expect(ZPM_EXE.exists(), "zpm.exe must exist before running zpm semver tests")

    if SANDBOX.exists():
        shutil.rmtree(SANDBOX)
    project_dir = SANDBOX / "project"

    rc, out = run_zpm("init", project_dir)
    expect(rc == 0, "zpm init should create a sandbox project", out)

    rc, out = run_zpm("--project", project_dir, "add", "math@^1.2.3")
    expect(rc == 0, "zpm add should accept caret SemVer constraints", out)
    manifest = (project_dir / "zenith.ztproj").read_text(encoding="utf-8")
    expect('math = "^1.2.3"' in manifest, "manifest should keep the caret SemVer constraint", manifest)

    rc, out = run_zpm("--project", project_dir, "add", "json@~2.0.1")
    expect(rc == 0, "zpm add should accept tilde SemVer constraints", out)
    manifest = (project_dir / "zenith.ztproj").read_text(encoding="utf-8")
    expect('json = "~2.0.1"' in manifest, "manifest should keep the tilde SemVer constraint", manifest)

    rc, out = run_zpm("--project", project_dir, "add", "bad@1.2")
    expect(rc != 0, "zpm add should reject incomplete SemVer", out)
    expect("invalid SemVer constraint" in out, "invalid SemVer should produce an action-first error", out)

    manifest_path = project_dir / "zenith.ztproj"
    manifest_path.write_text(
        manifest.replace('json = "~2.0.1"', 'json = "latest"'),
        encoding="utf-8",
    )
    rc, out = run_zpm("install", project_dir)
    expect(rc != 0, "zpm install should reject non-SemVer dependency strings", out)
    expect("dependency json has invalid SemVer constraint" in out, "install error should name the invalid dependency", out)

    print("[OK] zpm semver")


if __name__ == "__main__":
    main()
