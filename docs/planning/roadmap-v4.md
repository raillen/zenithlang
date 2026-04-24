# Zenith Roadmap 4.0

## Objetivo

Definir o ciclo R4 com foco em:

- estabilidade real para alpha.2;
- produto com valor de mercado;
- ecossistema pratico (web + FFI + tooling);
- base consistente para consumers reais do ecossistema, incluindo Borealis.

## Escopo

- Planejamento do ciclo R4 (R4.M0 ate R4.M9).
- Prioridade em entrega utilizavel por dev externo.
- Integracao entre linguagem, tooling e distribuicao.

## Dependencias

- Upstream:
  - `docs/planning/roadmap-v2.md`
  - `docs/planning/checklist-v2.md`
  - `language/spec/*`
- Downstream:
  - `docs/planning/checklist-v4.md`
  - `docs/reports/release/*`
  - `docs/reports/compatibility/*`
  - `packages/borealis/*`
  - `packages/borealis/decisions/*`
  - `docs/planning/borealis-roadmap-v1.md`
  - `docs/planning/borealis-checklist-v1.md`

Status: proposto  
Data: 2026-04-21  
Base: `0.3.0-alpha.1` publicado (`R2.M12` fechado)

## Leitura rapida

Direcao do R4:

1. estabilizar alpha.2 (warnings + budgets + release flow);
2. entregar DX forte (LSP + VSCode);
3. entregar backend de valor de mercado (web + sqlite + http);
4. publicar produtos simples que geram adocao;
5. manter o ecossistema facil de consumir por packages reais como Borealis.

Modelo de capacidade inicial:

- 35% estabilidade e qualidade;
- 30% produto e DX;
- 25% features de plataforma (web + FFI);
- 10% exploracao (UI/grafica).

## Gates obrigatorios do ciclo

Nenhuma milestone avanca sem:

1. `python build.py` verde
2. `python run_all_tests.py` verde
3. `tests/perf/gate_pr.ps1` verde
4. sem crash novo em fuzz/corpus
5. checklist da milestone com evidencia

Para release alpha.2:

1. `tests/perf/gate_nightly.ps1` verde
2. budgets atualizados e justificados
3. zero erro novo de compatibilidade
4. artefato com install limpo validado

## Fora de escopo imediato

- framework grafico completo;
- toolkit UI nativo multiplataforma;
- engine de jogo.

Regra: so iniciar UI/grafica depois de tracao real em CLI/web/backend.

## Fases do Roadmap 4.0

## R4.M0 - Alpha.2 stabilization base

Objetivo:

- fechar pendencias tecnicas do corte alpha.1.

Entregas:

- reduzir warnings de build para nivel controlado;
- fechar item pendente de budgets (`checklist-v2`, M5);
- consolidar baseline de qualidade/perf por plataforma;
- automatizar script de release com validacao de install limpo.

## R4.M1 - Compass LSP v1 + VSCode beta

Objetivo:

- fluxo de edicao pronto para uso diario.

Entregas:

- diagnostics em tempo real;
- hover de simbolos;
- go-to-definition;
- format-on-save com `zt fmt`;
- comando rapido `check/build/run` no editor.

## R4.M2 - ZPM MVP

Objetivo:

- instalar e compartilhar pacote sem friccao.

Entregas:

- `zpm init`;
- `zpm add <pkg>`;
- `zpm install`;
- lockfile reproduzivel;
- `zpm publish` (fluxo minimo).

## R4.M3 - Zenith Web Lite (sinatra-like)

Objetivo:

- entregar o primeiro framework web minimo e produtivo.

Entregas:

- rotas `get/post/put/delete`;
- path params e query params;
- middleware simples;
- helpers de `json` request/response;
- tratamento padrao de erro.

Nao objetivo deste corte:

- ORM completo;
- websocket completo;
- arquitetura de microservicos.

## R4.M4 - FFI 1.0 + Binding Pack C (fase 1)

Objetivo:

- conectar Zenith ao ecossistema C com seguranca basica.

Entregas:

- contrato FFI estavel;
- binding oficial `sqlite3`;
- binding oficial `libcurl` (cliente HTTP/TLS);
- mapeamento de erros para `core.Error`;
- exemplos E2E com testes.

## R4.M5 - Produto 1: VSCode Marketplace

Objetivo:

- canal oficial de distribuicao para adocao.

Entregas:

- extensao publicada no Marketplace;
- templates de projeto;
- snippets basicos;
- modo rapido de arquivo unico no CLI (`zt check/run arquivo.zt`) para onboarding e exemplos curtos, mantendo `zenith.ztproj` como fluxo canonico de projeto;
- guia de onboarding de 15 minutos.

## R4.M6 - Produto 2: Zenith CI Gate

Objetivo:

- gate de qualidade/performance pronto para times pequenos.

Entregas:

- action/app para GitHub com `check + test + perf`;
- relatorio de falha com links de artefato;
- regra de bloqueio por regressao critica.

## R4.M7 - Produto 3: Playground + trilha de aprendizado

Objetivo:

- facilitar teste da linguagem sem setup local.

Entregas:

- playground web (compilar/executar exemplos curtos);
- colecao de exemplos oficiais;
- trilha de aprendizado curta (iniciante -> app real).

## R4.M8 - UI/Grafica: decisao com POC

Objetivo:

- decidir com dados, sem abrir frente grande cedo.

Entregas:

- 1 POC pequeno de UI (ex.: wrapper simples);
- comparacao custo x impacto;
- decisao formal: continuar ou adiar.

## R4.M9 - Alpha.2 release

Objetivo:

- publicar `0.3.0-alpha.2` com foco em uso real.

Entregas:

- notas de release;
- pacote + checksums;
- matriz de compatibilidade atualizada;
- relatorio final de risco residual.

## Definicao de pronto por milestone

Uma milestone so fecha quando tiver:

1. spec atualizada (se houver mudanca de comportamento);
2. testes positivos e negativos;
3. evidencia em checklist;
4. sem regressao acima dos budgets;
5. docs de uso para dev externo.

## KPI minimo do ciclo R4

- tempo para "hello web api" <= 15 min;
- plugin VSCode com instalacao e uso validado;
- 2 bindings C oficiais em producao (`sqlite3`, `libcurl`);
- alpha.2 sem P0 aberto.
