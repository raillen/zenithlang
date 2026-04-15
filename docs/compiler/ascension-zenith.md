# Ascension Compiler (Zenith)

> Status atual: trilha self-hosted parcial
> Implementacao ativa oficial hoje: `ztc.lua` + parser/binder/codegen em Lua
> Leitura recomendada do estado real: `../roadmap_estabilizacao.md`

Ascension continua sendo a linha self-hosted do projeto, mas nao deve ser descrita como "motor concluido" neste momento. O repositorio tem duas trilhas tecnicas diferentes:

- trilha ativa: usada hoje para `check`, `build` e `run`
- trilha self-hosted: `src/compiler/*.zt`, ainda em busca de paridade

## O que e verdade hoje

- a trilha ativa em Lua compila e testa as features principais da linguagem
- a base `.zt` do compilador existe e avancou, mas ainda depende de bootstrap e escapes
- self-hosting completo ainda nao e um estado verificado do repositorio atual

## O que Ascension entrega hoje

- experimentacao concreta de parser, binder, lowering, diagnostics e codegen em Zenith
- base para testes de soberania em `tests/ascension/`
- terreno de migracao para reduzir dependencia do bootstrap em Lua

## O que ainda falta para chamar de self-hosting completo

- `ztc check src/compiler/syntax.zt` sem falhas massivas
- remocao dos hacks principais de `native lua`
- paridade minima entre a sintaxe usada pelo compilador self-hosted e a linguagem ativa
- caminho verificavel de bootstrap confiavel

## Onde isso mora no repositorio

- `src/compiler/`
- `src/compiler/syntax.zt`
- `src/compiler/syntax_bridge.zt`
- `tests/ascension/`

## Papel atual do bootstrap em Lua

O bootstrap em Lua ainda e o caminho oficial de compilacao e validacao. Ascension, hoje, deve ser tratado como trilha de paridade/self-hosting e nao como substituto concluido da trilha ativa.
