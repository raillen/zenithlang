#!/usr/bin/env python3
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


REQUIRED_DOCS = {
    "surface freeze": (
        ROOT / "docs" / "internal" / "release" / "surface-freeze.md",
        ["stable", "experimental", "internal", "deferred", "Promotion Rule"],
    ),
    "gate evidence": (
        ROOT / "docs" / "internal" / "reports" / "stabilization-phase0-gate-evidence.md",
        ["python build.py", "zt check zenith.ztproj --all --ci", "smoke --no-perf", "pr_gate --no-perf"],
    ),
    "diagnostic audit": (
        ROOT / "docs" / "internal" / "reports" / "stabilization-diagnostic-audit.md",
        ["ACTION", "WHY", "NEXT", "No known P0/P1 diagnostic blocker"],
    ),
}


CHECKLIST_MARKERS = [
    "- [x] `python build.py` green.",
    "- [x] `./zt.exe check zenith.ztproj --all --ci` green.",
    "- [x] `python run_suite.py smoke --no-perf` green.",
    "- [x] Behavior tests for each new public surface.",
    "- [x] Negative tests for each new error.",
    "- [x] Spec or decision updated before changing semantics.",
    "- [x] S.01",
    "- [x] S.02",
    "- [x] S.03",
    "- [x] S.04",
]


def fail(message):
    print(f"FAIL: {message}", file=sys.stderr)
    return 1


def read_text(path):
    return path.read_text(encoding="utf-8")


def assert_contains(name, path, tokens):
    if not path.exists():
        raise AssertionError(f"{name} is missing: {path}")
    text = read_text(path)
    missing = [token for token in tokens if token not in text]
    if missing:
        raise AssertionError(f"{name} is missing required text: {', '.join(missing)}")


def main():
    try:
        workflow = ROOT / ".github" / "workflows" / "ci.yml"
        assert_contains(
            "CI workflow",
            workflow,
            ["windows-latest", "ubuntu-latest", "python build.py", "smoke --no-perf", "check zenith.ztproj --all --ci"],
        )

        for name, (path, tokens) in REQUIRED_DOCS.items():
            assert_contains(name, path, tokens)

        checklist = read_text(ROOT / "docs" / "internal" / "planning" / "checklist-v7.md")
        missing = [marker for marker in CHECKLIST_MARKERS if marker not in checklist]
        if missing:
            raise AssertionError(f"checklist is missing Phase 0 markers: {', '.join(missing)}")

        roadmap = read_text(ROOT / "docs" / "internal" / "planning" / "roadmap-v7.md")
        for marker in ["| S.01 |", "| S.02 |", "| S.03 |", "| S.04 |"]:
            line_start = roadmap.find(marker)
            if line_start < 0:
                raise AssertionError(f"roadmap is missing marker: {marker}")
            line_end = roadmap.find("\n", line_start)
            line = roadmap[line_start:line_end if line_end >= 0 else len(roadmap)]
            if "| done |" not in line:
                raise AssertionError(f"roadmap marker is not done: {line}")
    except Exception as exc:
        return fail(str(exc))

    print("phase0 stabilization ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

