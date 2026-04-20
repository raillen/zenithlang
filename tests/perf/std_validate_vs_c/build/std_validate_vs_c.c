#include "runtime/c/zenith_rt.h"
#include <stdio.h>


typedef struct zt_app_main__time_Instant {
    zt_int millis;
} zt_app_main__time_Instant;

typedef struct zt_app_main__time_Duration {
    zt_int millis;
} zt_app_main__time_Duration;

typedef enum zt_app_main__format_BytesStyle_tag {
    zt_app_main__format_BytesStyle__Binary,
    zt_app_main__format_BytesStyle__Decimal
} zt_app_main__format_BytesStyle_tag;

typedef struct zt_app_main__format_BytesStyle {
    zt_app_main__format_BytesStyle_tag tag;
} zt_app_main__format_BytesStyle;
static zt_int zt_app_main__mixed_value(zt_int seed);
static zt_int zt_app_main__bench_std_validate_between(void);
static zt_int zt_app_main__bench_c_validate_between(void);
static zt_int zt_app_main__bench_inline_between(void);
static zt_outcome_void_core_error zt_app_main__report_case(zt_text *label, zt_int elapsed_ms, zt_int hits);
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
static zt_app_main__time_Instant zt_app_main__time_now(void);
static zt_outcome_void_core_error zt_app_main__time_sleep(zt_app_main__time_Duration duration);
static zt_app_main__time_Duration zt_app_main__time_since(zt_app_main__time_Instant start);
static zt_app_main__time_Duration zt_app_main__time_until(zt_app_main__time_Instant target);
static zt_app_main__time_Duration zt_app_main__time_diff(zt_app_main__time_Instant a, zt_app_main__time_Instant b);
static zt_app_main__time_Instant zt_app_main__time_add(zt_app_main__time_Instant at, zt_app_main__time_Duration duration);
static zt_app_main__time_Instant zt_app_main__time_sub(zt_app_main__time_Instant at, zt_app_main__time_Duration duration);
static zt_app_main__time_Instant zt_app_main__time_from_unix(zt_int ts);
static zt_app_main__time_Instant zt_app_main__time_from_unix_ms(zt_int ts);
static zt_int zt_app_main__time_to_unix(zt_app_main__time_Instant at);
static zt_int zt_app_main__time_to_unix_ms(zt_app_main__time_Instant at);
static zt_app_main__time_Duration zt_app_main__time_milliseconds(zt_int n);
static zt_app_main__time_Duration zt_app_main__time_seconds(zt_int n);
static zt_app_main__time_Duration zt_app_main__time_minutes(zt_int n);
static zt_app_main__time_Duration zt_app_main__time_hours(zt_int n);
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

static zt_int zt_app_main__mixed_value(zt_int seed) {
    goto zt_block_entry;

zt_block_entry:
    return zt_sub_i64(zt_rem_i64(zt_mul_i64(seed, 73), 301), 100);
}

static zt_int zt_app_main__bench_std_validate_between(void) {
    zt_int iterations;
    zt_int min_value;
    zt_int max_value;
    zt_int i;
    zt_int hits;
    zt_int value;
    goto zt_block_entry;

zt_block_entry:
    iterations = 5000000;
    min_value = (-20);
    max_value = 120;
    i = 0;
    hits = 0;
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
    if ((i < iterations)) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
    value = zt_app_main__mixed_value(i);
    if (zt_app_main__validate_between(value, min_value, max_value)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    hits = zt_add_i64(hits, 1);
    goto zt_block_if_join_2;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    i = zt_add_i64(i, 1);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
    return hits;
}

static zt_int zt_app_main__bench_c_validate_between(void) {
    zt_int iterations;
    zt_int min_value;
    zt_int max_value;
    zt_int i;
    zt_int hits;
    zt_int value;
    goto zt_block_entry;

zt_block_entry:
    iterations = 5000000;
    min_value = (-20);
    max_value = 120;
    i = 0;
    hits = 0;
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
    if ((i < iterations)) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
    value = zt_app_main__mixed_value(i);
    if (zt_validate_between_i64(value, min_value, max_value)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    hits = zt_add_i64(hits, 1);
    goto zt_block_if_join_2;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    i = zt_add_i64(i, 1);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
    return hits;
}

static zt_int zt_app_main__bench_inline_between(void) {
    zt_int iterations;
    zt_int min_value;
    zt_int max_value;
    zt_int i;
    zt_int hits;
    zt_int value;
    goto zt_block_entry;

zt_block_entry:
    iterations = 5000000;
    min_value = (-20);
    max_value = 120;
    i = 0;
    hits = 0;
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
    if ((i < iterations)) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
    value = zt_app_main__mixed_value(i);
    if (((value >= min_value) && (value <= max_value))) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    hits = zt_add_i64(hits, 1);
    goto zt_block_if_join_2;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    i = zt_add_i64(i, 1);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
    return hits;
}

static zt_outcome_void_core_error zt_app_main__report_case(zt_text *label, zt_int elapsed_ms, zt_int hits) {
    zt_outcome_void_core_error __zt_try_result_0 = {0};
    zt_outcome_void_core_error __zt_try_result_1 = {0};
    zt_outcome_void_core_error __zt_try_result_2 = {0};
    zt_outcome_void_core_error __zt_try_result_3 = {0};
    zt_outcome_void_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    __zt_try_result_0 = zt_app_main__io_print(label, false);
    if (zt_outcome_void_core_error_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_0);
    goto zt_cleanup;

zt_block_try_success_0:
    goto zt_block_try_after_2;

zt_block_try_after_2:
    zt_outcome_void_core_error_dispose(&__zt_try_result_1);
    __zt_try_result_1 = zt_app_main__io_print(zt_text_from_utf8_literal(" ms=0x"), false);
    if (zt_outcome_void_core_error_is_success(__zt_try_result_1)) goto zt_block_try_success_3;
    goto zt_block_try_failure_4;

zt_block_try_failure_4:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_1);
    goto zt_cleanup;

zt_block_try_success_3:
    goto zt_block_try_after_5;

zt_block_try_after_5:
    zt_outcome_void_core_error_dispose(&__zt_try_result_2);
    __zt_try_result_2 = zt_app_main__io_print(zt_app_main__format_hex(elapsed_ms), false);
    if (zt_outcome_void_core_error_is_success(__zt_try_result_2)) goto zt_block_try_success_6;
    goto zt_block_try_failure_7;

zt_block_try_failure_7:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_2);
    goto zt_cleanup;

zt_block_try_success_6:
    goto zt_block_try_after_8;

zt_block_try_after_8:
    zt_outcome_void_core_error_dispose(&__zt_try_result_3);
    __zt_try_result_3 = zt_app_main__io_print(zt_text_from_utf8_literal(" hits=0x"), false);
    if (zt_outcome_void_core_error_is_success(__zt_try_result_3)) goto zt_block_try_success_9;
    goto zt_block_try_failure_10;

zt_block_try_failure_10:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_3);
    goto zt_cleanup;

zt_block_try_success_9:
    goto zt_block_try_after_11;

zt_block_try_after_11:
    zt_return_value = zt_app_main__io_print_line(zt_app_main__format_hex(hits), false);
    goto zt_cleanup;

zt_cleanup:
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    zt_outcome_void_core_error_dispose(&__zt_try_result_1);
    zt_outcome_void_core_error_dispose(&__zt_try_result_2);
    zt_outcome_void_core_error_dispose(&__zt_try_result_3);
    return zt_return_value;
}

static zt_outcome_void_core_error zt_app_main__main(void) {
    zt_int iterations;
    zt_app_main__time_Instant start_at;
    zt_app_main__time_Duration elapsed;
    zt_outcome_void_core_error __zt_try_result_0 = {0};
    zt_outcome_void_core_error __zt_try_result_1 = {0};
    zt_outcome_void_core_error __zt_try_result_2 = {0};
    zt_int hits_std;
    zt_outcome_void_core_error __zt_try_result_3 = {0};
    zt_int hits_c;
    zt_outcome_void_core_error __zt_try_result_4 = {0};
    zt_int hits_inline;
    zt_outcome_void_core_error __zt_try_result_5 = {0};
    zt_outcome_void_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
    iterations = 5000000;
    start_at = zt_app_main__time_now();
    elapsed = zt_app_main__time_seconds(0);
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    __zt_try_result_0 = zt_app_main__io_print_line(zt_text_from_utf8_literal("benchmark std.validate.between vs c helper"), false);
    if (zt_outcome_void_core_error_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_0);
    goto zt_cleanup;

zt_block_try_success_0:
    goto zt_block_try_after_2;

zt_block_try_after_2:
    zt_outcome_void_core_error_dispose(&__zt_try_result_1);
    __zt_try_result_1 = zt_app_main__io_print(zt_text_from_utf8_literal("iterations=0x"), false);
    if (zt_outcome_void_core_error_is_success(__zt_try_result_1)) goto zt_block_try_success_3;
    goto zt_block_try_failure_4;

zt_block_try_failure_4:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_1);
    goto zt_cleanup;

zt_block_try_success_3:
    goto zt_block_try_after_5;

zt_block_try_after_5:
    zt_outcome_void_core_error_dispose(&__zt_try_result_2);
    __zt_try_result_2 = zt_app_main__io_print_line(zt_app_main__format_hex(iterations), false);
    if (zt_outcome_void_core_error_is_success(__zt_try_result_2)) goto zt_block_try_success_6;
    goto zt_block_try_failure_7;

zt_block_try_failure_7:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_2);
    goto zt_cleanup;

zt_block_try_success_6:
    goto zt_block_try_after_8;

zt_block_try_after_8:
    start_at = zt_app_main__time_now();
    hits_std = zt_app_main__bench_std_validate_between();
    elapsed = zt_app_main__time_since(start_at);
    zt_outcome_void_core_error_dispose(&__zt_try_result_3);
    __zt_try_result_3 = zt_app_main__report_case(zt_text_from_utf8_literal("std.validate.between"), (elapsed.millis), hits_std);
    if (zt_outcome_void_core_error_is_success(__zt_try_result_3)) goto zt_block_try_success_9;
    goto zt_block_try_failure_10;

zt_block_try_failure_10:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_3);
    goto zt_cleanup;

zt_block_try_success_9:
    goto zt_block_try_after_11;

zt_block_try_after_11:
    start_at = zt_app_main__time_now();
    hits_c = zt_app_main__bench_c_validate_between();
    elapsed = zt_app_main__time_since(start_at);
    zt_outcome_void_core_error_dispose(&__zt_try_result_4);
    __zt_try_result_4 = zt_app_main__report_case(zt_text_from_utf8_literal("c.zt_validate_between_i64"), (elapsed.millis), hits_c);
    if (zt_outcome_void_core_error_is_success(__zt_try_result_4)) goto zt_block_try_success_12;
    goto zt_block_try_failure_13;

zt_block_try_failure_13:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_4);
    goto zt_cleanup;

zt_block_try_success_12:
    goto zt_block_try_after_14;

zt_block_try_after_14:
    start_at = zt_app_main__time_now();
    hits_inline = zt_app_main__bench_inline_between();
    elapsed = zt_app_main__time_since(start_at);
    zt_outcome_void_core_error_dispose(&__zt_try_result_5);
    __zt_try_result_5 = zt_app_main__report_case(zt_text_from_utf8_literal("inline >= <="), (elapsed.millis), hits_inline);
    if (zt_outcome_void_core_error_is_success(__zt_try_result_5)) goto zt_block_try_success_15;
    goto zt_block_try_failure_16;

zt_block_try_failure_16:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_5);
    goto zt_cleanup;

zt_block_try_success_15:
    goto zt_block_try_after_17;

zt_block_try_after_17:
    if (((hits_std != hits_c) || (hits_std != hits_inline))) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    zt_return_value = zt_outcome_void_core_error_failure_message("benchmark mismatch: different hit counts across implementations");
    goto zt_cleanup;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    zt_return_value = zt_outcome_void_core_error_success();
    goto zt_cleanup;

zt_cleanup:
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    zt_outcome_void_core_error_dispose(&__zt_try_result_1);
    zt_outcome_void_core_error_dispose(&__zt_try_result_2);
    zt_outcome_void_core_error_dispose(&__zt_try_result_3);
    zt_outcome_void_core_error_dispose(&__zt_try_result_4);
    zt_outcome_void_core_error_dispose(&__zt_try_result_5);
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

static zt_app_main__time_Instant zt_app_main__time_now(void) {
    goto zt_block_entry;

zt_block_entry:
    return ((zt_app_main__time_Instant){.millis = zt_host_time_now_unix_ms()});
}

static zt_outcome_void_core_error zt_app_main__time_sleep(zt_app_main__time_Duration duration) {
    goto zt_block_entry;

zt_block_entry:
    return zt_host_time_sleep_ms((duration.millis));
}

static zt_app_main__time_Duration zt_app_main__time_since(zt_app_main__time_Instant start) {
    goto zt_block_entry;

zt_block_entry:
    return zt_app_main__time_diff(start, zt_app_main__time_now());
}

static zt_app_main__time_Duration zt_app_main__time_until(zt_app_main__time_Instant target) {
    goto zt_block_entry;

zt_block_entry:
    return zt_app_main__time_diff(zt_app_main__time_now(), target);
}

static zt_app_main__time_Duration zt_app_main__time_diff(zt_app_main__time_Instant a, zt_app_main__time_Instant b) {
    goto zt_block_entry;

zt_block_entry:
    return ((zt_app_main__time_Duration){.millis = zt_sub_i64((b.millis), (a.millis))});
}

static zt_app_main__time_Instant zt_app_main__time_add(zt_app_main__time_Instant at, zt_app_main__time_Duration duration) {
    goto zt_block_entry;

zt_block_entry:
    return ((zt_app_main__time_Instant){.millis = zt_add_i64((at.millis), (duration.millis))});
}

static zt_app_main__time_Instant zt_app_main__time_sub(zt_app_main__time_Instant at, zt_app_main__time_Duration duration) {
    goto zt_block_entry;

zt_block_entry:
    return ((zt_app_main__time_Instant){.millis = zt_sub_i64((at.millis), (duration.millis))});
}

static zt_app_main__time_Instant zt_app_main__time_from_unix(zt_int ts) {
    goto zt_block_entry;

zt_block_entry:
    return ((zt_app_main__time_Instant){.millis = zt_mul_i64(ts, 1000)});
}

static zt_app_main__time_Instant zt_app_main__time_from_unix_ms(zt_int ts) {
    goto zt_block_entry;

zt_block_entry:
    return ((zt_app_main__time_Instant){.millis = ts});
}

static zt_int zt_app_main__time_to_unix(zt_app_main__time_Instant at) {
    goto zt_block_entry;

zt_block_entry:
    return zt_div_i64((at.millis), 1000);
}

static zt_int zt_app_main__time_to_unix_ms(zt_app_main__time_Instant at) {
    goto zt_block_entry;

zt_block_entry:
    return (at.millis);
}

static zt_app_main__time_Duration zt_app_main__time_milliseconds(zt_int n) {
    goto zt_block_entry;

zt_block_entry:
    return ((zt_app_main__time_Duration){.millis = n});
}

static zt_app_main__time_Duration zt_app_main__time_seconds(zt_int n) {
    goto zt_block_entry;

zt_block_entry:
    return ((zt_app_main__time_Duration){.millis = zt_mul_i64(n, 1000)});
}

static zt_app_main__time_Duration zt_app_main__time_minutes(zt_int n) {
    goto zt_block_entry;

zt_block_entry:
    return ((zt_app_main__time_Duration){.millis = zt_mul_i64(n, 60000)});
}

static zt_app_main__time_Duration zt_app_main__time_hours(zt_int n) {
    goto zt_block_entry;

zt_block_entry:
    return ((zt_app_main__time_Duration){.millis = zt_mul_i64(n, 3600000)});
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