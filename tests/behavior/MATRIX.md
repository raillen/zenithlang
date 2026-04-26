# Zenith Next Behavior Matrix

This matrix is the M16 executable coverage map for the current C backend cut.

For the canonical M32 layered/risk matrix, see language/spec/conformance-matrix.md.

Legend:

- `valid`: project must build and the executable exit code is checked.
- `invalid`: project must fail verification/build and diagnostics are checked by fragment.
- `deferred`: accepted surface area not executable in this cut.

## Valid Projects

| Project | Feature | Expected exit |
| --- | --- | --- |
| `simple_app` | Project smoke, `emit-c` golden and integer return | `42` |
| `control_flow_while` | `while` lowering and C emission | `6` |
| `control_flow_repeat` | `repeat N times` lowering and C emission | `9` |
| `control_flow_match` | `match case/default` lowering and C emission | `7` |
| `enum_match` | Enum construction and payload match (`case Enum.Variant(...)`) | `0` |
| `control_flow_break_continue` | `break` and `continue` in loops | `8` |
| `control_flow_for_list` | `for item, index in list<int>` lowering and C emission | `13` |
| `control_flow_for_map` | `for key, value in map<text,text>` lowering and C emission | `6` |
| `functions_calls` | Direct calls, recursion and mixed return types | `6` |
| `functions_named_args` | Named arguments in declaration order | `6` |
| `functions_defaults` | Trailing default parameters | `18` |
| `structs_constructor` | Struct constructor | `42` |
| `structs_field_defaults` | Struct field defaults | `117` |
| `structs_field_read` | Struct field read | `11` |
| `structs_field_update` | Struct field update on `var` | `12` |
| `methods_inherent` | Inherent method via `apply Type` | `7` |
| `methods_inherent_apply` | Inherent mutating method through method call syntax | `7` |
| `methods_mutating` | Receiver mutation through `mut func` method | `6` |
| `methods_trait_apply` | Trait method through `apply Trait to Type` | `8` |
| `list_basic` | `list<int>` literal, index and update | `18` |
| `list_text_basic` | `list<text>` literal, index and update | `0` |
| `list_dyn_trait_basic` | `list<dyn<TextRepresentable>>` heterogenea com `for` + `to_text()` | `16` |
| `list_slice_len` | `list<int>` slice and `len(list)` | `37` |
| `text_slice_len` | `text` slice and `len(text)` | `8` |
| `text_utf8_index_slice` | `text` index/slice por code point com UTF-8 multi-byte | `21` |
| `std_text_basic` | `std.text` alpha-safe (`trim`, busca, predicados, `limit`, `to_utf8`) | `0` |
| `std_concurrent_boundary_copy_basic` | `std.concurrent` transfer helpers (`copy_int`, `copy_text`, `copy_bytes`, `copy_list_int`, `copy_list_text`, `copy_map_text_text`) | `0` |
| `map_basic` | `map<text,text>` literal, index and update | `0` |
| `map_empty_expected_type` | Empty map literal `{}` with expected `map<int,bool>` in init and function call | `0` |
| `map_int_text_basic` | `map<int,text>` literal, index, update and `len(map)` | `7` |
| `map_struct_expected_type` | Empty map literal `{}` with expected `map<int,Flag>` and generated `optional<Flag>` helper | `0` |
| `map_safe_get` | Safe lookup `map.get(key) -> optional<text>` sem panic em chave ausente | `15` |
| `list_safe_get` | Safe lookup `list.get(index) -> optional<int>` sem panic em indice ausente | `27` |
| `map_len_basic` | `len(map<text,text>)` | `2` |
| `value_semantics_collections` | Copy/mutate isolation for `list` and `map` via COW in runtime/backend | `131` |
| `value_semantics_struct_managed` | Copy/mutate isolation para struct com campos `list/map` via rebind COW | `131` |
| `value_semantics_arc_isolation` | Chain-copy isolation (`a -> b -> c`) for `list` and `map` under COW/RC | `158` |
| `value_semantics_optional_result_managed` | `optional<list<int>>` creation/copy and `result<list<int>, text>` `?` with COW-safe list mutation | `0` |
| `optional_struct_qualified_managed` | `optional<mod.Struct>` com `struct` qualificada, retorno direto, atribuicao/call-site implicitos, campo opcional em outra `struct` e isolamento de `list<text>` | `0` |
| `std_collections_managed_arc` | Copy/mutate isolation para `grid2d<text>`, `pqueue<text>`, `circbuf<text>`, `btreemap<text,text>`, `btreeset<text>` e `grid3d<text>` | `12` |
| `std_collections_queue_stack_cow` | `queue/stack` com retorno estruturado (`colecao + item`) e isolamento por copia em `dequeue/pop` | `0` |
| `optional_result_basic` | 
one`, `success(...)` and `error(...)` | `0` |
| `result_question_basic` | `result<T,E>` `?` propagation in const/var initialization | `0` |
| `optional_question_basic` | `optional<T>` `?` propagation with `none` short-circuit in const initialization | `0` |
| `edge_boundaries_empty` | Edge values: `u8/u16/u32/u64` bounds, near `int` limits, and empty `text/list/map/bytes` invariants | `0` |
| `bytes_hex_literal` | `hex bytes "..."`, `len(bytes)`, byte indexing and byte slicing | `9` |
| `std_bytes_utf8` | `std.bytes.empty`, `std.text.to_utf8`, `std.text.from_utf8` and UTF-8 failure path | `14` |
| `std_bytes_ops` | `std.bytes.from_list`, `std.bytes.to_list`, `std.bytes.join`, `std.bytes.starts_with`, `std.bytes.ends_with` and `std.bytes.contains` | `7` |
| `std_validate_basic` | `std.validate` baseline predicates (`between`, `one_of`, `one_of_text`, text length checks) | `42` |
| `std_small_helpers` | R6 small helper set for `std.validate`, `std.text`, `std.list` and `std.map` | `0` |
| `std_math_basic` | `std.math` baseline (`abs`, `min`, `max`, `clamp`, `deg_to_rad`, `approx_equal`) | `22` |
| `std_random_basic` | `std.random` baseline (`seed`, `next`, `between`) plus `public var` state tracking | `0` |
| `std_random_state_observability` | `std.random` public state observability (`seeded`, `last_seed`, `draw_count`, `stats`) | `0` |
| `std_random_between_branches` | `std.random.between` branch behavior (`min == max`, `max < min`) with draw count invariants | `0` |
| `std_format_basic` | `std.format` com `BytesStyle` tipado (`hex`, `bin`, `bytes(style: ...)`, `bytes_binary`, `bytes_decimal`) | `0` |
| `fmt_interpolation_basic` | `fmt "..."` end-to-end com expressao, chamada, bool e escape de chaves | `0` |
| `to_text_builtin_basic` | Builtin `to_text(value)` via `TextRepresentable` para `int` e `bool` | `0` |
| `todo_builtin_fail` | Builtin `todo(message)` fatal path | `runtime.todo` |
| `unreachable_builtin_fail` | Builtin `unreachable(message)` fatal path | `runtime.unreachable` |
| `check_intrinsic_message_fail` | Builtin `check(condition, message)` fatal path with custom message | `runtime.check` |
| `std_fs_basic` | `std.fs` baseline (`write_text`, `exists`, `read_text`) via host runtime wrappers | `check-pass` |
| `std_fs_ops_basic` | `std.fs` create/list/metadata/copy/move/remove com caminhos reais | `0` |
| `std_fs_path_basic` | `std.fs.path` baseline (`join`, `base`, `dir`, `ext`, 
ame_without_extension`, `has_ext`, `change_ext`, 
ormalize`, `absolute`, `relative`, `is_absolute`, `is_relative`) via compile-probe | `0` |
| `std_json_basic` | `std.json` baseline (`parse`, `stringify`, `pretty`) para objeto plano `map<text,text>` | `0` |
| `std_test_basic` | `std.test` helper direto em `main` (`skip` => skipped outcome, `fail` => failed outcome) | `test-skip` |
| `std_test_attr_pass_skip` | `zt test` com `attr test` exercitando 1 pass e 1 skip | `test ok (pass=1 skip=1)` |
| `std_test_attr_fail` | `zt test` com `attr test` exercitando 1 pass, 1 skip e 1 fail | `test failed (pass=1 skip=1 fail=1)` |
| `std_test_helpers_pass` | `std.test` helper assertions no caminho feliz (`is_true`, `is_false`, `equal_*`, `not_equal_*`) | `0` |
| `std_time_basic` | `std.time` tipado (`Instant`, `Duration`, 
ow`, `sleep`, `since`, `until`, conversoes unix) | `0` |
| `std_os_basic` | `std.os` tipado (`Platform`, `Arch`, `pid`, `platform`, `arch`, `env`, `current_dir`, `change_dir`) | `0` |
| `std_os_process_basic` | `std.os.process` com `ExitStatus` tipado (`run`, `exit_code`) e comando explicito (`program` + `args`) | `0` |
| `std_net_basic` | `std.net` TCP client baseline (`connect`, `read_some`, `write_all`, `close`, `is_closed`) em loopback local via `run-loopback.ps1` | `0` |
| `tooling_gate_smoke` | projeto canario para gate de `zt fmt --check` e `zt doc check` no runner oficial | `0` |
| `multifile_import_alias` | Multi-file source root and import alias | `42` |
| `public_const_module` | Top-level `public const` imported via alias (`module.CONST`) | `42` |
| `public_var_module` | Top-level `public var` imported via alias (`module.VAR`) | `42` |
| `public_var_module_state` | `public var` shares state across functions in the owning module | `4` |
| `readability_block_depth_pass` | Readability warning (`style.block_too_deep`) is reported but does not block normal run | `0` |
| `readability_enum_default_pass` | Readability warning (`control_flow.enum_default_case`) is reported but does not block normal run | `0` |
| `readability_function_length_pass` | Readability warning (`style.function_too_long`) is reported but does not block normal run | `0` |
| `readability_warnings_pass` | Readability warnings (`name.similar`, `name.confusing`) are reported but do not block normal run | `0` |
| `closure_capture_basic` | Anonymous closure with immutable by-value capture | `0` |
| `lambda_hof_basic` | Lambda sugar `func(...) => expr` with `map_int`, `filter_int`, `reduce_int` and immutable capture | `0` |
| `lazy_explicit_order_basic` | Explicit `lazy<int>` runs the thunk only on `force_int` | `0` |
| `borealis_backend_fallback_stub` | Borealis desktop-profile request (`backend_id=1`) with safe fallback to stub when adapter is unavailable; covers window + draw + input queries | `0` |
| `borealis_raylib_binding_stub` | `borealis.raylib` binding smoke in stub-safe mode: shapes, text, input, `measure_text`, `raymath` helpers, easing functions, `require_available()`, clear empty-path errors for texture/sound and stub-safe texture draw fallback | `0` |
| `borealis_raylib_assets_real` | `borealis.raylib` real-assets probe: conditional native `.png`/`.wav` loading, texture metadata, texture draw, audio init and `load/play/stop/unload` when Raylib is available | `0` |
| `borealis_foundations_stub` | Borealis foundations smoke: typed asset loaders, asset metadata/stable ids, typed events, save, storage, services, database, UI/HUD widgets, editor metadata and persistent settings coverage, including empty-string persistence semantics, settings profiles and clear key-kind conflicts | `0` |
| `borealis_ecs_hybrid_stub` | Borealis ECS hybrid (stub run-pass): component store API em `borealis.engine.ecs` | `0` |
| `borealis_runtime_gameplay_stub` | Borealis runtime/gameplay smoke: contracts, entities, movement, controllers, vehicles, animation, audio, ai, camera, input, world and procedural working together | `0` |
| `where_contracts_ok` | Runtime `where` contracts on parameter, struct construction and field assignment | `40` |

## Invalid Projects

| Project | Expected diagnostic |
| --- | --- |
| `error_syntax` | Parser span and expectation text |
| `error_type_mismatch` | Semantic type mismatch span |
| `check_intrinsic_message_fail` | Runtime check failure preserves caller message |
| `functions_main_signature_error` | C entrypoint signature restriction |
| `functions_invalid_call_error` | Missing argument diagnostic with source span |
| `multifile_missing_import` | Missing import rejection |
| `multifile_namespace_mismatch` | Namespace/path mismatch rejection |
| `multifile_duplicate_symbol` | Duplicate effective symbol rejection |
| `multifile_import_cycle` | Import cycle rejection |
| `multifile_private_access` | Access to non-public symbol via import alias is rejected |
| `public_var_cross_namespace_write_error` | Cross-namespace mutation of `public var` via import alias is rejected |
| `readability_block_depth_strict_error` | Strict diagnostics profile promotes block-depth warning to error |
| `readability_enum_default_strict_error` | Strict diagnostics profile promotes enum-default warning to error |
| `readability_function_length_strict_error` | Strict diagnostics profile promotes function-length warning to error |
| `readability_warnings_strict_error` | Strict diagnostics profile promotes name readability warnings to error |
| `closure_mut_capture_error` | Mutation of a captured closure variable is rejected |
| `dyn_generic_trait_error` | Generic trait rejected as `dyn<Trait>` with generic/where guidance |
| `lambda_return_mismatch_error` | Lambda return must match the expected `func(...) -> ...` type |
| `lazy_reuse_error` | Runtime contract rejects forcing the same one-shot `lazy<int>` twice |
| `std_concurrent_boundary_copy_unsupported_error` | `std.concurrent.copy_text(...)` rejects values outside the accepted payload type |
| `std_random_cross_namespace_write_error` | Cross-namespace mutation of `std.random` `public var` via import alias is rejected |
| `project_unknown_key_manifest` | Manifest unknown key diagnostic (`project.*`) |
| `fmt_interpolation_type_error` | `fmt` rejeita tipo sem `TextRepresentable<T>` |
| `monomorphization_limit_error` | Monomorphization gate diagnostic when generic instantiations exceed `build.monomorphization_limit` |
| `mutability_const_reassign_error` | Const reassignment mutability diagnostic |
| `noncanonical_string_error` | Suggests `text` instead of `string` |
| `noncanonical_let_error` | Suggests `const` or `var` instead of `let` |
| `noncanonical_and_error` | Suggests `and` instead of `&&` |
| `noncanonical_or_error` | Suggests `or` instead of `||` |
| `noncanonical_not_error` | Suggests `not value` instead of `!value` |
| `noncanonical_null_error` | Suggests `optional<T>` and `none` instead of `null` |
| `noncanonical_throw_error` | Suggests `result<T,E>`, `error(...)`, or `panic(...)` instead of `throw` |
| `noncanonical_abstract_error` | Suggests `trait` instead of `abstract` |
| `noncanonical_virtual_error` | Suggests `dyn<Trait>` instead of `virtual` |
| `noncanonical_union_error` | Suggests `enum` with payload instead of `union` |
| `noncanonical_partial_error` | Suggests `apply` and namespace/file organization instead of `partial` |
| `optional_question_outside_optional_error` | `optional<T>?` rejected outside `optional<U>` return context |
| `result_optional_propagation_error` | `?` propagation rejected outside `result<T,E>` return context |
| `runtime_index_error` | Runtime index diagnostic from C runtime guard |
| `std_test_helpers_bool_fail` | Runtime test diagnostic from `test.is_true(false)` |
| `std_test_helpers_equal_fail` | Runtime test diagnostic with expected/received values from `test.equal_int(...)` |
| `std_test_helpers_not_equal_fail` | Runtime test diagnostic from `test.not_equal_text(...)` |
| `todo_builtin_fail` | Runtime todo diagnostic from `todo(message)` |
| `unreachable_builtin_fail` | Runtime unreachable diagnostic from `unreachable(message)` |
| `where_contract_param_error` | Runtime contract violation on parameter `where` |
| `where_contract_construct_error` | Runtime contract violation on struct construction `where` |
| `where_contract_field_assign_error` | Runtime contract violation on field assignment `where` |

## Deferred Surface Forms

These forms remain accepted language direction but are not in the M16 executable behavior matrix:

- Generic collection iteration beyond the C backend combinations already covered by behavior tests.
- Full generic monomorphization beyond the current checked semantic model.
- Enum value construction and exhaustive enum matching in generated C (semantic coverage exists in `tests/semantic`; check path is validated with fixtures `tests/behavior/enum_match` / `tests/behavior/enum_match_non_exhaustive_error`; full build E2E remains blocked while `compiler/zir/lowering/from_hir.c` is a stub in source).
- Broader stdlib-facing collection APIs beyond the current compiler intrinsic `len(...)`.







