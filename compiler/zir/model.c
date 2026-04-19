#include "compiler/zir/model.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct zir_string_buffer {
    char *data;
    size_t length;
    size_t capacity;
} zir_string_buffer;

static char *zir_strdup_owned(const char *text) {
    size_t length;
    char *copy;
    if (text == NULL) return NULL;
    length = strlen(text);
    copy = (char *)malloc(length + 1);
    if (copy == NULL) return NULL;
    memcpy(copy, text, length + 1);
    return copy;
}

static void zir_free_owned_nonempty(const char *text) {
    if (text != NULL && text[0] != '\0') {
        free((void *)text);
    }
}

static void *zir_grow_array(void *items, size_t *capacity, size_t item_size) {
    size_t new_capacity;
    void *new_items;
    if (capacity == NULL) return NULL;
    new_capacity = *capacity == 0 ? 4 : (*capacity * 2);
    new_items = realloc(items, new_capacity * item_size);
    if (new_items != NULL) *capacity = new_capacity;
    return new_items;
}

static void zir_string_buffer_init(zir_string_buffer *buffer) {
    buffer->data = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
}

static int zir_string_buffer_reserve(zir_string_buffer *buffer, size_t additional) {
    size_t needed;
    size_t new_capacity;
    char *new_data;
    if (buffer == NULL) return 0;
    needed = buffer->length + additional + 1;
    if (needed <= buffer->capacity) return 1;
    new_capacity = buffer->capacity == 0 ? 64 : buffer->capacity;
    while (new_capacity < needed) new_capacity *= 2;
    new_data = (char *)realloc(buffer->data, new_capacity);
    if (new_data == NULL) return 0;
    buffer->data = new_data;
    buffer->capacity = new_capacity;
    return 1;
}

static int zir_string_buffer_append_n(zir_string_buffer *buffer, const char *text, size_t length) {
    if (buffer == NULL || text == NULL) return 0;
    if (!zir_string_buffer_reserve(buffer, length)) return 0;
    memcpy(buffer->data + buffer->length, text, length);
    buffer->length += length;
    buffer->data[buffer->length] = '\0';
    return 1;
}

static int zir_string_buffer_append(zir_string_buffer *buffer, const char *text) {
    if (text == NULL) return zir_string_buffer_append_n(buffer, "", 0);
    return zir_string_buffer_append_n(buffer, text, strlen(text));
}

static int zir_string_buffer_append_char(zir_string_buffer *buffer, char value) {
    return zir_string_buffer_append_n(buffer, &value, 1);
}

static int zir_string_buffer_append_format(zir_string_buffer *buffer, const char *format, ...) {
    va_list args;
    va_list copy;
    int needed;
    if (buffer == NULL || format == NULL) return 0;
    va_start(args, format);
    va_copy(copy, args);
    needed = vsnprintf(NULL, 0, format, copy);
    va_end(copy);
    if (needed < 0) {
        va_end(args);
        return 0;
    }
    if (!zir_string_buffer_reserve(buffer, (size_t)needed)) {
        va_end(args);
        return 0;
    }
    vsnprintf(buffer->data + buffer->length, buffer->capacity - buffer->length, format, args);
    buffer->length += (size_t)needed;
    va_end(args);
    return 1;
}

static int zir_render_string_literal(zir_string_buffer *buffer, const char *text, int with_const_prefix) {
    size_t i;
    if (with_const_prefix && !zir_string_buffer_append(buffer, "const ")) return 0;
    if (!zir_string_buffer_append_char(buffer, '"')) return 0;
    if (text == NULL) text = "";
    for (i = 0; text[i] != '\0'; i++) {
        switch (text[i]) {
            case '\\': if (!zir_string_buffer_append(buffer, "\\\\")) return 0; break;
            case '"': if (!zir_string_buffer_append(buffer, "\\\"")) return 0; break;
            case '\n': if (!zir_string_buffer_append(buffer, "\\n")) return 0; break;
            case '\r': if (!zir_string_buffer_append(buffer, "\\r")) return 0; break;
            case '\t': if (!zir_string_buffer_append(buffer, "\\t")) return 0; break;
            default: if (!zir_string_buffer_append_char(buffer, text[i])) return 0; break;
        }
    }
    return zir_string_buffer_append_char(buffer, '"');
}

zir_span zir_make_span(const char *source_name, size_t line, size_t column) {
    zir_span span;
    span.source_name = source_name;
    span.line = line;
    span.column = column;
    return span;
}

int zir_span_is_known(zir_span span) {
    return span.source_name != NULL &&
           span.source_name[0] != '\0' &&
           span.line > 0 &&
           span.column > 0;
}

const char *zir_expr_kind_name(zir_expr_kind kind) {
    switch (kind) {
        case ZIR_EXPR_NAME: return "name";
        case ZIR_EXPR_INT: return "int";
        case ZIR_EXPR_FLOAT: return "float";
        case ZIR_EXPR_BOOL: return "bool";
        case ZIR_EXPR_STRING: return "string";
        case ZIR_EXPR_BYTES: return "bytes";
        case ZIR_EXPR_COPY: return "copy";
        case ZIR_EXPR_UNARY: return "unary";
        case ZIR_EXPR_BINARY: return "binary";
        case ZIR_EXPR_CALL_DIRECT: return "call_direct";
        case ZIR_EXPR_CALL_EXTERN: return "call_extern";
        case ZIR_EXPR_CALL_RUNTIME_INTRINSIC: return "call_runtime_intrinsic";
        case ZIR_EXPR_MAKE_STRUCT: return "make_struct";
        case ZIR_EXPR_MAKE_LIST: return "make_list";
        case ZIR_EXPR_MAKE_MAP: return "make_map";
        case ZIR_EXPR_GET_FIELD: return "get_field";
        case ZIR_EXPR_SET_FIELD: return "set_field";
        case ZIR_EXPR_INDEX_SEQ: return "index_seq";
        case ZIR_EXPR_SLICE_SEQ: return "slice_seq";
        case ZIR_EXPR_LIST_LEN: return "list_len";
        case ZIR_EXPR_MAP_LEN: return "map_len";
        case ZIR_EXPR_LIST_PUSH: return "list_push";
        case ZIR_EXPR_LIST_SET: return "list_set";
        case ZIR_EXPR_MAP_SET: return "map_set";
        case ZIR_EXPR_OPTIONAL_PRESENT: return "optional_present";
        case ZIR_EXPR_OPTIONAL_EMPTY: return "optional_empty";
        case ZIR_EXPR_OPTIONAL_IS_PRESENT: return "optional_is_present";
        case ZIR_EXPR_COALESCE: return "coalesce";
        case ZIR_EXPR_OUTCOME_SUCCESS: return "outcome_success";
        case ZIR_EXPR_OUTCOME_FAILURE: return "outcome_failure";
        case ZIR_EXPR_OUTCOME_IS_SUCCESS: return "outcome_is_success";
        case ZIR_EXPR_OUTCOME_VALUE: return "outcome_value";
        case ZIR_EXPR_TRY_PROPAGATE: return "try_propagate";
        default: return "unknown";
    }
}

zir_expr_list zir_expr_list_make(void) {
    zir_expr_list list;
    list.items = NULL;
    list.count = 0;
    list.capacity = 0;
    return list;
}

void zir_expr_list_push(zir_expr_list *list, zir_expr *expr) {
    zir_expr **new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (zir_expr **)zir_grow_array(list->items, &list->capacity, sizeof(zir_expr *));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = expr;
}

void zir_expr_list_dispose(zir_expr_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) zir_expr_dispose(list->items[i]);
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

zir_named_expr_list zir_named_expr_list_make(void) {
    zir_named_expr_list list;
    list.items = NULL;
    list.count = 0;
    list.capacity = 0;
    return list;
}

void zir_named_expr_list_push(zir_named_expr_list *list, zir_named_expr entry) {
    zir_named_expr *new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (zir_named_expr *)zir_grow_array(list->items, &list->capacity, sizeof(zir_named_expr));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = entry;
}

void zir_named_expr_list_dispose(zir_named_expr_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) {
        free((void *)list->items[i].name);
        zir_expr_dispose(list->items[i].value);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

zir_map_entry_list zir_map_entry_list_make(void) {
    zir_map_entry_list list;
    list.items = NULL;
    list.count = 0;
    list.capacity = 0;
    return list;
}

void zir_map_entry_list_push(zir_map_entry_list *list, zir_map_entry entry) {
    zir_map_entry *new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (zir_map_entry *)zir_grow_array(list->items, &list->capacity, sizeof(zir_map_entry));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = entry;
}

void zir_map_entry_list_dispose(zir_map_entry_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) {
        zir_expr_dispose(list->items[i].key);
        zir_expr_dispose(list->items[i].value);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

static zir_expr *zir_expr_make(zir_expr_kind kind) {
    zir_expr *expr = (zir_expr *)calloc(1, sizeof(zir_expr));
    if (expr == NULL) return NULL;
    expr->kind = kind;
    expr->span = zir_make_span(NULL, 0, 0);
    return expr;
}

static zir_expr *zir_expr_make_text_like(zir_expr_kind kind, const char *text) {
    zir_expr *expr = zir_expr_make(kind);
    if (expr == NULL) return NULL;
    expr->as.text.text = zir_strdup_owned(text);
    return expr;
}

zir_expr *zir_expr_make_name(const char *text) { return zir_expr_make_text_like(ZIR_EXPR_NAME, text); }
zir_expr *zir_expr_make_int(const char *text) { return zir_expr_make_text_like(ZIR_EXPR_INT, text); }
zir_expr *zir_expr_make_float(const char *text) { return zir_expr_make_text_like(ZIR_EXPR_FLOAT, text); }
zir_expr *zir_expr_make_string(const char *text) { return zir_expr_make_text_like(ZIR_EXPR_STRING, text); }
zir_expr *zir_expr_make_bytes(const char *hex_text) { return zir_expr_make_text_like(ZIR_EXPR_BYTES, hex_text); }

zir_expr *zir_expr_make_bool(int value) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_BOOL);
    if (expr == NULL) return NULL;
    expr->as.bool_literal.value = value ? 1 : 0;
    return expr;
}

zir_expr *zir_expr_make_copy(zir_expr *value) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_COPY);
    if (expr == NULL) return NULL;
    expr->as.single.value = value;
    return expr;
}

zir_expr *zir_expr_make_unary(const char *op_name, zir_expr *operand) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_UNARY);
    if (expr == NULL) return NULL;
    expr->as.unary.op_name = zir_strdup_owned(op_name);
    expr->as.unary.operand = operand;
    return expr;
}

zir_expr *zir_expr_make_binary(const char *op_name, zir_expr *left, zir_expr *right) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_BINARY);
    if (expr == NULL) return NULL;
    expr->as.binary.op_name = zir_strdup_owned(op_name);
    expr->as.binary.left = left;
    expr->as.binary.right = right;
    return expr;
}

static zir_expr *zir_expr_make_call_like(zir_expr_kind kind, const char *callee_name) {
    zir_expr *expr = zir_expr_make(kind);
    if (expr == NULL) return NULL;
    expr->as.call.callee_name = zir_strdup_owned(callee_name);
    expr->as.call.args = zir_expr_list_make();
    return expr;
}

zir_expr *zir_expr_make_call_direct(const char *callee_name) { return zir_expr_make_call_like(ZIR_EXPR_CALL_DIRECT, callee_name); }
zir_expr *zir_expr_make_call_extern(const char *callee_name) { return zir_expr_make_call_like(ZIR_EXPR_CALL_EXTERN, callee_name); }
zir_expr *zir_expr_make_call_runtime_intrinsic(const char *callee_name) { return zir_expr_make_call_like(ZIR_EXPR_CALL_RUNTIME_INTRINSIC, callee_name); }

zir_expr *zir_expr_make_make_struct(const char *type_name) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_MAKE_STRUCT);
    if (expr == NULL) return NULL;
    expr->as.make_struct.type_name = zir_strdup_owned(type_name);
    expr->as.make_struct.fields = zir_named_expr_list_make();
    return expr;
}

zir_expr *zir_expr_make_make_list(const char *item_type_name) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_MAKE_LIST);
    if (expr == NULL) return NULL;
    expr->as.make_list.item_type_name = zir_strdup_owned(item_type_name);
    expr->as.make_list.items = zir_expr_list_make();
    return expr;
}

zir_expr *zir_expr_make_make_map(const char *key_type_name, const char *value_type_name) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_MAKE_MAP);
    if (expr == NULL) return NULL;
    expr->as.make_map.key_type_name = zir_strdup_owned(key_type_name);
    expr->as.make_map.value_type_name = zir_strdup_owned(value_type_name);
    expr->as.make_map.entries = zir_map_entry_list_make();
    return expr;
}

zir_expr *zir_expr_make_get_field(zir_expr *object, const char *field_name) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_GET_FIELD);
    if (expr == NULL) return NULL;
    expr->as.field.object = object;
    expr->as.field.field_name = zir_strdup_owned(field_name);
    return expr;
}

zir_expr *zir_expr_make_set_field(zir_expr *object, const char *field_name, zir_expr *value) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_SET_FIELD);
    if (expr == NULL) return NULL;
    expr->as.field.object = object;
    expr->as.field.field_name = zir_strdup_owned(field_name);
    expr->as.field.value = value;
    return expr;
}

zir_expr *zir_expr_make_index_seq(zir_expr *sequence, zir_expr *index) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_INDEX_SEQ);
    if (expr == NULL) return NULL;
    expr->as.sequence.first = sequence;
    expr->as.sequence.second = index;
    return expr;
}

zir_expr *zir_expr_make_slice_seq(zir_expr *sequence, zir_expr *start, zir_expr *end) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_SLICE_SEQ);
    if (expr == NULL) return NULL;
    expr->as.sequence.first = sequence;
    expr->as.sequence.second = start;
    expr->as.sequence.third = end;
    return expr;
}

zir_expr *zir_expr_make_list_len(zir_expr *sequence) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_LIST_LEN);
    if (expr == NULL) return NULL;
    expr->as.single.value = sequence;
    return expr;
}

zir_expr *zir_expr_make_map_len(zir_expr *map) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_MAP_LEN);
    if (expr == NULL) return NULL;
    expr->as.single.value = map;
    return expr;
}

zir_expr *zir_expr_make_list_push(zir_expr *target, zir_expr *value) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_LIST_PUSH);
    if (expr == NULL) return NULL;
    expr->as.sequence.first = target;
    expr->as.sequence.second = value;
    return expr;
}

zir_expr *zir_expr_make_list_set(zir_expr *target, zir_expr *index, zir_expr *value) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_LIST_SET);
    if (expr == NULL) return NULL;
    expr->as.sequence.first = target;
    expr->as.sequence.second = index;
    expr->as.sequence.third = value;
    return expr;
}

zir_expr *zir_expr_make_map_set(zir_expr *target, zir_expr *key, zir_expr *value) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_MAP_SET);
    if (expr == NULL) return NULL;
    expr->as.sequence.first = target;
    expr->as.sequence.second = key;
    expr->as.sequence.third = value;
    return expr;
}

zir_expr *zir_expr_make_optional_present(zir_expr *value) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_OPTIONAL_PRESENT);
    if (expr == NULL) return NULL;
    expr->as.single.value = value;
    return expr;
}

zir_expr *zir_expr_make_optional_empty(const char *type_name) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_OPTIONAL_EMPTY);
    if (expr == NULL) return NULL;
    expr->as.type_only.type_name = zir_strdup_owned(type_name);
    return expr;
}

zir_expr *zir_expr_make_optional_is_present(zir_expr *value) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_OPTIONAL_IS_PRESENT);
    if (expr == NULL) return NULL;
    expr->as.single.value = value;
    return expr;
}

zir_expr *zir_expr_make_coalesce(zir_expr *value, zir_expr *fallback) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_COALESCE);
    if (expr == NULL) return NULL;
    expr->as.sequence.first = value;
    expr->as.sequence.second = fallback;
    return expr;
}

zir_expr *zir_expr_make_outcome_success(zir_expr *value) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_OUTCOME_SUCCESS);
    if (expr == NULL) return NULL;
    expr->as.single.value = value;
    return expr;
}

zir_expr *zir_expr_make_outcome_failure(zir_expr *value) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_OUTCOME_FAILURE);
    if (expr == NULL) return NULL;
    expr->as.single.value = value;
    return expr;
}

zir_expr *zir_expr_make_outcome_is_success(zir_expr *value) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_OUTCOME_IS_SUCCESS);
    if (expr == NULL) return NULL;
    expr->as.single.value = value;
    return expr;
}

zir_expr *zir_expr_make_outcome_value(zir_expr *value) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_OUTCOME_VALUE);
    if (expr == NULL) return NULL;
    expr->as.single.value = value;
    return expr;
}

zir_expr *zir_expr_make_try_propagate(zir_expr *value) {
    zir_expr *expr = zir_expr_make(ZIR_EXPR_TRY_PROPAGATE);
    if (expr == NULL) return NULL;
    expr->as.single.value = value;
    return expr;
}

void zir_expr_call_add_arg(zir_expr *expr, zir_expr *arg) {
    if (expr == NULL) return;
    if (expr->kind != ZIR_EXPR_CALL_DIRECT && expr->kind != ZIR_EXPR_CALL_EXTERN && expr->kind != ZIR_EXPR_CALL_RUNTIME_INTRINSIC) return;
    zir_expr_list_push(&expr->as.call.args, arg);
}

void zir_expr_make_struct_add_field(zir_expr *expr, const char *name, zir_expr *value) {
    zir_named_expr entry;
    if (expr == NULL || expr->kind != ZIR_EXPR_MAKE_STRUCT) return;
    entry.name = zir_strdup_owned(name);
    entry.value = value;
    entry.span = zir_make_span(NULL, 0, 0);
    zir_named_expr_list_push(&expr->as.make_struct.fields, entry);
}

void zir_expr_make_list_add_item(zir_expr *expr, zir_expr *item) {
    if (expr == NULL || expr->kind != ZIR_EXPR_MAKE_LIST) return;
    zir_expr_list_push(&expr->as.make_list.items, item);
}

void zir_expr_make_map_add_entry(zir_expr *expr, zir_expr *key, zir_expr *value) {
    zir_map_entry entry;
    if (expr == NULL || expr->kind != ZIR_EXPR_MAKE_MAP) return;
    entry.key = key;
    entry.value = value;
    entry.span = zir_make_span(NULL, 0, 0);
    zir_map_entry_list_push(&expr->as.make_map.entries, entry);
}

static int zir_render_expr(zir_string_buffer *buffer, const zir_expr *expr);

static int zir_render_expr_list(zir_string_buffer *buffer, const zir_expr_list *list) {
    size_t i;
    if (buffer == NULL || list == NULL) return 0;
    for (i = 0; i < list->count; i++) {
        if (i > 0 && !zir_string_buffer_append(buffer, ", ")) return 0;
        if (!zir_render_expr(buffer, list->items[i])) return 0;
    }
    return 1;
}

static int zir_render_expr(zir_string_buffer *buffer, const zir_expr *expr) {
    size_t i;
    if (expr == NULL) return zir_string_buffer_append(buffer, "");
    switch (expr->kind) {
        case ZIR_EXPR_NAME:
        case ZIR_EXPR_INT:
        case ZIR_EXPR_FLOAT:
            return zir_string_buffer_append(buffer, expr->as.text.text != NULL ? expr->as.text.text : "");
        case ZIR_EXPR_BOOL:
            return zir_string_buffer_append(buffer, expr->as.bool_literal.value ? "true" : "false");
        case ZIR_EXPR_STRING:
            return zir_render_string_literal(buffer, expr->as.text.text, 1);
        case ZIR_EXPR_BYTES:
            if (!zir_string_buffer_append(buffer, "const bytes \"")) return 0;
            if (expr->as.text.text != NULL) {
                for (i = 0; expr->as.text.text[i] != '\0'; i++) {
                    if (i > 0 && (i % 2) == 0 && !zir_string_buffer_append_char(buffer, ' ')) return 0;
                    if (!zir_string_buffer_append_char(buffer, expr->as.text.text[i])) return 0;
                }
            }
            return zir_string_buffer_append_char(buffer, '"');
        case ZIR_EXPR_COPY:
            return zir_string_buffer_append(buffer, "copy ") && zir_render_expr(buffer, expr->as.single.value);
        case ZIR_EXPR_UNARY:
            return zir_string_buffer_append_format(buffer, "unary.%s ", expr->as.unary.op_name != NULL ? expr->as.unary.op_name : "") &&
                   zir_render_expr(buffer, expr->as.unary.operand);
        case ZIR_EXPR_BINARY:
            return zir_string_buffer_append_format(buffer, "binary.%s ", expr->as.binary.op_name != NULL ? expr->as.binary.op_name : "") &&
                   zir_render_expr(buffer, expr->as.binary.left) &&
                   zir_string_buffer_append(buffer, ", ") &&
                   zir_render_expr(buffer, expr->as.binary.right);
        case ZIR_EXPR_CALL_DIRECT:
        case ZIR_EXPR_CALL_EXTERN:
        case ZIR_EXPR_CALL_RUNTIME_INTRINSIC:
            return zir_string_buffer_append(buffer,
                       expr->kind == ZIR_EXPR_CALL_DIRECT ? "call_direct " :
                       (expr->kind == ZIR_EXPR_CALL_EXTERN ? "call_extern " : "call_runtime_intrinsic ")) &&
                   zir_string_buffer_append(buffer, expr->as.call.callee_name != NULL ? expr->as.call.callee_name : "") &&
                   zir_string_buffer_append_char(buffer, '(') &&
                   zir_render_expr_list(buffer, &expr->as.call.args) &&
                   zir_string_buffer_append_char(buffer, ')');
        case ZIR_EXPR_MAKE_STRUCT:
            if (!zir_string_buffer_append_format(buffer, "make_struct %s { ", expr->as.make_struct.type_name != NULL ? expr->as.make_struct.type_name : "")) return 0;
            for (i = 0; i < expr->as.make_struct.fields.count; i++) {
                if (i > 0 && !zir_string_buffer_append(buffer, ", ")) return 0;
                if (!zir_string_buffer_append_format(buffer, "%s: ", expr->as.make_struct.fields.items[i].name != NULL ? expr->as.make_struct.fields.items[i].name : "")) return 0;
                if (!zir_render_expr(buffer, expr->as.make_struct.fields.items[i].value)) return 0;
            }
            return zir_string_buffer_append(buffer, " }");
        case ZIR_EXPR_MAKE_LIST:
            if (!zir_string_buffer_append_format(buffer, "make_list<%s> [", expr->as.make_list.item_type_name != NULL ? expr->as.make_list.item_type_name : "")) return 0;
            if (!zir_render_expr_list(buffer, &expr->as.make_list.items)) return 0;
            return zir_string_buffer_append_char(buffer, ']');
        case ZIR_EXPR_MAKE_MAP:
            if (!zir_string_buffer_append_format(buffer, "make_map<%s,%s> [", expr->as.make_map.key_type_name != NULL ? expr->as.make_map.key_type_name : "", expr->as.make_map.value_type_name != NULL ? expr->as.make_map.value_type_name : "")) return 0;
            for (i = 0; i < expr->as.make_map.entries.count; i++) {
                if (i > 0 && !zir_string_buffer_append(buffer, ", ")) return 0;
                if (!zir_render_expr(buffer, expr->as.make_map.entries.items[i].key)) return 0;
                if (!zir_string_buffer_append(buffer, ": ")) return 0;
                if (!zir_render_expr(buffer, expr->as.make_map.entries.items[i].value)) return 0;
            }
            return zir_string_buffer_append_char(buffer, ']');
        case ZIR_EXPR_GET_FIELD:
            return zir_string_buffer_append(buffer, "get_field ") &&
                   zir_render_expr(buffer, expr->as.field.object) &&
                   zir_string_buffer_append_format(buffer, ", %s", expr->as.field.field_name != NULL ? expr->as.field.field_name : "");
        case ZIR_EXPR_SET_FIELD:
            return zir_string_buffer_append(buffer, "set_field ") &&
                   zir_render_expr(buffer, expr->as.field.object) &&
                   zir_string_buffer_append_format(buffer, ", %s, ", expr->as.field.field_name != NULL ? expr->as.field.field_name : "") &&
                   zir_render_expr(buffer, expr->as.field.value);
        case ZIR_EXPR_INDEX_SEQ:
            return zir_string_buffer_append(buffer, "index_seq ") &&
                   zir_render_expr(buffer, expr->as.sequence.first) &&
                   zir_string_buffer_append(buffer, ", ") &&
                   zir_render_expr(buffer, expr->as.sequence.second);
        case ZIR_EXPR_SLICE_SEQ:
            return zir_string_buffer_append(buffer, "slice_seq ") &&
                   zir_render_expr(buffer, expr->as.sequence.first) &&
                   zir_string_buffer_append(buffer, ", ") &&
                   zir_render_expr(buffer, expr->as.sequence.second) &&
                   zir_string_buffer_append(buffer, ", ") &&
                   zir_render_expr(buffer, expr->as.sequence.third);
        case ZIR_EXPR_LIST_LEN:
            return zir_string_buffer_append(buffer, "list_len ") && zir_render_expr(buffer, expr->as.single.value);
        case ZIR_EXPR_MAP_LEN:
            return zir_string_buffer_append(buffer, "map_len ") && zir_render_expr(buffer, expr->as.single.value);
        case ZIR_EXPR_LIST_PUSH:
            return zir_string_buffer_append(buffer, "list_push ") &&
                   zir_render_expr(buffer, expr->as.sequence.first) &&
                   zir_string_buffer_append(buffer, ", ") &&
                   zir_render_expr(buffer, expr->as.sequence.second);
        case ZIR_EXPR_LIST_SET:
            return zir_string_buffer_append(buffer, "list_set ") &&
                   zir_render_expr(buffer, expr->as.sequence.first) &&
                   zir_string_buffer_append(buffer, ", ") &&
                   zir_render_expr(buffer, expr->as.sequence.second) &&
                   zir_string_buffer_append(buffer, ", ") &&
                   zir_render_expr(buffer, expr->as.sequence.third);
        case ZIR_EXPR_MAP_SET:
            return zir_string_buffer_append(buffer, "map_set ") &&
                   zir_render_expr(buffer, expr->as.sequence.first) &&
                   zir_string_buffer_append(buffer, ", ") &&
                   zir_render_expr(buffer, expr->as.sequence.second) &&
                   zir_string_buffer_append(buffer, ", ") &&
                   zir_render_expr(buffer, expr->as.sequence.third);
        case ZIR_EXPR_OPTIONAL_PRESENT:
            return zir_string_buffer_append(buffer, "optional_present ") && zir_render_expr(buffer, expr->as.single.value);
        case ZIR_EXPR_OPTIONAL_EMPTY:
            return zir_string_buffer_append_format(buffer, "optional_empty<%s>", expr->as.type_only.type_name != NULL ? expr->as.type_only.type_name : "");
        case ZIR_EXPR_OPTIONAL_IS_PRESENT:
            return zir_string_buffer_append(buffer, "optional_is_present ") && zir_render_expr(buffer, expr->as.single.value);
        case ZIR_EXPR_COALESCE:
            return zir_string_buffer_append(buffer, "coalesce ") &&
                   zir_render_expr(buffer, expr->as.sequence.first) &&
                   zir_string_buffer_append(buffer, ", ") &&
                   zir_render_expr(buffer, expr->as.sequence.second);
        case ZIR_EXPR_OUTCOME_SUCCESS:
            if (expr->as.single.value == NULL) return zir_string_buffer_append(buffer, "outcome_success");
            return zir_string_buffer_append(buffer, "outcome_success ") && zir_render_expr(buffer, expr->as.single.value);
        case ZIR_EXPR_OUTCOME_FAILURE:
            return zir_string_buffer_append(buffer, "outcome_failure ") && zir_render_expr(buffer, expr->as.single.value);
        case ZIR_EXPR_OUTCOME_IS_SUCCESS:
            return zir_string_buffer_append(buffer, "outcome_is_success ") && zir_render_expr(buffer, expr->as.single.value);
        case ZIR_EXPR_OUTCOME_VALUE:
            return zir_string_buffer_append(buffer, "outcome_value ") && zir_render_expr(buffer, expr->as.single.value);
        case ZIR_EXPR_TRY_PROPAGATE:
            return zir_string_buffer_append(buffer, "try_propagate ") && zir_render_expr(buffer, expr->as.single.value);
        default:
            return zir_string_buffer_append(buffer, "<unsupported>");
    }
}

char *zir_expr_render_alloc(const zir_expr *expr) {
    zir_string_buffer buffer;
    zir_string_buffer_init(&buffer);
    if (!zir_render_expr(&buffer, expr)) {
        free(buffer.data);
        return NULL;
    }
    if (buffer.data == NULL) return zir_strdup_owned("");
    return buffer.data;
}

void zir_expr_dispose(zir_expr *expr) {
    if (expr == NULL) return;
    switch (expr->kind) {
        case ZIR_EXPR_NAME:
        case ZIR_EXPR_INT:
        case ZIR_EXPR_FLOAT:
        case ZIR_EXPR_STRING:
        case ZIR_EXPR_BYTES:
            free((void *)expr->as.text.text);
            break;
        case ZIR_EXPR_COPY:
        case ZIR_EXPR_LIST_LEN:
        case ZIR_EXPR_MAP_LEN:
        case ZIR_EXPR_OPTIONAL_PRESENT:
        case ZIR_EXPR_OPTIONAL_IS_PRESENT:
        case ZIR_EXPR_OUTCOME_SUCCESS:
        case ZIR_EXPR_OUTCOME_FAILURE:
        case ZIR_EXPR_OUTCOME_IS_SUCCESS:
        case ZIR_EXPR_OUTCOME_VALUE:
        case ZIR_EXPR_TRY_PROPAGATE:
            zir_expr_dispose(expr->as.single.value);
            break;
        case ZIR_EXPR_UNARY:
            free((void *)expr->as.unary.op_name);
            zir_expr_dispose(expr->as.unary.operand);
            break;
        case ZIR_EXPR_BINARY:
            free((void *)expr->as.binary.op_name);
            zir_expr_dispose(expr->as.binary.left);
            zir_expr_dispose(expr->as.binary.right);
            break;
        case ZIR_EXPR_CALL_DIRECT:
        case ZIR_EXPR_CALL_EXTERN:
        case ZIR_EXPR_CALL_RUNTIME_INTRINSIC:
            free((void *)expr->as.call.callee_name);
            zir_expr_list_dispose(&expr->as.call.args);
            break;
        case ZIR_EXPR_MAKE_STRUCT:
            free((void *)expr->as.make_struct.type_name);
            zir_named_expr_list_dispose(&expr->as.make_struct.fields);
            break;
        case ZIR_EXPR_MAKE_LIST:
            free((void *)expr->as.make_list.item_type_name);
            zir_expr_list_dispose(&expr->as.make_list.items);
            break;
        case ZIR_EXPR_MAKE_MAP:
            free((void *)expr->as.make_map.key_type_name);
            free((void *)expr->as.make_map.value_type_name);
            zir_map_entry_list_dispose(&expr->as.make_map.entries);
            break;
        case ZIR_EXPR_GET_FIELD:
            zir_expr_dispose(expr->as.field.object);
            free((void *)expr->as.field.field_name);
            break;
        case ZIR_EXPR_SET_FIELD:
            zir_expr_dispose(expr->as.field.object);
            free((void *)expr->as.field.field_name);
            zir_expr_dispose(expr->as.field.value);
            break;
        case ZIR_EXPR_INDEX_SEQ:
        case ZIR_EXPR_COALESCE:
        case ZIR_EXPR_LIST_PUSH:
            zir_expr_dispose(expr->as.sequence.first);
            zir_expr_dispose(expr->as.sequence.second);
            break;
        case ZIR_EXPR_SLICE_SEQ:
        case ZIR_EXPR_LIST_SET:
        case ZIR_EXPR_MAP_SET:
            zir_expr_dispose(expr->as.sequence.first);
            zir_expr_dispose(expr->as.sequence.second);
            zir_expr_dispose(expr->as.sequence.third);
            break;
        case ZIR_EXPR_OPTIONAL_EMPTY:
            free((void *)expr->as.type_only.type_name);
            break;
        default:
            break;
    }
    free(expr);
}

zir_param zir_make_param(const char *name, const char *type_name, zir_expr *where_clause) {
    zir_param param;
    param.name = name;
    param.type_name = type_name;
    param.span = zir_make_span(NULL, 0, 0);
    param.where_clause = where_clause;
    return param;
}

zir_field_decl zir_make_field_decl(const char *name, const char *type_name, zir_expr *where_clause) {
    zir_field_decl field_decl;
    field_decl.name = name;
    field_decl.type_name = type_name;
    field_decl.span = zir_make_span(NULL, 0, 0);
    field_decl.where_clause = where_clause;
    return field_decl;
}

zir_instruction zir_make_assign_instruction(const char *dest_name, const char *type_name, const char *expr_text) {
    zir_instruction instruction;
    instruction.kind = ZIR_INSTR_ASSIGN;
    instruction.dest_name = dest_name;
    instruction.type_name = type_name;
    instruction.expr_text = expr_text;
    instruction.span = zir_make_span(NULL, 0, 0);
    instruction.expr = NULL;
    return instruction;
}

zir_instruction zir_make_assign_instruction_expr(const char *dest_name, const char *type_name, zir_expr *expr) {
    zir_instruction instruction = zir_make_assign_instruction(dest_name, type_name, NULL);
    instruction.expr = expr;
    instruction.expr_text = zir_expr_render_alloc(expr);
    return instruction;
}

zir_instruction zir_make_effect_instruction(const char *expr_text) {
    zir_instruction instruction;
    instruction.kind = ZIR_INSTR_EFFECT;
    instruction.dest_name = "";
    instruction.type_name = "";
    instruction.expr_text = expr_text;
    instruction.span = zir_make_span(NULL, 0, 0);
    instruction.expr = NULL;
    return instruction;
}

zir_instruction zir_make_effect_instruction_expr(zir_expr *expr) {
    zir_instruction instruction = zir_make_effect_instruction(NULL);
    instruction.expr = expr;
    instruction.expr_text = zir_expr_render_alloc(expr);
    return instruction;
}

zir_instruction zir_make_check_contract_instruction_expr(zir_expr *expr) {
    zir_instruction instruction = zir_make_effect_instruction(NULL);
    instruction.kind = ZIR_INSTR_CHECK_CONTRACT;
    instruction.expr = expr;
    instruction.expr_text = zir_expr_render_alloc(expr);
    return instruction;
}

zir_terminator zir_make_return_terminator(const char *value_text) {
    zir_terminator terminator;
    terminator.kind = ZIR_TERM_RETURN;
    terminator.value_text = value_text;
    terminator.target_label = "";
    terminator.condition_text = "";
    terminator.then_label = "";
    terminator.else_label = "";
    terminator.message_text = "";
    terminator.span = zir_make_span(NULL, 0, 0);
    terminator.value = NULL;
    terminator.condition = NULL;
    terminator.message = NULL;
    return terminator;
}

zir_terminator zir_make_return_terminator_expr(zir_expr *value) {
    zir_terminator terminator = zir_make_return_terminator(NULL);
    terminator.value = value;
    terminator.value_text = zir_expr_render_alloc(value);
    return terminator;
}

zir_terminator zir_make_jump_terminator(const char *target_label) {
    zir_terminator terminator = zir_make_return_terminator("");
    terminator.kind = ZIR_TERM_JUMP;
    terminator.target_label = target_label;
    return terminator;
}

zir_terminator zir_make_branch_if_terminator(const char *condition_text, const char *then_label, const char *else_label) {
    zir_terminator terminator = zir_make_return_terminator("");
    terminator.kind = ZIR_TERM_BRANCH_IF;
    terminator.condition_text = condition_text;
    terminator.then_label = then_label;
    terminator.else_label = else_label;
    return terminator;
}

zir_terminator zir_make_branch_if_terminator_expr(zir_expr *condition, const char *then_label, const char *else_label) {
    zir_terminator terminator = zir_make_branch_if_terminator(NULL, then_label, else_label);
    terminator.condition = condition;
    terminator.condition_text = zir_expr_render_alloc(condition);
    return terminator;
}

zir_terminator zir_make_panic_terminator(const char *message_text) {
    zir_terminator terminator = zir_make_return_terminator("");
    terminator.kind = ZIR_TERM_PANIC;
    terminator.message_text = message_text;
    return terminator;
}

zir_terminator zir_make_panic_terminator_expr(zir_expr *message) {
    zir_terminator terminator = zir_make_panic_terminator(NULL);
    terminator.message = message;
    terminator.message_text = zir_expr_render_alloc(message);
    return terminator;
}

zir_terminator zir_make_unreachable_terminator(void) {
    zir_terminator terminator = zir_make_return_terminator("");
    terminator.kind = ZIR_TERM_UNREACHABLE;
    return terminator;
}

zir_block zir_make_block(const char *label, const zir_instruction *instructions, size_t instruction_count, zir_terminator terminator) {
    zir_block block;
    block.label = label;
    block.instructions = instructions;
    block.instruction_count = instruction_count;
    block.terminator = terminator;
    block.span = zir_make_span(NULL, 0, 0);
    return block;
}

zir_struct_decl zir_make_struct_decl(const char *name, const zir_field_decl *fields, size_t field_count) {
    zir_struct_decl struct_decl;
    struct_decl.name = name;
    struct_decl.fields = fields;
    struct_decl.field_count = field_count;
    struct_decl.span = zir_make_span(NULL, 0, 0);
    return struct_decl;
}

zir_enum_variant_field_decl zir_make_enum_variant_field_decl(const char *name, const char *type_name) {
    zir_enum_variant_field_decl field_decl;
    field_decl.name = name;
    field_decl.type_name = type_name;
    field_decl.span = zir_make_span(NULL, 0, 0);
    return field_decl;
}

zir_enum_variant_decl zir_make_enum_variant_decl(const char *name, const zir_enum_variant_field_decl *fields, size_t field_count) {
    zir_enum_variant_decl variant_decl;
    variant_decl.name = name;
    variant_decl.fields = fields;
    variant_decl.field_count = field_count;
    variant_decl.span = zir_make_span(NULL, 0, 0);
    return variant_decl;
}

zir_enum_decl zir_make_enum_decl(const char *name, const zir_enum_variant_decl *variants, size_t variant_count) {
    zir_enum_decl enum_decl;
    enum_decl.name = name;
    enum_decl.variants = variants;
    enum_decl.variant_count = variant_count;
    enum_decl.span = zir_make_span(NULL, 0, 0);
    return enum_decl;
}

zir_function zir_make_function(const char *name, const zir_param *params, size_t param_count, const char *return_type, const zir_block *blocks, size_t block_count) {
    zir_function function_decl;
    function_decl.name = name;
    function_decl.params = params;
    function_decl.param_count = param_count;
    function_decl.return_type = return_type;
    function_decl.receiver_type_name = NULL;
    function_decl.implemented_trait_name = NULL;
    function_decl.is_mutating = 0;
    function_decl.blocks = blocks;
    function_decl.block_count = block_count;
    function_decl.span = zir_make_span(NULL, 0, 0);
    return function_decl;
}

zir_module zir_make_module(const char *name, const zir_function *functions, size_t function_count) {
    return zir_make_module_with_structs(name, NULL, 0, functions, function_count);
}

zir_module zir_make_module_with_structs(const char *name, const zir_struct_decl *structs, size_t struct_count, const zir_function *functions, size_t function_count) {
    zir_module module_decl;
    module_decl.name = name;
    module_decl.structs = structs;
    module_decl.struct_count = struct_count;
    module_decl.enums = NULL;
    module_decl.enum_count = 0;
    module_decl.functions = functions;
    module_decl.function_count = function_count;
    module_decl.span = zir_make_span(NULL, 0, 0);
    return module_decl;
}

zir_module zir_make_module_with_decls(
        const char *name,
        const zir_struct_decl *structs,
        size_t struct_count,
        const zir_enum_decl *enums,
        size_t enum_count,
        const zir_function *functions,
        size_t function_count) {
    zir_module module_decl = zir_make_module_with_structs(name, structs, struct_count, functions, function_count);
    module_decl.enums = enums;
    module_decl.enum_count = enum_count;
    return module_decl;
}

void zir_instruction_dispose_owned(zir_instruction *instruction) {
    if (instruction == NULL) return;
    zir_free_owned_nonempty(instruction->dest_name);
    zir_free_owned_nonempty(instruction->type_name);
    zir_free_owned_nonempty(instruction->expr_text);
    zir_expr_dispose(instruction->expr);
    memset(instruction, 0, sizeof(*instruction));
}

void zir_terminator_dispose_owned(zir_terminator *terminator) {
    if (terminator == NULL) return;
    zir_free_owned_nonempty(terminator->value_text);
    zir_free_owned_nonempty(terminator->target_label);
    zir_free_owned_nonempty(terminator->condition_text);
    zir_free_owned_nonempty(terminator->then_label);
    zir_free_owned_nonempty(terminator->else_label);
    zir_free_owned_nonempty(terminator->message_text);
    zir_expr_dispose(terminator->value);
    zir_expr_dispose(terminator->condition);
    zir_expr_dispose(terminator->message);
    memset(terminator, 0, sizeof(*terminator));
}

void zir_block_dispose_owned(zir_block *block) {
    size_t i;
    zir_instruction *instructions;
    if (block == NULL) return;
    free((void *)block->label);
    instructions = (zir_instruction *)block->instructions;
    for (i = 0; i < block->instruction_count; i++) zir_instruction_dispose_owned(&instructions[i]);
    free(instructions);
    zir_terminator_dispose_owned(&block->terminator);
    memset(block, 0, sizeof(*block));
}

void zir_struct_decl_dispose_owned(zir_struct_decl *struct_decl) {
    size_t i;
    zir_field_decl *fields;
    if (struct_decl == NULL) return;
    free((void *)struct_decl->name);
    fields = (zir_field_decl *)struct_decl->fields;
    for (i = 0; i < struct_decl->field_count; i++) {
        free((void *)fields[i].name);
        free((void *)fields[i].type_name);
        zir_expr_dispose(fields[i].where_clause);
    }
    free(fields);
    memset(struct_decl, 0, sizeof(*struct_decl));
}

void zir_enum_decl_dispose_owned(zir_enum_decl *enum_decl) {
    size_t i;
    zir_enum_variant_decl *variants;
    if (enum_decl == NULL) return;
    free((void *)enum_decl->name);
    variants = (zir_enum_variant_decl *)enum_decl->variants;
    for (i = 0; i < enum_decl->variant_count; i++) {
        size_t j;
        zir_enum_variant_field_decl *fields = (zir_enum_variant_field_decl *)variants[i].fields;
        free((void *)variants[i].name);
        for (j = 0; j < variants[i].field_count; j++) {
            free((void *)fields[j].name);
            free((void *)fields[j].type_name);
        }
        free(fields);
    }
    free(variants);
    memset(enum_decl, 0, sizeof(*enum_decl));
}

void zir_function_dispose_owned(zir_function *function_decl) {
    size_t i;
    zir_param *params;
    zir_block *blocks;
    if (function_decl == NULL) return;
    free((void *)function_decl->name);
    free((void *)function_decl->return_type);
    free((void *)function_decl->receiver_type_name);
    free((void *)function_decl->implemented_trait_name);
    params = (zir_param *)function_decl->params;
    for (i = 0; i < function_decl->param_count; i++) {
        free((void *)params[i].name);
        free((void *)params[i].type_name);
        zir_expr_dispose(params[i].where_clause);
    }
    free(params);
    blocks = (zir_block *)function_decl->blocks;
    for (i = 0; i < function_decl->block_count; i++) zir_block_dispose_owned(&blocks[i]);
    free(blocks);
    memset(function_decl, 0, sizeof(*function_decl));
}

void zir_module_dispose_owned(zir_module *module_decl) {
    size_t i;
    zir_struct_decl *structs;
    zir_enum_decl *enums;
    zir_function *functions;
    if (module_decl == NULL) return;
    free((void *)module_decl->name);
    structs = (zir_struct_decl *)module_decl->structs;
    for (i = 0; i < module_decl->struct_count; i++) zir_struct_decl_dispose_owned(&structs[i]);
    free(structs);
    enums = (zir_enum_decl *)module_decl->enums;
    for (i = 0; i < module_decl->enum_count; i++) zir_enum_decl_dispose_owned(&enums[i]);
    free(enums);
    functions = (zir_function *)module_decl->functions;
    for (i = 0; i < module_decl->function_count; i++) zir_function_dispose_owned(&functions[i]);
    free(functions);
    memset(module_decl, 0, sizeof(*module_decl));
}

