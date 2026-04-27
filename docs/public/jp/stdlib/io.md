# モジュール`std.io`

## 定数と関数

### `﻿--- @target: namespace
# std.io â€” Entrada e SaÃ­da PadrÃ£o

MÃ³dulo textual sÃ­ncrono para streams padrÃ£o de entrada, saÃ­da e erro.
Usa handles tipados (`io.入力`, `io.出力`) e funÃ§Ãµes livres com parÃ¢metros opcionais `に：` e `から：`.
Todas as operaÃ§Ãµes retornam `結果` com erro tipado `コア.エラー`.
---`

入力

### ````zt
public struct Input
    handle: int
end
```

入力ストリームの型付きハンドル。
デフォルトのハンドル`io.input`標準入力 (stdin) を表します。
---`

Output

### ````zt
public struct Output
    handle: int
end
```

出力ストリームの型付きハンドル。
デフォルトのハンドルは次のとおりです。`io.output`(標準出力) と`io.stderr`(標準エラー)。
---`

Error

### ````zt
public enum Error
    ReadFailed
    WriteFailed
    Unknown
end
```

I/O 操作の入力エラー。
---`

input

### ````zt
public const input: io.Input
```

デフォルトの入力ハンドル (標準入力)。
---`

output

### ````zt
public const output: io.Output
```

デフォルトの出力ハンドル (stdout)。
---`

err

### ````zt
public const err: io.Output
```

エラー出力ハンドル (stderr)。
名前付き`err`コンストラクターとの競合を避けるため`error(...)`の`result`。
---`

read_line

### ````zt
public func read_line(from: io.Input = io.input) -> result<optional<text>, core.Error>
```

単一行の入力を読み取ります。
戻る`none`コンテンツがない状態で EOF に達したとき。
行終端文字 (`\n`、`\r\n`）は削除されます。

@param from – 入力ハンドル (デフォルト:`io.input`）。
@return 行は次のようになります`optional<text>`、または I/O エラー。
---`

read_all

### ````zt
public func read_all(from: io.Input = io.input) -> result<text, core.Error>
```

EOF まで入力時に利用可能なすべてのコンテンツを読み取ります。

@param from – 入力ハンドル (デフォルト:`io.input`）。
@return すべてのテキストが読み取られたか、I/O エラーが発生しました。
---`

write

### ````zt
public func write(value: text, to: io.Output = io.output) -> result<void, core.Error>
```

指定された出力ストリームに生のテキストを書き込みます。
改行は追加しません。

@param value – 書き込まれるテキスト。
@param to – 出力ハンドル (デフォルト:`io.output`）。
@戻る`void`成功時、または I/O エラー。
---`

印刷する

