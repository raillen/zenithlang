# 仮想ディスパッチを行う方法`dyn<Trait>`

> 対象者: ユーザー
> ステータス: 現在
> 表面: 公開
> 真実の情報源: いいえ

＃＃ 問題

同じコントラクトの背後に異なる型を保存したいとします。

クラスのある言語では、これは多くの場合、`virtual`。

ゼニスでは、`dyn<Trait>`。

## 短い答え

使用`dyn Trait`異質性が現実にあるとき。

```zt
const shape: dyn Shape = circle
```

## 完全な例

```zt
namespace app.main

trait Shape
    func area() -> int
end

struct Circle
    radius: int
end

apply Shape to Circle
    func area() -> int
        return 3 * self.radius * self.radius
    end
end

struct Rect
    w: int
    h: int
end

apply Shape to Rect
    func area() -> int
        return self.w * self.h
    end
end

public func main() -> int
    const circle: Circle = Circle(radius: 10)
    const rect: Rect = Rect(w: 5, h: 4)

    const a: dyn Shape = circle
    const b: dyn Shape = rect

    return a.area() + b.area()
end
```

＃＃ なぜ

`dyn<Trait>`タイプのコストと意図を示します。

コードを読めば、呼び出しが動的ディスパッチを経由することがわかります。

## よくあるエラー

言葉を探さないでください`virtual`:

```zt
virtual func area() -> int
```

使用：

```zt
trait Shape
    func area() -> int
end
```

その後：

```zt
const shape: dyn Shape = circle
```

## 使用しない場合

使用しないでください`dyn`すべての値が同じ具象型を持つかどうか。

この場合、具象型または制約付きのジェネリック関数を優先します。

```zt
func draw_one<T>(shape: T) -> int
where T is Shape
```

このルールを使用します。

- 実際の異質性:`dyn<Trait>`;
- 具象型はまだ知られています: 汎用`where`。

## 電流制限

現在のサブセット`dyn<Trait>`限界があります。

例えば：

- 一般的な特性は現在の動的サブセットに入りません。
- dyn 形質の突然変異方法は限られています。
- パラメータ/戻り値の型はサブセット規則に従う必要があります。

## も参照

- `docs/public/cookbook/abstract-methods-with-trait.md`
- `docs/reference/language/types.md`
- `language/spec/dyn-dispatch.md`
