# Zenith Current

> Status: current
> Recomendado para: estudo da linguagem atual, contribuicao e validacao da trilha ativa
> Referencia de estado real: ../roadmap_estabilizacao.md

Zenith Current representa o recorte mais fiel do Zenith de hoje, mas esse "hoje" precisa ser lido com precisao: a linguagem atual e sustentada pela trilha ativa em Lua, nao por self-hosting completo.

## O que define a linha atual

- tipagem explicita
- blocos visiveis e leitura vertical
- Optional e Outcome como parte da ergonomia real
- trait, apply, where, validate, match, check, attempt/rescue
- parser, binder, lowering, diagnostics e codegen funcionais na trilha ativa
- separacao clara entre trilha ativa e trilha self-hosted

## O que esta estabilizado

- Fases 1-11 concluidas e testadas na trilha ativa
- politica de null promovida para erro dedicado ZT-S106
- UFCS, genericos, pattern matching e indexacao 1-based validados
- demo principal compilando sem native lua proprio

## O que nao deve ser escondido

- o compilador ativo oficial ainda e `ztc.lua`
- o core self-hosted canonico existe em `src/compiler/syntax.zt`; a paridade ampla fora do core nao e mais o bloqueio principal
- Fase 12 esta concluida no recorte `.zt`: demo, stdlib e compilador self-hosted sem blocos `native lua`

## Para quem esta chegando agora

Se voce esta chegando agora, esta e a linha correta para estudar junto com:

- ../roadmap_estabilizacao.md
- ../compiler/ascension-zenith.md
- ../specification/current-core.md
- ../specification/decisions/001-self-attrs-validate.md
