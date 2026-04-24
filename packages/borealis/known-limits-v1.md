# Borealis Known Limits v1

Data: 2026-04-22
Versao alvo: `0.1.0`

## O que ja esta bom para usar

- arquitetura e naming base do Borealis estao fechados
- `./zt.exe check packages/borealis/zenith.ztproj --all` passa
- runtime desktop com fallback seguro para stub esta funcionando
- binding `borealis.raylib` ja cobre smoke de shapes, texto, input, `measure_text`, textura/som e helpers matematicos em modo stub-safe
- backend Raylib real foi validado no workspace Windows x64 com binario vendorizado em `packages/borealis/native/raylib/windows-x64/lib/raylib.dll`
- exemplo completo de desktop esta disponivel em `packages/borealis/examples/raylib_desktop_app`

## Limites conhecidos do backend C atual

1. O caminho estrutural que bloqueava `optional<Struct>` e tipos qualificados do Borealis foi fechado nesta fase.
Isso inclui:
- retorno direto de `struct` para `optional<Struct>`
- atribuicao implicita para `optional<Struct>`
- campo `optional<Struct>` dentro de outra `struct`
- payload gerenciado como `list<text>` dentro da `struct`

2. Ainda vale continuar o hardening do emitter C para combinacoes mais amplas de generics ricos.
Exemplos de trilha futura:
- composicoes mais profundas de `map`, `optional` e `outcome`
- ampliacao de coverage do ECS tipado alem dos stubs atuais
- redução de casos especiais no emitter para manter previsibilidade

3. Parte dos modulos do Borealis ainda esta em scaffold funcional inicial.
Isso significa:
- a arquitetura publica esta pronta para evolucao
- o proximo ganho principal passa a ser aprofundar comportamento de package, nao destravar `optional<Struct>` no backend C

## Limites do backend desktop

- o adapter Raylib atual depende da biblioteca nativa estar disponivel no ambiente ou em `packages/borealis/native/raylib`
- quando Raylib nao estiver disponivel, o runtime volta para stub de forma segura
- o runtime ja procura `BOREALIS_RAYLIB_PATH`, `ZENITH_RAYLIB_PATH`, o diretorio do executavel, o diretorio atual e o layout nativo do modulo, incluindo subpastas `lib` do layout oficial
- esta fase ainda nao fecha benchmark/perfil visual real como criterio de release
- no workspace local, o compilador agora prioriza runtime/stdlib ao lado do `./zt.exe` antes de usar `ZENITH_HOME`, para evitar divergencia entre versao instalada e versao em edicao

## Limites de package/ZPM

- o manifesto do Borealis ja esta preparado como lib versionada
- a decisao de ZPM ja esta alinhada
- o comando `zpm` ainda nao esta disponivel neste workspace, entao a distribuicao permanece preparada, mas nao publicada

## O que fica para a proxima volta tecnica

1. aprofundar os modulos Borealis alem do scaffold
2. ampliar coverage de testes para combinacoes mais ricas de generics no emitter C
3. validar o backend Raylib real com a biblioteca nativa presente no ambiente ou no layout `packages/borealis/native/raylib`
