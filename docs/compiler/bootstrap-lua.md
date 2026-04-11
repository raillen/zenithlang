# Bootstrap Compiler (Lua)

> Status: `Deprecated`
> Papel atual: `ztc.lua` como legado técnico preservado
> Recomendado para: estudo histórico, comparação de arquitetura e manutenção do legado.

O Bootstrap Compiler em Lua foi a primeira linha operacional do Zenith. Ele tornou possível validar a linguagem, amadurecer a sintaxe e abrir o caminho para que Ascension chegasse ao ponto de auto-hospedagem completa.

## O papel dele hoje

Eu não trato essa linha como lixo técnico ou algo a ser escondido. Hoje ela continua importante como:

- memoria operacional da linguagem
- referência histórica para quem quer entender a evolução do compilador
- base comparativa para estudos, auditoria e preservacao do legado

## O que ele entregou ao Zenith

- parsing
- binding
- lowering
- codegen Lua
- runtime base
- suporte a `.ztproj`
- o primeiro caminho estavel de execucao e testes do ecossistema

## Onde ele mora

Os pontos mais importantes do bootstrap estao em:

- `ztc.lua`
- `src/syntax/`
- `src/semantic/`
- `src/lowering/`
- `src/backend/lua/`

## Uso recomendado

Quem quiser contribuir com a linha oficial deve estudar primeiro Ascension. O bootstrap fica preservado, mas separado, para evitar confusao entre o caminho atual e a história técnica do projeto.