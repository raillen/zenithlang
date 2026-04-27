# モジュール`std.os`

現在のプロセスのクエリと状態変更用のモジュール。
子プロセスはそのまま残ります`std.os.process`。

## 定数と関数

### `Platform`

```zt
public enum Platform
    Windows
    Linux
    MacOS
    Unknown
end
```

オペレーティング システム プラットフォーム。
使用`os.platform()`現在の値を取得します。

### `Arch`

```zt
public enum Arch
    X64
    X86
    Arm64
    Unknown
end
```

プロセッサーのアーキテクチャ。
使用`os.arch()`現在の値を取得します。

### `Error`

```zt
public enum Error
    NotFound
    PermissionDenied
    IOError
    Unknown
end
```

オペレーティング システム操作の入力エラー。

### `args`

```zt
public func args() -> list<text>
```

プロセス引数のベクトルを返します。

### `env`

```zt
public func env(name: text) -> optional<text>
```

環境変数をクエリします。
戻る`none`変数が存在しない場合。

@param name — 変数名。
@return 変数値、または`none`。

### `pid`

```zt
public func pid() -> int
```

現在のプロセスの PID を返します。

### `platform`

```zt
public func platform() -> os.Platform
```

オペレーティング システムのプラットフォームを返します。

### `arch`

```zt
public func arch() -> os.Arch
```

プロセッサのアーキテクチャを返します。

### `current_dir`

```zt
public func current_dir() -> result<text, os.Error>
```

現在の作業ディレクトリを返します。

### `change_dir`

```zt
public func change_dir(path: text) -> result<void, os.Error>
```

現在の作業ディレクトリを変更します。

@param パス — 新しい作業ディレクトリ。
@戻る`void`成功かエラーか。

