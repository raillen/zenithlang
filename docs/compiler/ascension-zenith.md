# Ascension Compiler (Zenith)

> Status atual: core self-hosted canonico estabilizado; consolidacao industrial ainda em andamento
> Implementacao ativa oficial hoje: `ztc.lua` + parser/binder/codegen em Lua
> Leitura recomendada do estado real: `../roadmap_estabilizacao.md`

Ascension continua sendo a linha self-hosted do projeto, mas nao deve ser descrita como "motor concluido" neste momento. O repositorio tem duas trilhas tecnicas diferentes:

- trilha ativa: usada hoje para `check`, `build` e `run`
- trilha self-hosted: `src/compiler/syntax.zt` como caminho canonico; `syntax_bridge.zt` congelado como legado

## O que e verdade hoje

- a trilha ativa em Lua compila e testa as features principais da linguagem
- a base `.zt` do compilador valida bootstrap canonico e nao carrega blocos `native lua` remanescentes
- o core self-hosted ja e estado verificado; o residuo agora e consolidacao de produto e docs

## O que Ascension entrega hoje

- experimentacao concreta de parser, binder, lowering, diagnostics e codegen em Zenith
- base para testes de soberania em `tests/ascension/`
- terreno de migracao para reduzir dependencia do bootstrap em Lua

## O que ainda falta para chamar de self-hosting completo

- manutencao do contrato sem `native lua` em `.zt`
- especificacao curta do core estabilizado
- limpeza documental definitiva do `syntax_bridge.zt` como legado
- hardening pontual fora do bootstrap, sem reabrir o core

## Onde isso mora no repositorio

- `src/compiler/`
- `src/compiler/syntax.zt`
- `src/compiler/syntax_bridge.zt`
- `tests/ascension/`

## Papel atual do bootstrap em Lua

O bootstrap em Lua ainda e o caminho oficial de compilacao e validacao. Ascension, hoje, deve ser tratado como trilha de paridade/self-hosting e nao como substituto concluido da trilha ativa.
