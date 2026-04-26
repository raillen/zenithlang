# Zenith Roadmap 3.0

## Objetivo

Definir o ciclo R3 para fechar lacunas de confiabilidade e linguagem sem romper a filosofia reading-first.

Foco do ciclo:

- fechar primeiro a entrega prioritaria de `public var` em escopo de namespace, sem introduzir `global`;
- fechar M34 com hardening final de diagnosticos;
- fechar M35 com concorrencia, FFI estavel e base de dyn dispatch;
- evoluir features funcionais (closures/lambdas/HOF) com rollout seguro.

## Escopo

- Planejamento do ciclo R3 (R3.M0 ate R3.M9).
- Coordenacao de uma trilha paralela Borealis (R3.B0 ate R3.B9).
- Entregas orientadas a producao, nao a experimento isolado.
- Regras explicitas do que entra e do que nao entra.

## Dependencias

- Upstream:
  - `docs/internal/planning/roadmap-v2.md`
  - `docs/internal/planning/checklist-v2.md`
  - `language/spec/*`
  - `language/decisions/086-namespace-public-var-and-controlled-mutation.md`
- Downstream:
  - `docs/internal/planning/checklist-v3.md`
  - `docs/internal/planning/borealis-roadmap-v1.md`
  - `docs/internal/planning/borealis-checklist-v1.md`
  - `docs/internal/reports/release/*`
  - `docs/internal/reports/compatibility/*`

Status: finalizado
Data: 2026-04-22
Base: `0.3.0-alpha.1` publicado (`R2.M12` fechado)

## Atualizacao de status (2026-04-22)

`R3.P1` foi entregue neste corte.

Sinalizacao de entrega:

- `public var` habilitado em escopo de namespace (leitura externa qualificada, escrita restrita ao namespace dono);
- sem introducao de `global` na sintaxe canonica;
- stdlib piloto implementado em `std.random` com estado observavel e testes dedicados;
- docs normativas atualizadas em `language/spec/*` e `language/decisions/086-*`.

## Atualizacao de status (2026-04-23)

`R3.P1.A`, `R3.M0`, `R3.M1`, `R3.M2`, `R3.M3`, `R3.M4`, `R3.M5`, `R3.M6`, `R3.M7`, `R3.M8` e `R3.M9` foram fechadas. `R3.M2` fechou como **Phase 1 completa**; Decision 091 moveu `task`, `channel`, `Shared<T>` e corrida/cancelamento para `R4.CF1`.

Sinalizacao de entrega:

- analise futura de estado compartilhado de namespace publicada em `docs/internal/reports/R3.P1.A-namespace-shared-state-analysis.md` (proposta base + sincronizacao futura + guideline + criterios de promocao);
- kickoff do ciclo R3 publicado em `docs/internal/reports/release/R3.M0-kickoff-report.md` (baselines de qualidade/performance congeladas referenciadas em `docs/internal/governance/baselines/`);
- matriz de riscos R3 publicada em `docs/internal/reports/R3-risk-matrix.md` (zero `P0` sem owner/prazo);
- checkpoint inicial da trilha Borealis publicado em `docs/internal/reports/R3.M0-borealis-alignment-checkpoint.md`;
- hardening final de diagnosticos M34 publicado em `docs/internal/reports/release/R3.M1-diagnostics-hardening-report.md` (bloco `NEXT` no renderer action-first, campos `effort=`/`action=`/`next=` no CI renderer, suite dedicada `tests/driver/test_diagnostics_rendering.c` com 41/41 pass);
- concorrencia base Phase 1 publicada em `docs/internal/reports/release/R3.M2-concurrency-base-report.md` (boundary contract via `std.concurrent.copy_*`, spec authoritative em `language/spec/concurrency.md`, integracao R3.P1.A em Decision 087, determinismo coberto por `tests/behavior/std_concurrent_boundary_copy_determinism`); `task`/`channel`/`Shared<T>` foram formalmente movidos para `R4.CF1` por Decision 091;
- FFI 1.0 Phase 1 publicada em `docs/internal/reports/release/R3.M3-ffi-1.0-report.md` (spec authoritative em `language/spec/ffi.md` com ABI contract + matriz de tipos permitidos/bloqueados + regras de ownership; shielding empiricamente verificada em `tests/behavior/extern_c_text_len_e2e`; negativa struct-as-arg em `tests/behavior/extern_c_struct_arg_error` com diagnostic pinada); arity/return negatives, callbacks e ABI annotations deferidos e rastreados na matriz de riscos;
- dyn dispatch Phase 1 publicada: spec em `language/spec/dyn-dispatch.md`, decision em `language/decisions/088-dyn-dispatch-minimum-subset.md`, runtime structures (`zt_vtable`, `zt_dyn_value`, `zt_list_dyn`) em `runtime/c/zenith_rt.h` e `runtime/c/zenith_rt.c`, validacao de subset no checker (`zt_checker_validate_dyn_trait`), type resolution para `dyn<Trait>` e `list<dyn<Trait>>` no emitter; vtable generation, boxing e dynamic dispatch em progresso.

## Diretrizes de linguagem do ciclo

1. `trait` continua como contrato oficial.
2. `interface` nao entra como conceito novo.
3. Se alias `interface` existir no futuro, formatter canoniza para `trait`.
4. `mixins` ficam fora do R3.
5. Design patterns ficam em `docs/public/stdlib/` ou `docs/public/guides/`, nao em sintaxe.
6. `lazy` so entra de forma explicita (`lazy<T>`/iteradores), sem avaliacao implicita global.

## Gates obrigatorios

Nenhuma milestone avanca sem:

1. `python build.py` verde
2. `python run_all_tests.py` verde
3. `tests/perf/gate_pr.ps1` verde
4. sem crash novo em fuzz/corpus
5. checklist da milestone com evidencia minima

Para release do ciclo:

1. `tests/perf/gate_nightly.ps1` verde
2. budget atualizado e justificado
3. sem P0 aberto sem aceite formal
4. install limpo validado

## Fases do Roadmap 3.0

## R3.P1 - Prioridade 1: public var de namespace (concluida em 2026-04-22)

Objetivo:

- habilitar `public var` em escopo de namespace com regra de mutacao controlada.

Entregas:

- fechar a mini-RFC `Decision 086` e manter o texto normativo como base oficial;
- aceitar `public var` apenas em declaracao top-level de namespace;
- permitir leitura externa qualificada (`alias.var_publica`);
- bloquear escrita externa de `public var` fora do namespace dono neste corte;
- manter semantica explicita de que `public` != `global` e que `global` nao existe na sintaxe canonica;
- preservar comportamento atual de `public const` sem regressao;
- publicar testes positivos/negativos e criterio de init deterministico para storage de modulo;
- documentar efeitos colaterais esperados de `public var` e mitigacoes recomendadas;
- documentar interacao entre `public var` e `mut func` (mutacao de namespace != mutacao de `self`);
- registrar diretriz de adocao na stdlib: sem refactor amplo, apenas uso pontual quando houver ganho claro.

## R3.P1.A - Analise futura: estado compartilhado de namespace

Objetivo:

- preparar a evolucao segura de `public var` para cenarios com concorrencia e maior escala.

Entregas:

- analise de riscos de estado compartilhado (ordem de execucao, isolamento de teste, debug);
- proposta de modelo para `public var` em concorrencia (`single-thread` por padrao vs wrappers explicitos);
- proposta de primitives futuras (`atomic`/sincronizacao explicita) sem introduzir `global`;
- guideline de migracao para stdlib/packages com checklist de quando usar e quando evitar `public var`;
- criterios de aceite para promover esta analise para milestone de implementacao.

## R3.M0 - Baseline e alinhamento

Objetivo:

- consolidar baseline tecnica para evitar drift durante o ciclo.

Entregas:

- sincronizar roadmap/checklist/status para um estado unico;
- congelar baseline de qualidade/performance do inicio do ciclo;
- publicar matriz de riscos R3 (P0/P1/P2) com owner e prazo.
- publicar checkpoint inicial da trilha Borealis em paralelo.

## R3.M1 - M34 hardening final

Objetivo:

- finalizar diagnosticos action-first como UX padrao confiavel.

Entregas:

- consolidar perfis `beginner`, `balanced`, `full`;
- padrao `ACTION/WHY/NEXT` em todos os caminhos centrais;
- calibrar effort hints (`quick fix`, `moderate`, `requires thinking`);
- fechar cobertura para `--focus`, `--since`, `summary`, `resume`;
- adicionar teste de regressao para cada bug de diagnostico novo.

## R3.M2 - M35.1 concorrencia base

Objetivo:

- abrir concorrencia sem quebrar previsibilidade de ownership.
- status: Phase 1 entregue; escopo de runtime completo formalmente movido para `R4.CF1`.

Entregas:

- modelo oficial `task` + `channel` com isolamento por fronteira;
- contrato explicito de copia na fronteira entre tasks;
- `Shared<T>` apenas como caminho avancado e explicito;
- integrar conclusoes de `R3.P1.A` para semantica de `public var` em contexto concorrente;
- testes de corrida, ordem, cancelamento e determinismo do runtime;
- docs de semantica de concorrencia no spec.

Escopo movido para `R4.CF1`:

- `task` + `channel` ainda nao foram implementados;
- `Shared<T>` ainda nao foi implementado;
- testes de corrida e cancelamento dependem da surface de runtime;
- Decision 091 registra que isso nao bloqueia o release final R3, desde que R3 nao prometa essas surfaces.

## R3.M3 - M35.2 FFI 1.0

Objetivo:

- tornar interop C estavel e auditavel.

Entregas:

- contrato ABI estavel para `extern c`;
- regras oficiais de ownership na fronteira FFI;
- blindagem automatica do backend para chamadas externas;
- matriz de tipos permitidos/bloqueados em FFI;
- suite de testes negativos (assinatura invalida, ownership invalido, retorno invalido).

## R3.M4 - M35.3 dyn dispatch minimo

Objetivo:

- fechar dyn dispatch para casos prioritarios sem abrir complexidade total.

Entregas:

- `dyn Trait` em subset oficial documentado;
- colecao heterogenea minima com cobertura E2E;
- custo de dispatch medido e com budget;
- diagnosticos claros para limites do subset.

## R3.M5 - Callables tipados e delegates

Objetivo:

- criar base segura para callback e composicao funcional.

Entregas:

- tipo callable/delegate com assinatura explicita;
- regras de compatibilidade de assinatura;
- interop de delegate com FFI onde seguro;
- testes de escape e ownership de callable.

Nota:

- delegates entram depois de M35 para evitar modelo duplo inconsistente.

## R3.M6 - Closures v1 (restritas)

Objetivo:

- introduzir closures com baixo risco semantico.
- status: implementado em 2026-04-24 no corte R3.M6.

Entregas:

- captura imutavel por valor no primeiro corte;
- proibicao explicita de captura mutavel ate fase posterior;
- modelo de lifetime definido no runtime e no checker;
- `func(...)` como fat pointer gerenciado (`fn` + `ctx`);
- ABI interna com `zt_ctx` em funcoes Zenith;
- diagnostico `closure.mut_capture_unsupported`;
- testes positivos e negativos no behavior suite.

Observacao:

- closures v1 nao introduzem variaveis globais;
- capturas sao snapshots imutaveis;
- mutabilidade compartilhada em closures fica para uma feature futura, com analise propria.

## R3.M7 - Lambdas v1 + HOF de stdlib

Objetivo:

- habilitar expressividade funcional sem perder legibilidade.
- status: implementado em 2026-04-24 no corte R3.M7.

Entregas:

- sintaxe de lambda minimal `func(...) => expr`, consistente com formatter;
- HOFs base em `std.collections` no subset oficial (`map_int`, `filter_int`, `reduce_int`);
- guidelines de legibilidade para nao degradar leitura;
- benchmark `micro_lambda_hof_run` para evitar regressao severa em hot path.

Observacao:

- lambdas v1 sao acucar para closures v1;
- retorno e inferido pelo tipo `func(...) -> ...` esperado;
- parametros continuam tipados;
- block lambdas e HOFs genericos ficam para fase futura.

## R3.M8 - Lazy explicito

Objetivo:

- adicionar lazy apenas quando observavel e controlado.
- status: implementado em 2026-04-24 no corte R3.M8 para `lazy<int>` one-shot explicito.

Entregas:

- `lazy<T>`/iteradores lazy explicitamente tipados;
- proibicao de lazy implicito em expressoes comuns;
- testes de ordem de avaliacao e consumo unico;
- docs com exemplos de uso correto e armadilhas.

Observacao:

- o tipo `lazy<T>` ja existe no compiler;
- o runtime executavel inicial cobre `lazy<int>` via `std.lazy.once_int/force_int/is_consumed_int`;
- `lazy<T>` generico, valores reutilizaveis e iteradores lazy ficam para uma fase futura;
- nenhuma expressao comum virou lazy implicitamente.

## R3.M9 - Release do ciclo R3

Objetivo:

- fechar ciclo com pacote validado para uso externo mais exigente.
- status: `0.3.0-alpha.3` finalizado em 2026-04-24 com artefatos verificaveis locais.

Entregas:

- changelog do ciclo R3;
- relatorio final de qualidade/performance/compatibilidade;
- pacote, checksum e validacao de install limpo;
- limite conhecido e risco residual publicados.
- checkpoint final de alinhamento R3 x Borealis publicado.

Resultado do corte:

- pacote final e checksum gerados;
- install limpo validado com `check` e `run` em app minimo;
- relatorios de release, compatibilidade, limites e Borealis publicados;
- `tests/perf/gate_pr.ps1` verde;
- `tests/perf/gate_nightly.ps1` verde com override documentado de baseline R3.M9;
- R3.M2 residual foi formalmente movido para `R4.CF1` por Decision 091;
- release final local concluido.

## Trilha paralela Borealis (R3.B0 ate R3.B9)

R3 inclui uma trilha paralela para a game lib Borealis, sem mover Borealis para `stdlib` neste ciclo.

Referencias oficiais da trilha:

- `docs/internal/planning/borealis-roadmap-v1.md`
- `docs/internal/planning/borealis-checklist-v1.md`

Checkpoint obrigatorio de alinhamento:

1. no fechamento de `R3.M0`;
2. no fechamento de `R3.M5`;
3. no fechamento de `R3.M9`.

## Fora de escopo do R3

- `mixins`
- metaprogramacao macro ampla
- lazy implicito global
- abrir segundo conceito oficial equivalente a `trait`

## Definicao de pronto por milestone

Uma milestone so fecha quando tiver:

1. spec atualizada (quando houver mudanca de comportamento);
2. testes positivos e negativos;
3. evidencia no checklist;
4. sem regressao acima de budget;
5. docs de uso e de limite conhecido atualizadas.
