# Roadmap de Consolidação: Zenith Core

Este documento acompanha a consolidação técnica do núcleo da linguagem Zenith e registra o caminho que levou at? Ascension Final.

## 1. Visão Geral

Eu mantenho Zenith como uma linguagem focada em acessibilidade cognitiva, clareza sintática e maturidade técnica, sem abrir mao de performance nem de leitura hospitaleira.

## 2. Fases 1 a 3: O Coração (v0.2.5) - Concluído

Foco: estabelecer a gramática, o sistema de tipos e a infraestrutura básica.

- [X] ADTs e `match`
- [X] sistema de tipos com `trait`, genericos e `where`
- [X] async/await integrado
- [X] interoperabilidade com `native lua`
- [X] lambdas, spread, slicing e ergonomia de superfície

## 3. Fase 4: Tooling e Fundação (v0.2.5) - Concluído

Foco: permitir que Zenith interagisse com o mundo real.

- [X] CLI estavel com suporte a build e run
- [X] project system via `.ztproj`
- [X] bibliotecas base como `std.os`, `std.fs` e `std.json`

## 4. Fase 5: Utilidades e Ecossistema (v0.2.6 - v0.2.7) - Concluído

Foco: expandir o ferramental para uso pratico.

- [X] `std.time` e `std.text`
- [X] ZPM para dependencias externas
- [X] manual interativo e recursos de apoio

## 5. Fase 6: Blindagem e Qualidade (v0.2.8 - v0.2.9) - Concluído

Foco: blindar a plataforma antes do self-hosting.

- [X] ZTest integrado ao CLI
- [X] relatórios visuais e exportacao de dados
- [X] execucao paralela e isolamento de suites
- [X] snapshot testing e ampliacao de cobertura global
## 6. Fase 7: Zenith Ascension (v0.3.0 - v0.3.5) - Em Aberto

Foco: auto-hospedagem, otimização e soberania total do pipeline.

- [X] **v0.3.0 (Soberano)**: Motor Ascension Finalizado (FFI, Multi-target, Bundle).
- [X] **v0.3.1 (ZDoc)**: Suporte a arquivos .zdoc e Pureza CLI consolidada.
- [ ] **v0.3.2 (Lexer)**: Analisador léxico reescrito em Zenith puro.
- [ ] **v0.3.3 (Parser)**: Algoritmo Pratt Parser portado para Zenith.
- [ ] **v0.3.4 (Binder)**: Tabela de símbolos e resolução semântica nativa.
- [ ] **v0.3.5 (Emitter)**: Gerador de código (Codegen) operando em Zenith.

---
## 7. Estudos que continuam depois de Ascension

Esses itens registram a transição para o ecossistema Zenith v0.3.1:

- [X] **Zenith Doc-Linking (ZDoc)**: Suporte a arquivos `.zdoc` externos para documentação desacoplada, utilizando a tag `@target` para associar prosa a símbolos sem poluir o código-fonte.
- [X] **Módulo Regex Nativo**: Implementação do Fluent Builder para construção de padrões rítmicos e legíveis.
- [X] **Manual Interativo (ZMan)**: Refatoração para Zenith puro, integrando o sistema de Regex para colorização sem dependência de blocos nativos.
- [X] **Ampliação da Pureza CLI**: Redução drástica de `native lua` nas ferramentas centrais. ZPM, ZTest e ZMan agora são orquestrados em Zenith soberano.

## 8. Próximos Passos (Transcendência v1.0)

Com o motor Ascension Finalizado e a CLI Purificada, o foco desloca-se para a fundação visual e industrial:

- [ ] **Zenith Intersect (UI)**: Framework declarativo e reativo.
- [ ] **Zenith Warp (Games)**: Motor de jogo industrial sobre Raylib.
- [ ] **Zenith Orion (Data)**: ORM tipo-seguro para SQLite/Postgres.

*Atualizado em: 2026-04-11 (Zenith v0.3.1)*