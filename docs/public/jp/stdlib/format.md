# モジュール`std.format`

プレゼンテーション用の純粋な書式設定モジュール。
戻る`text`また、I/O、解析、シリアル化は実行されません。
補完物`fmt`そして`TextRepresentable<T>`。

## 定数と関数

### `BytesStyle`

```zt
public enum BytesStyle
    Binary
    Decimal
end
```

サイズをバイト単位でフォーマットするための型付きスタイル。`Binary`ベース 1024 (KiB、MiB) を使用します。`Decimal`1000 進数 (KB、MB) を使用します。

### `number`

```zt
public func number(value: float, decimals: int = 0) -> text
```

構成可能な小数精度で数値をフォーマットします。

@param value — フォーマットする数値。
@param decimals — 小数点以下の桁数 (デフォルト: 0)。
@return 数値の書式設定されたテキスト。

### `percent`

```zt
public func percent(value: float, decimals: int = 0) -> text
```

値をパーセンテージとしてフォーマットします。`percent(0.25)`生成する`"25%"`。

@param value — フォーマットする小数点 (0.0 ～ 1.0)。
@param decimals — 小数点以下の桁数 (デフォルト: 0)。
@return パーセントでフォーマットされたテキスト。

### `date`

```zt
public func date(millis: int, style: text = "iso") -> text
```

名前付きスタイルを使用して、エポックミリ秒からの日付をフォーマットします。

@param millis — エポックからのミリ秒単位のタイムスタンプ。
@param スタイル — 書式設定スタイル:`"iso"`、`"short"`、`"long"`。
@return 書式設定された日付テキスト。

### `datetime`

```zt
public func datetime(millis: int, style: text = "short", locale: text = "") -> text
```

日付と時刻をエポックミリ秒でフォーマットします。
ロケールは使用時に明示的に指定されます。

@param millis — エポックからのミリ秒単位のタイムスタンプ。
@param style — 書式設定スタイル。
@param locale — 明示的なロケール (例:`"pt-BR"`）。デフォルトでは空です。
@return 書式設定された日付/時刻テキスト。

### `date_pattern`

```zt
public func date_pattern(millis: int, pattern: text) -> text
```

明示的なパターンを使用して日付をフォーマットします。

@param millis — エポックからのミリ秒単位のタイムスタンプ。
@param パターン — 書式設定パターン (例:`"yyyy-mm-dd"`）。
@return 標準に従ってフォーマットされた日付テキスト。

### `datetime_pattern`

```zt
public func datetime_pattern(millis: int, pattern: text) -> text
```

明示的なパターンを使用して日付と時刻をフォーマットします。

@param millis — エポックからのミリ秒単位のタイムスタンプ。
@param パターン — 書式設定パターン (例:`"yyyy-mm-dd HH:mm:ss"`）。
@return 標準に従ってフォーマットされた日付/時刻テキスト。

### `bytes`

```zt
public func bytes(value: int, style: format.BytesStyle = format.BytesStyle.Binary, decimals: int = 1) -> text
```

読みやすい表示のためにサイズをバイト単位でフォーマットします。

@param value — バイト単位のサイズ。
@param スタイル —`Binary`(基数 1024) または`Decimal`(基数 1000)。
@param decimals — 小数点以下の桁数 (デフォルト: 1)。
@return フォーマットされたテキスト (例:`"1.5 KiB"`、`"1.5 KB"`）。

### `hex`

```zt
public func hex(value: int) -> text
```

整数を 16 進表現でフォーマットします。

@param value — 整数値。
@return 16 進数のテキスト。

### `bin`

```zt
public func bin(value: int) -> text
```

整数をバイナリ表現にフォーマットします。

@param value — 整数値。
@return バイナリのテキスト。

