#include "runtime/c/zenith_rt.h"
#include <stdio.h>

static zt_int zt_app_main__mixed_value(zt_int seed);
static zt_int zt_app_main__bench_std_validate_between(void);
static zt_int zt_app_main__bench_c_validate_between(void);
static zt_int zt_app_main__bench_inline_between(void);
static zt_outcome_void_text zt_app_main__report_case(zt_text *label, zt_int elapsed_ms, zt_int hits);
static zt_outcome_void_text zt_app_main__main(void);
static zt_text *zt_app_main__format_hex(zt_int value);
static zt_text *zt_app_main__format_bin(zt_int value);
static zt_text *zt_app_main__format_bytes(zt_int value, zt_int decimals);
static zt_text *zt_app_main__format_bytes_decimal(zt_int value, zt_int decimals);
static zt_outcome_optional_text_text zt_app_main__io_read_line(void);
static zt_outcome_text_text zt_app_main__io_read_all(void);
static zt_outcome_void_text zt_app_main__io_write(zt_text *value, zt_bool target_stderr);
static zt_outcome_void_text zt_app_main__io_print(zt_text *value, zt_bool target_stderr);
static zt_outcome_void_text zt_app_main__io_print_line(zt_text *value, zt_bool target_stderr);
static zt_outcome_void_text zt_app_main__io_eprint(zt_text *value);
static zt_outcome_void_text zt_app_main__io_eprint_line(zt_text *value);
static zt_int zt_app_main__time_now(void);
static zt_outcome_void_text zt_app_main__time_sleep(zt_int duration_ms);
static zt_int zt_app_main__time_since(zt_int start_unix_ms);
static zt_int zt_app_main__time_until(zt_int end_unix_ms);
static zt_int zt_app_main__time_diff(zt_int a_unix_ms, zt_int b_unix_ms);
static zt_int zt_app_main__time_add(zt_int at_unix_ms, zt_int duration_ms);
static zt_int zt_app_main__time_sub(zt_int at_unix_ms, zt_int duration_ms);
static zt_int zt_app_main__time_from_unix(zt_int ts);
static zt_int zt_app_main__time_from_unix_ms(zt_int ts);
static zt_int zt_app_main__time_to_unix(zt_int unix_ms);
static zt_int zt_app_main__time_to_unix_ms(zt_int unix_ms);
static zt_int zt_app_main__time_milliseconds(zt_int n);
static zt_int zt_app_main__time_seconds(zt_int n);
static zt_int zt_app_main__time_minutes(zt_int n);
static zt_int zt_app_main__time_hours(zt_int n);
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

static zt_outcome_void_text zt_app_main__report_case(zt_text *label, zt_int elapsed_ms, zt_int hits) {
    zt_outcome_void_text __zt_try_result_0;
    zt_outcome_void_text __zt_try_result_1;
    zt_outcome_void_text __zt_try_result_2;
    zt_outcome_void_text __zt_try_result_3;
    goto zt_block_entry;

zt_block_entry:
    __zt_try_result_0 = zt_app_main__io_print(label, false);
    if (zt_outcome_void_text_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
    return zt_outcome_void_text_propagate(__zt_try_result_0);

zt_block_try_success_0:
    goto zt_block_try_after_2;

zt_block_try_after_2:
    __zt_try_result_1 = zt_app_main__io_print(zt_text_from_utf8_literal(" ms=0x"), false);
    if (zt_outcome_void_text_is_success(__zt_try_result_1)) goto zt_block_try_success_3;
    goto zt_block_try_failure_4;

zt_block_try_failure_4:
    return zt_outcome_void_text_propagate(__zt_try_result_1);

zt_block_try_success_3:
    goto zt_block_try_after_5;

zt_block_try_after_5:
    __zt_try_result_2 = zt_app_main__io_print(zt_app_main__format_hex(elapsed_ms), false);
    if (zt_outcome_void_text_is_success(__zt_try_result_2)) goto zt_block_try_success_6;
    goto zt_block_try_failure_7;

zt_block_try_failure_7:
    return zt_outcome_void_text_propagate(__zt_try_result_2);

zt_block_try_success_6:
    goto zt_block_try_after_8;

zt_block_try_after_8:
    __zt_try_result_3 = zt_app_main__io_print(zt_text_from_utf8_literal(" hits=0x"), false);
    if (zt_outcome_void_text_is_success(__zt_try_result_3)) goto zt_block_try_success_9;
    goto zt_block_try_failure_10;

zt_block_try_failure_10:
    return zt_outcome_void_text_propagate(__zt_try_result_3);

zt_block_try_success_9:
    goto zt_block_try_after_11;

zt_block_try_after_11:
    return zt_app_main__io_print_line(zt_app_main__format_hex(hits), false);
}

static zt_outcome_void_text zt_app_main__main(void) {
    zt_int iterations;
    zt_int start_ms;
    zt_int elapsed_ms;
    zt_outcome_void_text __zt_try_result_0;
    zt_outcome_void_text __zt_try_result_1;
    zt_outcome_void_text __zt_try_result_2;
    zt_int hits_std;
    zt_outcome_void_text __zt_try_result_3;
    zt_int hits_c;
    zt_outcome_void_text __zt_try_result_4;
    zt_int hits_inline;
    zt_outcome_void_text __zt_try_result_5;
    goto zt_block_entry;

zt_block_entry:
    iterations = 5000000;
    start_ms = 0;
    elapsed_ms = 0;
    __zt_try_result_0 = zt_app_main__io_print_line(zt_text_from_utf8_literal("benchmark std.validate.between vs c helper"), false);
    if (zt_outcome_void_text_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
    return zt_outcome_void_text_propagate(__zt_try_result_0);

zt_block_try_success_0:
    goto zt_block_try_after_2;

zt_block_try_after_2:
    __zt_try_result_1 = zt_app_main__io_print(zt_text_from_utf8_literal("iterations=0x"), false);
    if (zt_outcome_void_text_is_success(__zt_try_result_1)) goto zt_block_try_success_3;
    goto zt_block_try_failure_4;

zt_block_try_failure_4:
    return zt_outcome_void_text_propagate(__zt_try_result_1);

zt_block_try_success_3:
    goto zt_block_try_after_5;

zt_block_try_after_5:
    __zt_try_result_2 = zt_app_main__io_print_line(zt_app_main__format_hex(iterations), false);
    if (zt_outcome_void_text_is_success(__zt_try_result_2)) goto zt_block_try_success_6;
    goto zt_block_try_failure_7;

zt_block_try_failure_7:
    return zt_outcome_void_text_propagate(__zt_try_result_2);

zt_block_try_success_6:
    goto zt_block_try_after_8;

zt_block_try_after_8:
    start_ms = zt_app_main__time_now();
    hits_std = zt_app_main__bench_std_validate_between();
    elapsed_ms = zt_app_main__time_since(start_ms);
    __zt_try_result_3 = zt_app_main__report_case(zt_text_from_utf8_literal("std.validate.between"), elapsed_ms, hits_std);
    if (zt_outcome_void_text_is_success(__zt_try_result_3)) goto zt_block_try_success_9;
    goto zt_block_try_failure_10;

zt_block_try_failure_10:
    return zt_outcome_void_text_propagate(__zt_try_result_3);

zt_block_try_success_9:
    goto zt_block_try_after_11;

zt_block_try_after_11:
    start_ms = zt_app_main__time_now();
    hits_c = zt_app_main__bench_c_validate_between();
    elapsed_ms = zt_app_main__time_since(start_ms);
    __zt_try_result_4 = zt_app_main__report_case(zt_text_from_utf8_literal("c.zt_validate_between_i64"), elapsed_ms, hits_c);
    if (zt_outcome_void_text_is_success(__zt_try_result_4)) goto zt_block_try_success_12;
    goto zt_block_try_failure_13;

zt_block_try_failure_13:
    return zt_outcome_void_text_propagate(__zt_try_result_4);

zt_block_try_success_12:
    goto zt_block_try_after_14;

zt_block_try_after_14:
    start_ms = zt_app_main__time_now();
    hits_inline = zt_app_main__bench_inline_between();
    elapsed_ms = zt_app_main__time_since(start_ms);
    __zt_try_result_5 = zt_app_main__report_case(zt_text_from_utf8_literal("inline >= <="), elapsed_ms, hits_inline);
    if (zt_outcome_void_text_is_success(__zt_try_result_5)) goto zt_block_try_success_15;
    goto zt_block_try_failure_16;

zt_block_try_failure_16:
    return zt_outcome_void_text_propagate(__zt_try_result_5);

zt_block_try_success_15:
    goto zt_block_try_after_17;

zt_block_try_after_17:
    if (((hits_std != hits_c) || (hits_std != hits_inline))) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    return zt_outcome_void_text_failure_message("benchmark mismatch: different hit counts across implementations");

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    return zt_outcome_void_text_success();
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

static zt_text *zt_app_main__format_bytes(zt_int value, zt_int decimals) {
    goto zt_block_entry;

zt_block_entry:
    return zt_format_bytes_binary(value, decimals);
}

static zt_text *zt_app_main__format_bytes_decimal(zt_int value, zt_int decimals) {
    goto zt_block_entry;

zt_block_entry:
    return zt_format_bytes_decimal(value, decimals);
}

static zt_outcome_optional_text_text zt_app_main__io_read_line(void) {
    goto zt_block_entry;

zt_block_entry:
    return zt_host_read_line_stdin();
}

static zt_outcome_text_text zt_app_main__io_read_all(void) {
    goto zt_block_entry;

zt_block_entry:
    return zt_host_read_all_stdin();
}

static zt_outcome_void_text zt_app_main__io_write(zt_text *value, zt_bool target_stderr) {
    goto zt_block_entry;

zt_block_entry:
    if (target_stderr) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    return zt_host_write_stderr(value);

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    return zt_host_write_stdout(value);
}

static zt_outcome_void_text zt_app_main__io_print(zt_text *value, zt_bool target_stderr) {
    goto zt_block_entry;

zt_block_entry:
    return zt_app_main__io_write(value, target_stderr);
}

static zt_outcome_void_text zt_app_main__io_print_line(zt_text *value, zt_bool target_stderr) {
    zt_outcome_void_text __zt_try_result_0;
    goto zt_block_entry;

zt_block_entry:
    __zt_try_result_0 = zt_app_main__io_print(value, target_stderr);
    if (zt_outcome_void_text_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
    return zt_outcome_void_text_propagate(__zt_try_result_0);

zt_block_try_success_0:
    goto zt_block_try_after_2;

zt_block_try_after_2:
    return zt_app_main__io_print(zt_text_from_utf8_literal("\n"), target_stderr);
}

static zt_outcome_void_text zt_app_main__io_eprint(zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_app_main__io_write(value, true);
}

static zt_outcome_void_text zt_app_main__io_eprint_line(zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_app_main__io_print_line(value, true);
}

static zt_int zt_app_main__time_now(void) {
    goto zt_block_entry;

zt_block_entry:
    return zt_host_time_now_unix_ms();
}

static zt_outcome_void_text zt_app_main__time_sleep(zt_int duration_ms) {
    goto zt_block_entry;

zt_block_entry:
    return zt_host_time_sleep_ms(duration_ms);
}

static zt_int zt_app_main__time_since(zt_int start_unix_ms) {
    goto zt_block_entry;

zt_block_entry:
    return zt_app_main__time_diff(start_unix_ms, zt_app_main__time_now());
}

static zt_int zt_app_main__time_until(zt_int end_unix_ms) {
    goto zt_block_entry;

zt_block_entry:
    return zt_app_main__time_diff(zt_app_main__time_now(), end_unix_ms);
}

static zt_int zt_app_main__time_diff(zt_int a_unix_ms, zt_int b_unix_ms) {
    goto zt_block_entry;

zt_block_entry:
    return zt_sub_i64(b_unix_ms, a_unix_ms);
}

static zt_int zt_app_main__time_add(zt_int at_unix_ms, zt_int duration_ms) {
    goto zt_block_entry;

zt_block_entry:
    return zt_add_i64(at_unix_ms, duration_ms);
}

static zt_int zt_app_main__time_sub(zt_int at_unix_ms, zt_int duration_ms) {
    goto zt_block_entry;

zt_block_entry:
    return zt_sub_i64(at_unix_ms, duration_ms);
}

static zt_int zt_app_main__time_from_unix(zt_int ts) {
    goto zt_block_entry;

zt_block_entry:
    return zt_mul_i64(ts, 1000);
}

static zt_int zt_app_main__time_from_unix_ms(zt_int ts) {
    goto zt_block_entry;

zt_block_entry:
    return ts;
}

static zt_int zt_app_main__time_to_unix(zt_int unix_ms) {
    goto zt_block_entry;

zt_block_entry:
    return zt_div_i64(unix_ms, 1000);
}

static zt_int zt_app_main__time_to_unix_ms(zt_int unix_ms) {
    goto zt_block_entry;

zt_block_entry:
    return unix_ms;
}

static zt_int zt_app_main__time_milliseconds(zt_int n) {
    goto zt_block_entry;

zt_block_entry:
    return n;
}

static zt_int zt_app_main__time_seconds(zt_int n) {
    goto zt_block_entry;

zt_block_entry:
    return zt_mul_i64(n, 1000);
}

static zt_int zt_app_main__time_minutes(zt_int n) {
    goto zt_block_entry;

zt_block_entry:
    return zt_mul_i64(n, 60000);
}

static zt_int zt_app_main__time_hours(zt_int n) {
    goto zt_block_entry;

zt_block_entry:
    return zt_mul_i64(n, 3600000);
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
    count = zt_text_len(value);
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
    __zt_for_len_2 = zt_text_len(__zt_for_iter_0);
    goto zt_block_for_cond_0;

zt_block_for_cond_0:
    if ((__zt_for_index_1 < __zt_for_len_2)) goto zt_block_for_body_1;
    goto zt_block_for_exit_3;

zt_block_for_body_1:
    if (ch != NULL) { zt_release(ch); ch = NULL; }
    ch = zt_text_index(__zt_for_iter_0, __zt_for_index_1);
    if (((((ch == space) || (ch == tab)) || (ch == newline)) || (ch == carriage))) goto zt_block_if_then_0;
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

int main(void) {
    zt_outcome_void_text __zt_main_result = zt_app_main__main();
    if (!__zt_main_result.is_success) {
        if (__zt_main_result.error != NULL) {
            (void)zt_host_write_stderr(__zt_main_result.error);
        }
        return 1;
    }
    return 0;
}