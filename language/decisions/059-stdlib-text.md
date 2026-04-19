# Decision 059 - Stdlib Text Module

- Status: accepted
- Date: 2026-04-17
- Type: stdlib / text
- Scope: text transformation, text predicates, Unicode casing tables, splitting, replacing, padding, masking, limiting and truncation

## Summary

`std.text` is the pure, locale-neutral-by-default module for inspecting and transforming `text`.

It does not perform IO, formatting, parsing, JSON handling or path logic.

The module works by code point in the MVP language model.

It also owns explicit UTF-8 conversion between `text` and `bytes`.

## Decision

Canonical import:

```zt
import std.text as text
```

Accepted first-wave API direction:

- `trim`
- `trim_start`
- `trim_end`
- `contains`
- `starts_with`
- `ends_with`
- `has_prefix`
- `has_suffix`
- `has_whitespace`
- `index_of`
- `last_index_of`
- `replace_all`
- `replace_first`
- `split`
- `split_lines`
- `join`
- `is_empty`
- `to_lower`
- `to_upper`
- `capitalize`
- `title_case`
- `is_digits`
- `pad_left`
- `pad_right`
- `mask`
- `limit`
- `truncate`
- `to_utf8`
- `from_utf8`
- `text.Error`

Examples:

```zt
const clean: text = text.trim(name)
const prefixed: bool = text.starts_with(name, "Dr.")
const same_prefixed: bool = text.has_prefix(name, "Dr.")
const spaced: bool = text.has_whitespace(full_name)

const parts: list<text> = text.split(csv_line, ",")
const lines: list<text> = text.split_lines(body)

const all_replaced: text = text.replace_all(message, "Zenit", "Zenith")
const first_replaced: text = text.replace_first(message, "Zenit", "Zenith")

const padded_left: text = text.pad_left(code, width: 4, fill: "0")
const padded_right: text = text.pad_right(label, width: 12)

const masked: text = text.mask(card, visible_start: 2, visible_end: 2)
const shortened: text = text.truncate(message, max_len: 20)
```

Normative rules for this cut:

- `std.text` is pure
- operations are locale-neutral by default
- text operations use code-point semantics in the MVP model
- `len(...)` remains in the core language and is not duplicated here
- text slicing remains in the core language and is not duplicated here
- `concat` is not added because `+` and `join(...)` already cover the intended use cases
- `split_once` is not part of this cut
- `split_lines` replaces the earlier `lines` naming
- `replace_all` replaces the earlier generic `replace` naming
- `replace_first` is accepted as the explicit single-replacement variant
- `pad_left` and `pad_right` belong in `std.text`, not in `std.format`
- `reverse` is deferred
- `clean_accents` is removed from the MVP discussion
- a separate user-facing `char` type is not introduced in the MVP
- `text.to_utf8(...)` converts valid Zenith text to immutable `bytes`
- `text.from_utf8(...)` decodes `bytes` into `text` and returns `result<text, text.Error>`

## Casing

The accepted casing functions are:

- `to_lower(value: text) -> text`
- `to_upper(value: text) -> text`
- `capitalize(value: text) -> text`
- `title_case(value: text) -> text`

Implementation direction:

- Unicode casing uses offline-generated tables
- runtime C consults those generated tables
- locale-specific special casing is not part of the MVP

Semantics:

- `capitalize` uppercases the first code point and leaves the remainder unchanged
- `title_case` is the stronger title-oriented transformation and may normalize more than `capitalize`

## Search, Split, Replace And Padding

Accepted directions:

```zt
text.index_of(value, needle)
text.last_index_of(value, needle)

text.split(value, separator)
text.split_lines(value)

text.replace_all(value, search, replacement)
text.replace_first(value, search, replacement)

text.pad_left(value, width: 8)
text.pad_left(value, width: 8, fill: "0")
text.pad_right(value, width: 8)
```

Semantics:

- `split_lines` is the explicit line-splitting name for the MVP surface
- `replace_all` replaces every non-overlapping match
- `replace_first` replaces only the first match
- `pad_left` and `pad_right` expand a text to at least the requested width
- `fill` in padding must be non-empty text
- padding width is defined in code-point terms in the MVP model

## Digits And Whitespace

Accepted predicates:

```zt
text.is_digits(value)
text.has_whitespace(value)
```

Semantics:

- `is_digits` returns `true` only when the text is non-empty and every character is an ASCII digit `0-9`
- Unicode numeric categories are not part of `is_digits`
- `has_whitespace` returns `true` when the text contains at least one whitespace character

## Mask, Limit And Truncate

Accepted directions:

```zt
text.mask(t, visible_start: 2, visible_end: 2)
text.mask(t, visible_start: 2, visible_end: 2, char: "*")

text.limit(t, max_len: 10)

text.truncate(t, max_len: 10)
text.truncate(t, max_len: 10, suffix: "...")
```

Semantics:

- `mask` preserves the configured visible prefix and suffix and masks the middle
- `mask` returns the original text when the visible regions already cover the whole value
- `char` in `mask` must be non-empty text
- `limit` cuts text to a maximum length without adding a suffix
- `truncate` cuts text to fit within `max_len` and appends a suffix when appropriate
- all three operate by code point in the MVP model

## UTF-8 Conversion

Accepted conversion functions:

```zt
text.to_utf8(value: text) -> bytes
text.from_utf8(data: bytes) -> result<text, text.Error>
```

Semantics:

- `to_utf8` is infallible because Zenith `text` is valid UTF-8
- `from_utf8` is fallible because arbitrary `bytes` may not be valid UTF-8
- `text.Error` is the typed error family for text decoding failures
- no implicit conversion exists between `text` and `bytes`

## Rationale

`std.text` should stay focused on intrinsic text operations.

Keeping it pure and locale-neutral by default preserves predictability.

Offline-generated Unicode tables allow useful case functions without scattering ad-hoc Unicode logic across the runtime.

Separating `replace_all` and `replace_first` keeps replacement intent visible.

Keeping padding in `std.text` avoids turning `std.format` into a generic text-transformation bucket.

## Non-Canonical Forms

Putting IO or formatting in `std.text`:

```zt
text.print(value)
text.format_number(value)
```

Adding `concat` as a parallel API to `+` and `join(...)`.

Using `is_digits` to mean Unicode numeric categories:

```text
"Ù¡Ù¢Ù£"
```

Adding a user-facing `char` type just to support text helpers.

Treating `reverse` as part of the MVP without clearly defining grapheme semantics.

## Out of Scope

This decision does not yet define:

- regex
- accent removal
- emoji detection
- grapheme-aware reverse
- locale-aware sorting/casing
- parsing from text to numeric types
- encodings beyond UTF-8
- `split_once`
- a user-visible char type
