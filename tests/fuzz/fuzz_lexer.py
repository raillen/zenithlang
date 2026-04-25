"""
R2.M2 - Lexer fuzz driver.

Picks a seed from corpus/valid|invalid and applies byte-level mutators.
Detects crashes/timeouts; persists offending seeds.
"""

from __future__ import annotations

import argparse
import json
import random
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from harness import run_check, persist_crash   # noqa: E402
from mutators import LEXER_MUTATORS, apply_random  # noqa: E402
from minimizer import minimize  # noqa: E402

CORPUS_VALID   = HERE / "corpus" / "valid"
CORPUS_INVALID = HERE / "corpus" / "invalid"


def load_corpus() -> list[str]:
    out: list[str] = []
    for d in (CORPUS_VALID, CORPUS_INVALID):
        if not d.exists():
            continue
        for p in sorted(d.glob("*.zt")):
            out.append(p.read_text(encoding="utf-8", errors="replace"))
    if not out:
        out.append("func main() -> int\n    return 0\nend\n")
    return out


def run_campaign(iters: int, seed: int, timeout_sec: float,
                 minimize_crashes: bool, verbose: bool) -> int:
    rng = random.Random(seed)
    corpus = load_corpus()
    crashes = 0
    timeouts = 0

    for i in range(iters):
        base = rng.choice(corpus)
        k = rng.randint(1, 3)
        fuzzed = apply_random(rng, base, LEXER_MUTATORS, k=k)
        res = run_check(fuzzed, timeout_sec=timeout_sec)

        if res.kind == "crash":
            crashes += 1
            if minimize_crashes:
                def pred(t: str) -> bool:
                    return run_check(t, timeout_sec=timeout_sec).kind == "crash"
                fuzzed = minimize(fuzzed, pred, max_steps=32)
            p = persist_crash(fuzzed, f"lexer_{seed}_{i:05d}")
            print(f"  [CRASH] iter={i} reason={res.reason} -> {p}")
        elif res.kind == "timeout":
            timeouts += 1
            p = persist_crash(fuzzed, f"lexer_timeout_{seed}_{i:05d}")
            print(f"  [TIMEOUT] iter={i} {res.elapsed_ms}ms -> {p}")
        elif verbose:
            print(f"  [{res.kind:6s}] iter={i} rc={res.rc} {res.elapsed_ms}ms")

    print(json.dumps({
        "driver": "fuzz_lexer", "seed": seed, "iters": iters,
        "crashes": crashes, "timeouts": timeouts,
    }))
    return 1 if (crashes or timeouts) else 0


def main() -> int:
    ap = argparse.ArgumentParser(description="Zenith lexer fuzz harness")
    ap.add_argument("--iters", type=int, default=100)
    ap.add_argument("--seed", type=int, default=20260421)
    ap.add_argument("--timeout", type=float, default=8.0)
    ap.add_argument("--no-minimize", action="store_true")
    ap.add_argument("--verbose", "-v", action="store_true")
    args = ap.parse_args()
    return run_campaign(args.iters, args.seed, args.timeout,
                        not args.no_minimize, args.verbose)


if __name__ == "__main__":
    raise SystemExit(main())
