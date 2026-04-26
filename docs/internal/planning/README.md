# Planning Docs

Este diretorio guarda planos de execucao.

## Fonte ativa

| Tipo | Documento |
| --- | --- |
| Roadmap atual | `roadmap-v4.md` |
| Checklist atual | `checklist-v4.md` |
| Roadmap de ergonomia proposto | `roadmap-v6.md` |
| Checklist de ergonomia proposto | `checklist-v6.md` |
| Roadmap de memoria manual opcional | `manual-memory-roadmap-v1.md` |
| Checklist de memoria manual opcional | `manual-memory-checklist-v1.md` |
| Roadmap de documentacao | `documentation-roadmap-v1.md` |
| Checklist de documentacao | `documentation-checklist-v1.md` |
| Roadmap de estabilizacao | `stabilization-roadmap.md` |
| Roadmap final para producao | `production-readiness-roadmap-v1.md` |
| Checklist final para producao | `production-readiness-checklist-v1.md` |

Use estes arquivos antes de criar novos documentos.

## Historico por ciclo

- `roadmap-v1.md`, `checklist-v1.md`, `cascade-v1.md`
  Historico inicial.
- `roadmap-v2.md`, `checklist-v2.md`, `cascade-v2.md`
  Ciclo 2.
- `roadmap-v3.md`, `checklist-v3.md`
  Ciclo 3.
- `roadmap-v4.md`, `checklist-v4.md`
  Ciclo atual.
- `roadmap-v5.md`, `checklist-v5.md`
  Produto e ecossistema.
- `roadmap-v6.md`, `checklist-v6.md`
  Ergonomia, legibilidade e acessibilidade sem sintaxe pesada.
- `manual-memory-roadmap-v1.md`, `manual-memory-checklist-v1.md`
  Possibilidade futura de memoria manual opcional, ainda a decidir.
- `documentation-roadmap-v1.md`, `documentation-checklist-v1.md`
  Reestruturacao da documentacao publica e de referencia.

## Producao

- `production-readiness-roadmap-v1.md`
  Roadmap final para sair de alpha avancado / beta inicial ate producao publica.
- `production-readiness-checklist-v1.md`
  Checklist operacional para fechar repo, gates, stdlib, tooling, dogfooding e release.

## Borealis

- `borealis-roadmap-v1.md`
  Planejamento do package Borealis.
- `borealis-checklist-v1.md`
  Checklist de implementacao do package.
- `borealis-engine-roadmap-v2.md`
  Planejamento de engine/editor.
- `borealis-engine-studio-roadmap-v3.md`
  Roadmap ativo para scheduler interno, jobs e assets async-like da engine/Studio.
- `borealis-engine-studio-checklist-v3.md`
  Checklist operacional do roadmap v3 da engine/Studio.

## Planos complementares

- `language-maturity-host-async-roadmap-v1.md`
- `language-maturity-host-async-checklist-v1.md`
- `self-hosting-roadmap-v1.md`
- `self-hosting-checklist-v1.md`
- `m9-feature-pack-a-design.md`
- `m10-stdlib-priorities.md`
- `r3-m5-progress.txt`

Estes arquivos sao apoio de milestone.
Nao devem substituir o roadmap ou checklist ativos.

## Self-hosting

- `self-hosting-roadmap-v1.md`
  Plano de maturidade para preparar Zenith para self-hosting sem reescrever o compilador agora.
- `self-hosting-checklist-v1.md`
  Checklist operacional para ferramentas pequenas, lexer/parser prototipo e decisao de bootstrap.

## Dependencias

Upstream:

- `language/spec/README.md`
- `language/decisions/README.md`
- `compiler/CODE_MAP.md`

Downstream:

- implementacao no codigo;
- suites de teste;
- relatorios em `docs/internal/reports/`;
- saidas operacionais em `reports/`.

## Regra de manutencao

Quando uma entrega muda:

1. atualize o roadmap ativo se a direcao mudou;
2. atualize o checklist ativo se a tarefa mudou;
3. atualize specs ou decisions se o comportamento da linguagem mudou;
4. registre evidencia em `docs/internal/reports/` quando a entrega fechar.
