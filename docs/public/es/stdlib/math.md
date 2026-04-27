# Módulo`std.math`

## Constantes y funciones

### `﻿--- @target: namespace
# std.math â€” MatemÃ¡tica NumÃ©rica

MÃ³dulo puro de matemÃ¡tica numÃ©rica.
Inclui aritmÃ©tica escalar, trigonometria, logaritmos, exponenciais, arredondamento e constantes.
---`

pi

### ````zt
public const pi: float = 3.141592653589793
```

Pi constante (Ï€).
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

Devuelve el valor IEEE 754 de infinito positivo.
---`

nan

### ````zt
public func nan() -> float
```

Devuelve el valor IEEE 754 NaN (no es un número).
---`

abs

### ````zt
public func abs(value: float) -> float
```

Devuelve el valor absoluto.
@return Valor absoluto.
---`

min

### ````zt
public func min(a: float, b: float) -> float
```

Devuelve el menor de dos valores.
---`

max

### ````zt
public func max(a: float, b: float) -> float
```

Devuelve el mayor de dos valores.
---`

clamp

### ````zt
public func clamp(value: float, min: float, max: float) -> float
```

Limita un valor al rango`[min, max]`.

Valor @param: valor a limitar.
@param min: límite inferior (inclusive).
@param max: límite superior (inclusive).
@return Valor limitado.
---`

pow

### ````zt
public func pow(base: float, exponent: float) -> float
```

Elevar`base`al poder`exponent`.
---`

sqrt

### ````zt
public func sqrt(value: float) -> float
```

Raíz cuadrada. Devolver`nan`para valores negativos.
---`

floor

### ````zt
public func floor(value: float) -> float
```

Redondea hacia abajo (hacia -âˆž).
---`

ceil

### ````zt
public func ceil(value: float) -> float
```

Redondea hacia arriba (hacia +âˆž).
---`

round

### ````zt
public func round(value: float) -> float
```

Rondas que utilizan la semántica de "la mitad de cero".
---`

trunc

### ````zt
public func trunc(value: float) -> float
```

Trunca la parte fraccionaria (hacia cero).
---`

deg_to_rad

### ````zt
public func deg_to_rad(x: float) -> float
```

Convierte grados a radianes.
---`

rad_to_deg

### ````zt
public func rad_to_deg(x: float) -> float
```

Convierte radianes a grados.
---`

approx_equal

### ````zt
public func approx_equal(a: float, b: float, epsilon: float) -> bool
```

Comprueba la igualdad aproximada entre dos flotadores.

@param a – Primer valor.
@param b: segundo valor.
@param épsilon: tolerancia máxima.
@devolver`true`si`|a - b| <= epsilon`.
---`

sin

### ````zt
public func sin(value: float) -> float
```

Seno en radianes.
---`

cos

### ````zt
public func cos(value: float) -> float
```

Coseno en radianes.
---`

tan

### ````zt
public func tan(value: float) -> float
```

Tangente en radianes.
---`

asin

### ````zt
public func asin(value: float) -> float
```

Arcoseno. Devolver`nan`por valores fuera`[-1, 1]`.
---`

acos

### ````zt
public func acos(value: float) -> float
```

Arcocoseno. Devolver`nan`por valores fuera`[-1, 1]`.
---`

atan

### ````zt
public func atan(value: float) -> float
```

Arctangente.
---`

atan2

### ````zt
public func atan2(y: float, x: float) -> float
```

Arcotangente de dos argumentos (consciente de cuadrantes).

@param y: componente vertical.
@param x – Componente horizontal.
@return Ángulo en radianes.
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

Logaritmo en base 10.
---`

log2

### ````zt
public func log2(value: float) -> float
```

Logaritmo de base 2.
---`

log

### ````zt
public func log(value: float, base: float) -> float
```

Logaritmo en base arbitraria.

Valor @param – Valor.
@param base – Base del logaritmo.
@return Resultado del logaritmo.
---`

exp

### ````zt
public func exp(value: float) -> float
```

Exponencial (valor e^).
---`

is_nan

### ````zt
public func is_nan(value: float) -> bool
```

Comprueba si el valor es NaN.
---`

is_infinite

### ````zt
public func is_infinite(value: float) -> bool
```

Comprueba si el valor es infinito (positivo o negativo).
---`

es_finito

