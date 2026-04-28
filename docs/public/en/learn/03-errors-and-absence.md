# 03 - Ausencia e erros

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Objetivo

Escolher entre `optional<T>`, `result<T,E>` e `panic`.

## Regra curta

Use:

- `optional<T>` para ausencia normal;
- `result<T,E>` para falha recuperavel com motivo;
- `panic(...)` para erro fatal;
- `todo(...)` para caminho incompleto que ainda não deve passar silenciosamente;
- `unreachable(...)` para caminho que deveria ser impossível;
- `check(condition, message)` para validar uma condição interna com contexto.

## Ausencia com `optional`

```zt
func find_name(id: int) -> optional<text>
    if id == 1
        return "Ada"
    end

    return none
end
```

Helpers pequenos:

```zt
const name: optional<text> = find_name(2)

if name.is_none()
    return "missing"
end

return name.or("anonymous")
```

Para sair cedo quando o valor esta ausente:

```zt
func score_or_default(input: optional<int>) -> int
    const score: int = input.or_return(0)
    return score + 1
end
```

## Erro recuperavel com `result`

```zt
func parse_port(value: text) -> result<int, text>
    if value == ""
        return error("port is empty")
    end

    return success(8080)
end
```

Helpers pequenos:

```zt
const port: result<int, text> = parse_port("8080")

if port.is_error()
    return 1
end

return 0
```

Use `match` quando precisar extrair o valor com lógica propria.

Para adicionar contexto a um `core.Error` antes de propagar:

```zt
func load_score() -> result<int, core.Error>
    const score: int = read_score().or_wrap("load score")?
    return success(score)
end
```

## Erro fatal com `panic`

```zt
func require_ready(ready: bool)
    if not ready
        panic("system is not ready")
    end
end
```

## Falha intencional com contexto

```zt
func calculate_score(value: int) -> int
    check(value > 0, "score must be positive")
    return value * 2
end

func not_ready_yet() -> int
    todo("finish score rules")
    return 0
end

func impossible_state() -> int
    unreachable("state machine reached impossible branch")
    return 0
end
```

## Como decidir

| Situação | Use |
| --- | --- |
| item não encontrado | `optional<T>` |
| arquivo não abriu | `result<T,E>` |
| contrato interno quebrou | `panic(...)` |
| regra ainda não implementada | `todo(...)` |
| caminho impossível foi executado | `unreachable(...)` |
| condição interna falsa | `check(condition, message)` |

## Próximo

Leia:

- `docs/public/cookbook/absence-without-null.md`
- `docs/public/cookbook/errors-without-exceptions.md`
- `docs/public/cookbook/intentional-failure-builtins.md`
