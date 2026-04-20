#include "runtime/c/zenith_rt.h"
#include <stdio.h>

static zt_outcome_void_core_error zt_app_main__main(void);
static zt_bytes *zt_app_main__bytes_empty(void);
static zt_bytes *zt_app_main__bytes_from_list(zt_list_i64 *values);
static zt_list_i64 *zt_app_main__bytes_to_list(zt_bytes *value);
static zt_bytes *zt_app_main__bytes_join(zt_bytes *left, zt_bytes *right);
static zt_bool zt_app_main__bytes_starts_with(zt_bytes *value, zt_bytes *prefix);
static zt_bool zt_app_main__bytes_ends_with(zt_bytes *value, zt_bytes *suffix);
static zt_bool zt_app_main__bytes_contains(zt_bytes *value, zt_bytes *part);
static zt_list_i64 *zt_app_main__collections_queue_int_new(void);
static zt_list_i64 *zt_app_main__collections_queue_int_enqueue(zt_list_i64 *queue, zt_int value);
static zt_optional_i64 zt_app_main__collections_queue_int_dequeue(zt_list_i64 *queue);
static zt_optional_i64 zt_app_main__collections_queue_int_peek(zt_list_i64 *queue);
static zt_list_text *zt_app_main__collections_queue_text_new(void);
static zt_list_text *zt_app_main__collections_queue_text_enqueue(zt_list_text *queue, zt_text *value);
static zt_optional_text zt_app_main__collections_queue_text_dequeue(zt_list_text *queue);
static zt_optional_text zt_app_main__collections_queue_text_peek(zt_list_text *queue);
static zt_list_i64 *zt_app_main__collections_stack_int_new(void);
static zt_list_i64 *zt_app_main__collections_stack_int_push(zt_list_i64 *stack, zt_int value);
static zt_optional_i64 zt_app_main__collections_stack_int_pop(zt_list_i64 *stack);
static zt_optional_i64 zt_app_main__collections_stack_int_peek(zt_list_i64 *stack);
static zt_list_text *zt_app_main__collections_stack_text_new(void);
static zt_list_text *zt_app_main__collections_stack_text_push(zt_list_text *stack, zt_text *value);
static zt_optional_text zt_app_main__collections_stack_text_pop(zt_list_text *stack);
static zt_optional_text zt_app_main__collections_stack_text_peek(zt_list_text *stack);
static zt_grid2d_i64 *zt_app_main__collections_grid2d_int_new(zt_int rows, zt_int cols);
static zt_int zt_app_main__collections_grid2d_int_get(zt_grid2d_i64 *grid, zt_int row, zt_int col);
static zt_grid2d_i64 *zt_app_main__collections_grid2d_int_set(zt_grid2d_i64 *grid, zt_int row, zt_int col, zt_int value);
static zt_grid2d_i64 *zt_app_main__collections_grid2d_int_fill(zt_grid2d_i64 *grid, zt_int value);
static zt_int zt_app_main__collections_grid2d_int_rows(zt_grid2d_i64 *grid);
static zt_int zt_app_main__collections_grid2d_int_cols(zt_grid2d_i64 *grid);
static zt_grid2d_text *zt_app_main__collections_grid2d_text_new(zt_int rows, zt_int cols);
static zt_text *zt_app_main__collections_grid2d_text_get(zt_grid2d_text *grid, zt_int row, zt_int col);
static zt_grid2d_text *zt_app_main__collections_grid2d_text_set(zt_grid2d_text *grid, zt_int row, zt_int col, zt_text *value);
static zt_grid2d_text *zt_app_main__collections_grid2d_text_fill(zt_grid2d_text *grid, zt_text *value);
static zt_int zt_app_main__collections_grid2d_text_rows(zt_grid2d_text *grid);
static zt_int zt_app_main__collections_grid2d_text_cols(zt_grid2d_text *grid);
static zt_pqueue_i64 *zt_app_main__collections_pqueue_int_new(void);
static zt_pqueue_i64 *zt_app_main__collections_pqueue_int_push(zt_pqueue_i64 *heap, zt_int value);
static zt_optional_i64 zt_app_main__collections_pqueue_int_pop(zt_pqueue_i64 *heap);
static zt_optional_i64 zt_app_main__collections_pqueue_int_peek(zt_pqueue_i64 *heap);
static zt_int zt_app_main__collections_pqueue_int_len(zt_pqueue_i64 *heap);
static zt_pqueue_text *zt_app_main__collections_pqueue_text_new(void);
static zt_pqueue_text *zt_app_main__collections_pqueue_text_push(zt_pqueue_text *heap, zt_text *value);
static zt_optional_text zt_app_main__collections_pqueue_text_pop(zt_pqueue_text *heap);
static zt_optional_text zt_app_main__collections_pqueue_text_peek(zt_pqueue_text *heap);
static zt_int zt_app_main__collections_pqueue_text_len(zt_pqueue_text *heap);
static zt_circbuf_i64 *zt_app_main__collections_circbuf_int_new(zt_int capacity);
static zt_circbuf_i64 *zt_app_main__collections_circbuf_int_push(zt_circbuf_i64 *buf, zt_int value);
static zt_optional_i64 zt_app_main__collections_circbuf_int_pop(zt_circbuf_i64 *buf);
static zt_optional_i64 zt_app_main__collections_circbuf_int_peek(zt_circbuf_i64 *buf);
static zt_int zt_app_main__collections_circbuf_int_len(zt_circbuf_i64 *buf);
static zt_int zt_app_main__collections_circbuf_int_capacity(zt_circbuf_i64 *buf);
static zt_bool zt_app_main__collections_circbuf_int_is_full(zt_circbuf_i64 *buf);
static zt_circbuf_text *zt_app_main__collections_circbuf_text_new(zt_int capacity);
static zt_circbuf_text *zt_app_main__collections_circbuf_text_push(zt_circbuf_text *buf, zt_text *value);
static zt_optional_text zt_app_main__collections_circbuf_text_pop(zt_circbuf_text *buf);
static zt_optional_text zt_app_main__collections_circbuf_text_peek(zt_circbuf_text *buf);
static zt_int zt_app_main__collections_circbuf_text_len(zt_circbuf_text *buf);
static zt_int zt_app_main__collections_circbuf_text_capacity(zt_circbuf_text *buf);
static zt_bool zt_app_main__collections_circbuf_text_is_full(zt_circbuf_text *buf);
static zt_btreemap_text_text *zt_app_main__collections_btreemap_text_new(void);
static zt_btreemap_text_text *zt_app_main__collections_btreemap_text_set(zt_btreemap_text_text *self_map, zt_text *key, zt_text *value);
static zt_text *zt_app_main__collections_btreemap_text_get(zt_btreemap_text_text *self_map, zt_text *key);
static zt_optional_text zt_app_main__collections_btreemap_text_get_optional(zt_btreemap_text_text *self_map, zt_text *key);
static zt_bool zt_app_main__collections_btreemap_text_contains(zt_btreemap_text_text *self_map, zt_text *key);
static zt_btreemap_text_text *zt_app_main__collections_btreemap_text_remove(zt_btreemap_text_text *self_map, zt_text *key);
static zt_int zt_app_main__collections_btreemap_text_len(zt_btreemap_text_text *self_map);
static zt_btreeset_text *zt_app_main__collections_btreeset_text_new(void);
static zt_btreeset_text *zt_app_main__collections_btreeset_text_insert(zt_btreeset_text *set, zt_text *value);
static zt_bool zt_app_main__collections_btreeset_text_contains(zt_btreeset_text *set, zt_text *value);
static zt_btreeset_text *zt_app_main__collections_btreeset_text_remove(zt_btreeset_text *set, zt_text *value);
static zt_int zt_app_main__collections_btreeset_text_len(zt_btreeset_text *set);
static zt_grid3d_i64 *zt_app_main__collections_grid3d_int_new(zt_int depth, zt_int rows, zt_int cols);
static zt_int zt_app_main__collections_grid3d_int_get(zt_grid3d_i64 *grid, zt_int layer, zt_int row, zt_int col);
static zt_grid3d_i64 *zt_app_main__collections_grid3d_int_set(zt_grid3d_i64 *grid, zt_int layer, zt_int row, zt_int col, zt_int value);
static zt_grid3d_i64 *zt_app_main__collections_grid3d_int_fill(zt_grid3d_i64 *grid, zt_int value);
static zt_int zt_app_main__collections_grid3d_int_depth(zt_grid3d_i64 *grid);
static zt_int zt_app_main__collections_grid3d_int_rows(zt_grid3d_i64 *grid);
static zt_int zt_app_main__collections_grid3d_int_cols(zt_grid3d_i64 *grid);
static zt_grid3d_text *zt_app_main__collections_grid3d_text_new(zt_int depth, zt_int rows, zt_int cols);
static zt_text *zt_app_main__collections_grid3d_text_get(zt_grid3d_text *grid, zt_int layer, zt_int row, zt_int col);
static zt_grid3d_text *zt_app_main__collections_grid3d_text_set(zt_grid3d_text *grid, zt_int layer, zt_int row, zt_int col, zt_text *value);
static zt_grid3d_text *zt_app_main__collections_grid3d_text_fill(zt_grid3d_text *grid, zt_text *value);
static zt_int zt_app_main__collections_grid3d_text_depth(zt_grid3d_text *grid);
static zt_int zt_app_main__collections_grid3d_text_rows(zt_grid3d_text *grid);
static zt_int zt_app_main__collections_grid3d_text_cols(zt_grid3d_text *grid);
static zt_bool zt_app_main__collections_pqueue_int_is_empty(zt_pqueue_i64 *heap);
static zt_bool zt_app_main__collections_pqueue_text_is_empty(zt_pqueue_text *heap);
static zt_bool zt_app_main__collections_circbuf_int_is_empty(zt_circbuf_i64 *buf);
static zt_bool zt_app_main__collections_circbuf_text_is_empty(zt_circbuf_text *buf);
static zt_bool zt_app_main__collections_btreemap_text_is_empty(zt_btreemap_text_text *self_map);
static zt_bool zt_app_main__collections_btreeset_text_is_empty(zt_btreeset_text *set);
static zt_int zt_app_main__collections_grid2d_int_size(zt_grid2d_i64 *grid);
static zt_int zt_app_main__collections_grid2d_text_size(zt_grid2d_text *grid);
static zt_int zt_app_main__collections_grid3d_int_size(zt_grid3d_i64 *grid);
static zt_int zt_app_main__collections_grid3d_text_size(zt_grid3d_text *grid);
static zt_outcome_optional_text_core_error zt_app_main__io_read_line(zt_bool from);
static zt_outcome_text_core_error zt_app_main__io_read_all(zt_bool from);
static zt_outcome_void_core_error zt_app_main__io_write(zt_text *value, zt_bool to);
static zt_outcome_void_core_error zt_app_main__io_print(zt_text *value, zt_bool to);
static zt_outcome_void_core_error zt_app_main__io_print_line(zt_text *value, zt_bool to);
static zt_outcome_void_core_error zt_app_main__io_eprint(zt_text *value);
static zt_outcome_void_core_error zt_app_main__io_eprint_line(zt_text *value);

static zt_outcome_void_core_error zt_app_main__main(void) {
    zt_int score;
    zt_bytes *payload = NULL;
    zt_bytes *joined = NULL;
    zt_list_i64 *roundtrip = NULL;
    zt_list_i64 *xs = NULL;
    zt_text *mode_key = NULL;
    zt_text *mode_value = NULL;
    zt_text *theme_key = NULL;
    zt_text *theme_value = NULL;
    zt_text *dark_value = NULL;
    zt_map_text_text *cfg = NULL;
    zt_grid2d_i64 *g2 = NULL;
    zt_pqueue_i64 *pq = NULL;
    zt_circbuf_i64 *cb = NULL;
    zt_btreemap_text_text *bm = NULL;
    zt_btreeset_text *bs = NULL;
    zt_grid3d_i64 *g3 = NULL;
    zt_outcome_void_core_error __zt_try_result_0 = {0};
    zt_outcome_void_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
    score = 0;
    if (payload != NULL) { zt_release(payload); payload = NULL; }
    {
        zt_list_i64 *zt_ffi_arg0 = zt_list_i64_from_array(((zt_int[]){1, 2, 3, 4}), 4);
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        payload = zt_bytes_from_list_i64(zt_ffi_arg0);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
    if (joined != NULL) { zt_release(joined); joined = NULL; }
    {
        zt_bytes *zt_ffi_arg0 = payload;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_bytes *zt_ffi_arg1 = zt_bytes_from_array((const uint8_t[]){0x05, 0x06}, 2);
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        joined = zt_bytes_join(zt_ffi_arg0, zt_ffi_arg1);
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
    if (roundtrip != NULL) { zt_release(roundtrip); roundtrip = NULL; }
    {
        zt_bytes *zt_ffi_arg0 = joined;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        roundtrip = zt_bytes_to_list_i64(zt_ffi_arg0);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
    if ((zt_list_i64_len(roundtrip) == 6)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    score = zt_add_i64(score, 1);
    goto zt_block_if_join_2;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    if (xs != NULL) { zt_release(xs); xs = NULL; }
    xs = zt_list_i64_from_array(((zt_int[]){10, 20, 30}), 3);
    xs = zt_list_i64_set_owned(xs, 1, 99);
    if ((zt_list_i64_get(xs, 1) == 99)) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
    score = zt_add_i64(score, 1);
    goto zt_block_if_join_5;

zt_block_if_else_4:
    goto zt_block_if_join_5;

zt_block_if_join_5:
    if (mode_key != NULL) { zt_release(mode_key); mode_key = NULL; }
    mode_key = zt_text_from_utf8_literal("mode");
    if (mode_value != NULL) { zt_release(mode_value); mode_value = NULL; }
    mode_value = zt_text_from_utf8_literal("debug");
    if (theme_key != NULL) { zt_release(theme_key); theme_key = NULL; }
    theme_key = zt_text_from_utf8_literal("theme");
    if (theme_value != NULL) { zt_release(theme_value); theme_value = NULL; }
    theme_value = zt_text_from_utf8_literal("paper");
    if (dark_value != NULL) { zt_release(dark_value); dark_value = NULL; }
    dark_value = zt_text_from_utf8_literal("dark");
    if (cfg != NULL) { zt_release(cfg); cfg = NULL; }
    cfg = zt_map_text_text_from_arrays(((zt_text *[]){mode_key, theme_key}), ((zt_text *[]){mode_value, theme_value}), 2);
    cfg = zt_map_text_text_set_owned(cfg, theme_key, dark_value);
    if (zt_text_eq(zt_map_text_text_get(cfg, theme_key), dark_value)) goto zt_block_if_then_6;
    goto zt_block_if_else_7;

zt_block_if_then_6:
    score = zt_add_i64(score, 1);
    goto zt_block_if_join_8;

zt_block_if_else_7:
    goto zt_block_if_join_8;

zt_block_if_join_8:
    if (g2 != NULL) { zt_release(g2); g2 = NULL; }
    g2 = zt_app_main__collections_grid2d_int_new(2, 2);
    if (((zt_app_main__collections_grid2d_int_rows(g2) == 2) && (zt_app_main__collections_grid2d_int_cols(g2) == 2))) goto zt_block_if_then_9;
    goto zt_block_if_else_10;

zt_block_if_then_9:
    score = zt_add_i64(score, 1);
    goto zt_block_if_join_11;

zt_block_if_else_10:
    goto zt_block_if_join_11;

zt_block_if_join_11:
    if (pq != NULL) { zt_release(pq); pq = NULL; }
    pq = zt_app_main__collections_pqueue_int_new();
    if ((zt_app_main__collections_pqueue_int_is_empty(pq) && (zt_app_main__collections_pqueue_int_len(pq) == 0))) goto zt_block_if_then_12;
    goto zt_block_if_else_13;

zt_block_if_then_12:
    score = zt_add_i64(score, 1);
    goto zt_block_if_join_14;

zt_block_if_else_13:
    goto zt_block_if_join_14;

zt_block_if_join_14:
    if (cb != NULL) { zt_release(cb); cb = NULL; }
    cb = zt_app_main__collections_circbuf_int_new(4);
    if (((zt_app_main__collections_circbuf_int_len(cb) == 0) && (zt_app_main__collections_circbuf_int_capacity(cb) == 4))) goto zt_block_if_then_15;
    goto zt_block_if_else_16;

zt_block_if_then_15:
    score = zt_add_i64(score, 1);
    goto zt_block_if_join_17;

zt_block_if_else_16:
    goto zt_block_if_join_17;

zt_block_if_join_17:
    if (bm != NULL) { zt_release(bm); bm = NULL; }
    bm = zt_app_main__collections_btreemap_text_new();
    if ((zt_app_main__collections_btreemap_text_is_empty(bm) && (zt_app_main__collections_btreemap_text_len(bm) == 0))) goto zt_block_if_then_18;
    goto zt_block_if_else_19;

zt_block_if_then_18:
    score = zt_add_i64(score, 1);
    goto zt_block_if_join_20;

zt_block_if_else_19:
    goto zt_block_if_join_20;

zt_block_if_join_20:
    if (bs != NULL) { zt_release(bs); bs = NULL; }
    bs = zt_app_main__collections_btreeset_text_new();
    if ((zt_app_main__collections_btreeset_text_is_empty(bs) && (zt_app_main__collections_btreeset_text_len(bs) == 0))) goto zt_block_if_then_21;
    goto zt_block_if_else_22;

zt_block_if_then_21:
    score = zt_add_i64(score, 1);
    goto zt_block_if_join_23;

zt_block_if_else_22:
    goto zt_block_if_join_23;

zt_block_if_join_23:
    if (g3 != NULL) { zt_release(g3); g3 = NULL; }
    g3 = zt_app_main__collections_grid3d_int_new(2, 2, 2);
    if ((((zt_app_main__collections_grid3d_int_depth(g3) == 2) && (zt_app_main__collections_grid3d_int_rows(g3) == 2)) && (zt_app_main__collections_grid3d_int_cols(g3) == 2))) goto zt_block_if_then_24;
    goto zt_block_if_else_25;

zt_block_if_then_24:
    score = zt_add_i64(score, 1);
    goto zt_block_if_join_26;

zt_block_if_else_25:
    goto zt_block_if_join_26;

zt_block_if_join_26:
    if ((score < 9)) goto zt_block_if_then_27;
    goto zt_block_if_else_28;

zt_block_if_then_27:
    zt_return_value = zt_outcome_void_core_error_failure_message("m36 runtime score below expected");
    goto zt_cleanup;

zt_block_if_else_28:
    goto zt_block_if_join_29;

zt_block_if_join_29:
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    __zt_try_result_0 = zt_app_main__io_print_line(zt_text_from_utf8_literal("m36-runtime-core-ok"), false);
    if (zt_outcome_void_core_error_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_0);
    goto zt_cleanup;

zt_block_try_success_0:
    goto zt_block_try_after_2;

zt_block_try_after_2:
    zt_return_value = zt_outcome_void_core_error_success();
    goto zt_cleanup;

zt_cleanup:
    if (payload != NULL) { zt_release(payload); payload = NULL; }
    if (joined != NULL) { zt_release(joined); joined = NULL; }
    if (roundtrip != NULL) { zt_release(roundtrip); roundtrip = NULL; }
    if (xs != NULL) { zt_release(xs); xs = NULL; }
    if (mode_key != NULL) { zt_release(mode_key); mode_key = NULL; }
    if (mode_value != NULL) { zt_release(mode_value); mode_value = NULL; }
    if (theme_key != NULL) { zt_release(theme_key); theme_key = NULL; }
    if (theme_value != NULL) { zt_release(theme_value); theme_value = NULL; }
    if (dark_value != NULL) { zt_release(dark_value); dark_value = NULL; }
    if (cfg != NULL) { zt_release(cfg); cfg = NULL; }
    if (g2 != NULL) { zt_release(g2); g2 = NULL; }
    if (pq != NULL) { zt_release(pq); pq = NULL; }
    if (cb != NULL) { zt_release(cb); cb = NULL; }
    if (bm != NULL) { zt_release(bm); bm = NULL; }
    if (bs != NULL) { zt_release(bs); bs = NULL; }
    if (g3 != NULL) { zt_release(g3); g3 = NULL; }
    zt_outcome_void_core_error_dispose(&__zt_try_result_0);
    return zt_return_value;
}

static zt_bytes *zt_app_main__bytes_empty(void) {
    goto zt_block_entry;

zt_block_entry:
    return zt_bytes_empty();
}

static zt_bytes *zt_app_main__bytes_from_list(zt_list_i64 *values) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_bytes *zt_ffi_return_value = NULL;
    {
        zt_list_i64 *zt_ffi_arg0 = values;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_ffi_return_value = zt_bytes_from_list_i64(zt_ffi_arg0);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_list_i64 *zt_app_main__bytes_to_list(zt_bytes *value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_list_i64 *zt_ffi_return_value = NULL;
    {
        zt_bytes *zt_ffi_arg0 = value;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_ffi_return_value = zt_bytes_to_list_i64(zt_ffi_arg0);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_bytes *zt_app_main__bytes_join(zt_bytes *left, zt_bytes *right) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_bytes *zt_ffi_return_value = NULL;
    {
        zt_bytes *zt_ffi_arg0 = left;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_bytes *zt_ffi_arg1 = right;
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        zt_ffi_return_value = zt_bytes_join(zt_ffi_arg0, zt_ffi_arg1);
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_bool zt_app_main__bytes_starts_with(zt_bytes *value, zt_bytes *prefix) {
    goto zt_block_entry;

zt_block_entry:
    return zt_bytes_starts_with(value, prefix);
}

static zt_bool zt_app_main__bytes_ends_with(zt_bytes *value, zt_bytes *suffix) {
    goto zt_block_entry;

zt_block_entry:
    return zt_bytes_ends_with(value, suffix);
}

static zt_bool zt_app_main__bytes_contains(zt_bytes *value, zt_bytes *part) {
    goto zt_block_entry;

zt_block_entry:
    return zt_bytes_contains(value, part);
}

static zt_list_i64 *zt_app_main__collections_queue_int_new(void) {
    goto zt_block_entry;

zt_block_entry:
    return zt_queue_i64_new();
}

static zt_list_i64 *zt_app_main__collections_queue_int_enqueue(zt_list_i64 *queue, zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_list_i64 *zt_ffi_return_value = NULL;
    {
        zt_list_i64 *zt_ffi_arg0 = queue;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_ffi_return_value = zt_queue_i64_enqueue(zt_ffi_arg0, value);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_optional_i64 zt_app_main__collections_queue_int_dequeue(zt_list_i64 *queue) {
    goto zt_block_entry;

zt_block_entry:
    return zt_queue_i64_dequeue(queue);
}

static zt_optional_i64 zt_app_main__collections_queue_int_peek(zt_list_i64 *queue) {
    goto zt_block_entry;

zt_block_entry:
    return zt_queue_i64_peek(queue);
}

static zt_list_text *zt_app_main__collections_queue_text_new(void) {
    goto zt_block_entry;

zt_block_entry:
    return zt_queue_text_new();
}

static zt_list_text *zt_app_main__collections_queue_text_enqueue(zt_list_text *queue, zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_list_text *zt_ffi_return_value = NULL;
    {
        zt_list_text *zt_ffi_arg0 = queue;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg1 = value;
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        zt_ffi_return_value = zt_queue_text_enqueue(zt_ffi_arg0, zt_ffi_arg1);
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_optional_text zt_app_main__collections_queue_text_dequeue(zt_list_text *queue) {
    goto zt_block_entry;

zt_block_entry:
    return zt_queue_text_dequeue(queue);
}

static zt_optional_text zt_app_main__collections_queue_text_peek(zt_list_text *queue) {
    goto zt_block_entry;

zt_block_entry:
    return zt_queue_text_peek(queue);
}

static zt_list_i64 *zt_app_main__collections_stack_int_new(void) {
    goto zt_block_entry;

zt_block_entry:
    return zt_stack_i64_new();
}

static zt_list_i64 *zt_app_main__collections_stack_int_push(zt_list_i64 *stack, zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_list_i64 *zt_ffi_return_value = NULL;
    {
        zt_list_i64 *zt_ffi_arg0 = stack;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_ffi_return_value = zt_stack_i64_push(zt_ffi_arg0, value);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_optional_i64 zt_app_main__collections_stack_int_pop(zt_list_i64 *stack) {
    goto zt_block_entry;

zt_block_entry:
    return zt_stack_i64_pop(stack);
}

static zt_optional_i64 zt_app_main__collections_stack_int_peek(zt_list_i64 *stack) {
    goto zt_block_entry;

zt_block_entry:
    return zt_stack_i64_peek(stack);
}

static zt_list_text *zt_app_main__collections_stack_text_new(void) {
    goto zt_block_entry;

zt_block_entry:
    return zt_stack_text_new();
}

static zt_list_text *zt_app_main__collections_stack_text_push(zt_list_text *stack, zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_list_text *zt_ffi_return_value = NULL;
    {
        zt_list_text *zt_ffi_arg0 = stack;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg1 = value;
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        zt_ffi_return_value = zt_stack_text_push(zt_ffi_arg0, zt_ffi_arg1);
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_optional_text zt_app_main__collections_stack_text_pop(zt_list_text *stack) {
    goto zt_block_entry;

zt_block_entry:
    return zt_stack_text_pop(stack);
}

static zt_optional_text zt_app_main__collections_stack_text_peek(zt_list_text *stack) {
    goto zt_block_entry;

zt_block_entry:
    return zt_stack_text_peek(stack);
}

static zt_grid2d_i64 *zt_app_main__collections_grid2d_int_new(zt_int rows, zt_int cols) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid2d_i64_new(rows, cols);
}

static zt_int zt_app_main__collections_grid2d_int_get(zt_grid2d_i64 *grid, zt_int row, zt_int col) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid2d_i64_get(grid, row, col);
}

static zt_grid2d_i64 *zt_app_main__collections_grid2d_int_set(zt_grid2d_i64 *grid, zt_int row, zt_int col, zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_grid2d_i64 *zt_ffi_return_value = NULL;
    {
        zt_grid2d_i64 *zt_ffi_arg0 = grid;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_ffi_return_value = zt_grid2d_i64_set_owned(zt_ffi_arg0, row, col, value);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_grid2d_i64 *zt_app_main__collections_grid2d_int_fill(zt_grid2d_i64 *grid, zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_grid2d_i64 *zt_ffi_return_value = NULL;
    {
        zt_grid2d_i64 *zt_ffi_arg0 = grid;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_ffi_return_value = zt_grid2d_i64_fill_owned(zt_ffi_arg0, value);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_int zt_app_main__collections_grid2d_int_rows(zt_grid2d_i64 *grid) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid2d_i64_rows(grid);
}

static zt_int zt_app_main__collections_grid2d_int_cols(zt_grid2d_i64 *grid) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid2d_i64_cols(grid);
}

static zt_grid2d_text *zt_app_main__collections_grid2d_text_new(zt_int rows, zt_int cols) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid2d_text_new(rows, cols);
}

static zt_text *zt_app_main__collections_grid2d_text_get(zt_grid2d_text *grid, zt_int row, zt_int col) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_text *zt_ffi_return_value = NULL;
    {
        zt_grid2d_text *zt_ffi_arg0 = grid;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_ffi_return_value = zt_grid2d_text_get(zt_ffi_arg0, row, col);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_grid2d_text *zt_app_main__collections_grid2d_text_set(zt_grid2d_text *grid, zt_int row, zt_int col, zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_grid2d_text *zt_ffi_return_value = NULL;
    {
        zt_grid2d_text *zt_ffi_arg0 = grid;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg3 = value;
        if (zt_ffi_arg3 != NULL) { zt_retain(zt_ffi_arg3); }
        zt_ffi_return_value = zt_grid2d_text_set_owned(zt_ffi_arg0, row, col, zt_ffi_arg3);
        if (zt_ffi_arg3 != NULL) { zt_release(zt_ffi_arg3); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_grid2d_text *zt_app_main__collections_grid2d_text_fill(zt_grid2d_text *grid, zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_grid2d_text *zt_ffi_return_value = NULL;
    {
        zt_grid2d_text *zt_ffi_arg0 = grid;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg1 = value;
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        zt_ffi_return_value = zt_grid2d_text_fill_owned(zt_ffi_arg0, zt_ffi_arg1);
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_int zt_app_main__collections_grid2d_text_rows(zt_grid2d_text *grid) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid2d_text_rows(grid);
}

static zt_int zt_app_main__collections_grid2d_text_cols(zt_grid2d_text *grid) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid2d_text_cols(grid);
}

static zt_pqueue_i64 *zt_app_main__collections_pqueue_int_new(void) {
    goto zt_block_entry;

zt_block_entry:
    return zt_pqueue_i64_new();
}

static zt_pqueue_i64 *zt_app_main__collections_pqueue_int_push(zt_pqueue_i64 *heap, zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_pqueue_i64 *zt_ffi_return_value = NULL;
    {
        zt_pqueue_i64 *zt_ffi_arg0 = heap;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_ffi_return_value = zt_pqueue_i64_push_owned(zt_ffi_arg0, value);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_optional_i64 zt_app_main__collections_pqueue_int_pop(zt_pqueue_i64 *heap) {
    goto zt_block_entry;

zt_block_entry:
    return zt_pqueue_i64_pop(heap);
}

static zt_optional_i64 zt_app_main__collections_pqueue_int_peek(zt_pqueue_i64 *heap) {
    goto zt_block_entry;

zt_block_entry:
    return zt_pqueue_i64_peek(heap);
}

static zt_int zt_app_main__collections_pqueue_int_len(zt_pqueue_i64 *heap) {
    goto zt_block_entry;

zt_block_entry:
    return zt_pqueue_i64_len(heap);
}

static zt_pqueue_text *zt_app_main__collections_pqueue_text_new(void) {
    goto zt_block_entry;

zt_block_entry:
    return zt_pqueue_text_new();
}

static zt_pqueue_text *zt_app_main__collections_pqueue_text_push(zt_pqueue_text *heap, zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_pqueue_text *zt_ffi_return_value = NULL;
    {
        zt_pqueue_text *zt_ffi_arg0 = heap;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg1 = value;
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        zt_ffi_return_value = zt_pqueue_text_push_owned(zt_ffi_arg0, zt_ffi_arg1);
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_optional_text zt_app_main__collections_pqueue_text_pop(zt_pqueue_text *heap) {
    goto zt_block_entry;

zt_block_entry:
    return zt_pqueue_text_pop(heap);
}

static zt_optional_text zt_app_main__collections_pqueue_text_peek(zt_pqueue_text *heap) {
    goto zt_block_entry;

zt_block_entry:
    return zt_pqueue_text_peek(heap);
}

static zt_int zt_app_main__collections_pqueue_text_len(zt_pqueue_text *heap) {
    goto zt_block_entry;

zt_block_entry:
    return zt_pqueue_text_len(heap);
}

static zt_circbuf_i64 *zt_app_main__collections_circbuf_int_new(zt_int capacity) {
    goto zt_block_entry;

zt_block_entry:
    return zt_circbuf_i64_new(capacity);
}

static zt_circbuf_i64 *zt_app_main__collections_circbuf_int_push(zt_circbuf_i64 *buf, zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_circbuf_i64 *zt_ffi_return_value = NULL;
    {
        zt_circbuf_i64 *zt_ffi_arg0 = buf;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_ffi_return_value = zt_circbuf_i64_push_owned(zt_ffi_arg0, value);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_optional_i64 zt_app_main__collections_circbuf_int_pop(zt_circbuf_i64 *buf) {
    goto zt_block_entry;

zt_block_entry:
    return zt_circbuf_i64_pop(buf);
}

static zt_optional_i64 zt_app_main__collections_circbuf_int_peek(zt_circbuf_i64 *buf) {
    goto zt_block_entry;

zt_block_entry:
    return zt_circbuf_i64_peek(buf);
}

static zt_int zt_app_main__collections_circbuf_int_len(zt_circbuf_i64 *buf) {
    goto zt_block_entry;

zt_block_entry:
    return zt_circbuf_i64_len(buf);
}

static zt_int zt_app_main__collections_circbuf_int_capacity(zt_circbuf_i64 *buf) {
    goto zt_block_entry;

zt_block_entry:
    return zt_circbuf_i64_capacity(buf);
}

static zt_bool zt_app_main__collections_circbuf_int_is_full(zt_circbuf_i64 *buf) {
    goto zt_block_entry;

zt_block_entry:
    return zt_circbuf_i64_is_full(buf);
}

static zt_circbuf_text *zt_app_main__collections_circbuf_text_new(zt_int capacity) {
    goto zt_block_entry;

zt_block_entry:
    return zt_circbuf_text_new(capacity);
}

static zt_circbuf_text *zt_app_main__collections_circbuf_text_push(zt_circbuf_text *buf, zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_circbuf_text *zt_ffi_return_value = NULL;
    {
        zt_circbuf_text *zt_ffi_arg0 = buf;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg1 = value;
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        zt_ffi_return_value = zt_circbuf_text_push_owned(zt_ffi_arg0, zt_ffi_arg1);
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_optional_text zt_app_main__collections_circbuf_text_pop(zt_circbuf_text *buf) {
    goto zt_block_entry;

zt_block_entry:
    return zt_circbuf_text_pop(buf);
}

static zt_optional_text zt_app_main__collections_circbuf_text_peek(zt_circbuf_text *buf) {
    goto zt_block_entry;

zt_block_entry:
    return zt_circbuf_text_peek(buf);
}

static zt_int zt_app_main__collections_circbuf_text_len(zt_circbuf_text *buf) {
    goto zt_block_entry;

zt_block_entry:
    return zt_circbuf_text_len(buf);
}

static zt_int zt_app_main__collections_circbuf_text_capacity(zt_circbuf_text *buf) {
    goto zt_block_entry;

zt_block_entry:
    return zt_circbuf_text_capacity(buf);
}

static zt_bool zt_app_main__collections_circbuf_text_is_full(zt_circbuf_text *buf) {
    goto zt_block_entry;

zt_block_entry:
    return zt_circbuf_text_is_full(buf);
}

static zt_btreemap_text_text *zt_app_main__collections_btreemap_text_new(void) {
    goto zt_block_entry;

zt_block_entry:
    return zt_btreemap_text_text_new();
}

static zt_btreemap_text_text *zt_app_main__collections_btreemap_text_set(zt_btreemap_text_text *self_map, zt_text *key, zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_btreemap_text_text *zt_ffi_return_value = NULL;
    {
        zt_btreemap_text_text *zt_ffi_arg0 = self_map;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg1 = key;
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        zt_text *zt_ffi_arg2 = value;
        if (zt_ffi_arg2 != NULL) { zt_retain(zt_ffi_arg2); }
        zt_ffi_return_value = zt_btreemap_text_text_set_owned(zt_ffi_arg0, zt_ffi_arg1, zt_ffi_arg2);
        if (zt_ffi_arg2 != NULL) { zt_release(zt_ffi_arg2); }
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_text *zt_app_main__collections_btreemap_text_get(zt_btreemap_text_text *self_map, zt_text *key) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_text *zt_ffi_return_value = NULL;
    {
        zt_btreemap_text_text *zt_ffi_arg0 = self_map;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg1 = key;
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        zt_ffi_return_value = zt_btreemap_text_text_get(zt_ffi_arg0, zt_ffi_arg1);
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_optional_text zt_app_main__collections_btreemap_text_get_optional(zt_btreemap_text_text *self_map, zt_text *key) {
    goto zt_block_entry;

zt_block_entry:
    return zt_btreemap_text_text_get_optional(self_map, key);
}

static zt_bool zt_app_main__collections_btreemap_text_contains(zt_btreemap_text_text *self_map, zt_text *key) {
    goto zt_block_entry;

zt_block_entry:
    return zt_btreemap_text_text_contains(self_map, key);
}

static zt_btreemap_text_text *zt_app_main__collections_btreemap_text_remove(zt_btreemap_text_text *self_map, zt_text *key) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_btreemap_text_text *zt_ffi_return_value = NULL;
    {
        zt_btreemap_text_text *zt_ffi_arg0 = self_map;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg1 = key;
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        zt_ffi_return_value = zt_btreemap_text_text_remove_owned(zt_ffi_arg0, zt_ffi_arg1);
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_int zt_app_main__collections_btreemap_text_len(zt_btreemap_text_text *self_map) {
    goto zt_block_entry;

zt_block_entry:
    return zt_btreemap_text_text_len(self_map);
}

static zt_btreeset_text *zt_app_main__collections_btreeset_text_new(void) {
    goto zt_block_entry;

zt_block_entry:
    return zt_btreeset_text_new();
}

static zt_btreeset_text *zt_app_main__collections_btreeset_text_insert(zt_btreeset_text *set, zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_btreeset_text *zt_ffi_return_value = NULL;
    {
        zt_btreeset_text *zt_ffi_arg0 = set;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg1 = value;
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        zt_ffi_return_value = zt_btreeset_text_insert_owned(zt_ffi_arg0, zt_ffi_arg1);
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_bool zt_app_main__collections_btreeset_text_contains(zt_btreeset_text *set, zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    return zt_btreeset_text_contains(set, value);
}

static zt_btreeset_text *zt_app_main__collections_btreeset_text_remove(zt_btreeset_text *set, zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_btreeset_text *zt_ffi_return_value = NULL;
    {
        zt_btreeset_text *zt_ffi_arg0 = set;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg1 = value;
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        zt_ffi_return_value = zt_btreeset_text_remove_owned(zt_ffi_arg0, zt_ffi_arg1);
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_int zt_app_main__collections_btreeset_text_len(zt_btreeset_text *set) {
    goto zt_block_entry;

zt_block_entry:
    return zt_btreeset_text_len(set);
}

static zt_grid3d_i64 *zt_app_main__collections_grid3d_int_new(zt_int depth, zt_int rows, zt_int cols) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid3d_i64_new(depth, rows, cols);
}

static zt_int zt_app_main__collections_grid3d_int_get(zt_grid3d_i64 *grid, zt_int layer, zt_int row, zt_int col) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid3d_i64_get(grid, layer, row, col);
}

static zt_grid3d_i64 *zt_app_main__collections_grid3d_int_set(zt_grid3d_i64 *grid, zt_int layer, zt_int row, zt_int col, zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_grid3d_i64 *zt_ffi_return_value = NULL;
    {
        zt_grid3d_i64 *zt_ffi_arg0 = grid;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_ffi_return_value = zt_grid3d_i64_set_owned(zt_ffi_arg0, layer, row, col, value);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_grid3d_i64 *zt_app_main__collections_grid3d_int_fill(zt_grid3d_i64 *grid, zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_grid3d_i64 *zt_ffi_return_value = NULL;
    {
        zt_grid3d_i64 *zt_ffi_arg0 = grid;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_ffi_return_value = zt_grid3d_i64_fill_owned(zt_ffi_arg0, value);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_int zt_app_main__collections_grid3d_int_depth(zt_grid3d_i64 *grid) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid3d_i64_depth(grid);
}

static zt_int zt_app_main__collections_grid3d_int_rows(zt_grid3d_i64 *grid) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid3d_i64_rows(grid);
}

static zt_int zt_app_main__collections_grid3d_int_cols(zt_grid3d_i64 *grid) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid3d_i64_cols(grid);
}

static zt_grid3d_text *zt_app_main__collections_grid3d_text_new(zt_int depth, zt_int rows, zt_int cols) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid3d_text_new(depth, rows, cols);
}

static zt_text *zt_app_main__collections_grid3d_text_get(zt_grid3d_text *grid, zt_int layer, zt_int row, zt_int col) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_text *zt_ffi_return_value = NULL;
    {
        zt_grid3d_text *zt_ffi_arg0 = grid;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_ffi_return_value = zt_grid3d_text_get(zt_ffi_arg0, layer, row, col);
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_grid3d_text *zt_app_main__collections_grid3d_text_set(zt_grid3d_text *grid, zt_int layer, zt_int row, zt_int col, zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_grid3d_text *zt_ffi_return_value = NULL;
    {
        zt_grid3d_text *zt_ffi_arg0 = grid;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg4 = value;
        if (zt_ffi_arg4 != NULL) { zt_retain(zt_ffi_arg4); }
        zt_ffi_return_value = zt_grid3d_text_set_owned(zt_ffi_arg0, layer, row, col, zt_ffi_arg4);
        if (zt_ffi_arg4 != NULL) { zt_release(zt_ffi_arg4); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_grid3d_text *zt_app_main__collections_grid3d_text_fill(zt_grid3d_text *grid, zt_text *value) {
    goto zt_block_entry;

zt_block_entry:
    {
        zt_grid3d_text *zt_ffi_return_value = NULL;
    {
        zt_grid3d_text *zt_ffi_arg0 = grid;
        if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
        zt_text *zt_ffi_arg1 = value;
        if (zt_ffi_arg1 != NULL) { zt_retain(zt_ffi_arg1); }
        zt_ffi_return_value = zt_grid3d_text_fill_owned(zt_ffi_arg0, zt_ffi_arg1);
        if (zt_ffi_arg1 != NULL) { zt_release(zt_ffi_arg1); }
        if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
    }
        return zt_ffi_return_value;
    }
}

static zt_int zt_app_main__collections_grid3d_text_depth(zt_grid3d_text *grid) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid3d_text_depth(grid);
}

static zt_int zt_app_main__collections_grid3d_text_rows(zt_grid3d_text *grid) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid3d_text_rows(grid);
}

static zt_int zt_app_main__collections_grid3d_text_cols(zt_grid3d_text *grid) {
    goto zt_block_entry;

zt_block_entry:
    return zt_grid3d_text_cols(grid);
}

static zt_bool zt_app_main__collections_pqueue_int_is_empty(zt_pqueue_i64 *heap) {
    goto zt_block_entry;

zt_block_entry:
    return (zt_pqueue_i64_len(heap) == 0);
}

static zt_bool zt_app_main__collections_pqueue_text_is_empty(zt_pqueue_text *heap) {
    goto zt_block_entry;

zt_block_entry:
    return (zt_pqueue_text_len(heap) == 0);
}

static zt_bool zt_app_main__collections_circbuf_int_is_empty(zt_circbuf_i64 *buf) {
    goto zt_block_entry;

zt_block_entry:
    return (zt_circbuf_i64_len(buf) == 0);
}

static zt_bool zt_app_main__collections_circbuf_text_is_empty(zt_circbuf_text *buf) {
    goto zt_block_entry;

zt_block_entry:
    return (zt_circbuf_text_len(buf) == 0);
}

static zt_bool zt_app_main__collections_btreemap_text_is_empty(zt_btreemap_text_text *self_map) {
    goto zt_block_entry;

zt_block_entry:
    return (zt_btreemap_text_text_len(self_map) == 0);
}

static zt_bool zt_app_main__collections_btreeset_text_is_empty(zt_btreeset_text *set) {
    goto zt_block_entry;

zt_block_entry:
    return (zt_btreeset_text_len(set) == 0);
}

static zt_int zt_app_main__collections_grid2d_int_size(zt_grid2d_i64 *grid) {
    goto zt_block_entry;

zt_block_entry:
    return zt_mul_i64(zt_grid2d_i64_rows(grid), zt_grid2d_i64_cols(grid));
}

static zt_int zt_app_main__collections_grid2d_text_size(zt_grid2d_text *grid) {
    goto zt_block_entry;

zt_block_entry:
    return zt_mul_i64(zt_grid2d_text_rows(grid), zt_grid2d_text_cols(grid));
}

static zt_int zt_app_main__collections_grid3d_int_size(zt_grid3d_i64 *grid) {
    goto zt_block_entry;

zt_block_entry:
    return zt_mul_i64(zt_mul_i64(zt_grid3d_i64_depth(grid), zt_grid3d_i64_rows(grid)), zt_grid3d_i64_cols(grid));
}

static zt_int zt_app_main__collections_grid3d_text_size(zt_grid3d_text *grid) {
    goto zt_block_entry;

zt_block_entry:
    return zt_mul_i64(zt_mul_i64(zt_grid3d_text_depth(grid), zt_grid3d_text_rows(grid)), zt_grid3d_text_cols(grid));
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