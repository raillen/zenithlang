# Como representar ausencia sem `null`

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problema

Voce quer dizer que um valor pode nao existir.

Em muitas linguagens isso vira `null`.

Em Zenith, use `optional<T>` e `none`.

## Resposta curta

Use `optional<T>` quando a ausencia for normal.

```zt
func find_name(id: int) -> optional<text>
    if id == 1
        return "Ada"
    end

    return none
end
```

## Exemplo completo

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

## Por que

`optional<T>` deixa a ausencia visivel no tipo.

Quem chama a funcao precisa lidar com os dois caminhos:

- valor presente;
- valor ausente.

Isso evita erro escondido por `null`.

## Erro comum

Nao escreva:

```zt
func find_name(id: int) -> text
    return null
end
```

Escreva:

```zt
func find_name(id: int) -> optional<text>
    return none
end
```

## Quando nao usar

Nao use `optional<T>` para erro recuperavel com motivo.

Se precisa dizer por que falhou, use `result<T, E>`.

## Veja tambem

- `docs/public/cookbook/errors-without-exceptions.md`
- `docs/reference/language/errors-and-results.md`
- `language/spec/surface-syntax.md`
