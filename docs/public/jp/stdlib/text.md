# モジュール`std.text`

純粋でロケールに依存しないテキスト用モジュール。
現在の MVP モデルのコード ポイントによって動作します。
このアルファカットでは、モジュールはすでに安全に配信されているサブセットのみを公開します。

## 定数と関数

### `to_utf8`

```zt
public func to_utf8(value: text) -> bytes
```

変換する`text`に有効です`bytes`UTF-8。

### `from_utf8`

```zt
public func from_utf8(value: bytes) -> result<text, text>
```

デコードしてみる`bytes`UTF-8。
このアルファカットでは、エラーは引き続き次のように入力されます。`text`。

### `trim`

```zt
public func trim(value: text) -> text
```

両端の空白を削除します。

### `trim_start`

```zt
public func trim_start(value: text) -> text
```

先頭から空白を削除します。

### `trim_end`

```zt
public func trim_end(value: text) -> text
```

末尾の空白を削除します。

### `contains`

```zt
public func contains(value: text, needle: text) -> bool
```

戻る`true`いつ`needle`に登場する`value`。

### `starts_with`

```zt
public func starts_with(value: text, prefix: text) -> bool
```

戻る`true`いつ`value`から始める`prefix`。

### `ends_with`

```zt
public func ends_with(value: text, suffix: text) -> bool
```

戻る`true`いつ`value`で終わる`suffix`。

### `has_prefix`

```zt
public func has_prefix(value: text, prefix: text) -> bool
```

明示的なセマンティック エイリアス`starts_with`。

### `has_suffix`

```zt
public func has_suffix(value: text, suffix: text) -> bool
```

明示的なセマンティック エイリアス`ends_with`。

### `has_whitespace`

```zt
public func has_whitespace(value: text) -> bool
```

戻る`true`少なくとも 1 つの空白文字がある場合。

### `index_of`

```zt
public func index_of(value: text, needle: text) -> int
```

最初に出現したインデックスを返します。または`-1`。

### `last_index_of`

```zt
public func last_index_of(value: text, needle: text) -> int
```

最後に出現したインデックスを返します。または`-1`。

### `is_empty`

```zt
public func is_empty(value: text) -> bool
```

戻る`true`テキストが空の場合。

### `is_blank`

```zt
public func is_blank(value: text) -> bool
```

戻る`true`テキストが空かスペースのみの場合。

### `is_digits`

```zt
public func is_digits(value: text) -> bool
```

戻る`true`ASCII 数字で構成される空ではないテキストのみ`0-9`。

### `limit`

```zt
public func limit(value: text, max_len: int) -> text
```

テキストを最大まで切り取る`max_len`コードポイントをサフィックスを追加せずに使用します。

