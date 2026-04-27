# 部分クラスを使用せずにメソッドを分離する方法

> 対象者: ユーザー
> ステータス: 現在
> 表面: 公開
> 真実の情報源: いいえ

＃＃ 問題

すべてを同じブロックに入れずに、データとメソッドを整理しておきたいと考えています。

C# ではこれは次のようになります`partial class`。

ゼニスでは、`struct`データ用と`apply`行動のために。

## 短い答え

データを宣言します`struct`。

メソッドを追加する`apply Type`。

```zt
struct Player
    hp: int
end

apply Player
    func is_alive() -> bool
        return self.hp > 0
    end
end
```

## 完全な例

```zt
namespace app.main

struct Player
    hp: int
end

apply Player
    mut func heal(amount: int)
        self.hp = self.hp + amount
        return
    end
end

public func main() -> int
    var player: Player = Player(hp: 5)
    player.heal(2)
    return player.hp
end
```

＃＃ なぜ

`apply`データ定義を断片化することなく動作を分離できます。

これは以下を読むのに役立ちます。

- `struct`値の形式を示します。
-`apply Type`固有の動作を示します。
-`apply Trait to Type`契約の履行を示します。

## よくあるエラー

書かないでください:

```zt
partial struct Player
    hp: int
end
```

書く：

```zt
struct Player
    hp: int
end

apply Player
    func is_alive() -> bool
        return self.hp > 0
    end
end
```

## 使用しない場合

理由もなくメソッドをあまりにも多くのファイルに分散させないでください。

使用`apply`行動を隠すことではなく、整理することです。

## も参照

- `docs/public/cookbook/abstract-methods-with-trait.md`
- `language/decisions/010-structs-traits-apply-enums-and-match.md`
