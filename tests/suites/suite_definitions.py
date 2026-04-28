"""
R2.M1 - Suite definitions for Zenith test infrastructure 2.0.

Suites:
  smoke     - fast sanity check (~30s), run on every local change
  pr_gate   - full correctness gate (~3min), required before merge
  nightly   - extended coverage + perf (~15min), run on CI nightly
  stress    - long-running stability + fuzz replay (~60min+), scheduled

Each suite entry:
  id        - unique test identifier
  layer     - compiler layer for per-layer reporting
  kind      - "behavior" | "unit" | "formatter" | "perf" | "stdlib"
  suite     - list of suite names this test belongs to
  timeout   - max seconds for this test
  seed      - stable seed for reproducibility (fuzz/stress only)
"""

LAYER_FRONTEND = "frontend"
LAYER_SEMANTIC = "semantic"
LAYER_HIR_ZIR = "hir_zir"
LAYER_BACKEND = "backend"
LAYER_RUNTIME = "runtime"
LAYER_STDLIB = "stdlib"
LAYER_TOOLING = "tooling"
LAYER_PERF = "perf"

SUITE_SMOKE = "smoke"
SUITE_PR_GATE = "pr_gate"
SUITE_NIGHTLY = "nightly"
SUITE_STRESS = "stress"

# ---------------------------------------------------------------------------
# Time limits per suite (seconds)
# ---------------------------------------------------------------------------
SUITE_TIME_LIMITS = {
    SUITE_SMOKE: 60,
    SUITE_PR_GATE: 300,
    SUITE_NIGHTLY: 1200,
    SUITE_STRESS: 7200,
}

# ---------------------------------------------------------------------------
# Stable seeds for reproducible fuzz/stress replay
# ---------------------------------------------------------------------------
STABLE_SEEDS = {
    "fuzz_lexer": 20260421,
    "fuzz_parser": 20260421,
    "fuzz_semantic": 20260423,
    "stress_rt": 20260421,
}

# ---------------------------------------------------------------------------
# Behavior test classification
# ---------------------------------------------------------------------------

# Tests that must fail at check stage
BEHAVIOR_CHECK_FAIL = {
    "callable_escape_container_error",
    "callable_escape_public_var_error",
    "callable_escape_struct_field_error",
    "callable_invalid_func_ref_error",
    "callable_signature_mismatch_error",
    "check_intrinsic_type_error",
    "closure_mut_capture_error",
    "dyn_generic_trait_error",
    "enum_match_non_exhaustive_error",
    "error_syntax",
    "error_type_mismatch",
    "extern_c_struct_arg_error",
    "fmt_interpolation_type_error",
    "functions_invalid_call_error",
    "functions_param_ordering_error",
    "list_first_type_error",
    "list_last_type_error",
    "list_rest_type_error",
    "list_skip_type_error",
    "lambda_return_mismatch_error",
    "multifile_duplicate_symbol",
    "multifile_import_cycle",
    "multifile_missing_import",
    "multifile_namespace_mismatch",
    "methods_mutating_const_receiver_error",
    "mutability_const_reassign_error",
    "noncanonical_abstract_error",
    "noncanonical_and_error",
    "noncanonical_let_error",
    "noncanonical_not_error",
    "noncanonical_null_error",
    "noncanonical_or_error",
    "noncanonical_partial_error",
    "noncanonical_string_error",
    "noncanonical_throw_error",
    "noncanonical_union_error",
    "noncanonical_virtual_error",
    "public_var_cross_namespace_write_error",
    "readability_block_depth_strict_error",
    "readability_enum_default_strict_error",
    "readability_function_length_strict_error",
    "readability_warnings_strict_error",
    "set_empty_inference_error",
    "set_mutation_const_error",
    "set_operation_type_error",
    "std_concurrent_boundary_copy_unsupported_error",
    "std_random_cross_namespace_write_error",
    "optional_question_outside_optional_error",
    "project_unknown_key_manifest",
    "monomorphization_limit_error",
    "where_contract_param_where_invalid_error",
    "where_contract_param_where_non_bool_error",
}

# Tests that must fail at build stage
BEHAVIOR_BUILD_FAIL = {
    "functions_main_signature_error",
    "result_optional_propagation_error",
}

# Tests that must fail at run stage
BEHAVIOR_RUN_FAIL = {
    "check_intrinsic_message_fail",
    "lazy_reuse_error",
    "runtime_index_error",
    "std_test_helpers_bool_fail",
    "std_test_helpers_equal_fail",
    "std_test_helpers_not_equal_fail",
    "std_test_throws_fail",
    "todo_builtin_fail",
    "unreachable_builtin_fail",
    "where_contract_construct_error",
    "where_contract_field_assign_error",
    "where_contract_param_error",
}

# Tests that must pass through run stage
BEHAVIOR_RUN_PASS = {
    "borealis_backend_fallback_stub",
    "borealis_foundations_stub",
    "borealis_input_transitions_stub",
    "borealis_render2d_stub",
    "borealis_scene_entities_stub",
    "closure_capture_basic",
    "extern_c_text_len_e2e",
    "std_concurrent_boundary_copy_determinism",
    "borealis_ecs_hybrid_stub",
    "extern_c_puts_e2e",
    "fmt_interpolation_basic",
    "lambda_hof_basic",
    "lazy_explicit_order_basic",
    "list_first_basic",
    "list_float_primitive_storage",
    "list_last_basic",
    "list_primitive_numeric_matrix",
    "list_rest_basic",
    "list_skip_basic",
    "edge_boundaries_empty",
    "optional_result_helpers_absence_error",
    "optional_result_helpers_pass",
    "optional_or_return_basic",
    "optional_primitive_specialized",
    "optional_question_basic",
    "readability_block_depth_pass",
    "readability_enum_default_pass",
    "readability_function_length_pass",
    "readability_warnings_pass",
    "set_core_api_basic",
    "set_iteration_basic",
    "set_operations_basic",
    "std_collections_queue_stack_cow",
    "std_console_basic",
    "std_fs_aliases_basic",
    "std_math_basic",
    "std_regex_basic",
    "std_small_helpers",
    "std_time_basic",
    "syntax_coherence_core",
    "syntax_coherence_inline_constraints",
    "result_or_wrap_basic",
    "std_test_helpers_pass",
    "std_test_throws_pass",
    "list_dyn_textrepresentable",
    "to_text_builtin_basic",
    "range_builtin_basic",
    "using_basic",
}

# Optional diagnostic fragment files for invalid behavior tests.
# If mapped here, runners should require all non-empty lines from the file.
BEHAVIOR_DIAGNOSTIC_FRAGMENT_FILES = {
    "callable_escape_container_error": "tests/fixtures/diagnostics/callable_escape_container_error.contains.txt",
    "callable_escape_public_var_error": "tests/fixtures/diagnostics/callable_escape_public_var_error.contains.txt",
    "callable_escape_struct_field_error": "tests/fixtures/diagnostics/callable_escape_struct_field_error.contains.txt",
    "callable_invalid_func_ref_error": "tests/fixtures/diagnostics/callable_invalid_func_ref_error.contains.txt",
    "callable_signature_mismatch_error": "tests/fixtures/diagnostics/callable_signature_mismatch_error.contains.txt",
    "check_intrinsic_type_error": "tests/fixtures/diagnostics/check_intrinsic_type_error.contains.txt",
    "check_intrinsic_message_fail": "tests/fixtures/diagnostics/check_intrinsic_message_fail.contains.txt",
    "closure_mut_capture_error": "tests/fixtures/diagnostics/closure_mut_capture_error.contains.txt",
    "dyn_generic_trait_error": "tests/fixtures/diagnostics/dyn_generic_trait_error.contains.txt",
    "enum_match_non_exhaustive_error": "tests/fixtures/diagnostics/enum_match_non_exhaustive.contains.txt",
    "error_syntax": "tests/fixtures/diagnostics/error_syntax.contains.txt",
    "error_type_mismatch": "tests/fixtures/diagnostics/error_type_mismatch.contains.txt",
    "extern_c_struct_arg_error": "tests/fixtures/diagnostics/extern_c_struct_arg_error.contains.txt",
    "fmt_interpolation_type_error": "tests/fixtures/diagnostics/fmt_interpolation_type_error.contains.txt",
    "functions_invalid_call_error": "tests/fixtures/diagnostics/functions_invalid_call.contains.txt",
    "functions_main_signature_error": "tests/fixtures/diagnostics/functions_main_signature.contains.txt",
    "functions_param_ordering_error": "tests/fixtures/diagnostics/functions_param_ordering_error.contains.txt",
    "lambda_return_mismatch_error": "tests/fixtures/diagnostics/lambda_return_mismatch_error.contains.txt",
    "list_first_type_error": "tests/fixtures/diagnostics/list_first_type_error.contains.txt",
    "list_last_type_error": "tests/fixtures/diagnostics/list_last_type_error.contains.txt",
    "list_rest_type_error": "tests/fixtures/diagnostics/list_rest_type_error.contains.txt",
    "list_skip_type_error": "tests/fixtures/diagnostics/list_skip_type_error.contains.txt",
    "lazy_reuse_error": "tests/fixtures/diagnostics/lazy_reuse_error.contains.txt",
    "monomorphization_limit_error": "tests/fixtures/diagnostics/monomorphization_limit_error.contains.txt",
    "multifile_duplicate_symbol": "tests/fixtures/diagnostics/multifile_duplicate_symbol.contains.txt",
    "multifile_import_cycle": "tests/fixtures/diagnostics/multifile_import_cycle.contains.txt",
    "multifile_missing_import": "tests/fixtures/diagnostics/multifile_missing_import.contains.txt",
    "multifile_namespace_mismatch": "tests/fixtures/diagnostics/multifile_namespace_mismatch.contains.txt",
    "methods_mutating_const_receiver_error": "tests/fixtures/diagnostics/methods_mutating_const_receiver_error.contains.txt",
    "mutability_const_reassign_error": "tests/fixtures/diagnostics/mutability_const_reassign.contains.txt",
    "noncanonical_abstract_error": "tests/fixtures/diagnostics/noncanonical_abstract_error.contains.txt",
    "noncanonical_and_error": "tests/fixtures/diagnostics/noncanonical_and_error.contains.txt",
    "noncanonical_let_error": "tests/fixtures/diagnostics/noncanonical_let_error.contains.txt",
    "noncanonical_not_error": "tests/fixtures/diagnostics/noncanonical_not_error.contains.txt",
    "noncanonical_null_error": "tests/fixtures/diagnostics/noncanonical_null_error.contains.txt",
    "noncanonical_or_error": "tests/fixtures/diagnostics/noncanonical_or_error.contains.txt",
    "noncanonical_partial_error": "tests/fixtures/diagnostics/noncanonical_partial_error.contains.txt",
    "noncanonical_string_error": "tests/fixtures/diagnostics/noncanonical_string_error.contains.txt",
    "noncanonical_throw_error": "tests/fixtures/diagnostics/noncanonical_throw_error.contains.txt",
    "noncanonical_union_error": "tests/fixtures/diagnostics/noncanonical_union_error.contains.txt",
    "noncanonical_virtual_error": "tests/fixtures/diagnostics/noncanonical_virtual_error.contains.txt",
    "public_var_cross_namespace_write_error": "tests/fixtures/diagnostics/public_var_cross_namespace_write_error.contains.txt",
    "readability_block_depth_strict_error": "tests/fixtures/diagnostics/readability_block_depth_strict_error.contains.txt",
    "readability_enum_default_strict_error": "tests/fixtures/diagnostics/readability_enum_default_strict_error.contains.txt",
    "readability_function_length_strict_error": "tests/fixtures/diagnostics/readability_function_length_strict_error.contains.txt",
    "readability_warnings_strict_error": "tests/fixtures/diagnostics/readability_warnings_strict_error.contains.txt",
    "set_empty_inference_error": "tests/fixtures/diagnostics/set_empty_inference_error.contains.txt",
    "set_mutation_const_error": "tests/fixtures/diagnostics/set_mutation_const_error.contains.txt",
    "set_operation_type_error": "tests/fixtures/diagnostics/set_operation_type_error.contains.txt",
    "std_concurrent_boundary_copy_unsupported_error": "tests/fixtures/diagnostics/std_concurrent_boundary_copy_unsupported_error.contains.txt",
    "optional_question_outside_optional_error": "tests/fixtures/diagnostics/optional_question_outside_optional_error.contains.txt",
    "project_unknown_key_manifest": "tests/fixtures/diagnostics/project_unknown_key_manifest.contains.txt",
    "result_optional_propagation_error": "tests/fixtures/diagnostics/result_optional_propagation.contains.txt",
    "runtime_index_error": "tests/fixtures/diagnostics/runtime_index_error.contains.txt",
    "std_random_cross_namespace_write_error": "tests/fixtures/diagnostics/std_random_cross_namespace_write_error.contains.txt",
    "std_test_helpers_bool_fail": "tests/fixtures/diagnostics/std_test_helpers_bool_fail.contains.txt",
    "std_test_helpers_equal_fail": "tests/fixtures/diagnostics/std_test_helpers_equal_fail.contains.txt",
    "std_test_helpers_not_equal_fail": "tests/fixtures/diagnostics/std_test_helpers_not_equal_fail.contains.txt",
    "std_test_throws_fail": "tests/fixtures/diagnostics/std_test_throws_fail.contains.txt",
    "todo_builtin_fail": "tests/fixtures/diagnostics/todo_builtin_fail.contains.txt",
    "unreachable_builtin_fail": "tests/fixtures/diagnostics/unreachable_builtin_fail.contains.txt",
    "where_contract_construct_error": "tests/fixtures/diagnostics/where_contract_construct_error.contains.txt",
    "where_contract_field_assign_error": "tests/fixtures/diagnostics/where_contract_field_assign_error.contains.txt",
    "where_contract_param_error": "tests/fixtures/diagnostics/where_contract_param_error.contains.txt",
    "where_contract_param_where_invalid_error": "tests/fixtures/diagnostics/where_contract_param_where_invalid_error.contains.txt",
    "where_contract_param_where_non_bool_error": "tests/fixtures/diagnostics/where_contract_param_where_non_bool_error.contains.txt",
}

# ---------------------------------------------------------------------------
# Suite membership for behavior tests
# Smoke: small representative subset; pr_gate: all; nightly: all + extras
# ---------------------------------------------------------------------------
BEHAVIOR_SMOKE_SUBSET = {
    "simple_app",
    "set_iteration_basic",
    "set_operations_basic",
    "set_core_api_basic",
    "set_empty_inference_error",
    "set_mutation_const_error",
    "set_operation_type_error",
    "std_small_helpers",
    "std_io_basic",
    "std_collections_queue_stack_cow",
    "std_console_basic",
    "std_fs_aliases_basic",
    "std_math_basic",
    "std_regex_basic",
    "std_time_basic",
    "list_dyn_textrepresentable",
    "result_question_basic",
    "result_or_wrap_basic",
    "optional_result_helpers_pass",
    "optional_or_return_basic",
    "optional_primitive_specialized",
    "optional_result_basic",
    "enum_match_non_exhaustive_error",
    "error_type_mismatch",
    "functions_invalid_call_error",
    "list_first_basic",
    "list_first_type_error",
    "list_float_primitive_storage",
    "list_last_basic",
    "list_last_type_error",
    "list_primitive_numeric_matrix",
    "list_rest_basic",
    "list_rest_type_error",
    "list_skip_basic",
    "list_skip_type_error",
    "multifile_import_alias",
    "where_contract_param_error",
}


def behavior_suites(name: str) -> list:
    """Return list of suites a behavior test belongs to."""
    suites = [SUITE_PR_GATE, SUITE_NIGHTLY]
    if name in BEHAVIOR_SMOKE_SUBSET:
        suites = [SUITE_SMOKE] + suites
    return suites


def behavior_timeout(name: str, suite: str) -> int:
    """Return timeout in seconds for a behavior test in a given suite."""
    if suite == SUITE_SMOKE:
        return 30
    if (
        name in BEHAVIOR_BUILD_FAIL
        or name in BEHAVIOR_RUN_FAIL
        or name in BEHAVIOR_RUN_PASS
    ):
        return 120
    return 45
