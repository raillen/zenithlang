# Borealis Module Decision - Storage

- Status: accepted
- Date: 2026-04-23
- Type: module / generic persistence
- Scope: `borealis.game.storage`

## Summary

`borealis.game.storage` e a camada generica de persistencia local.

Ela existe para guardar dados simples que nao sao necessariamente "save de progresso", como:

- cache local;
- preferencias auxiliares;
- blobs de texto;
- dados temporarios entre cenas e ferramentas.

## Direcao de implementacao

- API pequena e clara;
- separar `valor` de `existencia`;
- servir como base futura para `save`, cache e arquivos reais;
- manter operacoes comuns faceis para quem esta prototipando.

## Estado atual da implementacao

Hoje o modulo usa duas estruturas internas:

- `storage_values: map<text, text>` guarda o valor;
- `storage_present: map<text, bool>` guarda se a chave existe.

Isso permite um comportamento correto e facil de explicar:

- `""` continua sendo valor valido;
- chave removida volta `none`;
- `exists(key)` nao depende do conteudo salvo.

## API atual

- `write(key, value) -> result<void, core.Error>`: grava um valor de texto em uma chave.
- `read(key) -> optional<text>`: le o valor salvo. Retorna `none` se a chave nao existir.
- `remove(key) -> result<void, core.Error>`: remove logicamente a chave.
- `exists(key) -> bool`: informa se a chave existe.
- `copy(from, to) -> result<void, core.Error>`: copia o valor de uma chave para outra.
- `move(from, to) -> result<void, core.Error>`: move o valor e remove a chave antiga.
- `clear() -> void`: limpa todo o armazenamento em memoria.

## Regras de comportamento

- chave vazia gera erro em escrita, remocao, copia e movimento;
- `read("")` retorna `none`;
- string vazia ainda e um valor valido;
- `copy` e `move` falham se a origem nao existir;
- `clear()` existe para testes, scaffolds e prototipos locais.

## Proximos passos desejados

- `write_text`, `write_json`, `write_binary`;
- backend real de arquivo;
- listagem de entradas por prefixo ou pasta logica;
- integracao futura com cache de assets e save persistente.
