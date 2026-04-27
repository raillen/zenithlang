# How to make an abstract method with`trait`

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problem

You want to declare behavior that multiple types must implement.

In Zenith, use`trait`.

Do not use`abstract class`.

## Short answer

Declare the contract with`trait`.

Implement with`apply Trait to Type`.

```zt
trait Scoreable
    func score() -> int
end
```

## Complete example

```zt
namespace app.main

trait Scoreable
    func score() -> int
end

struct Player
    hp: int
end

apply Scoreable to Player
    func score() -> int
        return self.hp + 1
    end
end

public func main() -> int
    const player: Player = Player(hp: 5)
    return player.score()
end
```

## Why

`trait`makes the intention explicit:

- the type needs to offer that behavior;
- the implementation is separate;
- there is no hidden classical heritage.

## Common error

Don't write:

```zt
abstract class Scoreable
    abstract func score() -> int
end
```

Write:

```zt
trait Scoreable
    func score() -> int
end
```

## When not to use

Do not use`trait`if there is only one concrete type and there is no shared contract.

In this case, use inherent method with`apply Type`.

## See also

- `docs/public/cookbook/partial-class-with-apply.md`
- `docs/public/cookbook/virtual-dispatch-with-dyn.md`
- `language/decisions/010-structs-traits-apply-enums-and-match.md`
