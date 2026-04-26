# Reports

Este diretorio guarda relatorios curados.

Use aqui apenas documentos que precisam ser lidos depois.
Saidas geradas por teste, benchmark ou build ficam em `reports/`.

## Fonte atual para pendencias

| Necessidade | Fonte |
| --- | --- |
| Pendencias corretivas atuais | `reports/pending-language-issues-current.md` |
| Roadmap ativo | `docs/internal/planning/roadmap-v4.md` |
| Checklist ativo | `docs/internal/planning/checklist-v4.md` |

`reports/pending-language-issues-current.md` e a fonte operacional atual para pendencias corretivas.
Relatorios antigos em `docs/internal/reports/` sao evidencia historica ou fechamento de milestone.

## Estrutura

- `audit/`
  Auditorias tecnicas.
- `compatibility/`
  Relatorios de compatibilidade por versao.
- `fuzz/`
  Evidencias de fuzzing.
- `overrides/`
  Justificativas de aceite excepcional.
- `perf/`
  Metodologia, comparativos e resumos curados.
- `raw/`
  Historico bruto preservado sem normalizacao editorial.
- `release/`
  Notas e relatorios de release.
- `semantic/`
  Relatorios de semantica e matriz negativa.
- `triage/`
  Triage automatizada. Use `latest.md` e `latest.json` como entrada principal.

## Regra para novos relatorios

Crie novo arquivo aqui quando:

- a informacao precisa sobreviver ao build local;
- a evidencia fecha uma milestone;
- o conteudo tem decisao, risco ou contexto humano.

Nao crie novo arquivo aqui quando:

- o arquivo e log bruto;
- o arquivo e resultado local reproduzivel;
- o arquivo e screenshot temporario;
- o arquivo apenas duplica o checklist ativo.

## Relatorios principais

- `audit-report.md`
- `implementation-deep-analysis.md`
- `checklist-deep-analysis-report.md`
- `checklist-final-analysis-report.md`
- `gate-red-fixed-report.md`
- `R3-risk-matrix.md`
- `R3.M5-phase1-phase2-checkpoint.md`
- `stdlib-public-var-analysis-2026-04-22.md`
