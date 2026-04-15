# Zenith Compiler - Roadmap de Estabilizacao Real

> Nota: documento editorial/historico de estabilizacao.
> Para o estado corrente do produto self-hosted, consulte `roadmap/selfhost-pos100.md`, `specification/current-core.md` e `language/current.md`.

Atualizado em: 2026-04-15

Este documento registra o estado verificado da implementacao ativa do Zenith.

Ha duas trilhas diferentes no repositorio:

- Trilha ativa: parser/binder/codegen em Lua e runtime Lua. Esta continua sendo a referencia operacional da bateria principal e do backend atual.
- Trilha self-hosted: arquivos `.zt` do proprio compilador. O core canonico foi oficializado, o corte 100% self-hosted foi fechado e o residual pos-100 foi encerrado em roadmaps especificos.

## Leitura Rapida

| Grupo | Estado |
|---|---|
| Fases 1-6 | Concluidas na trilha ativa |
| Fase 7 | Concluida na trilha ativa com politica de compatibilidade |
| Fases 8-11 | Concluidas e validadas na trilha ativa |
| Fase 12 | Concluida: demo, stdlib e compilador self-hosted sem `native lua` em `.zt` |
| Self-hosting | Operacional: SH-1 a SH-5 concluidas; core canonico verde e bootstrap stage2/stage3 deterministico |

## Matriz de Status

| Fase | Tema | Status Real | Leitura |
|---|---|---|---|
| 1 | Pipeline semantico + DiagnosticBag | Concluida | `ztc check/build/run` funciona na trilha ativa |
| 2 | Traits e `apply` | Concluida | Parser, binder e runtime cobrem fluxo principal |
| 3 | String interpolation | Concluida | Lexer/parser/codegen suportam interpolacao |
| 4 | Operador `?` e `Outcome<T>` | Concluida | Optional/Outcome e early-return cobertos |
| 5 | Source maps | Concluida | `.lua.map` emitido no build |
| 6 | Type checker minimo | Concluida | Chamadas, retornos, agregados, defaults e contratos validados |
| 7 | Politica de `null` | Concluida na trilha ativa | uso direto de `null` gera erro dedicado `ZT-S106`; em nao-nullable ainda acumula `ZT-S100` |
| 8 | UFCS | Concluida | Metodo > global > virtual; receiver injetado corretamente |
| 9 | Pattern matching | Concluida | Destructuring, guards, `Color.Red` e exaustividade cobertos |
| 10 | Generics reais | Concluida | Type erasure, constraints e chamadas genericas funcionais |
| 11 | Indice 0 vs 1 | Concluida | Sequencias 1-based com `ZT-W002` e erro runtime `ZT-R011` |
| 12 | Reduzir `native lua` | Concluida | `demo.zt`, `src/stdlib` e `src/compiler` com 0 ocorrencias em `.zt` |

## Verificacao Executada

Bateria executada em 2026-04-14:

| Area | Comando | Resultado |
|---|---|---|
| Lexer | `lua tests\\lexer_tests\\test_lexer.lua` | 54/54 |
| Parser | `lua tests\\parser_tests\\test_parser.lua` | 62/62 |
| Nova sintaxe | `lua tests\\parser_tests\\test_new_syntax.lua` | 13/13 |
| Semantica | `lua tests\\semantic_tests\\test_semantic.lua` | 24/24 |
| Binder | `lua tests\\semantic_tests\\test_binder.lua` | 13/13 |
| Codegen | `lua tests\\codegen_tests\\test_codegen.lua` | OK |
| Optional/Outcome | `lua tests\\codegen_tests\\test_option_result_codegen.lua` | OK |
| Optional/Outcome stdlib | `lua ztc.lua run tests\\stdlib\\test_optional.zt` | OK |
| Where | `lua tests\\codegen_tests\\test_where_lowering.lua` | OK |
| Validate | `lua tests\\codegen_tests\\test_validate_lowering.lua` | OK |
| Match lowering | `lua tests\\codegen_tests\\test_match_lowering.lua` | OK |
| Async/await | `lua tests\\codegen_tests\\test_async_codegen.lua` | OK |
| Stdlib collections | `lua ztc.lua check src\\stdlib\\collections.zt` | OK |
| UFCS hardening | `lua ztc.lua check tests\\semantic\\test_ufcs_hardening.zt` | OK |
| Match hardening | `lua ztc.lua check tests\\semantic\\test_match_hardening.zt` | OK com `ZT-W100` esperado |
| Generics hardening | `lua ztc.lua check tests\\semantic\\test_generics_hardening.zt` | OK |
| Traits | `lua ztc.lua check tests\\core\\test_traits_v02.zt` | OK |
| Traits legado | `lua ztc.lua check tests\\core\\traits_test.zt` | OK |
| Fase 11 indexacao | `lua ztc.lua check tests\\semantic\\test_fase11_indexing.zt` | OK com 3 `ZT-W002` esperados |
| Fase 11 runtime | `lua .codex_fase11_safety.lua` | Falha esperada com `ZT-R011` |
| Fase 12 demo | `lua ztc.lua build demo.zt .codex_demo.lua` | OK |

Testes negativos esperados:

- `tests/fase7_null_error.zt`: falha corretamente em `var y: int = null`, com `ZT-S106` + `ZT-S100`.
- `tests/fase7_null.zt`: falha corretamente ja no caso nullable com `ZT-S106`, e no caso nao-nullable acumula `ZT-S100`.
- `tests/core/test_generic_constraints.zt`: falha corretamente quando `Dog` nao satisfaz `Greetable`.

Verificacao complementar da Trilha B em 2026-04-15:

| Area | Comando | Resultado |
|---|---|---|
| Self-hosted canonico | `lua ztc.lua check src\\compiler\\syntax.zt` | OK sem erros e sem warnings `ZT-W001` |
| Bridge legado | `lua ztc.lua check src\\compiler\\syntax_bridge.zt` | OK; arquivo congelado como stub legado |
| Hacks `string.char` em `syntax.zt` | `rg "string\\.char" src\\compiler\\syntax.zt` | 0 ocorrencias |
| Hacks `string.char` em `syntax_bridge.zt` | `rg "string\\.char" src\\compiler\\syntax_bridge.zt` | 0 ocorrencias |
| `null` em `syntax.zt` | `rg "\\bnull\\b" src\\compiler\\syntax.zt` | 1 linha textual: literal `"null"` do lexer/parser; 0 warnings `ZT-W001` |
| `null` em `syntax_bridge.zt` | `rg "\\bnull\\b" src\\compiler\\syntax_bridge.zt` | 0 ocorrencias |
| Lvalue indexado no codegen Lua | `lua tests\\codegen_tests\\test_index_assignment_codegen.lua` | OK |
| Bootstrap self-hosted | `lua tools\\bootstrap.lua` | OK; stage1/stage2/stage3 carregam e stage2/stage3 sao identicos |

## Fase 1 - Pipeline Semantico + DiagnosticBag

Status: concluida.

Evidencias:

- `ztc.lua` possui `check`, `build` e `run`.
- O fluxo parse -> bind -> lower/codegen esta integrado.
- `DiagnosticBag` nao quebra mais quando um diagnostico nao possui `span`.

Residuo:

- Existem entrypoints secundarios e documentacao historica que ainda podem divergir do caminho canonico.

## Fase 2 - Traits e apply

Status: concluida.

Evidencias:

- `trait` e `apply` sao aceitos pelo parser.
- O binder valida implementacoes e constraints basicas.
- Testes de traits passam na trilha ativa.

Residuo:

- A paridade ampla fora do core segue como consolidacao posterior; o bootstrap canonico nao precisa mais ser reaberto.

## Fase 3 - String Interpolation

Status: concluida.

Evidencias:

- Lexer/parser suportam fragmentos interpolados.
- Codegen usa concatenacao segura via runtime quando necessario.

Residuo:

- Os hacks antigos de caracteres foram removidos na SH-2; o residuo aqui passou a ser reduzir native lua legitimo remanescente.

## Fase 4 - Operador ? e Outcome<T>

Status: concluida.

Evidencias:

- Optional/Outcome passam no teste end-to-end.
- `zt.bang`, `Optional`, `Outcome` e early-return estao no runtime/codegen.

Residuo:

- Catalogo de diagnosticos pode ficar mais especifico no futuro.

## Fase 5 - Source Maps

Status: concluida.

Evidencias:

- `ztc.lua build` emite `.lua.map`.
- Builds de bootstrap e controle geraram saida com mapa.

Residuo:

- O mapa ainda e simples, por linha, nao um source map completo padrao.

## Fase 6 - Type Checker Minimo

Status: concluida na trilha ativa.

Entregue:

- Atribuicoes e retornos validados.
- Chamadas posicionais e nomeadas validadas.
- Parametros genericos resolvidos antes da assinatura real.
- Generics explicitamente instanciados em chamadas.
- Literais de lista/mapa validados elemento a elemento.
- Defaults de campos de struct validados.
- `where` e `validate` exigem predicado booleano.
- `std.validation` pode ser usado por contratos.

Residuo:

- Boa parte da logica ainda mora no binder. Isso e divida arquitetural, nao lacuna funcional da fase.

## Fase 7 - Politica de null

Status: concluida na trilha ativa com compatibilidade.

Regra atual:

- `T? = null`: erro semantico `ZT-S106`.
- `T = null`: erro semantico `ZT-S106` e tambem `ZT-S100`.
- Uso direto de `null` continua desencorajado; a linguagem deve preferir `Optional.Empty`, `Outcome.Failure` ou sentinelas explicitas.

Residuo:

- `BuiltinTypes.NULL` ainda existe por compatibilidade.
- A base self-hosted canonica nao usa mais null cru em consumidores; o residuo ficou restrito ao literal textual null do parser, compatibilidade de linguagem e runtime Lua.
- O parser ainda reconhece `null` por compatibilidade historica, mas o binder agora o rejeita com `ZT-S106`.

## Fase 8 - UFCS

Status: concluida.

Entregue:

- Prioridade correta: metodo de struct antes de funcao global, funcao global antes de virtual runtime.
- `self` em metodo e reconhecido como parametro real da linguagem, mas nao e duplicado na assinatura Lua com `:`.
- Chamadas UFCS globais emitem `func(receiver, ...)`.
- Chamadas UFCS virtuais emitem `zt.nome(receiver, ...)`.

Virtuais cobertos:

- `len`, `split`, `push`, `pop`, `keys`
- `is_empty`, `is_present`, `unwrap`, `to_text`

## Fase 9 - Pattern Matching

Status: concluida.

Entregue:

- Destructuring de listas e structs.
- Rest pattern 1-based coerente com Fase 11.
- Guards com `when`.
- Padroes qualificados como `Color.Red`.
- Aviso `ZT-W100` para enum nao exaustivo.

Residuo:

- O build atual ainda possui caminho de match nao dessugarado em alguns casos de programa completo; o lowering focado passa. Isso deve virar hardening de codegen, nao bloqueio semantico da fase.

## Fase 10 - Generics Reais

Status: concluida.

Entregue:

- Type erasure preservado no Lua gerado.
- Structs e funcoes genericas com constraints.
- Chamadas explicitas como `func<T>(x)` instanciam parametros antes da validacao.
- Violacoes de constraint falham de forma previsivel.

Residuo:

- Mensagens ainda usam familias genericas `ZT-S100/104/105`.

## Fase 11 - Indice 0 vs 1

Status: concluida.

Filosofia:

- Sequencias sao 1-based: `list`, `text`, slices e ranges usados como indice comecam em `1`.
- `map[K]` e chave, nao indice ordinal.
- APIs espaciais podem ser 0-based quando documentadas como coordenadas, por exemplo `grid.get(0, 0)`.

Entregue:

- `lista[0]`, `texto[0]` e range `0..N` geram `ZT-W002`.
- Acesso dinamico fora dos limites em lista/texto usa erro proprio `ZT-R011`.
- Mapas continuam com acesso direto por chave.
- Lowering de rest pattern usa slicing 1-based.

Residuo:

- Documentar exemplos canonicos em especificacao curta.
- Separar explicitamente "indice ordinal" de "coordenada" na stdlib de grid.

## Fase 12 - Reduzir native lua

Status: concluida.

Entregue:

- `demo.zt` nao contem `native lua`.
- `demo.zt` compila com `ztc.lua build`.
- Helpers UFCS reduziram a necessidade de escapes no caminho de demonstracao.
- Primeira fatia de stdlib essencial limpa: `std.core`, `std.collections` e `std.time` sem blocos `native lua`; `extern` top-level corrigido para bindings de runtime.
- Segunda fatia concluida: `std.test`, `std.os`, `std.text` e `std.text.regex` tambem ficaram sem blocos `native lua`; a stdlib `.zt` esta em 0 ocorrencias.
- Terceira fatia concluida: helpers e CLI self-hosted foram movidos para `extern`/runtime; `src/compiler` tambem esta em 0 ocorrencias de `native lua` em `.zt`.

Medicao atual:

- `demo.zt`: 0 ocorrencias de `native lua`.
- `src/stdlib`: 0 ocorrencias de `native lua` em arquivos `.zt`.
- `src/compiler`: 0 ocorrencias de `native lua` em arquivos `.zt`.
- Total medido em `src/stdlib src/compiler`: 0 ocorrencias em arquivos `.zt`.

Criterio para concluir a fase inteira:

- Reduzir `native lua` da stdlib essencial.
- Separar interop legitima com Lua de buraco de abstracao.
- Remover hacks de caracteres/tabelas cruas da trilha self-hosted.

## Trilha B - Paridade e Self-Hosting

Estas etapas nao substituem as Fases 1-12. Elas existem porque a base `.zt` do compilador ainda nao tem a mesma maturidade da trilha ativa.

| Item | Objetivo | Status Real | Evidencia |
|---|---|---|---|
| SH-1 | Fazer `ztc check src/compiler/syntax.zt` deixar de falhar massivamente | Concluida | `lua ztc.lua check src\\compiler\\syntax.zt` passa sem erros e sem warnings |
| SH-2 | Remover hacks de bootstrap como `native lua string.char(...)` | Concluida | `syntax.zt` e `syntax_bridge.zt` tem 0 ocorrencias de `string.char` |
| SH-3 | Alinhar a sintaxe usada pelo compilador self-hosted com a linguagem ativa | Concluida | `syntax.zt` e o caminho canonico; `syntax_bridge.zt` foi congelado explicitamente como legado parseavel |
| SH-4 | Reduzir `null`, tabelas Lua cruas e escapes excessivos | Concluida na trilha segura | A migracao massiva de `null` foi evitada; ausencia foi encapsulada por helpers; `syntax.zt` nao emite `ZT-W001` |
| SH-5 | Definir caminho verificavel de bootstrap confiavel | Concluida | `tools/bootstrap.lua` verifica `syntax.zt`, gera `stage1`, `stage2` e `stage3`, valida load e confirma determinismo no fluxo atual de `.selfhost-artifacts/bootstrap/summary.txt` |

Leitura consolidada:

- A Trilha B operacional nao esta mais aberta: SH-1, SH-2, SH-3, SH-4 e SH-5 foram atingidas.
- O bloqueio de bootstrap caiu; o stage2 compila o proprio `syntax.zt` e o stage3 fica identico ao stage2.
- O saneamento de `null` da base self-hosted canonica foi concluido sem importacao massiva insegura da stdlib.

### SH-1 - `syntax.zt` controlado

Status: concluida.

Evidencias:

- `lua ztc.lua check src\\compiler\\syntax.zt` passa sem erros.
- O arquivo nao emite mais `ZT-W001`.

Residuo:

- A promocao global para `ZT-S106` foi aplicada sem reabrir a Trilha B.
- O caminho canonico self-hosted deve ser tratado como `syntax.zt`, nao `syntax_bridge.zt`.

### SH-2 - hacks `string.char`

Status: concluida.

Evidencias:

- `syntax.zt` tem 0 ocorrencias de `string.char(...)`.
- `syntax_bridge.zt` continua sem hacks de caractere ao ficar como stub legado.

Residuo:

- Ainda existem escapes `native lua` legitimos/temporarios em `syntax.zt`, mas eles nao sao mais os hacks `string.char` que bloqueavam o bootstrap.
### SH-3 - alinhamento sintatico

Status: concluida.

Evidencias:

- `syntax.zt` ja fala o dialeto canonico aceito pela trilha ativa.
- `syntax_bridge.zt` foi reduzido a um stub legado explicito, com `check` verde e mensagem de aposentadoria.

Residuo:

- O repositorio ainda carrega um arquivo legado, mas ele nao disputa mais o caminho canonico.
- O proximo ganho aqui e remover referencias documentais antigas ao bridge, nao reabrir a sintaxe do bridge.

### SH-4 - `null`, tabelas cruas e escapes

Status: concluida na trilha segura.

Evidencias:

- A superficie `Optional`/`Outcome` foi alinhada com o runtime na trilha ativa.
- O lookup de operadores do lexer usa `TokenKind.BAD` como sentinel interno, removendo `null` dessa familia.
- Guards de listas/tabelas internas foram centralizados em `has_slot`, reduzindo `null` cru sem migracao massiva para `Optional`.
- Slots sequenciais de tabelas Lua em `syntax.zt` foram encapsulados por `has_slot(items, index)`.
- `syntax.zt` possui apenas a string `"null"` usada para reconhecer o literal da linguagem; `ZT-W001` caiu para zero.
- `syntax_bridge.zt` nao contribui mais para a metrica de `null` da trilha ativa.
- Lookup de simbolos foi encapsulado com `symbol_is_present`, `symbol_is_missing` e `scope_has_local_symbol`, reduzindo comparacoes diretas contra `null`.
- Cache e escopo de modulos foram encapsulados com `module_cache_has`, `module_cache_get`, `module_ast_is_present` e `module_scope_is_missing`.
- Diagnosticos do binder foram centralizados em `report_error_if_possible`, e import ausente agora emite `ZT-2003` em vez de ser ignorado.
- Parser recuperavel, campos opcionais de AST e spans opcionais foram migrados para `empty_value()`/helpers, sem `return null`, `: null`, `== null` ou `!= null`.
- Plano operacional: `docs/roadmap/null-to-optional.md`.

Residuo:

- A base self-hosted ainda usa helpers temporarios sobre `nil` Lua (`empty_value`, `value_is_present`) ate existir contrato ADT tipado interno sem regressao.
- A promocao para `ZT-S106` foi consolidada; o residuo passou a ser debito pequeno fora do core, nao mais bloqueio da trilha.

### SH-5 - bootstrap verificavel

Status: concluida.

Evidencias:

- `tools/bootstrap.lua` e um verificador nao-destrutivo, orientado a `src/compiler/syntax.zt`.
- O script gera `syntax_stage1.lua`, `syntax_stage2.lua` e `syntax_stage3.lua`, valida `loadfile` em cada etapa e grava resumo no fluxo atual de `.selfhost-artifacts/bootstrap/summary.txt`.
- `stage2` e `stage3` sao identicos, portanto o bootstrap esta deterministico.

Residuo:

- A promocao continua opt-in via `--promote`; a execucao padrao valida sem sobrescrever o compilador ativo.
- O saneamento de `null` foi concluido em SH-4 sem troca massiva insegura; a superficie `Optional`/`Outcome` segue coberta por `tests/stdlib/test_optional.zt`.
## Roadmap Reorganizado

Entregue na trilha ativa:

- Fase 1
- Fase 2
- Fase 3
- Fase 4
- Fase 5
- Fase 6
- Fase 7
- Fase 8
- Fase 9
- Fase 10
- Fase 11

Parcial:

- Nenhuma fase ativa neste recorte.

Trilha separada:

- Politica de `null` ja promovida para erro dedicado `ZT-S106`.

Concluida na Trilha B:

- SH-1
- SH-2
- SH-3
- SH-4
- SH-5

## Ordem Recomendada

O ciclo posterior ao fechamento da Trilha B foi consolidado em `docs/roadmap/selfhost-consolidacao.md`.

1. Reduzir `native lua` remanescente no compilador self-hosted: concluido.
2. Consolidar docs e cleanup apos a promocao de `ZT-S106`.
3. Limpar o legado documental de `syntax_bridge.zt`.
4. Escrever a especificacao curta do core estabilizado.
5. Fechar o hardening restante de stdlib e `match`.

## Resumo Executivo

O compilador ativo tem Fases 1-12 concluidas e testadas no recorte `.zt`: demo, stdlib e compilador self-hosted estao sem blocos `native lua`.

Na Trilha B, SH-1, SH-2, SH-3, SH-4 e SH-5 deixaram de ser problema: `src/compiler/syntax.zt` passa no `check` sem `ZT-W001`, `syntax_bridge.zt` foi congelado como legado explicito, os hacks `string.char` foram removidos e o bootstrap self-hosted passa com stage2/stage3 deterministico.

O principal trabalho restante nao e reabrir UFCS, generics, match, indexacao, bootstrap, saneamento de `null` ou Fase 12. O ciclo de consolidacao foi fechado; `std.events`, `std.fs`, `std.os`, `std.os.process` e o shim legado `src/cli/ztc.zt` tambem foram estabilizados em checks locais. O residuo agora volta a ser manutencao normal e hardening incremental.
