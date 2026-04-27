# モジュール`std.fs`

同期ファイルシステムモジュール。
パスは`text`。
パス操作は継続します`std.fs.path`。

## 定数と関数

### `Error`

```zt
public enum Error
    NotFound
    PermissionDenied
    AlreadyExists
    NotADirectory
    IsADirectory
    IOError
    InvalidPath
    Unknown
end
```

ファイルシステム操作の入力エラー。

### `Metadata`

```zt
public struct Metadata
    size_bytes: int
    modified_at_ms: int
    created_at_ms: optional<int>
    is_file: bool
    is_dir: bool
end
```

現在のアルファ版で公開されているメタデータ。
タイムスタンプは Unix ミリ秒を使用します。

### `read_text`

```zt
public func read_text(path: text) -> result<text, fs.Error>
```

UTF-8 テキスト ファイルを読み取ります。

### `write_text`

```zt
public func write_text(path: text, content: text) -> result<void, fs.Error>
```

テキストを書き込み、以前のコンテンツを置き換えます。

### `append_text`

```zt
public func append_text(path: text, content: text) -> result<void, fs.Error>
```

ファイルの末尾にテキストを追加します。

### `exists`

```zt
public func exists(path: text) -> result<bool, fs.Error>
```

パスが存在するかどうかを確認します。

### `is_file`

```zt
public func is_file(path: text) -> result<bool, fs.Error>
```

パスがファイルを指しているかどうかを確認します。

### `is_dir`

```zt
public func is_dir(path: text) -> result<bool, fs.Error>
```

パスがディレクトリを指しているかどうかを確認します。

### `create_dir`

```zt
public func create_dir(path: text) -> result<void, fs.Error>
```

ディレクトリを作成します。

### `create_dir_all`

```zt
public func create_dir_all(path: text) -> result<void, fs.Error>
```

ディレクトリと必要な国を作成します。

### `list_dir`

```zt
public func list_dir(path: text) -> result<list<text>, fs.Error>
```

ディレクトリ内のエントリの名前をリストします。

### `remove_file`

```zt
public func remove_file(path: text) -> result<void, fs.Error>
```

ファイルを削除します。

### `remove_dir`

```zt
public func remove_dir(path: text) -> result<void, fs.Error>
```

空のディレクトリを削除します。

### `remove_dir_all`

```zt
public func remove_dir_all(path: text) -> result<void, fs.Error>
```

ディレクトリを再帰的に削除します。

### `copy_file`

```zt
public func copy_file(from: text, to: text) -> result<void, fs.Error>
```

ファイルをコピーします。

### `move`

```zt
public func move(from: text, to: text) -> result<void, fs.Error>
```

エントリを移動または名前変更します。

### `metadata`

```zt
public func metadata(path: text) -> result<fs.Metadata, fs.Error>
```

エントリの完全なメタデータを返します。

### `size`

```zt
public func size(path: text) -> result<int, fs.Error>
```

サイズをバイト単位で返します。

### `modified_at`

```zt
public func modified_at(path: text) -> result<int, fs.Error>
```

変更タイムスタンプを Unix ミリ秒単位で返します。

### `created_at`

```zt
public func created_at(path: text) -> result<optional<int>, fs.Error>
```

使用可能な場合は、作成タイムスタンプを Unix ミリ秒単位で返します。

