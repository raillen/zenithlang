# モジュール`std.validate`

小さな純粋なブールヘルパー`where`そして定期的なコンディション。
IO、解析、変換はありません。

## 定数と関数

### `between`

```zt
public func between(value: int, min: int, max: int) -> bool
```

範囲チェックも含めて。

### `positive`

```zt
public func positive(value: int) -> bool
```

`true`値が`> 0`。

### `non_negative`

```zt
public func non_negative(value: int) -> bool
```

`true`値が`>= 0`。

### `negative`

```zt
public func negative(value: int) -> bool
```

`true`値が`< 0`。

### `non_zero`

```zt
public func non_zero(value: int) -> bool
```

`true`値がゼロではない場合。

### `one_of`

```zt
public func one_of(value: int, candidates: list<int>) -> bool
```

`true`いつ`value`中にあります`candidates`。

### `one_of_text`

```zt
public func one_of_text(value: text, candidates: list<text>) -> bool
```

`true`テキスト値が内部にある場合`candidates`。

### `not_empty`

```zt
public func not_empty(value: text) -> bool
```

`true`テキストの長さが`> 0`。

### `not_empty_text`

```zt
public func not_empty_text(value: text) -> bool
```

エイリアスのクリア`not_empty`値がテキストの場合。

### `min_length`

```zt
public func min_length(value: text, min: int) -> bool
```

`true`いつ`len(value) >= min`。

### `min_len`

```zt
public func min_len(value: text, min: int) -> bool
```

の短いエイリアス`min_length`。

### `max_length`

```zt
public func max_length(value: text, max: int) -> bool
```

`true`いつ`len(value) <= max`。

### `max_len`

```zt
public func max_len(value: text, max: int) -> bool
```

の短いエイリアス`max_length`。

### `length_between`

```zt
public func length_between(value: text, min: int, max: int) -> bool
```

`true`テキストの長さが指定されている場合`[min, max]`。

