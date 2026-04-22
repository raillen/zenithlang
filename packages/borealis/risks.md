# Borealis Risk Baseline

Data: 2026-04-22  
Ciclo: Borealis 1.0

## P0

1. Divergencia entre API documentada e API real
- Impacto: onboarding quebra e exemplos ficam invalidos.
- Owner: Borealis Core.
- Prazo: antes de fechar `R3.B3`.
- Mitigacao: atualizar docs no mesmo PR de API e rodar `./zt.exe check packages/borealis/zenith.ztproj --all`.

## P1

1. Semantica de input incompleta (`key_pressed` e `key_released` ainda stub)
- Impacto: gameplay por frame pode ficar inconsistente.
- Owner: Borealis Input.
- Prazo: fechamento de `R3.B2`.
- Mitigacao: implementar transicao por frame + testes de borda.

2. Duplicacao de tipos entre camadas `game` e `engine`
- Impacto: custo de manutencao maior e risco de drift.
- Owner: Borealis Architecture.
- Prazo: ate `R3.B5`.
- Mitigacao: unificar contratos internos apos estabilizar naming v1.

## P2

1. Backend desktop real ainda nao ligado por padrao
- Impacto: limita validacao visual/performance real.
- Owner: Borealis Backend.
- Prazo: `R3.B7`.
- Mitigacao: manter stub estavel e fechar linker profile antes do E2E desktop.
