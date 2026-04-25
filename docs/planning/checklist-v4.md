# Zenith Language Checklist 4.0 (Core)

> Checklist operacional para o motor da linguagem Zenith.
> Status: Em progresso | Atualizado: 2026-04-25

---

## 1. Fase 1: Estabilização & Infra

### R4.M0 - Alpha.2 stabilization base
- [x] Reduzir warnings de build para zero.
- [x] Consolidar baseline de qualidade/perf por plataforma.
- [x] Automatizar script de release unificado (Build.bat + Driver).
- [ ] Validar ARC em cenários de alta carga (Stress Test).

---

## 2. Fase 2: Concorrência (Surface)

### R4.CF1 - Concurrency Full
- [ ] Implementar keyword `task`.
- [ ] Implementar canais (`chan<T>`).
- [ ] Validar `Shared<T>` com contagem de referência atômica.
- [ ] Adicionar testes de race condition no compilador.

---

## 3. Fase 3: Interoperabilidade (FFI)

### R4.M2 - FFI Architecture 1.0
- [ ] Estabilizar tipos FFI na Spec.
- [ ] Implementar conversão automática de `string` para `*int8` (C-String).
- [ ] Adicionar suporte a `extern fn` com convenção de chamada customizável.

---

## 4. Fase 4: Otimização de Memória

### R4.M1 - ARC Optimization
- [ ] Implementar "Inline ARC" para structs pequenas.
- [ ] Otimizar eliminação de incrementos redundantes em retornos de função.
- [ ] Validar budgets de latência do coletor (ARC).

---

## 5. Fase 5: Release Alpha.2

### R4.M4 - Alpha.2 Official Release
- [ ] Gerar binário unificado (Compilador + Stdlib).
- [ ] Validar matriz de compatibilidade (Windows 10/11, x64).
- [ ] Gerar relatório de evidências de estabilidade.
