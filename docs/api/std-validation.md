# Modulo std.validation

std.validation concentra predicados booleanos reutilizaveis para contratos de campo com validate e para regras livres em where.

## APIs disponiveis

| API | Descricao |
| :-- | :-- |
| min_value(it: any, min: any) -> bool | Garante valor minimo. |
| max_value(it: any, max: any) -> bool | Garante valor maximo. |
| between(it: any, min: any, max: any) -> bool | Garante intervalo fechado. |
| one_of(it: any, options: list<any>) -> bool | Garante pertencimento ao conjunto. |
| non_empty(it: any) -> bool | Garante valor nao vazio. |
| not_blank(it: text) -> bool | Garante texto nao branco. |
| len_min(it: any, min: int) -> bool | Garante tamanho minimo. |
| len_max(it: any, max: int) -> bool | Garante tamanho maximo. |
| len_between(it: any, min: int, max: int) -> bool | Garante faixa de tamanho. |
| matches(it: text, pattern: text) -> bool | Garante casamento com padrao. |
| email(it: text) -> bool | Garante formato de email. |
| url(it: text) -> bool | Garante formato de URL. |
| uuid(it: text) -> bool | Garante formato de UUID. |

## Uso com validate

~~~zt
import std.validation as validation

struct User
    age: int validate validation.min_value(18), validation.max_value(120)
    email: text validate validation.email
    name: text validate validation.non_empty, validation.len_max(80)
end
~~~

## Uso com where

where continua sendo o lugar para expressoes livres.

~~~zt
struct Article
    slug: text
        validate validation.non_empty
        where not text.contains(it, " ")
end
~~~

## Observacoes

- validate e exclusivo de campos de struct na trilha ativa
- validate e reescrito para chamadas sobre it
- where e validate podem coexistir no mesmo campo
