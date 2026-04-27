# How to represent error without exception

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problem

You want to represent a recoverable failure.

In Zenith, recoverable error uses`result<T, E>`.

`panic(...)`it becomes a fatal error.

## Short answer

Use`result<T, E>`when the caller must decide what to do.

```zt
func parse_score(value: text) -> result<int, text>
    if value == ""
        return error("score is empty")
    end

    return success(10)
end
```

## Complete example

```zt
namespace app.main

func parse_score(value: text) -> result<int, text>
    if value == ""
        return error("score is empty")
    end

    return success(10)
end

func score_or_zero(value: text) -> int
    const parsed: result<int, text> = parse_score(value)

    match parsed
        case success(score) ->
            return score
        case error(message) ->
            return 0
    end
end

public func main() -> int
    return score_or_zero("")
end
```

## Why

`result<T, E>`separates success and failure in type.

This helps reading:

- `success(value)`carries the value;
-`error(reason)`carries the reason for the failure.

## Common error

Do not use`panic`for normal business flow:

```zt
func parse_score(value: text) -> int
    if value == ""
        panic("score is empty")
    end

    return 10
end
```

Prefer:

```zt
func parse_score(value: text) -> result<int, text>
    if value == ""
        return error("score is empty")
    end

    return success(10)
end
```

## When not to use

Use`optional<T>`when the absence is normal and does not require a reason.

Use`panic(...)`when the program has encountered a fatal or impossible state.

## See also

- `docs/public/cookbook/absence-without-null.md`
- `docs/reference/language/errors-and-results.md`
- `language/decisions/037-panic-fatal-errors-and-attempt.md`
