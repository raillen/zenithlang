# Módulo `std.text`

O `std.text` concentra utilitários de transformação, busca, divisão e sanitização de strings.

## Transformação

| API | Descrição |
| :-- | :-- |
| `to_upper(t: text) -> text` | Maiúsculas. |
| `to_lower(t: text) -> text` | Minúsculas. |
| `capitalize(t: text) -> text` | Primeira letra em destaque. |
| `trim(t: text) -> text` | Remove espaços laterais. |
| `reverse(t: text) -> text` | Inverte caracteres. |
| `to_snake_case(t: text) -> text` | Converte para `snake_case`. |
| `to_camel_case(t: text) -> text` | Converte para `camelCase`. |
| `slugify(t: text) -> text` | Gera texto amigável para URL. |
| `clean_accents(t: text) -> text` | Remove acentos. |

## Busca e predicados

| API | Descrição |
| :-- | :-- |
| `contains(t: text, search: text) -> bool` | Testa substring. |
| `starts_with(t: text, prefix: text) -> bool` | Prefixo. |
| `ends_with(t: text, suffix: text) -> bool` | Sufixo. |
| `is_numeric(t: text) -> bool` | Apenas números. |
| `is_emoji(t: text) -> bool` | Detecta emoji. |
| `count_chars(t: text) -> int` | Conta caracteres úteis. |

## Divisão e composição

| API | Descrição |
| :-- | :-- |
| `split(t: text, sep: text) -> list<text>` | Divide o texto. |
| `join(parts: list<text>, sep: text) -> text` | Junta partes. |
| `lines(t: text) -> list<text>` | Divide por linhas. |

## Segurança e interface

| API | Descrição |
| :-- | :-- |
| `mask(t: text, visible_start: int, visible_end: int, char: text = "*") -> text` | Oculta parte do valor. |
| `truncate(t: text, max_len: int, suffix: text = "...") -> text` | Corta com sufixo. |

## Exemplo

```zt
import std.text

pub func main() -> int
    print(text.slugify("Zenith Lang v2"))
    print(text.truncate("documentação atualizada", 12))
    return 0
end
```
