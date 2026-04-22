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

LAYER_FRONTEND   = "frontend"
LAYER_SEMANTIC   = "semantic"
LAYER_HIR_ZIR    = "hir_zir"
LAYER_BACKEND    = "backend"
LAYER_RUNTIME    = "runtime"
LAYER_STDLIB     = "stdlib"
LAYER_TOOLING    = "tooling"
LAYER_PERF       = "perf"

SUITE_SMOKE    = "smoke"
SUITE_PR_GATE  = "pr_gate"
SUITE_NIGHTLY  = "nightly"
SUITE_STRESS   = "stress"

# ---------------------------------------------------------------------------
# Time limits per suite (seconds)
# ---------------------------------------------------------------------------
SUITE_TIME_LIMITS = {
    SUITE_SMOKE:   60,
    SUITE_PR_GATE: 300,
    SUITE_NIGHTLY: 1200,
    SUITE_STRESS:  7200,
}

# ---------------------------------------------------------------------------
# Stable seeds for reproducible fuzz/stress replay
# ---------------------------------------------------------------------------
STABLE_SEEDS = {
    "fuzz_lexer":  20260421,
    "fuzz_parser": 20260421,
    "stress_rt":   20260421,
}

# ---------------------------------------------------------------------------
# Behavior test classification
# ---------------------------------------------------------------------------

# Tests that must fail at check stage
BEHAVIOR_CHECK_FAIL = {
    "check_intrinsic_type_error",
    "enum_match_non_exhaustive_error",
    "error_syntax",
    "error_type_mismatch",
    "fmt_interpolation_type_error",
    "functions_invalid_call_error",
    "functions_param_ordering_error",
    "multifile_duplicate_symbol",
    "multifile_import_cycle",
    "multifile_missing_import",
    "multifile_namespace_mismatch",
    "mutability_const_reassign_error",
    "public_var_cross_namespace_write_error",
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
    "runtime_index_error",
    "where_contract_construct_error",
    "where_contract_field_assign_error",
    "where_contract_param_error",
}

# Tests that must pass through run stage
BEHAVIOR_RUN_PASS = {
    "borealis_backend_fallback_stub",
    "borealis_ecs_hybrid_stub",
    "extern_c_puts_e2e",
    "fmt_interpolation_basic",
    "edge_boundaries_empty",
    "optional_question_basic",
}

# Optional diagnostic fragment files for invalid behavior tests.
# If mapped here, runners should require all non-empty lines from the file.
BEHAVIOR_DIAGNOSTIC_FRAGMENT_FILES = {
    "check_intrinsic_type_error": "tests/fixtures/diagnostics/check_intrinsic_type_error.contains.txt",
    "enum_match_non_exhaustive_error": "tests/fixtures/diagnostics/enum_match_non_exhaustive.contains.txt",
    "error_syntax": "tests/fixtures/diagnostics/error_syntax.contains.txt",
    "error_type_mismatch": "tests/fixtures/diagnostics/error_type_mismatch.contains.txt",
    "fmt_interpolation_type_error": "tests/fixtures/diagnostics/fmt_interpolation_type_error.contains.txt",
    "functions_invalid_call_error": "tests/fixtures/diagnostics/functions_invalid_call.contains.txt",
    "functions_main_signature_error": "tests/fixtures/diagnostics/functions_main_signature.contains.txt",
    "functions_param_ordering_error": "tests/fixtures/diagnostics/functions_param_ordering_error.contains.txt",
    "monomorphization_limit_error": "tests/fixtures/diagnostics/monomorphization_limit_error.contains.txt",
    "multifile_duplicate_symbol": "tests/fixtures/diagnostics/multifile_duplicate_symbol.contains.txt",
    "multifile_import_cycle": "tests/fixtures/diagnostics/multifile_import_cycle.contains.txt",
    "multifile_missing_import": "tests/fixtures/diagnostics/multifile_missing_import.contains.txt",
    "multifile_namespace_mismatch": "tests/fixtures/diagnostics/multifile_namespace_mismatch.contains.txt",
    "mutability_const_reassign_error": "tests/fixtures/diagnostics/mutability_const_reassign.contains.txt",
    "public_var_cross_namespace_write_error": "tests/fixtures/diagnostics/public_var_cross_namespace_write_error.contains.txt",
    "optional_question_outside_optional_error": "tests/fixtures/diagnostics/optional_question_outside_optional_error.contains.txt",
    "project_unknown_key_manifest": "tests/fixtures/diagnostics/project_unknown_key_manifest.contains.txt",
    "result_optional_propagation_error": "tests/fixtures/diagnostics/result_optional_propagation.contains.txt",
    "runtime_index_error": "tests/fixtures/diagnostics/runtime_index_error.contains.txt",
    "std_random_cross_namespace_write_error": "tests/fixtures/diagnostics/std_random_cross_namespace_write_error.contains.txt",
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
    "std_io_basic",
    "result_question_basic",
    "optional_result_basic",
    "enum_match_non_exhaustive_error",
    "error_type_mismatch",
    "functions_invalid_call_error",
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
    if name in BEHAVIOR_BUILD_FAIL or name in BEHAVIOR_RUN_FAIL or name in BEHAVIOR_RUN_PASS:
        return 120
    return 45
