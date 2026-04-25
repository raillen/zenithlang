#include "runtime/c/zenith_rt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef enum zt_preview_runner__io_Error_tag {
    zt_preview_runner__io_Error__ReadFailed,
    zt_preview_runner__io_Error__WriteFailed,
    zt_preview_runner__io_Error__Unknown
} zt_preview_runner__io_Error_tag;

typedef struct zt_preview_runner__io_Error {
    zt_preview_runner__io_Error_tag tag;
} zt_preview_runner__io_Error;

typedef struct zt_preview_runner__SceneLoadInfo {
    zt_text *path;
    zt_int entity_count;
    zt_bool loaded;
} zt_preview_runner__SceneLoadInfo;

typedef struct zt_preview_runner__io_Input {
    zt_int handle;
} zt_preview_runner__io_Input;

typedef struct zt_preview_runner__io_Output {
    zt_int handle;
} zt_preview_runner__io_Output;
static zt_outcome_void_core_error zt_preview_runner__main(void *zt_ctx);
static zt_outcome_i64_core_error zt_preview_runner__respond_hello(void *zt_ctx, zt_int seq);
static zt_outcome_i64_core_error zt_preview_runner__respond_play_mode(void *zt_ctx, zt_int seq);
static zt_outcome_i64_core_error zt_preview_runner__respond_pause_play_mode(void *zt_ctx, zt_int seq);
static zt_outcome_i64_core_error zt_preview_runner__respond_stop_play_mode(void *zt_ctx, zt_int seq);
static zt_outcome_i64_core_error zt_preview_runner__respond_pong(void *zt_ctx, zt_int seq, zt_text *line);
static zt_outcome_i64_core_error zt_preview_runner__respond_open_scene(void *zt_ctx, zt_int seq, zt_text *line);
static zt_preview_runner__SceneLoadInfo zt_preview_runner__load_scene_info(void *zt_ctx, zt_text *scene_path);
static zt_outcome_i64_core_error zt_preview_runner__respond_open_project(void *zt_ctx, zt_int seq);
static zt_text *zt_preview_runner__build_envelope(void *zt_ctx, zt_int seq, zt_text *channel, zt_text *kind, zt_text *payload);
static zt_text *zt_preview_runner__extract_ping_token(void *zt_ctx, zt_text *line);
static zt_text *zt_preview_runner__extract_payload_path(void *zt_ctx, zt_text *line);
static zt_int zt_preview_runner__count_occurrences(void *zt_ctx, zt_text *value, zt_text *needle);
static zt_text *zt_preview_runner__bool_to_text(void *zt_ctx, zt_bool value);
static zt_text *zt_preview_runner__int_to_text(void *zt_ctx, zt_int value);
static zt_outcome_optional_text_core_error zt_preview_runner__io_read_line(void *zt_ctx, zt_preview_runner__io_Input from);
static zt_outcome_text_core_error zt_preview_runner__io_read_all(void *zt_ctx, zt_preview_runner__io_Input from);
static zt_outcome_void_core_error zt_preview_runner__io_write(void *zt_ctx, zt_text *value, zt_preview_runner__io_Output to);
static zt_outcome_void_core_error zt_preview_runner__io_print(void *zt_ctx, zt_text *value, zt_preview_runner__io_Output to);
static zt_bool zt_preview_runner__text__eq(void *zt_ctx, zt_text *a, zt_text *b);
static zt_text *zt_preview_runner__text__slice_from(void *zt_ctx, zt_text *value, zt_int start);
static zt_text *zt_preview_runner__text__slice_to(void *zt_ctx, zt_text *value, zt_int finish);
static zt_bool zt_preview_runner__text__is_whitespace_char(void *zt_ctx, zt_text *ch);
static zt_bool zt_preview_runner__text__is_ascii_digit_char(void *zt_ctx, zt_text *ch);
static zt_bool zt_preview_runner__text__starts_at(void *zt_ctx, zt_text *value, zt_text *needle, zt_int start);
static zt_bytes *zt_preview_runner__text_to_utf8(void *zt_ctx, zt_text *value);
static zt_outcome_text_text zt_preview_runner__text_from_utf8(void *zt_ctx, zt_bytes *value);
static zt_text *zt_preview_runner__text_trim(void *zt_ctx, zt_text *value);
static zt_text *zt_preview_runner__text_trim_start(void *zt_ctx, zt_text *value);
static zt_text *zt_preview_runner__text_trim_end(void *zt_ctx, zt_text *value);
static zt_bool zt_preview_runner__text_contains(void *zt_ctx, zt_text *value, zt_text *needle);
static zt_text *zt_preview_runner__text_join(void *zt_ctx, zt_list_text *parts);
static zt_text *zt_preview_runner__text_replace_all(void *zt_ctx, zt_text *value, zt_text *needle, zt_text *replacement);
static zt_bool zt_preview_runner__text_starts_with(void *zt_ctx, zt_text *value, zt_text *prefix);
static zt_bool zt_preview_runner__text_ends_with(void *zt_ctx, zt_text *value, zt_text *suffix);
static zt_bool zt_preview_runner__text_has_prefix(void *zt_ctx, zt_text *value, zt_text *prefix);
static zt_bool zt_preview_runner__text_has_suffix(void *zt_ctx, zt_text *value, zt_text *suffix);
static zt_bool zt_preview_runner__text_has_whitespace(void *zt_ctx, zt_text *value);
static zt_int zt_preview_runner__text_index_of(void *zt_ctx, zt_text *value, zt_text *needle);
static zt_int zt_preview_runner__text_last_index_of(void *zt_ctx, zt_text *value, zt_text *needle);
static zt_bool zt_preview_runner__text_is_empty(void *zt_ctx, zt_text *value);
static zt_bool zt_preview_runner__text_is_digits(void *zt_ctx, zt_text *value);
static zt_text *zt_preview_runner__text_limit(void *zt_ctx, zt_text *value, zt_int max_len);

static zt_outcome_void_core_error zt_preview_runner__main(void *zt_ctx) {
    zt_outcome_void_core_error __zt_try_result_0 = {0};
    zt_bool running;
    zt_int next_seq;
    zt_outcome_optional_text_core_error __zt_try_result_1 = {0};
    zt_optional_text line_opt = {0};
    zt_optional_text __zt_match_subject_0 = {0};
    zt_text *line = NULL;
    zt_text *trimmed = NULL;
    zt_outcome_i64_core_error __zt_try_result_2 = {0};
    zt_outcome_i64_core_error __zt_try_result_3 = {0};
    zt_outcome_i64_core_error __zt_try_result_4 = {0};
    zt_outcome_i64_core_error __zt_try_result_5 = {0};
    zt_outcome_i64_core_error __zt_try_result_6 = {0};
    zt_outcome_i64_core_error __zt_try_result_7 = {0};
    zt_outcome_i64_core_error __zt_try_result_8 = {0};
    zt_outcome_void_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_outcome_void_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__io_write(NULL, zt_text_from_utf8_literal("{\"protocol\":1,\"seq\":0,\"channel\":\"event\",\"kind\":\"status\",\"payload\":{\"status\":\"starting\"}}\n"), ((zt_preview_runner__io_Output){.handle = 1}));
        zt_outcome_void_core_error_dispose(&__zt_try_result_0);
        __zt_try_result_0 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_void_core_error_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_0);
    goto zt_cleanup;

zt_block_try_success_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_try_after_2;

zt_block_try_after_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    running = true;
    next_seq = 1;
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (running) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_outcome_optional_text_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__io_read_line(NULL, ((zt_preview_runner__io_Input){.handle = 0}));
        zt_outcome_optional_text_core_error_dispose(&__zt_try_result_1);
        __zt_try_result_1 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_optional_text_core_error_is_success(__zt_try_result_1)) goto zt_block_try_success_3;
    goto zt_block_try_failure_4;

zt_block_try_failure_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_void_core_error_failure((__zt_try_result_1).error);
    goto zt_cleanup;

zt_block_try_success_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_optional_text __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_outcome_optional_text_core_error_value(__zt_try_result_1);
        line_opt = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    goto zt_block_try_after_5;

zt_block_try_after_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_optional_text __zt_assign_tmp = {0};
        __zt_assign_tmp = line_opt;
        __zt_match_subject_0 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    goto zt_block_match_case_1;

zt_block_match_case_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((zt_optional_text_is_present(__zt_match_subject_0) == false)) goto zt_block_match_body_2;
    goto zt_block_match_case_3;

zt_block_match_body_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    running = false;
    goto zt_block_match_after_0;

zt_block_match_case_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_optional_text_is_present(__zt_match_subject_0)) goto zt_block_match_body_4;
    goto zt_block_match_after_0;

zt_block_match_body_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_optional_text_value(__zt_match_subject_0);
        if (line != NULL) { zt_release(line); line = NULL; }
        line = __zt_assign_tmp;
    }
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__text_trim(NULL, line);
        if (trimmed != NULL) { zt_release(trimmed); trimmed = NULL; }
        trimmed = __zt_assign_tmp;
    }
    if (zt_preview_runner__text_is_empty(NULL, trimmed)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_while_cond_0;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_preview_runner__text_contains(NULL, trimmed, zt_text_from_utf8_literal("\"kind\":\"hello\""))) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_outcome_i64_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__respond_hello(NULL, next_seq);
        zt_outcome_i64_core_error_dispose(&__zt_try_result_2);
        __zt_try_result_2 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_i64_core_error_is_success(__zt_try_result_2)) goto zt_block_try_success_6;
    goto zt_block_try_failure_7;

zt_block_try_failure_7:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_void_core_error_failure((__zt_try_result_2).error);
    goto zt_cleanup;

zt_block_try_success_6:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    next_seq = zt_outcome_i64_core_error_value(__zt_try_result_2);
    goto zt_block_try_after_8;

zt_block_try_after_8:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_5;

zt_block_if_else_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_preview_runner__text_contains(NULL, trimmed, zt_text_from_utf8_literal("\"kind\":\"enter_play_mode\""))) goto zt_block_if_then_6;
    goto zt_block_if_else_7;

zt_block_if_then_6:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_outcome_i64_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__respond_play_mode(NULL, next_seq);
        zt_outcome_i64_core_error_dispose(&__zt_try_result_3);
        __zt_try_result_3 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_i64_core_error_is_success(__zt_try_result_3)) goto zt_block_try_success_9;
    goto zt_block_try_failure_10;

zt_block_try_failure_10:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_void_core_error_failure((__zt_try_result_3).error);
    goto zt_cleanup;

zt_block_try_success_9:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    next_seq = zt_outcome_i64_core_error_value(__zt_try_result_3);
    goto zt_block_try_after_11;

zt_block_try_after_11:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_8;

zt_block_if_else_7:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_preview_runner__text_contains(NULL, trimmed, zt_text_from_utf8_literal("\"kind\":\"pause_play_mode\""))) goto zt_block_if_then_9;
    goto zt_block_if_else_10;

zt_block_if_then_9:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_outcome_i64_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__respond_pause_play_mode(NULL, next_seq);
        zt_outcome_i64_core_error_dispose(&__zt_try_result_4);
        __zt_try_result_4 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_i64_core_error_is_success(__zt_try_result_4)) goto zt_block_try_success_12;
    goto zt_block_try_failure_13;

zt_block_try_failure_13:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_void_core_error_failure((__zt_try_result_4).error);
    goto zt_cleanup;

zt_block_try_success_12:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    next_seq = zt_outcome_i64_core_error_value(__zt_try_result_4);
    goto zt_block_try_after_14;

zt_block_try_after_14:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_11;

zt_block_if_else_10:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_preview_runner__text_contains(NULL, trimmed, zt_text_from_utf8_literal("\"kind\":\"stop_play_mode\""))) goto zt_block_if_then_12;
    goto zt_block_if_else_13;

zt_block_if_then_12:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_outcome_i64_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__respond_stop_play_mode(NULL, next_seq);
        zt_outcome_i64_core_error_dispose(&__zt_try_result_5);
        __zt_try_result_5 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_i64_core_error_is_success(__zt_try_result_5)) goto zt_block_try_success_15;
    goto zt_block_try_failure_16;

zt_block_try_failure_16:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_void_core_error_failure((__zt_try_result_5).error);
    goto zt_cleanup;

zt_block_try_success_15:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    next_seq = zt_outcome_i64_core_error_value(__zt_try_result_5);
    goto zt_block_try_after_17;

zt_block_try_after_17:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_14;

zt_block_if_else_13:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_preview_runner__text_contains(NULL, trimmed, zt_text_from_utf8_literal("\"kind\":\"ping\""))) goto zt_block_if_then_15;
    goto zt_block_if_else_16;

zt_block_if_then_15:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_outcome_i64_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__respond_pong(NULL, next_seq, trimmed);
        zt_outcome_i64_core_error_dispose(&__zt_try_result_6);
        __zt_try_result_6 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_i64_core_error_is_success(__zt_try_result_6)) goto zt_block_try_success_18;
    goto zt_block_try_failure_19;

zt_block_try_failure_19:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_void_core_error_failure((__zt_try_result_6).error);
    goto zt_cleanup;

zt_block_try_success_18:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    next_seq = zt_outcome_i64_core_error_value(__zt_try_result_6);
    goto zt_block_try_after_20;

zt_block_try_after_20:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_17;

zt_block_if_else_16:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_preview_runner__text_contains(NULL, trimmed, zt_text_from_utf8_literal("\"kind\":\"open_scene\""))) goto zt_block_if_then_18;
    goto zt_block_if_else_19;

zt_block_if_then_18:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_outcome_i64_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__respond_open_scene(NULL, next_seq, trimmed);
        zt_outcome_i64_core_error_dispose(&__zt_try_result_7);
        __zt_try_result_7 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_i64_core_error_is_success(__zt_try_result_7)) goto zt_block_try_success_21;
    goto zt_block_try_failure_22;

zt_block_try_failure_22:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_void_core_error_failure((__zt_try_result_7).error);
    goto zt_cleanup;

zt_block_try_success_21:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    next_seq = zt_outcome_i64_core_error_value(__zt_try_result_7);
    goto zt_block_try_after_23;

zt_block_try_after_23:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_20;

zt_block_if_else_19:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_preview_runner__text_contains(NULL, trimmed, zt_text_from_utf8_literal("\"kind\":\"open_project\""))) goto zt_block_if_then_21;
    goto zt_block_if_else_22;

zt_block_if_then_21:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_outcome_i64_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__respond_open_project(NULL, next_seq);
        zt_outcome_i64_core_error_dispose(&__zt_try_result_8);
        __zt_try_result_8 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_i64_core_error_is_success(__zt_try_result_8)) goto zt_block_try_success_24;
    goto zt_block_try_failure_25;

zt_block_try_failure_25:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_void_core_error_failure((__zt_try_result_8).error);
    goto zt_cleanup;

zt_block_try_success_24:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    next_seq = zt_outcome_i64_core_error_value(__zt_try_result_8);
    goto zt_block_try_after_26;

zt_block_try_after_26:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_23;

zt_block_if_else_22:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_preview_runner__text_contains(NULL, trimmed, zt_text_from_utf8_literal("\"kind\":\"select_entity\""))) goto zt_block_if_then_24;
    goto zt_block_if_else_25;

zt_block_if_then_24:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    next_seq = zt_add_i64(next_seq, 1);
    goto zt_block_if_join_26;

zt_block_if_else_25:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_preview_runner__text_contains(NULL, trimmed, zt_text_from_utf8_literal("\"kind\":\"focus_entity\""))) goto zt_block_if_then_27;
    goto zt_block_if_else_28;

zt_block_if_then_27:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    next_seq = zt_add_i64(next_seq, 1);
    goto zt_block_if_join_29;

zt_block_if_else_28:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_29;

zt_block_if_join_29:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_26;

zt_block_if_join_26:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_23;

zt_block_if_join_23:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_20;

zt_block_if_join_20:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_17;

zt_block_if_join_17:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_14;

zt_block_if_join_14:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_11;

zt_block_if_join_11:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_8;

zt_block_if_join_8:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_5;

zt_block_if_join_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_match_after_0;

zt_block_match_after_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_void_core_error_success();
    goto zt_cleanup;

zt_cleanup:
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    zt_outcome_optional_text_core_error_dispose(&__zt_try_result_1);
    if (line != NULL) { zt_release(line); line = NULL; }
    if (trimmed != NULL) { zt_release(trimmed); trimmed = NULL; }
    zt_outcome_i64_core_error_dispose(&__zt_try_result_2);
    zt_outcome_i64_core_error_dispose(&__zt_try_result_3);
    zt_outcome_i64_core_error_dispose(&__zt_try_result_4);
    zt_outcome_i64_core_error_dispose(&__zt_try_result_5);
    zt_outcome_i64_core_error_dispose(&__zt_try_result_6);
    zt_outcome_i64_core_error_dispose(&__zt_try_result_7);
    zt_outcome_i64_core_error_dispose(&__zt_try_result_8);
    return zt_return_value;
}

static zt_outcome_i64_core_error zt_preview_runner__respond_hello(void *zt_ctx, zt_int seq) {
    zt_int s;
    zt_text *hello_resp = NULL;
    zt_outcome_void_core_error __zt_try_result_0 = {0};
    zt_text *status_resp = NULL;
    zt_outcome_void_core_error __zt_try_result_1 = {0};
    zt_outcome_i64_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    s = seq;
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__build_envelope(NULL, s, zt_text_from_utf8_literal("event"), zt_text_from_utf8_literal("hello"), zt_text_from_utf8_literal("{\"role\":\"preview\",\"project_path\":\"\"}"));
        if (hello_resp != NULL) { zt_release(hello_resp); hello_resp = NULL; }
        hello_resp = __zt_assign_tmp;
    }
    {
        zt_outcome_void_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__io_write(NULL, hello_resp, ((zt_preview_runner__io_Output){.handle = 1}));
        zt_outcome_void_core_error_dispose(&__zt_try_result_0);
        __zt_try_result_0 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_void_core_error_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_i64_core_error_failure((__zt_try_result_0).error);
    goto zt_cleanup;

zt_block_try_success_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_try_after_2;

zt_block_try_after_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    s = zt_add_i64(s, 1);
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__build_envelope(NULL, s, zt_text_from_utf8_literal("event"), zt_text_from_utf8_literal("status"), zt_text_from_utf8_literal("{\"status\":\"ready\"}"));
        if (status_resp != NULL) { zt_release(status_resp); status_resp = NULL; }
        status_resp = __zt_assign_tmp;
    }
    {
        zt_outcome_void_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__io_write(NULL, status_resp, ((zt_preview_runner__io_Output){.handle = 1}));
        zt_outcome_void_core_error_dispose(&__zt_try_result_1);
        __zt_try_result_1 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_void_core_error_is_success(__zt_try_result_1)) goto zt_block_try_success_3;
    goto zt_block_try_failure_4;

zt_block_try_failure_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_i64_core_error_failure((__zt_try_result_1).error);
    goto zt_cleanup;

zt_block_try_success_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_try_after_5;

zt_block_try_after_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    s = zt_add_i64(s, 1);
    zt_return_value = zt_outcome_i64_core_error_success(s);
    goto zt_cleanup;

zt_cleanup:
    if (hello_resp != NULL) { zt_release(hello_resp); hello_resp = NULL; }
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    if (status_resp != NULL) { zt_release(status_resp); status_resp = NULL; }
    zt_outcome_void_core_error_dispose(&__zt_try_result_1);
    return zt_return_value;
}

static zt_outcome_i64_core_error zt_preview_runner__respond_play_mode(void *zt_ctx, zt_int seq) {
    zt_text *status_resp = NULL;
    zt_outcome_void_core_error __zt_try_result_0 = {0};
    zt_outcome_i64_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__build_envelope(NULL, seq, zt_text_from_utf8_literal("event"), zt_text_from_utf8_literal("status"), zt_text_from_utf8_literal("{\"status\":\"playing\"}"));
        if (status_resp != NULL) { zt_release(status_resp); status_resp = NULL; }
        status_resp = __zt_assign_tmp;
    }
    {
        zt_outcome_void_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__io_write(NULL, status_resp, ((zt_preview_runner__io_Output){.handle = 1}));
        zt_outcome_void_core_error_dispose(&__zt_try_result_0);
        __zt_try_result_0 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_void_core_error_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_i64_core_error_failure((__zt_try_result_0).error);
    goto zt_cleanup;

zt_block_try_success_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_try_after_2;

zt_block_try_after_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_i64_core_error_success(zt_add_i64(seq, 1));
    goto zt_cleanup;

zt_cleanup:
    if (status_resp != NULL) { zt_release(status_resp); status_resp = NULL; }
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    return zt_return_value;
}

static zt_outcome_i64_core_error zt_preview_runner__respond_pause_play_mode(void *zt_ctx, zt_int seq) {
    zt_text *status_resp = NULL;
    zt_outcome_void_core_error __zt_try_result_0 = {0};
    zt_outcome_i64_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__build_envelope(NULL, seq, zt_text_from_utf8_literal("event"), zt_text_from_utf8_literal("status"), zt_text_from_utf8_literal("{\"status\":\"paused\"}"));
        if (status_resp != NULL) { zt_release(status_resp); status_resp = NULL; }
        status_resp = __zt_assign_tmp;
    }
    {
        zt_outcome_void_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__io_write(NULL, status_resp, ((zt_preview_runner__io_Output){.handle = 1}));
        zt_outcome_void_core_error_dispose(&__zt_try_result_0);
        __zt_try_result_0 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_void_core_error_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_i64_core_error_failure((__zt_try_result_0).error);
    goto zt_cleanup;

zt_block_try_success_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_try_after_2;

zt_block_try_after_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_i64_core_error_success(zt_add_i64(seq, 1));
    goto zt_cleanup;

zt_cleanup:
    if (status_resp != NULL) { zt_release(status_resp); status_resp = NULL; }
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    return zt_return_value;
}

static zt_outcome_i64_core_error zt_preview_runner__respond_stop_play_mode(void *zt_ctx, zt_int seq) {
    zt_text *status_resp = NULL;
    zt_outcome_void_core_error __zt_try_result_0 = {0};
    zt_outcome_i64_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__build_envelope(NULL, seq, zt_text_from_utf8_literal("event"), zt_text_from_utf8_literal("status"), zt_text_from_utf8_literal("{\"status\":\"stopped\"}"));
        if (status_resp != NULL) { zt_release(status_resp); status_resp = NULL; }
        status_resp = __zt_assign_tmp;
    }
    {
        zt_outcome_void_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__io_write(NULL, status_resp, ((zt_preview_runner__io_Output){.handle = 1}));
        zt_outcome_void_core_error_dispose(&__zt_try_result_0);
        __zt_try_result_0 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_void_core_error_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_i64_core_error_failure((__zt_try_result_0).error);
    goto zt_cleanup;

zt_block_try_success_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_try_after_2;

zt_block_try_after_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_i64_core_error_success(zt_add_i64(seq, 1));
    goto zt_cleanup;

zt_cleanup:
    if (status_resp != NULL) { zt_release(status_resp); status_resp = NULL; }
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    return zt_return_value;
}

static zt_outcome_i64_core_error zt_preview_runner__respond_pong(void *zt_ctx, zt_int seq, zt_text *line) {
    zt_text *token = NULL;
    zt_text *pong_payload = NULL;
    zt_text *pong_resp = NULL;
    zt_outcome_void_core_error __zt_try_result_0 = {0};
    zt_outcome_i64_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__extract_ping_token(NULL, line);
        if (token != NULL) { zt_release(token); token = NULL; }
        token = __zt_assign_tmp;
    }
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__text_join(NULL, zt_list_text_from_array(((zt_text *[]){zt_text_from_utf8_literal("{\"token\":\""), token, zt_text_from_utf8_literal("\"}")}), 3));
        if (pong_payload != NULL) { zt_release(pong_payload); pong_payload = NULL; }
        pong_payload = __zt_assign_tmp;
    }
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__build_envelope(NULL, seq, zt_text_from_utf8_literal("heartbeat"), zt_text_from_utf8_literal("pong"), pong_payload);
        if (pong_resp != NULL) { zt_release(pong_resp); pong_resp = NULL; }
        pong_resp = __zt_assign_tmp;
    }
    {
        zt_outcome_void_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__io_write(NULL, pong_resp, ((zt_preview_runner__io_Output){.handle = 1}));
        zt_outcome_void_core_error_dispose(&__zt_try_result_0);
        __zt_try_result_0 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_void_core_error_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_i64_core_error_failure((__zt_try_result_0).error);
    goto zt_cleanup;

zt_block_try_success_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_try_after_2;

zt_block_try_after_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_i64_core_error_success(zt_add_i64(seq, 1));
    goto zt_cleanup;

zt_cleanup:
    if (token != NULL) { zt_release(token); token = NULL; }
    if (pong_payload != NULL) { zt_release(pong_payload); pong_payload = NULL; }
    if (pong_resp != NULL) { zt_release(pong_resp); pong_resp = NULL; }
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    return zt_return_value;
}

static zt_outcome_i64_core_error zt_preview_runner__respond_open_scene(void *zt_ctx, zt_int seq, zt_text *line) {
    zt_int s;
    zt_text *loading = NULL;
    zt_outcome_void_core_error __zt_try_result_0 = {0};
    zt_text *scene_path = NULL;
    zt_preview_runner__SceneLoadInfo scene_load = {0};
    zt_text *diagnostic_payload = NULL;
    zt_text *diagnostic = NULL;
    zt_outcome_void_core_error __zt_try_result_1 = {0};
    zt_text *ready = NULL;
    zt_outcome_void_core_error __zt_try_result_2 = {0};
    zt_outcome_i64_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    s = seq;
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__build_envelope(NULL, s, zt_text_from_utf8_literal("event"), zt_text_from_utf8_literal("status"), zt_text_from_utf8_literal("{\"status\":\"loading\"}"));
        if (loading != NULL) { zt_release(loading); loading = NULL; }
        loading = __zt_assign_tmp;
    }
    {
        zt_outcome_void_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__io_write(NULL, loading, ((zt_preview_runner__io_Output){.handle = 1}));
        zt_outcome_void_core_error_dispose(&__zt_try_result_0);
        __zt_try_result_0 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_void_core_error_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_i64_core_error_failure((__zt_try_result_0).error);
    goto zt_cleanup;

zt_block_try_success_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_try_after_2;

zt_block_try_after_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    s = zt_add_i64(s, 1);
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__extract_payload_path(NULL, line);
        if (scene_path != NULL) { zt_release(scene_path); scene_path = NULL; }
        scene_path = __zt_assign_tmp;
    }
    {
        zt_preview_runner__SceneLoadInfo __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__load_scene_info(NULL, scene_path);
    if (scene_load.path != NULL) { zt_release(scene_load.path); scene_load.path = NULL; }
        scene_load = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__text_join(NULL, zt_list_text_from_array(((zt_text *[]){zt_text_from_utf8_literal("{\"level\":\"info\",\"message\":\"open_scene path="), (scene_load.path), zt_text_from_utf8_literal(" entities="), zt_preview_runner__int_to_text(NULL, (scene_load.entity_count)), zt_text_from_utf8_literal(" loaded="), zt_preview_runner__bool_to_text(NULL, (scene_load.loaded)), zt_text_from_utf8_literal("\"}")}), 7));
        if (diagnostic_payload != NULL) { zt_release(diagnostic_payload); diagnostic_payload = NULL; }
        diagnostic_payload = __zt_assign_tmp;
    }
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__build_envelope(NULL, s, zt_text_from_utf8_literal("event"), zt_text_from_utf8_literal("diagnostic"), diagnostic_payload);
        if (diagnostic != NULL) { zt_release(diagnostic); diagnostic = NULL; }
        diagnostic = __zt_assign_tmp;
    }
    {
        zt_outcome_void_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__io_write(NULL, diagnostic, ((zt_preview_runner__io_Output){.handle = 1}));
        zt_outcome_void_core_error_dispose(&__zt_try_result_1);
        __zt_try_result_1 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_void_core_error_is_success(__zt_try_result_1)) goto zt_block_try_success_3;
    goto zt_block_try_failure_4;

zt_block_try_failure_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_i64_core_error_failure((__zt_try_result_1).error);
    goto zt_cleanup;

zt_block_try_success_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_try_after_5;

zt_block_try_after_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    s = zt_add_i64(s, 1);
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__build_envelope(NULL, s, zt_text_from_utf8_literal("event"), zt_text_from_utf8_literal("status"), zt_text_from_utf8_literal("{\"status\":\"ready\"}"));
        if (ready != NULL) { zt_release(ready); ready = NULL; }
        ready = __zt_assign_tmp;
    }
    {
        zt_outcome_void_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__io_write(NULL, ready, ((zt_preview_runner__io_Output){.handle = 1}));
        zt_outcome_void_core_error_dispose(&__zt_try_result_2);
        __zt_try_result_2 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_void_core_error_is_success(__zt_try_result_2)) goto zt_block_try_success_6;
    goto zt_block_try_failure_7;

zt_block_try_failure_7:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_i64_core_error_failure((__zt_try_result_2).error);
    goto zt_cleanup;

zt_block_try_success_6:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_try_after_8;

zt_block_try_after_8:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    s = zt_add_i64(s, 1);
    zt_return_value = zt_outcome_i64_core_error_success(s);
    goto zt_cleanup;

zt_cleanup:
    if (loading != NULL) { zt_release(loading); loading = NULL; }
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    if (scene_path != NULL) { zt_release(scene_path); scene_path = NULL; }
    if (scene_load.path != NULL) { zt_release(scene_load.path); scene_load.path = NULL; }
    if (diagnostic_payload != NULL) { zt_release(diagnostic_payload); diagnostic_payload = NULL; }
    if (diagnostic != NULL) { zt_release(diagnostic); diagnostic = NULL; }
    zt_outcome_void_core_error_dispose(&__zt_try_result_1);
    if (ready != NULL) { zt_release(ready); ready = NULL; }
    zt_outcome_void_core_error_dispose(&__zt_try_result_2);
    return zt_return_value;
}

static zt_preview_runner__SceneLoadInfo zt_preview_runner__load_scene_info(void *zt_ctx, zt_text *scene_path) {
    zt_outcome_text_core_error __zt_match_subject_0 = {0};
    zt_text *content = NULL;
    zt_core_error read_error = {0};
    zt_text *package_path = NULL;
    zt_outcome_text_core_error __zt_match_subject_1 = {0};
    zt_text *content2 = NULL;
    zt_core_error package_read_error = {0};
    zt_preview_runner__SceneLoadInfo zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_preview_runner__text_is_empty(NULL, scene_path)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = ((zt_preview_runner__SceneLoadInfo){.path = zt_text_from_utf8_literal(""), .entity_count = 0, .loaded = false});
    goto zt_cleanup;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_outcome_text_core_error __zt_assign_tmp = {0};
    {
        zt_text *zt_ffi_arg0 = scene_path;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        __zt_assign_tmp = zt_host_read_file(zt_ffi_arg0);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        zt_outcome_text_core_error_dispose(&__zt_match_subject_0);
        __zt_match_subject_0 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    goto zt_block_match_case_1;

zt_block_match_case_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_outcome_text_core_error_is_success(__zt_match_subject_0)) goto zt_block_match_body_2;
    goto zt_block_match_case_3;

zt_block_match_body_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_outcome_text_core_error_value(__zt_match_subject_0);
        if (content != NULL) { zt_release(content); content = NULL; }
        content = __zt_assign_tmp;
    }
    zt_return_value = ((zt_preview_runner__SceneLoadInfo){.path = (zt_retain(scene_path), scene_path), .entity_count = zt_preview_runner__count_occurrences(NULL, content, zt_text_from_utf8_literal("\"stable_id\"")), .loaded = true});
    goto zt_cleanup;

zt_block_match_case_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((zt_outcome_text_core_error_is_success(__zt_match_subject_0) == false)) goto zt_block_match_body_4;
    goto zt_block_match_after_0;

zt_block_match_body_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = (__zt_match_subject_0.error);
        __zt_assign_tmp = zt_core_error_clone(__zt_assign_tmp);
        zt_core_error_dispose(&read_error);
        read_error = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__text_join(NULL, zt_list_text_from_array(((zt_text *[]){zt_text_from_utf8_literal("packages/borealis/"), scene_path}), 2));
        if (package_path != NULL) { zt_release(package_path); package_path = NULL; }
        package_path = __zt_assign_tmp;
    }
    {
        zt_outcome_text_core_error __zt_assign_tmp = {0};
    {
        zt_text *zt_ffi_arg0 = package_path;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        __zt_assign_tmp = zt_host_read_file(zt_ffi_arg0);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        zt_outcome_text_core_error_dispose(&__zt_match_subject_1);
        __zt_match_subject_1 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    goto zt_block_match_case_6;

zt_block_match_case_6:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_outcome_text_core_error_is_success(__zt_match_subject_1)) goto zt_block_match_body_7;
    goto zt_block_match_case_8;

zt_block_match_body_7:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_outcome_text_core_error_value(__zt_match_subject_1);
        if (content2 != NULL) { zt_release(content2); content2 = NULL; }
        content2 = __zt_assign_tmp;
    }
    zt_return_value = ((zt_preview_runner__SceneLoadInfo){.path = (zt_retain(package_path), package_path), .entity_count = zt_preview_runner__count_occurrences(NULL, content2, zt_text_from_utf8_literal("\"stable_id\"")), .loaded = true});
    goto zt_cleanup;

zt_block_match_case_8:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((zt_outcome_text_core_error_is_success(__zt_match_subject_1) == false)) goto zt_block_match_body_9;
    goto zt_block_match_after_5;

zt_block_match_body_9:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = (__zt_match_subject_1.error);
        __zt_assign_tmp = zt_core_error_clone(__zt_assign_tmp);
        zt_core_error_dispose(&package_read_error);
        package_read_error = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    zt_return_value = ((zt_preview_runner__SceneLoadInfo){.path = (zt_retain(scene_path), scene_path), .entity_count = 0, .loaded = false});
    goto zt_cleanup;

zt_block_match_after_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_match_after_0;

zt_block_match_after_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_panic("unreachable");

zt_cleanup:
    zt_outcome_text_core_error_dispose(&__zt_match_subject_0);
    if (content != NULL) { zt_release(content); content = NULL; }
    zt_core_error_dispose(&read_error);
    if (package_path != NULL) { zt_release(package_path); package_path = NULL; }
    zt_outcome_text_core_error_dispose(&__zt_match_subject_1);
    if (content2 != NULL) { zt_release(content2); content2 = NULL; }
    zt_core_error_dispose(&package_read_error);
    return zt_return_value;
}

static zt_outcome_i64_core_error zt_preview_runner__respond_open_project(void *zt_ctx, zt_int seq) {
    zt_int s;
    zt_text *loading = NULL;
    zt_outcome_void_core_error __zt_try_result_0 = {0};
    zt_text *ready = NULL;
    zt_outcome_void_core_error __zt_try_result_1 = {0};
    zt_outcome_i64_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    s = seq;
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__build_envelope(NULL, s, zt_text_from_utf8_literal("event"), zt_text_from_utf8_literal("status"), zt_text_from_utf8_literal("{\"status\":\"loading\"}"));
        if (loading != NULL) { zt_release(loading); loading = NULL; }
        loading = __zt_assign_tmp;
    }
    {
        zt_outcome_void_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__io_write(NULL, loading, ((zt_preview_runner__io_Output){.handle = 1}));
        zt_outcome_void_core_error_dispose(&__zt_try_result_0);
        __zt_try_result_0 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_void_core_error_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_i64_core_error_failure((__zt_try_result_0).error);
    goto zt_cleanup;

zt_block_try_success_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_try_after_2;

zt_block_try_after_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    s = zt_add_i64(s, 1);
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__build_envelope(NULL, s, zt_text_from_utf8_literal("event"), zt_text_from_utf8_literal("status"), zt_text_from_utf8_literal("{\"status\":\"ready\"}"));
        if (ready != NULL) { zt_release(ready); ready = NULL; }
        ready = __zt_assign_tmp;
    }
    {
        zt_outcome_void_core_error __zt_assign_tmp = {0};
        __zt_assign_tmp = zt_preview_runner__io_write(NULL, ready, ((zt_preview_runner__io_Output){.handle = 1}));
        zt_outcome_void_core_error_dispose(&__zt_try_result_1);
        __zt_try_result_1 = __zt_assign_tmp;
        memset(&__zt_assign_tmp, 0, sizeof(__zt_assign_tmp));
    }
    if (zt_outcome_void_core_error_is_success(__zt_try_result_1)) goto zt_block_try_success_3;
    goto zt_block_try_failure_4;

zt_block_try_failure_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_outcome_i64_core_error_failure((__zt_try_result_1).error);
    goto zt_cleanup;

zt_block_try_success_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_try_after_5;

zt_block_try_after_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    s = zt_add_i64(s, 1);
    zt_return_value = zt_outcome_i64_core_error_success(s);
    goto zt_cleanup;

zt_cleanup:
    if (loading != NULL) { zt_release(loading); loading = NULL; }
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    if (ready != NULL) { zt_release(ready); ready = NULL; }
    zt_outcome_void_core_error_dispose(&__zt_try_result_1);
    return zt_return_value;
}

static zt_text *zt_preview_runner__build_envelope(void *zt_ctx, zt_int seq, zt_text *channel, zt_text *kind, zt_text *payload) {
    zt_text *zt_return_value = NULL;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_preview_runner__text_join(NULL, zt_list_text_from_array(((zt_text *[]){zt_text_from_utf8_literal("{\"protocol\":1,\"seq\":"), zt_preview_runner__int_to_text(NULL, seq), zt_text_from_utf8_literal(",\"channel\":\""), channel, zt_text_from_utf8_literal("\",\"kind\":\""), kind, zt_text_from_utf8_literal("\",\"payload\":"), payload, zt_text_from_utf8_literal("}\n")}), 9));
    goto zt_cleanup;

zt_cleanup:
    return zt_return_value;
}

static zt_text *zt_preview_runner__extract_ping_token(void *zt_ctx, zt_text *line) {
    zt_text *key = NULL;
    zt_int start_idx;
    zt_int value_start;
    zt_int end_idx;
    zt_text *zt_return_value = NULL;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_text_from_utf8_literal("\"token\":\"");
        if (key != NULL) { zt_release(key); key = NULL; }
        key = __zt_assign_tmp;
    }
    start_idx = zt_preview_runner__text_index_of(NULL, line, key);
    if ((start_idx < 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_from_utf8_literal("");
    goto zt_cleanup;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    value_start = zt_add_i64(start_idx, zt_text_len(key));
    end_idx = value_start;
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((end_idx < zt_text_len(line))) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_text_eq(zt_text_index(line, end_idx), zt_text_from_utf8_literal("\""))) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_while_exit_2;

zt_block_if_else_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_5;

zt_block_if_join_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    end_idx = zt_add_i64(end_idx, 1);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((value_start >= zt_text_len(line))) goto zt_block_if_then_6;
    goto zt_block_if_else_7;

zt_block_if_then_6:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_from_utf8_literal("");
    goto zt_cleanup;

zt_block_if_else_7:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_8;

zt_block_if_join_8:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((end_idx > zt_text_len(line))) goto zt_block_if_then_9;
    goto zt_block_if_else_10;

zt_block_if_then_9:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *zt_ffi_arg0 = line;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        end_idx = zt_text_len(zt_ffi_arg0);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
    goto zt_block_if_join_11;

zt_block_if_else_10:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_11;

zt_block_if_join_11:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_preview_runner__text__slice_from(NULL, zt_preview_runner__text__slice_to(NULL, line, zt_sub_i64(end_idx, 1)), value_start);
    goto zt_cleanup;

zt_cleanup:
    if (key != NULL) { zt_release(key); key = NULL; }
    return zt_return_value;
}

static zt_text *zt_preview_runner__extract_payload_path(void *zt_ctx, zt_text *line) {
    zt_text *key = NULL;
    zt_int start_idx;
    zt_int value_start;
    zt_int end_idx;
    zt_text *zt_return_value = NULL;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_text_from_utf8_literal("\"path\":\"");
        if (key != NULL) { zt_release(key); key = NULL; }
        key = __zt_assign_tmp;
    }
    start_idx = zt_preview_runner__text_index_of(NULL, line, key);
    if ((start_idx < 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_from_utf8_literal("");
    goto zt_cleanup;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    value_start = zt_add_i64(start_idx, zt_text_len(key));
    end_idx = value_start;
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((end_idx < zt_text_len(line))) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_text_eq(zt_text_index(line, end_idx), zt_text_from_utf8_literal("\""))) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_while_exit_2;

zt_block_if_else_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_5;

zt_block_if_join_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    end_idx = zt_add_i64(end_idx, 1);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((value_start >= zt_text_len(line))) goto zt_block_if_then_6;
    goto zt_block_if_else_7;

zt_block_if_then_6:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_from_utf8_literal("");
    goto zt_cleanup;

zt_block_if_else_7:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_8;

zt_block_if_join_8:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((end_idx > zt_text_len(line))) goto zt_block_if_then_9;
    goto zt_block_if_else_10;

zt_block_if_then_9:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *zt_ffi_arg0 = line;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        end_idx = zt_text_len(zt_ffi_arg0);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
    goto zt_block_if_join_11;

zt_block_if_else_10:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_11;

zt_block_if_join_11:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_preview_runner__text__slice_from(NULL, zt_preview_runner__text__slice_to(NULL, line, zt_sub_i64(end_idx, 1)), value_start);
    goto zt_cleanup;

zt_cleanup:
    if (key != NULL) { zt_release(key); key = NULL; }
    return zt_return_value;
}

static zt_int zt_preview_runner__count_occurrences(void *zt_ctx, zt_text *value, zt_text *needle) {
    zt_int count;
    zt_int index;
    zt_text *rest = NULL;
    zt_int found;
    zt_int zt_return_value;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_preview_runner__text_is_empty(NULL, needle)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = 0;
    goto zt_cleanup;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    count = 0;
    index = 0;
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((index < zt_text_len(value))) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__text__slice_from(NULL, value, index);
        if (rest != NULL) { zt_release(rest); rest = NULL; }
        rest = __zt_assign_tmp;
    }
    found = zt_preview_runner__text_index_of(NULL, rest, needle);
    if ((found < 0)) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_while_exit_2;

zt_block_if_else_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_5;

zt_block_if_join_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    count = zt_add_i64(count, 1);
    index = zt_add_i64(zt_add_i64(index, found), zt_text_len(needle));
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = count;
    goto zt_cleanup;

zt_cleanup:
    if (rest != NULL) { zt_release(rest); rest = NULL; }
    return zt_return_value;
}

static zt_text *zt_preview_runner__bool_to_text(void *zt_ctx, zt_bool value) {
    zt_text *zt_return_value = NULL;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (value) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_from_utf8_literal("true");
    goto zt_cleanup;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_from_utf8_literal("false");
    goto zt_cleanup;

zt_cleanup:
    return zt_return_value;
}

static zt_text *zt_preview_runner__int_to_text(void *zt_ctx, zt_int value) {
    zt_int v;
    zt_text *out = NULL;
    zt_int digit;
    zt_text *zt_return_value = NULL;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((value == 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_from_utf8_literal("0");
    goto zt_cleanup;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    v = value;
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_text_from_utf8_literal("");
        if (out != NULL) { zt_release(out); out = NULL; }
        out = __zt_assign_tmp;
    }
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((v > 0)) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    digit = zt_rem_i64(v, 10);
    if ((digit == 0)) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__text_join(NULL, zt_list_text_from_array(((zt_text *[]){zt_text_from_utf8_literal("0"), out}), 2));
        if (out != NULL) { zt_release(out); out = NULL; }
        out = __zt_assign_tmp;
    }
    goto zt_block_if_join_5;

zt_block_if_else_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((digit == 1)) goto zt_block_if_then_6;
    goto zt_block_if_else_7;

zt_block_if_then_6:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__text_join(NULL, zt_list_text_from_array(((zt_text *[]){zt_text_from_utf8_literal("1"), out}), 2));
        if (out != NULL) { zt_release(out); out = NULL; }
        out = __zt_assign_tmp;
    }
    goto zt_block_if_join_8;

zt_block_if_else_7:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((digit == 2)) goto zt_block_if_then_9;
    goto zt_block_if_else_10;

zt_block_if_then_9:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__text_join(NULL, zt_list_text_from_array(((zt_text *[]){zt_text_from_utf8_literal("2"), out}), 2));
        if (out != NULL) { zt_release(out); out = NULL; }
        out = __zt_assign_tmp;
    }
    goto zt_block_if_join_11;

zt_block_if_else_10:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((digit == 3)) goto zt_block_if_then_12;
    goto zt_block_if_else_13;

zt_block_if_then_12:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__text_join(NULL, zt_list_text_from_array(((zt_text *[]){zt_text_from_utf8_literal("3"), out}), 2));
        if (out != NULL) { zt_release(out); out = NULL; }
        out = __zt_assign_tmp;
    }
    goto zt_block_if_join_14;

zt_block_if_else_13:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((digit == 4)) goto zt_block_if_then_15;
    goto zt_block_if_else_16;

zt_block_if_then_15:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__text_join(NULL, zt_list_text_from_array(((zt_text *[]){zt_text_from_utf8_literal("4"), out}), 2));
        if (out != NULL) { zt_release(out); out = NULL; }
        out = __zt_assign_tmp;
    }
    goto zt_block_if_join_17;

zt_block_if_else_16:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((digit == 5)) goto zt_block_if_then_18;
    goto zt_block_if_else_19;

zt_block_if_then_18:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__text_join(NULL, zt_list_text_from_array(((zt_text *[]){zt_text_from_utf8_literal("5"), out}), 2));
        if (out != NULL) { zt_release(out); out = NULL; }
        out = __zt_assign_tmp;
    }
    goto zt_block_if_join_20;

zt_block_if_else_19:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((digit == 6)) goto zt_block_if_then_21;
    goto zt_block_if_else_22;

zt_block_if_then_21:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__text_join(NULL, zt_list_text_from_array(((zt_text *[]){zt_text_from_utf8_literal("6"), out}), 2));
        if (out != NULL) { zt_release(out); out = NULL; }
        out = __zt_assign_tmp;
    }
    goto zt_block_if_join_23;

zt_block_if_else_22:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((digit == 7)) goto zt_block_if_then_24;
    goto zt_block_if_else_25;

zt_block_if_then_24:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__text_join(NULL, zt_list_text_from_array(((zt_text *[]){zt_text_from_utf8_literal("7"), out}), 2));
        if (out != NULL) { zt_release(out); out = NULL; }
        out = __zt_assign_tmp;
    }
    goto zt_block_if_join_26;

zt_block_if_else_25:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((digit == 8)) goto zt_block_if_then_27;
    goto zt_block_if_else_28;

zt_block_if_then_27:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__text_join(NULL, zt_list_text_from_array(((zt_text *[]){zt_text_from_utf8_literal("8"), out}), 2));
        if (out != NULL) { zt_release(out); out = NULL; }
        out = __zt_assign_tmp;
    }
    goto zt_block_if_join_29;

zt_block_if_else_28:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((digit == 9)) goto zt_block_if_then_30;
    goto zt_block_if_else_31;

zt_block_if_then_30:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_preview_runner__text_join(NULL, zt_list_text_from_array(((zt_text *[]){zt_text_from_utf8_literal("9"), out}), 2));
        if (out != NULL) { zt_release(out); out = NULL; }
        out = __zt_assign_tmp;
    }
    goto zt_block_if_join_32;

zt_block_if_else_31:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_32;

zt_block_if_join_32:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_29;

zt_block_if_join_29:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_26;

zt_block_if_join_26:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_23;

zt_block_if_join_23:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_20;

zt_block_if_join_20:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_17;

zt_block_if_join_17:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_14;

zt_block_if_join_14:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_11;

zt_block_if_join_11:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_8;

zt_block_if_join_8:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_5;

zt_block_if_join_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    v = zt_div_i64(v, 10);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = out;
    out = NULL;
    goto zt_cleanup;

zt_cleanup:
    if (out != NULL) { zt_release(out); out = NULL; }
    return zt_return_value;
}

static zt_outcome_optional_text_core_error zt_preview_runner__io_read_line(void *zt_ctx, zt_preview_runner__io_Input from) {
    zt_outcome_optional_text_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_host_read_line_stdin();
    goto zt_cleanup;

zt_cleanup:
    return zt_return_value;
}

static zt_outcome_text_core_error zt_preview_runner__io_read_all(void *zt_ctx, zt_preview_runner__io_Input from) {
    zt_outcome_text_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_host_read_all_stdin();
    goto zt_cleanup;

zt_cleanup:
    return zt_return_value;
}

static zt_outcome_void_core_error zt_preview_runner__io_write(void *zt_ctx, zt_text *value, zt_preview_runner__io_Output to) {
    zt_outcome_void_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (((to.handle) == 2)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_host_write_stderr(value);
    goto zt_cleanup;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_host_write_stdout(value);
    goto zt_cleanup;

zt_cleanup:
    return zt_return_value;
}

static zt_outcome_void_core_error zt_preview_runner__io_print(void *zt_ctx, zt_text *value, zt_preview_runner__io_Output to) {
    zt_outcome_void_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_preview_runner__io_write(NULL, value, to);
    goto zt_cleanup;

zt_cleanup:
    return zt_return_value;
}

static zt_bool zt_preview_runner__text__eq(void *zt_ctx, zt_text *a, zt_text *b) {
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return zt_text_eq(a, b);
}

static zt_text *zt_preview_runner__text__slice_from(void *zt_ctx, zt_text *value, zt_int start) {
    zt_text *zt_return_value = NULL;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((zt_text_len(value) == 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_from_utf8_literal("");
    goto zt_cleanup;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((start <= 0)) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = (zt_retain(value), value);
    goto zt_cleanup;

zt_block_if_else_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_5;

zt_block_if_join_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((start >= zt_text_len(value))) goto zt_block_if_then_6;
    goto zt_block_if_else_7;

zt_block_if_then_6:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_from_utf8_literal("");
    goto zt_cleanup;

zt_block_if_else_7:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_8;

zt_block_if_join_8:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_slice(value, start, zt_sub_i64(zt_text_len(value), 1));
    goto zt_cleanup;

zt_cleanup:
    return zt_return_value;
}

static zt_text *zt_preview_runner__text__slice_to(void *zt_ctx, zt_text *value, zt_int finish) {
    zt_text *zt_return_value = NULL;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((zt_text_len(value) == 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_from_utf8_literal("");
    goto zt_cleanup;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((finish < 0)) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_from_utf8_literal("");
    goto zt_cleanup;

zt_block_if_else_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_5;

zt_block_if_join_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((finish >= zt_text_len(value))) goto zt_block_if_then_6;
    goto zt_block_if_else_7;

zt_block_if_then_6:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = (zt_retain(value), value);
    goto zt_cleanup;

zt_block_if_else_7:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_8;

zt_block_if_join_8:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_slice(value, 0, finish);
    goto zt_cleanup;

zt_cleanup:
    return zt_return_value;
}

static zt_bool zt_preview_runner__text__is_whitespace_char(void *zt_ctx, zt_text *ch) {
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((((zt_text_eq(ch, zt_text_from_utf8_literal(" ")) || zt_text_eq(ch, zt_text_from_utf8_literal("\t"))) || zt_text_eq(ch, zt_text_from_utf8_literal("\n"))) || zt_text_eq(ch, zt_text_from_utf8_literal("\r")))) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return true;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return false;
}

static zt_bool zt_preview_runner__text__is_ascii_digit_char(void *zt_ctx, zt_text *ch) {
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (((((zt_text_eq(ch, zt_text_from_utf8_literal("0")) || zt_text_eq(ch, zt_text_from_utf8_literal("1"))) || zt_text_eq(ch, zt_text_from_utf8_literal("2"))) || zt_text_eq(ch, zt_text_from_utf8_literal("3"))) || zt_text_eq(ch, zt_text_from_utf8_literal("4")))) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return true;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (((((zt_text_eq(ch, zt_text_from_utf8_literal("5")) || zt_text_eq(ch, zt_text_from_utf8_literal("6"))) || zt_text_eq(ch, zt_text_from_utf8_literal("7"))) || zt_text_eq(ch, zt_text_from_utf8_literal("8"))) || zt_text_eq(ch, zt_text_from_utf8_literal("9")))) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return true;

zt_block_if_else_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_5;

zt_block_if_join_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return false;
}

static zt_bool zt_preview_runner__text__starts_at(void *zt_ctx, zt_text *value, zt_text *needle, zt_int start) {
    zt_int offset;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    offset = 0;
    if ((zt_text_len(needle) == 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return true;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((start < 0)) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return false;

zt_block_if_else_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_5;

zt_block_if_join_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((zt_add_i64(start, zt_text_len(needle)) > zt_text_len(value))) goto zt_block_if_then_6;
    goto zt_block_if_else_7;

zt_block_if_then_6:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return false;

zt_block_if_else_7:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_8;

zt_block_if_join_8:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((offset < zt_text_len(needle))) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((zt_preview_runner__text__eq(NULL, zt_text_index(value, zt_add_i64(start, offset)), zt_text_index(needle, offset)) == false)) goto zt_block_if_then_9;
    goto zt_block_if_else_10;

zt_block_if_then_9:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return false;

zt_block_if_else_10:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_11;

zt_block_if_join_11:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    offset = zt_add_i64(offset, 1);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return true;
}

static zt_bytes *zt_preview_runner__text_to_utf8(void *zt_ctx, zt_text *value) {
    zt_bytes *zt_return_value = NULL;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *zt_ffi_arg0 = value;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_return_value = zt_text_to_utf8_bytes(zt_ffi_arg0);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
    goto zt_cleanup;

zt_cleanup:
    return zt_return_value;
}

static zt_outcome_text_text zt_preview_runner__text_from_utf8(void *zt_ctx, zt_bytes *value) {
    zt_outcome_text_text zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_from_utf8_bytes(value);
    goto zt_cleanup;

zt_cleanup:
    return zt_return_value;
}

static zt_text *zt_preview_runner__text_trim(void *zt_ctx, zt_text *value) {
    zt_text *zt_return_value = NULL;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_preview_runner__text_trim_end(NULL, zt_preview_runner__text_trim_start(NULL, value));
    goto zt_cleanup;

zt_cleanup:
    return zt_return_value;
}

static zt_text *zt_preview_runner__text_trim_start(void *zt_ctx, zt_text *value) {
    zt_int index;
    zt_text *zt_return_value = NULL;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    index = 0;
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((index < zt_text_len(value))) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((zt_preview_runner__text__is_whitespace_char(NULL, zt_text_index(value, index)) == false)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_preview_runner__text__slice_from(NULL, value, index);
    goto zt_cleanup;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    index = zt_add_i64(index, 1);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_from_utf8_literal("");
    goto zt_cleanup;

zt_cleanup:
    return zt_return_value;
}

static zt_text *zt_preview_runner__text_trim_end(void *zt_ctx, zt_text *value) {
    zt_int index;
    zt_text *zt_return_value = NULL;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    index = zt_sub_i64(zt_text_len(value), 1);
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((index >= 0)) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((zt_preview_runner__text__is_whitespace_char(NULL, zt_text_index(value, index)) == false)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_preview_runner__text__slice_to(NULL, value, index);
    goto zt_cleanup;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    index = zt_sub_i64(index, 1);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_from_utf8_literal("");
    goto zt_cleanup;

zt_cleanup:
    return zt_return_value;
}

static zt_bool zt_preview_runner__text_contains(void *zt_ctx, zt_text *value, zt_text *needle) {
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return (zt_preview_runner__text_index_of(NULL, value, needle) >= 0);
}

static zt_text *zt_preview_runner__text_join(void *zt_ctx, zt_list_text *parts) {
    zt_text *joined = NULL;
    zt_int index;
    zt_text *zt_return_value = NULL;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_text_from_utf8_literal("");
        if (joined != NULL) { zt_release(joined); joined = NULL; }
        joined = __zt_assign_tmp;
    }
    index = 0;
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((index < zt_list_text_len(parts))) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
    {
        zt_text *zt_ffi_arg0 = joined;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg1 = zt_list_text_get(parts, index);
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        __zt_assign_tmp = zt_text_concat(zt_ffi_arg0, zt_ffi_arg1);
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        if (joined != NULL) { zt_release(joined); joined = NULL; }
        joined = __zt_assign_tmp;
    }
    index = zt_add_i64(index, 1);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = joined;
    joined = NULL;
    goto zt_cleanup;

zt_cleanup:
    if (joined != NULL) { zt_release(joined); joined = NULL; }
    return zt_return_value;
}

static zt_text *zt_preview_runner__text_replace_all(void *zt_ctx, zt_text *value, zt_text *needle, zt_text *replacement) {
    zt_text *replaced = NULL;
    zt_int index;
    zt_text *zt_return_value = NULL;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
        __zt_assign_tmp = zt_text_from_utf8_literal("");
        if (replaced != NULL) { zt_release(replaced); replaced = NULL; }
        replaced = __zt_assign_tmp;
    }
    index = 0;
    if ((zt_text_len(needle) == 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = (zt_retain(value), value);
    goto zt_cleanup;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((index < zt_text_len(value))) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_preview_runner__text__starts_at(NULL, value, needle, index)) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
    {
        zt_text *zt_ffi_arg0 = replaced;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg1 = replacement;
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        __zt_assign_tmp = zt_text_concat(zt_ffi_arg0, zt_ffi_arg1);
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        if (replaced != NULL) { zt_release(replaced); replaced = NULL; }
        replaced = __zt_assign_tmp;
    }
    index = zt_add_i64(index, zt_text_len(needle));
    goto zt_block_if_join_5;

zt_block_if_else_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    {
        zt_text *__zt_assign_tmp = NULL;
    {
        zt_text *zt_ffi_arg0 = replaced;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg1 = zt_text_index(value, index);
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        __zt_assign_tmp = zt_text_concat(zt_ffi_arg0, zt_ffi_arg1);
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        if (replaced != NULL) { zt_release(replaced); replaced = NULL; }
        replaced = __zt_assign_tmp;
    }
    index = zt_add_i64(index, 1);
    goto zt_block_if_join_5;

zt_block_if_join_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = replaced;
    replaced = NULL;
    goto zt_cleanup;

zt_cleanup:
    if (replaced != NULL) { zt_release(replaced); replaced = NULL; }
    return zt_return_value;
}

static zt_bool zt_preview_runner__text_starts_with(void *zt_ctx, zt_text *value, zt_text *prefix) {
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return zt_preview_runner__text__starts_at(NULL, value, prefix, 0);
}

static zt_bool zt_preview_runner__text_ends_with(void *zt_ctx, zt_text *value, zt_text *suffix) {
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((zt_text_len(suffix) == 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return true;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((zt_text_len(suffix) > zt_text_len(value))) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return false;

zt_block_if_else_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_5;

zt_block_if_join_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return zt_preview_runner__text__starts_at(NULL, value, suffix, zt_sub_i64(zt_text_len(value), zt_text_len(suffix)));
}

static zt_bool zt_preview_runner__text_has_prefix(void *zt_ctx, zt_text *value, zt_text *prefix) {
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return zt_preview_runner__text_starts_with(NULL, value, prefix);
}

static zt_bool zt_preview_runner__text_has_suffix(void *zt_ctx, zt_text *value, zt_text *suffix) {
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return zt_preview_runner__text_ends_with(NULL, value, suffix);
}

static zt_bool zt_preview_runner__text_has_whitespace(void *zt_ctx, zt_text *value) {
    zt_int index;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    index = 0;
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((index < zt_text_len(value))) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_preview_runner__text__is_whitespace_char(NULL, zt_text_index(value, index))) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return true;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    index = zt_add_i64(index, 1);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return false;
}

static zt_int zt_preview_runner__text_index_of(void *zt_ctx, zt_text *value, zt_text *needle) {
    zt_int index;
    zt_int max_index;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    index = 0;
    max_index = zt_sub_i64(zt_text_len(value), zt_text_len(needle));
    if ((zt_text_len(needle) == 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return 0;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((zt_text_len(needle) > zt_text_len(value))) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return (-1);

zt_block_if_else_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_5;

zt_block_if_join_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((index <= max_index)) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_preview_runner__text__starts_at(NULL, value, needle, index)) goto zt_block_if_then_6;
    goto zt_block_if_else_7;

zt_block_if_then_6:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return index;

zt_block_if_else_7:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_8;

zt_block_if_join_8:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    index = zt_add_i64(index, 1);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return (-1);
}

static zt_int zt_preview_runner__text_last_index_of(void *zt_ctx, zt_text *value, zt_text *needle) {
    zt_int index;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    index = zt_sub_i64(zt_text_len(value), zt_text_len(needle));
    if ((zt_text_len(needle) == 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return zt_text_len(value);

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((zt_text_len(needle) > zt_text_len(value))) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return (-1);

zt_block_if_else_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_5;

zt_block_if_join_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((index >= 0)) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if (zt_preview_runner__text__starts_at(NULL, value, needle, index)) goto zt_block_if_then_6;
    goto zt_block_if_else_7;

zt_block_if_then_6:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return index;

zt_block_if_else_7:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_8;

zt_block_if_join_8:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    index = zt_sub_i64(index, 1);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return (-1);
}

static zt_bool zt_preview_runner__text_is_empty(void *zt_ctx, zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return (zt_text_len(value) == 0);
}

static zt_bool zt_preview_runner__text_is_digits(void *zt_ctx, zt_text *value) {
    zt_int index;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    index = 0;
    if ((zt_text_len(value) == 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return false;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((index < zt_text_len(value))) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((zt_preview_runner__text__is_ascii_digit_char(NULL, zt_text_index(value, index)) == false)) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return false;

zt_block_if_else_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_5;

zt_block_if_join_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    index = zt_add_i64(index, 1);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    return true;
}

static zt_text *zt_preview_runner__text_limit(void *zt_ctx, zt_text *value, zt_int max_len) {
    zt_text *zt_return_value = NULL;
    goto zt_block_entry;

zt_block_entry:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((max_len <= 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_from_utf8_literal("");
    goto zt_cleanup;

zt_block_if_else_1:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_2;

zt_block_if_join_2:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    if ((zt_text_len(value) <= max_len)) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = (zt_retain(value), value);
    goto zt_cleanup;

zt_block_if_else_4:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    goto zt_block_if_join_5;

zt_block_if_join_5:
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((unused));
#endif
    zt_return_value = zt_text_slice(value, 0, zt_sub_i64(max_len, 1));
    goto zt_cleanup;

zt_cleanup:
    return zt_return_value;
}

int main(int argc, char **argv) {
    zt_runtime_capture_process_args(argc, argv);
    zt_outcome_void_core_error __zt_main_result = zt_preview_runner__main(NULL);
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