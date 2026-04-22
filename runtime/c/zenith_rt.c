#include "runtime/c/zenith_rt.h"
#include "runtime/c/zenith_rt_templates.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <direct.h>
#include <process.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#endif

#ifdef _WIN32
typedef SOCKET zt_socket_handle;
#define ZT_NET_INVALID_SOCKET INVALID_SOCKET
#else
typedef int zt_socket_handle;
#define ZT_NET_INVALID_SOCKET (-1)
#endif

static void zt_pqueue_i64_ensure_capacity(zt_pqueue_i64 *heap, size_t needed);
static void zt_pqueue_text_ensure_capacity(zt_pqueue_text *heap, size_t needed);
static void zt_btreemap_text_text_ensure_capacity(zt_btreemap_text_text *map, size_t needed);
static void zt_btreeset_text_ensure_capacity(zt_btreeset_text *set, size_t needed);

#define ZT_DYNAMIC_HEAP_BASE 1024u
#define ZT_DYNAMIC_HEAP_CAPACITY 512u

typedef struct zt_dynamic_heap_entry {
    uint32_t kind;
    zt_heap_free_fn free_fn;
    zt_heap_clone_fn clone_fn;
} zt_dynamic_heap_entry;

static zt_dynamic_heap_entry zt_dynamic_heaps[ZT_DYNAMIC_HEAP_CAPACITY];
static size_t zt_dynamic_heap_count = 0;

static const zt_dynamic_heap_entry *zt_find_dynamic_heap_entry(uint32_t kind) {
    size_t index;

    if (kind < ZT_DYNAMIC_HEAP_BASE) {
        return NULL;
    }

    index = (size_t)(kind - ZT_DYNAMIC_HEAP_BASE);
    if (index >= zt_dynamic_heap_count) {
        return NULL;
    }

    if (zt_dynamic_heaps[index].kind != kind) {
        return NULL;
    }

    return &zt_dynamic_heaps[index];
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
        case ZT_ERR_TEST_FAILED: return "test.fail";
        case ZT_ERR_TEST_SKIPPED: return "test.skip";
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
        case ZT_ERR_TEST_FAILED:
            return "Use check(...) or explicit conditions before calling test.fail(...).";
        case ZT_ERR_TEST_SKIPPED:
            return "Skip marks the current test as not executed.";
        default:
            return "Review runtime preconditions for this operation.";
    }
}

static void zt_runtime_print_error(const zt_runtime_error_info *error) {
    const char *stable_code = zt_runtime_stable_code(error->kind);
    const char *help = zt_runtime_default_help(error->kind);
    const char *level = "error";

    if (error->kind == ZT_ERR_TEST_FAILED) level = "fail";
    if (error->kind == ZT_ERR_TEST_SKIPPED) level = "skip";

    fprintf(stderr, "%s[%s]\n", level, stable_code);
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

zt_outcome_bytes_text zt_outcome_bytes_text_failure_message(const char *message) {
    zt_text *error = zt_text_from_utf8_literal(zt_safe_message(message));
    zt_outcome_bytes_text outcome = zt_outcome_bytes_text_failure(error);
    zt_release(error);
    return outcome;
}

zt_outcome_optional_bytes_text zt_outcome_optional_bytes_text_failure_message(const char *message) {
    zt_text *error = zt_text_from_utf8_literal(zt_safe_message(message));
    zt_outcome_optional_bytes_text outcome = zt_outcome_optional_bytes_text_failure(error);
    zt_release(error);
    return outcome;
}

zt_outcome_net_connection_text zt_outcome_net_connection_text_failure_message(const char *message) {
    zt_text *error = zt_text_from_utf8_literal(zt_safe_message(message));
    zt_outcome_net_connection_text outcome = zt_outcome_net_connection_text_failure(error);
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

static zt_outcome_text_core_error zt_host_default_read_file(const zt_text *path);
static zt_outcome_void_core_error zt_host_default_write_file(const zt_text *path, const zt_text *value);
static zt_bool zt_host_default_path_exists(const zt_text *path);
static zt_outcome_optional_text_core_error zt_host_default_read_line_stdin(void);
static zt_outcome_text_core_error zt_host_default_read_all_stdin(void);
static zt_outcome_void_core_error zt_host_default_write_stdout(const zt_text *value);
static zt_outcome_void_core_error zt_host_default_write_stderr(const zt_text *value);
static zt_int zt_host_default_time_now_unix_ms(void);
static zt_outcome_void_core_error zt_host_default_time_sleep_ms(zt_int duration_ms);
static void zt_host_default_random_seed(zt_int seed);
static zt_int zt_host_default_random_next_i64(void);
static zt_outcome_text_core_error zt_host_default_os_current_dir(void);
static zt_outcome_void_core_error zt_host_default_os_change_dir(const zt_text *path);
static zt_optional_text zt_host_default_os_env(const zt_text *name);
static zt_int zt_host_default_os_pid(void);
static zt_text *zt_host_default_os_platform(void);
static zt_text *zt_host_default_os_arch(void);
static zt_outcome_i64_core_error zt_host_default_process_run(const zt_text *program, const zt_list_text *args, zt_optional_text cwd);

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
        case ZT_ERR_TEST_FAILED:
            return "test_failed";
        case ZT_ERR_TEST_SKIPPED:
            return "test_skipped";
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

/* zt_free_list_i64 and zt_free_list_text: generated by ZT_DEFINE_LIST_IMPL */

static void zt_free_dyn_text_repr(zt_dyn_text_repr *value) {
    if (value == NULL) {
        return;
    }

    if (value->tag == (uint32_t)ZT_DYN_TEXT_REPR_TEXT && value->text_value != NULL) {
        zt_release(value->text_value);
        value->text_value = NULL;
    }

    free(value);
}

static void zt_free_list_dyn_text_repr(zt_list_dyn_text_repr *list) {
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
static void zt_net_close_socket_handle(intptr_t handle) {
    zt_socket_handle socket_value = (zt_socket_handle)handle;

    if (socket_value == ZT_NET_INVALID_SOCKET) {
        return;
    }

#ifdef _WIN32
    closesocket(socket_value);
#else
    close(socket_value);
#endif
}

static void zt_free_net_connection(zt_net_connection *connection) {
    if (connection == NULL) {
        return;
    }

    if (!connection->closed) {
        zt_net_close_socket_handle(connection->socket_handle);
        connection->closed = true;
    }

    connection->socket_handle = (intptr_t)ZT_NET_INVALID_SOCKET;
    free(connection);
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

static void zt_runtime_require_net_connection(const zt_net_connection *connection, const char *message) {
    if (connection == NULL) {
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

static void zt_runtime_require_dyn_text_repr(const zt_dyn_text_repr *value, const char *message) {
    if (value == NULL) {
        zt_runtime_error(ZT_ERR_PANIC, message);
    }
}

static void zt_runtime_require_list_dyn_text_repr(const zt_list_dyn_text_repr *list, const char *message) {
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

/* ── Monomorphization: list<int> and list<text> ────────────────────────────── */
ZT_DEFINE_LIST_IMPL(i64, zt_int, ZT_HEAP_LIST_I64, 0)
ZT_DEFINE_LIST_IMPL(text, zt_text *, ZT_HEAP_LIST_TEXT, 1)
ZT_DEFINE_LIST_IMPL(f64, zt_float, ZT_HEAP_LIST_F64, 0)
ZT_DEFINE_MAP_IMPL(
    text_text,
    zt_text *,
    zt_text *,
    zt_optional_text,
    ZT_HEAP_MAP_TEXT_TEXT,
    1,
    1,
    zt_text_eq,
    zt_optional_text_present,
    zt_optional_text_empty)

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

/* zt_list_i64_reserve and zt_list_text_reserve: generated by ZT_DEFINE_LIST_IMPL */

static void zt_list_dyn_text_repr_reserve(zt_list_dyn_text_repr *list, size_t min_capacity) {
    size_t new_capacity;
    zt_dyn_text_repr **new_data;

    zt_runtime_require_list_dyn_text_repr(list, "zt_list_dyn_text_repr_reserve requires list");

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

    new_data = (zt_dyn_text_repr **)realloc(list->data, new_capacity * sizeof(zt_dyn_text_repr *));
    if (new_data == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to grow list<dyn<TextRepresentable>> buffer");
    }

    list->data = new_data;
    list->capacity = new_capacity;
}




















static void zt_free_grid2d_i64(zt_grid2d_i64 *grid);
static void zt_free_grid2d_text(zt_grid2d_text *grid);
static void zt_free_pqueue_i64(zt_pqueue_i64 *heap);
static void zt_free_pqueue_text(zt_pqueue_text *heap);
static void zt_free_circbuf_i64(zt_circbuf_i64 *buf);
static void zt_free_circbuf_text(zt_circbuf_text *buf);
static void zt_free_btreemap_text_text(zt_btreemap_text_text *map);
static void zt_free_btreeset_text(zt_btreeset_text *set);
static void zt_free_grid3d_i64(zt_grid3d_i64 *grid);
static void zt_free_grid3d_text(zt_grid3d_text *grid);
static void zt_free_net_connection(zt_net_connection *connection);

uint32_t zt_register_dynamic_heap_kind(zt_heap_free_fn free_fn, zt_heap_clone_fn clone_fn) {
    size_t index;
    uint32_t kind;

    if (free_fn == NULL || clone_fn == NULL) {
        zt_runtime_error(ZT_ERR_PANIC, "dynamic heap kinds require free and clone callbacks");
    }

    for (index = 0; index < zt_dynamic_heap_count; index += 1) {
        if (zt_dynamic_heaps[index].free_fn == free_fn &&
                zt_dynamic_heaps[index].clone_fn == clone_fn) {
            return zt_dynamic_heaps[index].kind;
        }
    }

    if (zt_dynamic_heap_count >= ZT_DYNAMIC_HEAP_CAPACITY) {
        zt_runtime_error(ZT_ERR_PLATFORM, "dynamic heap kind registry exhausted");
    }

    kind = ZT_DYNAMIC_HEAP_BASE + (uint32_t)zt_dynamic_heap_count;
    zt_dynamic_heaps[zt_dynamic_heap_count].kind = kind;
    zt_dynamic_heaps[zt_dynamic_heap_count].free_fn = free_fn;
    zt_dynamic_heaps[zt_dynamic_heap_count].clone_fn = clone_fn;
    zt_dynamic_heap_count += 1;
    return kind;
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
        case ZT_HEAP_LIST_F64:
            zt_free_list_f64((zt_list_f64 *)ref);
            return;
        case ZT_HEAP_DYN_TEXT_REPR:
            zt_free_dyn_text_repr((zt_dyn_text_repr *)ref);
            return;
        case ZT_HEAP_LIST_DYN_TEXT_REPR:
            zt_free_list_dyn_text_repr((zt_list_dyn_text_repr *)ref);
            return;
        case ZT_HEAP_MAP_TEXT_TEXT:
            zt_free_map_text_text((zt_map_text_text *)ref);
            return;
        case ZT_HEAP_GRID2D_I64:
            zt_free_grid2d_i64((zt_grid2d_i64 *)ref);
            return;
        case ZT_HEAP_GRID2D_TEXT:
            zt_free_grid2d_text((zt_grid2d_text *)ref);
            return;
        case ZT_HEAP_PQUEUE_I64:
            zt_free_pqueue_i64((zt_pqueue_i64 *)ref);
            return;
        case ZT_HEAP_PQUEUE_TEXT:
            zt_free_pqueue_text((zt_pqueue_text *)ref);
            return;
        case ZT_HEAP_CIRCBUF_I64:
            zt_free_circbuf_i64((zt_circbuf_i64 *)ref);
            return;
        case ZT_HEAP_CIRCBUF_TEXT:
            zt_free_circbuf_text((zt_circbuf_text *)ref);
            return;
        case ZT_HEAP_BTREEMAP_TEXT_TEXT:
            zt_free_btreemap_text_text((zt_btreemap_text_text *)ref);
            return;
        case ZT_HEAP_BTREESET_TEXT:
            zt_free_btreeset_text((zt_btreeset_text *)ref);
            return;
        case ZT_HEAP_GRID3D_I64:
            zt_free_grid3d_i64((zt_grid3d_i64 *)ref);
            return;
        case ZT_HEAP_GRID3D_TEXT:
            zt_free_grid3d_text((zt_grid3d_text *)ref);
            return;
        case ZT_HEAP_NET_CONNECTION:
            zt_free_net_connection((zt_net_connection *)ref);
            return;
        case ZT_HEAP_IMMORTAL_OUTCOME_VOID_TEXT:
            return;
        case ZT_HEAP_UNKNOWN:
        default:
            {
                const zt_dynamic_heap_entry *entry = zt_find_dynamic_heap_entry(header->kind);
                if (entry != NULL && entry->free_fn != NULL) {
                    entry->free_fn(ref);
                    return;
                }
                free(ref);
                return;
            }
    }
}

void *zt_deep_copy(void *ref) {
    zt_header *header;
    size_t i;

    if (ref == NULL) {
        return NULL;
    }

    header = zt_header_from_ref(ref);
    if (header->kind == (uint32_t)ZT_HEAP_IMMORTAL_OUTCOME_VOID_TEXT) {
        return ref;
    }

    switch ((zt_heap_kind)header->kind) {
        case ZT_HEAP_TEXT: {
            zt_text *t = (zt_text *)ref;
            return zt_text_from_utf8(t->data, t->len);
        }
        case ZT_HEAP_BYTES: {
            zt_bytes *b = (zt_bytes *)ref;
            return zt_bytes_from_array(b->data, b->len);
        }
        case ZT_HEAP_LIST_I64: {
            zt_list_i64 *l = (zt_list_i64 *)ref;
            return zt_list_i64_from_array(l->data, l->len);
        }
        case ZT_HEAP_LIST_F64: {
            zt_list_f64 *l = (zt_list_f64 *)ref;
            return zt_list_f64_from_array(l->data, l->len);
        }
        case ZT_HEAP_LIST_TEXT: {
            zt_list_text *l = (zt_list_text *)ref;
            zt_list_text *clone = zt_list_text_new();
            zt_list_text_reserve(clone, l->len);
            for (i = 0; i < l->len; i += 1) {
                clone->data[i] = (zt_text *)zt_deep_copy(l->data[i]);
            }
            clone->len = l->len;
            return clone;
        }
        case ZT_HEAP_MAP_TEXT_TEXT: {
            return zt_map_text_text_deep_copy((const zt_map_text_text *)ref);
        }
        case ZT_HEAP_GRID2D_I64: {
            zt_grid2d_i64 *g = (zt_grid2d_i64 *)ref;
            zt_grid2d_i64 *clone = zt_grid2d_i64_new((zt_int)g->rows, (zt_int)g->cols);
            memcpy(clone->data, g->data, g->len * sizeof(zt_int));
            return clone;
        }
        case ZT_HEAP_GRID2D_TEXT: {
            zt_grid2d_text *g = (zt_grid2d_text *)ref;
            zt_grid2d_text *clone = zt_grid2d_text_new((zt_int)g->rows, (zt_int)g->cols);
            for (i = 0; i < g->len; i += 1) {
                clone->data[i] = (zt_text *)zt_deep_copy(g->data[i]);
            }
            return clone;
        }
        case ZT_HEAP_GRID3D_I64: {
            zt_grid3d_i64 *g = (zt_grid3d_i64 *)ref;
            zt_grid3d_i64 *clone = zt_grid3d_i64_new((zt_int)g->depth, (zt_int)g->rows, (zt_int)g->cols);
            memcpy(clone->data, g->data, g->len * sizeof(zt_int));
            return clone;
        }
        case ZT_HEAP_GRID3D_TEXT: {
            zt_grid3d_text *g = (zt_grid3d_text *)ref;
            zt_grid3d_text *clone = zt_grid3d_text_new((zt_int)g->depth, (zt_int)g->rows, (zt_int)g->cols);
            for (i = 0; i < g->len; i += 1) {
                clone->data[i] = (zt_text *)zt_deep_copy(g->data[i]);
            }
            return clone;
        }
        case ZT_HEAP_PQUEUE_I64: {
            zt_pqueue_i64 *q = (zt_pqueue_i64 *)ref;
            zt_pqueue_i64 *clone = zt_pqueue_i64_new();
            zt_pqueue_i64_ensure_capacity(clone, q->len);
            memcpy(clone->data, q->data, q->len * sizeof(zt_int));
            clone->len = q->len;
            return clone;
        }
        case ZT_HEAP_PQUEUE_TEXT: {
            zt_pqueue_text *q = (zt_pqueue_text *)ref;
            zt_pqueue_text *clone = zt_pqueue_text_new();
            zt_pqueue_text_ensure_capacity(clone, q->len);
            for (i = 0; i < q->len; i += 1) {
                clone->data[i] = (zt_text *)zt_deep_copy(q->data[i]);
            }
            clone->len = q->len;
            return clone;
        }
        case ZT_HEAP_CIRCBUF_I64: {
            zt_circbuf_i64 *b = (zt_circbuf_i64 *)ref;
            zt_circbuf_i64 *clone = zt_circbuf_i64_new((zt_int)b->capacity);
            memcpy(clone->data, b->data, b->capacity * sizeof(zt_int));
            clone->head = b->head;
            clone->len = b->len;
            return clone;
        }
        case ZT_HEAP_CIRCBUF_TEXT: {
            zt_circbuf_text *b = (zt_circbuf_text *)ref;
            zt_circbuf_text *clone = zt_circbuf_text_new((zt_int)b->capacity);
            for (i = 0; i < b->capacity; i += 1) {
                clone->data[i] = (zt_text *)zt_deep_copy(b->data[i]);
            }
            clone->head = b->head;
            clone->len = b->len;
            return clone;
        }
        case ZT_HEAP_BTREEMAP_TEXT_TEXT: {
            zt_btreemap_text_text *m = (zt_btreemap_text_text *)ref;
            zt_btreemap_text_text *clone = zt_btreemap_text_text_new();
            zt_btreemap_text_text_ensure_capacity(clone, m->len);
            for (i = 0; i < m->len; i += 1) {
                clone->keys[i] = (zt_text *)zt_deep_copy(m->keys[i]);
                clone->values[i] = (zt_text *)zt_deep_copy(m->values[i]);
            }
            clone->len = m->len;
            return clone;
        }
        case ZT_HEAP_BTREESET_TEXT: {
            zt_btreeset_text *s = (zt_btreeset_text *)ref;
            zt_btreeset_text *clone = zt_btreeset_text_new();
            zt_btreeset_text_ensure_capacity(clone, s->len);
            for (i = 0; i < s->len; i += 1) {
                clone->data[i] = (zt_text *)zt_deep_copy(s->data[i]);
            }
            clone->len = s->len;
            return clone;
        }
        case ZT_HEAP_DYN_TEXT_REPR: {
            zt_dyn_text_repr *d = (zt_dyn_text_repr *)ref;
            zt_dyn_text_repr *clone = (zt_dyn_text_repr *)calloc(1, sizeof(zt_dyn_text_repr));
            clone->header.rc = 1;
            clone->header.kind = (uint32_t)ZT_HEAP_DYN_TEXT_REPR;
            clone->tag = d->tag;
            clone->int_value = d->int_value;
            clone->float_value = d->float_value;
            clone->bool_value = d->bool_value;
            clone->text_value = (zt_text *)zt_deep_copy(d->text_value);
            return clone;
        }
        case ZT_HEAP_LIST_DYN_TEXT_REPR: {
            zt_list_dyn_text_repr *l = (zt_list_dyn_text_repr *)ref;
            zt_list_dyn_text_repr *clone = (zt_list_dyn_text_repr *)calloc(1, sizeof(zt_list_dyn_text_repr));
            clone->header.rc = 1;
            clone->header.kind = (uint32_t)ZT_HEAP_LIST_DYN_TEXT_REPR;
            clone->len = l->len;
            clone->capacity = l->len;
            clone->data = (zt_dyn_text_repr **)calloc(l->len, sizeof(zt_dyn_text_repr *));
            for (i = 0; i < l->len; i += 1) {
                clone->data[i] = (zt_dyn_text_repr *)zt_deep_copy(l->data[i]);
            }
            return clone;
        }
        case ZT_HEAP_UNKNOWN:
        default:
            {
                const zt_dynamic_heap_entry *entry = zt_find_dynamic_heap_entry(header->kind);
                if (entry != NULL && entry->clone_fn != NULL) {
                    return entry->clone_fn(ref);
                }
                return NULL;
            }
    }
}

typedef struct zt_shared_ops {
    void *(*snapshot)(const void *value);
} zt_shared_ops;

typedef struct zt_shared_handle {
    atomic_uint rc;
    void *value;
    const zt_shared_ops *ops;
} zt_shared_handle;

struct zt_shared_text {
    zt_shared_handle handle;
};

struct zt_shared_bytes {
    zt_shared_handle handle;
};

static void *zt_shared_text_snapshot_value(const void *value) {
    const zt_text *text = (const zt_text *)value;

    zt_runtime_require_text(text, "shared<text> snapshot requires text");
    return zt_text_from_utf8(zt_text_data(text), text->len);
}

static void *zt_shared_bytes_snapshot_value(const void *value) {
    const zt_bytes *bytes = (const zt_bytes *)value;

    zt_runtime_require_bytes(bytes, "shared<bytes> snapshot requires bytes");
    return zt_bytes_from_array(bytes->data, bytes->len);
}

static const zt_shared_ops zt_shared_text_ops = {
    zt_shared_text_snapshot_value
};

static const zt_shared_ops zt_shared_bytes_ops = {
    zt_shared_bytes_snapshot_value
};

static void zt_shared_handle_init(zt_shared_handle *handle, void *value, const zt_shared_ops *ops) {
    if (handle == NULL || value == NULL || ops == NULL || ops->snapshot == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "invalid shared handle initialization");
    }

    atomic_init(&handle->rc, 1u);
    zt_retain(value);
    handle->value = value;
    handle->ops = ops;
}

static void zt_shared_handle_retain(zt_shared_handle *handle) {
    uint32_t current;

    if (handle == NULL) {
        return;
    }

    current = atomic_load_explicit(&handle->rc, memory_order_relaxed);
    for (;;) {
        if (current == UINT32_MAX) {
            zt_runtime_error(ZT_ERR_PLATFORM, "shared reference count overflow");
        }
        if (atomic_compare_exchange_weak_explicit(
                &handle->rc,
                &current,
                current + 1,
                memory_order_relaxed,
                memory_order_relaxed)) {
            return;
        }
    }
}

static zt_bool zt_shared_handle_release(zt_shared_handle *handle) {
    uint32_t current;

    if (handle == NULL) {
        return false;
    }

    current = atomic_load_explicit(&handle->rc, memory_order_acquire);
    for (;;) {
        if (current == 0) {
            zt_runtime_error(ZT_ERR_PLATFORM, "release on shared handle with rc=0");
        }
        if (atomic_compare_exchange_weak_explicit(
                &handle->rc,
                &current,
                current - 1,
                memory_order_acq_rel,
                memory_order_acquire)) {
            return current == 1;
        }
    }
}

static const void *zt_shared_handle_borrow(const zt_shared_handle *handle, const char *message) {
    if (handle == NULL || handle->value == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, message);
    }

    return handle->value;
}

static void *zt_shared_handle_snapshot(const zt_shared_handle *handle, const char *message) {
    const void *value = zt_shared_handle_borrow(handle, message);
    return handle->ops->snapshot(value);
}

static uint32_t zt_shared_handle_ref_count(const zt_shared_handle *handle, const char *message) {
    if (handle == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, message);
    }

    return atomic_load_explicit(&handle->rc, memory_order_acquire);
}

zt_shared_text *zt_shared_text_new(zt_text *value) {
    zt_shared_text *shared;

    zt_runtime_require_text(value, "zt_shared_text_new requires text");
    shared = (zt_shared_text *)calloc(1, sizeof(zt_shared_text));
    if (shared == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate shared<text> box");
    }

    zt_shared_handle_init(&shared->handle, value, &zt_shared_text_ops);
    return shared;
}

zt_shared_text *zt_shared_text_retain(zt_shared_text *shared) {
    if (shared == NULL) {
        return NULL;
    }

    zt_shared_handle_retain(&shared->handle);
    return shared;
}

void zt_shared_text_release(zt_shared_text *shared) {
    if (shared == NULL) {
        return;
    }

    if (zt_shared_handle_release(&shared->handle)) {
        zt_release(shared->handle.value);
        free(shared);
    }
}

const zt_text *zt_shared_text_borrow(const zt_shared_text *shared) {
    return (const zt_text *)zt_shared_handle_borrow(
        shared != NULL ? &shared->handle : NULL,
        "zt_shared_text_borrow requires shared text");
}

zt_text *zt_shared_text_snapshot(const zt_shared_text *shared) {
    return (zt_text *)zt_shared_handle_snapshot(
        shared != NULL ? &shared->handle : NULL,
        "zt_shared_text_snapshot requires shared text");
}

uint32_t zt_shared_text_ref_count(const zt_shared_text *shared) {
    return zt_shared_handle_ref_count(
        shared != NULL ? &shared->handle : NULL,
        "zt_shared_text_ref_count requires shared text");
}

zt_shared_bytes *zt_shared_bytes_new(zt_bytes *value) {
    zt_shared_bytes *shared;

    zt_runtime_require_bytes(value, "zt_shared_bytes_new requires bytes");
    shared = (zt_shared_bytes *)calloc(1, sizeof(zt_shared_bytes));
    if (shared == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate shared<bytes> box");
    }

    zt_shared_handle_init(&shared->handle, value, &zt_shared_bytes_ops);
    return shared;
}

zt_shared_bytes *zt_shared_bytes_retain(zt_shared_bytes *shared) {
    if (shared == NULL) {
        return NULL;
    }

    zt_shared_handle_retain(&shared->handle);
    return shared;
}

void zt_shared_bytes_release(zt_shared_bytes *shared) {
    if (shared == NULL) {
        return;
    }

    if (zt_shared_handle_release(&shared->handle)) {
        zt_release(shared->handle.value);
        free(shared);
    }
}

const zt_bytes *zt_shared_bytes_borrow(const zt_shared_bytes *shared) {
    return (const zt_bytes *)zt_shared_handle_borrow(
        shared != NULL ? &shared->handle : NULL,
        "zt_shared_bytes_borrow requires shared bytes");
}

zt_bytes *zt_shared_bytes_snapshot(const zt_shared_bytes *shared) {
    return (zt_bytes *)zt_shared_handle_snapshot(
        shared != NULL ? &shared->handle : NULL,
        "zt_shared_bytes_snapshot requires shared bytes");
}

uint32_t zt_shared_bytes_ref_count(const zt_shared_bytes *shared) {
    return zt_shared_handle_ref_count(
        shared != NULL ? &shared->handle : NULL,
        "zt_shared_bytes_ref_count requires shared bytes");
}

void zt_runtime_report_error(zt_error_kind kind, const char *message, const char *code, zt_runtime_span span) {
    zt_runtime_store_error(kind, message, code, span);
}

static int zt_runtime_exit_code_for_kind(zt_error_kind kind) {
    switch (kind) {
        case ZT_ERR_TEST_FAILED:
            return ZT_EXIT_CODE_TEST_FAILED;
        case ZT_ERR_TEST_SKIPPED:
            return ZT_EXIT_CODE_TEST_SKIPPED;
        default:
            return ZT_EXIT_CODE_RUNTIME_ERROR;
    }
}

void zt_runtime_error_ex(zt_error_kind kind, const char *message, const char *code, zt_runtime_span span) {
    zt_runtime_report_error(kind, message, code, span);
    zt_runtime_print_error(&zt_last_error);
    exit(zt_runtime_exit_code_for_kind(kind));
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

void zt_test_fail(zt_text *message) {
    const char *raw = (message != NULL && message->data != NULL) ? message->data : "";
    const char *final_message = raw[0] != '\0' ? raw : "test failed";
    zt_runtime_error_ex(ZT_ERR_TEST_FAILED, final_message, "test.fail", zt_runtime_span_unknown());
}

void zt_test_skip(zt_text *reason) {
    const char *raw = (reason != NULL && reason->data != NULL) ? reason->data : "";
    const char *final_message = raw[0] != '\0' ? raw : "test skipped";
    zt_runtime_error_ex(ZT_ERR_TEST_SKIPPED, final_message, "test.skip", zt_runtime_span_unknown());
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
    if (left_len == 0) {
        zt_retain((void *)b);
        return (zt_text *)b;
    }
    if (right_len == 0) {
        zt_retain((void *)a);
        return (zt_text *)a;
    }

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

    if (start_pos == 0 && end_pos + 1 == value->len) {
        zt_retain((void *)value);
        return (zt_text *)value;
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

    if (a->data[0] != b->data[0]) {
        return false;
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

/* zt_list_i64: new, from_array, push, push_owned, get, set, set_owned, len, slice
 * generated by ZT_DEFINE_LIST_IMPL(i64, zt_int, ZT_HEAP_LIST_I64, 0) */

zt_optional_i64 zt_list_i64_get_optional(const zt_list_i64 *list, zt_int index_0) {
    zt_runtime_require_list_i64(list, "zt_list_i64_get_optional requires list");

    if (index_0 < 0 || (size_t)index_0 >= list->len) {
        return zt_optional_i64_empty();
    }
    return zt_optional_i64_present(list->data[index_0]);
}

/* zt_list_text: new, from_array, push, push_owned, get, set, set_owned, len, slice
 * generated by ZT_DEFINE_LIST_IMPL(text, zt_text *, ZT_HEAP_LIST_TEXT, 1) */


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

zt_list_i64 *zt_queue_i64_new(void) {
    return zt_list_i64_new();
}

zt_list_i64 *zt_queue_i64_enqueue(zt_list_i64 *queue, zt_int value) {
    return zt_list_i64_push_owned(queue, value);
}

zt_list_i64 *zt_queue_i64_enqueue_owned(zt_list_i64 *queue, zt_int value) {
    return zt_queue_i64_enqueue(queue, value);
}

zt_optional_i64 zt_queue_i64_dequeue(zt_list_i64 *queue) {
    zt_int value;

    zt_runtime_require_list_i64(queue, "zt_queue_i64_dequeue requires queue");
    if (queue->len == 0) {
        return zt_optional_i64_empty();
    }

    value = queue->data[0];
    if (queue->len > 1) {
        memmove(queue->data, queue->data + 1, (queue->len - 1) * sizeof(zt_int));
    }
    queue->len -= 1;
    return zt_optional_i64_present(value);
}

zt_optional_i64 zt_queue_i64_peek(const zt_list_i64 *queue) {
    zt_runtime_require_list_i64(queue, "zt_queue_i64_peek requires queue");
    if (queue->len == 0) {
        return zt_optional_i64_empty();
    }
    return zt_optional_i64_present(queue->data[0]);
}

zt_list_text *zt_queue_text_new(void) {
    return zt_list_text_new();
}

zt_list_text *zt_queue_text_enqueue(zt_list_text *queue, zt_text *value) {
    return zt_list_text_push_owned(queue, value);
}

zt_list_text *zt_queue_text_enqueue_owned(zt_list_text *queue, zt_text *value) {
    return zt_queue_text_enqueue(queue, value);
}

zt_optional_text zt_queue_text_dequeue(zt_list_text *queue) {
    zt_text *value;
    zt_optional_text opt;

    zt_runtime_require_list_text(queue, "zt_queue_text_dequeue requires queue");
    if (queue->len == 0) {
        return zt_optional_text_empty();
    }

    value = queue->data[0];
    if (queue->len > 1) {
        memmove(queue->data, queue->data + 1, (queue->len - 1) * sizeof(zt_text *));
    }
    queue->len -= 1;
    queue->data[queue->len] = NULL;

    zt_runtime_require_text(value, "queue<text> entry cannot be null");
    opt.is_present = true;
    opt.value = value;
    return opt;
}

zt_optional_text zt_queue_text_peek(const zt_list_text *queue) {
    zt_text *value;

    zt_runtime_require_list_text(queue, "zt_queue_text_peek requires queue");
    if (queue->len == 0) {
        return zt_optional_text_empty();
    }

    value = queue->data[0];
    zt_runtime_require_text(value, "queue<text> entry cannot be null");
    return zt_optional_text_present(value);
}

zt_list_i64 *zt_stack_i64_new(void) {
    return zt_list_i64_new();
}

zt_list_i64 *zt_stack_i64_push(zt_list_i64 *stack, zt_int value) {
    return zt_list_i64_push_owned(stack, value);
}

zt_list_i64 *zt_stack_i64_push_owned(zt_list_i64 *stack, zt_int value) {
    return zt_stack_i64_push(stack, value);
}

zt_optional_i64 zt_stack_i64_pop(zt_list_i64 *stack) {
    zt_int value;

    zt_runtime_require_list_i64(stack, "zt_stack_i64_pop requires stack");
    if (stack->len == 0) {
        return zt_optional_i64_empty();
    }

    value = stack->data[stack->len - 1];
    stack->len -= 1;
    return zt_optional_i64_present(value);
}

zt_optional_i64 zt_stack_i64_peek(const zt_list_i64 *stack) {
    zt_runtime_require_list_i64(stack, "zt_stack_i64_peek requires stack");
    if (stack->len == 0) {
        return zt_optional_i64_empty();
    }
    return zt_optional_i64_present(stack->data[stack->len - 1]);
}

zt_list_text *zt_stack_text_new(void) {
    return zt_list_text_new();
}

zt_list_text *zt_stack_text_push(zt_list_text *stack, zt_text *value) {
    return zt_list_text_push_owned(stack, value);
}

zt_list_text *zt_stack_text_push_owned(zt_list_text *stack, zt_text *value) {
    return zt_stack_text_push(stack, value);
}

zt_optional_text zt_stack_text_pop(zt_list_text *stack) {
    zt_text *value;
    zt_optional_text opt;

    zt_runtime_require_list_text(stack, "zt_stack_text_pop requires stack");
    if (stack->len == 0) {
        return zt_optional_text_empty();
    }

    value = stack->data[stack->len - 1];
    stack->len -= 1;
    stack->data[stack->len] = NULL;

    zt_runtime_require_text(value, "stack<text> entry cannot be null");
    opt.is_present = true;
    opt.value = value;
    return opt;
}

zt_optional_text zt_stack_text_peek(const zt_list_text *stack) {
    zt_text *value;

    zt_runtime_require_list_text(stack, "zt_stack_text_peek requires stack");
    if (stack->len == 0) {
        return zt_optional_text_empty();
    }

    value = stack->data[stack->len - 1];
    zt_runtime_require_text(value, "stack<text> entry cannot be null");
    return zt_optional_text_present(value);
}



static zt_dyn_text_repr *zt_dyn_text_repr_alloc(zt_dyn_text_repr_tag tag) {
    zt_dyn_text_repr *value = (zt_dyn_text_repr *)calloc(1, sizeof(zt_dyn_text_repr));
    if (value == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate dyn<TextRepresentable> box");
    }
    value->header.rc = 1;
    value->header.kind = (uint32_t)ZT_HEAP_DYN_TEXT_REPR;
    value->tag = (uint32_t)tag;
    return value;
}

zt_dyn_text_repr *zt_dyn_text_repr_from_i64(zt_int value) {
    zt_dyn_text_repr *boxed = zt_dyn_text_repr_alloc(ZT_DYN_TEXT_REPR_INT);
    boxed->int_value = value;
    return boxed;
}

zt_dyn_text_repr *zt_dyn_text_repr_from_float(zt_float value) {
    zt_dyn_text_repr *boxed = zt_dyn_text_repr_alloc(ZT_DYN_TEXT_REPR_FLOAT);
    boxed->float_value = value;
    return boxed;
}

zt_dyn_text_repr *zt_dyn_text_repr_from_bool(zt_bool value) {
    zt_dyn_text_repr *boxed = zt_dyn_text_repr_alloc(ZT_DYN_TEXT_REPR_BOOL);
    boxed->bool_value = value;
    return boxed;
}

zt_dyn_text_repr *zt_dyn_text_repr_from_text_owned(zt_text *value) {
    zt_dyn_text_repr *boxed;
    zt_runtime_require_text(value, "zt_dyn_text_repr_from_text_owned requires text");
    boxed = zt_dyn_text_repr_alloc(ZT_DYN_TEXT_REPR_TEXT);
    boxed->text_value = value;
    return boxed;
}

zt_dyn_text_repr *zt_dyn_text_repr_from_text(const zt_text *value) {
    zt_runtime_require_text(value, "zt_dyn_text_repr_from_text requires text");
    return zt_dyn_text_repr_from_text_owned(zt_text_deep_copy(value));
}

zt_dyn_text_repr *zt_dyn_text_repr_clone(const zt_dyn_text_repr *value) {
    zt_runtime_require_dyn_text_repr(value, "zt_dyn_text_repr_clone requires value");

    switch ((zt_dyn_text_repr_tag)value->tag) {
        case ZT_DYN_TEXT_REPR_INT:
            return zt_dyn_text_repr_from_i64(value->int_value);
        case ZT_DYN_TEXT_REPR_FLOAT:
            return zt_dyn_text_repr_from_float(value->float_value);
        case ZT_DYN_TEXT_REPR_BOOL:
            return zt_dyn_text_repr_from_bool(value->bool_value);
        case ZT_DYN_TEXT_REPR_TEXT:
            zt_runtime_require_text(value->text_value, "dyn<TextRepresentable> text payload cannot be null");
            return zt_dyn_text_repr_from_text(value->text_value);
        default:
            zt_runtime_error(ZT_ERR_PANIC, "unknown dyn<TextRepresentable> tag in clone");
            return NULL;
    }
}

zt_text *zt_dyn_text_repr_to_text(const zt_dyn_text_repr *value) {
    char buffer[96];

    zt_runtime_require_dyn_text_repr(value, "zt_dyn_text_repr_to_text requires value");

    switch ((zt_dyn_text_repr_tag)value->tag) {
        case ZT_DYN_TEXT_REPR_INT:
            snprintf(buffer, sizeof(buffer), "%lld", (long long)value->int_value);
            return zt_text_from_utf8_literal(buffer);
        case ZT_DYN_TEXT_REPR_FLOAT:
            snprintf(buffer, sizeof(buffer), "%.17g", (double)value->float_value);
            return zt_text_from_utf8_literal(buffer);
        case ZT_DYN_TEXT_REPR_BOOL:
            return zt_text_from_utf8_literal(value->bool_value ? "true" : "false");
        case ZT_DYN_TEXT_REPR_TEXT:
            zt_runtime_require_text(value->text_value, "dyn<TextRepresentable> text payload cannot be null");
            return zt_text_deep_copy(value->text_value);
        default:
            zt_runtime_error(ZT_ERR_PANIC, "unknown dyn<TextRepresentable> tag in to_text");
            return NULL;
    }
}

zt_int zt_dyn_text_repr_text_len(const zt_dyn_text_repr *value) {
    zt_text *text;
    zt_int length;

    zt_runtime_require_dyn_text_repr(value, "zt_dyn_text_repr_text_len requires value");
    text = zt_dyn_text_repr_to_text(value);
    length = zt_text_len(text);
    zt_release(text);
    return length;
}

zt_list_dyn_text_repr *zt_list_dyn_text_repr_new(void) {
    zt_list_dyn_text_repr *list = (zt_list_dyn_text_repr *)calloc(1, sizeof(zt_list_dyn_text_repr));
    if (list == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate list<dyn<TextRepresentable>> header");
    }

    list->header.rc = 1;
    list->header.kind = (uint32_t)ZT_HEAP_LIST_DYN_TEXT_REPR;
    return list;
}

zt_list_dyn_text_repr *zt_list_dyn_text_repr_from_array(const zt_dyn_text_repr *const *items, size_t count) {
    zt_list_dyn_text_repr *list;
    size_t index;

    list = zt_list_dyn_text_repr_new();
    if (count == 0) {
        return list;
    }

    if (items == NULL) {
        zt_runtime_error(ZT_ERR_PANIC, "zt_list_dyn_text_repr_from_array requires items");
    }

    zt_list_dyn_text_repr_reserve(list, count);
    for (index = 0; index < count; index += 1) {
        zt_runtime_require_dyn_text_repr(items[index], "zt_list_dyn_text_repr_from_array requires dyn items");
        list->data[index] = (zt_dyn_text_repr *)items[index];
        zt_retain(list->data[index]);
    }
    list->len = count;
    return list;
}

zt_list_dyn_text_repr *zt_list_dyn_text_repr_from_array_owned(zt_dyn_text_repr *const *items, size_t count) {
    zt_list_dyn_text_repr *list;
    size_t index;

    list = zt_list_dyn_text_repr_new();
    if (count == 0) {
        return list;
    }

    if (items == NULL) {
        zt_runtime_error(ZT_ERR_PANIC, "zt_list_dyn_text_repr_from_array_owned requires items");
    }

    zt_list_dyn_text_repr_reserve(list, count);
    for (index = 0; index < count; index += 1) {
        zt_runtime_require_dyn_text_repr(items[index], "zt_list_dyn_text_repr_from_array_owned requires dyn items");
        list->data[index] = items[index];
    }
    list->len = count;
    return list;
}

zt_dyn_text_repr *zt_list_dyn_text_repr_get(const zt_list_dyn_text_repr *list, zt_int index_0) {
    zt_dyn_text_repr *value;

    zt_runtime_require_list_dyn_text_repr(list, "zt_list_dyn_text_repr_get requires list");
    if (index_0 < 0 || (size_t)index_0 >= list->len) {
        zt_runtime_error(ZT_ERR_INDEX, "list<dyn<TextRepresentable>> index out of bounds");
    }

    value = list->data[(size_t)index_0];
    zt_runtime_require_dyn_text_repr(value, "list<dyn<TextRepresentable>> entry cannot be null");
    zt_retain(value);
    return value;
}

zt_int zt_list_dyn_text_repr_len(const zt_list_dyn_text_repr *list) {
    zt_runtime_require_list_dyn_text_repr(list, "zt_list_dyn_text_repr_len requires list");
    return (zt_int)list->len;
}

zt_list_dyn_text_repr *zt_list_dyn_text_repr_deep_copy(const zt_list_dyn_text_repr *list) {
    zt_list_dyn_text_repr *copy;
    size_t index;

    zt_runtime_require_list_dyn_text_repr(list, "zt_list_dyn_text_repr_deep_copy requires list");

    copy = zt_list_dyn_text_repr_new();
    if (list->len == 0) {
        return copy;
    }

    zt_list_dyn_text_repr_reserve(copy, list->len);
    for (index = 0; index < list->len; index += 1) {
        copy->data[index] = zt_dyn_text_repr_clone(list->data[index]);
    }
    copy->len = list->len;
    return copy;
}

zt_text *zt_thread_boundary_copy_text(const zt_text *value) {
    zt_runtime_require_text(value, "zt_thread_boundary_copy_text requires text");
    return zt_text_deep_copy(value);
}

zt_bytes *zt_thread_boundary_copy_bytes(const zt_bytes *value) {
    zt_runtime_require_bytes(value, "zt_thread_boundary_copy_bytes requires bytes");
    return zt_bytes_from_array(value->data, value->len);
}

zt_list_text *zt_thread_boundary_copy_list_text(const zt_list_text *list) {
    zt_runtime_require_list_text(list, "zt_thread_boundary_copy_list_text requires list");
    return zt_list_text_deep_copy(list);
}

zt_dyn_text_repr *zt_thread_boundary_copy_dyn_text_repr(const zt_dyn_text_repr *value) {
    zt_runtime_require_dyn_text_repr(value, "zt_thread_boundary_copy_dyn_text_repr requires value");
    return zt_dyn_text_repr_clone(value);
}

zt_list_dyn_text_repr *zt_thread_boundary_copy_list_dyn_text_repr(const zt_list_dyn_text_repr *list) {
    zt_runtime_require_list_dyn_text_repr(list, "zt_thread_boundary_copy_list_dyn_text_repr requires list");
    return zt_list_dyn_text_repr_deep_copy(list);
}
/* ── Monomorphization: optional<T> ──────────────────────────────────────────── */
ZT_DEFINE_OPTIONAL_IMPL(i64,          zt_int,           0)
ZT_DEFINE_OPTIONAL_IMPL(text,         zt_text *,        1)
ZT_DEFINE_OPTIONAL_IMPL(bytes,        zt_bytes *,       1)
ZT_DEFINE_OPTIONAL_IMPL(list_i64,     zt_list_i64 *,    1)
ZT_DEFINE_OPTIONAL_IMPL(list_text,    zt_list_text *,   1)
ZT_DEFINE_OPTIONAL_IMPL(map_text_text, zt_map_text_text *, 1)


zt_core_error zt_core_error_make(zt_text *code, zt_text *message, zt_optional_text context) {
    zt_core_error error;

    zt_runtime_require_text(code, "core.Error requires code text");
    zt_runtime_require_text(message, "core.Error requires message text");

    error.code = code;
    error.message = message;
    error.context = context;
    zt_retain(code);
    zt_retain(message);
    if (context.is_present) {
        zt_runtime_require_text(context.value, "core.Error context cannot be null");
        zt_retain(context.value);
    }
    return error;
}

zt_core_error zt_core_error_from_message(const char *code, const char *message) {
    zt_text *code_text = zt_text_from_utf8_literal(code != NULL ? code : "error");
    zt_text *message_text = zt_text_from_utf8_literal(zt_safe_message(message));
    zt_core_error error = zt_core_error_make(code_text, message_text, zt_optional_text_empty());
    zt_release(code_text);
    zt_release(message_text);
    return error;
}

zt_core_error zt_core_error_from_text(const char *code, zt_text *message) {
    zt_text *code_text;
    zt_core_error error;

    zt_runtime_require_text(message, "core.Error message cannot be null");
    code_text = zt_text_from_utf8_literal(code != NULL ? code : "error");
    error = zt_core_error_make(code_text, message, zt_optional_text_empty());
    zt_release(code_text);
    return error;
}

zt_core_error zt_core_error_clone(zt_core_error error) {
    zt_core_error copy;
    copy.code = error.code;
    copy.message = error.message;
    copy.context = error.context;
    if (copy.code != NULL) zt_retain(copy.code);
    if (copy.message != NULL) zt_retain(copy.message);
    if (copy.context.is_present && copy.context.value != NULL) zt_retain(copy.context.value);
    return copy;
}

void zt_core_error_dispose(zt_core_error *error) {
    if (error == NULL) return;
    if (error->code != NULL) zt_release(error->code);
    if (error->message != NULL) zt_release(error->message);
    if (error->context.is_present && error->context.value != NULL) zt_release(error->context.value);
    error->code = NULL;
    error->message = NULL;
    error->context = zt_optional_text_empty();
}

zt_text *zt_core_error_message_or_default(zt_core_error error) {
    if (error.message != NULL) {
        zt_retain(error.message);
        return error.message;
    }
    return zt_text_from_utf8_literal("error");
}

/* ── Monomorphization: outcome<V,text> ─────────────────────────────────────── */
ZT_DEFINE_OUTCOME_IMPL(i64_text,  zt_int,     zt_text *, 0)
ZT_DEFINE_OUTCOME_IMPL(text_text, zt_text *,  zt_text *, 1)
ZT_DEFINE_OUTCOME_IMPL(bytes_text, zt_bytes *, zt_text *, 1)


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

zt_outcome_optional_bytes_text zt_outcome_optional_bytes_text_success(zt_optional_bytes value) {
    zt_outcome_optional_bytes_text outcome;

    outcome.is_success = true;
    outcome.value = value;
    outcome.error = NULL;
    if (value.is_present) {
        zt_runtime_require_bytes(value.value, "zt_outcome_optional_bytes_text_success requires present bytes");
        zt_retain(value.value);
    }
    return outcome;
}

zt_outcome_optional_bytes_text zt_outcome_optional_bytes_text_failure(zt_text *error) {
    zt_outcome_optional_bytes_text outcome;

    zt_runtime_require_text(error, "zt_outcome_optional_bytes_text_failure requires error text");

    outcome.is_success = false;
    outcome.value = zt_optional_bytes_empty();
    outcome.error = error;
    zt_retain(error);
    return outcome;
}

zt_bool zt_outcome_optional_bytes_text_is_success(zt_outcome_optional_bytes_text outcome) {
    return outcome.is_success;
}

zt_optional_bytes zt_outcome_optional_bytes_text_value(zt_outcome_optional_bytes_text outcome) {
    zt_optional_bytes value;

    if (!outcome.is_success) {
        zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    }

    value = outcome.value;
    if (value.is_present) {
        zt_runtime_require_bytes(value.value, "outcome<optional<bytes>,text> present value cannot be null");
        zt_retain(value.value);
    }
    return value;
}

zt_outcome_optional_bytes_text zt_outcome_optional_bytes_text_propagate(zt_outcome_optional_bytes_text outcome) {
    return outcome;
}

ZT_DEFINE_OUTCOME_IMPL(net_connection_text, zt_net_connection *, zt_text *, 1)
ZT_DEFINE_OUTCOME_IMPL(list_i64_text,  zt_list_i64 *,  zt_text *, 1)
ZT_DEFINE_OUTCOME_IMPL(list_text_text, zt_list_text *, zt_text *, 1)


ZT_DEFINE_OUTCOME_IMPL(map_text_text, zt_map_text_text *, zt_text *, 1)

zt_outcome_map_text_text zt_outcome_map_text_text_failure_message(const char *message) {
    zt_text *error = zt_text_from_utf8_literal(zt_safe_message(message));
    zt_outcome_map_text_text outcome = zt_outcome_map_text_text_failure(error);
    zt_release(error);
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


zt_outcome_i64_core_error zt_outcome_i64_core_error_success(zt_int value) {
    zt_outcome_i64_core_error outcome;
    outcome.is_success = true;
    outcome.value = value;
    outcome.error = (zt_core_error){0};
    return outcome;
}

zt_outcome_i64_core_error zt_outcome_i64_core_error_failure(zt_core_error error) {
    zt_outcome_i64_core_error outcome;
    outcome.is_success = false;
    outcome.value = 0;
    outcome.error = error.message != NULL ? zt_core_error_clone(error) : zt_core_error_from_message("error", "error");
    return outcome;
}

zt_outcome_i64_core_error zt_outcome_i64_core_error_failure_message(const char *message) {
    zt_core_error error = zt_core_error_from_message("error", message);
    zt_outcome_i64_core_error outcome = zt_outcome_i64_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_outcome_i64_core_error zt_outcome_i64_core_error_failure_text(zt_text *message) {
    zt_core_error error = zt_core_error_from_text("error", message);
    zt_outcome_i64_core_error outcome = zt_outcome_i64_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_bool zt_outcome_i64_core_error_is_success(zt_outcome_i64_core_error outcome) {
    return outcome.is_success;
}

zt_int zt_outcome_i64_core_error_value(zt_outcome_i64_core_error outcome) {
    if (!outcome.is_success) zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    return outcome.value;
}

zt_outcome_i64_core_error zt_outcome_i64_core_error_propagate(zt_outcome_i64_core_error outcome) {
    if (outcome.is_success) return outcome;
    return zt_outcome_i64_core_error_failure(outcome.error);
}

void zt_outcome_i64_core_error_dispose(zt_outcome_i64_core_error *outcome) {
    if (outcome == NULL) return;
    if (!outcome->is_success) zt_core_error_dispose(&outcome->error);
    memset(outcome, 0, sizeof(*outcome));
}

zt_outcome_void_core_error zt_outcome_void_core_error_success(void) {
    zt_outcome_void_core_error outcome;
    outcome.is_success = true;
    outcome.error = (zt_core_error){0};
    return outcome;
}

zt_outcome_void_core_error zt_outcome_void_core_error_failure(zt_core_error error) {
    zt_outcome_void_core_error outcome;
    outcome.is_success = false;
    outcome.error = error.message != NULL ? zt_core_error_clone(error) : zt_core_error_from_message("error", "error");
    return outcome;
}

zt_outcome_void_core_error zt_outcome_void_core_error_failure_message(const char *message) {
    zt_core_error error = zt_core_error_from_message("error", message);
    zt_outcome_void_core_error outcome = zt_outcome_void_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_outcome_void_core_error zt_outcome_void_core_error_failure_text(zt_text *message) {
    zt_core_error error = zt_core_error_from_text("error", message);
    zt_outcome_void_core_error outcome = zt_outcome_void_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_bool zt_outcome_void_core_error_is_success(zt_outcome_void_core_error outcome) {
    return outcome.is_success;
}

zt_outcome_void_core_error zt_outcome_void_core_error_propagate(zt_outcome_void_core_error outcome) {
    if (outcome.is_success) return outcome;
    return zt_outcome_void_core_error_failure(outcome.error);
}

void zt_outcome_void_core_error_dispose(zt_outcome_void_core_error *outcome) {
    if (outcome == NULL) return;
    if (!outcome->is_success) zt_core_error_dispose(&outcome->error);
    memset(outcome, 0, sizeof(*outcome));
}

zt_outcome_text_core_error zt_outcome_text_core_error_success(zt_text *value) {
    zt_outcome_text_core_error outcome;
    zt_runtime_require_text(value, "zt_outcome_text_text_success requires value text");
    outcome.is_success = true;
    outcome.value = value;
    outcome.error = (zt_core_error){0};
    zt_retain(value);
    return outcome;
}

zt_outcome_text_core_error zt_outcome_text_core_error_failure(zt_core_error error) {
    zt_outcome_text_core_error outcome;
    outcome.is_success = false;
    outcome.value = NULL;
    outcome.error = error.message != NULL ? zt_core_error_clone(error) : zt_core_error_from_message("error", "error");
    return outcome;
}

zt_outcome_text_core_error zt_outcome_text_core_error_failure_message(const char *message) {
    zt_core_error error = zt_core_error_from_message("error", message);
    zt_outcome_text_core_error outcome = zt_outcome_text_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_outcome_text_core_error zt_outcome_text_core_error_failure_text(zt_text *message) {
    zt_core_error error = zt_core_error_from_text("error", message);
    zt_outcome_text_core_error outcome = zt_outcome_text_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_bool zt_outcome_text_core_error_is_success(zt_outcome_text_core_error outcome) {
    return outcome.is_success;
}

zt_text *zt_outcome_text_core_error_value(zt_outcome_text_core_error outcome) {
    if (!outcome.is_success) zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    zt_runtime_require_text(outcome.value, "outcome<text,core.Error> success value cannot be null");
    zt_retain(outcome.value);
    return outcome.value;
}

zt_outcome_text_core_error zt_outcome_text_core_error_propagate(zt_outcome_text_core_error outcome) {
    if (outcome.is_success) return zt_outcome_text_core_error_success(outcome.value);
    return zt_outcome_text_core_error_failure(outcome.error);
}

void zt_outcome_text_core_error_dispose(zt_outcome_text_core_error *outcome) {
    if (outcome == NULL) return;
    if (outcome->is_success) {
        if (outcome->value != NULL) zt_release(outcome->value);
    } else {
        zt_core_error_dispose(&outcome->error);
    }
    memset(outcome, 0, sizeof(*outcome));
}

zt_outcome_optional_text_core_error zt_outcome_optional_text_core_error_success(zt_optional_text value) {
    zt_outcome_optional_text_core_error outcome;
    outcome.is_success = true;
    outcome.value = value;
    outcome.error = (zt_core_error){0};
    if (value.is_present) {
        zt_runtime_require_text(value.value, "zt_outcome_optional_text_text_success requires present text");
        zt_retain(value.value);
    }
    return outcome;
}

zt_outcome_optional_text_core_error zt_outcome_optional_text_core_error_failure(zt_core_error error) {
    zt_outcome_optional_text_core_error outcome;
    outcome.is_success = false;
    outcome.value = zt_optional_text_empty();
    outcome.error = error.message != NULL ? zt_core_error_clone(error) : zt_core_error_from_message("error", "error");
    return outcome;
}

zt_outcome_optional_text_core_error zt_outcome_optional_text_core_error_failure_message(const char *message) {
    zt_core_error error = zt_core_error_from_message("error", message);
    zt_outcome_optional_text_core_error outcome = zt_outcome_optional_text_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_outcome_optional_text_core_error zt_outcome_optional_text_core_error_failure_text(zt_text *message) {
    zt_core_error error = zt_core_error_from_text("error", message);
    zt_outcome_optional_text_core_error outcome = zt_outcome_optional_text_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_bool zt_outcome_optional_text_core_error_is_success(zt_outcome_optional_text_core_error outcome) {
    return outcome.is_success;
}

zt_optional_text zt_outcome_optional_text_core_error_value(zt_outcome_optional_text_core_error outcome) {
    zt_optional_text value;
    if (!outcome.is_success) zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    value = outcome.value;
    if (value.is_present) {
        zt_runtime_require_text(value.value, "outcome<optional<text>,core.Error> present value cannot be null");
        zt_retain(value.value);
    }
    return value;
}

zt_outcome_optional_text_core_error zt_outcome_optional_text_core_error_propagate(zt_outcome_optional_text_core_error outcome) {
    if (outcome.is_success) return zt_outcome_optional_text_core_error_success(outcome.value);
    return zt_outcome_optional_text_core_error_failure(outcome.error);
}

void zt_outcome_optional_text_core_error_dispose(zt_outcome_optional_text_core_error *outcome) {
    if (outcome == NULL) return;
    if (outcome->is_success) {
        if (outcome->value.is_present && outcome->value.value != NULL) zt_release(outcome->value.value);
    } else {
        zt_core_error_dispose(&outcome->error);
    }
    memset(outcome, 0, sizeof(*outcome));
}

zt_outcome_optional_bytes_core_error zt_outcome_optional_bytes_core_error_success(zt_optional_bytes value) {
    zt_outcome_optional_bytes_core_error outcome;
    outcome.is_success = true;
    outcome.value = value;
    outcome.error = (zt_core_error){0};
    if (value.is_present) {
        zt_runtime_require_bytes(value.value, "zt_outcome_optional_bytes_core_error_success requires present bytes");
        zt_retain(value.value);
    }
    return outcome;
}

zt_outcome_optional_bytes_core_error zt_outcome_optional_bytes_core_error_failure(zt_core_error error) {
    zt_outcome_optional_bytes_core_error outcome;
    outcome.is_success = false;
    outcome.value = zt_optional_bytes_empty();
    outcome.error = error.message != NULL ? zt_core_error_clone(error) : zt_core_error_from_message("error", "error");
    return outcome;
}

zt_outcome_optional_bytes_core_error zt_outcome_optional_bytes_core_error_failure_message(const char *message) {
    zt_core_error error = zt_core_error_from_message("error", message);
    zt_outcome_optional_bytes_core_error outcome = zt_outcome_optional_bytes_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_outcome_optional_bytes_core_error zt_outcome_optional_bytes_core_error_failure_text(zt_text *message) {
    zt_core_error error = zt_core_error_from_text("error", message);
    zt_outcome_optional_bytes_core_error outcome = zt_outcome_optional_bytes_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_bool zt_outcome_optional_bytes_core_error_is_success(zt_outcome_optional_bytes_core_error outcome) {
    return outcome.is_success;
}

zt_optional_bytes zt_outcome_optional_bytes_core_error_value(zt_outcome_optional_bytes_core_error outcome) {
    zt_optional_bytes value;
    if (!outcome.is_success) zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    value = outcome.value;
    if (value.is_present) {
        zt_runtime_require_bytes(value.value, "outcome<optional<bytes>,core.Error> present value cannot be null");
        zt_retain(value.value);
    }
    return value;
}

zt_outcome_optional_bytes_core_error zt_outcome_optional_bytes_core_error_propagate(zt_outcome_optional_bytes_core_error outcome) {
    if (outcome.is_success) return zt_outcome_optional_bytes_core_error_success(outcome.value);
    return zt_outcome_optional_bytes_core_error_failure(outcome.error);
}

void zt_outcome_optional_bytes_core_error_dispose(zt_outcome_optional_bytes_core_error *outcome) {
    if (outcome == NULL) return;
    if (outcome->is_success) {
        if (outcome->value.is_present && outcome->value.value != NULL) zt_release(outcome->value.value);
    } else {
        zt_core_error_dispose(&outcome->error);
    }
    memset(outcome, 0, sizeof(*outcome));
}

zt_outcome_net_connection_core_error zt_outcome_net_connection_core_error_success(zt_net_connection *value) {
    zt_outcome_net_connection_core_error outcome;
    zt_runtime_require_net_connection(value, "zt_outcome_net_connection_core_error_success requires connection");
    memset(&outcome, 0, sizeof(outcome));
    outcome.is_success = true;
    outcome.value = value;
    zt_retain(value);
    return outcome;
}

zt_outcome_net_connection_core_error zt_outcome_net_connection_core_error_failure(zt_core_error error) {
    zt_outcome_net_connection_core_error outcome;
    memset(&outcome, 0, sizeof(outcome));
    outcome.is_success = false;
    outcome.value = NULL;
    outcome.error = error.message != NULL ? zt_core_error_clone(error) : zt_core_error_from_message("error", "error");
    return outcome;
}

zt_outcome_net_connection_core_error zt_outcome_net_connection_core_error_failure_message(const char *message) {
    zt_core_error error = zt_core_error_from_message("error", message);
    zt_outcome_net_connection_core_error outcome = zt_outcome_net_connection_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_outcome_net_connection_core_error zt_outcome_net_connection_core_error_failure_text(zt_text *message) {
    zt_core_error error = zt_core_error_from_text("error", message);
    zt_outcome_net_connection_core_error outcome = zt_outcome_net_connection_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_bool zt_outcome_net_connection_core_error_is_success(zt_outcome_net_connection_core_error outcome) {
    return outcome.is_success;
}

zt_net_connection *zt_outcome_net_connection_core_error_value(zt_outcome_net_connection_core_error outcome) {
    if (!outcome.is_success) zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    zt_runtime_require_net_connection(outcome.value, "outcome<net.Connection,core.Error> success value cannot be null");
    zt_retain(outcome.value);
    return outcome.value;
}

zt_outcome_net_connection_core_error zt_outcome_net_connection_core_error_propagate(zt_outcome_net_connection_core_error outcome) {
    if (outcome.is_success) return zt_outcome_net_connection_core_error_success(outcome.value);
    return zt_outcome_net_connection_core_error_failure(outcome.error);
}

void zt_outcome_net_connection_core_error_dispose(zt_outcome_net_connection_core_error *outcome) {
    if (outcome == NULL) return;
    if (outcome->is_success) {
        if (outcome->value != NULL) zt_release(outcome->value);
    } else {
        zt_core_error_dispose(&outcome->error);
    }
    memset(outcome, 0, sizeof(*outcome));
}
zt_outcome_list_i64_core_error zt_outcome_list_i64_core_error_success(zt_list_i64 *value) {
    zt_outcome_list_i64_core_error outcome;
    zt_runtime_require_list_i64(value, "zt_outcome_list_i64_core_error_success requires value list");
    memset(&outcome, 0, sizeof(outcome));
    outcome.is_success = true;
    outcome.value = value;
    zt_retain(value);
    return outcome;
}

zt_outcome_list_i64_core_error zt_outcome_list_i64_core_error_failure(zt_core_error error) {
    zt_outcome_list_i64_core_error outcome;
    memset(&outcome, 0, sizeof(outcome));
    outcome.is_success = false;
    outcome.error = error.message != NULL ? zt_core_error_clone(error) : zt_core_error_from_message("error", "error");
    return outcome;
}

zt_outcome_list_i64_core_error zt_outcome_list_i64_core_error_failure_message(const char *message) {
    zt_core_error error = zt_core_error_from_message("error", message);
    zt_outcome_list_i64_core_error outcome = zt_outcome_list_i64_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_outcome_list_i64_core_error zt_outcome_list_i64_core_error_failure_text(zt_text *message) {
    zt_core_error error = zt_core_error_from_text("error", message);
    zt_outcome_list_i64_core_error outcome = zt_outcome_list_i64_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_bool zt_outcome_list_i64_core_error_is_success(zt_outcome_list_i64_core_error outcome) {
    return outcome.is_success;
}

zt_list_i64 *zt_outcome_list_i64_core_error_value(zt_outcome_list_i64_core_error outcome) {
    if (!outcome.is_success) zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    zt_runtime_require_list_i64(outcome.value, "outcome<list<int>,core.Error> success value cannot be null");
    zt_retain(outcome.value);
    return outcome.value;
}

zt_outcome_list_i64_core_error zt_outcome_list_i64_core_error_propagate(zt_outcome_list_i64_core_error outcome) {
    if (outcome.is_success) return zt_outcome_list_i64_core_error_success(outcome.value);
    return zt_outcome_list_i64_core_error_failure(outcome.error);
}

void zt_outcome_list_i64_core_error_dispose(zt_outcome_list_i64_core_error *outcome) {
    if (outcome == NULL) return;
    if (outcome->is_success) {
        if (outcome->value != NULL) zt_release(outcome->value);
    } else {
        zt_core_error_dispose(&outcome->error);
    }
    memset(outcome, 0, sizeof(*outcome));
}

zt_outcome_map_text_text_core_error zt_outcome_map_text_text_core_error_success(zt_map_text_text *value) {
    zt_outcome_map_text_text_core_error outcome;
    zt_runtime_require_map_text_text(value, "zt_outcome_map_text_text_success requires value map");
    memset(&outcome, 0, sizeof(outcome));
    outcome.is_success = true;
    outcome.value = value;
    zt_retain(value);
    return outcome;
}

zt_outcome_map_text_text_core_error zt_outcome_map_text_text_core_error_failure(zt_core_error error) {
    zt_outcome_map_text_text_core_error outcome;
    memset(&outcome, 0, sizeof(outcome));
    outcome.is_success = false;
    outcome.error = error.message != NULL ? zt_core_error_clone(error) : zt_core_error_from_message("error", "error");
    return outcome;
}

zt_outcome_map_text_text_core_error zt_outcome_map_text_text_core_error_failure_message(const char *message) {
    zt_core_error error = zt_core_error_from_message("error", message);
    zt_outcome_map_text_text_core_error outcome = zt_outcome_map_text_text_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_outcome_map_text_text_core_error zt_outcome_map_text_text_core_error_failure_text(zt_text *message) {
    zt_core_error error = zt_core_error_from_text("error", message);
    zt_outcome_map_text_text_core_error outcome = zt_outcome_map_text_text_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_bool zt_outcome_map_text_text_core_error_is_success(zt_outcome_map_text_text_core_error outcome) {
    return outcome.is_success;
}

zt_map_text_text *zt_outcome_map_text_text_core_error_value(zt_outcome_map_text_text_core_error outcome) {
    if (!outcome.is_success) zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    zt_runtime_require_map_text_text(outcome.value, "outcome<map<text,text>,core.Error> success value cannot be null");
    zt_retain(outcome.value);
    return outcome.value;
}

zt_outcome_map_text_text_core_error zt_outcome_map_text_text_core_error_propagate(zt_outcome_map_text_text_core_error outcome) {
    if (outcome.is_success) return zt_outcome_map_text_text_core_error_success(outcome.value);
    return zt_outcome_map_text_text_core_error_failure(outcome.error);
}

void zt_outcome_map_text_text_core_error_dispose(zt_outcome_map_text_text_core_error *outcome) {
    if (outcome == NULL) return;
    if (outcome->is_success) {
        if (outcome->value != NULL) zt_release(outcome->value);
    } else {
        zt_core_error_dispose(&outcome->error);
    }
    memset(outcome, 0, sizeof(*outcome));
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

static zt_outcome_text_core_error zt_host_default_read_file(const zt_text *path) {
    const char *path_data;
    FILE *file;
    long size_long;
    size_t size;
    char *buffer;
    size_t read_count;
    zt_text *value;
    zt_outcome_text_core_error outcome;

    zt_runtime_require_text(path, "zt_host_read_file requires path");
    path_data = zt_text_data(path);

    file = fopen(path_data, "rb");
    if (file == NULL) {
        return zt_outcome_text_core_error_failure_message(strerror(errno));
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return zt_outcome_text_core_error_failure_message(strerror(errno));
    }

    size_long = ftell(file);
    if (size_long < 0) {
        fclose(file);
        return zt_outcome_text_core_error_failure_message(strerror(errno));
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return zt_outcome_text_core_error_failure_message(strerror(errno));
    }

    size = (size_t)size_long;
    buffer = (char *)malloc(size + 1);
    if (buffer == NULL) {
        fclose(file);
        return zt_outcome_text_core_error_failure_message("failed to allocate file buffer");
    }

    read_count = 0;
    if (size > 0) {
        read_count = fread(buffer, 1, size, file);
        if (read_count != size) {
            free(buffer);
            fclose(file);
            return zt_outcome_text_core_error_failure_message(ferror(file) ? strerror(errno) : "failed to read full file");
        }
    }

    buffer[size] = '\0';
    fclose(file);

    value = zt_text_from_utf8(buffer, size);
    free(buffer);
    outcome = zt_outcome_text_core_error_success(value);
    zt_release(value);
    return outcome;
}

static zt_outcome_void_core_error zt_host_default_write_file(const zt_text *path, const zt_text *value) {
    const char *path_data;
    FILE *file;
    size_t write_count;

    zt_runtime_require_text(path, "zt_host_write_file requires path");
    zt_runtime_require_text(value, "zt_host_write_file requires value");

    path_data = zt_text_data(path);
    file = fopen(path_data, "wb");
    if (file == NULL) {
        return zt_outcome_void_core_error_failure_message(strerror(errno));
    }

    if (value->len > 0) {
        write_count = fwrite(value->data, 1, value->len, file);
        if (write_count != value->len) {
            fclose(file);
            return zt_outcome_void_core_error_failure_message(strerror(errno));
        }
    }

    if (fclose(file) != 0) {
        return zt_outcome_void_core_error_failure_message(strerror(errno));
    }

    return zt_outcome_void_core_error_success();
}

static zt_bool zt_host_default_path_exists(const zt_text *path) {
    struct stat info;
    const char *path_data;

    zt_runtime_require_text(path, "zt_host_path_exists requires path");
    path_data = zt_text_data(path);

    return stat(path_data, &info) == 0;
}

static zt_outcome_optional_text_core_error zt_host_default_read_line_stdin(void) {
    zt_outcome_optional_text_core_error outcome;
    size_t capacity = 128;
    size_t len = 0;
    char *buffer = (char *)malloc(capacity);
    int ch;

    if (buffer == NULL) {
        return zt_outcome_optional_text_core_error_failure_message("failed to allocate stdin line buffer");
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
                return zt_outcome_optional_text_core_error_failure_message("failed to grow stdin line buffer");
            }
            buffer = new_buffer;
            capacity = new_capacity;
        }

        buffer[len++] = (char)ch;
    }

    if (ferror(stdin)) {
        free(buffer);
        clearerr(stdin);
        return zt_outcome_optional_text_core_error_failure_message(strerror(errno));
    }

    if (ch == EOF && len == 0) {
        free(buffer);
        return zt_outcome_optional_text_core_error_success(zt_optional_text_empty());
    }

    {
        zt_text *line;
        zt_optional_text value;

        line = zt_text_from_utf8(buffer, len);
        free(buffer);

        value = zt_optional_text_present(line);
        zt_release(line);

        outcome = zt_outcome_optional_text_core_error_success(value);
        return outcome;
    }
}

static zt_outcome_text_core_error zt_host_default_read_all_stdin(void) {
    size_t capacity = 256;
    size_t len = 0;
    char *buffer = (char *)malloc(capacity);
    int ch;
    zt_text *value;
    zt_outcome_text_core_error outcome;

    if (buffer == NULL) {
        return zt_outcome_text_core_error_failure_message("failed to allocate stdin buffer");
    }

    while ((ch = fgetc(stdin)) != EOF) {
        if (len + 1 >= capacity) {
            size_t new_capacity = capacity * 2;
            char *new_buffer = (char *)realloc(buffer, new_capacity);
            if (new_buffer == NULL) {
                free(buffer);
                return zt_outcome_text_core_error_failure_message("failed to grow stdin buffer");
            }
            buffer = new_buffer;
            capacity = new_capacity;
        }
        buffer[len++] = (char)ch;
    }

    if (ferror(stdin)) {
        free(buffer);
        clearerr(stdin);
        return zt_outcome_text_core_error_failure_message(strerror(errno));
    }

    value = zt_text_from_utf8(buffer, len);
    free(buffer);
    outcome = zt_outcome_text_core_error_success(value);
    zt_release(value);
    return outcome;
}
static zt_outcome_void_core_error zt_host_default_write_stream(FILE *stream, const zt_text *value, const char *label) {
    size_t write_count;

    zt_runtime_require_text(value, label);

    if (value->len > 0) {
        write_count = fwrite(value->data, 1, value->len, stream);
        if (write_count != value->len) {
            return zt_outcome_void_core_error_failure_message(strerror(errno));
        }
    }

    if (fflush(stream) != 0) {
        return zt_outcome_void_core_error_failure_message(strerror(errno));
    }

    return zt_outcome_void_core_error_success();
}

static zt_outcome_void_core_error zt_host_default_write_stdout(const zt_text *value) {
    return zt_host_default_write_stream(stdout, value, "zt_host_write_stdout requires text");
}

static zt_outcome_void_core_error zt_host_default_write_stderr(const zt_text *value) {
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

static zt_outcome_void_core_error zt_host_default_time_sleep_ms(zt_int duration_ms) {
    if (duration_ms < 0) {
        return zt_outcome_void_core_error_failure_message("time.sleep requires non-negative duration");
    }

#ifdef _WIN32
    Sleep((DWORD)duration_ms);
    return zt_outcome_void_core_error_success();
#else
    struct timespec req;
    struct timespec rem;

    req.tv_sec = (time_t)(duration_ms / 1000);
    req.tv_nsec = (long)((duration_ms % 1000) * 1000000L);

    while (nanosleep(&req, &rem) != 0) {
        if (errno != EINTR) {
            return zt_outcome_void_core_error_failure_message(strerror(errno));
        }
        req = rem;
    }

    return zt_outcome_void_core_error_success();
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

static zt_outcome_text_core_error zt_host_default_os_current_dir(void) {
    size_t capacity = 256;
    char *buffer = NULL;

    while (1) {
        char *grown = (char *)realloc(buffer, capacity);
        if (grown == NULL) {
            free(buffer);
            return zt_outcome_text_core_error_failure_message("os.current_dir allocation failed");
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
            zt_outcome_text_core_error failure = zt_outcome_text_core_error_failure_message(strerror(errno));
            free(buffer);
            return failure;
        }

        if (capacity > (SIZE_MAX / 2)) {
            free(buffer);
            return zt_outcome_text_core_error_failure_message("os.current_dir path too long");
        }
        capacity *= 2;
    }

    {
        zt_text *text = zt_text_from_utf8_literal(buffer);
        zt_outcome_text_core_error outcome = zt_outcome_text_core_error_success(text);
        zt_release(text);
        free(buffer);
        return outcome;
    }
}

static zt_outcome_void_core_error zt_host_default_os_change_dir(const zt_text *path) {
    char *path_copy = zt_host_strdup_text(path, "os.change_dir requires path text");
    int rc;
#ifdef _WIN32
    rc = _chdir(path_copy);
#else
    rc = chdir(path_copy);
#endif
    free(path_copy);
    if (rc != 0) {
        return zt_outcome_void_core_error_failure_message(strerror(errno));
    }
    return zt_outcome_void_core_error_success();
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

static zt_outcome_i64_core_error zt_host_default_process_run(const zt_text *program, const zt_list_text *args, zt_optional_text cwd) {
    char *command = NULL;
    size_t length = 0;
    size_t capacity = 0;
    int system_status;
    int exit_code;
    char *saved_cwd = NULL;
    zt_bool cwd_changed = false;

    if (program == NULL || program->len == 0) {
        return zt_outcome_i64_core_error_failure_message("process.run requires non-empty program");
    }

    if (args == NULL) {
        return zt_outcome_i64_core_error_failure_message("process.run requires args list");
    }

    if (!zt_host_command_append(&command, &length, &capacity, zt_text_data(program))) {
        free(command);
        return zt_outcome_i64_core_error_failure_message("process.run command allocation failed");
    }

    {
        size_t i;
        for (i = 0; i < args->len; i += 1) {
            zt_text *arg = args->data[i];
            if (arg == NULL) {
                free(command);
                return zt_outcome_i64_core_error_failure_message("process.run args cannot contain null text");
            }
            if (!zt_host_command_append(&command, &length, &capacity, " ")) {
                free(command);
                return zt_outcome_i64_core_error_failure_message("process.run command allocation failed");
            }
            if (!zt_host_command_append(&command, &length, &capacity, zt_text_data(arg))) {
                free(command);
                return zt_outcome_i64_core_error_failure_message("process.run command allocation failed");
            }
        }
    }

    if (cwd.is_present) {
        zt_outcome_text_core_error cwd_now = zt_host_default_os_current_dir();
        if (!cwd_now.is_success) {
            free(command);
            {
                zt_outcome_i64_core_error fail_outcome = zt_outcome_i64_core_error_failure(cwd_now.error);
                zt_core_error_dispose(&cwd_now.error);
                return fail_outcome;
            }
        }
        saved_cwd = zt_host_strdup_text(cwd_now.value, "process.run failed to copy cwd");
        zt_release(cwd_now.value);

        if (cwd.value == NULL) {
            free(command);
            free(saved_cwd);
            return zt_outcome_i64_core_error_failure_message("process.run cwd present with null text");
        }

        {
            zt_outcome_void_core_error cd_outcome = zt_host_default_os_change_dir(cwd.value);
            if (!cd_outcome.is_success) {
                free(command);
                free(saved_cwd);
                {
                    zt_outcome_i64_core_error fail_outcome = zt_outcome_i64_core_error_failure(cd_outcome.error);
                    zt_core_error_dispose(&cd_outcome.error);
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
            zt_outcome_void_core_error restore_ignored = zt_host_default_os_change_dir(saved);
            if (!restore_ignored.is_success) {
                zt_core_error_dispose(&restore_ignored.error);
            }
            zt_release(saved);
        }
        free(saved_cwd);
        return zt_outcome_i64_core_error_failure_message(strerror(errno));
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
        zt_outcome_void_core_error restore = zt_host_default_os_change_dir(saved);
        zt_release(saved);
        free(saved_cwd);
        if (!restore.is_success) {
            {
                zt_outcome_i64_core_error fail_outcome = zt_outcome_i64_core_error_failure(restore.error);
                zt_core_error_dispose(&restore.error);
                return fail_outcome;
            }
        }
    }

    return zt_outcome_i64_core_error_success((zt_int)exit_code);
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

zt_outcome_text_core_error zt_host_read_file(const zt_text *path) {
    return zt_host_api_state.read_file(path);
}


zt_outcome_void_core_error zt_host_write_file(const zt_text *path, const zt_text *value) {
    return zt_host_api_state.write_file(path, value);
}

zt_bool zt_host_path_exists(const zt_text *path) {
    return zt_host_api_state.path_exists(path);
}
zt_outcome_optional_text_core_error zt_host_read_line_stdin(void) {
    return zt_host_api_state.read_line_stdin();
}

zt_outcome_text_core_error zt_host_read_all_stdin(void) {
    return zt_host_api_state.read_all_stdin();
}

zt_outcome_void_core_error zt_host_write_stdout(const zt_text *value) {
    return zt_host_api_state.write_stdout(value);
}

zt_outcome_void_core_error zt_host_write_stderr(const zt_text *value) {
    return zt_host_api_state.write_stderr(value);
}

zt_int zt_host_time_now_unix_ms(void) {
    return zt_host_api_state.time_now_unix_ms();
}

zt_outcome_void_core_error zt_host_time_sleep_ms(zt_int duration_ms) {
    return zt_host_api_state.time_sleep_ms(duration_ms);
}

void zt_host_random_seed(zt_int seed) {
    zt_host_api_state.random_seed(seed);
}

zt_int zt_host_random_next_i64(void) {
    return zt_host_api_state.random_next_i64();
}

zt_outcome_text_core_error zt_host_os_current_dir(void) {
    return zt_host_api_state.os_current_dir();
}

zt_outcome_void_core_error zt_host_os_change_dir(const zt_text *path) {
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

zt_outcome_i64_core_error zt_host_process_run(const zt_text *program, const zt_list_text *args, zt_optional_text cwd) {
    return zt_host_api_state.process_run(program, args, cwd);
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

zt_outcome_map_text_text_core_error zt_json_parse_map_text_text(const zt_text *input) {
    const char *cursor;
    const char *end;
    zt_map_text_text *map;

    zt_runtime_require_text(input, "zt_json_parse_map_text_text requires input");

    cursor = input->data;
    end = input->data + input->len;
    cursor = zt_json_skip_whitespace(cursor, end);

    if (cursor >= end || *cursor != '{') {
        return zt_outcome_map_text_text_core_error_failure_message("std.json.parse expects a JSON object");
    }

    cursor++;
    map = zt_map_text_text_new();
    cursor = zt_json_skip_whitespace(cursor, end);

    if (cursor < end && *cursor == '}') {
        zt_outcome_map_text_text_core_error ok;
        cursor++;
        cursor = zt_json_skip_whitespace(cursor, end);
        if (cursor != end) {
            zt_release(map);
            return zt_outcome_map_text_text_core_error_failure_message("unexpected trailing content after JSON object");
        }
        ok = zt_outcome_map_text_text_core_error_success(map);
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
            return zt_outcome_map_text_text_core_error_failure_message("expected quoted JSON object key");
        }

        if (!zt_json_parse_string(&cursor, end, &key, &error_message)) {
            zt_release(map);
            return zt_outcome_map_text_text_core_error_failure_message(error_message);
        }

        cursor = zt_json_skip_whitespace(cursor, end);
        if (cursor >= end || *cursor != ':') {
            zt_release(key);
            zt_release(map);
            return zt_outcome_map_text_text_core_error_failure_message("expected ':' after JSON object key");
        }

        cursor++;
        cursor = zt_json_skip_whitespace(cursor, end);

        if (cursor < end && *cursor == '"') {
            if (!zt_json_parse_string(&cursor, end, &value_text, &error_message)) {
                zt_release(key);
                zt_release(map);
                return zt_outcome_map_text_text_core_error_failure_message(error_message);
            }
        } else {
            if (!zt_json_parse_unquoted_value(&cursor, end, &value_text, &error_message)) {
                zt_release(key);
                zt_release(map);
                return zt_outcome_map_text_text_core_error_failure_message(error_message);
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
            zt_outcome_map_text_text_core_error ok;
            cursor++;
            cursor = zt_json_skip_whitespace(cursor, end);
            if (cursor != end) {
                zt_release(map);
                return zt_outcome_map_text_text_core_error_failure_message("unexpected trailing content after JSON object");
            }
            ok = zt_outcome_map_text_text_core_error_success(map);
            zt_release(map);
            return ok;
        }

        zt_release(map);
        return zt_outcome_map_text_text_core_error_failure_message("expected ',' or '}' in JSON object");
    }

    zt_release(map);
    return zt_outcome_map_text_text_core_error_failure_message("unterminated JSON object");
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

static zt_bool zt_text_equals_literal(const zt_text *value, const char *literal) {
    size_t literal_len;
    if (value == NULL || literal == NULL) return false;
    literal_len = strlen(literal);
    if (value->len != literal_len) return false;
    return memcmp(value->data, literal, literal_len) == 0;
}

static zt_int zt_format_clamp_decimals(zt_int decimals) {
    if (decimals < 0) return 0;
    if (decimals > 9) return 9;
    return decimals;
}

static zt_bool zt_unix_ms_to_utc_tm(zt_int millis, struct tm *out_tm, int *out_ms_part) {
    zt_int seconds;
    zt_int ms_part;
    time_t epoch_seconds;

    if (out_tm == NULL) return false;

    seconds = millis / 1000;
    ms_part = millis % 1000;
    if (ms_part < 0) {
        ms_part += 1000;
        seconds -= 1;
    }

    epoch_seconds = (time_t)seconds;
    if ((zt_int)epoch_seconds != seconds) {
        return false;
    }

#ifdef _WIN32
    if (gmtime_s(out_tm, &epoch_seconds) != 0) return false;
#else
    if (gmtime_r(&epoch_seconds, out_tm) == NULL) return false;
#endif

    if (out_ms_part != NULL) {
        *out_ms_part = (int)ms_part;
    }
    return true;
}

zt_text *zt_format_number(zt_float value, zt_int decimals) {
    char format_spec[16];
    char buffer[128];
    zt_int clamped_decimals = zt_format_clamp_decimals(decimals);

    snprintf(format_spec, sizeof(format_spec), "%%.%df", (int)clamped_decimals);
    snprintf(buffer, sizeof(buffer), format_spec, (double)value);
    return zt_text_from_utf8_literal(buffer);
}

zt_text *zt_format_percent(zt_float value, zt_int decimals) {
    char format_spec[20];
    char buffer[128];
    zt_int clamped_decimals = zt_format_clamp_decimals(decimals);

    snprintf(format_spec, sizeof(format_spec), "%%.%df%%%%", (int)clamped_decimals);
    snprintf(buffer, sizeof(buffer), format_spec, (double)(value * 100.0));
    return zt_text_from_utf8_literal(buffer);
}

zt_text *zt_format_date(zt_int millis, const zt_text *style) {
    struct tm utc_tm;
    const char *pattern = "%Y-%m-%d";
    char buffer[128];

    if (zt_text_equals_literal(style, "long")) {
        pattern = "%A, %d %B %Y";
    } else if (zt_text_equals_literal(style, "short")) {
        pattern = "%Y-%m-%d";
    } else if (zt_text_equals_literal(style, "iso")) {
        pattern = "%Y-%m-%d";
    }

    if (!zt_unix_ms_to_utc_tm(millis, &utc_tm, NULL)) {
        return zt_text_from_utf8_literal("1970-01-01");
    }

    if (strftime(buffer, sizeof(buffer), pattern, &utc_tm) == 0) {
        return zt_text_from_utf8_literal("1970-01-01");
    }
    return zt_text_from_utf8_literal(buffer);
}

zt_text *zt_format_datetime(zt_int millis, const zt_text *style, const zt_text *locale) {
    struct tm utc_tm;
    int ms_part = 0;
    char buffer[160];
    (void)locale;

    if (!zt_unix_ms_to_utc_tm(millis, &utc_tm, &ms_part)) {
        return zt_text_from_utf8_literal("1970-01-01T00:00:00.000Z");
    }

    if (zt_text_equals_literal(style, "short")) {
        if (strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &utc_tm) == 0) {
            return zt_text_from_utf8_literal("1970-01-01 00:00");
        }
        return zt_text_from_utf8_literal(buffer);
    }

    if (zt_text_equals_literal(style, "long")) {
        if (strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", &utc_tm) == 0) {
            return zt_text_from_utf8_literal("1970-01-01 00:00:00 UTC");
        }
        return zt_text_from_utf8_literal(buffer);
    }

    if (strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &utc_tm) == 0) {
        return zt_text_from_utf8_literal("1970-01-01T00:00:00.000Z");
    }

    {
        char iso_buffer[176];
        snprintf(iso_buffer, sizeof(iso_buffer), "%s.%03dZ", buffer, ms_part);
        return zt_text_from_utf8_literal(iso_buffer);
    }
}

zt_text *zt_format_date_pattern(zt_int millis, const zt_text *pattern) {
    struct tm utc_tm;
    const char *strftime_pattern = "%Y-%m-%d";
    char buffer[128];

    if (zt_text_equals_literal(pattern, "yyyy-MM-dd")) {
        strftime_pattern = "%Y-%m-%d";
    } else if (zt_text_equals_literal(pattern, "dd/MM/yyyy")) {
        strftime_pattern = "%d/%m/%Y";
    } else if (zt_text_equals_literal(pattern, "MM/dd/yyyy")) {
        strftime_pattern = "%m/%d/%Y";
    }

    if (!zt_unix_ms_to_utc_tm(millis, &utc_tm, NULL)) {
        return zt_text_from_utf8_literal("1970-01-01");
    }

    if (strftime(buffer, sizeof(buffer), strftime_pattern, &utc_tm) == 0) {
        return zt_text_from_utf8_literal("1970-01-01");
    }
    return zt_text_from_utf8_literal(buffer);
}

zt_text *zt_format_datetime_pattern(zt_int millis, const zt_text *pattern) {
    struct tm utc_tm;
    const char *strftime_pattern = "%Y-%m-%dT%H:%M:%S";
    char buffer[160];

    if (zt_text_equals_literal(pattern, "yyyy-MM-dd HH:mm:ss")) {
        strftime_pattern = "%Y-%m-%d %H:%M:%S";
    } else if (zt_text_equals_literal(pattern, "yyyy-MM-ddTHH:mm:ss")) {
        strftime_pattern = "%Y-%m-%dT%H:%M:%S";
    } else if (zt_text_equals_literal(pattern, "HH:mm:ss")) {
        strftime_pattern = "%H:%M:%S";
    }

    if (!zt_unix_ms_to_utc_tm(millis, &utc_tm, NULL)) {
        return zt_text_from_utf8_literal("1970-01-01T00:00:00");
    }

    if (strftime(buffer, sizeof(buffer), strftime_pattern, &utc_tm) == 0) {
        return zt_text_from_utf8_literal("1970-01-01T00:00:00");
    }
    return zt_text_from_utf8_literal(buffer);
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

zt_float zt_math_pow(zt_float base, zt_float exponent) {
    return pow(base, exponent);
}

zt_float zt_math_sqrt(zt_float value) {
    return sqrt(value);
}

zt_float zt_math_floor(zt_float value) {
    return floor(value);
}

zt_float zt_math_ceil(zt_float value) {
    return ceil(value);
}

zt_float zt_math_round_half_away_from_zero(zt_float value) {
    return round(value);
}

zt_float zt_math_trunc(zt_float value) {
    return trunc(value);
}

zt_float zt_math_sin(zt_float value) {
    return sin(value);
}

zt_float zt_math_cos(zt_float value) {
    return cos(value);
}

zt_float zt_math_tan(zt_float value) {
    return tan(value);
}

zt_float zt_math_asin(zt_float value) {
    return asin(value);
}

zt_float zt_math_acos(zt_float value) {
    return acos(value);
}

zt_float zt_math_atan(zt_float value) {
    return atan(value);
}

zt_float zt_math_atan2(zt_float y, zt_float x) {
    return atan2(y, x);
}

zt_float zt_math_ln(zt_float value) {
    return log(value);
}

zt_float zt_math_log10(zt_float value) {
    return log10(value);
}

zt_float zt_math_log_ten(zt_float value) {
    return log10(value);
}

zt_float zt_math_log2(zt_float value) {
    return log2(value);
}

zt_float zt_math_log(zt_float value, zt_float base) {
    if (base <= 0.0 || base == 1.0) {
        return NAN;
    }
    return log(value) / log(base);
}

zt_float zt_math_exp(zt_float value) {
    return exp(value);
}

zt_bool zt_math_is_nan(zt_float value) {
    return isnan(value) ? true : false;
}

zt_bool zt_math_is_infinite(zt_float value) {
    return isinf(value) ? true : false;
}

zt_bool zt_math_is_finite(zt_float value) {
    return isfinite(value) ? true : false;
}
static int zt_net_startup(char *message, size_t capacity) {
#ifdef _WIN32
    static int started = 0;
    WSADATA data;
    int code;

    if (started) {
        return 1;
    }

    code = WSAStartup(MAKEWORD(2, 2), &data);
    if (code != 0) {
        snprintf(message, capacity, "net.StartupFailed: WSAStartup failed with code %d", code);
        return 0;
    }

    started = 1;
#else
    (void)message;
    (void)capacity;
#endif
    return 1;
}

static int zt_net_last_error_code(void) {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

static int zt_net_would_block_code(int code) {
#ifdef _WIN32
    return code == WSAEWOULDBLOCK || code == WSAEINPROGRESS || code == WSAEALREADY;
#else
    return code == EINPROGRESS || code == EWOULDBLOCK || code == EAGAIN || code == EALREADY;
#endif
}

static void zt_net_format_error(char *buffer, size_t capacity, const char *prefix, int code) {
    if (buffer == NULL || capacity == 0) {
        return;
    }

#ifdef _WIN32
    snprintf(buffer, capacity, "%s (socket error %d)", prefix, code);
#else
    snprintf(buffer, capacity, "%s: %s", prefix, strerror(code));
#endif
}

static int zt_net_set_nonblocking(zt_socket_handle socket_value, char *message, size_t capacity) {
#ifdef _WIN32
    u_long mode = 1;
    if (ioctlsocket(socket_value, FIONBIO, &mode) != 0) {
        zt_net_format_error(message, capacity, "net.PlatformError: failed to set socket nonblocking", zt_net_last_error_code());
        return 0;
    }
#else
    int flags = fcntl(socket_value, F_GETFL, 0);
    if (flags < 0 || fcntl(socket_value, F_SETFL, flags | O_NONBLOCK) < 0) {
        zt_net_format_error(message, capacity, "net.PlatformError: failed to set socket nonblocking", errno);
        return 0;
    }
#endif
    return 1;
}

static int zt_net_wait_socket(zt_socket_handle socket_value, int wait_read, zt_int timeout_ms, int *out_error) {
    fd_set read_set;
    fd_set write_set;
    fd_set *read_ptr = NULL;
    fd_set *write_ptr = NULL;
    struct timeval tv;
    struct timeval *tv_ptr = NULL;
    int rc;

    if (out_error != NULL) {
        *out_error = 0;
    }

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    if (wait_read) {
        FD_SET(socket_value, &read_set);
        read_ptr = &read_set;
    } else {
        FD_SET(socket_value, &write_set);
        write_ptr = &write_set;
    }

    if (timeout_ms >= 0) {
        tv.tv_sec = (long)(timeout_ms / 1000);
        tv.tv_usec = (long)((timeout_ms % 1000) * 1000);
        tv_ptr = &tv;
    }

    do {
#ifdef _WIN32
        rc = select(0, read_ptr, write_ptr, NULL, tv_ptr);
#else
        rc = select(socket_value + 1, read_ptr, write_ptr, NULL, tv_ptr);
#endif
    } while (rc < 0 && zt_net_last_error_code() == EINTR);

    if (rc < 0 && out_error != NULL) {
        *out_error = zt_net_last_error_code();
    }

    return rc;
}

static int zt_net_socket_error(zt_socket_handle socket_value, int *out_error) {
    int socket_error = 0;
#ifdef _WIN32
    int length = (int)sizeof(socket_error);
#else
    socklen_t length = (socklen_t)sizeof(socket_error);
#endif

    if (getsockopt(socket_value, SOL_SOCKET, SO_ERROR, (char *)&socket_error, &length) != 0) {
        if (out_error != NULL) {
            *out_error = zt_net_last_error_code();
        }
        return 0;
    }

    if (out_error != NULL) {
        *out_error = socket_error;
    }
    return 1;
}

static zt_net_connection *zt_net_connection_new(zt_socket_handle socket_value, zt_int default_timeout_ms) {
    zt_net_connection *connection;

    connection = (zt_net_connection *)calloc(1, sizeof(zt_net_connection));
    if (connection == NULL) {
        zt_net_close_socket_handle((intptr_t)socket_value);
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate net.Connection");
    }

    connection->header.rc = 1;
    connection->header.kind = (uint32_t)ZT_HEAP_NET_CONNECTION;
    connection->socket_handle = (intptr_t)socket_value;
    connection->default_timeout_ms = default_timeout_ms;
    connection->closed = false;
    return connection;
}

static zt_core_error zt_net_core_error_from_prefixed_message(const char *message) {
    const char *safe_message = zt_safe_message(message);
    const char *colon = strchr(safe_message, ':');
    const char *detail = safe_message;
    zt_text *code_text;
    zt_text *message_text;
    zt_core_error error;

    if (colon != NULL && colon != safe_message) {
        size_t code_len = (size_t)(colon - safe_message);
        detail = colon + 1;
        while (*detail == ' ') detail += 1;
        code_text = zt_text_from_utf8(safe_message, code_len);
    } else {
        code_text = zt_text_from_utf8_literal("error");
    }

    message_text = zt_text_from_utf8_literal(detail);
    error = zt_core_error_make(code_text, message_text, zt_optional_text_empty());
    zt_release(code_text);
    zt_release(message_text);
    return error;
}

static zt_outcome_net_connection_core_error zt_net_connection_core_error_failure_prefixed(const char *message) {
    zt_core_error error = zt_net_core_error_from_prefixed_message(message);
    zt_outcome_net_connection_core_error outcome = zt_outcome_net_connection_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

static zt_outcome_optional_bytes_core_error zt_net_optional_bytes_core_error_failure_prefixed(const char *message) {
    zt_core_error error = zt_net_core_error_from_prefixed_message(message);
    zt_outcome_optional_bytes_core_error outcome = zt_outcome_optional_bytes_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

static zt_outcome_void_core_error zt_net_void_core_error_failure_prefixed(const char *message) {
    zt_core_error error = zt_net_core_error_from_prefixed_message(message);
    zt_outcome_void_core_error outcome = zt_outcome_void_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

zt_outcome_net_connection_core_error zt_net_connect(const zt_text *host, zt_int port, zt_int timeout_ms) {
    char message[256];
    char port_buffer[32];
    struct addrinfo hints;
    struct addrinfo *addresses = NULL;
    struct addrinfo *entry;
    int gai_code;
    zt_outcome_net_connection_core_error outcome;

    zt_runtime_require_text(host, "zt_net_connect requires host text");

    if (host->len == 0) {
        return zt_net_connection_core_error_failure_prefixed("net.InvalidAddress: host cannot be empty");
    }
    if (port < 1 || port > 65535) {
        return zt_net_connection_core_error_failure_prefixed("net.InvalidPort: port must be between 1 and 65535");
    }
    if (timeout_ms < 0) {
        return zt_net_connection_core_error_failure_prefixed("net.InvalidTimeout: connect timeout must be >= 0 milliseconds");
    }
    if (!zt_net_startup(message, sizeof(message))) {
        return zt_net_connection_core_error_failure_prefixed(message);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    snprintf(port_buffer, sizeof(port_buffer), "%lld", (long long)port);

    gai_code = getaddrinfo(zt_text_data(host), port_buffer, &hints, &addresses);
    if (gai_code != 0) {
#ifdef _WIN32
        snprintf(message, sizeof(message), "net.DnsFailed: getaddrinfo failed with code %d", gai_code);
#else
        snprintf(message, sizeof(message), "net.DnsFailed: %s", gai_strerror(gai_code));
#endif
        return zt_net_connection_core_error_failure_prefixed(message);
    }

    snprintf(message, sizeof(message), "net.ConnectionFailed: no address was reachable");
    for (entry = addresses; entry != NULL; entry = entry->ai_next) {
        zt_socket_handle socket_value;
        int rc;
        int code;

        socket_value = socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol);
        if (socket_value == ZT_NET_INVALID_SOCKET) {
            zt_net_format_error(message, sizeof(message), "net.SystemLimit: failed to create socket", zt_net_last_error_code());
            continue;
        }

        if (!zt_net_set_nonblocking(socket_value, message, sizeof(message))) {
            zt_net_close_socket_handle((intptr_t)socket_value);
            continue;
        }

        rc = connect(socket_value, entry->ai_addr, (int)entry->ai_addrlen);
        if (rc == 0) {
            zt_net_connection *connection = zt_net_connection_new(socket_value, timeout_ms);
            outcome = zt_outcome_net_connection_core_error_success(connection);
            zt_release(connection);
            freeaddrinfo(addresses);
            return outcome;
        }

        code = zt_net_last_error_code();
        if (!zt_net_would_block_code(code)) {
            zt_net_format_error(message, sizeof(message), "net.ConnectionFailed: connect failed", code);
            zt_net_close_socket_handle((intptr_t)socket_value);
            continue;
        }

        rc = zt_net_wait_socket(socket_value, 0, timeout_ms, &code);
        if (rc == 0) {
            snprintf(message, sizeof(message), "net.Timeout: connection timed out after %lld ms", (long long)timeout_ms);
            zt_net_close_socket_handle((intptr_t)socket_value);
            continue;
        }
        if (rc < 0) {
            zt_net_format_error(message, sizeof(message), "net.ConnectionFailed: connect wait failed", code);
            zt_net_close_socket_handle((intptr_t)socket_value);
            continue;
        }

        if (!zt_net_socket_error(socket_value, &code) || code != 0) {
            zt_net_format_error(message, sizeof(message), "net.ConnectionFailed: connect failed", code);
            zt_net_close_socket_handle((intptr_t)socket_value);
            continue;
        }

        {
            zt_net_connection *connection = zt_net_connection_new(socket_value, timeout_ms);
            outcome = zt_outcome_net_connection_core_error_success(connection);
            zt_release(connection);
            freeaddrinfo(addresses);
            return outcome;
        }
    }

    freeaddrinfo(addresses);
    return zt_net_connection_core_error_failure_prefixed(message);
}

static zt_int zt_net_effective_timeout_ms(const zt_net_connection *connection, zt_int timeout_ms) {
    return timeout_ms >= 0 ? timeout_ms : connection->default_timeout_ms;
}

zt_outcome_optional_bytes_core_error zt_net_read_some(zt_net_connection *connection, zt_int max, zt_int timeout_ms) {
    zt_socket_handle socket_value;
    zt_int effective_timeout;
    uint8_t *buffer;
    int wait_error = 0;
    int wait_result;
    int recv_count;
    zt_bytes *bytes_value;
    zt_optional_bytes optional;
    zt_outcome_optional_bytes_core_error outcome;

    zt_runtime_require_net_connection(connection, "zt_net_read_some requires connection");

    if (connection->closed) {
        return zt_net_optional_bytes_core_error_failure_prefixed("net.NotConnected: connection is closed");
    }
    if (max <= 0) {
        return zt_net_optional_bytes_core_error_failure_prefixed("net.InvalidReadSize: max must be > 0");
    }
    if (max > INT_MAX) {
        return zt_net_optional_bytes_core_error_failure_prefixed("net.Overflow: max is too large for this platform");
    }

    effective_timeout = zt_net_effective_timeout_ms(connection, timeout_ms);
    socket_value = (zt_socket_handle)connection->socket_handle;
    wait_result = zt_net_wait_socket(socket_value, 1, effective_timeout, &wait_error);
    if (wait_result == 0) {
        return zt_net_optional_bytes_core_error_failure_prefixed("net.Timeout: read timed out");
    }
    if (wait_result < 0) {
        char message[256];
        zt_net_format_error(message, sizeof(message), "net.ReadFailed: read wait failed", wait_error);
        return zt_net_optional_bytes_core_error_failure_prefixed(message);
    }

    buffer = (uint8_t *)malloc((size_t)max);
    if (buffer == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate net read buffer");
    }

    recv_count = recv(socket_value, (char *)buffer, (int)max, 0);
    if (recv_count == 0) {
        free(buffer);
        outcome = zt_outcome_optional_bytes_core_error_success(zt_optional_bytes_empty());
        return outcome;
    }
    if (recv_count < 0) {
        char message[256];
        int code = zt_net_last_error_code();
        free(buffer);
        zt_net_format_error(message, sizeof(message), "net.ReadFailed: recv failed", code);
        return zt_net_optional_bytes_core_error_failure_prefixed(message);
    }

    bytes_value = zt_bytes_from_array(buffer, (size_t)recv_count);
    free(buffer);
    optional = zt_optional_bytes_present(bytes_value);
    zt_release(bytes_value);
    outcome = zt_outcome_optional_bytes_core_error_success(optional);
    if (optional.is_present) {
        zt_release(optional.value);
    }
    return outcome;
}

zt_outcome_void_core_error zt_net_write_all(zt_net_connection *connection, const zt_bytes *data, zt_int timeout_ms) {
    zt_socket_handle socket_value;
    zt_int effective_timeout;
    size_t offset = 0;

    zt_runtime_require_net_connection(connection, "zt_net_write_all requires connection");
    zt_runtime_require_bytes(data, "zt_net_write_all requires data bytes");

    if (connection->closed) {
        return zt_net_void_core_error_failure_prefixed("net.NotConnected: connection is closed");
    }

    socket_value = (zt_socket_handle)connection->socket_handle;
    effective_timeout = zt_net_effective_timeout_ms(connection, timeout_ms);
    while (offset < data->len) {
        size_t remaining = data->len - offset;
        int chunk = remaining > 65536u ? 65536 : (int)remaining;
        int wait_error = 0;
        int wait_result = zt_net_wait_socket(socket_value, 0, effective_timeout, &wait_error);
        int sent;

        if (wait_result == 0) {
            return zt_net_void_core_error_failure_prefixed("net.Timeout: write timed out");
        }
        if (wait_result < 0) {
            char message[256];
            zt_net_format_error(message, sizeof(message), "net.WriteFailed: write wait failed", wait_error);
            return zt_net_void_core_error_failure_prefixed(message);
        }

        sent = send(socket_value, (const char *)(data->data + offset), chunk, 0);
        if (sent < 0) {
            char message[256];
            int code = zt_net_last_error_code();
            if (zt_net_would_block_code(code)) {
                continue;
            }
            zt_net_format_error(message, sizeof(message), "net.WriteFailed: send failed", code);
            return zt_net_void_core_error_failure_prefixed(message);
        }
        if (sent == 0) {
            return zt_net_void_core_error_failure_prefixed("net.PeerReset: connection closed while writing");
        }

        offset += (size_t)sent;
    }

    return zt_outcome_void_core_error_success();
}

zt_outcome_void_core_error zt_net_close(zt_net_connection *connection) {
    zt_runtime_require_net_connection(connection, "zt_net_close requires connection");

    if (connection->closed) {
        return zt_outcome_void_core_error_success();
    }

    zt_net_close_socket_handle(connection->socket_handle);
    connection->socket_handle = (intptr_t)ZT_NET_INVALID_SOCKET;
    connection->closed = true;
    return zt_outcome_void_core_error_success();
}

zt_bool zt_net_is_closed(const zt_net_connection *connection) {
    zt_runtime_require_net_connection(connection, "zt_net_is_closed requires connection");
    return connection->closed;
}

zt_int zt_net_error_kind_index(zt_core_error error) {
    const zt_text *code = error.code;

    if (zt_text_eq(code, zt_text_from_utf8_literal("net.ConnectionRefused"))) return 1;
    if (zt_text_eq(code, zt_text_from_utf8_literal("net.HostUnreachable"))) return 2;
    if (zt_text_eq(code, zt_text_from_utf8_literal("net.DnsFailed"))) return 2;
    if (zt_text_eq(code, zt_text_from_utf8_literal("net.Timeout"))) return 3;
    if (zt_text_eq(code, zt_text_from_utf8_literal("net.AddressInUse"))) return 4;
    if (zt_text_eq(code, zt_text_from_utf8_literal("net.AlreadyConnected"))) return 5;
    if (zt_text_eq(code, zt_text_from_utf8_literal("net.NotConnected"))) return 6;
    if (zt_text_eq(code, zt_text_from_utf8_literal("net.NetworkDown"))) return 7;
    if (zt_text_eq(code, zt_text_from_utf8_literal("net.Overflow"))) return 8;
    if (zt_text_eq(code, zt_text_from_utf8_literal("net.PeerReset"))) return 9;
    if (zt_text_eq(code, zt_text_from_utf8_literal("net.SystemLimit"))) return 10;
    return 0;
}

#define ZT_BOREALIS_BACKEND_STUB 0
#define ZT_BOREALIS_STUB_WINDOW_ID 1

static zt_core_error zt_borealis_backend_missing_error(void) {
    return zt_core_error_from_message(
        "borealis.backend_not_linked",
        "Borealis backend not linked. Configure [build].linker_flags in zenith.ztproj.");
}

static zt_outcome_i64_core_error zt_borealis_backend_missing_i64(void) {
    zt_core_error error = zt_borealis_backend_missing_error();
    zt_outcome_i64_core_error outcome = zt_outcome_i64_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

static zt_outcome_void_core_error zt_borealis_backend_missing_void(void) {
    zt_core_error error = zt_borealis_backend_missing_error();
    zt_outcome_void_core_error outcome = zt_outcome_void_core_error_failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

static zt_bool zt_borealis_is_stub_window(zt_int window_id) {
    return window_id == ZT_BOREALIS_STUB_WINDOW_ID;
}

zt_outcome_i64_core_error zt_borealis_open_window(const zt_text *title, zt_int width, zt_int height, zt_int target_fps, zt_int backend_id) {
    (void)title;
    (void)width;
    (void)height;
    (void)target_fps;

    if (backend_id == ZT_BOREALIS_BACKEND_STUB) {
        return zt_outcome_i64_core_error_success(ZT_BOREALIS_STUB_WINDOW_ID);
    }

    return zt_borealis_backend_missing_i64();
}

zt_outcome_void_core_error zt_borealis_close_window(zt_int window_id) {
    if (zt_borealis_is_stub_window(window_id)) {
        return zt_outcome_void_core_error_success();
    }
    return zt_borealis_backend_missing_void();
}

zt_bool zt_borealis_window_should_close(zt_int window_id) {
    (void)window_id;
    return true;
}

zt_outcome_void_core_error zt_borealis_begin_frame(zt_int window_id, zt_int clear_r, zt_int clear_g, zt_int clear_b, zt_int clear_a) {
    (void)clear_r;
    (void)clear_g;
    (void)clear_b;
    (void)clear_a;

    if (zt_borealis_is_stub_window(window_id)) {
        return zt_outcome_void_core_error_success();
    }
    return zt_borealis_backend_missing_void();
}

zt_outcome_void_core_error zt_borealis_end_frame(zt_int window_id) {
    if (zt_borealis_is_stub_window(window_id)) {
        return zt_outcome_void_core_error_success();
    }
    return zt_borealis_backend_missing_void();
}

zt_outcome_void_core_error zt_borealis_draw_rect(
        zt_int window_id,
        zt_float x,
        zt_float y,
        zt_float width,
        zt_float height,
        zt_int color_r,
        zt_int color_g,
        zt_int color_b,
        zt_int color_a) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)color_r;
    (void)color_g;
    (void)color_b;
    (void)color_a;

    if (zt_borealis_is_stub_window(window_id)) {
        return zt_outcome_void_core_error_success();
    }
    return zt_borealis_backend_missing_void();
}

zt_outcome_void_core_error zt_borealis_draw_text(
        zt_int window_id,
        const zt_text *value,
        zt_int x,
        zt_int y,
        zt_int size,
        zt_int color_r,
        zt_int color_g,
        zt_int color_b,
        zt_int color_a) {
    (void)value;
    (void)x;
    (void)y;
    (void)size;
    (void)color_r;
    (void)color_g;
    (void)color_b;
    (void)color_a;

    if (zt_borealis_is_stub_window(window_id)) {
        return zt_outcome_void_core_error_success();
    }
    return zt_borealis_backend_missing_void();
}

zt_bool zt_borealis_is_key_down(zt_int window_id, zt_int input_code) {
    (void)window_id;
    (void)input_code;
    return false;
}

static zt_bool zt_path_is_separator_char(char value) {
    return value == '/' || value == '\\';
}

static zt_bool zt_path_is_drive_letter(char value) {
    return (value >= 'A' && value <= 'Z') || (value >= 'a' && value <= 'z');
}

static void zt_path_parse_prefix(const char *data, size_t len, size_t *start, zt_bool *is_absolute, char drive[3]) {
    size_t cursor = 0;

    if (start == NULL || is_absolute == NULL || drive == NULL) {
        return;
    }

    *start = 0;
    *is_absolute = false;
    drive[0] = '\0';
    drive[1] = '\0';
    drive[2] = '\0';

    if (data == NULL || len == 0) {
        return;
    }

    if (len >= 2 && zt_path_is_drive_letter(data[0]) && data[1] == ':') {
        drive[0] = (char)toupper((unsigned char)data[0]);
        drive[1] = ':';
        drive[2] = '\0';
        cursor = 2;

        if (cursor < len && zt_path_is_separator_char(data[cursor])) {
            *is_absolute = true;
            cursor += 1;
            while (cursor < len && zt_path_is_separator_char(data[cursor])) {
                cursor += 1;
            }
        }

        *start = cursor;
        return;
    }

    if (zt_path_is_separator_char(data[0])) {
        *is_absolute = true;
        cursor = 1;
        while (cursor < len && zt_path_is_separator_char(data[cursor])) {
            cursor += 1;
        }
        *start = cursor;
    }
}

static zt_bool zt_path_segment_is_dot(const char *data, size_t start, size_t length) {
    return length == 1 && data[start] == '.';
}

static zt_bool zt_path_segment_is_dot_dot(const char *data, size_t start, size_t length) {
    return length == 2 && data[start] == '.' && data[start + 1] == '.';
}

static void zt_path_collect_segments(
        const char *data,
        size_t len,
        size_t start,
        size_t *segment_starts,
        size_t *segment_lengths,
        size_t *out_count) {
    size_t index;
    size_t count;

    if (out_count == NULL) {
        return;
    }

    *out_count = 0;
    if (data == NULL || segment_starts == NULL || segment_lengths == NULL) {
        return;
    }

    index = start;
    count = 0;

    while (index < len) {
        size_t part_start;
        size_t part_len;

        while (index < len && zt_path_is_separator_char(data[index])) {
            index += 1;
        }

        part_start = index;

        while (index < len && !zt_path_is_separator_char(data[index])) {
            index += 1;
        }

        part_len = index - part_start;
        if (part_len == 0) {
            continue;
        }

        segment_starts[count] = part_start;
        segment_lengths[count] = part_len;
        count += 1;
    }

    *out_count = count;
}

zt_text *zt_path_normalize(const zt_text *value) {
    const char *data;
    size_t len;
    size_t start;
    zt_bool absolute;
    char drive[3];
    size_t max_segments;
    size_t *segment_starts;
    size_t *segment_lengths;
    size_t segment_count;
    size_t index;
    char *output;
    size_t output_capacity;
    size_t output_length;
    zt_text *result;

    zt_runtime_require_text(value, "zt_path_normalize requires text");

    data = value->data;
    len = value->len;
    start = 0;
    absolute = false;
    zt_path_parse_prefix(data, len, &start, &absolute, drive);

    max_segments = len + 1;
    segment_starts = (size_t *)malloc(max_segments * sizeof(size_t));
    segment_lengths = (size_t *)malloc(max_segments * sizeof(size_t));
    if (segment_starts == NULL || segment_lengths == NULL) {
        free(segment_starts);
        free(segment_lengths);
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate path normalization buffers");
    }

    segment_count = 0;
    index = start;

    while (index < len) {
        size_t part_start;
        size_t part_len;

        while (index < len && zt_path_is_separator_char(data[index])) {
            index += 1;
        }

        part_start = index;

        while (index < len && !zt_path_is_separator_char(data[index])) {
            index += 1;
        }

        part_len = index - part_start;
        if (part_len == 0) {
            continue;
        }

        if (zt_path_segment_is_dot(data, part_start, part_len)) {
            continue;
        }

        if (zt_path_segment_is_dot_dot(data, part_start, part_len)) {
            if (segment_count > 0) {
                size_t prev_start = segment_starts[segment_count - 1];
                size_t prev_len = segment_lengths[segment_count - 1];
                if (!zt_path_segment_is_dot_dot(data, prev_start, prev_len)) {
                    segment_count -= 1;
                    continue;
                }
            }

            if (!absolute) {
                segment_starts[segment_count] = part_start;
                segment_lengths[segment_count] = part_len;
                segment_count += 1;
            }
            continue;
        }

        segment_starts[segment_count] = part_start;
        segment_lengths[segment_count] = part_len;
        segment_count += 1;
    }

    output_capacity = (len * 2) + 16;
    output = (char *)malloc(output_capacity);
    if (output == NULL) {
        free(segment_starts);
        free(segment_lengths);
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate normalized path output");
    }

    output_length = 0;

    if (drive[0] != '\0') {
        output[output_length++] = drive[0];
        output[output_length++] = ':';
        if (absolute) {
            output[output_length++] = '/';
        }
    } else if (absolute) {
        output[output_length++] = '/';
    }

    if (segment_count == 0) {
        if (output_length == 0) {
            output[output_length++] = '.';
        }
    } else {
        for (index = 0; index < segment_count; index += 1) {
            zt_bool suppress_separator = (drive[0] != '\0' && !absolute && output_length == 2);
            if (output_length > 0 && output[output_length - 1] != '/' && !suppress_separator) {
                output[output_length++] = '/';
            }
            memcpy(output + output_length, data + segment_starts[index], segment_lengths[index]);
            output_length += segment_lengths[index];
        }
    }

    output[output_length] = '\0';
    result = zt_text_from_utf8(output, output_length);

    free(output);
    free(segment_starts);
    free(segment_lengths);

    return result;
}

zt_bool zt_path_is_absolute(const zt_text *value) {
    size_t start = 0;
    zt_bool absolute = false;
    char drive[3];

    zt_runtime_require_text(value, "zt_path_is_absolute requires text");
    zt_path_parse_prefix(value->data, value->len, &start, &absolute, drive);
    return absolute;
}

zt_text *zt_path_absolute(const zt_text *value, const zt_text *base) {
    zt_text *normalized_path;
    zt_text *normalized_base;
    zt_text *result;

    zt_runtime_require_text(value, "zt_path_absolute requires path");
    zt_runtime_require_text(base, "zt_path_absolute requires base");

    normalized_path = zt_path_normalize(value);
    if (zt_path_is_absolute(normalized_path)) {
        return normalized_path;
    }

    normalized_base = zt_path_normalize(base);
    if (normalized_base->len == 0 || (normalized_base->len == 1 && normalized_base->data[0] == '.')) {
        result = zt_path_normalize(normalized_path);
        zt_release(normalized_path);
        zt_release(normalized_base);
        return result;
    }

    {
        zt_text *slash = zt_text_from_utf8_literal("/");
        zt_text *combined;

        if (normalized_base->len > 0 && zt_path_is_separator_char(normalized_base->data[normalized_base->len - 1])) {
            combined = zt_text_concat(normalized_base, normalized_path);
        } else {
            zt_text *with_separator = zt_text_concat(normalized_base, slash);
            combined = zt_text_concat(with_separator, normalized_path);
            zt_release(with_separator);
        }

        result = zt_path_normalize(combined);

        zt_release(combined);
        zt_release(slash);
    }

    zt_release(normalized_path);
    zt_release(normalized_base);

    return result;
}

zt_text *zt_path_relative(const zt_text *value, const zt_text *from) {
    zt_text *normalized_value;
    zt_text *normalized_from;
    size_t value_start;
    size_t from_start;
    zt_bool value_absolute;
    zt_bool from_absolute;
    char value_drive[3];
    char from_drive[3];
    size_t *value_starts;
    size_t *value_lengths;
    size_t value_count;
    size_t *from_starts;
    size_t *from_lengths;
    size_t from_count;
    size_t common;
    char *buffer;
    size_t capacity;
    size_t length;
    size_t index;
    zt_text *result;

    zt_runtime_require_text(value, "zt_path_relative requires path");
    zt_runtime_require_text(from, "zt_path_relative requires from");

    normalized_value = zt_path_normalize(value);
    normalized_from = zt_path_normalize(from);

    value_start = 0;
    value_absolute = false;
    value_drive[0] = '\0';
    value_drive[1] = '\0';
    value_drive[2] = '\0';
    zt_path_parse_prefix(normalized_value->data, normalized_value->len, &value_start, &value_absolute, value_drive);

    from_start = 0;
    from_absolute = false;
    from_drive[0] = '\0';
    from_drive[1] = '\0';
    from_drive[2] = '\0';
    zt_path_parse_prefix(normalized_from->data, normalized_from->len, &from_start, &from_absolute, from_drive);

    if (value_absolute != from_absolute || strcmp(value_drive, from_drive) != 0) {
        result = zt_text_from_utf8(normalized_value->data, normalized_value->len);
        zt_release(normalized_value);
        zt_release(normalized_from);
        return result;
    }

    value_starts = (size_t *)malloc((normalized_value->len + 1) * sizeof(size_t));
    value_lengths = (size_t *)malloc((normalized_value->len + 1) * sizeof(size_t));
    from_starts = (size_t *)malloc((normalized_from->len + 1) * sizeof(size_t));
    from_lengths = (size_t *)malloc((normalized_from->len + 1) * sizeof(size_t));
    if (value_starts == NULL || value_lengths == NULL || from_starts == NULL || from_lengths == NULL) {
        free(value_starts);
        free(value_lengths);
        free(from_starts);
        free(from_lengths);
        zt_release(normalized_value);
        zt_release(normalized_from);
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate path relative buffers");
    }

    zt_path_collect_segments(normalized_value->data, normalized_value->len, value_start, value_starts, value_lengths, &value_count);
    zt_path_collect_segments(normalized_from->data, normalized_from->len, from_start, from_starts, from_lengths, &from_count);

    common = 0;
    while (common < value_count && common < from_count) {
        size_t left_len = value_lengths[common];
        size_t right_len = from_lengths[common];
        if (left_len != right_len) {
            break;
        }
        if (memcmp(normalized_value->data + value_starts[common], normalized_from->data + from_starts[common], left_len) != 0) {
            break;
        }
        common += 1;
    }

    capacity = normalized_value->len + normalized_from->len + 8;
    buffer = (char *)malloc(capacity);
    if (buffer == NULL) {
        free(value_starts);
        free(value_lengths);
        free(from_starts);
        free(from_lengths);
        zt_release(normalized_value);
        zt_release(normalized_from);
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to allocate relative path output");
    }

    length = 0;

    for (index = common; index < from_count; index += 1) {
        if (length > 0) {
            buffer[length++] = '/';
        }
        buffer[length++] = '.';
        buffer[length++] = '.';
    }

    for (index = common; index < value_count; index += 1) {
        if (length > 0) {
            buffer[length++] = '/';
        }
        memcpy(buffer + length, normalized_value->data + value_starts[index], value_lengths[index]);
        length += value_lengths[index];
    }

    if (length == 0) {
        buffer[length++] = '.';
    }

    buffer[length] = '\0';
    result = zt_text_from_utf8(buffer, length);

    free(buffer);
    free(value_starts);
    free(value_lengths);
    free(from_starts);
    free(from_lengths);
    zt_release(normalized_value);
    zt_release(normalized_from);

    return result;
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

ZT_DEFINE_GRID2D_IMPL(i64, zt_int, ZT_HEAP_GRID2D_I64, 0, "int", 0)
ZT_DEFINE_GRID2D_IMPL(text, zt_text *, ZT_HEAP_GRID2D_TEXT, 1, "text", zt_text_from_utf8_literal(""))

ZT_DEFINE_PQUEUE_IMPL(i64, zt_int, ZT_HEAP_PQUEUE_I64, zt_optional_i64, zt_optional_i64_present, zt_optional_i64_empty, 0, "int", (_lhs < _rhs))
ZT_DEFINE_PQUEUE_IMPL(text, zt_text *, ZT_HEAP_PQUEUE_TEXT, zt_optional_text, zt_optional_text_present, zt_optional_text_empty, 1, "text", (_lhs != NULL && _rhs != NULL && strcmp(_lhs->data, _rhs->data) < 0))

ZT_DEFINE_CIRCBUF_IMPL(i64, zt_int, ZT_HEAP_CIRCBUF_I64, zt_optional_i64, zt_optional_i64_present, zt_optional_i64_empty, 0, "int")
ZT_DEFINE_CIRCBUF_IMPL(text, zt_text *, ZT_HEAP_CIRCBUF_TEXT, zt_optional_text, zt_optional_text_present, zt_optional_text_empty, 1, "text")

ZT_DEFINE_BTREEMAP_IMPL(text_text, zt_text *, zt_text *, ZT_HEAP_BTREEMAP_TEXT_TEXT, zt_optional_text_present, zt_optional_text_empty, 1, 1, "text,text", strcmp(_lhs->data, key->data), NULL)
ZT_DEFINE_BTREESET_IMPL(text, zt_text *, ZT_HEAP_BTREESET_TEXT, 1, "text", strcmp(_lhs->data, value->data))

ZT_DEFINE_GRID3D_IMPL(i64, zt_int, ZT_HEAP_GRID3D_I64, 0, "int", 0)
ZT_DEFINE_GRID3D_IMPL(text, zt_text *, ZT_HEAP_GRID3D_TEXT, 1, "text", zt_text_from_utf8_literal(""))







