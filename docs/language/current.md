# Zenith Current

> Status: current
> Recomendado para: estudo da linguagem atual, contribuicao e validacao do produto atual
> Referencia de estado real: ../roadmap/selfhost-pos100.md

Zenith Current representa o recorte mais fiel do Zenith de hoje: a linguagem se apresenta por um front door 100% self-hosted, com a trilha legada isolada apenas para recuperacao extraordinaria e com o residuo pos-100 ja fechado.

## O que define a linha atual

- front door oficial em `ztc.lua` com caminho estritamente self-hosted;
- tipagem explicita;
- blocos visiveis e leitura vertical;
- `Optional` e `Outcome` como parte da ergonomia real;
- `trait`, `apply`, `where`, `validate`, `match`, `check`, `attempt/rescue`;
- parser, binder, lowering, diagnostics e codegen funcionais na trilha ativa;
- separacao clara entre core self-hosted canonico e superficie experimental adjacente.

## O que esta estabilizado

- Fases 1-12 concluidas e testadas na base ativa;
- front door oficial 100% self-hosted;
- bootstrap deterministico e gate recorrente de release self-hosted;
- politica de `null` promovida para erro dedicado `ZT-S106`;
- UFCS, genericos, pattern matching e indexacao 1-based validados;
- politica explicita de artefatos para bootstrap, release, auditoria e smokes locais.

## O que nao deve ser escondido

- `ztc.lua` continua existindo como wrapper oficial do produto, mas opera apenas em modo self-hosted;
- a trilha legada ficou isolada em `tools/ztc_legacy.lua` e `tools/bootstrap_legacy_recovery.lua`;
- o core self-hosted canonico vive em `src/compiler/syntax.zt`;
- `binder.zt`, `parser.zt` e `lexer.zt` em `.zt` nao fazem parte do caminho oficial neste momento; continuam como superficie experimental;
- Fase 12 segue concluida no recorte `.zt`: demo, stdlib e compilador self-hosted sem blocos `native lua`.

## Para quem esta chegando agora

Se voce esta chegando agora, esta e a linha correta para estudar junto com:

- ../roadmap/selfhost-100.md
- ../roadmap/selfhost-pos100.md
- ../compiler/ascension-zenith.md
- ../specification/current-core.md
- ../specification/selfhost-abi.md
- ../specification/selfhost-artifacts.md
- ../specification/decisions/001-self-attrs-validate.md
- ../specification/decisions/002-selfhost-architecture-cutover.md
- ../specification/decisions/004-selfhost-100-cutover.md
