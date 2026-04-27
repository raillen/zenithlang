# Cómo representar la ausencia sin`null`

> Audiencia: usuario
> Estado: actual
> Superficie: pública
> Fuente de la verdad: no

## Problema

Quiere decir que es posible que no exista un valor.

En muchos idiomas esto se convierte en`null`.

En Zenith, utilice`optional<T>`y`none`.

## Respuesta corta

Usar`optional<T>`cuando la ausencia es normal.

```zt
func find_name(id: int) -> optional<text>
    if id == 1
        return "Ada"
    end

    return none
end
```

## Ejemplo completo

```zt
namespace app.main

func find_name(id: int) -> optional<text>
    if id == 1
        return "Ada"
    end

    return none
end

func display_name(id: int) -> text
    const maybe_name: optional<text> = find_name(id)

    match maybe_name
        case value name ->
            return name
        case none ->
            return "anonymous"
    end
end

public func main() -> int
    if display_name(2) != "anonymous"
        return 1
    end

    return 0
end
```

## Por qué

`optional<T>`deja la ausencia visible en el tipo.

Quien llame a la función debe ocuparse de ambos caminos:

- valor presente;
- valor faltante.

Esto evita errores ocultos por`null`.

## Error común

No escribas:

```zt
func find_name(id: int) -> text
    return null
end
```

Escribir:

```zt
func find_name(id: int) -> optional<text>
    return none
end
```

## Cuando no usar

no usar`optional<T>`por error recuperable con motivo.

Si necesita decir por qué falló, utilice`result<T, E>`.

## Ver también

- `docs/public/cookbook/errors-without-exceptions.md`
- `docs/reference/language/errors-and-results.md`
- `language/spec/surface-syntax.md`
