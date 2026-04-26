# Como fazer metodo abstrato com `trait`

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problema

Voce quer declarar um comportamento que varios tipos devem implementar.

Em Zenith, use `trait`.

Nao use `abstract class`.

## Resposta curta

Declare o contrato com `trait`.

Implemente com `apply Trait to Type`.

```zt
trait Scoreable
    func score() -> int
end
```

## Exemplo completo

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

## Por que

`trait` deixa a intencao explicita:

- o tipo precisa oferecer aquele comportamento;
- a implementacao fica separada;
- nao existe heranca classica escondida.

## Erro comum

Nao escreva:

```zt
abstract class Scoreable
    abstract func score() -> int
end
```

Escreva:

```zt
trait Scoreable
    func score() -> int
end
```

## Quando nao usar

Nao use `trait` se so existe um tipo concreto e nao ha contrato compartilhado.

Nesse caso, use metodo inerente com `apply Type`.

## Veja tambem

- `docs/public/cookbook/partial-class-with-apply.md`
- `docs/public/cookbook/virtual-dispatch-with-dyn.md`
- `language/decisions/010-structs-traits-apply-enums-and-match.md`
