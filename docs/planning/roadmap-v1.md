# Zenith Next Implementation Roadmap

## Objetivo

Registrar o plano historico de implementacao que fechou o ciclo M0-M38.

## Escopo

- Historico tecnico da consolidacao do compilador v2.
- Referencia para auditoria retroativa e rastreabilidade.

## Dependencias

- Upstream:
  - `language/spec/*`
- Downstream:
  - `docs/planning/checklist-v1.md`
  - `docs/planning/cascade-v1.md`
  - `docs/planning/roadmap-v2.md`
- Codigo/Testes relacionados:
  - `tests/conformance/*`
  - `tests/behavior/*`

## Como usar

Use este documento como base historica. Para trabalho atual, priorize `docs/planning/roadmap-v2.md`.

Status em Abril 2026: M0-M16 concluidos para o corte executavel atual do MVP em C. Depois das decisoes 001-056, os specs canonicos foram consolidados em `language/spec/`. O root do repositorio agora aponta para a raiz como caminho oficial, e a linha Lua/self-host anterior foi isolada em `_legacy/`. O compilador C atual ainda pode carregar manifestos do formato anterior em alguns pontos; esse desalinhamento esta registrado como trabalho pos-M16 antes da stdlib final.

## Cascade Navigation

Para sair do milestone abstrato e chegar nos arquivos, simbolos e testes mais provaveis, use `docs/planning/cascade-v1.md` junto com este roadmap.

Ordem recomendada: roadmap -> checklist -> cascade -> code maps -> source -> tests.

Este documento traduz o estado atual do projeto em uma sequencia de marcos
executaveis. A premissa e simples:

- a surface syntax do MVP ja esta suficientemente fechada;
- `stdlib/process` continua fora do primeiro frontend;
- o backend C atual e o primeiro target de plataforma;
- `zenith.ztproj` e a entrada de projeto do MVP;
- `.zt` e a linguagem-fonte de usuario;
- ZIR estruturada e o contrato interno principal;
- ZIR textual permanece apenas para debug, fixtures e golden tests.

## Principios

- implementar por camadas, nao por features isoladas;
- manter o backend C atual compilando enquanto o frontend nasce;
- tratar `language/spec/` como fonte canonica consolidada;
- usar ZIR estruturada como IR principal;
- deixar ZIR textual apenas para debug, fixtures e golden tests.
- todo recurso novo da linguagem precisa de behavior test executavel antes de ser considerado fechado.

## M0. Congelar contratos

Objetivo: parar churn antes de abrir o frontend.

Entregas:

- `language/spec/` confirmado como spec canonica consolidada;
- lista explicita do que esta fora do MVP inicial:
  - `stdlib/process`
  - CLI de processo
  - extras futuros de stdlib
- decisao explicita sobre ZIR:
  - estruturada internamente
  - textual apenas para debug e fixtures

Dependencias: nenhuma.

Pode rodar em paralelo: nao.

## M1. Lexer

Objetivo: transformar `.zt` em uma stream de tokens com spans confiaveis.

Arquivos principais:

- `compiler/frontend/lexer/token.h`
- `compiler/frontend/lexer/token.c`
- `compiler/frontend/lexer/lexer.h`
- `compiler/frontend/lexer/lexer.c`

Escopo minimo:

- keywords
- identificadores
- numeros
- strings
- triple-quoted text
- operadores e delimitadores
- trivia e comentarios
- spans em todos os tokens relevantes

Testes:

- `tests/frontend/test_lexer.c`

Dependencias: `M0`.

Pode rodar em paralelo:

- desenho inicial da AST

## M2. AST e parser sintatico

Objetivo: transformar `.zt` em AST com diagnosticos sintaticos decentes.

Arquivos principais:

- `compiler/frontend/ast/model.h`
- `compiler/frontend/ast/model.c`
- `compiler/frontend/parser/parser.h`
- `compiler/frontend/parser/parser.c`

Escopo minimo:

- `namespace`
- `import`
- `func`
- `struct`
- `trait`
- `apply`
- `enum`
- statements
- expressoes
- `where`
- `match`
- parametros nomeados
- parametros com default

Testes:

- `tests/frontend/test_parser.c`
- fixtures `.zt`

Dependencias: `M1`.

Pode rodar em paralelo:

- AST e parser podem evoluir juntos

## M3. Diagnostics e binder

Objetivo: resolver nomes e escopos antes da tipagem completa.

Arquivos principais:

- `compiler/semantic/diagnostics/diagnostics.h`
- `compiler/semantic/diagnostics/diagnostics.c`
- `compiler/semantic/symbols/symbols.h`
- `compiler/semantic/symbols/symbols.c`
- `compiler/semantic/binder/binder.h`
- `compiler/semantic/binder/binder.c`

Escopo minimo:

- namespaces
- imports
- escopo lexico
- `public`
- shadowing proibido
- nomes duplicados
- prelude implicito de `core.*`

Testes:

- `tests/semantic/test_binder.c`

Dependencias: `M2`.

Pode rodar em paralelo:

- diagnostics e symbols

## M4. Sistema de tipos e validacao semantica

Objetivo: fazer valer as decisoes 001-027 no compilador.

Arquivos principais:

- `compiler/semantic/types/types.h`
- `compiler/semantic/types/types.c`
- `compiler/semantic/types/checker.h`
- `compiler/semantic/types/checker.c`

Escopo minimo:

- `const` vs `var`
- metodos com `!`
- `optional`
- `result`
- generics
- `where`
- traits do `core.*`
- `==` e `!=`
- regras de `map<K, V>`
- conversoes numericas
- overflow inteiro
- named args e defaults

Testes:

- `tests/semantic/test_types.c`
- `tests/semantic/test_constraints.c`

Dependencias: `M3`.

Pode rodar em paralelo:

- validadores especificos por subsistema

## M5. HIR

Objetivo: produzir uma arvore resolvida e tipada, sem acucar sintatico.

Arquivos principais:

- `compiler/hir/nodes/model.h`
- `compiler/hir/nodes/model.c`
- `compiler/hir/lowering/from_ast.h`
- `compiler/hir/lowering/from_ast.c`

Escopo minimo:

- desugar de named args
- desugar de defaults
- normalizacao de `match`
- resolucao de chamadas e metodos
- explicitar mutacao e constraints
- remover ambiguidades sintaticas

Testes:

- `tests/semantic/test_hir_lowering.c`

Dependencias: `M4`.

Pode rodar em paralelo: pouco.

## M6. ZIR v2 estruturada

Objetivo: remover a dependencia de `expr_text` como contrato principal.

Arquivos principais:

- `compiler/zir/model.h`
- `compiler/zir/model.c`
- `compiler/zir/lowering/from_hir.h`
- `compiler/zir/lowering/from_hir.c`

Direcao:

- ZIR interna estruturada
- printer/parser textual continuam existindo
- printer/parser textual deixam de ser o centro da arquitetura

Escopo minimo:

- operandos estruturados
- terminadores estruturados
- tipos canonicos da IR
- spans em tudo

Testes:

- adaptar `tests/zir/*`
- criar `tests/zir/test_lowering.c`

Dependencias: `M5`.

Pode rodar em paralelo:

- printer e verifier, depois que o modelo estabilizar

## M7. Adaptar backend C

Objetivo: religar emitter/legalization a ZIR nova e a linguagem nova.

Arquivos principais:

- `compiler/targets/c/legalization.c`
- `compiler/targets/c/emitter.c`

Escopo minimo:

- remover parse textual de expressoes no backend
- alinhar `optional/result`
- alinhar regras novas da linguagem
- manter runtime C atual como base

Testes:

- manter `tests/targets/c/*` passando
- adicionar casos vindos do lowering real

Dependencias: `M6`.

Pode rodar em paralelo:

- emitter e legalization, com ownership claro de arquivos

## M8. Driver real e zenith.ztproj

Objetivo: compilar um programa Zenith de ponta a ponta.

Arquivos principais:

- `compiler/driver/main.c`
- `compiler/project/ztproj.h`
- `compiler/project/ztproj.c`

Escopo minimo:

- ler `zenith.ztproj`
- resolver source root do manifesto
- localizar entrypoint
- pipeline `.zt -> AST -> semantic -> HIR -> ZIR -> C`

Testes:

- `tests/driver/test_project.c`
- `tests/behavior/*`

Dependencias: `M7`.

Pode rodar em paralelo:

- parser de `zenith.ztproj` pode nascer antes, sem integrar ainda

## M9. Conformance e migracao

Status: concluido para o corte MVP atual.

Objetivo: provar que a trilha nova substitui a atual para o MVP.

Entregas:

- smoke tests end-to-end em `tests/conformance/test_m9.c`
- golden test de C gerado para `tests/behavior/simple_app`
- fixtures de erro sintatico e semantico com spans completos
- comparacao de comportamento observavel: `simple_app` deve sair com codigo `42`
- docs do driver atualizadas para o pipeline real

Resultado do cutover:

- entrada primaria: projeto Zenith com `zenith.ztproj`
- fonte de usuario: `.zt`
- target inicial: C via `gcc`
- entrada secundaria: `.zir` textual apenas para debug/fixture

Dependencias: `M8`.

Pode rodar em paralelo:

- novos behavior tests
- ampliacao dos golden tests

## M10. Controle de fluxo executavel

Objetivo: fazer os blocos de controle ja aceitos pelo frontend chegarem ate C
com comportamento observavel.

Arquivos principais:

- `compiler/zir/lowering/from_hir.c`
- `compiler/zir/model.h`
- `compiler/zir/model.c`
- `compiler/targets/c/emitter.c`
- `compiler/targets/c/legalization.c`

Escopo minimo:

- baixar `while` para ZIR/C
- baixar `repeat N times` para ZIR/C
- baixar `match` para ZIR/C
- baixar `break` e `continue`
- decidir e implementar `for item in collection` para o subset MVP
- decidir e implementar `for key, value in map` para o subset MVP
- preservar spans em blocos gerados
- rejeitar controle de fluxo ainda nao suportado com diagnostico claro

Testes:

- `tests/behavior/control_flow_while/`
- `tests/behavior/control_flow_repeat/`
- `tests/behavior/control_flow_match/`
- `tests/behavior/control_flow_break_continue/`
- `tests/behavior/control_flow_for_list/`
- `tests/behavior/control_flow_for_map/`
- `tests/conformance/test_m10.c`

Dependencias: `M9`.

Pode rodar em paralelo:

- casos de behavior e golden C podem ser preparados enquanto o lowering evolui

## M11. Funcoes, chamadas e entrypoints completos

Objetivo: tornar chamadas de funcoes de usuario confiaveis no caminho real
`.zt -> C`.

Arquivos principais:

- `compiler/semantic/types/checker.c`
- `compiler/hir/lowering/from_ast.c`
- `compiler/zir/lowering/from_hir.c`
- `compiler/targets/c/emitter.c`
- `compiler/driver/main.c`

Escopo minimo:

- chamadas diretas entre funcoes de usuario
- parametros posicionais
- parametros nomeados
- parametros com default
- validacao final de assinatura de `main`
- retorno `int`, `bool`, `float`, `text`, `void`
- recursao simples, se nao exigir mudanca estrutural grande
- diagnosticos para chamada invalida com spans

Testes:

- `tests/behavior/functions_calls/`
- `tests/behavior/functions_named_args/`
- `tests/behavior/functions_defaults/`
- `tests/behavior/functions_main_signature_error/`

Dependencias: `M10`.

Pode rodar em paralelo:

- diagnosticos negativos e golden C

## M12. Structs, campos, metodos e apply

Objetivo: fazer composicao por `struct`, `trait` e `apply` funcionar
ponta a ponta no backend C.

Arquivos principais:

- `compiler/semantic/binder/binder.c`
- `compiler/semantic/types/checker.c`
- `compiler/hir/lowering/from_ast.c`
- `compiler/zir/lowering/from_hir.c`
- `compiler/targets/c/emitter.c`

Escopo minimo:

- construtor de struct via `Type(field: value)`
- defaults de campos
- leitura de campo
- atribuicao de campo em `var`
- metodos inerentes via `apply to Type`
- metodos de trait via `apply Trait to Type`
- receiver explicito no IR
- metodos mutantes com `!`
- diagnosticos para mutacao invalida
- layout C estavel para structs do usuario

Testes:

- `tests/behavior/structs_constructor/`
- `tests/behavior/structs_field_update/`
- `tests/behavior/methods_inherent/`
- `tests/behavior/methods_trait_apply/`
- `tests/behavior/methods_mutating/`

Dependencias: `M11`.

Pode rodar em paralelo:

- emitter C de structs e validacao semantica de mutacao

## M13. Collections, optional/result e ownership no runtime C

Objetivo: fechar os tipos compostos centrais do MVP no caminho real
`.zt -> C`.

Arquivos principais:

- `runtime/c/zenith_rt.h`
- `runtime/c/zenith_rt.c`
- `compiler/semantic/types/checker.c`
- `compiler/zir/lowering/from_hir.c`
- `compiler/targets/c/emitter.c`

Escopo minimo:

- `list<T>` literal
- indexacao de lista
- slice de lista
- atualizacao de lista
- tamanho de lista
- `map<K, V>` literal
- indexacao de map
- atualizacao de map
- tamanho de map
- `optional<T>` com `none`
- `result<T, E>` com `success` e `error`
- propagacao/unwrap do subset MVP, se ja definido na surface
- regras de ownership/release no C gerado
- diagnosticos para uso invalido de collection/result

Status atual:

- Fechado no pipeline real: literais `list<int>`, `list<text>`, `map<text,text>`, indexacao 0-based, atualizacao por indice/chave, slice de texto/lista, `len(...)` para texto/list/map suportados, `none`, `success` e `error`.
- Fechado para o subset MVP: propagacao `?` de `result<T,E>` em inicializacao `const`/`var` dentro de funcoes que retornam `result<T,E>`.
- Ainda aberto fora do corte executavel: `?` de `optional`, unwrap em posicoes arbitrarias de expressao, combinacoes genericas alem do subset C atual e checagens runtime geradas a partir de `where`.

Testes:

- `tests/behavior/list_basic/`
- `tests/behavior/list_text_basic/`
- `tests/behavior/map_basic/`
- `tests/behavior/optional_result_basic/`
- `tests/behavior/list_slice_len/`
- `tests/behavior/text_slice_len/`
- `tests/behavior/map_len_basic/`
- `tests/behavior/result_question_basic/`
- `tests/conformance/test_m13.c`
- `tests/runtime/c/test_runtime.c`

Dependencias: `M12`.

Pode rodar em paralelo:

- runtime C e lowering ZIR dos tipos compostos

## M14. Projeto multi-arquivo, namespaces e imports reais

Objetivo: sair do entrypoint unico e compilar projetos Zenith com multiplos
arquivos dentro do source root.

Arquivos principais:

- `compiler/project/ztproj.c`
- `compiler/driver/main.c`
- `compiler/semantic/binder/binder.c`
- `compiler/hir/lowering/from_ast.c`

Escopo minimo:

- varrer source root
- carregar todos os `.zt`
- validar consistencia entre path e `namespace`
- resolver imports qualificados
- resolver imports com alias
- rejeitar imports inexistentes
- rejeitar ciclos quando forem invalidos para o MVP
- agregar ASTs em um programa semanticamente unico
- baixar multiplos modulos para HIR/ZIR
- emitir C com nomes legalizados sem colisao
- manter entrypoint de projeto no manifesto legado

Status atual:

- Fechado no driver: varredura recursiva de `source.root`, validacao path/namespace, rejeicao de import ausente, rejeicao de ciclo de import, AST agregado, chamadas qualificadas por alias e entrypoint canonico via `app.entry`.
- O corte atual agrega os arquivos em um unico programa HIR/ZIR e usa o alias do import como prefixo interno para evitar colisao no C emitido.
- Regra MVP: ciclos de import sao invalidos e geram diagnostico de projeto antes do lowering.

Testes:

- `tests/behavior/multifile_import_alias/`
- `tests/behavior/multifile_missing_import/`
- `tests/behavior/multifile_namespace_mismatch/`
- `tests/conformance/test_m14.c`
- `tests/behavior/multifile_duplicate_symbol/`
- `tests/behavior/multifile_import_cycle/`

Dependencias: `M13`.

Pode rodar em paralelo:

- loader de projeto e binder multi-arquivo

## M15. Fechamento semantico da surface syntax MVP

Objetivo: alinhar parser, binder, typechecker e HIR com a spec de surface
sem deixar recursos aceitos pelo parser sem contrato semantico.

Status atual:

- Concluido para o corte executavel atual.
- A cobertura aceita pelo parser e a cobertura executavel no backend C estao registradas em `language/surface-implementation-status.md`.
- As formas aceitas mas nao executaveis ainda possuem status explicito e nao entram na matriz M16 como comportamento fechado.

Arquivos principais:

- `language/spec/surface-syntax.md`
- `compiler/frontend/parser/parser.c`
- `compiler/semantic/binder/binder.c`
- `compiler/semantic/types/checker.c`
- `compiler/hir/lowering/from_ast.c`

Escopo minimo:

- revisar tudo que o parser aceita
- marcar explicitamente o que fica fora do MVP executavel
- fechar generics do MVP
- fechar constraints `where`
- fechar invariantes/refinements possiveis no primeiro backend
- fechar traits do `core.*` no typechecker
- fechar regras de igualdade e hash
- fechar conversoes numericas explicitas
- fechar overflow e unsigned conforme spec
- melhorar mensagens de erro de casos frequentes
- garantir spans em diagnostics de parser/semantic/lowering

Testes:

- fixtures happy path de parser
- fixtures de erro sintatico
- fixtures de erro semantico
- behavior tests para cada item executavel
- golden diagnostics para erros essenciais

Dependencias: `M14`.

Pode rodar em paralelo:

- ampliacao de fixtures e melhorias de diagnostics

## M16. Conformance final frontend/backend

Objetivo: declarar a cobertura frontend/backend da linguagem MVP como completa
para o target C.

Status atual:

- Concluido para o target C no corte executavel atual.
- A matriz de behavior esta em `tests/behavior/MATRIX.md`.
- O harness agregador `tests/conformance/test_m16.c` compila todos os projetos validos, executa os binarios, valida diagnosticos principais e confere golden C/fragments representativos.
- Os harnesses de conformance (`M9`..`M16`) executam builds em sandbox temporario dentro de `.ztc-tmp` com `output = "build_tmp"`, evitando conflito/lock de executavel no Windows.
- Pendencias de stdlib, CLI final e ZPM estao listadas fora do bloqueio frontend/backend em `language/surface-implementation-status.md`.

Arquivos principais:

- `tests/conformance/`
- `tests/behavior/`
- `compiler/driver/main.c`
- `docs/planning/checklist-v1.md`
- `docs/planning/roadmap-v1.md`

Escopo minimo:

- matriz de behavior tests por feature da surface syntax MVP
- golden C para features representativas, nao para cada detalhe trivial
- golden diagnostics para erros principais
- build end-to-end de todos os projetos validos
- execucao dos binarios e comparacao de saida/codigo observavel
- confirmar que backend C nao depende de parse textual de ZIR
- confirmar que `.zir` textual segue apenas como debug/fixture
- atualizar docs para declarar o corte final frontend/backend
- listar explicitamente o que fica para stdlib/CLI final/ZPM

Testes:

- `tests/conformance/test_m16.c`
- todos os harnesses existentes
- behavior matrix completa

Dependencias: `M15`.

Pode rodar em paralelo:

- golden diagnostics e behavior tests por feature

## M17. Consolidacao dos specs canonicos

Status: concluido como documentacao normativa.

Objetivo: transformar as decisoes aceitas em specs canonicos de leitura e implementacao.

Entregas:

- `language/spec/README.md`
- `language/spec/surface-syntax.md`
- `language/spec/project-model.md`
- `language/spec/compiler-model.md`
- `language/spec/tooling-model.md`
- `language/surface-spec.md` reduzido a ponteiro de compatibilidade
- `language/README.md` atualizado para apontar para os specs consolidados

Dependencias: decisoes 001-056.

Pode rodar em paralelo: nao, pois consolida a base para as proximas fases.

## M18. Alinhar implementacao ao manifesto e tooling final

Status: concluido para o manifesto canonico e o driver bootstrap atual.

Objetivo: alinhar o driver/projeto atual ao spec consolidado de `zenith.ztproj` e CLI.

Status atual:

- Fechado no parser/driver bootstrap: `[project]`, `[source]`, `[app]`, `[lib]`, `[build]`, `[test]` e `[zdoc]`.
- `project.kind = "app"` exige `app.entry`; `project.kind = "lib"` exige `lib.root_namespace`.
- Behavior fixtures foram migradas para `source.root`, `app.entry` e `build.target = "native"`.
- Diagnostics de projeto agora falam em `source.root` na superficie de usuario.
- O driver bootstrap ainda se chama `zt-next`; a CLI canonica `zt` permanece especificada em `language/spec/tooling-model.md`.
- O root do repositorio foi cortado para Zenith Next: README raiz aponta para `` e a implementacao Lua/self-host anterior foi arquivada em `_legacy/`, conforme Decision 076.

Escopo minimo:

- trocar o parser de manifesto do formato legado para `[project]`, `[source]`, `[app]`, `[lib]`, `[build]`, `[test]` e `[zdoc]`
- substituir `source_root` legado por `source.root` na superficie de usuario e diagnostics
- substituir entrypoint antigo por `app.entry` para projetos `kind = "app"`
- validar `project.kind = "app"` e `project.kind = "lib"`
- validar `lib.root_namespace` em projetos lib
- atualizar fixtures `zenith.ztproj` dos behavior tests
- atualizar docs do driver para diferenciar CLI bootstrap atual de CLI canonica `zt`
- manter diagnostics claros para manifestos invalidos

Dependencias: M17.

Pode rodar em paralelo:

- preparacao de fixtures
- atualizacao de docs do driver

## M19. Stdlib MVP

Status: em andamento (subset bytes/UTF-8 do M19 concluido).

Objetivo: definir e implementar a primeira stdlib real sobre o runtime C e a surface consolidada.

Escopo inicial sugerido para discussao antes de implementacao:

- `std.io` (Foco estrito em Terminal. O `print(text)` exige string limpa, sem conversÃƒÆ’Ã‚Â£o mÃƒÆ’Ã‚Â¡gica)
- `std.bytes`
- `std.fs`
- `std.fs.path`
- `std.json`
- `std.math` (Inclui vetores simples de ÃƒÆ’Ã‚Âlgebra Linear para Games)
- `std.collections` (Queue, PriorityQueue, Grid2D/3D, BTree baseadas)
- `std.random` (Algoritmos como Xoshiro/PCG para simulaÃƒÆ’Ã‚Â§ÃƒÆ’Ã‚Âµes/jogos)
- `std.text`
- `std.validate`
- `std.time`
- `std.format`
- `std.os`
- `std.os.process`
- `std.test`

Dependencias: M17; idealmente M18 para evitar retrabalho em manifestos e comandos.

Progresso implementado: o literal `hex bytes "..."` ja passa por parser contextual, typechecker, HIR/ZIR, emissor C e runtime `zt_bytes`, incluindo `len(bytes)`, indexacao e slicing no subset C behavior-tested. Tambem estao implementados `std.bytes.empty`, `std.bytes.from_list`, `std.bytes.to_list`, `std.bytes.join`, `std.bytes.starts_with`, `std.bytes.ends_with`, `std.bytes.contains`, `std.text.to_utf8` e `std.text.from_utf8` com erro tipado para UTF-8 invalido, cobertura em `tests/runtime/c/test_runtime.c` e fixtures de comportamento `tests/behavior/std_bytes_utf8` e `tests/behavior/std_bytes_ops` (no subset C atual, `from_list`/`to_list` usam `list<int>`).

Observacao: Decision 079 define que a `std.bytes` e as conversÃƒÆ’Ã‚Âµes UTF-8 reais do `M19` devem focar exclusivamente no subset vital para validar os behavior tests atravÃƒÆ’Ã‚Â©s do backend C com ARC nÃƒÆ’Ã‚Â£o-atÃƒÆ’Ã‚Â´mico. OmissÃƒÆ’Ã‚Âµes de alta complexidade devem fluir para as fases posteriores.

Observacao: Decision 073 define que nao ha `std.stream` publico no MVP. APIs de IO, FS, processo e rede devem comecar com tipos concretos de seus proprios modulos.

Observacao: Decision 074 define que `std.net` e sincrono/blocking no MVP, com `timeout` explicito em `net.connect(...)` e erro recuperavel `net.Error.Timeout`.

Observacao: Decision 075 define o primeiro modelo de `std.net`: TCP cliente, `host: text`, `port: int`, `net.Connection`, `read_some!`, `write_all!`, sem UDP, listener ou `net.Address` publico no MVP.

## M20. ZDoc e ZPM tooling inicial

Status: concluido para o corte inicial bootstrap.

Objetivo: implementar validacao inicial de ZDoc e o esqueleto local de ZPM sem registry remoto.

Escopo inicial:

- `zt doc check` para `.zdoc` pareado e `zdoc/guides/`
- diagnosticos de `@target`, `@page`, `@param`, `@return` e `@link`
- leitura conceitual de `[dependencies]` e `[dev_dependencies]`
- criacao futura de `zenith.lock`
- comandos `zpm add/update/publish` apenas depois do resolvedor local

Dependencias: M18 e parte da stdlib/tooling.
Progresso implementado neste corte:

- modulo inicial `compiler/project/zdoc.{h,c}` com scanner/checker de `.zdoc`
- validacao de `@target` para docs pareados
- validacao de `@page` para `zdoc/guides/`
- warnings `doc.unresolved_link` para `@link` nao resolvido
- comando bootstrap `doc-check <project|zenith.ztproj>` no driver atual
- parser de manifesto agora le e preserva entradas de `[dependencies]` e `[dev_dependencies]`
- schema inicial documentado em `language/spec/lockfile-schema.md`

## M21. Fechamento dos specs finais

Status: concluido.

Objetivo: eliminar ambiguidades entre decisions historicas, specs canonicos e status real de implementacao.

Arquivos principais:

- `language/spec/surface-syntax.md`
- `language/spec/stdlib-model.md`
- `language/spec/runtime-model.md`
- `language/spec/backend-scalability-risk-model.md`
- `language/spec/diagnostics-model.md`
- `language/spec/formatter-model.md`
- `language/spec/implementation-status.md`
- `LANGUAGE_COHERENCE_CLOSURE.md`

Escopo minimo:

- consolidar `void`, `where` e `const` collections sem ambiguidades
- tratar decisions antigas como historico quando conflitarem com specs novos
- criar spec final para stdlib
- criar spec final para runtime
- criar spec final para riscos de escalabilidade do backend
- criar spec final para diagnostics
- criar spec final para formatter
- criar spec final para tracking de implementation status

Dependencias: M18.

## M22. Formatter obrigatorio

Status: concluido.

Objetivo: implementar `zt fmt` como parte central da filosofia reading-first.

Escopo minimo:

- formatar namespace/imports
- formatar attrs
- formatar funcoes e named args
- formatar `where` em campos e parametros
- formatar structs, enums e match
- preservar comentarios
- garantir idempotencia
- criar golden tests de formatter

Dependencias: M21.

## M23. Diagnostics renderer real

Status: concluido.

Objetivo: transformar o modelo das Decisions 038/039 e `diagnostics-model.md` em renderer real.

Status atual:

- renderer detalhado ativo no driver para parser e `zt_diag_list` (binding/type/HIR/ZIR)
- header canonico `severity[stable.code]` e secoes `where`/`code`/`help` em uso no fluxo principal
- catalogo inicial de codes estaveis registrado em `language/spec/diagnostic-code-catalog.md`
- fixtures de syntax/type ajustadas ao novo formato e validadas no `test_m16`
- parser com recuperacao por sincronizacao em fronteiras de declaracao/corpo e `expect` garantindo progresso ao consumir token inesperado
- ZIR parse/verify e backend C emit migrados para o renderer unificado (`zt_diag_list`)
- erros de projeto no caminho principal (path/source root/agregacao) migrados para o renderer detalhado
- runtime panic/index/assert/check migrado para formato estruturado com `error[runtime.*]`, `where`, `code` e `help`
- lista de diagnosticos com deduplicacao recente e limite total para conter cascatas parser/semantic

Escopo minimo:

- unificar estrutura de diagnosticos entre stages
- implementar renderer terminal detalhado
- adicionar codes estaveis
- adicionar help acionavel para erros frequentes
- evitar cascata de erros
- criar golden diagnostics por stage

Dependencias: M21.

## M24. Runtime C ownership and backend scalability hardening

Status: concluido para o corte inicial bootstrap.

Pendencias pos-MVP deste tema foram extraidas para `M35` (Concurrency/FFI/Dyn Dispatch), mantendo M24 fechado para o corte bootstrap.



Objetivo: tornar RC, COW, temporarios, cleanup e custos do backend C confiaveis para value semantics e programas maiores.

Escopo minimo:

- auditar `zt_retain` e `zt_release`
- documentar RC sem cycle collection como risco MVP
- definir gate para `weak<T>`, handles/arenas, ownership graph ou cycle collection antes de callbacks/UI/games oficiais
- definir cleanup em retorno normal
- definir cleanup em propagacao `?`
- definir cleanup de temporarios
- definir comportamento em panic fatal
- cobrir text, bytes, list, map, optional, result, structs e enums
- trocar wrappers `optional<T>` e `result<T,E>` para stack/in-place quando pratico
- garantir `result<void,E>` success sem heap allocation no modelo final
- progresso: runtime C atual ja usa singleton imortal para `result<void,E>.success` (sem heap por chamada)
- progresso: guards de bounds em text/bytes/list/map ja emitem `ZT_ERR_INDEX` no runtime C.
- progresso: behavior `tests/behavior/runtime_index_error` valida o caminho de erro sem segfault.
- progresso: runtime C expoe `zt_list_i64_set_owned`, `zt_list_text_set_owned` e `zt_map_text_text_set_owned` com detach-on-write em `rc > 1`.
- progresso: emitter C gera rebind automatico (`x = zt_*_set_owned(...)`) em mutacoes de list/map com alvo simbolo.
- progresso: runtime de `map<text,text>` estabilizado no subset MVP sem segfault em insercao/get/iteracao (fallback seguro por busca linear).
- progresso: emissao de `eq/ne` para `outcome<text,text>` corrigida no backend C usando helper semantico (`zt_outcome_text_text_eq`).
- definir politica de heap apenas para payload managed, escape, tamanho excessivo ou bootstrap debt
- implementar canonical type keys para monomorfizacao
- implementar cache/deduplicacao de instancias monomorfizadas
- criar diagnostico ou build report para explosao de monomorfizacao
- progresso: gate de monomorfizacao ativo no driver via `build.monomorphization_limit`, com diagnosticos `project.invalid_monomorphization_limit` e `project.monomorphization_limit_exceeded`.

Dependencias: M21; recomendado antes de expandir stdlib pesada.

## M25. Value semantics conformance

Status: concluido para o corte inicial bootstrap.

Pendencia bloqueada herdada de M24 foi extraida para `M35`: behavior de colecao heterogenea por iterador `dyn Trait` (junto da implementacao de dispatch dinamico no backend C).


Objetivo: provar em behavior tests que copia semantica nao vaza mutacao compartilhada.

Escopo minimo:

- copy/mutate de `list<T>`
- copy/mutate de `map<K,V>`
- copy/mutate de structs com campos managed
- optional/result com payload managed
- enums com payload managed quando M27 existir
- `const` collections rejeitam mutacao observavel
- progresso: behavior `tests/behavior/value_semantics_collections` cobre copy/mutate de list/map sem vazamento de mutacao.
- progresso: behavior `tests/behavior/value_semantics_struct_managed` cobre copy/mutate de struct com campos managed (`list<int>` e `map<text,text>`).
- progresso: behavior `tests/behavior/value_semantics_arc_isolation` cobre cadeia de copias (`a -> b -> c`) para validar detach sob COW/RC.
- progresso: behavior `tests/behavior/value_semantics_optional_result_managed` cobre `optional<list<int>>` e `result<list<int>, text>` com payload managed no fluxo executavel.
- progresso: cobertura integrada em `tests/conformance/test_m13.c` e `tests/conformance/test_m16.c`.
- progresso: suite semantica cobre `const` collection com mutacao indexada rejeitada em `tests/semantic/test_types.c` (`ZT_DIAG_INVALID_MUTATION`).
- progresso: runtime C cobre copy/mutate de payload managed em `optional<list<int>>` e `result<list<int>, text>` (`tests/runtime/c/test_runtime.c`).

Dependencias: M24.

## M26. Runtime where contracts completos

Status: concluido para o corte bootstrap atual.

Objetivo: executar contratos de runtime em todos os pontos MVP aceitos.

Escopo minimo:

- checar field `where` em construcao
- checar field `where` em atribuicao de campo
- checar parameter `where` em chamadas
- garantir que construtores com `where` nao mudam secretamente para `result<T,E>`
- documentar e testar padrao explicito `try_create_*` para validacao recuperavel
- reportar `runtime.contract`
- incluir predicate e span quando disponivel
- incluir valor quando seguro
- progresso: HIR->ZIR agora preserva `where_clause` em params e fields.
- progresso: lowering injeta `check_contract` em entrada de funcao (parametros), construcao de struct e atribuicao de campo.
- progresso: emitter C injeta mensagem legivel de predicate e, para tipos primitivos seguros (`int`, `float`, `bool`), inclui valor observado no erro via helpers dedicados do runtime C.
- progresso: verifier ZIR cobre `ZIR_INSTR_CHECK_CONTRACT`.
- progresso: behavior fixtures `where_contracts_ok`, `where_contract_param_error`, `where_contract_construct_error` e `where_contract_field_assign_error` integrados ao conformance `test_m16`.

Dependencias: M23 e M24.

## M27. Enums com payload e match forte

Status: concluido no source e no pipeline E2E (binder/typechecker/HIR + lowering ZIR + backend C + behavior checks).

Objetivo: fechar modelagem de estados sem POO classica.

Escopo minimo:

- layout de enum com tag e payload em HIR/ZIR/C
- construtores qualificados
- match por case de enum
- binding de payload
- case default
- diagnosticos de match invalido
- match sem `case default` deve ser exaustivo quando o enum e conhecido
- missing enum case deve gerar diagnostico compile-time
- `case default` deve ser documentado como opt-out de exaustividade futura
- progresso: parser/AST e HIR agora preservam declaracoes `enum` com variantes e payload nomeado/posicional.
- progresso: lowering ZIR de enum payload implementado e validado por teste dedicado (`tests/zir/test_enum_lowering.c`).
- progresso: emissao C de layout tagged union implementada e validada por teste dedicado (`tests/targets/c/test_enum_payload.c`).
- progresso: cobertura semantica de enum/match reforcada em `tests/semantic/test_types.c` e binder em `tests/semantic/test_binder.c`.
- fechado no source: construtores qualificados, match por case de enum com binding de payload, `case default` validado (duplicado/fora da posicao final), e exaustividade em enum conhecido sem default via `ZT_DIAG_NON_EXHAUSTIVE_MATCH`.
- progresso: `compiler/zir/lowering/from_hir.c` saiu do stub e voltou a baixar `enum`/`match` (incluindo payload binding) no caminho real de build; `tests/behavior/enum_match` voltou a validar em `zt check`.

Dependencias: M24.

## M28. Bytes, UTF-8 e stdlib base

Status: em andamento (subset bytes/UTF-8 e operacoes base de std.bytes concluido).

Objetivo: fechar text/binary antes de filesystem, process, JSON e network crescerem.

Escopo minimo:

- completar `std.bytes`
- completar `std.text.to_utf8`
- completar `std.text.from_utf8`
- retornar erro tipado para UTF-8 invalido
- behavior tests para bytes/text
- iniciar implementacao minima dos modulos stdlib base

Dependencias: M19 e M24.

## M29. API segura de collections

Status: concluido para o corte inicial bootstrap.

Objetivo: manter indexacao estrita sem incentivar panic como fluxo normal.

Escopo minimo:

- definir namespace final das APIs seguras
- implementar lookup seguro de map retornando `optional<V>`
- implementar acesso seguro de list se aceito
- atualizar exemplos oficiais para usar API segura quando ausencia for esperada
- manter `map[key]` como acesso estrito/assertivo

Progresso implementado neste corte:

- `map.get(key)` foi fechado no subset atual para `map<text,text>`, com retorno `optional<text>` no typechecker/HIR/ZIR/backend C.
- `list.get(index)` foi fechado no subset atual para `list<int>` e `list<text>`, com retorno `optional<T>` no typechecker/HIR/ZIR/backend C.
- runtime C ganhou `zt_map_text_text_get_optional(...)`, preservando `map[key]` como acesso estrito com panic em chave ausente.
- runtime C ganhou `zt_list_i64_get_optional(...)` e `zt_list_text_get_optional(...)`, preservando indexacao direta como acesso estrito com panic em indice ausente.
- behavior `tests/behavior/map_safe_get` cobre chave presente (unwrap com `?`) e chave ausente sem panic.
- behavior `tests/behavior/list_safe_get` cobre indice presente e indice ausente sem panic.

Dependencias: M13 e M21.

## M30. CLI final

Status: concluido para o corte bootstrap atual.

Objetivo: entregar tooling previsivel para usuario final.

Comandos obrigatorios:

- `zt check`
- `zt build`
- `zt run`
- `zt test`
- `zt fmt`
- `zt doc check`

Progresso implementado neste corte:

- parser de CLI aceita `doc check` como forma canonica (alem de `doc-check` legado);
- aliases canonicos ativos: `check -> verify`, `run -> build --run`;
- `zt test` implementado como alias bootstrap para `zt check` ate o runner dedicado por `attr test`;
- `zt fmt` implementado como gate bootstrap (`fmt` e `fmt --check`) ate o formatter deterministico de `M22`;
- descoberta automatica de projeto ativa quando `input` e omitido (busca ascendente de `zenith.ztproj` a partir do diretório atual).
- `zt run` no bootstrap atual preserva o exit code observavel do programa executado.
- renderer de diagnostics unificado aplicado tambem ao fluxo de `zt doc check`.
- modo `--ci` adicionado para saida curta/deterministica em pipelines.

Dependencias: M22, M23 e M28.

## M31. ZDoc funcional

Status: concluido.

Objetivo: fazer a separacao entre codigo e documentacao longa ser util na pratica.

Escopo minimo:

- parser inicial de `.zdoc`
- validacao de `@target`
- validacao de `@page`
- warning de `@link` nao resolvido
- warning de public API sem documentacao
- suporte a docs privadas excluidas do output publico por default

Progresso implementado neste corte:

- parser/checker inicial de `.zdoc` implementado;
- validacoes `@target`, `@page` e `@link` implementadas;
- warning `doc.unresolved_link` implementado;
- warning `doc.missing_public_doc` implementado para simbolos publicos sem bloco ZDoc;
- resolucao de `@target` em docs pareadas restrita ao arquivo-fonte pareado;
- links globais em docs/guides restritos a simbolos publicos;
- docs privadas suportadas em arquivos pareados sem warning por ausencia;
- `zt doc check` integrado ao driver com renderer de diagnostics unificado.

Dependencias: M23 e M30.

## M32. Matriz de conformidade final

Status: concluido para o corte inicial bootstrap.

Objetivo: transformar "feito" em um criterio testavel e nao subjetivo.

Escopo minimo:

- matriz por feature com status de parser, semantic, lowering, backend, runtime, diagnostics e formatter
- matriz de riscos usando o label `Risk` do implementation status
- conformance para specs finais
- conformance para stdlib base
- conformance para CLI
- conformance para ZDoc
- cobertura explicita de RC cycles, monomorfizacao, heap/stack wrappers, where runtime e match exaustivo
- suite final verde antes de declarar release MVP estavel

Dependencias: M21-M31.

## M33. Implementacao das Stdlibs MVP

Status: concluido para o corte MVP atual (modulos MVP implementados, ZDoc fisico presente e hardening basico de ownership ARC em collections validado por behavior tests E2E).

Objetivo: Fornecer os modulos da biblioteca padrao (alem da base de bytes/utf-8 iniciada em M19), fundamentados num backend e runtime estaveis. As decisoes de design destas APIs ja estao fechadas e documentadas em `language/decisions/`.

Escopo minimo:

- Implementar `std.io` (Terminal, print explicito sem conversao magica)
- Implementar `std.fs` e `std.fs.path`
- Implementar `std.json` (Parser/Emitter basico)
- Implementar `std.math` (Vetores e algebra linear basica)
- Implementar `std.collections` (Queue, Grid2D, etc.)
- Implementar `std.random` (Xoshiro/PCG)
- Implementar `std.validate`
- Implementar `std.time` (Instant, Duration)
- Implementar `std.format`
- Implementar `std.os` e `std.os.process`
- Implementar `std.test` (Harness para `@test`)
- Implementar `std.net` (TCP Client, Multi-IP DNS)
- Criar behavior tests para cada modulo
- Validar ownership ARC em cada implementacao
- progresso: `std.io` cobre output textual explicito (`write`/`print` e variants de linha) e leitura de stdin (`read_line`/`read_all`) no backend C atual, com `result<optional<text>, text>`/`result<text, text>` e suporte de `outcome<optional<text>,text>` no emitter/runtime.
- pendente: migrar `std.io` para os tipos canonicos de stream/erro (`io.Input`/`io.Output`/`io.Error`) conforme decision 058.
- progresso: `std.fs` recebeu baseline inicial (`read_text`, `write_text`, `exists`) com wrappers host (`zt_host_read_file`/`zt_host_write_file`/`zt_host_path_exists`) e behavior `std_fs_basic`.
- progresso: `std.fs.path` recebeu baseline inicial (`join`) com behavior `std_fs_path_basic`; operacoes adicionais (`base`, `dir`, `ext`, `normalize`, `absolute`, `relative`, `is_file`, `is_dir`, `list`, `create_dir`, metadados) seguem pendentes.
- progresso: std.json recebeu baseline inicial (parse, stringify, pretty) para map<text,text>, com behavior std_json_basic; no MVP atual o parser aceita objeto JSON plano e valores escalares (sem nested/object-array e sem \\uXXXX).
- progresso: emitter C passou a cobrir Outcome<map<text,text>,text> em outcome_is_success/outcome_value/try_propagate, incluindo propagacao T? para retorno result<void,text> quando T = map<text,text> (e demais outcomes ja materializados).
- progresso: `std.validate` recebeu baseline puro (predicados int/text) com behavior `std_validate_basic`.
- progresso: `std.math` recebeu baseline inicial (operacoes escalares int + helpers angulares/`approx_equal`) com behavior `std_math_basic`; funcoes avancadas continuam pendentes.
- progresso: `std.format` recebeu baseline inicial (`hex`, `bin`, `bytes` [binary default], `bytes_decimal`) com behavior `std_format_basic`; seletor tipado (`BytesStyle`) e familias `number/percent/date/datetime` seguem pendentes.
- progresso: `std.test` agora possui harness real no fluxo atual de `zt test`, incluindo descoberta de `attr test`, runner temporario, estados de pass/skip/fail e cobertura em `std_test_basic`, `std_test_attr_pass_skip` e `std_test_attr_fail`.
- progresso: `std.time` recebeu baseline inicial com wrappers de host runtime no target C (`now`, `sleep`, `since`, `until`, `diff`, `add`, `sub`, conversoes unix e helpers de duracao), com behavior `std_time_basic`; tipagem canonica `Instant`/`Duration` permanece pendente neste corte e esta representada temporariamente como `int` unix-ms.
- progresso: `std.os` recebeu baseline inicial (`pid`, `platform`, `arch`, `env`, `current_dir`, `change_dir`) via wrappers host runtime, com behavior `std_os_basic`; tipos canonicos (`os.Platform`, `os.Arch`, `os.Error`) seguem pendentes.
- progresso: `std.os.process` recebeu baseline inicial com `run(program, args, cwd)` sem shell implicito e retorno de status em `int`, com behavior `std_os_process_basic`; tipos canonicos (`process.ExitStatus`, `process.Error`, `process.CapturedRun`) seguem pendentes.
- progresso: `std.net` recebeu baseline inicial com TCP client blocking de timeout explicito em `int` ms (`connect`, `read_some`, `write_all`, `close`, `is_closed`) e classificacao `net.kind(core.Error)`, apoiado pelo runtime C ja existente; o fixture `std_net_basic` agora possui harness local de loopback (`run-loopback.ps1`) para validar o fluxo E2E no host atual.
- progresso: `std.collections` agora possui behavior dedicado `std_collections_managed_arc`, cobrindo copy/mutate isolation em `grid2d<text>`, `pqueue<text>`, `circbuf<text>`, `btreemap<text,text>`, `btreeset<text>` e `grid3d<text>` via `zt verify`, `zt build` e `zt run`.

Dependencias: M24 (ARC nao-atomico para buffers e resources) e M26 (Runtime where contracts para `std.validate`).

## M34. Cognitive Accessibility by Design

Status: iniciado (spec consolidada em `language/spec/cognitive-accessibility.md`).

Objetivo: transformar diretrizes de acessibilidade cognitiva em comportamento observavel no toolchain.

Escopo minimo:

- manter `language/spec/cognitive-accessibility.md` como referencia canonica de acessibilidade cognitiva
- manter fontes de pesquisa e nivel de evidencia atualizados dentro do spec
- implementar perfis de diagnostico (`beginner`, `balanced`, `full`) no fluxo principal de `zt check` e `zt build`
- implementar formato de diagnostico action-first (`ACTION`, `WHY`, `NEXT`)
- implementar hints de esforco opcionais (`quick fix`, `moderate`, `requires thinking`)
- implementar checks de nomes confundiveis com sugestao ativa para `name.unresolved`
- implementar retomada de contexto com `zt summary`/`zt resume`
- implementar reducao de ruido com `zt check --focus` e `zt check --since`
- definir e medir metricas de impacto com coleta opt-in

Dependencias: M23 e M30.

Pode rodar em paralelo:

- M33 (stdlibs)
- M32 (matriz de conformidade final)



## M35. Concurrency/FFI/Dyn Dispatch

Status: planejado (post-MVP).

Objetivo: fechar recursos de concorrencia, interoperabilidade FFI e dispatch dinamico que ficaram explicitamente fora do corte bootstrap de M24/M25.

Escopo minimo:

- implementar isolamento estrito de threads com deep-copy entre fronteiras
- esbocar e validar wrapper `Shared<T>` para casos raros de ARC atomico explicito
- implementar e baixar `dyn Trait` (fat pointers) para colecoes heterogeneas
- automatizar blindagem de referencias C contra ARC durante blocos FFI `extern("C")`
- criar behavior test para colecao heterogenea baseada em iterador `dyn Trait`

Dependencias: M24, M25, M32.

## M38. Hardening de Coerencia Frontend->Backend (extern/where/params)

Status: planejado (gate de coerencia antes de release).

Objetivo: fechar inconsistencias entre parser/binder/typechecker/HIR/backend que hoje permitem codigo aceito quebrar tarde no pipeline.

Escopo minimo:

- corrigir lowering HIR de `extern c` para preservar simbolo ABI sem mangling indevido
- validar chamadas `extern c` em behavior E2E com fixture minima (`puts` ou equivalente)
- normalizar AST de `where` em parametros para o mesmo contrato semantico de `where` canonico
- validar `where` de parametro no binder/typechecker com regra de tipo booleano obrigatoria
- conectar `parameter_validation` ao pipeline e rejeitar required-after-default com diagnostico claro
- alinhar interpolacao entre docs e implementacao (ou reativar suporte ponta a ponta, ou remover da surface canonica)
- eliminar risco de truncamento em nomes longos no parser, com erro estavel quando exceder limite

Dependencias: M16, M23, M33.

## Ordem recomendada (execucao pratica)

1. `M0`
2. `M1`
3. `M2`
4. `M3`
5. `M4`
6. `M5`
7. `M6`
8. `M7`
9. `M8`
10. `M9`
11. `M10`
12. `M11`
13. `M12`
14. `M13`
15. `M14`
16. `M15`
17. `M16`
18. `M17`
19. `M18`
20. `M19`
21. `M20`
22. `M21`
23. `M23`
24. `M24`
25. `M25`
26. `M26`
27. `M27`
28. `M29`
29. `M28`
30. `M22`
31. `M30`
32. `M31`
33. `M34`
34. `M33`
35. `M38`
36. `M32`
37. `M35`

Justificativa curta: diagnostics e runtime precisam fechar antes de formatter, CLI e ZDoc para evitar retrabalho estrutural.

## O que reaproveitar

- runtime C atual
- testes ativos de runtime, ZIR e target C
- infraestrutura de spans do verifier

## O que adaptar

- ZIR atual
- emitter C
- legalization C
- driver
- docs de arquitetura e roadmap
- lowering de controle de fluxo para ZIR/C
- projeto multi-arquivo
- conformance final frontend/backend

## O que deixar de usar como contrato principal

- `.zir` como entrada principal do usuario
- parse textual de expressao dentro do backend
- nomes antigos de surface como `Outcome` e `Optional`

## Cutover do MVP

O corte MVP atual considera `compiler/driver/main.c` como driver real:

- `project-info`, `verify`, `emit-c` e `build` aceitam projeto ou `zenith.ztproj`.
- `.zt` passa pelo pipeline `.zt -> AST -> semantic -> HIR -> ZIR -> C`.
- `build` gera C e executavel via `gcc`.
- erros de parser/semantic/verifier preservam span quando disponivel.
- os testes de M9 validam C gerado, build e codigo de saida observavel.

## Primeiro ciclo recomendado

Se o objetivo for comecar agora sem abrir frente demais:

1. concluir `M0`
2. concluir `M1`
3. levar `M2` ate:
   - parse de arquivo
   - `namespace`
   - `import`
   - declaracoes de topo
   - expressoes basicas

Esse primeiro corte ja entrega um compilador que le Zenith, mesmo antes da
tipagem completa.




