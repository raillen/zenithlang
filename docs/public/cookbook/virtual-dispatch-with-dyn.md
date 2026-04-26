# Como fazer dispatch virtual com `dyn<Trait>`

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problema

Voce quer guardar tipos diferentes atras do mesmo contrato.

Em linguagens com classes, isso costuma ser `virtual`.

Em Zenith, use `dyn<Trait>`.

## Resposta curta

Use `dyn Trait` quando a heterogeneidade for real.

```zt
const shape: dyn Shape = circle
```

## Exemplo completo

```zt
namespace app.main

trait Shape
    func area() -> int
end

struct Circle
    radius: int
end

apply Shape to Circle
    func area() -> int
        return 3 * self.radius * self.radius
    end
end

struct Rect
    w: int
    h: int
end

apply Shape to Rect
    func area() -> int
        return self.w * self.h
    end
end

public func main() -> int
    const circle: Circle = Circle(radius: 10)
    const rect: Rect = Rect(w: 5, h: 4)

    const a: dyn Shape = circle
    const b: dyn Shape = rect

    return a.area() + b.area()
end
```

## Por que

`dyn<Trait>` mostra o custo e a intencao no tipo.

Quem le o codigo sabe que a chamada passa por dispatch dinamico.

## Erro comum

Nao procure uma palavra `virtual`:

```zt
virtual func area() -> int
```

Use:

```zt
trait Shape
    func area() -> int
end
```

e depois:

```zt
const shape: dyn Shape = circle
```

## Quando nao usar

Nao use `dyn` se todos os valores tem o mesmo tipo concreto.

Nesse caso, prefira o tipo concreto ou uma funcao generica com constraint.

```zt
func draw_one<T>(shape: T) -> int
where T is Shape
```

Use esta regra:

- heterogeneidade real: `dyn<Trait>`;
- tipo concreto ainda conhecido: generic com `where`.

## Limites atuais

O subset atual de `dyn<Trait>` tem limites.

Por exemplo:

- traits genericas nao entram no subset dinamico atual;
- metodos mutating em dyn trait sao limitados;
- tipos de parametros/retornos precisam seguir as regras do subset.

## Veja tambem

- `docs/public/cookbook/abstract-methods-with-trait.md`
- `docs/reference/language/types.md`
- `language/spec/dyn-dispatch.md`
