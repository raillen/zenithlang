# Roadmap: Consolidacao Self-Hosted

Atualizado em: 2026-04-15

Este plano cobriu o ciclo posterior ao fechamento da Trilha B.

O bootstrap self-hosted canonico ja esta operacional: `src/compiler/syntax.zt` passa no `check`, nao emite `ZT-W001` e o bootstrap `stage2/stage3` e deterministico. Nesta rodada, o ciclo foi fechado com consolidacao da politica `ZT-S106`, reducao adicional de `native lua`, limpeza do legado documental de `syntax_bridge.zt` e especificacao curta do core estabilizado.

## 1. Estado de Partida

Base verificada:

| Area | Estado atual | Evidencia |
|---|---|---|
| Core self-hosted | Verde | `lua ztc.lua check src\\compiler\\syntax.zt` |
| Bootstrap | Deterministico | `lua tools\\bootstrap.lua` |
| `null` em `syntax.zt` | 1 ocorrencia textual do literal `"null"`; 0 warnings `ZT-W001` | `rg "\\bnull\\b" src\\compiler\\syntax.zt` |
| `native lua` no compilador | 0 ocorrencias em `.zt` | `rg "native lua" src\\compiler -g "*.zt"` |
| `native lua` na stdlib | 0 ocorrencias em `.zt` | `rg "native lua" src\\stdlib -g "*.zt"` |
| Bridge legado | Congelado como stub parseavel | `lua ztc.lua check src\\compiler\\syntax_bridge.zt` |

## 2. Definicao de Pronto

Este ciclo foi considerado concluido quando:

- a linha self-hosted continuar com bootstrap deterministico e `check` verde;
- a politica ativa de `null` legado estiver consolidada em binder, docs e testes;
- o debito remanescente de `native lua` estiver classificado e reduzido;
- a documentacao parar de tratar `syntax_bridge.zt` como caminho vivo;
- existir uma especificacao curta do core estabilizado;
- o residuo de hardening fora do bootstrap estiver isolado em testes e itens pequenos.

## 3. Roadmap 1 a 5

### 1. Reduzir `native lua` no core self-hosted

Status: concluida.

Objetivo: separar interop legitima de buraco de abstracao no compilador canonico.

Tarefas:

- inventariar cada uso de `native lua` em `src/compiler`;
- classificar por grupo: interop inevitavel, helper temporario, debito de abstracao;
- eliminar primeiro os casos que podem virar Zenith puro sem reabrir o bootstrap;
- manter contagem de ocorrencias no `roadmap_estabilizacao.md`.

Criterio de aceite:

- a contagem em `src/compiler` cai sem regressao de bootstrap;
- cada escape remanescente tem justificativa curta e localizada.

### 2. Consolidar a politica ativa de `null` legado (`ZT-S106`)

Status: concluida.

Objetivo: fechar a semantica publica de compatibilidade de `null`.

Tarefas:

- documentar exatamente onde `null` ainda e aceito por compatibilidade;
- decidir entre tres politicas: manter warning, promover para erro em codigo novo, ou promover com allowlist de compatibilidade;
- alinhar binder, catalogo de erros e docs de stdlib.

Criterio de aceite:

- existe uma regra unica, escrita e rastreavel para `ZT-S106`;
- a mensagem do diagnostico e os docs nao divergem.

### 3. Limpar o legado documental de `syntax_bridge.zt`

Status: concluida.

Objetivo: remover ambiguidade editorial sobre qual compilador self-hosted e o canonico.

Tarefas:

- revisar `docs/roadmap*`, `docs/` principais e notas historicas;
- marcar `syntax_bridge.zt` apenas como stub legado parseavel;
- apontar sempre para `src/compiler/syntax.zt` como caminho oficial self-hosted.

Criterio de aceite:

- nao restam docs principais descrevendo a trilha self-hosted como aberta por causa do bridge;
- o bridge aparece apenas como legado, nunca como alvo de paridade.

### 4. Especificar o core estabilizado

Status: concluida.

Objetivo: transformar o comportamento hoje protegido por testes em especificacao curta.

Tarefas:

- consolidar uma especificacao curta para Optional/Outcome, politica de `null`, indexacao 1-based e contratos centrais do binder;
- registrar o que e garantia de linguagem e o que ainda e detalhe de implementacao;
- usar a especificacao para reduzir ambiguidade em docs editoriais.

Criterio de aceite:

- existe um documento curto, versionavel, cobrindo as decisoes semanticas ja estabilizadas;
- novas mudancas passam a comparar implementacao contra especificacao, nao apenas contra comportamento historico.

### 5. Hardening fora do bootstrap

Status: concluida.

Objetivo: fechar o residuo tecnico que ainda impede chamar a linha de self-hosted de consolidada como produto.

Tarefas:

- continuar a Fase 12 na stdlib essencial;
- isolar e corrigir o que ainda faltar de codegen completo para `match` fora dos testes de lowering/hardening;
- manter testes negativos e end-to-end do core como gate de regressao.

Criterio de aceite:

- os residuos remanescentes deixam de ser blocos amplos e passam a caber em issues pequenas;
- nao sobra trabalho estrutural escondido atras do rotulo `self-hosted parcial`.

## 4. Ordem Recomendada

1. Reducao de `native lua` no compilador: concluida (`18 -> 0` no recorte `.zt`).
2. Consolidacao de `ZT-S106`: concluida em binder, docs e testes.
3. Limpeza documental de `syntax_bridge.zt`: concluida nos docs principais.
4. Especificacao curta do core: concluida com `docs/specification/current-core.md`.
5. Hardening fora do bootstrap: concluido nesta fatia com correcao do codegen de `is` para `union`/`nullable` e bateria central verde.

## 5. Validacao Minima por Corte

- `lua ztc.lua check src\\compiler\\syntax.zt`
- `lua tools\\bootstrap.lua`
- `lua tests\\semantic_tests\\test_binder.lua`
- `lua tests\\semantic_tests\\test_semantic.lua`
- `lua tests\\codegen_tests\\test_codegen.lua`
- `lua tests\\codegen_tests\\test_match_lowering.lua`
- `lua tests\\codegen_tests\\test_option_result_codegen.lua`
- `lua ztc.lua run tests\\stdlib\\test_optional.zt`
## 6. Fechamento

Status final: concluido operacionalmente.

Entregue nesta rodada:

- `src/compiler/syntax.zt`: `native lua` caiu de `18` para `0` ocorrencias em `.zt`, sem reabrir o bootstrap.
- `ZT-S106`: politica de `null` consolidada em binder, testes e docs.
- `syntax_bridge.zt`: legado documental limpo nos docs principais; caminho canonico reforcado como `src/compiler/syntax.zt`.
- `docs/specification/current-core.md`: especificacao curta criada para o core estabilizado.
- codegen Lua: `is` agora suporta `union` e `nullable` sem falha interna, cobrindo `tests/semantic/test_alias_unions.zt`.
- stdlib fora do core: `std.events`, `std.fs`, `std.os` e `std.os.process` passam no `check`; os testes de `events`, `fs` e `process` executam.
- `src/cli/ztc.zt`: substituido por shim legado verificavel, apontando para `ztc.lua` e `src/compiler/syntax.zt`.

Residuo apos o fechamento deste roadmap:

- `native lua` remanescente em `.zt` foi zerado na stdlib e no compilador; interop legitima ficou concentrada no runtime Lua.
- falsos negativos fora do core canonico foram reduzidos; os que sobrarem devem ser tratados como manutencao pontual, nao como reabertura da trilha.
