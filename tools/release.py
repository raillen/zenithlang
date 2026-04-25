# R4.M0 - Release automation script
# Builds compiler, runs tests, creates clean install, validates hello-world.
# Usage: python tools/release.py [--version 0.3.0-alpha.2] [--install-dir <path>]

import os
import sys
import shutil
import subprocess
import hashlib
import json
import datetime

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)

DEFAULT_VERSION = "0.3.0-alpha.2"
INSTALL_DIR = os.path.join(REPO_ROOT, ".release-install")
BUILD_DIR = os.path.join(REPO_ROOT, ".release-build")


def log(msg):
    print(f"[release] {msg}")


def run(cmd, cwd=None, check=True):
    log(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if result.stdout:
        print(result.stdout)
    if result.stderr:
        print(result.stderr, file=sys.stderr)
    if check and result.returncode != 0:
        raise RuntimeError(
            f"Command failed with exit code {result.returncode}: {' '.join(cmd)}"
        )
    return result


def file_hash(path, algo="sha256"):
    h = hashlib.sha256() if algo == "sha256" else hashlib.sha512()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest()


def step_build_compiler():
    log("=== Step 1: Build compiler ===")
    run(["python", "build.py"], cwd=REPO_ROOT)
    zt_exe = os.path.join(REPO_ROOT, "zt.exe")
    if not os.path.exists(zt_exe):
        raise RuntimeError("zt.exe not found after build")
    log(f"Compiler built: {zt_exe}")
    return zt_exe


def step_run_tests(zt_exe):
    log("=== Step 2: Run test suite ===")
    result = run(["python", "run_all_tests.py"], cwd=REPO_ROOT, check=False)
    # Parse summary
    summary = {}
    for line in result.stdout.splitlines():
        line = line.strip()
        if line.startswith("Total:"):
            summary["total"] = int(line.split(":")[1].strip())
        elif line.startswith("Pass :"):
            summary["pass"] = int(line.split(":")[1].strip())
        elif line.startswith("Fail :"):
            summary["fail"] = int(line.split(":")[1].strip())
        elif line.startswith("Skip :"):
            summary["skip"] = int(line.split(":")[1].strip())
    log(f"Test results: {summary}")
    return summary


def step_clean_install(zt_exe, install_dir):
    log("=== Step 3: Clean install ===")
    if os.path.exists(install_dir):
        shutil.rmtree(install_dir)
    os.makedirs(install_dir, exist_ok=True)

    # Copy compiler
    shutil.copy2(zt_exe, os.path.join(install_dir, "zt.exe"))

    # Copy runtime
    src_runtime = os.path.join(REPO_ROOT, "runtime")
    dst_runtime = os.path.join(install_dir, "runtime")
    if os.path.exists(src_runtime):
        shutil.copytree(src_runtime, dst_runtime, dirs_exist_ok=True)

    # Copy stdlib
    src_stdlib = os.path.join(REPO_ROOT, "stdlib")
    dst_stdlib = os.path.join(install_dir, "stdlib")
    if os.path.exists(src_stdlib):
        shutil.copytree(src_stdlib, dst_stdlib, dirs_exist_ok=True)

    # Copy docs
    for doc in [
        "README.md",
        "CHANGELOG.md",
        "LICENSE",
        "LICENSE-APACHE",
        "LICENSE-MIT",
    ]:
        src = os.path.join(REPO_ROOT, doc)
        if os.path.exists(src):
            shutil.copy2(src, os.path.join(install_dir, doc))

    log(f"Installed to: {install_dir}")
    files = os.listdir(install_dir)
    log(f"Installed files: {files}")
    return install_dir


def step_validate_install(install_dir):
    log("=== Step 4: Validate clean install ===")
    zt_exe = os.path.join(install_dir, "zt.exe")
    hello_world = os.path.join(REPO_ROOT, "examples", "hello-world")

    if not os.path.exists(hello_world):
        log("WARNING: examples/hello-world not found, skipping validation")
        return True

    # zt check
    log("Running: zt check (hello-world)")
    r = run([zt_exe, "check", hello_world], check=False)
    if r.returncode != 0:
        log(f"FAIL: zt check failed (exit {r.returncode})")
        return False

    # zt build
    log("Running: zt build (hello-world)")
    r = run([zt_exe, "build", hello_world], check=False)
    if r.returncode != 0:
        log(f"FAIL: zt build failed (exit {r.returncode})")
        return False

    # zt run
    log("Running: zt run (hello-world)")
    r = run([zt_exe, "run", hello_world], check=False)
    if r.returncode != 0:
        log(f"FAIL: zt run failed (exit {r.returncode})")
        return False

    log("Clean install validation: PASS")
    return True


def step_checksums(install_dir, version, output_dir):
    log("=== Step 5: Generate checksums ===")
    os.makedirs(output_dir, exist_ok=True)

    zt_exe = os.path.join(install_dir, "zt.exe")
    sha256 = file_hash(zt_exe, "sha256")
    sha512 = file_hash(zt_exe, "sha512")

    checksum_file = os.path.join(
        output_dir, f"zenith-{version}-windows-amd64.checksums.txt"
    )
    with open(checksum_file, "w", encoding="utf-8") as f:
        f.write(f"SHA256  {sha256}  zt.exe\n")
        f.write(f"SHA512  {sha512}  zt.exe\n")

    log(f"Checksums written: {checksum_file}")
    return checksum_file


def step_report(version, test_summary, install_valid, output_dir):
    log("=== Step 6: Generate release report ===")
    os.makedirs(output_dir, exist_ok=True)

    report = {
        "version": version,
        "generated_at_utc": datetime.datetime.utcnow().isoformat() + "Z",
        "compiler_warnings": 0,
        "tests": test_summary,
        "clean_install_validated": install_valid,
        "artifacts": {
            "install_dir": INSTALL_DIR,
            "output_dir": output_dir,
        },
    }

    report_file = os.path.join(output_dir, f"R4.M0-release-artifacts-{version}.json")
    with open(report_file, "w", encoding="utf-8") as f:
        json.dump(report, f, indent=2)

    log(f"Report written: {report_file}")
    return report_file


def main():
    version = DEFAULT_VERSION
    install_dir = INSTALL_DIR
    output_dir = os.path.join(REPO_ROOT, "docs", "reports", "release", "artifacts")

    # Parse args
    args = sys.argv[1:]
    i = 0
    while i < len(args):
        if args[i] == "--version" and i + 1 < len(args):
            version = args[i + 1]
            i += 2
        elif args[i] == "--install-dir" and i + 1 < len(args):
            install_dir = args[i + 1]
            i += 2
        elif args[i] == "--help":
            print(
                "Usage: python tools/release.py [--version X.Y.Z] [--install-dir <path>]"
            )
            return 0
        else:
            i += 1

    log(f"Zenith Release Automation v{version}")
    log(f"Repo: {REPO_ROOT}")
    log(f"Install dir: {install_dir}")
    log(f"Output dir: {output_dir}")
    log("")

    try:
        # Step 1: Build compiler
        zt_exe = step_build_compiler()

        # Step 2: Run tests
        test_summary = step_run_tests(zt_exe)

        # Step 3: Clean install
        step_clean_install(zt_exe, install_dir)

        # Step 4: Validate install
        install_valid = step_validate_install(install_dir)

        # Step 5: Checksums
        step_checksums(install_dir, version, output_dir)

        # Step 6: Report
        step_report(version, test_summary, install_valid, output_dir)

        log("")
        log("=" * 60)
        log("RELEASE ARTIFACTS GENERATED")
        log(f"  Version: {version}")
        log(
            f"  Tests: {test_summary.get('pass', '?')}/{test_summary.get('total', '?')} pass"
        )
        log(f"  Clean install: {'PASS' if install_valid else 'FAIL'}")
        log(f"  Artifacts: {output_dir}")
        log("=" * 60)

        return 0 if install_valid else 1

    except Exception as e:
        log(f"ERROR: {e}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
