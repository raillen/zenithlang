import os
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT = ROOT / ("zt.exe" if os.name == "nt" else "zt")


def run_zt(*args):
    completed = subprocess.run(
        [str(ZT), *args],
        cwd=str(ROOT),
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    return completed.returncode, (completed.stdout or "") + (completed.stderr or "")


def expect(condition, message, output=""):
    if condition:
        return
    print(f"[FAIL] {message}")
    if output:
        print(output.encode("utf-8", errors="replace").decode("utf-8", errors="replace"))
    sys.exit(1)


def expect_no_fragments(output, fragments, label):
    for fragment in fragments:
        expect(fragment not in output, f"{label} should not include {fragment!r}", output)


def main():
    project = "tests/behavior/bytes_hex_literal/zenith.ztproj"
    syntax_error_project = "tests/behavior/error_syntax/zenith.ztproj"
    noisy_native = [
        "compiling runtime cache:",
        "compiling:",
        "warning[native.",
        "built:",
        "running:",
        "exit code:",
        ".ztc-tmp",
        "gcc ",
    ]
    rich_or_ansi = ["\x1b[", "📖", "❌", "⚠", "📌", "➡️", "─"]

    rc, out = run_zt("build", project)
    expect(rc == 0, "zt build should pass", out)
    expect("build ok:" in out, "zt build should print a compact success line", out)
    expect_no_fragments(out, noisy_native, "zt build")

    rc, out = run_zt("run", project)
    expect(rc == 9, "fixture run should preserve program exit code", out)
    expect("program exited with code 9" in out, "zt run should explain non-zero program exit", out)
    expect_no_fragments(out, noisy_native, "zt run")

    rc, out = run_zt("test", "tests/behavior/callable_basic/zenith.ztproj")
    expect(rc == 0, "zt test fallback fixture should pass", out)
    expect(out.strip() == "test ok", "zt test fallback should stay concise", out)
    expect_no_fragments(out, noisy_native, "zt test")

    rc, out = run_zt("doc", "show", "std.regex.replace_all")
    expect(rc == 0, "zt doc show should pass", out)
    expect("replace_all" in out, "zt doc show should print the target docs", out)
    expect("project.invalid_input" not in out, "zt doc show should not parse the symbol as a project path", out)
    expect_no_fragments(out, rich_or_ansi, "zt doc show")

    rc, out = run_zt("check", syntax_error_project)
    expect(rc != 0, "invalid fixture should fail", out)
    expect("ACTION:" in out and "WHY:" in out and "NEXT:" in out, "diagnostics should keep accessible sections", out)
    expect_no_fragments(out, rich_or_ansi, "diagnostics")

    print("[OK] clean cli output")


if __name__ == "__main__":
    main()
