# Falhas intencionais com check, todo e unreachable

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problema

Voce quer que um erro pare o programa com uma mensagem clara.

Mas cada caso tem uma intencao diferente.

## Resposta curta

Use:

- `check(condition, message)` quando uma condicao interna precisa ser verdadeira;
- `todo(message)` quando o caminho ainda nao foi implementado;
- `unreachable(message)` quando o caminho deveria ser impossivel;
- `panic(message)` para uma falha fatal geral.

## Codigo recomendado

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

## Explicacao

`check` comunica uma validacao.

`todo` comunica trabalho incompleto.

`unreachable` comunica uma violacao de controle de fluxo.

Esses nomes ajudam a ler o motivo da falha antes de depurar.

## Erro comum

Nao use `todo` para erro esperado de negocio.

```zt
func find_user(id: int) -> result<text, text>
    if id < 0
        return error("invalid id")
    end

    todo("database lookup")
    return success("Ada")
end
```

O `todo` acima so faz sentido enquanto a busca ainda nao existe.

Quando a busca existir, ausencia normal deve virar `optional<T>` ou `result<T,E>`.

## Quando nao usar

Nao use esses builtins para fluxo normal.

Use `optional<T>` quando faltar valor for esperado.

Use `result<T,E>` quando o chamador puder tratar a falha.
