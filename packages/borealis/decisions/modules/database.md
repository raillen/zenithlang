# Borealis Module Decision - Database

- Status: accepted
- Date: 2026-04-23
- Type: module / database backend
- Scope: `borealis.game.database`

## Summary

`borealis.game.database` continua sendo um modulo futuro para bancos reais, como SQLite.

Mesmo assim, nesta fase ele ja ganhou um stub em memoria mais util para:

- exemplos;
- testes;
- prototipos de fluxo com conexao, execucao e consulta.

## Direcao de implementacao

- manter `database` separado de `save` e `storage`;
- tratar banco como camada de query/update, nao como armazenamento generico;
- comecar pequeno, mas com contrato claro;
- preparar o caminho para um backend nativo depois.

## Estado atual da implementacao

Hoje o modulo trabalha com conexoes stub identificadas por `id`.

Ele ja oferece:

- abertura e fechamento de conexao;
- checagem de conexao aberta;
- `exec` com retorno simples de linhas afetadas no stub;
- `query` com resposta deterministica para testes;
- rastreamento do ultimo SQL por conexao.

## API atual

- `open(config) -> result<DatabaseConnection, core.Error>`: abre uma conexao stub.
- `close(connection) -> DatabaseConnection`: fecha a conexao.
- `is_open(connection) -> bool`: informa se a conexao ainda esta aberta.
- `exec(connection, sql) -> result<int, core.Error>`: executa SQL sem retorno de linhas.
- `query(connection, sql) -> result<text, core.Error>`: executa consulta stub e devolve texto previsivel.
- `last_sql(connection) -> optional<text>`: retorna o ultimo SQL executado na conexao.
- `reset() -> void`: limpa o estado interno do modulo.

## Regras de comportamento

- `driver` vazio gera erro em `open`;
- `sql` vazio gera erro em `exec` e `query`;
- conexao fechada gera erro em `exec` e `query`;
- `query` monta uma resposta previsivel com `driver`, `path` e `sql`;
- `last_sql(...)` retorna `none` quando a conexao ainda nao executou nada;
- `reset()` existe para testes, exemplos e scaffolds.

## Proximos passos desejados

- `exec` e `query` com backend SQLite real;
- resultados estruturados em linhas/colunas;
- statements preparados e bind de parametros;
- transacoes e helpers de migracao.
