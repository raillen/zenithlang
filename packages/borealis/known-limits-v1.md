# Borealis Known Limits v1

Data: 2026-04-22
Versao alvo: `0.1.0`

## O que ja esta bom para usar

- arquitetura e naming base do Borealis estao fechados
- `./zt.exe check packages/borealis/zenith.ztproj --all` passa
- runtime desktop com fallback seguro para stub esta funcionando
- exemplo completo de desktop esta disponivel em `packages/borealis/examples/raylib_desktop_app`

## Limites conhecidos do backend C atual

1. Tipos `struct` qualificados por namespace ainda bloqueiam alguns cenarios `run-pass`.
Exemplos:
- `scene.Scene`
- `game.GameContext`
- `entities.Entity`

2. Algumas combinacoes de colecoes tipadas ainda nao estao fechadas no emitter C atual.
Exemplos observados nesta fase:
- `map<int, bool>`
- certos fluxos com `map<int, text>`
- certos retornos de `optional<text>` em modulos Borealis
- `optional<entities.Entity>` em fixture de `scene/entities`

3. Parte dos scaffolds modulares esta validada em `check`, mas nao em `run-pass`.
Isso significa:
- a API esta pronta para evolucao
- a execucao nativa ainda depende de fechar mais compatibilidade no backend C

## Limites do backend desktop

- o adapter Raylib atual depende da biblioteca nativa estar disponivel no ambiente
- quando Raylib nao estiver disponivel, o runtime volta para stub de forma segura
- esta fase ainda nao fecha benchmark/perfil visual real como criterio de release
- no workspace local, o compilador agora prioriza runtime/stdlib ao lado do `./zt.exe` antes de usar `ZENITH_HOME`, para evitar divergencia entre versao instalada e versao em edicao

## Limites de package/ZPM

- o manifesto do Borealis ja esta preparado como lib versionada
- a decisao de ZPM ja esta alinhada
- o comando `zpm` ainda nao esta disponivel neste workspace, entao a distribuicao permanece preparada, mas nao publicada

## O que fica para a proxima volta tecnica

1. fechar compatibilidade do emitter C para tipos qualificados
2. fechar compatibilidade de colecoes tipadas que ainda falham em `run-pass`
3. retomar os fixtures Borealis hoje bloqueados no backend C
