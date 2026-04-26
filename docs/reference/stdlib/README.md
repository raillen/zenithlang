# Standard Library Reference

> Referencia publica da stdlib.
> Audience: package-author, advanced-user
> Surface: reference
> Status: current

## Objetivo

Consultar APIs publicas da stdlib alpha.

## Paginas

| Pagina | Uso |
| --- | --- |
| `modules.md` | lista de modulos e responsabilidade |
| `io-json.md` | entrada/saida e JSON |
| `text-bytes-format.md` | texto, bytes e formatacao |
| `filesystem-os-time.md` | filesystem, caminhos, OS, processo e tempo |
| `collections.md` | estruturas auxiliares em `std.collections`, `std.list` e `std.map` |
| `math-random-validate.md` | math, random e validacao |
| `concurrency-lazy-test-net.md` | concorrencia base, lazy, test e net |

## Fontes

- Codigo: `stdlib/std/`.
- ZDoc: `stdlib/zdoc/`.
- Modelo normativo: `language/spec/stdlib-model.md`.

## Regra

A stdlib esta em alpha.

Antes de publicar exemplo novo, valide com `zt check`.
