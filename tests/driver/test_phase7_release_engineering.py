#!/usr/bin/env python3
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ZT_EXE = ROOT / ("zt.exe" if sys.platform.startswith("win") else "zt")


RELEASE_DOCS = {
    "semver": (
        ROOT / "docs" / "internal" / "release" / "semver-policy.md",
        ["SemVer", "breaking changes", "0.x alpha", "public surface"],
    ),
    "deprecation": (
        ROOT / "docs" / "internal" / "release" / "deprecation-policy.md",
        ["one release", "warning", "canonical replacement"],
    ),
    "docs canonical": (
        ROOT / "docs" / "internal" / "release" / "docs-canonical-policy.md",
        ["English docs are canonical", "best-effort translations", "docs/public/en"],
    ),
    "borealis dogfood": (
        ROOT / "docs" / "internal" / "release" / "borealis-clean-clone-dogfood.md",
        ["Borealis", "clean-clone", "zt check packages/borealis/zenith.ztproj --all"],
    ),
    "rc freeze": (
        ROOT / "docs" / "internal" / "release" / "release-candidate-freeze.md",
        ["release candidate", "no known open P0/P1 bugs", "clean install"],
    ),
}


BOREALIS_PROJECTS = [
    ("borealis package", ["check", str(ROOT / "packages" / "borealis" / "zenith.ztproj"), "--all"], None),
    (
        "borealis foundations",
        ["run", str(ROOT / "tests" / "behavior" / "borealis_foundations_stub" / "zenith.ztproj")],
        None,
    ),
    (
        "borealis scene entities",
        ["run", str(ROOT / "tests" / "behavior" / "borealis_scene_entities_stub" / "zenith.ztproj")],
        None,
    ),
]


def fail(message):
    print(f"FAIL: {message}", file=sys.stderr)
    return 1


def read_text(path):
    return path.read_text(encoding="utf-8")


def assert_doc(name, path, required):
    if not path.exists():
        raise AssertionError(f"{name} doc is missing: {path}")
    text = read_text(path)
    missing = [token for token in required if token not in text]
    if missing:
        raise AssertionError(f"{name} doc is missing required text: {', '.join(missing)}")


def run_zt(name, args, expected):
    result = subprocess.run(
        [str(ZT_EXE), *args],
        cwd=str(ROOT),
        text=True,
        encoding="utf-8",
        errors="replace",
        capture_output=True,
        timeout=180,
        check=False,
    )
    output = (result.stdout or "") + (result.stderr or "")
    if result.returncode != 0:
        raise AssertionError(f"{name} failed with exit {result.returncode}\n{output}")
    if expected and expected not in output:
        raise AssertionError(f"{name} did not print expected marker: {expected}\n{output}")


def main():
    if not ZT_EXE.exists():
        return fail("zt executable must exist before running Phase 7 release engineering")

    try:
        for name, (path, required) in RELEASE_DOCS.items():
            assert_doc(name, path, required)

        if not (ROOT / "docs" / "public" / "en" / "README.md").exists():
            raise AssertionError("canonical English docs entrypoint is missing")
        if not (ROOT / "packages" / "borealis" / "borealis.editor.json").exists():
            raise AssertionError("Borealis editor manifest is missing")
        if not (ROOT / "packages" / "borealis" / "zpm-prep-v1.md").exists():
            raise AssertionError("Borealis package prep notes are missing")

        for name, args, expected in BOREALIS_PROJECTS:
            run_zt(name, args, expected)
    except Exception as exc:
        return fail(str(exc))

    print("phase7 release engineering ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
