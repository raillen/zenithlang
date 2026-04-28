# Module `std.regex`

`std.regex` provides simple regex helpers for text validation and search.

Use it when you need to:

- validate a text format;
- check if a pattern appears in text;
- collect simple matches.

## API

```zt
import std.regex as regex

const compiled: regex.Regex = regex.compile("^[A-Za-z]+[0-9]+$")?

if regex.is_match(compiled.pattern, "player123")
    -- valid text
end

const numbers: list<text> = regex.find_all("[0-9]+", "a12 b7 c345")
```

## Current support

- ASCII literals;
- `.`, `^`, `$`;
- `*`, `+`, `?`;
- classes like `[abc]`, `[a-z]`, `[^0-9]`;
- escapes `\d`, `\w`, `\s`.

## Limits

This cut does not include groups, captures, flags, replace, or full Unicode regex.

Invalid pattern behavior:

- `compile` returns `error(regex.Error.InvalidPattern)`;
- `is_match` returns `false`;
- `find_all` returns an empty list.
