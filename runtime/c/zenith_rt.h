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
    ZT_HEAP_IMMORTAL_OUTCOME_VOID_TEXT = 255
} zt_heap_kind;

typedef struct zt_header {
    uint32_t rc;
    uint32_t kind;
} zt_header;

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

typedef struct zt_outcome_optional_text_text {
    zt_bool is_success;
    zt_optional_text value;
    zt_text *error;
} zt_outcome_optional_text_text;
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

typedef enum zt_error_kind {
    ZT_ERR_ASSERT,
    ZT_ERR_CHECK,
    ZT_ERR_INDEX,
    ZT_ERR_UNWRAP,
    ZT_ERR_PANIC,
    ZT_ERR_IO,
    ZT_ERR_MATH,
    ZT_ERR_PLATFORM,
    ZT_ERR_CONTRACT
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

void zt_runtime_error(zt_error_kind kind, const char *message);
void zt_runtime_error_ex(zt_error_kind kind, const char *message, const char *code, zt_runtime_span span);
void zt_runtime_error_with_span(zt_error_kind kind, const char *message, zt_runtime_span span);
void zt_assert(zt_bool condition, const char *message);
void zt_check(zt_bool condition, const char *message);
void zt_panic(const char *message);
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

zt_map_text_text *zt_map_text_text_new(void);
zt_map_text_text *zt_map_text_text_from_arrays(zt_text *const *keys, zt_text *const *values, size_t count);
void zt_map_text_text_set(zt_map_text_text *map, zt_text *key, zt_text *value);
zt_map_text_text *zt_map_text_text_set_owned(zt_map_text_text *map, zt_text *key, zt_text *value);
zt_text *zt_map_text_text_get(const zt_map_text_text *map, const zt_text *key);
zt_optional_text zt_map_text_text_get_optional(const zt_map_text_text *map, const zt_text *key);
zt_text *zt_map_text_text_key_at(const zt_map_text_text *map, zt_int index_0);
zt_text *zt_map_text_text_value_at(const zt_map_text_text *map, zt_int index_0);
zt_int zt_map_text_text_len(const zt_map_text_text *map);

zt_optional_i64 zt_optional_i64_present(zt_int value);
zt_optional_i64 zt_optional_i64_empty(void);
zt_bool zt_optional_i64_is_present(zt_optional_i64 value);
zt_int zt_optional_i64_coalesce(zt_optional_i64 value, zt_int fallback);

zt_optional_text zt_optional_text_present(zt_text *value);
zt_optional_text zt_optional_text_empty(void);
zt_bool zt_optional_text_is_present(zt_optional_text value);
zt_text *zt_optional_text_coalesce(zt_optional_text value, zt_text *fallback);

zt_optional_list_i64 zt_optional_list_i64_present(zt_list_i64 *value);
zt_optional_list_i64 zt_optional_list_i64_empty(void);
zt_bool zt_optional_list_i64_is_present(zt_optional_list_i64 value);
zt_list_i64 *zt_optional_list_i64_coalesce(zt_optional_list_i64 value, zt_list_i64 *fallback);

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
    zt_outcome_text_text (*read_file)(const zt_text *path);
    zt_outcome_void_text (*write_file)(const zt_text *path, const zt_text *value);
    zt_bool (*path_exists)(const zt_text *path);
    zt_outcome_optional_text_text (*read_line_stdin)(void);
    zt_outcome_text_text (*read_all_stdin)(void);
    zt_outcome_void_text (*write_stdout)(const zt_text *value);
    zt_outcome_void_text (*write_stderr)(const zt_text *value);
    zt_int (*time_now_unix_ms)(void);
    zt_outcome_void_text (*time_sleep_ms)(zt_int duration_ms);
    zt_outcome_text_text (*os_current_dir)(void);
    zt_outcome_void_text (*os_change_dir)(const zt_text *path);
    zt_optional_text (*os_env)(const zt_text *name);
    zt_int (*os_pid)(void);
    zt_text *(*os_platform)(void);
    zt_text *(*os_arch)(void);
    zt_outcome_i64_text (*process_run)(const zt_text *program, const zt_list_text *args, zt_optional_text cwd);
} zt_host_api;

void zt_host_set_api(const zt_host_api *api);
const zt_host_api *zt_host_get_api(void);
zt_outcome_text_text zt_host_read_file(const zt_text *path);
zt_outcome_void_text zt_host_write_file(const zt_text *path, const zt_text *value);
zt_bool zt_host_path_exists(const zt_text *path);
zt_outcome_optional_text_text zt_host_read_line_stdin(void);
zt_outcome_text_text zt_host_read_all_stdin(void);
zt_outcome_void_text zt_host_write_stdout(const zt_text *value);
zt_outcome_void_text zt_host_write_stderr(const zt_text *value);
zt_int zt_host_time_now_unix_ms(void);
zt_outcome_void_text zt_host_time_sleep_ms(zt_int duration_ms);
zt_outcome_text_text zt_host_os_current_dir(void);
zt_outcome_void_text zt_host_os_change_dir(const zt_text *path);
zt_optional_text zt_host_os_env(const zt_text *name);
zt_int zt_host_os_pid(void);
zt_text *zt_host_os_platform(void);
zt_text *zt_host_os_arch(void);
zt_outcome_i64_text zt_host_process_run(const zt_text *program, const zt_list_text *args, zt_optional_text cwd);

zt_outcome_text_text zt_outcome_text_text_success(zt_text *value);
zt_outcome_text_text zt_outcome_text_text_failure(zt_text *error);
zt_outcome_text_text zt_outcome_text_text_failure_message(const char *message);
zt_bool zt_outcome_text_text_is_success(zt_outcome_text_text outcome);
zt_text *zt_outcome_text_text_value(zt_outcome_text_text outcome);
zt_outcome_text_text zt_outcome_text_text_propagate(zt_outcome_text_text outcome);
zt_bool zt_outcome_text_text_eq(zt_outcome_text_text left, zt_outcome_text_text right);
zt_outcome_optional_text_text zt_outcome_optional_text_text_success(zt_optional_text value);
zt_outcome_optional_text_text zt_outcome_optional_text_text_failure(zt_text *error);
zt_outcome_optional_text_text zt_outcome_optional_text_text_failure_message(const char *message);
zt_bool zt_outcome_optional_text_text_is_success(zt_outcome_optional_text_text outcome);
zt_optional_text zt_outcome_optional_text_text_value(zt_outcome_optional_text_text outcome);
zt_outcome_optional_text_text zt_outcome_optional_text_text_propagate(zt_outcome_optional_text_text outcome);
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

zt_outcome_map_text_text zt_json_parse_map_text_text(const zt_text *input);
zt_text *zt_json_stringify_map_text_text(const zt_map_text_text *value);
zt_text *zt_json_pretty_map_text_text(const zt_map_text_text *value, zt_int indent);

zt_text *zt_format_hex_i64(zt_int value);
zt_text *zt_format_bin_i64(zt_int value);
zt_text *zt_format_bytes_binary(zt_int value, zt_int decimals);
zt_text *zt_format_bytes_decimal(zt_int value, zt_int decimals);

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


