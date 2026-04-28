# `std.list`

Pequenos helpers para valores `list<T>` embutidos.

## Operacoes principais

- `list.is_empty(items)` retorna `true` quando a lista nao tem itens.
- `list.first(items)` retorna o primeiro item como `optional<T>`.
- `list.last(items)` retorna o ultimo item como `optional<T>`.
- `list.rest(items)` retorna todos os itens menos o primeiro.
- `list.skip(items, count)` retorna todos os itens depois de pular `count` itens.

```zt
import std.list as list

const values: list<int> = [4, 8]
const first: optional<int> = list.first(values)
const last: optional<int> = list.last(values)
const rest: list<int> = list.rest(values)
const after_two: list<int> = list.skip(values, 2)

if first.is_some()
    print(first.or(0))
end
```

## Notas

- `list.first(empty_values)` retorna `none` quando `empty_values` nao tem itens.
- `list.last(empty_values)` tambem retorna `none`.
- `list.rest(empty_values)` retorna uma lista vazia.
- `list.rest(single_item)` tambem retorna uma lista vazia.
- `list.skip(items, 0)` retorna uma copia de `items`.
- `list.skip(items, count)` retorna uma lista vazia quando `count` e maior ou igual ao tamanho da lista.
- Use helpers de `optional<T>` como `.is_some()`, `.is_none()` e `.or(fallback)`.
- O backend atual cobre `list<int>` e `list<text>`.
