# Documentacao Zenith

Este diretorio concentra documentacao operacional e tecnica que nao deve ficar solta na raiz.

## Objetivo

- Facilitar navegacao rapida para pessoas e LLMs.
- Reduzir duplicidade e drift entre roadmap, checklist, specs e codigo.
- Padronizar escrita para leitura de baixa friccao (TDAH e dislexia).

## Estrutura

- `docs/planning/`
  - Roadmaps, checklists e cascades de execucao.
- `docs/reports/`
  - Auditorias, analises profundas e relatorios de validacao.
- `docs/guides/`
  - Guias práticos e material de onboarding tecnico.
- `docs/licensing/`
  - Licenciamento, contribuicoes e marca.
- `docs/standards/`
  - Regras de escrita e padroes de documentacao.

## Ordem de leitura recomendada

1. `README.md` (raiz)
2. `docs/planning/roadmap-v2.md`
3. `docs/planning/checklist-v2.md`
4. `docs/planning/cascade-v2.md`
5. `language/spec/README.md`
6. `compiler/CODE_MAP.md`

## Matriz de dependencias entre documentos

| Documento | Depende de | Alimenta |
| --- | --- | --- |
| `docs/planning/roadmap-v2.md` | `docs/planning/checklist-v1.md`, `language/spec/*` | `docs/planning/checklist-v2.md`, `docs/planning/cascade-v2.md` |
| `docs/planning/checklist-v2.md` | `docs/planning/roadmap-v2.md` | Execucao por milestone e validacao de entrega |
| `docs/planning/cascade-v2.md` | `roadmap-v2.md`, `checklist-v2.md`, `compiler/*_MAP.md` | Implementacao no codigo e suites de teste |
| `docs/planning/roadmap-v1.md` | `language/spec/*` | `checklist-v1.md`, `cascade-v1.md` |
| `docs/planning/checklist-v1.md` | `roadmap-v1.md` | Historico de fechamento M0-M38 |
| `docs/planning/cascade-v1.md` | `roadmap-v1.md`, `checklist-v1.md` | Navegacao historica e auditoria retroativa |
| `docs/reports/*` | Resultado de testes, checklist e auditorias | Decisoes de priorizacao e ajustes de roadmap |

## Padrao editorial

Use o padrao em `docs/standards/documentation-style-guide.md` para qualquer novo documento.

Resumo rapido:

- Frases curtas e diretas.
- Titulos previsiveis e repetiveis.
- Dependencias sempre explicitas.
- Emoji so quando necessario, no maximo 1 por secao, e somente para sinalizacao relevante.
