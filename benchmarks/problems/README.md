# Micro Problem Set

Este conjunto define problemas pequenos e estaveis para regressao.

O benchmark executavel fica em `tests/perf/micro_algorithm_core`.

## Problemas

- fibonacci: chamada recursiva pequena, usada para custo de chamada e CPU.
- sort: ordenacao por insercao em lista curta, usada para loops, indexacao e escrita em lista.
- string ops: `trim`, busca, prefixo, sufixo e indice em `std.text`.
- allocation: criacao de listas e fatias em loop, usada para custo de alocacao basico.

## Escopo

Estes casos nao tentam representar uma aplicacao completa.

Eles existem para responder uma pergunta simples: uma mudanca deixou um caminho essencial claramente mais lento?

