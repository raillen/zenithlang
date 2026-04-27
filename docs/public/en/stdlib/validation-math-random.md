# Validation, mathematics and random

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Summary

Use:

- `std.validate`for clear predicates;
-`std.math`for numeric operations;
-`std.random`for pseudorandom numbers in alpha cutoff.

## Validation

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

## Mathematics

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

##Random

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

## Beware of status

`std.random`exposes observable public state:

- `random.seeded`;
- `random.last_seed`;
- `random.draw_count`.

Other namespaces can read this state, but must not write directly.

Use`random.seed(...)`.

## See also

- `docs/reference/stdlib/math-random-validate.md`
