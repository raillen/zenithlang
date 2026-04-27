# Fallos intencionales con check, todo e inalcanzable

> Audiencia: usuario
> Estado: actual
> Superficie: pública
> Fuente de la verdad: no

## Problema

Quiere que un error detenga el programa con un mensaje claro.

Pero cada caso tiene una intención diferente.

## Respuesta corta

Usar:

- `check(condition, message)`cuando una condición interna debe ser cierta;
-`todo(message)`cuando el camino aún no ha sido implementado;
-`unreachable(message)`cuando el camino debería ser imposible;
-`panic(message)`por un defecto fatal general.

## Código recomendado

```zt
namespace app.main

func score(value: int) -> int
    check(value > 0, "score must be positive")
    return value * 2
end

func load_bonus() -> int
    todo("load bonus from profile")
    return 0
end

func mode_to_score(mode: text) -> int
    if mode == "easy"
        return 1
    end

    if mode == "hard"
        return 2
    end

    unreachable("unknown mode")
    return 0
end
```

## Explicación

`check`comunica validación.

`todo`reporta trabajo incompleto.

`unreachable`informa una violación del control de flujo.

Estos nombres le ayudarán a leer el motivo del error antes de realizar la depuración.

## Error común

no usar`todo`por error comercial esperado.

```zt
func find_user(id: int) -> result<text, text>
    if id < 0
        return error("invalid id")
    end

    todo("database lookup")
    return success("Ada")
end
```

EL`todo`arriba sólo tiene sentido mientras la búsqueda aún no exista.

Cuando la búsqueda existe, la ausencia normal debe convertirse en`optional<T>`o`result<T,E>`.

## Cuando no usar

No utilice estas funciones integradas para un flujo normal.

Usar`optional<T>`cuando se espera un valor faltante.

Usar`result<T,E>`cuando la persona que llama puede manejar la falla.