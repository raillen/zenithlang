# Module`std.validate`

Small pure bool helpers for`where`and regular conditions.
No IO, no parsing, no transforms.

## Constants and Functions

### `between`

```zt
public func between(value: int, min: int, max: int) -> bool
```

Including range check.

### `positive`

```zt
public func positive(value: int) -> bool
```

`true`when value is`> 0`.

### `non_negative`

```zt
public func non_negative(value: int) -> bool
```

`true`when value is`>= 0`.

### `negative`

```zt
public func negative(value: int) -> bool
```

`true`when value is`< 0`.

### `non_zero`

```zt
public func non_zero(value: int) -> bool
```

`true`when value is not zero.

### `one_of`

```zt
public func one_of(value: int, candidates: list<int>) -> bool
```

`true`when`value`is inside`candidates`.

### `one_of_text`

```zt
public func one_of_text(value: text, candidates: list<text>) -> bool
```

`true`when text value is inside`candidates`.

### `not_empty`

```zt
public func not_empty(value: text) -> bool
```

`true`when text length is`> 0`.

### `not_empty_text`

```zt
public func not_empty_text(value: text) -> bool
```

Clear alias for`not_empty`when the value is text.

### `min_length`

```zt
public func min_length(value: text, min: int) -> bool
```

`true`when`len(value) >= min`.

### `min_len`

```zt
public func min_len(value: text, min: int) -> bool
```

Short alias for`min_length`.

### `max_length`

```zt
public func max_length(value: text, max: int) -> bool
```

`true`when`len(value) <= max`.

### `max_len`

```zt
public func max_len(value: text, max: int) -> bool
```

Short alias for`max_length`.

### `length_between`

```zt
public func length_between(value: text, min: int, max: int) -> bool
```

`true`when text length is in`[min, max]`.

