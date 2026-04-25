"""
R2.M2 - Input minimizer (simple delta-debugging).

Given a source that triggers a 'crash' or 'timeout' predicate, repeatedly
try to remove chunks while preserving the trigger, shrinking the report.
"""

from __future__ import annotations

from typing import Callable


def _try_drop_line(text: str, i: int) -> str:
    lines = text.splitlines()
    if i >= len(lines):
        return text
    del lines[i]
    return "\n".join(lines)


def _try_drop_range(text: str, a: int, b: int) -> str:
    return text[:a] + text[b:]


def minimize(source: str, predicate: Callable[[str], bool], max_steps: int = 64) -> str:
    """Shrink `source` while `predicate(source)` remains True.

    Strategy (cheap):
      1. Try deleting each line individually.
      2. Then try shrinking character ranges in halves.
    """
    current = source

    # Phase 1 - per line
    changed = True
    steps = 0
    while changed and steps < max_steps:
        changed = False
        lines = current.splitlines()
        for i in range(len(lines)):
            steps += 1
            if steps > max_steps:
                break
            candidate = _try_drop_line(current, i)
            if candidate and candidate != current and predicate(candidate):
                current = candidate
                changed = True
                break

    # Phase 2 - character halves
    chunk = max(1, len(current) // 2)
    while chunk >= 1 and steps < max_steps:
        a = 0
        while a < len(current) and steps < max_steps:
            steps += 1
            b = min(len(current), a + chunk)
            candidate = _try_drop_range(current, a, b)
            if candidate and candidate != current and predicate(candidate):
                current = candidate
            else:
                a += chunk
        chunk //= 2

    return current
