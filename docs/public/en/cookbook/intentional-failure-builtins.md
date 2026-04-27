# Intentional failures with check, todo and unreachable

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problem

You want an error to stop the program with a clear message.

But each case has a different intention.

## Short answer

Use:

- `check(condition, message)`when an internal condition needs to be true;
-`todo(message)`when the path has not yet been implemented;
-`unreachable(message)`when the path should be impossible;
-`panic(message)`for a general fatal flaw.

## Recommended code

```zt
namespace app.main

func score(value: int) -> int
    check(value > 0, "score must be positive")
    return value * 2
end

func load_bonus() -> int
    todo("load bonus from profile")
    return 0
end

func mode_to_score(mode: text) -> int
    if mode == "easy"
        return 1
    end

    if mode == "hard"
        return 2
    end

    unreachable("unknown mode")
    return 0
end
```

## Explanation

`check`communicates validation.

`todo`reports incomplete work.

`unreachable`reports a flow control violation.

These names help you read the reason for the failure before debugging.

## Common error

Do not use`todo`for expected business error.

```zt
func find_user(id: int) -> result<text, text>
    if id < 0
        return error("invalid id")
    end

    todo("database lookup")
    return success("Ada")
end
```

THE`todo`above only makes sense while the search does not yet exist.

When the search exists, normal absence should become`optional<T>`or`result<T,E>`.

## When not to use

Don't use these builtins for normal flow.

Use`optional<T>`when missing value is expected.

Use`result<T,E>`when the caller can handle the fault.