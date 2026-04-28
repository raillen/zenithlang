# Modulo `std.regex`

`std.regex` ofrece helpers simples de regex para validar y buscar texto.

Usalo cuando necesitas:

- validar un formato de texto;
- comprobar si un patron aparece en un texto;
- recoger coincidencias simples.

## API

```zt
import std.regex as regex

const compiled: regex.Regex = regex.compile("^[A-Za-z]+[0-9]+$")?

if regex.is_match(compiled.pattern, "player123")
    -- texto valido
end

const numbers: list<text> = regex.find_all("[0-9]+", "a12 b7 c345")
```

## Soporte actual

- literales ASCII;
- `.`, `^`, `$`;
- `*`, `+`, `?`;
- clases como `[abc]`, `[a-z]`, `[^0-9]`;
- escapes `\d`, `\w`, `\s`.

## Limites

Este corte no incluye grupos, capturas, flags, replace ni regex Unicode completa.

Patron invalido:

- `compile` retorna `error(regex.Error.InvalidPattern)`;
- `is_match` retorna `false`;
- `find_all` retorna una lista vacia.
