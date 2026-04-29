import os
import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT = ROOT / ("zt.exe" if os.name == "nt" else "zt")
TARGET = ROOT / "tests" / "tmp" / "zt-create-current-app"


EXPECTED_MAIN = """namespace app.main

func main()
    print("Hello from Zenith")
end
"""


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


def require(condition, message, output=""):
    if condition:
        return
    raise AssertionError(f"{message}\n--- output ---\n{output}")


def main():
    if TARGET.exists():
        shutil.rmtree(TARGET)

    rc, out = run_zt("create", str(TARGET), "--app")
    require(rc == 0, "zt create --app should succeed", out)

    main_file = TARGET / "src" / "app" / "main.zt"
    require(main_file.exists(), "zt create --app should write src/app/main.zt", out)
    require(main_file.read_text(encoding="utf-8") == EXPECTED_MAIN, "app scaffold should use the current minimal main")

    rc, out = run_zt("check", str(TARGET / "zenith.ztproj"), "--all")
    require(rc == 0, "created app should pass zt check", out)

    rc, out = run_zt("run", str(TARGET / "zenith.ztproj"))
    require(rc == 0, "created app should pass zt run", out)

    print("create scaffold test passed")


if __name__ == "__main__":
    main()
