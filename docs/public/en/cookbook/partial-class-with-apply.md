# How to separate methods without partial class

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problem

You want to keep data and methods organized without putting everything in the same block.

In C# this can become`partial class`.

In Zenith, use`struct`for data and`apply`for behavior.

## Short answer

Declare the data with`struct`.

Add methods with`apply Type`.

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

## Complete example

```zt
namespace app.main

struct Player
    hp: int
end

apply Player
    mut func heal(amount: int)
        self.hp = self.hp + amount
        return
    end
end

public func main() -> int
    var player: Player = Player(hp: 5)
    player.heal(2)
    return player.hp
end
```

## Why

`apply`allows you to separate behavior without fragmenting the data definition.

This helps reading:

- `struct`shows the value format;
-`apply Type`shows inherent behavior;
-`apply Trait to Type`shows contract implementation.

## Common error

Don't write:

```zt
partial struct Player
    hp: int
end
```

Write:

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

## When not to use

Don't spread methods across too many files for no reason.

Use`apply`to organize, not to hide behavior.

## See also

- `docs/public/cookbook/abstract-methods-with-trait.md`
- `language/decisions/010-structs-traits-apply-enums-and-match.md`
