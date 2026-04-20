# Portal de Documentacao Zenith

Este e o ponto de entrada principal da documentacao tecnica do Zenith.

## 1. Estado Atual

- [Zenith Current](language/current.md): linha correta da linguagem atual
- [Core Atual Estabilizado](specification/current-core.md): contrato curto do core e do produto atual
- [Self-Hosted Pos-100](roadmap/selfhost-pos100.md): fechamento do residual operacional/editorial
- [Cronograma Mestre](roadmap/MASTER.md): visao editorial e historica da evolucao do projeto

## 2. Compilador e Runtime

- [Ascension Compiler](compiler/ascension-zenith.md): estado atual da linha self-hosted oficial
- [Self-Hosted Host ABI](specification/selfhost-abi.md): fronteira minima entre compilador e host
- [Politica de Artefatos Self-Hosted](specification/selfhost-artifacts.md): destino correto de bootstrap, release e temporarios

## 3. Guia de Aprendizado

- [Manual de Sintaxe](learn/handbook/syntax.md): guia pratico da surface syntax atual
- [Manual de Idiomatismos](learn/handbook/idioms.md): melhores praticas para a linguagem atual
- [Teoria do Compilador](learn/): lexer, parser, binder, lowering e runtime

## 4. Especificacao Tecnica

- [Gramatica e Sintaxe](specification/grammar.md): regras formais implementadas hoje
- [Sistema de Tipos](specification/types.md): genericos, Optional, Outcome, nulabilidade e validate
- [Semantica e Escopo](specification/semantics.md): resolucao de nomes, UFCS e indexacao segura
- [Regras de Transpilacao](specification/transpilation.md): mapeamento da linguagem para o backend atual
- [Sistema de Projeto](specification/project-system.md): modulos, pacotes e organizacao
- [RFC 001 - Self, Atributos e Validate](specification/decisions/001-self-attrs-validate.md): decisao implementada para @, #[...] e validate
- [Catalogo de Erros](support/error-catalog.md): diagnosticos de compilacao, warnings e runtime

## 5. API e Suporte

- [Biblioteca Padrao](api/): referencia dos modulos std.*, incluindo std.core, std.text e std.validation
- [Zenith Legacy](language/legacy.md): contexto historico preservado
- [Zenith Crowbar e suporte](support/): manutencao, depuracao e IDE

---
Zenith: clareza sintatica, previsibilidade semantica e soberania tecnica.