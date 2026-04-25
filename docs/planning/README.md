# Planning Docs

Este diretorio guarda planos de execucao.

## Fonte ativa

| Tipo | Documento |
| --- | --- |
| Roadmap atual | `roadmap-v4.md` |
| Checklist atual | `checklist-v4.md` |
| Roadmap de estabilizacao | `stabilization-roadmap.md` |

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

## Borealis

- `borealis-roadmap-v1.md`
  Planejamento do package Borealis.
- `borealis-checklist-v1.md`
  Checklist de implementacao do package.
- `borealis-engine-roadmap-v2.md`
  Planejamento de engine/editor.

## Planos complementares

- `m9-feature-pack-a-design.md`
- `m10-stdlib-priorities.md`
- `r3-m5-progress.txt`

Estes arquivos sao apoio de milestone.
Nao devem substituir o roadmap ou checklist ativos.

## Dependencias

Upstream:

- `language/spec/README.md`
- `language/decisions/README.md`
- `compiler/CODE_MAP.md`

Downstream:

- implementacao no codigo;
- suites de teste;
- relatorios em `docs/reports/`;
- saidas operacionais em `reports/`.

## Regra de manutencao

Quando uma entrega muda:

1. atualize o roadmap ativo se a direcao mudou;
2. atualize o checklist ativo se a tarefa mudou;
3. atualize specs ou decisions se o comportamento da linguagem mudou;
4. registre evidencia em `docs/reports/` quando a entrega fechar.
