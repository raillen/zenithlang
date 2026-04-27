# Equivalentes Zenith para conceptos de otros idiomas

> Audiencia: usuario
> Estado: actual
> Superficie: pública
> Fuente de la verdad: no

## Objetivo

Esta guía le muestra el camino de Zenith hacia ideas comunes en C#, Java, TypeScript,
Óxido y listo.

Utilice este archivo cuando conozca el concepto pero no sepa su nombre en
Zenit.

## Resumen rápido

| Se você procura | Em Zenith use |
| --- | --- |
| inferencia local ampla | tipo explícito em `const`o`var`|
| unión segura |`enum`con carga útil |
| método abstracto |`trait`|
| interfaz con implementación separada |`trait` + `apply Trait to Type`|
| despacho virtual |`dyn<Trait>`cuando hay heterogeneidad real |
| clase parcial |`apply Type`y organización por espacio de nombres/archivo |
| excepción recuperable |`result<T, E>` |
| `null` | `optional<T>`y`none` |

## Declaración local con tipo explícito

Zenith no utiliza inferencia local amplia en este ciclo.

Escriba el tipo al lado del nombre:

```zt
const name: text = "Ana"
var retry_count: int = 0
```

No escribas:

```zt
let name = "Ana"
```

Por qué:

- el tipo es visible durante la lectura;
- los literales vacíos son menos ambiguos;
- El diagnóstico puede explicar mejor el error.

## Unión Segura

En TypeScript, Rust o C, puedes pensar en unión.

En Zenith, utilice`enum`con carga útil:

```zt
enum Shape
    Circle(radius: int)
    Rectangle(width: int, height: int)
    Point
end
```

No crees una palabra clave`union`.

EL`match`hace explícito cada caso.

Ver también:

- `docs/public/cookbook/safe-union-with-enum.md`
- `language/decisions/029-executable-enums-with-payload.md`

## Método abstracto

En C# o Java, puedes pensar en`abstract class`.

En Zenith, utilice`trait`:

```zt
trait Scoreable
    func score() -> int
end
```

Luego implemente el contrato:

```zt
apply Scoreable to Player
    func score() -> int
        return self.hp
    end
end
```

no agregar`abstract`.

Ver también:

- `docs/public/cookbook/abstract-methods-with-trait.md`
- `language/decisions/010-structs-traits-apply-enums-and-match.md`

## Envío virtual

En C# o Java, puedes pensar en el método`virtual`.

En Zenith, utilice`dyn<Trait>`cuando es necesario pasar valores de diferentes tipos
por el mismo contrato:

```zt
const shape: dyn Shape = circle
```

no usar`dyn`si todos los valores tienen el mismo tipo concreto.

En este caso, prefiera el tipo concreto o una función genérica con restricción.

```zt
func render_one<T>(shape: T) -> int
where T is Shape
```

Regla sencilla:

- varios tipos concretos en la misma lista o variable:`dyn<Trait>`;
- un tipo concreto por llamada: genéricos con`where`.

Ver también:

- `docs/public/cookbook/virtual-dispatch-with-dyn.md`
- `language/spec/dyn-dispatch.md`

## Clase parcial

Cª#,`partial class`Puede separar el código en varios archivos.

En Zenith, los datos y el comportamiento están separados por diseño:

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

Usar`apply Type`para métodos inherentes.

Usar`apply Trait to Type`para implementar un contrato.

no agregar`partial`.

Ver también:

- `docs/public/cookbook/partial-class-with-apply.md`
- `language/decisions/010-structs-traits-apply-enums-and-match.md`

## Excepción recuperable

En Java, C# o TypeScript, puedes pensar en`throw`.

En Zenith, el fallo recuperable debería aparecer del tipo:

```zt
func load_user(id: int) -> result<User, core.Error>
```

Usar`panic`sólo hasta un fracaso fatal.

Ver también:

- `docs/public/cookbook/errors-without-exceptions.md`
- `language/decisions/037-panic-fatal-errors-and-attempt.md`

## Ausencia sin nulo

En Java, TypeScript, Go o C#, puedes pensar en`null`o`nil`.

En Zenith, utilice`optional<T>`:

```zt
const maybe_name: optional<text> = none
```

Esto deja la ausencia visible en el tipo.

Ver también:

- `docs/public/cookbook/absence-without-null.md`
- `language/decisions/041-no-null-and-optional-absence.md`

## Palabras que R6 no añade

R6 no agrega estas palabras clave ni atajos:

- `abstract`;
- `virtual`;
- `partial`;
- `union`;
- `?.`;
- `??`;
- ternario;
- operador de tubería;
- sobrecarga;
- macros;
- retorno implícito.

Estas ideas permanecen fuera de ciclo para preservar la legibilidad, la previsibilidad y
diagnósticos claros.