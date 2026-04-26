# Coverage Snapshot

Gerado em: `2026-04-25T20:50:11.157052+00:00`

Escopo:
- cobertura focada por cenario;
- nao e cobertura unificada do projeto inteiro;
- serve para dar visibilidade objetiva por modulo critico.

| Cenario | Arquivo | Linhas | Branches | Calls | Nota |
|---|---|---:|---:|---:|---|
| `frontend/parser_depth` | `compiler/frontend/parser/parser.c` | 22.90% (1476) | 31.32% (846) | 18.79% (660) | Guarda de profundidade do parser. |
| `semantic/numeric_guardrails` | `compiler/semantic/types/checker.c` | 11.50% (2591) | 12.69% (2639) | 6.19% (1067) | Overflow inteiro e faixa de float no checker. |
| `zir/lowering_depth` | `compiler/zir/lowering/from_hir.c` | 4.69% (2070) | 5.95% (1461) | 1.37% (1382) | Guarda estrutural do lowering HIR -> ZIR. |
| `backend/emitter_stream` | `compiler/targets/c/emitter.c` | 13.56% (5458) | 13.55% (6110) | 7.79% (2580) | Caminho spill + stream do emitter C. |
| `runtime/process_run` | `runtime/c/zenith_rt.c` | 10.67% (5566) | 11.07% (3994) | 7.30% (2575) | Execucao de processo segura, cwd e overflows fatais. |
| `runtime/text_utf8_guardrails` | `runtime/c/zenith_rt.c` | 6.90% (5566) | 8.01% (3994) | 2.95% (2575) | Construtor UTF-8 validado e falha em entrada invalida. |

Leitura:
- percentuais baixos aqui nao significam regressao por si so;
- eles mostram quanto cada cenario focado toca no arquivo alvo;
- o objetivo e acompanhar tendencia e descobrir buracos evidentes.

