# モジュール`std.math`

## 定数と関数

### `﻿--- @target: namespace
# std.math â€” MatemÃ¡tica NumÃ©rica

MÃ³dulo puro de matemÃ¡tica numÃ©rica.
Inclui aritmÃ©tica escalar, trigonometria, logaritmos, exponenciais, arredondamento e constantes.
---`

円周率

### ````zt
public const pi: float = 3.141592653589793
```

定数円周率 (Ï€)。
---`

e

### ````zt
public const e: float = 2.718281828459045
```

オイラー定数 (e)。
---`

tau

### ````zt
public const tau: float = 6.283185307179586
```

タウ定数 (Ïœ = 2Ï€)。
---`

infinity

### ````zt
public func infinity() -> float
```

正の無限大の IEEE 754 値を返します。
---`

nan

### ````zt
public func nan() -> float
```

IEEE 754 NaN (非数値) 値を返します。
---`

abs

### ````zt
public func abs(value: float) -> float
```

絶対値を返します。
@return 絶対値。
---`

min

### ````zt
public func min(a: float, b: float) -> float
```

2 つの値のうち小さい方を返します。
---`

max

### ````zt
public func max(a: float, b: float) -> float
```

2 つの値の大きい方を返します。
---`

clamp

### ````zt
public func clamp(value: float, min: float, max: float) -> float
```

値を範囲に制限します`[min, max]`。

@param value – 制限する値。
@param min – 下限値 (両端を含む)。
@param max – 上限（両端を含む）。
@return 制限された値。
---`

pow

### ````zt
public func pow(base: float, exponent: float) -> float
```

高める`base`力へ`exponent`。
---`

sqrt

### ````zt
public func sqrt(value: float) -> float
```

平方根。戻る`nan`負の値の場合。
---`

floor

### ````zt
public func floor(value: float) -> float
```

切り捨てます (-â ^ž 方向)。
---`

ceil

### ````zt
public func ceil(value: float) -> float
```

切り上げます (+â ^ž 方向)。
---`

round

### ````zt
public func round(value: float) -> float
```

「ゼロから半分離れた」セマンティクスを使用してラウンドします。
---`

trunc

### ````zt
public func trunc(value: float) -> float
```

小数部分を切り捨てます (0 方向に向かって)。
---`

deg_to_rad

### ````zt
public func deg_to_rad(x: float) -> float
```

度をラジアンに変換します。
---`

rad_to_deg

### ````zt
public func rad_to_deg(x: float) -> float
```

ラジアンを度に変換します。
---`

approx_equal

### ````zt
public func approx_equal(a: float, b: float, epsilon: float) -> bool
```

2 つの float 間のおおよその等しいことをチェックします。

@param a – 最初の値。
@param b – 2 番目の値。
@param epsilon — 最大許容誤差。
@戻る`true`もし`|a - b| <= epsilon`。
---`

sin

### ````zt
public func sin(value: float) -> float
```

ラジアン単位の正弦。
---`

cos

### ````zt
public func cos(value: float) -> float
```

ラジアン単位のコサイン。
---`

tan

### ````zt
public func tan(value: float) -> float
```

ラジアン単位のタンジェント。
---`

asin

### ````zt
public func asin(value: float) -> float
```

アークサイン。戻る`nan`外の価値観に対して`[-1, 1]`。
---`

acos

### ````zt
public func acos(value: float) -> float
```

逆余弦。戻る`nan`外の価値観に対して`[-1, 1]`。
---`

atan

### ````zt
public func atan(value: float) -> float
```

逆正接。
---`

atan2

### ````zt
public func atan2(y: float, x: float) -> float
```

2 引数の逆正接 (象限認識)。

@param y – 垂直コンポーネント。
@param x – 水平コンポーネント。
@return ラジアン単位の角度。
---`

ln

### ````zt
public func ln(value: float) -> float
```

自然対数 (底 e)。
---`

log_ten

### ````zt
public func log_ten(value: float) -> float
```

底が 10 の対数。
---`

log2

### ````zt
public func log2(value: float) -> float
```

底 2 の対数。
---`

log

### ````zt
public func log(value: float, base: float) -> float
```

任意の底の対数。

@param value – 値。
@param base – 対数の底。
@return 対数の結果。
---`

exp

### ````zt
public func exp(value: float) -> float
```

指数関数 (e^ 値)。
---`

is_nan

### ````zt
public func is_nan(value: float) -> bool
```

値が NaN かどうかを確認します。
---`

is_infinite

### ````zt
public func is_infinite(value: float) -> bool
```

値が無限 (正または負) かどうかを確認します。
---`

有限です

