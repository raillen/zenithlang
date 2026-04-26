# Como representar erro sem exception

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problema

Voce quer representar uma falha recuperavel.

Em Zenith, erro recuperavel usa `result<T, E>`.

`panic(...)` fica para erro fatal.

## Resposta curta

Use `result<T, E>` quando o chamador deve decidir o que fazer.

```zt
func parse_score(value: text) -> result<int, text>
    if value == ""
        return error("score is empty")
    end

    return success(10)
end
```

## Exemplo completo

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

## Por que

`result<T, E>` separa sucesso e falha no tipo.

Isso ajuda a leitura:

- `success(value)` carrega o valor;
- `error(reason)` carrega o motivo da falha.

## Erro comum

Nao use `panic` para fluxo normal de negocio:

```zt
func parse_score(value: text) -> int
    if value == ""
        panic("score is empty")
    end

    return 10
end
```

Prefira:

```zt
func parse_score(value: text) -> result<int, text>
    if value == ""
        return error("score is empty")
    end

    return success(10)
end
```

## Quando nao usar

Use `optional<T>` quando a ausencia for normal e nao precisar de motivo.

Use `panic(...)` quando o programa encontrou um estado fatal ou impossivel.

## Veja tambem

- `docs/public/cookbook/absence-without-null.md`
- `docs/reference/language/errors-and-results.md`
- `language/decisions/037-panic-fatal-errors-and-attempt.md`
