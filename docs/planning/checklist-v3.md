# Zenith Checklist 3.0

## Objetivo

Executar e validar as entregas do roadmap 3.0 com evidencias objetivas.

## Escopo

- Itens de execucao por milestone do R3.
- Gates obrigatorios de qualidade, performance e confiabilidade.

## Dependencias

- Upstream:
  - `docs/planning/roadmap-v3.md`
  - `docs/planning/borealis-roadmap-v1.md`
  - `docs/planning/borealis-checklist-v1.md`
- Downstream:
  - `docs/reports/*`
  - `docs/reports/release/*`
  - `docs/reports/compatibility/*`
- Codigo/Testes relacionados:
  - `python build.py`
  - `python run_all_tests.py`
  - `tests/perf/gate_pr.ps1`
  - `tests/perf/gate_nightly.ps1`

## Como usar

1. Marque apenas itens com evidencia valida.
2. Execute os gates obrigatorios antes de fechar milestone.
3. Registre risco residual quando houver.

Checklist operacional derivado de `docs/planning/roadmap-v3.md`.

Status inicial: em execucao
Data de criacao: 2026-04-22

## Gates obrigatorios por milestone

- [ ] `python build.py` verde
- [ ] `python run_all_tests.py` verde
- [ ] `tests/perf/gate_pr.ps1` verde
- [ ] sem crash novo em fuzz/corpus
- [ ] evidencia anexada na milestone

Para release:

- [ ] `tests/perf/gate_nightly.ps1` verde
- [ ] budget atualizado e justificado
- [ ] sem P0 aberto sem aceite formal
- [ ] install limpo validado

## R3.P1 - Prioridade 1: public var de namespace

- [x] Fechar alinhamento com `language/decisions/086-namespace-public-var-and-controlled-mutation.md`
- [x] Aceitar parser/AST para `public var` em escopo top-level de namespace
- [x] Bloquear `public var` fora do escopo de namespace
- [x] Permitir leitura externa qualificada de `public var` (`alias.var_publica`)
- [x] Bloquear escrita externa de `public var` fora do namespace dono
- [x] Preservar comportamento existente de `public const` sem regressao
- [x] Atualizar formatter/ZDoc/diagnosticos para distinguir `public` de `global`
- [x] Criar testes positivos para leitura de `public var` via import alias
- [x] Criar testes negativos para tentativa de escrita cross-namespace
- [x] Registrar estrategia de init deterministico para storage de modulo
- [x] Documentar efeitos colaterais esperados de `public var`
- [x] Documentar mitigacoes recomendadas para `public var` (reset de teste, API explicita, invariantes)
- [x] Documentar relacao entre `public var` e `mut func` (`self` vs estado de namespace)
- [x] Publicar diretriz de adocao na stdlib (sem refactor amplo, adocao pontual orientada a criterio)

Criterio de aceite:

- [x] `public var` entregue como prioridade 1, sem quebrar suites atuais de visibilidade/modulo

Evidencia (2026-04-22):

- `language/decisions/086-namespace-public-var-and-controlled-mutation.md`
- `language/spec/surface-syntax.md`
- `language/surface-implementation-status.md`
- `stdlib/std/random.zt`
- `tests/behavior/public_var_module`
- `tests/behavior/public_var_module_state`
- `tests/behavior/public_var_cross_namespace_write_error`
- `tests/behavior/std_random_basic`
- `tests/behavior/std_random_state_observability`
- `tests/behavior/std_random_between_branches`
- `tests/behavior/std_random_cross_namespace_write_error`

## R3.P1.A - Analise futura: estado compartilhado de namespace

- [ ] Publicar analise de risco para estado compartilhado de namespace
- [ ] Definir proposta para `public var` sob concorrencia (modelo base + limites)
- [ ] Definir proposta de sincronizacao futura (`atomic`/wrappers explicitos)
- [ ] Publicar guideline de migracao para stdlib/packages
- [ ] Definir criterio para promover analise para milestone de implementacao

Criterio de aceite:

- [ ] Analise futura publicada e referenciada por `R3.M2`

## R3.M0 - Baseline e alinhamento

- [ ] Sincronizar roadmap/checklist/status para estado unico
- [ ] Congelar baseline de qualidade do ciclo
- [ ] Congelar baseline de performance do ciclo
- [ ] Publicar matriz de riscos R3 (P0/P1/P2) com owner e prazo
- [ ] Abrir relatorio de kickoff do ciclo
- [ ] Publicar checkpoint inicial de alinhamento com Borealis

Criterio de aceite:

- [ ] Nenhum P0 sem owner/prazo no inicio do ciclo

## R3.M1 - M34 hardening final

- [ ] Consolidar perfis `beginner`, `balanced`, `full`
- [ ] Garantir padrao `ACTION/WHY/NEXT` nos caminhos centrais
- [ ] Calibrar effort hints (`quick fix`, `moderate`, `requires thinking`)
- [ ] Cobrir `--focus`, `--since`, `summary`, `resume` com testes dedicados
- [ ] Adicionar regressao automatica para bug novo de diagnostico
- [ ] Atualizar docs de diagnosticos e acessibilidade

Criterio de aceite:

- [ ] Diagnosticos action-first consistentes e testados ponta a ponta

## R3.M2 - M35.1 concorrencia base

- [ ] Definir e implementar modelo oficial `task` + `channel`
- [ ] Implementar contrato explicito de copia na fronteira entre tasks
- [ ] Implementar caminho avancado explicito para `Shared<T>`
- [ ] Integrar conclusoes de `R3.P1.A` na proposta de semantica concorrente para `public var`
- [ ] Criar testes de corrida, ordem e cancelamento
- [ ] Criar testes de determinismo para entradas iguais
- [ ] Publicar spec de semantica de concorrencia

Criterio de aceite:

- [ ] Concurrency base executavel sem violar isolamento/ownership

## R3.M3 - M35.2 FFI 1.0

- [ ] Definir contrato ABI oficial para `extern c`
- [ ] Definir regras oficiais de ownership na fronteira FFI
- [ ] Implementar blindagem automatica no backend para chamadas externas
- [ ] Publicar matriz de tipos permitidos/bloqueados no FFI
- [ ] Criar suite de testes negativos de assinatura/ownership/retorno
- [ ] Criar fixtures E2E com chamadas externas representativas

Criterio de aceite:

- [ ] FFI 1.0 estavel e auditavel com cobertura positiva e negativa

## R3.M4 - M35.3 dyn dispatch minimo

- [ ] Definir subset oficial de `dyn Trait`
- [ ] Implementar lowering/backend/runtime para subset definido
- [ ] Criar cobertura E2E para colecao heterogenea minima
- [ ] Medir custo de dispatch e registrar budget
- [ ] Garantir diagnosticos claros para limites do subset

Criterio de aceite:

- [ ] dyn dispatch minimo fechado com comportamento previsivel

## R3.M5 - Callables tipados e delegates

- [ ] Implementar tipo callable/delegate com assinatura explicita
- [ ] Implementar regra de compatibilidade de assinatura
- [ ] Integrar delegate com FFI somente nos casos seguros
- [ ] Criar testes de escape e ownership de callable
- [ ] Atualizar docs de linguagem e exemplos de uso

Criterio de aceite:

- [ ] Delegates funcionais sem quebrar regras de ownership/lifetime

## R3.M6 - Closures v1 (restritas)

- [ ] Implementar closures com captura imutavel
- [ ] Bloquear captura mutavel no primeiro corte
- [ ] Definir modelo de lifetime no spec e no checker
- [ ] Cobrir erros de captura invalida com testes negativos
- [ ] Cobrir casos positivos de composicao basica

Criterio de aceite:

- [ ] Closures v1 estaveis com restricoes explicitas e testadas

## R3.M7 - Lambdas v1 + HOF de stdlib

- [ ] Implementar sintaxe minima de lambda
- [ ] Alinhar formatter para forma canonica de lambda
- [ ] Implementar HOFs base (`map`, `filter`, `reduce`) no subset oficial
- [ ] Criar benchmark para impacto de lambdas/HOF em hot path
- [ ] Publicar guideline de legibilidade para uso funcional

Criterio de aceite:

- [ ] Lambdas/HOFs entregues sem regressao critica de performance

## R3.M8 - Lazy explicito

- [ ] Implementar `lazy<T>`/iteradores lazy explicitamente tipados
- [ ] Bloquear lazy implicito em expressoes comuns
- [ ] Cobrir ordem de avaliacao com testes
- [ ] Cobrir consumo unico/reuso invalido com testes
- [ ] Atualizar docs com exemplos e armadilhas

Criterio de aceite:

- [ ] Lazy disponivel apenas em modo explicito e previsivel

## R3.M9 - Release do ciclo R3

- [ ] Fechar changelog final do ciclo R3
- [ ] Publicar relatorio final de qualidade/performance/compatibilidade
- [ ] Gerar pacote de release + checksum
- [ ] Validar install limpo em ambiente sem residuos
- [ ] Publicar limites conhecidos e risco residual
- [ ] Publicar checkpoint final de alinhamento R3 x Borealis

Criterio de aceite:

- [ ] Release R3 publicado com artefatos verificaveis e sem P0 aberto

## Fora de escopo do ciclo (controle)

- [ ] Nao introduzir `mixins` no R3
- [ ] Nao introduzir metaprogramacao macro ampla no R3
- [ ] Nao introduzir lazy implicito global no R3
- [ ] Nao introduzir segundo conceito oficial equivalente a `trait`

## Regras continuas

- [ ] Todo bug novo gera teste de regressao
- [ ] Toda feature nova entra com teste positivo e negativo
- [ ] Toda mudanca de comportamento atualiza docs no mesmo PR
- [ ] Toda regressao critica de performance bloqueia merge
- [ ] Toda divergencia spec x codigo recebe classificacao P0/P1/P2

## Trilha paralela Borealis (controle de alinhamento)

- [ ] Manter `borealis-roadmap-v1` e `borealis-checklist-v1` sincronizados com decisoes do R3
- [ ] Executar checkpoint de alinhamento Borealis no fechamento de `R3.M0`
- [ ] Executar checkpoint de alinhamento Borealis no fechamento de `R3.M5`
- [ ] Executar checkpoint de alinhamento Borealis no fechamento de `R3.M9`
- [ ] Registrar divergencias entre trilha R3 e trilha Borealis com owner e prazo

## Evidencia minima para marcar item como concluido

- [ ] Comando executado + resultado
- [ ] Arquivo de teste novo ou alterado
- [ ] Commit/PR de fechamento
- [ ] Risco residual (se houver)
