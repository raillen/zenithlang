#ifndef ZENITH_NEXT_RUNTIME_C_ZENITH_RT_H
#define ZENITH_NEXT_RUNTIME_C_ZENITH_RT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZT_RUNTIME_ABI_VERSION_MAJOR 0
#define ZT_RUNTIME_ABI_VERSION_MINOR 4
#define ZT_EXIT_CODE_RUNTIME_ERROR 1
#define ZT_EXIT_CODE_TEST_FAILED 120
#define ZT_EXIT_CODE_TEST_SKIPPED 121

typedef int64_t zt_int;
typedef double zt_float;
typedef bool zt_bool;

typedef enum zt_heap_kind {
    ZT_HEAP_UNKNOWN = 0,
    ZT_HEAP_TEXT = 1,
    ZT_HEAP_LIST_I64 = 2,
    ZT_HEAP_LIST_TEXT = 3,
    ZT_HEAP_OPTIONAL_TEXT = 4,
    ZT_HEAP_OUTCOME_I64_TEXT = 5,
    ZT_HEAP_OUTCOME_VOID_TEXT = 6,
    ZT_HEAP_OPTIONAL_LIST_I64 = 7,
    ZT_HEAP_OUTCOME_TEXT_TEXT = 8,
    ZT_HEAP_MAP_TEXT_TEXT = 9,
    ZT_HEAP_OPTIONAL_LIST_TEXT = 10,
    ZT_HEAP_OPTIONAL_MAP_TEXT_TEXT = 11,
    ZT_HEAP_OUTCOME_LIST_I64_TEXT = 12,
    ZT_HEAP_OUTCOME_LIST_TEXT_TEXT = 13,
    ZT_HEAP_OUTCOME_MAP_TEXT_TEXT = 14,
    ZT_HEAP_BYTES = 15,
    ZT_HEAP_GRID2D_I64 = 16,
    ZT_HEAP_GRID2D_TEXT = 17,
    ZT_HEAP_PQUEUE_I64 = 18,
    ZT_HEAP_PQUEUE_TEXT = 19,
    ZT_HEAP_CIRCBUF_I64 = 20,
    ZT_HEAP_CIRCBUF_TEXT = 21,
    ZT_HEAP_BTREEMAP_TEXT_TEXT = 22,
    ZT_HEAP_BTREESET_TEXT = 23,
    ZT_HEAP_GRID3D_I64 = 24,
    ZT_HEAP_GRID3D_TEXT = 25,
    ZT_HEAP_NET_CONNECTION = 26,
    ZT_HEAP_DYN_TEXT_REPR = 27,
    ZT_HEAP_LIST_DYN_TEXT_REPR = 28,
    ZT_HEAP_LIST_F64 = 29,
    ZT_HEAP_IMMORTAL_OUTCOME_VOID_TEXT = 255
} zt_heap_kind;

typedef struct zt_header {
    uint32_t rc;
    uint32_t kind;
} zt_header;

typedef void (*zt_heap_free_fn)(void *ref);
typedef void *(*zt_heap_clone_fn)(void *ref);

static inline zt_bool zt_i64_eq(zt_int left, zt_int right) {
    return left == right;
}

typedef struct zt_text {
    zt_header header;
    size_t len;
    char *data;
} zt_text;

typedef struct zt_bytes {
    zt_header header;
    size_t len;
    uint8_t *data;
} zt_bytes;

typedef enum zt_dyn_text_repr_tag {
    ZT_DYN_TEXT_REPR_INT = 1,
    ZT_DYN_TEXT_REPR_FLOAT = 2,
    ZT_DYN_TEXT_REPR_BOOL = 3,
    ZT_DYN_TEXT_REPR_TEXT = 4
} zt_dyn_text_repr_tag;

typedef struct zt_dyn_text_repr {
    zt_header header;
    uint32_t tag;
    zt_int int_value;
    zt_float float_value;
    zt_bool bool_value;
    zt_text *text_value;
} zt_dyn_text_repr;

typedef struct zt_list_dyn_text_repr {
    zt_header header;
    size_t len;
    size_t capacity;
    zt_dyn_text_repr **data;
} zt_list_dyn_text_repr;

typedef struct zt_net_connection zt_net_connection;
typedef struct zt_shared_text zt_shared_text;
typedef struct zt_shared_bytes zt_shared_bytes;

typedef struct zt_list_i64 {
    zt_header header;
    size_t len;
    size_t capacity;
    zt_int *data;
} zt_list_i64;

typedef struct zt_list_text {
    zt_header header;
    size_t len;
    size_t capacity;
    zt_text **data;
} zt_list_text;

typedef struct zt_list_f64 {
    zt_header header;
    size_t len;
    size_t capacity;
    zt_float *data;
} zt_list_f64;

typedef struct zt_map_text_text {
    zt_header header;
    size_t len;
    size_t capacity;
    zt_text **keys;
    zt_text **values;
    size_t hash_capacity;
    size_t *hash_indices;
} zt_map_text_text;

typedef struct zt_optional_i64 {
    zt_bool is_present;
    zt_int value;
} zt_optional_i64;

typedef struct zt_optional_text {
    zt_bool is_present;
    zt_text *value;
} zt_optional_text;

typedef struct zt_optional_bytes {
    zt_bool is_present;
    zt_bytes *value;
} zt_optional_bytes;

typedef struct zt_optional_list_i64 {
    zt_bool is_present;
    zt_list_i64 *value;
} zt_optional_list_i64;

typedef struct zt_optional_list_text {
    zt_bool is_present;
    zt_list_text *value;
} zt_optional_list_text;

typedef struct zt_optional_map_text_text {
    zt_bool is_present;
    zt_map_text_text *value;
} zt_optional_map_text_text;

typedef struct zt_core_error {
    zt_text *code;
    zt_text *message;
    zt_optional_text context;
} zt_core_error;

zt_core_error zt_core_error_make(zt_text *code, zt_text *message, zt_optional_text context);
zt_core_error zt_core_error_from_message(const char *code, const char *message);
zt_core_error zt_core_error_from_text(const char *code, zt_text *message);
zt_core_error zt_core_error_clone(zt_core_error error);
void zt_core_error_dispose(zt_core_error *error);
zt_text *zt_core_error_message_or_default(zt_core_error error);

typedef struct zt_outcome_i64_text {
    zt_bool is_success;
    zt_int value;
    zt_text *error;
} zt_outcome_i64_text;

typedef struct zt_outcome_void_text {
    zt_bool is_success;
    zt_text *error;
} zt_outcome_void_text;

typedef struct zt_outcome_text_text {
    zt_bool is_success;
    zt_text *value;
    zt_text *error;
} zt_outcome_text_text;

typedef struct zt_outcome_bytes_text {
    zt_bool is_success;
    zt_bytes *value;
    zt_text *error;
} zt_outcome_bytes_text;

typedef struct zt_outcome_optional_text_text {
    zt_bool is_success;
    zt_optional_text value;
    zt_text *error;
} zt_outcome_optional_text_text;
typedef struct zt_outcome_optional_bytes_text {
    zt_bool is_success;
    zt_optional_bytes value;
    zt_text *error;
} zt_outcome_optional_bytes_text;

typedef struct zt_outcome_net_connection_text {
    zt_bool is_success;
    zt_net_connection *value;
    zt_text *error;
} zt_outcome_net_connection_text;

typedef struct zt_outcome_list_i64_text {
    zt_bool is_success;
    zt_list_i64 *value;
    zt_text *error;
} zt_outcome_list_i64_text;

typedef struct zt_outcome_list_text_text {
    zt_bool is_success;
    zt_list_text *value;
    zt_text *error;
} zt_outcome_list_text_text;

typedef struct zt_outcome_map_text_text {
    zt_bool is_success;
    zt_map_text_text *value;
    zt_text *error;
} zt_outcome_map_text_text;

typedef struct zt_outcome_i64_core_error {
    zt_bool is_success;
    zt_int value;
    zt_core_error error;
} zt_outcome_i64_core_error;

typedef struct zt_outcome_void_core_error {
    zt_bool is_success;
    zt_core_error error;
} zt_outcome_void_core_error;

typedef struct zt_outcome_text_core_error {
    zt_bool is_success;
    zt_text *value;
    zt_core_error error;
} zt_outcome_text_core_error;

typedef struct zt_outcome_optional_text_core_error {
    zt_bool is_success;
    zt_optional_text value;
    zt_core_error error;
} zt_outcome_optional_text_core_error;

typedef struct zt_outcome_optional_bytes_core_error {
    zt_bool is_success;
    zt_optional_bytes value;
    zt_core_error error;
} zt_outcome_optional_bytes_core_error;

typedef struct zt_outcome_net_connection_core_error {
    zt_bool is_success;
    zt_net_connection *value;
    zt_core_error error;
} zt_outcome_net_connection_core_error;

typedef struct zt_outcome_list_i64_core_error {
    zt_bool is_success;
    zt_list_i64 *value;
    zt_core_error error;
} zt_outcome_list_i64_core_error;

typedef struct zt_outcome_map_text_text_core_error {
    zt_bool is_success;
    zt_map_text_text *value;
    zt_core_error error;
} zt_outcome_map_text_text_core_error;

typedef struct zt_grid2d_i64 {
    zt_header header;
    size_t rows;
    size_t cols;
    size_t len;
    size_t capacity;
    zt_int *data;
} zt_grid2d_i64;

typedef struct zt_grid2d_text {
    zt_header header;
    size_t rows;
    size_t cols;
    size_t len;
    size_t capacity;
    zt_text **data;
} zt_grid2d_text;

typedef struct zt_pqueue_i64 {
    zt_header header;
    size_t len;
    size_t capacity;
    zt_int *data;
} zt_pqueue_i64;

typedef struct zt_pqueue_text {
    zt_header header;
    size_t len;
    size_t capacity;
    zt_text **data;
} zt_pqueue_text;

typedef struct zt_circbuf_i64 {
    zt_header header;
    size_t len;
    size_t capacity;
    size_t head;
    zt_int *data;
} zt_circbuf_i64;

typedef struct zt_circbuf_text {
    zt_header header;
    size_t len;
    size_t capacity;
    size_t head;
    zt_text **data;
} zt_circbuf_text;

typedef struct zt_btreemap_text_text {
    zt_header header;
    size_t len;
    size_t capacity;
    zt_text **keys;
    zt_text **values;
} zt_btreemap_text_text;

typedef struct zt_btreeset_text {
    zt_header header;
    size_t len;
    size_t capacity;
    zt_text **data;
} zt_btreeset_text;

typedef struct zt_grid3d_i64 {
    zt_header header;
    size_t depth;
    size_t rows;
    size_t cols;
    size_t len;
    size_t capacity;
    zt_int *data;
} zt_grid3d_i64;

typedef struct zt_grid3d_text {
    zt_header header;
    size_t depth;
    size_t rows;
    size_t cols;
    size_t len;
    size_t capacity;
    zt_text **data;
} zt_grid3d_text;

struct zt_net_connection {
    zt_header header;
    intptr_t socket_handle;
    zt_int default_timeout_ms;
    zt_bool closed;
};

typedef enum zt_error_kind {
    ZT_ERR_ASSERT,
    ZT_ERR_CHECK,
    ZT_ERR_INDEX,
    ZT_ERR_UNWRAP,
    ZT_ERR_PANIC,
    ZT_ERR_IO,
    ZT_ERR_MATH,
    ZT_ERR_PLATFORM,
    ZT_ERR_CONTRACT,
    ZT_ERR_TEST_FAILED,
    ZT_ERR_TEST_SKIPPED
} zt_error_kind;

typedef struct zt_runtime_span {
    const char *source_name;
    zt_int line;
    zt_int column;
} zt_runtime_span;

typedef struct zt_runtime_error_info {
    zt_bool has_error;
    zt_error_kind kind;
    const char *message;
    const char *code;
    zt_runtime_span span;
} zt_runtime_error_info;

const char *zt_error_kind_name(zt_error_kind kind);
zt_runtime_span zt_runtime_span_unknown(void);
zt_runtime_span zt_runtime_make_span(const char *source_name, zt_int line, zt_int column);
zt_bool zt_runtime_span_is_known(zt_runtime_span span);
const zt_runtime_error_info *zt_runtime_last_error(void);
void zt_runtime_clear_error(void);
void zt_runtime_report_error(zt_error_kind kind, const char *message, const char *code, zt_runtime_span span);

void zt_retain(void *ref);
void zt_release(void *ref);
void *zt_deep_copy(void *ref);
uint32_t zt_register_dynamic_heap_kind(zt_heap_free_fn free_fn, zt_heap_clone_fn clone_fn);

zt_shared_text *zt_shared_text_new(zt_text *value);
zt_shared_text *zt_shared_text_retain(zt_shared_text *shared);
void zt_shared_text_release(zt_shared_text *shared);
const zt_text *zt_shared_text_borrow(const zt_shared_text *shared);
zt_text *zt_shared_text_snapshot(const zt_shared_text *shared);
uint32_t zt_shared_text_ref_count(const zt_shared_text *shared);

zt_shared_bytes *zt_shared_bytes_new(zt_bytes *value);
zt_shared_bytes *zt_shared_bytes_retain(zt_shared_bytes *shared);
void zt_shared_bytes_release(zt_shared_bytes *shared);
const zt_bytes *zt_shared_bytes_borrow(const zt_shared_bytes *shared);
zt_bytes *zt_shared_bytes_snapshot(const zt_shared_bytes *shared);
uint32_t zt_shared_bytes_ref_count(const zt_shared_bytes *shared);

void zt_runtime_error(zt_error_kind kind, const char *message);
void zt_runtime_error_ex(zt_error_kind kind, const char *message, const char *code, zt_runtime_span span);
void zt_runtime_error_with_span(zt_error_kind kind, const char *message, zt_runtime_span span);
void zt_assert(zt_bool condition, const char *message);
void zt_check(zt_bool condition, const char *message);
void zt_panic(const char *message);
void zt_test_fail(zt_text *message);
void zt_test_skip(zt_text *reason);
void zt_contract_failed(const char *message, zt_runtime_span span);
void zt_contract_failed_i64(const char *message, zt_int value, zt_runtime_span span);
void zt_contract_failed_float(const char *message, zt_float value, zt_runtime_span span);
void zt_contract_failed_bool(const char *message, zt_bool value, zt_runtime_span span);

zt_text *zt_text_from_utf8(const char *data, size_t len);
zt_text *zt_text_from_utf8_literal(const char *data);
zt_text *zt_text_concat(const zt_text *a, const zt_text *b);
zt_text *zt_text_index(const zt_text *value, zt_int index_0);
zt_text *zt_text_slice(const zt_text *value, zt_int start_0, zt_int end_0);
zt_bool zt_text_eq(const zt_text *a, const zt_text *b);
zt_int zt_text_len(const zt_text *value);
const char *zt_text_data(const zt_text *value);

zt_bytes *zt_bytes_empty(void);
zt_bytes *zt_bytes_from_array(const uint8_t *data, size_t len);
zt_bytes *zt_bytes_from_list_i64(const zt_list_i64 *values);
zt_list_i64 *zt_bytes_to_list_i64(const zt_bytes *value);
zt_bytes *zt_bytes_join(const zt_bytes *left, const zt_bytes *right);
zt_bool zt_bytes_starts_with(const zt_bytes *value, const zt_bytes *prefix);
zt_bool zt_bytes_ends_with(const zt_bytes *value, const zt_bytes *suffix);
zt_bool zt_bytes_contains(const zt_bytes *value, const zt_bytes *part);
zt_bytes *zt_text_to_utf8_bytes(const zt_text *value);
zt_outcome_text_text zt_text_from_utf8_bytes(const zt_bytes *value);
zt_int zt_bytes_len(const zt_bytes *value);
uint8_t zt_bytes_get(const zt_bytes *value, zt_int index_0);
zt_bytes *zt_bytes_slice(const zt_bytes *value, zt_int start_0, zt_int end_0);

zt_list_i64 *zt_list_i64_new(void);
zt_list_i64 *zt_list_i64_from_array(const zt_int *items, size_t count);
void zt_list_i64_push(zt_list_i64 *list, zt_int value);
zt_list_i64 *zt_list_i64_push_owned(zt_list_i64 *list, zt_int value);
zt_int zt_list_i64_get(const zt_list_i64 *list, zt_int index_0);
zt_optional_i64 zt_list_i64_get_optional(const zt_list_i64 *list, zt_int index_0);
void zt_list_i64_set(zt_list_i64 *list, zt_int index_0, zt_int value);
zt_list_i64 *zt_list_i64_set_owned(zt_list_i64 *list, zt_int index_0, zt_int value);
zt_int zt_list_i64_len(const zt_list_i64 *list);
zt_list_i64 *zt_list_i64_slice(const zt_list_i64 *list, zt_int start_0, zt_int end_0);

zt_list_text *zt_list_text_new(void);
zt_list_text *zt_list_text_from_array(zt_text *const *items, size_t count);
void zt_list_text_push(zt_list_text *list, zt_text *value);
zt_list_text *zt_list_text_push_owned(zt_list_text *list, zt_text *value);
zt_text *zt_list_text_get(const zt_list_text *list, zt_int index_0);
zt_optional_text zt_list_text_get_optional(const zt_list_text *list, zt_int index_0);
void zt_list_text_set(zt_list_text *list, zt_int index_0, zt_text *value);
zt_list_text *zt_list_text_set_owned(zt_list_text *list, zt_int index_0, zt_text *value);
zt_int zt_list_text_len(const zt_list_text *list);
zt_list_text *zt_list_text_slice(const zt_list_text *list, zt_int start_0, zt_int end_0);
zt_list_text *zt_list_text_deep_copy(const zt_list_text *list);

zt_list_f64 *zt_list_f64_new(void);
zt_list_f64 *zt_list_f64_from_array(const zt_float *items, size_t count);
void zt_list_f64_push(zt_list_f64 *list, zt_float value);
zt_list_f64 *zt_list_f64_push_owned(zt_list_f64 *list, zt_float value);
zt_float zt_list_f64_get(const zt_list_f64 *list, zt_int index_0);
void zt_list_f64_set(zt_list_f64 *list, zt_int index_0, zt_float value);
zt_list_f64 *zt_list_f64_set_owned(zt_list_f64 *list, zt_int index_0, zt_float value);
zt_int zt_list_f64_len(const zt_list_f64 *list);
zt_list_f64 *zt_list_f64_slice(const zt_list_f64 *list, zt_int start_0, zt_int end_0);

zt_dyn_text_repr *zt_dyn_text_repr_from_i64(zt_int value);
zt_dyn_text_repr *zt_dyn_text_repr_from_float(zt_float value);
zt_dyn_text_repr *zt_dyn_text_repr_from_bool(zt_bool value);
zt_dyn_text_repr *zt_dyn_text_repr_from_text(const zt_text *value);
zt_dyn_text_repr *zt_dyn_text_repr_from_text_owned(zt_text *value);
zt_dyn_text_repr *zt_dyn_text_repr_clone(const zt_dyn_text_repr *value);
zt_text *zt_dyn_text_repr_to_text(const zt_dyn_text_repr *value);
zt_int zt_dyn_text_repr_text_len(const zt_dyn_text_repr *value);

zt_list_dyn_text_repr *zt_list_dyn_text_repr_new(void);
zt_list_dyn_text_repr *zt_list_dyn_text_repr_from_array(const zt_dyn_text_repr *const *items, size_t count);
zt_list_dyn_text_repr *zt_list_dyn_text_repr_from_array_owned(zt_dyn_text_repr *const *items, size_t count);
zt_dyn_text_repr *zt_list_dyn_text_repr_get(const zt_list_dyn_text_repr *list, zt_int index_0);
zt_int zt_list_dyn_text_repr_len(const zt_list_dyn_text_repr *list);
zt_list_dyn_text_repr *zt_list_dyn_text_repr_deep_copy(const zt_list_dyn_text_repr *list);

zt_text *zt_thread_boundary_copy_text(const zt_text *value);
zt_bytes *zt_thread_boundary_copy_bytes(const zt_bytes *value);
zt_list_text *zt_thread_boundary_copy_list_text(const zt_list_text *list);
zt_dyn_text_repr *zt_thread_boundary_copy_dyn_text_repr(const zt_dyn_text_repr *value);
zt_list_dyn_text_repr *zt_thread_boundary_copy_list_dyn_text_repr(const zt_list_dyn_text_repr *list);

zt_list_i64 *zt_queue_i64_new(void);
zt_list_i64 *zt_queue_i64_enqueue(zt_list_i64 *queue, zt_int value);
zt_list_i64 *zt_queue_i64_enqueue_owned(zt_list_i64 *queue, zt_int value);
zt_optional_i64 zt_queue_i64_dequeue(zt_list_i64 *queue);
zt_optional_i64 zt_queue_i64_peek(const zt_list_i64 *queue);

zt_list_text *zt_queue_text_new(void);
zt_list_text *zt_queue_text_enqueue(zt_list_text *queue, zt_text *value);
zt_list_text *zt_queue_text_enqueue_owned(zt_list_text *queue, zt_text *value);
zt_optional_text zt_queue_text_dequeue(zt_list_text *queue);
zt_optional_text zt_queue_text_peek(const zt_list_text *queue);

zt_list_i64 *zt_stack_i64_new(void);
zt_list_i64 *zt_stack_i64_push(zt_list_i64 *stack, zt_int value);
zt_list_i64 *zt_stack_i64_push_owned(zt_list_i64 *stack, zt_int value);
zt_optional_i64 zt_stack_i64_pop(zt_list_i64 *stack);
zt_optional_i64 zt_stack_i64_peek(const zt_list_i64 *stack);

zt_list_text *zt_stack_text_new(void);
zt_list_text *zt_stack_text_push(zt_list_text *stack, zt_text *value);
zt_list_text *zt_stack_text_push_owned(zt_list_text *stack, zt_text *value);
zt_optional_text zt_stack_text_pop(zt_list_text *stack);
zt_optional_text zt_stack_text_peek(const zt_list_text *stack);

zt_map_text_text *zt_map_text_text_new(void);
zt_map_text_text *zt_map_text_text_from_arrays(zt_text *const *keys, zt_text *const *values, size_t count);
void zt_map_text_text_set(zt_map_text_text *map, zt_text *key, zt_text *value);
zt_map_text_text *zt_map_text_text_set_owned(zt_map_text_text *map, zt_text *key, zt_text *value);
zt_text *zt_map_text_text_get(const zt_map_text_text *map, const zt_text *key);
zt_optional_text zt_map_text_text_get_optional(const zt_map_text_text *map, const zt_text *key);
zt_text *zt_map_text_text_key_at(const zt_map_text_text *map, zt_int index_0);
zt_text *zt_map_text_text_value_at(const zt_map_text_text *map, zt_int index_0);
zt_int zt_map_text_text_len(const zt_map_text_text *map);

zt_grid2d_i64 *zt_grid2d_i64_new(zt_int rows, zt_int cols);
zt_int zt_grid2d_i64_get(const zt_grid2d_i64 *grid, zt_int row, zt_int col);
zt_grid2d_i64 *zt_grid2d_i64_set(zt_grid2d_i64 *grid, zt_int row, zt_int col, zt_int value);
zt_grid2d_i64 *zt_grid2d_i64_set_owned(zt_grid2d_i64 *grid, zt_int row, zt_int col, zt_int value);
zt_grid2d_i64 *zt_grid2d_i64_fill(zt_grid2d_i64 *grid, zt_int value);
zt_grid2d_i64 *zt_grid2d_i64_fill_owned(zt_grid2d_i64 *grid, zt_int value);
zt_int zt_grid2d_i64_rows(const zt_grid2d_i64 *grid);
zt_int zt_grid2d_i64_cols(const zt_grid2d_i64 *grid);

zt_grid2d_text *zt_grid2d_text_new(zt_int rows, zt_int cols);
zt_text *zt_grid2d_text_get(const zt_grid2d_text *grid, zt_int row, zt_int col);
zt_grid2d_text *zt_grid2d_text_set(zt_grid2d_text *grid, zt_int row, zt_int col, zt_text *value);
zt_grid2d_text *zt_grid2d_text_set_owned(zt_grid2d_text *grid, zt_int row, zt_int col, zt_text *value);
zt_grid2d_text *zt_grid2d_text_fill(zt_grid2d_text *grid, zt_text *value);
zt_grid2d_text *zt_grid2d_text_fill_owned(zt_grid2d_text *grid, zt_text *value);
zt_int zt_grid2d_text_rows(const zt_grid2d_text *grid);
zt_int zt_grid2d_text_cols(const zt_grid2d_text *grid);

zt_pqueue_i64 *zt_pqueue_i64_new(void);
zt_pqueue_i64 *zt_pqueue_i64_push(zt_pqueue_i64 *heap, zt_int value);
zt_pqueue_i64 *zt_pqueue_i64_push_owned(zt_pqueue_i64 *heap, zt_int value);
zt_optional_i64 zt_pqueue_i64_pop(zt_pqueue_i64 *heap);
zt_optional_i64 zt_pqueue_i64_peek(const zt_pqueue_i64 *heap);
zt_int zt_pqueue_i64_len(const zt_pqueue_i64 *heap);

zt_pqueue_text *zt_pqueue_text_new(void);
zt_pqueue_text *zt_pqueue_text_push(zt_pqueue_text *heap, zt_text *value);
zt_pqueue_text *zt_pqueue_text_push_owned(zt_pqueue_text *heap, zt_text *value);
zt_optional_text zt_pqueue_text_pop(zt_pqueue_text *heap);
zt_optional_text zt_pqueue_text_peek(const zt_pqueue_text *heap);
zt_int zt_pqueue_text_len(const zt_pqueue_text *heap);

zt_circbuf_i64 *zt_circbuf_i64_new(zt_int capacity);
zt_circbuf_i64 *zt_circbuf_i64_push(zt_circbuf_i64 *buf, zt_int value);
zt_circbuf_i64 *zt_circbuf_i64_push_owned(zt_circbuf_i64 *buf, zt_int value);
zt_optional_i64 zt_circbuf_i64_pop(zt_circbuf_i64 *buf);
zt_optional_i64 zt_circbuf_i64_peek(const zt_circbuf_i64 *buf);
zt_int zt_circbuf_i64_len(const zt_circbuf_i64 *buf);
zt_int zt_circbuf_i64_capacity(const zt_circbuf_i64 *buf);
zt_bool zt_circbuf_i64_is_full(const zt_circbuf_i64 *buf);

zt_circbuf_text *zt_circbuf_text_new(zt_int capacity);
zt_circbuf_text *zt_circbuf_text_push(zt_circbuf_text *buf, zt_text *value);
zt_circbuf_text *zt_circbuf_text_push_owned(zt_circbuf_text *buf, zt_text *value);
zt_optional_text zt_circbuf_text_pop(zt_circbuf_text *buf);
zt_optional_text zt_circbuf_text_peek(const zt_circbuf_text *buf);
zt_int zt_circbuf_text_len(const zt_circbuf_text *buf);
zt_int zt_circbuf_text_capacity(const zt_circbuf_text *buf);
zt_bool zt_circbuf_text_is_full(const zt_circbuf_text *buf);

zt_btreemap_text_text *zt_btreemap_text_text_new(void);
zt_btreemap_text_text *zt_btreemap_text_text_set(zt_btreemap_text_text *map, zt_text *key, zt_text *value);
zt_btreemap_text_text *zt_btreemap_text_text_set_owned(zt_btreemap_text_text *map, zt_text *key, zt_text *value);
zt_text *zt_btreemap_text_text_get(const zt_btreemap_text_text *map, const zt_text *key);
zt_optional_text zt_btreemap_text_text_get_optional(const zt_btreemap_text_text *map, const zt_text *key);
zt_bool zt_btreemap_text_text_contains(const zt_btreemap_text_text *map, const zt_text *key);
zt_btreemap_text_text *zt_btreemap_text_text_remove(zt_btreemap_text_text *map, const zt_text *key);
zt_btreemap_text_text *zt_btreemap_text_text_remove_owned(zt_btreemap_text_text *map, const zt_text *key);
zt_int zt_btreemap_text_text_len(const zt_btreemap_text_text *map);

zt_btreeset_text *zt_btreeset_text_new(void);
zt_btreeset_text *zt_btreeset_text_insert(zt_btreeset_text *set, zt_text *value);
zt_btreeset_text *zt_btreeset_text_insert_owned(zt_btreeset_text *set, zt_text *value);
zt_bool zt_btreeset_text_contains(const zt_btreeset_text *set, const zt_text *value);
zt_btreeset_text *zt_btreeset_text_remove(zt_btreeset_text *set, const zt_text *value);
zt_btreeset_text *zt_btreeset_text_remove_owned(zt_btreeset_text *set, const zt_text *value);
zt_int zt_btreeset_text_len(const zt_btreeset_text *set);

zt_grid3d_i64 *zt_grid3d_i64_new(zt_int depth, zt_int rows, zt_int cols);
zt_int zt_grid3d_i64_get(const zt_grid3d_i64 *grid, zt_int layer, zt_int row, zt_int col);
zt_grid3d_i64 *zt_grid3d_i64_set(zt_grid3d_i64 *grid, zt_int layer, zt_int row, zt_int col, zt_int value);
zt_grid3d_i64 *zt_grid3d_i64_set_owned(zt_grid3d_i64 *grid, zt_int layer, zt_int row, zt_int col, zt_int value);
zt_grid3d_i64 *zt_grid3d_i64_fill(zt_grid3d_i64 *grid, zt_int value);
zt_grid3d_i64 *zt_grid3d_i64_fill_owned(zt_grid3d_i64 *grid, zt_int value);
zt_int zt_grid3d_i64_depth(const zt_grid3d_i64 *grid);
zt_int zt_grid3d_i64_rows(const zt_grid3d_i64 *grid);
zt_int zt_grid3d_i64_cols(const zt_grid3d_i64 *grid);

zt_grid3d_text *zt_grid3d_text_new(zt_int depth, zt_int rows, zt_int cols);
zt_text *zt_grid3d_text_get(const zt_grid3d_text *grid, zt_int layer, zt_int row, zt_int col);
zt_grid3d_text *zt_grid3d_text_set(zt_grid3d_text *grid, zt_int layer, zt_int row, zt_int col, zt_text *value);
zt_grid3d_text *zt_grid3d_text_set_owned(zt_grid3d_text *grid, zt_int layer, zt_int row, zt_int col, zt_text *value);
zt_grid3d_text *zt_grid3d_text_fill(zt_grid3d_text *grid, zt_text *value);
zt_grid3d_text *zt_grid3d_text_fill_owned(zt_grid3d_text *grid, zt_text *value);
zt_int zt_grid3d_text_depth(const zt_grid3d_text *grid);
zt_int zt_grid3d_text_rows(const zt_grid3d_text *grid);
zt_int zt_grid3d_text_cols(const zt_grid3d_text *grid);

zt_optional_i64 zt_optional_i64_present(zt_int value);
zt_optional_i64 zt_optional_i64_empty(void);
zt_bool zt_optional_i64_is_present(zt_optional_i64 value);
zt_int zt_optional_i64_coalesce(zt_optional_i64 value, zt_int fallback);
zt_int zt_optional_i64_value(zt_optional_i64 value);

zt_optional_text zt_optional_text_present(zt_text *value);
zt_optional_text zt_optional_text_empty(void);
zt_bool zt_optional_text_is_present(zt_optional_text value);
zt_text *zt_optional_text_coalesce(zt_optional_text value, zt_text *fallback);
zt_text *zt_optional_text_value(zt_optional_text value);

zt_optional_bytes zt_optional_bytes_present(zt_bytes *value);
zt_optional_bytes zt_optional_bytes_empty(void);
zt_bool zt_optional_bytes_is_present(zt_optional_bytes value);
zt_bytes *zt_optional_bytes_coalesce(zt_optional_bytes value, zt_bytes *fallback);
zt_bytes *zt_optional_bytes_value(zt_optional_bytes value);

zt_optional_list_i64 zt_optional_list_i64_present(zt_list_i64 *value);
zt_optional_list_i64 zt_optional_list_i64_empty(void);
zt_bool zt_optional_list_i64_is_present(zt_optional_list_i64 value);
zt_list_i64 *zt_optional_list_i64_coalesce(zt_optional_list_i64 value, zt_list_i64 *fallback);
zt_list_i64 *zt_optional_list_i64_value(zt_optional_list_i64 value);

zt_optional_list_text zt_optional_list_text_present(zt_list_text *value);
zt_optional_list_text zt_optional_list_text_empty(void);
zt_bool zt_optional_list_text_is_present(zt_optional_list_text value);
zt_list_text *zt_optional_list_text_coalesce(zt_optional_list_text value, zt_list_text *fallback);

zt_optional_map_text_text zt_optional_map_text_text_present(zt_map_text_text *value);
zt_optional_map_text_text zt_optional_map_text_text_empty(void);
zt_bool zt_optional_map_text_text_is_present(zt_optional_map_text_text value);
zt_map_text_text *zt_optional_map_text_text_coalesce(zt_optional_map_text_text value, zt_map_text_text *fallback);

zt_outcome_i64_text zt_outcome_i64_text_success(zt_int value);
zt_outcome_i64_text zt_outcome_i64_text_failure(zt_text *error);
zt_outcome_i64_text zt_outcome_i64_text_failure_message(const char *message);
zt_bool zt_outcome_i64_text_is_success(zt_outcome_i64_text outcome);
zt_int zt_outcome_i64_text_value(zt_outcome_i64_text outcome);
zt_outcome_i64_text zt_outcome_i64_text_propagate(zt_outcome_i64_text outcome);

zt_outcome_void_text zt_outcome_void_text_success(void);
zt_outcome_void_text zt_outcome_void_text_failure(zt_text *error);
zt_outcome_void_text zt_outcome_void_text_failure_message(const char *message);
zt_bool zt_outcome_void_text_is_success(zt_outcome_void_text outcome);
zt_outcome_void_text zt_outcome_void_text_propagate(zt_outcome_void_text outcome);

typedef struct zt_host_api {
    zt_outcome_text_core_error (*read_file)(const zt_text *path);
    zt_outcome_void_core_error (*write_file)(const zt_text *path, const zt_text *value);
    zt_bool (*path_exists)(const zt_text *path);
    zt_outcome_optional_text_core_error (*read_line_stdin)(void);
    zt_outcome_text_core_error (*read_all_stdin)(void);
    zt_outcome_void_core_error (*write_stdout)(const zt_text *value);
    zt_outcome_void_core_error (*write_stderr)(const zt_text *value);
    zt_int (*time_now_unix_ms)(void);
    zt_outcome_void_core_error (*time_sleep_ms)(zt_int duration_ms);
    void (*random_seed)(zt_int seed);
    zt_int (*random_next_i64)(void);
    zt_outcome_text_core_error (*os_current_dir)(void);
    zt_outcome_void_core_error (*os_change_dir)(const zt_text *path);
    zt_optional_text (*os_env)(const zt_text *name);
    zt_int (*os_pid)(void);
    zt_text *(*os_platform)(void);
    zt_text *(*os_arch)(void);
    zt_outcome_i64_core_error (*process_run)(const zt_text *program, const zt_list_text *args, zt_optional_text cwd);
} zt_host_api;

void zt_host_set_api(const zt_host_api *api);
const zt_host_api *zt_host_get_api(void);
zt_outcome_text_core_error zt_host_read_file(const zt_text *path);
zt_outcome_void_core_error zt_host_write_file(const zt_text *path, const zt_text *value);
zt_bool zt_host_path_exists(const zt_text *path);
zt_outcome_optional_text_core_error zt_host_read_line_stdin(void);
zt_outcome_text_core_error zt_host_read_all_stdin(void);
zt_outcome_void_core_error zt_host_write_stdout(const zt_text *value);
zt_outcome_void_core_error zt_host_write_stderr(const zt_text *value);
zt_int zt_host_time_now_unix_ms(void);
zt_outcome_void_core_error zt_host_time_sleep_ms(zt_int duration_ms);
void zt_host_random_seed(zt_int seed);
zt_int zt_host_random_next_i64(void);
zt_outcome_text_core_error zt_host_os_current_dir(void);
zt_outcome_void_core_error zt_host_os_change_dir(const zt_text *path);
zt_optional_text zt_host_os_env(const zt_text *name);
zt_int zt_host_os_pid(void);
zt_text *zt_host_os_platform(void);
zt_text *zt_host_os_arch(void);
zt_outcome_i64_core_error zt_host_process_run(const zt_text *program, const zt_list_text *args, zt_optional_text cwd);

zt_outcome_text_text zt_outcome_text_text_success(zt_text *value);
zt_outcome_text_text zt_outcome_text_text_failure(zt_text *error);
zt_outcome_text_text zt_outcome_text_text_failure_message(const char *message);
zt_bool zt_outcome_text_text_is_success(zt_outcome_text_text outcome);
zt_text *zt_outcome_text_text_value(zt_outcome_text_text outcome);
zt_outcome_text_text zt_outcome_text_text_propagate(zt_outcome_text_text outcome);
zt_outcome_bytes_text zt_outcome_bytes_text_success(zt_bytes *value);
zt_outcome_bytes_text zt_outcome_bytes_text_failure(zt_text *error);
zt_outcome_bytes_text zt_outcome_bytes_text_failure_message(const char *message);
zt_bool zt_outcome_bytes_text_is_success(zt_outcome_bytes_text outcome);
zt_bytes *zt_outcome_bytes_text_value(zt_outcome_bytes_text outcome);
zt_outcome_bytes_text zt_outcome_bytes_text_propagate(zt_outcome_bytes_text outcome);

zt_bool zt_outcome_text_text_eq(zt_outcome_text_text left, zt_outcome_text_text right);
zt_outcome_optional_text_text zt_outcome_optional_text_text_success(zt_optional_text value);
zt_outcome_optional_text_text zt_outcome_optional_text_text_failure(zt_text *error);
zt_outcome_optional_text_text zt_outcome_optional_text_text_failure_message(const char *message);
zt_bool zt_outcome_optional_text_text_is_success(zt_outcome_optional_text_text outcome);
zt_optional_text zt_outcome_optional_text_text_value(zt_outcome_optional_text_text outcome);
zt_outcome_optional_text_text zt_outcome_optional_text_text_propagate(zt_outcome_optional_text_text outcome);
zt_outcome_optional_bytes_text zt_outcome_optional_bytes_text_success(zt_optional_bytes value);
zt_outcome_optional_bytes_text zt_outcome_optional_bytes_text_failure(zt_text *error);
zt_outcome_optional_bytes_text zt_outcome_optional_bytes_text_failure_message(const char *message);
zt_bool zt_outcome_optional_bytes_text_is_success(zt_outcome_optional_bytes_text outcome);
zt_optional_bytes zt_outcome_optional_bytes_text_value(zt_outcome_optional_bytes_text outcome);
zt_outcome_optional_bytes_text zt_outcome_optional_bytes_text_propagate(zt_outcome_optional_bytes_text outcome);

zt_outcome_net_connection_text zt_outcome_net_connection_text_success(zt_net_connection *value);
zt_outcome_net_connection_text zt_outcome_net_connection_text_failure(zt_text *error);
zt_outcome_net_connection_text zt_outcome_net_connection_text_failure_message(const char *message);
zt_bool zt_outcome_net_connection_text_is_success(zt_outcome_net_connection_text outcome);
zt_net_connection *zt_outcome_net_connection_text_value(zt_outcome_net_connection_text outcome);
zt_outcome_net_connection_text zt_outcome_net_connection_text_propagate(zt_outcome_net_connection_text outcome);

zt_outcome_list_i64_text zt_outcome_list_i64_text_success(zt_list_i64 *value);
zt_outcome_list_i64_text zt_outcome_list_i64_text_failure(zt_text *error);
zt_outcome_list_i64_text zt_outcome_list_i64_text_failure_message(const char *message);
zt_bool zt_outcome_list_i64_text_is_success(zt_outcome_list_i64_text outcome);
zt_list_i64 *zt_outcome_list_i64_text_value(zt_outcome_list_i64_text outcome);
zt_outcome_list_i64_text zt_outcome_list_i64_text_propagate(zt_outcome_list_i64_text outcome);

zt_outcome_list_text_text zt_outcome_list_text_text_success(zt_list_text *value);
zt_outcome_list_text_text zt_outcome_list_text_text_failure(zt_text *error);
zt_bool zt_outcome_list_text_text_is_success(zt_outcome_list_text_text outcome);
zt_list_text *zt_outcome_list_text_text_value(zt_outcome_list_text_text outcome);
zt_outcome_list_text_text zt_outcome_list_text_text_propagate(zt_outcome_list_text_text outcome);

zt_outcome_map_text_text zt_outcome_map_text_text_success(zt_map_text_text *value);
zt_outcome_map_text_text zt_outcome_map_text_text_failure(zt_text *error);
zt_outcome_map_text_text zt_outcome_map_text_text_failure_message(const char *message);
zt_bool zt_outcome_map_text_text_is_success(zt_outcome_map_text_text outcome);
zt_map_text_text *zt_outcome_map_text_text_value(zt_outcome_map_text_text outcome);
zt_outcome_map_text_text zt_outcome_map_text_text_propagate(zt_outcome_map_text_text outcome);

zt_outcome_i64_core_error zt_outcome_i64_core_error_success(zt_int value);
zt_outcome_i64_core_error zt_outcome_i64_core_error_failure(zt_core_error error);
zt_outcome_i64_core_error zt_outcome_i64_core_error_failure_message(const char *message);
zt_outcome_i64_core_error zt_outcome_i64_core_error_failure_text(zt_text *message);
zt_bool zt_outcome_i64_core_error_is_success(zt_outcome_i64_core_error outcome);
zt_int zt_outcome_i64_core_error_value(zt_outcome_i64_core_error outcome);
zt_outcome_i64_core_error zt_outcome_i64_core_error_propagate(zt_outcome_i64_core_error outcome);
void zt_outcome_i64_core_error_dispose(zt_outcome_i64_core_error *outcome);

zt_outcome_void_core_error zt_outcome_void_core_error_success(void);
zt_outcome_void_core_error zt_outcome_void_core_error_failure(zt_core_error error);
zt_outcome_void_core_error zt_outcome_void_core_error_failure_message(const char *message);
zt_outcome_void_core_error zt_outcome_void_core_error_failure_text(zt_text *message);
zt_bool zt_outcome_void_core_error_is_success(zt_outcome_void_core_error outcome);
zt_outcome_void_core_error zt_outcome_void_core_error_propagate(zt_outcome_void_core_error outcome);
void zt_outcome_void_core_error_dispose(zt_outcome_void_core_error *outcome);

zt_outcome_text_core_error zt_outcome_text_core_error_success(zt_text *value);
zt_outcome_text_core_error zt_outcome_text_core_error_failure(zt_core_error error);
zt_outcome_text_core_error zt_outcome_text_core_error_failure_message(const char *message);
zt_outcome_text_core_error zt_outcome_text_core_error_failure_text(zt_text *message);
zt_bool zt_outcome_text_core_error_is_success(zt_outcome_text_core_error outcome);
zt_text *zt_outcome_text_core_error_value(zt_outcome_text_core_error outcome);
zt_outcome_text_core_error zt_outcome_text_core_error_propagate(zt_outcome_text_core_error outcome);
void zt_outcome_text_core_error_dispose(zt_outcome_text_core_error *outcome);

zt_outcome_optional_text_core_error zt_outcome_optional_text_core_error_success(zt_optional_text value);
zt_outcome_optional_text_core_error zt_outcome_optional_text_core_error_failure(zt_core_error error);
zt_outcome_optional_text_core_error zt_outcome_optional_text_core_error_failure_message(const char *message);
zt_outcome_optional_text_core_error zt_outcome_optional_text_core_error_failure_text(zt_text *message);
zt_bool zt_outcome_optional_text_core_error_is_success(zt_outcome_optional_text_core_error outcome);
zt_optional_text zt_outcome_optional_text_core_error_value(zt_outcome_optional_text_core_error outcome);
zt_outcome_optional_text_core_error zt_outcome_optional_text_core_error_propagate(zt_outcome_optional_text_core_error outcome);
void zt_outcome_optional_text_core_error_dispose(zt_outcome_optional_text_core_error *outcome);

zt_outcome_optional_bytes_core_error zt_outcome_optional_bytes_core_error_success(zt_optional_bytes value);
zt_outcome_optional_bytes_core_error zt_outcome_optional_bytes_core_error_failure(zt_core_error error);
zt_outcome_optional_bytes_core_error zt_outcome_optional_bytes_core_error_failure_message(const char *message);
zt_outcome_optional_bytes_core_error zt_outcome_optional_bytes_core_error_failure_text(zt_text *message);
zt_bool zt_outcome_optional_bytes_core_error_is_success(zt_outcome_optional_bytes_core_error outcome);
zt_optional_bytes zt_outcome_optional_bytes_core_error_value(zt_outcome_optional_bytes_core_error outcome);
zt_outcome_optional_bytes_core_error zt_outcome_optional_bytes_core_error_propagate(zt_outcome_optional_bytes_core_error outcome);
void zt_outcome_optional_bytes_core_error_dispose(zt_outcome_optional_bytes_core_error *outcome);

zt_outcome_net_connection_core_error zt_outcome_net_connection_core_error_success(zt_net_connection *value);
zt_outcome_net_connection_core_error zt_outcome_net_connection_core_error_failure(zt_core_error error);
zt_outcome_net_connection_core_error zt_outcome_net_connection_core_error_failure_message(const char *message);
zt_outcome_net_connection_core_error zt_outcome_net_connection_core_error_failure_text(zt_text *message);
zt_bool zt_outcome_net_connection_core_error_is_success(zt_outcome_net_connection_core_error outcome);
zt_net_connection *zt_outcome_net_connection_core_error_value(zt_outcome_net_connection_core_error outcome);
zt_outcome_net_connection_core_error zt_outcome_net_connection_core_error_propagate(zt_outcome_net_connection_core_error outcome);
void zt_outcome_net_connection_core_error_dispose(zt_outcome_net_connection_core_error *outcome);

zt_outcome_list_i64_core_error zt_outcome_list_i64_core_error_success(zt_list_i64 *value);
zt_outcome_list_i64_core_error zt_outcome_list_i64_core_error_failure(zt_core_error error);
zt_outcome_list_i64_core_error zt_outcome_list_i64_core_error_failure_message(const char *message);
zt_outcome_list_i64_core_error zt_outcome_list_i64_core_error_failure_text(zt_text *message);
zt_bool zt_outcome_list_i64_core_error_is_success(zt_outcome_list_i64_core_error outcome);
zt_list_i64 *zt_outcome_list_i64_core_error_value(zt_outcome_list_i64_core_error outcome);
zt_outcome_list_i64_core_error zt_outcome_list_i64_core_error_propagate(zt_outcome_list_i64_core_error outcome);
void zt_outcome_list_i64_core_error_dispose(zt_outcome_list_i64_core_error *outcome);

zt_outcome_map_text_text_core_error zt_outcome_map_text_text_core_error_success(zt_map_text_text *value);
zt_outcome_map_text_text_core_error zt_outcome_map_text_text_core_error_failure(zt_core_error error);
zt_outcome_map_text_text_core_error zt_outcome_map_text_text_core_error_failure_message(const char *message);
zt_outcome_map_text_text_core_error zt_outcome_map_text_text_core_error_failure_text(zt_text *message);
zt_bool zt_outcome_map_text_text_core_error_is_success(zt_outcome_map_text_text_core_error outcome);
zt_map_text_text *zt_outcome_map_text_text_core_error_value(zt_outcome_map_text_text_core_error outcome);
zt_outcome_map_text_text_core_error zt_outcome_map_text_text_core_error_propagate(zt_outcome_map_text_text_core_error outcome);
void zt_outcome_map_text_text_core_error_dispose(zt_outcome_map_text_text_core_error *outcome);

zt_outcome_map_text_text_core_error zt_json_parse_map_text_text(const zt_text *input);
zt_text *zt_json_stringify_map_text_text(const zt_map_text_text *value);
zt_text *zt_json_pretty_map_text_text(const zt_map_text_text *value, zt_int indent);

zt_text *zt_format_number(zt_float value, zt_int decimals);
zt_text *zt_format_percent(zt_float value, zt_int decimals);
zt_text *zt_format_date(zt_int millis, const zt_text *style);
zt_text *zt_format_datetime(zt_int millis, const zt_text *style, const zt_text *locale);
zt_text *zt_format_date_pattern(zt_int millis, const zt_text *pattern);
zt_text *zt_format_datetime_pattern(zt_int millis, const zt_text *pattern);
zt_text *zt_format_hex_i64(zt_int value);
zt_text *zt_format_bin_i64(zt_int value);
zt_text *zt_format_bytes_binary(zt_int value, zt_int decimals);
zt_text *zt_format_bytes_decimal(zt_int value, zt_int decimals);

zt_float zt_math_pow(zt_float base, zt_float exponent);
zt_float zt_math_sqrt(zt_float value);
zt_float zt_math_floor(zt_float value);
zt_float zt_math_ceil(zt_float value);
zt_float zt_math_round_half_away_from_zero(zt_float value);
zt_float zt_math_trunc(zt_float value);
zt_float zt_math_sin(zt_float value);
zt_float zt_math_cos(zt_float value);
zt_float zt_math_tan(zt_float value);
zt_float zt_math_asin(zt_float value);
zt_float zt_math_acos(zt_float value);
zt_float zt_math_atan(zt_float value);
zt_float zt_math_atan2(zt_float y, zt_float x);
zt_float zt_math_ln(zt_float value);
zt_float zt_math_log10(zt_float value);
zt_float zt_math_log_ten(zt_float value);
zt_float zt_math_log2(zt_float value);
zt_float zt_math_log(zt_float value, zt_float base);
zt_float zt_math_exp(zt_float value);
zt_bool zt_math_is_nan(zt_float value);
zt_bool zt_math_is_infinite(zt_float value);
zt_bool zt_math_is_finite(zt_float value);

zt_outcome_net_connection_core_error zt_net_connect(const zt_text *host, zt_int port, zt_int timeout_ms);
zt_outcome_optional_bytes_core_error zt_net_read_some(zt_net_connection *connection, zt_int max, zt_int timeout_ms);
zt_outcome_void_core_error zt_net_write_all(zt_net_connection *connection, const zt_bytes *data, zt_int timeout_ms);
zt_outcome_void_core_error zt_net_close(zt_net_connection *connection);
zt_bool zt_net_is_closed(const zt_net_connection *connection);
zt_int zt_net_error_kind_index(zt_core_error error);

zt_outcome_i64_core_error zt_borealis_open_window(const zt_text *title, zt_int width, zt_int height, zt_int target_fps, zt_int backend_id);
zt_outcome_void_core_error zt_borealis_close_window(zt_int window_id);
zt_bool zt_borealis_window_should_close(zt_int window_id);
zt_outcome_void_core_error zt_borealis_begin_frame(zt_int window_id, zt_int clear_r, zt_int clear_g, zt_int clear_b, zt_int clear_a);
zt_outcome_void_core_error zt_borealis_end_frame(zt_int window_id);
zt_outcome_void_core_error zt_borealis_draw_rect(
    zt_int window_id,
    zt_float x,
    zt_float y,
    zt_float width,
    zt_float height,
    zt_int color_r,
    zt_int color_g,
    zt_int color_b,
    zt_int color_a);
zt_outcome_void_core_error zt_borealis_draw_text(
    zt_int window_id,
    const zt_text *value,
    zt_int x,
    zt_int y,
    zt_int size,
    zt_int color_r,
    zt_int color_g,
    zt_int color_b,
    zt_int color_a);
zt_bool zt_borealis_is_key_down(zt_int window_id, zt_int input_code);

zt_text *zt_path_normalize(const zt_text *value);
zt_bool zt_path_is_absolute(const zt_text *value);
zt_text *zt_path_absolute(const zt_text *value, const zt_text *base);
zt_text *zt_path_relative(const zt_text *value, const zt_text *from);

zt_int zt_add_i64(zt_int a, zt_int b);
zt_int zt_sub_i64(zt_int a, zt_int b);
zt_int zt_mul_i64(zt_int a, zt_int b);
zt_int zt_div_i64(zt_int a, zt_int b);
zt_int zt_rem_i64(zt_int a, zt_int b);
zt_bool zt_validate_between_i64(zt_int value, zt_int min, zt_int max);

#ifdef __cplusplus
}
#endif

#endif







