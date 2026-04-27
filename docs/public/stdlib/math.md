# Módulo `std.math`

## Constantes e Funções

### `﻿--- @target: namespace
# std.math â€” MatemÃ¡tica NumÃ©rica

MÃ³dulo puro de matemÃ¡tica numÃ©rica.
Inclui aritmÃ©tica escalar, trigonometria, logaritmos, exponenciais, arredondamento e constantes.
---`

pi

### ````zt
public const pi: float = 3.141592653589793
```

Constante pi (Ï€).
---`

e

### ````zt
public const e: float = 2.718281828459045
```

Constante de Euler (e).
---`

tau

### ````zt
public const tau: float = 6.283185307179586
```

Constante tau (Ï„ = 2Ï€).
---`

infinity

### ````zt
public func infinity() -> float
```

Retorna o valor IEEE 754 de infinito positivo.
---`

nan

### ````zt
public func nan() -> float
```

Retorna o valor IEEE 754 NaN (Not a Number).
---`

abs

### ````zt
public func abs(value: float) -> float
```

Retorna o valor absoluto.
@return Valor absoluto.
---`

min

### ````zt
public func min(a: float, b: float) -> float
```

Retorna o menor entre dois valores.
---`

max

### ````zt
public func max(a: float, b: float) -> float
```

Retorna o maior entre dois valores.
---`

clamp

### ````zt
public func clamp(value: float, min: float, max: float) -> float
```

Limita um valor ao intervalo `[min, max]`.

@param value â€” Valor a limitar.
@param min â€” Limite inferior (inclusivo).
@param max â€” Limite superior (inclusivo).
@return Valor limitado.
---`

pow

### ````zt
public func pow(base: float, exponent: float) -> float
```

Eleva `base` Ã  potÃªncia `exponent`.
---`

sqrt

### ````zt
public func sqrt(value: float) -> float
```

Raiz quadrada. Retorna `nan` para valores negativos.
---`

floor

### ````zt
public func floor(value: float) -> float
```

Arredonda para baixo (em direÃ§Ã£o a -âˆž).
---`

ceil

### ````zt
public func ceil(value: float) -> float
```

Arredonda para cima (em direÃ§Ã£o a +âˆž).
---`

round

### ````zt
public func round(value: float) -> float
```

Arredonda usando semÃ¢ntica "half away from zero".
---`

trunc

### ````zt
public func trunc(value: float) -> float
```

Trunca a parte fracionÃ¡ria (em direÃ§Ã£o a zero).
---`

deg_to_rad

### ````zt
public func deg_to_rad(x: float) -> float
```

Converte graus para radianos.
---`

rad_to_deg

### ````zt
public func rad_to_deg(x: float) -> float
```

Converte radianos para graus.
---`

approx_equal

### ````zt
public func approx_equal(a: float, b: float, epsilon: float) -> bool
```

Verifica igualdade aproximada entre dois floats.

@param a â€” Primeiro valor.
@param b â€” Segundo valor.
@param epsilon â€” TolerÃ¢ncia mÃ¡xima.
@return `true` se `|a - b| <= epsilon`.
---`

sin

### ````zt
public func sin(value: float) -> float
```

Seno em radianos.
---`

cos

### ````zt
public func cos(value: float) -> float
```

Cosseno em radianos.
---`

tan

### ````zt
public func tan(value: float) -> float
```

Tangente em radianos.
---`

asin

### ````zt
public func asin(value: float) -> float
```

Arco-seno. Retorna `nan` para valores fora de `[-1, 1]`.
---`

acos

### ````zt
public func acos(value: float) -> float
```

Arco-cosseno. Retorna `nan` para valores fora de `[-1, 1]`.
---`

atan

### ````zt
public func atan(value: float) -> float
```

Arco-tangente.
---`

atan2

### ````zt
public func atan2(y: float, x: float) -> float
```

Arco-tangente de dois argumentos (quadrant-aware).

@param y â€” Componente vertical.
@param x â€” Componente horizontal.
@return Ã‚ngulo em radianos.
---`

ln

### ````zt
public func ln(value: float) -> float
```

Logaritmo natural (base e).
---`

log_ten

### ````zt
public func log_ten(value: float) -> float
```

Logaritmo base 10.
---`

log2

### ````zt
public func log2(value: float) -> float
```

Logaritmo base 2.
---`

log

### ````zt
public func log(value: float, base: float) -> float
```

Logaritmo em base arbitrÃ¡ria.

@param value â€” Valor.
@param base â€” Base do logaritmo.
@return Resultado do logaritmo.
---`

exp

### ````zt
public func exp(value: float) -> float
```

Exponencial (e^value).
---`

is_nan

### ````zt
public func is_nan(value: float) -> bool
```

Verifica se o valor Ã© NaN.
---`

is_infinite

### ````zt
public func is_infinite(value: float) -> bool
```

Verifica se o valor Ã© infinito (positivo ou negativo).
---`

is_finite

