# Mapeamento mental para Zenith

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Resumo

Esta página mostra como escrever em Zenith conceitos comuns de outras linguagens.

Ela evita procurar features que Zenith decidiu resolver de outro jeito.

## Tabela rápida

| Em outras linguagens | Em Zenith |
| --- | --- |
| `null` | `optional<T>` e `none` |
| exception recuperavel | `result<T,E>` |
| `throw` fatal | `panic(...)` |
| union/sum type | `enum` com payload |
| abstract method | `trait` |
| virtual method | `dyn<Trait>` |
| partial class | `struct` + `apply` |
| interface | `trait` |
| class data + methods | `struct` + `apply` |
| string | `text` |
| implicit conversion to string | `to_text(value)` |

## Exemplos

Ausencia:

```zt
func find_user(id: int) -> optional<text>
    return none
end
```

Erro:

```zt
func read_config(path: text) -> result<text, text>
    if path == ""
        return error("path is empty")
    end

    return success("{}")
end
```

Union segura:

```zt
enum Message
    Text(value: text)
    Quit
end
```

Contrato:

```zt
trait Drawable
    func draw()
end
```

## Onde ver receitas completas

- `docs/public/cookbook/absence-without-null.md`
- `docs/public/cookbook/errors-without-exceptions.md`
- `docs/public/cookbook/safe-union-with-enum.md`
- `docs/public/cookbook/abstract-methods-with-trait.md`
- `docs/public/cookbook/virtual-dispatch-with-dyn.md`
- `docs/public/cookbook/partial-class-with-apply.md`
