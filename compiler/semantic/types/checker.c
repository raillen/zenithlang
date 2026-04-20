#include "compiler/semantic/types/checker.h"

#include "compiler/semantic/types/types.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum zt_binding_kind {
    ZT_BINDING_VALUE,
    ZT_BINDING_TYPE_PARAM
} zt_binding_kind;

typedef struct zt_binding {
    zt_binding_kind kind;
    char *name;
    zt_type *type;
    int is_mutable;
} zt_binding;

typedef struct zt_binding_scope {
    struct zt_binding_scope *parent;
    zt_binding *items;
    size_t count;
    size_t capacity;
} zt_binding_scope;

typedef struct zt_decl_entry {
    char *name;
    const zt_ast_node *node;
} zt_decl_entry;

typedef struct zt_import_entry {
    char *local_name;
    const zt_ast_node *node;
} zt_import_entry;

typedef struct zt_decl_list {
    zt_decl_entry *items;
    size_t count;
    size_t capacity;
} zt_decl_list;

typedef struct zt_import_list {
    zt_import_entry *items;
    size_t count;
    size_t capacity;
} zt_import_list;

typedef struct zt_apply_list {
    const zt_ast_node **items;
    size_t count;
    size_t capacity;
} zt_apply_list;

typedef struct zt_module_catalog {
    zt_decl_list decls;
    zt_import_list imports;
    zt_apply_list applies;
} zt_module_catalog;

typedef struct zt_checker {
    const zt_ast_node *root;
    zt_check_result *result;
    zt_module_catalog catalog;
} zt_checker;

typedef struct zt_expr_info {
    zt_type *type;
    int is_int_literal;
    int is_float_literal;
    int has_int_value;
    long long int_value;
    const char *literal_text;
} zt_expr_info;

typedef struct zt_function_context {
    zt_type *return_type;
    zt_type *self_type;
    int in_mutating_method;
    zt_binding_scope *scope;
} zt_function_context;

static char *zt_checker_strdup(const char *text) {
    size_t len;
    char *copy;

    if (text == NULL) return NULL;
    len = strlen(text);
    copy = (char *)malloc(len + 1);
    if (copy == NULL) return NULL;
    memcpy(copy, text, len + 1);
    return copy;
}

static void zt_expr_info_dispose(zt_expr_info *info) {
    if (info == NULL) return;
    zt_type_dispose(info->type);
    info->type = NULL;
    info->is_int_literal = 0;
    info->is_float_literal = 0;
    info->has_int_value = 0;
    info->int_value = 0;
    info->literal_text = NULL;
}

static zt_expr_info zt_expr_info_make(zt_type *type) {
    zt_expr_info info;
    info.type = type;
    info.is_int_literal = 0;
    info.is_float_literal = 0;
    info.has_int_value = 0;
    info.int_value = 0;
    info.literal_text = NULL;
    return info;
}

static void zt_binding_scope_init(zt_binding_scope *scope, zt_binding_scope *parent) {
    if (scope == NULL) return;
    scope->parent = parent;
    scope->items = NULL;
    scope->count = 0;
    scope->capacity = 0;
}

static void zt_binding_scope_dispose(zt_binding_scope *scope) {
    size_t i;

    if (scope == NULL) return;
    for (i = 0; i < scope->count; i++) {
        free(scope->items[i].name);
        zt_type_dispose(scope->items[i].type);
    }
    free(scope->items);
    scope->items = NULL;
    scope->count = 0;
    scope->capacity = 0;
    scope->parent = NULL;
}

static void zt_binding_scope_declare(zt_binding_scope *scope, zt_binding_kind kind, const char *name, zt_type *type, int is_mutable) {
    zt_binding *entry;
    zt_binding *new_items;
    size_t new_capacity;

    if (scope == NULL || name == NULL || type == NULL) {
        zt_type_dispose(type);
        return;
    }

    if (scope->count >= scope->capacity) {
        new_capacity = scope->capacity == 0 ? 8 : scope->capacity * 2;
        new_items = (zt_binding *)realloc(scope->items, new_capacity * sizeof(zt_binding));
        if (new_items == NULL) {
            zt_type_dispose(type);
            return;
        }
        scope->items = new_items;
        scope->capacity = new_capacity;
    }

    entry = &scope->items[scope->count++];
    entry->kind = kind;
    entry->name = zt_checker_strdup(name);
    entry->type = type;
    entry->is_mutable = is_mutable;
}

static zt_binding *zt_binding_scope_lookup(zt_binding_scope *scope, const char *name, zt_binding_kind kind) {
    zt_binding_scope *cursor = scope;

    while (cursor != NULL) {
        size_t i;
        for (i = 0; i < cursor->count; i++) {
            if (cursor->items[i].kind == kind && cursor->items[i].name != NULL && strcmp(cursor->items[i].name, name) == 0) {
                return &cursor->items[i];
            }
        }
        cursor = cursor->parent;
    }
    return NULL;
}

static void zt_decl_list_push(zt_decl_list *list, const char *name, const zt_ast_node *node) {
    zt_decl_entry *new_items;
    size_t new_capacity;
    if (list == NULL || name == NULL || node == NULL) return;
    if (list->count >= list->capacity) {
        new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        new_items = (zt_decl_entry *)realloc(list->items, new_capacity * sizeof(zt_decl_entry));
        if (new_items == NULL) return;
        list->items = new_items;
        list->capacity = new_capacity;
    }
    list->items[list->count].name = zt_checker_strdup(name);
    list->items[list->count].node = node;
    list->count += 1;
}

static void zt_import_list_push(zt_import_list *list, const char *local_name, const zt_ast_node *node) {
    zt_import_entry *new_items;
    size_t new_capacity;
    if (list == NULL || local_name == NULL || node == NULL) return;
    if (list->count >= list->capacity) {
        new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        new_items = (zt_import_entry *)realloc(list->items, new_capacity * sizeof(zt_import_entry));
        if (new_items == NULL) return;
        list->items = new_items;
        list->capacity = new_capacity;
    }
    list->items[list->count].local_name = zt_checker_strdup(local_name);
    list->items[list->count].node = node;
    list->count += 1;
}

static void zt_catalog_init(zt_module_catalog *catalog) {
    if (catalog == NULL) return;
    memset(catalog, 0, sizeof(*catalog));
}

static void zt_catalog_dispose(zt_module_catalog *catalog) {
    size_t i;

    if (catalog == NULL) return;
    for (i = 0; i < catalog->decls.count; i++) {
        free(catalog->decls.items[i].name);
    }
    for (i = 0; i < catalog->imports.count; i++) {
        free(catalog->imports.items[i].local_name);
    }
    free(catalog->decls.items);
    free(catalog->imports.items);
    free(catalog->applies.items);
    memset(catalog, 0, sizeof(*catalog));
}

static const zt_ast_node *zt_catalog_find_decl(const zt_module_catalog *catalog, const char *name) {
    size_t i;
    if (catalog == NULL || name == NULL) return NULL;
    for (i = 0; i < catalog->decls.count; i++) {
        if (strcmp(catalog->decls.items[i].name, name) == 0) return catalog->decls.items[i].node;
    }
    return NULL;
}

static int zt_catalog_has_import_alias(const zt_module_catalog *catalog, const char *name) {
    size_t i;
    if (catalog == NULL || name == NULL) return 0;
    for (i = 0; i < catalog->imports.count; i++) {
        if (strcmp(catalog->imports.items[i].local_name, name) == 0) return 1;
    }
    return 0;
}

static const char *zt_catalog_import_path_for_alias(const zt_module_catalog *catalog, const char *name) {
    size_t i;
    if (catalog == NULL || name == NULL) return NULL;
    for (i = 0; i < catalog->imports.count; i++) {
        if (strcmp(catalog->imports.items[i].local_name, name) == 0) {
            const zt_ast_node *import_decl = catalog->imports.items[i].node;
            if (import_decl != NULL && import_decl->kind == ZT_AST_IMPORT_DECL) {
                return import_decl->as.import_decl.path;
            }
            return NULL;
        }
    }
    return NULL;
}
static void zt_apply_list_push(zt_apply_list *list, const zt_ast_node *node) {
    const zt_ast_node **new_items;
    size_t new_capacity;
    if (list == NULL || node == NULL) return;
    if (list->count >= list->capacity) {
        new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        new_items = (const zt_ast_node **)realloc((void *)list->items, new_capacity * sizeof(const zt_ast_node *));
        if (new_items == NULL) return;
        list->items = new_items;
        list->capacity = new_capacity;
    }
    list->items[list->count] = node;
    list->count += 1;
}

static void zt_catalog_build(zt_module_catalog *catalog, const zt_ast_node *root) {
    size_t i;

    if (catalog == NULL || root == NULL || root->kind != ZT_AST_FILE) return;

    for (i = 0; i < root->as.file.imports.count; i++) {
        const zt_ast_node *import_decl = root->as.file.imports.items[i];
        const char *path;
        const char *local_name;
        const char *last_dot;
        if (import_decl == NULL) continue;
        path = import_decl->as.import_decl.path;
        if (import_decl->as.import_decl.alias != NULL) {
            local_name = import_decl->as.import_decl.alias;
        } else {
            last_dot = strrchr(path, '.');
            local_name = last_dot != NULL ? last_dot + 1 : path;
        }
        zt_import_list_push(&catalog->imports, local_name, import_decl);
    }

    for (i = 0; i < root->as.file.declarations.count; i++) {
        const zt_ast_node *decl = root->as.file.declarations.items[i];
        if (decl == NULL) continue;
        switch (decl->kind) {
            case ZT_AST_FUNC_DECL:
                zt_decl_list_push(&catalog->decls, decl->as.func_decl.name, decl);
                break;
            case ZT_AST_STRUCT_DECL:
                zt_decl_list_push(&catalog->decls, decl->as.struct_decl.name, decl);
                break;
            case ZT_AST_TRAIT_DECL:
                zt_decl_list_push(&catalog->decls, decl->as.trait_decl.name, decl);
                break;
            case ZT_AST_ENUM_DECL:
                zt_decl_list_push(&catalog->decls, decl->as.enum_decl.name, decl);
                break;
            case ZT_AST_EXTERN_DECL: {
                size_t f;
                for (f = 0; f < decl->as.extern_decl.functions.count; f++) {
                    const zt_ast_node *func = decl->as.extern_decl.functions.items[f];
                    if (func != NULL) zt_decl_list_push(&catalog->decls, func->as.func_decl.name, func);
                }
                break;
            }
            case ZT_AST_APPLY_DECL:
                zt_apply_list_push(&catalog->applies, decl);
                break;
            default:
                break;
        }
    }
}

static int zt_name_has_dot(const char *name) {
    return name != NULL && strchr(name, '.') != NULL;
}

static const char *zt_type_base_name(const zt_type *type) {
    return type != NULL ? type->name : NULL;
}

static size_t zt_decl_type_param_count(const zt_ast_node *decl) {
    if (decl == NULL) return 0;
    switch (decl->kind) {
        case ZT_AST_STRUCT_DECL: return decl->as.struct_decl.type_params.count;
        case ZT_AST_TRAIT_DECL: return decl->as.trait_decl.type_params.count;
        case ZT_AST_ENUM_DECL: return decl->as.enum_decl.type_params.count;
        case ZT_AST_FUNC_DECL: return decl->as.func_decl.type_params.count;
        default: return 0;
    }
}

static zt_type *zt_builtin_type_by_name(const char *name) {
    if (name == NULL) return NULL;
    if (strcmp(name, "bool") == 0) return zt_type_make(ZT_TYPE_BOOL);
    if (strcmp(name, "int") == 0) return zt_type_make(ZT_TYPE_INT);
    if (strcmp(name, "int8") == 0) return zt_type_make(ZT_TYPE_INT8);
    if (strcmp(name, "int16") == 0) return zt_type_make(ZT_TYPE_INT16);
    if (strcmp(name, "int32") == 0) return zt_type_make(ZT_TYPE_INT32);
    if (strcmp(name, "int64") == 0) return zt_type_make(ZT_TYPE_INT64);
    if (strcmp(name, "uint8") == 0) return zt_type_make(ZT_TYPE_UINT8);
    if (strcmp(name, "uint16") == 0) return zt_type_make(ZT_TYPE_UINT16);
    if (strcmp(name, "uint32") == 0) return zt_type_make(ZT_TYPE_UINT32);
    if (strcmp(name, "uint64") == 0) return zt_type_make(ZT_TYPE_UINT64);
    if (strcmp(name, "float") == 0) return zt_type_make(ZT_TYPE_FLOAT);
    if (strcmp(name, "float32") == 0) return zt_type_make(ZT_TYPE_FLOAT32);
    if (strcmp(name, "float64") == 0) return zt_type_make(ZT_TYPE_FLOAT64);
    if (strcmp(name, "text") == 0) return zt_type_make(ZT_TYPE_TEXT);
    if (strcmp(name, "bytes") == 0) return zt_type_make(ZT_TYPE_BYTES);
    if (strcmp(name, "void") == 0) return zt_type_make(ZT_TYPE_VOID);
    return NULL;
}

static int zt_type_expected_arity(const char *name) {
    if (name == NULL) return -1;
    if (strcmp(name, "optional") == 0) return 1;
    if (strcmp(name, "list") == 0) return 1;
    if (strcmp(name, "result") == 0) return 2;
    if (strcmp(name, "map") == 0) return 2;
    return -1;
}

static void zt_type_describe(const zt_type *type, char *buffer, size_t buffer_size) {
    zt_type_format(type, buffer, buffer_size);
}

static void zt_checker_diag_type(zt_checker *checker, zt_diag_code code, zt_source_span span, const char *prefix, const zt_type *expected, const zt_type *actual) {
    char expected_buf[256];
    char actual_buf[256];
    zt_type_describe(expected, expected_buf, sizeof(expected_buf));
    zt_type_describe(actual, actual_buf, sizeof(actual_buf));
    zt_diag_list_add(&checker->result->diagnostics, code, span, "%s: expected %s but got %s", prefix, expected_buf, actual_buf);
}

static int zt_parse_signed_literal_value(const char *text, long long *out_value) {
    char buffer[128];
    size_t in_i;
    size_t out_i = 0;
    char *end = NULL;
    long long value;

    if (text == NULL || out_value == NULL) return 0;
    for (in_i = 0; text[in_i] != '\0' && out_i + 1 < sizeof(buffer); in_i++) {
        if (text[in_i] != '_') buffer[out_i++] = text[in_i];
    }
    buffer[out_i] = '\0';
    value = strtoll(buffer, &end, 0);
    if (end == NULL || *end != '\0') return 0;
    *out_value = value;
    return 1;
}

static int zt_parse_double_literal_value(const char *text, double *out_value) {
    char buffer[128];
    size_t in_i;
    size_t out_i = 0;
    char *end = NULL;
    double value;

    if (text == NULL || out_value == NULL) return 0;
    for (in_i = 0; text[in_i] != '\0' && out_i + 1 < sizeof(buffer); in_i++) {
        if (text[in_i] != '_') buffer[out_i++] = text[in_i];
    }
    buffer[out_i] = '\0';
    value = strtod(buffer, &end);
    if (end == NULL || *end != '\0') return 0;
    *out_value = value;
    return 1;
}

static int zt_int_literal_fits_kind(const char *text, zt_type_kind kind) {
    long long value;
    if (!zt_parse_signed_literal_value(text, &value)) return 0;
    switch (kind) {
        case ZT_TYPE_INT8: return value >= -128 && value <= 127;
        case ZT_TYPE_INT16: return value >= -32768 && value <= 32767;
        case ZT_TYPE_INT32: return value >= (-2147483647LL - 1) && value <= 2147483647LL;
        case ZT_TYPE_INT64:
        case ZT_TYPE_INT:
            return 1;
        case ZT_TYPE_UINT8: return value >= 0 && value <= 255;
        case ZT_TYPE_UINT16: return value >= 0 && value <= 65535;
        case ZT_TYPE_UINT32: return value >= 0 && value <= 4294967295LL;
        case ZT_TYPE_UINT64: return value >= 0;
        default:
            return 0;
    }
}

static int zt_int_value_fits_kind(long long value, zt_type_kind kind) {
    switch (kind) {
        case ZT_TYPE_INT8: return value >= -128 && value <= 127;
        case ZT_TYPE_INT16: return value >= -32768 && value <= 32767;
        case ZT_TYPE_INT32: return value >= (-2147483647LL - 1) && value <= 2147483647LL;
        case ZT_TYPE_INT64:
        case ZT_TYPE_INT:
            return 1;
        case ZT_TYPE_UINT8: return value >= 0 && value <= 255;
        case ZT_TYPE_UINT16: return value >= 0 && value <= 65535;
        case ZT_TYPE_UINT32: return value >= 0 && value <= 4294967295LL;
        case ZT_TYPE_UINT64: return value >= 0;
        default:
            return 0;
    }
}

static int zt_expr_matches_integral_type(const zt_expr_info *info, const zt_type *type) {
    if (info == NULL || type == NULL || !zt_type_is_integral(type)) return 0;
    if (info->type != NULL && zt_type_equals(info->type, type)) return 1;
    if (info->has_int_value) return zt_int_value_fits_kind(info->int_value, type->kind);
    if (info->is_int_literal && info->literal_text != NULL) return zt_int_literal_fits_kind(info->literal_text, type->kind);
    return 0;
}

static int zt_checker_compute_integral_binary(zt_token_kind op, long long left, long long right, long long *out_value) {
    if (out_value == NULL) return 0;
    switch (op) {
        case ZT_TOKEN_PLUS:
            *out_value = left + right;
            return 1;
        case ZT_TOKEN_MINUS:
            *out_value = left - right;
            return 1;
        case ZT_TOKEN_STAR:
            *out_value = left * right;
            return 1;
        case ZT_TOKEN_SLASH:
            if (right == 0) return 0;
            *out_value = left / right;
            return 1;
        case ZT_TOKEN_PERCENT:
            if (right == 0) return 0;
            *out_value = left % right;
            return 1;
        default:
            return 0;
    }
}

static int zt_type_is_core_trait_name(const char *name) {
    return name != NULL &&
           (strcmp(name, "Equatable") == 0 ||
            strcmp(name, "Hashable") == 0 ||
            strcmp(name, "TextRepresentable") == 0);
}

static int zt_type_param_has_trait(zt_binding_scope *scope, const char *type_param_name, const char *trait_name) {
    zt_binding *binding = zt_binding_scope_lookup(scope, type_param_name, ZT_BINDING_TYPE_PARAM);
    zt_type *constraint_type;

    if (binding == NULL || binding->type == NULL || binding->type->kind != ZT_TYPE_RESULT) {
        return 0;
    }

    constraint_type = binding->type;
    if (constraint_type->args.count == 0 || constraint_type->args.items[0] == NULL || constraint_type->args.items[0]->name == NULL) return 0;
    return strcmp(constraint_type->args.items[0]->name, trait_name) == 0;
}

static int zt_checker_type_implements_trait(zt_checker *checker, zt_binding_scope *scope, const zt_type *type, const char *trait_name);
static zt_type *zt_checker_resolve_type(zt_checker *checker, const zt_ast_node *node, zt_binding_scope *scope);
static zt_expr_info zt_checker_check_expression(zt_checker *checker, const zt_ast_node *node, zt_binding_scope *scope, zt_function_context *fn_ctx, const zt_type *expected_type);
static void zt_checker_check_block(zt_checker *checker, const zt_ast_node *block, zt_binding_scope *parent_scope, zt_function_context *fn_ctx);

static zt_type *zt_checker_make_core_constraint_type(const char *trait_name) {
    zt_type_list args = zt_type_list_make();
    zt_type_list_push(&args, zt_type_make_named(ZT_TYPE_USER, trait_name));
    return zt_type_make_with_args(ZT_TYPE_RESULT, "__constraint__", args);
}

static void zt_checker_register_type_params(zt_checker *checker, const zt_ast_node_list *type_params, const zt_ast_node_list *constraints, zt_binding_scope *scope) {
    size_t i;

    (void)checker;
    if (type_params == NULL || scope == NULL) return;

    for (i = 0; i < type_params->count; i++) {
        const zt_ast_node *param = type_params->items[i];
        if (param == NULL || param->kind != ZT_AST_TYPE_SIMPLE) continue;
        zt_binding_scope_declare(scope, ZT_BINDING_TYPE_PARAM, param->as.type_simple.name, zt_type_make_named(ZT_TYPE_TYPE_PARAM, param->as.type_simple.name), 0);
    }

    if (constraints == NULL) return;
    for (i = 0; i < constraints->count; i++) {
        const zt_ast_node *constraint = constraints->items[i];
        zt_binding *binding;
        const char *trait_name;

        if (constraint == NULL || constraint->kind != ZT_AST_GENERIC_CONSTRAINT) continue;
        binding = zt_binding_scope_lookup(scope, constraint->as.generic_constraint.type_param_name, ZT_BINDING_TYPE_PARAM);
        if (binding == NULL || constraint->as.generic_constraint.trait_type == NULL) continue;

        if (constraint->as.generic_constraint.trait_type->kind == ZT_AST_TYPE_SIMPLE) {
            trait_name = constraint->as.generic_constraint.trait_type->as.type_simple.name;
        } else if (constraint->as.generic_constraint.trait_type->kind == ZT_AST_TYPE_GENERIC) {
            trait_name = constraint->as.generic_constraint.trait_type->as.type_generic.name;
        } else {
            continue;
        }

        if (zt_type_is_core_trait_name(trait_name)) {
            zt_type_dispose(binding->type);
            binding->type = zt_checker_make_core_constraint_type(trait_name);
        }
    }
}

static zt_type *zt_checker_resolve_user_type(zt_checker *checker, const char *name, zt_type_list args, zt_source_span span) {
    const zt_ast_node *decl;
    int expected_arity;

    if (zt_type_is_core_trait_name(name)) {
        return zt_type_make_with_args(ZT_TYPE_USER, name, args);
    }

    if (zt_name_has_dot(name)) {
        return zt_type_make_with_args(ZT_TYPE_USER, name, args);
    }

    decl = zt_catalog_find_decl(&checker->catalog, name);
    if (decl == NULL) {
        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_TYPE, span, "unknown type '%s'", name);
        zt_type_list_dispose(&args);
        return zt_type_make(ZT_TYPE_UNKNOWN);
    }

    expected_arity = (int)zt_decl_type_param_count(decl);
    if (expected_arity != (int)args.count) {
        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_TYPE, span, "type '%s' expects %d type arguments but got %d", name, expected_arity, (int)args.count);
        zt_type_list_dispose(&args);
        return zt_type_make(ZT_TYPE_UNKNOWN);
    }

    return zt_type_make_with_args(ZT_TYPE_USER, name, args);
}

static zt_type *zt_checker_resolve_type(zt_checker *checker, const zt_ast_node *node, zt_binding_scope *scope) {
    zt_type *builtin;
    zt_binding *binding;
    zt_type_list args;
    size_t i;
    int expected_arity;
    const char *name;

    if (node == NULL) return zt_type_make(ZT_TYPE_VOID);

    if (node->kind == ZT_AST_TYPE_SIMPLE) {
        name = node->as.type_simple.name;
        builtin = zt_builtin_type_by_name(name);
        if (builtin != NULL) return builtin;

        binding = zt_binding_scope_lookup(scope, name, ZT_BINDING_TYPE_PARAM);
        if (binding != NULL) return zt_type_clone(binding->type);

        return zt_checker_resolve_user_type(checker, name, zt_type_list_make(), node->span);
    }

    if (node->kind != ZT_AST_TYPE_GENERIC) {
        return zt_type_make(ZT_TYPE_UNKNOWN);
    }

    name = node->as.type_generic.name;
    args = zt_type_list_make();
    for (i = 0; i < node->as.type_generic.type_args.count; i++) {
        zt_type_list_push(&args, zt_checker_resolve_type(checker, node->as.type_generic.type_args.items[i], scope));
    }

    expected_arity = zt_type_expected_arity(name);
    if (expected_arity >= 0) {
        if ((int)args.count != expected_arity) {
            zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_TYPE, node->span, "type '%s' expects %d type arguments but got %d", name, expected_arity, (int)args.count);
            zt_type_list_dispose(&args);
            return zt_type_make(ZT_TYPE_UNKNOWN);
        }

        if (strcmp(name, "optional") == 0) return zt_type_make_with_args(ZT_TYPE_OPTIONAL, NULL, args);
        if (strcmp(name, "result") == 0) return zt_type_make_with_args(ZT_TYPE_RESULT, NULL, args);
        if (strcmp(name, "list") == 0) return zt_type_make_with_args(ZT_TYPE_LIST, NULL, args);
        if (strcmp(name, "map") == 0) {
            zt_type *map_type = zt_type_make_with_args(ZT_TYPE_MAP, NULL, args);
            if (!zt_checker_type_implements_trait(checker, scope, map_type->args.items[0], "Hashable") ||
                !zt_checker_type_implements_trait(checker, scope, map_type->args.items[0], "Equatable")) {
                char key_buf[256];
                zt_type_describe(map_type->args.items[0], key_buf, sizeof(key_buf));
                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_MAP_KEY_TYPE, node->span, "map key type '%s' must implement Hashable and Equatable", key_buf);
            }
            return map_type;
        }
    }

    binding = zt_binding_scope_lookup(scope, name, ZT_BINDING_TYPE_PARAM);
    if (binding != NULL) {
        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_TYPE, node->span, "generic type parameter '%s' cannot be applied with type arguments", name);
        zt_type_list_dispose(&args);
        return zt_type_make(ZT_TYPE_UNKNOWN);
    }

    return zt_checker_resolve_user_type(checker, name, args, node->span);
}

static int zt_checker_type_implements_trait(zt_checker *checker, zt_binding_scope *scope, const zt_type *type, const char *trait_name) {
    size_t i;

    if (type == NULL || trait_name == NULL) return 0;

    switch (type->kind) {
        case ZT_TYPE_BOOL:
        case ZT_TYPE_INT:
        case ZT_TYPE_INT8:
        case ZT_TYPE_INT16:
        case ZT_TYPE_INT32:
        case ZT_TYPE_INT64:
        case ZT_TYPE_UINT8:
        case ZT_TYPE_UINT16:
        case ZT_TYPE_UINT32:
        case ZT_TYPE_UINT64:
            return strcmp(trait_name, "Equatable") == 0 ||
                   strcmp(trait_name, "Hashable") == 0 ||
                   strcmp(trait_name, "TextRepresentable") == 0;
        case ZT_TYPE_FLOAT:
        case ZT_TYPE_FLOAT32:
        case ZT_TYPE_FLOAT64:
            return strcmp(trait_name, "Equatable") == 0 ||
                   strcmp(trait_name, "TextRepresentable") == 0;
        case ZT_TYPE_TEXT:
            return strcmp(trait_name, "Equatable") == 0 ||
                   strcmp(trait_name, "Hashable") == 0 ||
                   strcmp(trait_name, "TextRepresentable") == 0;
        case ZT_TYPE_BYTES:
            return strcmp(trait_name, "Equatable") == 0 ||
                   strcmp(trait_name, "Hashable") == 0;
        case ZT_TYPE_OPTIONAL:
            return type->args.count == 1 && zt_checker_type_implements_trait(checker, scope, type->args.items[0], trait_name);
        case ZT_TYPE_LIST:
            if (strcmp(trait_name, "Hashable") == 0) return 0;
            return type->args.count == 1 && zt_checker_type_implements_trait(checker, scope, type->args.items[0], trait_name);
        case ZT_TYPE_RESULT:
            if (type->args.count != 2) return 0;
            return zt_checker_type_implements_trait(checker, scope, type->args.items[0], trait_name) &&
                   zt_checker_type_implements_trait(checker, scope, type->args.items[1], trait_name);
        case ZT_TYPE_MAP:
            return 0;
        case ZT_TYPE_TYPE_PARAM:
            if (scope == NULL || type->name == NULL) return 0;
            return zt_type_param_has_trait(scope, type->name, trait_name);
        case ZT_TYPE_USER:
            for (i = 0; i < checker->catalog.applies.count; i++) {
                const zt_ast_node *apply_decl = checker->catalog.applies.items[i];
                if (apply_decl == NULL || apply_decl->kind != ZT_AST_APPLY_DECL) continue;
                if (apply_decl->as.apply_decl.trait_name == NULL) continue;
                if (strcmp(apply_decl->as.apply_decl.trait_name, trait_name) != 0) continue;
                if (type->name != NULL && strcmp(apply_decl->as.apply_decl.target_name, type->name) == 0) {
                    return 1;
                }
            }
            return 0;
        default:
            return 0;
    }
}

static int zt_checker_same_or_contextually_assignable(zt_checker *checker, zt_binding_scope *scope, const zt_type *expected, const zt_expr_info *actual, zt_source_span span) {
    zt_type *inner;
    const zt_type *effective_expected;
    int allow_optional_present;
    double float_value;

    if (expected == NULL || actual == NULL || actual->type == NULL) return 1;
    if (actual->type->kind == ZT_TYPE_UNKNOWN || expected->kind == ZT_TYPE_UNKNOWN) return 1;

    if (actual->type->kind == ZT_TYPE_NONE_LITERAL) {
        return expected->kind == ZT_TYPE_OPTIONAL;
    }

    if (actual->type->kind == ZT_TYPE_SUCCESS_WRAPPER) {
        if (expected->kind != ZT_TYPE_RESULT || expected->args.count != 2) return 0;
        if (actual->type->args.count == 0 || actual->type->args.items[0] == NULL) {
            return expected->args.items[0]->kind == ZT_TYPE_VOID;
        }
        inner = actual->type->args.items[0];
        return zt_type_equals(expected->args.items[0], inner) || inner->kind == ZT_TYPE_UNKNOWN;
    }

    if (actual->type->kind == ZT_TYPE_ERROR_WRAPPER) {
        if (expected->kind != ZT_TYPE_RESULT || expected->args.count != 2) return 0;
        if (actual->type->args.count == 0 || actual->type->args.items[0] == NULL) return 0;
        inner = actual->type->args.items[0];
        return zt_type_equals(expected->args.items[1], inner) || inner->kind == ZT_TYPE_UNKNOWN;
    }

    allow_optional_present =
        expected->kind == ZT_TYPE_OPTIONAL &&
        expected->args.count == 1 &&
        actual->type->kind != ZT_TYPE_OPTIONAL;
    effective_expected = allow_optional_present ? expected->args.items[0] : expected;

    /* When the expression has already been typed as the expected type,
       any overflow/invalid-conversion diagnostic should have been emitted
       at the expression site. Avoid cascading assignment/return errors. */
    if (zt_type_equals(effective_expected, actual->type)) return 1;

    if (actual->has_int_value || actual->is_int_literal) {
        long long value = actual->int_value;
        if (!actual->has_int_value && actual->literal_text != NULL) {
            if (!zt_parse_signed_literal_value(actual->literal_text, &value)) {
                value = 0;
            }
        }
        if (zt_type_is_integral(effective_expected)) {
            if (zt_int_value_fits_kind(value, effective_expected->kind)) return 1;
            if (actual->literal_text != NULL) {
                zt_diag_list_add(
                    &checker->result->diagnostics,
                    ZT_DIAG_INVALID_CONVERSION,
                    span,
                    "integer literal '%s' does not fit into %s",
                    actual->literal_text,
                    zt_type_kind_name(effective_expected->kind));
            } else {
                zt_diag_list_add(
                    &checker->result->diagnostics,
                    ZT_DIAG_INVALID_CONVERSION,
                    span,
                    "integer value does not fit into %s",
                    zt_type_kind_name(effective_expected->kind));
            }
            return 0;
        }
        return zt_type_equals(effective_expected, actual->type);
    }

    if (actual->is_float_literal) {
        if (zt_type_is_float(effective_expected)) return 1;
        if (zt_type_is_integral(effective_expected) && zt_parse_double_literal_value(actual->literal_text, &float_value)) {
            double truncated = (double)((long long)float_value);
            return float_value == truncated && zt_int_literal_fits_kind(actual->literal_text, effective_expected->kind);
        }
        return 0;
    }

    if (expected->kind == ZT_TYPE_OPTIONAL && actual->type->kind == ZT_TYPE_OPTIONAL && expected->args.count == 1 && actual->type->args.count == 1) {
        return zt_type_equals(expected->args.items[0], actual->type->args.items[0]);
    }

    (void)scope;
    return 0;
}

static int zt_checker_is_numeric_conversion_target(const char *name) {
    zt_type *type = zt_builtin_type_by_name(name);
    int result = zt_type_is_numeric(type);
    zt_type_dispose(type);
    return result;
}

static int zt_checker_validate_conversion(zt_checker *checker, const char *target_name, const zt_expr_info *arg, zt_source_span span) {
    zt_type *target = zt_builtin_type_by_name(target_name);
    int ok = 0;
    double float_value;

    if (target == NULL || arg == NULL || arg->type == NULL) {
        zt_type_dispose(target);
        return 0;
    }

    if (!zt_type_is_numeric(target) || !zt_type_is_numeric(arg->type)) {
        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_CONVERSION, span, "explicit conversion '%s(...)' requires a numeric source and target", target_name);
        zt_type_dispose(target);
        return 0;
    }

    ok = 1;
    if (zt_type_is_integral(target) && arg->is_float_literal) {
        if (zt_parse_double_literal_value(arg->literal_text, &float_value)) {
            double truncated = (double)((long long)float_value);
            ok = (float_value == truncated) && zt_int_literal_fits_kind(arg->literal_text, target->kind);
        } else {
            ok = 0;
        }
    } else if (zt_type_is_integral(target) && (arg->has_int_value || arg->is_int_literal)) {
        if (arg->has_int_value) ok = zt_int_value_fits_kind(arg->int_value, target->kind);
        else ok = zt_int_literal_fits_kind(arg->literal_text, target->kind);
    }

    if (!ok) {
        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_CONVERSION, span, "invalid explicit conversion to %s", target_name);
    }
    zt_type_dispose(target);
    return ok;
}

static void zt_checker_bind_struct_field_type_params(zt_binding_scope *scope, const zt_ast_node *decl, const zt_type *receiver_type) {
    size_t i;
    if (decl == NULL || receiver_type == NULL) return;
    if (decl->kind != ZT_AST_STRUCT_DECL) return;
    for (i = 0; i < decl->as.struct_decl.type_params.count && i < receiver_type->args.count; i++) {
        const zt_ast_node *param = decl->as.struct_decl.type_params.items[i];
        if (param != NULL && param->kind == ZT_AST_TYPE_SIMPLE) {
            zt_binding_scope_declare(scope, ZT_BINDING_TYPE_PARAM, param->as.type_simple.name, zt_type_clone(receiver_type->args.items[i]), 0);
        }
    }
}

static const zt_ast_node *zt_checker_find_struct_field_decl(const zt_ast_node *struct_decl, const char *field_name) {
    size_t i;
    if (struct_decl == NULL || struct_decl->kind != ZT_AST_STRUCT_DECL || field_name == NULL) return NULL;
    for (i = 0; i < struct_decl->as.struct_decl.fields.count; i++) {
        const zt_ast_node *field = struct_decl->as.struct_decl.fields.items[i];
        if (field != NULL && strcmp(field->as.struct_field.name, field_name) == 0) return field;
    }
    return NULL;
}


static const zt_ast_node *zt_checker_find_enum_variant_decl(const zt_ast_node *enum_decl, const char *variant_name, size_t *out_index) {
    size_t i;
    if (out_index != NULL) *out_index = 0;
    if (enum_decl == NULL || enum_decl->kind != ZT_AST_ENUM_DECL || variant_name == NULL) return NULL;
    for (i = 0; i < enum_decl->as.enum_decl.variants.count; i++) {
        const zt_ast_node *variant = enum_decl->as.enum_decl.variants.items[i];
        if (variant != NULL &&
            variant->kind == ZT_AST_ENUM_VARIANT &&
            variant->as.enum_variant.name != NULL &&
            strcmp(variant->as.enum_variant.name, variant_name) == 0) {
            if (out_index != NULL) *out_index = i;
            return variant;
        }
    }
    return NULL;
}

static const zt_ast_node *zt_checker_enum_variant_field_type_node(const zt_ast_node *field_node) {
    if (field_node == NULL) return NULL;
    if (field_node->kind == ZT_AST_PARAM) return field_node->as.param.type_node;
    if (field_node->kind == ZT_AST_STRUCT_FIELD) return field_node->as.struct_field.type_node;
    return field_node;
}

static const char *zt_checker_enum_variant_field_name(const zt_ast_node *field_node, size_t index, char *fallback_buffer, size_t fallback_size) {
    if (field_node != NULL) {
        if (field_node->kind == ZT_AST_PARAM && field_node->as.param.name != NULL) return field_node->as.param.name;
        if (field_node->kind == ZT_AST_STRUCT_FIELD && field_node->as.struct_field.name != NULL) return field_node->as.struct_field.name;
    }
    if (fallback_buffer != NULL && fallback_size > 0) {
        snprintf(fallback_buffer, fallback_size, "value%zu", index + 1);
        return fallback_buffer;
    }
    return NULL;
}

static const zt_ast_node *zt_checker_find_apply_method(const zt_checker *checker, const zt_type *receiver_type, const char *method_name, const zt_ast_node **out_apply_decl) {
    size_t i;

    if (out_apply_decl != NULL) *out_apply_decl = NULL;
    if (checker == NULL || receiver_type == NULL || receiver_type->name == NULL || method_name == NULL) return NULL;

    for (i = 0; i < checker->catalog.applies.count; i++) {
        const zt_ast_node *apply_decl = checker->catalog.applies.items[i];
        size_t m;
        if (apply_decl == NULL || apply_decl->kind != ZT_AST_APPLY_DECL) continue;
        if (strcmp(apply_decl->as.apply_decl.target_name, receiver_type->name) != 0) continue;
        for (m = 0; m < apply_decl->as.apply_decl.methods.count; m++) {
            const zt_ast_node *method = apply_decl->as.apply_decl.methods.items[m];
            if (method != NULL && method->kind == ZT_AST_FUNC_DECL && strcmp(method->as.func_decl.name, method_name) == 0) {
                if (out_apply_decl != NULL) *out_apply_decl = apply_decl;
                return method;
            }
        }
    }

    return NULL;
}

static int zt_checker_build_qualified_name(const zt_ast_node *node, char *buffer, size_t buffer_size) {
    size_t len;
    size_t field_len;

    if (node == NULL || buffer == NULL || buffer_size == 0) return 0;

    if (node->kind == ZT_AST_IDENT_EXPR) {
        if (node->as.ident_expr.name == NULL) return 0;
        if (snprintf(buffer, buffer_size, "%s", node->as.ident_expr.name) < 0) return 0;
        return strlen(buffer) < buffer_size;
    }

    if (node->kind == ZT_AST_FIELD_EXPR) {
        if (node->as.field_expr.field_name == NULL) return 0;
        if (!zt_checker_build_qualified_name(node->as.field_expr.object, buffer, buffer_size)) return 0;
        len = strlen(buffer);
        field_len = strlen(node->as.field_expr.field_name);
        if (len + 1 + field_len + 1 > buffer_size) return 0;
        buffer[len] = '.';
        memcpy(buffer + len + 1, node->as.field_expr.field_name, field_len + 1);
        return 1;
    }

    return 0;
}

static zt_expr_info zt_checker_check_field_expr(zt_checker *checker, const zt_ast_node *node, zt_binding_scope *scope, zt_function_context *fn_ctx) {
    zt_expr_info object_info;
    zt_expr_info result;
    const zt_ast_node *decl;
    const zt_ast_node *field_decl;
    zt_binding_scope type_scope;

    object_info = zt_checker_check_expression(checker, node->as.field_expr.object, scope, fn_ctx, NULL);
    result = zt_expr_info_make(zt_type_make(ZT_TYPE_UNKNOWN));

    if (object_info.type == NULL || object_info.type->kind == ZT_TYPE_UNKNOWN) {
        zt_expr_info_dispose(&object_info);
        return result;
    }

    if (node->as.field_expr.object->kind == ZT_AST_IDENT_EXPR &&
        zt_catalog_has_import_alias(&checker->catalog, node->as.field_expr.object->as.ident_expr.name)) {
        zt_expr_info_dispose(&object_info);
        return result;
    }

    if (object_info.type->kind == ZT_TYPE_USER) {
        decl = zt_catalog_find_decl(&checker->catalog, zt_type_base_name(object_info.type));
        if (decl != NULL && decl->kind == ZT_AST_STRUCT_DECL) {
            field_decl = zt_checker_find_struct_field_decl(decl, node->as.field_expr.field_name);
            if (field_decl != NULL) {
                zt_type_dispose(result.type);
                zt_binding_scope_init(&type_scope, scope);
                zt_checker_bind_struct_field_type_params(&type_scope, decl, object_info.type);
                result.type = zt_checker_resolve_type(checker, field_decl->as.struct_field.type_node, &type_scope);
                zt_binding_scope_dispose(&type_scope);
            }
        }
    }

    zt_expr_info_dispose(&object_info);
    return result;
}

static int zt_checker_expression_is_mutable_target(const zt_ast_node *node, zt_binding_scope *scope, zt_function_context *fn_ctx) {
    zt_binding *binding;

    if (node == NULL) return 0;
    switch (node->kind) {
        case ZT_AST_IDENT_EXPR:
            if (strcmp(node->as.ident_expr.name, "self") == 0) {
                return fn_ctx != NULL && fn_ctx->in_mutating_method;
            }
            binding = zt_binding_scope_lookup(scope, node->as.ident_expr.name, ZT_BINDING_VALUE);
            return binding != NULL && binding->is_mutable;
        case ZT_AST_FIELD_EXPR:
            return zt_checker_expression_is_mutable_target(node->as.field_expr.object, scope, fn_ctx);
        case ZT_AST_INDEX_EXPR:
            return zt_checker_expression_is_mutable_target(node->as.index_expr.object, scope, fn_ctx);
        default:
            return 0;
    }
}

static zt_expr_info zt_checker_check_call_expr(zt_checker *checker, const zt_ast_node *node, zt_binding_scope *scope, zt_function_context *fn_ctx, const zt_type *expected_type) {
    zt_expr_info result = zt_expr_info_make(zt_type_make(ZT_TYPE_UNKNOWN));
    const zt_ast_node *callee = node->as.call_expr.callee;
    size_t i;

    if (callee == NULL) return result;

    if (callee->kind == ZT_AST_IDENT_EXPR) {
        const char *name = callee->as.ident_expr.name;
        const zt_ast_node *decl = zt_catalog_find_decl(&checker->catalog, name);

        if (zt_checker_is_numeric_conversion_target(name)) {
            zt_expr_info arg_info;
            zt_type_dispose(result.type);
            result.type = zt_builtin_type_by_name(name);
            if (node->as.call_expr.named_args.count != 0 || node->as.call_expr.positional_args.count != 1) {
                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_CALL, node->span, "explicit conversion '%s(...)' expects exactly one positional argument", name);
                return result;
            }
            arg_info = zt_checker_check_expression(checker, node->as.call_expr.positional_args.items[0], scope, fn_ctx, NULL);
            zt_checker_validate_conversion(checker, name, &arg_info, node->span);
            if (zt_type_is_integral(result.type) && arg_info.has_int_value && zt_int_value_fits_kind(arg_info.int_value, result.type->kind)) {
                result.has_int_value = 1;
                result.int_value = arg_info.int_value;
            }
            zt_expr_info_dispose(&arg_info);
            return result;
        }

        if (name != NULL && strcmp(name, "len") == 0) {
            zt_expr_info arg_info;
            zt_type_dispose(result.type);
            result.type = zt_type_make(ZT_TYPE_INT);
            if (node->as.call_expr.named_args.count != 0 || node->as.call_expr.positional_args.count != 1) {
                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_CALL, node->span, "len(...) expects exactly one positional argument");
                return result;
            }

            arg_info = zt_checker_check_expression(checker, node->as.call_expr.positional_args.items[0], scope, fn_ctx, NULL);
            if (arg_info.type == NULL ||
                    !(arg_info.type->kind == ZT_TYPE_TEXT ||
                      arg_info.type->kind == ZT_TYPE_BYTES ||
                      arg_info.type->kind == ZT_TYPE_LIST ||
                      arg_info.type->kind == ZT_TYPE_MAP)) {
                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_TYPE, node->span, "len(...) requires text, bytes, list<T> or map<K, V>");
            }
            zt_expr_info_dispose(&arg_info);
            return result;
        }

        if (decl != NULL && decl->kind == ZT_AST_STRUCT_DECL) {
            const zt_ast_node *struct_decl = decl;
            size_t positional_count = node->as.call_expr.positional_args.count;
            size_t named_count = node->as.call_expr.named_args.count;
            int used_fields[128];
            size_t last_named_index = positional_count == 0 ? 0 : positional_count - 1;
            memset(used_fields, 0, sizeof(used_fields));

            zt_type_dispose(result.type);
            result.type = zt_type_make_named(ZT_TYPE_USER, struct_decl->as.struct_decl.name);

            if (struct_decl->as.struct_decl.fields.count > sizeof(used_fields) / sizeof(used_fields[0])) {
                return result;
            }

            for (i = 0; i < positional_count; i++) {
                const zt_ast_node *field;
                zt_type *field_type;
                zt_expr_info arg_info;

                if (i >= struct_decl->as.struct_decl.fields.count) {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, node->span, "too many positional arguments for constructor '%s'", name);
                    break;
                }

                field = struct_decl->as.struct_decl.fields.items[i];
                field_type = zt_checker_resolve_type(checker, field->as.struct_field.type_node, scope);
                arg_info = zt_checker_check_expression(checker, node->as.call_expr.positional_args.items[i], scope, fn_ctx, field_type);
                if (!zt_checker_same_or_contextually_assignable(checker, scope, field_type, &arg_info, node->as.call_expr.positional_args.items[i]->span)) {
                    zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, node->as.call_expr.positional_args.items[i]->span, "constructor field type mismatch", field_type, arg_info.type);
                }
                used_fields[i] = 1;
                zt_expr_info_dispose(&arg_info);
                zt_type_dispose(field_type);
            }

            for (i = 0; i < named_count; i++) {
                const zt_ast_named_arg *arg = &node->as.call_expr.named_args.items[i];
                size_t f;
                int found = 0;

                for (f = 0; f < struct_decl->as.struct_decl.fields.count; f++) {
                    const zt_ast_node *field = struct_decl->as.struct_decl.fields.items[f];
                    if (field == NULL || strcmp(field->as.struct_field.name, arg->name) != 0) continue;

                    {
                        zt_type *field_type;
                        zt_expr_info arg_info;

                        if (f < positional_count || used_fields[f]) {
                            zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, arg->span, "field '%s' passed more than once in constructor '%s'", arg->name, name);
                        }
                        if (f < last_named_index) {
                            zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, arg->span, "named field '%s' is out of declaration order in constructor '%s'", arg->name, name);
                        } else {
                            last_named_index = f;
                        }

                        used_fields[f] = 1;
                        field_type = zt_checker_resolve_type(checker, field->as.struct_field.type_node, scope);
                        arg_info = zt_checker_check_expression(checker, arg->value, scope, fn_ctx, field_type);
                        if (!zt_checker_same_or_contextually_assignable(checker, scope, field_type, &arg_info, arg->span)) {
                            zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, arg->span, "constructor field type mismatch", field_type, arg_info.type);
                        }
                        zt_expr_info_dispose(&arg_info);
                        zt_type_dispose(field_type);
                    }

                    found = 1;
                    break;
                }

                if (!found) {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, arg->span, "unknown named field '%s' in constructor '%s'", arg->name, name);
                }
            }

            for (i = 0; i < struct_decl->as.struct_decl.fields.count; i++) {
                const zt_ast_node *field = struct_decl->as.struct_decl.fields.items[i];
                if (!used_fields[i] && field->as.struct_field.default_value == NULL) {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, node->span, "missing field '%s' in constructor '%s'", field->as.struct_field.name, name);
                }
            }

            return result;
        }

        if (decl != NULL && decl->kind == ZT_AST_FUNC_DECL) {
            const zt_ast_node *func_decl = decl;
            size_t positional_count = node->as.call_expr.positional_args.count;
            size_t named_count = node->as.call_expr.named_args.count;
            int used_params[128];
            size_t last_named_index = positional_count == 0 ? 0 : positional_count - 1;
            memset(used_params, 0, sizeof(used_params));

            zt_type_dispose(result.type);
            result.type = zt_checker_resolve_type(checker, func_decl->as.func_decl.return_type, scope);

            if (func_decl->as.func_decl.params.count > sizeof(used_params) / sizeof(used_params[0])) {
                return result;
            }

            for (i = 0; i < positional_count; i++) {
                const zt_ast_node *param;
                zt_type *param_type;
                zt_expr_info arg_info;
                if (i >= func_decl->as.func_decl.params.count) {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, node->span, "too many positional arguments for '%s'", name);
                    break;
                }
                param = func_decl->as.func_decl.params.items[i];
                param_type = zt_checker_resolve_type(checker, param->as.param.type_node, scope);
                arg_info = zt_checker_check_expression(checker, node->as.call_expr.positional_args.items[i], scope, fn_ctx, param_type);
                if (!zt_checker_same_or_contextually_assignable(checker, scope, param_type, &arg_info, node->as.call_expr.positional_args.items[i]->span)) {
                    zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, node->as.call_expr.positional_args.items[i]->span, "argument type mismatch", param_type, arg_info.type);
                }
                used_params[i] = 1;
                zt_expr_info_dispose(&arg_info);
                zt_type_dispose(param_type);
            }

            for (i = 0; i < named_count; i++) {
                const zt_ast_named_arg *arg = &node->as.call_expr.named_args.items[i];
                size_t p;
                int found = 0;
                for (p = 0; p < func_decl->as.func_decl.params.count; p++) {
                    const zt_ast_node *param = func_decl->as.func_decl.params.items[p];
                    if (strcmp(param->as.param.name, arg->name) == 0) {
                        zt_type *param_type;
                        zt_expr_info arg_info;
                        if (p < positional_count || used_params[p]) {
                            zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, arg->span, "parameter '%s' passed more than once", arg->name);
                        }
                        if (p < last_named_index) {
                            zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, arg->span, "named argument '%s' is out of declaration order", arg->name);
                        } else {
                            last_named_index = p;
                        }
                        used_params[p] = 1;
                        param_type = zt_checker_resolve_type(checker, param->as.param.type_node, scope);
                        arg_info = zt_checker_check_expression(checker, arg->value, scope, fn_ctx, param_type);
                        if (!zt_checker_same_or_contextually_assignable(checker, scope, param_type, &arg_info, arg->span)) {
                            zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, arg->span, "named argument type mismatch", param_type, arg_info.type);
                        }
                        zt_expr_info_dispose(&arg_info);
                        zt_type_dispose(param_type);
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, arg->span, "unknown named argument '%s' in call to '%s'", arg->name, name);
                }
            }

            for (i = 0; i < func_decl->as.func_decl.params.count; i++) {
                const zt_ast_node *param = func_decl->as.func_decl.params.items[i];
                if (!used_params[i] && param->as.param.default_value == NULL) {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, node->span, "missing argument for parameter '%s' in call to '%s'", param->as.param.name, name);
                }
            }
            return result;
        }
    }

    if (callee->kind == ZT_AST_FIELD_EXPR) {
        const zt_ast_node *apply_decl = NULL;
        const zt_ast_node *method_decl = NULL;
        zt_expr_info object_info = zt_checker_check_expression(checker, callee->as.field_expr.object, scope, fn_ctx, NULL);
        
        if (callee->as.field_expr.object != NULL &&
            callee->as.field_expr.object->kind == ZT_AST_IDENT_EXPR &&
            callee->as.field_expr.field_name != NULL) {
            const char *enum_name = callee->as.field_expr.object->as.ident_expr.name;
            const char *variant_name = callee->as.field_expr.field_name;
            const zt_ast_node *enum_decl = zt_catalog_find_decl(&checker->catalog, enum_name);
            if (enum_decl != NULL && enum_decl->kind == ZT_AST_ENUM_DECL) {
                const zt_ast_node *variant_decl = zt_checker_find_enum_variant_decl(enum_decl, variant_name, NULL);
                size_t positional_count = node->as.call_expr.positional_args.count;
                size_t named_count = node->as.call_expr.named_args.count;
                size_t field_count = variant_decl != NULL ? variant_decl->as.enum_variant.fields.count : 0;
                size_t arg_index;
                int *used_fields = NULL;

                zt_type_dispose(result.type);
                result.type = zt_type_make_named(ZT_TYPE_USER, enum_name);

                if (variant_decl == NULL) {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, node->span, "unknown enum variant '%s.%s'", enum_name, variant_name);
                    zt_expr_info_dispose(&object_info);
                    return result;
                }

                if (field_count > 0) {
                    used_fields = (int *)calloc(field_count, sizeof(int));
                    if (used_fields == NULL) {
                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_CALL, node->span, "out of memory validating enum constructor");
                        zt_expr_info_dispose(&object_info);
                        return result;
                    }
                }

                for (arg_index = 0; arg_index < positional_count; arg_index++) {
                    const zt_ast_node *field_node;
                    const zt_ast_node *field_type_node;
                    zt_type *field_type;
                    zt_expr_info arg_info;

                    if (arg_index >= field_count) {
                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, node->span, "too many positional arguments for enum variant '%s.%s'", enum_name, variant_name);
                        break;
                    }

                    field_node = variant_decl->as.enum_variant.fields.items[arg_index];
                    field_type_node = zt_checker_enum_variant_field_type_node(field_node);
                    field_type = zt_checker_resolve_type(checker, field_type_node, scope);
                    arg_info = zt_checker_check_expression(checker, node->as.call_expr.positional_args.items[arg_index], scope, fn_ctx, field_type);
                    if (!zt_checker_same_or_contextually_assignable(checker, scope, field_type, &arg_info, node->as.call_expr.positional_args.items[arg_index]->span)) {
                        zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, node->as.call_expr.positional_args.items[arg_index]->span, "enum payload type mismatch", field_type, arg_info.type);
                    }
                    if (used_fields != NULL) used_fields[arg_index] = 1;
                    zt_expr_info_dispose(&arg_info);
                    zt_type_dispose(field_type);
                }

                for (arg_index = 0; arg_index < named_count; arg_index++) {
                    const zt_ast_named_arg *arg = &node->as.call_expr.named_args.items[arg_index];
                    size_t f;
                    int found = 0;
                    for (f = 0; f < field_count; f++) {
                        const zt_ast_node *field_node = variant_decl->as.enum_variant.fields.items[f];
                        const zt_ast_node *field_type_node;
                        const char *field_name;
                        char fallback_name[32];
                        if (field_node == NULL) continue;
                        field_name = zt_checker_enum_variant_field_name(field_node, f, fallback_name, sizeof(fallback_name));
                        if (field_name == NULL || strcmp(field_name, arg->name) != 0) continue;

                        field_type_node = zt_checker_enum_variant_field_type_node(field_node);
                        if (used_fields != NULL && used_fields[f]) {
                            zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, arg->span, "payload field '%s' passed more than once in '%s.%s'", arg->name, enum_name, variant_name);
                        } else {
                            zt_type *field_type = zt_checker_resolve_type(checker, field_type_node, scope);
                            zt_expr_info arg_info = zt_checker_check_expression(checker, arg->value, scope, fn_ctx, field_type);
                            if (!zt_checker_same_or_contextually_assignable(checker, scope, field_type, &arg_info, arg->span)) {
                                zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, arg->span, "enum payload type mismatch", field_type, arg_info.type);
                            }
                            zt_expr_info_dispose(&arg_info);
                            zt_type_dispose(field_type);
                            if (used_fields != NULL) used_fields[f] = 1;
                        }

                        found = 1;
                        break;
                    }

                    if (!found) {
                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, arg->span, "unknown payload field '%s' in enum variant '%s.%s'", arg->name, enum_name, variant_name);
                    }
                }

                for (arg_index = 0; arg_index < field_count; arg_index++) {
                    if (used_fields != NULL && !used_fields[arg_index]) {
                        const zt_ast_node *field_node = variant_decl->as.enum_variant.fields.items[arg_index];
                        const char *field_name;
                        char fallback_name[32];
                        field_name = zt_checker_enum_variant_field_name(field_node, arg_index, fallback_name, sizeof(fallback_name));
                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, node->span, "missing payload field '%s' in enum variant '%s.%s'", field_name != NULL ? field_name : "<field>", enum_name, variant_name);
                    }
                }

                free(used_fields);
                zt_expr_info_dispose(&object_info);
                return result;
            }
        }

        if (callee->as.field_expr.object->kind == ZT_AST_IDENT_EXPR &&
            zt_catalog_has_import_alias(&checker->catalog, callee->as.field_expr.object->as.ident_expr.name)) {
            const char *alias = callee->as.field_expr.object->as.ident_expr.name;
            const char *import_path = zt_catalog_import_path_for_alias(&checker->catalog, alias);
            const char *member = callee->as.field_expr.field_name;

            if (import_path != NULL && strcmp(import_path, "std.text") == 0 && member != NULL) {
                if (strcmp(member, "to_utf8") == 0) {
                    if (node->as.call_expr.named_args.count != 0 || node->as.call_expr.positional_args.count != 1) {
                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_CALL, node->span, "std.text.to_utf8(...) expects exactly one positional argument");
                    } else {
                        zt_expr_info arg_info = zt_checker_check_expression(checker, node->as.call_expr.positional_args.items[0], scope, fn_ctx, NULL);
                        if (arg_info.type == NULL || arg_info.type->kind != ZT_TYPE_TEXT) {
                            zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_TYPE_MISMATCH, node->span, "std.text.to_utf8(...) requires a text argument");
                        }
                        zt_expr_info_dispose(&arg_info);
                    }
                    zt_type_dispose(result.type);
                    result.type = zt_type_make(ZT_TYPE_BYTES);
                    zt_expr_info_dispose(&object_info);
                    return result;
                }

                if (strcmp(member, "from_utf8") == 0) {
                    zt_type_list args = zt_type_list_make();
                    if (node->as.call_expr.named_args.count != 0 || node->as.call_expr.positional_args.count != 1) {
                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_CALL, node->span, "std.text.from_utf8(...) expects exactly one positional argument");
                    } else {
                        zt_expr_info arg_info = zt_checker_check_expression(checker, node->as.call_expr.positional_args.items[0], scope, fn_ctx, NULL);
                        if (arg_info.type == NULL || arg_info.type->kind != ZT_TYPE_BYTES) {
                            zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_TYPE_MISMATCH, node->span, "std.text.from_utf8(...) requires a bytes argument");
                        }
                        zt_expr_info_dispose(&arg_info);
                    }
                    zt_type_dispose(result.type);
                    zt_type_list_push(&args, zt_type_make(ZT_TYPE_TEXT));
                    zt_type_list_push(&args, zt_type_make(ZT_TYPE_TEXT));
                    result.type = zt_type_make_with_args(ZT_TYPE_RESULT, NULL, args);
                    zt_expr_info_dispose(&object_info);
                    return result;
                }
            }

            if (import_path != NULL && strcmp(import_path, "std.bytes") == 0 && member != NULL) {
                if (strcmp(member, "empty") == 0) {
                    if (node->as.call_expr.named_args.count != 0 || node->as.call_expr.positional_args.count != 0) {
                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_CALL, node->span, "std.bytes.empty() expects no arguments");
                    }
                    zt_type_dispose(result.type);
                    result.type = zt_type_make(ZT_TYPE_BYTES);
                    zt_expr_info_dispose(&object_info);
                    return result;
                }

                if (strcmp(member, "from_list") == 0) {
                    if (node->as.call_expr.named_args.count != 0 || node->as.call_expr.positional_args.count != 1) {
                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_CALL, node->span, "std.bytes.from_list(...) expects exactly one positional argument");
                    } else {
                        zt_expr_info arg_info = zt_checker_check_expression(checker, node->as.call_expr.positional_args.items[0], scope, fn_ctx, NULL);
                        if (arg_info.type == NULL || arg_info.type->kind != ZT_TYPE_LIST || arg_info.type->args.count != 1 || arg_info.type->args.items[0] == NULL || arg_info.type->args.items[0]->kind != ZT_TYPE_INT) {
                            zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_TYPE_MISMATCH, node->span, "std.bytes.from_list(...) requires a list<int> argument in the current backend subset");
                        }
                        zt_expr_info_dispose(&arg_info);
                    }
                    zt_type_dispose(result.type);
                    result.type = zt_type_make(ZT_TYPE_BYTES);
                    zt_expr_info_dispose(&object_info);
                    return result;
                }

                if (strcmp(member, "to_list") == 0) {
                    zt_type_list args = zt_type_list_make();
                    if (node->as.call_expr.named_args.count != 0 || node->as.call_expr.positional_args.count != 1) {
                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_CALL, node->span, "std.bytes.to_list(...) expects exactly one positional argument");
                    } else {
                        zt_expr_info arg_info = zt_checker_check_expression(checker, node->as.call_expr.positional_args.items[0], scope, fn_ctx, NULL);
                        if (arg_info.type == NULL || arg_info.type->kind != ZT_TYPE_BYTES) {
                            zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_TYPE_MISMATCH, node->span, "std.bytes.to_list(...) requires a bytes argument");
                        }
                        zt_expr_info_dispose(&arg_info);
                    }
                    zt_type_dispose(result.type);
                    zt_type_list_push(&args, zt_type_make(ZT_TYPE_INT));
                    result.type = zt_type_make_with_args(ZT_TYPE_LIST, NULL, args);
                    zt_expr_info_dispose(&object_info);
                    return result;
                }

                if (strcmp(member, "join") == 0 || strcmp(member, "starts_with") == 0 || strcmp(member, "ends_with") == 0 || strcmp(member, "contains") == 0) {
                    if (node->as.call_expr.named_args.count != 0 || node->as.call_expr.positional_args.count != 2) {
                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_CALL, node->span, "std.bytes.%s(...) expects exactly two positional bytes arguments in the current backend subset", member);
                    } else {
                        zt_expr_info left_arg = zt_checker_check_expression(checker, node->as.call_expr.positional_args.items[0], scope, fn_ctx, NULL);
                        zt_expr_info right_arg = zt_checker_check_expression(checker, node->as.call_expr.positional_args.items[1], scope, fn_ctx, NULL);
                        if (left_arg.type == NULL || left_arg.type->kind != ZT_TYPE_BYTES || right_arg.type == NULL || right_arg.type->kind != ZT_TYPE_BYTES) {
                            zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_TYPE_MISMATCH, node->span, "std.bytes.%s(...) requires bytes arguments", member);
                        }
                        zt_expr_info_dispose(&left_arg);
                        zt_expr_info_dispose(&right_arg);
                    }

                    zt_type_dispose(result.type);
                    if (strcmp(member, "join") == 0) {
                        result.type = zt_type_make(ZT_TYPE_BYTES);
                    } else {
                        result.type = zt_type_make(ZT_TYPE_BOOL);
                    }
                    zt_expr_info_dispose(&object_info);
                    return result;
                }
            }
            /* Generic catalog lookup for module-prefixed functions (e.g. io.print_line) */
            {
                char combined_name[512];
                const zt_ast_node *func_decl;
                snprintf(combined_name, sizeof(combined_name), "%s.%s", alias, member);
                func_decl = zt_catalog_find_decl(&checker->catalog, combined_name);
                if (func_decl != NULL && func_decl->kind == ZT_AST_FUNC_DECL) {
                    zt_type_dispose(result.type);
                    result.type = zt_checker_resolve_type(checker, func_decl->as.func_decl.return_type, scope);
                    zt_expr_info_dispose(&object_info);
                    return result;
                }
            }

            zt_type_dispose(result.type);
            result.type = expected_type != NULL ? zt_type_clone(expected_type) : zt_type_make(ZT_TYPE_UNKNOWN);
            zt_expr_info_dispose(&object_info);
            return result;
        }

        if (object_info.type != NULL &&
            object_info.type->kind == ZT_TYPE_LIST &&
            object_info.type->args.count == 1 &&
            callee->as.field_expr.field_name != NULL &&
            strcmp(callee->as.field_expr.field_name, "get") == 0) {
            zt_type *index_type = zt_type_make(ZT_TYPE_INT);
            zt_type_list optional_args = zt_type_list_make();
            if (node->as.call_expr.named_args.count != 0 || node->as.call_expr.positional_args.count != 1) {
                zt_diag_list_add(
                    &checker->result->diagnostics,
                    ZT_DIAG_INVALID_CALL,
                    node->span,
                    "list.get(...) expects exactly one positional index argument");
            } else {
                zt_expr_info index_arg =
                    zt_checker_check_expression(checker, node->as.call_expr.positional_args.items[0], scope, fn_ctx, index_type);
                if (index_arg.type == NULL || !zt_type_is_integral(index_arg.type)) {
                    zt_checker_diag_type(
                        checker,
                        ZT_DIAG_TYPE_MISMATCH,
                        node->as.call_expr.positional_args.items[0]->span,
                        "list.get index type mismatch",
                        index_type,
                        index_arg.type);
                }
                zt_expr_info_dispose(&index_arg);
            }
            zt_type_dispose(index_type);

            zt_type_dispose(result.type);
            zt_type_list_push(&optional_args, zt_type_clone(object_info.type->args.items[0]));
            result.type = zt_type_make_with_args(ZT_TYPE_OPTIONAL, NULL, optional_args);
            zt_expr_info_dispose(&object_info);
            return result;
        }

        if (object_info.type != NULL &&
            object_info.type->kind == ZT_TYPE_MAP &&
            object_info.type->args.count == 2 &&
            callee->as.field_expr.field_name != NULL &&
            strcmp(callee->as.field_expr.field_name, "get") == 0) {
            zt_type *key_type = zt_type_clone(object_info.type->args.items[0]);
            zt_type_list optional_args = zt_type_list_make();
            if (node->as.call_expr.named_args.count != 0 || node->as.call_expr.positional_args.count != 1) {
                zt_diag_list_add(
                    &checker->result->diagnostics,
                    ZT_DIAG_INVALID_CALL,
                    node->span,
                    "map.get(...) expects exactly one positional key argument");
            } else {
                zt_expr_info key_arg =
                    zt_checker_check_expression(checker, node->as.call_expr.positional_args.items[0], scope, fn_ctx, key_type);
                if (!zt_checker_same_or_contextually_assignable(checker, scope, key_type, &key_arg, node->as.call_expr.positional_args.items[0]->span)) {
                    zt_checker_diag_type(
                        checker,
                        ZT_DIAG_TYPE_MISMATCH,
                        node->as.call_expr.positional_args.items[0]->span,
                        "map.get key type mismatch",
                        key_type,
                        key_arg.type);
                }
                zt_expr_info_dispose(&key_arg);
            }
            zt_type_dispose(key_type);

            zt_type_dispose(result.type);
            zt_type_list_push(&optional_args, zt_type_clone(object_info.type->args.items[1]));
            result.type = zt_type_make_with_args(ZT_TYPE_OPTIONAL, NULL, optional_args);
            zt_expr_info_dispose(&object_info);
            return result;
        }

        if (object_info.type != NULL && object_info.type->kind == ZT_TYPE_TYPE_PARAM) {
            if (strcmp(callee->as.field_expr.field_name, "to_text") == 0 &&
                zt_checker_type_implements_trait(checker, scope, object_info.type, "TextRepresentable")) {
                zt_type_dispose(result.type);
                result.type = zt_type_make(ZT_TYPE_TEXT);
                if (node->as.call_expr.positional_args.count != 0 || node->as.call_expr.named_args.count != 0) {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_CALL, node->span, "to_text() expects no arguments");
                }
            }
            zt_expr_info_dispose(&object_info);
            return result;
        }

        method_decl = zt_checker_find_apply_method(checker, object_info.type, callee->as.field_expr.field_name, &apply_decl);
        if (method_decl != NULL) {
            zt_binding_scope method_scope;
            size_t p;
            zt_type_dispose(result.type);
            zt_binding_scope_init(&method_scope, scope);

            if (apply_decl != NULL && apply_decl->as.apply_decl.trait_name == NULL) {
                for (p = 0; p < apply_decl->as.apply_decl.target_type_params.count && p < object_info.type->args.count; p++) {
                    const zt_ast_node *tp = apply_decl->as.apply_decl.target_type_params.items[p];
                    if (tp != NULL && tp->kind == ZT_AST_TYPE_SIMPLE) {
                        zt_binding_scope_declare(&method_scope, ZT_BINDING_TYPE_PARAM, tp->as.type_simple.name, zt_type_clone(object_info.type->args.items[p]), 0);
                    }
                }
            }

            result.type = zt_checker_resolve_type(checker, method_decl->as.func_decl.return_type, &method_scope);
            if (node->as.call_expr.positional_args.count > method_decl->as.func_decl.params.count) {
                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, node->span, "too many arguments for method '%s'", callee->as.field_expr.field_name);
            }

            for (p = 0; p < node->as.call_expr.positional_args.count && p < method_decl->as.func_decl.params.count; p++) {
                const zt_ast_node *param = method_decl->as.func_decl.params.items[p];
                zt_type *param_type = zt_checker_resolve_type(checker, param->as.param.type_node, &method_scope);
                zt_expr_info arg_info = zt_checker_check_expression(checker, node->as.call_expr.positional_args.items[p], scope, fn_ctx, param_type);
                if (!zt_checker_same_or_contextually_assignable(checker, scope, param_type, &arg_info, node->as.call_expr.positional_args.items[p]->span)) {
                    zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, node->as.call_expr.positional_args.items[p]->span, "method argument type mismatch", param_type, arg_info.type);
                }
                zt_expr_info_dispose(&arg_info);
                zt_type_dispose(param_type);
            }

            zt_binding_scope_dispose(&method_scope);
        }

        zt_expr_info_dispose(&object_info);
        return result;
    }

    (void)expected_type;
    return result;
}

static zt_expr_info zt_checker_check_expression(zt_checker *checker, const zt_ast_node *node, zt_binding_scope *scope, zt_function_context *fn_ctx, const zt_type *expected_type) {
    zt_expr_info info = zt_expr_info_make(zt_type_make(ZT_TYPE_UNKNOWN));
    zt_binding *binding;
    zt_expr_info left_info;
    zt_expr_info right_info;
    zt_expr_info operand_info;

    if (node == NULL) {
        zt_type_dispose(info.type);
        info.type = zt_type_make(ZT_TYPE_VOID);
        return info;
    }

    switch (node->kind) {
        case ZT_AST_INT_EXPR:
            zt_type_dispose(info.type);
            info.type = zt_type_make(ZT_TYPE_INT);
            info.is_int_literal = 1;
            info.has_int_value = zt_parse_signed_literal_value(node->as.int_expr.value, &info.int_value);
            info.literal_text = node->as.int_expr.value;
            return info;
        case ZT_AST_FLOAT_EXPR:
            zt_type_dispose(info.type);
            info.type = zt_type_make(ZT_TYPE_FLOAT);
            info.is_float_literal = 1;
            info.literal_text = node->as.float_expr.value;
            return info;
        case ZT_AST_STRING_EXPR:
            zt_type_dispose(info.type);
            info.type = zt_type_make(ZT_TYPE_TEXT);
            return info;
        case ZT_AST_BYTES_EXPR:
            zt_type_dispose(info.type);
            info.type = zt_type_make(ZT_TYPE_BYTES);
            return info;
        case ZT_AST_BOOL_EXPR:
            zt_type_dispose(info.type);
            info.type = zt_type_make(ZT_TYPE_BOOL);
            return info;
        case ZT_AST_NONE_EXPR:
            zt_type_dispose(info.type);
            info.type = zt_type_make(ZT_TYPE_NONE_LITERAL);
            return info;
        case ZT_AST_SUCCESS_EXPR: {
            zt_expr_info inner_info;
            zt_type_list args = zt_type_list_make();
            if (node->as.success_expr.value != NULL) {
                const zt_type *expected_inner = (expected_type != NULL && expected_type->kind == ZT_TYPE_RESULT && expected_type->args.count == 2)
                    ? expected_type->args.items[0]
                    : NULL;
                inner_info = zt_checker_check_expression(checker, node->as.success_expr.value, scope, fn_ctx, expected_inner);
                zt_type_list_push(&args, zt_type_clone(inner_info.type));
                zt_expr_info_dispose(&inner_info);
            }
            zt_type_dispose(info.type);
            info.type = zt_type_make_with_args(ZT_TYPE_SUCCESS_WRAPPER, NULL, args);
            return info;
        }
        case ZT_AST_ERROR_EXPR: {
            zt_expr_info inner_info;
            zt_type_list args = zt_type_list_make();
            if (node->as.error_expr.value != NULL) {
                const zt_type *expected_inner = (expected_type != NULL && expected_type->kind == ZT_TYPE_RESULT && expected_type->args.count == 2)
                    ? expected_type->args.items[1]
                    : NULL;
                inner_info = zt_checker_check_expression(checker, node->as.error_expr.value, scope, fn_ctx, expected_inner);
                zt_type_list_push(&args, zt_type_clone(inner_info.type));
                zt_expr_info_dispose(&inner_info);
            }
            zt_type_dispose(info.type);
            info.type = zt_type_make_with_args(ZT_TYPE_ERROR_WRAPPER, NULL, args);
            return info;
        }
        case ZT_AST_IDENT_EXPR:
            binding = zt_binding_scope_lookup(scope, node->as.ident_expr.name, ZT_BINDING_VALUE);
            if (binding != NULL) {
                zt_type_dispose(info.type);
                info.type = zt_type_clone(binding->type);
            }
            return info;
        case ZT_AST_GROUPED_EXPR:
            zt_expr_info_dispose(&info);
            return zt_checker_check_expression(checker, node->as.grouped_expr.inner, scope, fn_ctx, expected_type);
        case ZT_AST_LIST_EXPR: {
            zt_type_list args = zt_type_list_make();
            zt_type *element_type = NULL;
            size_t i;
            if (node->as.list_expr.elements.count == 0 && expected_type != NULL && expected_type->kind == ZT_TYPE_LIST) {
                zt_type_dispose(info.type);
                info.type = zt_type_clone(expected_type);
                return info;
            }
            for (i = 0; i < node->as.list_expr.elements.count; i++) {
                zt_expr_info element_info = zt_checker_check_expression(checker, node->as.list_expr.elements.items[i], scope, fn_ctx, element_type);
                if (element_type == NULL) {
                    element_type = zt_type_clone(element_info.type);
                } else if (!zt_checker_same_or_contextually_assignable(checker, scope, element_type, &element_info, node->as.list_expr.elements.items[i]->span)) {
                    zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, node->as.list_expr.elements.items[i]->span, "list element type mismatch", element_type, element_info.type);
                }
                zt_expr_info_dispose(&element_info);
            }
            if (element_type == NULL) element_type = zt_type_make(ZT_TYPE_UNKNOWN);
            zt_type_list_push(&args, element_type);
            zt_type_dispose(info.type);
            info.type = zt_type_make_with_args(ZT_TYPE_LIST, NULL, args);
            return info;
        }
        case ZT_AST_MAP_EXPR: {
            zt_type_list args = zt_type_list_make();
            zt_type *key_type = NULL;
            zt_type *value_type = NULL;
            size_t i;
            if (node->as.map_expr.entries.count == 0 && expected_type != NULL && expected_type->kind == ZT_TYPE_MAP) {
                zt_type_dispose(info.type);
                info.type = zt_type_clone(expected_type);
                return info;
            }
            for (i = 0; i < node->as.map_expr.entries.count; i++) {
                zt_expr_info key_info = zt_checker_check_expression(checker, node->as.map_expr.entries.items[i].key, scope, fn_ctx, key_type);
                zt_expr_info value_info = zt_checker_check_expression(checker, node->as.map_expr.entries.items[i].value, scope, fn_ctx, value_type);
                if (key_type == NULL) key_type = zt_type_clone(key_info.type);
                if (value_type == NULL) value_type = zt_type_clone(value_info.type);
                if (key_type != NULL && !zt_checker_same_or_contextually_assignable(checker, scope, key_type, &key_info, node->as.map_expr.entries.items[i].key->span)) {
                    zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, node->as.map_expr.entries.items[i].key->span, "map key type mismatch", key_type, key_info.type);
                }
                if (value_type != NULL && !zt_checker_same_or_contextually_assignable(checker, scope, value_type, &value_info, node->as.map_expr.entries.items[i].value->span)) {
                    zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, node->as.map_expr.entries.items[i].value->span, "map value type mismatch", value_type, value_info.type);
                }
                zt_expr_info_dispose(&key_info);
                zt_expr_info_dispose(&value_info);
            }
            if (key_type == NULL) key_type = zt_type_make(ZT_TYPE_UNKNOWN);
            if (value_type == NULL) value_type = zt_type_make(ZT_TYPE_UNKNOWN);
            zt_type_list_push(&args, key_type);
            zt_type_list_push(&args, value_type);
            zt_type_dispose(info.type);
            info.type = zt_type_make_with_args(ZT_TYPE_MAP, NULL, args);
            if (!zt_checker_type_implements_trait(checker, scope, info.type->args.items[0], "Hashable") ||
                !zt_checker_type_implements_trait(checker, scope, info.type->args.items[0], "Equatable")) {
                char key_buf[256];
                zt_type_describe(info.type->args.items[0], key_buf, sizeof(key_buf));
                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_MAP_KEY_TYPE, node->span, "map key type '%s' must implement Hashable and Equatable", key_buf);
            }
            return info;
        }
        case ZT_AST_FIELD_EXPR:
            zt_expr_info_dispose(&info);
            return zt_checker_check_field_expr(checker, node, scope, fn_ctx);
        case ZT_AST_INDEX_EXPR: {
            zt_expr_info object_info = zt_checker_check_expression(checker, node->as.index_expr.object, scope, fn_ctx, NULL);
            zt_expr_info index_info = zt_checker_check_expression(checker, node->as.index_expr.index, scope, fn_ctx, NULL);
            int is_map_index = object_info.type != NULL &&
                object_info.type->kind == ZT_TYPE_MAP &&
                object_info.type->args.count == 2;
            zt_type_dispose(info.type);
            info.type = zt_type_make(ZT_TYPE_UNKNOWN);
            if (!is_map_index && !zt_type_is_integral(index_info.type)) {
                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_TYPE_MISMATCH, node->as.index_expr.index->span, "index expression must be integral");
            }
            if (object_info.type != NULL) {
                if (object_info.type->kind == ZT_TYPE_LIST && object_info.type->args.count == 1) {
                    zt_type_dispose(info.type);
                    info.type = zt_type_clone(object_info.type->args.items[0]);
                } else if (object_info.type->kind == ZT_TYPE_TEXT) {
                    zt_type_dispose(info.type);
                    info.type = zt_type_make(ZT_TYPE_TEXT);
                } else if (object_info.type->kind == ZT_TYPE_BYTES) {
                    zt_type_dispose(info.type);
                    info.type = zt_type_make(ZT_TYPE_UINT8);
                } else if (is_map_index) {
                    zt_expr_info key_actual = zt_checker_check_expression(checker, node->as.index_expr.index, scope, fn_ctx, object_info.type->args.items[0]);
                    if (!zt_checker_same_or_contextually_assignable(checker, scope, object_info.type->args.items[0], &key_actual, node->as.index_expr.index->span)) {
                        zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, node->as.index_expr.index->span, "map index key type mismatch", object_info.type->args.items[0], key_actual.type);
                    }
                    zt_expr_info_dispose(&key_actual);
                    zt_type_dispose(info.type);
                    info.type = zt_type_clone(object_info.type->args.items[1]);
                }
            }
            zt_expr_info_dispose(&object_info);
            zt_expr_info_dispose(&index_info);
            return info;
        }
        case ZT_AST_SLICE_EXPR: {
            zt_expr_info object_info = zt_checker_check_expression(checker, node->as.slice_expr.object, scope, fn_ctx, NULL);
            zt_type_dispose(info.type);
            info.type = zt_type_make(ZT_TYPE_UNKNOWN);

            if (node->as.slice_expr.start != NULL) {
                zt_expr_info start_info = zt_checker_check_expression(checker, node->as.slice_expr.start, scope, fn_ctx, NULL);
                if (!zt_type_is_integral(start_info.type)) {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_TYPE_MISMATCH, node->as.slice_expr.start->span, "slice start must be integral");
                }
                zt_expr_info_dispose(&start_info);
            }

            if (node->as.slice_expr.end != NULL) {
                zt_expr_info end_info = zt_checker_check_expression(checker, node->as.slice_expr.end, scope, fn_ctx, NULL);
                if (!zt_type_is_integral(end_info.type)) {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_TYPE_MISMATCH, node->as.slice_expr.end->span, "slice end must be integral");
                }
                zt_expr_info_dispose(&end_info);
            }

            if (object_info.type != NULL &&
                    ((object_info.type->kind == ZT_TYPE_LIST && object_info.type->args.count == 1) ||
                     object_info.type->kind == ZT_TYPE_TEXT ||
                     object_info.type->kind == ZT_TYPE_BYTES)) {
                zt_type_dispose(info.type);
                info.type = zt_type_clone(object_info.type);
            } else {
                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_TYPE, node->span, "slice requires text, bytes or list<T>");
            }

            zt_expr_info_dispose(&object_info);
            return info;
        }
        case ZT_AST_CALL_EXPR:
            zt_expr_info_dispose(&info);
            return zt_checker_check_call_expr(checker, node, scope, fn_ctx, expected_type);
        default:
            break;
    }

    if (node->kind == ZT_AST_UNARY_EXPR) {
        operand_info = zt_checker_check_expression(checker, node->as.unary_expr.operand, scope, fn_ctx, NULL);
        zt_type_dispose(info.type);
        info.type = zt_type_make(ZT_TYPE_UNKNOWN);
        switch (node->as.unary_expr.op) {
            case ZT_TOKEN_MINUS:
                if (!zt_type_is_numeric(operand_info.type)) {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_OPERATOR, node->span, "unary '-' requires a numeric operand");
                } else {
                    zt_type_dispose(info.type);
                    info.type = zt_type_clone(operand_info.type);
                    if (operand_info.has_int_value) {
                        info.has_int_value = 1;
                        info.int_value = -operand_info.int_value;
                    }
                }
                break;
            case ZT_TOKEN_NOT:
            case ZT_TOKEN_BANG:
                if (operand_info.type->kind != ZT_TYPE_BOOL) {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_OPERATOR, node->span, "'not' requires a bool operand");
                } else {
                    zt_type_dispose(info.type);
                    info.type = zt_type_make(ZT_TYPE_BOOL);
                }
                break;
            case ZT_TOKEN_QUESTION:
                if (operand_info.type->kind == ZT_TYPE_OPTIONAL && operand_info.type->args.count == 1) {
                    zt_type_dispose(info.type);
                    info.type = zt_type_clone(operand_info.type->args.items[0]);
                } else if (operand_info.type->kind == ZT_TYPE_RESULT && operand_info.type->args.count == 2) {
                    zt_type_dispose(info.type);
                    info.type = zt_type_clone(operand_info.type->args.items[0]);
                } else {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_OPERATOR, node->span, "'?' requires optional<T> or result<T, E>");
                }
                break;
            default:
                break;
        }
        zt_expr_info_dispose(&operand_info);
        return info;
    }

    if (node->kind == ZT_AST_BINARY_EXPR) {
        left_info = zt_checker_check_expression(checker, node->as.binary_expr.left, scope, fn_ctx, NULL);
        right_info = zt_checker_check_expression(checker, node->as.binary_expr.right, scope, fn_ctx, NULL);
        zt_type_dispose(info.type);
        info.type = zt_type_make(ZT_TYPE_UNKNOWN);
        switch (node->as.binary_expr.op) {
            case ZT_TOKEN_PLUS:
                if (left_info.type->kind == ZT_TYPE_TEXT && right_info.type->kind == ZT_TYPE_TEXT) {
                    zt_type_dispose(info.type);
                    info.type = zt_type_make(ZT_TYPE_TEXT);
                } else if (zt_type_is_integral(left_info.type) && zt_expr_matches_integral_type(&right_info, left_info.type)) {
                    long long folded;
                    long long right_value = right_info.int_value;
                    zt_type_dispose(info.type);
                    info.type = zt_type_clone(left_info.type);
                    if (!right_info.has_int_value && right_info.literal_text != NULL) {
                        zt_parse_signed_literal_value(right_info.literal_text, &right_value);
                    }
                    if (left_info.has_int_value && (right_info.has_int_value || right_info.is_int_literal) &&
                        zt_checker_compute_integral_binary(node->as.binary_expr.op, left_info.int_value, right_value, &folded)) {
                        info.has_int_value = 1;
                        info.int_value = folded;
                        if (!zt_int_value_fits_kind(folded, info.type->kind)) {
                            zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INTEGER_OVERFLOW, node->span, "integer overflow in arithmetic expression");
                        }
                    }
                } else if (zt_type_is_integral(right_info.type) && zt_expr_matches_integral_type(&left_info, right_info.type)) {
                    long long folded;
                    long long left_value = left_info.int_value;
                    zt_type_dispose(info.type);
                    info.type = zt_type_clone(right_info.type);
                    if (!left_info.has_int_value && left_info.literal_text != NULL) {
                        zt_parse_signed_literal_value(left_info.literal_text, &left_value);
                    }
                    if ((left_info.has_int_value || left_info.is_int_literal) && right_info.has_int_value &&
                        zt_checker_compute_integral_binary(node->as.binary_expr.op, left_value, right_info.int_value, &folded)) {
                        info.has_int_value = 1;
                        info.int_value = folded;
                        if (!zt_int_value_fits_kind(folded, info.type->kind)) {
                            zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INTEGER_OVERFLOW, node->span, "integer overflow in arithmetic expression");
                        }
                    }
                } else if (zt_type_is_numeric(left_info.type) && zt_type_equals(left_info.type, right_info.type)) {
                    zt_type_dispose(info.type);
                    info.type = zt_type_clone(left_info.type);
                } else if (zt_type_is_float(left_info.type) && right_info.is_float_literal) {
                    zt_type_dispose(info.type);
                    info.type = zt_type_clone(left_info.type);
                } else if (zt_type_is_float(right_info.type) && left_info.is_float_literal) {
                    zt_type_dispose(info.type);
                    info.type = zt_type_clone(right_info.type);
                } else {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_OPERATOR, node->span, "'+' requires text + text or compatible numeric types");
                }
                break;
            case ZT_TOKEN_MINUS:
            case ZT_TOKEN_STAR:
            case ZT_TOKEN_SLASH:
            case ZT_TOKEN_PERCENT:
                if (zt_type_is_integral(left_info.type) && zt_expr_matches_integral_type(&right_info, left_info.type)) {
                    long long folded;
                    long long right_value = right_info.int_value;
                    zt_type_dispose(info.type);
                    info.type = zt_type_clone(left_info.type);
                    if (!right_info.has_int_value && right_info.literal_text != NULL) {
                        zt_parse_signed_literal_value(right_info.literal_text, &right_value);
                    }
                    if ((node->as.binary_expr.op == ZT_TOKEN_SLASH || node->as.binary_expr.op == ZT_TOKEN_PERCENT) &&
                        (right_info.has_int_value || right_info.is_int_literal) && right_value == 0) {
                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_OPERATOR, node->span, "division or modulo by zero");
                    } else if (left_info.has_int_value && (right_info.has_int_value || right_info.is_int_literal) &&
                               zt_checker_compute_integral_binary(node->as.binary_expr.op, left_info.int_value, right_value, &folded)) {
                        info.has_int_value = 1;
                        info.int_value = folded;
                        if (!zt_int_value_fits_kind(folded, info.type->kind)) {
                            zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INTEGER_OVERFLOW, node->span, "integer overflow in arithmetic expression");
                        }
                    }
                } else if (zt_type_is_integral(right_info.type) && zt_expr_matches_integral_type(&left_info, right_info.type)) {
                    long long folded;
                    long long left_value = left_info.int_value;
                    zt_type_dispose(info.type);
                    info.type = zt_type_clone(right_info.type);
                    if (!left_info.has_int_value && left_info.literal_text != NULL) {
                        zt_parse_signed_literal_value(left_info.literal_text, &left_value);
                    }
                    if ((node->as.binary_expr.op == ZT_TOKEN_SLASH || node->as.binary_expr.op == ZT_TOKEN_PERCENT) &&
                        right_info.has_int_value && right_info.int_value == 0) {
                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_OPERATOR, node->span, "division or modulo by zero");
                    } else if ((left_info.has_int_value || left_info.is_int_literal) && right_info.has_int_value &&
                               zt_checker_compute_integral_binary(node->as.binary_expr.op, left_value, right_info.int_value, &folded)) {
                        info.has_int_value = 1;
                        info.int_value = folded;
                        if (!zt_int_value_fits_kind(folded, info.type->kind)) {
                            zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INTEGER_OVERFLOW, node->span, "integer overflow in arithmetic expression");
                        }
                    }
                } else if (zt_type_is_numeric(left_info.type) && zt_type_equals(left_info.type, right_info.type)) {
                    zt_type_dispose(info.type);
                    info.type = zt_type_clone(left_info.type);
                } else if (zt_type_is_float(left_info.type) && right_info.is_float_literal) {
                    zt_type_dispose(info.type);
                    info.type = zt_type_clone(left_info.type);
                } else if (zt_type_is_float(right_info.type) && left_info.is_float_literal) {
                    zt_type_dispose(info.type);
                    info.type = zt_type_clone(right_info.type);
                } else {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_OPERATOR, node->span, "arithmetic operators require compatible numeric types");
                }
                break;
            case ZT_TOKEN_EQEQ:
            case ZT_TOKEN_NEQ:
                if (!zt_checker_type_implements_trait(checker, scope, left_info.type, "Equatable") ||
                    (!zt_checker_same_or_contextually_assignable(checker, scope, left_info.type, &right_info, node->as.binary_expr.right->span) &&
                     !(zt_type_is_integral(right_info.type) && zt_expr_matches_integral_type(&left_info, right_info.type)))) {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_OPERATOR, node->span, "equality requires compatible Equatable operands");
                }
                zt_type_dispose(info.type);
                info.type = zt_type_make(ZT_TYPE_BOOL);
                break;
            case ZT_TOKEN_LT:
            case ZT_TOKEN_LTE:
            case ZT_TOKEN_GT:
            case ZT_TOKEN_GTE:
                if ((zt_type_is_numeric(left_info.type) && zt_type_equals(left_info.type, right_info.type)) ||
                    (zt_type_is_integral(left_info.type) && zt_expr_matches_integral_type(&right_info, left_info.type)) ||
                    (zt_type_is_integral(right_info.type) && zt_expr_matches_integral_type(&left_info, right_info.type)) ||
                    (left_info.type->kind == ZT_TYPE_TEXT && right_info.type->kind == ZT_TYPE_TEXT)) {
                    zt_type_dispose(info.type);
                    info.type = zt_type_make(ZT_TYPE_BOOL);
                } else {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_OPERATOR, node->span, "comparison requires two equal numeric operands or two text operands");
                }
                break;
            case ZT_TOKEN_AND:
            case ZT_TOKEN_OR:
                if (left_info.type->kind == ZT_TYPE_BOOL && right_info.type->kind == ZT_TYPE_BOOL) {
                    zt_type_dispose(info.type);
                    info.type = zt_type_make(ZT_TYPE_BOOL);
                } else {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_OPERATOR, node->span, "logical operators require bool operands");
                }
                break;
            default:
                break;
        }
        zt_expr_info_dispose(&left_info);
        zt_expr_info_dispose(&right_info);
        return info;
    }

    return info;
}

static void zt_checker_check_statement(zt_checker *checker, const zt_ast_node *stmt, zt_binding_scope *scope, zt_function_context *fn_ctx) {
    zt_type *decl_type;
    zt_expr_info expr_info;
    zt_binding *binding;
    size_t i;

    if (stmt == NULL) return;

    switch (stmt->kind) {
        case ZT_AST_CONST_DECL:
            decl_type = zt_checker_resolve_type(checker, stmt->as.const_decl.type_node, scope);
            expr_info = zt_checker_check_expression(checker, stmt->as.const_decl.init_value, scope, fn_ctx, decl_type);
            if (!zt_checker_same_or_contextually_assignable(checker, scope, decl_type, &expr_info, stmt->as.const_decl.init_value != NULL ? stmt->as.const_decl.init_value->span : stmt->span)) {
                zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, stmt->span, "const initializer type mismatch", decl_type, expr_info.type);
            }
            zt_binding_scope_declare(scope, ZT_BINDING_VALUE, stmt->as.const_decl.name, zt_type_clone(decl_type), 0);
            zt_expr_info_dispose(&expr_info);
            zt_type_dispose(decl_type);
            break;
        case ZT_AST_VAR_DECL:
            decl_type = zt_checker_resolve_type(checker, stmt->as.var_decl.type_node, scope);
            expr_info = zt_checker_check_expression(checker, stmt->as.var_decl.init_value, scope, fn_ctx, decl_type);
            if (!zt_checker_same_or_contextually_assignable(checker, scope, decl_type, &expr_info, stmt->as.var_decl.init_value != NULL ? stmt->as.var_decl.init_value->span : stmt->span)) {
                zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, stmt->span, "var initializer type mismatch", decl_type, expr_info.type);
            }
            zt_binding_scope_declare(scope, ZT_BINDING_VALUE, stmt->as.var_decl.name, zt_type_clone(decl_type), 1);
            zt_expr_info_dispose(&expr_info);
            zt_type_dispose(decl_type);
            break;
        case ZT_AST_ASSIGN_STMT:
            binding = zt_binding_scope_lookup(scope, stmt->as.assign_stmt.name, ZT_BINDING_VALUE);
            if (binding != NULL) {
                expr_info = zt_checker_check_expression(checker, stmt->as.assign_stmt.value, scope, fn_ctx, binding->type);
                if (!binding->is_mutable) {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_CONST_REASSIGNMENT, stmt->span, "cannot assign to immutable binding '%s'", stmt->as.assign_stmt.name);
                } else if (!zt_checker_same_or_contextually_assignable(checker, scope, binding->type, &expr_info, stmt->as.assign_stmt.value->span)) {
                    zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, stmt->span, "assignment type mismatch", binding->type, expr_info.type);
                }
                zt_expr_info_dispose(&expr_info);
            }
            break;
        case ZT_AST_FIELD_ASSIGN_STMT: {
            zt_expr_info object_info = zt_checker_check_expression(checker, stmt->as.field_assign_stmt.object, scope, fn_ctx, NULL);
            zt_type *field_type = NULL;
            const zt_ast_node *struct_decl = NULL;
            const zt_ast_node *field_decl = NULL;
            zt_binding_scope type_scope;
            if (!zt_checker_expression_is_mutable_target(stmt->as.field_assign_stmt.object, scope, fn_ctx)) {
                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_MUTATION, stmt->span, "field assignment requires a mutable receiver");
            }
            if (object_info.type != NULL && object_info.type->kind == ZT_TYPE_USER) {
                struct_decl = zt_catalog_find_decl(&checker->catalog, zt_type_base_name(object_info.type));
                if (struct_decl != NULL && struct_decl->kind == ZT_AST_STRUCT_DECL) {
                    field_decl = zt_checker_find_struct_field_decl(struct_decl, stmt->as.field_assign_stmt.field_name);
                    if (field_decl != NULL) {
                        zt_binding_scope_init(&type_scope, scope);
                        zt_checker_bind_struct_field_type_params(&type_scope, struct_decl, object_info.type);
                        field_type = zt_checker_resolve_type(checker, field_decl->as.struct_field.type_node, &type_scope);
                        zt_binding_scope_dispose(&type_scope);
                    }
                }
            }
            if (field_type != NULL) {
                expr_info = zt_checker_check_expression(checker, stmt->as.field_assign_stmt.value, scope, fn_ctx, field_type);
                if (!zt_checker_same_or_contextually_assignable(checker, scope, field_type, &expr_info, stmt->as.field_assign_stmt.value->span)) {
                    zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, stmt->span, "field assignment type mismatch", field_type, expr_info.type);
                }
                zt_expr_info_dispose(&expr_info);
                zt_type_dispose(field_type);
            }
            zt_expr_info_dispose(&object_info);
            break;
        }
        case ZT_AST_INDEX_ASSIGN_STMT: {
            zt_expr_info object_info = zt_checker_check_expression(checker, stmt->as.index_assign_stmt.object, scope, fn_ctx, NULL);
            zt_type *expected_value = NULL;
            int is_map_target = object_info.type != NULL &&
                object_info.type->kind == ZT_TYPE_MAP &&
                object_info.type->args.count == 2;
            if (!zt_checker_expression_is_mutable_target(stmt->as.index_assign_stmt.object, scope, fn_ctx)) {
                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_MUTATION, stmt->span, "indexed assignment requires a mutable receiver");
            }
            expr_info = zt_checker_check_expression(checker, stmt->as.index_assign_stmt.index, scope, fn_ctx, NULL);
            if (!is_map_target && !zt_type_is_integral(expr_info.type)) {
                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_TYPE_MISMATCH, stmt->as.index_assign_stmt.index->span, "index assignment requires an integral index");
            }
            zt_expr_info_dispose(&expr_info);
            if (object_info.type != NULL) {
                if (object_info.type->kind == ZT_TYPE_LIST && object_info.type->args.count == 1) {
                    expected_value = zt_type_clone(object_info.type->args.items[0]);
                } else if (is_map_target) {
                    zt_expr_info key_info = zt_checker_check_expression(checker, stmt->as.index_assign_stmt.index, scope, fn_ctx, object_info.type->args.items[0]);
                    if (!zt_checker_same_or_contextually_assignable(checker, scope, object_info.type->args.items[0], &key_info, stmt->as.index_assign_stmt.index->span)) {
                        zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, stmt->span, "map key assignment type mismatch", object_info.type->args.items[0], key_info.type);
                    }
                    zt_expr_info_dispose(&key_info);
                    expected_value = zt_type_clone(object_info.type->args.items[1]);
                }
            }
            if (expected_value != NULL) {
                expr_info = zt_checker_check_expression(checker, stmt->as.index_assign_stmt.value, scope, fn_ctx, expected_value);
                if (!zt_checker_same_or_contextually_assignable(checker, scope, expected_value, &expr_info, stmt->as.index_assign_stmt.value->span)) {
                    zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, stmt->span, "indexed assignment type mismatch", expected_value, expr_info.type);
                }
                zt_expr_info_dispose(&expr_info);
                zt_type_dispose(expected_value);
            }
            zt_expr_info_dispose(&object_info);
            break;
        }
        case ZT_AST_RETURN_STMT:
            expr_info = zt_checker_check_expression(checker, stmt->as.return_stmt.value, scope, fn_ctx, fn_ctx != NULL ? fn_ctx->return_type : NULL);
            if (fn_ctx != NULL && fn_ctx->return_type != NULL) {
                if (stmt->as.return_stmt.value == NULL) {
                    if (fn_ctx->return_type->kind != ZT_TYPE_VOID) {
                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_RETURN, stmt->span, "return without value in non-void function");
                    }
                } else if (!zt_checker_same_or_contextually_assignable(checker, scope, fn_ctx->return_type, &expr_info, stmt->as.return_stmt.value->span)) {
                    zt_checker_diag_type(checker, ZT_DIAG_INVALID_RETURN, stmt->span, "return type mismatch", fn_ctx->return_type, expr_info.type);
                }
            }
            zt_expr_info_dispose(&expr_info);
            break;
        case ZT_AST_IF_STMT:
            expr_info = zt_checker_check_expression(checker, stmt->as.if_stmt.condition, scope, fn_ctx, NULL);
            if (expr_info.type->kind != ZT_TYPE_BOOL) {
                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_CONDITION_TYPE, stmt->as.if_stmt.condition->span, "if condition must be bool");
            }
            zt_expr_info_dispose(&expr_info);
            zt_checker_check_block(checker, stmt->as.if_stmt.then_block, scope, fn_ctx);
            if (stmt->as.if_stmt.else_block != NULL) {
                if (stmt->as.if_stmt.else_block->kind == ZT_AST_BLOCK) {
                    zt_checker_check_block(checker, stmt->as.if_stmt.else_block, scope, fn_ctx);
                } else {
                    zt_checker_check_statement(checker, stmt->as.if_stmt.else_block, scope, fn_ctx);
                }
            }
            break;
        case ZT_AST_WHILE_STMT:
            expr_info = zt_checker_check_expression(checker, stmt->as.while_stmt.condition, scope, fn_ctx, NULL);
            if (expr_info.type->kind != ZT_TYPE_BOOL) {
                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_CONDITION_TYPE, stmt->as.while_stmt.condition->span, "while condition must be bool");
            }
            zt_expr_info_dispose(&expr_info);
            zt_checker_check_block(checker, stmt->as.while_stmt.body, scope, fn_ctx);
            break;
        case ZT_AST_REPEAT_STMT:
            expr_info = zt_checker_check_expression(checker, stmt->as.repeat_stmt.count, scope, fn_ctx, NULL);
            if (!zt_type_is_integral(expr_info.type)) {
                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_TYPE_MISMATCH, stmt->as.repeat_stmt.count->span, "repeat count must be integral");
            }
            zt_expr_info_dispose(&expr_info);
            zt_checker_check_block(checker, stmt->as.repeat_stmt.body, scope, fn_ctx);
            break;
        case ZT_AST_FOR_STMT: {
            zt_binding_scope loop_scope;
            zt_expr_info iterable_info = zt_checker_check_expression(checker, stmt->as.for_stmt.iterable, scope, fn_ctx, NULL);
            zt_binding_scope_init(&loop_scope, scope);
            if (iterable_info.type != NULL) {
                if (iterable_info.type->kind == ZT_TYPE_LIST && iterable_info.type->args.count == 1) {
                    zt_binding_scope_declare(&loop_scope, ZT_BINDING_VALUE, stmt->as.for_stmt.item_name, zt_type_clone(iterable_info.type->args.items[0]), 1);
                    if (stmt->as.for_stmt.index_name != NULL) {
                        zt_binding_scope_declare(&loop_scope, ZT_BINDING_VALUE, stmt->as.for_stmt.index_name, zt_type_make(ZT_TYPE_INT), 1);
                    }
                } else if (iterable_info.type->kind == ZT_TYPE_MAP && iterable_info.type->args.count == 2) {
                    zt_binding_scope_declare(&loop_scope, ZT_BINDING_VALUE, stmt->as.for_stmt.item_name, zt_type_clone(iterable_info.type->args.items[0]), 1);
                    if (stmt->as.for_stmt.index_name != NULL) {
                        zt_binding_scope_declare(&loop_scope, ZT_BINDING_VALUE, stmt->as.for_stmt.index_name, zt_type_clone(iterable_info.type->args.items[1]), 1);
                    }
                } else if (iterable_info.type->kind == ZT_TYPE_TEXT) {
                    zt_binding_scope_declare(&loop_scope, ZT_BINDING_VALUE, stmt->as.for_stmt.item_name, zt_type_make(ZT_TYPE_TEXT), 1);
                    if (stmt->as.for_stmt.index_name != NULL) {
                        zt_binding_scope_declare(&loop_scope, ZT_BINDING_VALUE, stmt->as.for_stmt.index_name, zt_type_make(ZT_TYPE_INT), 1);
                    }
                } else {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_TYPE, stmt->as.for_stmt.iterable->span, "for loop requires iterable of type list<T>, map<K, V> or text");
                }
            }
            zt_checker_check_block(checker, stmt->as.for_stmt.body, &loop_scope, fn_ctx);
            zt_binding_scope_dispose(&loop_scope);
            zt_expr_info_dispose(&iterable_info);
            break;
        }
        case ZT_AST_MATCH_STMT: {
            zt_expr_info subject_info = zt_checker_check_expression(checker, stmt->as.match_stmt.subject, scope, fn_ctx, NULL);
            const zt_ast_node *subject_decl = NULL;
            int subject_is_enum = 0;
            int has_default = 0;
            int *seen_variants = NULL;
            size_t variant_count = 0;
            size_t c;

            if (subject_info.type != NULL && subject_info.type->kind == ZT_TYPE_USER) {
                subject_decl = zt_catalog_find_decl(&checker->catalog, zt_type_base_name(subject_info.type));
                if (subject_decl != NULL && subject_decl->kind == ZT_AST_ENUM_DECL) {
                    subject_is_enum = 1;
                    variant_count = subject_decl->as.enum_decl.variants.count;
                    if (variant_count > 0) {
                        seen_variants = (int *)calloc(variant_count, sizeof(int));
                    }
                }
            }

            for (c = 0; c < stmt->as.match_stmt.cases.count; c++) {
                const zt_ast_node *case_node = stmt->as.match_stmt.cases.items[c];
                zt_binding_scope case_scope;
                size_t p;

                if (case_node == NULL) continue;
                zt_binding_scope_init(&case_scope, scope);

                if (case_node->as.match_case.is_default) {
                    if (has_default) {
                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, case_node->span, "duplicate case default in match");
                    }
                    has_default = 1;
                    if (c + 1 < stmt->as.match_stmt.cases.count) {
                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, case_node->span, "case default must be the final case in match");
                    }
                    zt_checker_check_block(checker, case_node->as.match_case.body, &case_scope, fn_ctx);
                    zt_binding_scope_dispose(&case_scope);
                    continue;
                }

                if (subject_is_enum) {
                    for (p = 0; p < case_node->as.match_case.patterns.count; p++) {
                        const zt_ast_node *pattern = case_node->as.match_case.patterns.items[p];
                        const zt_ast_node *variant_decl = NULL;
                        size_t variant_index = 0;
                        size_t field_count = 0;
                        char pattern_enum_name[256];

                        if (pattern == NULL) continue;

                        if (pattern->kind == ZT_AST_FIELD_EXPR &&
                            pattern->as.field_expr.object != NULL &&
                            pattern->as.field_expr.field_name != NULL &&
                            zt_checker_build_qualified_name(pattern->as.field_expr.object, pattern_enum_name, sizeof(pattern_enum_name)) &&
                            strcmp(pattern_enum_name, subject_decl->as.enum_decl.name) == 0) {
                            variant_decl = zt_checker_find_enum_variant_decl(subject_decl, pattern->as.field_expr.field_name, &variant_index);
                            if (variant_decl == NULL) {
                                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, pattern->span, "unknown enum variant '%s.%s' in match", subject_decl->as.enum_decl.name, pattern->as.field_expr.field_name);
                                continue;
                            }
                            field_count = variant_decl->as.enum_variant.fields.count;
                            if (field_count > 0) {
                                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, pattern->span, "enum variant '%s.%s' requires payload bindings in this match pattern", subject_decl->as.enum_decl.name, pattern->as.field_expr.field_name);
                                continue;
                            }
                            if (seen_variants != NULL && variant_index < variant_count) seen_variants[variant_index] = 1;
                            continue;
                        }

                        if (pattern->kind == ZT_AST_CALL_EXPR &&
                            pattern->as.call_expr.callee != NULL &&
                            pattern->as.call_expr.callee->kind == ZT_AST_FIELD_EXPR &&
                            pattern->as.call_expr.callee->as.field_expr.object != NULL &&
                            pattern->as.call_expr.callee->as.field_expr.field_name != NULL &&
                            zt_checker_build_qualified_name(pattern->as.call_expr.callee->as.field_expr.object, pattern_enum_name, sizeof(pattern_enum_name)) &&
                            strcmp(pattern_enum_name, subject_decl->as.enum_decl.name) == 0) {
                            const zt_ast_node *callee_field = pattern->as.call_expr.callee;
                            variant_decl = zt_checker_find_enum_variant_decl(subject_decl, callee_field->as.field_expr.field_name, &variant_index);
                            if (variant_decl == NULL) {
                                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, pattern->span, "unknown enum variant '%s.%s' in match", subject_decl->as.enum_decl.name, callee_field->as.field_expr.field_name);
                                continue;
                            }

                            field_count = variant_decl->as.enum_variant.fields.count;
                            if (pattern->as.call_expr.named_args.count != 0) {
                                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, pattern->span, "enum match payload binding uses positional identifiers only");
                            }
                            if (pattern->as.call_expr.positional_args.count != field_count) {
                                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, pattern->span, "enum variant '%s.%s' expects %zu payload bindings in match", subject_decl->as.enum_decl.name, callee_field->as.field_expr.field_name, field_count);
                            }
                            if (case_node->as.match_case.patterns.count > 1 && field_count > 0) {
                                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, pattern->span, "payload enum case cannot be combined with other patterns in the same case");
                            }

                            {
                                size_t arg_index;
                                size_t bind_count = pattern->as.call_expr.positional_args.count < field_count
                                    ? pattern->as.call_expr.positional_args.count
                                    : field_count;
                                for (arg_index = 0; arg_index < bind_count; arg_index++) {
                                    const zt_ast_node *binding_expr = pattern->as.call_expr.positional_args.items[arg_index];
                                    const zt_ast_node *field_node = variant_decl->as.enum_variant.fields.items[arg_index];
                                    const zt_ast_node *field_type_node = zt_checker_enum_variant_field_type_node(field_node);
                                    zt_type *field_type = zt_checker_resolve_type(checker, field_type_node, scope);
                                    if (binding_expr == NULL || binding_expr->kind != ZT_AST_IDENT_EXPR) {
                                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, binding_expr != NULL ? binding_expr->span : pattern->span, "enum payload binding must use identifiers");
                                        zt_type_dispose(field_type);
                                        continue;
                                    }
                                    if (zt_binding_scope_lookup(&case_scope, binding_expr->as.ident_expr.name, ZT_BINDING_VALUE) != NULL) {
                                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, binding_expr->span, "duplicate or shadowed enum payload binding '%s'", binding_expr->as.ident_expr.name);
                                        zt_type_dispose(field_type);
                                        continue;
                                    }
                                    zt_binding_scope_declare(&case_scope, ZT_BINDING_VALUE, binding_expr->as.ident_expr.name, field_type, 0);
                                }
                            }

                            if (seen_variants != NULL && variant_index < variant_count) seen_variants[variant_index] = 1;
                            continue;
                        }

                        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_ARGUMENT, pattern->span, "invalid enum match pattern; use '%s.Variant' or '%s.Variant(binding, ...)'", subject_decl->as.enum_decl.name, subject_decl->as.enum_decl.name);
                    }

                    zt_checker_check_block(checker, case_node->as.match_case.body, &case_scope, fn_ctx);
                    zt_binding_scope_dispose(&case_scope);
                    continue;
                }

                for (p = 0; p < case_node->as.match_case.patterns.count; p++) {
                    zt_expr_info pattern_info = zt_checker_check_expression(checker, case_node->as.match_case.patterns.items[p], &case_scope, fn_ctx, subject_info.type);
                    if (!zt_checker_same_or_contextually_assignable(checker, &case_scope, subject_info.type, &pattern_info, case_node->as.match_case.patterns.items[p]->span)) {
                        zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, case_node->as.match_case.patterns.items[p]->span, "match pattern type mismatch", subject_info.type, pattern_info.type);
                    }
                    zt_expr_info_dispose(&pattern_info);
                }

                zt_checker_check_block(checker, case_node->as.match_case.body, &case_scope, fn_ctx);
                zt_binding_scope_dispose(&case_scope);
            }

            if (subject_is_enum && !has_default && subject_decl != NULL && subject_decl->kind == ZT_AST_ENUM_DECL) {
                size_t total_variants = subject_decl->as.enum_decl.variants.count;
                int all_covered = 1;
                for (i = 0; i < total_variants; i++) {
                    if (!seen_variants[i]) {
                        all_covered = 0;
                        break;
                    }
                }
                if (!all_covered) {
                    zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_NON_EXHAUSTIVE_MATCH, stmt->span, "match on enum '%s' is not exhaustive", subject_decl->as.enum_decl.name);
                    zt_diag_list_add_severity(&checker->result->diagnostics, ZT_DIAG_NON_EXHAUSTIVE_MATCH, ZT_DIAG_SEVERITY_NOTE, stmt->span, "add missing variants or a 'default' case");
                }
            }

            free(seen_variants);
            zt_expr_info_dispose(&subject_info);
            break;
        }
        case ZT_AST_EXPR_STMT:
            expr_info = zt_checker_check_expression(checker, stmt->as.expr_stmt.expr, scope, fn_ctx, NULL);
            zt_expr_info_dispose(&expr_info);
            break;
        case ZT_AST_BREAK_STMT:
        case ZT_AST_CONTINUE_STMT:
            break;
        default:
            break;
    }
}

static void zt_checker_check_block(zt_checker *checker, const zt_ast_node *block, zt_binding_scope *parent_scope, zt_function_context *fn_ctx) {
    zt_binding_scope local_scope;
    size_t i;

    if (block == NULL || block->kind != ZT_AST_BLOCK) return;
    zt_binding_scope_init(&local_scope, parent_scope);
    for (i = 0; i < block->as.block.statements.count; i++) {
        zt_checker_check_statement(checker, block->as.block.statements.items[i], &local_scope, fn_ctx);
    }
    zt_binding_scope_dispose(&local_scope);
}

static void zt_checker_check_where_clause(zt_checker *checker, const zt_ast_node *where_clause, zt_binding_scope *scope, const zt_type *subject_type) {
    zt_binding_scope where_scope;
    zt_expr_info cond_info;

    if (where_clause == NULL || where_clause->kind != ZT_AST_WHERE_CLAUSE || subject_type == NULL) return;
    zt_binding_scope_init(&where_scope, scope);
    zt_binding_scope_declare(&where_scope, ZT_BINDING_VALUE, "it", zt_type_clone(subject_type), 0);
    cond_info = zt_checker_check_expression(checker, where_clause->as.where_clause.condition, &where_scope, NULL, NULL);
    if (cond_info.type->kind != ZT_TYPE_BOOL) {
        zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_CONDITION_TYPE, where_clause->span, "where condition must be bool");
    }
    zt_expr_info_dispose(&cond_info);
    zt_binding_scope_dispose(&where_scope);
}

static void zt_checker_check_generic_constraints(zt_checker *checker, const zt_ast_node_list *constraints, zt_binding_scope *scope) {
    size_t i;
    for (i = 0; constraints != NULL && i < constraints->count; i++) {
        const zt_ast_node *constraint = constraints->items[i];
        zt_type *trait_type;
        const char *trait_name = NULL;
        if (constraint == NULL || constraint->kind != ZT_AST_GENERIC_CONSTRAINT) continue;
        trait_type = zt_checker_resolve_type(checker, constraint->as.generic_constraint.trait_type, scope);
        if (constraint->as.generic_constraint.trait_type != NULL) {
            if (constraint->as.generic_constraint.trait_type->kind == ZT_AST_TYPE_SIMPLE) {
                trait_name = constraint->as.generic_constraint.trait_type->as.type_simple.name;
            } else if (constraint->as.generic_constraint.trait_type->kind == ZT_AST_TYPE_GENERIC) {
                trait_name = constraint->as.generic_constraint.trait_type->as.type_generic.name;
            }
        }
        if (trait_name != NULL && !zt_type_is_core_trait_name(trait_name)) {
            const zt_ast_node *decl = zt_catalog_find_decl(&checker->catalog, trait_name);
            if (decl == NULL || decl->kind != ZT_AST_TRAIT_DECL) {
                zt_diag_list_add(&checker->result->diagnostics, ZT_DIAG_INVALID_TYPE, constraint->span, "constraint trait '%s' is not a trait", trait_name);
            }
        }
        zt_type_dispose(trait_type);
    }
}

static void zt_checker_check_func_like(zt_checker *checker, const zt_ast_node *func_decl, zt_binding_scope *parent_scope, zt_type *self_type, int in_mutating_method) {
    zt_binding_scope scope;
    zt_function_context fn_ctx;
    size_t i;

    zt_binding_scope_init(&scope, parent_scope);
    zt_checker_register_type_params(checker, &func_decl->as.func_decl.type_params, &func_decl->as.func_decl.constraints, &scope);
    zt_checker_check_generic_constraints(checker, &func_decl->as.func_decl.constraints, &scope);

    for (i = 0; i < func_decl->as.func_decl.params.count; i++) {
        const zt_ast_node *param = func_decl->as.func_decl.params.items[i];
        zt_type *param_type = zt_checker_resolve_type(checker, param->as.param.type_node, &scope);
        if (param->as.param.default_value != NULL) {
            zt_expr_info default_info = zt_checker_check_expression(checker, param->as.param.default_value, &scope, NULL, param_type);
            if (!zt_checker_same_or_contextually_assignable(checker, &scope, param_type, &default_info, param->as.param.default_value->span)) {
                zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, param->as.param.default_value->span, "default parameter type mismatch", param_type, default_info.type);
            }
            zt_expr_info_dispose(&default_info);
        }
        zt_binding_scope_declare(&scope, ZT_BINDING_VALUE, param->as.param.name, zt_type_clone(param_type), 0);
        zt_type_dispose(param_type);
    }

    fn_ctx.return_type = zt_checker_resolve_type(checker, func_decl->as.func_decl.return_type, &scope);
    fn_ctx.self_type = self_type;
    fn_ctx.in_mutating_method = in_mutating_method;
    fn_ctx.scope = &scope;

    if (self_type != NULL) {
        zt_binding_scope_declare(&scope, ZT_BINDING_VALUE, "self", zt_type_clone(self_type), in_mutating_method);
    }

    zt_checker_check_block(checker, func_decl->as.func_decl.body, &scope, &fn_ctx);

    zt_type_dispose(fn_ctx.return_type);
    zt_binding_scope_dispose(&scope);
}

static void zt_checker_check_decl(zt_checker *checker, const zt_ast_node *decl) {
    zt_binding_scope scope;
    size_t i;

    if (decl == NULL) return;
    zt_binding_scope_init(&scope, NULL);

    switch (decl->kind) {
        case ZT_AST_FUNC_DECL:
            zt_checker_check_func_like(checker, decl, &scope, NULL, 0);
            break;
        case ZT_AST_STRUCT_DECL:
            zt_checker_register_type_params(checker, &decl->as.struct_decl.type_params, &decl->as.struct_decl.constraints, &scope);
            zt_checker_check_generic_constraints(checker, &decl->as.struct_decl.constraints, &scope);
            for (i = 0; i < decl->as.struct_decl.fields.count; i++) {
                const zt_ast_node *field = decl->as.struct_decl.fields.items[i];
                zt_type *field_type = zt_checker_resolve_type(checker, field->as.struct_field.type_node, &scope);
                if (field->as.struct_field.default_value != NULL) {
                    zt_expr_info default_info = zt_checker_check_expression(checker, field->as.struct_field.default_value, &scope, NULL, field_type);
                    if (!zt_checker_same_or_contextually_assignable(checker, &scope, field_type, &default_info, field->as.struct_field.default_value->span)) {
                        zt_checker_diag_type(checker, ZT_DIAG_TYPE_MISMATCH, field->as.struct_field.default_value->span, "field default type mismatch", field_type, default_info.type);
                    }
                    zt_expr_info_dispose(&default_info);
                }
                zt_checker_check_where_clause(checker, field->as.struct_field.where_clause, &scope, field_type);
                zt_type_dispose(field_type);
            }
            break;
        case ZT_AST_TRAIT_DECL:
            zt_checker_register_type_params(checker, &decl->as.trait_decl.type_params, &decl->as.trait_decl.constraints, &scope);
            zt_checker_check_generic_constraints(checker, &decl->as.trait_decl.constraints, &scope);
            for (i = 0; i < decl->as.trait_decl.methods.count; i++) {
                const zt_ast_node *method = decl->as.trait_decl.methods.items[i];
                size_t p;
                for (p = 0; p < method->as.trait_method.params.count; p++) {
                    zt_type_dispose(zt_checker_resolve_type(checker, method->as.trait_method.params.items[p]->as.param.type_node, &scope));
                }
                zt_type_dispose(zt_checker_resolve_type(checker, method->as.trait_method.return_type, &scope));
            }
            break;
        case ZT_AST_ENUM_DECL:
            zt_checker_register_type_params(checker, &decl->as.enum_decl.type_params, &decl->as.enum_decl.constraints, &scope);
            zt_checker_check_generic_constraints(checker, &decl->as.enum_decl.constraints, &scope);
            for (i = 0; i < decl->as.enum_decl.variants.count; i++) {
                const zt_ast_node *variant = decl->as.enum_decl.variants.items[i];
                size_t f;
                for (f = 0; f < variant->as.enum_variant.fields.count; f++) {
                    const zt_ast_node *field = variant->as.enum_variant.fields.items[f];
                    if (field != NULL && field->kind == ZT_AST_PARAM) {
                        zt_type_dispose(zt_checker_resolve_type(checker, field->as.param.type_node, &scope));
                    }
                }
            }
            break;
        case ZT_AST_APPLY_DECL: {
            zt_type *self_type = NULL;
            if (decl->as.apply_decl.trait_name == NULL) {
                zt_checker_register_type_params(checker, &decl->as.apply_decl.target_type_params, &decl->as.apply_decl.constraints, &scope);
                self_type = zt_checker_resolve_user_type(checker, decl->as.apply_decl.target_name, zt_type_list_make(), decl->span);
                if (self_type != NULL && self_type->kind == ZT_TYPE_USER) {
                    size_t t;
                    for (t = 0; t < decl->as.apply_decl.target_type_params.count; t++) {
                        const zt_ast_node *tp = decl->as.apply_decl.target_type_params.items[t];
                        zt_binding *binding = zt_binding_scope_lookup(&scope, tp->as.type_simple.name, ZT_BINDING_TYPE_PARAM);
                        if (binding != NULL) zt_type_list_push(&self_type->args, zt_type_clone(binding->type));
                    }
                }
            } else {
                zt_type_list target_args = zt_type_list_make();
                size_t t;
                for (t = 0; t < decl->as.apply_decl.target_type_params.count; t++) {
                    zt_type_list_push(&target_args, zt_checker_resolve_type(checker, decl->as.apply_decl.target_type_params.items[t], &scope));
                }
                self_type = zt_checker_resolve_user_type(checker, decl->as.apply_decl.target_name, target_args, decl->span);
            }
            zt_checker_check_generic_constraints(checker, &decl->as.apply_decl.constraints, &scope);
            for (i = 0; i < decl->as.apply_decl.methods.count; i++) {
                const zt_ast_node *method = decl->as.apply_decl.methods.items[i];
                int is_mutating = method != NULL && method->as.func_decl.is_mutating;
                zt_checker_check_func_like(checker, method, &scope, self_type != NULL ? zt_type_clone(self_type) : NULL, is_mutating);
            }
            zt_type_dispose(self_type);
            break;
        }
        case ZT_AST_EXTERN_DECL:
            for (i = 0; i < decl->as.extern_decl.functions.count; i++) {
                const zt_ast_node *func = decl->as.extern_decl.functions.items[i];
                size_t p;
                for (p = 0; p < func->as.func_decl.params.count; p++) {
                    zt_type_dispose(zt_checker_resolve_type(checker, func->as.func_decl.params.items[p]->as.param.type_node, &scope));
                }
                zt_type_dispose(zt_checker_resolve_type(checker, func->as.func_decl.return_type, &scope));
            }
            break;
        default:
            break;
    }

    zt_binding_scope_dispose(&scope);
}

zt_check_result zt_check_file(const zt_ast_node *root) {
    zt_check_result result;
    zt_checker checker;
    size_t i;

    result.diagnostics = zt_diag_list_make();
    checker.root = root;
    checker.result = &result;
    zt_catalog_init(&checker.catalog);
    zt_catalog_build(&checker.catalog, root);

    if (root != NULL && root->kind == ZT_AST_FILE) {
        for (i = 0; i < root->as.file.declarations.count; i++) {
            zt_checker_check_decl(&checker, root->as.file.declarations.items[i]);
        }
    }

    zt_catalog_dispose(&checker.catalog);
    return result;
}

void zt_check_result_dispose(zt_check_result *result) {
    if (result == NULL) return;
    zt_diag_list_dispose(&result->diagnostics);
}

