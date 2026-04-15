# Roadmap de Ascensao: Linha Self-Hosted

> Nota: este documento nao deve mais ser lido como prova de self-hosting concluido.
> Estado verificado atual: ../roadmap_estabilizacao.md

Ascension continua sendo a trilha self-hosted do projeto, mas hoje ela deve ser tratada como programa de paridade e bootstrap, nao como motor concluido em substituicao ao caminho ativo.

## Status real

- trilha ativa oficial: `ztc.lua`
- trilha self-hosted: `src/compiler/*.zt`
- maturidade atual da trilha self-hosted: parcial

## Objetivos desta trilha

- aproximar o compilador em Zenith da linguagem ativa
- reduzir hacks de `native lua`
- permitir bootstrap verificavel no futuro
- transformar testes de soberania em validacao de paridade real

## Criterios de progresso

- `ztc check src/compiler/syntax.zt` com falhas controladas ou eliminadas
- reducao estrutural de `native lua`
- compatibilidade semantica entre a linguagem ativa e a base self-hosted
- caminho confiavel de build para a propria trilha self-hosted

*Atualizado em: 2026-04-14*
