# Borealis Migration Guide v1

Data: 2026-04-22
Versao alvo: `0.1.0`

## Resumo

Nesta baseline nao ha migracao quebrando API publica do Borealis.

Se voce estiver chegando agora:

- use `borealis.game` como entrada padrao
- use `borealis.engine` apenas quando precisar de controle tecnico explicito

## Nomes canonicos atuais

Loop facil:

- `game.start(...)`
- `game.running(...)`
- `game.frame_begin(...)`
- `game.frame_end(...)`
- `game.close(...)`

Input por frame:

- `game.key_down(...)`
- `game.key_pressed(...)`
- `game.key_released(...)`

## Sem alias legado obrigatorio

Nesta fase:

- nao existe tabela de alias legado exigindo migracao
- nao existe rename obrigatorio publicado para consumidores

## Recomendacao de adoção

1. Comece por `minimal_loop.zt`.
2. Passe para `render2d_base.zt`.
3. Adicione `scene` e `entities` quando precisar de organizacao.
4. Promova para `engine.ecs` quando o projeto pedir mais controle.
