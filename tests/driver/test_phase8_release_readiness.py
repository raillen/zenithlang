#!/usr/bin/env python3
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


DOCS = {
    "stable coverage": (
        ROOT / "docs" / "internal" / "reports" / "release" / "1.0-stable-feature-coverage.md",
        ["Stable Coverage Matrix", "Public docs", "Test evidence"],
    ),
    "p0 p1 record": (
        ROOT / "docs" / "internal" / "reports" / "release" / "1.0-no-p0-p1-record.md",
        ["No known open P0/P1 blocker", "pr_gate --no-perf"],
    ),
    "clean install matrix": (
        ROOT / "docs" / "internal" / "reports" / "release" / "1.0-clean-install-matrix.md",
        ["Windows x64", "Linux x64", "macOS", ".github/workflows/ci.yml"],
    ),
    "readiness report": (
        ROOT / "docs" / "internal" / "reports" / "release" / "1.0-readiness-report.md",
        ["It does not claim", "Release Blockers Before Tag", "public GitHub Release tag"],
    ),
    "announcement": (
        ROOT / "docs" / "public" / "en" / "announcements" / "zenith-1.0.md",
        ["Zenith 1.0", "What Is Stable", "What Is Deferred", "Status: draft"],
    ),
}


CHECKLIST_MARKERS = [
    "- [x] V.01",
    "- [x] V.02",
    "- [x] V.03",
    "- [x] V.04",
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
            ["windows-latest", "ubuntu-latest", "macos-latest", "python build.py", "smoke --no-perf"],
        )

        for name, (path, tokens) in DOCS.items():
            assert_contains(name, path, tokens)

        checklist = read_text(ROOT / "docs" / "internal" / "planning" / "checklist-v7.md")
        missing = [marker for marker in CHECKLIST_MARKERS if marker not in checklist]
        if missing:
            raise AssertionError(f"checklist is missing Phase 8 markers: {', '.join(missing)}")

        roadmap = read_text(ROOT / "docs" / "internal" / "planning" / "roadmap-v7.md")
        for marker in ["| V.01 |", "| V.02 |", "| V.03 |", "| V.04 |"]:
            line_start = roadmap.find(marker)
            if line_start < 0:
                raise AssertionError(f"roadmap is missing marker: {marker}")
            line_end = roadmap.find("\n", line_start)
            line = roadmap[line_start:line_end if line_end >= 0 else len(roadmap)]
            if "| done |" not in line:
                raise AssertionError(f"roadmap marker is not done: {line}")
    except Exception as exc:
        return fail(str(exc))

    print("phase8 release readiness ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

