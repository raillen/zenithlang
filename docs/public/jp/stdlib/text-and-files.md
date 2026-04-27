# テキストとファイル

> 対象者: ユーザー
> ステータス: 現在
> 表面: 公開
> 真実の情報源: いいえ

＃＃ まとめ

使用`std.text`テキストを変換します。

使用`std.fs`ファイルの読み取りと書き込みを行います。

＃＃ 文章

```zt
namespace app.main

import std.text as text

public func main() -> int
    const raw: text = "  Ada  "
    const name: text = text.trim(raw)

    if text.is_blank(name)
        return 1
    end

    if not text.starts_with(name, "A")
        return 2
    end

    return 0
end
```

## テキストファイル

```zt
namespace app.main

import std.fs as fs

public func main() -> result<void, fs.Error>
    fs.write_text("hello.txt", "hello")?

    const content: text = fs.read_text("hello.txt")?

    if content != "hello"
        return error(fs.Error.Unknown)
    end

    return success()
end
```

## ルール

- `text`そしてUTF-8。
-`bytes`そしてバイナリデータ。
-`fs.*`返品`result`操作が失敗する可能性がある場合。
- 使用`?`関数が戻ったときにエラーを伝播する`result`。

## も参照

- `docs/reference/stdlib/text-bytes-format.md`
- `docs/reference/stdlib/filesystem-os-time.md`
