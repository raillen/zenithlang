# Planning Docs

Este diretorio guarda planos de execucao.

## Fonte ativa

| Tipo | Documento |
| --- | --- |
| Roadmap unificado | `roadmap-v7.md` |
| Checklist unificado | `checklist-v7.md` |
| Roadmap LSP 1.0 | `lsp-1.0-roadmap.md` |

Estes dois arquivos substituem todos os roadmaps e checklists anteriores
(v1-v6, manual-memory, production-readiness, self-hosting,
language-maturity, documentation, stabilization, m9, m10).

A decisao de design que originou o v7 esta em
`language/decisions/093-language-design-session-v7.md`.

## Historico

Os roadmaps v1-v6 e checklists v1-v6 foram removidos em 2026-04-27.
Consulte o historico git para referencia.

Arquivos de apoio que permanecem:

- `cascade-v1.md`, `cascade-v2.md` — historico de sessoes.
- `r3-m5-progress.txt` — notas de milestone.

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
- `borealis-studio-roadmap-v1.md`
  Roadmap ativo da interface, fluxo inicial e release standalone do Borealis Studio.
- `borealis-studio-checklist-v1.md`
  Checklist operacional do roadmap v1 do Borealis Studio.

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
