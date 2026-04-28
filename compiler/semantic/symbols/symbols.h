#ifndef ZENITH_NEXT_COMPILER_SEMANTIC_SYMBOLS_H
#define ZENITH_NEXT_COMPILER_SEMANTIC_SYMBOLS_H

#include "compiler/frontend/lexer/token.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum zt_symbol_kind {
    ZT_SYMBOL_IMPORT,
    ZT_SYMBOL_FUNC,
    ZT_SYMBOL_STRUCT,
    ZT_SYMBOL_TRAIT,
    ZT_SYMBOL_ENUM,
    ZT_SYMBOL_TYPE_ALIAS,
    ZT_SYMBOL_EXTERN_FUNC,
    ZT_SYMBOL_PARAM,
    ZT_SYMBOL_LOCAL,
    ZT_SYMBOL_TYPE_PARAM,
    ZT_SYMBOL_IMPLICIT_SELF,
    ZT_SYMBOL_IMPLICIT_IT,
    ZT_SYMBOL_CORE_TRAIT
} zt_symbol_kind;

typedef struct zt_symbol {
    zt_symbol_kind kind;
    char *name;
    zt_source_span span;
    int is_implicit;
} zt_symbol;

typedef struct zt_scope {
    struct zt_scope *parent;
    zt_symbol *symbols;
    size_t count;
    size_t capacity;
} zt_scope;

const char *zt_symbol_kind_name(zt_symbol_kind kind);

void zt_scope_init(zt_scope *scope, zt_scope *parent);
void zt_scope_dispose(zt_scope *scope);

zt_symbol *zt_scope_lookup(zt_scope *scope, const char *name);
zt_symbol *zt_scope_lookup_current(zt_scope *scope, const char *name);
zt_symbol *zt_scope_lookup_parent_chain(zt_scope *scope, const char *name);

int zt_scope_declare(zt_scope *scope, zt_symbol_kind kind, const char *name, zt_source_span span, int is_implicit, zt_symbol **out_symbol);

#ifdef __cplusplus
}
#endif

#endif
