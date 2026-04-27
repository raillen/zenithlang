# 安全なユニオンを作成する方法`enum`ペイロードあり

> 対象者: ユーザー
> ステータス: 現在
> 表面: 公開
> 真実の情報源: いいえ

＃＃ 問題

さまざまな形式を使用できる値が必要です。

Zenithでのおすすめの方法と`enum`ペイロード付き。

キーワードを作成しないでください`union`そのために。

## 短い答え

使用`enum`データが入るケース付き。

```zt
enum Shape
    Circle(radius: int)
    Rectangle(width: int, height: int)
    Point
end
```

## 完全な例

```zt
namespace app.main

enum Shape
    Circle(radius: int)
    Rectangle(width: int, height: int)
    Point
end

func area(shape: Shape) -> int
    match shape
        case Shape.Circle(radius) ->
            return radius * radius
        case Shape.Rectangle(width, height) ->
            return width * height
        case Shape.Point ->
            return 0
    end
end

public func main() -> int
    const shape: Shape = Shape.Rectangle(width: 5, height: 4)
    return area(shape)
end
```

＃＃ なぜ

`enum`ペイロードを使用すると、次のことが明確になります。

- 値が取り得る形式。
- 各形状がどのようなデータを保持するか。
- コードが各ケースを処理する場所。

ザ`match`流れが見えるようになります。

## よくあるエラー

次のような形状を探さないでください。

```zt
union Shape = Circle | Rectangle | Point
```

使用：

```zt
enum Shape
    Circle(radius: int)
    Rectangle(width: int, height: int)
    Point
end
```

## 使用しない場合

使用しないでください`enum`多くのタイプ間で動作を共有するためのペイロードを備えています。

動作については、次を使用します。`trait`。

動的ディスパッチの場合は、次を使用します。`dyn<Trait>`。

## も参照

- `docs/public/cookbook/abstract-methods-with-trait.md`
- `docs/public/cookbook/virtual-dispatch-with-dyn.md`
- `docs/reference/language/types.md`
- `language/decisions/029-executable-enums-with-payload.md`
