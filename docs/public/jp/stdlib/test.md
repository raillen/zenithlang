# モジュール`std.test`

テスト用の補完モジュール。
テストは次のように宣言されます`attr test`。
使用`check(...)`単純な条件の場合。
使用`std.test`失敗したい場合は、スキップするか、より明確なメッセージで値を比較します。

## 定数と関数

### `fail`

```zt
public func fail(message: text = "test failed") -> void
```

現在のテストを直ちに失敗としてマークします。
フロー制御ブランチで役立ちます。たとえば、`match`の上`result`。

@param メッセージ - 失敗メッセージ。

### `skip`

```zt
public func skip(reason: text = "") -> void
```

現在のテストをスキップ済みとしてマークします。
スキップは結果が異なり、合格でも不合格でもありません。

@paramreason - スキップの理由。

### `is_true`

```zt
public func is_true(value: bool) -> void
```

次の場合は失敗します。`value`のために`false`。
メッセージには次のことが表示されます`expected true`そして`received false`。

### `is_false`

```zt
public func is_false(value: bool) -> void
```

次の場合は失敗します。`value`のために`true`。
メッセージには次のことが表示されます`expected false`そして`received true`。

### `equal_int`

```zt
public func equal_int(actual: int, expected: int) -> void
```

次の場合は失敗します。`actual != expected`。
メッセージには「Expected」と「Received」が表示されます。

### `equal_text`

```zt
public func equal_text(actual: text, expected: text) -> void
```

次の場合は失敗します。`actual != expected`。
メッセージには「Expected」と「Received」が表示されます。

### `not_equal_int`

```zt
public func not_equal_int(actual: int, expected: int) -> void
```

次の場合は失敗します。`actual == expected`。
メッセージは、値が異なる必要があることを示しています。

### `not_equal_text`

```zt
public func not_equal_text(actual: text, expected: text) -> void
```

次の場合は失敗します。`actual == expected`。
メッセージは、値が異なる必要があることを示しています。

