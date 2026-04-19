# Zenith Next Implementation Checklist

Checklist operacional derivado de `IMPLEMENTATION_ROADMAP.md`.

## M0. Congelar contratos

- [x] Revisar `language/spec/surface-syntax.md` e marcar claramente o que esta fora do MVP
- [x] Confirmar em doc que `stdlib/process` fica fora do primeiro frontend
- [x] Registrar formalmente que ZIR interna sera estruturada
- [x] Tratar ZIR textual como debug/fixture, nao como contrato central
- [x] Revisar docs antigas que ainda falam como se `.zir` fosse a entrada principal

## M1. Lexer

- [x] Criar `compiler/frontend/lexer/token.h`
- [x] Criar `compiler/frontend/lexer/token.c`
- [x] Criar `compiler/frontend/lexer/lexer.h`
- [x] Criar `compiler/frontend/lexer/lexer.c`
- [x] Definir enum de tokens
- [x] Definir estrutura de token com span
- [x] Implementar trivia e comentarios
- [x] Implementar identificadores e keywords
- [x] Implementar inteiros e floats
- [x] Implementar strings simples
- [x] Implementar multiline text com `"""`
- [x] Implementar operadores e delimitadores
- [x] Criar `tests/frontend/test_lexer.c`
- [x] Adicionar fixtures `.zt` minimas para o lexer

## M2. AST e parser sintatico

- [x] Criar `compiler/frontend/ast/model.h`
- [x] Criar `compiler/frontend/ast/model.c`
- [x] Criar `compiler/frontend/parser/parser.h`
- [x] Criar `compiler/frontend/parser/parser.c`
- [x] Modelar arquivo fonte, namespace e imports
- [x] Modelar declaracoes de topo
- [x] Modelar statements
- [x] Modelar expressoes
- [x] Modelar tipos e generics
- [x] Modelar `where`
- [x] Modelar `match`
- [x] Modelar parametros nomeados e defaults
- [x] Implementar parser de arquivo
- [x] Implementar parser de declaracoes
- [x] Implementar parser de expressoes com precedencia
- [x] Criar `tests/frontend/test_parser.c`
- [x] Adicionar fixtures `.zt` para parser happy path
- [x] Adicionar fixtures `.zt` com erros sintaticos

## M3. Diagnostics e binder

- [x] Criar `compiler/semantic/diagnostics/diagnostics.h`
- [x] Criar `compiler/semantic/diagnostics/diagnostics.c`
- [x] Criar `compiler/semantic/symbols/symbols.h`
- [x] Criar `compiler/semantic/symbols/symbols.c`
- [x] Criar `compiler/semantic/binder/binder.h`
- [x] Criar `compiler/semantic/binder/binder.c`
- [x] Definir estrutura de diagnostico com span
- [x] Definir tabela de simbolos por escopo
- [x] Resolver namespaces
- [x] Resolver imports
- [x] Resolver escopo lexico local
- [x] Tratar prelude implicito `core.*`
- [x] Rejeitar shadowing
- [x] Rejeitar nomes duplicados no mesmo escopo
- [x] Criar `tests/semantic/test_binder.c`

## M4. Sistema de tipos e validacao semantica

- [x] Criar `compiler/semantic/types/types.h`
- [x] Criar `compiler/semantic/types/types.c`
- [x] Criar `compiler/semantic/types/checker.h`
- [x] Criar `compiler/semantic/types/checker.c`
- [x] Implementar tipos builtin do MVP
- [x] Implementar tipos genericos do usuario
- [x] Implementar `optional` e `result`
- [x] Implementar rules de `const` vs `var`
- [x] Implementar validacao de metodo com `!`
- [x] Implementar `where` de refinement
- [x] Implementar `where` de constraints genericas
- [x] Implementar traits do `core.*`
- [x] Implementar semantica de `==` e `!=`
- [x] Implementar validacao de `map<K, V>`
- [x] Implementar named args
- [x] Implementar defaults
- [x] Implementar conversoes numericas explicitas
- [x] Implementar validacao de overflow inteiro no nivel semantico/IR
- [x] Criar `tests/semantic/test_types.c`
- [x] Criar `tests/semantic/test_constraints.c`

## M5. HIR

- [x] Criar `compiler/hir/nodes/model.h`
- [x] Criar `compiler/hir/nodes/model.c`
- [x] Criar `compiler/hir/lowering/from_ast.h`
- [x] Criar `compiler/hir/lowering/from_ast.c`
- [x] Definir nos tipados da HIR
- [x] Baixar AST resolvida para HIR
- [x] Desugar named args
- [x] Desugar parametros com default
- [x] Normalizar `match`
- [x] Normalizar chamadas de metodo e `apply`
- [x] Preservar spans na HIR
- [x] Criar `tests/semantic/test_hir_lowering.c`

## M6. ZIR v2 estruturada

- [x] Revisar `compiler/zir/model.h`
- [x] Revisar `compiler/zir/model.c`
- [x] Criar `compiler/zir/lowering/from_hir.h`
- [x] Criar `compiler/zir/lowering/from_hir.c`
- [x] Definir operandos estruturados
- [x] Definir terminadores estruturados
- [x] Remover `expr_text` como caminho principal
- [x] Remover `value_text` como caminho principal
- [x] Remover `condition_text` como caminho principal
- [x] Remover `message_text` como caminho principal
- [x] Adaptar printer textual para o novo modelo
- [x] Adaptar parser textual para fixtures/debug
- [x] Adaptar verifier para o novo modelo
- [x] Criar `tests/zir/test_lowering.c`
- [x] Atualizar `tests/zir/test_printer.c`
- [x] Atualizar `tests/zir/test_verifier.c`

## M7. Adaptar backend C

- [x] Revisar `compiler/targets/c/legalization.c`
- [x] Revisar `compiler/targets/c/emitter.c`
- [x] Remover parse textual de expressao no backend
- [x] Adaptar emitter para ZIR estruturada
- [x] Adaptar legalization para ZIR estruturada
- [x] Alinhar nomes de tipos com `optional` e `result`
- [x] Revisar casos ligados a traits/core quando baixarem para IR
- [x] Manter runtime C atual como base
- [x] Atualizar `tests/targets/c/test_legalization.c`
- [x] Atualizar `tests/targets/c/test_emitter.c`

## M8. Driver real e zenith.ztproj

- [x] Revisar `compiler/driver/main.c`
- [x] Criar `compiler/project/ztproj.h`
- [x] Criar `compiler/project/ztproj.c`
- [x] Implementar parser de `zenith.ztproj`
- [x] Resolver source root do manifesto
- [x] Resolver entrypoint do projeto
- [x] Ligar pipeline `.zt -> AST`
- [x] Ligar pipeline `AST -> semantic`
- [x] Ligar pipeline `semantic -> HIR`
- [x] Ligar pipeline `HIR -> ZIR`
- [x] Ligar pipeline `ZIR -> C`
- [x] Criar `tests/driver/test_project.c`
- [x] Popular `tests/behavior/`

## M9. Conformance e migracao

- [x] Criar smoke tests end-to-end
- [x] Criar golden tests do C gerado
- [x] Criar casos de erro com spans completos
- [x] Comparar comportamento observavel do MVP novo com o esperado
- [x] Atualizar docs de arquitetura para o pipeline real
- [x] Atualizar roadmap antigo para refletir o cutover

## M10. Controle de fluxo executavel

- [x] Revisar lowering atual de `ZT_HIR_WHILE_STMT`
- [x] Implementar `while` em ZIR estruturada
- [x] Implementar emissao C de `while`
- [x] Criar behavior `control_flow_while`
- [x] Revisar lowering atual de `ZT_HIR_REPEAT_STMT`
- [x] Implementar `repeat N times` em ZIR estruturada
- [x] Implementar emissao C de `repeat N times`
- [x] Criar behavior `control_flow_repeat`
- [x] Revisar lowering atual de `ZT_HIR_MATCH_STMT`
- [x] Implementar `match` em ZIR estruturada
- [x] Implementar emissao C de `match`
- [x] Criar behavior `control_flow_match`
- [x] Implementar `break`
- [x] Implementar `continue`
- [x] Criar behavior `control_flow_break_continue`
- [x] Definir subset executavel de `for item in collection`
- [x] Implementar `for item in collection`
- [x] Definir subset executavel de `for key, value in map`
- [x] Implementar `for key, value in map`
- [x] Atualizar conformance para controle de fluxo

## M11. Funcoes, chamadas e entrypoints completos

- [x] Revisar chamadas diretas no HIR -> ZIR
- [x] Revisar chamadas diretas no emitter C
- [x] Criar behavior `functions_calls`
- [x] Garantir parametros posicionais em runtime
- [x] Garantir parametros nomeados em runtime
- [x] Criar behavior `functions_named_args`
- [x] Garantir parametros com default em runtime
- [x] Criar behavior `functions_defaults`
- [x] Validar assinatura de `main` no driver/typechecker
- [x] Criar behavior `functions_main_signature_error`
- [x] Suportar retorno `int`
- [x] Suportar retorno `bool`
- [x] Suportar retorno `float`
- [x] Suportar retorno `text`
- [x] Suportar retorno `void`
- [x] Decidir suporte a recursao simples
- [x] Implementar recursao simples se aceita no MVP
- [x] Criar diagnostics para chamada invalida com spans

## M12. Structs, campos, metodos e apply

- [x] Revisar construtor de struct no HIR -> ZIR
- [x] Revisar construtor de struct no emitter C
- [x] Criar behavior `structs_constructor`
- [x] Implementar defaults de campo ponta a ponta
- [x] Criar behavior `structs_field_defaults`
- [x] Implementar leitura de campo ponta a ponta
- [x] Criar behavior `structs_field_read`
- [x] Implementar atribuicao de campo em `var`
- [x] Criar behavior `structs_field_update`
- [x] Implementar metodos inerentes via `apply to Type`
- [x] Criar behavior `methods_inherent`
- [x] Implementar metodos de trait via `apply Trait to Type`
- [x] Criar behavior `methods_trait_apply`
- [x] Definir receiver no IR para metodos
- [x] Implementar metodos mutantes com `!`
- [x] Criar behavior `methods_mutating`
- [x] Criar diagnostics para mutacao invalida
- [x] Estabilizar layout C de structs do usuario

## M13. Collections, optional/result e ownership no runtime C

- [x] Revisar runtime C de `list<int>`
- [x] Revisar runtime C de `list<text>`
- [x] Implementar `list<T>` literal ponta a ponta
- [x] Implementar indexacao de lista ponta a ponta
- [x] Implementar slice de lista ponta a ponta
- [x] Implementar atualizacao de lista ponta a ponta
- [x] Implementar tamanho de lista ponta a ponta
- [x] Criar behaviors de lista
- [x] Revisar runtime C de `map<text,text>`
- [x] Implementar `map<K,V>` literal ponta a ponta
- [x] Implementar indexacao de map ponta a ponta
- [x] Implementar atualizacao de map ponta a ponta
- [x] Implementar tamanho de map ponta a ponta
- [x] Criar behaviors de map
- [x] Implementar `optional<T>` com `none` ponta a ponta
- [x] Criar behavior `optional_result_basic` cobrindo `optional`
- [x] Implementar `result<T,E>` com `success` e `error` ponta a ponta
- [x] Criar behavior `optional_result_basic` cobrindo `result`
- [x] Definir subset MVP de propagacao/unwrap
- [x] Implementar propagacao/unwrap se aceito no MVP
- [x] Auditar ownership/release para collections e text no subset executavel atual
- [x] Atualizar `tests/runtime/c/test_runtime.c`

## M14. Projeto multi-arquivo, namespaces e imports reais

- [x] Implementar varredura do source root
- [x] Carregar todos os arquivos `.zt` do projeto
- [x] Validar consistencia entre path e `namespace`
- [x] Resolver imports qualificados
- [x] Resolver imports com alias
- [x] Rejeitar imports inexistentes
- [x] Rejeitar simbolos duplicados entre arquivos no namespace efetivo agregado
- [x] Decidir regra de ciclos de import no MVP
- [x] Implementar diagnostico para ciclos invalidos
- [x] Agregar ASTs em um programa semanticamente unico
- [x] Adaptar binder para multi-arquivo
- [x] Adaptar typechecker para multi-arquivo
- [x] Adaptar HIR para multiplos modulos/arquivos no corte agregado atual
- [x] Adaptar ZIR/emitter para nomes sem colisao via alias qualificado
- [x] Manter entrypoint de projeto no manifesto legado
- [x] Criar behavior `multifile_import_alias`
- [x] Criar behavior `multifile_missing_import`
- [x] Criar behavior `multifile_namespace_mismatch`
- [x] Criar behavior `multifile_duplicate_symbol`

## M15. Fechamento semantico da surface syntax MVP

- [x] Revisar `language/spec/surface-syntax.md` contra parser atual
- [x] Listar tudo que o parser aceita
- [x] Marcar explicitamente o que fica fora do MVP executavel
- [x] Fechar generics do MVP no typechecker
- [x] Fechar constraints `where`
- [x] Fechar refinements possiveis no primeiro backend
- [x] Fechar traits do `core.*` no typechecker
- [x] Fechar regras de igualdade
- [x] Fechar regras de hash
- [x] Fechar conversoes numericas explicitas
- [x] Fechar overflow e unsigned conforme spec
- [x] Melhorar diagnostics de casos frequentes
- [x] Garantir spans em parser diagnostics
- [x] Garantir spans em semantic diagnostics
- [x] Garantir spans em lowering diagnostics
- [x] Criar fixtures parser happy path
- [x] Criar fixtures parser error path
- [x] Criar fixtures semantic error path
- [x] Criar golden diagnostics essenciais

## M16. Conformance final frontend/backend

- [x] Criar matriz de behavior tests por feature MVP
- [x] Criar `tests/conformance/test_m16.c`
- [x] Rodar todos os projetos validos de `tests/behavior`
- [x] Comparar codigo de saida observavel de cada projeto valido
- [x] Comparar stdout/stderr observavel quando aplicavel
- [x] Criar golden C para features representativas
- [x] Criar golden diagnostics para erros principais
- [x] Confirmar que backend C nao depende de parse textual de ZIR
- [x] Confirmar que `.zir` textual e apenas debug/fixture
- [x] Atualizar docs para declarar cobertura frontend/backend final
- [x] Listar explicitamente pendencias movidas para stdlib
- [x] Listar explicitamente pendencias movidas para CLI final
- [x] Listar explicitamente pendencias movidas para ZPM
- [x] Executar suite completa de lexer/parser/semantic/HIR/ZIR/C/runtime/driver/conformance
- [x] Isolar artifacts de build dos harnesses de conformance em sandbox temporario (`.ztc-tmp/.../sandbox`) para evitar lock de `.exe` no Windows

## Checklist do primeiro ciclo recomendado

- [x] Fechar `M0`
- [x] Fechar `M1`
- [x] Em `M2`, chegar ate parse de:
  - [x] arquivo
  - [x] `namespace`
  - [x] `import`
  - [x] declaracoes de topo
  - [x] expressoes basicas


## M17. Consolidacao dos specs canonicos

- [x] Criar `language/spec/README.md`
- [x] Criar `language/spec/surface-syntax.md`
- [x] Criar `language/spec/project-model.md`
- [x] Criar `language/spec/compiler-model.md`
- [x] Criar `language/spec/tooling-model.md`
- [x] Reduzir `language/surface-spec.md` a ponteiro de compatibilidade
- [x] Atualizar `language/README.md` para apontar para os specs canonicos
- [x] Atualizar `README.md` para apontar para os specs canonicos
- [x] Atualizar roadmap/checklist para pos-M16

## M18. Alinhar implementacao ao manifesto e tooling final

- [x] Atualizar parser de `zenith.ztproj` para `[project]`, `[source]`, `[app]`, `[lib]`, `[build]`, `[test]` e `[zdoc]`
- [x] Validar `project.kind = "app"`
- [x] Validar `project.kind = "lib"`
- [x] Exigir `app.entry` para projetos app
- [x] Exigir `lib.root_namespace` para projetos lib
- [x] Atualizar fixtures `zenith.ztproj` dos behavior tests
- [x] Atualizar driver para usar `app.entry` no caminho canonico
- [x] Atualizar docs do driver para separar bootstrap atual de CLI canonica `zt`
- [x] Atualizar `language/surface-implementation-status.md` apos o alinhamento
- [x] Rodar suite de driver/conformance apos migracao do manifesto
- [x] Isolar implementacao Lua/self-host em `_legacy/`
- [x] Atualizar README raiz para apontar para `` como caminho oficial

## M19. Stdlib MVP

- [x] Fechar spec de `std.io`
- [x] Fechar spec de `std.bytes`
- [x] Fechar spec de `std.fs`
- [x] Fechar spec de `std.fs.path`
- [x] Fechar spec de `std.json`
- [x] Fechar spec de `std.math`
- [x] Fechar spec de `std.text`
- [x] Fechar spec de `std.validate`
- [x] Fechar spec de `std.time`
- [x] Fechar spec de `std.format`
- [x] Fechar spec de `std.os`
- [x] Fechar spec de `std.os.process`
- [x] Fechar spec de `std.test`
- [x] Fechar politica de streams gerais sem `std.stream` publico no MVP
- [x] Fechar politica de rede blocking com timeout explicito no MVP
- [x] Fechar modelo inicial de socket/endereco para `std.net`
- [x] Implementar literal `hex bytes "..."` no lexer/parser/typechecker/backend
- [x] Implementar modulo `std.bytes`
- [x] Implementar conversoes UTF-8 em `std.text`
- [x] Implementar modulo stdlib minimo necessario para behavior tests

## M20. ZDoc e ZPM tooling inicial

- [x] Implementar parser/checker inicial de `.zdoc`
- [x] Implementar validacao de `@target`
- [x] Implementar validacao de `@page` em `zdoc/guides/`
- [x] Implementar warnings de `@link` nao resolvido
- [x] Definir schema inicial de `zenith.lock`
- [x] Implementar esqueleto local de leitura de `[dependencies]` e `[dev_dependencies]`

## M21. Fechamento dos specs finais

- [x] Criar `LANGUAGE_COHERENCE_CLOSURE.md`
- [x] Criar decision de fechamento de coerencia
- [x] Criar decision de riscos backend/runtime
- [x] Criar `language/spec/stdlib-model.md`
- [x] Criar `language/spec/runtime-model.md`
- [x] Criar `language/spec/backend-scalability-risk-model.md`
- [x] Criar `language/spec/diagnostics-model.md`
- [x] Criar `language/spec/formatter-model.md`
- [x] Criar `language/spec/implementation-status.md`
- [x] Atualizar `language/spec/README.md`
- [x] Atualizar `README.md`
- [x] Clarificar `void` em `surface-syntax.md`
- [x] Clarificar papeis de `where` em `surface-syntax.md`
- [x] Clarificar que construtores com `where` nao viram `result` automaticamente
- [x] Clarificar `const` collections em `surface-syntax.md`
- [x] Auditar conflicts restantes entre decisions antigas e specs canonicos
- [x] Atualizar `language/surface-implementation-status.md` para usar `implementation-status.md`

## M22. Formatter obrigatorio

- [x] Criar modulo do formatter
- [x] Implementar parser-preserving formatting
- [x] Format namespace/imports
- [x] Format attrs
- [x] Format funcoes e named args
- [x] Format `where` em campos e parametros
- [x] Format structs
- [x] Format enums
- [x] Format match/case
- [x] Preservar comentarios
- [x] Garantir idempotencia
- [x] Criar golden tests de formatter
- [x] Expor `zt fmt` no CLI final

## M23. Diagnostics renderer real

- [x] Unificar diagnostic model entre project/lexer/parser/semantic/lowering/ZIR/backend/runtime
- [x] Unificar project/lexer/parser/semantic/lowering/ZIR/backend no renderer detalhado
- [x] Implementar renderer terminal detalhado
- [x] Definir catalogo inicial de diagnostic codes
- [x] Adicionar `help` acionavel para erros frequentes
- [x] Reduzir cascatas de erro em parser/semantic
- [x] Criar golden diagnostics de project
- [x] Criar golden diagnostics de syntax
- [x] Criar golden diagnostics de type
- [x] Criar golden diagnostics de mutability
- [x] Criar golden diagnostics de result/optional
- [x] Criar golden diagnostics de runtime

## M24. Runtime C ownership and backend scalability hardening

- [ ] Implementar ARC Não-Atômico genérico como padrão do back-end C.
- [ ] Implementar Isolamento estrito de threads, providenciando deep-copy entre passagens.
- [ ] Esboçar Wrapper `Shared<T>` para raras necessidades de ARC Atômico explícito.
- [ ] Implementar e baixar `dyn Trait` (Fat Pointers) para instanciar arrays genéricos heterogêneos.
- [ ] Automatizar blindagem de referências C contra o ARC durante execução de blocos FFI `extern("C")`.
- [ ] Injetar *bounds checks* preventivos nas coleções, acionando Panic (Unwind) em vez de C Segfault.
- [x] Cobrir guards de bounds em text/bytes/list/map no runtime C com erro `ZT_ERR_INDEX`.
- [ ] Mapear exceções e injeções matemáticas de Panic para overflow não tratado.
- [x] Implementar COW para collections managed onde for pragmatico.
- [x] Atualizar emitter C para rebind com `*_set_owned` em mutacoes de list/map.
- [x] Corrigir runtime de `map<text,text>` no MVP para insercao/get/iteracao sem crash (fallback seguro por busca linear).
- [x] Corrigir emissao de `eq/ne` para `outcome<text,text>` no backend C (comparacao semantica via helper, sem `!=` em struct C).
- [ ] Garantir propagação `?`, early returns e cleanups normais alinhados ao tempo do ARC.
- [ ] Representar `optional<T>` e `result<T,E>` in-place na *stack* sempre que possível.
- [x] Garantir `result<void,E>` success sem heap allocation no runtime C atual (singleton imortal).
- [ ] Definir limite/diagnóstico formal para a explosão de monomorfização da base genérica regular.

## M25. Value semantics and Behavior conformance

- [x] Criar behavior test para a divisa do Isolamento garantindo que *deep copy* quebra os laços de ARC.
- [ ] Criar behavior test para coleção heterogênea baseada em iterador `dyn Trait`.
- [x] Criar behavior test garantindo que acessos fora do limite lancem *Panic* limpo no lugar de crash.
- [x] Validar bounds guard em behavior conformance (`tests/behavior/runtime_index_error`).
- [x] Criar behavior para copy/mutate de list e map (COW validation).
- [x] Criar behavior para copy/mutate de struct com campos managed.
- [x] Criar behavior para optional/result com payload managed.
- [x] Criar diagnostics para mutação observável proibida em *const collection*.
- [x] Integrar nova value semantics (ARC puro) na matrix de testes oficial.

## M26. Runtime `where` contracts completos

- [x] Gerar checks de field `where` em construcao
- [x] Gerar checks de field `where` em atribuicao
- [x] Gerar checks de parameter `where` em chamada
- [x] Garantir que construtores com `where` nao mudam secretamente para `result<T,E>`
- [x] Criar exemplos canonicos `try_create_*` para validacao recuperavel
- [x] Reportar `runtime.contract`
- [x] Incluir predicate no erro quando disponivel
- [x] Incluir valor no erro quando seguro
- [x] Incluir span de origem no erro quando disponivel
- [x] Criar behavior tests de contrato valido/invalido
- [x] Criar golden diagnostics de contrato

## M27. Enums com payload e match forte

- [x] Fechar AST/HIR de enum payload
- [x] Baixar enum payload para ZIR
- [x] Emitir layout C com tag/payload
- [ ] Implementar construtores qualificados
- [ ] Implementar match por case de enum
- [ ] Implementar binding de payload
- [ ] Implementar case default
- [ ] Criar diagnostics de match invalido
- [ ] Implementar exaustividade quando enum conhecido e nao houver `case default`
- [ ] Criar diagnostics para enum case ausente
- [ ] Documentar `case default` como opt-out de exaustividade futura
- [ ] Criar behavior tests de enum/match

## M28. Bytes, UTF-8 e stdlib base

- [x] Implementar `std.bytes.empty`
- [x] Implementar `std.bytes.from_list`
- [x] Implementar `std.bytes.to_list`
- [x] Implementar `std.bytes.join`
- [x] Implementar `std.bytes.starts_with`
- [x] Implementar `std.bytes.ends_with`
- [x] Implementar `std.bytes.contains`
- [x] Implementar `std.text.to_utf8`
- [x] Implementar `std.text.from_utf8`
- [x] Retornar erro tipado para UTF-8 invalido
- [x] Criar behavior tests para bytes/text
- [x] Implementar stdlib minima para behavior tests

## M29. API segura de collections

- [x] Definir namespace final de APIs seguras de collection
- [x] Implementar safe map lookup retornando `optional<V>`
- [x] Decidir safe list access
- [x] Implementar safe list access se aceito
- [x] Atualizar exemplos oficiais para safe lookup em ausencia esperada
- [x] Documentar `map[key]` como acesso estrito/assertivo
- [x] Criar behavior tests

## M30. CLI final

- [x] Implementar `zt check`
- [x] Implementar `zt build`
- [x] Implementar `zt run`
- [x] Implementar `zt test`
- [x] Implementar `zt fmt`
- [x] Implementar `zt doc check`
- [x] Usar renderer de diagnostics em todos os comandos
- [x] Garantir project root discovery
- [x] Garantir saidas previsiveis para CI

Observacao M30:
- `zt test` esta implementado no bootstrap atual como alias para `zt check` (ainda sem runner dedicado por `attr test`).
- `zt fmt` esta implementado no bootstrap atual como gate de formatacao/check (formatter canonico de M22 segue em aberto).
- `--ci` habilita renderer curto e saida previsivel para pipelines (diagnostics em linha unica e logs de build reduzidos).

## M31. ZDoc funcional

- [x] Implementar parser inicial de `.zdoc`
- [x] Validar `@target`
- [x] Validar `@page`
- [x] Validar `@link`
- [x] Emitir warning para link nao resolvido
- [x] Emitir warning para public API sem docs
- [x] Suportar docs privadas
- [x] Excluir docs privadas do output publico por default
- [x] Integrar `zt doc check`

## M32. Matriz de conformidade final

- [ ] Atualizar matriz para usar labels de `implementation-status.md`
- [ ] Cobrir itens `Risk` explicitamente
- [ ] Cobrir RC cycles como risco/gate
- [ ] Cobrir monomorfizacao como risco/gate
- [ ] Cobrir heap-vs-stack wrappers como risco/gate
- [ ] Cobrir parser por feature
- [ ] Cobrir semantic por feature
- [ ] Cobrir lowering/backend por feature
- [ ] Cobrir runtime por feature
- [ ] Cobrir diagnostics por feature
- [ ] Cobrir formatter por feature
- [ ] Cobrir stdlib base
- [ ] Cobrir CLI final
- [ ] Cobrir ZDoc
- [ ] Cobrir where runtime e match exaustivo na matriz
- [ ] Rodar suite completa antes de declarar MVP estavel

## M33. Implementação das Stdlibs MVP

- [ ] Implementar `std.io` (Terminal, print explícito)
- [ ] Implementar `std.fs` e `std.fs.path`
- [ ] Implementar `std.json` (Parser/Emitter básico)
- [ ] Implementar `std.math` (Vetores e álgebra linear básica)
- [ ] Implementar `std.collections` (Queue, Grid2D, etc.)
- [ ] Implementar `std.random` (Xoshiro/PCG)
- [ ] Implementar `std.validate`
- [ ] Implementar `std.time` (Instant, Duration)
- [ ] Implementar `std.format`
- [ ] Implementar `std.os` e `std.os.process`
- [ ] Implementar `std.test` (Harness para `@test`)
- [ ] Implementar `std.net` (TCP Client, Multi-IP DNS)
- [ ] Criar behavior tests para cada módulo
- [ ] Validar ownership ARC em cada implementação

## Ordem de execucao pos-M20 (recomendada)

1. `M21`
2. `M23`
3. `M24`
4. `M25`
5. `M26`
6. `M27`
7. `M29`
8. `M28`
9. `M22`
10. `M30`
11. `M31`
12. `M33`
13. `M32`

Observacao: mantemos a numeracao original dos marcos para preservar referencias historicas, mas a execucao pratica segue a ordem acima.

## Release V1 Gates

Release v1 nao deve ser declarado pronto ate que todos os itens abaixo estejam verdes:

- [ ] `zt fmt` existe e e parte obrigatoria do fluxo canonico
- [ ] golden tests do formatter cobrem imports, attrs, `where`, named args, structs, enums, `match` e comentarios
- [ ] diagnostics sao uniformes entre project, lexer, parser, semantic, lowering, ZIR, backend e runtime
- [ ] diagnostics usam codigos estaveis e renderer estruturado
- [ ] `zenith.ztproj` e o modelo de CLI estao coerentes e user-facing
- [ ] runtime de `where`, collections, `optional`, `result`, bytes e UTF-8 nao possui ambiguidade canonica
- [ ] conformance cobre comportamento observavel, nao apenas parsing
- [ ] ZDoc esta funcional o suficiente para manter codigo publico limpo
- [ ] nenhuma feature critica de release possui duas formas canonicas conflitantes ou dois docs conflitantes

