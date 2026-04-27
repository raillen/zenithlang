# How to do virtual dispatch with`dyn<Trait>`

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problem

You want to store different types behind the same contract.

In languages ​​with classes, this is often`virtual`.

In Zenith, use`dyn<Trait>`.

## Short answer

Use`dyn Trait`when heterogeneity is real.

```zt
const shape: dyn Shape = circle
```

## Complete example

```zt
namespace app.main

trait Shape
    func area() -> int
end

struct Circle
    radius: int
end

apply Shape to Circle
    func area() -> int
        return 3 * self.radius * self.radius
    end
end

struct Rect
    w: int
    h: int
end

apply Shape to Rect
    func area() -> int
        return self.w * self.h
    end
end

public func main() -> int
    const circle: Circle = Circle(radius: 10)
    const rect: Rect = Rect(w: 5, h: 4)

    const a: dyn Shape = circle
    const b: dyn Shape = rect

    return a.area() + b.area()
end
```

## Why

`dyn<Trait>`shows the cost and intent in the type.

Anyone who reads the code knows that the call goes through dynamic dispatch.

## Common error

Don't look for a word`virtual`:

```zt
virtual func area() -> int
```

Use:

```zt
trait Shape
    func area() -> int
end
```

and then:

```zt
const shape: dyn Shape = circle
```

## When not to use

Do not use`dyn`whether all values ​​have the same concrete type.

In this case, prefer the concrete type or a generic function with constraint.

```zt
func draw_one<T>(shape: T) -> int
where T is Shape
```

Use this rule:

- real heterogeneity:`dyn<Trait>`;
- concrete type still known: generic with`where`.

## Current limits

The current subset of`dyn<Trait>`has limits.

For example:

- generic traits do not enter the current dynamic subset;
- mutating methods in dyn trait are limited;
- types of parameters/returns must follow the subset rules.

## See also

- `docs/public/cookbook/abstract-methods-with-trait.md`
- `docs/reference/language/types.md`
- `language/spec/dyn-dispatch.md`
