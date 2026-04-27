# Module`std.text`

Pure and locale-neutral module for text.
Operates by code point in the current MVP model.
In this alpha cut, the module publishes only the subset that is already safely delivered.

## Constants and Functions

### `to_utf8`

```zt
public func to_utf8(value: text) -> bytes
```

Convert`text`valid for`bytes`UTF-8.

### `from_utf8`

```zt
public func from_utf8(value: bytes) -> result<text, text>
```

Try to decode`bytes`UTF-8.
In this alpha cut, the error continues to be typed as`text`.

### `trim`

```zt
public func trim(value: text) -> text
```

Removes whitespace at both ends.

### `trim_start`

```zt
public func trim_start(value: text) -> text
```

Remove whitespace from the beginning.

### `trim_end`

```zt
public func trim_end(value: text) -> text
```

Remove whitespace from the end.

### `contains`

```zt
public func contains(value: text, needle: text) -> bool
```

Return`true`when`needle`appears in`value`.

### `starts_with`

```zt
public func starts_with(value: text, prefix: text) -> bool
```

Return`true`when`value`start with`prefix`.

### `ends_with`

```zt
public func ends_with(value: text, suffix: text) -> bool
```

Return`true`when`value`ends with`suffix`.

### `has_prefix`

```zt
public func has_prefix(value: text, prefix: text) -> bool
```

Explicit semantic alias for`starts_with`.

### `has_suffix`

```zt
public func has_suffix(value: text, suffix: text) -> bool
```

Explicit semantic alias for`ends_with`.

### `has_whitespace`

```zt
public func has_whitespace(value: text) -> bool
```

Return`true`when there is at least one whitespace character.

### `index_of`

```zt
public func index_of(value: text, needle: text) -> int
```

Returns the index of the first occurrence or`-1`.

### `last_index_of`

```zt
public func last_index_of(value: text, needle: text) -> int
```

Returns the index of the last occurrence or`-1`.

### `is_empty`

```zt
public func is_empty(value: text) -> bool
```

Return`true`when the text is empty.

### `is_blank`

```zt
public func is_blank(value: text) -> bool
```

Return`true`when the text is empty or has only spaces.

### `is_digits`

```zt
public func is_digits(value: text) -> bool
```

Return`true`only for non-empty text composed of ASCII digits`0-9`.

### `limit`

```zt
public func limit(value: text, max_len: int) -> text
```

Cut text to maximum`max_len`code points, without adding a suffix.

