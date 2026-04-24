# Borealis Risk Baseline

Data: 2026-04-22  
Ciclo: Borealis 1.0

## P0

1. Divergencia entre API documentada e API real
- Impacto: onboarding quebra e exemplos ficam invalidos.
- Owner: Borealis Core.
- Prazo: continuo durante todo o ciclo.
- Mitigacao: atualizar docs no mesmo PR de API e rodar `./zt.exe check packages/borealis/zenith.ztproj --all`.

## P1

1. Duplicacao de tipos entre camadas `game` e `engine`
- Impacto: custo de manutencao maior e risco de drift.
- Owner: Borealis Architecture.
- Prazo: acompanhamento continuo apos `R3.B5`.
- Mitigacao: unificar contratos internos apos estabilizar naming v1.

## P2

1. Backend desktop real ainda nao ligado por padrao
- Impacto: limita validacao visual/performance real.
- Owner: Borealis Backend.
- Prazo: fechamento tecnico final de `R3.B7`.
- Mitigacao: manter stub estavel e fechar linker profile antes do E2E desktop.

2. Fluxo ZPM ainda depende do comando `zpm`
- Impacto: package esta preparado, mas nao instalado/publicado por fluxo real.
- Owner: Borealis Packaging.
- Prazo: alinhado ao roadmap `R4.M2`.
- Mitigacao: manter manifesto, semver, changelog, guia e docs prontos para migracao direta.

3. Cache nativo compartilhado em `.ztc-tmp` durante execucoes simultaneas
- Impacto: rodadas paralelas de `./zt.exe run/build` podem disputar o mesmo `zenith_rt.o` e gerar falhas intermitentes de validacao.
- Owner: Compiler Runtime.
- Prazo: hardening tecnico apos fechamento dos bloqueios P1 do backend C.
- Mitigacao: manter validacoes Borealis em sequencia ate haver isolamento/lock do cache.

4. Hardening continuo do emitter C para generics ricos fora da coverage atual
- Impacto: o caminho que bloqueava `optional<Struct>` do Borealis foi fechado, mas combinacoes mais profundas de `map/optional/outcome` ainda merecem cobertura adicional.
- Owner: Compiler Backend.
- Prazo: acompanhamento continuo enquanto a API do Borealis aprofundar modulos alem do scaffold.
- Mitigacao: toda nova combinacao real descoberta no Borealis deve virar fixture de regressao.
