# `std.set`

> Status: alpha.
> Suporte atual do backend: `set<int>` e `set<text>`.

Use `std.set` para conjuntos hash-based.

```zt
import std.set as set
```

## Operacoes principais

- `set.empty()` cria um conjunto vazio.
- `set.of(a, b, c)` cria um conjunto a partir de valores.
- `set.is_empty(values)` retorna `true` quando o conjunto nao tem itens.
- `set.len(values)` retorna a quantidade de itens unicos.
- `set.has(values, value)` verifica se um valor existe.
- `set.add(values, value)` adiciona um item em um conjunto mutavel.
- `set.remove(values, value)` remove um item de um conjunto mutavel.
- `set.union(left, right)` retorna os itens dos dois conjuntos.
- `set.intersect(left, right)` retorna apenas os itens compartilhados.
- `set.difference(left, right)` retorna itens de `left` que nao existem em `right`.
- `for item in values` visita cada item uma vez.

`set.union`, `set.intersect` e `set.difference` retornam um novo conjunto.

```zt
var values: set<int> = set.empty()
set.add(values, 1)
set.add(values, 2)
set.remove(values, 1)

const left: set<int> = {1, 2, 3}
const right: set<int> = {3, 4}

const both: set<int> = set.union(left, right)
const shared: set<int> = set.intersect(left, right)
const only_left: set<int> = set.difference(left, right)

var total: int = 0
for item in both
    total = total + item
end
```

## Notas

- `set.empty()` precisa de um tipo esperado, como `var values: set<int> = set.empty()`.
- `set.of(...)` pode inferir o tipo dos itens a partir dos valores.
- Use `var`, nao `const`, ao chamar `set.add` ou `set.remove`.
- Os dois argumentos devem ter o mesmo tipo de set.
- `set<int>` combina apenas com `set<int>`.
- `set<text>` combina apenas com `set<text>`.
- A ordem de iteracao de `set` nao faz parte do contrato da linguagem.
