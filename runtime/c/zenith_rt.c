#include "runtime/c/zenith_rt.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#ifdef _WIN32
#include <direct.h>
#include <process.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

static uint32_t zt_hash_text(const zt_text *key) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < key->len; i++) {
        hash ^= (uint8_t)key->data[i];
        hash *= 16777619u;
    }
    return hash;
}

static const char *zt_safe_message(const char *message) {
    return message != NULL ? message : "runtime error";
}

static void zt_runtime_append_text(char *buffer, size_t capacity, const char *text) {
    size_t length;
    size_t available;
    size_t copy_length;

    if (buffer == NULL || capacity == 0 || text == NULL) return;
    length = strlen(buffer);
    if (length >= capacity - 1) return;

    available = (capacity - 1) - length;
    copy_length = strlen(text);
    if (copy_length > available) copy_length = available;
    memcpy(buffer + length, text, copy_length);
    buffer[length + copy_length] = '\0';
}

static zt_runtime_error_info zt_last_error;
static char zt_last_error_message[256];
static char zt_last_error_code[64];

static void zt_runtime_store_error(zt_error_kind kind, const char *message, const char *code, zt_runtime_span span) {
    snprintf(zt_last_error_message, sizeof(zt_last_error_message), "%s", zt_safe_message(message));

    if (code != NULL && code[0] != '\0') {
        snprintf(zt_last_error_code, sizeof(zt_last_error_code), "%s", code);
        zt_last_error.code = zt_last_error_code;
    } else {
        zt_last_error_code[0] = '\0';
        zt_last_error.code = NULL;
    }

    zt_last_error.has_error = true;
    zt_last_error.kind = kind;
    zt_last_error.message = zt_last_error_message;
    zt_last_error.span = span;
}

static const char *zt_runtime_stable_code(zt_error_kind kind) {
    switch (kind) {
        case ZT_ERR_ASSERT: return "runtime.assert";
        case ZT_ERR_CHECK: return "runtime.check";
        case ZT_ERR_PANIC: return "runtime.panic";
        case ZT_ERR_UNWRAP: return "runtime.unwrap";
        case ZT_ERR_IO: return "runtime.io";
        case ZT_ERR_INDEX: return "runtime.index";
        case ZT_ERR_MATH: return "runtime.math";
        case ZT_ERR_PLATFORM: return "runtime.platform";
        case ZT_ERR_CONTRACT: return "runtime.contract";
        default: return "runtime.error";
    }
}

static const char *zt_runtime_default_help(zt_error_kind kind) {
    switch (kind) {
        case ZT_ERR_ASSERT:
            return "Ensure assert conditions are true before runtime.";
        case ZT_ERR_CHECK:
            return "Handle check failures explicitly or validate inputs first.";
        case ZT_ERR_PANIC:
            return "Avoid panic as control flow; handle recoverable failures with result/optional.";
        case ZT_ERR_UNWRAP:
            return "Check optional/result before unwrapping.";
        case ZT_ERR_IO:
            return "Verify file paths, permissions and host environment.";
        case ZT_ERR_INDEX:
            return "Guard indexes and slice bounds before access.";
        case ZT_ERR_MATH:
            return "Check for arithmetic overflow or division by zero.";
        case ZT_ERR_PLATFORM:
            return "Check platform limits and allocation failures.";
        case ZT_ERR_CONTRACT:
            return "Ensure values satisfy type or field contracts.";
        default:
            return "Review runtime preconditions for this operation.";
    }
}

static void zt_runtime_print_error(const zt_runtime_error_info *error) {
    const char *stable_code = zt_runtime_stable_code(error->kind);
    const char *help = zt_runtime_default_help(error->kind);

    fprintf(stderr, "error[%s]\n", stable_code);
    fprintf(stderr, "%s\n", zt_safe_message(error->message));

    if (zt_runtime_span_is_known(error->span)) {
        const char *source_name = (error->span.source_name != NULL && error->span.source_name[0] != '\0')
            ? error->span.source_name
            : "<runtime>";
        fprintf(
            stderr,
            "\nwhere\n  %s:%lld:%lld\n",
            source_name,
            (long long)error->span.line,
            (long long)error->span.column
        );
    }

    if (error->code != NULL && error->code[0] != '\0') {
        fprintf(stderr, "\ncode\n  %s\n", error->code);
    }

    if (help != NULL && help[0] != '\0') {
        fprintf(stderr, "\nhelp\n  %s\n", help);
    }

    fprintf(stderr, "\n");
    fflush(stderr);
}
zt_outcome_i64_text zt_outcome_i64_text_failure_message(const char *message) {
    zt_text *error = zt_text_from_utf8_literal(zt_safe_message(message));
    zt_outcome_i64_text outcome = zt_outcome_i64_text_failure(error);
    zt_release(error);
    return outcome;
}

zt_outcome_text_text zt_outcome_text_text_failure_message(const char *message) {
    zt_text *error = zt_text_from_utf8_literal(zt_safe_message(message));
    zt_outcome_text_text outcome = zt_outcome_text_text_failure(error);
    zt_release(error);
    return outcome;
}

zt_outcome_optional_text_text zt_outcome_optional_text_text_failure_message(const char *message) {
    zt_text *error = zt_text_from_utf8_literal(zt_safe_message(message));
    zt_outcome_optional_text_text outcome = zt_outcome_optional_text_text_failure(error);
    zt_release(error);
    return outcome;
}
zt_outcome_void_text zt_outcome_void_text_failure_message(const char *message) {
    zt_text *error = zt_text_from_utf8_literal(zt_safe_message(message));
    zt_outcome_void_text outcome = zt_outcome_void_text_failure(error);
    zt_release(error);
    return outcome;
}

zt_outcome_list_i64_text zt_outcome_list_i64_text_failure_message(const char *message) {
    zt_text *error = zt_text_from_utf8_literal(zt_safe_message(message));
    zt_outcome_list_i64_text outcome = zt_outcome_list_i64_text_failure(error);
    zt_release(error);
    return outcome;
}

static zt_outcome_text_text zt_host_default_read_file(const zt_text *path);
static zt_outcome_void_text zt_host_default_write_file(const zt_text *path, const zt_text *value);
static zt_bool zt_host_default_path_exists(const zt_text *path);
static zt_outcome_optional_text_text zt_host_default_read_line_stdin(void);
static zt_outcome_text_text zt_host_default_read_all_stdin(void);
static zt_outcome_void_text zt_host_default_write_stdout(const zt_text *value);
static zt_outcome_void_text zt_host_default_write_stderr(const zt_text *value);
static zt_int zt_host_default_time_now_unix_ms(void);
static zt_outcome_void_text zt_host_default_time_sleep_ms(zt_int duration_ms);
static void zt_host_default_random_seed(zt_int seed);
static zt_int zt_host_default_random_next_i64(void);
static zt_outcome_text_text zt_host_default_os_current_dir(void);
static zt_outcome_void_text zt_host_default_os_change_dir(const zt_text *path);
static zt_optional_text zt_host_default_os_env(const zt_text *name);
static zt_int zt_host_default_os_pid(void);
static zt_text *zt_host_default_os_platform(void);
static zt_text *zt_host_default_os_arch(void);
static zt_outcome_i64_text zt_host_default_process_run(const zt_text *program, const zt_list_text *args, zt_optional_text cwd);

static zt_host_api zt_host_api_state = {
    zt_host_default_read_file,
    zt_host_default_write_file,
    zt_host_default_path_exists,
    zt_host_default_read_line_stdin,
    zt_host_default_read_all_stdin,
    zt_host_default_write_stdout,
    zt_host_default_write_stderr,
    zt_host_default_time_now_unix_ms,
    zt_host_default_time_sleep_ms,
    zt_host_default_random_seed,
    zt_host_default_random_next_i64,
    zt_host_default_os_current_dir,
    zt_host_default_os_change_dir,
    zt_host_default_os_env,
    zt_host_default_os_pid,
    zt_host_default_os_platform,
    zt_host_default_os_arch,
    zt_host_default_process_run
};

zt_runtime_span zt_runtime_span_unknown(void) {
    zt_runtime_span span;
    span.source_name = NULL;
    span.line = 0;
    span.column = 0;
    return span;
}

zt_runtime_span zt_runtime_make_span(const char *source_name, zt_int line, zt_int column) {
    zt_runtime_span span;
    span.source_name = source_name;
    span.line = line;
    span.column = column;
    return span;
}

zt_bool zt_runtime_span_is_known(zt_runtime_span span) {
    return span.source_name != NULL &&
           span.source_name[0] != '\0' &&
           span.line > 0 &&
           span.column > 0;
}

const zt_runtime_error_info *zt_runtime_last_error(void) {
    return &zt_last_error;
}

void zt_runtime_clear_error(void) {
    zt_last_error.has_error = false;
    zt_last_error.kind = ZT_ERR_PANIC;
    zt_last_error.message = NULL;
    zt_last_error.code = NULL;
    zt_last_error.span = zt_runtime_span_unknown();
    zt_last_error_message[0] = '\0';
    zt_last_error_code[0] = '\0';
}

const char *zt_error_kind_name(zt_error_kind kind) {
    switch (kind) {
        case ZT_ERR_ASSERT:
            return "assert";
        case ZT_ERR_CHECK:
            return "check";
        case ZT_ERR_INDEX:
            return "index";
        case ZT_ERR_UNWRAP:
            return "unwrap";
        case ZT_ERR_PANIC:
            return "panic";
        case ZT_ERR_IO:
            return "io";
        case ZT_ERR_MATH:
            return "math";
        case ZT_ERR_PLATFORM:
            return "platform";
        case ZT_ERR_CONTRACT:
            return "contract";
        default:
            return "unknown";
    }
}

static zt_header *zt_header_from_ref(void *ref) {
    return (zt_header *)ref;
}

static void zt_free_text(zt_text *value) {
    if (value == NULL) {
        return;
    }

    free(value->data);
    value->data = NULL;
    value->len = 0;
    free(value);
}

static void zt_free_bytes(zt_bytes *value) {
    if (value == NULL) {
        return;
    }

    free(value->data);
    value->data = NULL;
    value->len = 0;
    free(value);
}

static void zt_free_list_i64(zt_list_i64 *list) {
    if (list == NULL) {
        return;
    }

    free(list->data);
    list->data = NULL;
    list->len = 0;
    list->capacity = 0;
    free(list);
}

static void zt_free_list_text(zt_list_text *list) {
    size_t index;

    if (list == NULL) {
        return;
    }

    for (index = 0; index < list->len; index += 1) {
        zt_release(list->data[index]);
    }

    free(list->data);
    list->data = NULL;
    list->len = 0;
    list->capacity = 0;
    free(list);
}

static void zt_free_map_text_text(zt_map_text_text *map) {
    size_t index;

    if (map == NULL) {
        return;
    }

    for (index = 0; index < map->len; index += 1) {
        zt_release(map->keys[index]);
        zt_release(map->values[index]);
    }

    free(map->keys);
    free(map->values);
    free(map->hash_indices);
    map->keys = NULL;
    map->values = NULL;
    map->len = 0;
    map->capacity = 0;
    free(map);
}

static void zt_runtime_require_text(const zt_text *value, const char *message) {
    if (value == NULL) {
        zt_runtime_error(ZT_ERR_PANIC, message);
    }
}

static void zt_runtime_require_bytes(const zt_bytes *value, const char *message) {
    if (value == NULL) {
        zt_runtime_error(ZT_ERR_PANIC, message);
    }
}

static void zt_runtime_require_list_i64(const zt_list_i64 *list, const char *message) {
    if (list == NULL) {
        zt_runtime_error(ZT_ERR_PANIC, message);
    }
}

static void zt_runtime_require_list_text(const zt_list_text *list, const char *message) {
    if (list == NULL) {
        zt_runtime_error(ZT_ERR_PANIC, message);
    }
}

static void zt_runtime_require_map_text_text(const zt_map_text_text *map, const char *message) {
    if (map == NULL) {
        zt_runtime_error(ZT_ERR_PANIC, message);
    }
}

static size_t zt_normalize_slice_end(size_t length, zt_int end_0) {
    if (length == 0) {
        return 0;
    }

    if (end_0 == -1) {
        return length - 1;
    }

    if (end_0 < -1) {
        zt_runtime_error(ZT_ERR_INDEX, "slice end must be -1 or a 0-based index");
    }

    if ((size_t)end_0 >= length) {
        return length - 1;
    }

    return (size_t)end_0;
}

static zt_bool zt_utf8_is_continuation(uint8_t byte) {
    return (byte & 0xC0u) == 0x80u;
}

static zt_bool zt_utf8_validate(const uint8_t *data, size_t len, size_t *error_index, const char **error_reason) {
    size_t index = 0;

    if (len > 0 && data == NULL) {
        if (error_index != NULL) {
            *error_index = 0;
        }
        if (error_reason != NULL) {
            *error_reason = "missing input byte buffer";
        }
        return false;
    }

    while (index < len) {
        uint8_t first = data[index];

        if (first <= 0x7Fu) {
            index += 1;
            continue;
        }

        if (first >= 0xC2u && first <= 0xDFu) {
            if (index + 1 >= len) {
                if (error_index != NULL) {
                    *error_index = index;
                }
                if (error_reason != NULL) {
                    *error_reason = "truncated 2-byte sequence";
                }
                return false;
            }
            if (!zt_utf8_is_continuation(data[index + 1])) {
                if (error_index != NULL) {
                    *error_index = index + 1;
                }
                if (error_reason != NULL) {
                    *error_reason = "invalid continuation byte in 2-byte sequence";
                }
                return false;
            }
            index += 2;
            continue;
        }

        if (first == 0xE0u) {
            if (index + 2 >= len) {
                if (error_index != NULL) {
                    *error_index = index;
                }
                if (error_reason != NULL) {
                    *error_reason = "truncated 3-byte sequence";
                }
                return false;
            }
            if (!(data[index + 1] >= 0xA0u && data[index + 1] <= 0xBFu)) {
                if (error_index != NULL) {
                    *error_index = index + 1;
                }
                if (error_reason != NULL) {
                    *error_reason = "invalid second byte in 3-byte sequence";
                }
                return false;
            }
            if (!zt_utf8_is_continuation(data[index + 2])) {
                if (error_index != NULL) {
                    *error_index = index + 2;
                }
                if (error_reason != NULL) {
                    *error_reason = "invalid continuation byte in 3-byte sequence";
                }
                return false;
            }
            index += 3;
            continue;
        }

        if ((first >= 0xE1u && first <= 0xECu) || (first >= 0xEEu && first <= 0xEFu)) {
            if (index + 2 >= len) {
                if (error_index != NULL) {
                    *error_index = index;
                }
                if (error_reason != NULL) {
                    *error_reason = "truncated 3-byte sequence";
                }
                return false;
            }
            if (!zt_utf8_is_continuation(data[index + 1]) || !zt_utf8_is_continuation(data[index + 2])) {
                if (error_index != NULL) {
                    *error_index = !zt_utf8_is_continuation(data[index + 1]) ? index + 1 : index + 2;
                }
                if (error_reason != NULL) {
                    *error_reason = "invalid continuation byte in 3-byte sequence";
                }
                return false;
            }
            index += 3;
            continue;
        }

        if (first == 0xEDu) {
            if (index + 2 >= len) {
                if (error_index != NULL) {
                    *error_index = index;
                }
                if (error_reason != NULL) {
                    *error_reason = "truncated 3-byte sequence";
                }
                return false;
            }
            if (!(data[index + 1] >= 0x80u && data[index + 1] <= 0x9Fu)) {
                if (error_index != NULL) {
                    *error_index = index + 1;
                }
                if (error_reason != NULL) {
                    *error_reason = "invalid second byte for surrogate range";
                }
                return false;
            }
            if (!zt_utf8_is_continuation(data[index + 2])) {
                if (error_index != NULL) {
                    *error_index = index + 2;
                }
                if (error_reason != NULL) {
                    *error_reason = "invalid continuation byte in 3-byte sequence";
                }
                return false;
            }
            index += 3;
            continue;
        }

        if (first == 0xF0u) {
            if (index + 3 >= len) {
                if (error_index != NULL) {
                    *error_index = index;
                }
                if (error_reason != NULL) {
                    *error_reason = "truncated 4-byte sequence";
                }
                return false;
            }
            if (!(data[index + 1] >= 0x90u && data[index + 1] <= 0xBFu)) {
                if (error_index != NULL) {
                    *error_index = index + 1;
                }
                if (error_reason != NULL) {
                    *error_reason = "invalid second byte in 4-byte sequence";
                }
                return false;
            }
            if (!zt_utf8_is_continuation(data[index + 2]) || !zt_utf8_is_continuation(data[index + 3])) {
                if (error_index != NULL) {
                    *error_index = !zt_utf8_is_continuation(data[index + 2]) ? index + 2 : index + 3;
                }
                if (error_reason != NULL) {
                    *error_reason = "invalid continuation byte in 4-byte sequence";
                }
                return false;
            }
            index += 4;
            continue;
        }

        if (first >= 0xF1u && first <= 0xF3u) {
            if (index + 3 >= len) {
                if (error_index != NULL) {
                    *error_index = index;
                }
                if (error_reason != NULL) {
                    *error_reason = "truncated 4-byte sequence";
                }
                return false;
            }
            if (!zt_utf8_is_continuation(data[index + 1]) ||
                !zt_utf8_is_continuation(data[index + 2]) ||
                !zt_utf8_is_continuation(data[index + 3])) {
                if (error_index != NULL) {
                    if (!zt_utf8_is_continuation(data[index + 1])) {
                        *error_index = index + 1;
                    } else if (!zt_utf8_is_continuation(data[index + 2])) {
                        *error_index = index + 2;
                    } else {
                        *error_index = index + 3;
                    }
                }
                if (error_reason != NULL) {
                    *error_reason = "invalid continuation byte in 4-byte sequence";
                }
                return false;
            }
            index += 4;
            continue;
        }

        if (first == 0xF4u) {
            if (index + 3 >= len) {
                if (error_index != NULL) {
                    *error_index = index;
                }
                if (error_reason != NULL) {
                    *error_reason = "truncated 4-byte sequence";
                }
                return false;
            }
            if (!(data[index + 1] >= 0x80u && data[index + 1] <= 0x8Fu)) {
                if (error_index != NULL) {
                    *error_index = index + 1;
                }
                if (error_reason != NULL) {
                    *error_reason = "invalid second byte in 4-byte sequence";
                }
                return false;
            }
            if (!zt_utf8_is_continuation(data[index + 2]) || !zt_utf8_is_continuation(data[index + 3])) {
                if (error_index != NULL) {
                    *error_index = !zt_utf8_is_continuation(data[index + 2]) ? index + 2 : index + 3;
                }
                if (error_reason != NULL) {
                    *error_reason = "invalid continuation byte in 4-byte sequence";
                }
                return false;
            }
            index += 4;
            continue;
        }

        if (error_index != NULL) {
            *error_index = index;
        }
        if (error_reason != NULL) {
            *error_reason = "invalid leading byte";
        }
        return false;
    }

    if (error_index != NULL) {
        *error_index = len;
    }
    if (error_reason != NULL) {
        *error_reason = NULL;
    }
    return true;
}

static void zt_list_i64_reserve(zt_list_i64 *list, size_t min_capacity) {
    size_t new_capacity;
    zt_int *new_data;

    zt_runtime_require_list_i64(list, "zt_list_i64_reserve requires list");

    if (min_capacity <= list->capacity) {
        return;
    }

    new_capacity = list->capacity > 0 ? list->capacity : 4;
    while (new_capacity < min_capacity) {
        if (new_capacity > SIZE_MAX / 2) {
            zt_runtime_error(ZT_ERR_PLATFORM, "list capacity overflow");
        }
        new_capacity *= 2;
    }

    new_data = (zt_int *)realloc(list->data, new_capacity * sizeof(zt_int));
    if (new_data == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to grow list<int> buffer");
    }

    list->data = new_data;
    list->capacity = new_capacity;
}

static void zt_list_text_reserve(zt_list_text *list, size_t min_capacity) {
    size_t new_capacity;
    zt_text **new_data;

    zt_runtime_require_list_text(list, "zt_list_text_reserve requires list");

    if (min_capacity <= list->capacity) {
        return;
    }

    new_capacity = list->capacity > 0 ? list->capacity : 4;
    while (new_capacity < min_capacity) {
        if (new_capacity > SIZE_MAX / 2) {
            zt_runtime_error(ZT_ERR_PLATFORM, "list capacity overflow");
        }
        new_capacity *= 2;
    }

    new_data = (zt_text **)realloc(list->data, new_capacity * sizeof(zt_text *));
    if (new_data == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to grow list<text> buffer");
    }

    list->data = new_data;
    list->capacity = new_capacity;
}

static void zt_map_text_text_reserve(zt_map_text_text *map, size_t min_capacity) {
    size_t new_capacity;
    zt_text **new_keys;
    zt_text **new_values;

    zt_runtime_require_map_text_text(map, "zt_map_text_text_reserve requires map");

    if (min_capacity <= map->capacity) {
        return;
    }

    new_capacity = map->capacity > 0 ? map->capacity : 4;
    while (new_capacity < min_capacity) {
        if (new_capacity > SIZE_MAX / 2) {
            zt_runtime_error(ZT_ERR_PLATFORM, "map capacity overflow");
        }
        new_capacity *= 2;
    }

    new_keys = (zt_text **)realloc(map->keys, new_capacity * sizeof(zt_text *));
    if (new_keys == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to grow map<text,text> key buffer");
    }

    new_values = (zt_text **)realloc(map->values, new_capacity * sizeof(zt_text *));
    if (new_values == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to grow map<text,text> value buffer");
    }

    map->keys = new_keys;
    map->values = new_values;
    map->capacity = new_capacity;
}

static size_t zt_map_text_text_find_index(const zt_map_text_text *map, const zt_text *key, zt_bool *found) {
    size_t index;

    zt_runtime_require_map_text_text(map, "zt_map_text_text_find_index requires map");
    zt_runtime_require_text(key, "zt_map_text_text_find_index requires key");

    for (index = 0; index < map->len; index += 1) {
        if (zt_text_eq(map->keys[index], key)) {
            *found = true;
            return index;
        }
    }

    *found = false;
    return map->len;
}





















static zt_map_text_text *zt_map_text_text_alloc(void) {
    zt_map_text_text *map;

    map = (zt_map_text_text *)calloc(1, sizeof(zt_map_text_text));
    if (map == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate map<text,text> header");
    }

    map->header.rc = 1;
    map->header.kind = (uint32_t)ZT_HEAP_MAP_TEXT_TEXT;
    return map;
}

void zt_retain(void *ref) {
    zt_header *header;

    if (ref == NULL) {
        return;
    }

    header = zt_header_from_ref(ref);
    if (header->kind == (uint32_t)ZT_HEAP_IMMORTAL_OUTCOME_VOID_TEXT) {
        return;
    }
    if (header->rc == UINT32_MAX) {
        zt_runtime_error(ZT_ERR_PLATFORM, "reference count overflow");
    }

    header->rc += 1;
}

void zt_release(void *ref) {
    zt_header *header;

    if (ref == NULL) {
        return;
    }

    header = zt_header_from_ref(ref);
    if (header->kind == (uint32_t)ZT_HEAP_IMMORTAL_OUTCOME_VOID_TEXT) {
        return;
    }
    if (header->rc == 0) {
        zt_runtime_error(ZT_ERR_PLATFORM, "release on object with rc=0");
    }

    header->rc -= 1;
    if (header->rc > 0) {
        return;
    }

    switch ((zt_heap_kind)header->kind) {
        case ZT_HEAP_TEXT:
            zt_free_text((zt_text *)ref);
            return;
        case ZT_HEAP_BYTES:
            zt_free_bytes((zt_bytes *)ref);
            return;
        case ZT_HEAP_LIST_I64:
            zt_free_list_i64((zt_list_i64 *)ref);
            return;
        case ZT_HEAP_LIST_TEXT:
            zt_free_list_text((zt_list_text *)ref);
            return;
        case ZT_HEAP_MAP_TEXT_TEXT:
            zt_free_map_text_text((zt_map_text_text *)ref);
            return;
        case ZT_HEAP_IMMORTAL_OUTCOME_VOID_TEXT:
            return;
        case ZT_HEAP_UNKNOWN:
        default:
            free(ref);
            return;
    }
}

void zt_runtime_report_error(zt_error_kind kind, const char *message, const char *code, zt_runtime_span span) {
    zt_runtime_store_error(kind, message, code, span);
}

void zt_runtime_error_ex(zt_error_kind kind, const char *message, const char *code, zt_runtime_span span) {
    zt_runtime_report_error(kind, message, code, span);
    zt_runtime_print_error(&zt_last_error);
    exit(1);
}

void zt_runtime_error_with_span(zt_error_kind kind, const char *message, zt_runtime_span span) {
    zt_runtime_error_ex(kind, message, NULL, span);
}

void zt_runtime_error(zt_error_kind kind, const char *message) {
    zt_runtime_error_ex(kind, message, NULL, zt_runtime_span_unknown());
}

void zt_assert(zt_bool condition, const char *message) {
    if (!condition) {
        zt_runtime_error(ZT_ERR_ASSERT, message);
    }
}

void zt_check(zt_bool condition, const char *message) {
    if (!condition) {
        zt_runtime_error(ZT_ERR_CHECK, message);
    }
}

void zt_panic(const char *message) {
    zt_runtime_error(ZT_ERR_PANIC, message);
}

void zt_contract_failed(const char *message, zt_runtime_span span) {
    zt_runtime_error_with_span(ZT_ERR_CONTRACT, message, span);
}

void zt_contract_failed_i64(const char *message, zt_int value, zt_runtime_span span) {
    char full_message[512];
    char value_text[64];
    full_message[0] = '\0';
    snprintf(value_text, sizeof(value_text), "%lld", (long long)value);
    zt_runtime_append_text(full_message, sizeof(full_message), zt_safe_message(message));
    zt_runtime_append_text(full_message, sizeof(full_message), " (value: ");
    zt_runtime_append_text(full_message, sizeof(full_message), value_text);
    zt_runtime_append_text(full_message, sizeof(full_message), ")");
    zt_contract_failed(full_message, span);
}

void zt_contract_failed_float(const char *message, zt_float value, zt_runtime_span span) {
    char full_message[512];
    char value_text[64];
    full_message[0] = '\0';
    snprintf(value_text, sizeof(value_text), "%.17g", (double)value);
    zt_runtime_append_text(full_message, sizeof(full_message), zt_safe_message(message));
    zt_runtime_append_text(full_message, sizeof(full_message), " (value: ");
    zt_runtime_append_text(full_message, sizeof(full_message), value_text);
    zt_runtime_append_text(full_message, sizeof(full_message), ")");
    zt_contract_failed(full_message, span);
}

void zt_contract_failed_bool(const char *message, zt_bool value, zt_runtime_span span) {
    char full_message[512];
    full_message[0] = '\0';
    zt_runtime_append_text(full_message, sizeof(full_message), zt_safe_message(message));
    zt_runtime_append_text(full_message, sizeof(full_message), " (value: ");
    zt_runtime_append_text(full_message, sizeof(full_message), value ? "true" : "false");
    zt_runtime_append_text(full_message, sizeof(full_message), ")");
    zt_contract_failed(full_message, span);
}

zt_text *zt_text_from_utf8(const char *data, size_t len) {
    zt_text *value;

    value = (zt_text *)calloc(1, sizeof(zt_text));
    if (value == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate text header");
    }

    value->data = (char *)malloc(len + 1);
    if (value->data == NULL) {
        free(value);
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate text bytes");
    }

    if (data != NULL && len > 0) {
        memcpy(value->data, data, len);
    }

    value->data[len] = '\0';
    value->len = len;
    value->header.rc = 1;
    value->header.kind = (uint32_t)ZT_HEAP_TEXT;
    return value;
}

zt_text *zt_text_from_utf8_literal(const char *data) {
    if (data == NULL) {
        return zt_text_from_utf8("", 0);
    }

    return zt_text_from_utf8(data, strlen(data));
}

zt_text *zt_text_concat(const zt_text *a, const zt_text *b) {
    zt_text *value;
    size_t left_len;
    size_t right_len;

    zt_runtime_require_text(a, "zt_text_concat requires left text");
    zt_runtime_require_text(b, "zt_text_concat requires right text");

    left_len = a->len;
    right_len = b->len;
    value = zt_text_from_utf8(NULL, left_len + right_len);
    if (left_len > 0) {
        memcpy(value->data, a->data, left_len);
    }
    if (right_len > 0) {
        memcpy(value->data + left_len, b->data, right_len);
    }
    value->data[left_len + right_len] = '\0';
    return value;
}

zt_text *zt_text_index(const zt_text *value, zt_int index_0) {
    zt_runtime_require_text(value, "zt_text_index requires text");

    if (index_0 < 0 || (size_t)index_0 >= value->len) {
        zt_runtime_error(ZT_ERR_INDEX, "text index out of bounds");
    }

    return zt_text_from_utf8(value->data + (size_t)index_0, 1);
}

zt_text *zt_text_slice(const zt_text *value, zt_int start_0, zt_int end_0) {
    size_t start_pos;
    size_t end_pos;

    zt_runtime_require_text(value, "zt_text_slice requires text");

    if (start_0 < 0) {
        zt_runtime_error(ZT_ERR_INDEX, "slice start must be >= 0");
    }

    if (value->len == 0) {
        return zt_text_from_utf8("", 0);
    }

    start_pos = (size_t)start_0;
    end_pos = zt_normalize_slice_end(value->len, end_0);

    if (start_pos >= value->len || end_pos < start_pos) {
        return zt_text_from_utf8("", 0);
    }

    return zt_text_from_utf8(value->data + start_pos, end_pos - start_pos + 1);
}

zt_bool zt_text_eq(const zt_text *a, const zt_text *b) {
    zt_runtime_require_text(a, "zt_text_eq requires left text");
    zt_runtime_require_text(b, "zt_text_eq requires right text");

    if (a == b) {
        return true;
    }

    if (a->len != b->len) {
        return false;
    }

    if (a->len == 0) {
        return true;
    }

    return memcmp(a->data, b->data, a->len) == 0;
}

zt_int zt_text_len(const zt_text *value) {
    zt_runtime_require_text(value, "zt_text_len requires text");
    return (zt_int)value->len;
}

const char *zt_text_data(const zt_text *value) {
    zt_runtime_require_text(value, "zt_text_data requires text");
    return value->data != NULL ? value->data : "";
}

zt_text *zt_text_deep_copy(const zt_text *value) {
    zt_runtime_require_text(value, "zt_text_deep_copy requires text");
    return zt_text_from_utf8(value->data, value->len);
}

zt_bytes *zt_bytes_empty(void) {
    return zt_bytes_from_array(NULL, 0);
}

zt_bytes *zt_bytes_from_array(const uint8_t *data, size_t len) {
    zt_bytes *value;

    value = (zt_bytes *)calloc(1, sizeof(zt_bytes));
    if (value == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate bytes header");
    }

    if (len > 0) {
        value->data = (uint8_t *)malloc(len);
        if (value->data == NULL) {
            free(value);
            zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate bytes data");
        }
        if (data != NULL) {
            memcpy(value->data, data, len);
        } else {
            memset(value->data, 0, len);
        }
    }

    value->len = len;
    value->header.rc = 1;
    value->header.kind = (uint32_t)ZT_HEAP_BYTES;
    return value;
}

zt_bytes *zt_bytes_from_list_i64(const zt_list_i64 *values) {
    zt_bytes *bytes_value;
    size_t index;

    zt_runtime_require_list_i64(values, "zt_bytes_from_list_i64 requires list<int>");

    if (values->len == 0) {
        return zt_bytes_from_array(NULL, 0);
    }

    bytes_value = zt_bytes_from_array(NULL, values->len);
    for (index = 0; index < values->len; index += 1) {
        zt_int item = values->data[index];
        if (item < 0 || item > 255) {
            zt_release(bytes_value);
            zt_runtime_error(ZT_ERR_CHECK, "std.bytes.from_list expects items in the 0..255 range");
        }
        bytes_value->data[index] = (uint8_t)item;
    }

    return bytes_value;
}

zt_list_i64 *zt_bytes_to_list_i64(const zt_bytes *value) {
    zt_list_i64 *list;
    size_t index;

    zt_runtime_require_bytes(value, "zt_bytes_to_list_i64 requires bytes");

    list = zt_list_i64_new();
    if (value->len == 0) {
        return list;
    }

    zt_list_i64_reserve(list, value->len);
    for (index = 0; index < value->len; index += 1) {
        list->data[index] = (zt_int)value->data[index];
    }
    list->len = value->len;

    return list;
}

zt_bytes *zt_bytes_join(const zt_bytes *left, const zt_bytes *right) {
    zt_bytes *joined;

    zt_runtime_require_bytes(left, "zt_bytes_join requires left bytes");
    zt_runtime_require_bytes(right, "zt_bytes_join requires right bytes");

    joined = zt_bytes_from_array(NULL, left->len + right->len);
    if (left->len > 0) {
        memcpy(joined->data, left->data, left->len);
    }
    if (right->len > 0) {
        memcpy(joined->data + left->len, right->data, right->len);
    }

    return joined;
}

zt_bool zt_bytes_starts_with(const zt_bytes *value, const zt_bytes *prefix) {
    zt_runtime_require_bytes(value, "zt_bytes_starts_with requires bytes value");
    zt_runtime_require_bytes(prefix, "zt_bytes_starts_with requires bytes prefix");

    if (prefix->len > value->len) {
        return false;
    }

    if (prefix->len == 0) {
        return true;
    }

    return memcmp(value->data, prefix->data, prefix->len) == 0;
}

zt_bool zt_bytes_ends_with(const zt_bytes *value, const zt_bytes *suffix) {
    zt_runtime_require_bytes(value, "zt_bytes_ends_with requires bytes value");
    zt_runtime_require_bytes(suffix, "zt_bytes_ends_with requires bytes suffix");

    if (suffix->len > value->len) {
        return false;
    }

    if (suffix->len == 0) {
        return true;
    }

    return memcmp(value->data + (value->len - suffix->len), suffix->data, suffix->len) == 0;
}

zt_bool zt_bytes_contains(const zt_bytes *value, const zt_bytes *part) {
    size_t index;

    zt_runtime_require_bytes(value, "zt_bytes_contains requires bytes value");
    zt_runtime_require_bytes(part, "zt_bytes_contains requires bytes part");

    if (part->len == 0) {
        return true;
    }

    if (part->len > value->len) {
        return false;
    }

    for (index = 0; index + part->len <= value->len; index += 1) {
        if (memcmp(value->data + index, part->data, part->len) == 0) {
            return true;
        }
    }

    return false;
}
zt_bytes *zt_text_to_utf8_bytes(const zt_text *value) {
    zt_runtime_require_text(value, "zt_text_to_utf8_bytes requires text");
    return zt_bytes_from_array((const uint8_t *)value->data, value->len);
}

zt_outcome_text_text zt_text_from_utf8_bytes(const zt_bytes *value) {
    size_t error_index;
    const char *error_reason;
    char message[192];
    zt_text *text;
    zt_outcome_text_text outcome;

    zt_runtime_require_bytes(value, "zt_text_from_utf8_bytes requires bytes");

    if (!zt_utf8_validate(value->data, value->len, &error_index, &error_reason)) {
        snprintf(
            message,
            sizeof(message),
            "invalid UTF-8 at byte %llu: %s",
            (unsigned long long)error_index,
            error_reason != NULL ? error_reason : "malformed sequence"
        );
        return zt_outcome_text_text_failure_message(message);
    }

    text = zt_text_from_utf8((const char *)value->data, value->len);
    outcome = zt_outcome_text_text_success(text);
    zt_release(text);
    return outcome;
}

zt_int zt_bytes_len(const zt_bytes *value) {
    zt_runtime_require_bytes(value, "zt_bytes_len requires bytes");
    return (zt_int)value->len;
}

uint8_t zt_bytes_get(const zt_bytes *value, zt_int index_0) {
    zt_runtime_require_bytes(value, "zt_bytes_get requires bytes");

    if (index_0 < 0 || (size_t)index_0 >= value->len) {
        zt_runtime_error(ZT_ERR_INDEX, "bytes index out of bounds");
    }

    return value->data[(size_t)index_0];
}

zt_bytes *zt_bytes_slice(const zt_bytes *value, zt_int start_0, zt_int end_0) {
    size_t start_pos;
    size_t end_pos;

    zt_runtime_require_bytes(value, "zt_bytes_slice requires bytes");

    if (start_0 < 0) {
        zt_runtime_error(ZT_ERR_INDEX, "slice start must be >= 0");
    }

    if (value->len == 0) {
        return zt_bytes_from_array(NULL, 0);
    }

    start_pos = (size_t)start_0;
    end_pos = zt_normalize_slice_end(value->len, end_0);

    if (start_pos >= value->len || end_pos < start_pos) {
        return zt_bytes_from_array(NULL, 0);
    }

    return zt_bytes_from_array(value->data + start_pos, end_pos - start_pos + 1);
}

zt_list_i64 *zt_list_i64_new(void) {
    zt_list_i64 *list;

    list = (zt_list_i64 *)calloc(1, sizeof(zt_list_i64));
    if (list == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate list<int> header");
    }

    list->header.rc = 1;
    list->header.kind = (uint32_t)ZT_HEAP_LIST_I64;
    return list;
}

zt_list_i64 *zt_list_i64_from_array(const zt_int *items, size_t count) {
    zt_list_i64 *list;

    list = zt_list_i64_new();
    if (count == 0) {
        return list;
    }

    zt_list_i64_reserve(list, count);
    memcpy(list->data, items, count * sizeof(zt_int));
    list->len = count;
    return list;
}

zt_list_i64 *zt_list_i64_push_owned(zt_list_i64 *list, zt_int value) {
    zt_runtime_require_list_i64(list, "zt_list_i64_push_owned requires list");
    if (list->header.rc > 1u) {
        zt_list_i64 *clone = zt_list_i64_from_array(list->data, list->len);
        zt_release(list);
        list = clone;
    }
    zt_list_i64_push(list, value);
    return list;
}

void zt_list_i64_push(zt_list_i64 *list, zt_int value) {
    zt_runtime_require_list_i64(list, "zt_list_i64_push requires list");
    zt_list_i64_reserve(list, list->len + 1);
    list->data[list->len] = value;
    list->len += 1;
}

zt_int zt_list_i64_get(const zt_list_i64 *list, zt_int index_0) {
    zt_runtime_require_list_i64(list, "zt_list_i64_get requires list");

    if (index_0 < 0 || (size_t)index_0 >= list->len) {
        zt_runtime_error(ZT_ERR_INDEX, "list<int> index out of bounds");
    }

    return list->data[index_0];
}

zt_optional_i64 zt_list_i64_get_optional(const zt_list_i64 *list, zt_int index_0) {
    zt_runtime_require_list_i64(list, "zt_list_i64_get_optional requires list");

    if (index_0 < 0 || (size_t)index_0 >= list->len) {
        return zt_optional_i64_empty();
    }
    return zt_optional_i64_present(list->data[index_0]);
}

void zt_list_i64_set(zt_list_i64 *list, zt_int index_0, zt_int value) {
    zt_runtime_require_list_i64(list, "zt_list_i64_set requires list");

    if (index_0 < 0 || (size_t)index_0 >= list->len) {
        zt_runtime_error(ZT_ERR_INDEX, "list<int> index out of bounds");
    }

    list->data[index_0] = value;
}

zt_list_i64 *zt_list_i64_set_owned(zt_list_i64 *list, zt_int index_0, zt_int value) {
    zt_runtime_require_list_i64(list, "zt_list_i64_set_owned requires list");

    if (list->header.rc > 1u) {
        zt_list_i64 *clone = zt_list_i64_from_array(list->data, list->len);
        zt_release(list);
        list = clone;
    }

    zt_list_i64_set(list, index_0, value);
    return list;
}

zt_int zt_list_i64_len(const zt_list_i64 *list) {
    zt_runtime_require_list_i64(list, "zt_list_i64_len requires list");
    return (zt_int)list->len;
}

zt_list_i64 *zt_list_i64_slice(const zt_list_i64 *list, zt_int start_0, zt_int end_0) {
    size_t start_pos;
    size_t end_pos;

    zt_runtime_require_list_i64(list, "zt_list_i64_slice requires list");

    if (start_0 < 0) {
        zt_runtime_error(ZT_ERR_INDEX, "slice start must be >= 0");
    }

    if (list->len == 0) {
        return zt_list_i64_new();
    }

    start_pos = (size_t)start_0;
    end_pos = zt_normalize_slice_end(list->len, end_0);

    if (start_pos >= list->len || end_pos < start_pos) {
        return zt_list_i64_new();
    }

    return zt_list_i64_from_array(list->data + start_pos, end_pos - start_pos + 1);
}

zt_list_text *zt_list_text_new(void) {
    zt_list_text *list;

    list = (zt_list_text *)calloc(1, sizeof(zt_list_text));
    if (list == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate list<text> header");
    }

    list->header.rc = 1;
    list->header.kind = (uint32_t)ZT_HEAP_LIST_TEXT;
    return list;
}

zt_list_text *zt_list_text_from_array(zt_text *const *items, size_t count) {
    zt_list_text *list;
    size_t index;

    list = zt_list_text_new();
    if (count == 0) {
        return list;
    }

    if (items == NULL) {
        zt_runtime_error(ZT_ERR_PANIC, "zt_list_text_from_array requires items");
    }

    zt_list_text_reserve(list, count);
    for (index = 0; index < count; index += 1) {
        zt_runtime_require_text(items[index], "zt_list_text_from_array requires text items");
        zt_retain(items[index]);
        list->data[index] = items[index];
    }
    list->len = count;
    return list;
}

zt_list_text *zt_list_text_push_owned(zt_list_text *list, zt_text *value) {
    zt_runtime_require_list_text(list, "zt_list_text_push_owned requires list");
    if (list->header.rc > 1u) {
        zt_list_text *clone = zt_list_text_from_array(list->data, list->len);
        zt_release(list);
        list = clone;
    }
    zt_list_text_push(list, value);
    return list;
}

void zt_list_text_push(zt_list_text *list, zt_text *value) {
    zt_runtime_require_list_text(list, "zt_list_text_push requires list");
    zt_runtime_require_text(value, "zt_list_text_push requires text");
    zt_list_text_reserve(list, list->len + 1);
    zt_retain(value);
    list->data[list->len] = value;
    list->len += 1;
}

zt_text *zt_list_text_get(const zt_list_text *list, zt_int index_0) {
    zt_text *value;

    zt_runtime_require_list_text(list, "zt_list_text_get requires list");

    if (index_0 < 0 || (size_t)index_0 >= list->len) {
        zt_runtime_error(ZT_ERR_INDEX, "list<text> index out of bounds");
    }

    value = list->data[index_0];
    zt_runtime_require_text(value, "list<text> entry cannot be null");
    zt_retain(value);
    return value;
}

zt_optional_text zt_list_text_get_optional(const zt_list_text *list, zt_int index_0) {
    zt_text *value;

    zt_runtime_require_list_text(list, "zt_list_text_get_optional requires list");

    if (index_0 < 0 || (size_t)index_0 >= list->len) {
        return zt_optional_text_empty();
    }

    value = list->data[index_0];
    zt_runtime_require_text(value, "list<text> entry cannot be null");
    return zt_optional_text_present(value);
}

void zt_list_text_set(zt_list_text *list, zt_int index_0, zt_text *value) {
    zt_runtime_require_list_text(list, "zt_list_text_set requires list");
    zt_runtime_require_text(value, "zt_list_text_set requires text");

    if (index_0 < 0 || (size_t)index_0 >= list->len) {
        zt_runtime_error(ZT_ERR_INDEX, "list<text> index out of bounds");
    }

    zt_retain(value);
    zt_release(list->data[index_0]);
    list->data[index_0] = value;
}

zt_list_text *zt_list_text_set_owned(zt_list_text *list, zt_int index_0, zt_text *value) {
    zt_runtime_require_list_text(list, "zt_list_text_set_owned requires list");

    if (list->header.rc > 1u) {
        zt_list_text *clone = zt_list_text_from_array(list->data, list->len);
        zt_release(list);
        list = clone;
    }

    zt_list_text_set(list, index_0, value);
    return list;
}

zt_int zt_list_text_len(const zt_list_text *list) {
    zt_runtime_require_list_text(list, "zt_list_text_len requires list");
    return (zt_int)list->len;
}

zt_list_text *zt_list_text_slice(const zt_list_text *list, zt_int start_0, zt_int end_0) {
    size_t start_pos;
    size_t end_pos;

    zt_runtime_require_list_text(list, "zt_list_text_slice requires list");

    if (start_0 < 0) {
        zt_runtime_error(ZT_ERR_INDEX, "slice start must be >= 0");
    }

    if (list->len == 0) {
        return zt_list_text_new();
    }

    start_pos = (size_t)start_0;
    end_pos = zt_normalize_slice_end(list->len, end_0);

    if (start_pos >= list->len || end_pos < start_pos) {
        return zt_list_text_new();
    }

    return zt_list_text_from_array(list->data + start_pos, end_pos - start_pos + 1);
}

zt_list_text *zt_list_text_deep_copy(const zt_list_text *list) {
    zt_list_text *copy;
    size_t index;

    zt_runtime_require_list_text(list, "zt_list_text_deep_copy requires list");

    copy = zt_list_text_new();
    if (list->len == 0) {
        return copy;
    }

    zt_list_text_reserve(copy, list->len);
    for (index = 0; index < list->len; index += 1) {
        copy->data[index] = zt_text_deep_copy(list->data[index]);
    }
    copy->len = list->len;

    return copy;
}


zt_map_text_text *zt_map_text_text_new(void) {
    return zt_map_text_text_alloc();
}

void zt_map_text_text_set(zt_map_text_text *map, zt_text *key, zt_text *value) {
    zt_bool found;
    size_t index;

    zt_runtime_require_map_text_text(map, "zt_map_text_text_set requires map");
    zt_runtime_require_text(key, "zt_map_text_text_set requires key");
    zt_runtime_require_text(value, "zt_map_text_text_set requires value");

    index = zt_map_text_text_find_index(map, key, &found);
    if (found) {
        zt_retain(value);
        zt_release(map->values[index]);
        map->values[index] = value;
        return;
    }

    zt_map_text_text_reserve(map, map->len + 1);
    zt_retain(key);
    zt_retain(value);
    map->keys[map->len] = key;
    map->values[map->len] = value;
    map->len += 1;
}

zt_map_text_text *zt_map_text_text_set_owned(zt_map_text_text *map, zt_text *key, zt_text *value) {
    zt_runtime_require_map_text_text(map, "zt_map_text_text_set_owned requires map");

    if (map->header.rc > 1u) {
        zt_map_text_text *clone = zt_map_text_text_from_arrays(map->keys, map->values, map->len);
        zt_release(map);
        map = clone;
    }

    zt_map_text_text_set(map, key, value);
    return map;
}

zt_map_text_text *zt_map_text_text_from_arrays(zt_text *const *keys, zt_text *const *values, size_t count) {
    zt_map_text_text *map;
    size_t index;

    map = zt_map_text_text_new();
    if (count == 0) {
        return map;
    }

    if (keys == NULL || values == NULL) {
        zt_runtime_error(ZT_ERR_PANIC, "zt_map_text_text_from_arrays requires key/value arrays");
    }

    for (index = 0; index < count; index += 1) {
        zt_map_text_text_set(map, keys[index], values[index]);
    }

    return map;
}

zt_text *zt_map_text_text_get(const zt_map_text_text *map, const zt_text *key) {
    zt_bool found;
    size_t index;
    zt_text *value;

    zt_runtime_require_map_text_text(map, "zt_map_text_text_get requires map");
    zt_runtime_require_text(key, "zt_map_text_text_get requires key");

    index = zt_map_text_text_find_index(map, key, &found);
    if (!found) {
        zt_runtime_error(ZT_ERR_INDEX, "map<text,text> key not found");
    }

    value = map->values[index];
    zt_runtime_require_text(value, "map<text,text> value cannot be null");
    zt_retain(value);
    return value;
}

zt_optional_text zt_map_text_text_get_optional(const zt_map_text_text *map, const zt_text *key) {
    zt_bool found;
    size_t index;
    zt_runtime_require_map_text_text(map, "zt_map_text_text_get_optional requires map");
    zt_runtime_require_text(key, "zt_map_text_text_get_optional requires key");

    index = zt_map_text_text_find_index(map, key, &found);
    if (!found) {
        return zt_optional_text_empty();
    }
    return zt_optional_text_present(map->values[index]);
}

zt_text *zt_map_text_text_key_at(const zt_map_text_text *map, zt_int index_0) {
    zt_text *key;

    zt_runtime_require_map_text_text(map, "zt_map_text_text_key_at requires map");
    if (index_0 < 0 || (size_t)index_0 >= map->len) {
        zt_runtime_error(ZT_ERR_INDEX, "map<text,text> iteration key index out of bounds");
    }

    key = map->keys[(size_t)index_0];
    zt_runtime_require_text(key, "map<text,text> key cannot be null");
    zt_retain(key);
    return key;
}

zt_text *zt_map_text_text_value_at(const zt_map_text_text *map, zt_int index_0) {
    zt_text *value;

    zt_runtime_require_map_text_text(map, "zt_map_text_text_value_at requires map");
    if (index_0 < 0 || (size_t)index_0 >= map->len) {
        zt_runtime_error(ZT_ERR_INDEX, "map<text,text> iteration value index out of bounds");
    }

    value = map->values[(size_t)index_0];
    zt_runtime_require_text(value, "map<text,text> value cannot be null");
    zt_retain(value);
    return value;
}

zt_int zt_map_text_text_len(const zt_map_text_text *map) {
    zt_runtime_require_map_text_text(map, "zt_map_text_text_len requires map");
    return (zt_int)map->len;
}

zt_optional_i64 zt_optional_i64_present(zt_int value) {
    zt_optional_i64 optional;

    optional.is_present = true;
    optional.value = value;
    return optional;
}

zt_optional_i64 zt_optional_i64_empty(void) {
    zt_optional_i64 optional;

    optional.is_present = false;
    optional.value = 0;
    return optional;
}

zt_bool zt_optional_i64_is_present(zt_optional_i64 value) {
    return value.is_present;
}

zt_int zt_optional_i64_coalesce(zt_optional_i64 value, zt_int fallback) {
    return value.is_present ? value.value : fallback;
}

zt_optional_text zt_optional_text_present(zt_text *value) {
    zt_optional_text optional;

    zt_runtime_require_text(value, "zt_optional_text_present requires text");

    
    optional.is_present = true;
    optional.value = value;
    zt_retain(value);
    return optional;
}

zt_optional_text zt_optional_text_empty(void) {
    zt_optional_text optional;

    
    optional.is_present = false;
    optional.value = NULL;
    return optional;
}

zt_bool zt_optional_text_is_present(zt_optional_text value) {
    return value.is_present;
}

zt_text *zt_optional_text_coalesce(zt_optional_text value, zt_text *fallback) {
    zt_text *selected;
    zt_runtime_require_text(fallback, "zt_optional_text_coalesce requires fallback text");

    selected = value.is_present ? value.value : fallback;
    zt_runtime_require_text(selected, "optional<text> selected value cannot be null");
    zt_retain(selected);
    return selected;
}

zt_optional_list_i64 zt_optional_list_i64_present(zt_list_i64 *value) {
    zt_optional_list_i64 optional;

    zt_runtime_require_list_i64(value, "zt_optional_list_i64_present requires list");

    
    optional.is_present = true;
    optional.value = value;
    zt_retain(value);
    return optional;
}

zt_optional_list_i64 zt_optional_list_i64_empty(void) {
    zt_optional_list_i64 optional;

    
    optional.is_present = false;
    optional.value = NULL;
    return optional;
}

zt_bool zt_optional_list_i64_is_present(zt_optional_list_i64 value) {
    return value.is_present;
}

zt_list_i64 *zt_optional_list_i64_coalesce(zt_optional_list_i64 value, zt_list_i64 *fallback) {
    zt_list_i64 *selected;
    zt_runtime_require_list_i64(fallback, "zt_optional_list_i64_coalesce requires fallback list");

    selected = value.is_present ? value.value : fallback;
    zt_runtime_require_list_i64(selected, "optional<list<int>> selected value cannot be null");
    zt_retain(selected);
    return selected;
}

zt_optional_list_text zt_optional_list_text_present(zt_list_text *value) {
    zt_optional_list_text optional;

    zt_runtime_require_list_text(value, "zt_optional_list_text_present requires list");

    
    optional.is_present = true;
    optional.value = value;
    zt_retain(value);
    return optional;
}

zt_optional_list_text zt_optional_list_text_empty(void) {
    zt_optional_list_text optional;

    
    optional.is_present = false;
    optional.value = NULL;
    return optional;
}

zt_bool zt_optional_list_text_is_present(zt_optional_list_text value) {
    return value.is_present;
}

zt_list_text *zt_optional_list_text_coalesce(zt_optional_list_text value, zt_list_text *fallback) {
    zt_list_text *selected;
    zt_runtime_require_list_text(fallback, "zt_optional_list_text_coalesce requires fallback list");

    selected = value.is_present ? value.value : fallback;
    zt_runtime_require_list_text(selected, "optional<list<text>> selected value cannot be null");
    zt_retain(selected);
    return selected;
}

zt_optional_map_text_text zt_optional_map_text_text_present(zt_map_text_text *value) {
    zt_optional_map_text_text optional;

    zt_runtime_require_map_text_text(value, "zt_optional_map_text_text_present requires map");

    
    optional.is_present = true;
    optional.value = value;
    zt_retain(value);
    return optional;
}

zt_optional_map_text_text zt_optional_map_text_text_empty(void) {
    zt_optional_map_text_text optional;

    
    optional.is_present = false;
    optional.value = NULL;
    return optional;
}

zt_bool zt_optional_map_text_text_is_present(zt_optional_map_text_text value) {
    return value.is_present;
}

zt_map_text_text *zt_optional_map_text_text_coalesce(zt_optional_map_text_text value, zt_map_text_text *fallback) {
    zt_map_text_text *selected;
    zt_runtime_require_map_text_text(fallback, "zt_optional_map_text_text_coalesce requires fallback map");

    selected = value.is_present ? value.value : fallback;
    zt_runtime_require_map_text_text(selected, "optional<map<text,text>> selected value cannot be null");
    zt_retain(selected);
    return selected;
}

zt_outcome_i64_text zt_outcome_i64_text_success(zt_int value) {
    zt_outcome_i64_text outcome;

    
    outcome.is_success = true;
    outcome.value = value;
    return outcome;
}

zt_outcome_i64_text zt_outcome_i64_text_failure(zt_text *error) {
    zt_outcome_i64_text outcome;

    zt_runtime_require_text(error, "zt_outcome_i64_text_failure requires error text");

    
    outcome.is_success = false;
    outcome.value = 0;
    outcome.error = error;
    zt_retain(error);
    return outcome;
}

zt_bool zt_outcome_i64_text_is_success(zt_outcome_i64_text outcome) {
    return outcome.is_success;
}

zt_int zt_outcome_i64_text_value(zt_outcome_i64_text outcome) {

    if (!outcome.is_success) {
        zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    }

    return outcome.value;
}

zt_outcome_i64_text zt_outcome_i64_text_propagate(zt_outcome_i64_text outcome) {
    return outcome;
}

zt_outcome_text_text zt_outcome_text_text_success(zt_text *value) {
    zt_outcome_text_text outcome;

    zt_runtime_require_text(value, "zt_outcome_text_text_success requires value text");

    
    outcome.is_success = true;
    outcome.value = value;
    outcome.error = NULL;
    zt_retain(value);
    return outcome;
}

zt_outcome_text_text zt_outcome_text_text_failure(zt_text *error) {
    zt_outcome_text_text outcome;

    zt_runtime_require_text(error, "zt_outcome_text_text_failure requires error text");

    
    outcome.is_success = false;
    outcome.value = NULL;
    outcome.error = error;
    zt_retain(error);
    return outcome;
}

zt_bool zt_outcome_text_text_is_success(zt_outcome_text_text outcome) {
    return outcome.is_success;
}

zt_text *zt_outcome_text_text_value(zt_outcome_text_text outcome) {

    if (!outcome.is_success) {
        zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    }

    zt_runtime_require_text(outcome.value, "outcome<text,text> success value cannot be null");
    zt_retain(outcome.value);
    return outcome.value;
}

zt_outcome_text_text zt_outcome_text_text_propagate(zt_outcome_text_text outcome) {
    return outcome;
}

zt_bool zt_outcome_text_text_eq(zt_outcome_text_text left, zt_outcome_text_text right) {
    if (left.is_success != right.is_success) {
        return false;
    }

    if (left.is_success) {
        if (left.value == NULL || right.value == NULL) {
            return left.value == right.value;
        }
        return zt_text_eq(left.value, right.value);
    }

    if (left.error == NULL || right.error == NULL) {
        return left.error == right.error;
    }

    return zt_text_eq(left.error, right.error);
}

zt_outcome_optional_text_text zt_outcome_optional_text_text_success(zt_optional_text value) {
    zt_outcome_optional_text_text outcome;

    outcome.is_success = true;
    outcome.value = value;
    outcome.error = NULL;
    if (value.is_present) {
        zt_runtime_require_text(value.value, "zt_outcome_optional_text_text_success requires present text");
        zt_retain(value.value);
    }
    return outcome;
}

zt_outcome_optional_text_text zt_outcome_optional_text_text_failure(zt_text *error) {
    zt_outcome_optional_text_text outcome;

    zt_runtime_require_text(error, "zt_outcome_optional_text_text_failure requires error text");

    outcome.is_success = false;
    outcome.value = zt_optional_text_empty();
    outcome.error = error;
    zt_retain(error);
    return outcome;
}

zt_bool zt_outcome_optional_text_text_is_success(zt_outcome_optional_text_text outcome) {
    return outcome.is_success;
}

zt_optional_text zt_outcome_optional_text_text_value(zt_outcome_optional_text_text outcome) {
    zt_optional_text value;

    if (!outcome.is_success) {
        zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    }

    value = outcome.value;
    if (value.is_present) {
        zt_runtime_require_text(value.value, "outcome<optional<text>,text> present value cannot be null");
        zt_retain(value.value);
    }
    return value;
}

zt_outcome_optional_text_text zt_outcome_optional_text_text_propagate(zt_outcome_optional_text_text outcome) {
    return outcome;
}
zt_outcome_list_i64_text zt_outcome_list_i64_text_success(zt_list_i64 *value) {
    zt_outcome_list_i64_text outcome;

    zt_runtime_require_list_i64(value, "zt_outcome_list_i64_text_success requires value list");

    
    outcome.is_success = true;
    outcome.value = value;
    outcome.error = NULL;
    zt_retain(value);
    return outcome;
}

zt_outcome_list_i64_text zt_outcome_list_i64_text_failure(zt_text *error) {
    zt_outcome_list_i64_text outcome;

    zt_runtime_require_text(error, "zt_outcome_list_i64_text_failure requires error text");

    
    outcome.is_success = false;
    outcome.value = NULL;
    outcome.error = error;
    zt_retain(error);
    return outcome;
}

zt_bool zt_outcome_list_i64_text_is_success(zt_outcome_list_i64_text outcome) {
    return outcome.is_success;
}

zt_list_i64 *zt_outcome_list_i64_text_value(zt_outcome_list_i64_text outcome) {

    if (!outcome.is_success) {
        zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    }

    zt_runtime_require_list_i64(outcome.value, "outcome<list<int>,text> success value cannot be null");
    zt_retain(outcome.value);
    return outcome.value;
}

zt_outcome_list_i64_text zt_outcome_list_i64_text_propagate(zt_outcome_list_i64_text outcome) {
    return outcome;
}

zt_outcome_list_text_text zt_outcome_list_text_text_success(zt_list_text *value) {
    zt_outcome_list_text_text outcome;

    zt_runtime_require_list_text(value, "zt_outcome_list_text_text_success requires value list");

    
    outcome.is_success = true;
    outcome.value = value;
    outcome.error = NULL;
    zt_retain(value);
    return outcome;
}

zt_outcome_list_text_text zt_outcome_list_text_text_failure(zt_text *error) {
    zt_outcome_list_text_text outcome;

    zt_runtime_require_text(error, "zt_outcome_list_text_text_failure requires error text");

    
    outcome.is_success = false;
    outcome.value = NULL;
    outcome.error = error;
    zt_retain(error);
    return outcome;
}

zt_bool zt_outcome_list_text_text_is_success(zt_outcome_list_text_text outcome) {
    return outcome.is_success;
}

zt_list_text *zt_outcome_list_text_text_value(zt_outcome_list_text_text outcome) {

    if (!outcome.is_success) {
        zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    }

    zt_runtime_require_list_text(outcome.value, "outcome<list<text>,text> success value cannot be null");
    zt_retain(outcome.value);
    return outcome.value;
}

zt_outcome_list_text_text zt_outcome_list_text_text_propagate(zt_outcome_list_text_text outcome) {
    return outcome;
}

zt_outcome_map_text_text zt_outcome_map_text_text_success(zt_map_text_text *value) {
    zt_outcome_map_text_text outcome;

    zt_runtime_require_map_text_text(value, "zt_outcome_map_text_text_success requires value map");

    
    outcome.is_success = true;
    outcome.value = value;
    outcome.error = NULL;
    zt_retain(value);
    return outcome;
}

zt_outcome_map_text_text zt_outcome_map_text_text_failure(zt_text *error) {
    zt_outcome_map_text_text outcome;

    zt_runtime_require_text(error, "zt_outcome_map_text_text_failure requires error text");

    
    outcome.is_success = false;
    outcome.value = NULL;
    outcome.error = error;
    zt_retain(error);
    return outcome;
}

zt_outcome_map_text_text zt_outcome_map_text_text_failure_message(const char *message) {
    zt_text *error = zt_text_from_utf8_literal(zt_safe_message(message));
    zt_outcome_map_text_text outcome = zt_outcome_map_text_text_failure(error);
    zt_release(error);
    return outcome;
}

zt_bool zt_outcome_map_text_text_is_success(zt_outcome_map_text_text outcome) {
    return outcome.is_success;
}

zt_map_text_text *zt_outcome_map_text_text_value(zt_outcome_map_text_text outcome) {

    if (!outcome.is_success) {
        zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    }

    zt_runtime_require_map_text_text(outcome.value, "outcome<map<text,text>,text> success value cannot be null");
    zt_retain(outcome.value);
    return outcome.value;
}

zt_outcome_map_text_text zt_outcome_map_text_text_propagate(zt_outcome_map_text_text outcome) {
    return outcome;
}

zt_outcome_void_text zt_outcome_void_text_success(void) {
    zt_outcome_void_text outcome;
    outcome.is_success = true;
    outcome.error = NULL;
    return outcome;
}

zt_outcome_void_text zt_outcome_void_text_failure(zt_text *error) {
    zt_outcome_void_text outcome;

    zt_runtime_require_text(error, "zt_outcome_void_text_failure requires error text");

    
    outcome.is_success = false;
    outcome.error = error;
    zt_retain(error);
    return outcome;
}

zt_bool zt_outcome_void_text_is_success(zt_outcome_void_text outcome) {
    return outcome.is_success;
}

zt_outcome_void_text zt_outcome_void_text_propagate(zt_outcome_void_text outcome) {
    return outcome;
}

// Initial ARC cycle detection scaffolding
void zt_detect_cycles(zt_header *header) {
    if (header == NULL || header->rc == 0) {
        return;
    }

    // Marcar o objeto como visitado
    header->rc |= 0x80000000; // Usar o bit mais significativo como marcador

    // Iterate internal references (example for lists and maps)
    if (header->kind == ZT_HEAP_LIST_I64) {
        zt_list_i64 *list = (zt_list_i64 *)header;
        for (size_t i = 0; i < list->len; i++) {
            zt_detect_cycles((zt_header *)list->data[i]);
        }
    } else if (header->kind == ZT_HEAP_MAP_TEXT_TEXT) {
        zt_map_text_text *map = (zt_map_text_text *)header;
        for (size_t i = 0; i < map->len; i++) {
            zt_detect_cycles((zt_header *)map->keys[i]);
            zt_detect_cycles((zt_header *)map->values[i]);
        }
    }

    // Clear visit mark after verification
    header->rc &= ~0x80000000;
}

void zt_runtime_check_for_cycles(void) {
    // Public function to inspect cycles on managed objects
    // Exemplo simplificado: iterar sobre todos os objetos conhecidos
    // Real implementation depends on a global managed-object registry
}

// Memory pool scaffolding for frequently-used types
#include <stdlib.h>

#define POOL_SIZE 128

typedef struct zt_pool {
    void *objects[POOL_SIZE];
    size_t count;
} zt_pool;

static zt_pool zt_text_pool = { .count = 0 };

zt_text *zt_text_pool_alloc(void) {
    if (zt_text_pool.count > 0) {
        return zt_text_pool.objects[--zt_text_pool.count];
    }
    return malloc(sizeof(zt_text));
}

void zt_text_pool_free(zt_text *text) {
    if (zt_text_pool.count < POOL_SIZE) {
        zt_text_pool.objects[zt_text_pool.count++] = text;
    } else {
        free(text);
    }
}

zt_bool zt_validate_pointer(const void *ptr) {
    return ptr != NULL;
}

void zt_runtime_safe_function_example(const zt_text *text) {
    if (!zt_validate_pointer(text)) {
        fprintf(stderr, "Erro: ponteiro nulo passado para zt_runtime_safe_function_example\n");
        return;
    }

    // ... function logic ...
}

// Add integrity checks to memory-release helpers
void zt_validate_and_free_text(zt_text *value) {
    if (value == NULL || value->data == NULL || value->len == 0) {
        fprintf(stderr, "Erro: tentativa de liberar zt_text invalido\n");
        return;
    }
    zt_free_text(value);
}

void zt_validate_and_free_list_i64(zt_list_i64 *list) {
    if (list == NULL || list->data == NULL || list->len > list->capacity) {
        fprintf(stderr, "Erro: tentativa de liberar zt_list_i64 invalido\n");
        return;
    }
    zt_free_list_i64(list);
}

void zt_validate_and_free_map_text_text(zt_map_text_text *map) {
    if (map == NULL || map->keys == NULL || map->values == NULL || map->len > map->capacity) {
        fprintf(stderr, "Erro: tentativa de liberar zt_map_text_text invalido\n");
        return;
    }
    zt_free_map_text_text(map);
}

static zt_outcome_text_text zt_host_default_read_file(const zt_text *path) {
    const char *path_data;
    FILE *file;
    long size_long;
    size_t size;
    char *buffer;
    size_t read_count;
    zt_text *value;
    zt_outcome_text_text outcome;

    zt_runtime_require_text(path, "zt_host_read_file requires path");
    path_data = zt_text_data(path);

    file = fopen(path_data, "rb");
    if (file == NULL) {
        return zt_outcome_text_text_failure_message(strerror(errno));
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return zt_outcome_text_text_failure_message(strerror(errno));
    }

    size_long = ftell(file);
    if (size_long < 0) {
        fclose(file);
        return zt_outcome_text_text_failure_message(strerror(errno));
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return zt_outcome_text_text_failure_message(strerror(errno));
    }

    size = (size_t)size_long;
    buffer = (char *)malloc(size + 1);
    if (buffer == NULL) {
        fclose(file);
        return zt_outcome_text_text_failure_message("failed to allocate file buffer");
    }

    read_count = 0;
    if (size > 0) {
        read_count = fread(buffer, 1, size, file);
        if (read_count != size) {
            free(buffer);
            fclose(file);
            return zt_outcome_text_text_failure_message(ferror(file) ? strerror(errno) : "failed to read full file");
        }
    }

    buffer[size] = '\0';
    fclose(file);

    value = zt_text_from_utf8(buffer, size);
    free(buffer);
    outcome = zt_outcome_text_text_success(value);
    zt_release(value);
    return outcome;
}

static zt_outcome_void_text zt_host_default_write_file(const zt_text *path, const zt_text *value) {
    const char *path_data;
    FILE *file;
    size_t write_count;

    zt_runtime_require_text(path, "zt_host_write_file requires path");
    zt_runtime_require_text(value, "zt_host_write_file requires value");

    path_data = zt_text_data(path);
    file = fopen(path_data, "wb");
    if (file == NULL) {
        return zt_outcome_void_text_failure_message(strerror(errno));
    }

    if (value->len > 0) {
        write_count = fwrite(value->data, 1, value->len, file);
        if (write_count != value->len) {
            fclose(file);
            return zt_outcome_void_text_failure_message(strerror(errno));
        }
    }

    if (fclose(file) != 0) {
        return zt_outcome_void_text_failure_message(strerror(errno));
    }

    return zt_outcome_void_text_success();
}

static zt_bool zt_host_default_path_exists(const zt_text *path) {
    struct stat info;
    const char *path_data;

    zt_runtime_require_text(path, "zt_host_path_exists requires path");
    path_data = zt_text_data(path);

    return stat(path_data, &info) == 0;
}

static zt_outcome_optional_text_text zt_host_default_read_line_stdin(void) {
    zt_outcome_optional_text_text outcome;
    size_t capacity = 128;
    size_t len = 0;
    char *buffer = (char *)malloc(capacity);
    int ch;

    if (buffer == NULL) {
        return zt_outcome_optional_text_text_failure_message("failed to allocate stdin line buffer");
    }

    while ((ch = fgetc(stdin)) != EOF) {
        if (ch == '\r') {
            int maybe_lf = fgetc(stdin);
            if (maybe_lf != '\n' && maybe_lf != EOF) {
                ungetc(maybe_lf, stdin);
            }
            break;
        }
        if (ch == '\n') {
            break;
        }

        if (len + 1 >= capacity) {
            size_t new_capacity = capacity * 2;
            char *new_buffer = (char *)realloc(buffer, new_capacity);
            if (new_buffer == NULL) {
                free(buffer);
                return zt_outcome_optional_text_text_failure_message("failed to grow stdin line buffer");
            }
            buffer = new_buffer;
            capacity = new_capacity;
        }

        buffer[len++] = (char)ch;
    }

    if (ferror(stdin)) {
        free(buffer);
        clearerr(stdin);
        return zt_outcome_optional_text_text_failure_message(strerror(errno));
    }

    if (ch == EOF && len == 0) {
        free(buffer);
        return zt_outcome_optional_text_text_success(zt_optional_text_empty());
    }

    {
        zt_text *line;
        zt_optional_text value;

        line = zt_text_from_utf8(buffer, len);
        free(buffer);

        value = zt_optional_text_present(line);
        zt_release(line);

        outcome = zt_outcome_optional_text_text_success(value);
        return outcome;
    }
}

static zt_outcome_text_text zt_host_default_read_all_stdin(void) {
    size_t capacity = 256;
    size_t len = 0;
    char *buffer = (char *)malloc(capacity);
    int ch;
    zt_text *value;
    zt_outcome_text_text outcome;

    if (buffer == NULL) {
        return zt_outcome_text_text_failure_message("failed to allocate stdin buffer");
    }

    while ((ch = fgetc(stdin)) != EOF) {
        if (len + 1 >= capacity) {
            size_t new_capacity = capacity * 2;
            char *new_buffer = (char *)realloc(buffer, new_capacity);
            if (new_buffer == NULL) {
                free(buffer);
                return zt_outcome_text_text_failure_message("failed to grow stdin buffer");
            }
            buffer = new_buffer;
            capacity = new_capacity;
        }
        buffer[len++] = (char)ch;
    }

    if (ferror(stdin)) {
        free(buffer);
        clearerr(stdin);
        return zt_outcome_text_text_failure_message(strerror(errno));
    }

    value = zt_text_from_utf8(buffer, len);
    free(buffer);
    outcome = zt_outcome_text_text_success(value);
    zt_release(value);
    return outcome;
}
static zt_outcome_void_text zt_host_default_write_stream(FILE *stream, const zt_text *value, const char *label) {
    size_t write_count;

    zt_runtime_require_text(value, label);

    if (value->len > 0) {
        write_count = fwrite(value->data, 1, value->len, stream);
        if (write_count != value->len) {
            return zt_outcome_void_text_failure_message(strerror(errno));
        }
    }

    if (fflush(stream) != 0) {
        return zt_outcome_void_text_failure_message(strerror(errno));
    }

    return zt_outcome_void_text_success();
}

static zt_outcome_void_text zt_host_default_write_stdout(const zt_text *value) {
    return zt_host_default_write_stream(stdout, value, "zt_host_write_stdout requires text");
}

static zt_outcome_void_text zt_host_default_write_stderr(const zt_text *value) {
    return zt_host_default_write_stream(stderr, value, "zt_host_write_stderr requires text");
}

static zt_int zt_host_default_time_now_unix_ms(void) {
    struct timespec ts;
    long long millis;

    if (timespec_get(&ts, TIME_UTC) == 0) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to read system time");
    }

    millis = (long long)ts.tv_sec * 1000LL + (long long)(ts.tv_nsec / 1000000L);
    return (zt_int)millis;
}

static zt_outcome_void_text zt_host_default_time_sleep_ms(zt_int duration_ms) {
    if (duration_ms < 0) {
        return zt_outcome_void_text_failure_message("time.sleep requires non-negative duration");
    }

#ifdef _WIN32
    Sleep((DWORD)duration_ms);
    return zt_outcome_void_text_success();
#else
    struct timespec req;
    struct timespec rem;

    req.tv_sec = (time_t)(duration_ms / 1000);
    req.tv_nsec = (long)((duration_ms % 1000) * 1000000L);

    while (nanosleep(&req, &rem) != 0) {
        if (errno != EINTR) {
            return zt_outcome_void_text_failure_message(strerror(errno));
        }
        req = rem;
    }

    return zt_outcome_void_text_success();
#endif
}

static uint64_t zt_host_random_state = UINT64_C(0x9e3779b97f4a7c15);

static uint64_t zt_host_random_next_u64(void) {
    uint64_t x = zt_host_random_state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    zt_host_random_state = x;
    return x * UINT64_C(2685821657736338717);
}

static void zt_host_default_random_seed(zt_int seed) {
    uint64_t state = (uint64_t)seed;
    if (state == 0) {
        state = UINT64_C(0x9e3779b97f4a7c15);
    }
    zt_host_random_state = state;
    (void)zt_host_random_next_u64();
}

static zt_int zt_host_default_random_next_i64(void) {
    uint64_t value = zt_host_random_next_u64();
    return (zt_int)(value & UINT64_C(0x7fffffffffffffff));
}

static char *zt_host_strdup_text(const zt_text *value, const char *label) {
    char *copy;
    if (value == NULL) {
        zt_runtime_error(ZT_ERR_PANIC, label);
    }
    copy = (char *)malloc(value->len + 1);
    if (copy == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate host string");
    }
    memcpy(copy, value->data, value->len);
    copy[value->len] = '\0';
    return copy;
}

static zt_outcome_text_text zt_host_default_os_current_dir(void) {
    size_t capacity = 256;
    char *buffer = NULL;

    while (1) {
        char *grown = (char *)realloc(buffer, capacity);
        if (grown == NULL) {
            free(buffer);
            return zt_outcome_text_text_failure_message("os.current_dir allocation failed");
        }
        buffer = grown;

#ifdef _WIN32
        if (_getcwd(buffer, (int)capacity) != NULL) {
            break;
        }
#else
        if (getcwd(buffer, capacity) != NULL) {
            break;
        }
#endif

        if (errno != ERANGE) {
            zt_outcome_text_text failure = zt_outcome_text_text_failure_message(strerror(errno));
            free(buffer);
            return failure;
        }

        if (capacity > (SIZE_MAX / 2)) {
            free(buffer);
            return zt_outcome_text_text_failure_message("os.current_dir path too long");
        }
        capacity *= 2;
    }

    {
        zt_text *text = zt_text_from_utf8_literal(buffer);
        zt_outcome_text_text outcome = zt_outcome_text_text_success(text);
        zt_release(text);
        free(buffer);
        return outcome;
    }
}

static zt_outcome_void_text zt_host_default_os_change_dir(const zt_text *path) {
    char *path_copy = zt_host_strdup_text(path, "os.change_dir requires path text");
    int rc;
#ifdef _WIN32
    rc = _chdir(path_copy);
#else
    rc = chdir(path_copy);
#endif
    free(path_copy);
    if (rc != 0) {
        return zt_outcome_void_text_failure_message(strerror(errno));
    }
    return zt_outcome_void_text_success();
}

static zt_optional_text zt_host_default_os_env(const zt_text *name) {
    zt_optional_text empty = zt_optional_text_empty();
    char *name_copy = zt_host_strdup_text(name, "os.env requires variable name text");
    const char *value = getenv(name_copy);
    free(name_copy);

    if (value == NULL) {
        return empty;
    }

    {
        zt_text *text_value = zt_text_from_utf8_literal(value);
        zt_optional_text result = zt_optional_text_present(text_value);
        zt_release(text_value);
        return result;
    }
}

static zt_int zt_host_default_os_pid(void) {
#ifdef _WIN32
    return (zt_int)_getpid();
#else
    return (zt_int)getpid();
#endif
}

static zt_text *zt_host_default_os_platform(void) {
#if defined(_WIN32)
    return zt_text_from_utf8_literal("windows");
#elif defined(__APPLE__)
    return zt_text_from_utf8_literal("macos");
#elif defined(__linux__)
    return zt_text_from_utf8_literal("linux");
#else
    return zt_text_from_utf8_literal("unknown");
#endif
}

static zt_text *zt_host_default_os_arch(void) {
#if defined(__x86_64__) || defined(_M_X64)
    return zt_text_from_utf8_literal("x64");
#elif defined(__i386__) || defined(_M_IX86)
    return zt_text_from_utf8_literal("x86");
#elif defined(__aarch64__) || defined(_M_ARM64)
    return zt_text_from_utf8_literal("arm64");
#elif defined(__arm__) || defined(_M_ARM)
    return zt_text_from_utf8_literal("arm");
#elif defined(__riscv) && __riscv_xlen == 64
    return zt_text_from_utf8_literal("riscv64");
#else
    return zt_text_from_utf8_literal("unknown");
#endif
}

static int zt_host_command_append(char **buffer, size_t *length, size_t *capacity, const char *text) {
    size_t text_len;
    size_t needed;
    char *grown;

    if (buffer == NULL || length == NULL || capacity == NULL || text == NULL) {
        return 0;
    }

    text_len = strlen(text);
    needed = *length + text_len + 1;
    if (needed > *capacity) {
        size_t next_capacity = (*capacity == 0) ? 128 : *capacity;
        while (next_capacity < needed) {
            if (next_capacity > (SIZE_MAX / 2)) return 0;
            next_capacity *= 2;
        }
        grown = (char *)realloc(*buffer, next_capacity);
        if (grown == NULL) return 0;
        *buffer = grown;
        *capacity = next_capacity;
    }

    memcpy((*buffer) + *length, text, text_len);
    *length += text_len;
    (*buffer)[*length] = '\0';
    return 1;
}

static int zt_host_command_append_quoted(char **buffer, size_t *length, size_t *capacity, const char *text) {
    const unsigned char *cursor = (const unsigned char *)text;

    if (!zt_host_command_append(buffer, length, capacity, "\"")) return 0;

    while (*cursor != 0) {
        char ch = (char)*cursor;
        if (ch == '\\' || ch == '"') {
            if (!zt_host_command_append(buffer, length, capacity, "\\")) return 0;
        }
        {
            char one[2];
            one[0] = ch;
            one[1] = '\0';
            if (!zt_host_command_append(buffer, length, capacity, one)) return 0;
        }
        cursor += 1;
    }

    return zt_host_command_append(buffer, length, capacity, "\"");
}

static zt_outcome_i64_text zt_host_default_process_run(const zt_text *program, const zt_list_text *args, zt_optional_text cwd) {
    char *command = NULL;
    size_t length = 0;
    size_t capacity = 0;
    int system_status;
    int exit_code;
    char *saved_cwd = NULL;
    zt_bool cwd_changed = false;

    if (program == NULL || program->len == 0) {
        return zt_outcome_i64_text_failure_message("process.run requires non-empty program");
    }

    if (args == NULL) {
        return zt_outcome_i64_text_failure_message("process.run requires args list");
    }

    if (!zt_host_command_append(&command, &length, &capacity, zt_text_data(program))) {
        free(command);
        return zt_outcome_i64_text_failure_message("process.run command allocation failed");
    }

    {
        size_t i;
        for (i = 0; i < args->len; i += 1) {
            zt_text *arg = args->data[i];
            if (arg == NULL) {
                free(command);
                return zt_outcome_i64_text_failure_message("process.run args cannot contain null text");
            }
            if (!zt_host_command_append(&command, &length, &capacity, " ")) {
                free(command);
                return zt_outcome_i64_text_failure_message("process.run command allocation failed");
            }
            if (!zt_host_command_append(&command, &length, &capacity, zt_text_data(arg))) {
                free(command);
                return zt_outcome_i64_text_failure_message("process.run command allocation failed");
            }
        }
    }

    if (cwd.is_present) {
        zt_outcome_text_text cwd_now = zt_host_default_os_current_dir();
        if (!cwd_now.is_success) {
            free(command);
            {
                zt_outcome_i64_text fail_outcome = zt_outcome_i64_text_failure(cwd_now.error);
                zt_release(cwd_now.error);
                return fail_outcome;
            }
        }
        saved_cwd = zt_host_strdup_text(cwd_now.value, "process.run failed to copy cwd");
        zt_release(cwd_now.value);

        if (cwd.value == NULL) {
            free(command);
            free(saved_cwd);
            return zt_outcome_i64_text_failure_message("process.run cwd present with null text");
        }

        {
            zt_outcome_void_text cd_outcome = zt_host_default_os_change_dir(cwd.value);
            if (!cd_outcome.is_success) {
                free(command);
                free(saved_cwd);
                {
                    zt_outcome_i64_text fail_outcome = zt_outcome_i64_text_failure(cd_outcome.error);
                    zt_release(cd_outcome.error);
                    return fail_outcome;
                }
            }
            cwd_changed = true;
        }
    }

    system_status = system(command);
    free(command);

    if (system_status == -1) {
        if (cwd_changed) {
            zt_text *saved = zt_text_from_utf8_literal(saved_cwd);
            zt_outcome_void_text restore_ignored = zt_host_default_os_change_dir(saved);
            if (!restore_ignored.is_success) {
                zt_release(restore_ignored.error);
            }
            zt_release(saved);
        }
        free(saved_cwd);
        return zt_outcome_i64_text_failure_message(strerror(errno));
    }

#ifdef _WIN32
    exit_code = system_status;
#else
    if (WIFEXITED(system_status)) {
        exit_code = WEXITSTATUS(system_status);
    } else if (WIFSIGNALED(system_status)) {
        exit_code = 128 + WTERMSIG(system_status);
    } else {
        exit_code = system_status;
    }
#endif

    if (cwd_changed) {
        zt_text *saved = zt_text_from_utf8_literal(saved_cwd);
        zt_outcome_void_text restore = zt_host_default_os_change_dir(saved);
        zt_release(saved);
        free(saved_cwd);
        if (!restore.is_success) {
            {
                zt_outcome_i64_text fail_outcome = zt_outcome_i64_text_failure(restore.error);
                zt_release(restore.error);
                return fail_outcome;
            }
        }
    }

    return zt_outcome_i64_text_success((zt_int)exit_code);
}

void zt_host_set_api(const zt_host_api *api) {
    if (api == NULL) {
        zt_host_api_state.read_file = zt_host_default_read_file;
        zt_host_api_state.write_file = zt_host_default_write_file;
        zt_host_api_state.path_exists = zt_host_default_path_exists;
        zt_host_api_state.read_line_stdin = zt_host_default_read_line_stdin;
        zt_host_api_state.read_all_stdin = zt_host_default_read_all_stdin;
        zt_host_api_state.write_stdout = zt_host_default_write_stdout;
        zt_host_api_state.write_stderr = zt_host_default_write_stderr;
        zt_host_api_state.time_now_unix_ms = zt_host_default_time_now_unix_ms;
        zt_host_api_state.time_sleep_ms = zt_host_default_time_sleep_ms;
        zt_host_api_state.random_seed = zt_host_default_random_seed;
        zt_host_api_state.random_next_i64 = zt_host_default_random_next_i64;
        zt_host_api_state.os_current_dir = zt_host_default_os_current_dir;
        zt_host_api_state.os_change_dir = zt_host_default_os_change_dir;
        zt_host_api_state.os_env = zt_host_default_os_env;
        zt_host_api_state.os_pid = zt_host_default_os_pid;
        zt_host_api_state.os_platform = zt_host_default_os_platform;
        zt_host_api_state.os_arch = zt_host_default_os_arch;
        zt_host_api_state.process_run = zt_host_default_process_run;
        return;
    }

    zt_host_api_state.read_file = api->read_file != NULL ? api->read_file : zt_host_default_read_file;
    zt_host_api_state.write_file = api->write_file != NULL ? api->write_file : zt_host_default_write_file;
    zt_host_api_state.path_exists = api->path_exists != NULL ? api->path_exists : zt_host_default_path_exists;
    zt_host_api_state.read_line_stdin = api->read_line_stdin != NULL ? api->read_line_stdin : zt_host_default_read_line_stdin;
    zt_host_api_state.read_all_stdin = api->read_all_stdin != NULL ? api->read_all_stdin : zt_host_default_read_all_stdin;
    zt_host_api_state.write_stdout = api->write_stdout != NULL ? api->write_stdout : zt_host_default_write_stdout;
    zt_host_api_state.write_stderr = api->write_stderr != NULL ? api->write_stderr : zt_host_default_write_stderr;
    zt_host_api_state.time_now_unix_ms = api->time_now_unix_ms != NULL ? api->time_now_unix_ms : zt_host_default_time_now_unix_ms;
    zt_host_api_state.time_sleep_ms = api->time_sleep_ms != NULL ? api->time_sleep_ms : zt_host_default_time_sleep_ms;
    zt_host_api_state.random_seed = api->random_seed != NULL ? api->random_seed : zt_host_default_random_seed;
    zt_host_api_state.random_next_i64 = api->random_next_i64 != NULL ? api->random_next_i64 : zt_host_default_random_next_i64;
    zt_host_api_state.os_current_dir = api->os_current_dir != NULL ? api->os_current_dir : zt_host_default_os_current_dir;
    zt_host_api_state.os_change_dir = api->os_change_dir != NULL ? api->os_change_dir : zt_host_default_os_change_dir;
    zt_host_api_state.os_env = api->os_env != NULL ? api->os_env : zt_host_default_os_env;
    zt_host_api_state.os_pid = api->os_pid != NULL ? api->os_pid : zt_host_default_os_pid;
    zt_host_api_state.os_platform = api->os_platform != NULL ? api->os_platform : zt_host_default_os_platform;
    zt_host_api_state.os_arch = api->os_arch != NULL ? api->os_arch : zt_host_default_os_arch;
    zt_host_api_state.process_run = api->process_run != NULL ? api->process_run : zt_host_default_process_run;
}

const zt_host_api *zt_host_get_api(void) {
    return &zt_host_api_state;
}

zt_outcome_text_text zt_host_read_file(const zt_text *path) {
    return zt_host_api_state.read_file(path);
}


zt_outcome_void_text zt_host_write_file(const zt_text *path, const zt_text *value) {
    return zt_host_api_state.write_file(path, value);
}

zt_bool zt_host_path_exists(const zt_text *path) {
    return zt_host_api_state.path_exists(path);
}
zt_outcome_optional_text_text zt_host_read_line_stdin(void) {
    return zt_host_api_state.read_line_stdin();
}

zt_outcome_text_text zt_host_read_all_stdin(void) {
    return zt_host_api_state.read_all_stdin();
}

zt_outcome_void_text zt_host_write_stdout(const zt_text *value) {
    return zt_host_api_state.write_stdout(value);
}

zt_outcome_void_text zt_host_write_stderr(const zt_text *value) {
    return zt_host_api_state.write_stderr(value);
}

zt_int zt_host_time_now_unix_ms(void) {
    return zt_host_api_state.time_now_unix_ms();
}

zt_outcome_void_text zt_host_time_sleep_ms(zt_int duration_ms) {
    return zt_host_api_state.time_sleep_ms(duration_ms);
}

void zt_host_random_seed(zt_int seed) {
    zt_host_api_state.random_seed(seed);
}

zt_int zt_host_random_next_i64(void) {
    return zt_host_api_state.random_next_i64();
}

zt_outcome_text_text zt_host_os_current_dir(void) {
    return zt_host_api_state.os_current_dir();
}

zt_outcome_void_text zt_host_os_change_dir(const zt_text *path) {
    return zt_host_api_state.os_change_dir(path);
}

zt_optional_text zt_host_os_env(const zt_text *name) {
    return zt_host_api_state.os_env(name);
}

zt_int zt_host_os_pid(void) {
    return zt_host_api_state.os_pid();
}

zt_text *zt_host_os_platform(void) {
    return zt_host_api_state.os_platform();
}

zt_text *zt_host_os_arch(void) {
    return zt_host_api_state.os_arch();
}

zt_outcome_i64_text zt_host_process_run(const zt_text *program, const zt_list_text *args, zt_optional_text cwd) {
    return zt_host_api_state.process_run(program, args, cwd);
}

static zt_outcome_map_text_text zt_outcome_map_text_text_failure_message_local(const char *message) {
    zt_text *error = zt_text_from_utf8_literal(zt_safe_message(message));
    zt_outcome_map_text_text outcome = zt_outcome_map_text_text_failure(error);
    zt_release(error);
    return outcome;
}

static const char *zt_json_skip_whitespace(const char *cursor, const char *end) {
    while (cursor < end && isspace((unsigned char)*cursor)) {
        cursor++;
    }
    return cursor;
}

static void zt_json_buffer_reserve(char **buffer, size_t *capacity, size_t current_len, size_t additional) {
    size_t required;
    size_t next_capacity;
    char *resized;

    if (buffer == NULL || capacity == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "invalid JSON buffer state");
    }

    required = current_len + additional + 1;
    if (*capacity >= required) {
        return;
    }

    next_capacity = *capacity > 0 ? *capacity : 32;
    while (next_capacity < required) {
        next_capacity *= 2;
    }

    resized = (char *)realloc(*buffer, next_capacity);
    if (resized == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate JSON buffer");
    }

    *buffer = resized;
    *capacity = next_capacity;
}

static void zt_json_buffer_append_char(char **buffer, size_t *len, size_t *capacity, char value) {
    zt_json_buffer_reserve(buffer, capacity, *len, 1);
    (*buffer)[*len] = value;
    *len += 1;
    (*buffer)[*len] = '\0';
}

static void zt_json_buffer_append_bytes(char **buffer, size_t *len, size_t *capacity, const char *value, size_t value_len) {
    if (value_len == 0) {
        return;
    }

    zt_json_buffer_reserve(buffer, capacity, *len, value_len);
    memcpy((*buffer) + *len, value, value_len);
    *len += value_len;
    (*buffer)[*len] = '\0';
}

static void zt_json_buffer_append_escaped_text(char **buffer, size_t *len, size_t *capacity, const zt_text *value) {
    static const char *hex = "0123456789abcdef";
    size_t index;

    zt_runtime_require_text(value, "zt_json_buffer_append_escaped_text requires text");

    for (index = 0; index < value->len; index++) {
        unsigned char ch = (unsigned char)value->data[index];
        char escaped[7] = "\\u0000";

        switch (ch) {
            case '"':
                zt_json_buffer_append_bytes(buffer, len, capacity, "\\\"", 2);
                break;
            case '\\':
                zt_json_buffer_append_bytes(buffer, len, capacity, "\\\\", 2);
                break;
            case '\b':
                zt_json_buffer_append_bytes(buffer, len, capacity, "\\b", 2);
                break;
            case '\f':
                zt_json_buffer_append_bytes(buffer, len, capacity, "\\f", 2);
                break;
            case '\n':
                zt_json_buffer_append_bytes(buffer, len, capacity, "\\n", 2);
                break;
            case '\r':
                zt_json_buffer_append_bytes(buffer, len, capacity, "\\r", 2);
                break;
            case '\t':
                zt_json_buffer_append_bytes(buffer, len, capacity, "\\t", 2);
                break;
            default:
                if (ch < 0x20) {
                    escaped[4] = hex[(ch >> 4) & 0x0f];
                    escaped[5] = hex[ch & 0x0f];
                    zt_json_buffer_append_bytes(buffer, len, capacity, escaped, 6);
                } else {
                    zt_json_buffer_append_char(buffer, len, capacity, (char)ch);
                }
                break;
        }
    }
}

static zt_bool zt_json_parse_string(const char **cursor, const char *end, zt_text **out_value, const char **out_error) {
    const char *it;
    char *buffer;
    size_t len;
    size_t capacity;

    if (cursor == NULL || out_value == NULL || out_error == NULL) {
        return false;
    }

    it = *cursor;
    if (it >= end || *it != '"') {
        *out_error = "JSON string must start with quote";
        return false;
    }

    it++;
    capacity = 32;
    len = 0;
    buffer = (char *)malloc(capacity);
    if (buffer == NULL) {
        *out_error = "failed to allocate JSON string buffer";
        return false;
    }
    buffer[0] = '\0';

    while (it < end) {
        unsigned char ch = (unsigned char)*it;
        it++;

        if (ch == '"') {
            *out_value = zt_text_from_utf8(buffer, len);
            free(buffer);
            *cursor = it;
            return true;
        }

        if (ch == '\\') {
            unsigned char esc;
            if (it >= end) {
                free(buffer);
                *out_error = "unterminated JSON escape sequence";
                return false;
            }

            esc = (unsigned char)*it;
            it++;

            switch (esc) {
                case '"': zt_json_buffer_append_char(&buffer, &len, &capacity, '"'); break;
                case '\\': zt_json_buffer_append_char(&buffer, &len, &capacity, '\\'); break;
                case '/': zt_json_buffer_append_char(&buffer, &len, &capacity, '/'); break;
                case 'b': zt_json_buffer_append_char(&buffer, &len, &capacity, '\b'); break;
                case 'f': zt_json_buffer_append_char(&buffer, &len, &capacity, '\f'); break;
                case 'n': zt_json_buffer_append_char(&buffer, &len, &capacity, '\n'); break;
                case 'r': zt_json_buffer_append_char(&buffer, &len, &capacity, '\r'); break;
                case 't': zt_json_buffer_append_char(&buffer, &len, &capacity, '\t'); break;
                case 'u':
                    free(buffer);
                    *out_error = "unicode escapes are not supported in std.json MVP";
                    return false;
                default:
                    free(buffer);
                    *out_error = "invalid JSON escape sequence";
                    return false;
            }

            continue;
        }

        if (ch < 0x20) {
            free(buffer);
            *out_error = "control character in JSON string";
            return false;
        }

        zt_json_buffer_append_char(&buffer, &len, &capacity, (char)ch);
    }

    free(buffer);
    *out_error = "unterminated JSON string";
    return false;
}

static zt_bool zt_json_parse_unquoted_value(const char **cursor, const char *end, zt_text **out_value, const char **out_error) {
    const char *start;
    const char *finish;
    const char *trimmed_start;
    const char *trimmed_end;

    if (cursor == NULL || out_value == NULL || out_error == NULL) {
        return false;
    }

    start = *cursor;
    finish = start;

    while (finish < end && *finish != ',' && *finish != '}') {
        finish++;
    }

    trimmed_start = start;
    trimmed_end = finish;

    while (trimmed_start < trimmed_end && isspace((unsigned char)*trimmed_start)) {
        trimmed_start++;
    }
    while (trimmed_end > trimmed_start && isspace((unsigned char)*(trimmed_end - 1))) {
        trimmed_end--;
    }

    if (trimmed_start == trimmed_end) {
        *out_error = "expected JSON value";
        return false;
    }

    if (*trimmed_start == '{' || *trimmed_start == '[') {
        *out_error = "nested JSON values are not supported in std.json MVP";
        return false;
    }

    *out_value = zt_text_from_utf8(trimmed_start, (size_t)(trimmed_end - trimmed_start));
    *cursor = finish;
    return true;
}

zt_outcome_map_text_text zt_json_parse_map_text_text(const zt_text *input) {
    const char *cursor;
    const char *end;
    zt_map_text_text *map;

    zt_runtime_require_text(input, "zt_json_parse_map_text_text requires input");

    cursor = input->data;
    end = input->data + input->len;
    cursor = zt_json_skip_whitespace(cursor, end);

    if (cursor >= end || *cursor != '{') {
        return zt_outcome_map_text_text_failure_message_local("std.json.parse expects a JSON object");
    }

    cursor++;
    map = zt_map_text_text_new();
    cursor = zt_json_skip_whitespace(cursor, end);

    if (cursor < end && *cursor == '}') {
        zt_outcome_map_text_text ok;
        cursor++;
        cursor = zt_json_skip_whitespace(cursor, end);
        if (cursor != end) {
            zt_release(map);
            return zt_outcome_map_text_text_failure_message_local("unexpected trailing content after JSON object");
        }
        ok = zt_outcome_map_text_text_success(map);
        zt_release(map);
        return ok;
    }

    while (cursor < end) {
        zt_text *key = NULL;
        zt_text *value_text = NULL;
        const char *error_message = NULL;

        cursor = zt_json_skip_whitespace(cursor, end);
        if (cursor >= end || *cursor != '"') {
            zt_release(map);
            return zt_outcome_map_text_text_failure_message_local("expected quoted JSON object key");
        }

        if (!zt_json_parse_string(&cursor, end, &key, &error_message)) {
            zt_release(map);
            return zt_outcome_map_text_text_failure_message_local(error_message);
        }

        cursor = zt_json_skip_whitespace(cursor, end);
        if (cursor >= end || *cursor != ':') {
            zt_release(key);
            zt_release(map);
            return zt_outcome_map_text_text_failure_message_local("expected ':' after JSON object key");
        }

        cursor++;
        cursor = zt_json_skip_whitespace(cursor, end);

        if (cursor < end && *cursor == '"') {
            if (!zt_json_parse_string(&cursor, end, &value_text, &error_message)) {
                zt_release(key);
                zt_release(map);
                return zt_outcome_map_text_text_failure_message_local(error_message);
            }
        } else {
            if (!zt_json_parse_unquoted_value(&cursor, end, &value_text, &error_message)) {
                zt_release(key);
                zt_release(map);
                return zt_outcome_map_text_text_failure_message_local(error_message);
            }
        }

        zt_map_text_text_set(map, key, value_text);
        zt_release(key);
        zt_release(value_text);

        cursor = zt_json_skip_whitespace(cursor, end);
        if (cursor < end && *cursor == ',') {
            cursor++;
            continue;
        }

        if (cursor < end && *cursor == '}') {
            zt_outcome_map_text_text ok;
            cursor++;
            cursor = zt_json_skip_whitespace(cursor, end);
            if (cursor != end) {
                zt_release(map);
                return zt_outcome_map_text_text_failure_message_local("unexpected trailing content after JSON object");
            }
            ok = zt_outcome_map_text_text_success(map);
            zt_release(map);
            return ok;
        }

        zt_release(map);
        return zt_outcome_map_text_text_failure_message_local("expected ',' or '}' in JSON object");
    }

    zt_release(map);
    return zt_outcome_map_text_text_failure_message_local("unterminated JSON object");
}

zt_text *zt_json_stringify_map_text_text(const zt_map_text_text *value) {
    char *buffer;
    size_t len;
    size_t capacity;
    zt_int count;
    zt_int i;

    zt_runtime_require_map_text_text(value, "zt_json_stringify_map_text_text requires map");

    capacity = 64;
    len = 0;
    buffer = (char *)malloc(capacity);
    if (buffer == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate JSON buffer");
    }
    buffer[0] = '\0';

    zt_json_buffer_append_char(&buffer, &len, &capacity, '{');

    count = zt_map_text_text_len(value);
    for (i = 0; i < count; i++) {
        zt_text *key = zt_map_text_text_key_at(value, i);
        zt_text *item_value = zt_map_text_text_value_at(value, i);

        if (i > 0) {
            zt_json_buffer_append_char(&buffer, &len, &capacity, ',');
        }

        zt_json_buffer_append_char(&buffer, &len, &capacity, '"');
        zt_json_buffer_append_escaped_text(&buffer, &len, &capacity, key);
        zt_json_buffer_append_char(&buffer, &len, &capacity, '"');
        zt_json_buffer_append_char(&buffer, &len, &capacity, ':');
        zt_json_buffer_append_char(&buffer, &len, &capacity, '"');
        zt_json_buffer_append_escaped_text(&buffer, &len, &capacity, item_value);
        zt_json_buffer_append_char(&buffer, &len, &capacity, '"');
    }

    zt_json_buffer_append_char(&buffer, &len, &capacity, '}');

    {
        zt_text *result = zt_text_from_utf8(buffer, len);
        free(buffer);
        return result;
    }
}

zt_text *zt_json_pretty_map_text_text(const zt_map_text_text *value, zt_int indent) {
    char *buffer;
    size_t len;
    size_t capacity;
    zt_int count;
    zt_int i;
    size_t indent_size;

    zt_runtime_require_map_text_text(value, "zt_json_pretty_map_text_text requires map");

    indent_size = indent > 0 ? (size_t)indent : 0;
    capacity = 64;
    len = 0;
    buffer = (char *)malloc(capacity);
    if (buffer == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate JSON buffer");
    }
    buffer[0] = '\0';

    count = zt_map_text_text_len(value);
    if (count == 0) {
        zt_json_buffer_append_char(&buffer, &len, &capacity, '{');
        zt_json_buffer_append_char(&buffer, &len, &capacity, '}');
    } else {
        zt_json_buffer_append_char(&buffer, &len, &capacity, '{');
        zt_json_buffer_append_char(&buffer, &len, &capacity, '\n');

        for (i = 0; i < count; i++) {
            zt_text *key = zt_map_text_text_key_at(value, i);
            zt_text *item_value = zt_map_text_text_value_at(value, i);
            size_t spacing;

            for (spacing = 0; spacing < indent_size; spacing++) {
                zt_json_buffer_append_char(&buffer, &len, &capacity, ' ');
            }

            zt_json_buffer_append_char(&buffer, &len, &capacity, '"');
            zt_json_buffer_append_escaped_text(&buffer, &len, &capacity, key);
            zt_json_buffer_append_char(&buffer, &len, &capacity, '"');
            zt_json_buffer_append_char(&buffer, &len, &capacity, ':');
            zt_json_buffer_append_char(&buffer, &len, &capacity, ' ');
            zt_json_buffer_append_char(&buffer, &len, &capacity, '"');
            zt_json_buffer_append_escaped_text(&buffer, &len, &capacity, item_value);
            zt_json_buffer_append_char(&buffer, &len, &capacity, '"');

            if (i + 1 < count) {
                zt_json_buffer_append_char(&buffer, &len, &capacity, ',');
            }

            zt_json_buffer_append_char(&buffer, &len, &capacity, '\n');
        }

        zt_json_buffer_append_char(&buffer, &len, &capacity, '}');
    }

    {
        zt_text *result = zt_text_from_utf8(buffer, len);
        free(buffer);
        return result;
    }
}

static uint64_t zt_u64_magnitude(zt_int value) {
    if (value >= 0) {
        return (uint64_t)value;
    }
    return (uint64_t)(-(value + 1)) + 1u;
}

zt_text *zt_format_hex_i64(zt_int value) {
    char buffer[80];
    uint64_t magnitude = zt_u64_magnitude(value);

    if (value < 0) {
        snprintf(buffer, sizeof(buffer), "-%llx", (unsigned long long)magnitude);
    } else {
        snprintf(buffer, sizeof(buffer), "%llx", (unsigned long long)magnitude);
    }

    return zt_text_from_utf8_literal(buffer);
}

zt_text *zt_format_bin_i64(zt_int value) {
    char digits[65];
    size_t count = 0;
    uint64_t magnitude = zt_u64_magnitude(value);
    char buffer[96];

    if (magnitude == 0) {
        digits[count++] = '0';
    } else {
        while (magnitude > 0 && count < sizeof(digits)) {
            digits[count++] = (char)('0' + (magnitude & 1u));
            magnitude >>= 1u;
        }
    }

    if (value < 0) {
        size_t out = 0;
        buffer[out++] = '-';
        while (count > 0 && out + 1 < sizeof(buffer)) {
            buffer[out++] = digits[--count];
        }
        buffer[out] = '\0';
    } else {
        size_t out = 0;
        while (count > 0 && out + 1 < sizeof(buffer)) {
            buffer[out++] = digits[--count];
        }
        buffer[out] = '\0';
    }

    return zt_text_from_utf8_literal(buffer);
}

static zt_text *zt_format_bytes_impl(zt_int value, zt_float base, const char *const *units, size_t unit_count, zt_int decimals) {
    zt_float scaled = (zt_float)value;
    size_t unit_index = 0;
    zt_int clamped_decimals = decimals;
    char format_spec[16];
    char buffer[96];

    if (clamped_decimals < 0) {
        clamped_decimals = 0;
    }
    if (clamped_decimals > 6) {
        clamped_decimals = 6;
    }

    while ((scaled <= -base || scaled >= base) && (unit_index + 1) < unit_count) {
        scaled /= base;
        unit_index += 1;
    }

    snprintf(format_spec, sizeof(format_spec), "%%.%df %%s", (int)clamped_decimals);
    snprintf(buffer, sizeof(buffer), format_spec, (double)scaled, units[unit_index]);
    return zt_text_from_utf8_literal(buffer);
}

zt_text *zt_format_bytes_binary(zt_int value, zt_int decimals) {
    static const char *const units[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB"};
    return zt_format_bytes_impl(value, 1024.0, units, sizeof(units) / sizeof(units[0]), decimals);
}

zt_text *zt_format_bytes_decimal(zt_int value, zt_int decimals) {
    static const char *const units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    return zt_format_bytes_impl(value, 1000.0, units, sizeof(units) / sizeof(units[0]), decimals);
}
zt_int zt_add_i64(zt_int a, zt_int b) {
    zt_int result;
    if (__builtin_add_overflow(a, b, &result)) {
        zt_runtime_error(ZT_ERR_MATH, "arithmetic overflow");
    }
    return result;
}

zt_int zt_sub_i64(zt_int a, zt_int b) {
    zt_int result;
    if (__builtin_sub_overflow(a, b, &result)) {
        zt_runtime_error(ZT_ERR_MATH, "arithmetic overflow");
    }
    return result;
}

zt_int zt_mul_i64(zt_int a, zt_int b) {
    zt_int result;
    if (__builtin_mul_overflow(a, b, &result)) {
        zt_runtime_error(ZT_ERR_MATH, "arithmetic overflow");
    }
    return result;
}

zt_int zt_div_i64(zt_int a, zt_int b) {
    if (b == 0) {
        zt_runtime_error(ZT_ERR_MATH, "division by zero");
    }
    if (a == INT64_MIN && b == -1) {
        zt_runtime_error(ZT_ERR_MATH, "arithmetic overflow");
    }
    return a / b;
}

zt_int zt_rem_i64(zt_int a, zt_int b) {
    if (b == 0) {
        zt_runtime_error(ZT_ERR_MATH, "division by zero");
    }
    if (a == INT64_MIN && b == -1) {
        zt_runtime_error(ZT_ERR_MATH, "arithmetic overflow");
    }
    return a % b;
}


zt_bool zt_validate_between_i64(zt_int value, zt_int min, zt_int max) {
    return value >= min && value <= max;
}
