# Modulo `std.regex`

`std.regex` ofrece helpers simples de regex para validar y buscar texto.

Usalo cuando necesitas:

- validar un formato de texto;
- comprobar si un patron aparece en un texto;
- recoger coincidencias simples;
- dividir o reemplazar texto con un patron simple.

## API

```zt
import std.regex as regex

const compiled: regex.Regex = regex.compile("^[A-Za-z]+[0-9]+$")?

if regex.is_match(compiled.pattern, "player123")
    -- texto valido
end

const completo: bool = regex.full_match("[A-Za-z]+", "player")?
const primer_numero: optional<text> = regex.first("[0-9]+", "a12 b7")
const numbers: list<text> = regex.find_all("[0-9]+", "a12 b7 c345")
const partes: list<text> = regex.split("[,;]+", "red,green;blue")
const reemplazado: text = regex.replace_all("[0-9]+", "a12 b7", "N")
const patron_literal: text = regex.escape("a.c+")
```

Helpers principales:

| Helper | Uso |
| --- | --- |
| `regex.is_valid(pattern)` | Verifica si el patron es valido. |
| `regex.matches(pattern, input)` | Match con error explicito para patron invalido. |
| `regex.contains(pattern, input)` | Alias claro de `is_match`. |
| `regex.full_match(pattern, input)` | Exige que todo el texto coincida. |
| `regex.first(pattern, input)` | Devuelve el primer match como `optional<text>`. |
| `regex.count(pattern, input)` | Cuenta matches no vacios. |
| `regex.find_all(pattern, input)` | Devuelve todos los matches no vacios. |
| `regex.split(pattern, input)` | Divide por matches no vacios. |
| `regex.replace_all(pattern, input, replacement)` | Reemplaza todos los matches no vacios. |
| `regex.escape(input)` | Escapa metacaracteres basicos. |

## Soporte actual

- literales ASCII;
- `.`, `^`, `$`;
- `*`, `+`, `?`;
- clases como `[abc]`, `[a-z]`, `[^0-9]`;
- escapes `\d`, `\w`, `\s`.

## Limites

Este corte no incluye grupos, capturas, flags ni regex Unicode completa.

Patron invalido:

- `compile` retorna `error(regex.Error.InvalidPattern)`;
- `matches` y `full_match` retornan `error(regex.Error.InvalidPattern)`;
- `is_match` retorna `false`;
- `first` retorna `none`;
- `count` retorna `0`;
- `find_all` retorna una lista vacia;
- `split` retorna una lista con el texto original;
- `replace_all` retorna el texto original.
