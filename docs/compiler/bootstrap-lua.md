# Bootstrap Compiler (Lua)

> Status: current
> Papel atual: implementacao ativa oficial via `ztc.lua`
> Recomendado para: build real, check, run, testes e manutencao principal da trilha ativa.

O Bootstrap Compiler em Lua nao deve mais ser lido como legado tecnico. Hoje ele e a linha operacional do Zenith: parser, binder, lowering, runtime e codegen usados nas validacoes reais do repositorio.

## O papel dele hoje

- caminho oficial de `check`, `build` e `run`
- base real dos testes que validam as fases estabilizadas
- referencia operacional da linguagem atual
- plataforma de comparacao para a trilha self-hosted

## O que ele entrega ao Zenith

- parsing
- binding
- lowering
- codegen Lua
- runtime base
- source maps simples
- o caminho estavel de execucao e testes do ecossistema atual

## Onde ele mora

- `ztc.lua`
- `src/syntax/`
- `src/semantic/`
- `src/lowering/`
- `src/backend/lua/`

## Relacao com Ascension

Ascension continua importante, mas hoje ele deve ser tratado como trilha self-hosted em paridade parcial. O bootstrap em Lua permanece como caminho principal ate que a trilha self-hosted tenha validacao equivalente.
