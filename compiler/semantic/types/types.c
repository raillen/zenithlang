#include "compiler/semantic/types/types.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char *zt_types_strdup(const char *text) {
    size_t len;
    char *copy;

    if (text == NULL) return NULL;
    len = strlen(text);
    copy = (char *)malloc(len + 1);
    if (copy == NULL) return NULL;
    memcpy(copy, text, len + 1);
    return copy;
}

const char *zt_type_kind_name(zt_type_kind kind) {
    switch (kind) {
        case ZT_TYPE_UNKNOWN: return "unknown";
        case ZT_TYPE_NONE_LITERAL: return "none_literal";
        case ZT_TYPE_SUCCESS_WRAPPER: return "success_wrapper";
        case ZT_TYPE_ERROR_WRAPPER: return "error_wrapper";
        case ZT_TYPE_BOOL: return "bool";
        case ZT_TYPE_INT: return "int";
        case ZT_TYPE_INT8: return "int8";
        case ZT_TYPE_INT16: return "int16";
        case ZT_TYPE_INT32: return "int32";
        case ZT_TYPE_INT64: return "int64";
        case ZT_TYPE_UINT8: return "uint8";
        case ZT_TYPE_UINT16: return "uint16";
        case ZT_TYPE_UINT32: return "uint32";
        case ZT_TYPE_UINT64: return "uint64";
        case ZT_TYPE_FLOAT: return "float";
        case ZT_TYPE_FLOAT32: return "float32";
        case ZT_TYPE_FLOAT64: return "float64";
        case ZT_TYPE_TEXT: return "text";
        case ZT_TYPE_BYTES: return "bytes";
        case ZT_TYPE_VOID: return "void";
        case ZT_TYPE_USER: return "user";
        case ZT_TYPE_TYPE_PARAM: return "type_param";
        case ZT_TYPE_OPTIONAL: return "optional";
        case ZT_TYPE_RESULT: return "result";
        case ZT_TYPE_LIST: return "list";
        case ZT_TYPE_MAP: return "map";
        default: return "unknown";
    }
}

zt_type_list zt_type_list_make(void) {
    zt_type_list list;
    list.items = NULL;
    list.count = 0;
    list.capacity = 0;
    return list;
}

void zt_type_list_push(zt_type_list *list, zt_type *type) {
    zt_type **new_items;
    size_t new_capacity;

    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        new_items = (zt_type **)realloc(list->items, new_capacity * sizeof(zt_type *));
        if (new_items == NULL) return;
        list->items = new_items;
        list->capacity = new_capacity;
    }
    list->items[list->count++] = type;
}

void zt_type_list_dispose(zt_type_list *list) {
    size_t i;

    if (list == NULL) return;
    for (i = 0; i < list->count; i++) {
        zt_type_dispose(list->items[i]);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

zt_type *zt_type_make(zt_type_kind kind) {
    zt_type *type = (zt_type *)calloc(1, sizeof(zt_type));
    if (type == NULL) return NULL;
    type->kind = kind;
    type->args = zt_type_list_make();
    return type;
}

zt_type *zt_type_make_named(zt_type_kind kind, const char *name) {
    zt_type *type = zt_type_make(kind);
    if (type == NULL) return NULL;
    type->name = zt_types_strdup(name);
    return type;
}

zt_type *zt_type_make_with_args(zt_type_kind kind, const char *name, zt_type_list args) {
    zt_type *type = zt_type_make_named(kind, name);
    if (type == NULL) {
        zt_type_list_dispose(&args);
        return NULL;
    }
    type->args = args;
    return type;
}

zt_type *zt_type_clone(const zt_type *type) {
    zt_type *copy;
    size_t i;

    if (type == NULL) return NULL;
    copy = zt_type_make_named(type->kind, type->name);
    if (copy == NULL) return NULL;
    for (i = 0; i < type->args.count; i++) {
        zt_type_list_push(&copy->args, zt_type_clone(type->args.items[i]));
    }
    return copy;
}

void zt_type_dispose(zt_type *type) {
    if (type == NULL) return;
    free(type->name);
    zt_type_list_dispose(&type->args);
    free(type);
}

int zt_type_equals(const zt_type *left, const zt_type *right) {
    size_t i;

    if (left == right) return 1;
    if (left == NULL || right == NULL) return 0;
    if (left->kind != right->kind) return 0;
    if ((left->name == NULL) != (right->name == NULL)) return 0;
    if (left->name != NULL && strcmp(left->name, right->name) != 0) return 0;
    if (left->args.count != right->args.count) return 0;
    for (i = 0; i < left->args.count; i++) {
        if (!zt_type_equals(left->args.items[i], right->args.items[i])) return 0;
    }
    return 1;
}

static void zt_type_append(char *buffer, size_t buffer_size, size_t *cursor, const char *text) {
    size_t remaining;
    size_t len;

    if (buffer == NULL || buffer_size == 0 || cursor == NULL || text == NULL) return;
    if (*cursor >= buffer_size - 1) return;
    remaining = buffer_size - 1 - *cursor;
    len = strlen(text);
    if (len > remaining) len = remaining;
    memcpy(buffer + *cursor, text, len);
    *cursor += len;
    buffer[*cursor] = '\0';
}

static void zt_type_format_inner(const zt_type *type, char *buffer, size_t buffer_size, size_t *cursor) {
    size_t i;

    if (type == NULL) {
        zt_type_append(buffer, buffer_size, cursor, "<null>");
        return;
    }

    switch (type->kind) {
        case ZT_TYPE_UNKNOWN:
        case ZT_TYPE_NONE_LITERAL:
        case ZT_TYPE_SUCCESS_WRAPPER:
        case ZT_TYPE_ERROR_WRAPPER:
            zt_type_append(buffer, buffer_size, cursor, zt_type_kind_name(type->kind));
            break;
        case ZT_TYPE_USER:
        case ZT_TYPE_TYPE_PARAM:
            zt_type_append(buffer, buffer_size, cursor, type->name != NULL ? type->name : "<anon>");
            break;
        case ZT_TYPE_OPTIONAL:
        case ZT_TYPE_RESULT:
        case ZT_TYPE_LIST:
        case ZT_TYPE_MAP:
            zt_type_append(buffer, buffer_size, cursor, zt_type_kind_name(type->kind));
            zt_type_append(buffer, buffer_size, cursor, "<");
            for (i = 0; i < type->args.count; i++) {
                if (i != 0) zt_type_append(buffer, buffer_size, cursor, ", ");
                zt_type_format_inner(type->args.items[i], buffer, buffer_size, cursor);
            }
            zt_type_append(buffer, buffer_size, cursor, ">");
            break;
        default:
            zt_type_append(buffer, buffer_size, cursor, zt_type_kind_name(type->kind));
            break;
    }
}

void zt_type_format(const zt_type *type, char *buffer, size_t buffer_size) {
    size_t cursor = 0;

    if (buffer == NULL || buffer_size == 0) return;
    buffer[0] = '\0';
    zt_type_format_inner(type, buffer, buffer_size, &cursor);
}

int zt_type_is_integral(const zt_type *type) {
    if (type == NULL) return 0;
    switch (type->kind) {
        case ZT_TYPE_INT:
        case ZT_TYPE_INT8:
        case ZT_TYPE_INT16:
        case ZT_TYPE_INT32:
        case ZT_TYPE_INT64:
        case ZT_TYPE_UINT8:
        case ZT_TYPE_UINT16:
        case ZT_TYPE_UINT32:
        case ZT_TYPE_UINT64:
            return 1;
        default:
            return 0;
    }
}

int zt_type_is_signed_integral(const zt_type *type) {
    if (type == NULL) return 0;
    switch (type->kind) {
        case ZT_TYPE_INT:
        case ZT_TYPE_INT8:
        case ZT_TYPE_INT16:
        case ZT_TYPE_INT32:
        case ZT_TYPE_INT64:
            return 1;
        default:
            return 0;
    }
}

int zt_type_is_unsigned_integral(const zt_type *type) {
    if (type == NULL) return 0;
    switch (type->kind) {
        case ZT_TYPE_UINT8:
        case ZT_TYPE_UINT16:
        case ZT_TYPE_UINT32:
        case ZT_TYPE_UINT64:
            return 1;
        default:
            return 0;
    }
}

int zt_type_is_float(const zt_type *type) {
    if (type == NULL) return 0;
    return type->kind == ZT_TYPE_FLOAT ||
           type->kind == ZT_TYPE_FLOAT32 ||
           type->kind == ZT_TYPE_FLOAT64;
}

int zt_type_is_numeric(const zt_type *type) {
    return zt_type_is_integral(type) || zt_type_is_float(type);
}

