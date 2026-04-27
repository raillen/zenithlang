# Zenith equivalents for concepts from other languages

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Objective

This guide shows you the Zenith path to common ideas in C#, Java, TypeScript,
Rust and Go.

Use this file when you know the concept but don't know its name in
Zenit.

## Quick summary

| If you are looking for | In Zenith use |
| --- | --- |
| broad local inference | explicit type in`const`or`var`|
| secure union |`enum`with payload |
| abstract method |`trait`|
| interface with separate implementation |`trait` + `apply Trait to Type`|
| virtual dispatch |`dyn<Trait>`when there is real heterogeneity |
| partial class |`apply Type`and organization by namespace/file |
| recoverable exception |`result<T, E>` |
| `null` | `optional<T>`and`none` |

## Local declaration with explicit type

Zenith does not use wide local inference in this cycle.

Write the type next to the name:

```zt
const name: text = "Ana"
var retry_count: int = 0
```

Don't write:

```zt
let name = "Ana"
```

Why:

- the type is visible during reading;
- empty literals are less ambiguous;
- diagnostics can better explain the error.

## Safe Union

In TypeScript, Rust or C, you can think of union.

In Zenith, use`enum`with payload:

```zt
enum Shape
    Circle(radius: int)
    Rectangle(width: int, height: int)
    Point
end
```

Don't create a keyword`union`.

THE`match`makes each case explicit.

See also:

- `docs/public/cookbook/safe-union-with-enum.md`
- `language/decisions/029-executable-enums-with-payload.md`

## Abstract method

In C# or Java, you can think of`abstract class`.

In Zenith, use`trait`:

```zt
trait Scoreable
    func score() -> int
end
```

Then implement the contract:

```zt
apply Scoreable to Player
    func score() -> int
        return self.hp
    end
end
```

Do not add`abstract`.

See also:

- `docs/public/cookbook/abstract-methods-with-trait.md`
- `language/decisions/010-structs-traits-apply-enums-and-match.md`

## Virtual Dispatch

In C# or Java, you can think of method`virtual`.

In Zenith, use`dyn<Trait>`when values of different types need to pass
by the same contract:

```zt
const shape: dyn Shape = circle
```

Do not use`dyn`whether all values ​​have the same concrete type.

In this case, prefer the concrete type or a generic function with constraint.

```zt
func render_one<T>(shape: T) -> int
where T is Shape
```

Simple rule:

- several concrete types in the same list or variable:`dyn<Trait>`;
- one concrete type per call: generics with`where`.

See also:

- `docs/public/cookbook/virtual-dispatch-with-dyn.md`
- `language/spec/dyn-dispatch.md`

## Partial class

In C#,`partial class`You can separate code into several files.

In Zenith, data and behavior are separated by design:

```zt
struct Player
    hp: int
end

apply Player
    func is_alive() -> bool
        return self.hp > 0
    end
end
```

Use`apply Type`for inherent methods.

Use`apply Trait to Type`to implement a contract.

Do not add`partial`.

See also:

- `docs/public/cookbook/partial-class-with-apply.md`
- `language/decisions/010-structs-traits-apply-enums-and-match.md`

## Recoverable exception

In Java, C# or TypeScript, you can think of`throw`.

In Zenith, recoverable failure should appear in the type:

```zt
func load_user(id: int) -> result<User, core.Error>
```

Use`panic`only to fatal failure.

See also:

- `docs/public/cookbook/errors-without-exceptions.md`
- `language/decisions/037-panic-fatal-errors-and-attempt.md`

## Absence without null

In Java, TypeScript, Go or C#, you can think of`null`or`nil`.

In Zenith, use`optional<T>`:

```zt
const maybe_name: optional<text> = none
```

This leaves the absence visible in the type.

See also:

- `docs/public/cookbook/absence-without-null.md`
- `language/decisions/041-no-null-and-optional-absence.md`

## Words that R6 does not add

R6 does not add these keywords or shortcuts:

- `abstract`;
- `virtual`;
- `partial`;
- `union`;
- `?.`;
- `??`;
- ternary;
- pipe operator;
- overload;
- macros;
- implicit return.

These ideas remain out of cycle to preserve readability, predictability and
clear diagnoses.