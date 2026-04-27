# How to pass a function as a value

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problem

You want to pass a function to another part of the code.

In Zenith, use the callable type:

```zt
func(int, int) -> int
```

## Short answer

State the signature explicitly.

```zt
const op: func(int, int) -> int = add
```

## Complete example

```zt
namespace app.main

func add(a: int, b: int) -> int
    return a + b
end

func run_op(op: func(int, int) -> int) -> int
    return op(3, 4)
end

public func main() -> int
    const op: func(int, int) -> int = add
    return run_op(op)
end
```

## Why

The callable type shows:

- how many parameters the function receives;
- what types does it receive;
- which type it returns.

This avoids hidden callbacks.

## Common error

Don't try to use a generic function like callable in this cut.

Prefer a simple top-level function:

```zt
func add(a: int, b: int) -> int
    return a + b
end
```

## Current limits

Callables v1 are narrow on purpose.

In this cut, avoid:

- callable as struct field;
- callable inside list/map;
- partial application;
- method as value;
- generic function as value.

## See also

- `language/spec/callables.md`
- `language/decisions/089-callable-delegates-v1.md`
