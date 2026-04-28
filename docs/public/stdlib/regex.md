# Modulo `std.regex`

`std.regex` oferece regex simples para validacao e busca de texto.

Use quando voce precisa:

- validar se um texto segue um formato;
- testar se um padrao aparece em um texto;
- pegar todas as ocorrencias simples.

## API

```zt
import std.regex as regex

const compiled: regex.Regex = regex.compile("^[A-Za-z]+[0-9]+$")?

if regex.is_match(compiled.pattern, "player123")
    -- texto valido
end

const numbers: list<text> = regex.find_all("[0-9]+", "a12 b7 c345")
```

## Suporte atual

- literais ASCII;
- `.`, `^`, `$`;
- `*`, `+`, `?`;
- classes como `[abc]`, `[a-z]`, `[^0-9]`;
- escapes `\d`, `\w`, `\s`.

## Limites

Este corte nao inclui grupos, capturas, flags, replace ou regex Unicode completa.

Padrao invalido:

- `compile` retorna `error(regex.Error.InvalidPattern)`;
- `is_match` retorna `false`;
- `find_all` retorna lista vazia.
