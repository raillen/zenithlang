# モジュール`std.time`

中心となる時間モジュール`Instant`そして`Duration`。
使用`time.now()`システム クロック エントリ ポイントとして。
比較には通常の演算子 (`==`、`<`、`>`）。

## 定数と関数

### `Instant`

```zt
public struct Instant
    millis: int
end
```

絶対的な瞬間 (エポックからのミリ秒単位のタイムスタンプ) を表します。

### `Duration`

```zt
public struct Duration
    millis: int
end
```

時間間隔をミリ秒単位で表します。

### `Error`

```zt
public enum Error
    SleepInterrupted
    Unknown
end
```

一時的な操作の型付きエラー。

### `now`

```zt
public func now() -> time.Instant
```

システムクロックの現在の瞬間を返します。

### `sleep`

```zt
public func sleep(duration: time.Duration) -> result<void, time.Error>
```

指定した期間実行を一時停止します。

@paramduration — スリープ期間。
@戻る`void`成功した場合、または一時的なエラーが発生した場合。

### `since`

```zt
public func since(start: time.Instant) -> time.Duration
```

以降の期間を返します。`start`今この瞬間まで。

### `until`

```zt
public func until(target: time.Instant) -> time.Duration
```

現在の瞬間から次の時点までの期間を返します。`target`。

### `diff`

```zt
public func diff(a: time.Instant, b: time.Instant) -> time.Duration
```

2 つの瞬間間の時間差を返します (`b - a`）。

### `add`

```zt
public func add(at: time.Instant, duration: time.Duration) -> time.Instant
```

指定された期間中、1 インスタントを進めます。

### `sub`

```zt
public func sub(at: time.Instant, duration: time.Duration) -> time.Instant
```

指定された期間、瞬間を 1 つ巻き戻します。

### `from_unix`

```zt
public func from_unix(ts: int) -> time.Instant
```

を作成します`Instant`Unix タイムスタンプから秒単位で取得します。

### `from_unix_ms`

```zt
public func from_unix_ms(ts: int) -> time.Instant
```

を作成します`Instant`Unix タイムスタンプからミリ秒単位で取得します。

### `to_unix`

```zt
public func to_unix(at: time.Instant) -> int
```

を変換する`Instant`Unix のタイムスタンプ (秒単位)。

### `to_unix_ms`

```zt
public func to_unix_ms(at: time.Instant) -> int
```

を変換する`Instant`Unix の場合はミリ秒単位のタイムスタンプ。

### `milliseconds`

```zt
public func milliseconds(n: int) -> time.Duration
```

の期間を作成します`n`ミリ秒。

### `seconds`

```zt
public func seconds(n: int) -> time.Duration
```

の期間を作成します`n`秒。

### `minutes`

```zt
public func minutes(n: int) -> time.Duration
```

の期間を作成します`n`分。

### `hours`

```zt
public func hours(n: int) -> time.Duration
```

の期間を作成します`n`何時間も。

