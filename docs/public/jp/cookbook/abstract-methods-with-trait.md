# 抽象メソッドの作り方`trait`

> 対象者: ユーザー
> ステータス: 現在
> 表面: 公開
> 真実の情報源: いいえ

＃＃ 問題

複数の型が実装する必要がある動作を宣言したいとします。

ゼニスでは、`trait`。

使用しないでください`abstract class`。

## 短い答え

～との契約を宣言する`trait`。

で実装`apply Trait to Type`。

```zt
trait Scoreable
    func score() -> int
end
```

## 完全な例

```zt
namespace app.main

trait Scoreable
    func score() -> int
end

struct Player
    hp: int
end

apply Scoreable to Player
    func score() -> int
        return self.hp + 1
    end
end

public func main() -> int
    const player: Player = Player(hp: 5)
    return player.score()
end
```

＃＃ なぜ

`trait`意図を明確にします:

- 型はその動作を提供する必要があります。
- 実装は別個です。
- 隠された古典的な遺産はありません。

## よくあるエラー

書かないでください:

```zt
abstract class Scoreable
    abstract func score() -> int
end
```

書く：

```zt
trait Scoreable
    func score() -> int
end
```

## 使用しない場合

使用しないでください`trait`具体的なタイプが 1 つだけで、共有コントラクトがない場合。

この場合、固有のメソッドを使用します。`apply Type`。

## も参照

- `docs/public/cookbook/partial-class-with-apply.md`
- `docs/public/cookbook/virtual-dispatch-with-dyn.md`
- `language/decisions/010-structs-traits-apply-enums-and-match.md`
