# Coverage Snapshot

Gerado em: `2026-04-23T16:07:29.140532+00:00`

Escopo:
- cobertura focada por cenario;
- nao e cobertura unificada do projeto inteiro;
- serve para dar visibilidade objetiva por modulo critico.

| Cenario | Arquivo | Linhas | Branches | Calls | Nota |
|---|---|---:|---:|---:|---|
| `frontend/parser_depth` | `compiler/frontend/parser/parser.c` | 22.93% (1378) | 31.47% (804) | 18.74% (619) | Guarda de profundidade do parser. |
| `semantic/numeric_guardrails` | `compiler/semantic/types/checker.c` | 13.52% (2256) | 15.16% (2295) | 6.96% (963) | Overflow inteiro e faixa de float no checker. |
| `zir/lowering_depth` | `compiler/zir/lowering/from_hir.c` | 4.21% (1926) | 5.97% (1408) | 1.36% (1322) | Guarda estrutural do lowering HIR -> ZIR. |
| `backend/emitter_stream` | `compiler/targets/c/emitter.c` | 13.91% (4643) | 14.04% (5299) | 7.87% (2275) | Caminho spill + stream do emitter C. |
| `runtime/process_run` | `runtime/c/zenith_rt.c` | 12.28% (4838) | 12.67% (3472) | 8.14% (2323) | Execucao de processo segura, cwd e overflows fatais. |
| `runtime/text_utf8_guardrails` | `runtime/c/zenith_rt.c` | 7.94% (4838) | 9.16% (3472) | 3.31% (2323) | Construtor UTF-8 validado e falha em entrada invalida. |

Leitura:
- percentuais baixos aqui nao significam regressao por si so;
- eles mostram quanto cada cenario focado toca no arquivo alvo;
- o objetivo e acompanhar tendencia e descobrir buracos evidentes.

