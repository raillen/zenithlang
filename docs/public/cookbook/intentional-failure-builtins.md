# Falhas intencionais com check, todo e unreachable

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problema

Você quer que um erro pare o programa com uma mensagem clara.

Mas cada caso tem uma intencao diferente.

## Resposta curta

Use:

- `check(condition, message)` quando uma condição interna precisa ser verdadeira;
- `todo(message)` quando o caminho ainda não foi implementado;
- `unreachable(message)` quando o caminho deveria ser impossível;
- `panic(message)` para uma falha fatal geral.

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

## Explicacao

`check` comunica uma validação.

`todo` comunica trabalho incompleto.

`unreachable` comunica uma violacao de controle de fluxo.

Esses nomes ajudam a ler o motivo da falha antes de depurar.

## Erro comum

Não use `todo` para erro esperado de negócio.

```zt
func find_user(id: int) -> result<text, text>
    if id < 0
        return error("invalid id")
    end

    todo("database lookup")
    return success("Ada")
end
```

O `todo` acima só faz sentido enquanto a busca ainda não existe.

Quando a busca existir, ausencia normal deve virar `optional<T>` ou `result<T,E>`.

## Quando não usar

Não use esses builtins para fluxo normal.

Use `optional<T>` quando faltar valor for esperado.

Use `result<T,E>` quando o chamador puder tratar a falha.
