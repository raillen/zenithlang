# Module`std.math`

## Constants and Functions

### `﻿--- @target: namespace
# std.math â€” MatemÃ¡tica NumÃ©rica

MÃ³dulo puro de matemÃ¡tica numÃ©rica.
Inclui aritmÃ©tica escalar, trigonometria, logaritmos, exponenciais, arredondamento e constantes.
---`

pi

### ````zt
public const pi: float = 3.141592653589793
```

Constant pi (Ï€).
---`

e

### ````zt
public const e: float = 2.718281828459045
```

Euler constant (e).
---`

tau

### ````zt
public const tau: float = 6.283185307179586
```

Tau constant (Ï„ = 2Ï€).
---`

infinity

### ````zt
public func infinity() -> float
```

Returns the IEEE 754 value of positive infinity.
---`

nan

### ````zt
public func nan() -> float
```

Returns the IEEE 754 NaN (Not a Number) value.
---`

abs

### ````zt
public func abs(value: float) -> float
```

Returns the absolute value.
@return Absolute value.
---`

min

### ````zt
public func min(a: float, b: float) -> float
```

Returns the smaller of two values.
---`

max

### ````zt
public func max(a: float, b: float) -> float
```

Returns the greater of two values.
---`

clamp

### ````zt
public func clamp(value: float, min: float, max: float) -> float
```

Limits a value to the range`[min, max]`.

@param value – Value to limit.
@param min – Lower limit (inclusive).
@param max – Upper limit (inclusive).
@return Limited value.
---`

pow

### ````zt
public func pow(base: float, exponent: float) -> float
```

Elevate`base`To the power`exponent`.
---`

sqrt

### ````zt
public func sqrt(value: float) -> float
```

Square root. Return`nan`for negative values.
---`

floor

### ````zt
public func floor(value: float) -> float
```

Rounds down (towards -âˆž).
---`

ceil

### ````zt
public func ceil(value: float) -> float
```

Rounds up (towards +âˆž).
---`

round

### ````zt
public func round(value: float) -> float
```

Rounds using "half away from zero" semantics.
---`

trunc

### ````zt
public func trunc(value: float) -> float
```

Truncates the fractional part (towards zero).
---`

deg_to_rad

### ````zt
public func deg_to_rad(x: float) -> float
```

Converts degrees to radians.
---`

rad_to_deg

### ````zt
public func rad_to_deg(x: float) -> float
```

Converts radians to degrees.
---`

approx_equal

### ````zt
public func approx_equal(a: float, b: float, epsilon: float) -> bool
```

Checks approximate equality between two floats.

@param a – First value.
@param b – Second value.
@param epsilon — Maximum tolerance.
@return`true`if`|a - b| <= epsilon`.
---`

sin

### ````zt
public func sin(value: float) -> float
```

Sine in radians.
---`

cos

### ````zt
public func cos(value: float) -> float
```

Cosine in radians.
---`

tan

### ````zt
public func tan(value: float) -> float
```

Tangent in radians.
---`

asin

### ````zt
public func asin(value: float) -> float
```

Arcsine. Return`nan`for values ​​outside`[-1, 1]`.
---`

acos

### ````zt
public func acos(value: float) -> float
```

Arccosine. Return`nan`for values ​​outside`[-1, 1]`.
---`

atan

### ````zt
public func atan(value: float) -> float
```

Arctangent.
---`

atan2

### ````zt
public func atan2(y: float, x: float) -> float
```

Two-argument arctangent (quadrant-aware).

@param y â€” Vertical component.
@param x – Horizontal component.
@return Angle in radians.
---`

ln

### ````zt
public func ln(value: float) -> float
```

Natural logarithm (base e).
---`

log_ten

### ````zt
public func log_ten(value: float) -> float
```

Logarithm base 10.
---`

log2

### ````zt
public func log2(value: float) -> float
```

Base 2 logarithm.
---`

log

### ````zt
public func log(value: float, base: float) -> float
```

Logarithm in arbitrary base.

@param value – Value.
@param base – Base of the logarithm.
@return Result of the logarithm.
---`

exp

### ````zt
public func exp(value: float) -> float
```

Exponential (e^value).
---`

is_nan

### ````zt
public func is_nan(value: float) -> bool
```

Checks if the value is NaN.
---`

is_infinite

### ````zt
public func is_infinite(value: float) -> bool
```

Checks whether the value is infinite (positive or negative).
---`

is_finite

