#include "compiler/hir/lowering/from_ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct zt_scope_binding {
    char *name;
    zt_type *type;
    int is_mutable;
} zt_scope_binding;

typedef struct zt_scope {
    struct zt_scope *parent;
    zt_scope_binding *items;
    size_t count;
    size_t capacity;
} zt_scope;

typedef struct zt_struct_field_meta {
    char *name;
    zt_type *type;
    const zt_ast_node *default_value;
} zt_struct_field_meta;

typedef struct zt_struct_meta {
    char *name;
    zt_struct_field_meta *fields;
    size_t field_count;
    size_t field_capacity;
} zt_struct_meta;

typedef struct zt_func_param_meta {
    char *name;
    zt_type *type;
    const zt_ast_node *default_value;
} zt_func_param_meta;

typedef struct zt_func_meta {
    char *name;
    zt_type *return_type;
    zt_func_param_meta *params;
    size_t param_count;
    size_t param_capacity;
    int is_extern;
    char *extern_binding;
    char *extern_abi_name;
} zt_func_meta;

typedef struct zt_method_meta {
    char *receiver_type;
    char *method_name;
    char *lowered_name;
    char *trait_name;
    int is_mutating;
} zt_method_meta;

typedef struct zt_const_meta {
    char *name;
    zt_type *type;
    const zt_ast_node *init_value;
    int is_lowering;
} zt_const_meta;

typedef struct zt_var_meta {
    char *name;
    char *lowered_name;
    zt_type *type;
    const zt_ast_node *init_value;
    int is_lowering;
} zt_var_meta;

typedef struct zt_lower_ctx {
    zt_hir_lower_result result;
    zt_struct_meta *structs;
    size_t struct_count;
    size_t struct_capacity;
    zt_func_meta *funcs;
    size_t func_count;
    size_t func_capacity;
    zt_method_meta *methods;
    size_t method_count;
    size_t method_capacity;
    zt_const_meta *consts;
    size_t const_count;
    size_t const_capacity;
    zt_var_meta *vars;
    size_t var_count;
    size_t var_capacity;
    const zt_type *current_return_type;
    const zt_ast_node *root_ast;
} zt_lower_ctx;

static char *zt_lower_strdup(const char *text) {
    size_t len;
    char *copy;
    if (text == NULL) return NULL;
    len = strlen(text);
    copy = (char *)malloc(len + 1);
    if (copy == NULL) return NULL;
    memcpy(copy, text, len + 1);
    return copy;
}

static char *zt_lower_join_with_dot(const char *left, const char *right) {
    size_t left_len;
    size_t right_len;
    char *out;
    if (left == NULL || right == NULL) return NULL;
    left_len = strlen(left);
    right_len = strlen(right);
    out = (char *)malloc(left_len + 1 + right_len + 1);
    if (out == NULL) return NULL;
    memcpy(out, left, left_len);
    out[left_len] = '.';
    memcpy(out + left_len + 1, right, right_len + 1);
    return out;
}

static void *zt_lower_grow_array(void *items, size_t *capacity, size_t item_size) {
    size_t new_capacity;
    void *grown;
    if (capacity == NULL) return NULL;
    new_capacity = *capacity == 0 ? 4u : (*capacity * 2u);
    grown = realloc(items, new_capacity * item_size);
    if (grown != NULL) *capacity = new_capacity;
    return grown;
}

static int zt_text_eq(const char *a, const char *b) {
    if (a == b) return 1;
    if (a == NULL || b == NULL) return 0;
    return strcmp(a, b) == 0;
}

static const char *zt_last_segment(const char *name) {
    const char *dot;
    if (name == NULL) return NULL;
    dot = strrchr(name, '.');
    return dot != NULL ? dot + 1 : name;
}

static int zt_name_eq(const char *a, const char *b) {
    if (zt_text_eq(a, b)) return 1;
    return zt_text_eq(zt_last_segment(a), zt_last_segment(b));
}

static zt_type *zt_unknown_type(void) {
    return zt_type_make(ZT_TYPE_UNKNOWN);
}

static void zt_add_diag(zt_lower_ctx *ctx, zt_source_span span, const char *message) {
    if (ctx == NULL) return;
    zt_diag_list_add(
        &ctx->result.diagnostics,
        ZT_DIAG_INVALID_OPERATOR,
        span,
        "%s",
        message != NULL ? message : "lowering error");
}

static void zt_scope_init(zt_scope *scope, zt_scope *parent) {
    if (scope == NULL) return;
    memset(scope, 0, sizeof(*scope));
    scope->parent = parent;
}

static void zt_scope_dispose(zt_scope *scope) {
    size_t i;
    if (scope == NULL) return;
    for (i = 0; i < scope->count; i += 1) {
        free(scope->items[i].name);
        zt_type_dispose(scope->items[i].type);
    }
    free(scope->items);
    memset(scope, 0, sizeof(*scope));
}

static void zt_scope_set(zt_scope *scope, const char *name, const zt_type *type, int is_mutable) {
    size_t i;
    zt_scope_binding *grown;
    if (scope == NULL || name == NULL || type == NULL) return;
    for (i = 0; i < scope->count; i += 1) {
        if (zt_text_eq(scope->items[i].name, name)) {
            zt_type_dispose(scope->items[i].type);
            scope->items[i].type = zt_type_clone(type);
            scope->items[i].is_mutable = is_mutable;
            return;
        }
    }
    if (scope->count >= scope->capacity) {
        grown = (zt_scope_binding *)zt_lower_grow_array(scope->items, &scope->capacity, sizeof(zt_scope_binding));
        if (grown == NULL) return;
        scope->items = grown;
    }
    scope->items[scope->count].name = zt_lower_strdup(name);
    scope->items[scope->count].type = zt_type_clone(type);
    scope->items[scope->count].is_mutable = is_mutable;
    scope->count += 1;
}

static const zt_type *zt_scope_get(const zt_scope *scope, const char *name) {
    const zt_scope *cursor = scope;
    while (cursor != NULL) {
        size_t i;
        for (i = 0; i < cursor->count; i += 1) {
            if (zt_text_eq(cursor->items[i].name, name)) return cursor->items[i].type;
        }
        cursor = cursor->parent;
    }
    return NULL;
}

static zt_type_kind zt_builtin_kind(const char *name, int *is_builtin) {
    if (is_builtin != NULL) *is_builtin = 1;
    if (zt_text_eq(name, "bool")) return ZT_TYPE_BOOL;
    if (zt_text_eq(name, "int")) return ZT_TYPE_INT;
    if (zt_text_eq(name, "int8")) return ZT_TYPE_INT8;
    if (zt_text_eq(name, "int16")) return ZT_TYPE_INT16;
    if (zt_text_eq(name, "int32")) return ZT_TYPE_INT32;
    if (zt_text_eq(name, "int64")) return ZT_TYPE_INT64;
    if (zt_text_eq(name, "u8")) return ZT_TYPE_UINT8;
    if (zt_text_eq(name, "u16")) return ZT_TYPE_UINT16;
    if (zt_text_eq(name, "u32")) return ZT_TYPE_UINT32;
    if (zt_text_eq(name, "u64")) return ZT_TYPE_UINT64;
    if (zt_text_eq(name, "uint8")) return ZT_TYPE_UINT8;
    if (zt_text_eq(name, "uint16")) return ZT_TYPE_UINT16;
    if (zt_text_eq(name, "uint32")) return ZT_TYPE_UINT32;
    if (zt_text_eq(name, "uint64")) return ZT_TYPE_UINT64;
    if (zt_text_eq(name, "float")) return ZT_TYPE_FLOAT;
    if (zt_text_eq(name, "float32")) return ZT_TYPE_FLOAT32;
    if (zt_text_eq(name, "float64")) return ZT_TYPE_FLOAT64;
    if (zt_text_eq(name, "text")) return ZT_TYPE_TEXT;
    if (zt_text_eq(name, "core.Error")) return ZT_TYPE_CORE_ERROR;
    if (zt_text_eq(name, "bytes")) return ZT_TYPE_BYTES;
    if (zt_text_eq(name, "void")) return ZT_TYPE_VOID;
    if (is_builtin != NULL) *is_builtin = 0;
    return ZT_TYPE_USER;
}

static zt_type *zt_lower_type_from_ast(zt_lower_ctx *ctx, const zt_ast_node *node);
static char *zt_build_module_var_symbol(const char *name);

static zt_type *zt_lower_type_from_generic(zt_lower_ctx *ctx, const char *name, const zt_ast_node_list *args) {
    zt_type_list lowered_args;
    size_t i;
    zt_type_kind kind;
    const char *display_name;

    lowered_args = zt_type_list_make();
    if (args != NULL) {
        for (i = 0; i < args->count; i += 1) {
            zt_type_list_push(&lowered_args, zt_lower_type_from_ast(ctx, args->items[i]));
        }
    }

    if (zt_text_eq(name, "optional") || zt_text_eq(name, "Optional")) {
        kind = ZT_TYPE_OPTIONAL;
        display_name = "optional";
    } else if (zt_text_eq(name, "result") || zt_text_eq(name, "Outcome")) {
        kind = ZT_TYPE_RESULT;
        display_name = "result";
    } else if (zt_text_eq(name, "list")) {
        kind = ZT_TYPE_LIST;
        display_name = "list";
    } else if (zt_text_eq(name, "map")) {
        kind = ZT_TYPE_MAP;
        display_name = "map";
    } else if (zt_text_eq(name, "grid2d")) {
        kind = ZT_TYPE_GRID2D;
        display_name = "grid2d";
    } else if (zt_text_eq(name, "pqueue")) {
        kind = ZT_TYPE_PQUEUE;
        display_name = "pqueue";
    } else if (zt_text_eq(name, "circbuf")) {
        kind = ZT_TYPE_CIRCBUF;
        display_name = "circbuf";
    } else if (zt_text_eq(name, "btreemap")) {
        kind = ZT_TYPE_BTREEMAP;
        display_name = "btreemap";
    } else if (zt_text_eq(name, "btreeset")) {
        kind = ZT_TYPE_BTREESET;
        display_name = "btreeset";
    } else if (zt_text_eq(name, "grid3d")) {
        kind = ZT_TYPE_GRID3D;
        display_name = "grid3d";
    } else if (zt_text_eq(name, "dyn")) {
        kind = ZT_TYPE_DYN;
        display_name = "dyn";
    } else {
        kind = ZT_TYPE_USER;
        display_name = name;
    }

    return zt_type_make_with_args(kind, display_name, lowered_args);
}

static zt_type *zt_lower_type_from_ast(zt_lower_ctx *ctx, const zt_ast_node *node) {
    (void)ctx;
    if (node == NULL) return zt_type_make(ZT_TYPE_VOID);
    if (node->kind == ZT_AST_TYPE_SIMPLE) {
        int is_builtin = 0;
        zt_type_kind kind = zt_builtin_kind(node->as.type_simple.name, &is_builtin);
        if (is_builtin) return zt_type_make(kind);
        return zt_type_make_named(ZT_TYPE_USER, node->as.type_simple.name);
    }
    if (node->kind == ZT_AST_TYPE_DYN) {
        zt_type_list args = zt_type_list_make();
        zt_type *inner_type = zt_lower_type_from_ast(ctx, node->as.type_dyn.inner_type);
        zt_type_list_push(&args, inner_type != NULL ? inner_type : zt_unknown_type());
        return zt_type_make_with_args(ZT_TYPE_DYN, "dyn", args);
    }
    if (node->kind == ZT_AST_TYPE_GENERIC) {
        return zt_lower_type_from_generic(ctx, node->as.type_generic.name, &node->as.type_generic.type_args);
    }
    if (node->kind == ZT_AST_TYPE_CALLABLE) {
        zt_type_list args = zt_type_list_make();
        zt_type *return_type = zt_lower_type_from_ast(ctx, node->as.type_callable.return_type);
        zt_type_list_push(&args, return_type != NULL ? return_type : zt_type_make(ZT_TYPE_VOID));
        {
            size_t i;
            for (i = 0; i < node->as.type_callable.params.count; i++) {
                zt_type *pt = zt_lower_type_from_ast(ctx, node->as.type_callable.params.items[i]);
                zt_type_list_push(&args, pt != NULL ? pt : zt_type_make(ZT_TYPE_VOID));
            }
        }
        return zt_type_make_with_args(ZT_TYPE_CALLABLE, "func", args);
    }
    return zt_unknown_type();
}

static zt_struct_meta *zt_push_struct_meta(zt_lower_ctx *ctx) {
    zt_struct_meta *grown;
    if (ctx->struct_count >= ctx->struct_capacity) {
        grown = (zt_struct_meta *)zt_lower_grow_array(ctx->structs, &ctx->struct_capacity, sizeof(zt_struct_meta));
        if (grown == NULL) return NULL;
        ctx->structs = grown;
    }
    memset(&ctx->structs[ctx->struct_count], 0, sizeof(zt_struct_meta));
    return &ctx->structs[ctx->struct_count++];
}

static zt_func_meta *zt_push_func_meta(zt_lower_ctx *ctx) {
    zt_func_meta *grown;
    if (ctx->func_count >= ctx->func_capacity) {
        grown = (zt_func_meta *)zt_lower_grow_array(ctx->funcs, &ctx->func_capacity, sizeof(zt_func_meta));
        if (grown == NULL) return NULL;
        ctx->funcs = grown;
    }
    memset(&ctx->funcs[ctx->func_count], 0, sizeof(zt_func_meta));
    return &ctx->funcs[ctx->func_count++];
}

static zt_method_meta *zt_push_method_meta(zt_lower_ctx *ctx) {
    zt_method_meta *grown;
    if (ctx->method_count >= ctx->method_capacity) {
        grown = (zt_method_meta *)zt_lower_grow_array(ctx->methods, &ctx->method_capacity, sizeof(zt_method_meta));
        if (grown == NULL) return NULL;
        ctx->methods = grown;
    }
    memset(&ctx->methods[ctx->method_count], 0, sizeof(zt_method_meta));
    return &ctx->methods[ctx->method_count++];
}

static zt_const_meta *zt_push_const_meta(zt_lower_ctx *ctx) {
    zt_const_meta *grown;
    if (ctx->const_count >= ctx->const_capacity) {
        grown = (zt_const_meta *)zt_lower_grow_array(ctx->consts, &ctx->const_capacity, sizeof(zt_const_meta));
        if (grown == NULL) return NULL;
        ctx->consts = grown;
    }
    memset(&ctx->consts[ctx->const_count], 0, sizeof(zt_const_meta));
    return &ctx->consts[ctx->const_count++];
}

static zt_var_meta *zt_push_var_meta(zt_lower_ctx *ctx) {
    zt_var_meta *grown;
    if (ctx->var_count >= ctx->var_capacity) {
        grown = (zt_var_meta *)zt_lower_grow_array(ctx->vars, &ctx->var_capacity, sizeof(zt_var_meta));
        if (grown == NULL) return NULL;
        ctx->vars = grown;
    }
    memset(&ctx->vars[ctx->var_count], 0, sizeof(zt_var_meta));
    return &ctx->vars[ctx->var_count++];
}

static zt_struct_field_meta *zt_push_struct_field_meta(zt_struct_meta *meta) {
    zt_struct_field_meta *grown;
    if (meta->field_count >= meta->field_capacity) {
        grown = (zt_struct_field_meta *)zt_lower_grow_array(meta->fields, &meta->field_capacity, sizeof(zt_struct_field_meta));
        if (grown == NULL) return NULL;
        meta->fields = grown;
    }
    memset(&meta->fields[meta->field_count], 0, sizeof(zt_struct_field_meta));
    return &meta->fields[meta->field_count++];
}

static zt_func_param_meta *zt_push_func_param_meta(zt_func_meta *meta) {
    zt_func_param_meta *grown;
    if (meta->param_count >= meta->param_capacity) {
        grown = (zt_func_param_meta *)zt_lower_grow_array(meta->params, &meta->param_capacity, sizeof(zt_func_param_meta));
        if (grown == NULL) return NULL;
        meta->params = grown;
    }
    memset(&meta->params[meta->param_count], 0, sizeof(zt_func_param_meta));
    return &meta->params[meta->param_count++];
}

static const zt_struct_meta *zt_find_struct_meta(const zt_lower_ctx *ctx, const char *name) {
    size_t i;
    const zt_struct_meta *fallback = NULL;
    if (name == NULL) return NULL;
    for (i = 0; i < ctx->struct_count; i += 1) {
        if (zt_text_eq(ctx->structs[i].name, name)) return &ctx->structs[i];
    }
    for (i = 0; i < ctx->struct_count; i += 1) {
        if (zt_name_eq(ctx->structs[i].name, name)) {
            if (fallback != NULL) return NULL;
            fallback = &ctx->structs[i];
        }
    }
    return fallback;
}

static const zt_ast_node *zt_find_enum_decl_ast(const zt_lower_ctx *ctx, const char *name) {
    size_t i;
    const zt_ast_node *fallback = NULL;
    if (ctx == NULL || ctx->root_ast == NULL || ctx->root_ast->kind != ZT_AST_FILE || name == NULL) return NULL;
    for (i = 0; i < ctx->root_ast->as.file.declarations.count; i += 1) {
        const zt_ast_node *decl = ctx->root_ast->as.file.declarations.items[i];
        if (decl == NULL || decl->kind != ZT_AST_ENUM_DECL) continue;
        if (zt_text_eq(decl->as.enum_decl.name, name)) return decl;
    }
    for (i = 0; i < ctx->root_ast->as.file.declarations.count; i += 1) {
        const zt_ast_node *decl = ctx->root_ast->as.file.declarations.items[i];
        if (decl == NULL || decl->kind != ZT_AST_ENUM_DECL) continue;
        if (zt_name_eq(decl->as.enum_decl.name, name)) {
            if (fallback != NULL) return NULL;
            fallback = decl;
        }
    }
    return fallback;
}

static const zt_ast_node *zt_find_enum_variant_ast(const zt_ast_node *enum_decl, const char *variant_name, size_t *out_index) {
    size_t i;
    if (out_index != NULL) *out_index = 0;
    if (enum_decl == NULL || enum_decl->kind != ZT_AST_ENUM_DECL || variant_name == NULL) return NULL;
    for (i = 0; i < enum_decl->as.enum_decl.variants.count; i += 1) {
        const zt_ast_node *variant = enum_decl->as.enum_decl.variants.items[i];
        if (variant == NULL || variant->kind != ZT_AST_ENUM_VARIANT || variant->as.enum_variant.name == NULL) continue;
        if (strcmp(variant->as.enum_variant.name, variant_name) == 0) {
            if (out_index != NULL) *out_index = i;
            return variant;
        }
    }
    return NULL;
}

static const zt_ast_node *zt_enum_variant_field_type_node(const zt_ast_node *field_node) {
    if (field_node == NULL) return NULL;
    if (field_node->kind == ZT_AST_PARAM) return field_node->as.param.type_node;
    if (field_node->kind == ZT_AST_STRUCT_FIELD) return field_node->as.struct_field.type_node;
    return field_node;
}

static const char *zt_enum_variant_field_name(const zt_ast_node *field_node, size_t index, char *fallback, size_t fallback_size) {
    if (field_node != NULL) {
        if (field_node->kind == ZT_AST_PARAM && field_node->as.param.name != NULL) return field_node->as.param.name;
        if (field_node->kind == ZT_AST_STRUCT_FIELD && field_node->as.struct_field.name != NULL) return field_node->as.struct_field.name;
    }
    if (fallback != NULL && fallback_size > 0) {
        snprintf(fallback, fallback_size, "value%zu", index + 1);
        return fallback;
    }
    return NULL;
}

static const zt_struct_field_meta *zt_find_struct_field_meta(const zt_struct_meta *meta, const char *field) {
    size_t i;
    if (meta == NULL || field == NULL) return NULL;
    for (i = 0; i < meta->field_count; i += 1) {
        if (zt_text_eq(meta->fields[i].name, field)) return &meta->fields[i];
    }
    return NULL;
}

static const zt_func_meta *zt_find_func_meta(const zt_lower_ctx *ctx, const char *name) {
    size_t i;
    if (ctx == NULL || name == NULL) return NULL;

    for (i = 0; i < ctx->func_count; i += 1) {
        if (ctx->funcs[i].name != NULL && strcmp(ctx->funcs[i].name, name) == 0) return &ctx->funcs[i];
    }

    for (i = 0; i < ctx->func_count; i += 1) {
        if (zt_name_eq(ctx->funcs[i].name, name)) return &ctx->funcs[i];
    }
    return NULL;
}

static const zt_method_meta *zt_find_method_meta(const zt_lower_ctx *ctx, const char *receiver_type, const char *method_name) {
    const zt_method_meta *fallback = NULL;
    size_t i;
    if (receiver_type == NULL || method_name == NULL) return NULL;
    for (i = 0; i < ctx->method_count; i += 1) {
        const zt_method_meta *m = &ctx->methods[i];
        if (!zt_name_eq(m->receiver_type, receiver_type)) continue;
        if (!zt_text_eq(m->method_name, method_name)) continue;
        if (m->trait_name == NULL) return m;
        if (fallback == NULL) fallback = m;
    }
    return fallback;
}

static zt_const_meta *zt_find_const_meta(zt_lower_ctx *ctx, const char *name) {
    size_t i;
    if (ctx == NULL || name == NULL) return NULL;
    for (i = 0; i < ctx->const_count; i += 1) {
        if (zt_name_eq(ctx->consts[i].name, name)) return &ctx->consts[i];
    }
    return NULL;
}

static const zt_scope_binding *zt_scope_find_binding(const zt_scope *scope, const char *name) {
    const zt_scope *cursor = scope;
    while (cursor != NULL) {
        size_t i;
        for (i = 0; i < cursor->count; i += 1) {
            if (zt_text_eq(cursor->items[i].name, name)) return &cursor->items[i];
        }
        cursor = cursor->parent;
    }
    return NULL;
}

static int zt_scope_expr_is_mutable_target(const zt_scope *scope, const zt_ast_node *expr) {
    const zt_scope_binding *binding;
    if (expr == NULL) return 0;
    switch (expr->kind) {
        case ZT_AST_IDENT_EXPR:
            binding = zt_scope_find_binding(scope, expr->as.ident_expr.name);
            return binding != NULL && binding->is_mutable;
        case ZT_AST_FIELD_EXPR:
            return zt_scope_expr_is_mutable_target(scope, expr->as.field_expr.object);
        case ZT_AST_INDEX_EXPR:
            return zt_scope_expr_is_mutable_target(scope, expr->as.index_expr.object);
        default:
            return 0;
    }
}

static zt_var_meta *zt_find_var_meta(zt_lower_ctx *ctx, const char *name) {
    size_t i;
    if (ctx == NULL || name == NULL) return NULL;
    for (i = 0; i < ctx->var_count; i += 1) {
        if (zt_name_eq(ctx->vars[i].name, name)) return &ctx->vars[i];
    }
    return NULL;
}

static char *zt_build_apply_name(const char *target, const char *trait, const char *method) {
    char buffer[1024];
    if (target == NULL || method == NULL) return zt_lower_strdup("<invalid>");
    if (trait != NULL) {
        snprintf(buffer, sizeof(buffer), "%s__%s__%s", target, trait, method);
    } else {
        snprintf(buffer, sizeof(buffer), "%s__%s", target, method);
    }
    return zt_lower_strdup(buffer);
}

static void zt_collect_struct_symbols(zt_lower_ctx *ctx, const zt_ast_node *decl) {
    size_t i;
    zt_struct_meta *meta;
    if (decl == NULL || decl->kind != ZT_AST_STRUCT_DECL) return;
    meta = zt_push_struct_meta(ctx);
    if (meta == NULL) return;
    meta->name = zt_lower_strdup(decl->as.struct_decl.name);
    for (i = 0; i < decl->as.struct_decl.fields.count; i += 1) {
        const zt_ast_node *field = decl->as.struct_decl.fields.items[i];
        zt_struct_field_meta *slot;
        if (field == NULL || field->kind != ZT_AST_STRUCT_FIELD) continue;
        slot = zt_push_struct_field_meta(meta);
        if (slot == NULL) continue;
        slot->name = zt_lower_strdup(field->as.struct_field.name);
        slot->type = zt_lower_type_from_ast(ctx, field->as.struct_field.type_node);
        slot->default_value = field->as.struct_field.default_value;
    }
}

static zt_func_meta *zt_collect_func_symbol(
        zt_lower_ctx *ctx,
        const char *name,
        const zt_ast_node_list *params,
        const zt_ast_node *return_type) {
    size_t i;
    zt_func_meta *meta = zt_push_func_meta(ctx);
    if (meta == NULL) return NULL;
    meta->name = zt_lower_strdup(name);
    meta->return_type = return_type != NULL ? zt_lower_type_from_ast(ctx, return_type) : zt_type_make(ZT_TYPE_VOID);
    for (i = 0; i < params->count; i += 1) {
        const zt_ast_node *param = params->items[i];
        zt_func_param_meta *p;
        if (param == NULL || param->kind != ZT_AST_PARAM) continue;
        p = zt_push_func_param_meta(meta);
        if (p == NULL) continue;
        p->name = zt_lower_strdup(param->as.param.name);
        p->type = zt_lower_type_from_ast(ctx, param->as.param.type_node);
        p->default_value = param->as.param.default_value;
    }
    return meta;
}

static void zt_collect_apply_symbols(zt_lower_ctx *ctx, const zt_ast_node *decl) {
    size_t i;
    if (decl == NULL || decl->kind != ZT_AST_APPLY_DECL) return;
    for (i = 0; i < decl->as.apply_decl.methods.count; i += 1) {
        const zt_ast_node *method = decl->as.apply_decl.methods.items[i];
        zt_method_meta *m;
        char *lowered_name;
        if (method == NULL || method->kind != ZT_AST_FUNC_DECL) continue;
        lowered_name = zt_build_apply_name(
            decl->as.apply_decl.target_name,
            decl->as.apply_decl.trait_name,
            method->as.func_decl.name);
        zt_collect_func_symbol(ctx, lowered_name, &method->as.func_decl.params, method->as.func_decl.return_type);
        m = zt_push_method_meta(ctx);
        if (m != NULL) {
            m->receiver_type = zt_lower_strdup(decl->as.apply_decl.target_name);
            m->method_name = zt_lower_strdup(method->as.func_decl.name);
            m->lowered_name = zt_lower_strdup(lowered_name);
            m->trait_name = zt_lower_strdup(decl->as.apply_decl.trait_name);
            m->is_mutating = method->as.func_decl.is_mutating;
        }
        free(lowered_name);
    }
}

static void zt_collect_const_symbol(zt_lower_ctx *ctx, const zt_ast_node *decl) {
    zt_const_meta *meta;
    if (decl == NULL || decl->kind != ZT_AST_CONST_DECL) return;
    meta = zt_push_const_meta(ctx);
    if (meta == NULL) return;
    meta->name = zt_lower_strdup(decl->as.const_decl.name);
    meta->type = zt_lower_type_from_ast(ctx, decl->as.const_decl.type_node);
    meta->init_value = decl->as.const_decl.init_value;
    meta->is_lowering = 0;
}

static void zt_collect_var_symbol(zt_lower_ctx *ctx, const zt_ast_node *decl) {
    zt_var_meta *meta;
    if (decl == NULL || decl->kind != ZT_AST_VAR_DECL) return;
    meta = zt_push_var_meta(ctx);
    if (meta == NULL) return;
    meta->name = zt_lower_strdup(decl->as.var_decl.name);
    meta->lowered_name = zt_build_module_var_symbol(meta->name);
    meta->type = zt_lower_type_from_ast(ctx, decl->as.var_decl.type_node);
    meta->init_value = decl->as.var_decl.init_value;
    meta->is_lowering = 0;
}

static void zt_collect_extern_symbols(zt_lower_ctx *ctx, const zt_ast_node *decl) {
    size_t i;
    if (ctx == NULL || decl == NULL || decl->kind != ZT_AST_EXTERN_DECL) return;

    for (i = 0; i < decl->as.extern_decl.functions.count; i += 1) {
        const zt_ast_node *func = decl->as.extern_decl.functions.items[i];
        zt_func_meta *meta;
        if (func == NULL || func->kind != ZT_AST_FUNC_DECL) continue;

        meta = zt_collect_func_symbol(ctx, func->as.func_decl.name, &func->as.func_decl.params, func->as.func_decl.return_type);
        if (meta == NULL) continue;

        meta->is_extern = 1;
        meta->extern_binding = zt_lower_strdup(decl->as.extern_decl.binding);
        meta->extern_abi_name = zt_lower_strdup(zt_last_segment(func->as.func_decl.name));
    }
}

static void zt_collect_symbols(zt_lower_ctx *ctx, const zt_ast_node *root) {
    size_t i;
    if (root == NULL || root->kind != ZT_AST_FILE) return;
    for (i = 0; i < root->as.file.declarations.count; i += 1) {
        const zt_ast_node *decl = root->as.file.declarations.items[i];
        if (decl != NULL && decl->kind == ZT_AST_STRUCT_DECL) zt_collect_struct_symbols(ctx, decl);
    }
    for (i = 0; i < root->as.file.declarations.count; i += 1) {
        const zt_ast_node *decl = root->as.file.declarations.items[i];
        if (decl == NULL) continue;
        if (decl->kind == ZT_AST_FUNC_DECL) {
            zt_collect_func_symbol(ctx, decl->as.func_decl.name, &decl->as.func_decl.params, decl->as.func_decl.return_type);
        } else if (decl->kind == ZT_AST_EXTERN_DECL) {
            zt_collect_extern_symbols(ctx, decl);
        } else if (decl->kind == ZT_AST_APPLY_DECL) {
            zt_collect_apply_symbols(ctx, decl);
        } else if (decl->kind == ZT_AST_CONST_DECL) {
            zt_collect_const_symbol(ctx, decl);
        } else if (decl->kind == ZT_AST_VAR_DECL) {
            zt_collect_var_symbol(ctx, decl);
        }
    }
}

static int zt_expr_to_path(const zt_ast_node *expr, char *buffer, size_t capacity) {
    char left[512];
    size_t left_len;
    size_t right_len;
    if (expr == NULL || buffer == NULL || capacity == 0) return 0;
    if (expr->kind == ZT_AST_IDENT_EXPR) {
        if (expr->as.ident_expr.name == NULL) return 0;
        if (strlen(expr->as.ident_expr.name) + 1 > capacity) return 0;
        memcpy(buffer, expr->as.ident_expr.name, strlen(expr->as.ident_expr.name) + 1);
        return 1;
    }
    if (expr->kind == ZT_AST_FIELD_EXPR) {
        if (!zt_expr_to_path(expr->as.field_expr.object, left, sizeof(left))) return 0;
        left_len = strlen(left);
        right_len = expr->as.field_expr.field_name != NULL ? strlen(expr->as.field_expr.field_name) : 0;
        if (left_len + 1 + right_len + 1 > capacity) return 0;
        memcpy(buffer, left, left_len);
        buffer[left_len] = '.';
        if (right_len > 0) memcpy(buffer + left_len + 1, expr->as.field_expr.field_name, right_len);
        buffer[left_len + 1 + right_len] = '\0';
        return 1;
    }
    return 0;
}

static zt_hir_expr *zt_make_expr(zt_hir_expr_kind kind, zt_source_span span, zt_type *type) {
    zt_hir_expr *expr = zt_hir_expr_make(kind, span);
    if (expr == NULL) {
        zt_type_dispose(type);
        return NULL;
    }
    zt_type_dispose(expr->type);
    expr->type = type != NULL ? type : zt_unknown_type();
    return expr;
}

static zt_hir_expr *zt_lower_expr(zt_lower_ctx *ctx, zt_scope *scope, const zt_ast_node *expr, const zt_type *expected);

static zt_hir_expr_list zt_lower_call_args(
        zt_lower_ctx *ctx,
        zt_scope *scope,
        const zt_ast_node *call_expr,
        const zt_func_meta *meta) {
    zt_hir_expr_list out = zt_hir_expr_list_make();
    size_t i;
    size_t positional_count;
    size_t named_count;

    if (call_expr == NULL || call_expr->kind != ZT_AST_CALL_EXPR) return out;

    positional_count = call_expr->as.call_expr.positional_args.count;
    named_count = call_expr->as.call_expr.named_args.count;

    if (meta == NULL) {
        size_t total = positional_count + named_count;
        if (total > 0) {
            out.items = (zt_hir_expr **)calloc(total, sizeof(zt_hir_expr *));
            if (out.items != NULL) out.capacity = total;
        }

        for (i = 0; i < positional_count; i += 1) {
            zt_hir_expr_list_push(&out, zt_lower_expr(ctx, scope, call_expr->as.call_expr.positional_args.items[i], NULL));
        }
        for (i = 0; i < named_count; i += 1) {
            zt_hir_expr_list_push(&out, zt_lower_expr(ctx, scope, call_expr->as.call_expr.named_args.items[i].value, NULL));
        }
        if (named_count > 0) {
            zt_add_diag(ctx, call_expr->span, "named arguments require known signature during lowering");
        }
        return out;
    }

    {
        const size_t param_count = meta->param_count;
        zt_hir_expr *ordered_stack[16];
        unsigned char filled_stack[16];
        zt_hir_expr **ordered = ordered_stack;
        unsigned char *filled = filled_stack;
        int heap_alloc = 0;
        size_t storage_count = param_count > 0 ? param_count : 1;
        size_t pos = 0;

        memset(ordered_stack, 0, sizeof(ordered_stack));
        memset(filled_stack, 0, sizeof(filled_stack));

        if (storage_count > 16) {
            ordered = (zt_hir_expr **)calloc(storage_count, sizeof(zt_hir_expr *));
            filled = (unsigned char *)calloc(storage_count, sizeof(unsigned char));
            heap_alloc = 1;
        }

        if (ordered == NULL || filled == NULL) {
            if (heap_alloc) {
                free(ordered);
                free(filled);
            }
            zt_add_diag(ctx, call_expr->span, "out of memory lowering call args");
            return out;
        }

        if (param_count > 0) {
            out.items = (zt_hir_expr **)calloc(param_count, sizeof(zt_hir_expr *));
            if (out.items != NULL) out.capacity = param_count;
        }

        for (i = 0; i < positional_count; i += 1) {
            if (pos >= param_count) {
                zt_add_diag(ctx, call_expr->span, "too many positional args");
                break;
            }
            ordered[pos] = zt_lower_expr(ctx, scope, call_expr->as.call_expr.positional_args.items[i], meta->params[pos].type);
            filled[pos] = 1;
            pos += 1;
        }

        for (i = 0; i < named_count; i += 1) {
            const zt_ast_named_arg *named = &call_expr->as.call_expr.named_args.items[i];
            size_t p;
            int found = 0;
            for (p = 0; p < param_count; p += 1) {
                if (zt_text_eq(meta->params[p].name, named->name)) {
                    found = 1;
                    if (filled[p]) {
                        zt_add_diag(ctx, named->span, "duplicate argument");
                    } else {
                        ordered[p] = zt_lower_expr(ctx, scope, named->value, meta->params[p].type);
                        filled[p] = 1;
                    }
                    break;
                }
            }
            if (!found) zt_add_diag(ctx, named->span, "unknown named argument");
        }

        for (i = 0; i < param_count; i += 1) {
            if (!filled[i]) {
                if (meta->params[i].default_value != NULL) {
                    ordered[i] = zt_lower_expr(ctx, scope, meta->params[i].default_value, meta->params[i].type);
                } else {
                    zt_add_diag(ctx, call_expr->span, "missing required argument");
                    ordered[i] = zt_make_expr(ZT_HIR_IDENT_EXPR, call_expr->span, zt_unknown_type());
                    if (ordered[i] != NULL) ordered[i]->as.ident_expr.name = zt_lower_strdup("<missing>");
                }
            }
            zt_hir_expr_list_push(&out, ordered[i]);
        }

        if (heap_alloc) {
            free(ordered);
            free(filled);
        }
    }

    return out;
}

static int zt_is_module_var_symbol_char(char ch) {
    return ((ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9'));
}

static char *zt_build_module_var_symbol(const char *name) {
    size_t len;
    size_t capacity;
    char *out;
    size_t i;
    size_t pos = 0;
    const char *prefix = "zt_modvar_";

    if (name == NULL) return NULL;
    len = strlen(name);
    capacity = strlen(prefix) + (len * 6) + 1;
    out = (char *)malloc(capacity);
    if (out == NULL) return NULL;

    memcpy(out, prefix, strlen(prefix));
    pos = strlen(prefix);

    for (i = 0; i < len && pos + 1 < capacity; i += 1) {
        unsigned char ch = (unsigned char)name[i];
        if (zt_is_module_var_symbol_char((char)ch)) {
            out[pos++] = (char)ch;
            continue;
        }
        if (ch == '_') {
            if (pos + 2 >= capacity) break;
            out[pos++] = '_';
            out[pos++] = '_';
            continue;
        }
        if (ch == '.') {
            if (pos + 3 >= capacity) break;
            out[pos++] = '_';
            out[pos++] = 'd';
            out[pos++] = '_';
            continue;
        }
        if (pos + 5 >= capacity) break;
        snprintf(out + pos, capacity - pos, "_x%02x_", (unsigned int)ch);
        pos += 5;
    }

    out[pos] = '\0';
    return out;
}

static zt_hir_expr *zt_make_text_literal_expr(zt_source_span span, const char *value) {
    zt_hir_expr *expr = zt_make_expr(ZT_HIR_STRING_EXPR, span, zt_type_make(ZT_TYPE_TEXT));
    if (expr != NULL) {
        expr->as.string_expr.value = zt_lower_strdup(value != NULL ? value : "");
    }
    return expr;
}

static zt_hir_expr *zt_make_text_concat_expr(zt_source_span span, zt_hir_expr *left, zt_hir_expr *right) {
    zt_hir_expr *call = zt_make_expr(ZT_HIR_CALL_EXPR, span, zt_type_make(ZT_TYPE_TEXT));
    zt_hir_expr_list args = zt_hir_expr_list_make();
    if (call == NULL) {
        zt_hir_expr_dispose(left);
        zt_hir_expr_dispose(right);
        return NULL;
    }
    zt_hir_expr_list_push(&args, left);
    zt_hir_expr_list_push(&args, right);
    call->as.call_expr.callee_name = zt_lower_strdup("path.zt_text_concat");
    call->as.call_expr.args = args;
    return call;
}

static zt_type *zt_make_dyn_textrepresentable_type(void) {
    zt_type_list args = zt_type_list_make();
    zt_type_list_push(&args, zt_type_make_named(ZT_TYPE_USER, "TextRepresentable"));
    return zt_type_make_with_args(ZT_TYPE_DYN, "dyn", args);
}

static zt_hir_expr *zt_make_fmt_to_text_expr(
        zt_lower_ctx *ctx,
        zt_scope *scope,
        const zt_ast_node *part) {
    zt_hir_expr *value_expr = zt_lower_expr(ctx, scope, part, NULL);
    zt_hir_expr *boxed_expr = NULL;
    zt_hir_expr *to_text_call = NULL;
    zt_hir_expr_list boxed_args = zt_hir_expr_list_make();
    zt_hir_expr_list to_text_args = zt_hir_expr_list_make();
    zt_source_span span = part != NULL ? part->span : zt_source_span_unknown();
    const char *box_callee = NULL;
    zt_type_kind value_kind;

    if (value_expr == NULL) {
        return zt_make_text_literal_expr(span, "");
    }

    if (value_expr->type != NULL && value_expr->type->kind == ZT_TYPE_TEXT) {
        return value_expr;
    }

    if (value_expr->type == NULL) {
        zt_hir_expr_dispose(value_expr);
        zt_add_diag(ctx, span, "fmt interpolation could not infer expression type");
        return zt_make_text_literal_expr(span, "");
    }

    value_kind = value_expr->type->kind;
    if (value_kind == ZT_TYPE_DYN) {
        boxed_expr = value_expr;
    } else {
        if (value_kind == ZT_TYPE_BOOL) {
            box_callee = "core.fmt_box_bool";
        } else if (value_kind == ZT_TYPE_FLOAT || value_kind == ZT_TYPE_FLOAT32 || value_kind == ZT_TYPE_FLOAT64) {
            box_callee = "core.fmt_box_float";
        } else if (zt_type_is_integral(value_expr->type)) {
            box_callee = "core.fmt_box_i64";
        } else if (value_kind == ZT_TYPE_TEXT) {
            box_callee = "core.fmt_box_text";
        } else {
            char type_name[256];
            char message[320];
            zt_type_format(value_expr->type, type_name, sizeof(type_name));
            snprintf(message, sizeof(message), "fmt interpolation does not support type '%s' in this backend cut", type_name);
            zt_add_diag(ctx, span, message);
            zt_hir_expr_dispose(value_expr);
            return zt_make_text_literal_expr(span, "");
        }

        boxed_expr = zt_make_expr(ZT_HIR_CALL_EXPR, span, zt_make_dyn_textrepresentable_type());
        if (boxed_expr == NULL) {
            zt_hir_expr_dispose(value_expr);
            return zt_make_text_literal_expr(span, "");
        }

        zt_hir_expr_list_push(&boxed_args, value_expr);
        boxed_expr->as.call_expr.callee_name = zt_lower_strdup(box_callee);
        boxed_expr->as.call_expr.args = boxed_args;
    }

    to_text_call = zt_make_expr(ZT_HIR_CALL_EXPR, span, zt_type_make(ZT_TYPE_TEXT));
    if (to_text_call == NULL) {
        zt_hir_expr_dispose(boxed_expr);
        return zt_make_text_literal_expr(span, "");
    }
    zt_hir_expr_list_push(&to_text_args, boxed_expr);
    to_text_call->as.call_expr.callee_name = zt_lower_strdup("core.dyn_text_repr_to_text");
    to_text_call->as.call_expr.args = to_text_args;
    return to_text_call;
}

static zt_hir_expr *zt_lower_fmt_expr(
        zt_lower_ctx *ctx,
        zt_scope *scope,
        const zt_ast_node *expr) {
    zt_hir_expr *acc = NULL;
    size_t i;
    if (expr == NULL || expr->kind != ZT_AST_FMT_EXPR) return zt_make_text_literal_expr(zt_source_span_unknown(), "");

    for (i = 0; i < expr->as.fmt_expr.parts.count; i += 1) {
        const zt_ast_node *part = expr->as.fmt_expr.parts.items[i];
        zt_hir_expr *piece = NULL;
        if (part == NULL) continue;

        if (part->kind == ZT_AST_STRING_EXPR) {
            piece = zt_lower_expr(ctx, scope, part, zt_type_make(ZT_TYPE_TEXT));
        } else {
            piece = zt_make_fmt_to_text_expr(ctx, scope, part);
        }

        if (piece == NULL) continue;
        if (acc == NULL) {
            acc = piece;
            continue;
        }

        acc = zt_make_text_concat_expr(expr->span, acc, piece);
    }

    if (acc == NULL) {
        acc = zt_make_text_literal_expr(expr->span, "");
    }
    return acc;
}

static zt_hir_expr *zt_lower_struct_constructor(
        zt_lower_ctx *ctx,
        zt_scope *scope,
        const zt_ast_node *call_expr,
        const char *type_name,
        const zt_struct_meta *meta) {
    zt_hir_expr *construct;
    size_t i;
    zt_hir_expr **provided;
    unsigned char *filled;

    construct = zt_make_expr(ZT_HIR_CONSTRUCT_EXPR, call_expr->span, zt_type_make_named(ZT_TYPE_USER, type_name));
    if (construct == NULL) return NULL;
    construct->as.construct_expr.type_name = zt_lower_strdup(type_name);
    construct->as.construct_expr.fields = zt_hir_field_init_list_make();

    provided = (zt_hir_expr **)calloc(meta->field_count > 0 ? meta->field_count : 1, sizeof(zt_hir_expr *));
    filled = (unsigned char *)calloc(meta->field_count > 0 ? meta->field_count : 1, sizeof(unsigned char));
    if (provided == NULL || filled == NULL) {
        free(provided);
        free(filled);
        zt_hir_expr_dispose(construct);
        zt_add_diag(ctx, call_expr->span, "out of memory in struct constructor lowering");
        return NULL;
    }

    {
        size_t pos = 0;
        for (i = 0; i < call_expr->as.call_expr.positional_args.count; i += 1) {
            if (pos >= meta->field_count) {
                zt_add_diag(ctx, call_expr->span, "too many positional args in constructor");
                break;
            }
            provided[pos] = zt_lower_expr(ctx, scope, call_expr->as.call_expr.positional_args.items[i], meta->fields[pos].type);
            filled[pos] = 1;
            pos += 1;
        }

        for (i = 0; i < call_expr->as.call_expr.named_args.count; i += 1) {
            const zt_ast_named_arg *named = &call_expr->as.call_expr.named_args.items[i];
            size_t f;
            int found = 0;
            for (f = 0; f < meta->field_count; f += 1) {
                if (zt_text_eq(meta->fields[f].name, named->name)) {
                    found = 1;
                    if (filled[f]) {
                        zt_add_diag(ctx, named->span, "duplicate field argument");
                    } else {
                        provided[f] = zt_lower_expr(ctx, scope, named->value, meta->fields[f].type);
                        filled[f] = 1;
                    }
                    break;
                }
            }
            if (!found) zt_add_diag(ctx, named->span, "unknown constructor field");
        }

        for (i = 0; i < meta->field_count; i += 1) {
            zt_hir_field_init init;
            memset(&init, 0, sizeof(init));
            init.span = call_expr->span;
            init.name = zt_lower_strdup(meta->fields[i].name);
            if (filled[i]) {
                init.value = provided[i];
            } else if (meta->fields[i].default_value != NULL) {
                init.value = zt_lower_expr(ctx, scope, meta->fields[i].default_value, meta->fields[i].type);
            } else {
                zt_add_diag(ctx, call_expr->span, "missing required field in constructor");
                init.value = zt_make_expr(ZT_HIR_IDENT_EXPR, call_expr->span, zt_unknown_type());
                if (init.value != NULL) init.value->as.ident_expr.name = zt_lower_strdup("<missing>");
            }
            zt_hir_field_init_list_push(&construct->as.construct_expr.fields, init);
        }
    }

    free(provided);
    free(filled);
    return construct;
}

static zt_hir_expr *zt_lower_enum_unit_variant(
        const zt_ast_node *expr,
        const char *enum_name,
        size_t variant_index) {
    zt_hir_expr *construct;
    zt_hir_field_init tag_init;
    zt_hir_expr *tag_expr;
    char tag_buffer[32];

    if (expr == NULL || enum_name == NULL) return NULL;

    construct = zt_make_expr(ZT_HIR_CONSTRUCT_EXPR, expr->span, zt_type_make_named(ZT_TYPE_USER, enum_name));
    if (construct == NULL) return NULL;
    construct->as.construct_expr.type_name = zt_lower_strdup(enum_name);
    construct->as.construct_expr.fields = zt_hir_field_init_list_make();

    memset(&tag_init, 0, sizeof(tag_init));
    snprintf(tag_buffer, sizeof(tag_buffer), "%zu", variant_index);
    tag_expr = zt_make_expr(ZT_HIR_INT_EXPR, expr->span, zt_type_make(ZT_TYPE_INT));
    if (tag_expr != NULL) tag_expr->as.int_expr.value = zt_lower_strdup(tag_buffer);
    tag_init.span = expr->span;
    tag_init.name = zt_lower_strdup("__zt_enum_tag");
    tag_init.value = tag_expr;
    zt_hir_field_init_list_push(&construct->as.construct_expr.fields, tag_init);

    return construct;
}
static zt_hir_expr *zt_lower_enum_constructor(
        zt_lower_ctx *ctx,
        zt_scope *scope,
        const zt_ast_node *call_expr,
        const char *enum_name,
        const zt_ast_node *variant_decl,
        size_t variant_index) {
    zt_hir_expr *construct;
    size_t field_count = variant_decl != NULL ? variant_decl->as.enum_variant.fields.count : 0;
    zt_hir_expr **provided = NULL;
    unsigned char *filled = NULL;
    size_t i;

    construct = zt_make_expr(ZT_HIR_CONSTRUCT_EXPR, call_expr->span, zt_type_make_named(ZT_TYPE_USER, enum_name));
    if (construct == NULL) return NULL;
    construct->as.construct_expr.type_name = zt_lower_strdup(enum_name);
    construct->as.construct_expr.fields = zt_hir_field_init_list_make();

    {
        zt_hir_field_init tag_init;
        zt_hir_expr *tag_expr;
        char tag_buffer[32];
        memset(&tag_init, 0, sizeof(tag_init));
        snprintf(tag_buffer, sizeof(tag_buffer), "%zu", variant_index);
        tag_expr = zt_make_expr(ZT_HIR_INT_EXPR, call_expr->span, zt_type_make(ZT_TYPE_INT));
        if (tag_expr != NULL) tag_expr->as.int_expr.value = zt_lower_strdup(tag_buffer);
        tag_init.span = call_expr->span;
        tag_init.name = zt_lower_strdup("__zt_enum_tag");
        tag_init.value = tag_expr;
        zt_hir_field_init_list_push(&construct->as.construct_expr.fields, tag_init);
    }

    if (field_count == 0) {
        if (call_expr->as.call_expr.positional_args.count != 0 || call_expr->as.call_expr.named_args.count != 0) {
            zt_add_diag(ctx, call_expr->span, "enum variant without payload cannot receive constructor arguments");
        }
        return construct;
    }

    provided = (zt_hir_expr **)calloc(field_count, sizeof(zt_hir_expr *));
    filled = (unsigned char *)calloc(field_count, sizeof(unsigned char));
    if (provided == NULL || filled == NULL) {
        free(provided);
        free(filled);
        zt_hir_expr_dispose(construct);
        zt_add_diag(ctx, call_expr->span, "out of memory in enum constructor lowering");
        return NULL;
    }

    for (i = 0; i < call_expr->as.call_expr.positional_args.count; i += 1) {
        const zt_ast_node *field_node;
        const zt_ast_node *field_type_node;
        zt_type *expected_type;
        if (i >= field_count) {
            zt_add_diag(ctx, call_expr->span, "too many positional args in enum constructor");
            break;
        }
        field_node = variant_decl->as.enum_variant.fields.items[i];
        field_type_node = zt_enum_variant_field_type_node(field_node);
        expected_type = zt_lower_type_from_ast(ctx, field_type_node);
        provided[i] = zt_lower_expr(ctx, scope, call_expr->as.call_expr.positional_args.items[i], expected_type);
        filled[i] = 1;
        zt_type_dispose(expected_type);
    }

    for (i = 0; i < call_expr->as.call_expr.named_args.count; i += 1) {
        const zt_ast_named_arg *named = &call_expr->as.call_expr.named_args.items[i];
        size_t f;
        int found = 0;
        for (f = 0; f < field_count; f += 1) {
            const zt_ast_node *field_node = variant_decl->as.enum_variant.fields.items[f];
            const zt_ast_node *field_type_node;
            const char *field_name;
            char fallback_name[32];
            zt_type *expected_type;
            if (field_node == NULL) continue;
            field_name = zt_enum_variant_field_name(field_node, f, fallback_name, sizeof(fallback_name));
            if (field_name == NULL || strcmp(field_name, named->name) != 0) continue;

            field_type_node = zt_enum_variant_field_type_node(field_node);
            if (filled[f]) {
                zt_add_diag(ctx, named->span, "duplicate enum payload argument");
            } else {
                expected_type = zt_lower_type_from_ast(ctx, field_type_node);
                provided[f] = zt_lower_expr(ctx, scope, named->value, expected_type);
                filled[f] = 1;
                zt_type_dispose(expected_type);
            }
            found = 1;
            break;
        }
        if (!found) zt_add_diag(ctx, named->span, "unknown enum payload field");
    }

    for (i = 0; i < field_count; i += 1) {
        const zt_ast_node *field_node = variant_decl->as.enum_variant.fields.items[i];
        const char *field_name;
        char fallback_name[32];
        zt_hir_field_init init;
        memset(&init, 0, sizeof(init));
        init.span = call_expr->span;
        field_name = zt_enum_variant_field_name(field_node, i, fallback_name, sizeof(fallback_name));
        init.name = zt_lower_strdup(field_name != NULL ? field_name : "<field>");
        if (filled[i]) {
            init.value = provided[i];
        } else {
            zt_add_diag(ctx, call_expr->span, "missing enum payload field in constructor");
            init.value = zt_make_expr(ZT_HIR_IDENT_EXPR, call_expr->span, zt_unknown_type());
            if (init.value != NULL) init.value->as.ident_expr.name = zt_lower_strdup("<missing>");
        }
        zt_hir_field_init_list_push(&construct->as.construct_expr.fields, init);
    }

    free(provided);
    free(filled);
    return construct;
}

static zt_hir_expr *zt_lower_call_expr(
        zt_lower_ctx *ctx,
        zt_scope *scope,
        const zt_ast_node *expr,
        const zt_type *expected) {
    char callee_path[512];
    const zt_ast_node *callee;
    const zt_func_meta *func_meta = NULL;
    const zt_struct_meta *struct_meta = NULL;
    zt_hir_expr *result;

    callee = expr->as.call_expr.callee;
    callee_path[0] = '\0';

    if (callee != NULL && callee->kind == ZT_AST_FIELD_EXPR) {
        if (zt_expr_to_path(callee, callee_path, sizeof(callee_path))) {
            struct_meta = zt_find_struct_meta(ctx, callee_path);
            if (struct_meta != NULL) {
                return zt_lower_struct_constructor(
                    ctx,
                    scope,
                    expr,
                    struct_meta->name != NULL ? struct_meta->name : callee_path,
                    struct_meta);
            }
        }

        if (callee->as.field_expr.object != NULL &&
            callee->as.field_expr.field_name != NULL) {
            char enum_name_path[512];
            if (zt_expr_to_path(callee->as.field_expr.object, enum_name_path, sizeof(enum_name_path))) {
                const zt_ast_node *enum_decl = zt_find_enum_decl_ast(ctx, enum_name_path);
                if (enum_decl != NULL) {
                    size_t variant_index = 0;
                    const zt_ast_node *variant_decl = zt_find_enum_variant_ast(enum_decl, callee->as.field_expr.field_name, &variant_index);
                    if (variant_decl != NULL) {
                        return zt_lower_enum_constructor(
                            ctx,
                            scope,
                            expr,
                            enum_decl->as.enum_decl.name,
                            variant_decl,
                            variant_index);
                    }
                }
            }
        }

        zt_hir_expr *receiver = zt_lower_expr(ctx, scope, callee->as.field_expr.object, NULL);
        const zt_type *receiver_type = receiver != NULL ? receiver->type : NULL;
        if (receiver_type != NULL &&
            receiver_type->kind == ZT_TYPE_LIST &&
            receiver_type->args.count == 1 &&
            receiver_type->args.items[0] != NULL &&
            callee->as.field_expr.field_name != NULL &&
            strcmp(callee->as.field_expr.field_name, "get") == 0) {
            zt_hir_expr_list args = zt_hir_expr_list_make();
            zt_hir_expr *index_expr = NULL;
            zt_type *index_type = zt_type_make(ZT_TYPE_INT);
            zt_type_list optional_args = zt_type_list_make();
            zt_type *return_type;
            const char *callee_name = NULL;

            if (expr->as.call_expr.named_args.count != 0 || expr->as.call_expr.positional_args.count != 1) {
                zt_add_diag(ctx, expr->span, "list.get lowering expects exactly one positional index argument");
            }

            zt_hir_expr_list_push(&args, receiver);
            if (expr->as.call_expr.positional_args.count > 0) {
                index_expr = zt_lower_expr(ctx, scope, expr->as.call_expr.positional_args.items[0], index_type);
            } else {
                index_expr = zt_make_expr(ZT_HIR_IDENT_EXPR, expr->span, zt_unknown_type());
                if (index_expr != NULL) index_expr->as.ident_expr.name = zt_lower_strdup("<missing_index>");
            }
            zt_hir_expr_list_push(&args, index_expr);
            zt_type_dispose(index_type);

            zt_type_list_push(&optional_args, zt_type_clone(receiver_type->args.items[0]));
            return_type = zt_type_make_with_args(ZT_TYPE_OPTIONAL, "optional", optional_args);
            result = zt_make_expr(ZT_HIR_CALL_EXPR, expr->span, return_type);
            if (result == NULL) {
                zt_hir_expr_list_dispose(&args);
                return NULL;
            }

            if (receiver_type->args.items[0]->kind == ZT_TYPE_INT) {
                callee_name = "core.list_get_i64";
            } else if (receiver_type->args.items[0]->kind == ZT_TYPE_TEXT) {
                callee_name = "core.list_get_text";
            } else {
                callee_name = "core.list_get_unsupported";
                zt_add_diag(ctx, expr->span, "list.get lowering currently supports list<int> and list<text> in the C backend subset");
            }

            result->as.call_expr.callee_name = zt_lower_strdup(callee_name);
            result->as.call_expr.args = args;
            return result;
        }

        if (receiver_type != NULL &&
            receiver_type->kind == ZT_TYPE_MAP &&
            receiver_type->args.count == 2 &&
            callee->as.field_expr.field_name != NULL &&
            strcmp(callee->as.field_expr.field_name, "get") == 0) {
            zt_hir_expr_list args = zt_hir_expr_list_make();
            zt_hir_expr *key_expr = NULL;
            zt_type *key_type = zt_type_clone(receiver_type->args.items[0]);
            zt_type_list optional_args = zt_type_list_make();
            zt_type *return_type;

            if (expr->as.call_expr.named_args.count != 0 || expr->as.call_expr.positional_args.count != 1) {
                zt_add_diag(ctx, expr->span, "map.get lowering expects exactly one positional key argument");
            }

            zt_hir_expr_list_push(&args, receiver);
            if (expr->as.call_expr.positional_args.count > 0) {
                key_expr = zt_lower_expr(ctx, scope, expr->as.call_expr.positional_args.items[0], key_type);
            } else {
                key_expr = zt_make_expr(ZT_HIR_IDENT_EXPR, expr->span, zt_unknown_type());
                if (key_expr != NULL) key_expr->as.ident_expr.name = zt_lower_strdup("<missing_key>");
            }
            zt_hir_expr_list_push(&args, key_expr);
            zt_type_dispose(key_type);

            zt_type_list_push(&optional_args, zt_type_clone(receiver_type->args.items[1]));
            return_type = zt_type_make_with_args(ZT_TYPE_OPTIONAL, "optional", optional_args);
            result = zt_make_expr(ZT_HIR_CALL_EXPR, expr->span, return_type);
            if (result == NULL) {
                zt_hir_expr_list_dispose(&args);
                return NULL;
            }
            result->as.call_expr.callee_name = zt_lower_strdup("core.map_get");
            result->as.call_expr.args = args;
            return result;
        }

        if (receiver_type != NULL &&
            receiver_type->kind == ZT_TYPE_DYN &&
            receiver_type->args.count == 1 &&
            receiver_type->args.items[0] != NULL &&
            receiver_type->args.items[0]->name != NULL &&
            zt_text_eq(receiver_type->args.items[0]->name, "TextRepresentable") &&
            callee->as.field_expr.field_name != NULL &&
            strcmp(callee->as.field_expr.field_name, "to_text") == 0) {
            zt_hir_expr_list args = zt_hir_expr_list_make();

            if (expr->as.call_expr.named_args.count != 0 || expr->as.call_expr.positional_args.count != 0) {
                zt_add_diag(ctx, expr->span, "dyn<TextRepresentable>.to_text lowering expects no arguments");
            }

            zt_hir_expr_list_push(&args, receiver);
            result = zt_make_expr(ZT_HIR_CALL_EXPR, expr->span, zt_type_make(ZT_TYPE_TEXT));
            if (result == NULL) {
                zt_hir_expr_list_dispose(&args);
                return NULL;
            }
            result->as.call_expr.callee_name = zt_lower_strdup("core.dyn_text_repr_to_text");
            result->as.call_expr.args = args;
            return result;
        }

        /* R3.M4: Detect dyn method calls BEFORE looking up method meta */
        if (receiver_type != NULL && receiver_type->kind == ZT_TYPE_DYN &&
            receiver_type->args.count > 0 && receiver_type->args.items[0] != NULL &&
            callee->as.field_expr.field_name != NULL) {
            zt_hir_expr_list args = zt_lower_call_args(ctx, scope, expr, NULL);
            zt_type *return_type = expected != NULL ? zt_type_clone(expected) : zt_type_make(ZT_TYPE_TEXT);

            result = zt_make_expr(ZT_HIR_DYN_METHOD_CALL_EXPR, expr->span, return_type);
            if (result == NULL) {
                zt_hir_expr_dispose(receiver);
                zt_hir_expr_list_dispose(&args);
                return NULL;
            }
            result->as.dyn_method_call_expr.receiver = receiver;
            result->as.dyn_method_call_expr.method_name = zt_lower_strdup(callee->as.field_expr.field_name);
            result->as.dyn_method_call_expr.trait_name = zt_lower_strdup(receiver_type->args.items[0]->name);
            result->as.dyn_method_call_expr.args = args;
            return result;
        }

        const zt_method_meta *method = zt_find_method_meta(
            ctx,
            receiver_type != NULL ? receiver_type->name : NULL,
            callee->as.field_expr.field_name);
        if (method != NULL) {
            if (method->is_mutating &&
                !zt_scope_expr_is_mutable_target(scope, callee->as.field_expr.object)) {
                char diag[256];
                snprintf(diag, sizeof(diag), "mutating method '%s' requires a mutable receiver", callee->as.field_expr.field_name);
                zt_add_diag(ctx, expr->span, diag);
            }
            zt_hir_expr_list args = zt_lower_call_args(ctx, scope, expr, zt_find_func_meta(ctx, method->lowered_name));
            zt_type *return_type = expected != NULL
                ? zt_type_clone(expected)
                : (zt_find_func_meta(ctx, method->lowered_name) != NULL
                   ? zt_type_clone(zt_find_func_meta(ctx, method->lowered_name)->return_type)
                   : zt_unknown_type());

            /* R3.M4: Detect dyn method calls */
            if (receiver_type != NULL && receiver_type->kind == ZT_TYPE_DYN &&
                receiver_type->args.count > 0 && receiver_type->args.items[0] != NULL) {
                result = zt_make_expr(ZT_HIR_DYN_METHOD_CALL_EXPR, expr->span, return_type);
                if (result == NULL) {
                    zt_hir_expr_dispose(receiver);
                    zt_hir_expr_list_dispose(&args);
                    return NULL;
                }
                result->as.dyn_method_call_expr.receiver = receiver;
                result->as.dyn_method_call_expr.method_name = zt_lower_strdup(method->lowered_name);
                result->as.dyn_method_call_expr.trait_name = zt_lower_strdup(receiver_type->args.items[0]->name);
                result->as.dyn_method_call_expr.args = args;
                return result;
            }

            result = zt_make_expr(ZT_HIR_METHOD_CALL_EXPR, expr->span, return_type);
            if (result == NULL) {
                zt_hir_expr_dispose(receiver);
                zt_hir_expr_list_dispose(&args);
                return NULL;
            }
            result->as.method_call_expr.receiver = receiver;
            result->as.method_call_expr.method_name = zt_lower_strdup(method->lowered_name);
            result->as.method_call_expr.args = args;
            return result;
        }
        zt_hir_expr_dispose(receiver);
    }

    if (!zt_expr_to_path(callee, callee_path, sizeof(callee_path))) {
        zt_add_diag(ctx, expr->span, "unsupported call target");
        result = zt_make_expr(ZT_HIR_IDENT_EXPR, expr->span, zt_unknown_type());
        if (result != NULL) result->as.ident_expr.name = zt_lower_strdup("<invalid_call>");
        return result;
    }

    if (callee != NULL && callee->kind == ZT_AST_IDENT_EXPR) {
        struct_meta = zt_find_struct_meta(ctx, callee_path);
        if (struct_meta != NULL) {
            return zt_lower_struct_constructor(ctx, scope, expr, callee_path, struct_meta);
        }

        /* R3.M5 Phase 2b: Check if callee is a callable variable (indirect call) */
        {
            const zt_type *callee_type = zt_scope_get(scope, callee->as.ident_expr.name);
            if (callee_type != NULL && callee_type->kind == ZT_TYPE_CALLABLE) {
                zt_hir_expr *callable_expr = zt_lower_expr(ctx, scope, callee, NULL);
                zt_hir_expr_list args = zt_lower_call_args(ctx, scope, expr, NULL);
                zt_type *return_type = expected != NULL
                    ? zt_type_clone(expected)
                    : (callee_type->args.count > 0 ? zt_type_clone(callee_type->args.items[0]) : zt_unknown_type());

                result = zt_make_expr(ZT_HIR_CALL_INDIRECT_EXPR, expr->span, return_type);
                if (result == NULL) {
                    zt_hir_expr_dispose(callable_expr);
                    zt_hir_expr_list_dispose(&args);
                    return NULL;
                }
                result->as.call_indirect_expr.callable = callable_expr;
                result->as.call_indirect_expr.args = args;
                return result;
            }
        }
    }

    func_meta = zt_find_func_meta(ctx, callee_path);
    {
        zt_hir_expr_list args = zt_lower_call_args(ctx, scope, expr, func_meta);
        char *callee_name = NULL;
        zt_type *return_type = expected != NULL
            ? zt_type_clone(expected)
            : (func_meta != NULL ? zt_type_clone(func_meta->return_type) :
               (zt_text_eq(callee_path, "len") ? zt_type_make(ZT_TYPE_INT) : zt_unknown_type()));
        result = zt_make_expr(ZT_HIR_CALL_EXPR, expr->span, return_type);
        if (result == NULL) {
            zt_hir_expr_list_dispose(&args);
            return NULL;
        }

        if (func_meta != NULL &&
            func_meta->is_extern &&
            func_meta->extern_binding != NULL &&
            func_meta->extern_abi_name != NULL &&
            strcmp(func_meta->extern_binding, "c") == 0) {
            callee_name = zt_lower_join_with_dot(func_meta->extern_binding, func_meta->extern_abi_name);
        }
        if (callee_name == NULL) {
            callee_name = zt_lower_strdup(callee_path);
        }

        result->as.call_expr.callee_name = callee_name;
        result->as.call_expr.args = args;
        return result;
    }
}

static zt_hir_expr *zt_lower_expr(zt_lower_ctx *ctx, zt_scope *scope, const zt_ast_node *expr, const zt_type *expected) {
    zt_hir_expr *out = NULL;
    if (expr == NULL) return NULL;

    switch (expr->kind) {
        case ZT_AST_IDENT_EXPR: {
            const zt_type *bound_type = zt_scope_get(scope, expr->as.ident_expr.name);
            zt_const_meta *const_meta = NULL;
            zt_var_meta *var_meta = NULL;
            const char *resolved_name = expr->as.ident_expr.name;
            if (bound_type == NULL) {
                const_meta = zt_find_const_meta(ctx, expr->as.ident_expr.name);
                var_meta = zt_find_var_meta(ctx, expr->as.ident_expr.name);
            }
            if (const_meta != NULL && const_meta->init_value != NULL) {
                if (const_meta->is_lowering) {
                    zt_add_diag(ctx, expr->span, "recursive const reference during lowering");
                } else {
                    zt_hir_expr *inlined;
                    const_meta->is_lowering = 1;
                    inlined = zt_lower_expr(ctx, scope, const_meta->init_value, const_meta->type);
                    const_meta->is_lowering = 0;
                    if (inlined != NULL) return inlined;
                }
            }
            if (var_meta != NULL) {
                if (bound_type == NULL) {
                    bound_type = var_meta->type;
                }
                if (var_meta->lowered_name != NULL) {
                    resolved_name = var_meta->lowered_name;
                }
            }
            {
                zt_type *type = zt_type_clone(bound_type);
                if (type == NULL) type = expected != NULL ? zt_type_clone(expected) : zt_unknown_type();

                /* R3.M5 Phase 2b: When expected type is callable and ident resolves to a
                 * top-level function, emit a func-ref expression instead of ident.
                 * The checker has already validated that the func is suitable (non-generic,
                 * non-mutating, no receiver). */
                if (expected != NULL && expected->kind == ZT_TYPE_CALLABLE) {
                    const zt_func_meta *fmeta = zt_find_func_meta(ctx, expr->as.ident_expr.name);
                    if (fmeta != NULL) {
                        zt_type *callable_type = zt_type_clone(expected);
                        out = zt_make_expr(ZT_HIR_FUNC_REF_EXPR, expr->span, zt_type_clone(callable_type));
                        if (out != NULL) {
                            out->as.func_ref_expr.func_name = zt_lower_strdup(fmeta->name);
                            out->as.func_ref_expr.callable_type = callable_type;
                        }
                        return out;
                    }
                }

                out = zt_make_expr(ZT_HIR_IDENT_EXPR, expr->span, type);
                if (out != NULL) out->as.ident_expr.name = zt_lower_strdup(resolved_name);
                return out;
            }
        }

        case ZT_AST_INT_EXPR:
            out = zt_make_expr(ZT_HIR_INT_EXPR, expr->span, zt_type_make(ZT_TYPE_INT));
            if (out != NULL) out->as.int_expr.value = zt_lower_strdup(expr->as.int_expr.value);
            return out;

        case ZT_AST_FLOAT_EXPR:
            out = zt_make_expr(ZT_HIR_FLOAT_EXPR, expr->span, zt_type_make(ZT_TYPE_FLOAT));
            if (out != NULL) out->as.float_expr.value = zt_lower_strdup(expr->as.float_expr.value);
            return out;

        case ZT_AST_STRING_EXPR:
            out = zt_make_expr(ZT_HIR_STRING_EXPR, expr->span, zt_type_make(ZT_TYPE_TEXT));
            if (out != NULL) out->as.string_expr.value = zt_lower_strdup(expr->as.string_expr.value);
            return out;

        case ZT_AST_FMT_EXPR:
            return zt_lower_fmt_expr(ctx, scope, expr);

        case ZT_AST_BYTES_EXPR:
            out = zt_make_expr(ZT_HIR_BYTES_EXPR, expr->span, zt_type_make(ZT_TYPE_BYTES));
            if (out != NULL) out->as.bytes_expr.value = zt_lower_strdup(expr->as.bytes_expr.value);
            return out;

        case ZT_AST_BOOL_EXPR:
            out = zt_make_expr(ZT_HIR_BOOL_EXPR, expr->span, zt_type_make(ZT_TYPE_BOOL));
            if (out != NULL) out->as.bool_expr.value = expr->as.bool_expr.value;
            return out;

        case ZT_AST_NONE_EXPR: {
            zt_type *none_type = expected != NULL && expected->kind == ZT_TYPE_OPTIONAL
                ? zt_type_clone(expected)
                : zt_type_make(ZT_TYPE_NONE_LITERAL);
            return zt_make_expr(ZT_HIR_NONE_EXPR, expr->span, none_type);
        }

        case ZT_AST_SUCCESS_EXPR: {
            zt_hir_expr *value = zt_lower_expr(ctx, scope, expr->as.success_expr.value, NULL);
            zt_type *type;
            if (expected != NULL && expected->kind == ZT_TYPE_RESULT) {
                type = zt_type_clone(expected);
            } else {
                zt_type_list args = zt_type_list_make();
                zt_type_list_push(&args, value != NULL ? zt_type_clone(value->type) : zt_unknown_type());
                zt_type_list_push(&args, zt_type_make(ZT_TYPE_TEXT));
                type = zt_type_make_with_args(ZT_TYPE_RESULT, "result", args);
            }
            out = zt_make_expr(ZT_HIR_SUCCESS_EXPR, expr->span, type);
            if (out != NULL) out->as.success_expr.value = value;
            return out;
        }

        case ZT_AST_ERROR_EXPR: {
            zt_hir_expr *value = zt_lower_expr(ctx, scope, expr->as.error_expr.value, zt_type_make(ZT_TYPE_TEXT));
            zt_type *type;
            if (expected != NULL && expected->kind == ZT_TYPE_RESULT) {
                type = zt_type_clone(expected);
            } else {
                zt_type_list args = zt_type_list_make();
                zt_type_list_push(&args, zt_type_make(ZT_TYPE_VOID));
                zt_type_list_push(&args, value != NULL ? zt_type_clone(value->type) : zt_type_make(ZT_TYPE_TEXT));
                type = zt_type_make_with_args(ZT_TYPE_RESULT, "result", args);
            }
            out = zt_make_expr(ZT_HIR_ERROR_EXPR, expr->span, type);
            if (out != NULL) out->as.error_expr.value = value;
            return out;
        }

        case ZT_AST_LIST_EXPR: {
            size_t i;
            zt_hir_expr_list elements = zt_hir_expr_list_make();
            const zt_type *inner_expected = NULL;
            zt_type *list_type;
            if (expected != NULL && expected->kind == ZT_TYPE_LIST && expected->args.count > 0) {
                inner_expected = expected->args.items[0];
            }
            for (i = 0; i < expr->as.list_expr.elements.count; i += 1) {
                zt_hir_expr_list_push(&elements, zt_lower_expr(ctx, scope, expr->as.list_expr.elements.items[i], inner_expected));
            }
            if (expected != NULL && expected->kind == ZT_TYPE_LIST) {
                list_type = zt_type_clone(expected);
            } else {
                zt_type_list args = zt_type_list_make();
                zt_type_list_push(&args, elements.count > 0 && elements.items[0] != NULL ? zt_type_clone(elements.items[0]->type) : zt_unknown_type());
                list_type = zt_type_make_with_args(ZT_TYPE_LIST, "list", args);
            }
            out = zt_make_expr(ZT_HIR_LIST_EXPR, expr->span, list_type);
            if (out == NULL) {
                zt_hir_expr_list_dispose(&elements);
                return NULL;
            }
            out->as.list_expr.elements = elements;
            return out;
        }

        case ZT_AST_MAP_EXPR: {
            size_t i;
            zt_hir_map_entry_list entries = zt_hir_map_entry_list_make();
            const zt_type *expected_key = NULL;
            const zt_type *expected_val = NULL;
            zt_type *map_type;
            if (expected != NULL && expected->kind == ZT_TYPE_MAP && expected->args.count >= 2) {
                expected_key = expected->args.items[0];
                expected_val = expected->args.items[1];
            }
            for (i = 0; i < expr->as.map_expr.entries.count; i += 1) {
                zt_hir_map_entry entry;
                memset(&entry, 0, sizeof(entry));
                entry.span = expr->as.map_expr.entries.items[i].span;
                entry.key = zt_lower_expr(ctx, scope, expr->as.map_expr.entries.items[i].key, expected_key);
                entry.value = zt_lower_expr(ctx, scope, expr->as.map_expr.entries.items[i].value, expected_val);
                zt_hir_map_entry_list_push(&entries, entry);
            }
            if (expected != NULL && expected->kind == ZT_TYPE_MAP) {
                map_type = zt_type_clone(expected);
            } else {
                zt_type_list args = zt_type_list_make();
                zt_type_list_push(&args, entries.count > 0 && entries.items[0].key != NULL ? zt_type_clone(entries.items[0].key->type) : zt_unknown_type());
                zt_type_list_push(&args, entries.count > 0 && entries.items[0].value != NULL ? zt_type_clone(entries.items[0].value->type) : zt_unknown_type());
                map_type = zt_type_make_with_args(ZT_TYPE_MAP, "map", args);
            }
            out = zt_make_expr(ZT_HIR_MAP_EXPR, expr->span, map_type);
            if (out == NULL) {
                zt_hir_map_entry_list_dispose(&entries);
                return NULL;
            }
            out->as.map_expr.entries = entries;
            return out;
        }

        case ZT_AST_UNARY_EXPR: {
            zt_hir_expr *operand = zt_lower_expr(ctx, scope, expr->as.unary_expr.operand, NULL);
            zt_type *type = zt_unknown_type();
            if (expr->as.unary_expr.op == ZT_TOKEN_QUESTION) {
                if (operand != NULL && operand->type != NULL &&
                        operand->type->kind == ZT_TYPE_RESULT &&
                        operand->type->args.count > 0) {
                    zt_type_dispose(type);
                    type = zt_type_clone(operand->type->args.items[0]);
                } else if (operand != NULL && operand->type != NULL &&
                        operand->type->kind == ZT_TYPE_OPTIONAL &&
                        operand->type->args.count > 0) {
                    zt_type_dispose(type);
                    type = zt_type_clone(operand->type->args.items[0]);
                }
            } else if (operand != NULL && operand->type != NULL) {
                zt_type_dispose(type);
                type = zt_type_clone(operand->type);
            }
            out = zt_make_expr(ZT_HIR_UNARY_EXPR, expr->span, type);
            if (out != NULL) {
                out->as.unary_expr.op = expr->as.unary_expr.op;
                out->as.unary_expr.operand = operand;
            } else {
                zt_hir_expr_dispose(operand);
            }
            return out;
        }

        case ZT_AST_BINARY_EXPR: {
            zt_hir_expr *left = zt_lower_expr(ctx, scope, expr->as.binary_expr.left, NULL);
            zt_hir_expr *right = zt_lower_expr(ctx, scope, expr->as.binary_expr.right, NULL);
            zt_type *type = zt_unknown_type();
            if (expr->as.binary_expr.op == ZT_TOKEN_EQEQ || expr->as.binary_expr.op == ZT_TOKEN_NEQ ||
                    expr->as.binary_expr.op == ZT_TOKEN_LT || expr->as.binary_expr.op == ZT_TOKEN_LTE ||
                    expr->as.binary_expr.op == ZT_TOKEN_GT || expr->as.binary_expr.op == ZT_TOKEN_GTE ||
                    expr->as.binary_expr.op == ZT_TOKEN_AND || expr->as.binary_expr.op == ZT_TOKEN_OR) {
                zt_type_dispose(type);
                type = zt_type_make(ZT_TYPE_BOOL);
            } else if (left != NULL && left->type != NULL) {
                zt_type_dispose(type);
                type = zt_type_clone(left->type);
            }
            out = zt_make_expr(ZT_HIR_BINARY_EXPR, expr->span, type);
            if (out != NULL) {
                out->as.binary_expr.op = expr->as.binary_expr.op;
                out->as.binary_expr.left = left;
                out->as.binary_expr.right = right;
            } else {
                zt_hir_expr_dispose(left);
                zt_hir_expr_dispose(right);
            }
            return out;
        }

        case ZT_AST_FIELD_EXPR: {
            if (expr->as.field_expr.object != NULL &&
                expr->as.field_expr.object->kind == ZT_AST_IDENT_EXPR &&
                expr->as.field_expr.field_name != NULL) {
                char qualified_name[512];
                zt_const_meta *const_meta;
                zt_var_meta *var_meta;
                snprintf(qualified_name, sizeof(qualified_name), "%s.%s", expr->as.field_expr.object->as.ident_expr.name, expr->as.field_expr.field_name);
                const_meta = zt_find_const_meta(ctx, qualified_name);
                if (const_meta != NULL && const_meta->init_value != NULL) {
                    if (const_meta->is_lowering) {
                        zt_add_diag(ctx, expr->span, "recursive const reference during lowering");
                    } else {
                        zt_hir_expr *inlined;
                        const_meta->is_lowering = 1;
                        inlined = zt_lower_expr(ctx, scope, const_meta->init_value, const_meta->type);
                        const_meta->is_lowering = 0;
                        if (inlined != NULL) return inlined;
                    }
                }
                var_meta = zt_find_var_meta(ctx, qualified_name);
                if (var_meta != NULL) {
                    zt_type *type = zt_type_clone(var_meta->type);
                    if (type == NULL) type = expected != NULL ? zt_type_clone(expected) : zt_unknown_type();
                    out = zt_make_expr(ZT_HIR_IDENT_EXPR, expr->span, type);
                    if (out != NULL) {
                        out->as.ident_expr.name = zt_lower_strdup(
                            var_meta->lowered_name != NULL ? var_meta->lowered_name : qualified_name);
                    }
                    return out;
                }
            }
            if (expr->as.field_expr.object != NULL && expr->as.field_expr.field_name != NULL) {
                char enum_name_path[512];
                if (zt_expr_to_path(expr->as.field_expr.object, enum_name_path, sizeof(enum_name_path))) {
                    size_t variant_index = 0;
                    const zt_ast_node *enum_decl = zt_find_enum_decl_ast(ctx, enum_name_path);
                    if (enum_decl != NULL) {
                        const zt_ast_node *variant_decl = zt_find_enum_variant_ast(enum_decl, expr->as.field_expr.field_name, &variant_index);
                        if (variant_decl != NULL) {
                            if (variant_decl->as.enum_variant.fields.count == 0) {
                                return zt_lower_enum_unit_variant(expr, enum_decl->as.enum_decl.name, variant_index);
                            }
                            zt_add_diag(ctx, expr->span, "enum variant with payload requires constructor call");
                        }
                    }
                }
            }
            zt_hir_expr *object = zt_lower_expr(ctx, scope, expr->as.field_expr.object, NULL);
            zt_type *field_type = zt_unknown_type();
            if (object != NULL && object->type != NULL && object->type->kind == ZT_TYPE_CORE_ERROR) {
                if (zt_text_eq(expr->as.field_expr.field_name, "context")) {
                    zt_type_list args = zt_type_list_make();
                    zt_type_dispose(field_type);
                    zt_type_list_push(&args, zt_type_make(ZT_TYPE_TEXT));
                    field_type = zt_type_make_with_args(ZT_TYPE_OPTIONAL, NULL, args);
                } else if (zt_text_eq(expr->as.field_expr.field_name, "code") ||
                           zt_text_eq(expr->as.field_expr.field_name, "message")) {
                    zt_type_dispose(field_type);
                    field_type = zt_type_make(ZT_TYPE_TEXT);
                }
            } else if (object != NULL && object->type != NULL && object->type->kind == ZT_TYPE_USER) {
                const zt_struct_meta *meta = zt_find_struct_meta(ctx, object->type->name);
                const zt_struct_field_meta *field = zt_find_struct_field_meta(meta, expr->as.field_expr.field_name);
                if (field != NULL && field->type != NULL) {
                    zt_type_dispose(field_type);
                    field_type = zt_type_clone(field->type);
                }
            }
            out = zt_make_expr(ZT_HIR_FIELD_EXPR, expr->span, field_type);
            if (out != NULL) {
                out->as.field_expr.object = object;
                out->as.field_expr.field_name = zt_lower_strdup(expr->as.field_expr.field_name);
            } else {
                zt_hir_expr_dispose(object);
            }
            return out;
        }

        case ZT_AST_INDEX_EXPR: {
            zt_hir_expr *object = zt_lower_expr(ctx, scope, expr->as.index_expr.object, NULL);
            zt_hir_expr *index = zt_lower_expr(ctx, scope, expr->as.index_expr.index, NULL);
            zt_type *item_type = zt_unknown_type();
            if (object != NULL && object->type != NULL) {
                if (object->type->kind == ZT_TYPE_LIST && object->type->args.count > 0) {
                    zt_type_dispose(item_type);
                    item_type = zt_type_clone(object->type->args.items[0]);
                } else if (object->type->kind == ZT_TYPE_MAP && object->type->args.count > 1) {
                    zt_type_dispose(item_type);
                    item_type = zt_type_clone(object->type->args.items[1]);
                } else if (object->type->kind == ZT_TYPE_TEXT) {
                    zt_type_dispose(item_type);
                    item_type = zt_type_make(ZT_TYPE_TEXT);
                } else if (object->type->kind == ZT_TYPE_BYTES) {
                    zt_type_dispose(item_type);
                    item_type = zt_type_make(ZT_TYPE_UINT8);
                }
            }
            out = zt_make_expr(ZT_HIR_INDEX_EXPR, expr->span, item_type);
            if (out != NULL) {
                out->as.index_expr.object = object;
                out->as.index_expr.index = index;
            } else {
                zt_hir_expr_dispose(object);
                zt_hir_expr_dispose(index);
            }
            return out;
        }

        case ZT_AST_SLICE_EXPR: {
            zt_hir_expr *object = zt_lower_expr(ctx, scope, expr->as.slice_expr.object, NULL);
            zt_hir_expr *start = zt_lower_expr(ctx, scope, expr->as.slice_expr.start, NULL);
            zt_hir_expr *end = zt_lower_expr(ctx, scope, expr->as.slice_expr.end, NULL);
            zt_type *slice_type = object != NULL && object->type != NULL ? zt_type_clone(object->type) : zt_unknown_type();
            out = zt_make_expr(ZT_HIR_SLICE_EXPR, expr->span, slice_type);
            if (out != NULL) {
                out->as.slice_expr.object = object;
                out->as.slice_expr.start = start;
                out->as.slice_expr.end = end;
            } else {
                zt_hir_expr_dispose(object);
                zt_hir_expr_dispose(start);
                zt_hir_expr_dispose(end);
            }
            return out;
        }

        case ZT_AST_CALL_EXPR:
            return zt_lower_call_expr(ctx, scope, expr, expected);

        case ZT_AST_GROUPED_EXPR:
            return zt_lower_expr(ctx, scope, expr->as.grouped_expr.inner, expected);

        case ZT_AST_VALUE_BINDING: {
            zt_type *binding_type = expected != NULL && expected->kind == ZT_TYPE_OPTIONAL && expected->args.count > 0
                ? zt_type_clone(expected->args.items[0])
                : zt_unknown_type();
            out = zt_make_expr(ZT_HIR_VALUE_BINDING_EXPR, expr->span, binding_type);
            if (out != NULL) out->as.value_binding_expr.name = zt_lower_strdup(expr->as.value_binding.name);
            return out;
        }

        default:
            zt_add_diag(ctx, expr->span, "unsupported expression during AST->HIR lowering");
            out = zt_make_expr(ZT_HIR_IDENT_EXPR, expr->span, zt_unknown_type());
            if (out != NULL) out->as.ident_expr.name = zt_lower_strdup("<unsupported>");
            return out;
    }
}

static zt_hir_stmt *zt_lower_stmt(zt_lower_ctx *ctx, zt_scope *scope, const zt_ast_node *stmt);

static zt_hir_stmt *zt_lower_block(zt_lower_ctx *ctx, zt_scope *parent, const zt_ast_node *block) {
    zt_hir_stmt *out;
    zt_scope nested;
    size_t i;
    if (block == NULL || block->kind != ZT_AST_BLOCK) return NULL;
    out = zt_hir_stmt_make(ZT_HIR_BLOCK_STMT, block->span);
    if (out == NULL) return NULL;
    out->as.block_stmt.statements = zt_hir_stmt_list_make();
    zt_scope_init(&nested, parent);
    for (i = 0; i < block->as.block.statements.count; i += 1) {
        zt_hir_stmt_list_push(&out->as.block_stmt.statements, zt_lower_stmt(ctx, &nested, block->as.block.statements.items[i]));
    }
    zt_scope_dispose(&nested);
    return out;
}

static zt_hir_stmt *zt_lower_stmt(zt_lower_ctx *ctx, zt_scope *scope, const zt_ast_node *stmt) {
    if (stmt == NULL) return NULL;

    switch (stmt->kind) {
        case ZT_AST_BLOCK:
            return zt_lower_block(ctx, scope, stmt);

        case ZT_AST_IF_STMT: {
            zt_hir_stmt *out = zt_hir_stmt_make(ZT_HIR_IF_STMT, stmt->span);
            if (out == NULL) return NULL;
            out->as.if_stmt.condition = zt_lower_expr(ctx, scope, stmt->as.if_stmt.condition, zt_type_make(ZT_TYPE_BOOL));
            out->as.if_stmt.then_block = zt_lower_stmt(ctx, scope, stmt->as.if_stmt.then_block);
            out->as.if_stmt.else_block = zt_lower_stmt(ctx, scope, stmt->as.if_stmt.else_block);
            return out;
        }

        case ZT_AST_WHILE_STMT: {
            zt_hir_stmt *out = zt_hir_stmt_make(ZT_HIR_WHILE_STMT, stmt->span);
            if (out == NULL) return NULL;
            out->as.while_stmt.condition = zt_lower_expr(ctx, scope, stmt->as.while_stmt.condition, zt_type_make(ZT_TYPE_BOOL));
            out->as.while_stmt.body = zt_lower_stmt(ctx, scope, stmt->as.while_stmt.body);
            return out;
        }

        case ZT_AST_FOR_STMT: {
            zt_hir_stmt *out = zt_hir_stmt_make(ZT_HIR_FOR_STMT, stmt->span);
            zt_scope loop_scope;
            zt_type *item_type;
            zt_type *second_type;
            if (out == NULL) return NULL;
            out->as.for_stmt.iterable = zt_lower_expr(ctx, scope, stmt->as.for_stmt.iterable, NULL);
            out->as.for_stmt.item_name = zt_lower_strdup(stmt->as.for_stmt.item_name);
            out->as.for_stmt.index_name = zt_lower_strdup(stmt->as.for_stmt.index_name);

            zt_scope_init(&loop_scope, scope);
            item_type = zt_unknown_type();
            second_type = zt_type_make(ZT_TYPE_INT);

            if (out->as.for_stmt.iterable != NULL && out->as.for_stmt.iterable->type != NULL) {
                const zt_type *iter_type = out->as.for_stmt.iterable->type;
                if (iter_type->kind == ZT_TYPE_LIST && iter_type->args.count > 0) {
                    zt_type_dispose(item_type);
                    item_type = zt_type_clone(iter_type->args.items[0]);
                } else if (iter_type->kind == ZT_TYPE_TEXT) {
                    zt_type_dispose(item_type);
                    item_type = zt_type_make(ZT_TYPE_TEXT);
                } else if (iter_type->kind == ZT_TYPE_MAP && iter_type->args.count > 1) {
                    zt_type_dispose(item_type);
                    zt_type_dispose(second_type);
                    item_type = zt_type_clone(iter_type->args.items[0]);
                    second_type = zt_type_clone(iter_type->args.items[1]);
                }
            }

            zt_scope_set(&loop_scope, stmt->as.for_stmt.item_name, item_type, 1);
            if (stmt->as.for_stmt.index_name != NULL) {
                zt_scope_set(&loop_scope, stmt->as.for_stmt.index_name, second_type, 1);
            }

            zt_type_dispose(item_type);
            zt_type_dispose(second_type);

            out->as.for_stmt.body = zt_lower_stmt(ctx, &loop_scope, stmt->as.for_stmt.body);
            zt_scope_dispose(&loop_scope);
            return out;
        }

        case ZT_AST_REPEAT_STMT: {
            zt_hir_stmt *out = zt_hir_stmt_make(ZT_HIR_REPEAT_STMT, stmt->span);
            if (out == NULL) return NULL;
            out->as.repeat_stmt.count = zt_lower_expr(ctx, scope, stmt->as.repeat_stmt.count, zt_type_make(ZT_TYPE_INT));
            out->as.repeat_stmt.body = zt_lower_stmt(ctx, scope, stmt->as.repeat_stmt.body);
            return out;
        }

        case ZT_AST_RETURN_STMT: {
            zt_hir_stmt *out = zt_hir_stmt_make(ZT_HIR_RETURN_STMT, stmt->span);
            if (out == NULL) return NULL;
            out->as.return_stmt.value = zt_lower_expr(ctx, scope, stmt->as.return_stmt.value, ctx->current_return_type);
            return out;
        }

        case ZT_AST_CONST_DECL: {
            zt_hir_stmt *out = zt_hir_stmt_make(ZT_HIR_CONST_STMT, stmt->span);
            zt_type *declared_type = zt_lower_type_from_ast(ctx, stmt->as.const_decl.type_node);
            if (out == NULL) {
                zt_type_dispose(declared_type);
                return NULL;
            }
            out->as.const_stmt.name = zt_lower_strdup(stmt->as.const_decl.name);
            out->as.const_stmt.type = zt_type_clone(declared_type);
            out->as.const_stmt.init_value = zt_lower_expr(ctx, scope, stmt->as.const_decl.init_value, declared_type);
            zt_scope_set(scope, stmt->as.const_decl.name, declared_type, 0);
            zt_type_dispose(declared_type);
            return out;
        }

        case ZT_AST_VAR_DECL: {
            zt_hir_stmt *out = zt_hir_stmt_make(ZT_HIR_VAR_STMT, stmt->span);
            zt_type *declared_type = zt_lower_type_from_ast(ctx, stmt->as.var_decl.type_node);
            if (out == NULL) {
                zt_type_dispose(declared_type);
                return NULL;
            }
            out->as.var_stmt.name = zt_lower_strdup(stmt->as.var_decl.name);
            out->as.var_stmt.type = zt_type_clone(declared_type);
            out->as.var_stmt.init_value = zt_lower_expr(ctx, scope, stmt->as.var_decl.init_value, declared_type);
            zt_scope_set(scope, stmt->as.var_decl.name, declared_type, 1);
            zt_type_dispose(declared_type);
            return out;
        }

        case ZT_AST_ASSIGN_STMT: {
            zt_hir_stmt *out = zt_hir_stmt_make(ZT_HIR_ASSIGN_STMT, stmt->span);
            const zt_type *assigned_type = zt_scope_get(scope, stmt->as.assign_stmt.name);
            const char *target_name = stmt->as.assign_stmt.name;
            if (out == NULL) return NULL;
            if (assigned_type == NULL) {
                zt_var_meta *var_meta = zt_find_var_meta(ctx, stmt->as.assign_stmt.name);
                if (var_meta != NULL && var_meta->type != NULL) {
                    target_name = var_meta->lowered_name != NULL ? var_meta->lowered_name : stmt->as.assign_stmt.name;
                    zt_scope_set(scope, target_name, var_meta->type, 1);
                    assigned_type = zt_scope_get(scope, target_name);
                }
            }
            out->as.assign_stmt.name = zt_lower_strdup(target_name);
            out->as.assign_stmt.value = zt_lower_expr(ctx, scope, stmt->as.assign_stmt.value, assigned_type);
            return out;
        }

        case ZT_AST_INDEX_ASSIGN_STMT: {
            zt_hir_stmt *out = zt_hir_stmt_make(ZT_HIR_INDEX_ASSIGN_STMT, stmt->span);
            zt_type *value_expected = NULL;
            if (out == NULL) return NULL;
            if (stmt->as.index_assign_stmt.object != NULL &&
                stmt->as.index_assign_stmt.object->kind == ZT_AST_IDENT_EXPR &&
                stmt->as.index_assign_stmt.object->as.ident_expr.name != NULL &&
                zt_scope_get(scope, stmt->as.index_assign_stmt.object->as.ident_expr.name) == NULL) {
                zt_var_meta *var_meta = zt_find_var_meta(ctx, stmt->as.index_assign_stmt.object->as.ident_expr.name);
                if (var_meta != NULL && var_meta->type != NULL) {
                    zt_scope_set(scope, stmt->as.index_assign_stmt.object->as.ident_expr.name, var_meta->type, 1);
                }
            }
            out->as.index_assign_stmt.object = zt_lower_expr(ctx, scope, stmt->as.index_assign_stmt.object, NULL);
            if (out->as.index_assign_stmt.object != NULL && out->as.index_assign_stmt.object->type != NULL) {
                const zt_type *t = out->as.index_assign_stmt.object->type;
                if (t->kind == ZT_TYPE_LIST && t->args.count > 0) value_expected = zt_type_clone(t->args.items[0]);
                else if (t->kind == ZT_TYPE_MAP && t->args.count > 1) value_expected = zt_type_clone(t->args.items[1]);
                else if (t->kind == ZT_TYPE_BYTES) value_expected = zt_type_make(ZT_TYPE_UINT8);
            }
            out->as.index_assign_stmt.index = zt_lower_expr(ctx, scope, stmt->as.index_assign_stmt.index, NULL);
            out->as.index_assign_stmt.value = zt_lower_expr(ctx, scope, stmt->as.index_assign_stmt.value, value_expected);
            zt_type_dispose(value_expected);
            return out;
        }

        case ZT_AST_FIELD_ASSIGN_STMT: {
            zt_hir_stmt *out = zt_hir_stmt_make(ZT_HIR_FIELD_ASSIGN_STMT, stmt->span);
            zt_type *field_type = NULL;
            if (out == NULL) return NULL;
            if (stmt->as.field_assign_stmt.object != NULL &&
                stmt->as.field_assign_stmt.object->kind == ZT_AST_IDENT_EXPR &&
                stmt->as.field_assign_stmt.object->as.ident_expr.name != NULL &&
                zt_scope_get(scope, stmt->as.field_assign_stmt.object->as.ident_expr.name) == NULL) {
                zt_var_meta *var_meta = zt_find_var_meta(ctx, stmt->as.field_assign_stmt.object->as.ident_expr.name);
                if (var_meta != NULL && var_meta->type != NULL) {
                    zt_scope_set(scope, stmt->as.field_assign_stmt.object->as.ident_expr.name, var_meta->type, 1);
                }
            }
            out->as.field_assign_stmt.object = zt_lower_expr(ctx, scope, stmt->as.field_assign_stmt.object, NULL);
            if (out->as.field_assign_stmt.object != NULL && out->as.field_assign_stmt.object->type != NULL &&
                    out->as.field_assign_stmt.object->type->kind == ZT_TYPE_USER) {
                const zt_struct_meta *meta = zt_find_struct_meta(ctx, out->as.field_assign_stmt.object->type->name);
                const zt_struct_field_meta *field = zt_find_struct_field_meta(meta, stmt->as.field_assign_stmt.field_name);
                if (field != NULL) field_type = zt_type_clone(field->type);
            }
            out->as.field_assign_stmt.field_name = zt_lower_strdup(stmt->as.field_assign_stmt.field_name);
            out->as.field_assign_stmt.value = zt_lower_expr(ctx, scope, stmt->as.field_assign_stmt.value, field_type);
            zt_type_dispose(field_type);
            return out;
        }

        case ZT_AST_MATCH_STMT: {
            zt_hir_stmt *out = zt_hir_stmt_make(ZT_HIR_MATCH_STMT, stmt->span);
            zt_type *subject_type;
            size_t i;
            if (out == NULL) return NULL;
            out->as.match_stmt.subject = zt_lower_expr(ctx, scope, stmt->as.match_stmt.subject, NULL);
            subject_type = out->as.match_stmt.subject != NULL ? zt_type_clone(out->as.match_stmt.subject->type) : zt_unknown_type();
            out->as.match_stmt.cases = zt_hir_match_case_list_make();

            for (i = 0; i < stmt->as.match_stmt.cases.count; i += 1) {
                const zt_ast_node *case_node = stmt->as.match_stmt.cases.items[i];
                zt_hir_match_case c;
                size_t p;
                if (case_node == NULL || case_node->kind != ZT_AST_MATCH_CASE) continue;
                memset(&c, 0, sizeof(c));
                c.span = case_node->span;
                c.is_default = case_node->as.match_case.is_default;
                c.patterns = zt_hir_expr_list_make();
                for (p = 0; p < case_node->as.match_case.patterns.count; p += 1) {
                    zt_hir_expr_list_push(&c.patterns, zt_lower_expr(ctx, scope, case_node->as.match_case.patterns.items[p], subject_type));
                }
                c.body = zt_lower_stmt(ctx, scope, case_node->as.match_case.body);
                zt_hir_match_case_list_push(&out->as.match_stmt.cases, c);
            }

            zt_type_dispose(subject_type);
            return out;
        }

        case ZT_AST_BREAK_STMT:
            return zt_hir_stmt_make(ZT_HIR_BREAK_STMT, stmt->span);

        case ZT_AST_CONTINUE_STMT:
            return zt_hir_stmt_make(ZT_HIR_CONTINUE_STMT, stmt->span);

        case ZT_AST_EXPR_STMT: {
            zt_hir_stmt *out = zt_hir_stmt_make(ZT_HIR_EXPR_STMT, stmt->span);
            if (out == NULL) return NULL;
            out->as.expr_stmt.expr = zt_lower_expr(ctx, scope, stmt->as.expr_stmt.expr, NULL);
            return out;
        }

        default:
            zt_add_diag(ctx, stmt->span, "unsupported statement during AST->HIR lowering");
            return zt_hir_stmt_make(ZT_HIR_EXPR_STMT, stmt->span);
    }
}

static zt_hir_decl *zt_lower_struct_decl(zt_lower_ctx *ctx, const zt_ast_node *decl) {
    zt_hir_decl *out;
    size_t i;
    if (decl == NULL || decl->kind != ZT_AST_STRUCT_DECL) return NULL;
    out = zt_hir_decl_make(ZT_HIR_STRUCT_DECL, decl->span);
    if (out == NULL) return NULL;

    out->as.struct_decl.name = zt_lower_strdup(decl->as.struct_decl.name);
    out->as.struct_decl.is_public = decl->as.struct_decl.is_public;
    out->as.struct_decl.type_params = zt_hir_string_list_make();
    out->as.struct_decl.fields = zt_hir_field_decl_list_make();

    for (i = 0; i < decl->as.struct_decl.type_params.count; i += 1) {
        const zt_ast_node *tp = decl->as.struct_decl.type_params.items[i];
        const char *tp_name = NULL;
        if (tp == NULL) continue;
        if (tp->kind == ZT_AST_TYPE_SIMPLE) tp_name = tp->as.type_simple.name;
        if (tp->kind == ZT_AST_TYPE_GENERIC) tp_name = tp->as.type_generic.name;
        if (tp_name != NULL) zt_hir_string_list_push(&out->as.struct_decl.type_params, zt_lower_strdup(tp_name));
    }

    for (i = 0; i < decl->as.struct_decl.fields.count; i += 1) {
        const zt_ast_node *field = decl->as.struct_decl.fields.items[i];
        zt_hir_field_decl hfield;
        zt_type *bool_type;
        if (field == NULL || field->kind != ZT_AST_STRUCT_FIELD) continue;
        memset(&hfield, 0, sizeof(hfield));
        hfield.name = zt_lower_strdup(field->as.struct_field.name);
        hfield.type = zt_lower_type_from_ast(ctx, field->as.struct_field.type_node);
        hfield.default_value = field->as.struct_field.default_value != NULL
            ? zt_lower_expr(ctx, NULL, field->as.struct_field.default_value, hfield.type)
            : NULL;
        
        bool_type = zt_type_make(ZT_TYPE_BOOL);
        hfield.where_clause = (field->as.struct_field.where_clause != NULL && field->as.struct_field.where_clause->kind == ZT_AST_WHERE_CLAUSE)
            ? zt_lower_expr(ctx, NULL, field->as.struct_field.where_clause->as.where_clause.condition, bool_type)
            : NULL;
        zt_type_dispose(bool_type);
        
        zt_hir_field_decl_list_push(&out->as.struct_decl.fields, hfield);
    }

    return out;
}

static zt_hir_decl *zt_lower_enum_decl(zt_lower_ctx *ctx, const zt_ast_node *decl) {
    zt_hir_decl *out;
    size_t i;
    if (decl == NULL || decl->kind != ZT_AST_ENUM_DECL) return NULL;
    out = zt_hir_decl_make(ZT_HIR_ENUM_DECL, decl->span);
    if (out == NULL) return NULL;

    out->as.enum_decl.name = zt_lower_strdup(decl->as.enum_decl.name);
    out->as.enum_decl.is_public = decl->as.enum_decl.is_public;
    out->as.enum_decl.type_params = zt_hir_string_list_make();
    out->as.enum_decl.variants = zt_hir_enum_variant_list_make();

    for (i = 0; i < decl->as.enum_decl.type_params.count; i += 1) {
        const zt_ast_node *tp = decl->as.enum_decl.type_params.items[i];
        const char *tp_name = NULL;
        if (tp == NULL) continue;
        if (tp->kind == ZT_AST_TYPE_SIMPLE) tp_name = tp->as.type_simple.name;
        if (tp->kind == ZT_AST_TYPE_GENERIC) tp_name = tp->as.type_generic.name;
        if (tp_name != NULL) zt_hir_string_list_push(&out->as.enum_decl.type_params, zt_lower_strdup(tp_name));
    }

    for (i = 0; i < decl->as.enum_decl.variants.count; i += 1) {
        const zt_ast_node *variant = decl->as.enum_decl.variants.items[i];
        zt_hir_enum_variant lowered_variant;
        size_t f;

        if (variant == NULL || variant->kind != ZT_AST_ENUM_VARIANT) continue;

        memset(&lowered_variant, 0, sizeof(lowered_variant));
        lowered_variant.name = zt_lower_strdup(variant->as.enum_variant.name);
        lowered_variant.fields = zt_hir_enum_variant_field_list_make();

        for (f = 0; f < variant->as.enum_variant.fields.count; f += 1) {
            const zt_ast_node *field = variant->as.enum_variant.fields.items[f];
            zt_hir_enum_variant_field lowered_field;

            memset(&lowered_field, 0, sizeof(lowered_field));
            if (field == NULL) {
                lowered_field.type = zt_unknown_type();
            } else if (field->kind == ZT_AST_PARAM) {
                lowered_field.name = zt_lower_strdup(field->as.param.name);
                lowered_field.type = zt_lower_type_from_ast(ctx, field->as.param.type_node);
            } else if (field->kind == ZT_AST_STRUCT_FIELD) {
                lowered_field.name = zt_lower_strdup(field->as.struct_field.name);
                lowered_field.type = zt_lower_type_from_ast(ctx, field->as.struct_field.type_node);
            } else {
                lowered_field.type = zt_lower_type_from_ast(ctx, field);
            }

            zt_hir_enum_variant_field_list_push(&lowered_variant.fields, lowered_field);
        }

        zt_hir_enum_variant_list_push(&out->as.enum_decl.variants, lowered_variant);
    }

    return out;
}

static zt_hir_decl *zt_lower_function_decl_core(
        zt_lower_ctx *ctx,
        const zt_ast_node *func,
        const char *lowered_name,
        const char *receiver_type,
        const char *trait_name) {
    zt_hir_decl *out;
    zt_scope fn_scope;
    const zt_type *saved_return;
    size_t i;

    if (func == NULL || func->kind != ZT_AST_FUNC_DECL || lowered_name == NULL) return NULL;
    out = zt_hir_decl_make(ZT_HIR_FUNC_DECL, func->span);
    if (out == NULL) return NULL;

    out->as.func_decl.name = zt_lower_strdup(lowered_name);
    out->as.func_decl.is_public = func->as.func_decl.is_public;
    out->as.func_decl.is_mutating = func->as.func_decl.is_mutating;
    out->as.func_decl.receiver_type_name = zt_lower_strdup(receiver_type);
    out->as.func_decl.implemented_trait_name = zt_lower_strdup(trait_name);
    out->as.func_decl.type_params = zt_hir_string_list_make();
    out->as.func_decl.params = zt_hir_param_list_make();
    out->as.func_decl.return_type = func->as.func_decl.return_type != NULL
        ? zt_lower_type_from_ast(ctx, func->as.func_decl.return_type)
        : zt_type_make(ZT_TYPE_VOID);

    for (i = 0; i < func->as.func_decl.type_params.count; i += 1) {
        const zt_ast_node *tp = func->as.func_decl.type_params.items[i];
        const char *tp_name = NULL;
        if (tp == NULL) continue;
        if (tp->kind == ZT_AST_TYPE_SIMPLE) tp_name = tp->as.type_simple.name;
        if (tp->kind == ZT_AST_TYPE_GENERIC) tp_name = tp->as.type_generic.name;
        if (tp_name != NULL) zt_hir_string_list_push(&out->as.func_decl.type_params, zt_lower_strdup(tp_name));
    }

    zt_scope_init(&fn_scope, NULL);
    if (receiver_type != NULL) {
        zt_type *self_type = zt_type_make_named(ZT_TYPE_USER, receiver_type);
        zt_scope_set(&fn_scope, "self", self_type, func->as.func_decl.is_mutating);
        zt_type_dispose(self_type);
    }

    for (i = 0; i < func->as.func_decl.params.count; i += 1) {
        const zt_ast_node *param = func->as.func_decl.params.items[i];
        zt_hir_param hparam;
        zt_type *pt;
        zt_type *bool_type;
        if (param == NULL || param->kind != ZT_AST_PARAM) continue;
        pt = zt_lower_type_from_ast(ctx, param->as.param.type_node);
        memset(&hparam, 0, sizeof(hparam));
        hparam.name = zt_lower_strdup(param->as.param.name);
        hparam.type = zt_type_clone(pt);
        
        zt_scope_set(&fn_scope, param->as.param.name, pt, 0);

        bool_type = zt_type_make(ZT_TYPE_BOOL);
        if (param->as.param.where_clause != NULL) {
            if (param->as.param.where_clause->kind == ZT_AST_WHERE_CLAUSE) {
                hparam.where_clause = zt_lower_expr(ctx, &fn_scope, param->as.param.where_clause->as.where_clause.condition, bool_type);
            } else {
                hparam.where_clause = zt_lower_expr(ctx, &fn_scope, param->as.param.where_clause, bool_type);
            }
        } else {
            hparam.where_clause = NULL;
        }
        zt_type_dispose(bool_type);
        
        zt_hir_param_list_push(&out->as.func_decl.params, hparam);
        zt_type_dispose(pt);
    }

    saved_return = ctx->current_return_type;
    ctx->current_return_type = out->as.func_decl.return_type;
    out->as.func_decl.body = zt_lower_stmt(ctx, &fn_scope, func->as.func_decl.body);
    ctx->current_return_type = saved_return;

    zt_scope_dispose(&fn_scope);
    return out;
}

static void zt_lower_apply_decl(zt_lower_ctx *ctx, zt_hir_module *module, const zt_ast_node *decl) {
    size_t i;
    if (decl == NULL || decl->kind != ZT_AST_APPLY_DECL) return;
    for (i = 0; i < decl->as.apply_decl.methods.count; i += 1) {
        const zt_ast_node *method = decl->as.apply_decl.methods.items[i];
        zt_hir_decl *fn;
        char *lowered_name;
        if (method == NULL || method->kind != ZT_AST_FUNC_DECL) continue;
        lowered_name = zt_build_apply_name(
            decl->as.apply_decl.target_name,
            decl->as.apply_decl.trait_name,
            method->as.func_decl.name);
        fn = zt_lower_function_decl_core(
            ctx,
            method,
            lowered_name,
            decl->as.apply_decl.target_name,
            decl->as.apply_decl.trait_name);
        zt_hir_decl_list_push(&module->declarations, fn);
        free(lowered_name);
    }
}

static void zt_lower_module_vars(zt_lower_ctx *ctx, zt_hir_module *module) {
    size_t i;
    zt_scope module_scope;

    if (ctx == NULL || module == NULL) return;
    zt_scope_init(&module_scope, NULL);

    for (i = 0; i < ctx->var_count; i += 1) {
        const zt_var_meta *meta = &ctx->vars[i];
        zt_hir_module_var hir_var;
        memset(&hir_var, 0, sizeof(hir_var));
        hir_var.span = meta->init_value != NULL ? meta->init_value->span : module->span;
        hir_var.name = zt_lower_strdup(meta->lowered_name != NULL ? meta->lowered_name : meta->name);
        hir_var.type = zt_type_clone(meta->type);
        hir_var.init_value = meta->init_value != NULL
            ? zt_lower_expr(ctx, &module_scope, meta->init_value, meta->type)
            : NULL;
        zt_hir_module_var_list_push(&module->module_vars, hir_var);
    }

    zt_scope_dispose(&module_scope);
}

zt_hir_lower_result zt_lower_ast_to_hir(const zt_ast_node *root) {
    zt_lower_ctx ctx;
    size_t i;
    memset(&ctx, 0, sizeof(ctx));
    ctx.result.diagnostics = zt_diag_list_make();
    ctx.root_ast = root;

    if (root == NULL || root->kind != ZT_AST_FILE) {
        zt_diag_list_add(
            &ctx.result.diagnostics,
            ZT_DIAG_INVALID_OPERATOR,
            root != NULL ? root->span : zt_source_span_make("<hir>", 1, 1, 1),
            "AST root must be a file node");
        return ctx.result;
    }

    zt_collect_symbols(&ctx, root);

    ctx.result.module = zt_hir_module_make(root->span);
    if (ctx.result.module == NULL) {
        zt_diag_list_add(
            &ctx.result.diagnostics,
            ZT_DIAG_INVALID_OPERATOR,
            root->span,
            "out of memory creating HIR module");
        return ctx.result;
    }

    ctx.result.module->module_name = zt_lower_strdup(root->as.file.module_name != NULL ? root->as.file.module_name : "main");
    ctx.result.module->module_vars = zt_hir_module_var_list_make();
    ctx.result.module->declarations = zt_hir_decl_list_make();

    zt_lower_module_vars(&ctx, ctx.result.module);

    for (i = 0; i < root->as.file.declarations.count; i += 1) {
        const zt_ast_node *decl = root->as.file.declarations.items[i];
        if (decl == NULL) continue;
        if (decl->kind == ZT_AST_STRUCT_DECL) {
            zt_hir_decl_list_push(&ctx.result.module->declarations, zt_lower_struct_decl(&ctx, decl));
        } else if (decl->kind == ZT_AST_ENUM_DECL) {
            zt_hir_decl_list_push(&ctx.result.module->declarations, zt_lower_enum_decl(&ctx, decl));
        } else if (decl->kind == ZT_AST_FUNC_DECL) {
            zt_hir_decl_list_push(&ctx.result.module->declarations, zt_lower_function_decl_core(&ctx, decl, decl->as.func_decl.name, NULL, NULL));
        } else if (decl->kind == ZT_AST_APPLY_DECL) {
            zt_lower_apply_decl(&ctx, ctx.result.module, decl);
        }
    }

    for (i = 0; i < ctx.struct_count; i += 1) {
        size_t f;
        free(ctx.structs[i].name);
        for (f = 0; f < ctx.structs[i].field_count; f += 1) {
            free(ctx.structs[i].fields[f].name);
            zt_type_dispose(ctx.structs[i].fields[f].type);
        }
        free(ctx.structs[i].fields);
    }
    free(ctx.structs);

    for (i = 0; i < ctx.func_count; i += 1) {
        size_t p;
        free(ctx.funcs[i].name);
        free(ctx.funcs[i].extern_binding);
        free(ctx.funcs[i].extern_abi_name);
        zt_type_dispose(ctx.funcs[i].return_type);
        for (p = 0; p < ctx.funcs[i].param_count; p += 1) {
            free(ctx.funcs[i].params[p].name);
            zt_type_dispose(ctx.funcs[i].params[p].type);
        }
        free(ctx.funcs[i].params);
    }
    free(ctx.funcs);

    for (i = 0; i < ctx.method_count; i += 1) {
        free(ctx.methods[i].receiver_type);
        free(ctx.methods[i].method_name);
        free(ctx.methods[i].lowered_name);
        free(ctx.methods[i].trait_name);
    }
    free(ctx.methods);

    for (i = 0; i < ctx.const_count; i += 1) {
        free(ctx.consts[i].name);
        zt_type_dispose(ctx.consts[i].type);
    }
    free(ctx.consts);

    for (i = 0; i < ctx.var_count; i += 1) {
        free(ctx.vars[i].name);
        free(ctx.vars[i].lowered_name);
        zt_type_dispose(ctx.vars[i].type);
    }
    free(ctx.vars);

    return ctx.result;
}

void zt_hir_lower_result_dispose(zt_hir_lower_result *result) {
    if (result == NULL) return;
    zt_hir_module_dispose(result->module);
    result->module = NULL;
    zt_diag_list_dispose(&result->diagnostics);
}






