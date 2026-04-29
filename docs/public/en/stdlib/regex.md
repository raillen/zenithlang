# Module `std.regex`

`std.regex` provides simple regex helpers for text validation and search.

Use it when you need to:

- validate a text format;
- check if a pattern appears in text;
- collect simple matches.
- split or replace text with a simple pattern.

## API

```zt
import std.regex as regex

const compiled: regex.Regex = regex.compile("^[A-Za-z]+[0-9]+$")?

if regex.is_match(compiled.pattern, "player123")
    -- valid text
end

const whole: bool = regex.full_match("[A-Za-z]+", "player")?
const first_number: optional<text> = regex.first("[0-9]+", "a12 b7")
const numbers: list<text> = regex.find_all("[0-9]+", "a12 b7 c345")
const parts: list<text> = regex.split("[,;]+", "red,green;blue")
const masked: text = regex.replace_all("[0-9]+", "a12 b7", "N")
const literal_pattern: text = regex.escape("a.c+")
```

Common helpers:

| Helper | Use |
| --- | --- |
| `regex.is_valid(pattern)` | Check if the pattern is accepted. |
| `regex.matches(pattern, input)` | Match with explicit invalid-pattern errors. |
| `regex.contains(pattern, input)` | Clear alias for `is_match`. |
| `regex.full_match(pattern, input)` | Require the whole input to match. |
| `regex.first(pattern, input)` | Return the first match as `optional<text>`. |
| `regex.count(pattern, input)` | Count non-empty matches. |
| `regex.find_all(pattern, input)` | Return all non-empty matches. |
| `regex.split(pattern, input)` | Split by non-empty matches. |
| `regex.replace_all(pattern, input, replacement)` | Replace all non-empty matches. |
| `regex.escape(input)` | Escape basic regex metacharacters. |

## Current support

- ASCII literals;
- `.`, `^`, `$`;
- `*`, `+`, `?`;
- classes like `[abc]`, `[a-z]`, `[^0-9]`;
- escapes `\d`, `\w`, `\s`.

## Limits

This cut does not include groups, captures, flags, or full Unicode regex.

Invalid pattern behavior:

- `compile` returns `error(regex.Error.InvalidPattern)`;
- `matches` and `full_match` return `error(regex.Error.InvalidPattern)`;
- `is_match` returns `false`;
- `first` returns `none`;
- `count` returns `0`;
- `find_all` returns an empty list;
- `split` returns a list with the original input;
- `replace_all` returns the original input.
