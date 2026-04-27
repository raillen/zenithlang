# モジュール`std.json`

このモジュールは、現在のランタイムにおける JSON の安定したサブセットをカバーします。

現在の範囲:
- キーを含む JSON オブジェクト`text`と価値観`text`- 天頂での表現:`map<text,text>`

現時点では範囲外です:
- 一般的な配列
- 入力された数字
- リッチモデルの bool/null
- 型付き JSON ツリー (`Value/Object/Array`）

## 定数と関数

### `parse`

```zt
public func parse(input: text) -> result<map<text,text>, core.Error>
```

JSON テキストを次のように変換します`map<text,text>`。

@param は JSON テキストを入力します。
@return 成功した場合はマップ、またはエラーが発生した場合はマップ`core.Error`。

### `stringify`

```zt
public func stringify(value: map<text,text>) -> text
```

変換する`map<text,text>`圧縮された JSON に変換します。

### `pretty`

```zt
public func pretty(value: map<text,text>, indent: int = 2) -> text
```

変換する`map<text,text>`フォーマットされた JSON に変換します。

@param indent レベルごとのスペース (デフォルト: 2)。

### `read`

```zt
public func read(file_path: text) -> result<map<text,text>, core.Error>
```

1 回の呼び出しでファイルを読み取り、解析します。

### `write`

```zt
public func write(file_path: text, value: map<text,text>) -> result<void, core.Error>
```

シリアル化してファイルに書き込みます。

