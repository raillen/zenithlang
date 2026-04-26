# R2.M10 - Prioridade de Modulos Stdlib

Data: 2026-04-21

Critério usado (impacto):

1. Uso em contratos `where` e validação de entrada
2. Uso em fluxo de app (`io/fs/os/time`)
3. Risco de regressão em build/teste

Top prioridade para M10:

| Ordem | Modulo | Motivo curto | Ação M10 |
| --- | --- | --- | --- |
| 1 | `std.validate` | usado cedo em contrato e regra de dominio | ampliar predicados pequenos + teste |
| 2 | `std.text` | base de `validate` e UX de dados textuais | manter como dependencia de predicado |
| 3 | `std.io` | entrada/saida em exemplos e smoke | manter gate de doc/check ativo |
| 4 | `std.fs` + `std.fs.path` | automação local e scripts | manter cobertura behavior |
| 5 | `std.os` + `std.os.process` | integração host | preservar estabilidade de API |

Entregue neste ciclo M10:

- `std.validate.one_of_text`
- cobertura em `tests/behavior/std_validate_basic`
