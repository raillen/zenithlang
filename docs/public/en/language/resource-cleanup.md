# Cleanup With `using`

> Audience: user
> Surface: public
> Status: current

## Core idea

Use `using` when a value needs a clear end.

It helps with cases like:

- opening and closing a resource;
- recording a cleanup action;
- keeping cleanup order clear on early exits.

## Block form

```zt
using handle = open_handle()
    use(handle)
end
```

In this form, `handle` exists only inside the block.

## Flat form

```zt
using handle = open_handle()
use(handle)
return 0
```

In this form, `handle` exists until the current block ends.

In a function body, that usually means the rest of the function.

Inside a loop, it means the current iteration.

## Explicit cleanup

```zt
using handle = open_handle() then close_handle(handle)
use(handle)
```

The expression after `then` runs when the scope ends.

It also runs before:

- `return`;
- propagation with `?`;
- `break`;
- `continue`.

## Order

When several `using` values are active, cleanup is LIFO.

That means:

1. the last active `using` cleans up first;
2. the first active `using` cleans up last.

```zt
using first = 1 then cleanup(first)
using second = 2 then cleanup(second)

return 0
```

In this example, `cleanup(second)` runs before `cleanup(first)`.

## Important rule

If the initializer fails with `?`, the new `using` cleanup is not active yet.

```zt
using value = load_value()? then cleanup(value)
```

If `load_value()?` propagates an error, `cleanup(value)` does not run, because `value` was not created.
