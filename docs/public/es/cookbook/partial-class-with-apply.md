# Cómo separar métodos sin clase parcial

> Audiencia: usuario
> Estado: actual
> Superficie: pública
> Fuente de la verdad: no

## Problema

Quiere mantener los datos y los métodos organizados sin poner todo en el mismo bloque.

En C# esto puede convertirse`partial class`.

En Zenith, utilice`struct`para datos y`apply`por el comportamiento.

## Respuesta corta

Declarar los datos con`struct`.

Agregar métodos con`apply Type`.

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

## Ejemplo completo

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

## Por qué

`apply`le permite separar el comportamiento sin fragmentar la definición de datos.

Esto ayuda a leer:

- `struct`muestra el formato del valor;
-`apply Type`muestra un comportamiento inherente;
-`apply Trait to Type`muestra la implementación del contrato.

## Error común

No escribas:

```zt
partial struct Player
    hp: int
end
```

Escribir:

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

## Cuando no usar

No distribuya métodos en demasiados archivos sin ningún motivo.

Usar`apply`organizar, no ocultar el comportamiento.

## Ver también

- `docs/public/cookbook/abstract-methods-with-trait.md`
- `language/decisions/010-structs-traits-apply-enums-and-match.md`
