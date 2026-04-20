# Roadmap: Oficializacao Self-Hosted

> Estado: historico.
> Superado por `selfhost-100.md` para o corte 100% e por `selfhost-pos100.md` para o estado operacional/editorial atual.

Atualizado em: 2026-04-15

Status do ciclo: concluido em 2026-04-15.

Este documento registra o fechamento do ciclo aberto apos `selfhost-consolidacao.md`.

A Trilha B e a consolidacao operacional do core canonico ja estavam fechadas. O objetivo deste roadmap era transformar a linha self-hosted em face oficial do produto, sem reabrir as Fases 1-12 da trilha ativa. Esse objetivo foi concluido no recorte seguro definido neste proprio plano.

## 1. Estado Final

| Area | Estado final | Evidencia |
|---|---|---|
| Front door oficial | `ztc.lua` com engine self-hosted por padrao | `lua ztc.lua --selfhost --version` |
| Fallback oficial | `--strict-selfhost` explicito para bootstrap e rede de seguranca | `lua ztc.lua --strict-selfhost --version` |
| Core self-hosted canonico | `src/compiler/syntax.zt` | `lua ztc.lua --selfhost check src/compiler/syntax.zt` |
| Bootstrap | deterministico com promocao controlada | `lua tools/bootstrap.lua` |
| ABI de host | pequena, nomeada e congelada | `tools/selfhost_abi.lua`, `docs/specification/selfhost-abi.md` |
| Gate de release | recorrente e automatizado | `lua tools/selfhost_release.lua` |
| Superficie adjacente | congelada como experimental fora do caminho oficial | RFC 002 + docs canonicos |

## 2. Definicao de Pronto Cumprida

Este ciclo foi encerrado porque todos os criterios de corte ficaram satisfeitos:

- a documentacao principal parou de descrever a trilha historica como face principal da linguagem;
- existe caminho self-hosted oficial para `check`, `build` e `run`, com fallback legado explicito;
- a fronteira host/runtime do compilador self-hosted ficou pequena, nomeada e versionavel;
- a superficie self-hosted fora do core deixou de ficar em estado ambiguo;
- a trilha historica foi rebaixada para bootstrap/fallback controlado.

## 3. Quatro Frentes Encerradas

### Frente 1. Front door oficial

Resultado entregue:

- `ztc.lua` passou a operar com engine self-hosted por padrao;
- `--strict-selfhost` e `--strict-selfhost` viraram mecanismos explicitos de transicao;
- `check`, `build` e `run` funcionam pela frente self-hosted no corpus de promocao;
- a documentacao principal foi reescrita para apresentar a trilha self-hosted como face oficial do produto.

### Frente 2. Soberania alem do core canonico

Resultado entregue:

- `src/compiler/syntax.zt` permanece como unidade canonica unica;
- `src/compiler/syntax_bridge.zt` continua apenas como legado parseavel;
- `binder.zt`, `parser.zt` e `lexer.zt` ficaram explicitamente congelados como experimentais;
- o caminho oficial deixou de depender editorialmente de qualquer modulo `.zt` adjacente incompleto.

### Frente 3. Fronteira host/runtime e bootstrap

Resultado entregue:

- a ABI `zt.selfhost.*` foi inventariada em manifesto e em documento curto;
- a regra de extensao da ABI ficou explicita: runtime, `extern`, manifesto, docs e gate precisam andar juntos;
- o bootstrap continua deterministico e agora faz parte do fluxo de promocao self-hosted;
- helpers fora da ABI nomeada deixaram de ser tratados como contrato implicito.

### Frente 4. Gate de produto e release

Resultado entregue:

- `tools/selfhost_release.lua` virou gate recorrente de promocao;
- o gate valida manifesto ABI, bindings de runtime, bootstrap, promocao para `ztc_selfhost.lua` e corpus minimo;
- a regressao self-hosted deixou de ser observacao editorial e passou a ser falha de release;
- builtins de CLI podem usar fallback legado controlado quando ainda nao cabem na superficie promovida.

## 4. Fases 1 a 5

### Fase 1. Congelar arquitetura e criterios de corte

Status: concluida em 2026-04-15.

Entregue:

- RFC 002 registrando a arquitetura canonica;
- corpus minimo de promocao congelado;
- ABI inicial `zt.selfhost.*` congelada para o ciclo.

### Fase 2. Promover o front door em modo dual

Status: concluida em 2026-04-15.

Entregue:

- `ztc.lua` passou a priorizar o compilador self-hosted;
- `--strict-selfhost` e `--strict-selfhost` ficaram disponiveis como trilhas explicitas;
- `tools/bootstrap.lua` foi ajustado para usar a trilha legada apenas onde isso faz parte do bootstrap, nao como face do produto;
- docs principais passaram a chegar no self-hosted antes do legado.

### Fase 3. Promover ou congelar a superficie adjacente

Status: concluida em 2026-04-15.

Entregue:

- congelamento editorial definitivo de `binder.zt`, `parser.zt` e `lexer.zt`;
- confirmacao de que o caminho oficial continua monolitico em `src/compiler/syntax.zt`;
- remocao da ambiguidade entre laboratorio e modulo promovido.

### Fase 4. Selar a ABI de host e o bootstrap

Status: concluida em 2026-04-15.

Entregue:

- manifesto `tools/selfhost_abi.lua`;
- documento curto `docs/specification/selfhost-abi.md`;
- verificacao de consistencia entre `extern`, runtime host e manifesto;
- bootstrap deterministico incorporado ao gate de release.

### Fase 5. Oficializar release e rebaixar a trilha historica

Status: concluida em 2026-04-15.

Entregue:

- gate `lua tools/selfhost_release.lua`;
- promocao controlada para `ztc_selfhost.lua`;
- `ztc.lua` reposicionado como front door oficial self-hosted com fallback legado;
- docs de status atualizados para refletir a nova identidade do produto.

## 5. Validacao Minima Fechada

Recorte verde usado para encerrar este roadmap:

- `lua ztc.lua --selfhost --version`
- `lua ztc.lua --strict-selfhost --version`
- `lua ztc.lua --selfhost check src/compiler/syntax.zt`
- `lua ztc.lua --selfhost build demo.zt .selfhost-release-demo.lua`
- `lua ztc.lua --selfhost run tests/stdlib/test_optional.zt`
- `lua ztc.lua --selfhost zpm version`
- `lua tools/bootstrap.lua`
- `lua tools/selfhost_release.lua`
- `lua tests/parser_tests/test_parser.lua`
- `lua tests/semantic_tests/test_binder.lua`
- `lua tests/semantic_tests/test_semantic.lua`
- `lua tests/codegen_tests/test_codegen.lua`
- `lua tests/codegen_tests/test_match_lowering.lua`
- `lua tests/codegen_tests/test_option_result_codegen.lua`

## 6. Limites Deliberados do Corte

Este fechamento nao afirma:

- eliminacao total do runtime Lua;
- promocao automatica dos modulos experimentais adjacentes;
- paridade self-hosted irrestrita para toda a superficie de builtins de CLI;
- fim de hardening incremental no compilador.

O que ele afirma e mais preciso:

- a linha self-hosted virou a face oficial do produto;
- a trilha historica ficou preservada como bootstrap/fallback;
- o core canonico, a ABI e o gate de release estao fechados no recorte seguro.

## 7. Fechamento

A discussao deixa de ser "o self-hosted ja pode ser oficial?" e passa a ser apenas manutencao normal de produto e reducao gradual da superficie de fallback.

Marco final deste ciclo:

- `src/compiler/syntax.zt` continua sendo o core canonico;
- `ztc.lua` apresenta a linguagem pela trilha self-hosted;
- a ABI de host ficou curta e documentada;
- o release self-hosted tem gate recorrente;
- o legado em Lua virou suporte de bootstrap, nao identidade principal.
