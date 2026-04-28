#include "compiler/semantic/symbols/symbols.h"

#include <stdlib.h>
#include <string.h>

static char *zt_symbols_strdup(const char *text) {
    size_t len;
    char *copy;

    if (text == NULL) return NULL;
    len = strlen(text);
    copy = (char *)malloc(len + 1);
    if (copy == NULL) return NULL;
    memcpy(copy, text, len + 1);
    return copy;
}

const char *zt_symbol_kind_name(zt_symbol_kind kind) {
    switch (kind) {
        case ZT_SYMBOL_IMPORT: return "import";
        case ZT_SYMBOL_FUNC: return "func";
        case ZT_SYMBOL_STRUCT: return "struct";
        case ZT_SYMBOL_TRAIT: return "trait";
        case ZT_SYMBOL_ENUM: return "enum";
        case ZT_SYMBOL_TYPE_ALIAS: return "type_alias";
        case ZT_SYMBOL_EXTERN_FUNC: return "extern_func";
        case ZT_SYMBOL_PARAM: return "param";
        case ZT_SYMBOL_LOCAL: return "local";
        case ZT_SYMBOL_TYPE_PARAM: return "type_param";
        case ZT_SYMBOL_IMPLICIT_SELF: return "implicit_self";
        case ZT_SYMBOL_IMPLICIT_IT: return "implicit_it";
        case ZT_SYMBOL_CORE_TRAIT: return "core_trait";
        default: return "unknown";
    }
}

void zt_scope_init(zt_scope *scope, zt_scope *parent) {
    if (scope == NULL) return;
    scope->parent = parent;
    scope->symbols = NULL;
    scope->count = 0;
    scope->capacity = 0;
}

void zt_scope_dispose(zt_scope *scope) {
    size_t i;

    if (scope == NULL) return;
    for (i = 0; i < scope->count; i++) {
        free(scope->symbols[i].name);
    }
    free(scope->symbols);
    scope->symbols = NULL;
    scope->count = 0;
    scope->capacity = 0;
    scope->parent = NULL;
}

zt_symbol *zt_scope_lookup_current(zt_scope *scope, const char *name) {
    size_t i;

    if (scope == NULL || name == NULL) return NULL;
    for (i = 0; i < scope->count; i++) {
        if (scope->symbols[i].name != NULL && strcmp(scope->symbols[i].name, name) == 0) {
            return &scope->symbols[i];
        }
    }
    return NULL;
}

zt_symbol *zt_scope_lookup_parent_chain(zt_scope *scope, const char *name) {
    zt_scope *cursor;

    if (scope == NULL) return NULL;
    cursor = scope->parent;
    while (cursor != NULL) {
        zt_symbol *symbol = zt_scope_lookup_current(cursor, name);
        if (symbol != NULL) return symbol;
        cursor = cursor->parent;
    }
    return NULL;
}

zt_symbol *zt_scope_lookup(zt_scope *scope, const char *name) {
    zt_scope *cursor = scope;
    while (cursor != NULL) {
        zt_symbol *symbol = zt_scope_lookup_current(cursor, name);
        if (symbol != NULL) return symbol;
        cursor = cursor->parent;
    }
    return NULL;
}

int zt_scope_declare(zt_scope *scope, zt_symbol_kind kind, const char *name, zt_source_span span, int is_implicit, zt_symbol **out_symbol) {
    zt_symbol *entry;

    if (out_symbol != NULL) *out_symbol = NULL;
    if (scope == NULL || name == NULL) return 0;

    if (scope->count >= scope->capacity) {
        size_t new_capacity = scope->capacity == 0 ? 8 : scope->capacity * 2;
        zt_symbol *new_symbols = (zt_symbol *)realloc(scope->symbols, new_capacity * sizeof(zt_symbol));
        if (new_symbols == NULL) return 0;
        scope->symbols = new_symbols;
        scope->capacity = new_capacity;
    }

    entry = &scope->symbols[scope->count++];
    entry->kind = kind;
    entry->name = zt_symbols_strdup(name);
    entry->span = span;
    entry->is_implicit = is_implicit;

    if (out_symbol != NULL) *out_symbol = entry;
    return 1;
}
