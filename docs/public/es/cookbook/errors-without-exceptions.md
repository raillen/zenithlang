# Cómo representar el error sin excepción

> Audiencia: usuario
> Estado: actual
> Superficie: pública
> Fuente de la verdad: no

## Problema

Quieres representar un fracaso recuperable.

En Zenith, los errores recuperables utilizan`result<T, E>`.

`panic(...)`se convierte en un error fatal.

## Respuesta corta

Usar`result<T, E>`cuando la persona que llama debe decidir qué hacer.

```zt
func parse_score(value: text) -> result<int, text>
    if value == ""
        return error("score is empty")
    end

    return success(10)
end
```

## Ejemplo completo

```zt
namespace app.main

func parse_score(value: text) -> result<int, text>
    if value == ""
        return error("score is empty")
    end

    return success(10)
end

func score_or_zero(value: text) -> int
    const parsed: result<int, text> = parse_score(value)

    match parsed
        case success(score) ->
            return score
        case error(message) ->
            return 0
    end
end

public func main() -> int
    return score_or_zero("")
end
```

## Por qué

`result<T, E>`separa el éxito y el fracaso en tipo.

Esto ayuda a leer:

- `success(value)`lleva el valor;
-`error(reason)`lleva la razón del fracaso.

## Error común

no usar`panic`para el flujo normal de negocios:

```zt
func parse_score(value: text) -> int
    if value == ""
        panic("score is empty")
    end

    return 10
end
```

Preferir:

```zt
func parse_score(value: text) -> result<int, text>
    if value == ""
        return error("score is empty")
    end

    return success(10)
end
```

## Cuando no usar

Usar`optional<T>`cuando la ausencia es normal y no requiere motivo.

Usar`panic(...)`cuando el programa ha encontrado un estado fatal o imposible.

## Ver también

- `docs/public/cookbook/absence-without-null.md`
- `docs/reference/language/errors-and-results.md`
- `language/decisions/037-panic-fatal-errors-and-attempt.md`
