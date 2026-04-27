# Cómo hacer una unión segura con`enum`con carga útil

> Audiencia: usuario
> Estado: actual
> Superficie: pública
> Fuente de la verdad: no

## Problema

Quiere un valor que pueda tener diferentes formatos.

En Zenith, la forma recomendada y`enum`con carga útil.

No crees una palabra clave`union`por eso.

## Respuesta corta

Usar`enum`con estuches que llevan datos.

```zt
enum Shape
    Circle(radius: int)
    Rectangle(width: int, height: int)
    Point
end
```

## Ejemplo completo

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

## Por qué

`enum`con carga útil deja claro:

- qué formas puede tener el valor;
- qué datos lleva cada forma;
- donde el código maneja cada caso.

EL`match`hace visible el flujo.

## Error común

No busques una forma como esta:

```zt
union Shape = Circle | Rectangle | Point
```

Usar:

```zt
enum Shape
    Circle(radius: int)
    Rectangle(width: int, height: int)
    Point
end
```

## Cuando no usar

no usar`enum`con carga útil para comportamiento compartido entre muchos tipos.

Para el comportamiento, utilice`trait`.

Para despacho dinámico, use`dyn<Trait>`.

## Ver también

- `docs/public/cookbook/abstract-methods-with-trait.md`
- `docs/public/cookbook/virtual-dispatch-with-dyn.md`
- `docs/reference/language/types.md`
- `language/decisions/029-executable-enums-with-payload.md`
