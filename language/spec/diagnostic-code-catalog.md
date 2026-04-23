# Zenith Diagnostic Code Catalog

- Status: canonical catalog for current alpha implementation
- Date: 2026-04-23
- Source of truth: compiler diagnostics come from `compiler/semantic/diagnostics/diagnostics.h` and `compiler/semantic/diagnostics/diagnostics.c`; runtime codes come from `runtime/c/zenith_rt.h` and `runtime/c/zenith_rt.c`

## Purpose

Define stable diagnostic codes rendered by the detailed terminal diagnostics format.

## Project/Manifest Codes

| Internal enum | Stable code | Default help focus |
| --- | --- | --- |
| `ZT_DIAG_PROJECT_ERROR` | `project.error` | Check project structure and manifest values |
| `ZT_DIAG_PROJECT_IMPORT_CYCLE` | `project.import_cycle` | Remove circular imports |
| `ZT_DIAG_PROJECT_MISSING_ENTRY` | `project.missing_entry` | Provide a valid `app.entry` namespace |
| `ZT_DIAG_PROJECT_INVALID_NAMESPACE` | `project.invalid_namespace` | Align namespace and directory layout |
| `ZT_DIAG_PROJECT_UNRESOLVED_IMPORT` | `project.unresolved_import` | Ensure imported namespace exists |
| `ZT_DIAG_PROJECT_INVALID_INPUT` | `project.invalid_input` | Check path and manifest readability |
| `ZT_DIAG_PROJECT_INVALID_SECTION` | `project.invalid_section` | Use supported manifest sections |
| `ZT_DIAG_PROJECT_UNKNOWN_KEY` | `project.unknown_key` | Use supported keys per section |
| `ZT_DIAG_PROJECT_INVALID_ASSIGNMENT` | `project.invalid_assignment` | Use valid `key = value` assignments |
| `ZT_DIAG_PROJECT_INVALID_STRING` | `project.invalid_string` | Use valid quoted strings |
| `ZT_DIAG_PROJECT_MISSING_FIELD` | `project.missing_field` | Provide required fields |
| `ZT_DIAG_PROJECT_INVALID_KIND` | `project.invalid_kind` | Set `project.kind` to `app` or `lib` |
| `ZT_DIAG_PROJECT_INVALID_TARGET` | `project.invalid_target` | Set `build.target` to supported value |
| `ZT_DIAG_PROJECT_INVALID_PROFILE` | `project.invalid_profile` | Set `build.profile` to supported value |
| `ZT_DIAG_PROJECT_PATH_TOO_LONG` | `project.path_too_long` | Shorten path values |
| `ZT_DIAG_PROJECT_TOO_MANY_DEPENDENCIES` | `project.too_many_dependencies` | Reduce dependencies or raise limits |
| `ZT_DIAG_PROJECT_INVALID_MONOMORPHIZATION_LIMIT` | `project.invalid_monomorphization_limit` | Set `build.monomorphization_limit` to a positive integer |
| `ZT_DIAG_PROJECT_MONOMORPHIZATION_LIMIT_EXCEEDED` | `project.monomorphization_limit_exceeded` | Reduce generic instantiations or raise the limit |

## Syntax Codes

| Internal enum | Stable code | Default help focus |
| --- | --- | --- |
| `ZT_DIAG_SYNTAX_ERROR` | `syntax.error` | Review syntax around the span |
| `ZT_DIAG_UNEXPECTED_TOKEN` | `syntax.unexpected_token` | Check separators, delimiters, and ordering |
| `ZT_DIAG_STRUCTURE_LIMIT_EXCEEDED` | `compiler.limit_exceeded` | Split deeply nested constructs into smaller parts |

## Semantic/Binder/Type Codes

| Internal enum | Stable code | Default help focus |
| --- | --- | --- |
| `ZT_DIAG_DUPLICATE_NAME` | `name.duplicate` | Rename one declaration |
| `ZT_DIAG_SHADOWING` | `name.shadowing` | Avoid hiding outer names |
| `ZT_DIAG_UNRESOLVED_NAME` | `name.unresolved` | Declare/import before use |
| `ZT_DIAG_INVALID_CONSTRAINT_TARGET` | `generic.constraint_target` | Constrain only generic parameters |
| `ZT_DIAG_INVALID_TYPE` | `type.invalid` | Check type name and generic arguments |
| `ZT_DIAG_TYPE_MISMATCH` | `type.mismatch` | Convert value or change expected type |
| `ZT_DIAG_INVALID_ASSIGNMENT` | `mutability.invalid_assignment` | Ensure mutable and compatible target |
| `ZT_DIAG_CONST_REASSIGNMENT` | `mutability.const_update` | Use `var` when reassignment is required |
| `ZT_DIAG_INVALID_RETURN` | `control_flow.invalid_return` | Match function return contract |
| `ZT_DIAG_INVALID_CONDITION_TYPE` | `control_flow.invalid_condition` | Conditions must be `bool` |
| `ZT_DIAG_INVALID_CALL` | `type.invalid_call` | Call function values with valid arity |
| `ZT_DIAG_INVALID_ARGUMENT` | `type.invalid_argument` | Check argument names/order/arity |
| `ZT_DIAG_INVALID_OPERATOR` | `type.invalid_operator` | Use operator-compatible operands |
| `ZT_DIAG_INVALID_MAP_KEY_TYPE` | `type.invalid_map_key` | Key must implement required traits |
| `ZT_DIAG_INVALID_MUTATION` | `mutability.invalid_update` | Mutate only mutable targets/receivers or owner-namespace `public var` |
| `ZT_DIAG_INVALID_CONVERSION` | `type.invalid_conversion` | Use supported explicit conversion |
| `ZT_DIAG_INTEGER_OVERFLOW` | `type.integer_overflow` | Use wider type or smaller range |

## ZIR/Backend Codes

| Internal enum | Stable code | Default help focus |
| --- | --- | --- |
| `ZT_DIAG_ZIR_PARSE_ERROR` | `zir.parse` | Fix textual ZIR syntax/structure |
| `ZT_DIAG_ZIR_VERIFY_ERROR` | `zir.verify` | Fix verifier invariants (SSA/CFG/types) |
| `ZT_DIAG_BACKEND_C_EMIT_ERROR` | `backend.c.emit` | Adjust unsupported constructs before C emit |
| `ZT_DIAG_BACKEND_C_LEGALIZE_ERROR` | `backend.c.legalize` | Rewrite sequence expressions to legal C forms |

## ZDoc Codes

| Internal enum | Stable code | Default help focus |
| --- | --- | --- |
| `ZT_DIAG_DOC_MALFORMED_BLOCK` | `doc.malformed_block` | Keep valid `--- ... ---` block structure |
| `ZT_DIAG_DOC_MISSING_TARGET` | `doc.missing_target` | Add `@target` to paired API docs |
| `ZT_DIAG_DOC_MISSING_PAGE` | `doc.missing_page` | Add `@page` to guide docs |
| `ZT_DIAG_DOC_UNRESOLVED_TARGET` | `doc.unresolved_target` | Point to existing symbols only |
| `ZT_DIAG_DOC_UNRESOLVED_LINK` | `doc.unresolved_link` | Fix or remove broken `@link` references |
| `ZT_DIAG_DOC_INVALID_GUIDE_TAG` | `doc.invalid_guide_tag` | Use guide tags only in `zdoc/guides` |
| `ZT_DIAG_DOC_INVALID_PAIRED_TAG` | `doc.invalid_paired_tag` | Use paired tags only in source-paired docs |
| `ZT_DIAG_DOC_MISSING_PUBLIC_DOC` | `doc.missing_public_doc` | Add ZDoc for each public symbol |

## Runtime Structured Codes

Runtime errors emit the same detailed shape (`error[...]`, `where`, `code`, `help`) with these stable codes:

| Runtime kind | Stable code |
| --- | --- |
| `ZT_ERR_ASSERT` | `runtime.assert` |
| `ZT_ERR_CHECK` | `runtime.check` |
| `ZT_ERR_CONTRACT` | `runtime.contract` |
| `ZT_ERR_INDEX` | `runtime.index` |
| `ZT_ERR_IO` | `runtime.io` |
| `ZT_ERR_MATH` | `runtime.math` |
| `ZT_ERR_PANIC` | `runtime.panic` |
| `ZT_ERR_PLATFORM` | `runtime.platform` |
| `ZT_ERR_UNWRAP` | `runtime.unwrap` |

Historical note:

- older proposal names such as `runtime.bounds`, `runtime.divide_by_zero`, `runtime.utf8`, `runtime.allocation`, and `runtime.map_key` are not the current alpha runtime contract.

## Test Runtime Codes

| Runtime kind | Stable code |
| --- | --- |
| `ZT_ERR_TEST_FAILED` | `test.fail` |
| `ZT_ERR_TEST_SKIPPED` | `test.skip` |

## Current Stage Coverage in Renderer

Implemented in detailed renderer path:

- project
- lexer/parser syntax diagnostics
- binder/type/HIR/ZIR lowering diagnostics
- textual ZIR parse/verify diagnostics
- backend C emitter diagnostics
- zdoc diagnostics (`zt doc check`)
- runtime panic/index/assert/check diagnostics

M23 closure: this stage matrix is unified for compiler stages and runtime output formatting.

Also implemented:

- short CI renderer (`--ci`) with one-line stable diagnostics.
