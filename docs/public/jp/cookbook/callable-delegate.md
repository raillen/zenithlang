# 関数を値として渡す方法

> 対象者: ユーザー
> ステータス: 現在
> 表面: 公開
> 真実の情報源: いいえ

＃＃ 問題

関数をコードの別の部分に渡したいとします。

Zenith では、呼び出し可能な型を使用します。

```zt
func(int, int) -> int
```

## 短い答え

署名を明示的に記載します。

```zt
const op: func(int, int) -> int = add
```

## 完全な例

```zt
namespace app.main

func add(a: int, b: int) -> int
    return a + b
end

func run_op(op: func(int, int) -> int) -> int
    return op(3, 4)
end

public func main() -> int
    const op: func(int, int) -> int = add
    return run_op(op)
end
```

＃＃ なぜ

呼び出し可能な型は次を示します。

- 関数が受け取るパラメータの数。
- どのような型を受け取りますか。
- 返されるタイプ。

これにより、非表示のコールバックが回避されます。

## よくあるエラー

このカットでは callable のような汎用関数を使用しないでください。

単純なトップレベル関数を好みます。

```zt
func add(a: int, b: int) -> int
    return a + b
end
```

## 電流制限

Callables v1 は意図的に狭いです。

このカットでは、次のことは避けてください。

- 構造体フィールドとして呼び出し可能。
- リスト/マップ内で呼び出し可能。
- 部分的な適用;
- 値としてのメソッド。
- 値としての汎用関数。

## も参照

- `language/spec/callables.md`
- `language/decisions/089-callable-delegates-v1.md`
