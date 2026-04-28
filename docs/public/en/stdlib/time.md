# Module`std.time`

Temporal module centered on`Instant`and`Duration`.
Use`time.now()`as the system clock entry point.
Comparison uses ordinary operators (`==`, `<`, `>`).

## Constants and Functions

### `Instant`

```zt
public struct Instant
    millis: int
end
```

Represents an absolute moment (timestamp in milliseconds since epoch).

### `Duration`

```zt
public struct Duration
    millis: int
end
```

Represents a time interval in milliseconds.

### `now`

```zt
public func now() -> time.Instant
```

Returns the current instant of the system clock.

### `now_ms`

```zt
public func now_ms() -> int
```

Returns the current Unix timestamp in milliseconds.

### `sleep`

```zt
public func sleep(duration: time.Duration) -> result<void, core.Error>
```

Suspends execution for the specified period.

@param duration — Sleep duration.
@return `void` on success, or host error.

### `sleep_ms`

```zt
public func sleep_ms(ms: int) -> result<void, core.Error>
```

Suspends execution for `ms` milliseconds.

### `since`

```zt
public func since(start: time.Instant) -> time.Duration
```

Returns the duration since`start`until the current moment.

### `until`

```zt
public func until(target: time.Instant) -> time.Duration
```

Returns the duration from the current moment until`target`.

### `diff`

```zt
public func diff(a: time.Instant, b: time.Instant) -> time.Duration
```

Returns the temporal difference between two instants (`b - a`).

### `elapsed`

```zt
public func elapsed(start: time.Instant, finish: time.Instant) -> int
```

Returns the difference in milliseconds between `start` and `finish`.

### `add`

```zt
public func add(at: time.Instant, duration: time.Duration) -> time.Instant
```

Advance one instant for the specified duration.

### `sub`

```zt
public func sub(at: time.Instant, duration: time.Duration) -> time.Instant
```

Rewinds one instant for the specified duration.

### `from_unix`

```zt
public func from_unix(ts: int) -> time.Instant
```

Create a`Instant`from Unix timestamp in seconds.

### `from_unix_ms`

```zt
public func from_unix_ms(ts: int) -> time.Instant
```

Create a`Instant`from Unix timestamp in milliseconds.

### `to_unix`

```zt
public func to_unix(at: time.Instant) -> int
```

Convert a`Instant`for Unix timestamp in seconds.

### `to_unix_ms`

```zt
public func to_unix_ms(at: time.Instant) -> int
```

Convert a`Instant`for Unix timestamp in milliseconds.

### `milliseconds`

```zt
public func milliseconds(n: int) -> time.Duration
```

Creates a duration of`n`milliseconds.

### `seconds`

```zt
public func seconds(n: int) -> time.Duration
```

Creates a duration of`n`seconds.

### `minutes`

```zt
public func minutes(n: int) -> time.Duration
```

Creates a duration of`n`minutes.

### `hours`

```zt
public func hours(n: int) -> time.Duration
```

Creates a duration of`n`hours.

