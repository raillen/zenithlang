# モジュール`std.fs.path`

パス上の字句操作の純粋なモジュール。
ファイルシステムにアクセスせず、シンボリックリンクを解決しません。
すべての操作はテキストベースです。

## 定数と関数

### `join`

```zt
public func join(parts: list<text>) -> text
```

プラットフォーム セパレーターを使用して、複数のパス パーツを結合します。

@param Parts — パス部分のリスト。
@return 結合されたフルパス。

### `base_name`

```zt
public func base_name(value: text) -> text
```

ファイル名 (パスの最後の部分) を返します。

@param 値 — フルパス。
@return ファイル名、または`""`パスが区切り文字で終わっている場合。

### `name_without_extension`

```zt
public func name_without_extension(value: text) -> text
```

拡張子を除いたファイル名を返します。

### `extension`

```zt
public func extension(value: text) -> optional<text>
```

ファイル拡張子を返す、または`none`ない場合は。

@param 値 — パスまたはファイル名。
@return ドットのない拡張子、または`none`。

### `parent`

```zt
public func parent(value: text) -> optional<text>
```

親ディレクトリを返します、または`none`父親がいなかったら。

@param 値 — パス。
@return 親ディレクトリのパス、または`none`。

### `normalize`

```zt
public func normalize(value: text) -> text
```

を解くことでパスを正規化します`.`そして`..`語彙的に。

### `absolute`

```zt
public func absolute(value: text, base: text) -> text
```

を使用して絶対パスに変換します`base`明示的な。
現在のディレクトリを暗黙的に読み取りません。

@param 値 — 相対パス。
@param Base — 解決の明示的な基準。
@return 絶対パス。

### `relative`

```zt
public func relative(value: text, from: text) -> text
```

の相対パスを計算します。`value`から`from`。

### `is_absolute`

```zt
public func is_absolute(value: text) -> bool
```

パスが絶対パスかどうかを確認します。

### `is_relative`

```zt
public func is_relative(value: text) -> bool
```

パスが相対パスかどうかを確認します。

### `has_extension`

```zt
public func has_extension(value: text, expected: text) -> bool
```

パスが予期した長さであるかどうかを確認します。
出発点は、`expected`自動的に処理されます。

@param value — チェックするパス。
@param Expected — 期待される拡張子 (ドットの有無にかかわらず)。
@戻る`true`拡張子が一致する場合。

### `change_extension`

```zt
public func change_extension(value: text, new_ext: text) -> text
```

パス内のファイル拡張子を変更します。
出発点は、`new_ext`自動的に処理されます。

@param 値 — 元のパス。
@param new_ext — 新しい拡張子 (ドットの有無にかかわらず)。
@return 拡張子が変更されたパス。

