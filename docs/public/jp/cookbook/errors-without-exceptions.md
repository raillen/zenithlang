# 例外なくエラーを表現する方法

> 対象者: ユーザー
> ステータス: 現在
> 表面: 公開
> 真実の情報源: いいえ

＃＃ 問題

回復可能な障害を表現したいと考えています。

Zenith では、回復可能なエラーは次のようになります。`result<T, E>`。

`panic(...)`致命的なエラーになります。

## 短い答え

使用`result<T, E>`発信者が何をすべきかを決定しなければならないとき。

```zt
func parse_score(value: text) -> result<int, text>
    if value == ""
        return error("score is empty")
    end

    return success(10)
end
```

## 完全な例

```zt
namespace app.main

func parse_score(value: text) -> result<int, text>
    if value == ""
        return error("score is empty")
    end

    return success(10)
end

func score_or_zero(value: text) -> int
    const parsed: result<int, text> = parse_score(value)

    match parsed
        case success(score) ->
            return score
        case error(message) ->
            return 0
    end
end

public func main() -> int
    return score_or_zero("")
end
```

＃＃ なぜ

`result<T, E>`成功と失敗をタイプ別に分けます。

これは以下を読むのに役立ちます。

- `success(value)`価値を運ぶ。
-`error(reason)`失敗の原因を抱えています。

## よくあるエラー

使用しないでください`panic`通常のビジネス フローの場合:

```zt
func parse_score(value: text) -> int
    if value == ""
        panic("score is empty")
    end

    return 10
end
```

好む：

```zt
func parse_score(value: text) -> result<int, text>
    if value == ""
        return error("score is empty")
    end

    return success(10)
end
```

## 使用しない場合

使用`optional<T>`欠勤が通常であり、理由を必要としない場合。

使用`panic(...)`プログラムが致命的または不可能な状態に遭遇したとき。

## も参照

- `docs/public/cookbook/absence-without-null.md`
- `docs/reference/language/errors-and-results.md`
- `language/decisions/037-panic-fatal-errors-and-attempt.md`
