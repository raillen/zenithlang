#include "compiler/targets/c/emitter.h"
#include "compiler/targets/c/legalization.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct c_local_decl {
    const char *name;
    const char *type_name;
} c_local_decl;

static const zir_module *c_active_module_for_symbol_lookup = NULL;
static const zir_module_var *c_find_module_var_decl(const zir_module *module_decl, const char *name);
static int c_is_module_var_name(const zir_module *module_decl, const char *name);

/* Buffer allocation strategy: use heap for large buffers to prevent stack overflow */
#define ZT_EMITTER_STACK_BUFFER_THRESHOLD 2048  /* 2KB threshold */

static char *c_emitter_alloc_buffer(size_t size, char *stack_buf, size_t stack_size) {
    if (size <= stack_size && size <= ZT_EMITTER_STACK_BUFFER_THRESHOLD) {
        return stack_buf;  /* Use stack buffer for small sizes */
    }
    /* Use heap for large buffers */
    char *heap_buf = (char *)malloc(size);
    if (heap_buf == NULL) {
        return stack_buf;  /* Fallback to stack if malloc fails */
    }
    return heap_buf;
}

static void c_emitter_free_buffer(char *buf, char *stack_buf) {
    if (buf != NULL && buf != stack_buf) {
        free(buf);  /* Only free if it's from heap */
    }
}

/* Buffer truncation for rollback on errors */
static size_t c_buffer_mark(c_string_buffer *buffer) {
    return buffer->length;
}

static void c_buffer_truncate(c_string_buffer *buffer, size_t mark) {
    if (mark <= buffer->length) {
        buffer->length = mark;
        buffer->data[mark] = '\0';
    }
}

/* Type name canonicalization: lowercase + remove spaces */
static void c_canonicalize_type(char *dest, size_t capacity, const char *type_name) {
    size_t i = 0, j = 0;
    
    if (type_name == NULL || capacity == 0) {
        if (capacity > 0) dest[0] = '\0';
        return;
    }
    
    while (type_name[i] != '\0' && j + 1 < capacity) {
        char ch = type_name[i];
        if (ch >= 'A' && ch <= 'Z') {
            dest[j++] = ch + 32;  /* lowercase */
        } else if (ch != ' ') {
            dest[j++] = ch;  /* skip spaces */
        }
        i++;
    }
    dest[j] = '\0';

    /* Canonical alias: result<T,E> is lowered as outcome<T,E> in the C backend. */
    if (strncmp(dest, "result<", 7) == 0) {
        char alias[128];
        snprintf(alias, sizeof(alias), "outcome%s", dest + 6);
        if (strlen(alias) + 1 <= capacity) {
            strncpy(dest, alias, capacity - 1);
            dest[capacity - 1] = '\0';
        }
    }

}

static const char *c_safe_text(const char *text) {
    return text != NULL ? text : "";
}

static void c_emit_set_result(c_emit_result *result, c_emit_error_code code, const char *format, ...) {
    va_list args;

    if (result == NULL) {
        return;
    }

    result->ok = (code == C_EMIT_OK);
    result->code = code;
    result->message[0] = '\0';

    if (format == NULL || format[0] == '\0') {
        return;
    }

    va_start(args, format);
    vsnprintf(result->message, sizeof(result->message), format, args);
    va_end(args);
}

void c_emit_result_init(c_emit_result *result) {
    c_emit_set_result(result, C_EMIT_OK, "");
}

const char *c_emit_error_code_name(c_emit_error_code code) {
    switch (code) {
        case C_EMIT_OK:
            return "c.emit.ok";
        case C_EMIT_INVALID_INPUT:
            return "c.emit.invalid_input";
        case C_EMIT_UNSUPPORTED_TYPE:
            return "c.emit.unsupported_type";
        case C_EMIT_UNSUPPORTED_EXPR:
            return "c.emit.unsupported_expr";
        case C_EMIT_INVALID_MAIN_SIGNATURE:
            return "c.emit.invalid_main_signature";
        default:
            return "c.emit.unknown_error";
    }
}

static int c_buffer_reserve(c_string_buffer *buffer, size_t additional) {
    size_t needed = buffer->length + additional + 1;
    size_t new_capacity;
    char *new_data;

    if (needed <= buffer->capacity) {
        return 1;
    }

    new_capacity = buffer->capacity > 0 ? buffer->capacity : 256;
    while (new_capacity < needed) {
        new_capacity *= 2;
    }

    new_data = (char *)realloc(buffer->data, new_capacity);
    if (new_data == NULL) {
        return 0;
    }

    buffer->data = new_data;
    buffer->capacity = new_capacity;
    return 1;
}

static int c_buffer_append_n(c_string_buffer *buffer, const char *text, size_t length) {
    if (!c_buffer_reserve(buffer, length)) {
        return 0;
    }

    memcpy(buffer->data + buffer->length, text, length);
    buffer->length += length;
    buffer->data[buffer->length] = '\0';
    return 1;
}

static int c_buffer_append(c_string_buffer *buffer, const char *text) {
    const char *safe = c_safe_text(text);
    return c_buffer_append_n(buffer, safe, strlen(safe));
}

static int c_buffer_append_vformat(c_string_buffer *buffer, const char *format, va_list args) {
    va_list copy;
    int needed;

    va_copy(copy, args);
    needed = vsnprintf(NULL, 0, format, copy);
    va_end(copy);

    if (needed < 0) {
        return 0;
    }

    if (!c_buffer_reserve(buffer, (size_t)needed)) {
        return 0;
    }

    vsnprintf(buffer->data + buffer->length, buffer->capacity - buffer->length, format, args);
    buffer->length += (size_t)needed;
    return 1;
}

static int c_buffer_append_format(c_string_buffer *buffer, const char *format, ...) {
    va_list args;
    int ok;

    va_start(args, format);
    ok = c_buffer_append_vformat(buffer, format, args);
    va_end(args);
    return ok;
}

void c_emitter_init(c_emitter *emitter) {
    emitter->buffer.data = NULL;
    emitter->buffer.length = 0;
    emitter->buffer.capacity = 0;
    emitter->newline = "\n";
}

void c_emitter_reset(c_emitter *emitter) {
    emitter->buffer.length = 0;
    if (emitter->buffer.data != NULL) {
        emitter->buffer.data[0] = '\0';
    }
}

void c_emitter_dispose(c_emitter *emitter) {
    free(emitter->buffer.data);
    emitter->buffer.data = NULL;
    emitter->buffer.length = 0;
    emitter->buffer.capacity = 0;
}

const char *c_emitter_text(const c_emitter *emitter) {
    return emitter->buffer.data != NULL ? emitter->buffer.data : "";
}

static int c_begin_line(c_emitter *emitter) {
    if (emitter->buffer.length > 0) {
        return c_buffer_append(&emitter->buffer, emitter->newline);
    }
    return 1;
}

static int c_emit_line(c_emitter *emitter, const char *text) {
    return c_begin_line(emitter) && c_buffer_append(&emitter->buffer, c_safe_text(text));
}

static void c_copy_sanitized(char *dest, size_t capacity, const char *source) {
    size_t out_index = 0;
    size_t index = 0;

    if (capacity == 0) {
        return;
    }

    while (source != NULL && source[index] != '\0' && out_index + 1 < capacity) {
        char ch = source[index];
        if ((ch >= 'a' && ch <= 'z') ||
                (ch >= 'A' && ch <= 'Z') ||
                (ch >= '0' && ch <= '9') ||
                ch == '_') {
            dest[out_index++] = ch;
        } else {
            dest[out_index++] = '_';
        }
        index += 1;
    }

    if (out_index == 0 && capacity > 1) {
        dest[out_index++] = '_';
    }

    dest[out_index] = '\0';
}

static const zir_struct_decl *c_find_user_struct(const zir_module *module_decl, const char *type_name) {
    size_t index;

    if (module_decl == NULL || type_name == NULL) return NULL;
    for (index = 0; index < module_decl->struct_count; index += 1) {
        const zir_struct_decl *struct_decl = &module_decl->structs[index];
        if (struct_decl->name != NULL && strcmp(struct_decl->name, type_name) == 0) {
            return struct_decl;
        }
    }
    return NULL;
}
static const zir_enum_decl *c_find_user_enum(const zir_module *module_decl, const char *type_name) {
    size_t index;

    if (module_decl == NULL || type_name == NULL) return NULL;
    for (index = 0; index < module_decl->enum_count; index += 1) {
        const zir_enum_decl *enum_decl = &module_decl->enums[index];
        if (enum_decl->name != NULL && strcmp(enum_decl->name, type_name) == 0) {
            return enum_decl;
        }
    }
    return NULL;
}

static const zir_function *c_find_function_decl(const zir_module *module_decl, const char *name) {
    size_t index;

    if (module_decl == NULL || name == NULL) return NULL;
    for (index = 0; index < module_decl->function_count; index += 1) {
        const zir_function *function_decl = &module_decl->functions[index];
        if (function_decl->name != NULL && strcmp(function_decl->name, name) == 0) {
            return function_decl;
        }
    }
    return NULL;
}

static void c_build_function_symbol(const zir_module *module_decl, const zir_function *function_decl, char *dest, size_t capacity) {
    char module_name[64];
    char function_name[64];

    c_copy_sanitized(module_name, sizeof(module_name), c_safe_text(module_decl->name));
    c_copy_sanitized(function_name, sizeof(function_name), c_safe_text(function_decl->name));
    snprintf(dest, capacity, "zt_%s__%s", module_name, function_name);
}

static void c_build_struct_symbol(const zir_module *module_decl, const zir_struct_decl *struct_decl, char *dest, size_t capacity) {
    char module_name[64];
    char struct_name[64];

    c_copy_sanitized(module_name, sizeof(module_name), c_safe_text(module_decl->name));
    c_copy_sanitized(struct_name, sizeof(struct_name), c_safe_text(struct_decl != NULL ? struct_decl->name : NULL));
    snprintf(dest, capacity, "zt_%s__%s", module_name, struct_name);
}
static void c_build_enum_symbol(const zir_module *module_decl, const zir_enum_decl *enum_decl, char *dest, size_t capacity) {
    char module_name[64];
    char enum_name[64];

    c_copy_sanitized(module_name, sizeof(module_name), c_safe_text(module_decl->name));
    c_copy_sanitized(enum_name, sizeof(enum_name), c_safe_text(enum_decl != NULL ? enum_decl->name : NULL));
    snprintf(dest, capacity, "zt_%s__%s", module_name, enum_name);
}

static const zir_field_decl *c_find_struct_field(const zir_struct_decl *struct_decl, const char *field_name) {
    size_t index;

    if (struct_decl == NULL || field_name == NULL) return NULL;
    for (index = 0; index < struct_decl->field_count; index += 1) {
        const zir_field_decl *field = &struct_decl->fields[index];
        if (field->name != NULL && strcmp(field->name, field_name) == 0) {
            return field;
        }
    }
    return NULL;
}

static const zir_enum_variant_field_decl *c_find_enum_variant_field(const zir_enum_variant_decl *variant, const char *field_name) {
    size_t index;
    if (variant == NULL || field_name == NULL) return NULL;
    for (index = 0; index < variant->field_count; index += 1) {
        const zir_enum_variant_field_decl *field = &variant->fields[index];
        if (field->name != NULL && strcmp(field->name, field_name) == 0) {
            return field;
        }
    }
    return NULL;
}

static void c_build_block_label(const char *label, char *dest, size_t capacity) {
    char sanitized[64];
    c_copy_sanitized(sanitized, sizeof(sanitized), c_safe_text(label));
    snprintf(dest, capacity, "zt_block_%s", sanitized);
}

static int c_is_blank(const char *text) {
    size_t index = 0;

    if (text == NULL) {
        return 1;
    }

    while (text[index] != '\0') {
        if (text[index] != ' ' && text[index] != '\t' && text[index] != '\n' && text[index] != '\r') {
            return 0;
        }
        index += 1;
    }

    return 1;
}

static int c_is_identifier_start(char ch) {
    return ((ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            ch == '_');
}

static int c_is_identifier_char(char ch) {
    return c_is_identifier_start(ch) || (ch >= '0' && ch <= '9');
}

static int c_is_identifier_only(const char *text) {
    size_t index = 0;

    if (text == NULL || text[0] == '\0' || !c_is_identifier_start(text[0])) {
        return 0;
    }

    index = 1;
    while (text[index] != '\0') {
        if (!c_is_identifier_char(text[index])) {
            return 0;
        }
        index += 1;
    }

    return 1;
}

static int c_copy_trimmed(char *dest, size_t capacity, const char *text) {
    const char *start = text;
    const char *end;
    size_t length;

    if (text == NULL) {
        if (capacity > 0) {
            dest[0] = '\0';
        }
        return 1;
    }

    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') {
        start += 1;
    }

    end = start + strlen(start);
    while (end > start &&
            (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n' || end[-1] == '\r')) {
        end -= 1;
    }

    length = (size_t)(end - start);
    if (length + 1 > capacity) {
        return 0;
    }

    memcpy(dest, start, length);
    dest[length] = '\0';
    return 1;
}

static int c_copy_trimmed_segment(char *dest, size_t capacity, const char *start, const char *end) {
    while (start < end && (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')) {
        start += 1;
    }

    while (end > start &&
            (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n' || end[-1] == '\r')) {
        end -= 1;
    }

    if ((size_t)(end - start) + 1 > capacity) {
        return 0;
    }

    memcpy(dest, start, (size_t)(end - start));
    dest[end - start] = '\0';
    return 1;
}

/* Type lookup table with binary search for O(log n) performance */
typedef enum c_type_category {
    C_TYPE_PRIMITIVE,      /* int, float, bool, void */
    C_TYPE_INTEGER,        /* int8, int16, int32, int64, uint* */
    C_TYPE_MANAGED,        /* text, bytes, list, map, optional, outcome */
    C_TYPE_USER_STRUCT     /* User-defined structs */
} c_type_category;

typedef struct c_type_mapping {
    const char *canonical_name;    /* lowercase, no spaces */
    const char *c_name;           /* C type name */
    c_type_category category;
    int is_managed;               /* Requires retain/release */
} c_type_mapping;

/* Sorted alphabetically by canonical_name for binary search */
static const c_type_mapping C_TYPE_TABLE[] = {
    {"bool", "zt_bool", C_TYPE_PRIMITIVE, 0},
    {"btreemap<text,text>", "zt_btreemap_text_text *", C_TYPE_MANAGED, 1},
    {"btreeset<text>", "zt_btreeset_text *", C_TYPE_MANAGED, 1},
    {"bytes", "zt_bytes *", C_TYPE_MANAGED, 1},
    {"circbuf<int>", "zt_circbuf_i64 *", C_TYPE_MANAGED, 1},
    {"circbuf<text>", "zt_circbuf_text *", C_TYPE_MANAGED, 1},
    {"connection", "zt_net_connection *", C_TYPE_MANAGED, 1},
    {"core.error", "zt_core_error", C_TYPE_MANAGED, 0},
    {"dyn<textrepresentable>", "zt_dyn_text_repr *", C_TYPE_MANAGED, 1},
    {"float", "zt_float", C_TYPE_PRIMITIVE, 0},
    {"grid2d<int>", "zt_grid2d_i64 *", C_TYPE_MANAGED, 1},
    {"grid2d<text>", "zt_grid2d_text *", C_TYPE_MANAGED, 1},
    {"grid3d<int>", "zt_grid3d_i64 *", C_TYPE_MANAGED, 1},
    {"grid3d<text>", "zt_grid3d_text *", C_TYPE_MANAGED, 1},
    {"int", "zt_int", C_TYPE_PRIMITIVE, 0},
    {"int16", "int16_t", C_TYPE_INTEGER, 0},
    {"int32", "int32_t", C_TYPE_INTEGER, 0},
    {"int64", "int64_t", C_TYPE_INTEGER, 0},
    {"int8", "int8_t", C_TYPE_INTEGER, 0},
    {"list<dyn<textrepresentable>>", "zt_list_dyn_text_repr *", C_TYPE_MANAGED, 1},
    {"list<float>", "zt_list_f64 *", C_TYPE_MANAGED, 1},
    {"list<int>", "zt_list_i64 *", C_TYPE_MANAGED, 1},
    {"list<text>", "zt_list_text *", C_TYPE_MANAGED, 1},
    {"map<text,text>", "zt_map_text_text *", C_TYPE_MANAGED, 1},
    {"net.connection", "zt_net_connection *", C_TYPE_MANAGED, 1},
    {"optional<bytes>", "zt_optional_bytes", C_TYPE_MANAGED, 0},
    {"optional<int>", "zt_optional_i64", C_TYPE_MANAGED, 0},
    {"optional<list<int>>", "zt_optional_list_i64", C_TYPE_MANAGED, 0},
    {"optional<list<text>>", "zt_optional_list_text", C_TYPE_MANAGED, 0},
    {"optional<map<text,text>>", "zt_optional_map_text_text", C_TYPE_MANAGED, 0},
    {"optional<text>", "zt_optional_text", C_TYPE_MANAGED, 0},
    {"outcome<bytes,text>", "zt_outcome_bytes_text", C_TYPE_MANAGED, 0},
    {"outcome<connection,core.error>", "zt_outcome_net_connection_core_error", C_TYPE_MANAGED, 0},
    {"outcome<connection,text>", "zt_outcome_net_connection_text", C_TYPE_MANAGED, 0},
    {"outcome<int,core.error>", "zt_outcome_i64_core_error", C_TYPE_MANAGED, 0},
    {"outcome<int,text>", "zt_outcome_i64_text", C_TYPE_MANAGED, 0},
    {"outcome<list<int>,core.error>", "zt_outcome_list_i64_core_error", C_TYPE_MANAGED, 0},
    {"outcome<list<int>,text>", "zt_outcome_list_i64_text", C_TYPE_MANAGED, 0},
    {"outcome<list<text>,text>", "zt_outcome_list_text_text", C_TYPE_MANAGED, 0},
    {"outcome<map<text,text>,core.error>", "zt_outcome_map_text_text_core_error", C_TYPE_MANAGED, 0},
    {"outcome<map<text,text>,text>", "zt_outcome_map_text_text", C_TYPE_MANAGED, 0},
    {"outcome<net.connection,core.error>", "zt_outcome_net_connection_core_error", C_TYPE_MANAGED, 0},
    {"outcome<net.connection,text>", "zt_outcome_net_connection_text", C_TYPE_MANAGED, 0},
    {"outcome<optional<bytes>,core.error>", "zt_outcome_optional_bytes_core_error", C_TYPE_MANAGED, 0},
    {"outcome<optional<bytes>,text>", "zt_outcome_optional_bytes_text", C_TYPE_MANAGED, 0},
    {"outcome<optional<text>,core.error>", "zt_outcome_optional_text_core_error", C_TYPE_MANAGED, 0},
    {"outcome<optional<text>,text>", "zt_outcome_optional_text_text", C_TYPE_MANAGED, 0},
    {"outcome<text,core.error>", "zt_outcome_text_core_error", C_TYPE_MANAGED, 0},
    {"outcome<text,text>", "zt_outcome_text_text", C_TYPE_MANAGED, 0},
    {"outcome<void,core.error>", "zt_outcome_void_core_error", C_TYPE_MANAGED, 0},
    {"outcome<void,text>", "zt_outcome_void_text", C_TYPE_MANAGED, 0},
    {"pqueue<int>", "zt_pqueue_i64 *", C_TYPE_MANAGED, 1},
    {"pqueue<text>", "zt_pqueue_text *", C_TYPE_MANAGED, 1},
    {"text", "zt_text *", C_TYPE_MANAGED, 1},
    {"uint16", "uint16_t", C_TYPE_INTEGER, 0},
    {"uint32", "uint32_t", C_TYPE_INTEGER, 0},
    {"uint64", "uint64_t", C_TYPE_INTEGER, 0},
    {"uint8", "uint8_t", C_TYPE_INTEGER, 0},
    {"void", "void", C_TYPE_PRIMITIVE, 0}
};

#define C_TYPE_TABLE_COUNT (sizeof(C_TYPE_TABLE) / sizeof(C_TYPE_TABLE[0]))

/* Binary search for O(log n) type lookup */
static const c_type_mapping* c_type_lookup(const char *type_name) {
    char canonical[128];
    
    if (type_name == NULL) return NULL;
    
    /* Canonicalize type name */
    c_canonicalize_type(canonical, sizeof(canonical), type_name);
    if (canonical[0] == '\0') return NULL;
    
    /* Binary search */
    size_t left = 0;
    size_t right = C_TYPE_TABLE_COUNT - 1;
    
    while (left <= right) {
        size_t mid = left + (right - left) / 2;
        int cmp = strcmp(canonical, C_TYPE_TABLE[mid].canonical_name);
        
        if (cmp == 0) {
            return &C_TYPE_TABLE[mid];
        } else if (cmp < 0) {
            if (mid == 0) break;
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    
    return NULL;  /* Not found */
}

typedef struct c_resolved_type_mapping {
    const char *canonical_name;
    const char *c_name;
    c_type_category category;
    int is_managed;
    char canonical_storage[128];
    char c_name_storage[128];
} c_resolved_type_mapping;

static int c_parse_unary_type_name(
        const char *type_name,
        const char *prefix,
        char *canonical_name,
        size_t canonical_capacity,
        char *inner_type_name,
        size_t inner_capacity) {
    const char *inner;
    const char *end;

    if (canonical_name == NULL || inner_type_name == NULL || prefix == NULL) {
        return 0;
    }

    c_canonicalize_type(canonical_name, canonical_capacity, type_name);
    if (strncmp(canonical_name, prefix, strlen(prefix)) != 0) {
        return 0;
    }

    inner = canonical_name + strlen(prefix);
    end = canonical_name + strlen(canonical_name);
    if (end <= inner || end[-1] != '>') {
        return 0;
    }

    return c_copy_trimmed_segment(inner_type_name, inner_capacity, inner, end - 1);
}

static int c_parse_binary_type_name(
        const char *type_name,
        const char *prefix,
        char *canonical_name,
        size_t canonical_capacity,
        char *left_type_name,
        size_t left_capacity,
        char *right_type_name,
        size_t right_capacity) {
    const char *inner;
    const char *end;
    const char *cursor;
    const char *comma = NULL;
    int depth = 0;

    if (canonical_name == NULL || left_type_name == NULL || right_type_name == NULL || prefix == NULL) {
        return 0;
    }

    c_canonicalize_type(canonical_name, canonical_capacity, type_name);
    if (strncmp(canonical_name, prefix, strlen(prefix)) != 0) {
        return 0;
    }

    inner = canonical_name + strlen(prefix);
    end = canonical_name + strlen(canonical_name);
    if (end <= inner || end[-1] != '>') {
        return 0;
    }
    end -= 1;

    for (cursor = inner; cursor < end; cursor += 1) {
        if (*cursor == '<') {
            depth += 1;
        } else if (*cursor == '>') {
            if (depth > 0) depth -= 1;
        } else if (*cursor == ',' && depth == 0) {
            comma = cursor;
            break;
        }
    }

    if (comma == NULL) {
        return 0;
    }

    return c_copy_trimmed_segment(left_type_name, left_capacity, inner, comma) &&
           c_copy_trimmed_segment(right_type_name, right_capacity, comma + 1, end);
}

static void c_build_generated_map_symbol(const char *canonical_name, char *dest, size_t capacity) {
    char sanitized[192];

    c_copy_sanitized(sanitized, sizeof(sanitized), canonical_name);
    snprintf(dest, capacity, "zt_map_generated_%s", sanitized);
}

static int c_resolve_type_mapping(const char *type_name, c_resolved_type_mapping *resolved) {
    const c_type_mapping *mapping;
    char canonical_name[128];
    char key_type_name[128];
    char value_type_name[128];

    if (resolved == NULL) {
        return 0;
    }

    memset(resolved, 0, sizeof(*resolved));

    mapping = c_type_lookup(type_name);
    if (mapping != NULL) {
        resolved->canonical_name = mapping->canonical_name;
        resolved->c_name = mapping->c_name;
        resolved->category = mapping->category;
        resolved->is_managed = mapping->is_managed;
        return 1;
    }

    if (c_parse_binary_type_name(
            type_name,
            "map<",
            canonical_name,
            sizeof(canonical_name),
            key_type_name,
            sizeof(key_type_name),
            value_type_name,
            sizeof(value_type_name))) {
        snprintf(resolved->canonical_storage, sizeof(resolved->canonical_storage), "%s", canonical_name);
        c_build_generated_map_symbol(canonical_name, resolved->c_name_storage, sizeof(resolved->c_name_storage));
        strncat(resolved->c_name_storage, " *", sizeof(resolved->c_name_storage) - strlen(resolved->c_name_storage) - 1);
        resolved->canonical_name = resolved->canonical_storage;
        resolved->c_name = resolved->c_name_storage;
        resolved->category = C_TYPE_MANAGED;
        resolved->is_managed = 1;
        return 1;
    }

    return 0;
}

/* Levenshtein distance for type suggestion */
static int c_levenshtein_distance(const char *s1, const char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    int matrix[32][32];  /* Max type name length: 32 chars */
    
    if (len1 > 31 || len2 > 31) return 999;  /* Too long */
    
    for (int i = 0; i <= len1; i++) matrix[i][0] = i;
    for (int j = 0; j <= len2; j++) matrix[0][j] = j;
    
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i-1] == s2[j-1]) ? 0 : 1;
            matrix[i][j] = matrix[i-1][j-1] + cost;  /* substitution */
            int deletion = matrix[i-1][j] + 1;
            int insertion = matrix[i][j-1] + 1;
            if (deletion < matrix[i][j]) matrix[i][j] = deletion;
            if (insertion < matrix[i][j]) matrix[i][j] = insertion;
        }
    }
    
    return matrix[len1][len2];
}

/* Suggest closest valid type name using Levenshtein distance */
static const char *c_type_suggest_closest(const char *invalid_type) {
    if (invalid_type == NULL || invalid_type[0] == '\0') return NULL;
    
    const char *best_match = NULL;
    int best_distance = 999;
    
    for (size_t i = 0; i < C_TYPE_TABLE_COUNT; i++) {
        int distance = c_levenshtein_distance(invalid_type, C_TYPE_TABLE[i].canonical_name);
        if (distance < best_distance) {
            best_distance = distance;
            best_match = C_TYPE_TABLE[i].canonical_name;
        }
    }
    
    /* Only suggest if distance is reasonable (< 40% of length) */
    int len = strlen(invalid_type);
    if (best_distance < len * 0.4 && best_distance <= 3) {
        return best_match;
    }
    
    return NULL;  /* No good match */
}

/* Optimized: single lookup instead of 11 separate calls */
static int c_type_is_managed(const char *type_name) {
    c_resolved_type_mapping mapping;
    return c_resolve_type_mapping(type_name, &mapping) && mapping.is_managed;
}

/* Generic type checker - replaces deprecated specialized checks. */
static inline int c_type_is(const char *type_name, const char *canonical_name) {
    c_resolved_type_mapping mapping;
    return c_resolve_type_mapping(type_name, &mapping) && strcmp(mapping.canonical_name, canonical_name) == 0;
}

static int c_parse_outcome_type_name(
        const char *type_name,
        char *canonical_name,
        size_t canonical_capacity,
        char *value_type_name,
        size_t value_capacity,
        char *error_type_name,
        size_t error_capacity) {
    const char *inner;
    const char *end;
    const char *cursor;
    const char *comma = NULL;
    int depth = 0;

    if (canonical_name == NULL || value_type_name == NULL || error_type_name == NULL) {
        return 0;
    }

    c_canonicalize_type(canonical_name, canonical_capacity, type_name);
    if (strncmp(canonical_name, "outcome<", 8) != 0) {
        return 0;
    }

    inner = canonical_name + 8;
    end = canonical_name + strlen(canonical_name);
    if (end <= inner || end[-1] != '>') {
        return 0;
    }
    end -= 1;

    for (cursor = inner; cursor < end; cursor += 1) {
        if (*cursor == '<') {
            depth += 1;
        } else if (*cursor == '>') {
            if (depth > 0) depth -= 1;
        } else if (*cursor == ',' && depth == 0) {
            comma = cursor;
            break;
        }
    }

    if (comma == NULL) {
        return 0;
    }

    return c_copy_trimmed_segment(value_type_name, value_capacity, inner, comma) &&
           c_copy_trimmed_segment(error_type_name, error_capacity, comma + 1, end);
}

static void c_build_generated_outcome_symbol(const char *canonical_name, char *dest, size_t capacity) {
    char sanitized[192];

    c_copy_sanitized(sanitized, sizeof(sanitized), canonical_name);
    snprintf(dest, capacity, "zt_generated_%s", sanitized);
}

static int c_type_is_builtin_managed_value(const char *type_name) {
    char canonical_name[128];
    char value_type_name[128];
    char error_type_name[128];

    c_canonicalize_type(canonical_name, sizeof(canonical_name), type_name);
    if (strcmp(canonical_name, "core.error") == 0) {
        return 1;
    }

    return c_parse_outcome_type_name(
        type_name,
        canonical_name,
        sizeof(canonical_name),
        value_type_name,
        sizeof(value_type_name),
        error_type_name,
        sizeof(error_type_name));
}

static int c_type_is_struct_with_managed_fields(const zir_module *module_decl, const char *type_name);

static int c_struct_has_managed_fields(const zir_module *module_decl, const zir_struct_decl *struct_decl) {
    size_t field_index;

    if (struct_decl == NULL) return 0;

    for (field_index = 0; field_index < struct_decl->field_count; field_index += 1) {
        const char *field_type_name = struct_decl->fields[field_index].type_name;
        if (c_type_is_managed(field_type_name) ||
                c_type_is_builtin_managed_value(field_type_name) ||
                c_type_is_struct_with_managed_fields(module_decl, field_type_name)) {
            return 1;
        }
    }

    return 0;
}

static int c_type_is_struct_with_managed_fields(const zir_module *module_decl, const char *type_name) {
    return c_struct_has_managed_fields(module_decl, c_find_user_struct(module_decl, type_name));
}

static int c_type_needs_managed_cleanup(const zir_module *module_decl, const char *type_name) {
    return c_type_is_managed(type_name) ||
           c_type_is_builtin_managed_value(type_name) ||
           c_type_is_struct_with_managed_fields(module_decl, type_name);
}

static int c_type_to_c(const zir_module *module_decl, const char *type_name, char *dest, size_t capacity, c_emit_result *result) {
    /* Try lookup table first (O(log n)) */
    c_resolved_type_mapping mapping;
    if (c_resolve_type_mapping(type_name, &mapping)) {
        snprintf(dest, capacity, "%s", mapping.c_name);
        return 1;
    }
    
    {
        char canonical_name[128];
        char value_type_name[128];
        char error_type_name[128];
        if (c_parse_outcome_type_name(
                type_name,
                canonical_name,
                sizeof(canonical_name),
                value_type_name,
                sizeof(value_type_name),
                error_type_name,
                sizeof(error_type_name))) {
            c_build_generated_outcome_symbol(canonical_name, dest, capacity);
            return 1;
        }
    }

    /* Fallback: check for user-defined structs */
    {
        const zir_struct_decl *struct_decl = c_find_user_struct(module_decl, type_name);
        if (struct_decl != NULL) {
            c_build_struct_symbol(module_decl, struct_decl, dest, capacity);
            return 1;
        }
    }

    {
        const zir_enum_decl *enum_decl = c_find_user_enum(module_decl, type_name);
        if (enum_decl != NULL) {
            c_build_enum_symbol(module_decl, enum_decl, dest, capacity);
            return 1;
        }
    }

    c_emit_set_result(
        result,
        C_EMIT_UNSUPPORTED_TYPE,
        "type '%s' is not supported. Valid types include: int, float, bool, text, bytes, list<T>, map<K,V>, optional<T>, outcome<T,E>.",
        c_safe_text(type_name)
    );
    
    /* Add suggestion if close match found */
    const char *suggestion = c_type_suggest_closest(type_name);
    if (suggestion != NULL) {
        char enhanced_message[512];
        snprintf(enhanced_message, sizeof(enhanced_message),
                "type '%s' is not supported. Did you mean '%s'?",
                c_safe_text(type_name), suggestion);
        c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "%s", enhanced_message);
    }
    
    return 0;
}

static int c_emit_trimmed_text(c_emitter *emitter, const char *text) {
    char value[256];

    if (!c_copy_trimmed(value, sizeof(value), text)) {
        return 0;
    }

    return c_buffer_append(&emitter->buffer, value);
}

static int c_emit_c_string_literal(c_emitter *emitter, const char *text) {
    size_t index;

    if (!c_buffer_append(&emitter->buffer, "\"")) {
        return 0;
    }

    if (text == NULL) {
        text = "";
    }

    for (index = 0; text[index] != '\0'; index += 1) {
        switch (text[index]) {
            case '\\':
                if (!c_buffer_append(&emitter->buffer, "\\\\")) return 0;
                break;
            case '"':
                if (!c_buffer_append(&emitter->buffer, "\\\"")) return 0;
                break;
            case '\n':
                if (!c_buffer_append(&emitter->buffer, "\\n")) return 0;
                break;
            case '\r':
                if (!c_buffer_append(&emitter->buffer, "\\r")) return 0;
                break;
            case '\t':
                if (!c_buffer_append(&emitter->buffer, "\\t")) return 0;
                break;
            default:
                if (!c_buffer_append_n(&emitter->buffer, text + index, 1)) return 0;
                break;
        }
    }

    return c_buffer_append(&emitter->buffer, "\"");
}

static int c_hex_digit_value(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return 10 + (ch - 'a');
    if (ch >= 'A' && ch <= 'F') return 10 + (ch - 'A');
    return -1;
}

static int c_emit_hex_bytes_literal(c_emitter *emitter, const char *hex_text, c_emit_result *result) {
    size_t length;
    size_t index;

    if (hex_text == NULL) hex_text = "";
    length = strlen(hex_text);
    if ((length % 2) != 0) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "bytes literal has an odd hex digit count");
        return 0;
    }

    if (length == 0) {
        return c_buffer_append(&emitter->buffer, "zt_bytes_from_array(NULL, 0)");
    }

    if (!c_buffer_append(&emitter->buffer, "zt_bytes_from_array((const uint8_t[]){")) return 0;
    for (index = 0; index < length; index += 2) {
        if (c_hex_digit_value(hex_text[index]) < 0 || c_hex_digit_value(hex_text[index + 1]) < 0) {
            c_emit_set_result(result, C_EMIT_INVALID_INPUT, "bytes literal contains invalid hex digits");
            return 0;
        }
        if (index > 0 && !c_buffer_append(&emitter->buffer, ", ")) return 0;
        if (!c_buffer_append_format(&emitter->buffer, "0x%c%c", hex_text[index], hex_text[index + 1])) return 0;
    }

    return c_buffer_append_format(&emitter->buffer, "}, %zu)", length / 2);
}

static int c_emit_typed_name(c_string_buffer *buffer, const char *c_type, const char *name) {
    size_t type_len = strlen(c_type);

    if (type_len > 0 && c_type[type_len - 1] == '*') {
        return c_buffer_append_format(buffer, "%s%s", c_type, name);
    }

    return c_buffer_append_format(buffer, "%s %s", c_type, name);
}

static const char *c_find_symbol_type(const zir_function *function_decl, const char *name) {
    size_t param_index;
    size_t block_index;

    if (function_decl != NULL) {
        for (param_index = 0; param_index < function_decl->param_count; param_index += 1) {
            if (strcmp(function_decl->params[param_index].name, name) == 0) {
                return function_decl->params[param_index].type_name;
            }
        }

        for (block_index = 0; block_index < function_decl->block_count; block_index += 1) {
            size_t instruction_index;
            const zir_block *block = &function_decl->blocks[block_index];

            for (instruction_index = 0; instruction_index < block->instruction_count; instruction_index += 1) {
                const zir_instruction *instruction = &block->instructions[instruction_index];

                if (instruction->kind == ZIR_INSTR_ASSIGN &&
                        instruction->dest_name != NULL &&
                        strcmp(instruction->dest_name, name) == 0) {
                    return instruction->type_name;
                }
            }
        }
    }

    if (c_active_module_for_symbol_lookup != NULL) {
        const zir_module_var *module_var = c_find_module_var_decl(c_active_module_for_symbol_lookup, name);
        if (module_var != NULL) {
            return module_var->type_name;
        }
    }

    return NULL;
}

/* Helper: copy trimmed text with heap allocation for safety */
static int c_copy_trimmed_alloc(char **dest, const char *source) {
    *dest = (char *)malloc(256);
    if (*dest == NULL) return 0;
    return c_copy_trimmed(*dest, 256, source);
}

static int c_expression_is_text(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text)) {
        return 0;
    }

    int result = 0;
    if (strncmp(trimmed, "const \"", 7) == 0) {
        result = 1;
    } else if (c_is_identifier_only(trimmed)) {
        type_name = c_find_symbol_type(function_decl, trimmed);
        result = c_type_is(type_name, "text");
    }
    
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_text_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "text");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_bytes_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "bytes");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_net_connection_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "net.connection") || c_type_is(type_name, "connection");
    free(trimmed);
    return result;
}
static int c_expression_is_materialized_core_error_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "core.error");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_list_i64_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "list<int>");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_map_text_text_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "map<text,text>");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_type_ref(
        const zir_function *function_decl,
        const char *expr_text,
        const char *expected_type_name) {
    char *trimmed;
    const char *type_name;
    int result = 0;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    if (type_name != NULL && expected_type_name != NULL && c_type_is(type_name, expected_type_name)) {
        result = 1;
    }

    free(trimmed);
    return result;
}

typedef struct c_map_spec {
    char canonical_name[128];
    char key_type_name[128];
    char value_type_name[128];
    char c_type_name[128];
    char new_fn[160];
    char from_arrays_fn[160];
    char set_fn[160];
    char set_owned_fn[160];
    char get_fn[160];
    char get_optional_fn[160];
    char key_at_fn[160];
    char value_at_fn[160];
    char len_fn[160];
    char optional_value_type_name[128];
    char optional_present_fn[160];
    char optional_empty_fn[160];
    char key_eq_fn[64];
    int is_generated;
} c_map_spec;

static int c_type_is_plain_value(const zir_module *module_decl, const char *type_name) {
    return !c_type_is_managed(type_name) &&
           !c_type_is_builtin_managed_value(type_name) &&
           !c_type_is_struct_with_managed_fields(module_decl, type_name);
}

static int c_map_value_optional_support(
        const zir_module *module_decl,
        const char *value_type_name,
        char *optional_type_name,
        size_t optional_type_capacity,
        char *present_fn,
        size_t present_fn_capacity,
        char *empty_fn,
        size_t empty_fn_capacity) {
    (void)module_decl;

    if (c_type_is(value_type_name, "int")) {
        snprintf(optional_type_name, optional_type_capacity, "zt_optional_i64");
        snprintf(present_fn, present_fn_capacity, "zt_optional_i64_present");
        snprintf(empty_fn, empty_fn_capacity, "zt_optional_i64_empty");
        return 1;
    }

    if (c_type_is(value_type_name, "text")) {
        snprintf(optional_type_name, optional_type_capacity, "zt_optional_text");
        snprintf(present_fn, present_fn_capacity, "zt_optional_text_present");
        snprintf(empty_fn, empty_fn_capacity, "zt_optional_text_empty");
        return 1;
    }

    if (c_type_is(value_type_name, "bytes")) {
        snprintf(optional_type_name, optional_type_capacity, "zt_optional_bytes");
        snprintf(present_fn, present_fn_capacity, "zt_optional_bytes_present");
        snprintf(empty_fn, empty_fn_capacity, "zt_optional_bytes_empty");
        return 1;
    }

    if (c_type_is(value_type_name, "list<int>")) {
        snprintf(optional_type_name, optional_type_capacity, "zt_optional_list_i64");
        snprintf(present_fn, present_fn_capacity, "zt_optional_list_i64_present");
        snprintf(empty_fn, empty_fn_capacity, "zt_optional_list_i64_empty");
        return 1;
    }

    if (c_type_is(value_type_name, "list<text>")) {
        snprintf(optional_type_name, optional_type_capacity, "zt_optional_list_text");
        snprintf(present_fn, present_fn_capacity, "zt_optional_list_text_present");
        snprintf(empty_fn, empty_fn_capacity, "zt_optional_list_text_empty");
        return 1;
    }

    if (c_type_is(value_type_name, "map<text,text>")) {
        snprintf(optional_type_name, optional_type_capacity, "zt_optional_map_text_text");
        snprintf(present_fn, present_fn_capacity, "zt_optional_map_text_text_present");
        snprintf(empty_fn, empty_fn_capacity, "zt_optional_map_text_text_empty");
        return 1;
    }

    return 0;
}

static int c_map_key_eq_support(const char *key_type_name, char *key_eq_fn, size_t key_eq_fn_capacity) {
    if (c_type_is(key_type_name, "int")) {
        snprintf(key_eq_fn, key_eq_fn_capacity, "zt_i64_eq");
        return 1;
    }

    if (c_type_is(key_type_name, "text")) {
        snprintf(key_eq_fn, key_eq_fn_capacity, "zt_text_eq");
        return 1;
    }

    return 0;
}

static int c_map_spec_for_type(const zir_module *module_decl, const char *type_name, c_map_spec *spec) {
    c_resolved_type_mapping mapping;
    char canonical_name[128];
    char key_type_name[128];
    char value_type_name[128];

    if (spec == NULL) {
        return 0;
    }

    memset(spec, 0, sizeof(*spec));
    if (!c_parse_binary_type_name(
            type_name,
            "map<",
            canonical_name,
            sizeof(canonical_name),
            key_type_name,
            sizeof(key_type_name),
            value_type_name,
            sizeof(value_type_name))) {
        return 0;
    }

    if (!c_map_key_eq_support(key_type_name, spec->key_eq_fn, sizeof(spec->key_eq_fn))) {
        return 0;
    }

    if (!(c_type_is_managed(key_type_name) || c_type_is_plain_value(module_decl, key_type_name))) {
        return 0;
    }

    if (!(c_type_is_managed(value_type_name) || c_type_is_plain_value(module_decl, value_type_name))) {
        return 0;
    }

    if (!c_map_value_optional_support(
            module_decl,
            value_type_name,
            spec->optional_value_type_name,
            sizeof(spec->optional_value_type_name),
            spec->optional_present_fn,
            sizeof(spec->optional_present_fn),
            spec->optional_empty_fn,
            sizeof(spec->optional_empty_fn))) {
        return 0;
    }

    snprintf(spec->canonical_name, sizeof(spec->canonical_name), "%s", canonical_name);
    snprintf(spec->key_type_name, sizeof(spec->key_type_name), "%s", key_type_name);
    snprintf(spec->value_type_name, sizeof(spec->value_type_name), "%s", value_type_name);

    if (c_resolve_type_mapping(type_name, &mapping)) {
        snprintf(spec->c_type_name, sizeof(spec->c_type_name), "%s", mapping.c_name);
        spec->is_generated = strcmp(mapping.canonical_name, "map<text,text>") != 0;
    } else {
        return 0;
    }

    if (spec->is_generated) {
        size_t length = strlen(spec->c_type_name);
        if (length < 3 || strcmp(spec->c_type_name + length - 2, " *") != 0) {
            return 0;
        }
        spec->c_type_name[length - 2] = '\0';
    } else if (strstr(spec->c_type_name, " *") != NULL) {
        size_t length = strlen(spec->c_type_name);
        spec->c_type_name[length - 2] = '\0';
    }

    snprintf(spec->new_fn, sizeof(spec->new_fn), "%s_new", spec->c_type_name);
    snprintf(spec->from_arrays_fn, sizeof(spec->from_arrays_fn), "%s_from_arrays", spec->c_type_name);
    snprintf(spec->set_fn, sizeof(spec->set_fn), "%s_set", spec->c_type_name);
    snprintf(spec->set_owned_fn, sizeof(spec->set_owned_fn), "%s_set_owned", spec->c_type_name);
    snprintf(spec->get_fn, sizeof(spec->get_fn), "%s_get", spec->c_type_name);
    snprintf(spec->get_optional_fn, sizeof(spec->get_optional_fn), "%s_get_optional", spec->c_type_name);
    snprintf(spec->key_at_fn, sizeof(spec->key_at_fn), "%s_key_at", spec->c_type_name);
    snprintf(spec->value_at_fn, sizeof(spec->value_at_fn), "%s_value_at", spec->c_type_name);
    snprintf(spec->len_fn, sizeof(spec->len_fn), "%s_len", spec->c_type_name);

    strncat(spec->c_type_name, " *", sizeof(spec->c_type_name) - strlen(spec->c_type_name) - 1);
    return 1;
}

static int c_expression_is_materialized_optional_text_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "optional<text>");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_optional_bytes_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "optional<bytes>");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_optional_list_i64_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "optional<list<int>>");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_outcome_i64_text_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "outcome<int,text>");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_outcome_void_text_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "outcome<void,text>");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_outcome_text_text_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "outcome<text,text>");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_outcome_optional_text_text_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "outcome<optional<text>,text>");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_outcome_bytes_text_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "outcome<bytes,text>");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_outcome_optional_bytes_text_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "outcome<optional<bytes>,text>");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_outcome_net_connection_text_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "outcome<connection,text>") || c_type_is(type_name, "outcome<net.connection,text>");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_outcome_list_i64_text_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "outcome<list<int>,text>");
    free(trimmed);
    return result;
}

static int c_expression_is_materialized_outcome_map_text_text_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text) || !c_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    int result = c_type_is(type_name, "outcome<map<text,text>,text>");
    free(trimmed);
    return result;
}

typedef struct c_outcome_spec {
    char canonical_name[128];
    char display_name[128];
    char value_type_name[128];
    char error_type_name[128];
    char c_type_name[128];
    char success_fn[160];
    char failure_fn[160];
    char failure_message_fn[160];
    char failure_text_fn[160];
    char is_success_fn[160];
    char value_fn[160];
    char propagate_fn[160];
    char dispose_fn[160];
    int has_value;
    int error_is_core;
    int error_is_text;
    int is_generated;
} c_outcome_spec;

static int c_outcome_spec_for_type(const char *type_name, c_outcome_spec *spec) {
    c_resolved_type_mapping mapping;
    char canonical_name[128];
    char value_type_name[128];
    char error_type_name[128];

    if (spec == NULL) {
        return 0;
    }

    memset(spec, 0, sizeof(*spec));
    if (!c_parse_outcome_type_name(
            type_name,
            canonical_name,
            sizeof(canonical_name),
            value_type_name,
            sizeof(value_type_name),
            error_type_name,
            sizeof(error_type_name))) {
        return 0;
    }

    snprintf(spec->canonical_name, sizeof(spec->canonical_name), "%s", canonical_name);
    snprintf(spec->display_name, sizeof(spec->display_name), "%s", canonical_name);
    snprintf(spec->value_type_name, sizeof(spec->value_type_name), "%s", value_type_name);
    snprintf(spec->error_type_name, sizeof(spec->error_type_name), "%s", error_type_name);
    spec->has_value = strcmp(value_type_name, "void") != 0;
    spec->error_is_core = strcmp(error_type_name, "core.error") == 0;
    spec->error_is_text = strcmp(error_type_name, "text") == 0;

    if (c_resolve_type_mapping(type_name, &mapping)) {
        snprintf(spec->c_type_name, sizeof(spec->c_type_name), "%s", mapping.c_name);
    } else {
        c_build_generated_outcome_symbol(canonical_name, spec->c_type_name, sizeof(spec->c_type_name));
        spec->is_generated = 1;
    }

    snprintf(spec->success_fn, sizeof(spec->success_fn), "%s_success", spec->c_type_name);
    snprintf(spec->failure_fn, sizeof(spec->failure_fn), "%s_failure", spec->c_type_name);
    snprintf(spec->is_success_fn, sizeof(spec->is_success_fn), "%s_is_success", spec->c_type_name);
    snprintf(spec->propagate_fn, sizeof(spec->propagate_fn), "%s_propagate", spec->c_type_name);
    snprintf(spec->dispose_fn, sizeof(spec->dispose_fn), "%s_dispose", spec->c_type_name);
    if (spec->has_value) {
        snprintf(spec->value_fn, sizeof(spec->value_fn), "%s_value", spec->c_type_name);
    }
    if (spec->error_is_text || spec->error_is_core) {
        snprintf(spec->failure_message_fn, sizeof(spec->failure_message_fn), "%s_failure_message", spec->c_type_name);
    }
    if (spec->error_is_core) {
        snprintf(spec->failure_text_fn, sizeof(spec->failure_text_fn), "%s_failure_text", spec->c_type_name);
    }

    return 1;
}

static int c_outcome_spec_for_expr(const zir_function *function_decl, const char *expr_text, c_outcome_spec *spec) {
    char *trimmed;
    const char *type_name;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text)) {
        return 0;
    }

    if (!c_is_identifier_only(trimmed)) {
        free(trimmed);
        return 0;
    }

    type_name = c_find_symbol_type(function_decl, trimmed);
    free(trimmed);
    return c_outcome_spec_for_type(type_name, spec);
}

static int c_outcome_spec_for_expected(const char *expected_type_name, int has_value, c_outcome_spec *spec) {
    if (c_is_blank(expected_type_name)) {
        return c_outcome_spec_for_type(has_value ? "outcome<int,text>" : "outcome<void,text>", spec);
    }

    return c_outcome_spec_for_type(expected_type_name, spec);
}

static int c_type_requires_generated_map_helper(const zir_module *module_decl, const char *type_name) {
    c_map_spec spec;
    return c_map_spec_for_type(module_decl, type_name, &spec) && spec.is_generated;
}

static int c_module_requires_template_header(const zir_module *module_decl) {
    size_t struct_index;
    size_t function_index;

    if (module_decl == NULL) {
        return 0;
    }

    for (struct_index = 0; struct_index < module_decl->struct_count; struct_index += 1) {
        const zir_struct_decl *struct_decl = &module_decl->structs[struct_index];
        size_t field_index;
        for (field_index = 0; field_index < struct_decl->field_count; field_index += 1) {
            if (c_type_requires_generated_map_helper(module_decl, struct_decl->fields[field_index].type_name)) {
                return 1;
            }
        }
    }

    for (function_index = 0; function_index < module_decl->function_count; function_index += 1) {
        const zir_function *function_decl = &module_decl->functions[function_index];
        size_t param_index;
        size_t block_index;

        if (c_type_requires_generated_map_helper(module_decl, function_decl->return_type)) {
            return 1;
        }

        for (param_index = 0; param_index < function_decl->param_count; param_index += 1) {
            if (c_type_requires_generated_map_helper(module_decl, function_decl->params[param_index].type_name)) {
                return 1;
            }
        }

        for (block_index = 0; block_index < function_decl->block_count; block_index += 1) {
            const zir_block *block = &function_decl->blocks[block_index];
            size_t instruction_index;
            for (instruction_index = 0; instruction_index < block->instruction_count; instruction_index += 1) {
                const zir_instruction *instruction = &block->instructions[instruction_index];
                if (instruction->type_name != NULL &&
                        c_type_requires_generated_map_helper(module_decl, instruction->type_name)) {
                    return 1;
                }
            }
        }
    }

    return 0;
}

static int c_type_requires_generated_outcome_helper(const char *type_name) {
    c_outcome_spec spec;
    return c_outcome_spec_for_type(type_name, &spec) && spec.is_generated;
}

static int c_module_requires_string_header(const zir_module *module_decl) {
    size_t struct_index;
    size_t function_index;

    if (module_decl == NULL) {
        return 0;
    }

    for (struct_index = 0; struct_index < module_decl->struct_count; struct_index += 1) {
        const zir_struct_decl *struct_decl = &module_decl->structs[struct_index];
        size_t field_index;
        for (field_index = 0; field_index < struct_decl->field_count; field_index += 1) {
            if (c_type_requires_generated_outcome_helper(struct_decl->fields[field_index].type_name)) {
                return 1;
            }
        }
    }

    for (function_index = 0; function_index < module_decl->function_count; function_index += 1) {
        const zir_function *function_decl = &module_decl->functions[function_index];
        size_t param_index;
        size_t block_index;

        if (c_type_requires_generated_outcome_helper(function_decl->return_type)) {
            return 1;
        }

        for (param_index = 0; param_index < function_decl->param_count; param_index += 1) {
            if (c_type_requires_generated_outcome_helper(function_decl->params[param_index].type_name)) {
                return 1;
            }
        }

        for (block_index = 0; block_index < function_decl->block_count; block_index += 1) {
            const zir_block *block = &function_decl->blocks[block_index];
            size_t instruction_index;
            for (instruction_index = 0; instruction_index < block->instruction_count; instruction_index += 1) {
                const zir_instruction *instruction = &block->instructions[instruction_index];
                if (instruction->type_name != NULL &&
                        c_type_requires_generated_outcome_helper(instruction->type_name)) {
                    return 1;
                }
            }
        }
    }

    return 0;
}
static int c_expression_is_copyable_managed_value_ref(const zir_function *function_decl, const char *expr_text) {
    char *trimmed;
    int result;

    if (!c_copy_trimmed_alloc(&trimmed, expr_text)) {
        return 0;
    }

    result = c_is_identifier_only(trimmed) || strncmp(trimmed, "get_field ", 10) == 0;
    free(trimmed);
    (void)function_decl;
    return result;
}

static int c_outcome_success_value_is_supported(
        const zir_module *module_decl,
        const zir_function *function_decl,
        const c_outcome_spec *spec,
        const char *value_expr) {
    if (spec == NULL || !spec->has_value) return 0;

    if (c_type_is_managed(spec->value_type_name) ||
            c_type_is_builtin_managed_value(spec->value_type_name) ||
            c_type_is_struct_with_managed_fields(module_decl, spec->value_type_name)) {
        return c_expression_is_copyable_managed_value_ref(function_decl, value_expr);
    }

    return 1;
}

static int c_emit_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const char *expr_text,
        const char *expected_type_name,
        c_emit_result *result);

static int c_outcome_emit_failure_from_error_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const c_outcome_spec *spec,
        const char *error_expr,
        c_emit_result *result) {
    int error_needs_copyable_ref;

    if (spec == NULL) {
        return 0;
    }

    error_needs_copyable_ref =
        c_type_is_managed(spec->error_type_name) ||
        c_type_is_builtin_managed_value(spec->error_type_name) ||
        c_type_is_struct_with_managed_fields(module_decl, spec->error_type_name);

    if (spec->error_is_core && c_expression_is_materialized_core_error_ref(function_decl, error_expr)) {
        return c_buffer_append_format(&emitter->buffer, "%s(", spec->failure_fn) &&
               c_emit_expr(emitter, module_decl, function_decl, error_expr, "core.Error", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (spec->error_is_core) {
        if (!c_expression_is_copyable_managed_value_ref(function_decl, error_expr)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_EXPR,
                "outcome_failure for %s currently requires a materialized text or core.Error value, got '%s'",
                spec->display_name,
                error_expr
            );
            return 0;
        }

        return c_buffer_append_format(&emitter->buffer, "%s(", spec->failure_text_fn) &&
               c_emit_expr(emitter, module_decl, function_decl, error_expr, "text", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (spec->error_is_text) {
        if (!c_expression_is_copyable_managed_value_ref(function_decl, error_expr)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_EXPR,
                "outcome_failure for %s currently requires a materialized text value, got '%s'",
                spec->display_name,
                error_expr
            );
            return 0;
        }

        return c_buffer_append_format(&emitter->buffer, "%s(", spec->failure_fn) &&
               c_emit_expr(emitter, module_decl, function_decl, error_expr, "text", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (error_needs_copyable_ref && !c_expression_is_copyable_managed_value_ref(function_decl, error_expr)) {
        c_emit_set_result(
            result,
            C_EMIT_UNSUPPORTED_EXPR,
            "outcome_failure for %s currently requires a materialized error value, got '%s'",
            spec->display_name,
            error_expr
        );
        return 0;
    }

    return c_buffer_append_format(&emitter->buffer, "%s(", spec->failure_fn) &&
           c_emit_expr(emitter, module_decl, function_decl, error_expr, spec->error_type_name, result) &&
           c_buffer_append(&emitter->buffer, ")");
}

static int c_emit_value(c_emitter *emitter, const char *text) {
    return c_emit_trimmed_text(emitter, text);
}

static int c_parse_binary(const char *text, char *op_name, size_t op_capacity, char *left, size_t left_capacity, char *right, size_t right_capacity) {
    const char *space = strchr(text, ' ');
    const char *comma;

    if (space == NULL) {
        return 0;
    }

    if ((size_t)(space - text) + 1 > op_capacity) {
        return 0;
    }

    memcpy(op_name, text, (size_t)(space - text));
    op_name[space - text] = '\0';

    comma = strchr(space + 1, ',');
    if (comma == NULL) {
        return 0;
    }

    return c_copy_trimmed_segment(left, left_capacity, space + 1, comma) &&
           c_copy_trimmed(right, right_capacity, comma + 1);
}

static int c_split_two_operands(const char *text, char *left, size_t left_capacity, char *right, size_t right_capacity) {
    const char *comma = strchr(text, ',');

    if (comma == NULL) {
        return 0;
    }

    return c_copy_trimmed_segment(left, left_capacity, text, comma) &&
           c_copy_trimmed(right, right_capacity, comma + 1);
}

static int c_split_three_operands(
        const char *text,
        char *first,
        size_t first_capacity,
        char *second,
        size_t second_capacity,
        char *third,
        size_t third_capacity) {
    const char *comma1 = strchr(text, ',');
    const char *comma2;

    if (comma1 == NULL) {
        return 0;
    }

    comma2 = strchr(comma1 + 1, ',');
    if (comma2 == NULL) {
        return 0;
    }

    return c_copy_trimmed_segment(first, first_capacity, text, comma1) &&
           c_copy_trimmed_segment(second, second_capacity, comma1 + 1, comma2) &&
           c_copy_trimmed_segment(third, third_capacity, comma2 + 1, text + strlen(text));
}

static int c_segment_is_blank(const char *start, const char *end) {
    while (start < end) {
        if (*start != ' ' && *start != '\t' && *start != '\n' && *start != '\r') {
            return 0;
        }
        start += 1;
    }

    return 1;
}

static const char *c_find_top_level_comma(const char *start, const char *end) {
    int paren_depth = 0;
    int bracket_depth = 0;
    int brace_depth = 0;
    const char *cursor = start;

    while (cursor < end) {
        if (*cursor == '"') {
            cursor += 1;
            while (cursor < end) {
                if (*cursor == '\\' && cursor + 1 < end) {
                    cursor += 2;
                    continue;
                }
                if (*cursor == '"') {
                    cursor += 1;
                    break;
                }
                cursor += 1;
            }
            continue;
        }

        switch (*cursor) {
            case '(':
                paren_depth += 1;
                break;
            case ')':
                if (paren_depth > 0) {
                    paren_depth -= 1;
                }
                break;
            case '[':
                bracket_depth += 1;
                break;
            case ']':
                if (bracket_depth > 0) {
                    bracket_depth -= 1;
                }
                break;
            case '{':
                brace_depth += 1;
                break;
            case '}':
                if (brace_depth > 0) {
                    brace_depth -= 1;
                }
                break;
            case ',':
                if (paren_depth == 0 && bracket_depth == 0 && brace_depth == 0) {
                    return cursor;
                }
                break;
            default:
                break;
        }

        cursor += 1;
    }

    return NULL;
}

static const char *c_find_top_level_colon(const char *start, const char *end) {
    int paren_depth = 0;
    int bracket_depth = 0;
    int brace_depth = 0;
    const char *cursor = start;

    while (cursor < end) {
        if (*cursor == '"') {
            cursor += 1;
            while (cursor < end) {
                if (*cursor == '\\' && cursor + 1 < end) {
                    cursor += 2;
                    continue;
                }
                if (*cursor == '"') {
                    cursor += 1;
                    break;
                }
                cursor += 1;
            }
            continue;
        }

        switch (*cursor) {
            case '(':
                paren_depth += 1;
                break;
            case ')':
                if (paren_depth > 0) {
                    paren_depth -= 1;
                }
                break;
            case '[':
                bracket_depth += 1;
                break;
            case ']':
                if (bracket_depth > 0) {
                    bracket_depth -= 1;
                }
                break;
            case '{':
                brace_depth += 1;
                break;
            case '}':
                if (brace_depth > 0) {
                    brace_depth -= 1;
                }
                break;
            case ':':
                if (paren_depth == 0 && bracket_depth == 0 && brace_depth == 0) {
                    return cursor;
                }
                break;
            default:
                break;
        }

        cursor += 1;
    }

    return NULL;
}

static const char *c_math_function(const char *op_name) {
    if (strcmp(op_name, "add") == 0) return "zt_add_i64";
    if (strcmp(op_name, "sub") == 0) return "zt_sub_i64";
    if (strcmp(op_name, "mul") == 0) return "zt_mul_i64";
    if (strcmp(op_name, "div") == 0) return "zt_div_i64";
    if (strcmp(op_name, "mod") == 0) return "zt_rem_i64";
    return NULL;
}

static const char *c_binary_operator(const char *op_name) {
    if (strcmp(op_name, "lt") == 0) return "<";
    if (strcmp(op_name, "le") == 0) return "<=";
    if (strcmp(op_name, "gt") == 0) return ">";
    if (strcmp(op_name, "ge") == 0) return ">=";
    if (strcmp(op_name, "eq") == 0) return "==";
    if (strcmp(op_name, "ne") == 0) return "!=";
    if (strcmp(op_name, "and") == 0) return "&&";
    if (strcmp(op_name, "or") == 0) return "||";
    return NULL;
}

static const char *c_binary_operator_suggestion(const char *invalid_op) {
    /* Simple suggestion: return first valid operator */
    (void)invalid_op;
    return "add";  /* Could implement Levenshtein distance in future */
}

static const char *c_unary_operator(const char *op_name) {
    if (strcmp(op_name, "neg") == 0) return "-";
    if (strcmp(op_name, "not") == 0) return "!";
    return NULL;
}

static int c_emit_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const char *expr_text,
        const char *expected_type_name,
        c_emit_result *result);

static void c_copy_legalize_result(c_emit_result *result, const c_legalize_result *legalize_result) {
    if (legalize_result == NULL) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "missing legalization result");
        return;
    }

    switch (legalize_result->code) {
        case C_LEGALIZE_OK:
            c_emit_set_result(result, C_EMIT_OK, "%s", c_safe_text(legalize_result->message));
            return;
        case C_LEGALIZE_INVALID_INPUT:
            c_emit_set_result(result, C_EMIT_INVALID_INPUT, "%s", c_safe_text(legalize_result->message));
            return;
        case C_LEGALIZE_UNSUPPORTED_TYPE:
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "%s", c_safe_text(legalize_result->message));
            return;
        case C_LEGALIZE_UNSUPPORTED_EXPR:
        default:
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "%s", c_safe_text(legalize_result->message));
            return;
    }
}

static int c_emit_legalized_seq_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const c_legalized_seq_expr *legalized,
        c_emit_result *result) {
    const char *arg1_type_name = "int";
    char map_type_name[128];
    char key_type_name[128];
    char value_type_name[128];

    if (legalized == NULL) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "legalized sequence expression cannot be null");
        return 0;
    }

    if (!(c_buffer_append_format(&emitter->buffer, "%s(", c_safe_text(legalized->runtime_name)) &&
            c_emit_expr(emitter, module_decl, function_decl, legalized->sequence_expr, legalized->sequence_type_name, result))) {
        return 0;
    }

    if (legalized->kind == C_LEGALIZED_SEQ_LIST_I64_LEN ||
            legalized->kind == C_LEGALIZED_SEQ_LIST_TEXT_LEN ||
            legalized->kind == C_LEGALIZED_SEQ_MAP_LEN) {
        return c_buffer_append(&emitter->buffer, ")");
    }

    if (c_parse_binary_type_name(
            legalized->sequence_type_name,
            "map<",
            map_type_name,
            sizeof(map_type_name),
            key_type_name,
            sizeof(key_type_name),
            value_type_name,
            sizeof(value_type_name))) {
        arg1_type_name = key_type_name;
    }

    if (!(c_buffer_append(&emitter->buffer, ", ") &&
            c_emit_expr(emitter, module_decl, function_decl, legalized->arg1_expr, arg1_type_name, result))) {
        return 0;
    }

    if (legalized->kind == C_LEGALIZED_SEQ_TEXT_SLICE ||
            legalized->kind == C_LEGALIZED_SEQ_LIST_I64_SLICE ||
            legalized->kind == C_LEGALIZED_SEQ_LIST_TEXT_SLICE) {
        if (!(c_buffer_append(&emitter->buffer, ", ") &&
                c_emit_expr(emitter, module_decl, function_decl, legalized->arg2_expr, "int", result))) {
            return 0;
        }
    }

    return c_buffer_append(&emitter->buffer, ")");
}
static int c_emit_owned_managed_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const char *expr_text,
        const char *expected_type_name,
        c_emit_result *result) {
    char trimmed[256];
    const char *type_name;

    if (!c_copy_trimmed(trimmed, sizeof(trimmed), expr_text)) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "expression is too large for the current emitter");
        return 0;
    }

    if (c_is_identifier_only(trimmed)) {
        type_name = c_find_symbol_type(function_decl, trimmed);
        if (c_type_is_managed(type_name)) {
            return c_buffer_append_format(&emitter->buffer, "(zt_retain(%s), %s)", trimmed, trimmed);
        }
        return c_emit_expr(emitter, module_decl, function_decl, expr_text, expected_type_name, result);
    }

    if (strncmp(trimmed, "get_field ", 10) == 0) {
        char target[128];
        char field[128];

        if (!c_split_two_operands(trimmed + 10, target, sizeof(target), field, sizeof(field))) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid get_field expression '%s'", trimmed);
            return 0;
        }

        if (c_is_identifier_only(target) || strcmp(target, "self") == 0) {
            if (!(c_buffer_append(&emitter->buffer, "(zt_retain(") &&
                  c_emit_expr(emitter, module_decl, function_decl, expr_text, expected_type_name, result) &&
                  c_buffer_append(&emitter->buffer, "), ") &&
                  c_emit_expr(emitter, module_decl, function_decl, expr_text, expected_type_name, result) &&
                  c_buffer_append(&emitter->buffer, ")"))) {
                return 0;
            }
            return 1;
        }
    }

    return c_emit_expr(emitter, module_decl, function_decl, expr_text, expected_type_name, result);
}

static int c_emit_make_list_i64_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const char *expr_text,
        c_emit_result *result) {
    const char *open = strchr(expr_text, '[');
    const char *close = strrchr(expr_text, ']');
    const char *cursor;
    int first = 1;
    size_t item_count = 0;

    if (strncmp(expr_text, "make_list<int>", 14) != 0 || open == NULL || close == NULL || close < open) {
        c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, 
                                 "invalid make_list expression '%s'. Expected format: make_list<T>[item1, item2, ...]", 
                                 c_safe_text(expr_text));
        return 0;
    }

    cursor = open + 1;
    if (c_segment_is_blank(cursor, close)) {
        return c_buffer_append(&emitter->buffer, "zt_list_i64_new()");
    }

    if (!c_buffer_append(&emitter->buffer, "zt_list_i64_from_array(((zt_int[]){")) {
        return 0;
    }

    while (cursor < close) {
        const char *comma = c_find_top_level_comma(cursor, close);
        const char *segment_end = comma != NULL ? comma : close;
        char item[256];

        if (!c_copy_trimmed_segment(item, sizeof(item), cursor, segment_end) || c_is_blank(item)) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, 
                             "invalid make_list item in '%s'. Each item must be a valid expression.", 
                             c_safe_text(expr_text));
            return 0;
        }

        if (!first && !c_buffer_append(&emitter->buffer, ", ")) {
            return 0;
        }

        if (!c_emit_expr(emitter, module_decl, function_decl, item, "int", result)) {
            return 0;
        }

        item_count += 1;
        if (comma == NULL) {
            break;
        }

        first = 0;
        cursor = comma + 1;
    }

    return c_buffer_append_format(&emitter->buffer, "}), %zu)", item_count);
}

static int c_emit_make_list_text_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const char *expr_text,
        c_emit_result *result) {
    const char *open = strchr(expr_text, '[');
    const char *close = strrchr(expr_text, ']');
    const char *cursor;
    int first = 1;
    size_t item_count = 0;

    if (strncmp(expr_text, "make_list<text>", 15) != 0 || open == NULL || close == NULL || close < open) {
        c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, 
                                 "invalid make_list expression '%s'. Expected format: make_list<T>[item1, item2, ...]", 
                                 c_safe_text(expr_text));
        return 0;
    }

    cursor = open + 1;
    if (c_segment_is_blank(cursor, close)) {
        return c_buffer_append(&emitter->buffer, "zt_list_text_new()");
    }

    if (!c_buffer_append(&emitter->buffer, "zt_list_text_from_array(((zt_text *[]){")) {
        return 0;
    }

    while (cursor < close) {
        const char *comma = c_find_top_level_comma(cursor, close);
        const char *segment_end = comma != NULL ? comma : close;
        char item[256];

        if (!c_copy_trimmed_segment(item, sizeof(item), cursor, segment_end) || c_is_blank(item)) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, 
                             "invalid make_list item in '%s'. Each item must be a valid expression.", 
                             c_safe_text(expr_text));
            return 0;
        }

        if (!c_expression_is_materialized_text_ref(function_decl, item)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_EXPR,
                "make_list<text> currently requires text locals or params, got '%s'",
                item
            );
            return 0;
        }

        if (!first && !c_buffer_append(&emitter->buffer, ", ")) {
            return 0;
        }

        if (!c_emit_expr(emitter, module_decl, function_decl, item, "text", result)) {
            return 0;
        }

        item_count += 1;
        if (comma == NULL) {
            break;
        }

        first = 0;
        cursor = comma + 1;
    }

    return c_buffer_append_format(&emitter->buffer, "}), %zu)", item_count);
}

static int c_emit_make_map_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const char *expr_text,
        c_emit_result *result) {
    char make_map_text[128];
    char map_type_name[128];
    char key_type_name[128];
    char value_type_name[128];
    char key_c_type[128];
    char value_c_type[128];
    c_map_spec spec;
    const char *open = strchr(expr_text, '[');
    const char *close = strrchr(expr_text, ']');
    const char *cursor;
    int first = 1;
    size_t item_count = 0;

    if (open == NULL ||
            close == NULL ||
            close < open ||
            !c_copy_trimmed_segment(make_map_text, sizeof(make_map_text), expr_text, open) ||
            !c_parse_binary_type_name(
                make_map_text,
                "make_map<",
                map_type_name,
                sizeof(map_type_name),
                key_type_name,
                sizeof(key_type_name),
                value_type_name,
                sizeof(value_type_name))) {
        c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid make_map expression '%s'", c_safe_text(expr_text));
        return 0;
    }

    snprintf(map_type_name, sizeof(map_type_name), "map<%s,%s>", key_type_name, value_type_name);
    if (!c_map_spec_for_type(module_decl, map_type_name, &spec) ||
            !c_type_to_c(module_decl, key_type_name, key_c_type, sizeof(key_c_type), result) ||
            !c_type_to_c(module_decl, value_type_name, value_c_type, sizeof(value_c_type), result)) {
        c_emit_set_result(
            result,
            C_EMIT_UNSUPPORTED_TYPE,
            "make_map<%s,%s> is not supported by the current C emitter subset",
            key_type_name,
            value_type_name
        );
        return 0;
    }

    cursor = open + 1;
    if (c_segment_is_blank(cursor, close)) {
        return c_buffer_append_format(&emitter->buffer, "%s()", spec.new_fn);
    }

    if (!c_buffer_append_format(&emitter->buffer, "%s(((", spec.from_arrays_fn) ||
            !c_buffer_append(&emitter->buffer, key_c_type) ||
            !c_buffer_append(&emitter->buffer, "[]){")) {
        return 0;
    }

    while (cursor < close) {
        const char *comma = c_find_top_level_comma(cursor, close);
        const char *segment_end = comma != NULL ? comma : close;
        const char *colon = c_find_top_level_colon(cursor, segment_end);
        char key_expr[256];
        char value_expr[256];

        if (colon == NULL ||
                !c_copy_trimmed_segment(key_expr, sizeof(key_expr), cursor, colon) ||
                !c_copy_trimmed_segment(value_expr, sizeof(value_expr), colon + 1, segment_end) ||
                c_is_blank(key_expr) ||
                c_is_blank(value_expr)) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid make_map entry in '%s'", c_safe_text(expr_text));
            return 0;
        }

        if ((c_type_is_managed(key_type_name) &&
                    !c_expression_is_materialized_type_ref(function_decl, key_expr, key_type_name)) ||
                (c_type_is_managed(value_type_name) &&
                    !c_expression_is_materialized_type_ref(function_decl, value_expr, value_type_name))) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_EXPR,
                "make_map<%s,%s> currently requires materialized managed entries, got '%s: %s'",
                key_type_name,
                value_type_name,
                key_expr,
                value_expr
            );
            return 0;
        }

        if (!first && !c_buffer_append(&emitter->buffer, ", ")) {
            return 0;
        }

        if (!c_emit_expr(emitter, module_decl, function_decl, key_expr, key_type_name, result)) {
            return 0;
        }

        item_count += 1;
        if (comma == NULL) {
            break;
        }

        first = 0;
        cursor = comma + 1;
    }

    if (!c_buffer_append(&emitter->buffer, "}), ((") ||
            !c_buffer_append(&emitter->buffer, value_c_type) ||
            !c_buffer_append(&emitter->buffer, "[]){")) {
        return 0;
    }

    cursor = open + 1;
    first = 1;
    while (cursor < close) {
        const char *comma = c_find_top_level_comma(cursor, close);
        const char *segment_end = comma != NULL ? comma : close;
        const char *colon = c_find_top_level_colon(cursor, segment_end);
        char value_expr[256];

        if (colon == NULL ||
                !c_copy_trimmed_segment(value_expr, sizeof(value_expr), colon + 1, segment_end) ||
                c_is_blank(value_expr)) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid make_map entry in '%s'", c_safe_text(expr_text));
            return 0;
        }

        if (!first && !c_buffer_append(&emitter->buffer, ", ")) {
            return 0;
        }

        if (!c_emit_expr(emitter, module_decl, function_decl, value_expr, value_type_name, result)) {
            return 0;
        }

        if (comma == NULL) {
            break;
        }

        first = 0;
        cursor = comma + 1;
    }

    return c_buffer_append_format(&emitter->buffer, "}), %zu)", item_count);
}

static int c_emit_call_args(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const char *callee_text,
        const char *args_text,
        int direct_call,
        c_emit_result *result) {
    const char *cursor = args_text;
    const char *end = args_text + strlen(args_text);
    int first = 1;

    if (c_segment_is_blank(cursor, end)) {
        return 1;
    }

    while (cursor < end) {
        const char *comma = c_find_top_level_comma(cursor, end);
        const char *segment_end = comma != NULL ? comma : end;
        char arg[256];

        if (!c_copy_trimmed_segment(arg, sizeof(arg), cursor, segment_end)) {
            return 0;
        }

        if (!first && !c_buffer_append(&emitter->buffer, ", ")) {
            return 0;
        }

        if (!direct_call && strcmp(callee_text, "c.puts") == 0) {
            if (strncmp(arg, "const \"", 7) == 0) {
                if (!c_buffer_append(&emitter->buffer, arg + 6)) {
                    return 0;
                }
            } else if (c_expression_is_text(function_decl, arg)) {
                if (!(c_buffer_append(&emitter->buffer, "zt_text_data(") &&
                        c_emit_expr(emitter, module_decl, function_decl, arg, "text", result) &&
                        c_buffer_append(&emitter->buffer, ")"))) {
                    return 0;
                }
            } else if (!c_emit_expr(emitter, module_decl, function_decl, arg, NULL, result)) {
                return 0;
            }
        } else if (!c_emit_expr(emitter, module_decl, function_decl, arg, NULL, result)) {
            return 0;
        }

        if (comma == NULL) {
            break;
        }

        first = 0;
        cursor = comma + 1;
    }

    return 1;
}

static int c_emit_call_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const char *callee_text,
        const char *args_text,
        int direct_call,
        c_emit_result *result) {
    char callee[256];
    char mangled[256];
    const char *call_name = NULL;

    if (!c_copy_trimmed(callee, sizeof(callee), callee_text)) {
        return 0;
    }

    if (direct_call) {
        zir_function pseudo_function;
        pseudo_function.name = callee;
        c_build_function_symbol(module_decl, &pseudo_function, mangled, sizeof(mangled));
        call_name = mangled;
    } else if (strncmp(callee, "c.", 2) == 0) {
        call_name = callee + 2;
    } else {
        c_emit_set_result(
            result,
            C_EMIT_UNSUPPORTED_EXPR,
            "extern call '%s' is not supported by the current C emitter subset",
            callee
        );
        return 0;
    }

    return c_buffer_append_format(&emitter->buffer, "%s(", call_name) &&
           c_emit_call_args(emitter, module_decl, function_decl, callee, args_text, direct_call, result) &&
           c_buffer_append(&emitter->buffer, ")");
}

static int c_emit_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const char *expr_text,
        const char *expected_type_name,
        c_emit_result *result) {
    char trimmed[256];

    if (!c_copy_trimmed(trimmed, sizeof(trimmed), expr_text)) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "expression is too large for the current emitter");
        return 0;
    }

    if (strncmp(trimmed, "const ", 6) == 0) {
        if (c_type_is(expected_type_name, "text")) {
            return c_buffer_append_format(&emitter->buffer, "zt_text_from_utf8_literal(%s)", trimmed + 6);
        }
        return c_emit_value(emitter, trimmed + 6);
    }

    if (strncmp(trimmed, "make_list<int>", 14) == 0) {
        if (!c_is_blank(expected_type_name) && !c_type_is(expected_type_name, "list<int>")) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_TYPE,
                "make_list<int> produces list<int>, but the expected type is '%s'",
                c_safe_text(expected_type_name)
            );
            return 0;
        }

        return c_emit_make_list_i64_expr(emitter, module_decl, function_decl, trimmed, result);
    }

    if (strncmp(trimmed, "make_list<text>", 15) == 0) {
        if (!c_is_blank(expected_type_name) && !c_type_is(expected_type_name, "list<text>")) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_TYPE,
                "make_list<text> produces list<text>, but the expected type is '%s'",
                c_safe_text(expected_type_name)
            );
            return 0;
        }

        return c_emit_make_list_text_expr(emitter, module_decl, function_decl, trimmed, result);
    }

    if (strncmp(trimmed, "make_map<", 9) == 0) {
        char declared_map_type[128];
        char key_type_name[128];
        char value_type_name[128];
        const char *open = strchr(trimmed, '[');

        if (open == NULL ||
                !c_copy_trimmed_segment(declared_map_type, sizeof(declared_map_type), trimmed, open) ||
                !c_parse_binary_type_name(
                    declared_map_type,
                    "make_map<",
                    declared_map_type,
                    sizeof(declared_map_type),
                    key_type_name,
                    sizeof(key_type_name),
                    value_type_name,
                    sizeof(value_type_name))) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid make_map expression '%s'", c_safe_text(trimmed));
            return 0;
        }

        snprintf(declared_map_type, sizeof(declared_map_type), "map<%s,%s>", key_type_name, value_type_name);
        if (!c_is_blank(expected_type_name) && !c_type_is(expected_type_name, declared_map_type)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_TYPE,
                "%s produces %s, but the expected type is '%s'",
                c_safe_text(trimmed),
                declared_map_type,
                c_safe_text(expected_type_name)
            );
            return 0;
        }

        return c_emit_make_map_expr(emitter, module_decl, function_decl, trimmed, result);
    }

    if (strcmp(trimmed, "optional_empty<int>") == 0) {
        if (!c_is_blank(expected_type_name) && !c_type_is(expected_type_name, "optional<int>")) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_TYPE,
                "optional_empty<int> produces Optional<int>, but the expected type is '%s'",
                c_safe_text(expected_type_name)
            );
            return 0;
        }

        return c_buffer_append(&emitter->buffer, "zt_optional_i64_empty()");
    }

    if (strcmp(trimmed, "optional_empty<text>") == 0) {
        if (!c_is_blank(expected_type_name) && !c_type_is(expected_type_name, "optional<text>")) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_TYPE,
                "optional_empty<text> produces Optional<text>, but the expected type is '%s'",
                c_safe_text(expected_type_name)
            );
            return 0;
        }

        return c_buffer_append(&emitter->buffer, "zt_optional_text_empty()");
    }

    if (strcmp(trimmed, "optional_empty<list<int>>") == 0) {
        if (!c_is_blank(expected_type_name) && !c_type_is(expected_type_name, "optional<list<int>>")) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_TYPE,
                "optional_empty<list<int>> produces Optional<list<int>>, but the expected type is '%s'",
                c_safe_text(expected_type_name)
            );
            return 0;
        }

        return c_buffer_append(&emitter->buffer, "zt_optional_list_i64_empty()");
    }

    if (strncmp(trimmed, "optional_present ", 17) == 0) {
        const char *present_expr = trimmed + 17;

        if (c_type_is(expected_type_name, "optional<list<int>>")) {
            if (!c_expression_is_materialized_list_i64_ref(function_decl, present_expr)) {
                c_emit_set_result(
                    result,
                    C_EMIT_UNSUPPORTED_EXPR,
                    "optional_present for Optional<list<int>> currently requires a materialized list<int> value, got '%s'",
                    present_expr
                );
                return 0;
            }

            return c_buffer_append(&emitter->buffer, "zt_optional_list_i64_present(") &&
                   c_emit_expr(emitter, module_decl, function_decl, present_expr, "list<int>", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (c_type_is(expected_type_name, "optional<bytes>")) {
            if (!c_expression_is_materialized_bytes_ref(function_decl, present_expr)) {
                c_emit_set_result(
                    result,
                    C_EMIT_UNSUPPORTED_EXPR,
                    "optional_present for Optional<bytes> currently requires a materialized bytes value, got '%s'",
                    present_expr
                );
                return 0;
            }

            return c_buffer_append(&emitter->buffer, "zt_optional_bytes_present(") &&
                   c_emit_expr(emitter, module_decl, function_decl, present_expr, "bytes", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (c_type_is(expected_type_name, "optional<text>")) {
            if (!c_expression_is_materialized_text_ref(function_decl, present_expr)) {
                c_emit_set_result(
                    result,
                    C_EMIT_UNSUPPORTED_EXPR,
                    "optional_present for Optional<text> currently requires a materialized text value, got '%s'",
                    present_expr
                );
                return 0;
            }

            return c_buffer_append(&emitter->buffer, "zt_optional_text_present(") &&
                   c_emit_expr(emitter, module_decl, function_decl, present_expr, "text", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (!c_is_blank(expected_type_name) &&
                !c_type_is(expected_type_name, "optional<int>") &&
                !c_type_is(expected_type_name, "optional<text>") &&
                !c_type_is(expected_type_name, "optional<list<int>>")) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_TYPE,
                "optional_present produces Optional<int>, Optional<bytes>, Optional<text> or Optional<list<int>>, but the expected type is '%s'",
                c_safe_text(expected_type_name)
            );
            return 0;
        }

        return c_buffer_append(&emitter->buffer, "zt_optional_i64_present(") &&
               c_emit_expr(emitter, module_decl, function_decl, present_expr, "int", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (strncmp(trimmed, "optional_is_present ", 20) == 0) {
        const char *optional_expr = trimmed + 20;

        if (c_expression_is_materialized_optional_list_i64_ref(function_decl, optional_expr)) {
            return c_buffer_append(&emitter->buffer, "zt_optional_list_i64_is_present(") &&
                   c_emit_expr(emitter, module_decl, function_decl, optional_expr, "Optional<list<int>>", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (c_expression_is_materialized_optional_bytes_ref(function_decl, optional_expr)) {
            return c_buffer_append(&emitter->buffer, "zt_optional_bytes_is_present(") &&
                   c_emit_expr(emitter, module_decl, function_decl, optional_expr, "Optional<bytes>", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (c_expression_is_materialized_optional_text_ref(function_decl, optional_expr)) {
            return c_buffer_append(&emitter->buffer, "zt_optional_text_is_present(") &&
                   c_emit_expr(emitter, module_decl, function_decl, optional_expr, "Optional<text>", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        return c_buffer_append(&emitter->buffer, "zt_optional_i64_is_present(") &&
               c_emit_expr(emitter, module_decl, function_decl, optional_expr, "Optional<int>", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (strncmp(trimmed, "optional_value ", 15) == 0) {
        const char *optional_expr = trimmed + 15;

        if (c_expression_is_materialized_optional_list_i64_ref(function_decl, optional_expr)) {
            return c_buffer_append(&emitter->buffer, "zt_optional_list_i64_value(") &&
                   c_emit_expr(emitter, module_decl, function_decl, optional_expr, "Optional<list<int>>", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (c_expression_is_materialized_optional_bytes_ref(function_decl, optional_expr)) {
            return c_buffer_append(&emitter->buffer, "zt_optional_bytes_value(") &&
                   c_emit_expr(emitter, module_decl, function_decl, optional_expr, "Optional<bytes>", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (c_expression_is_materialized_optional_text_ref(function_decl, optional_expr)) {
            return c_buffer_append(&emitter->buffer, "zt_optional_text_value(") &&
                   c_emit_expr(emitter, module_decl, function_decl, optional_expr, "Optional<text>", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        return c_buffer_append(&emitter->buffer, "zt_optional_i64_value(") &&
               c_emit_expr(emitter, module_decl, function_decl, optional_expr, "Optional<int>", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (strncmp(trimmed, "coalesce ", 9) == 0) {
        char optional_value[128];
        char fallback_value[128];

        if (!c_split_two_operands(trimmed + 9, optional_value, sizeof(optional_value), fallback_value, sizeof(fallback_value))) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid coalesce expression '%s'", trimmed);
            return 0;
        }

        if (c_type_is(expected_type_name, "list<int>") || c_expression_is_materialized_optional_list_i64_ref(function_decl, optional_value)) {
            if (!c_expression_is_materialized_optional_list_i64_ref(function_decl, optional_value)) {
                c_emit_set_result(
                    result,
                    C_EMIT_UNSUPPORTED_EXPR,
                    "coalesce for Optional<list<int>> currently requires a materialized optional value, got '%s'",
                    optional_value
                );
                return 0;
            }

            if (!c_expression_is_materialized_list_i64_ref(function_decl, fallback_value)) {
                c_emit_set_result(
                    result,
                    C_EMIT_UNSUPPORTED_EXPR,
                    "coalesce for Optional<list<int>> currently requires a materialized fallback list<int>, got '%s'",
                    fallback_value
                );
                return 0;
            }

            return c_buffer_append(&emitter->buffer, "zt_optional_list_i64_coalesce(") &&
                   c_emit_expr(emitter, module_decl, function_decl, optional_value, "Optional<list<int>>", result) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_expr(emitter, module_decl, function_decl, fallback_value, "list<int>", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (c_type_is(expected_type_name, "bytes") || c_expression_is_materialized_optional_bytes_ref(function_decl, optional_value)) {
            if (!c_expression_is_materialized_optional_bytes_ref(function_decl, optional_value)) {
                c_emit_set_result(
                    result,
                    C_EMIT_UNSUPPORTED_EXPR,
                    "coalesce for Optional<bytes> currently requires a materialized optional value, got '%s'",
                    optional_value
                );
                return 0;
            }

            if (!c_expression_is_materialized_bytes_ref(function_decl, fallback_value)) {
                c_emit_set_result(
                    result,
                    C_EMIT_UNSUPPORTED_EXPR,
                    "coalesce for Optional<bytes> currently requires a materialized fallback bytes, got '%s'",
                    fallback_value
                );
                return 0;
            }

            return c_buffer_append(&emitter->buffer, "zt_optional_bytes_coalesce(") &&
                   c_emit_expr(emitter, module_decl, function_decl, optional_value, "Optional<bytes>", result) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_expr(emitter, module_decl, function_decl, fallback_value, "bytes", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (c_type_is(expected_type_name, "text") || c_expression_is_materialized_optional_text_ref(function_decl, optional_value)) {
            if (!c_expression_is_materialized_optional_text_ref(function_decl, optional_value)) {
                c_emit_set_result(
                    result,
                    C_EMIT_UNSUPPORTED_EXPR,
                    "coalesce for Optional<text> currently requires a materialized optional value, got '%s'",
                    optional_value
                );
                return 0;
            }

            if (!c_expression_is_materialized_text_ref(function_decl, fallback_value)) {
                c_emit_set_result(
                    result,
                    C_EMIT_UNSUPPORTED_EXPR,
                    "coalesce for Optional<text> currently requires a materialized fallback text, got '%s'",
                    fallback_value
                );
                return 0;
            }

            return c_buffer_append(&emitter->buffer, "zt_optional_text_coalesce(") &&
                   c_emit_expr(emitter, module_decl, function_decl, optional_value, "Optional<text>", result) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_expr(emitter, module_decl, function_decl, fallback_value, "text", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        return c_buffer_append(&emitter->buffer, "zt_optional_i64_coalesce(") &&
               c_emit_expr(emitter, module_decl, function_decl, optional_value, "Optional<int>", result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_expr(emitter, module_decl, function_decl, fallback_value, "int", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (strcmp(trimmed, "outcome_success") == 0) {
        c_outcome_spec spec;

        if (!c_outcome_spec_for_expected(expected_type_name, 0, &spec) || spec.has_value) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_TYPE,
                "outcome_success without a value produces Outcome<void,E>, but the expected type is '%s'",
                c_safe_text(expected_type_name)
            );
            return 0;
        }

        return c_buffer_append_format(&emitter->buffer, "%s()", spec.success_fn);
    }

    if (strncmp(trimmed, "outcome_success ", 16) == 0) {
        const char *value_expr = trimmed + 16;
        c_outcome_spec spec;

        if (!c_outcome_spec_for_expected(expected_type_name, 1, &spec) || !spec.has_value) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_TYPE,
                "outcome_success with a value requires Outcome<T,E>, but the expected type is '%s'",
                c_safe_text(expected_type_name)
            );
            return 0;
        }

        if (!c_outcome_success_value_is_supported(module_decl, function_decl, &spec, value_expr)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_EXPR,
                "outcome_success for %s currently requires a materialized %s value, got '%s'",
                spec.display_name,
                spec.value_type_name,
                value_expr
            );
            return 0;
        }

        return c_buffer_append_format(&emitter->buffer, "%s(", spec.success_fn) &&
               c_emit_expr(emitter, module_decl, function_decl, value_expr, spec.value_type_name, result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (strncmp(trimmed, "outcome_failure ", 16) == 0) {
        const char *error_expr = trimmed + 16;
        c_outcome_spec spec;
        char error_trimmed[256];

        if (!c_outcome_spec_for_expected(expected_type_name, 1, &spec)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_TYPE,
                "outcome_failure requires Outcome<T,E>, but the expected type is '%s'",
                c_safe_text(expected_type_name)
            );
            return 0;
        }

        if (!c_copy_trimmed(error_trimmed, sizeof(error_trimmed), error_expr)) {
            c_emit_set_result(result, C_EMIT_INVALID_INPUT, "outcome_failure expression is too large");
            return 0;
        }

        if (strncmp(error_trimmed, "const ", 6) == 0) {
            if (spec.failure_message_fn[0] == 0) {
                c_emit_set_result(
                    result,
                    C_EMIT_UNSUPPORTED_EXPR,
                    "outcome_failure with string literal is supported only when E is text or core.Error, got '%s'",
                    spec.display_name
                );
                return 0;
            }
            return c_buffer_append_format(&emitter->buffer, "%s(%s)", spec.failure_message_fn, error_trimmed + 6);
        }

        return c_outcome_emit_failure_from_error_expr(emitter, module_decl, function_decl, &spec, error_expr, result);
    }

    if (strncmp(trimmed, "outcome_is_success ", 19) == 0) {
        const char *outcome_expr = trimmed + 19;
        c_outcome_spec spec;

        if (!c_outcome_spec_for_expr(function_decl, outcome_expr, &spec)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_EXPR,
                "outcome_is_success currently requires a materialized supported outcome value, got '%s'",
                outcome_expr
            );
            return 0;
        }

        return c_buffer_append_format(&emitter->buffer, "%s(", spec.is_success_fn) &&
               c_emit_expr(emitter, module_decl, function_decl, outcome_expr, spec.display_name, result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (strncmp(trimmed, "outcome_value ", 14) == 0) {
        const char *outcome_expr = trimmed + 14;
        c_outcome_spec spec;

        if (!c_outcome_spec_for_expr(function_decl, outcome_expr, &spec)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_EXPR,
                "outcome_value currently requires a materialized supported outcome value, got '%s'",
                outcome_expr
            );
            return 0;
        }

        if (!spec.has_value || spec.value_fn[0] == 0) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "outcome_value is not valid for %s", spec.display_name);
            return 0;
        }

        return c_buffer_append_format(&emitter->buffer, "%s(", spec.value_fn) &&
               c_emit_expr(emitter, module_decl, function_decl, outcome_expr, spec.display_name, result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (strncmp(trimmed, "try_propagate ", 14) == 0) {
        const char *outcome_expr = trimmed + 14;
        c_outcome_spec expected_spec;
        c_outcome_spec source_spec;

        if (!c_outcome_spec_for_expected(expected_type_name, 1, &expected_spec)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_TYPE,
                "try_propagate requires Outcome<T,E>, but the expected type is '%s'",
                c_safe_text(expected_type_name)
            );
            return 0;
        }

        if (!c_outcome_spec_for_expr(function_decl, outcome_expr, &source_spec)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_EXPR,
                "try_propagate currently requires a materialized supported outcome value, got '%s'",
                outcome_expr
            );
            return 0;
        }

        if (strcmp(expected_spec.error_type_name, source_spec.error_type_name) != 0) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_EXPR,
                "try_propagate cannot convert %s into %s because the error types differ",
                source_spec.display_name,
                expected_spec.display_name
            );
            return 0;
        }

        if (strcmp(expected_spec.canonical_name, source_spec.canonical_name) == 0) {
            return c_buffer_append_format(&emitter->buffer, "%s(", source_spec.propagate_fn) &&
                   c_emit_expr(emitter, module_decl, function_decl, outcome_expr, source_spec.display_name, result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        return c_buffer_append_format(&emitter->buffer, "%s((", expected_spec.failure_fn) &&
               c_emit_expr(emitter, module_decl, function_decl, outcome_expr, source_spec.display_name, result) &&
               c_buffer_append(&emitter->buffer, ").error)");
    }
    if (strncmp(trimmed, "list_len ", 9) == 0 || strncmp(trimmed, "map_len ", 8) == 0) {
        c_legalized_seq_expr legalized;
        c_legalize_result legalize_result;

        c_legalize_result_init(&legalize_result);
        if (!c_legalize_list_len_expr(function_decl, trimmed, expected_type_name, &legalized, &legalize_result)) {
            c_copy_legalize_result(result, &legalize_result);
            return 0;
        }

        return c_emit_legalized_seq_expr(emitter, module_decl, function_decl, &legalized, result);
    }
    if (strncmp(trimmed, "binary.", 7) == 0) {
        char op_name[64];
        char left[128];
        char right[128];
        const char *c_op;

        if (!c_parse_binary(trimmed + 7, op_name, sizeof(op_name), left, sizeof(left), right, sizeof(right))) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid binary expression '%s'", trimmed);
            return 0;
        }

        c_op = c_math_function(op_name);
        if (c_op != NULL) {
            return c_buffer_append_format(&emitter->buffer, "%s(", c_op) &&
                   c_emit_value(emitter, left) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_value(emitter, right) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        c_op = c_binary_operator(op_name);
        if (c_op == NULL) {
            const char *suggestion = c_binary_operator_suggestion(op_name);
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, 
                             "binary operation '%s' is not recognized. "
                             "Valid operations: add, sub, mul, div, mod, lt, le, gt, ge, eq, ne, and, or. "
                             "Did you mean '%s'?", op_name, suggestion);
            return 0;
        }

        return c_buffer_append(&emitter->buffer, "(") &&
               c_emit_value(emitter, left) &&
               c_buffer_append_format(&emitter->buffer, " %s ", c_op) &&
               c_emit_value(emitter, right) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (strncmp(trimmed, "unary.", 6) == 0) {
        const char *space = strchr(trimmed + 6, ' ');
        char op_name[64];
        char operand[128];
        const char *c_op;

        if (space == NULL) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid unary expression '%s'", trimmed);
            return 0;
        }

        if ((size_t)(space - (trimmed + 6)) + 1 > sizeof(op_name)) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "unary expression '%s' is too large", trimmed);
            return 0;
        }

        memcpy(op_name, trimmed + 6, (size_t)(space - (trimmed + 6)));
        op_name[space - (trimmed + 6)] = '\0';
        if (!c_copy_trimmed(operand, sizeof(operand), space + 1)) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid unary operand '%s'", trimmed);
            return 0;
        }

        c_op = c_unary_operator(op_name);
        if (c_op == NULL) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "unary operation '%s' is not supported yet", op_name);
            return 0;
        }

        return c_buffer_append_format(&emitter->buffer, "(%s", c_op) &&
               c_emit_value(emitter, operand) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (strncmp(trimmed, "index_seq ", 10) == 0 || strncmp(trimmed, "slice_seq ", 10) == 0) {
        c_legalized_seq_expr legalized;
        c_legalize_result legalize_result;

        c_legalize_result_init(&legalize_result);
        if (!c_legalize_seq_expr(function_decl, trimmed, expected_type_name, &legalized, &legalize_result)) {
            c_copy_legalize_result(result, &legalize_result);
            return 0;
        }

        return c_emit_legalized_seq_expr(emitter, module_decl, function_decl, &legalized, result);
    }

    if (strncmp(trimmed, "call_extern ", 12) == 0 || strncmp(trimmed, "call_direct ", 12) == 0) {
        const int direct_call = trimmed[5] == 'd';
        const char *call_body = trimmed + 12;
        const char *open = strchr(call_body, '(');
        const char *close = strrchr(call_body, ')');
        
        /* Use heap conditional for large buffers */
        char stack_callee[128];
        char stack_args[192];
        char *callee = c_emitter_alloc_buffer(sizeof(stack_callee), stack_callee, sizeof(stack_callee));
        char *args = c_emitter_alloc_buffer(sizeof(stack_args), stack_args, sizeof(stack_args));

        if (open == NULL || close == NULL || close < open) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid call expression '%s'", trimmed);
            c_emitter_free_buffer(callee, stack_callee);
            c_emitter_free_buffer(args, stack_args);
            return 0;
        }

        if ((size_t)(open - call_body) + 1 > sizeof(stack_callee) ||
                (size_t)(close - (open + 1)) + 1 > sizeof(stack_args)) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "call expression '%s' is too large", trimmed);
            c_emitter_free_buffer(callee, stack_callee);
            c_emitter_free_buffer(args, stack_args);
            return 0;
        }

        memcpy(callee, call_body, (size_t)(open - call_body));
        callee[open - call_body] = '\0';
        memcpy(args, open + 1, (size_t)(close - (open + 1)));
        args[close - (open + 1)] = '\0';

        int result_ok = c_emit_call_expr(emitter, module_decl, function_decl, callee, args, direct_call, result);
        
        c_emitter_free_buffer(callee, stack_callee);
        c_emitter_free_buffer(args, stack_args);
        
        return result_ok;
    }

    if (strncmp(trimmed, "get_field ", 10) == 0) {
        char target[128];
        char field[128];

        if (!c_split_two_operands(trimmed + 10, target, sizeof(target), field, sizeof(field))) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid get_field expression '%s'", trimmed);
            return 0;
        }

        return c_buffer_append(&emitter->buffer, "(") &&
               c_emit_value(emitter, target) &&
               c_buffer_append(&emitter->buffer, ".") &&
               c_buffer_append(&emitter->buffer, field) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (strchr(trimmed, ' ') == NULL) {
        return c_emit_value(emitter, trimmed);
    }

    c_emit_set_result(
        result,
        C_EMIT_UNSUPPORTED_EXPR,
        "expression '%s' is not recognized. Valid expressions include: literals (int, float, bool, text), identifiers, binary/unary operations, function calls, make_list, make_map, optional, outcome, try_propagate.",
        trimmed
    );
    return 0;
}

static int c_emit_owned_managed_zir_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        const char *expected_type_name,
        c_emit_result *result);

static int c_emit_zir_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        const char *expected_type_name,
        c_emit_result *result);

static int c_emit_zir_expr_as_legacy(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        const char *expected_type_name,
        c_emit_result *result) {
    char *rendered;
    int ok;

    rendered = zir_expr_render_alloc(expr);
    if (rendered == NULL) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "unable to render structured ZIR expression fallback");
        return 0;
    }

    ok = c_emit_expr(emitter, module_decl, function_decl, rendered, expected_type_name, result);
    free(rendered);
    return ok;
}

static int c_zir_expr_name_text(const zir_expr *expr, char *dest, size_t capacity) {
    if (expr == NULL || expr->kind != ZIR_EXPR_NAME || capacity == 0) {
        return 0;
    }

    snprintf(dest, capacity, "%s", c_safe_text(expr->as.text.text));
    return dest[0] != '\0';
}

static int c_zir_expr_resolve_type(
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        char *dest,
        size_t capacity) {
    const char *type_name;

    if (dest == NULL || capacity == 0 || expr == NULL) {
        return 0;
    }

    dest[0] = '\0';

    switch (expr->kind) {
        case ZIR_EXPR_NAME:
            type_name = c_find_symbol_type(function_decl, expr->as.text.text);
            if (type_name == NULL) return 0;
            snprintf(dest, capacity, "%s", type_name);
            return 1;

        case ZIR_EXPR_INT:
            snprintf(dest, capacity, "int");
            return 1;

        case ZIR_EXPR_FLOAT:
            snprintf(dest, capacity, "float");
            return 1;

        case ZIR_EXPR_BOOL:
            snprintf(dest, capacity, "bool");
            return 1;

        case ZIR_EXPR_STRING:
            snprintf(dest, capacity, "text");
            return 1;

        case ZIR_EXPR_BYTES:
            snprintf(dest, capacity, "bytes");
            return 1;

        case ZIR_EXPR_COPY:
            return c_zir_expr_resolve_type(module_decl, function_decl, expr->as.single.value, dest, capacity);

        case ZIR_EXPR_MAKE_LIST:
            snprintf(dest, capacity, "list<%s>", c_safe_text(expr->as.make_list.item_type_name));
            return 1;

        case ZIR_EXPR_MAKE_MAP:
            snprintf(
                dest,
                capacity,
                "map<%s,%s>",
                c_safe_text(expr->as.make_map.key_type_name),
                c_safe_text(expr->as.make_map.value_type_name)
            );
            return 1;

        case ZIR_EXPR_OPTIONAL_EMPTY:
            snprintf(dest, capacity, "optional<%s>", c_safe_text(expr->as.type_only.type_name));
            return 1;

        case ZIR_EXPR_OPTIONAL_PRESENT: {
            char inner_type[96];
            if (!c_zir_expr_resolve_type(module_decl, function_decl, expr->as.single.value, inner_type, sizeof(inner_type))) return 0;
            snprintf(dest, capacity, "optional<%s>", inner_type);
            return 1;
        }

        case ZIR_EXPR_MAKE_STRUCT:
            snprintf(dest, capacity, "%s", c_safe_text(expr->as.make_struct.type_name));
            return 1;

        case ZIR_EXPR_GET_FIELD: {
            char object_type[96];
            const zir_struct_decl *struct_decl;
            const zir_field_decl *field_decl;
            const zir_enum_decl *enum_decl;
            const char *resolved_field_type = NULL;
            size_t variant_index;

            if (!c_zir_expr_resolve_type(module_decl, function_decl, expr->as.field.object, object_type, sizeof(object_type))) {
                return 0;
            }

            if (c_type_is(object_type, "core.Error")) {
                const char *field_name = c_safe_text(expr->as.field.field_name);
                if (strcmp(field_name, "code") == 0 || strcmp(field_name, "message") == 0) {
                    snprintf(dest, capacity, "text");
                    return 1;
                }
                if (strcmp(field_name, "context") == 0) {
                    snprintf(dest, capacity, "optional<text>");
                    return 1;
                }
                return 0;
            }

            struct_decl = c_find_user_struct(module_decl, object_type);
            field_decl = c_find_struct_field(struct_decl, expr->as.field.field_name);
            if (field_decl != NULL) {
                snprintf(dest, capacity, "%s", c_safe_text(field_decl->type_name));
                return 1;
            }

            enum_decl = c_find_user_enum(module_decl, object_type);
            if (enum_decl == NULL) {
                return 0;
            }

            if (strcmp(c_safe_text(expr->as.field.field_name), "__zt_enum_tag") == 0) {
                snprintf(dest, capacity, "int");
                return 1;
            }

            for (variant_index = 0; variant_index < enum_decl->variant_count; variant_index += 1) {
                const zir_enum_variant_decl *variant = &enum_decl->variants[variant_index];
                const zir_enum_variant_field_decl *variant_field = c_find_enum_variant_field(variant, expr->as.field.field_name);
                if (variant_field == NULL) continue;
                if (resolved_field_type == NULL) {
                    resolved_field_type = variant_field->type_name;
                } else if (!c_type_is(resolved_field_type, variant_field->type_name)) {
                    return 0;
                }
            }

            if (resolved_field_type == NULL) {
                return 0;
            }

            snprintf(dest, capacity, "%s", c_safe_text(resolved_field_type));
            return 1;
        }

        case ZIR_EXPR_INDEX_SEQ: {
            char sequence_type[96];
            char map_type_name[96];
            char key_type_name[96];
            char value_type_name[96];

            if (!c_zir_expr_resolve_type(module_decl, function_decl, expr->as.sequence.first, sequence_type, sizeof(sequence_type))) {
                return 0;
            }

            if (c_type_is(sequence_type, "text")) {
                snprintf(dest, capacity, "text");
                return 1;
            }
            if (c_type_is(sequence_type, "bytes")) {
                snprintf(dest, capacity, "uint8");
                return 1;
            }
            if (c_type_is(sequence_type, "list<int>")) {
                snprintf(dest, capacity, "int");
                return 1;
            }
            if (c_type_is(sequence_type, "list<text>")) {
                snprintf(dest, capacity, "text");
                return 1;
            }
            if (c_type_is(sequence_type, "list<dyn<textrepresentable>>")) {
                snprintf(dest, capacity, "dyn<textrepresentable>");
                return 1;
            }
            if (c_parse_binary_type_name(
                    sequence_type,
                    "map<",
                    map_type_name,
                    sizeof(map_type_name),
                    key_type_name,
                    sizeof(key_type_name),
                    value_type_name,
                    sizeof(value_type_name))) {
                snprintf(dest, capacity, "%s", value_type_name);
                return 1;
            }
            return 0;
        }

        case ZIR_EXPR_SLICE_SEQ: {
            char sequence_type[96];

            if (!c_zir_expr_resolve_type(module_decl, function_decl, expr->as.sequence.first, sequence_type, sizeof(sequence_type))) {
                return 0;
            }

            if (c_type_is(sequence_type, "text")) {
                snprintf(dest, capacity, "text");
                return 1;
            }
            if (c_type_is(sequence_type, "bytes")) {
                snprintf(dest, capacity, "bytes");
                return 1;
            }
            if (c_type_is(sequence_type, "list<int>")) {
                snprintf(dest, capacity, "list<int>");
                return 1;
            }
            if (c_type_is(sequence_type, "list<text>")) {
                snprintf(dest, capacity, "list<text>");
                return 1;
            }
            if (c_type_is(sequence_type, "list<dyn<textrepresentable>>")) {
                snprintf(dest, capacity, "list<dyn<textrepresentable>>");
                return 1;
            }
            return 0;
        }

        case ZIR_EXPR_LIST_LEN:
        case ZIR_EXPR_MAP_LEN:
            snprintf(dest, capacity, "int");
            return 1;

        default:
            return 0;
    }
}

static int c_zir_expr_is_text(const zir_function *function_decl, const zir_expr *expr) {
    char type_name[96];

    if (expr == NULL) return 0;
    if (expr->kind == ZIR_EXPR_STRING) return 1;
    return c_zir_expr_resolve_type(NULL, function_decl, expr, type_name, sizeof(type_name)) &&
           c_type_is(type_name, "text");
}

static int c_zir_expr_is_materialized_text_ref(const zir_function *function_decl, const zir_expr *expr) {
    char name[128];
    const char *type_name;

    if (!c_zir_expr_name_text(expr, name, sizeof(name))) return 0;
    type_name = c_find_symbol_type(function_decl, name);
    return c_type_is(type_name, "text");
}

static int c_zir_expr_is_materialized_bytes_ref(const zir_function *function_decl, const zir_expr *expr) {
    char name[128];
    const char *type_name;

    if (!c_zir_expr_name_text(expr, name, sizeof(name))) return 0;
    type_name = c_find_symbol_type(function_decl, name);
    return c_type_is(type_name, "bytes");
}

static int c_zir_expr_is_materialized_list_i64_ref(const zir_function *function_decl, const zir_expr *expr) {
    char name[128];
    const char *type_name;

    if (!c_zir_expr_name_text(expr, name, sizeof(name))) return 0;
    type_name = c_find_symbol_type(function_decl, name);
    return c_type_is(type_name, "list<int>");
}

static int c_zir_expr_is_materialized_type_ref(
        const zir_function *function_decl,
        const zir_expr *expr,
        const char *expected_type_name) {
    char name[128];
    const char *type_name;

    if (!c_zir_expr_name_text(expr, name, sizeof(name))) return 0;
    type_name = c_find_symbol_type(function_decl, name);
    return c_type_is(type_name, expected_type_name);
}

static int c_zir_expr_is_materialized_optional_text_ref(const zir_function *function_decl, const zir_expr *expr) {
    char name[128];
    const char *type_name;

    if (!c_zir_expr_name_text(expr, name, sizeof(name))) return 0;
    type_name = c_find_symbol_type(function_decl, name);
    return c_type_is(type_name, "optional<text>");
}

static int c_zir_expr_is_materialized_optional_bytes_ref(const zir_function *function_decl, const zir_expr *expr) {
    char name[128];
    const char *type_name;

    if (!c_zir_expr_name_text(expr, name, sizeof(name))) return 0;
    type_name = c_find_symbol_type(function_decl, name);
    return c_type_is(type_name, "optional<bytes>");
}
static int c_zir_expr_is_materialized_optional_list_i64_ref(const zir_function *function_decl, const zir_expr *expr) {
    char name[128];
    const char *type_name;

    if (!c_zir_expr_name_text(expr, name, sizeof(name))) return 0;
    type_name = c_find_symbol_type(function_decl, name);
    return c_type_is(type_name, "optional<list<int>>");
}

static const char *c_extern_call_expected_arg_type(const char *callee, size_t index);

static int c_emit_zir_call_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        c_emit_result *result) {
    /* Use heap-allocated buffers for ZIR call expressions */
    char *callee = (char *)malloc(256);
    char *mangled = NULL;
    const char *call_name;
    const zir_function *callee_function = NULL;
    int direct_call;
    size_t index;
    int ok = 1;

    if (expr == NULL || callee == NULL) {
        if (callee == NULL) {
            c_emit_set_result(result, C_EMIT_INVALID_INPUT, "failed to allocate memory for ZIR call expression");
        } else {
            c_emit_set_result(result, C_EMIT_INVALID_INPUT, "call expression cannot be null");
        }
        if (callee) free(callee);
        return 0;
    }

    snprintf(callee, 256, "%s", c_safe_text(expr->as.call.callee_name));
    direct_call = expr->kind == ZIR_EXPR_CALL_DIRECT;

    if (direct_call) {
        zir_function pseudo_function;
        memset(&pseudo_function, 0, sizeof(pseudo_function));
        pseudo_function.name = callee;
        callee_function = c_find_function_decl(module_decl, callee);
        mangled = (char *)malloc(256);
        if (mangled == NULL) {
            free(callee);
            c_emit_set_result(result, C_EMIT_INVALID_INPUT, "failed to allocate memory for mangled symbol");
            return 0;
        }
        c_build_function_symbol(module_decl, &pseudo_function, mangled, 256);
        call_name = mangled;
    } else if (expr->kind == ZIR_EXPR_CALL_EXTERN && strncmp(callee, "c.", 2) == 0) {
        call_name = callee + 2;
        {
            static const struct { const char *base; const char *owned; } cow_mutations[] = {
                {"zt_grid2d_i64_set", "zt_grid2d_i64_set_owned"},
                {"zt_grid2d_i64_fill", "zt_grid2d_i64_fill_owned"},
                {"zt_grid2d_text_set", "zt_grid2d_text_set_owned"},
                {"zt_grid2d_text_fill", "zt_grid2d_text_fill_owned"},
                {"zt_grid3d_i64_set", "zt_grid3d_i64_set_owned"},
                {"zt_grid3d_i64_fill", "zt_grid3d_i64_fill_owned"},
                {"zt_grid3d_text_set", "zt_grid3d_text_set_owned"},
                {"zt_grid3d_text_fill", "zt_grid3d_text_fill_owned"},
                {"zt_pqueue_i64_push", "zt_pqueue_i64_push_owned"},
                {"zt_pqueue_text_push", "zt_pqueue_text_push_owned"},
                {"zt_circbuf_i64_push", "zt_circbuf_i64_push_owned"},
                {"zt_circbuf_text_push", "zt_circbuf_text_push_owned"},
                {"zt_btreemap_text_text_set", "zt_btreemap_text_text_set_owned"},
                {"zt_btreemap_text_text_remove", "zt_btreemap_text_text_remove_owned"},
                {"zt_btreeset_text_insert", "zt_btreeset_text_insert_owned"},
                {"zt_btreeset_text_remove", "zt_btreeset_text_remove_owned"},
                {NULL, NULL}
            };
            int ci;
            for (ci = 0; cow_mutations[ci].base != NULL; ci += 1) {
                if (strcmp(call_name, cow_mutations[ci].base) == 0) {
                    if (expr->as.call.args.count > 0) {
                        const zir_expr *first_arg = expr->as.call.args.items[0];
                        char arg_name[128];
                        if (c_zir_expr_name_text(first_arg, arg_name, sizeof(arg_name)) ||
                            (first_arg != NULL && first_arg->kind == ZIR_EXPR_GET_FIELD)) {
                            call_name = cow_mutations[ci].owned;
                        }
                    }
                    break;
                }
            }
        }
    } else {
        c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "call '%s' is not supported by the current C emitter subset", callee);
        free(callee);
        if (mangled) free(mangled);
        return 0;
    }

    if (!c_buffer_append_format(&emitter->buffer, "%s(", call_name)) {
        free(callee);
        if (mangled) free(mangled);
        return 0;
    }

    for (index = 0; index < expr->as.call.args.count; index += 1) {
        const zir_expr *arg = expr->as.call.args.items[index];
        const char *expected_arg_type = NULL;

        if (index > 0 && !c_buffer_append(&emitter->buffer, ", ")) {
            free(callee);
            if (mangled) free(mangled);
            return 0;
        }

        if (direct_call &&
                callee_function != NULL &&
                index < callee_function->param_count) {
            expected_arg_type = callee_function->params[index].type_name;
        } else if (!direct_call) {
            expected_arg_type = c_extern_call_expected_arg_type(callee, index);
        }

        if (direct_call &&
                callee_function != NULL &&
                callee_function->receiver_type_name != NULL &&
                callee_function->is_mutating &&
                index == 0) {
            if (!(c_buffer_append(&emitter->buffer, "&(") &&
                    c_emit_zir_expr(emitter, module_decl, function_decl, arg, NULL, result) &&
                    c_buffer_append(&emitter->buffer, ")"))) {
                free(callee);
                if (mangled) free(mangled);
                return 0;
            }
        } else if (!direct_call && strcmp(callee, "c.puts") == 0) {
            if (arg != NULL && arg->kind == ZIR_EXPR_STRING) {
                if (!c_emit_c_string_literal(emitter, arg->as.text.text)) {
                    free(callee);
                    if (mangled) free(mangled);
                    return 0;
                }
            } else if (c_zir_expr_is_text(function_decl, arg)) {
                if (!(c_buffer_append(&emitter->buffer, "zt_text_data(") &&
                        c_emit_zir_expr(emitter, module_decl, function_decl, arg, "text", result) &&
                        c_buffer_append(&emitter->buffer, ")"))) {
                    free(callee);
                    if (mangled) free(mangled);
                    return 0;
                }
            } else if (!c_emit_zir_expr(emitter, module_decl, function_decl, arg, NULL, result)) {
                free(callee);
                if (mangled) free(mangled);
                return 0;
            }
        } else if (!c_emit_zir_expr(emitter, module_decl, function_decl, arg, expected_arg_type, result)) {
            free(callee);
            if (mangled) free(mangled);
            return 0;
        }
    }

    ok = c_buffer_append(&emitter->buffer, ")");
    free(callee);
    if (mangled) free(mangled);
    return ok;
}

static const char *c_extern_call_expected_arg_type(const char *callee, size_t index) {
    if (callee == NULL) {
        return NULL;
    }

    if (strcmp(callee, "c.zt_check") == 0 && index == 0) {
        return "bool";
    }

    if (strcmp(callee, "c.zt_text_concat") == 0 && (index == 0 || index == 1)) {
        return "text";
    }

    if (strcmp(callee, "c.zt_dyn_text_repr_from_i64") == 0 && index == 0) {
        return "int";
    }

    if (strcmp(callee, "c.zt_dyn_text_repr_from_float") == 0 && index == 0) {
        return "float";
    }

    if (strcmp(callee, "c.zt_dyn_text_repr_from_bool") == 0 && index == 0) {
        return "bool";
    }

    if (strcmp(callee, "c.zt_dyn_text_repr_from_text") == 0 && index == 0) {
        return "text";
    }

    if (index == 0 &&
            (strcmp(callee, "c.zt_host_write_stdout") == 0 ||
             strcmp(callee, "c.zt_host_write_stderr") == 0)) {
        return "text";
    }

    if (strcmp(callee, "c.zt_core_error_make") == 0) {
        if (index == 0 || index == 1) {
            return "text";
        }
        if (index == 2) {
            return "optional<text>";
        }
    }

    return NULL;
}

static const zir_module_var *c_find_module_var_decl(const zir_module *module_decl, const char *name) {
    size_t index;

    if (module_decl == NULL || name == NULL) return NULL;
    for (index = 0; index < module_decl->module_var_count; index += 1) {
        const zir_module_var *module_var = &module_decl->module_vars[index];
        if (module_var->name != NULL && strcmp(module_var->name, name) == 0) {
            return module_var;
        }
    }

    return NULL;
}

static int c_is_module_var_name(const zir_module *module_decl, const char *name) {
    return c_find_module_var_decl(module_decl, name) != NULL;
}

static int c_zir_call_extern_needs_ffi_shield(
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr) {
    size_t index;
    char type_name[96];

    if (expr == NULL || expr->kind != ZIR_EXPR_CALL_EXTERN) {
        return 0;
    }

    if (strncmp(c_safe_text(expr->as.call.callee_name), "c.", 2) != 0) {
        return 0;
    }

    for (index = 0; index < expr->as.call.args.count; index += 1) {
        if (c_zir_expr_resolve_type(module_decl, function_decl, expr->as.call.args.items[index], type_name, sizeof(type_name)) &&
                c_type_is_managed(type_name)) {
            return 1;
        }
    }

    return 0;
}

static int c_legacy_call_extern_needs_ffi_shield(const zir_function *function_decl, const char *expr_text) {
    char trimmed[256];
    const char *call_body;
    const char *open;
    const char *close;
    char callee[128];
    const char *cursor;
    const char *end;

    if (expr_text == NULL || !c_copy_trimmed(trimmed, sizeof(trimmed), expr_text)) {
        return 0;
    }

    if (strncmp(trimmed, "call_extern ", 12) != 0) {
        return 0;
    }

    call_body = trimmed + 12;
    open = strchr(call_body, '(');
    close = strrchr(call_body, ')');
    if (open == NULL || close == NULL || close < open) {
        return 0;
    }

    if (!c_copy_trimmed_segment(callee, sizeof(callee), call_body, open) ||
            strncmp(callee, "c.", 2) != 0) {
        return 0;
    }

    cursor = open + 1;
    end = close;
    while (cursor < end) {
        const char *comma = c_find_top_level_comma(cursor, end);
        const char *segment_end = comma != NULL ? comma : end;
        char arg[256];
        const char *type_name;

        if (!c_copy_trimmed_segment(arg, sizeof(arg), cursor, segment_end)) {
            return 0;
        }

        if (c_is_identifier_only(arg)) {
            type_name = c_find_symbol_type(function_decl, arg);
            if (c_type_is_managed(type_name)) {
                return 1;
            }
        }

        if (comma == NULL) {
            break;
        }

        cursor = comma + 1;
    }

    return 0;
}

static int c_emit_ffi_shield_retain(c_emitter *emitter, const char *name) {
    return c_begin_line(emitter) &&
           c_buffer_append_format(&emitter->buffer, "        if (%s != NULL) { zt_retain(%s); }", name, name);
}

static int c_emit_ffi_shield_release(c_emitter *emitter, const char *name) {
    return c_begin_line(emitter) &&
           c_buffer_append_format(&emitter->buffer, "        if (%s != NULL) { zt_release(%s); }", name, name);
}

static int c_emit_zir_ffi_call_arg(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const char *callee,
        const zir_expr *arg,
        size_t index,
        const char *override_name,
        const char *override_type_name,
        c_emit_result *result) {
    const char *expected_arg_type = c_extern_call_expected_arg_type(callee, index);

    if (strcmp(callee, "c.puts") == 0) {
        if (override_name != NULL && c_type_is(override_type_name, "text")) {
            return c_buffer_append(&emitter->buffer, "zt_text_data(") &&
                   c_buffer_append(&emitter->buffer, override_name) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (override_name == NULL && arg != NULL && arg->kind == ZIR_EXPR_STRING) {
            return c_emit_c_string_literal(emitter, arg->as.text.text);
        }

        if ((override_name == NULL && c_zir_expr_is_text(function_decl, arg))) {
            return c_buffer_append(&emitter->buffer, "zt_text_data(") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, arg, "text", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }
    }

    if (strcmp(callee, "c.zt_check") == 0 && index == 1) {
        if (override_name != NULL && c_type_is(override_type_name, "text")) {
            return c_buffer_append(&emitter->buffer, "zt_text_data(") &&
                   c_buffer_append(&emitter->buffer, override_name) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (override_name == NULL && arg != NULL && arg->kind == ZIR_EXPR_STRING) {
            return c_emit_c_string_literal(emitter, arg->as.text.text);
        }

        if (override_name == NULL && c_zir_expr_is_text(function_decl, arg)) {
            return c_buffer_append(&emitter->buffer, "zt_text_data(") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, arg, "text", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }
    }

    if (override_name != NULL) {
        return c_buffer_append(&emitter->buffer, override_name);
    }

    return c_emit_zir_expr(emitter, module_decl, function_decl, arg, expected_arg_type, result);
}

static int c_emit_legacy_ffi_call_arg(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const char *callee,
        const char *arg_text,
        size_t index,
        const char *override_name,
        const char *override_type_name,
        c_emit_result *result) {
    const char *expected_arg_type = c_extern_call_expected_arg_type(callee, index);

    if (strcmp(callee, "c.puts") == 0) {
        if (override_name != NULL && c_type_is(override_type_name, "text")) {
            return c_buffer_append(&emitter->buffer, "zt_text_data(") &&
                   c_buffer_append(&emitter->buffer, override_name) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (override_name == NULL && strncmp(arg_text, "const \"", 7) == 0) {
            return c_buffer_append(&emitter->buffer, arg_text + 6);
        }

        if (override_name == NULL && c_expression_is_text(function_decl, arg_text)) {
            return c_buffer_append(&emitter->buffer, "zt_text_data(") &&
                   c_emit_expr(emitter, module_decl, function_decl, arg_text, "text", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }
    }

    if (strcmp(callee, "c.zt_check") == 0 && index == 1) {
        if (override_name != NULL && c_type_is(override_type_name, "text")) {
            return c_buffer_append(&emitter->buffer, "zt_text_data(") &&
                   c_buffer_append(&emitter->buffer, override_name) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (override_name == NULL && strncmp(arg_text, "const \"", 7) == 0) {
            return c_buffer_append(&emitter->buffer, arg_text + 6);
        }

        if (override_name == NULL && c_expression_is_text(function_decl, arg_text)) {
            return c_buffer_append(&emitter->buffer, "zt_text_data(") &&
                   c_emit_expr(emitter, module_decl, function_decl, arg_text, "text", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }
    }

    if (override_name != NULL) {
        return c_buffer_append(&emitter->buffer, override_name);
    }

    return c_emit_expr(emitter, module_decl, function_decl, arg_text, expected_arg_type, result);
}

static int c_emit_ffi_shielded_zir_call_statement(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        const char *assign_target,
        c_emit_result *result) {
    const size_t arg_count = expr != NULL ? expr->as.call.args.count : 0;
    int *shielded = NULL;
    char (*arg_types)[96] = NULL;
    char (*arg_c_types)[128] = NULL;
    char temp_name[64];
    const char *callee;
    size_t index;
    int has_shielded = 0;
    int ok = 0;

    if (expr == NULL || expr->kind != ZIR_EXPR_CALL_EXTERN) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "ffi shield requires an extern call expression");
        return 0;
    }

    callee = c_safe_text(expr->as.call.callee_name);
    if (strncmp(callee, "c.", 2) != 0) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "ffi shield requires a c.* extern call");
        return 0;
    }

    shielded = (int *)calloc(arg_count > 0 ? arg_count : 1, sizeof(int));
    arg_types = (char (*)[96])calloc(arg_count > 0 ? arg_count : 1, sizeof(*arg_types));
    arg_c_types = (char (*)[128])calloc(arg_count > 0 ? arg_count : 1, sizeof(*arg_c_types));
    if (shielded == NULL || arg_types == NULL || arg_c_types == NULL) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "failed to allocate ffi shield metadata");
        goto cleanup;
    }

    for (index = 0; index < arg_count; index += 1) {
        if (c_zir_expr_resolve_type(module_decl, function_decl, expr->as.call.args.items[index], arg_types[index], sizeof(arg_types[index])) &&
                c_type_is_managed(arg_types[index])) {
            shielded[index] = 1;
            has_shielded = 1;
            if (!c_type_to_c(module_decl, arg_types[index], arg_c_types[index], sizeof(arg_c_types[index]), result)) {
                goto cleanup;
            }
        }
    }

    if (!has_shielded) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "ffi shield requested for call without managed args");
        goto cleanup;
    }

    if (!c_buffer_append(&emitter->buffer, "    {")) {
        goto cleanup;
    }

    for (index = 0; index < arg_count; index += 1) {
        if (!shielded[index]) {
            continue;
        }

        snprintf(temp_name, sizeof(temp_name), "zt_ffi_arg%zu", index);
        if (!(c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "        ") &&
                c_emit_typed_name(&emitter->buffer, arg_c_types[index], temp_name) &&
                c_buffer_append(&emitter->buffer, " = ") &&
                c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.call.args.items[index], arg_types[index], result) &&
                c_buffer_append(&emitter->buffer, ";"))) {
            goto cleanup;
        }

        if (!c_emit_ffi_shield_retain(emitter, temp_name)) {
            goto cleanup;
        }
    }

    if (!(c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "        "))) {
        goto cleanup;
    }

    if (assign_target != NULL && !(c_buffer_append_format(&emitter->buffer, "%s = ", assign_target))) {
        goto cleanup;
    }

    if (!(c_buffer_append_format(&emitter->buffer, "%s(", callee + 2))) {
        goto cleanup;
    }

    for (index = 0; index < arg_count; index += 1) {
        const char *override_name = NULL;
        const char *override_type_name = NULL;

        if (index > 0 && !c_buffer_append(&emitter->buffer, ", ")) {
            goto cleanup;
        }

        if (shielded[index]) {
            snprintf(temp_name, sizeof(temp_name), "zt_ffi_arg%zu", index);
            override_name = temp_name;
            override_type_name = arg_types[index];
        }

        if (!c_emit_zir_ffi_call_arg(
                emitter,
                module_decl,
                function_decl,
                callee,
                expr->as.call.args.items[index],
                index,
                override_name,
                override_type_name,
                result)) {
            goto cleanup;
        }
    }

    if (!c_buffer_append(&emitter->buffer, ");")) {
        goto cleanup;
    }

    for (index = arg_count; index > 0; index -= 1) {
        if (!shielded[index - 1]) {
            continue;
        }

        snprintf(temp_name, sizeof(temp_name), "zt_ffi_arg%zu", index - 1);
        if (!c_emit_ffi_shield_release(emitter, temp_name)) {
            goto cleanup;
        }
    }

    if (!(c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    }"))) {
        goto cleanup;
    }

    ok = 1;

cleanup:
    free(shielded);
    free(arg_types);
    free(arg_c_types);
    return ok;
}

static int c_emit_ffi_shielded_legacy_call_statement(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const char *expr_text,
        const char *assign_target,
        c_emit_result *result) {
    char trimmed[256];
    const char *call_body;
    const char *open;
    const char *close;
    const char *cursor;
    const char *end;
    char callee[128];
    size_t arg_count = 0;
    char (*args)[256] = NULL;
    int *shielded = NULL;
    char (*arg_types)[96] = NULL;
    char (*arg_c_types)[128] = NULL;
    char temp_name[64];
    size_t index = 0;
    int has_shielded = 0;
    int ok = 0;

    if (expr_text == NULL || !c_copy_trimmed(trimmed, sizeof(trimmed), expr_text)) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "ffi shield requires a valid legacy call expression");
        return 0;
    }

    if (strncmp(trimmed, "call_extern ", 12) != 0) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "ffi shield requires a legacy extern call");
        return 0;
    }

    call_body = trimmed + 12;
    open = strchr(call_body, '(');
    close = strrchr(call_body, ')');
    if (open == NULL || close == NULL || close < open) {
        c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid call expression '%s'", trimmed);
        return 0;
    }

    if (!c_copy_trimmed_segment(callee, sizeof(callee), call_body, open) ||
            strncmp(callee, "c.", 2) != 0) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "ffi shield requires a c.* extern call");
        return 0;
    }

    cursor = open + 1;
    end = close;
    if (!c_segment_is_blank(cursor, end)) {
        while (cursor < end) {
            const char *comma = c_find_top_level_comma(cursor, end);
            arg_count += 1;
            if (comma == NULL) {
                break;
            }
            cursor = comma + 1;
        }
    }

    args = (char (*)[256])calloc(arg_count > 0 ? arg_count : 1, sizeof(*args));
    shielded = (int *)calloc(arg_count > 0 ? arg_count : 1, sizeof(int));
    arg_types = (char (*)[96])calloc(arg_count > 0 ? arg_count : 1, sizeof(*arg_types));
    arg_c_types = (char (*)[128])calloc(arg_count > 0 ? arg_count : 1, sizeof(*arg_c_types));
    if (args == NULL || shielded == NULL || arg_types == NULL || arg_c_types == NULL) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "failed to allocate legacy ffi shield metadata");
        goto cleanup;
    }

    cursor = open + 1;
    index = 0;
    while (cursor < end && index < arg_count) {
        const char *comma = c_find_top_level_comma(cursor, end);
        const char *segment_end = comma != NULL ? comma : end;
        const char *type_name;

        if (!c_copy_trimmed_segment(args[index], sizeof(args[index]), cursor, segment_end)) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid call argument in '%s'", trimmed);
            goto cleanup;
        }

        if (c_is_identifier_only(args[index])) {
            type_name = c_find_symbol_type(function_decl, args[index]);
            if (c_type_is_managed(type_name)) {
                shielded[index] = 1;
                has_shielded = 1;
                snprintf(arg_types[index], sizeof(arg_types[index]), "%s", c_safe_text(type_name));
                if (!c_type_to_c(module_decl, arg_types[index], arg_c_types[index], sizeof(arg_c_types[index]), result)) {
                    goto cleanup;
                }
            }
        }

        index += 1;
        if (comma == NULL) {
            break;
        }
        cursor = comma + 1;
    }

    if (!has_shielded) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "ffi shield requested for legacy call without managed args");
        goto cleanup;
    }

    if (!c_buffer_append(&emitter->buffer, "    {")) {
        goto cleanup;
    }

    for (index = 0; index < arg_count; index += 1) {
        if (!shielded[index]) {
            continue;
        }

        snprintf(temp_name, sizeof(temp_name), "zt_ffi_arg%zu", index);
        if (!(c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "        ") &&
                c_emit_typed_name(&emitter->buffer, arg_c_types[index], temp_name) &&
                c_buffer_append(&emitter->buffer, " = ") &&
                c_emit_expr(emitter, module_decl, function_decl, args[index], arg_types[index], result) &&
                c_buffer_append(&emitter->buffer, ";"))) {
            goto cleanup;
        }

        if (!c_emit_ffi_shield_retain(emitter, temp_name)) {
            goto cleanup;
        }
    }

    if (!(c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "        "))) {
        goto cleanup;
    }

    if (assign_target != NULL && !(c_buffer_append_format(&emitter->buffer, "%s = ", assign_target))) {
        goto cleanup;
    }

    if (!c_buffer_append_format(&emitter->buffer, "%s(", callee + 2)) {
        goto cleanup;
    }

    for (index = 0; index < arg_count; index += 1) {
        const char *override_name = NULL;
        const char *override_type_name = NULL;

        if (index > 0 && !c_buffer_append(&emitter->buffer, ", ")) {
            goto cleanup;
        }

        if (shielded[index]) {
            snprintf(temp_name, sizeof(temp_name), "zt_ffi_arg%zu", index);
            override_name = temp_name;
            override_type_name = arg_types[index];
        }

        if (!c_emit_legacy_ffi_call_arg(
                emitter,
                module_decl,
                function_decl,
                callee,
                args[index],
                index,
                override_name,
                override_type_name,
                result)) {
            goto cleanup;
        }
    }

    if (!c_buffer_append(&emitter->buffer, ");")) {
        goto cleanup;
    }

    for (index = arg_count; index > 0; index -= 1) {
        if (!shielded[index - 1]) {
            continue;
        }

        snprintf(temp_name, sizeof(temp_name), "zt_ffi_arg%zu", index - 1);
        if (!c_emit_ffi_shield_release(emitter, temp_name)) {
            goto cleanup;
        }
    }

    if (!(c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    }"))) {
        goto cleanup;
    }

    ok = 1;

cleanup:
    free(args);
    free(shielded);
    free(arg_types);
    free(arg_c_types);
    return ok;
}


static int c_emit_ffi_shielded_zir_return(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        int with_cleanup,
        c_emit_result *result) {
    char return_type_c[64];

    if (expr == NULL || expr->kind != ZIR_EXPR_CALL_EXTERN) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "ffi return shield requires a structured extern call");
        return 0;
    }

    if (with_cleanup) {
        return c_begin_line(emitter) &&
               c_emit_ffi_shielded_zir_call_statement(
                   emitter,
                   module_decl,
                   function_decl,
                   expr,
                   "zt_return_value",
                   result) &&
               c_begin_line(emitter) &&
               c_buffer_append(&emitter->buffer, "    goto zt_cleanup;");
    }

    if (!c_type_to_c(module_decl, function_decl->return_type, return_type_c, sizeof(return_type_c), result)) {
        return 0;
    }

    return c_begin_line(emitter) &&
           c_buffer_append(&emitter->buffer, "    {") &&
           c_begin_line(emitter) &&
           c_buffer_append(&emitter->buffer, "        ") &&
           c_emit_typed_name(&emitter->buffer, return_type_c, "zt_ffi_return_value") &&
           c_buffer_append(&emitter->buffer, " = NULL;") &&
           c_begin_line(emitter) &&
           c_emit_ffi_shielded_zir_call_statement(
               emitter,
               module_decl,
               function_decl,
               expr,
               "zt_ffi_return_value",
               result) &&
           c_begin_line(emitter) &&
           c_buffer_append(&emitter->buffer, "        return zt_ffi_return_value;") &&
           c_begin_line(emitter) &&
           c_buffer_append(&emitter->buffer, "    }");
}

static int c_is_mutating_self_expr(const zir_function *function_decl, const zir_expr *expr) {
    return function_decl != NULL &&
           function_decl->receiver_type_name != NULL &&
           function_decl->is_mutating &&
           expr != NULL &&
           expr->kind == ZIR_EXPR_NAME &&
           strcmp(c_safe_text(expr->as.text.text), "self") == 0;
}

static const zir_named_expr *c_find_make_struct_field_init(const zir_expr *expr, const char *field_name) {
    size_t index;
    if (expr == NULL || expr->kind != ZIR_EXPR_MAKE_STRUCT || field_name == NULL) return NULL;
    for (index = 0; index < expr->as.make_struct.fields.count; index += 1) {
        const zir_named_expr *field_init = &expr->as.make_struct.fields.items[index];
        if (strcmp(c_safe_text(field_init->name), field_name) == 0) {
            return field_init;
        }
    }
    return NULL;
}

static int c_emit_zir_make_struct_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        c_emit_result *result) {
    const zir_struct_decl *struct_decl;
    const zir_enum_decl *enum_decl;
    char c_type[64];
    size_t index;

    struct_decl = c_find_user_struct(module_decl, expr->as.make_struct.type_name);
    enum_decl = c_find_user_enum(module_decl, expr->as.make_struct.type_name);
    if (struct_decl == NULL && enum_decl == NULL) {
        c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "unknown user struct '%s' in make_struct", c_safe_text(expr->as.make_struct.type_name));
        return 0;
    }

    if (!c_type_to_c(module_decl, expr->as.make_struct.type_name, c_type, sizeof(c_type), result)) {
        return 0;
    }

    if (enum_decl != NULL) {
        const zir_named_expr *tag_init = c_find_make_struct_field_init(expr, "__zt_enum_tag");
        const zir_enum_variant_decl *variant = NULL;
        char enum_symbol[128];
        char variant_member[64];
        char variant_tag[192];
        char *endptr = NULL;
        long tag_index_long;
        size_t tag_index;

        if (tag_init == NULL || tag_init->value == NULL || tag_init->value->kind != ZIR_EXPR_INT) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "enum constructor '%s' requires __zt_enum_tag int field", c_safe_text(expr->as.make_struct.type_name));
            return 0;
        }

        tag_index_long = strtol(c_safe_text(tag_init->value->as.text.text), &endptr, 10);
        if (endptr == NULL || *endptr != '\0' || tag_index_long < 0 || (size_t)tag_index_long >= enum_decl->variant_count) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid enum tag '%s' in constructor '%s'", c_safe_text(tag_init->value->as.text.text), c_safe_text(expr->as.make_struct.type_name));
            return 0;
        }
        tag_index = (size_t)tag_index_long;
        variant = &enum_decl->variants[tag_index];

        c_build_enum_symbol(module_decl, enum_decl, enum_symbol, sizeof(enum_symbol));
        c_copy_sanitized(variant_member, sizeof(variant_member), c_safe_text(variant->name));
        snprintf(variant_tag, sizeof(variant_tag), "%s__%s", enum_symbol, variant_member);

        if (!(c_buffer_append(&emitter->buffer, "((") &&
                c_buffer_append(&emitter->buffer, c_type) &&
                c_buffer_append(&emitter->buffer, "){.tag = ") &&
                c_buffer_append(&emitter->buffer, variant_tag))) {
            return 0;
        }

        if (variant->field_count > 0) {
            if (!(c_buffer_append(&emitter->buffer, ", .as.") &&
                    c_buffer_append(&emitter->buffer, variant_member) &&
                    c_buffer_append(&emitter->buffer, " = {"))) {
                return 0;
            }

            for (index = 0; index < variant->field_count; index += 1) {
                const zir_enum_variant_field_decl *field_decl = &variant->fields[index];
                const zir_named_expr *field_init = c_find_make_struct_field_init(expr, field_decl->name);
                if (field_init == NULL) {
                    c_emit_set_result(
                        result,
                        C_EMIT_UNSUPPORTED_EXPR,
                        "missing field '%s' in enum constructor '%s.%s'",
                        c_safe_text(field_decl->name),
                        c_safe_text(enum_decl->name),
                        c_safe_text(variant->name));
                    return 0;
                }
                if (index > 0 && !c_buffer_append(&emitter->buffer, ", ")) {
                    return 0;
                }
                if (!(c_buffer_append(&emitter->buffer, ".") &&
                        c_buffer_append(&emitter->buffer, c_safe_text(field_decl->name)) &&
                        c_buffer_append(&emitter->buffer, " = ") &&
                        c_emit_zir_expr(
                            emitter,
                            module_decl,
                            function_decl,
                            field_init->value,
                            field_decl->type_name,
                            result))) {
                    return 0;
                }
            }

            if (!c_buffer_append(&emitter->buffer, "}")) {
                return 0;
            }
        }

        return c_buffer_append(&emitter->buffer, "})");
    }

    if (!(c_buffer_append(&emitter->buffer, "((") &&
            c_buffer_append(&emitter->buffer, c_type) &&
            c_buffer_append(&emitter->buffer, "){"))) {
        return 0;
    }

    for (index = 0; index < expr->as.make_struct.fields.count; index += 1) {
        const zir_named_expr *field_init = &expr->as.make_struct.fields.items[index];
        const zir_field_decl *field_decl = c_find_struct_field(struct_decl, field_init->name);
        if (field_decl == NULL) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "unknown field '%s' in make_struct %s", c_safe_text(field_init->name), c_safe_text(expr->as.make_struct.type_name));
            return 0;
        }
        if (index > 0 && !c_buffer_append(&emitter->buffer, ", ")) {
            return 0;
        }
        if (!(c_buffer_append(&emitter->buffer, ".") &&
                c_buffer_append(&emitter->buffer, c_safe_text(field_init->name)) &&
                c_buffer_append(&emitter->buffer, " = ") &&
                c_emit_zir_expr(
                    emitter,
                    module_decl,
                    function_decl,
                    field_init->value,
                    field_decl->type_name,
                    result))) {
            return 0;
        }
    }

    return c_buffer_append(&emitter->buffer, "})");
}

static int c_emit_zir_make_list_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        c_emit_result *result) {
    size_t index;

    if (strcmp(c_safe_text(expr->as.make_list.item_type_name), "int") == 0) {
        if (expr->as.make_list.items.count == 0) {
            return c_buffer_append(&emitter->buffer, "zt_list_i64_new()");
        }

        if (!c_buffer_append(&emitter->buffer, "zt_list_i64_from_array(((zt_int[]){")) {
            return 0;
        }

        for (index = 0; index < expr->as.make_list.items.count; index += 1) {
            if (index > 0 && !c_buffer_append(&emitter->buffer, ", ")) return 0;
            if (!c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.make_list.items.items[index], "int", result)) return 0;
        }

        return c_buffer_append_format(&emitter->buffer, "}), %zu)", expr->as.make_list.items.count);
    }

    if (strcmp(c_safe_text(expr->as.make_list.item_type_name), "text") == 0) {
        if (expr->as.make_list.items.count == 0) {
            return c_buffer_append(&emitter->buffer, "zt_list_text_new()");
        }

        if (!c_buffer_append(&emitter->buffer, "zt_list_text_from_array(((zt_text *[]){")) {
            return 0;
        }

        for (index = 0; index < expr->as.make_list.items.count; index += 1) {
            const zir_expr *item = expr->as.make_list.items.items[index];

            if (!c_zir_expr_is_materialized_text_ref(function_decl, item)) {
                c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "make_list<text> currently requires text locals or params");
                return 0;
            }

            if (index > 0 && !c_buffer_append(&emitter->buffer, ", ")) return 0;
            if (!c_emit_zir_expr(emitter, module_decl, function_decl, item, "text", result)) return 0;
        }

        return c_buffer_append_format(&emitter->buffer, "}), %zu)", expr->as.make_list.items.count);
    }

    if (c_type_is(expr->as.make_list.item_type_name, "dyn<textrepresentable>")) {
        if (expr->as.make_list.items.count == 0) {
            return c_buffer_append(&emitter->buffer, "zt_list_dyn_text_repr_new()");
        }

        if (!c_buffer_append(&emitter->buffer, "zt_list_dyn_text_repr_from_array_owned(((zt_dyn_text_repr *[]){")) {
            return 0;
        }

        for (index = 0; index < expr->as.make_list.items.count; index += 1) {
            if (index > 0 && !c_buffer_append(&emitter->buffer, ", ")) return 0;
            if (!c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.make_list.items.items[index], "dyn<textrepresentable>", result)) return 0;
        }

        return c_buffer_append_format(&emitter->buffer, "}), %zu)", expr->as.make_list.items.count);
    }

    c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "make_list<%s> is not supported by the current C emitter subset", c_safe_text(expr->as.make_list.item_type_name));
    return 0;
}

static int c_emit_zir_index_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        const char *expected_type_name,
        c_emit_result *result) {
    char sequence_type[96];
    c_map_spec map_spec;

    if (!c_zir_expr_resolve_type(module_decl, function_decl, expr->as.sequence.first, sequence_type, sizeof(sequence_type))) {
        c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "unable to resolve structured sequence type");
        return 0;
    }

    if (c_type_is(sequence_type, "text")) {
        if (!c_is_blank(expected_type_name) && !c_type_is(expected_type_name, "text")) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "index_seq on text produces text, but expected '%s'", c_safe_text(expected_type_name));
            return 0;
        }

        return c_buffer_append(&emitter->buffer, "zt_text_index(") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, "text", result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (c_type_is(sequence_type, "bytes")) {
        if (!c_is_blank(expected_type_name) && strcmp(expected_type_name, "uint8") != 0) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "index_seq on bytes produces uint8, but expected '%s'", c_safe_text(expected_type_name));
            return 0;
        }

        return c_buffer_append(&emitter->buffer, "zt_bytes_get(") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, "bytes", result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (c_type_is(sequence_type, "list<int>")) {
        if (!c_is_blank(expected_type_name) && strcmp(expected_type_name, "int") != 0) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "index_seq on list<int> produces int, but expected '%s'", c_safe_text(expected_type_name));
            return 0;
        }

        return c_buffer_append(&emitter->buffer, "zt_list_i64_get(") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, "list<int>", result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (c_type_is(sequence_type, "list<text>")) {
        if (!c_is_blank(expected_type_name) && !c_type_is(expected_type_name, "text")) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "index_seq on list<text> produces text, but expected '%s'", c_safe_text(expected_type_name));
            return 0;
        }

        return c_buffer_append(&emitter->buffer, "zt_list_text_get(") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, "list<text>", result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (c_type_is(sequence_type, "list<dyn<textrepresentable>>")) {
        if (!c_is_blank(expected_type_name) && !c_type_is(expected_type_name, "dyn<textrepresentable>")) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "index_seq on list<dyn<TextRepresentable>> produces dyn<TextRepresentable>, but expected '%s'", c_safe_text(expected_type_name));
            return 0;
        }

        return c_buffer_append(&emitter->buffer, "zt_list_dyn_text_repr_get(") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, "list<dyn<textrepresentable>>", result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (c_map_spec_for_type(module_decl, sequence_type, &map_spec)) {
        if (!c_is_blank(expected_type_name) && !c_type_is(expected_type_name, map_spec.value_type_name)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_TYPE,
                "index_seq on %s produces %s, but expected '%s'",
                map_spec.canonical_name,
                map_spec.value_type_name,
                c_safe_text(expected_type_name));
            return 0;
        }

        return c_buffer_append_format(&emitter->buffer, "%s(", map_spec.get_fn) &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, map_spec.canonical_name, result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, map_spec.key_type_name, result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "index_seq is not supported for '%s'", sequence_type);
    return 0;
}

static int c_emit_zir_slice_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        const char *expected_type_name,
        c_emit_result *result) {
    char sequence_type[96];

    if (!c_zir_expr_resolve_type(module_decl, function_decl, expr->as.sequence.first, sequence_type, sizeof(sequence_type))) {
        c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "unable to resolve structured sequence type");
        return 0;
    }

    if (c_type_is(sequence_type, "text")) {
        if (!c_is_blank(expected_type_name) && !c_type_is(expected_type_name, "text")) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "slice_seq on text produces text, but expected '%s'", c_safe_text(expected_type_name));
            return 0;
        }

        return c_buffer_append(&emitter->buffer, "zt_text_slice(") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, "text", result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, "int", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (c_type_is(sequence_type, "bytes")) {
        if (!c_is_blank(expected_type_name) && !c_type_is(expected_type_name, "bytes")) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "slice_seq on bytes produces bytes, but expected '%s'", c_safe_text(expected_type_name));
            return 0;
        }

        return c_buffer_append(&emitter->buffer, "zt_bytes_slice(") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, "bytes", result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, "int", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (c_type_is(sequence_type, "list<int>")) {
        return c_buffer_append(&emitter->buffer, "zt_list_i64_slice(") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, "list<int>", result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, "int", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    if (c_type_is(sequence_type, "list<text>")) {
        return c_buffer_append(&emitter->buffer, "zt_list_text_slice(") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, "list<text>", result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, "int", result) &&
               c_buffer_append(&emitter->buffer, ")");
    }

    c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "slice_seq is not supported for '%s'", sequence_type);
    return 0;
}
static int c_emit_owned_managed_zir_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        const char *expected_type_name,
        c_emit_result *result);

static int c_emit_zir_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        const char *expected_type_name,
        c_emit_result *result);

static int c_emit_zir_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        const char *expected_type_name,
        c_emit_result *result) {
    const char *op;

    if (expr == NULL) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "structured ZIR expression cannot be null");
        return 0;
    }

    if (!c_is_blank(expected_type_name) && c_type_is(expected_type_name, "dyn<textrepresentable>")) {
        char actual_type_name[96];
        int actual_known = c_zir_expr_resolve_type(module_decl, function_decl, expr, actual_type_name, sizeof(actual_type_name));
        int is_integral = 0;

        if (!actual_known) {
            if (expr->kind == ZIR_EXPR_INDEX_SEQ) {
                return c_emit_zir_index_expr(
                    emitter,
                    module_decl,
                    function_decl,
                    expr,
                    "dyn<textrepresentable>",
                    result);
            }

            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "unable to resolve expression type for dyn<TextRepresentable> boxing (kind=%s)", zir_expr_kind_name(expr->kind));
            return 0;
        }

        is_integral =
            c_type_is(actual_type_name, "int") ||
            c_type_is(actual_type_name, "int8") ||
            c_type_is(actual_type_name, "int16") ||
            c_type_is(actual_type_name, "int32") ||
            c_type_is(actual_type_name, "int64") ||
            c_type_is(actual_type_name, "uint8") ||
            c_type_is(actual_type_name, "uint16") ||
            c_type_is(actual_type_name, "uint32") ||
            c_type_is(actual_type_name, "uint64");

        if (c_type_is(actual_type_name, "dyn<textrepresentable>")) {
            return c_emit_owned_managed_zir_expr(
                emitter,
                module_decl,
                function_decl,
                expr,
                "dyn<textrepresentable>",
                result);
        }

        if (c_type_is(actual_type_name, "text")) {
            return c_buffer_append(&emitter->buffer, "zt_dyn_text_repr_from_text_owned(") &&
                   c_emit_owned_managed_zir_expr(
                       emitter,
                       module_decl,
                       function_decl,
                       expr,
                       "text",
                       result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (c_type_is(actual_type_name, "bool")) {
            return c_buffer_append(&emitter->buffer, "zt_dyn_text_repr_from_bool(") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr, "bool", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (c_type_is(actual_type_name, "float")) {
            return c_buffer_append(&emitter->buffer, "zt_dyn_text_repr_from_float(") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr, "float", result) &&
                   c_buffer_append(&emitter->buffer, ")");
        }

        if (is_integral) {
            return c_buffer_append(&emitter->buffer, "zt_dyn_text_repr_from_i64((zt_int)(") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr, actual_type_name, result) &&
                   c_buffer_append(&emitter->buffer, "))");
        }

        c_emit_set_result(
            result,
            C_EMIT_UNSUPPORTED_TYPE,
            "cannot box type '%s' as dyn<TextRepresentable>",
            actual_type_name);
        return 0;
    }
    switch (expr->kind) {
        case ZIR_EXPR_NAME:
        case ZIR_EXPR_INT:
        case ZIR_EXPR_FLOAT:
            return c_emit_value(emitter, expr->as.text.text);

        case ZIR_EXPR_BOOL:
            return c_buffer_append(&emitter->buffer, expr->as.bool_literal.value ? "true" : "false");

        case ZIR_EXPR_STRING:
            if (c_type_is(expected_type_name, "text")) {
                return c_buffer_append(&emitter->buffer, "zt_text_from_utf8_literal(") &&
                       c_emit_c_string_literal(emitter, expr->as.text.text) &&
                       c_buffer_append(&emitter->buffer, ")");
            }
            return c_emit_c_string_literal(emitter, expr->as.text.text);

        case ZIR_EXPR_BYTES:
            if (!c_is_blank(expected_type_name) && !c_type_is(expected_type_name, "bytes")) {
                c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "bytes literal cannot be emitted as '%s'", c_safe_text(expected_type_name));
                return 0;
            }
            return c_emit_hex_bytes_literal(emitter, expr->as.text.text, result);

        case ZIR_EXPR_COPY:
            return c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.single.value, expected_type_name, result);

        case ZIR_EXPR_UNARY:
            op = c_unary_operator(expr->as.unary.op_name);
            if (op == NULL) {
                c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "unary operation '%s' is not supported yet", c_safe_text(expr->as.unary.op_name));
                return 0;
            }
            return c_buffer_append_format(&emitter->buffer, "(%s", op) &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.unary.operand, NULL, result) &&
                   c_buffer_append(&emitter->buffer, ")");

        case ZIR_EXPR_BINARY:
            op = c_math_function(expr->as.binary.op_name);
            if (op != NULL) {
                return c_buffer_append_format(&emitter->buffer, "%s(", op) &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.binary.left, NULL, result) &&
                       c_buffer_append(&emitter->buffer, ", ") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.binary.right, NULL, result) &&
                       c_buffer_append(&emitter->buffer, ")");
            }

            if ((strcmp(expr->as.binary.op_name, "eq") == 0 || strcmp(expr->as.binary.op_name, "ne") == 0)) {
                char left_type_name[96];
                char right_type_name[96];
                int left_ok = c_zir_expr_resolve_type(module_decl, function_decl, expr->as.binary.left, left_type_name, sizeof(left_type_name));
                int right_ok = c_zir_expr_resolve_type(module_decl, function_decl, expr->as.binary.right, right_type_name, sizeof(right_type_name));

                if (left_ok && right_ok &&
                        c_type_is(left_type_name, "text") &&
                        c_type_is(right_type_name, "text")) {
                    if (strcmp(expr->as.binary.op_name, "eq") == 0) {
                        return c_buffer_append(&emitter->buffer, "zt_text_eq(") &&
                               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.binary.left, "text", result) &&
                               c_buffer_append(&emitter->buffer, ", ") &&
                               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.binary.right, "text", result) &&
                               c_buffer_append(&emitter->buffer, ")");
                    }

                    return c_buffer_append(&emitter->buffer, "(!zt_text_eq(") &&
                           c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.binary.left, "text", result) &&
                           c_buffer_append(&emitter->buffer, ", ") &&
                           c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.binary.right, "text", result) &&
                           c_buffer_append(&emitter->buffer, "))");
                }
                if (left_ok && right_ok &&
                        c_type_is(left_type_name, "outcome<text,text>") &&
                        c_type_is(right_type_name, "outcome<text,text>")) {
                    if (strcmp(expr->as.binary.op_name, "eq") == 0) {
                        return c_buffer_append(&emitter->buffer, "zt_outcome_text_text_eq(") &&
                               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.binary.left, "outcome<text,text>", result) &&
                               c_buffer_append(&emitter->buffer, ", ") &&
                               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.binary.right, "outcome<text,text>", result) &&
                               c_buffer_append(&emitter->buffer, ")");
                    }

                    return c_buffer_append(&emitter->buffer, "(!zt_outcome_text_text_eq(") &&
                           c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.binary.left, "outcome<text,text>", result) &&
                           c_buffer_append(&emitter->buffer, ", ") &&
                           c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.binary.right, "outcome<text,text>", result) &&
                           c_buffer_append(&emitter->buffer, "))");
                }
            }

            op = c_binary_operator(expr->as.binary.op_name);
            if (op == NULL) {
                const char *suggestion = c_binary_operator_suggestion(expr->as.binary.op_name);
                c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, 
                                 "binary operation '%s' is not recognized. "
                                 "Valid: add, sub, mul, div, mod, lt, le, gt, ge, eq, ne, and, or. "
                                 "Did you mean '%s'?", 
                                 c_safe_text(expr->as.binary.op_name), suggestion);
                return 0;
            }
            return c_buffer_append(&emitter->buffer, "(") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.binary.left, NULL, result) &&
                   c_buffer_append_format(&emitter->buffer, " %s ", op) &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.binary.right, NULL, result) &&
                   c_buffer_append(&emitter->buffer, ")");

        case ZIR_EXPR_CALL_DIRECT:
        case ZIR_EXPR_CALL_EXTERN:
            return c_emit_zir_call_expr(emitter, module_decl, function_decl, expr, result);

        case ZIR_EXPR_MAKE_STRUCT:
            return c_emit_zir_make_struct_expr(emitter, module_decl, function_decl, expr, result);

        case ZIR_EXPR_MAKE_LIST:
            return c_emit_zir_make_list_expr(emitter, module_decl, function_decl, expr, result);

        case ZIR_EXPR_GET_FIELD:
            if (c_is_mutating_self_expr(function_decl, expr->as.field.object)) {
                return c_buffer_append_format(&emitter->buffer, "(self->%s)", c_safe_text(expr->as.field.field_name));
            }
            {
                char object_type[96];
                const zir_enum_decl *enum_decl = NULL;
                if (c_zir_expr_resolve_type(module_decl, function_decl, expr->as.field.object, object_type, sizeof(object_type))) {
                    enum_decl = c_find_user_enum(module_decl, object_type);
                }
                if (enum_decl != NULL) {
                    if (strcmp(c_safe_text(expr->as.field.field_name), "__zt_enum_tag") == 0) {
                        return c_buffer_append(&emitter->buffer, "(") &&
                               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.field.object, NULL, result) &&
                               c_buffer_append(&emitter->buffer, ".tag)");
                    }

                    {
                        const zir_enum_variant_decl *resolved_variant = NULL;
                        size_t variant_index;
                        char variant_member[64];

                        for (variant_index = 0; variant_index < enum_decl->variant_count; variant_index += 1) {
                            const zir_enum_variant_decl *variant = &enum_decl->variants[variant_index];
                            if (c_find_enum_variant_field(variant, expr->as.field.field_name) == NULL) continue;
                            if (resolved_variant != NULL) {
                                c_emit_set_result(
                                    result,
                                    C_EMIT_UNSUPPORTED_EXPR,
                                    "ambiguous enum payload field '%s' in '%s'",
                                    c_safe_text(expr->as.field.field_name),
                                    c_safe_text(enum_decl->name));
                                return 0;
                            }
                            resolved_variant = variant;
                        }

                        if (resolved_variant == NULL) {
                            c_emit_set_result(
                                result,
                                C_EMIT_UNSUPPORTED_EXPR,
                                "unknown enum payload field '%s' in '%s'",
                                c_safe_text(expr->as.field.field_name),
                                c_safe_text(enum_decl->name));
                            return 0;
                        }

                        c_copy_sanitized(variant_member, sizeof(variant_member), c_safe_text(resolved_variant->name));
                        return c_buffer_append(&emitter->buffer, "(") &&
                               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.field.object, NULL, result) &&
                               c_buffer_append_format(
                                   &emitter->buffer,
                                   ".as.%s.%s)",
                                   variant_member,
                                   c_safe_text(expr->as.field.field_name));
                    }
                }
            }
            return c_buffer_append(&emitter->buffer, "(") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.field.object, NULL, result) &&
                   c_buffer_append_format(&emitter->buffer, ".%s)", c_safe_text(expr->as.field.field_name));

        case ZIR_EXPR_INDEX_SEQ:
            return c_emit_zir_index_expr(emitter, module_decl, function_decl, expr, expected_type_name, result);

        case ZIR_EXPR_SLICE_SEQ:
            return c_emit_zir_slice_expr(emitter, module_decl, function_decl, expr, expected_type_name, result);

        case ZIR_EXPR_LIST_LEN: {
            char sequence_type[96];

            if (!c_zir_expr_resolve_type(module_decl, function_decl, expr->as.single.value, sequence_type, sizeof(sequence_type))) {
                c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "unable to resolve list_len sequence type");
                return 0;
            }

            if (c_type_is(sequence_type, "list<int>")) {
                return c_buffer_append(&emitter->buffer, "zt_list_i64_len(") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.single.value, "list<int>", result) &&
                       c_buffer_append(&emitter->buffer, ")");
            }

            if (c_type_is(sequence_type, "list<text>")) {
                return c_buffer_append(&emitter->buffer, "zt_list_text_len(") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.single.value, "list<text>", result) &&
                       c_buffer_append(&emitter->buffer, ")");
            }

            if (c_type_is(sequence_type, "list<dyn<textrepresentable>>")) {
                return c_buffer_append(&emitter->buffer, "zt_list_dyn_text_repr_len(") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.single.value, "list<dyn<textrepresentable>>", result) &&
                       c_buffer_append(&emitter->buffer, ")");
            }

            c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "list_len is not supported for '%s'", sequence_type);
            return 0;
        }
        case ZIR_EXPR_MAP_LEN: {
            c_legalized_seq_expr legalized;
            c_legalize_result legalize_result;

            c_legalize_result_init(&legalize_result);
            if (!c_legalize_zir_list_len_expr(function_decl, expr, expected_type_name, &legalized, &legalize_result)) {
                c_copy_legalize_result(result, &legalize_result);
                return 0;
            }

            return c_emit_legalized_seq_expr(emitter, module_decl, function_decl, &legalized, result);
        }

        case ZIR_EXPR_OPTIONAL_EMPTY:
            if (strcmp(c_safe_text(expr->as.type_only.type_name), "int") == 0) return c_buffer_append(&emitter->buffer, "zt_optional_i64_empty()");
            if (strcmp(c_safe_text(expr->as.type_only.type_name), "text") == 0) return c_buffer_append(&emitter->buffer, "zt_optional_text_empty()");
            if (strcmp(c_safe_text(expr->as.type_only.type_name), "bytes") == 0) return c_buffer_append(&emitter->buffer, "zt_optional_bytes_empty()");
            if (strcmp(c_safe_text(expr->as.type_only.type_name), "list<int>") == 0) return c_buffer_append(&emitter->buffer, "zt_optional_list_i64_empty()");
            break;

        case ZIR_EXPR_OPTIONAL_PRESENT:
            if (c_type_is(expected_type_name, "optional<list<int>>")) {
                if (!c_zir_expr_is_materialized_list_i64_ref(function_decl, expr->as.single.value)) {
                    c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "optional_present for optional<list<int>> requires a materialized list<int>");
                    return 0;
                }
                return c_buffer_append(&emitter->buffer, "zt_optional_list_i64_present(") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.single.value, "list<int>", result) &&
                       c_buffer_append(&emitter->buffer, ")");
            }
            if (c_type_is(expected_type_name, "optional<bytes>")) {
                if (!c_zir_expr_is_materialized_bytes_ref(function_decl, expr->as.single.value)) {
                    c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "optional_present for optional<bytes> requires a materialized bytes");
                    return 0;
                }
                return c_buffer_append(&emitter->buffer, "zt_optional_bytes_present(") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.single.value, "bytes", result) &&
                       c_buffer_append(&emitter->buffer, ")");
            }
            if (c_type_is(expected_type_name, "optional<bytes>")) {
                if (!c_zir_expr_is_materialized_bytes_ref(function_decl, expr->as.single.value)) {
                    c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "optional_present for optional<bytes> requires materialized bytes");
                    return 0;
                }
                return c_buffer_append(&emitter->buffer, "zt_optional_bytes_present(") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.single.value, "bytes", result) &&
                       c_buffer_append(&emitter->buffer, ")");
            }
            if (c_type_is(expected_type_name, "optional<text>")) {
                if (!c_zir_expr_is_materialized_text_ref(function_decl, expr->as.single.value)) {
                    c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "optional_present for optional<text> requires a materialized text");
                    return 0;
                }
                return c_buffer_append(&emitter->buffer, "zt_optional_text_present(") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.single.value, "text", result) &&
                       c_buffer_append(&emitter->buffer, ")");
            }
            return c_buffer_append(&emitter->buffer, "zt_optional_i64_present(") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.single.value, "int", result) &&
                   c_buffer_append(&emitter->buffer, ")");

        case ZIR_EXPR_OPTIONAL_IS_PRESENT:
            if (c_zir_expr_is_materialized_optional_list_i64_ref(function_decl, expr->as.single.value)) {
                return c_buffer_append(&emitter->buffer, "zt_optional_list_i64_is_present(") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.single.value, "optional<list<int>>", result) &&
                       c_buffer_append(&emitter->buffer, ")");
            }
            if (c_zir_expr_is_materialized_optional_bytes_ref(function_decl, expr->as.single.value)) {
                return c_buffer_append(&emitter->buffer, "zt_optional_bytes_is_present(") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.single.value, "optional<bytes>", result) &&
                       c_buffer_append(&emitter->buffer, ")");
            }
            if (c_zir_expr_is_materialized_optional_text_ref(function_decl, expr->as.single.value)) {
                return c_buffer_append(&emitter->buffer, "zt_optional_text_is_present(") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.single.value, "optional<text>", result) &&
                       c_buffer_append(&emitter->buffer, ")");
            }
            return c_buffer_append(&emitter->buffer, "zt_optional_i64_is_present(") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.single.value, "optional<int>", result) &&
                   c_buffer_append(&emitter->buffer, ")");

        case ZIR_EXPR_COALESCE:
        case ZIR_EXPR_OPTIONAL_VALUE:
        case ZIR_EXPR_OUTCOME_SUCCESS:
        case ZIR_EXPR_OUTCOME_FAILURE:
        case ZIR_EXPR_OUTCOME_IS_SUCCESS:
        case ZIR_EXPR_OUTCOME_VALUE:
        case ZIR_EXPR_TRY_PROPAGATE:
        case ZIR_EXPR_MAKE_MAP:
        case ZIR_EXPR_CALL_RUNTIME_INTRINSIC:
        case ZIR_EXPR_SET_FIELD:
        case ZIR_EXPR_LIST_PUSH:
        case ZIR_EXPR_LIST_SET:
        case ZIR_EXPR_MAP_SET:
            return c_emit_zir_expr_as_legacy(emitter, module_decl, function_decl, expr, expected_type_name, result);

        default:
            break;
    }

    c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, 
                     "structured expression kind '%s' is not supported. Supported kinds: name, int, float, bool, string, bytes, binary, unary, call, index, slice, make_struct, make_list, make_map, optional, outcome, ternary.",
                     zir_expr_kind_name(expr->kind));
    return 0;
}

static int c_emit_owned_managed_zir_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        const char *expected_type_name,
        c_emit_result *result) {
    char name[128];
    const char *type_name;

    if (c_zir_expr_name_text(expr, name, sizeof(name))) {
        type_name = c_find_symbol_type(function_decl, name);
        if (c_type_is_managed(type_name)) {
            return c_buffer_append_format(&emitter->buffer, "(zt_retain(%s), %s)", name, name);
        }
    }

    if (c_type_is_managed(expected_type_name) && expr->kind == ZIR_EXPR_GET_FIELD) {
        if (c_is_mutating_self_expr(function_decl, expr->as.field.object) ||
                c_zir_expr_name_text(expr->as.field.object, name, sizeof(name))) {
            if (!(c_buffer_append(&emitter->buffer, "(zt_retain(") &&
                  c_emit_zir_expr(emitter, module_decl, function_decl, expr, expected_type_name, result) &&
                  c_buffer_append(&emitter->buffer, "), ") &&
                  c_emit_zir_expr(emitter, module_decl, function_decl, expr, expected_type_name, result) &&
                  c_buffer_append(&emitter->buffer, ")"))) {
                return 0;
            }
            return 1;
        }
    }

    if (c_type_is(expected_type_name, "dyn<textrepresentable>")) {
        return c_emit_zir_expr(emitter, module_decl, function_decl, expr, NULL, result);
    }

    return c_emit_zir_expr(emitter, module_decl, function_decl, expr, expected_type_name, result);
}

static int c_emit_effect_zir_expr(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_expr *expr,
        c_emit_result *result) {
    if (expr == NULL) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "effect expression cannot be null");
        return 0;
    }

    if (expr->kind == ZIR_EXPR_CALL_EXTERN &&
            c_zir_call_extern_needs_ffi_shield(module_decl, function_decl, expr)) {
        return c_emit_ffi_shielded_zir_call_statement(
            emitter,
            module_decl,
            function_decl,
            expr,
            NULL,
            result);
    }

    if (expr->kind == ZIR_EXPR_SET_FIELD) {
        if (c_is_mutating_self_expr(function_decl, expr->as.field.object)) {
            return c_buffer_append_format(&emitter->buffer, "    self->%s = ", c_safe_text(expr->as.field.field_name)) &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.field.value, NULL, result) &&
                   c_buffer_append(&emitter->buffer, ";");
        }
        return c_buffer_append(&emitter->buffer, "    ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.field.object, NULL, result) &&
               c_buffer_append_format(&emitter->buffer, ".%s = ", c_safe_text(expr->as.field.field_name)) &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.field.value, NULL, result) &&
               c_buffer_append(&emitter->buffer, ";");
    }

    if (expr->kind == ZIR_EXPR_LIST_SET) {
        const zir_expr *target_expr = expr->as.sequence.first;
        char target_type[96];
        char target_name[128];
        char target_object_name[128];
        int target_is_name = c_zir_expr_name_text(target_expr, target_name, sizeof(target_name));
        int target_is_field = target_expr != NULL && target_expr->kind == ZIR_EXPR_GET_FIELD;
        int target_field_object_is_self = 0;
        int target_field_object_is_name = 0;

        if (target_is_field) {
            target_field_object_is_self = c_is_mutating_self_expr(function_decl, target_expr->as.field.object);
            target_field_object_is_name = c_zir_expr_name_text(target_expr->as.field.object, target_object_name, sizeof(target_object_name));
        }

        if (!c_zir_expr_resolve_type(module_decl, function_decl, expr->as.sequence.first, target_type, sizeof(target_type))) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "unable to resolve list_set target type");
            return 0;
        }

        if (c_type_is(target_type, "list<int>")) {
            if (target_is_name) {
                return c_buffer_append_format(&emitter->buffer, "    %s = zt_list_i64_set_owned(", target_name) &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, "list<int>", result) &&
                       c_buffer_append(&emitter->buffer, ", ") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
                       c_buffer_append(&emitter->buffer, ", ") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, "int", result) &&
                       c_buffer_append(&emitter->buffer, ");");
            }

            if (target_is_field) {
                if (target_field_object_is_self) {
                    return c_buffer_append_format(
                               &emitter->buffer,
                               "    self->%s = zt_list_i64_set_owned(self->%s, ",
                               c_safe_text(target_expr->as.field.field_name),
                               c_safe_text(target_expr->as.field.field_name)) &&
                           c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
                           c_buffer_append(&emitter->buffer, ", ") &&
                           c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, "int", result) &&
                           c_buffer_append(&emitter->buffer, ");");
                }

                if (target_field_object_is_name) {
                    return c_buffer_append_format(
                               &emitter->buffer,
                               "    %s.%s = zt_list_i64_set_owned(%s.%s, ",
                               target_object_name,
                               c_safe_text(target_expr->as.field.field_name),
                               target_object_name,
                               c_safe_text(target_expr->as.field.field_name)) &&
                           c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
                           c_buffer_append(&emitter->buffer, ", ") &&
                           c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, "int", result) &&
                           c_buffer_append(&emitter->buffer, ");");
                }

                c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "list_set field target requires named object or mutating self");
                return 0;
            }

            return c_buffer_append(&emitter->buffer, "    zt_list_i64_set(") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, "list<int>", result) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, "int", result) &&
                   c_buffer_append(&emitter->buffer, ");");
        }

        if (c_type_is(target_type, "list<text>")) {
            if (!c_zir_expr_is_materialized_text_ref(function_decl, expr->as.sequence.third)) {
                c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "list_set on list<text> requires a materialized text value");
                return 0;
            }

            if (target_is_name) {
                return c_buffer_append_format(&emitter->buffer, "    %s = zt_list_text_set_owned(", target_name) &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, "list<text>", result) &&
                       c_buffer_append(&emitter->buffer, ", ") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
                       c_buffer_append(&emitter->buffer, ", ") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, "text", result) &&
                       c_buffer_append(&emitter->buffer, ");");
            }

            if (target_is_field) {
                if (target_field_object_is_self) {
                    return c_buffer_append_format(
                               &emitter->buffer,
                               "    self->%s = zt_list_text_set_owned(self->%s, ",
                               c_safe_text(target_expr->as.field.field_name),
                               c_safe_text(target_expr->as.field.field_name)) &&
                           c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
                           c_buffer_append(&emitter->buffer, ", ") &&
                           c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, "text", result) &&
                           c_buffer_append(&emitter->buffer, ");");
                }

                if (target_field_object_is_name) {
                    return c_buffer_append_format(
                               &emitter->buffer,
                               "    %s.%s = zt_list_text_set_owned(%s.%s, ",
                               target_object_name,
                               c_safe_text(target_expr->as.field.field_name),
                               target_object_name,
                               c_safe_text(target_expr->as.field.field_name)) &&
                           c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
                           c_buffer_append(&emitter->buffer, ", ") &&
                           c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, "text", result) &&
                           c_buffer_append(&emitter->buffer, ");");
                }

                c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "list_set field target requires named object or mutating self");
                return 0;
            }

            return c_buffer_append(&emitter->buffer, "    zt_list_text_set(") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, "list<text>", result) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, "int", result) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, "text", result) &&
                   c_buffer_append(&emitter->buffer, ");");
        }

        c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "list_set is not supported for '%s'", target_type);
        return 0;
    }

    if (expr->kind == ZIR_EXPR_MAP_SET) {
        const zir_expr *target_expr = expr->as.sequence.first;
        c_map_spec map_spec;
        char target_type[96];
        char target_name[128];
        char target_object_name[128];
        int target_is_name = c_zir_expr_name_text(target_expr, target_name, sizeof(target_name));
        int target_is_field = target_expr != NULL && target_expr->kind == ZIR_EXPR_GET_FIELD;
        int target_field_object_is_self = 0;
        int target_field_object_is_name = 0;

        if (target_is_field) {
            target_field_object_is_self = c_is_mutating_self_expr(function_decl, target_expr->as.field.object);
            target_field_object_is_name = c_zir_expr_name_text(target_expr->as.field.object, target_object_name, sizeof(target_object_name));
        }

        if (!c_zir_expr_resolve_type(module_decl, function_decl, expr->as.sequence.first, target_type, sizeof(target_type)) ||
                !c_map_spec_for_type(module_decl, target_type, &map_spec)) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_TYPE, "map_set is not supported for '%s'", c_safe_text(target_type));
            return 0;
        }

        if (c_type_is_managed(map_spec.key_type_name) &&
                !c_zir_expr_is_materialized_type_ref(function_decl, expr->as.sequence.second, map_spec.key_type_name)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_EXPR,
                "map_set on %s requires a materialized managed key of type %s",
                map_spec.canonical_name,
                map_spec.key_type_name);
            return 0;
        }

        if (c_type_is_managed(map_spec.value_type_name) &&
                !c_zir_expr_is_materialized_type_ref(function_decl, expr->as.sequence.third, map_spec.value_type_name)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_EXPR,
                "map_set on %s requires a materialized managed value of type %s",
                map_spec.canonical_name,
                map_spec.value_type_name);
            return 0;
        }

        if (target_is_name) {
            return c_buffer_append_format(&emitter->buffer, "    %s = %s(", target_name, map_spec.set_owned_fn) &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, map_spec.canonical_name, result) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, map_spec.key_type_name, result) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, map_spec.value_type_name, result) &&
                   c_buffer_append(&emitter->buffer, ");");
        }

        if (target_is_field) {
            if (target_field_object_is_self) {
                return c_buffer_append_format(
                           &emitter->buffer,
                           "    self->%s = %s(self->%s, ",
                           c_safe_text(target_expr->as.field.field_name),
                           map_spec.set_owned_fn,
                           c_safe_text(target_expr->as.field.field_name)) &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, map_spec.key_type_name, result) &&
                       c_buffer_append(&emitter->buffer, ", ") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, map_spec.value_type_name, result) &&
                       c_buffer_append(&emitter->buffer, ");");
            }

            if (target_field_object_is_name) {
                return c_buffer_append_format(
                           &emitter->buffer,
                           "    %s.%s = %s(%s.%s, ",
                           target_object_name,
                           c_safe_text(target_expr->as.field.field_name),
                           map_spec.set_owned_fn,
                           target_object_name,
                           c_safe_text(target_expr->as.field.field_name)) &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, map_spec.key_type_name, result) &&
                       c_buffer_append(&emitter->buffer, ", ") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, map_spec.value_type_name, result) &&
                       c_buffer_append(&emitter->buffer, ");");
            }

            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "map_set field target requires named object or mutating self");
            return 0;
        }

        return c_buffer_append_format(&emitter->buffer, "    %s(", map_spec.set_fn) &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.first, map_spec.canonical_name, result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.second, map_spec.key_type_name, result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_zir_expr(emitter, module_decl, function_decl, expr->as.sequence.third, map_spec.value_type_name, result) &&
               c_buffer_append(&emitter->buffer, ");");
    }
    return c_buffer_append(&emitter->buffer, "    ") &&
           c_emit_zir_expr(emitter, module_decl, function_decl, expr, NULL, result) &&
           c_buffer_append(&emitter->buffer, ";");
}


static int c_collect_locals(
        const zir_module *module_decl,
        const zir_function *function_decl,
        c_local_decl *locals,
        size_t capacity,
        size_t *out_count,
        c_emit_result *result) {
    size_t block_index;
    size_t local_count = 0;

    for (block_index = 0; block_index < function_decl->block_count; block_index += 1) {
        size_t instruction_index;
        const zir_block *block = &function_decl->blocks[block_index];

        for (instruction_index = 0; instruction_index < block->instruction_count; instruction_index += 1) {
            const zir_instruction *instruction = &block->instructions[instruction_index];
            size_t existing;
            int found = 0;

            if (instruction->kind != ZIR_INSTR_ASSIGN) {
                continue;
            }

            if (c_is_module_var_name(module_decl, instruction->dest_name)) {
                continue;
            }

            for (existing = 0; existing < local_count; existing += 1) {
                if (strcmp(locals[existing].name, instruction->dest_name) == 0) {
                    found = 1;
                    break;
                }
            }

            if (found) {
                continue;
            }

            if (local_count >= capacity) {
                c_emit_set_result(result, C_EMIT_INVALID_INPUT, "too many locals in function '%s'", c_safe_text(function_decl->name));
                return 0;
            }

            locals[local_count].name = instruction->dest_name;
            locals[local_count].type_name = instruction->type_name;
            local_count += 1;
        }
    }

    *out_count = local_count;
    return 1;
}

static int c_has_managed_locals(const zir_module *module_decl, const c_local_decl *locals, size_t local_count) {
    size_t index;

    for (index = 0; index < local_count; index += 1) {
        if (c_type_needs_managed_cleanup(module_decl, locals[index].type_name)) {
            return 1;
        }
    }

    return 0;
}

static int c_is_managed_local_name(
        const zir_module *module_decl,
        const c_local_decl *locals,
        size_t local_count,
        const char *name) {
    size_t index;

    for (index = 0; index < local_count; index += 1) {
        if (c_type_needs_managed_cleanup(module_decl, locals[index].type_name) &&
                strcmp(locals[index].name, name) == 0) {
            return 1;
        }
    }

    return 0;
}

static int c_emit_locals(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const c_local_decl *locals,
        size_t local_count,
        c_emit_result *result) {
    size_t index;
    int has_cleanup = c_has_managed_locals(module_decl, locals, local_count);

    for (index = 0; index < local_count; index += 1) {
        char c_type[64];
        int is_pointer_managed;
        int needs_value_cleanup;

        if (!c_type_to_c(module_decl, locals[index].type_name, c_type, sizeof(c_type), result)) {
            return 0;
        }

        is_pointer_managed = c_type_is_managed(locals[index].type_name);
        needs_value_cleanup = c_type_needs_managed_cleanup(module_decl, locals[index].type_name) && !is_pointer_managed;

        if (!c_begin_line(emitter) ||
                !c_buffer_append(&emitter->buffer, "    ")) {
            return 0;
        }

        if (is_pointer_managed) {
            if (!(c_emit_typed_name(&emitter->buffer, c_type, c_safe_text(locals[index].name)) &&
                    c_buffer_append(&emitter->buffer, " = NULL;"))) {
                return 0;
            }
        } else if (needs_value_cleanup) {
            if (!(c_emit_typed_name(&emitter->buffer, c_type, c_safe_text(locals[index].name)) &&
                    c_buffer_append(&emitter->buffer, " = {0};"))) {
                return 0;
            }
        } else {
            if (!(c_emit_typed_name(&emitter->buffer, c_type, c_safe_text(locals[index].name)) &&
                    c_buffer_append(&emitter->buffer, ";"))) {
                return 0;
            }
        }
    }

    if (has_cleanup && strcmp(function_decl->return_type, "void") != 0) {
        char return_type[64];
        int is_pointer_managed;
        int needs_value_cleanup;

        if (!c_type_to_c(module_decl, function_decl->return_type, return_type, sizeof(return_type), result)) {
            return 0;
        }

        is_pointer_managed = c_type_is_managed(function_decl->return_type);
        needs_value_cleanup = c_type_needs_managed_cleanup(module_decl, function_decl->return_type) && !is_pointer_managed;

        if (!c_begin_line(emitter) ||
                !c_buffer_append(&emitter->buffer, "    ")) {
            return 0;
        }

        if (is_pointer_managed) {
            if (!(c_emit_typed_name(&emitter->buffer, return_type, "zt_return_value") &&
                    c_buffer_append(&emitter->buffer, " = NULL;"))) {
                return 0;
            }
        } else if (needs_value_cleanup) {
            if (!(c_emit_typed_name(&emitter->buffer, return_type, "zt_return_value") &&
                    c_buffer_append(&emitter->buffer, " = {0};"))) {
                return 0;
            }
        } else {
            if (!(c_emit_typed_name(&emitter->buffer, return_type, "zt_return_value") &&
                    c_buffer_append(&emitter->buffer, ";"))) {
                return 0;
            }
        }
    }

    return 1;
}

static int c_begin_indented_line(c_emitter *emitter, const char *indent) {
    return c_begin_line(emitter) && c_buffer_append(&emitter->buffer, indent != NULL ? indent : "");
}

static int c_emit_release_for_struct_fields(
        c_emitter *emitter,
        const zir_module *module_decl,
        const char *value_name,
        const zir_struct_decl *struct_decl);

static int c_emit_retain_for_struct_fields(
        c_emitter *emitter,
        const zir_module *module_decl,
        const char *value_name,
        const zir_struct_decl *struct_decl);

static int c_emit_value_clone_in_place(
        c_emitter *emitter,
        const zir_module *module_decl,
        const char *type_name,
        const char *value_name,
        const char *indent,
        c_emit_result *result) {
    c_outcome_spec spec;

    if (c_type_is_managed(type_name)) {
        return c_begin_indented_line(emitter, indent) &&
               c_buffer_append_format(&emitter->buffer, "if (%s != NULL) { zt_retain(%s); }", value_name, value_name);
    }

    if (c_type_is(type_name, "core.error")) {
        return c_begin_indented_line(emitter, indent) &&
               c_buffer_append_format(&emitter->buffer, "%s = zt_core_error_clone(%s);", value_name, value_name);
    }

    if (c_outcome_spec_for_type(type_name, &spec)) {
        return c_begin_indented_line(emitter, indent) &&
               c_buffer_append_format(&emitter->buffer, "%s = %s(%s);", value_name, spec.propagate_fn, value_name);
    }

    if (c_type_is_struct_with_managed_fields(module_decl, type_name)) {
        return c_emit_retain_for_struct_fields(
            emitter,
            module_decl,
            value_name,
            c_find_user_struct(module_decl, type_name)
        );
    }

    (void)result;
    return 1;
}

static int c_emit_value_dispose_in_place(
        c_emitter *emitter,
        const zir_module *module_decl,
        const char *type_name,
        const char *value_name,
        const char *indent,
        c_emit_result *result) {
    c_outcome_spec spec;

    if (c_type_is_managed(type_name)) {
        return c_begin_indented_line(emitter, indent) &&
               c_buffer_append_format(
                   &emitter->buffer,
                   "if (%s != NULL) { zt_release(%s); %s = NULL; }",
                   value_name,
                   value_name,
                   value_name
               );
    }

    if (c_type_is(type_name, "core.error")) {
        return c_begin_indented_line(emitter, indent) &&
               c_buffer_append_format(&emitter->buffer, "zt_core_error_dispose(&%s);", value_name);
    }

    if (c_outcome_spec_for_type(type_name, &spec)) {
        return c_begin_indented_line(emitter, indent) &&
               c_buffer_append_format(&emitter->buffer, "%s(&%s);", spec.dispose_fn, value_name);
    }

    if (c_type_is_struct_with_managed_fields(module_decl, type_name)) {
        return c_emit_release_for_struct_fields(
            emitter,
            module_decl,
            value_name,
            c_find_user_struct(module_decl, type_name)
        );
    }

    (void)result;
    return 1;
}

static int c_emit_release_for_struct_fields(
        c_emitter *emitter,
        const zir_module *module_decl,
        const char *value_name,
        const zir_struct_decl *struct_decl) {
    size_t field_index;

    if (struct_decl == NULL) return 1;

    for (field_index = 0; field_index < struct_decl->field_count; field_index += 1) {
        const zir_field_decl *field_decl = &struct_decl->fields[field_index];
        char field_name[256];

        snprintf(field_name, sizeof(field_name), "%s.%s", c_safe_text(value_name), c_safe_text(field_decl->name));
        if (!c_emit_value_dispose_in_place(emitter, module_decl, field_decl->type_name, field_name, "    ", NULL)) {
            return 0;
        }
    }

    return 1;
}

static int c_emit_retain_for_struct_fields(
        c_emitter *emitter,
        const zir_module *module_decl,
        const char *value_name,
        const zir_struct_decl *struct_decl) {
    size_t field_index;

    if (struct_decl == NULL) return 1;

    for (field_index = 0; field_index < struct_decl->field_count; field_index += 1) {
        const zir_field_decl *field_decl = &struct_decl->fields[field_index];
        char field_name[256];

        snprintf(field_name, sizeof(field_name), "%s.%s", c_safe_text(value_name), c_safe_text(field_decl->name));
        if (!c_emit_value_clone_in_place(emitter, module_decl, field_decl->type_name, field_name, "    ", NULL)) {
            return 0;
        }
    }

    return 1;
}

static int c_emit_release_for_local(
        c_emitter *emitter,
        const zir_module *module_decl,
        const c_local_decl *local_decl) {
    return c_emit_value_dispose_in_place(
        emitter,
        module_decl,
        local_decl->type_name,
        c_safe_text(local_decl->name),
        "    ",
        NULL
    );
}

static int c_emit_effect_instruction(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_instruction *instruction,
        c_emit_result *result) {
    char trimmed[256];

    if (instruction->expr != NULL) {
        return c_emit_effect_zir_expr(emitter, module_decl, function_decl, instruction->expr, result);
    }

    if (!c_copy_trimmed(trimmed, sizeof(trimmed), instruction->expr_text)) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "effect expression is too large");
        return 0;
    }

    if (c_legacy_call_extern_needs_ffi_shield(function_decl, trimmed)) {
        return c_emit_ffi_shielded_legacy_call_statement(
            emitter,
            module_decl,
            function_decl,
            trimmed,
            NULL,
            result);
    }

    if (strncmp(trimmed, "set_field ", 10) == 0) {
        char target[128];
        char field[128];
        char value[128];

        if (!c_split_three_operands(trimmed + 10, target, sizeof(target), field, sizeof(field), value, sizeof(value))) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid set_field expression '%s'", trimmed);
            return 0;
        }

        return c_buffer_append(&emitter->buffer, "    ") &&
               c_emit_value(emitter, target) &&
               c_buffer_append(&emitter->buffer, ".") &&
               c_buffer_append(&emitter->buffer, field) &&
               c_buffer_append(&emitter->buffer, " = ") &&
               c_emit_value(emitter, value) &&
               c_buffer_append(&emitter->buffer, ";");
    }

    if (strncmp(trimmed, "list_set ", 9) == 0) {
        char target[128];
        char index_value[128];
        char assigned_value[128];
        const char *target_type;

        if (!c_split_three_operands(trimmed + 9, target, sizeof(target), index_value, sizeof(index_value), assigned_value, sizeof(assigned_value))) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid list_set expression '%s'", trimmed);
            return 0;
        }

        if (!c_is_identifier_only(target)) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "list_set requires a list symbol as target, got '%s'", target);
            return 0;
        }

        target_type = c_find_symbol_type(function_decl, target);
        if (c_type_is(target_type, "list<int>")) {
            return c_buffer_append_format(&emitter->buffer, "    %s = zt_list_i64_set_owned(", target) &&
                   c_emit_value(emitter, target) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_expr(emitter, module_decl, function_decl, index_value, "int", result) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_expr(emitter, module_decl, function_decl, assigned_value, "int", result) &&
                   c_buffer_append(&emitter->buffer, ");");
        }

        if (c_type_is(target_type, "list<text>")) {
            if (!c_expression_is_materialized_text_ref(function_decl, assigned_value)) {
                c_emit_set_result(
                    result,
                    C_EMIT_UNSUPPORTED_EXPR,
                    "list_set on list<text> currently requires a materialized text value, got '%s'",
                    assigned_value
                );
                return 0;
            }

            return c_buffer_append_format(&emitter->buffer, "    %s = zt_list_text_set_owned(", target) &&
                   c_emit_value(emitter, target) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_expr(emitter, module_decl, function_decl, index_value, "int", result) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_expr(emitter, module_decl, function_decl, assigned_value, "text", result) &&
                   c_buffer_append(&emitter->buffer, ");");
        }

        c_emit_set_result(
            result,
            C_EMIT_UNSUPPORTED_TYPE,
            "list_set is supported only for list<int> and list<text>, got '%s'",
            c_safe_text(target_type)
        );
        return 0;
    }

    if (strncmp(trimmed, "list_set ", 9) == 0) {
        char target[128];
        char index_value[128];
        char assigned_value[128];
        const char *target_type;

        if (!c_split_three_operands(trimmed + 9, target, sizeof(target), index_value, sizeof(index_value), assigned_value, sizeof(assigned_value))) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid list_set expression '%s'", trimmed);
            return 0;
        }

        if (!c_is_identifier_only(target)) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "list_set requires a list symbol as target, got '%s'", target);
            return 0;
        }

        target_type = c_find_symbol_type(function_decl, target);
        if (c_type_is(target_type, "list<int>")) {
            return c_buffer_append_format(&emitter->buffer, "    %s = zt_list_i64_set_owned(", target) &&
                   c_emit_value(emitter, target) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_expr(emitter, module_decl, function_decl, index_value, "int", result) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_expr(emitter, module_decl, function_decl, assigned_value, "int", result) &&
                   c_buffer_append(&emitter->buffer, ");");
        }

        if (c_type_is(target_type, "list<text>")) {
            if (!c_expression_is_materialized_text_ref(function_decl, assigned_value)) {
                c_emit_set_result(
                    result,
                    C_EMIT_UNSUPPORTED_EXPR,
                    "list_set on list<text> currently requires a materialized text value, got '%s'",
                    assigned_value
                );
                return 0;
            }

            return c_buffer_append_format(&emitter->buffer, "    %s = zt_list_text_set_owned(", target) &&
                   c_emit_value(emitter, target) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_expr(emitter, module_decl, function_decl, index_value, "int", result) &&
                   c_buffer_append(&emitter->buffer, ", ") &&
                   c_emit_expr(emitter, module_decl, function_decl, assigned_value, "text", result) &&
                   c_buffer_append(&emitter->buffer, ");");
        }

        c_emit_set_result(
            result,
            C_EMIT_UNSUPPORTED_TYPE,
            "list_set is supported only for list<int> and list<text>, got '%s'",
            c_safe_text(target_type)
        );
        return 0;
    }

    if (strncmp(trimmed, "map_set ", 8) == 0) {
        c_map_spec map_spec;
        char target[128];
        char key_value[128];
        char assigned_value[128];
        const char *target_type;

        if (!c_split_three_operands(trimmed + 8, target, sizeof(target), key_value, sizeof(key_value), assigned_value, sizeof(assigned_value))) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "invalid map_set expression '%s'", trimmed);
            return 0;
        }

        if (!c_is_identifier_only(target)) {
            c_emit_set_result(result, C_EMIT_UNSUPPORTED_EXPR, "map_set requires a map symbol as target, got '%s'", target);
            return 0;
        }

        target_type = c_find_symbol_type(function_decl, target);
        if (!c_map_spec_for_type(module_decl, target_type, &map_spec)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_TYPE,
                "map_set is not supported for '%s'",
                c_safe_text(target_type)
            );
            return 0;
        }

        if (c_type_is_managed(map_spec.key_type_name) &&
                !c_expression_is_materialized_type_ref(function_decl, key_value, map_spec.key_type_name)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_EXPR,
                "map_set on %s currently requires a materialized managed key of type %s, got '%s'",
                map_spec.canonical_name,
                map_spec.key_type_name,
                key_value
            );
            return 0;
        }

        if (c_type_is_managed(map_spec.value_type_name) &&
                !c_expression_is_materialized_type_ref(function_decl, assigned_value, map_spec.value_type_name)) {
            c_emit_set_result(
                result,
                C_EMIT_UNSUPPORTED_EXPR,
                "map_set on %s currently requires a materialized managed value of type %s, got '%s'",
                map_spec.canonical_name,
                map_spec.value_type_name,
                assigned_value
            );
            return 0;
        }

        return c_buffer_append_format(&emitter->buffer, "    %s = %s(", target, map_spec.set_owned_fn) &&
               c_emit_value(emitter, target) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_expr(emitter, module_decl, function_decl, key_value, map_spec.key_type_name, result) &&
               c_buffer_append(&emitter->buffer, ", ") &&
               c_emit_expr(emitter, module_decl, function_decl, assigned_value, map_spec.value_type_name, result) &&
               c_buffer_append(&emitter->buffer, ");");
    }
    return c_buffer_append(&emitter->buffer, "    ") &&
           c_emit_expr(emitter, module_decl, function_decl, trimmed, NULL, result) &&
           c_buffer_append(&emitter->buffer, ";");
}

static int c_emit_runtime_span_literal(c_emitter *emitter, zir_span span) {
    if (!c_buffer_append(&emitter->buffer, "zt_runtime_make_span(")) {
        return 0;
    }

    if (span.source_name != NULL && span.source_name[0] != '\0') {
        if (!c_emit_c_string_literal(emitter, span.source_name)) {
            return 0;
        }
    } else if (!c_buffer_append(&emitter->buffer, "NULL")) {
        return 0;
    }

    return c_buffer_append_format(&emitter->buffer, ", %zu, %zu)", span.line, span.column);
}

typedef enum c_contract_value_format_kind {
    C_CONTRACT_VALUE_NONE = 0,
    C_CONTRACT_VALUE_INT,
    C_CONTRACT_VALUE_FLOAT,
    C_CONTRACT_VALUE_BOOL
} c_contract_value_format_kind;

static c_contract_value_format_kind c_contract_value_format(const zir_instruction *instruction) {
    const char *subject;
    const char *subject_type;

    if (instruction == NULL) return C_CONTRACT_VALUE_NONE;

    subject = c_safe_text(instruction->dest_name);
    subject_type = c_safe_text(instruction->type_name);
    if (subject[0] == '\0' || subject_type[0] == '\0') return C_CONTRACT_VALUE_NONE;

    if (c_type_is(subject_type, "int")) return C_CONTRACT_VALUE_INT;
    if (c_type_is(subject_type, "float")) return C_CONTRACT_VALUE_FLOAT;
    if (c_type_is(subject_type, "bool")) return C_CONTRACT_VALUE_BOOL;
    return C_CONTRACT_VALUE_NONE;
}

static int c_emit_check_contract_instruction(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_instruction *instruction,
        c_emit_result *result) {
    char message[512];
    const char *predicate = c_safe_text(instruction->expr_text);
    const char *subject_name = c_safe_text(instruction->dest_name);
    c_contract_value_format_kind value_kind = c_contract_value_format(instruction);

    if (predicate[0] != '\0') {
        snprintf(message, sizeof(message), "contract failed: %s", predicate);
    } else {
        snprintf(message, sizeof(message), "contract failed");
    }

    if (!c_buffer_append(&emitter->buffer, "    if (!(")) {
        return 0;
    }

    if (instruction->expr != NULL) {
        if (!c_emit_zir_expr(emitter, module_decl, function_decl, instruction->expr, "bool", result)) {
            return 0;
        }
    } else if (!c_emit_expr(emitter, module_decl, function_decl, instruction->expr_text, "bool", result)) {
        return 0;
    }

    if (!c_buffer_append(&emitter->buffer, ")) { ")) {
        return 0;
    }

    if (value_kind != C_CONTRACT_VALUE_NONE) {
        if (value_kind == C_CONTRACT_VALUE_INT) {
            if (!c_buffer_append(&emitter->buffer, "zt_contract_failed_i64(")) {
                return 0;
            }
        } else if (value_kind == C_CONTRACT_VALUE_FLOAT) {
            if (!c_buffer_append(&emitter->buffer, "zt_contract_failed_float(")) {
                return 0;
            }
        } else {
            if (!c_buffer_append(&emitter->buffer, "zt_contract_failed_bool(")) {
                return 0;
            }
        }

        if (!c_emit_c_string_literal(emitter, message)) {
            return 0;
        }
        if (!c_buffer_append(&emitter->buffer, ", (")) {
            return 0;
        }
        if (value_kind == C_CONTRACT_VALUE_INT) {
            if (!c_buffer_append(&emitter->buffer, "zt_int)(")) {
                return 0;
            }
        } else if (value_kind == C_CONTRACT_VALUE_FLOAT) {
            if (!c_buffer_append(&emitter->buffer, "zt_float)(")) {
                return 0;
            }
        } else {
            if (!c_buffer_append(&emitter->buffer, "zt_bool)(")) {
                return 0;
            }
        }
        if (!c_emit_expr(emitter, module_decl, function_decl, subject_name, c_safe_text(instruction->type_name), result)) {
            return 0;
        }
        if (!c_buffer_append(&emitter->buffer, "), ")) {
            return 0;
        }
        if (!c_emit_runtime_span_literal(emitter, instruction->span)) {
            return 0;
        }
    } else {
        if (!c_buffer_append(&emitter->buffer, "zt_contract_failed(")) {
            return 0;
        }
        if (!c_emit_c_string_literal(emitter, message)) {
            return 0;
        }
        if (!c_buffer_append(&emitter->buffer, ", ")) {
            return 0;
        }
        if (!c_emit_runtime_span_literal(emitter, instruction->span)) {
            return 0;
        }
    }

    return c_buffer_append(&emitter->buffer, "); }");
}
static int c_emit_instruction(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const zir_instruction *instruction,
        c_emit_result *result) {
    int needs_managed_cleanup;
    int is_managed_pointer;

    if (instruction->kind == ZIR_INSTR_CHECK_CONTRACT) {
        return c_begin_line(emitter) &&
               c_emit_check_contract_instruction(emitter, module_decl, function_decl, instruction, result);
    }

    if (instruction->kind != ZIR_INSTR_ASSIGN) {
        return c_begin_line(emitter) &&
               c_emit_effect_instruction(emitter, module_decl, function_decl, instruction, result);
    }

    needs_managed_cleanup = c_type_needs_managed_cleanup(module_decl, instruction->type_name);
    is_managed_pointer = c_type_is_managed(instruction->type_name);

    if (needs_managed_cleanup &&
            !c_emit_value_dispose_in_place(
                emitter,
                module_decl,
                instruction->type_name,
                c_safe_text(instruction->dest_name),
                "    ",
                result)) {
        return 0;
    }

    if (instruction->expr != NULL &&
            instruction->expr->kind == ZIR_EXPR_CALL_EXTERN &&
            c_zir_call_extern_needs_ffi_shield(module_decl, function_decl, instruction->expr)) {
        return c_begin_line(emitter) &&
               c_emit_ffi_shielded_zir_call_statement(
                   emitter,
                   module_decl,
                   function_decl,
                   instruction->expr,
                   c_safe_text(instruction->dest_name),
                   result);
    }

    if (instruction->expr == NULL) {
        char trimmed[256];

        if (!c_copy_trimmed(trimmed, sizeof(trimmed), instruction->expr_text)) {
            c_emit_set_result(result, C_EMIT_INVALID_INPUT, "expression is too large");
            return 0;
        }

        if (c_legacy_call_extern_needs_ffi_shield(function_decl, trimmed)) {
            return c_begin_line(emitter) &&
                   c_emit_ffi_shielded_legacy_call_statement(
                       emitter,
                       module_decl,
                       function_decl,
                       trimmed,
                       c_safe_text(instruction->dest_name),
                       result);
        }
    }

    if (!(c_begin_line(emitter) &&
            c_buffer_append_format(&emitter->buffer, "    %s = ", c_safe_text(instruction->dest_name)))) {
        return 0;
    }

    if (instruction->expr != NULL) {
        if (is_managed_pointer) {
            if (!c_emit_owned_managed_zir_expr(
                    emitter,
                    module_decl,
                    function_decl,
                    instruction->expr,
                    instruction->type_name,
                    result)) {
                return 0;
            }
        } else if (!c_emit_zir_expr(emitter, module_decl, function_decl, instruction->expr, instruction->type_name, result)) {
            return 0;
        }
    } else {
        if (is_managed_pointer) {
            if (!c_emit_owned_managed_expr(
                    emitter,
                    module_decl,
                    function_decl,
                    instruction->expr_text,
                    instruction->type_name,
                    result)) {
                return 0;
            }
        } else if (!c_emit_expr(emitter, module_decl, function_decl, instruction->expr_text, instruction->type_name, result)) {
            return 0;
        }
    }

    if (!c_buffer_append(&emitter->buffer, ";")) {
        return 0;
    }

    if (needs_managed_cleanup && !is_managed_pointer) {
        int rhs_needs_retain = 0;

        if (instruction->expr != NULL) {
            rhs_needs_retain = instruction->expr->kind == ZIR_EXPR_NAME ||
                               instruction->expr->kind == ZIR_EXPR_GET_FIELD;
        } else {
            char trimmed[256];
            if (c_copy_trimmed(trimmed, sizeof(trimmed), instruction->expr_text)) {
                rhs_needs_retain = c_is_identifier_only(trimmed) ||
                                   strncmp(trimmed, "get_field ", 10) == 0;
            }
        }

        if (rhs_needs_retain &&
                !c_emit_value_clone_in_place(
                    emitter,
                    module_decl,
                    instruction->type_name,
                    c_safe_text(instruction->dest_name),
                    "    ",
                    result)) {
            return 0;
        }
    }

    return 1;
}

static int c_emit_return_cleanup_transfer(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const c_local_decl *locals,
        size_t local_count,
        const zir_expr *value_expr,
        const char *value_text,
        c_emit_result *result) {
    char trimmed[256];

    if (value_expr != NULL && value_expr->kind == ZIR_EXPR_NAME) {
        snprintf(trimmed, sizeof(trimmed), "%s", c_safe_text(value_expr->as.text.text));
    } else if (!c_copy_trimmed(trimmed, sizeof(trimmed), value_text)) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "return expression is too large");
        return 0;
    }

    if (c_type_is_managed(function_decl->return_type) &&
            c_is_identifier_only(trimmed) &&
            c_is_managed_local_name(module_decl, locals, local_count, trimmed)) {
        return c_begin_line(emitter) &&
               c_buffer_append(&emitter->buffer, "    zt_return_value = ") &&
               c_emit_value(emitter, trimmed) &&
               c_buffer_append(&emitter->buffer, ";") &&
               c_begin_line(emitter) &&
               c_buffer_append_format(&emitter->buffer, "    %s = NULL;", trimmed) &&
               c_begin_line(emitter) &&
               c_buffer_append(&emitter->buffer, "    goto zt_cleanup;");
    }

    if (c_type_needs_managed_cleanup(module_decl, function_decl->return_type) &&
            !c_type_is_managed(function_decl->return_type) &&
            c_is_identifier_only(trimmed) &&
            c_is_managed_local_name(module_decl, locals, local_count, trimmed)) {
        return c_begin_line(emitter) &&
               c_buffer_append(&emitter->buffer, "    zt_return_value = ") &&
               c_emit_value(emitter, trimmed) &&
               c_buffer_append(&emitter->buffer, ";") &&
               c_begin_line(emitter) &&
               c_buffer_append_format(&emitter->buffer, "    memset(&%s, 0, sizeof(%s));", trimmed, trimmed) &&
               c_begin_line(emitter) &&
               c_buffer_append(&emitter->buffer, "    goto zt_cleanup;");
    }

    if (value_expr != NULL &&
            c_type_is_managed(function_decl->return_type) &&
            value_expr->kind == ZIR_EXPR_CALL_EXTERN &&
            c_zir_call_extern_needs_ffi_shield(module_decl, function_decl, value_expr)) {
        return c_emit_ffi_shielded_zir_return(
            emitter,
            module_decl,
            function_decl,
            value_expr,
            1,
            result);
    }



    if (!(c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    zt_return_value = "))) {
        return 0;
    }

    if (value_expr != NULL) {
        if (c_type_is_managed(function_decl->return_type)) {
            return c_emit_owned_managed_zir_expr(emitter, module_decl, function_decl, value_expr, function_decl->return_type, result) &&
                   c_buffer_append(&emitter->buffer, ";") &&
                   c_begin_line(emitter) &&
                   c_buffer_append(&emitter->buffer, "    goto zt_cleanup;");
        }

        return c_emit_zir_expr(emitter, module_decl, function_decl, value_expr, function_decl->return_type, result) &&
               c_buffer_append(&emitter->buffer, ";") &&
               c_begin_line(emitter) &&
               c_buffer_append(&emitter->buffer, "    goto zt_cleanup;");
    }

    if (c_type_is_managed(function_decl->return_type)) {
        return c_emit_owned_managed_expr(emitter, module_decl, function_decl, value_text, function_decl->return_type, result) &&
               c_buffer_append(&emitter->buffer, ";") &&
               c_begin_line(emitter) &&
               c_buffer_append(&emitter->buffer, "    goto zt_cleanup;");
    }

    return c_emit_expr(emitter, module_decl, function_decl, value_text, function_decl->return_type, result) &&
           c_buffer_append(&emitter->buffer, ";") &&
           c_begin_line(emitter) &&
           c_buffer_append(&emitter->buffer, "    goto zt_cleanup;");
}

static int c_emit_terminator(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const c_local_decl *locals,
    size_t local_count,
    const zir_block *block,
    c_emit_result *result) {
    char label_name[96];
    const int has_cleanup = c_has_managed_locals(module_decl, locals, local_count);

    switch (block->terminator.kind) {
        case ZIR_TERM_RETURN:
            if (has_cleanup) {
                if (strcmp(function_decl->return_type, "void") == 0) {
                    return c_begin_line(emitter) &&
                           c_buffer_append(&emitter->buffer, "    goto zt_cleanup;");
                }

                return c_emit_return_cleanup_transfer(
                    emitter,
                    module_decl,
                    function_decl,
                    locals,
                    local_count,
                    block->terminator.value,
                    block->terminator.value_text,
                    result
                );
            }

            if (block->terminator.value == NULL && c_is_blank(block->terminator.value_text)) {
                return c_begin_line(emitter) &&
                       c_buffer_append(&emitter->buffer, "    return;");
            }

            if (block->terminator.value != NULL &&
                    c_type_is_managed(function_decl->return_type) &&
                    block->terminator.value->kind == ZIR_EXPR_CALL_EXTERN &&
                    c_zir_call_extern_needs_ffi_shield(module_decl, function_decl, block->terminator.value)) {
                return c_emit_ffi_shielded_zir_return(
                    emitter,
                    module_decl,
                    function_decl,
                    block->terminator.value,
                    0,
                    result);
            }



            if (!(c_begin_line(emitter) &&
                    c_buffer_append(&emitter->buffer, "    return "))) {
                return 0;
            }

            if (block->terminator.value != NULL) {
                if (c_type_is_managed(function_decl->return_type)) {
                    return c_emit_owned_managed_zir_expr(emitter, module_decl, function_decl, block->terminator.value, function_decl->return_type, result) &&
                           c_buffer_append(&emitter->buffer, ";");
                }

                return c_emit_zir_expr(emitter, module_decl, function_decl, block->terminator.value, function_decl->return_type, result) &&
                       c_buffer_append(&emitter->buffer, ";");
            }

            if (c_type_is_managed(function_decl->return_type)) {
                return c_emit_owned_managed_expr(emitter, module_decl, function_decl, block->terminator.value_text, function_decl->return_type, result) &&
                       c_buffer_append(&emitter->buffer, ";");
            }

            return c_emit_expr(emitter, module_decl, function_decl, block->terminator.value_text, function_decl->return_type, result) &&
                   c_buffer_append(&emitter->buffer, ";");

        case ZIR_TERM_JUMP:
            c_build_block_label(block->terminator.target_label, label_name, sizeof(label_name));
            return c_begin_line(emitter) &&
                   c_buffer_append_format(&emitter->buffer, "    goto %s;", label_name);

        case ZIR_TERM_BRANCH_IF:
            c_build_block_label(block->terminator.then_label, label_name, sizeof(label_name));
            if (!(c_begin_line(emitter) &&
                    c_buffer_append(&emitter->buffer, "    if ("))) {
                return 0;
            }

            if (block->terminator.condition != NULL) {
                if (!c_emit_zir_expr(emitter, module_decl, function_decl, block->terminator.condition, "bool", result)) {
                    return 0;
                }
            } else if (!c_emit_value(emitter, block->terminator.condition_text)) {
                return 0;
            }

            if (!c_buffer_append_format(&emitter->buffer, ") goto %s;", label_name)) {
                return 0;
            }

            c_build_block_label(block->terminator.else_label, label_name, sizeof(label_name));
            return c_begin_line(emitter) &&
                   c_buffer_append_format(&emitter->buffer, "    goto %s;", label_name);

        case ZIR_TERM_PANIC:
            if (!c_begin_line(emitter)) {
                return 0;
            }

            if (block->terminator.message != NULL && block->terminator.message->kind == ZIR_EXPR_STRING) {
                return c_buffer_append(&emitter->buffer, "    zt_panic(") &&
                       c_emit_c_string_literal(emitter, block->terminator.message->as.text.text) &&
                       c_buffer_append(&emitter->buffer, ");");
            }

            if (!c_is_blank(block->terminator.message_text) &&
                    block->terminator.message_text[0] == '"') {
                return c_buffer_append_format(&emitter->buffer, "    zt_panic(%s);", block->terminator.message_text);
            }

            if (block->terminator.message != NULL) {
                return c_buffer_append(&emitter->buffer, "    zt_panic(zt_text_data(") &&
                       c_emit_zir_expr(emitter, module_decl, function_decl, block->terminator.message, "text", result) &&
                       c_buffer_append(&emitter->buffer, "));");
            }

            return c_buffer_append(&emitter->buffer, "    zt_panic(\"panic\");");

        case ZIR_TERM_UNREACHABLE:
            return c_begin_line(emitter) &&
                   c_buffer_append(&emitter->buffer, "    zt_panic(\"unreachable\");");

        default:
            c_emit_set_result(result, C_EMIT_INVALID_INPUT, "unknown terminator kind");
            return 0;
    }
}

static int c_emit_cleanup(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        const c_local_decl *locals,
        size_t local_count,
        c_emit_result *result) {
    size_t index;
    int has_cleanup = c_has_managed_locals(module_decl, locals, local_count);

    (void)result;

    if (!has_cleanup) {
        return 1;
    }

    if (!c_begin_line(emitter) || !c_begin_line(emitter) || !c_buffer_append(&emitter->buffer, "zt_cleanup:")) {
        return 0;
    }

    for (index = 0; index < local_count; index += 1) {
        if (c_type_needs_managed_cleanup(module_decl, locals[index].type_name) &&
                !c_emit_release_for_local(emitter, module_decl, &locals[index])) {
            return 0;
        }
    }

    if (strcmp(function_decl->return_type, "void") == 0) {
        return c_begin_line(emitter) &&
               c_buffer_append(&emitter->buffer, "    return;");
    }

    return c_begin_line(emitter) &&
           c_buffer_append(&emitter->buffer, "    return zt_return_value;");
}

static int c_emit_struct_definitions(
        c_emitter *emitter,
        const zir_module *module_decl,
        c_emit_result *result) {
    size_t struct_index;

    for (struct_index = 0; struct_index < module_decl->struct_count; struct_index += 1) {
        const zir_struct_decl *struct_decl = &module_decl->structs[struct_index];
        char symbol[128];
        size_t field_index;

        c_build_struct_symbol(module_decl, struct_decl, symbol, sizeof(symbol));

        if (!(c_begin_line(emitter) &&
                c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "typedef struct %s {", symbol))) {
            return 0;
        }

        for (field_index = 0; field_index < struct_decl->field_count; field_index += 1) {
            char c_type[64];
            const zir_field_decl *field = &struct_decl->fields[field_index];
            if (!c_type_to_c(module_decl, field->type_name, c_type, sizeof(c_type), result)) {
                return 0;
            }
            if (!(c_begin_line(emitter) &&
                    c_buffer_append(&emitter->buffer, "    ") &&
                    c_emit_typed_name(&emitter->buffer, c_type, c_safe_text(field->name)) &&
                    c_buffer_append(&emitter->buffer, ";"))) {
                return 0;
            }
        }

        if (!(c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "} %s;", symbol))) {
            return 0;
        }
    }

    return 1;
}
static int c_emit_enum_definitions(
        c_emitter *emitter,
        const zir_module *module_decl,
        c_emit_result *result) {
    size_t enum_index;

    for (enum_index = 0; enum_index < module_decl->enum_count; enum_index += 1) {
        const zir_enum_decl *enum_decl = &module_decl->enums[enum_index];
        char symbol[128];
        char tag_symbol[160];
        int has_payload = 0;
        size_t variant_index;

        c_build_enum_symbol(module_decl, enum_decl, symbol, sizeof(symbol));
        snprintf(tag_symbol, sizeof(tag_symbol), "%s_tag", symbol);

        for (variant_index = 0; variant_index < enum_decl->variant_count; variant_index += 1) {
            if (enum_decl->variants[variant_index].field_count > 0) {
                has_payload = 1;
                break;
            }
        }

        if (!(c_begin_line(emitter) &&
                c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "typedef enum %s {", tag_symbol))) {
            return 0;
        }

        for (variant_index = 0; variant_index < enum_decl->variant_count; variant_index += 1) {
            const zir_enum_variant_decl *variant = &enum_decl->variants[variant_index];
            char variant_tag[192];
            char variant_name[64];
            c_copy_sanitized(variant_name, sizeof(variant_name), c_safe_text(variant->name));
            snprintf(variant_tag, sizeof(variant_tag), "%s__%s", symbol, variant_name);
            if (!(c_begin_line(emitter) &&
                    c_buffer_append_format(&emitter->buffer, "    %s%s", variant_tag, (variant_index + 1 < enum_decl->variant_count) ? "," : ""))) {
                return 0;
            }
        }

        if (!(c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "} %s;", tag_symbol))) {
            return 0;
        }

        if (!(c_begin_line(emitter) &&
                c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "typedef struct %s {", symbol) &&
                c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "    %s tag;", tag_symbol))) {
            return 0;
        }

        if (has_payload) {
            if (!(c_begin_line(emitter) && c_buffer_append(&emitter->buffer, "    union {"))) {
                return 0;
            }

            for (variant_index = 0; variant_index < enum_decl->variant_count; variant_index += 1) {
                const zir_enum_variant_decl *variant = &enum_decl->variants[variant_index];
                char variant_member[64];
                size_t field_index;

                c_copy_sanitized(variant_member, sizeof(variant_member), c_safe_text(variant->name));
                if (!(c_begin_line(emitter) && c_buffer_append(&emitter->buffer, "        struct {"))) {
                    return 0;
                }

                if (variant->field_count == 0) {
                    if (!(c_begin_line(emitter) && c_buffer_append(&emitter->buffer, "            char _unused;"))) {
                        return 0;
                    }
                }

                for (field_index = 0; field_index < variant->field_count; field_index += 1) {
                    const zir_enum_variant_field_decl *field = &variant->fields[field_index];
                    char c_type[64];
                    char field_name[64];
                    if (!c_type_to_c(module_decl, field->type_name, c_type, sizeof(c_type), result)) {
                        return 0;
                    }
                    c_copy_sanitized(field_name, sizeof(field_name), c_safe_text(field->name));
                    if (!(c_begin_line(emitter) &&
                            c_buffer_append(&emitter->buffer, "            ") &&
                            c_emit_typed_name(&emitter->buffer, c_type, field_name) &&
                            c_buffer_append(&emitter->buffer, ";"))) {
                        return 0;
                    }
                }

                if (!(c_begin_line(emitter) &&
                        c_buffer_append_format(&emitter->buffer, "        } %s;", variant_member))) {
                    return 0;
                }
            }

            if (!(c_begin_line(emitter) && c_buffer_append(&emitter->buffer, "    } as;"))) {
                return 0;
            }
        }

        if (!(c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "} %s;", symbol))) {
            return 0;
        }
    }

    return 1;
}

static int c_generated_map_is_emitted(
        char emitted[][128],
        size_t emitted_count,
        const char *canonical_name) {
    size_t index;

    for (index = 0; index < emitted_count; index += 1) {
        if (strcmp(emitted[index], canonical_name) == 0) {
            return 1;
        }
    }

    return 0;
}

static int c_generated_map_mark_emitted(
        char emitted[][128],
        size_t emitted_capacity,
        size_t *emitted_count,
        const char *canonical_name,
        c_emit_result *result) {
    if (*emitted_count >= emitted_capacity) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "too many generated map specializations in one module");
        return 0;
    }

    snprintf(emitted[*emitted_count], 128, "%s", canonical_name);
    *emitted_count += 1;
    return 1;
}

static int c_emit_generated_map_helpers_for_type(
        c_emitter *emitter,
        const zir_module *module_decl,
        const char *type_name,
        char emitted[][128],
        size_t emitted_capacity,
        size_t *emitted_count,
        c_emit_result *result) {
    c_map_spec spec;
    char key_c_type[128];
    char value_c_type[128];
    char base_type_name[128];
    char suffix[128];
    char heap_kind_fn[160];
    char free_fn[160];
    char clone_fn[160];
    size_t base_length;

    if (!c_map_spec_for_type(module_decl, type_name, &spec) || !spec.is_generated) {
        return 1;
    }

    if (c_generated_map_is_emitted(emitted, *emitted_count, spec.canonical_name)) {
        return 1;
    }

    if (!c_emit_generated_map_helpers_for_type(
            emitter,
            module_decl,
            spec.key_type_name,
            emitted,
            emitted_capacity,
            emitted_count,
            result)) {
        return 0;
    }

    if (!c_emit_generated_map_helpers_for_type(
            emitter,
            module_decl,
            spec.value_type_name,
            emitted,
            emitted_capacity,
            emitted_count,
            result)) {
        return 0;
    }

    if (!c_type_to_c(module_decl, spec.key_type_name, key_c_type, sizeof(key_c_type), result) ||
            !c_type_to_c(module_decl, spec.value_type_name, value_c_type, sizeof(value_c_type), result)) {
        return 0;
    }

    snprintf(base_type_name, sizeof(base_type_name), "%s", spec.c_type_name);
    base_length = strlen(base_type_name);
    if (base_length < 3 || strcmp(base_type_name + base_length - 2, " *") != 0) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "generated map C type '%s' must be a pointer type", spec.c_type_name);
        return 0;
    }
    base_type_name[base_length - 2] = '\0';

    if (strncmp(base_type_name, "zt_map_", 7) != 0) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "generated map base type '%s' has unexpected prefix", base_type_name);
        return 0;
    }

    snprintf(suffix, sizeof(suffix), "%s", base_type_name + 7);
    snprintf(heap_kind_fn, sizeof(heap_kind_fn), "%s_heap_kind", base_type_name);
    snprintf(free_fn, sizeof(free_fn), "zt_free_map_%s", suffix);
    snprintf(clone_fn, sizeof(clone_fn), "zt_map_%s_deep_copy", suffix);

    if (!(c_begin_line(emitter) &&
            c_begin_line(emitter) &&
            c_buffer_append_format(&emitter->buffer, "static uint32_t %s(void);", heap_kind_fn) &&
            c_begin_line(emitter) &&
            c_begin_line(emitter) &&
            c_buffer_append_format(
                &emitter->buffer,
                "ZT_DEFINE_MAP(%s, %s, %s, %s, %s(), %d, %d, %s, %s, %s)",
                suffix,
                key_c_type,
                value_c_type,
                spec.optional_value_type_name,
                heap_kind_fn,
                c_type_is_managed(spec.key_type_name) ? 1 : 0,
                c_type_is_managed(spec.value_type_name) ? 1 : 0,
                spec.key_eq_fn,
                spec.optional_present_fn,
                spec.optional_empty_fn) &&
            c_begin_line(emitter) &&
            c_begin_line(emitter) &&
            c_buffer_append_format(&emitter->buffer, "static uint32_t %s(void) {", heap_kind_fn) &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    static uint32_t kind = 0;") &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    if (kind == 0) {") &&
            c_begin_line(emitter) &&
            c_buffer_append_format(
                &emitter->buffer,
                "        kind = zt_register_dynamic_heap_kind((zt_heap_free_fn)%s, (zt_heap_clone_fn)%s);",
                free_fn,
                clone_fn) &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    }") &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    return kind;") &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "}"))) {
        return 0;
    }

    return c_generated_map_mark_emitted(emitted, emitted_capacity, emitted_count, spec.canonical_name, result);
}

static int c_emit_generated_map_helpers(
        c_emitter *emitter,
        const zir_module *module_decl,
        c_emit_result *result) {
    char emitted[128][128];
    size_t emitted_count = 0;
    size_t struct_index;
    size_t function_index;

    for (struct_index = 0; struct_index < module_decl->struct_count; struct_index += 1) {
        const zir_struct_decl *struct_decl = &module_decl->structs[struct_index];
        size_t field_index;
        for (field_index = 0; field_index < struct_decl->field_count; field_index += 1) {
            if (!c_emit_generated_map_helpers_for_type(
                    emitter,
                    module_decl,
                    struct_decl->fields[field_index].type_name,
                    emitted,
                    sizeof(emitted) / sizeof(emitted[0]),
                    &emitted_count,
                    result)) {
                return 0;
            }
        }
    }

    for (function_index = 0; function_index < module_decl->function_count; function_index += 1) {
        const zir_function *function_decl = &module_decl->functions[function_index];
        size_t param_index;
        size_t block_index;

        if (!c_emit_generated_map_helpers_for_type(
                emitter,
                module_decl,
                function_decl->return_type,
                emitted,
                sizeof(emitted) / sizeof(emitted[0]),
                &emitted_count,
                result)) {
            return 0;
        }

        for (param_index = 0; param_index < function_decl->param_count; param_index += 1) {
            if (!c_emit_generated_map_helpers_for_type(
                    emitter,
                    module_decl,
                    function_decl->params[param_index].type_name,
                    emitted,
                    sizeof(emitted) / sizeof(emitted[0]),
                    &emitted_count,
                    result)) {
                return 0;
            }
        }

        for (block_index = 0; block_index < function_decl->block_count; block_index += 1) {
            const zir_block *block = &function_decl->blocks[block_index];
            size_t instruction_index;
            for (instruction_index = 0; instruction_index < block->instruction_count; instruction_index += 1) {
                const zir_instruction *instruction = &block->instructions[instruction_index];
                if (instruction->type_name != NULL &&
                        !c_emit_generated_map_helpers_for_type(
                            emitter,
                            module_decl,
                            instruction->type_name,
                            emitted,
                            sizeof(emitted) / sizeof(emitted[0]),
                            &emitted_count,
                            result)) {
                    return 0;
                }
            }
        }
    }

    return 1;
}
static int c_generated_outcome_is_emitted(
        char emitted[][128],
        size_t emitted_count,
        const char *canonical_name) {
    size_t index;

    for (index = 0; index < emitted_count; index += 1) {
        if (strcmp(emitted[index], canonical_name) == 0) {
            return 1;
        }
    }

    return 0;
}

static int c_generated_outcome_mark_emitted(
        char emitted[][128],
        size_t emitted_capacity,
        size_t *emitted_count,
        const char *canonical_name,
        c_emit_result *result) {
    if (*emitted_count >= emitted_capacity) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "too many generated outcome specializations in one module");
        return 0;
    }

    snprintf(emitted[*emitted_count], 128, "%s", canonical_name);
    *emitted_count += 1;
    return 1;
}

static int c_emit_generated_outcome_helpers_for_type(
        c_emitter *emitter,
        const zir_module *module_decl,
        const char *type_name,
        char emitted[][128],
        size_t emitted_capacity,
        size_t *emitted_count,
        c_emit_result *result) {
    c_outcome_spec spec;
    char value_c_type[128];
    char error_c_type[128];

    if (!c_outcome_spec_for_type(type_name, &spec) || !spec.is_generated) {
        return 1;
    }

    if (c_generated_outcome_is_emitted(emitted, *emitted_count, spec.canonical_name)) {
        return 1;
    }

    if (!c_emit_generated_outcome_helpers_for_type(
            emitter,
            module_decl,
            spec.value_type_name,
            emitted,
            emitted_capacity,
            emitted_count,
            result)) {
        return 0;
    }

    if (!c_emit_generated_outcome_helpers_for_type(
            emitter,
            module_decl,
            spec.error_type_name,
            emitted,
            emitted_capacity,
            emitted_count,
            result)) {
        return 0;
    }

    if (!c_type_to_c(module_decl, spec.error_type_name, error_c_type, sizeof(error_c_type), result)) {
        return 0;
    }
    if (spec.has_value && !c_type_to_c(module_decl, spec.value_type_name, value_c_type, sizeof(value_c_type), result)) {
        return 0;
    }

    if (!(c_begin_line(emitter) &&
            c_begin_line(emitter) &&
            c_buffer_append_format(&emitter->buffer, "typedef struct %s {", spec.c_type_name) &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    zt_bool is_success;") )) {
        return 0;
    }

    if (spec.has_value) {
        if (!(c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    ") &&
                c_emit_typed_name(&emitter->buffer, value_c_type, "value") &&
                c_buffer_append(&emitter->buffer, ";"))) {
            return 0;
        }
    }

    if (!(c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    ") &&
            c_emit_typed_name(&emitter->buffer, error_c_type, "error") &&
            c_buffer_append(&emitter->buffer, ";") &&
            c_begin_line(emitter) &&
            c_buffer_append_format(&emitter->buffer, "} %s;", spec.c_type_name))) {
        return 0;
    }

    if (!(c_begin_line(emitter) &&
            c_begin_line(emitter))) {
        return 0;
    }

    if (spec.has_value) {
        if (!(c_buffer_append_format(&emitter->buffer, "static %s %s(%s value) {", spec.c_type_name, spec.success_fn, value_c_type) &&
                c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "    %s outcome;", spec.c_type_name) &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    memset(&outcome, 0, sizeof(outcome));") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    outcome.is_success = true;") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    outcome.value = value;"))) {
            return 0;
        }
        if (!c_emit_value_clone_in_place(emitter, module_decl, spec.value_type_name, "outcome.value", "    ", result)) {
            return 0;
        }
        if (!(c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    return outcome;") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "}"))) {
            return 0;
        }
    } else {
        if (!(c_buffer_append_format(&emitter->buffer, "static %s %s(void) {", spec.c_type_name, spec.success_fn) &&
                c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "    %s outcome;", spec.c_type_name) &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    memset(&outcome, 0, sizeof(outcome));") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    outcome.is_success = true;") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    return outcome;") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "}"))) {
            return 0;
        }
    }

    if (!(c_begin_line(emitter) &&
            c_begin_line(emitter) &&
            c_buffer_append_format(&emitter->buffer, "static %s %s(%s error) {", spec.c_type_name, spec.failure_fn, error_c_type) &&
            c_begin_line(emitter) &&
            c_buffer_append_format(&emitter->buffer, "    %s outcome;", spec.c_type_name) &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    memset(&outcome, 0, sizeof(outcome));") &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    outcome.is_success = false;") &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    outcome.error = error;"))) {
        return 0;
    }
    if (!c_emit_value_clone_in_place(emitter, module_decl, spec.error_type_name, "outcome.error", "    ", result)) {
        return 0;
    }
    if (!(c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    return outcome;") &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "}"))) {
        return 0;
    }

    if (spec.error_is_text) {
        if (!(c_begin_line(emitter) &&
                c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "static %s %s(const char *message) {", spec.c_type_name, spec.failure_message_fn) &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    zt_text *error = zt_text_from_utf8_literal(message != NULL ? message : \"error\");") &&
                c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "    %s outcome = %s(error);", spec.c_type_name, spec.failure_fn) &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    zt_release(error);") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    return outcome;") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "}"))) {
            return 0;
        }
    }

    if (spec.error_is_core) {
        if (!(c_begin_line(emitter) &&
                c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "static %s %s(const char *message) {", spec.c_type_name, spec.failure_message_fn) &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    zt_core_error error = zt_core_error_from_message(\"error\", message != NULL ? message : \"error\");") &&
                c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "    %s outcome = %s(error);", spec.c_type_name, spec.failure_fn) &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    zt_core_error_dispose(&error);") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    return outcome;") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "}"))) {
            return 0;
        }

        if (!(c_begin_line(emitter) &&
                c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "static %s %s(zt_text *message) {", spec.c_type_name, spec.failure_text_fn) &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    zt_core_error error = zt_core_error_from_text(\"error\", message);") &&
                c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "    %s outcome = %s(error);", spec.c_type_name, spec.failure_fn) &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    zt_core_error_dispose(&error);") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    return outcome;") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "}"))) {
            return 0;
        }
    }

    if (!(c_begin_line(emitter) &&
            c_begin_line(emitter) &&
            c_buffer_append_format(&emitter->buffer, "static zt_bool %s(%s outcome) {", spec.is_success_fn, spec.c_type_name) &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    return outcome.is_success;") &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "}"))) {
        return 0;
    }

    if (spec.has_value) {
        if (!(c_begin_line(emitter) &&
                c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "static %s %s(%s outcome) {", value_c_type, spec.value_fn, spec.c_type_name) &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    if (!outcome.is_success) zt_runtime_error(ZT_ERR_UNWRAP, \"outcome_value on failure\");") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    {") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "        ") &&
                c_emit_typed_name(&emitter->buffer, value_c_type, "value") &&
                c_buffer_append(&emitter->buffer, " = outcome.value;") )) {
            return 0;
        }
        if (!c_emit_value_clone_in_place(emitter, module_decl, spec.value_type_name, "value", "        ", result)) {
            return 0;
        }
        if (!(c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "        return value;") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "    }") &&
                c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "}"))) {
            return 0;
        }
    }

    if (!(c_begin_line(emitter) &&
            c_begin_line(emitter) &&
            c_buffer_append_format(&emitter->buffer, "static %s %s(%s outcome) {", spec.c_type_name, spec.propagate_fn, spec.c_type_name) &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    if (outcome.is_success) "))) {
        return 0;
    }

    if (spec.has_value) {
        if (!(c_buffer_append_format(&emitter->buffer, "return %s(outcome.value);", spec.success_fn))) {
            return 0;
        }
    } else if (!(c_buffer_append_format(&emitter->buffer, "return %s();", spec.success_fn))) {
        return 0;
    }

    if (!(c_begin_line(emitter) &&
            c_buffer_append_format(&emitter->buffer, "    return %s(outcome.error);", spec.failure_fn) &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "}"))) {
        return 0;
    }

    if (!(c_begin_line(emitter) &&
            c_begin_line(emitter) &&
            c_buffer_append_format(&emitter->buffer, "static void %s(%s *outcome) {", spec.dispose_fn, spec.c_type_name) &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    if (outcome == NULL) return;") &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    if (outcome->is_success) {") )) {
        return 0;
    }

    if (spec.has_value && !c_emit_value_dispose_in_place(emitter, module_decl, spec.value_type_name, "outcome->value", "        ", result)) {
        return 0;
    }

    if (!(c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    } else {") )) {
        return 0;
    }

    if (!c_emit_value_dispose_in_place(emitter, module_decl, spec.error_type_name, "outcome->error", "        ", result)) {
        return 0;
    }

    if (!(c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    }") &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "    memset(outcome, 0, sizeof(*outcome));") &&
            c_begin_line(emitter) &&
            c_buffer_append(&emitter->buffer, "}"))) {
        return 0;
    }

    return c_generated_outcome_mark_emitted(emitted, emitted_capacity, emitted_count, spec.canonical_name, result);
}

static int c_emit_generated_outcome_helpers(
        c_emitter *emitter,
        const zir_module *module_decl,
        c_emit_result *result) {
    char emitted[128][128];
    size_t emitted_count = 0;
    size_t struct_index;
    size_t function_index;

    for (struct_index = 0; struct_index < module_decl->struct_count; struct_index += 1) {
        const zir_struct_decl *struct_decl = &module_decl->structs[struct_index];
        size_t field_index;
        for (field_index = 0; field_index < struct_decl->field_count; field_index += 1) {
            if (!c_emit_generated_outcome_helpers_for_type(
                    emitter,
                    module_decl,
                    struct_decl->fields[field_index].type_name,
                    emitted,
                    sizeof(emitted) / sizeof(emitted[0]),
                    &emitted_count,
                    result)) {
                return 0;
            }
        }
    }

    for (function_index = 0; function_index < module_decl->function_count; function_index += 1) {
        const zir_function *function_decl = &module_decl->functions[function_index];
        size_t param_index;
        size_t block_index;

        if (!c_emit_generated_outcome_helpers_for_type(
                emitter,
                module_decl,
                function_decl->return_type,
                emitted,
                sizeof(emitted) / sizeof(emitted[0]),
                &emitted_count,
                result)) {
            return 0;
        }

        for (param_index = 0; param_index < function_decl->param_count; param_index += 1) {
            if (!c_emit_generated_outcome_helpers_for_type(
                    emitter,
                    module_decl,
                    function_decl->params[param_index].type_name,
                    emitted,
                    sizeof(emitted) / sizeof(emitted[0]),
                    &emitted_count,
                    result)) {
                return 0;
            }
        }

        for (block_index = 0; block_index < function_decl->block_count; block_index += 1) {
            const zir_block *block = &function_decl->blocks[block_index];
            size_t instruction_index;
            for (instruction_index = 0; instruction_index < block->instruction_count; instruction_index += 1) {
                const zir_instruction *instruction = &block->instructions[instruction_index];
                if (instruction->type_name != NULL &&
                        !c_emit_generated_outcome_helpers_for_type(
                            emitter,
                            module_decl,
                            instruction->type_name,
                            emitted,
                            sizeof(emitted) / sizeof(emitted[0]),
                            &emitted_count,
                            result)) {
                    return 0;
                }
            }
        }
    }

    return 1;
}

static void c_build_module_var_init_symbol(const zir_module *module_decl, char *dest, size_t capacity) {
    char module_name[64];
    c_copy_sanitized(module_name, sizeof(module_name), c_safe_text(module_decl->name));
    snprintf(dest, capacity, "zt_%s__init_module_vars", module_name);
}

static void c_build_module_var_init_guard_symbol(const zir_module *module_decl, char *dest, size_t capacity) {
    char module_name[64];
    c_copy_sanitized(module_name, sizeof(module_name), c_safe_text(module_decl->name));
    snprintf(dest, capacity, "zt_%s__module_vars_initialized", module_name);
}

static int c_emit_module_var_section(
        c_emitter *emitter,
        const zir_module *module_decl,
        c_emit_result *result) {
    size_t index;
    char init_symbol[128];
    char guard_symbol[128];
    zir_function pseudo_function;

    if (module_decl == NULL || module_decl->module_var_count == 0) {
        return 1;
    }

    memset(&pseudo_function, 0, sizeof(pseudo_function));
    pseudo_function.return_type = "void";

    for (index = 0; index < module_decl->module_var_count; index += 1) {
        const zir_module_var *module_var = &module_decl->module_vars[index];
        char c_type[64];
        int is_pointer_managed;
        int needs_value_cleanup;

        if (!c_type_to_c(module_decl, module_var->type_name, c_type, sizeof(c_type), result)) {
            return 0;
        }

        is_pointer_managed = c_type_is_managed(module_var->type_name);
        needs_value_cleanup = c_type_needs_managed_cleanup(module_decl, module_var->type_name) && !is_pointer_managed;

        if (!(c_begin_line(emitter) &&
                c_buffer_append(&emitter->buffer, "static "))) {
            return 0;
        }

        if (!c_emit_typed_name(&emitter->buffer, c_type, c_safe_text(module_var->name))) {
            return 0;
        }

        if (is_pointer_managed) {
            if (!c_buffer_append(&emitter->buffer, " = NULL;")) {
                return 0;
            }
        } else if (needs_value_cleanup) {
            if (!c_buffer_append(&emitter->buffer, " = {0};")) {
                return 0;
            }
        } else if (!c_buffer_append(&emitter->buffer, ";")) {
            return 0;
        }
    }

    c_build_module_var_init_symbol(module_decl, init_symbol, sizeof(init_symbol));
    c_build_module_var_init_guard_symbol(module_decl, guard_symbol, sizeof(guard_symbol));

    if (!(c_begin_line(emitter) &&
            c_buffer_append_format(&emitter->buffer, "static zt_bool %s = false;", guard_symbol) &&
            c_begin_line(emitter) &&
            c_buffer_append_format(&emitter->buffer, "static void %s(void) {", init_symbol) &&
            c_begin_line(emitter) &&
            c_buffer_append_format(&emitter->buffer, "    if (%s) return;", guard_symbol) &&
            c_begin_line(emitter) &&
            c_buffer_append_format(&emitter->buffer, "    %s = true;", guard_symbol))) {
        return 0;
    }

    for (index = 0; index < module_decl->module_var_count; index += 1) {
        const zir_module_var *module_var = &module_decl->module_vars[index];
        int is_pointer_managed = c_type_is_managed(module_var->type_name);

        if (module_var->init_expr == NULL && c_is_blank(module_var->init_expr_text)) {
            continue;
        }

        if (!(c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "    %s = ", c_safe_text(module_var->name)))) {
            return 0;
        }

        if (module_var->init_expr != NULL) {
            if (is_pointer_managed) {
                if (!c_emit_owned_managed_zir_expr(
                        emitter,
                        module_decl,
                        &pseudo_function,
                        module_var->init_expr,
                        module_var->type_name,
                        result)) {
                    return 0;
                }
            } else if (!c_emit_zir_expr(
                    emitter,
                    module_decl,
                    &pseudo_function,
                    module_var->init_expr,
                    module_var->type_name,
                    result)) {
                return 0;
            }
        } else if (is_pointer_managed) {
            if (!c_emit_owned_managed_expr(
                    emitter,
                    module_decl,
                    &pseudo_function,
                    module_var->init_expr_text,
                    module_var->type_name,
                    result)) {
                return 0;
            }
        } else if (!c_emit_expr(
                emitter,
                module_decl,
                &pseudo_function,
                module_var->init_expr_text,
                module_var->type_name,
                result)) {
            return 0;
        }

        if (!c_buffer_append(&emitter->buffer, ";")) {
            return 0;
        }
    }

    return c_begin_line(emitter) &&
           c_buffer_append(&emitter->buffer, "}");
}

static int c_emit_function_signature(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        int with_storage,
        c_emit_result *result) {
    char return_type[64];
    char symbol[128];
    size_t param_index;

    if (!c_type_to_c(module_decl, function_decl->return_type, return_type, sizeof(return_type), result)) {
        return 0;
    }

    c_build_function_symbol(module_decl, function_decl, symbol, sizeof(symbol));

    if (with_storage) {
        if (!(c_buffer_append(&emitter->buffer, "static ") &&
                c_emit_typed_name(&emitter->buffer, return_type, symbol) &&
                c_buffer_append(&emitter->buffer, "("))) {
            return 0;
        }
    } else {
        if (!(c_emit_typed_name(&emitter->buffer, return_type, symbol) &&
                c_buffer_append(&emitter->buffer, "("))) {
            return 0;
        }
    }

    if (function_decl->param_count == 0) {
        return c_buffer_append(&emitter->buffer, "void)");
    }

    for (param_index = 0; param_index < function_decl->param_count; param_index += 1) {
        char c_type[64];
        const zir_param *param = &function_decl->params[param_index];

        if (param_index > 0 && !c_buffer_append(&emitter->buffer, ", ")) {
            return 0;
        }

        if (!c_type_to_c(module_decl, param->type_name, c_type, sizeof(c_type), result)) {
            return 0;
        }

        if (function_decl->receiver_type_name != NULL &&
                function_decl->is_mutating &&
                param_index == 0 &&
                strcmp(c_safe_text(param->name), "self") == 0) {
            char pointer_type[80];
            snprintf(pointer_type, sizeof(pointer_type), "%s *", c_type);
            if (!c_emit_typed_name(&emitter->buffer, pointer_type, c_safe_text(param->name))) {
                return 0;
            }
        } else if (!c_emit_typed_name(&emitter->buffer, c_type, c_safe_text(param->name))) {
            return 0;
        }
    }

    return c_buffer_append(&emitter->buffer, ")");
}

static int c_emit_function_definition(
        c_emitter *emitter,
        const zir_module *module_decl,
        const zir_function *function_decl,
        c_emit_result *result) {
    size_t block_index;
    c_local_decl locals[128];
    size_t local_count = 0;

    if (!c_collect_locals(module_decl, function_decl, locals, sizeof(locals) / sizeof(locals[0]), &local_count, result)) {
        return 0;
    }

    if (!c_begin_line(emitter) || !c_begin_line(emitter)) {
        return 0;
    }

    if (!c_emit_function_signature(emitter, module_decl, function_decl, 1, result)) {
        return 0;
    }

    if (!c_buffer_append(&emitter->buffer, " {")) {
        return 0;
    }

    if (!c_emit_locals(emitter, module_decl, function_decl, locals, local_count, result)) {
        return 0;
    }

    if (module_decl->module_var_count > 0) {
        char init_symbol[128];
        c_build_module_var_init_symbol(module_decl, init_symbol, sizeof(init_symbol));
        if (!(c_begin_line(emitter) &&
                c_buffer_append_format(&emitter->buffer, "    %s();", init_symbol))) {
            return 0;
        }
    }

    if (function_decl->block_count > 0) {
        char entry_label[96];

        if (!c_begin_line(emitter)) {
            return 0;
        }

        c_build_block_label(function_decl->blocks[0].label, entry_label, sizeof(entry_label));
        if (!c_buffer_append_format(&emitter->buffer, "    goto %s;", entry_label)) {
            return 0;
        }
    }

    for (block_index = 0; block_index < function_decl->block_count; block_index += 1) {
        char label_name[96];
        const zir_block *block = &function_decl->blocks[block_index];
        size_t instruction_index;

        if (!c_begin_line(emitter) || !c_begin_line(emitter)) {
            return 0;
        }

        c_build_block_label(block->label, label_name, sizeof(label_name));
        if (!c_buffer_append_format(&emitter->buffer, "%s:", label_name)) {
            return 0;
        }

        for (instruction_index = 0; instruction_index < block->instruction_count; instruction_index += 1) {
            if (!c_emit_instruction(emitter, module_decl, function_decl, &block->instructions[instruction_index], result)) {
                return 0;
            }
        }

        if (!c_emit_terminator(emitter, module_decl, function_decl, locals, local_count, block, result)) {
            return 0;
        }
    }

    if (!c_emit_cleanup(emitter, module_decl, function_decl, locals, local_count, result)) {
        return 0;
    }

    return c_begin_line(emitter) && c_buffer_append(&emitter->buffer, "}");
}

static int c_emit_main_wrapper(c_emitter *emitter, const zir_module *module_decl, const zir_function *function_decl, c_emit_result *result) {
    char symbol[128];
    int returns_int;
    int returns_void;
    c_outcome_spec main_outcome_spec;
    int returns_outcome_void;

    if (function_decl->param_count != 0) {
        c_emit_set_result(
            result,
            C_EMIT_INVALID_MAIN_SIGNATURE,
            "main function cannot have parameters in the current C emitter subset"
        );
        return 0;
    }

    returns_int = c_type_is(function_decl->return_type, "int");
    returns_void = c_type_is(function_decl->return_type, "void");
    returns_outcome_void = c_outcome_spec_for_type(function_decl->return_type, &main_outcome_spec) && !main_outcome_spec.has_value;

    if (!returns_int && !returns_void && !returns_outcome_void) {
        c_emit_set_result(
            result,
            C_EMIT_INVALID_MAIN_SIGNATURE,
            "main must return int, void or result<void,E>, got '%s'",
            function_decl->return_type
        );
        return 0;
    }

    c_build_function_symbol(module_decl, function_decl, symbol, sizeof(symbol));

    if (returns_void) {
        return c_begin_line(emitter) &&
               c_begin_line(emitter) &&
               c_buffer_append(&emitter->buffer, "int main(void) {") &&
               c_begin_line(emitter) &&
               c_buffer_append_format(&emitter->buffer, "    %s();", symbol) &&
               c_begin_line(emitter) &&
               c_buffer_append(&emitter->buffer, "    return 0;") &&
               c_begin_line(emitter) &&
               c_buffer_append(&emitter->buffer, "}");
    }

    if (returns_int) {
        return c_begin_line(emitter) &&
               c_begin_line(emitter) &&
               c_buffer_append(&emitter->buffer, "int main(void) {") &&
               c_begin_line(emitter) &&
               c_buffer_append_format(&emitter->buffer, "    return (int)%s();", symbol) &&
               c_begin_line(emitter) &&
               c_buffer_append(&emitter->buffer, "}");
    }

    return c_begin_line(emitter) &&
           c_begin_line(emitter) &&
           c_buffer_append(&emitter->buffer, "int main(void) {") &&
           c_begin_line(emitter) &&
           c_buffer_append_format(&emitter->buffer, "    %s __zt_main_result = %s();", main_outcome_spec.c_type_name, symbol) &&
           c_begin_line(emitter) &&
           c_buffer_append(&emitter->buffer, "    if (!__zt_main_result.is_success) {") &&
           (!main_outcome_spec.error_is_core ||
                (c_begin_line(emitter) &&
                 c_buffer_append(&emitter->buffer, "        if (__zt_main_result.error.message != NULL) {") &&
                 c_begin_line(emitter) &&
                 c_buffer_append(&emitter->buffer, "            (void)zt_host_write_stderr(__zt_main_result.error.message);") &&
                 c_begin_line(emitter) &&
                 c_buffer_append(&emitter->buffer, "        }"))) &&
           (!main_outcome_spec.error_is_text ||
                (c_begin_line(emitter) &&
                 c_buffer_append(&emitter->buffer, "        if (__zt_main_result.error != NULL) {") &&
                 c_begin_line(emitter) &&
                 c_buffer_append(&emitter->buffer, "            (void)zt_host_write_stderr(__zt_main_result.error);") &&
                 c_begin_line(emitter) &&
                 c_buffer_append(&emitter->buffer, "        }"))) &&
           c_begin_line(emitter) &&
           c_buffer_append_format(&emitter->buffer, "        %s(&__zt_main_result);", main_outcome_spec.dispose_fn) &&
           c_begin_line(emitter) &&
           c_buffer_append(&emitter->buffer, "        return 1;") &&
           c_begin_line(emitter) &&
           c_buffer_append(&emitter->buffer, "    }") &&
           c_begin_line(emitter) &&
           c_buffer_append_format(&emitter->buffer, "    %s(&__zt_main_result);", main_outcome_spec.dispose_fn) &&
           c_begin_line(emitter) &&
           c_buffer_append(&emitter->buffer, "    return 0;") &&
           c_begin_line(emitter) &&
           c_buffer_append(&emitter->buffer, "}");
}

int c_emitter_emit_module(c_emitter *emitter, const zir_module *module_decl, c_emit_result *result) {
    size_t function_index;
    const zir_function *main_function = NULL;

    c_emitter_reset(emitter);
    c_emit_result_init(result);

    if (module_decl == NULL) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "module pointer cannot be null");
        return 0;
    }

    c_active_module_for_symbol_lookup = module_decl;

        if (!(c_emit_line(emitter, "#include \"runtime/c/zenith_rt.h\"") &&
            (!c_module_requires_template_header(module_decl) ||
                c_emit_line(emitter, "#include \"runtime/c/zenith_rt_templates.h\"")) &&
            c_emit_line(emitter, "#include <stdio.h>") &&
            (!c_module_requires_string_header(module_decl) ||
                c_emit_line(emitter, "#include <string.h>")))) {
        c_emit_set_result(result, C_EMIT_INVALID_INPUT, "unable to write emitter prologue");
        c_active_module_for_symbol_lookup = NULL;
        return 0;
    }

    if (!c_begin_line(emitter)) {
        c_active_module_for_symbol_lookup = NULL;
        return 0;
    }

    if (!c_emit_struct_definitions(emitter, module_decl, result)) {
        c_active_module_for_symbol_lookup = NULL;
        return 0;
    }

    if (!c_emit_enum_definitions(emitter, module_decl, result)) {
        c_active_module_for_symbol_lookup = NULL;
        return 0;
    }

    if (!c_emit_generated_map_helpers(emitter, module_decl, result)) {
        c_active_module_for_symbol_lookup = NULL;
        return 0;
    }

    if (!c_emit_generated_outcome_helpers(emitter, module_decl, result)) {
        c_active_module_for_symbol_lookup = NULL;
        return 0;
    }

    for (function_index = 0; function_index < module_decl->function_count; function_index += 1) {
        const zir_function *function_decl = &module_decl->functions[function_index];

        if (!c_begin_line(emitter)) {
            c_active_module_for_symbol_lookup = NULL;
            return 0;
        }

        if (!c_emit_function_signature(emitter, module_decl, function_decl, 1, result) ||
                !c_buffer_append(&emitter->buffer, ";")) {
            c_active_module_for_symbol_lookup = NULL;
            return 0;
        }

        if (strcmp(function_decl->name, "main") == 0) {
            main_function = function_decl;
        }
    }

    if (!c_emit_module_var_section(emitter, module_decl, result)) {
        c_active_module_for_symbol_lookup = NULL;
        return 0;
    }

    for (function_index = 0; function_index < module_decl->function_count; function_index += 1) {
        if (!c_emit_function_definition(emitter, module_decl, &module_decl->functions[function_index], result)) {
            c_active_module_for_symbol_lookup = NULL;
            return 0;
        }
    }

    if (main_function != NULL) {
        if (!c_emit_main_wrapper(emitter, module_decl, main_function, result)) {
            c_active_module_for_symbol_lookup = NULL;
            return 0;
        }
    }

    c_emit_result_init(result);
    c_active_module_for_symbol_lookup = NULL;
    return 1;
}








