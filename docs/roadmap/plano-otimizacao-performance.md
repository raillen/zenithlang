# Plano de Otimizacao e Performance

- Status: planejamento tecnico
- Data: 2026-04-20
- Escopo: compilador, runtime, stdlib, CLI e IDE

## Objetivo

Transformar performance em parte do ciclo oficial de engenharia.

Regra central:

- medir primeiro
- otimizar depois
- bloquear regressao sempre

## Leitura Rapida

Pilares do plano:

1. Medicao confiavel
2. Suite oficial de benchmarks
3. Gate de regressao em PR e release
4. Otimizacoes de alto impacto por camada

## Fase 1: Medicao Confiavel

Sem metrica consistente, qualquer otimizacao vira aposta.

### Metricas base

1. Latencia (mediana e p95)
2. Memoria de pico
3. Alocacoes
4. Tamanho de binario
5. Tempo por fase de compilacao

### Escopos de medicao

1. `zt check`
2. `zt build`
3. `zt run`

### Cenarios

1. `small`
2. `medium`
3. `large`

### Regra minima de coleta

1. Rodar 5 a 10 repeticoes por benchmark
2. Usar mediana como valor principal
3. Versionar baseline por maquina e SO

## Fase 2: Suite Oficial de Benchmarks

Criar runner unico em `tests/perf`.

### Grupos de benchmark

1. Compilador
- lexer
- parser
- binder
- typechecker
- lowering
- emitter

2. Runtime
- text
- bytes
- list
- map
- collections

3. Stdlib
- json
- format
- math
- random
- validate

### Saidas obrigatorias

1. JSON para CI
2. Markdown para leitura humana

## Fase 3: Gate de Regressao

### Politica

1. Definir budget por benchmark (`warn` e `fail`)
2. PR roda suite curta
3. Nightly roda suite longa
4. Release bloqueia com regressao acima do budget sem override documentado

### Objetivo operacional

Evitar regressao silenciosa.

## Fase 4: Otimizacoes de Maior Impacto

## 4.1 Compilador

1. Compilacao incremental por modulo/hash
2. Daemon persistente para reduzir cold start
3. Cache de resolucao semantica e lowering

## 4.2 Backend C

1. Reduzir duplicacao por monomorfizacao
2. Melhorar deduplicacao de instancias canonicas

## 4.3 Runtime

1. Reduzir churn de retain/release em hot paths
2. Priorizar wrappers stack-first para `optional` e `result`
3. Otimizar COW em colecoes grandes

## 4.4 CLI e IDE

1. Indexacao incremental de workspace
2. Cancelamento de jobs antigos
3. Streaming incremental de resultados

## KPIs sugeridos para 90 dias

1. `zt check` warm: -35% no tempo medio
2. `zt build` medium: -25% no tempo medio
3. `zt run` em app tipico: -20% na latencia total
4. Regressao de binario: <= +5% sem justificativa
5. Zero regressao silenciosa com gate ativo

## Sequencia Recomendada

1. Fechar Fase 1 (medicao)
2. Fechar Fase 2 (suite)
3. Fechar Fase 3 (gate)
4. Executar Fase 4 com prioridades guiadas pelos dados

## Definicao de Pronto

O plano esta operacional quando:

1. Existe baseline versionado
2. PR e release usam gate de performance
3. Regressao relevante falha pipeline
4. Relatorio de performance acompanha release candidate
