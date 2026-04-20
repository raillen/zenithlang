#include "runtime/c/zenith_rt.h"
#include <stdio.h>


typedef enum zt_app_main__format_BytesStyle_tag {
    zt_app_main__format_BytesStyle__Binary,
    zt_app_main__format_BytesStyle__Decimal
} zt_app_main__format_BytesStyle_tag;

typedef struct zt_app_main__format_BytesStyle {
    zt_app_main__format_BytesStyle_tag tag;
} zt_app_main__format_BytesStyle;
static zt_outcome_void_core_error zt_app_main__main(void);
static zt_app_main__format_BytesStyle zt_app_main__format_style_binary(void);
static zt_app_main__format_BytesStyle zt_app_main__format_style_decimal(void);
static zt_text *zt_app_main__format_hex(zt_int value);
static zt_text *zt_app_main__format_bin(zt_int value);
static zt_text *zt_app_main__format_bytes(zt_int value, zt_app_main__format_BytesStyle style, zt_int decimals);
static zt_text *zt_app_main__format_bytes_binary(zt_int value, zt_int decimals);
static zt_text *zt_app_main__format_bytes_decimal(zt_int value, zt_int decimals);
static zt_outcome_optional_text_core_error zt_app_main__io_read_line(zt_bool from);
static zt_outcome_text_core_error zt_app_main__io_read_all(zt_bool from);
static zt_outcome_void_core_error zt_app_main__io_write(zt_text *value, zt_bool to);
static zt_outcome_void_core_error zt_app_main__io_print(zt_text *value, zt_bool to);
static zt_outcome_void_core_error zt_app_main__io_print_line(zt_text *value, zt_bool to);
static zt_outcome_void_core_error zt_app_main__io_eprint(zt_text *value);
static zt_outcome_void_core_error zt_app_main__io_eprint_line(zt_text *value);
static zt_outcome_map_text_text_core_error zt_app_main__json_parse(zt_text *input);
static zt_text *zt_app_main__json_stringify(zt_map_text_text *value);
static zt_text *zt_app_main__json_pretty(zt_map_text_text *value, zt_int indent);
static zt_float zt_app_main__math_pi(void);
static zt_float zt_app_main__math_e(void);
static zt_float zt_app_main__math_tau(void);
static zt_int zt_app_main__math_abs(zt_int value);
static zt_int zt_app_main__math_min(zt_int a, zt_int b);
static zt_int zt_app_main__math_max(zt_int a, zt_int b);
static zt_int zt_app_main__math_clamp(zt_int value, zt_int min, zt_int max);
static zt_float zt_app_main__math_pow(zt_float base, zt_float exponent);
static zt_float zt_app_main__math_sqrt(zt_float value);
static zt_float zt_app_main__math_floor(zt_float value);
static zt_float zt_app_main__math_ceil(zt_float value);
static zt_float zt_app_main__math_round(zt_float value);
static zt_float zt_app_main__math_trunc(zt_float value);
static zt_float zt_app_main__math_deg_to_rad(zt_float x);
static zt_float zt_app_main__math_rad_to_deg(zt_float x);
static zt_bool zt_app_main__math_approx_equal(zt_float a, zt_float b, zt_float epsilon);
static zt_float zt_app_main__math_sin(zt_float value);
static zt_float zt_app_main__math_cos(zt_float value);
static zt_float zt_app_main__math_tan(zt_float value);
static zt_float zt_app_main__math_asin(zt_float value);
static zt_float zt_app_main__math_acos(zt_float value);
static zt_float zt_app_main__math_atan(zt_float value);
static zt_float zt_app_main__math_atan2(zt_float y, zt_float x);
static zt_float zt_app_main__math_ln(zt_float value);
static zt_float zt_app_main__math_log_ten(zt_float value);
static zt_float zt_app_main__math_log2(zt_float value);
static zt_float zt_app_main__math_log(zt_float value, zt_float base);
static zt_float zt_app_main__math_exp(zt_float value);
static zt_bool zt_app_main__math_is_nan(zt_float value);
static zt_bool zt_app_main__math_is_infinite(zt_float value);
static zt_bool zt_app_main__math_is_finite(zt_float value);
static void zt_app_main__random_seed(zt_int seed);
static zt_int zt_app_main__random_next(void);
static zt_outcome_i64_text zt_app_main__random_between(zt_int min, zt_int max);
static zt_bool zt_app_main__validate_between(zt_int value, zt_int min, zt_int max);
static zt_bool zt_app_main__validate_positive(zt_int value);
static zt_bool zt_app_main__validate_non_negative(zt_int value);
static zt_bool zt_app_main__validate_negative(zt_int value);
static zt_bool zt_app_main__validate_non_zero(zt_int value);
static zt_bool zt_app_main__validate_one_of(zt_int value, zt_list_i64 *candidates);
static zt_bool zt_app_main__validate_not_empty(zt_text *value);
static zt_bool zt_app_main__validate_min_length(zt_text *value, zt_int min);
static zt_bool zt_app_main__validate_max_length(zt_text *value, zt_int max);
static zt_bool zt_app_main__validate_length_between(zt_text *value, zt_int min, zt_int max);
static zt_bool zt_app_main__validate_no_whitespace(zt_text *value);
static zt_bool zt_app_main__validate_has_whitespace(zt_text *value);

static zt_outcome_void_core_error zt_app_main__main(void) {
    zt_text *raw = NULL;
    zt_outcome_map_text_text_core_error __zt_try_result_0 = {0};
    zt_map_text_text *parsed = NULL;
    zt_text *compact = NULL;
    zt_text *pretty = NULL;
    zt_text *hx = NULL;
    zt_text *bn = NULL;
    zt_text *by = NULL;
    zt_float root;
    zt_float rounded;
    zt_int rv;
    zt_int rv_mod;
    zt_bool between_ok;
    zt_outcome_void_core_error __zt_try_result_1 = {0};
    zt_outcome_void_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
    if (raw != NULL) { zt_release(raw); raw = NULL; }
    raw = zt_text_from_utf8_literal("{\"a\":\"1\",\"b\":\"2\"}");
    zt_outcome_map_text_text_core_error_dispose(&__zt_try_result_0);
    __zt_try_result_0 = zt_app_main__json_parse(raw);
    if (zt_outcome_map_text_text_core_error_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
    zt_return_value = zt_outcome_void_core_error_failure((__zt_try_result_0).error);
    goto zt_cleanup;

zt_block_try_success_0:
    if (parsed != NULL) { zt_release(parsed); parsed = NULL; }
    parsed = zt_outcome_map_text_text_core_error_value(__zt_try_result_0);
    goto zt_block_try_after_2;

zt_block_try_after_2:
    if (compact != NULL) { zt_release(compact); compact = NULL; }
    compact = zt_app_main__json_stringify(parsed);
    if (pretty != NULL) { zt_release(pretty); pretty = NULL; }
    pretty = zt_app_main__json_pretty(parsed, 2);
    if (hx != NULL) { zt_release(hx); hx = NULL; }
    hx = zt_app_main__format_hex(255);
    if (bn != NULL) { zt_release(bn); bn = NULL; }
    bn = zt_app_main__format_bin(255);
    if (by != NULL) { zt_release(by); by = NULL; }
    by = zt_app_main__format_bytes(4096, ((zt_app_main__format_BytesStyle){.tag = zt_app_main__format_BytesStyle__Binary}), 1);
    root = zt_app_main__math_sqrt(144.0);
    rounded = zt_app_main__math_round(2.5);
    zt_app_main__random_seed(12345);
    rv = zt_app_main__random_next();
    rv_mod = zt_rem_i64(rv, 100);
    between_ok = (zt_app_main__validate_non_negative(rv_mod) && (rv_mod <= 99));
    if ((((((zt_text_len(compact) == 0) || (zt_text_len(pretty) == 0)) || (zt_text_len(hx) == 0)) || (zt_text_len(bn) == 0)) || (zt_text_len(by) == 0))) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    zt_return_value = zt_outcome_void_core_error_failure_message("m36 stdlib text formatting failed");
    goto zt_cleanup;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    if (((root < 12.0) || (rounded < 3.0))) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
    zt_return_value = zt_outcome_void_core_error_failure_message("m36 stdlib math failed");
    goto zt_cleanup;

zt_block_if_else_4:
    goto zt_block_if_join_5;

zt_block_if_join_5:
    if ((!between_ok)) goto zt_block_if_then_6;
    goto zt_block_if_else_7;

zt_block_if_then_6:
    zt_return_value = zt_outcome_void_core_error_failure_message("m36 stdlib validate/random failed");
    goto zt_cleanup;

zt_block_if_else_7:
    goto zt_block_if_join_8;

zt_block_if_join_8:
    zt_outcome_void_core_error_dispose(&__zt_try_result_1);
    __zt_try_result_1 = zt_app_main__io_print_line(zt_text_from_utf8_literal("m36-stdlib-core-ok"), false);
    if (zt_outcome_void_core_error_is_success(__zt_try_result_1)) goto zt_block_try_success_3;
    goto zt_block_try_failure_4;

zt_block_try_failure_4:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_1);
    goto zt_cleanup;

zt_block_try_success_3:
    goto zt_block_try_after_5;

zt_block_try_after_5:
    zt_return_value = zt_outcome_void_core_error_success();
    goto zt_cleanup;

zt_cleanup:
    if (raw != NULL) { zt_release(raw); raw = NULL; }
    zt_outcome_map_text_text_core_error_dispose(&__zt_try_result_0);
    if (parsed != NULL) { zt_release(parsed); parsed = NULL; }
    if (compact != NULL) { zt_release(compact); compact = NULL; }
    if (pretty != NULL) { zt_release(pretty); pretty = NULL; }
    if (hx != NULL) { zt_release(hx); hx = NULL; }
    if (bn != NULL) { zt_release(bn); bn = NULL; }
    if (by != NULL) { zt_release(by); by = NULL; }
    zt_outcome_void_core_error_dispose(&__zt_try_result_1);
    return zt_return_value;
}

static zt_app_main__format_BytesStyle zt_app_main__format_style_binary(void) {
    goto zt_block_entry;

zt_block_entry:
    return ((zt_app_main__format_BytesStyle){.tag = zt_app_main__format_BytesStyle__Binary});
}

static zt_app_main__format_BytesStyle zt_app_main__format_style_decimal(void) {
    goto zt_block_entry;

zt_block_entry:
    return ((zt_app_main__format_BytesStyle){.tag = zt_app_main__format_BytesStyle__Decimal});
}

static zt_text *zt_app_main__format_hex(zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_format_hex_i64(value);
}

static zt_text *zt_app_main__format_bin(zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_format_bin_i64(value);
}

static zt_text *zt_app_main__format_bytes(zt_int value, zt_app_main__format_BytesStyle style, zt_int decimals) {
    zt_app_main__format_BytesStyle __zt_match_subject_0;
    goto zt_block_entry;

zt_block_entry:
    __zt_match_subject_0 = style;
    goto zt_block_match_case_1;

zt_block_match_case_1:
    if (((__zt_match_subject_0.tag) == 0)) goto zt_block_match_body_2;
    goto zt_block_match_case_3;

zt_block_match_body_2:
    return zt_format_bytes_binary(value, decimals);

zt_block_match_case_3:
    if (((__zt_match_subject_0.tag) == 1)) goto zt_block_match_body_4;
    goto zt_block_match_case_5;

zt_block_match_body_4:
    return zt_format_bytes_decimal(value, decimals);

zt_block_match_case_5:
    return zt_format_bytes_binary(value, decimals);

zt_block_match_after_0:
    zt_panic("unreachable");
}

static zt_text *zt_app_main__format_bytes_binary(zt_int value, zt_int decimals) {
    goto zt_block_entry;

zt_block_entry:
    return zt_app_main__format_bytes(value, zt_app_main__format_style_binary(), decimals);
}

static zt_text *zt_app_main__format_bytes_decimal(zt_int value, zt_int decimals) {
    goto zt_block_entry;

zt_block_entry:
    return zt_app_main__format_bytes(value, zt_app_main__format_style_decimal(), decimals);
}

static zt_outcome_optional_text_core_error zt_app_main__io_read_line(zt_bool from) {
    goto zt_block_entry;

zt_block_entry:
    if (from) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    return zt_host_read_line_stdin();

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    return zt_host_read_line_stdin();
}

static zt_outcome_text_core_error zt_app_main__io_read_all(zt_bool from) {
    goto zt_block_entry;

zt_block_entry:
    if (from) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    return zt_host_read_all_stdin();

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    return zt_host_read_all_stdin();
}

static zt_outcome_void_core_error zt_app_main__io_write(zt_text *value, zt_bool to) {
    goto zt_block_entry;

zt_block_entry:
    if (to) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    return zt_host_write_stderr(value);

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    return zt_host_write_stdout(value);
}

static zt_outcome_void_core_error zt_app_main__io_print(zt_text *value, zt_bool to) {
    goto zt_block_entry;

zt_block_entry:
    return zt_app_main__io_write(value, to);
}

static zt_outcome_void_core_error zt_app_main__io_print_line(zt_text *value, zt_bool to) {
    zt_outcome_void_core_error __zt_try_result_0 = {0};
    zt_outcome_void_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    __zt_try_result_0 = zt_app_main__io_print(value, to);
    if (zt_outcome_void_core_error_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_0);
    goto zt_cleanup;

zt_block_try_success_0:
    goto zt_block_try_after_2;

zt_block_try_after_2:
    zt_return_value = zt_app_main__io_print(zt_text_from_utf8_literal("\n"), to);
    goto zt_cleanup;

zt_cleanup:
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    return zt_return_value;
}

static zt_outcome_void_core_error zt_app_main__io_eprint(zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_app_main__io_write(value, true);
}

static zt_outcome_void_core_error zt_app_main__io_eprint_line(zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_app_main__io_print_line(value, true);
}

static zt_outcome_map_text_text_core_error zt_app_main__json_parse(zt_text *input) {
    goto zt_block_entry;

zt_block_entry:
    return zt_json_parse_map_text_text(input);
}

static zt_text *zt_app_main__json_stringify(zt_map_text_text *value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_text *zt_ffi_return_value = NULL;
    {
        zt_map_text_text *zt_ffi_arg0 = value;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_ffi_return_value = zt_json_stringify_map_text_text(zt_ffi_arg0);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_text *zt_app_main__json_pretty(zt_map_text_text *value, zt_int indent) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_text *zt_ffi_return_value = NULL;
    {
        zt_map_text_text *zt_ffi_arg0 = value;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_ffi_return_value = zt_json_pretty_map_text_text(zt_ffi_arg0, indent);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_float zt_app_main__math_pi(void) {
    goto zt_block_entry;

zt_block_entry:
    return 3.141592653589793;
}

static zt_float zt_app_main__math_e(void) {
    goto zt_block_entry;

zt_block_entry:
    return 2.718281828459045;
}

static zt_float zt_app_main__math_tau(void) {
    goto zt_block_entry;

zt_block_entry:
    return 6.283185307179586;
}

static zt_int zt_app_main__math_abs(zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    if ((value < 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    return zt_sub_i64(0, value);

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    return value;
}

static zt_int zt_app_main__math_min(zt_int a, zt_int b) {
    goto zt_block_entry;

zt_block_entry:
    if ((a < b)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    return a;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    return b;
}

static zt_int zt_app_main__math_max(zt_int a, zt_int b) {
    goto zt_block_entry;

zt_block_entry:
    if ((a > b)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    return a;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    return b;
}

static zt_int zt_app_main__math_clamp(zt_int value, zt_int min, zt_int max) {
    goto zt_block_entry;

zt_block_entry:
    if ((value < min)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    return min;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    if ((value > max)) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
    return max;

zt_block_if_else_4:
    goto zt_block_if_join_5;

zt_block_if_join_5:
    return value;
}

static zt_float zt_app_main__math_pow(zt_float base, zt_float exponent) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_pow(base, exponent);
}

static zt_float zt_app_main__math_sqrt(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_sqrt(value);
}

static zt_float zt_app_main__math_floor(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_floor(value);
}

static zt_float zt_app_main__math_ceil(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_ceil(value);
}

static zt_float zt_app_main__math_round(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_round_half_away_from_zero(value);
}

static zt_float zt_app_main__math_trunc(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_trunc(value);
}

static zt_float zt_app_main__math_deg_to_rad(zt_float x) {
    goto zt_block_entry;

zt_block_entry:
    return zt_mul_i64(x, zt_div_i64(3.141592653589793, 180.0));
}

static zt_float zt_app_main__math_rad_to_deg(zt_float x) {
    goto zt_block_entry;

zt_block_entry:
    return zt_mul_i64(x, zt_div_i64(180.0, 3.141592653589793));
}

static zt_bool zt_app_main__math_approx_equal(zt_float a, zt_float b, zt_float epsilon) {
    zt_float diff;
    goto zt_block_entry;

zt_block_entry:
    diff = zt_sub_i64(a, b);
    if ((diff < 0.0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    diff = zt_sub_i64(0.0, diff);
    goto zt_block_if_join_2;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    return (diff <= epsilon);
}

static zt_float zt_app_main__math_sin(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_sin(value);
}

static zt_float zt_app_main__math_cos(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_cos(value);
}

static zt_float zt_app_main__math_tan(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_tan(value);
}

static zt_float zt_app_main__math_asin(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_asin(value);
}

static zt_float zt_app_main__math_acos(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_acos(value);
}

static zt_float zt_app_main__math_atan(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_atan(value);
}

static zt_float zt_app_main__math_atan2(zt_float y, zt_float x) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_atan2(y, x);
}

static zt_float zt_app_main__math_ln(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_ln(value);
}

static zt_float zt_app_main__math_log_ten(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_log_ten(value);
}

static zt_float zt_app_main__math_log2(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_log2(value);
}

static zt_float zt_app_main__math_log(zt_float value, zt_float base) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_log(value, base);
}

static zt_float zt_app_main__math_exp(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_exp(value);
}

static zt_bool zt_app_main__math_is_nan(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_is_nan(value);
}

static zt_bool zt_app_main__math_is_infinite(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_is_infinite(value);
}

static zt_bool zt_app_main__math_is_finite(zt_float value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_math_is_finite(value);
}

static void zt_app_main__random_seed(zt_int seed) {
    goto zt_block_entry;

zt_block_entry:
    zt_host_random_seed(seed);
    return;
}

static zt_int zt_app_main__random_next(void) {
    goto zt_block_entry;

zt_block_entry:
    return zt_host_random_next_i64();
}

static zt_outcome_i64_text zt_app_main__random_between(zt_int min, zt_int max) {
    zt_int span;
    zt_int raw;
    zt_int offset;
    goto zt_block_entry;

zt_block_entry:
    if ((max < min)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    return zt_outcome_i64_text_failure_message("std.random.between expects max >= min");

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    if ((min == max)) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
    return zt_outcome_i64_text_success(min);

zt_block_if_else_4:
    goto zt_block_if_join_5;

zt_block_if_join_5:
    span = zt_add_i64(zt_sub_i64(max, min), 1);
    raw = zt_app_main__random_next();
    offset = zt_rem_i64(raw, span);
    return zt_outcome_i64_text_success(zt_add_i64(min, offset));
}

static zt_bool zt_app_main__validate_between(zt_int value, zt_int min, zt_int max) {
    goto zt_block_entry;

zt_block_entry:
    return ((value >= min) && (value <= max));
}

static zt_bool zt_app_main__validate_positive(zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    return (value > 0);
}

static zt_bool zt_app_main__validate_non_negative(zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    return (value >= 0);
}

static zt_bool zt_app_main__validate_negative(zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    return (value < 0);
}

static zt_bool zt_app_main__validate_non_zero(zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    return (value != 0);
}

static zt_bool zt_app_main__validate_one_of(zt_int value, zt_list_i64 *candidates) {
    zt_list_i64 *__zt_for_iter_0 = NULL;
    zt_int __zt_for_index_1;
    zt_int __zt_for_len_2;
    zt_int item;
    zt_bool zt_return_value;
    goto zt_block_entry;

zt_block_entry:
    if (__zt_for_iter_0 != NULL) { zt_release(__zt_for_iter_0); __zt_for_iter_0 = NULL; }
    __zt_for_iter_0 = (zt_retain(candidates), candidates);
    __zt_for_index_1 = 0;
    __zt_for_len_2 = zt_list_i64_len(__zt_for_iter_0);
    goto zt_block_for_cond_0;

zt_block_for_cond_0:
    if ((__zt_for_index_1 < __zt_for_len_2)) goto zt_block_for_body_1;
    goto zt_block_for_exit_3;

zt_block_for_body_1:
    item = zt_list_i64_get(__zt_for_iter_0, __zt_for_index_1);
    if ((item == value)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    zt_return_value = true;
    goto zt_cleanup;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    goto zt_block_for_step_2;

zt_block_for_step_2:
    __zt_for_index_1 = zt_add_i64(__zt_for_index_1, 1);
    goto zt_block_for_cond_0;

zt_block_for_exit_3:
    zt_return_value = false;
    goto zt_cleanup;

zt_cleanup:
    if (__zt_for_iter_0 != NULL) { zt_release(__zt_for_iter_0); __zt_for_iter_0 = NULL; }
    return zt_return_value;
}

static zt_bool zt_app_main__validate_not_empty(zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    return (zt_text_len(value) > 0);
}

static zt_bool zt_app_main__validate_min_length(zt_text *value, zt_int min) {
    goto zt_block_entry;

zt_block_entry:
    return (zt_text_len(value) >= min);
}

static zt_bool zt_app_main__validate_max_length(zt_text *value, zt_int max) {
    goto zt_block_entry;

zt_block_entry:
    return (zt_text_len(value) <= max);
}

static zt_bool zt_app_main__validate_length_between(zt_text *value, zt_int min, zt_int max) {
    zt_int count;
    goto zt_block_entry;

zt_block_entry:
    {
        zt_text *zt_ffi_arg0 = value;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        count = zt_text_len(zt_ffi_arg0);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
    return ((count >= min) && (count <= max));
}

static zt_bool zt_app_main__validate_no_whitespace(zt_text *value) {
    zt_text *space = NULL;
    zt_text *tab = NULL;
    zt_text *newline = NULL;
    zt_text *carriage = NULL;
    zt_text *__zt_for_iter_0 = NULL;
    zt_int __zt_for_index_1;
    zt_int __zt_for_len_2;
    zt_text *ch = NULL;
    zt_bool zt_return_value;
    goto zt_block_entry;

zt_block_entry:
    if (space != NULL) { zt_release(space); space = NULL; }
    space = zt_text_from_utf8_literal(" ");
    if (tab != NULL) { zt_release(tab); tab = NULL; }
    tab = zt_text_from_utf8_literal("\t");
    if (newline != NULL) { zt_release(newline); newline = NULL; }
    newline = zt_text_from_utf8_literal("\n");
    if (carriage != NULL) { zt_release(carriage); carriage = NULL; }
    carriage = zt_text_from_utf8_literal("\r");
    if (__zt_for_iter_0 != NULL) { zt_release(__zt_for_iter_0); __zt_for_iter_0 = NULL; }
    __zt_for_iter_0 = (zt_retain(value), value);
    __zt_for_index_1 = 0;
    {
        zt_text *zt_ffi_arg0 = __zt_for_iter_0;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        __zt_for_len_2 = zt_text_len(zt_ffi_arg0);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
    goto zt_block_for_cond_0;

zt_block_for_cond_0:
    if ((__zt_for_index_1 < __zt_for_len_2)) goto zt_block_for_body_1;
    goto zt_block_for_exit_3;

zt_block_for_body_1:
    if (ch != NULL) { zt_release(ch); ch = NULL; }
    {
        zt_text *zt_ffi_arg0 = __zt_for_iter_0;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        ch = zt_text_index(zt_ffi_arg0, __zt_for_index_1);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
    if ((((zt_text_eq(ch, space) || zt_text_eq(ch, tab)) || zt_text_eq(ch, newline)) || zt_text_eq(ch, carriage))) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    zt_return_value = false;
    goto zt_cleanup;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    goto zt_block_for_step_2;

zt_block_for_step_2:
    __zt_for_index_1 = zt_add_i64(__zt_for_index_1, 1);
    goto zt_block_for_cond_0;

zt_block_for_exit_3:
    zt_return_value = true;
    goto zt_cleanup;

zt_cleanup:
    if (space != NULL) { zt_release(space); space = NULL; }
    if (tab != NULL) { zt_release(tab); tab = NULL; }
    if (newline != NULL) { zt_release(newline); newline = NULL; }
    if (carriage != NULL) { zt_release(carriage); carriage = NULL; }
    if (__zt_for_iter_0 != NULL) { zt_release(__zt_for_iter_0); __zt_for_iter_0 = NULL; }
    if (ch != NULL) { zt_release(ch); ch = NULL; }
    return zt_return_value;
}

static zt_bool zt_app_main__validate_has_whitespace(zt_text *value) {
    zt_text *space = NULL;
    zt_text *tab = NULL;
    zt_text *newline = NULL;
    zt_text *carriage = NULL;
    zt_text *__zt_for_iter_0 = NULL;
    zt_int __zt_for_index_1;
    zt_int __zt_for_len_2;
    zt_text *ch = NULL;
    zt_bool zt_return_value;
    goto zt_block_entry;

zt_block_entry:
    if (space != NULL) { zt_release(space); space = NULL; }
    space = zt_text_from_utf8_literal(" ");
    if (tab != NULL) { zt_release(tab); tab = NULL; }
    tab = zt_text_from_utf8_literal("\t");
    if (newline != NULL) { zt_release(newline); newline = NULL; }
    newline = zt_text_from_utf8_literal("\n");
    if (carriage != NULL) { zt_release(carriage); carriage = NULL; }
    carriage = zt_text_from_utf8_literal("\r");
    if (__zt_for_iter_0 != NULL) { zt_release(__zt_for_iter_0); __zt_for_iter_0 = NULL; }
    __zt_for_iter_0 = (zt_retain(value), value);
    __zt_for_index_1 = 0;
    {
        zt_text *zt_ffi_arg0 = __zt_for_iter_0;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        __zt_for_len_2 = zt_text_len(zt_ffi_arg0);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
    goto zt_block_for_cond_0;

zt_block_for_cond_0:
    if ((__zt_for_index_1 < __zt_for_len_2)) goto zt_block_for_body_1;
    goto zt_block_for_exit_3;

zt_block_for_body_1:
    if (ch != NULL) { zt_release(ch); ch = NULL; }
    {
        zt_text *zt_ffi_arg0 = __zt_for_iter_0;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        ch = zt_text_index(zt_ffi_arg0, __zt_for_index_1);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
    if ((((zt_text_eq(ch, space) || zt_text_eq(ch, tab)) || zt_text_eq(ch, newline)) || zt_text_eq(ch, carriage))) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    zt_return_value = true;
    goto zt_cleanup;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    goto zt_block_for_step_2;

zt_block_for_step_2:
    __zt_for_index_1 = zt_add_i64(__zt_for_index_1, 1);
    goto zt_block_for_cond_0;

zt_block_for_exit_3:
    zt_return_value = false;
    goto zt_cleanup;

zt_cleanup:
    if (space != NULL) { zt_release(space); space = NULL; }
    if (tab != NULL) { zt_release(tab); tab = NULL; }
    if (newline != NULL) { zt_release(newline); newline = NULL; }
    if (carriage != NULL) { zt_release(carriage); carriage = NULL; }
    if (__zt_for_iter_0 != NULL) { zt_release(__zt_for_iter_0); __zt_for_iter_0 = NULL; }
    if (ch != NULL) { zt_release(ch); ch = NULL; }
    return zt_return_value;
}

int main(void) {
    zt_outcome_void_core_error __zt_main_result = zt_app_main__main();
    if (!__zt_main_result.is_success) {
        if (__zt_main_result.error.message != NULL) {
            (void)zt_host_write_stderr(__zt_main_result.error.message);
        }
        zt_outcome_void_core_error_dispose(&__zt_main_result);
        return 1;
    }
    zt_outcome_void_core_error_dispose(&__zt_main_result);
    return 0;
}