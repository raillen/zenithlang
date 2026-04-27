# モジュール`std.concurrent`

アイソレート間のコピーを明示的に行うための小さなモジュール。
スレッドは作成されません。
彼は暴露しない`jobs`まだ。

## 定数と関数

### `copy_int`

```zt
public func copy_int(value: int) -> int
```

同じものを返します`int`。
スカラーはすでに自然に転送可能です。

### `copy_bool`

```zt
public func copy_bool(value: bool) -> bool
```

同じものを返します`bool`。

### `copy_float`

```zt
public func copy_float(value: float) -> float
```

同じものを返します`float`。

### `copy_text`

```zt
public func copy_text(value: text) -> text
```

の安全なコピーを作成します`text`境界線を分離する場合。

### `copy_bytes`

```zt
public func copy_bytes(value: bytes) -> bytes
```

の安全なコピーを作成します`bytes`境界線を分離する場合。

### `copy_list_int`

```zt
public func copy_list_int(value: list<int>) -> list<int>
```

の安全なコピーを作成します`list<int>`。

### `copy_list_text`

```zt
public func copy_list_text(value: list<text>) -> list<text>
```

の安全なコピーを作成します`list<text>`。

### `copy_map_text_text`

```zt
public func copy_map_text_text(value: map<text,text>) -> map<text,text>
```

の安全なコピーを作成します`map<text,text>`。
現在のアルファ版では、これはダウンロード可能なマップの最初のパブリック ブロックです。

