# Validacao, matematica e random

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Resumo

Use:

- `std.validate` para predicados claros;
- `std.math` para operacoes numericas;
- `std.random` para numeros pseudoaleatorios no corte alpha.

## Validacao

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

## Matematica

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

## Random

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

## Cuidado com estado

`std.random` expoe estado publico observavel:

- `random.seeded`;
- `random.last_seed`;
- `random.draw_count`.

Outros namespaces podem ler esse estado, mas nao devem escrever diretamente.

Use `random.seed(...)`.

## Veja tambem

- `docs/reference/stdlib/math-random-validate.md`
