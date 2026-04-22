# Zenith Checklist 3.0

## Objetivo

Executar e validar as entregas do roadmap 3.0 com evidencias objetivas.

## Escopo

- Itens de execucao por milestone do R3.
- Gates obrigatorios de qualidade, performance e confiabilidade.

## Dependencias

- Upstream:
  - `docs/planning/roadmap-v3.md`
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

Status inicial: nao iniciado
Data de criacao: 2026-04-21

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

## R3.M0 - Baseline e alinhamento

- [ ] Sincronizar roadmap/checklist/status para estado unico
- [ ] Congelar baseline de qualidade do ciclo
- [ ] Congelar baseline de performance do ciclo
- [ ] Publicar matriz de riscos R3 (P0/P1/P2) com owner e prazo
- [ ] Abrir relatorio de kickoff do ciclo

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

## Evidencia minima para marcar item como concluido

- [ ] Comando executado + resultado
- [ ] Arquivo de teste novo ou alterado
- [ ] Commit/PR de fechamento
- [ ] Risco residual (se houver)
