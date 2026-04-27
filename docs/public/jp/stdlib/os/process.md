# モジュール`std.os.process`

子プロセスを実行するためのモジュール。
による明示的な実行`program` + `args`、シェルなし。
ゼロ以外の終了コードは有効な結果であり、エラーではありません。

## 定数と関数

### `ExitStatus`

```zt
public struct ExitStatus
    code: int
end
```

子プロセスの終了ステータス。
コード 0 は従来の成功を示します。他のコードは有効な結果です。

### `CapturedRun`

```zt
public struct CapturedRun
    status: process.ExitStatus
    stdout_text: text
    stderr_text: text
end
```

stdout および stderr からのテキスト キャプチャによる実行結果。

### `Error`

```zt
public enum Error
    NotFound
    PermissionDenied
    IOFailure
    DecodeFailed
    Unknown
end
```

トランスポート レベルのエラー (生成、待機、キャプチャ)。
ゼロ以外の終了コードはありません`process.Error`。

### `run`

```zt
public func run(program: text, args: list<text> = [], cwd: optional<text> = none) -> result<process.ExitStatus, process.Error>
```

プログラムを実行し、完了するまで待ちます。
シェルは使用しません。引数は明示的に渡されます。

@param プログラム — プログラムのパスまたは名前。
@param args — プログラム引数 (デフォルト:`[]`）。
@param cwd — オプションの作業ディレクトリ。
@return 終了ステータス、またはトランスポートエラー。

### `run_capture`

```zt
public func run_capture(program: text, args: list<text> = [], cwd: optional<text> = none) -> result<process.CapturedRun, process.Error>
```

プログラムを実行し、stdout と stderr をテキスト (UTF-8) としてキャプチャします。

@param プログラム — プログラムのパスまたは名前。
@param args — プログラム引数 (デフォルト:`[]`）。
@param cwd — オプションの作業ディレクトリ。
@return キャプチャされた結果、または転送エラー。

