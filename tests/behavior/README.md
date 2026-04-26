# Behavior Tests

Projetos Zenith usados para validar comportamento observavel do MVP novo.

## Projetos atuais

- `simple_app/`: projeto valido. O entrypoint retorna `40 + 2`, logo o executavel deve sair com codigo `42`.
- `error_type_mismatch/`: projeto invalido. Deve falhar em semantica com span no arquivo `.zt`.
- `error_syntax/`: projeto invalido. Deve falhar no parser com span no arquivo `.zt`.

## Golden C

`simple_app/golden/simple-app.c` e o golden do C gerado pelo driver para o projeto `simple_app`.

O teste de conformance normaliza `CRLF` e `LF` antes de comparar, para manter o golden legivel e evitar ruido de plataforma.

## Build artifacts

Pastas `build/` dentro dos projetos de behavior sao saida de teste e ficam ignoradas por `.gitignore`.

- `control_flow_while/`: projeto valido. Exercita `while` no pipeline real ate o executavel.
- `control_flow_repeat/`: projeto valido. Exercita `repeat N times` no pipeline real ate o executavel.
- `control_flow_match/`: projeto valido. Exercita `match case/default` no pipeline real ate o executavel.
- `control_flow_break_continue/`: projeto valido. Exercita `break` e `continue` dentro de `while`.

- `functions_calls/`: projeto valido. Exercita chamadas diretas, recursao simples e retornos `bool`/`float`/`text`/`void`/`int`.

- `functions_named_args/`: projeto valido. Exercita parametros nomeados em ordem declarada.

- `functions_defaults/`: projeto valido. Exercita valores padrao de parametros.

- `functions_main_signature_error/`: projeto invalido. Deve falhar na validacao do entrypoint `main`.

- `functions_invalid_call_error/`: projeto invalido. Deve falhar em semantica por argumento ausente com span.

- `structs_constructor/`: projeto valido. Exercita construcao de `struct` no pipeline real.

- `structs_field_defaults/`: projeto valido. Exercita defaults de campo em construtor de `struct`.

- `structs_field_read/`: projeto valido. Exercita leitura de campo de `struct`.

- `structs_field_update/`: projeto valido. Exercita atribuicao em campo de `struct`.

- `methods_inherent/`: projeto valido. Exercita metodo inerente via `apply Type`.

- `methods_mutating/`: projeto valido. Exercita metodo mutante com `mut func`.

- `methods_trait_apply/`: projeto valido. Exercita metodo de trait via `apply Trait to Type`.

- `list_basic/`: projeto valido. Exercita literal, indexacao 0-based e atualizacao de `list<int>`.
- `list_text_basic/`: projeto valido. Exercita literal, indexacao 0-based e atualizacao de `list<text>`.
- `list_dyn_trait_basic/`: projeto valido. Exercita `list<dyn<TextRepresentable>>` heterogenea com iteracao e `item.to_text()`.
- `dyn_generic_trait_error/`: projeto invalido. Garante que `dyn<GenericTrait<T>>` explica por que a trait nao e dinamica e sugere generics com `where`.
- `map_basic/`: projeto valido. Exercita literal, indexacao por chave e atualizacao de `map<text,text>`.
- `map_empty_expected_type/`: projeto valido. Exercita mapa vazio `{}` com tipo esperado `map<int,bool>` em atribuicao e chamada de funcao, cobrindo o caminho `make_map<...>` do emitter C.
- `map_int_text_basic/`: projeto valido. Exercita literal, indexacao por chave `int`, `map_len` e atualizacao de `map<int,text>`.
- `map_struct_expected_type/`: projeto valido. Exercita mapa vazio `{}` com tipo esperado `map<int,Flag>` e garante emissao do `optional<Flag>` auxiliar exigido pelo helper gerado de `map`.
- `optional_result_basic/`: projeto valido. Exercita 
one`, `success` e `error`.

- `multifile_import_alias/`: projeto valido. Exercita varredura de `source.root` e chamada qualificada via import alias.
- `public_const_module/`: projeto valido. Exercita `public const` em nivel de modulo com import alias (`mod.CONST`).
- `public_var_module/`: projeto valido. Exercita leitura de `public var` em nivel de modulo via import alias (`mod.VAR`).
- `public_var_module_state/`: projeto valido. Exercita persistencia de estado de `public var` entre funcoes do modulo.
- `readability_block_depth_pass/`: projeto valido. Garante que `warning[style.block_too_deep]` nao bloqueia build em modo normal.
- `readability_block_depth_strict_error/`: projeto invalido em check. Garante que `[diagnostics] profile = "strict"` promove warning de profundidade de bloco para erro.
- `readability_enum_default_pass/`: projeto valido. Garante que `warning[control_flow.enum_default_case]` nao bloqueia build em modo normal.
- `readability_enum_default_strict_error/`: projeto invalido em check. Garante que `[diagnostics] profile = "strict"` promove warning de `case default` em enum conhecido para erro.
- `readability_function_length_pass/`: projeto valido. Garante que `warning[style.function_too_long]` nao bloqueia build em modo normal.
- `readability_function_length_strict_error/`: projeto invalido em check. Garante que `[diagnostics] profile = "strict"` promove warning de funcao longa para erro.
- `readability_warnings_pass/`: projeto valido. Garante que `warning[name.similar]` e `warning[name.confusing]` nao bloqueiam build em modo normal.
- `readability_warnings_strict_error/`: projeto invalido em check. Garante que `[diagnostics] profile = "strict"` promove warnings de legibilidade de nomes para erro.
- `optional_struct_qualified_managed/`: projeto valido. Exercita `optional<mod.Struct>` com nome qualificado entre modulos, retorno direto de `struct`, atribuicao/call-site com wrap implicito, campo opcional dentro de outra `struct` e isolamento de `list<text>` no payload.
- `public_var_cross_namespace_write_error/`: projeto invalido. Garante que `public var` nao pode ser mutado fora do namespace de origem.
- `closure_capture_basic/`: projeto valido. Exercita closure anonima com captura imutavel por valor.
- `closure_mut_capture_error/`: projeto invalido. Garante que closure v1 nao permite mutar variavel capturada.
- `lambda_hof_basic/`: projeto valido. Exercita lambda de expressao (`func(...) => expr`) com `map_int`, `filter_int`, `reduce_int` e captura imutavel.
- `lambda_return_mismatch_error/`: projeto invalido. Garante que lambda infere retorno pelo tipo `func(...) -> ...` esperado e rejeita retorno incompatível.
- `lazy_explicit_order_basic/`: projeto valido. Exercita `lazy<int>` explicito, garantindo que o thunk nao roda na criacao e roda no `force_int`.
- `lazy_reuse_error/`: projeto invalido em runtime. Garante que `lazy<int>` e one-shot e rejeita segundo consumo.
- `std_random_basic/`: projeto valido. Exercita `std.random` baseline (`seed`, `next`, `between`) e valida o estado publico (`seeded`, `last_seed`, `draw_count`).
- `std_random_state_observability/`: projeto valido. Exercita leitura de estado publico de `std.random` e API `stats()`.
- `std_random_between_branches/`: projeto valido. Exercita ramos de `std.random.between` (`min == max` e `max < min`) sem consumo indevido de draw.
- `std_small_helpers/`: projeto valido. Exercita helpers pequenos de `std.validate`, `std.text`, `std.list` e `std.map`.
- `to_text_builtin_basic/`: projeto valido. Exercita `to_text(value)` como builtin core para valores `TextRepresentable`.
- `todo_builtin_fail/`: projeto invalido em runtime. Garante que `todo(message)` falha com mensagem clara.
- `unreachable_builtin_fail/`: projeto invalido em runtime. Garante que `unreachable(message)` falha com mensagem clara.
- `check_intrinsic_message_fail/`: projeto invalido em runtime. Garante que `check(condition, message)` preserva a mensagem recebida.
- `noncanonical_*_error/`: projetos invalidos. Exercitam sugestoes action-first para sintaxe comum de outras linguagens (`string`, `let`, `&&`, `||`, `!`, `null`, `throw`, `abstract`, `virtual`, `union`, `partial`).
- `std_random_cross_namespace_write_error/`: projeto invalido. Garante que `std.random.draw_count` nao pode ser mutado fora do namespace `std.random`.
- `borealis_backend_fallback_stub/`: projeto valido em `run-pass`. Solicita backend desktop (`backend_id=1`) e valida fallback seguro para stub (janela + draw + leitura de input) quando adapter nao esta disponivel no ambiente.
- `borealis_raylib_binding_stub/`: projeto valido em `run-pass`. Exercita o modulo `borealis.raylib` no caminho real do compilador, cobrindo shapes, texto, input, `measure_text`, helpers de `raymath`/`reasings`, `require_available()`, validacao clara para caminho vazio de textura/som e comportamento stub-safe para draw de textura sem DLL nativa.
- `borealis_raylib_assets_real/`: projeto valido em `run-pass`. Quando Raylib nativa estiver disponivel, carrega uma textura `.png` e um som `.wav` reais, valida dimensoes/handle, exercita draw de textura, inicializacao de audio e `load_sound/play/stop/unload` ponta a ponta. Quando a DLL nao estiver presente, o probe fecha com sucesso sem forcar ambiente.
- `borealis_foundations_stub/`: projeto valido em `run-pass`. Exercita assets, events tipados, save, storage, services, database, UI/HUD, editor metadata e settings persistente, incluindo loaders tipados de assets, metadata logica, ids estaveis, conflito claro de `kind` por chave, perfis de settings em `storage`, widgets de interface e persistencia de string vazia sem confundir com item removido.
- `borealis_ecs_hybrid_stub/`: projeto valido em `run-pass`. Exercita o subset inicial de componentes do ECS (`borealis.engine.ecs`) com stub autocontido para runtime atual.
- `borealis_runtime_gameplay_stub/`: projeto valido em `run-pass`. Exercita os modulos de runtime/jogabilidade do Borealis (`contracts`, `entities`, `movement`, `controllers`, `vehicles`, `animation`, `audio`, `ai`, `camera`, `input`, `world` e `procedural`) em um fluxo integrado.
- `multifile_missing_import/`: projeto invalido. Deve falhar quando um import nao existe em `source.root`.
- `multifile_namespace_mismatch/`: projeto invalido. Deve falhar quando 
amespace` nao corresponde ao caminho do arquivo.
- `multifile_duplicate_symbol/`: projeto invalido. Deve falhar quando dois arquivos geram o mesmo simbolo efetivo no programa agregado.
- `multifile_private_access/`: projeto invalido. Deve falhar quando um simbolo sem `public` e acessado via import alias.
- `monomorphization_limit_error/`: projeto invalido. Deve falhar no gate de monomorfizacao quando `build.monomorphization_limit` ficar abaixo das instancias genericas reais.
- `where_contracts_ok/`: projeto valido. Exercita contratos `where` em parametro, construcao de `struct` e atribuicao de campo.
- `where_contract_param_error/`: projeto invalido em runtime. Deve falhar com `error[runtime.contract]` por violacao de contrato em parametro.
- `where_contract_construct_error/`: projeto invalido em runtime. Deve falhar com `error[runtime.contract]` por violacao de contrato em construcao de `struct`.
- `where_contract_field_assign_error/`: projeto invalido em runtime. Deve falhar com `error[runtime.contract]` por violacao de contrato em atribuicao de campo.
- `std_net_basic/`: projeto valido. Exercita `std.net` no baseline atual via loopback TCP local. O script `run-loopback.ps1` sobe um servidor local em `127.0.0.1:41234`, executa o binario e fecha o listener automaticamente.
- `std_collections_managed_arc/`: projeto valido. Exercita copy/mutate isolation em `grid2d<text>`, `pqueue<text>`, `circbuf<text>`, `btreemap<text,text>`, `btreeset<text>` e `grid3d<text>`.
- `std_collections_queue_stack_cow/`: projeto valido. Exercita `queue/stack` com retorno estruturado em `dequeue/pop`, preservando isolamento apos copia compartilhada.
- `edge_boundaries_empty/`: projeto valido. Exercita valores-limite (`u8/u16/u32/u64`, `int` proximo ao limite) e estruturas vazias (`text/list/map/bytes`).
- `std_test_basic/`: projeto valido. Exercita `std.test` diretamente via `main`, validando os desfechos de `skip(...)` e `fail(...)` no comando `zt test`.
- `std_test_attr_pass_skip/`: projeto valido. Exercita o harness real de `zt test` com funcoes marcadas por `attr test`, cobrindo 1 caso pass e 1 caso skip.
- `std_test_attr_fail/`: projeto invalido para o runner. Exercita o harness real de `zt test` com funcoes marcadas por `attr test`, cobrindo 1 caso pass, 1 caso skip e 1 caso fail.
- `std_test_helpers_pass/`: projeto valido. Exercita `is_true`, `is_false`, `equal_int`, `equal_text`, `not_equal_int` e `not_equal_text` no caminho feliz.
- `std_test_helpers_bool_fail/`: projeto invalido em runtime. Congela a mensagem de falha de `is_true(false)`.
- `std_test_helpers_equal_fail/`: projeto invalido em runtime. Congela a mensagem esperado/recebido de `equal_int(actual, expected)`.
- `std_test_helpers_not_equal_fail/`: projeto invalido em runtime. Congela a mensagem de `not_equal_text` quando os textos sao iguais.

- `enum_match/`: fixture de comportamento para enum com payload + match com binding de payload (check semantico OK; build E2E bloqueado pelo stub de lowering HIR->ZIR no source atual).
- `enum_match_non_exhaustive_error/`: fixture invalida para diagnostico de match nao exaustivo em enum conhecido.

