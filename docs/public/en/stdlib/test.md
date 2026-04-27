# Module`std.test`

Complementary module for testing.
Tests are declared with`attr test`.
Use`check(...)`for simple conditions.
Use`std.test`when you want to fail, skip or compare values ​​with a clearer message.

## Constants and Functions

### `fail`

```zt
public func fail(message: text = "test failed") -> void
```

Marks the current test as failed immediately.
Useful in flow control branches, for example after a`match`on`result`.

@param message - Failure message.

### `skip`

```zt
public func skip(reason: text = "") -> void
```

Marks the current test as skipped.
Skip is a different result: neither pass nor fail.

@param reason - Reason for the skip.

### `is_true`

```zt
public func is_true(value: bool) -> void
```

Fails if`value`for`false`.
The message shows`expected true`and`received false`.

### `is_false`

```zt
public func is_false(value: bool) -> void
```

Fails if`value`for`true`.
The message shows`expected false`and`received true`.

### `equal_int`

```zt
public func equal_int(actual: int, expected: int) -> void
```

Fails if`actual != expected`.
The message shows Expected and Received.

### `equal_text`

```zt
public func equal_text(actual: text, expected: text) -> void
```

Fails if`actual != expected`.
The message shows Expected and Received.

### `not_equal_int`

```zt
public func not_equal_int(actual: int, expected: int) -> void
```

Fails if`actual == expected`.
The message shows that the value should be different.

### `not_equal_text`

```zt
public func not_equal_text(actual: text, expected: text) -> void
```

Fails if`actual == expected`.
The message shows that the value should be different.

