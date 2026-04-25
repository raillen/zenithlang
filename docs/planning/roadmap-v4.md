# Zenith Language Roadmap 4.0 (Core)

## Objetivo

Definir o ciclo R4 focado na **infraestrutura técnica** da linguagem Zenith:
- Estabilidade do motor de inferência e backends.
- Implementação completa do modelo de concorrência.
- Performance e segurança de memória (ARC refinements).

## Escopo

- Planejamento do motor da linguagem (R4.M0 ate R4.M9).
- Evolução da Spec e conformidade do compilador.
- Infraestrutura de Build e CI para desenvolvedores do core.

---

## 1. Backlog Técnico: Concorrência (R4.CF1)

Objetivo: Implementar a surface completa de concorrência.
- [ ] Modelo oficial `task`.
- [ ] Modelo oficial `channel`.
- [ ] `jobs.spawn/join` ou API equivalente.
- [ ] `Shared<T>` explícito e thread-safe.
- [ ] Checker-level `transferable` predicate.
- [ ] Testes de corrida e determinismo.

## 2. Fases do Roadmap Core 4.0

### R4.M0 - Alpha.2 stabilization base
- [x] Reduzir warnings de build para zero.
- [x] Consolidar baseline de qualidade/perf por plataforma.
- [x] Automatizar script de release unificado.

### R4.M1 - Memory Safety & ARC Optimization
- [ ] Implementar análise de escape para evitar alocações desnecessárias.
- [ ] Otimizar incrementos/decrementos de ARC em blocos contíguos.
- [ ] Detectar e reportar ciclos de referência simples estaticamente.

### R4.M2 - FFI Architecture 1.0
- [ ] Estabilização do contrato de chamadas externas.
- [ ] Mapeamento seguro de tipos (`int32`, `poid`, `cstr`).
- [ ] Suporte a callbacks C chamando código Zenith.

### R4.M3 - Unified Backend: C & Native (WIP)
- [ ] Melhorar a qualidade do C emitido (legibilidade para debugging).
- [ ] Infraestrutura básica para backend LLVM/Natividade (exploração).

### R4.M4 - Alpha.2 Official Release (Language Core)
- [ ] Congelamento da Spec para Alpha.2.
- [ ] Validação de compatibilidade cross-platform (Win/Linux).

---

## Gates de Qualidade (Core)
1. `python build.py` verde.
2. `python run_all_tests.py` verde.
3. Performance dentro dos budgets definidos.
4. Sem vazamentos de memória detectados em Valgrind/ASAN.

