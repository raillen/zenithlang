# Portal de Documentacao Zenith

Este e o ponto de entrada principal da documentacao tecnica do Zenith.

## 1. Estado Real do Projeto

- [Roadmap de Estabilizacao](roadmap_estabilizacao.md): estado verificado da implementacao ativa, com fases concluidas, fases parciais e trilha self-hosted separada
- [Cronograma Mestre](roadmap/MASTER.md): visao editorial e historica da evolucao do projeto

## 2. Fundacao e Visao

- [Manifesto Zenith](MANIFESTO.md): filosofia, pilares tecnicos e compromisso com acessibilidade
- [Mapa de Intencoes Visuais](specification/intent-map.md): dicionario de simbolos e operadores

## 3. Guia de Aprendizado

- [Manual de Sintaxe](learn/handbook/syntax.md): guia pratico da surface syntax atual
- [Manual de Idiomatismos](learn/handbook/idioms.md): melhores praticas para a trilha ativa
- [Teoria do Compilador](learn/): lexer, parser, binder e runtime

## 4. Especificacao Tecnica

- [Gramatica e Sintaxe](specification/grammar.md): regras formais implementadas hoje
- [Sistema de Tipos](specification/types.md): genericos, Optional, Outcome, nulabilidade e validate
- [Semantica e Escopo](specification/semantics.md): resolucao de nomes, UFCS e indexacao segura
- [Regras de Transpilacao](specification/transpilation.md): mapeamento de Zenith para Lua
- [Sistema de Projeto](specification/project-system.md): modulos, pacotes e organizacao
- [RFC 001 - Self, Atributos e Validate](specification/decisions/001-self-attrs-validate.md): decisao implementada para @, #[...] e validate

## 5. Engine e Runtime

- [Visao do Motor](engine/01-vision.md): pipeline geral de compilacao
- [Plano de Testes Ascension](roadmap/ascension-tests.md): suites e metas de validacao
- [Ascension Compiler](compiler/ascension-zenith.md): estado atual da trilha self-hosted

## 6. API e Suporte

- [Biblioteca Padrao](api/): referencia dos modulos std.*, incluindo std.core, std.text e std.validation
- [Catalogo de Erros](support/error-catalog.md): diagnosticos de compilacao, warnings e runtime
- [Zenith Crowbar e suporte](support/): manutencao, depuracao e IDE

---
Zenith: clareza sintatica, previsibilidade semantica e soberania tecnica.
