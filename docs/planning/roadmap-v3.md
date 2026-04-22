# Zenith Roadmap 3.0

## Objetivo

Definir o ciclo R3 para fechar lacunas de confiabilidade e linguagem sem romper a filosofia reading-first.

Foco do ciclo:

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
  - `docs/planning/roadmap-v2.md`
  - `docs/planning/checklist-v2.md`
  - `language/spec/*`
- Downstream:
  - `docs/planning/checklist-v3.md`
  - `docs/planning/borealis-roadmap-v1.md`
  - `docs/planning/borealis-checklist-v1.md`
  - `docs/reports/release/*`
  - `docs/reports/compatibility/*`

Status: proposto
Data: 2026-04-21
Base: `0.3.0-alpha.1` publicado (`R2.M12` fechado)

## Diretrizes de linguagem do ciclo

1. `trait` continua como contrato oficial.
2. `interface` nao entra como conceito novo.
3. Se alias `interface` existir no futuro, formatter canoniza para `trait`.
4. `mixins` ficam fora do R3.
5. Design patterns ficam em docs/stdlib/exemplos, nao em sintaxe.
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

Entregas:

- modelo oficial `task` + `channel` com isolamento por fronteira;
- contrato explicito de copia na fronteira entre tasks;
- `Shared<T>` apenas como caminho avancado e explicito;
- testes de corrida, ordem, cancelamento e determinismo do runtime;
- docs de semantica de concorrencia no spec.

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

Entregas:

- captura imutavel no primeiro corte;
- proibicao explicita de captura mutavel ate fase posterior;
- modelo de lifetime definido e testado;
- diagnosticos para captura invalida.

## R3.M7 - Lambdas v1 + HOF de stdlib

Objetivo:

- habilitar expressividade funcional sem perder legibilidade.

Entregas:

- sintaxe de lambda minimal e consistente com formatter;
- HOFs base em stdlib (`map`, `filter`, `reduce`) no subset oficial;
- guidelines de legibilidade para nao degradar leitura;
- benchmarks para evitar regressao severa em hot path.

## R3.M8 - Lazy explicito

Objetivo:

- adicionar lazy apenas quando observavel e controlado.

Entregas:

- `lazy<T>`/iteradores lazy explicitamente tipados;
- proibicao de lazy implicito em expressoes comuns;
- testes de ordem de avaliacao e consumo unico;
- docs com exemplos de uso correto e armadilhas.

## R3.M9 - Release do ciclo R3

Objetivo:

- fechar ciclo com pacote validado para uso externo mais exigente.

Entregas:

- changelog do ciclo R3;
- relatorio final de qualidade/performance/compatibilidade;
- pacote, checksum e validacao de install limpo;
- limite conhecido e risco residual publicados.
- checkpoint final de alinhamento R3 x Borealis publicado.

## Trilha paralela Borealis (R3.B0 ate R3.B9)

R3 inclui uma trilha paralela para a game lib Borealis, sem mover Borealis para `stdlib` neste ciclo.

Referencias oficiais da trilha:

- `docs/planning/borealis-roadmap-v1.md`
- `docs/planning/borealis-checklist-v1.md`

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
