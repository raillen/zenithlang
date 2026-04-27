# 検証、数学、ランダム

> 対象者: ユーザー
> ステータス: 現在
> 表面: 公開
> 真実の情報源: いいえ

＃＃ まとめ

使用：

- `std.validate`明確な述語の場合。
-`std.math`数値演算の場合。
-`std.random`アルファカットオフの擬似乱数の場合。

## 検証

```zt
namespace app.main

import std.validate as validate

public func main() -> int
    if not validate.between(10, 1, 20)
        return 1
    end

    if not validate.not_empty_text("Ada")
        return 2
    end

    if not validate.min_len("Ada", 3)
        return 3
    end

    return 0
end
```

## 数学

```zt
namespace app.main

import std.math as math

public func main() -> int
    const value: float = math.clamp(12.0, 0.0, 10.0)

    if not math.approx_equal(value, 10.0, 0.001)
        return 1
    end

    return 0
end
```

＃＃ランダム

```zt
namespace app.main

import std.random as random

public func main() -> result<int, text>
    random.seed(42)

    const value: int = random.between(1, 10)?

    if value < 1 or value > 10
        return error("random out of range")
    end

    return success(0)
end
```

## ステータスに注意

`std.random`観察可能な公開状態を公開します。

- `random.seeded`;
- `random.last_seed`;
- `random.draw_count`。

他の名前空間はこの状態を読み取ることができますが、直接書き込むことはできません。

使用`random.seed(...)`。

## も参照

- `docs/reference/stdlib/math-random-validate.md`
