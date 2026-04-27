# Module`std.concurrent`

Small module to make copying between isolates explicit.
It does not create threads.
He doesn't expose`jobs`yet.

## Constants and Functions

### `copy_int`

```zt
public func copy_int(value: int) -> int
```

Returns the same`int`.
Scalars are already naturally transferable.

### `copy_bool`

```zt
public func copy_bool(value: bool) -> bool
```

Returns the same`bool`.

### `copy_float`

```zt
public func copy_float(value: float) -> float
```

Returns the same`float`.

### `copy_text`

```zt
public func copy_text(value: text) -> text
```

Creates a secure copy of`text`for isolate borders.

### `copy_bytes`

```zt
public func copy_bytes(value: bytes) -> bytes
```

Creates a secure copy of`bytes`for isolate borders.

### `copy_list_int`

```zt
public func copy_list_int(value: list<int>) -> list<int>
```

Creates a secure copy of`list<int>`.

### `copy_list_text`

```zt
public func copy_list_text(value: list<text>) -> list<text>
```

Creates a secure copy of`list<text>`.

### `copy_map_text_text`

```zt
public func copy_map_text_text(value: map<text,text>) -> map<text,text>
```

Creates a secure copy of`map<text,text>`.
In the current alpha, this is the initial public block for downloadable maps.

