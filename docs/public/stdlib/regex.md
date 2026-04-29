# Modulo `std.regex`

`std.regex` oferece regex simples para validacao e busca de texto.

Use quando voce precisa:

- validar se um texto segue um formato;
- testar se um padrao aparece em um texto;
- pegar todas as ocorrencias simples;
- dividir ou substituir texto com um padrao simples.

## API

```zt
import std.regex as regex

const compiled: regex.Regex = regex.compile("^[A-Za-z]+[0-9]+$")?

if regex.is_match(compiled.pattern, "player123")
    -- texto valido
end

const inteiro: bool = regex.full_match("[A-Za-z]+", "player")?
const primeiro_numero: optional<text> = regex.first("[0-9]+", "a12 b7")
const numbers: list<text> = regex.find_all("[0-9]+", "a12 b7 c345")
const partes: list<text> = regex.split("[,;]+", "red,green;blue")
const mascarado: text = regex.replace_all("[0-9]+", "a12 b7", "N")
const padrao_literal: text = regex.escape("a.c+")
```

Helpers principais:

| Helper | Uso |
| --- | --- |
| `regex.is_valid(pattern)` | Verifica se o padrao e aceito. |
| `regex.matches(pattern, input)` | Faz match com erro explicito para padrao invalido. |
| `regex.contains(pattern, input)` | Alias legivel de `is_match`. |
| `regex.full_match(pattern, input)` | Exige que o texto inteiro combine. |
| `regex.first(pattern, input)` | Retorna o primeiro match como `optional<text>`. |
| `regex.count(pattern, input)` | Conta matches nao vazios. |
| `regex.find_all(pattern, input)` | Retorna todos os matches nao vazios. |
| `regex.split(pattern, input)` | Divide por matches nao vazios. |
| `regex.replace_all(pattern, input, replacement)` | Substitui todos os matches nao vazios. |
| `regex.escape(input)` | Escapa metacaracteres basicos de regex. |

## Suporte atual

- literais ASCII;
- `.`, `^`, `$`;
- `*`, `+`, `?`;
- classes como `[abc]`, `[a-z]`, `[^0-9]`;
- escapes `\d`, `\w`, `\s`.

## Limites

Este corte nao inclui grupos, capturas, flags ou regex Unicode completa.

Padrao invalido:

- `compile` retorna `error(regex.Error.InvalidPattern)`;
- `matches` e `full_match` retornam `error(regex.Error.InvalidPattern)`;
- `is_match` retorna `false`;
- `first` retorna `none`;
- `count` retorna `0`;
- `find_all` retorna lista vazia;
- `split` retorna uma lista com o texto original;
- `replace_all` retorna o texto original.
