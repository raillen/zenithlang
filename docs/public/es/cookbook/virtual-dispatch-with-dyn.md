# Cómo hacer despacho virtual con`dyn<Trait>`

> Audiencia: usuario
> Estado: actual
> Superficie: pública
> Fuente de la verdad: no

## Problema

Quiere almacenar diferentes tipos detrás del mismo contrato.

En idiomas con clases, esto suele ser`virtual`.

En Zenith, utilice`dyn<Trait>`.

## Respuesta corta

Usar`dyn Trait`cuando la heterogeneidad es real.

```zt
const shape: dyn Shape = circle
```

## Ejemplo completo

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

## Por qué

`dyn<Trait>`muestra el costo y la intención en el tipo.

Cualquiera que lea el código sabe que la llamada se realiza mediante envío dinámico.

## Error común

No busques una palabra`virtual`:

```zt
virtual func area() -> int
```

Usar:

```zt
trait Shape
    func area() -> int
end
```

y luego:

```zt
const shape: dyn Shape = circle
```

## Cuando no usar

no usar`dyn`si todos los valores tienen el mismo tipo concreto.

En este caso, prefiera el tipo concreto o una función genérica con restricción.

```zt
func draw_one<T>(shape: T) -> int
where T is Shape
```

Utilice esta regla:

- heterogeneidad real:`dyn<Trait>`;
- tipo concreto aún conocido: genérico con`where`.

## Límites actuales

El subconjunto actual de`dyn<Trait>`tiene límites.

Por ejemplo:

- los rasgos genéricos no entran en el subconjunto dinámico actual;
- los métodos de mutación en el rasgo dyn son limitados;
- los tipos de parámetros/devoluciones deben seguir las reglas del subconjunto.

## Ver también

- `docs/public/cookbook/abstract-methods-with-trait.md`
- `docs/reference/language/types.md`
- `language/spec/dyn-dispatch.md`
