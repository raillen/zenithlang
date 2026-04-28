import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT_EXE = ROOT / "zt.exe"
PROJECT = ROOT / "tests" / "behavior" / "std_test_attr_fail"


def run_zt(*args):
    completed = subprocess.run(
        [str(ZT_EXE), *args],
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
        safe_output = output.encode("utf-8", errors="replace").decode("utf-8", errors="replace")
        try:
            print(safe_output)
        except UnicodeEncodeError:
            print(safe_output.encode("cp1252", errors="replace").decode("cp1252", errors="replace"))
    sys.exit(1)


def main():
    expect(ZT_EXE.exists(), "zt.exe must exist before running driver tests")

    rc, out = run_zt("help", "test")
    expect(rc == 0, "zt help test should exit 0", out)
    expect("--filter <name>" in out, "zt help test should document --filter", out)

    rc, out = run_zt("test", str(PROJECT), "--ci", "--filter", "pass_case")
    expect(rc == 0, "pass_case filter should hide fail_case", out)
    expect("test ok (pass=1 skip=0)" in out, "pass_case filter should run only the passing case", out)
    expect("test pass app.tests.pass_case duration=" in out, "pass_case output should include name and duration", out)

    rc, out = run_zt("test", str(PROJECT), "--ci", "--filter", "skip_case")
    expect(rc == 0, "skip_case filter should exit 0", out)
    expect("test ok (pass=0 skip=1)" in out, "skip_case filter should run only the skipped case", out)
    expect("test skip app.tests.skip_case duration=" in out, "skip_case output should include name and duration", out)
    expect("stacktrace:" in out and "at app.tests.skip_case" in out, "skip_case output should include a source trace", out)

    rc, out = run_zt("test", str(PROJECT), "--ci", "--filter", "app.tests.fail_case")
    expect(rc != 0, "qualified fail_case filter should fail", out)
    expect("test failed (pass=0 skip=0 fail=1)" in out, "qualified fail_case filter should run only fail_case", out)
    expect("test fail app.tests.fail_case duration=" in out, "fail_case output should include name and duration", out)
    expect("stacktrace:" in out and "at app.tests.fail_case" in out, "fail_case output should include a source trace", out)

    rc, out = run_zt("check", str(PROJECT), "--filter", "pass_case")
    expect(rc != 0, "--filter should be rejected outside zt test", out)
    expect("option --filter is only valid for test" in out, "wrong command should explain --filter scope", out)

    print("[OK] zt test --filter")


if __name__ == "__main__":
    main()
