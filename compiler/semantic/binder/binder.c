#include "compiler/semantic/binder/binder.h"

#include <string.h>

typedef struct zt_binder {
    zt_bind_result *result;
} zt_binder;

static int zt_is_builtin_type_name(const char *name) {
    static const char *builtin_names[] = {
        "bool",
        "int", "int8", "int16", "int32", "int64",
        "uint8", "uint16", "uint32", "uint64",
        "float", "float32", "float64",
        "text",
        "bytes",
        "void",
        "list",
        "map",
        "optional",
        "result",
        NULL
    };
    size_t i;

    if (name == NULL) return 0;
    for (i = 0; builtin_names[i] != NULL; i++) {
        if (strcmp(name, builtin_names[i]) == 0) return 1;
    }
    return 0;
}

static const char *zt_import_local_name(const char *path) {
    const char *last_dot;
    if (path == NULL) return NULL;
    last_dot = strrchr(path, '.');
    return last_dot != NULL ? last_dot + 1 : path;
}

static void zt_bind_declare_name(zt_binder *binder, zt_scope *scope, zt_symbol_kind kind, const char *name, zt_source_span span, int is_implicit) {
    if (scope == NULL || name == NULL) return;

    if (zt_scope_lookup_current(scope, name) != NULL) {
        zt_diag_list_add(&binder->result->diagnostics, ZT_DIAG_DUPLICATE_NAME, span, "duplicate name '%s'", name);
        return;
    }

    if (zt_scope_lookup_parent_chain(scope, name) != NULL) {
        zt_diag_list_add(&binder->result->diagnostics, ZT_DIAG_SHADOWING, span, "name '%s' shadows an outer declaration", name);
        return;
    }

    zt_scope_declare(scope, kind, name, span, is_implicit, NULL);
}

static void zt_bind_expression(zt_binder *binder, const zt_ast_node *node, zt_scope *scope);
static void zt_bind_type_node(zt_binder *binder, const zt_ast_node *node, zt_scope *scope);
static void zt_bind_block(zt_binder *binder, const zt_ast_node *block, zt_scope *parent_scope);

static void zt_bind_simple_type_name(zt_binder *binder, const char *name, zt_source_span span, zt_scope *scope) {
    char qualifier[256];
    const char *dot;
    zt_symbol *symbol;

    if (name == NULL || zt_is_builtin_type_name(name)) return;

    dot = strchr(name, '.');
    if (dot != NULL) {
        size_t prefix_len = (size_t)(dot - name);
        if (prefix_len >= sizeof(qualifier)) prefix_len = sizeof(qualifier) - 1;
        memcpy(qualifier, name, prefix_len);
        qualifier[prefix_len] = '\0';
        symbol = zt_scope_lookup(scope, qualifier);
        if (symbol == NULL) {
            zt_diag_list_add(&binder->result->diagnostics, ZT_DIAG_UNRESOLVED_NAME, span, "unresolved name '%s'", qualifier);
        }
        return;
    }

    symbol = zt_scope_lookup(scope, name);
    if (symbol == NULL) {
        zt_diag_list_add(&binder->result->diagnostics, ZT_DIAG_UNRESOLVED_NAME, span, "unresolved name '%s'", name);
    }
}

static void zt_bind_type_node(zt_binder *binder, const zt_ast_node *node, zt_scope *scope) {
    size_t i;

    if (node == NULL) return;
    switch (node->kind) {
        case ZT_AST_TYPE_SIMPLE:
            zt_bind_simple_type_name(binder, node->as.type_simple.name, node->span, scope);
            break;
        case ZT_AST_TYPE_GENERIC:
            zt_bind_simple_type_name(binder, node->as.type_generic.name, node->span, scope);
            for (i = 0; i < node->as.type_generic.type_args.count; i++) {
                zt_bind_type_node(binder, node->as.type_generic.type_args.items[i], scope);
            }
            break;
        default:
            break;
    }
}

static void zt_bind_generic_constraints(zt_binder *binder, const zt_ast_node_list *constraints, zt_scope *scope) {
    size_t i;

    if (constraints == NULL) return;
    for (i = 0; i < constraints->count; i++) {
        const zt_ast_node *constraint = constraints->items[i];
        zt_symbol *symbol;

        if (constraint == NULL || constraint->kind != ZT_AST_GENERIC_CONSTRAINT) continue;
        symbol = zt_scope_lookup(scope, constraint->as.generic_constraint.type_param_name);
        if (symbol == NULL || symbol->kind != ZT_SYMBOL_TYPE_PARAM) {
            zt_diag_list_add(
                &binder->result->diagnostics,
                ZT_DIAG_INVALID_CONSTRAINT_TARGET,
                constraint->span,
                "constraint target '%s' is not a generic type parameter",
                constraint->as.generic_constraint.type_param_name);
        }
        zt_bind_type_node(binder, constraint->as.generic_constraint.trait_type, scope);
    }
}

static void zt_bind_expr_list(zt_binder *binder, const zt_ast_node_list *list, zt_scope *scope) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) {
        zt_bind_expression(binder, list->items[i], scope);
    }
}

static void zt_bind_expression(zt_binder *binder, const zt_ast_node *node, zt_scope *scope) {
    size_t i;
    zt_symbol *symbol;

    if (node == NULL) return;

    switch (node->kind) {
        case ZT_AST_IDENT_EXPR:
            symbol = zt_scope_lookup(scope, node->as.ident_expr.name);
            if (symbol == NULL) {
                zt_diag_list_add(&binder->result->diagnostics, ZT_DIAG_UNRESOLVED_NAME, node->span, "unresolved name '%s'", node->as.ident_expr.name);
            }
            break;
        case ZT_AST_BINARY_EXPR:
            zt_bind_expression(binder, node->as.binary_expr.left, scope);
            zt_bind_expression(binder, node->as.binary_expr.right, scope);
            break;
        case ZT_AST_UNARY_EXPR:
            zt_bind_expression(binder, node->as.unary_expr.operand, scope);
            break;
        case ZT_AST_CALL_EXPR:
            if (!(node->as.call_expr.callee != NULL &&
                    node->as.call_expr.callee->kind == ZT_AST_IDENT_EXPR &&
                    strcmp(node->as.call_expr.callee->as.ident_expr.name, "len") == 0)) {
                zt_bind_expression(binder, node->as.call_expr.callee, scope);
            }
            zt_bind_expr_list(binder, &node->as.call_expr.positional_args, scope);
            for (i = 0; i < node->as.call_expr.named_args.count; i++) {
                zt_bind_expression(binder, node->as.call_expr.named_args.items[i].value, scope);
            }
            break;
        case ZT_AST_FIELD_EXPR:
            zt_bind_expression(binder, node->as.field_expr.object, scope);
            break;
        case ZT_AST_INDEX_EXPR:
            zt_bind_expression(binder, node->as.index_expr.object, scope);
            zt_bind_expression(binder, node->as.index_expr.index, scope);
            break;
        case ZT_AST_SLICE_EXPR:
            zt_bind_expression(binder, node->as.slice_expr.object, scope);
            zt_bind_expression(binder, node->as.slice_expr.start, scope);
            zt_bind_expression(binder, node->as.slice_expr.end, scope);
            break;
        case ZT_AST_GROUPED_EXPR:
            zt_bind_expression(binder, node->as.grouped_expr.inner, scope);
            break;
        case ZT_AST_SUCCESS_EXPR:
            zt_bind_expression(binder, node->as.success_expr.value, scope);
            break;
        case ZT_AST_ERROR_EXPR:
            zt_bind_expression(binder, node->as.error_expr.value, scope);
            break;
        case ZT_AST_LIST_EXPR:
            zt_bind_expr_list(binder, &node->as.list_expr.elements, scope);
            break;
        case ZT_AST_MAP_EXPR:
            for (i = 0; i < node->as.map_expr.entries.count; i++) {
                zt_bind_expression(binder, node->as.map_expr.entries.items[i].key, scope);
                zt_bind_expression(binder, node->as.map_expr.entries.items[i].value, scope);
            }
            break;
        default:
            break;
    }
}

static void zt_bind_statement(zt_binder *binder, const zt_ast_node *node, zt_scope *scope) {
    size_t i;
    zt_scope child_scope;

    if (node == NULL) return;

    switch (node->kind) {
        case ZT_AST_CONST_DECL:
            zt_bind_type_node(binder, node->as.const_decl.type_node, scope);
            zt_bind_expression(binder, node->as.const_decl.init_value, scope);
            zt_bind_declare_name(binder, scope, ZT_SYMBOL_LOCAL, node->as.const_decl.name, node->span, 0);
            break;
        case ZT_AST_VAR_DECL:
            zt_bind_type_node(binder, node->as.var_decl.type_node, scope);
            zt_bind_expression(binder, node->as.var_decl.init_value, scope);
            zt_bind_declare_name(binder, scope, ZT_SYMBOL_LOCAL, node->as.var_decl.name, node->span, 0);
            break;
        case ZT_AST_ASSIGN_STMT:
            if (zt_scope_lookup(scope, node->as.assign_stmt.name) == NULL) {
                zt_diag_list_add(&binder->result->diagnostics, ZT_DIAG_UNRESOLVED_NAME, node->span, "unresolved name '%s'", node->as.assign_stmt.name);
            }
            zt_bind_expression(binder, node->as.assign_stmt.value, scope);
            break;
        case ZT_AST_INDEX_ASSIGN_STMT:
            zt_bind_expression(binder, node->as.index_assign_stmt.object, scope);
            zt_bind_expression(binder, node->as.index_assign_stmt.index, scope);
            zt_bind_expression(binder, node->as.index_assign_stmt.value, scope);
            break;
        case ZT_AST_FIELD_ASSIGN_STMT:
            zt_bind_expression(binder, node->as.field_assign_stmt.object, scope);
            zt_bind_expression(binder, node->as.field_assign_stmt.value, scope);
            break;
        case ZT_AST_RETURN_STMT:
            zt_bind_expression(binder, node->as.return_stmt.value, scope);
            break;
        case ZT_AST_IF_STMT:
            zt_bind_expression(binder, node->as.if_stmt.condition, scope);
            zt_bind_block(binder, node->as.if_stmt.then_block, scope);
            if (node->as.if_stmt.else_block != NULL) {
                if (node->as.if_stmt.else_block->kind == ZT_AST_IF_STMT) {
                    zt_bind_statement(binder, node->as.if_stmt.else_block, scope);
                } else {
                    zt_bind_block(binder, node->as.if_stmt.else_block, scope);
                }
            }
            break;
        case ZT_AST_WHILE_STMT:
            zt_bind_expression(binder, node->as.while_stmt.condition, scope);
            zt_bind_block(binder, node->as.while_stmt.body, scope);
            break;
        case ZT_AST_REPEAT_STMT:
            zt_bind_expression(binder, node->as.repeat_stmt.count, scope);
            zt_bind_block(binder, node->as.repeat_stmt.body, scope);
            break;
        case ZT_AST_FOR_STMT:
            zt_bind_expression(binder, node->as.for_stmt.iterable, scope);
            zt_scope_init(&child_scope, scope);
            zt_bind_declare_name(binder, &child_scope, ZT_SYMBOL_LOCAL, node->as.for_stmt.item_name, node->span, 0);
            if (node->as.for_stmt.index_name != NULL) {
                zt_bind_declare_name(binder, &child_scope, ZT_SYMBOL_LOCAL, node->as.for_stmt.index_name, node->span, 0);
            }
            zt_bind_block(binder, node->as.for_stmt.body, &child_scope);
            zt_scope_dispose(&child_scope);
            break;
        case ZT_AST_MATCH_STMT:
            zt_bind_expression(binder, node->as.match_stmt.subject, scope);
            for (i = 0; i < node->as.match_stmt.cases.count; i++) {
                const zt_ast_node *case_node = node->as.match_stmt.cases.items[i];
                size_t p;
                if (case_node == NULL) continue;
                for (p = 0; p < case_node->as.match_case.patterns.count; p++) {
                    zt_bind_expression(binder, case_node->as.match_case.patterns.items[p], scope);
                }
                zt_bind_block(binder, case_node->as.match_case.body, scope);
            }
            break;
        case ZT_AST_EXPR_STMT:
            zt_bind_expression(binder, node->as.expr_stmt.expr, scope);
            break;
        case ZT_AST_BREAK_STMT:
        case ZT_AST_CONTINUE_STMT:
            break;
        default:
            break;
    }
}

static void zt_bind_block(zt_binder *binder, const zt_ast_node *block, zt_scope *parent_scope) {
    zt_scope block_scope;
    size_t i;

    if (block == NULL || block->kind != ZT_AST_BLOCK) return;

    zt_scope_init(&block_scope, parent_scope);
    for (i = 0; i < block->as.block.statements.count; i++) {
        zt_bind_statement(binder, block->as.block.statements.items[i], &block_scope);
    }
    zt_scope_dispose(&block_scope);
}

static void zt_bind_type_param_defs(zt_binder *binder, const zt_ast_node_list *type_params, zt_scope *scope) {
    size_t i;
    if (type_params == NULL) return;
    for (i = 0; i < type_params->count; i++) {
        const zt_ast_node *param = type_params->items[i];
        if (param != NULL && param->kind == ZT_AST_TYPE_SIMPLE) {
            zt_bind_declare_name(binder, scope, ZT_SYMBOL_TYPE_PARAM, param->as.type_simple.name, param->span, 0);
        }
    }
}

static void zt_bind_param_list(zt_binder *binder, const zt_ast_node_list *params, zt_scope *scope) {
    size_t i;
    if (params == NULL) return;
    for (i = 0; i < params->count; i++) {
        const zt_ast_node *param = params->items[i];
        if (param == NULL || param->kind != ZT_AST_PARAM) continue;
        zt_bind_type_node(binder, param->as.param.type_node, scope);
        zt_bind_expression(binder, param->as.param.default_value, scope);
        zt_bind_declare_name(binder, scope, ZT_SYMBOL_PARAM, param->as.param.name, param->span, 0);
    }
}

static void zt_bind_where_clause(zt_binder *binder, const zt_ast_node *where_clause, zt_scope *scope) {
    zt_scope where_scope;

    if (where_clause == NULL || where_clause->kind != ZT_AST_WHERE_CLAUSE) return;

    zt_scope_init(&where_scope, scope);
    zt_bind_declare_name(binder, &where_scope, ZT_SYMBOL_IMPLICIT_IT, "it", where_clause->span, 1);
    zt_bind_expression(binder, where_clause->as.where_clause.condition, &where_scope);
    zt_scope_dispose(&where_scope);
}

static void zt_bind_decl(zt_binder *binder, const zt_ast_node *decl, zt_scope *module_scope) {
    zt_scope decl_scope;
    size_t i;

    if (decl == NULL) return;

    switch (decl->kind) {
        case ZT_AST_FUNC_DECL:
            zt_scope_init(&decl_scope, module_scope);
            zt_bind_type_param_defs(binder, &decl->as.func_decl.type_params, &decl_scope);
            zt_bind_param_list(binder, &decl->as.func_decl.params, &decl_scope);
            zt_bind_generic_constraints(binder, &decl->as.func_decl.constraints, &decl_scope);
            zt_bind_type_node(binder, decl->as.func_decl.return_type, &decl_scope);
            zt_bind_block(binder, decl->as.func_decl.body, &decl_scope);
            zt_scope_dispose(&decl_scope);
            break;
        case ZT_AST_STRUCT_DECL:
            zt_scope_init(&decl_scope, module_scope);
            zt_bind_type_param_defs(binder, &decl->as.struct_decl.type_params, &decl_scope);
            zt_bind_generic_constraints(binder, &decl->as.struct_decl.constraints, &decl_scope);
            for (i = 0; i < decl->as.struct_decl.fields.count; i++) {
                const zt_ast_node *field = decl->as.struct_decl.fields.items[i];
                if (field == NULL) continue;
                zt_bind_type_node(binder, field->as.struct_field.type_node, &decl_scope);
                zt_bind_expression(binder, field->as.struct_field.default_value, &decl_scope);
                zt_bind_where_clause(binder, field->as.struct_field.where_clause, &decl_scope);
            }
            zt_scope_dispose(&decl_scope);
            break;
        case ZT_AST_TRAIT_DECL:
            zt_scope_init(&decl_scope, module_scope);
            zt_bind_type_param_defs(binder, &decl->as.trait_decl.type_params, &decl_scope);
            zt_bind_generic_constraints(binder, &decl->as.trait_decl.constraints, &decl_scope);
            for (i = 0; i < decl->as.trait_decl.methods.count; i++) {
                const zt_ast_node *method = decl->as.trait_decl.methods.items[i];
                zt_scope method_scope;
                if (method == NULL) continue;
                zt_scope_init(&method_scope, &decl_scope);
                zt_bind_param_list(binder, &method->as.trait_method.params, &method_scope);
                zt_bind_type_node(binder, method->as.trait_method.return_type, &method_scope);
                zt_scope_dispose(&method_scope);
            }
            zt_scope_dispose(&decl_scope);
            break;
        case ZT_AST_ENUM_DECL:
            zt_scope_init(&decl_scope, module_scope);
            zt_bind_type_param_defs(binder, &decl->as.enum_decl.type_params, &decl_scope);
            zt_bind_generic_constraints(binder, &decl->as.enum_decl.constraints, &decl_scope);
            for (i = 0; i < decl->as.enum_decl.variants.count; i++) {
                const zt_ast_node *variant = decl->as.enum_decl.variants.items[i];
                size_t f;
                if (variant == NULL) continue;
                for (f = 0; f < variant->as.enum_variant.fields.count; f++) {
                    const zt_ast_node *field = variant->as.enum_variant.fields.items[f];
                    if (field == NULL) continue;
                    if (field->kind == ZT_AST_PARAM) {
                        zt_bind_type_node(binder, field->as.param.type_node, &decl_scope);
                    } else {
                        zt_bind_type_node(binder, field, &decl_scope);
                    }
                }
            }
            zt_scope_dispose(&decl_scope);
            break;
        case ZT_AST_APPLY_DECL:
            zt_scope_init(&decl_scope, module_scope);
            if (decl->as.apply_decl.trait_name != NULL && zt_scope_lookup(module_scope, decl->as.apply_decl.trait_name) == NULL) {
                zt_diag_list_add(&binder->result->diagnostics, ZT_DIAG_UNRESOLVED_NAME, decl->span, "unresolved name '%s'", decl->as.apply_decl.trait_name);
            }
            if (decl->as.apply_decl.target_name != NULL && zt_scope_lookup(module_scope, decl->as.apply_decl.target_name) == NULL) {
                zt_diag_list_add(&binder->result->diagnostics, ZT_DIAG_UNRESOLVED_NAME, decl->span, "unresolved name '%s'", decl->as.apply_decl.target_name);
            }
            if (decl->as.apply_decl.trait_name == NULL) {
                zt_bind_type_param_defs(binder, &decl->as.apply_decl.target_type_params, &decl_scope);
            } else {
                for (i = 0; i < decl->as.apply_decl.trait_type_params.count; i++) {
                    zt_bind_type_node(binder, decl->as.apply_decl.trait_type_params.items[i], module_scope);
                }
                for (i = 0; i < decl->as.apply_decl.target_type_params.count; i++) {
                    zt_bind_type_node(binder, decl->as.apply_decl.target_type_params.items[i], module_scope);
                }
            }
            zt_bind_generic_constraints(binder, &decl->as.apply_decl.constraints, &decl_scope);
            for (i = 0; i < decl->as.apply_decl.methods.count; i++) {
                const zt_ast_node *method = decl->as.apply_decl.methods.items[i];
                zt_scope method_scope;
                if (method == NULL) continue;
                zt_scope_init(&method_scope, &decl_scope);
                zt_bind_declare_name(binder, &method_scope, ZT_SYMBOL_IMPLICIT_SELF, "self", method->span, 1);
                zt_bind_param_list(binder, &method->as.func_decl.params, &method_scope);
                zt_bind_type_node(binder, method->as.func_decl.return_type, &method_scope);
                zt_bind_block(binder, method->as.func_decl.body, &method_scope);
                zt_scope_dispose(&method_scope);
            }
            zt_scope_dispose(&decl_scope);
            break;
        case ZT_AST_EXTERN_DECL:
            for (i = 0; i < decl->as.extern_decl.functions.count; i++) {
                const zt_ast_node *func = decl->as.extern_decl.functions.items[i];
                if (func == NULL) continue;
                zt_scope_init(&decl_scope, module_scope);
                zt_bind_param_list(binder, &func->as.func_decl.params, &decl_scope);
                zt_bind_type_node(binder, func->as.func_decl.return_type, &decl_scope);
                zt_scope_dispose(&decl_scope);
            }
            break;
        default:
            break;
    }
}

static void zt_bind_declare_top_level(zt_binder *binder, const zt_ast_node *decl, zt_scope *module_scope) {
    size_t i;

    if (decl == NULL) return;

    switch (decl->kind) {
        case ZT_AST_FUNC_DECL:
            zt_bind_declare_name(binder, module_scope, ZT_SYMBOL_FUNC, decl->as.func_decl.name, decl->span, 0);
            break;
        case ZT_AST_STRUCT_DECL:
            zt_bind_declare_name(binder, module_scope, ZT_SYMBOL_STRUCT, decl->as.struct_decl.name, decl->span, 0);
            break;
        case ZT_AST_TRAIT_DECL:
            zt_bind_declare_name(binder, module_scope, ZT_SYMBOL_TRAIT, decl->as.trait_decl.name, decl->span, 0);
            break;
        case ZT_AST_ENUM_DECL:
            zt_bind_declare_name(binder, module_scope, ZT_SYMBOL_ENUM, decl->as.enum_decl.name, decl->span, 0);
            break;
        case ZT_AST_EXTERN_DECL:
            for (i = 0; i < decl->as.extern_decl.functions.count; i++) {
                const zt_ast_node *func = decl->as.extern_decl.functions.items[i];
                if (func != NULL) {
                    zt_bind_declare_name(binder, module_scope, ZT_SYMBOL_EXTERN_FUNC, func->as.func_decl.name, func->span, 0);
                }
            }
            break;
        default:
            break;
    }
}

zt_bind_result zt_bind_file(const zt_ast_node *root) {
    static const char *core_traits[] = { "Equatable", "Hashable", "TextRepresentable", NULL };
    zt_bind_result result;
    zt_binder binder;
    size_t i;

    zt_scope_init(&result.module_scope, NULL);
    result.diagnostics = zt_diag_list_make();
    binder.result = &result;

    for (i = 0; core_traits[i] != NULL; i++) {
        zt_scope_declare(&result.module_scope, ZT_SYMBOL_CORE_TRAIT, core_traits[i], zt_source_span_unknown(), 1, NULL);
    }

    if (root == NULL || root->kind != ZT_AST_FILE) {
        return result;
    }

    for (i = 0; i < root->as.file.imports.count; i++) {
        const zt_ast_node *import_decl = root->as.file.imports.items[i];
        const char *local_name;
        if (import_decl == NULL) continue;
        local_name = import_decl->as.import_decl.alias != NULL ? import_decl->as.import_decl.alias : zt_import_local_name(import_decl->as.import_decl.path);
        zt_bind_declare_name(&binder, &result.module_scope, ZT_SYMBOL_IMPORT, local_name, import_decl->span, 0);
    }

    for (i = 0; i < root->as.file.declarations.count; i++) {
        zt_bind_declare_top_level(&binder, root->as.file.declarations.items[i], &result.module_scope);
    }

    for (i = 0; i < root->as.file.declarations.count; i++) {
        zt_bind_decl(&binder, root->as.file.declarations.items[i], &result.module_scope);
    }

    return result;
}

void zt_bind_result_dispose(zt_bind_result *result) {
    if (result == NULL) return;
    zt_scope_dispose(&result->module_scope);
    zt_diag_list_dispose(&result->diagnostics);
}

