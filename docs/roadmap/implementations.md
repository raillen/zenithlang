# Roadmap de Implementacoes

> Status: current
> Regra do roadmap: organizar por trilhas tecnicas e nivel de maturidade

Este roadmap responde a pergunta: o que existe hoje, em qual camada, e com qual grau de confianca?

## 1. Linguagem

### Current

- consolidar a linha 0.3.x como referencia da linguagem ativa
- documentar com precisao a diferenca entre trilha ativa e trilha self-hosted
- continuar reduzindo oscilacao sintatica e ambiguidade editorial

## 2. Compilador Ativo (Lua)

### Current

- manter `ztc.lua` como caminho oficial de `check`, `build` e `run`
- preservar parser, binder, runtime e codegen da trilha ativa como referencia operacional
- expandir cobertura de testes e polimento de diagnosticos

## 3. Compilador Self-Hosted (Zenith)

### Current

- tratar `src/compiler/syntax.zt` como caminho canonico do compilador self-hosted
- preservar `check` verde e bootstrap `stage2/stage3` deterministico como baseline de saude
- reduzir `native lua` remanescente sem reabrir o saneamento estrutural ja concluido
- consolidar politica de release, especificacao curta e docs antes de ampliar escopo de paridade

## 4. Stdlib e Ferramental

### Current

- consolidar modulos oficiais presentes
- melhorar coerencia entre docs, testes e disponibilidade real
- seguir reduzindo escapes onde nao forem interop legitima

## 5. Website e Documentacao

### Current

- refletir o estado real da trilha ativa em todos os docs principais
- manter `docs/` como fonte editorial principal
- distinguir docs historicos de docs de estado verificado

## 6. Alvos e Distribuicao

### Current

- manter distribuicao multiplataforma como capacidade da trilha ativa
- lapidar modos standalone e bundle
- fortalecer FFI e empacotamento moderno

### Vision

- compilacao direta para C
- bytecode ou IR mais autonomo
- novos backends nativos
