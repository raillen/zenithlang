# なしで欠席を表現する方法`null`

> 対象者: ユーザー
> ステータス: 現在
> 表面: 公開
> 真実の情報源: いいえ

＃＃ 問題

値が存在しない可能性があるということです。

多くの言語ではこれは次のようになります`null`。

ゼニスでは、`optional<T>`そして`none`。

## 短い答え

使用`optional<T>`欠勤が普通のとき。

```zt
func find_name(id: int) -> optional<text>
    if id == 1
        return "Ada"
    end

    return none
end
```

## 完全な例

```zt
namespace app.main

func find_name(id: int) -> optional<text>
    if id == 1
        return "Ada"
    end

    return none
end

func display_name(id: int) -> text
    const maybe_name: optional<text> = find_name(id)

    match maybe_name
        case value name ->
            return name
        case none ->
            return "anonymous"
    end
end

public func main() -> int
    if display_name(2) != "anonymous"
        return 1
    end

    return 0
end
```

＃＃ なぜ

`optional<T>`型の中に不在が見えるままになります。

関数を呼び出す人は両方のパスを処理する必要があります。

- 現在価値;
- 欠損値。

これにより、次のような原因で隠されたエラーが回避されます。`null`。

## よくあるエラー

書かないでください:

```zt
func find_name(id: int) -> text
    return null
end
```

書く：

```zt
func find_name(id: int) -> optional<text>
    return none
end
```

## 使用しない場合

使用しないでください`optional<T>`理由のある回復可能なエラーの場合。

失敗した理由を言う必要がある場合は、次を使用します。`result<T, E>`。

## も参照

- `docs/public/cookbook/errors-without-exceptions.md`
- `docs/reference/language/errors-and-results.md`
- `language/spec/surface-syntax.md`
