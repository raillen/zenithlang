# Como fazer union segura com `enum` com payload

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problema

Você quer um valor que pode ter formatos diferentes.

Em Zenith, a forma recomendada e `enum` com payload.

Não crie uma keyword `union` para isso.

## Resposta curta

Use `enum` com cases que carregam dados.

```zt
enum Shape
    Circle(radius: int)
    Rectangle(width: int, height: int)
    Point
end
```

## Exemplo completo

```zt
namespace app.main

enum Shape
    Circle(radius: int)
    Rectangle(width: int, height: int)
    Point
end

func area(shape: Shape) -> int
    match shape
        case Shape.Circle(radius) ->
            return radius * radius
        case Shape.Rectangle(width, height) ->
            return width * height
        case Shape.Point ->
            return 0
    end
end

public func main() -> int
    const shape: Shape = Shape.Rectangle(width: 5, height: 4)
    return area(shape)
end
```

## Por que

`enum` com payload deixa claro:

- quais formas o valor pode ter;
- quais dados cada forma carrega;
- onde o código trata cada caso.

O `match` torna o fluxo visivel.

## Erro comum

Não procure uma forma assim:

```zt
union Shape = Circle | Rectangle | Point
```

Use:

```zt
enum Shape
    Circle(radius: int)
    Rectangle(width: int, height: int)
    Point
end
```

## Quando não usar

Não use `enum` com payload para comportamento compartilhado entre muitos tipos.

Para comportamento, use `trait`.

Para dispatch dinâmico, use `dyn<Trait>`.

## Veja também

- `docs/public/cookbook/abstract-methods-with-trait.md`
- `docs/public/cookbook/virtual-dispatch-with-dyn.md`
- `docs/reference/language/types.md`
- `language/decisions/029-executable-enums-with-payload.md`
