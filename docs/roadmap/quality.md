# Roadmap de Qualidade e Blindagem (ZTest)

Este documento detalha a estratégia de estabilização industrial da linguagem Zenith através da Auditoria 500 e do orquestrador ZTest.

## 1. Objetivos de Qualidade

1.  Regressão Zero: Garantir que a evolução para o compilador nativo (Fase 10) não quebre funcionalidades do núcleo Lua.
2.  Confiança Estática: Validar o sistema de tipos contra casos de borda e aninhamentos complexos.
3.  Saúde Visível: Prover relatórios em tempo real sobre a estabilidade do ecossistema.

---

## 2. O Orquestrador ZTest

ZTest é a ferramenta oficial de teste do Zenith, integrada ao comando `zt test`.

- Framework Nativo: DSL com `describe`, `test` e assertions coloridas.
- Isolamento Industrial: Execução de suites em processos separados via `os.run_command`.
- Aura UI: Relatórios com timers de alta precisão e contadores de progresso.
- Snapshot Testing: Validação de dados complexos e saída de codegen via `assert_snapshot`.

---

## 3. O Desafio da Auditoria 500

A meta é atingir 500 testes de integração cobrindo todas as áreas da linguagem.

### Status de Cobertura (v0.2.8)
- Suites Ativas: 54
- Asserções Estimadas: >250
- Áreas Cobertas: Lexer, Parser, Semântica, Stdlib (FS, OS, JSON, Crypto, Time, Text, Net, Math, Events, Log).

---

## 4. Técnicas de QA Avançado

- Fuzz Testing: Meta para v0.3.x (Geração de gramática aleatória para estresse do parser).
- Memory Audit: Monitoramento de consumo de recursos em blocos native lua.
- Exportação de Relatório: Geração de `test-health.json` para o Dashboard Web.

---
*Atualizado em: 11 de Abril de 2026*
