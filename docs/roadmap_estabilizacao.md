# Zenith Compiler - Roadmap de Estabilizacao Real

Atualizado em: 2026-04-15

Este documento registra o estado verificado da implementacao ativa do Zenith.

Ha duas trilhas diferentes no repositorio:

- Trilha ativa: `ztc.lua`, parser/binder/codegen em Lua e runtime Lua. Esta e a trilha que compila os exemplos e testes principais hoje.
- Trilha self-hosted: arquivos `.zt` do proprio compilador. Ela ainda nao tem paridade total com a trilha ativa.

## Leitura Rapida

| Grupo | Estado |
|---|---|
| Fases 1-6 | Concluidas na trilha ativa |
| Fase 7 | Concluida na trilha ativa com politica de compatibilidade |
| Fases 8-11 | Concluidas e validadas na trilha ativa |
| Fase 12 | Parcial: demo limpo; stdlib/compiler ainda usam `native lua` |
| Self-hosting | Operacional: SH-1, SH-2, SH-3 e SH-5 concluidas; bootstrap stage2/stage3 deterministico |

## Matriz de Status

| Fase | Tema | Status Real | Leitura |
|---|---|---|---|
| 1 | Pipeline semantico + DiagnosticBag | Concluida | `ztc check/build/run` funciona na trilha ativa |
| 2 | Traits e `apply` | Concluida | Parser, binder e runtime cobrem fluxo principal |
| 3 | String interpolation | Concluida | Lexer/parser/codegen suportam interpolacao |
| 4 | Operador `?` e `Outcome<T>` | Concluida | Optional/Outcome e early-return cobertos |
| 5 | Source maps | Concluida | `.lua.map` emitido no build |
| 6 | Type checker minimo | Concluida | Chamadas, retornos, agregados, defaults e contratos validados |
| 7 | Politica de `null` | Concluida na trilha ativa | `null` em tipo nullable gera warning; em nao-nullable gera erro |
| 8 | UFCS | Concluida | Metodo > global > virtual; receiver injetado corretamente |
| 9 | Pattern matching | Concluida | Destructuring, guards, `Color.Red` e exaustividade cobertos |
| 10 | Generics reais | Concluida | Type erasure, constraints e chamadas genericas funcionais |
| 11 | Indice 0 vs 1 | Concluida | Sequencias 1-based com `ZT-W002` e erro runtime `ZT-R011` |
| 12 | Reduzir `native lua` | Parcial | `demo.zt` limpo; stdlib/compiler ainda dependem de escapes |

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

- `tests/fase7_null_error.zt`: falha corretamente em `var y: int = null`, com `ZT-W001` + `ZT-S100`.
- `tests/fase7_null.zt`: aceita a parte nullable com warning, mas falha corretamente no caso nao-nullable contido no arquivo.
- `tests/core/test_generic_constraints.zt`: falha corretamente quando `Dog` nao satisfaz `Greetable`.

Verificacao complementar da Trilha B em 2026-04-15:

| Area | Comando | Resultado |
|---|---|---|
| Self-hosted canonico | `lua ztc.lua check src\\compiler\\syntax.zt` | OK com 340 warnings `ZT-W001` |
| Bridge legado | `lua ztc.lua check src\\compiler\\syntax_bridge.zt` | OK; arquivo congelado como stub legado |
| Hacks `string.char` em `syntax.zt` | `rg "string\\.char" src\\compiler\\syntax.zt` | 0 ocorrencias |
| Hacks `string.char` em `syntax_bridge.zt` | `rg "string\\.char" src\\compiler\\syntax_bridge.zt` | 0 ocorrencias |
| `null` em `syntax.zt` | `rg "\\bnull\\b" src\\compiler\\syntax.zt` | 84 linhas; 340 warnings `ZT-W001` |
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

- A trilha self-hosted ainda precisa de paridade.

## Fase 3 - String Interpolation

Status: concluida.

Evidencias:

- Lexer/parser suportam fragmentos interpolados.
- Codegen usa concatenacao segura via runtime quando necessario.

Residuo:

- A base self-hosted ainda possui hacks antigos de caracteres.

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

- `T? = null`: permitido com warning `ZT-W001`.
- `T = null`: erro semantico `ZT-S100`.
- Uso direto de `null` continua desencorajado; a linguagem deve preferir `Optional.Empty`, `Outcome.Failure` ou sentinelas explicitas.

Residuo:

- `BuiltinTypes.NULL` ainda existe por compatibilidade.
- A base self-hosted ainda usa `null` cru em pontos internos.
- A proxima decisao e se `ZT-W001` vira erro duro em uma versao futura.

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
- Padrões qualificados como `Color.Red`.
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

Status: parcial.

Entregue:

- `demo.zt` nao contem `native lua`.
- `demo.zt` compila com `ztc.lua build`.
- Helpers UFCS reduziram a necessidade de escapes no caminho de demonstracao.

Medicao atual:

- `demo.zt`: 0 ocorrencias de `native lua`.
- `src/stdlib`: 38 ocorrencias de `native lua` em arquivos `.zt`.
- `src/compiler`: 15 ocorrencias de `native lua` em arquivos `.zt`.
- Total medido em `src/stdlib src/compiler`: 53 ocorrencias em arquivos `.zt`.

Criterio para concluir a fase inteira:

- Reduzir `native lua` da stdlib essencial.
- Separar interop legitima com Lua de buraco de abstracao.
- Remover hacks de caracteres/tabelas cruas da trilha self-hosted.

## Trilha B - Paridade e Self-Hosting

Estas etapas nao substituem as Fases 1-12. Elas existem porque a base `.zt` do compilador ainda nao tem a mesma maturidade da trilha ativa.

| Item | Objetivo | Status Real | Evidencia |
|---|---|---|---|
| SH-1 | Fazer `ztc check src/compiler/syntax.zt` deixar de falhar massivamente | Concluida | `lua ztc.lua check src\\compiler\\syntax.zt` passa; restam warnings, nao erro massivo |
| SH-2 | Remover hacks de bootstrap como `native lua string.char(...)` | Concluida | `syntax.zt` e `syntax_bridge.zt` tem 0 ocorrencias de `string.char` |
| SH-3 | Alinhar a sintaxe usada pelo compilador self-hosted com a linguagem ativa | Concluida | `syntax.zt` e o caminho canonico; `syntax_bridge.zt` foi congelado explicitamente como legado parseavel |
| SH-4 | Reduzir `null`, tabelas Lua cruas e escapes excessivos | Parcial | A migracao massiva de `null` foi evitada; slots sequenciais foram encapsulados com `has_slot`; `syntax.zt` ainda tem 84 linhas com `null` |
| SH-5 | Definir caminho verificavel de bootstrap confiavel | Concluida | `tools/bootstrap.lua` verifica `syntax.zt`, gera `stage1`, `stage2` e `stage3`, valida load e confirma determinismo em `.selfhost-bootstrap\\summary.txt` |

Leitura consolidada:

- A Trilha B operacional nao esta mais aberta: SH-1, SH-2, SH-3 e SH-5 foram atingidas.
- O bloqueio de bootstrap caiu; o stage2 compila o proprio `syntax.zt` e o stage3 fica identico ao stage2.
- O trabalho remanescente deixou de ser bootstrap e virou saneamento: reduzir `null`, tabelas cruas e escapes restantes sem uma importacao massiva insegura da stdlib.

### SH-1 - `syntax.zt` controlado

Status: concluida.

Evidencias:

- `lua ztc.lua check src\\compiler\\syntax.zt` passa sem erros.
- O arquivo ainda emite muitos `ZT-W001`, mas isso ja nao configura mais falha massiva.

Residuo:

- O warning flood mostra que SH-4 ainda nao terminou.
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

Status: parcial.

Evidencias:

- A superficie `Optional`/`Outcome` foi alinhada com o runtime na trilha ativa.
- O lookup de operadores do lexer usa `TokenKind.BAD` como sentinel interno, removendo `null` dessa familia.
- Guards de listas/tabelas internas foram centralizados em `has_slot`, reduzindo `null` cru sem migracao massiva para `Optional`.
- Slots sequenciais de tabelas Lua em `syntax.zt` foram encapsulados por `has_slot(items, index)`.
- `syntax.zt` ainda possui 84 linhas com `null` e 340 warnings `ZT-W001`.
- `syntax_bridge.zt` nao contribui mais para a metrica de `null` da trilha ativa.
- Lookup de simbolos foi encapsulado com `symbol_is_present`, `symbol_is_missing` e `scope_has_local_symbol`, reduzindo comparacoes diretas contra `null`.
- Plano operacional: `docs/roadmap/null-to-optional.md`.

Residuo:

- A politica de `null` ainda nao subiu de forma consistente para a base self-hosted.
- Boa parte do estado interno do compilador `.zt` ainda depende de sentinelas Lua e tabelas cruas.

### SH-5 - bootstrap verificavel

Status: concluida.

Evidencias:

- `tools/bootstrap.lua` e um verificador nao-destrutivo, orientado a `src/compiler/syntax.zt`.
- O script gera `syntax_stage1.lua`, `syntax_stage2.lua` e `syntax_stage3.lua`, valida `loadfile` em cada etapa e grava resumo em `.selfhost-bootstrap/summary.txt`.
- `stage2` e `stage3` sao identicos, portanto o bootstrap esta deterministico.

Residuo:

- A promocao continua opt-in via `--promote`; a execucao padrao valida sem sobrescrever o compilador ativo.
- O saneamento de `null` permanece separado em SH-4 para evitar uma troca massiva insegura; a superficie `Optional`/`Outcome` ja tem teste direto via `tests/stdlib/test_optional.zt`.
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

- Fase 12

Trilha separada:

- SH-4

Concluida na Trilha B:

- SH-1
- SH-2
- SH-3
- SH-5

## Ordem Recomendada

1. Executar `docs/roadmap/null-to-optional.md`: atacar SH-4 por fatias pequenas, trocando `null` apenas onde a semantica de ausencia estiver clara, sem importacao massiva da stdlib.
2. Reduzir escapes `native lua` remanescentes em `syntax.zt`, separando interop legitima de buraco de abstracao.
3. Limpar referencias documentais remanescentes ao `syntax_bridge.zt`, porque ele ja esta congelado como legado.
4. Continuar hardening de Fase 12 na stdlib essencial, porque ela ainda bloqueia parte da limpeza da Trilha B.
5. Transformar a filosofia de Fase 11 em especificacao curta e depois retomar apenas o que ainda faltar de codegen completo para `match`.

## Resumo Executivo

O compilador ativo tem Fases 1-11 concluidas e testadas. A Fase 12 esta concluida apenas para o demo/caminho de demonstracao, mas ainda nao para a stdlib e para o compilador self-hosted.

Na Trilha B, SH-1, SH-2, SH-3 e SH-5 deixaram de ser problema: `src/compiler/syntax.zt` passa no `check`, `syntax_bridge.zt` foi congelado como legado explicito, os hacks `string.char` foram removidos e o bootstrap self-hosted passa com stage2/stage3 deterministico.

O principal trabalho restante nao e reabrir UFCS, generics, match, indexacao ou bootstrap. O trabalho restante e reduzir dependencia estrutural de `native lua`, sanear `null` por fatias seguras e transformar decisoes ja tomadas em especificacao curta.
