# Como separar metodos sem partial class

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problema

Voce quer manter dados e metodos organizados sem colocar tudo no mesmo bloco.

Em C# isso pode virar `partial class`.

Em Zenith, use `struct` para dados e `apply` para comportamento.

## Resposta curta

Declare os dados com `struct`.

Adicione metodos com `apply Type`.

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

## Exemplo completo

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

## Por que

`apply` permite separar comportamento sem fragmentar a definicao dos dados.

Isso ajuda a leitura:

- `struct` mostra o formato do valor;
- `apply Type` mostra comportamento inerente;
- `apply Trait to Type` mostra implementacao de contrato.

## Erro comum

Nao escreva:

```zt
partial struct Player
    hp: int
end
```

Escreva:

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

## Quando nao usar

Nao espalhe metodos em arquivos demais sem motivo.

Use `apply` para organizar, nao para esconder comportamento.

## Veja tambem

- `docs/public/cookbook/abstract-methods-with-trait.md`
- `language/decisions/010-structs-traits-apply-enums-and-match.md`
