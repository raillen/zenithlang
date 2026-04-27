# Validación, matemáticas y aleatoria.

> Audiencia: usuario
> Estado: actual
> Superficie: pública
> Fuente de la verdad: no

## Resumen

Usar:

- `std.validate`para predicados claros;
-`std.math`para operaciones numéricas;
-`std.random`para números pseudoaleatorios en corte alfa.

## Validación

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

## Matemáticas

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

##Aleatorio

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

## Cuidado con el estado

`std.random`expone el estado público observable:

- `random.seeded`;
- `random.last_seed`;
- `random.draw_count`.

Otros espacios de nombres pueden leer este estado, pero no deben escribirlo directamente.

Usar`random.seed(...)`.

## Ver también

- `docs/reference/stdlib/math-random-validate.md`
