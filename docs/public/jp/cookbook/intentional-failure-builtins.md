# check、todo、到達不能による意図的な失敗

> 対象者: ユーザー
> ステータス: 現在
> 表面: 公開
> 真実の情報源: いいえ

＃＃ 問題

エラーが発生してプログラムを停止し、明確なメッセージが表示されるようにしたいとします。

しかし、それぞれのケースには異なる意図があります。

## 短い答え

使用：

- `check(condition, message)`内部条件が真である必要がある場合。
-`todo(message)`パスがまだ実装されていない場合。
-`unreachable(message)`その道が不可能であるはずのとき。
-`panic(message)`一般的な致命的な欠陥の場合。

## 推奨コード

```zt
namespace app.main

func score(value: int) -> int
    check(value > 0, "score must be positive")
    return value * 2
end

func load_bonus() -> int
    todo("load bonus from profile")
    return 0
end

func mode_to_score(mode: text) -> int
    if mode == "easy"
        return 1
    end

    if mode == "hard"
        return 2
    end

    unreachable("unknown mode")
    return 0
end
```

＃＃ 説明

`check`検証を伝えます。

`todo`未完了の作業を報告します。

`unreachable`フロー制御違反を報告します。

これらの名前は、デバッグ前に失敗の理由を読み取るのに役立ちます。

## よくあるエラー

使用しないでください`todo`予想されるビジネスエラーの場合。

```zt
func find_user(id: int) -> result<text, text>
    if id < 0
        return error("invalid id")
    end

    todo("database lookup")
    return success("Ada")
end
```

ザ`todo`上記は、検索がまだ存在しない場合にのみ意味を持ちます。

検索が存在する場合、通常の欠勤は次のようになります。`optional<T>`または`result<T,E>`。

## 使用しない場合

これらの組み込み機能は通常のフローには使用しないでください。

使用`optional<T>`欠損値が予期される場合。

使用`result<T,E>`呼び出し側が障害を処理できるとき。