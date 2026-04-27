# How to make a secure union with`enum`with payload

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problem

You want a value that can have different formats.

In Zenith, the recommended way and`enum`with payload.

Don't create a keyword`union`for that.

## Short answer

Use`enum`with cases that carry data.

```zt
enum Shape
    Circle(radius: int)
    Rectangle(width: int, height: int)
    Point
end
```

## Complete example

```zt
namespace app.main

enum Shape
    Circle(radius: int)
    Rectangle(width: int, height: int)
    Point
end

func area(shape: Shape) -> int
    match shape
        case Shape.Circle(radius) ->
            return radius * radius
        case Shape.Rectangle(width, height) ->
            return width * height
        case Shape.Point ->
            return 0
    end
end

public func main() -> int
    const shape: Shape = Shape.Rectangle(width: 5, height: 4)
    return area(shape)
end
```

## Why

`enum`with payload makes it clear:

- what forms the value can have;
- what data each shape carries;
- where the code handles each case.

THE`match`makes the flow visible.

## Common error

Don't look for a shape like this:

```zt
union Shape = Circle | Rectangle | Point
```

Use:

```zt
enum Shape
    Circle(radius: int)
    Rectangle(width: int, height: int)
    Point
end
```

## When not to use

Do not use`enum`with payload for shared behavior among many types.

For behavior, use`trait`.

For dynamic dispatch, use`dyn<Trait>`.

## See also

- `docs/public/cookbook/abstract-methods-with-trait.md`
- `docs/public/cookbook/virtual-dispatch-with-dyn.md`
- `docs/reference/language/types.md`
- `language/decisions/029-executable-enums-with-payload.md`
