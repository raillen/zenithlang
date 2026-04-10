# Módulo `std.math`

O `std.math` expõe as primitivas matemáticas atualmente declaradas na stdlib.

## Constantes

| API | Descrição |
| :-- | :-- |
| `PI: float` | Valor de π. |

## Funções numéricas

| API | Descrição |
| :-- | :-- |
| `abs(x: float) -> float` | Valor absoluto. |
| `floor(x: float) -> int` | Arredonda para baixo. |
| `ceil(x: float) -> int` | Arredonda para cima. |
| `sqrt(x: float) -> float` | Raiz quadrada. |
| `random() -> float` | Número pseudoaleatório do host Lua. |

## Trigonometria

| API | Descrição |
| :-- | :-- |
| `sin(x: float) -> float` | Seno. |
| `cos(x: float) -> float` | Cosseno. |
| `tan(x: float) -> float` | Tangente. |
| `rad(deg: float) -> float` | Graus para radianos. |
| `deg(rad: float) -> float` | Radianos para graus. |

## Exemplo

```zt
import std.math

pub func main() -> int
    var angulo = math.rad(180.0)
    print("PI aproximado: " + math.PI)
    print("sin(angulo): " + math.sin(angulo))
    return 0
end
```

## Observações

- Esta é a API real do repositório atual. Entradas antigas como `E`, `pow`, `round`, `asin`, `acos` e `atan` não estão declaradas no módulo atual.
