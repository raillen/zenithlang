# Cómo hacer un método abstracto con`trait`

> Audiencia: usuario
> Estado: actual
> Superficie: pública
> Fuente de la verdad: no

## Problema

Quiere declarar un comportamiento que deben implementar varios tipos.

En Zenith, utilice`trait`.

no usar`abstract class`.

## Respuesta corta

Declarar el contrato con`trait`.

Implementar con`apply Trait to Type`.

```zt
trait Scoreable
    func score() -> int
end
```

## Ejemplo completo

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

## Por qué

`trait`hace explícita la intención:

- el tipo necesita ofrecer ese comportamiento;
- la implementación es independiente;
- No hay herencia clásica oculta.

## Error común

No escribas:

```zt
abstract class Scoreable
    abstract func score() -> int
end
```

Escribir:

```zt
trait Scoreable
    func score() -> int
end
```

## Cuando no usar

no usar`trait`si sólo hay un tipo concreto y no existe contrato compartido.

En este caso, utilice el método inherente con`apply Type`.

## Ver también

- `docs/public/cookbook/partial-class-with-apply.md`
- `docs/public/cookbook/virtual-dispatch-with-dyn.md`
- `language/decisions/010-structs-traits-apply-enums-and-match.md`
