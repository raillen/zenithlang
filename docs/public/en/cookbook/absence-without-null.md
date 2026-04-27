# How to represent absence without`null`

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problem

You mean that a value may not exist.

In many languages ​​this becomes`null`.

In Zenith, use`optional<T>`and`none`.

## Short answer

Use`optional<T>`when absence is normal.

```zt
func find_name(id: int) -> optional<text>
    if id == 1
        return "Ada"
    end

    return none
end
```

## Complete example

```zt
namespace app.main

func find_name(id: int) -> optional<text>
    if id == 1
        return "Ada"
    end

    return none
end

func display_name(id: int) -> text
    const maybe_name: optional<text> = find_name(id)

    match maybe_name
        case value name ->
            return name
        case none ->
            return "anonymous"
    end
end

public func main() -> int
    if display_name(2) != "anonymous"
        return 1
    end

    return 0
end
```

## Why

`optional<T>`leaves the absence visible in the type.

Whoever calls the function needs to deal with both paths:

- present value;
- missing value.

This avoids errors hidden by`null`.

## Common error

Don't write:

```zt
func find_name(id: int) -> text
    return null
end
```

Write:

```zt
func find_name(id: int) -> optional<text>
    return none
end
```

## When not to use

Do not use`optional<T>`for recoverable error with reason.

If you need to say why you failed, use`result<T, E>`.

## See also

- `docs/public/cookbook/errors-without-exceptions.md`
- `docs/reference/language/errors-and-results.md`
- `language/spec/surface-syntax.md`
