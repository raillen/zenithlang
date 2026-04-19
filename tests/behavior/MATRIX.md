# Zenith Next Behavior Matrix

This matrix is the M16 executable coverage map for the current C backend cut.

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
| `list_slice_len` | `list<int>` slice and `len(list)` | `37` |
| `text_slice_len` | `text` slice and `len(text)` | `8` |
| `map_basic` | `map<text,text>` literal, index and update | `0` |
| `map_safe_get` | Safe lookup `map.get(key) -> optional<text>` sem panic em chave ausente | `15` |
| `list_safe_get` | Safe lookup `list.get(index) -> optional<int>` sem panic em indice ausente | `27` |
| `map_len_basic` | `len(map<text,text>)` | `2` |
| `value_semantics_collections` | Copy/mutate isolation for `list` and `map` via COW in runtime/backend | `131` |
| `value_semantics_struct_managed` | Copy/mutate isolation para struct com campos `list/map` via rebind COW | `131` |
| `value_semantics_arc_isolation` | Chain-copy isolation (`a -> b -> c`) for `list` and `map` under COW/RC | `158` |
| `value_semantics_optional_result_managed` | `optional<list<int>>` creation/copy and `result<list<int>, text>` `?` with COW-safe list mutation | `0` |
| `optional_result_basic` | `none`, `success(...)` and `error(...)` | `0` |
| `result_question_basic` | `result<T,E>` `?` propagation in const/var initialization | `0` |
| `bytes_hex_literal` | `hex bytes "..."`, `len(bytes)`, byte indexing and byte slicing | `9` |
| `std_bytes_utf8` | `std.bytes.empty`, `std.text.to_utf8`, `std.text.from_utf8` and UTF-8 failure path | `14` |
| `std_bytes_ops` | `std.bytes.from_list`, `std.bytes.to_list`, `std.bytes.join`, `std.bytes.starts_with`, `std.bytes.ends_with` and `std.bytes.contains` | `7` |
| `multifile_import_alias` | Multi-file source root and import alias | `42` |
| `where_contracts_ok` | Runtime `where` contracts on parameter, struct construction and field assignment | `40` |

## Invalid Projects

| Project | Expected diagnostic |
| --- | --- |
| `error_syntax` | Parser span and expectation text |
| `error_type_mismatch` | Semantic type mismatch span |
| `functions_main_signature_error` | C entrypoint signature restriction |
| `functions_invalid_call_error` | Missing argument diagnostic with source span |
| `multifile_missing_import` | Missing import rejection |
| `multifile_namespace_mismatch` | Namespace/path mismatch rejection |
| `multifile_duplicate_symbol` | Duplicate effective symbol rejection |
| `multifile_import_cycle` | Import cycle rejection |
| `project_unknown_key_manifest` | Manifest unknown key diagnostic (`project.*`) |
| `monomorphization_limit_error` | Monomorphization gate diagnostic when generic instantiations exceed `build.monomorphization_limit` |
| `mutability_const_reassign_error` | Const reassignment mutability diagnostic |
| `result_optional_propagation_error` | `?` propagation rejected outside `result<T,E>` return context |
| `runtime_index_error` | Runtime index diagnostic from C runtime guard |
| `where_contract_param_error` | Runtime contract violation on parameter `where` |
| `where_contract_construct_error` | Runtime contract violation on struct construction `where` |
| `where_contract_field_assign_error` | Runtime contract violation on field assignment `where` |

## Deferred Surface Forms

These forms remain accepted language direction but are not in the M16 executable behavior matrix:

- Generic collection iteration beyond the C backend combinations already covered by behavior tests.
- Optional `?` propagation and expression-level unwrap outside the current `result<T,E>` const/var initialization subset.
- Full generic monomorphization beyond the current checked semantic model.
- Enum value construction and exhaustive enum matching in generated C (semantic coverage exists in `tests/semantic`; check path is validated with fixtures `tests/behavior/enum_match` / `tests/behavior/enum_match_non_exhaustive_error`; full build E2E remains blocked while `compiler/zir/lowering/from_hir.c` is a stub in source).
- Broader stdlib-facing collection APIs beyond the current compiler intrinsic `len(...)`.



