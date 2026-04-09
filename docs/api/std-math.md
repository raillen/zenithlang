# Módulo de Matemática (`std.math`)

O módulo `math` fornece constantes e funções matemáticas comuns.

## Constantes

### `PI`
O valor da constante matemática π (aproximadamente 3.14159).

### `E`
O valor da base dos logaritmos naturais (aproximadamente 2.71828).

## Funções Básicas

### `abs(n: float) -> float`
Retorna o valor absoluto de um número.

### `sqrt(n: float) -> float`
Retorna a raiz quadrada de um número.

### `pow(base: float, exp: float) -> float`
Retorna a base elevada ao expoente.

### `round(n: float) -> int`
Arredonda um número para o inteiro mais próximo.

### `floor(n: float) -> int`
Arredonda um número para baixo.

### `ceil(n: float) -> int`
Arredonda um número para cima.

### `random() -> float`
Retorna um número pseudo-aleatório entre 0.0 e 1.0.

## Trigonometria e Conversão

Inclui `sin`, `cos`, `tan`, `asin`, `acos`, `atan`.

### `rad(deg: float) -> float`
Converte graus para radianos.

### `deg(rad: float) -> float`
Converte radianos para graus.

```zt
import std.math

var raio = 5.0
var area = math.PI * math.pow(raio, 2)
print("Área do círculo: {area}")
```
