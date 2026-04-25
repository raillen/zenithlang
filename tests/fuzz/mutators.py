"""
R2.M2 - Mutators for tokens and nesting.

All mutators take (rng, text) and return new text.
Mutators are intentionally small and composable; the drivers
(`fuzz_lexer`, `fuzz_parser`) choose which subset to apply.
"""

from __future__ import annotations

import random
import string

# Tokens frequently present in Zenith source (keywords + punctuation).
KEYWORDS = [
    "namespace", "func", "return", "if", "else", "elif", "end", "while",
    "for", "in", "break", "continue", "const", "let", "var", "type",
    "struct", "enum", "match", "case", "default", "import", "as",
    "true", "false", "null", "int", "float", "bool", "string", "void",
    "try", "catch", "throw", "where", "use", "self", "new", "unsafe",
]

PUNCT = [
    "(", ")", "[", "]", "{", "}", ",", ";", ":", "::", ".", "->",
    "=", "==", "!=", "<", ">", "<=", ">=", "+", "-", "*", "/", "%",
    "&&", "||", "!", "?", "??", "|", "&", "^", "~", "<<", ">>",
    "+=", "-=", "*=", "/=",
]

IDENT_CHARS = string.ascii_letters + string.digits + "_"


# ---------------------------------------------------------------------------
# Byte/char level mutators (target the lexer)
# ---------------------------------------------------------------------------

def flip_byte(rng: random.Random, text: str) -> str:
    if not text:
        return text
    i = rng.randrange(len(text))
    c = text[i]
    rep = rng.choice(string.printable[:-6])  # avoid weird control chars
    return text[:i] + rep + text[i + 1:]


def insert_random_bytes(rng: random.Random, text: str) -> str:
    i = rng.randrange(len(text) + 1) if text else 0
    n = rng.randint(1, 6)
    chunk = "".join(rng.choice(string.printable[:-6]) for _ in range(n))
    return text[:i] + chunk + text[i:]


def duplicate_slice(rng: random.Random, text: str) -> str:
    if len(text) < 2:
        return text + text
    a = rng.randrange(len(text))
    b = rng.randrange(a + 1, len(text) + 1)
    return text[:b] + text[a:b] + text[b:]


def delete_slice(rng: random.Random, text: str) -> str:
    if len(text) < 2:
        return text
    a = rng.randrange(len(text))
    b = rng.randrange(a + 1, len(text) + 1)
    return text[:a] + text[b:]


def insert_nonprintable(rng: random.Random, text: str) -> str:
    """Insert control / high-bit bytes to exercise the lexer boundaries."""
    i = rng.randrange(len(text) + 1) if text else 0
    bad = rng.choice(["\x00", "\x01", "\x07", "\x1b", "\xff", "\x7f"])
    return text[:i] + bad + text[i:]


def unterminated_string(rng: random.Random, text: str) -> str:
    i = rng.randrange(len(text) + 1) if text else 0
    return text[:i] + '"this never closes' + text[i:]


LEXER_MUTATORS = [
    flip_byte,
    insert_random_bytes,
    duplicate_slice,
    delete_slice,
    insert_nonprintable,
    unterminated_string,
]


# ---------------------------------------------------------------------------
# Token/structure mutators (target the parser)
# ---------------------------------------------------------------------------

def swap_keyword(rng: random.Random, text: str) -> str:
    kws_in_text = [k for k in KEYWORDS if k in text]
    if not kws_in_text:
        return text + " " + rng.choice(KEYWORDS)
    victim = rng.choice(kws_in_text)
    repl = rng.choice(KEYWORDS)
    return text.replace(victim, repl, 1)


def drop_closing(rng: random.Random, text: str) -> str:
    for close in ("end", ")", "]", "}"):
        if close in text:
            return text.replace(close, "", 1)
    return text


def duplicate_open(rng: random.Random, text: str) -> str:
    for open_ in ("(", "[", "{"):
        if open_ in text:
            i = text.index(open_)
            return text[:i] + open_ * rng.randint(2, 5) + text[i + 1:]
    return text + "(" * rng.randint(2, 5)


def deep_nesting(rng: random.Random, text: str) -> str:
    depth = rng.randint(20, 120)
    return "(" * depth + text + ")" * depth


def insert_random_punct(rng: random.Random, text: str) -> str:
    i = rng.randrange(len(text) + 1) if text else 0
    p = rng.choice(PUNCT)
    return text[:i] + " " + p + " " + text[i:]


def corrupt_identifier(rng: random.Random, text: str) -> str:
    # find an identifier-like run and replace it with something weird
    import re
    m = list(re.finditer(r"[A-Za-z_][A-Za-z_0-9]*", text))
    if not m:
        return text + " " + "".join(rng.choice(IDENT_CHARS) for _ in range(5))
    chosen = rng.choice(m)
    replacement = "".join(rng.choice(IDENT_CHARS + "!@#$") for _ in range(rng.randint(1, 8)))
    return text[:chosen.start()] + replacement + text[chosen.end():]


def shuffle_lines(rng: random.Random, text: str) -> str:
    lines = text.splitlines()
    if len(lines) < 3:
        return text
    rng.shuffle(lines)
    return "\n".join(lines)


PARSER_MUTATORS = [
    swap_keyword,
    drop_closing,
    duplicate_open,
    deep_nesting,
    insert_random_punct,
    corrupt_identifier,
    shuffle_lines,
]


def apply_random(rng: random.Random, text: str, pool, k: int = 1) -> str:
    out = text
    for _ in range(k):
        m = rng.choice(pool)
        try:
            out = m(rng, out)
        except Exception:
            # mutators are best-effort; never fail the harness due to a mutator
            pass
    return out
