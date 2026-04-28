import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT_EXE = ROOT / "zt.exe"


def run_zt(*args, input_text=None):
    completed = subprocess.run(
        [str(ZT_EXE), *args],
        cwd=str(ROOT),
        input=input_text,
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
    expect(ZT_EXE.exists(), "zt.exe must exist before running driver tests")

    rc, out = run_zt("help", "repl")
    expect(rc == 0, "zt help repl should exit 0", out)
    expect("zt repl --eval <expr>" in out, "zt help repl should document --eval", out)

    rc, out = run_zt("repl", "--eval", "1 + 2", "--ci")
    expect(rc == 0, "zt repl --eval should run arithmetic expressions", out)
    expect("3" in out.splitlines(), "arithmetic eval should print 3", out)

    rc, out = run_zt("repl", "--eval", '"ok"', "--ci")
    expect(rc == 0, "zt repl --eval should run text literal expressions", out)
    expect("ok" in out.splitlines(), "text eval should print ok", out)

    rc, out = run_zt("repl", "--ci", input_text="1 + 4\n:quit\n")
    expect(rc == 0, "interactive zt repl should accept stdin lines", out)
    expect("zt> 5" in out or "5" in out.splitlines(), "interactive eval should print 5", out)

    rc, out = run_zt("repl", "--eval", "missing_name", "--ci")
    expect(rc != 0, "zt repl --eval should fail invalid expressions", out)
    expect("unresolved name 'missing_name'" in out, "invalid eval should report the compiler diagnostic", out)

    print("[OK] zt repl")


if __name__ == "__main__":
    main()
