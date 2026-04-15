# Modulo std.text

O modulo std.text concentra utilitarios de transformacao, busca, divisao e sanitizacao de strings.

## Transformacao

| API | Descricao |
| :-- | :-- |
| to_upper(t: text) -> text | Maiusculas. |
| to_lower(t: text) -> text | Minusculas. |
| capitalize(t: text) -> text | Primeira letra em destaque. |
| trim(t: text) -> text | Remove espacos laterais. |
| reverse(t: text) -> text | Inverte caracteres. |
| to_snake_case(t: text) -> text | Converte para snake_case. |
| to_camel_case(t: text) -> text | Converte para camelCase. |
| slugify(t: text) -> text | Gera texto amigavel para URL. |
| clean_accents(t: text) -> text | Remove acentos. |

## Busca e predicados

| API | Descricao |
| :-- | :-- |
| contains(t: text, search: text) -> bool | Testa substring. |
| starts_with(t: text, prefix: text) -> bool | Prefixo. |
| ends_with(t: text, suffix: text) -> bool | Sufixo. |
| is_numeric(t: text) -> bool | Apenas numeros. |
| is_emoji(t: text) -> bool | Detecta emoji. |
| count_chars(t: text) -> int | Conta caracteres uteis. |

## Divisao e composicao

| API | Descricao |
| :-- | :-- |
| split(t: text, sep: text) -> list<text> | Divide o texto pelo separador informado. |
| join(parts: list<text>, sep: text) -> text | Junta partes. |
| lines(t: text) -> list<text> | Divide por linhas. |

## Helpers efetivos da trilha ativa

A trilha ativa tambem expoe alguns comportamentos por UFCS virtual no binder e no runtime.

- texto.len() -> int
- texto.to_text() -> text
- texto.split() -> list<text> com separador padrao ","
- texto.split("|") -> list<text>

Observacao importante:

- o modulo formal continua declarando split(t, sep)
- o default "," vale para o caminho UFCS virtual da trilha ativa

## Seguranca e interface

| API | Descricao |
| :-- | :-- |
| mask(t: text, visible_start: int, visible_end: int, char: text = "*" ) -> text | Oculta parte do valor. |
| truncate(t: text, max_len: int, suffix: text = "...") -> text | Corta com sufixo. |

## Exemplo

~~~zt
import std.text

pub func main() -> int
    print(text.slugify("Zenith Lang v2"))
    print("a,b,c".split().len())
    print(text.truncate("documentacao atualizada", 12))
    return 0
end
~~~
