# Roadmap de Implementações

> Status: `Current`
> Regra do roadmap: por trilhas de implementação, não por sprints.

Eu mantenho este roadmap para responder a pergunta certa:

> o que estou construindo, em que camada, e com qual grau de maturidade?

Eu não organizo essa visão por sprint porque, para Zenith, faz mais sentido acompanhar trilhas de implementação e amadurecimento técnico.

## 1. Linguagem

### Current

- consolidar a linha `0.3.x` como referência auto-hospedada da linguagem
- documentar com precisão a diferença entre current, legacy e vision
- continuar reduzindo oscilacao sintática e ambiguidade editorial

## 2. Bootstrap Compiler (Lua)

### Deprecated

- preservar o `ztc.lua` como legado técnico separado
- manter documentação clara para estudo histórico e comparação de arquitetura
- evitar que essa linha volte a ser confundida com o caminho oficial de build

## 3. Ascension Compiler (Zenith)

### Current

- manter o motor concluído saudavel, legivel e auditavel
- expandir otimizações, tooling nativo e ergonomia do ecossistema
- fortalecer distribuição, empacotamento e observabilidade do pipeline

## 4. Stdlib e Ferramental

### Current

- consolidar módulos oficiais j? presentes
- melhorar coerencia entre docs, testes e disponibilidade real
- fortalecer ZPM, ZTest e utilitarios de suporte

## 5. Website e Documentação

### Current

- refletir Ascension como linha oficial em todas as camadas do site
- manter `docs/` como fonte ?nica da verdade editorial
- mostrar com clareza badges como `Current`, `Stable`, `Deprecated` e `Vision`

## 6. Alvos e Distribuição

### Current

- manter distribuição multiplataforma como capacidade real do Zenith atual
- lapidar modos standalone e bundle
- fortalecer FFI e caminhos de empacotamento moderno

### Vision

- compilação direta para C
- bytecode ou IR mais autonoma
- novos backends nativos
- distribuição ainda mais independente do backend histórico