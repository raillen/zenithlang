#include "compiler/semantic/binder/binder.h"

#include "compiler/semantic/parameter_validation.h"
#include <ctype.h>
#include <string.h>

#define ZT_BIND_MAX_READABLE_BLOCK_DEPTH 6u
#define ZT_BIND_MAX_READABLE_FUNCTION_STATEMENTS 80u

typedef struct zt_binder {
    zt_bind_result *result;
    size_t block_depth;
} zt_binder;

static size_t zt_scope_collect_names(const zt_scope *scope, const char **out, size_t capacity) {
    size_t count = 0;
    const zt_scope *current = scope;
    while (current != NULL) {
        size_t i;
        for (i = 0; i < current->count && count < capacity; i += 1) {
            out[count] = current->symbols[i].name;
            count += 1;
        }
        current = current->parent;
    }
    return count;
}

static void zt_bind_emit_unresolved_with_suggestion(zt_binder *binder, const char *name, zt_source_span span, zt_scope *scope) {
    const char *candidates[256];
    size_t candidate_count;
    char suggestion[256];

    candidate_count = zt_scope_collect_names(scope, candidates, 256);

    if (candidate_count > 0 && zt_name_suggest(name, candidates, candidate_count, suggestion, sizeof(suggestion))) {
        zt_diag_list_add_suggestion(&binder->result->diagnostics,
            ZT_DIAG_UNRESOLVED_NAME, span, suggestion,
            "unresolved name '%s'", name);
    } else {
        zt_diag_list_add(&binder->result->diagnostics,
            ZT_DIAG_UNRESOLVED_NAME, span,
            "unresolved name '%s'", name);
    }
}

static void zt_bind_warn_confusing_name(zt_binder *binder, const char *name, zt_source_span span, int is_implicit) {
    int has_l = 0;
    int has_I = 0;
    int has_1 = 0;
    int has_O = 0;
    int has_0 = 0;
    const char *cursor;

    if (binder == NULL || binder->result == NULL || name == NULL || is_implicit) return;

    for (cursor = name; *cursor != '\0'; cursor += 1) {
        switch (*cursor) {
            case 'l': has_l = 1; break;
            case 'I': has_I = 1; break;
            case '1': has_1 = 1; break;
            case 'O': has_O = 1; break;
            case '0': has_0 = 1; break;
            default: break;
        }
    }

    if ((has_l + has_I + has_1) >= 2) {
        zt_diag_list_add_severity(
            &binder->result->diagnostics,
            ZT_DIAG_CONFUSING_NAME,
            ZT_DIAG_SEVERITY_WARNING,
            span,
            "name '%s' mixes confusable characters (l, I, 1)",
            name);
        return;
    }

    if (has_O && has_0) {
        zt_diag_list_add_severity(
            &binder->result->diagnostics,
            ZT_DIAG_CONFUSING_NAME,
            ZT_DIAG_SEVERITY_WARNING,
            span,
            "name '%s' mixes confusable characters (O, 0)",
            name);
    }
}

static size_t zt_bind_normalize_name(const char *name, char *out, size_t capacity) {
    size_t count = 0;
    const unsigned char *cursor;

    if (out == NULL || capacity == 0) return 0;
    out[0] = '\0';
    if (name == NULL) return 0;

    for (cursor = (const unsigned char *)name; *cursor != '\0' && count + 1 < capacity; cursor += 1) {
        if (*cursor == '_') continue;
        out[count] = (char)tolower(*cursor);
        count += 1;
    }
    out[count] = '\0';
    return count;
}

static int zt_bind_is_prefix_pair(const char *left, const char *right) {
    size_t left_len;
    size_t right_len;

    if (left == NULL || right == NULL) return 0;
    left_len = strlen(left);
    right_len = strlen(right);
    if (left_len == 0 || right_len == 0 || left_len == right_len) return 0;

    if (left_len < right_len) {
        return strncmp(left, right, left_len) == 0;
    }
    return strncmp(left, right, right_len) == 0;
}

static int zt_bind_edit_distance_at_most_one(const char *left, const char *right) {
    size_t left_len;
    size_t right_len;
    size_t i = 0;
    size_t j = 0;
    int edits = 0;

    if (left == NULL || right == NULL) return 0;
    left_len = strlen(left);
    right_len = strlen(right);
    if (left_len > right_len + 1 || right_len > left_len + 1) return 0;

    while (i < left_len && j < right_len) {
        if (left[i] == right[j]) {
            i += 1;
            j += 1;
            continue;
        }

        edits += 1;
        if (edits > 1) return 0;

        if (left_len == right_len) {
            i += 1;
            j += 1;
        } else if (left_len > right_len) {
            i += 1;
        } else {
            j += 1;
        }
    }

    if (i < left_len || j < right_len) edits += 1;
    return edits <= 1;
}

static int zt_bind_names_are_too_similar(const char *left, const char *right) {
    char left_norm[128];
    char right_norm[128];
    size_t left_len;
    size_t right_len;

    left_len = zt_bind_normalize_name(left, left_norm, sizeof(left_norm));
    right_len = zt_bind_normalize_name(right, right_norm, sizeof(right_norm));
    if (left_len < 6 || right_len < 6) return 0;
    if (strcmp(left_norm, right_norm) == 0) return strcmp(left, right) != 0;
    if (zt_bind_is_prefix_pair(left_norm, right_norm)) return 0;
    return zt_bind_edit_distance_at_most_one(left_norm, right_norm);
}

static void zt_bind_warn_similar_name(zt_binder *binder, zt_scope *scope, const char *name, zt_source_span span, int is_implicit) {
    size_t i;

    if (binder == NULL || binder->result == NULL || scope == NULL || name == NULL || is_implicit) return;
    if (strchr(name, '.') != NULL) return;

    for (i = 0; i < scope->count; i += 1) {
        const zt_symbol *existing = &scope->symbols[i];
        if (existing->is_implicit || existing->name == NULL) continue;
        if (strchr(existing->name, '.') != NULL) continue;
        if (!zt_bind_names_are_too_similar(existing->name, name)) continue;

        zt_diag_list_add_severity(
            &binder->result->diagnostics,
            ZT_DIAG_SIMILAR_NAME,
            ZT_DIAG_SEVERITY_WARNING,
            span,
            "name '%s' is very similar to '%s' in the same scope",
            name,
            existing->name);
        return;
    }
}

static int zt_is_builtin_type_name(const char *name) {
    static const char *builtin_names[] = {
        "bool",
        "int", "int8", "int16", "int32", "int64",
        "u8", "u16", "u32", "u64",
        "uint8", "uint16", "uint32", "uint64",
        "float", "float32", "float64",
        "text",
        "core.Error",
        "bytes",
        "void",
        "list",
        "set",
        "map",
        "grid2d",
        "pqueue",
        "circbuf",
        "btreemap",
        "btreeset",
        "grid3d",
        "optional",
        "result",
        "dyn",
        "lazy",
        NULL
    };
    size_t i;

    if (name == NULL) return 0;
    for (i = 0; builtin_names[i] != NULL; i++) {
        if (strcmp(name, builtin_names[i]) == 0) return 1;
    }
    return 0;
}

static int zt_is_intrinsic_name(const char *name) {
    return name != NULL &&
           (strcmp(name, "len") == 0 ||
            strcmp(name, "to_text") == 0 ||
            strcmp(name, "check") == 0 ||
            strcmp(name, "todo") == 0 ||
            strcmp(name, "unreachable") == 0 ||
            strcmp(name, "panic") == 0 ||
            strcmp(name, "print") == 0 ||
            strcmp(name, "read") == 0 ||
            strcmp(name, "debug") == 0 ||
            strcmp(name, "type_name") == 0 ||
            strcmp(name, "size_of") == 0 ||
            strcmp(name, "range") == 0);
}

static const char *zt_import_local_name(const char *path) {
    const char *last_dot;
    if (path == NULL) return NULL;
    last_dot = strrchr(path, '.');
    return last_dot != NULL ? last_dot + 1 : path;
}

static void zt_bind_declare_name(zt_binder *binder, zt_scope *scope, zt_symbol_kind kind, const char *name, zt_source_span span, int is_implicit) {
    if (scope == NULL || name == NULL) return;

    zt_bind_warn_confusing_name(binder, name, span, is_implicit);

    if (zt_scope_lookup_current(scope, name) != NULL) {
        zt_diag_list_add(&binder->result->diagnostics, ZT_DIAG_DUPLICATE_NAME, span, "duplicate name '%s'", name);
        return;
    }

    zt_bind_warn_similar_name(binder, scope, name, span, is_implicit);

    if (zt_scope_lookup_parent_chain(scope, name) != NULL) {
        zt_diag_list_add(&binder->result->diagnostics, ZT_DIAG_SHADOWING, span, "name '%s' shadows an outer declaration", name);
        return;
    }

    zt_scope_declare(scope, kind, name, span, is_implicit, NULL);
}

static void zt_bind_expression(zt_binder *binder, const zt_ast_node *node, zt_scope *scope);
static void zt_bind_type_node(zt_binder *binder, const zt_ast_node *node, zt_scope *scope);
static void zt_bind_block(zt_binder *binder, const zt_ast_node *block, zt_scope *parent_scope);

static int zt_bind_split_prefix(const char *name, char *prefix, size_t prefix_capacity) {
    const char *dot;
    size_t len;
    if (name == NULL || prefix == NULL || prefix_capacity == 0) return 0;
    dot = strrchr(name, '.');
    if (dot == NULL) return 0;
    len = (size_t)(dot - name);
    if (len == 0 || len >= prefix_capacity) return 0;
    memcpy(prefix, name, len);
    prefix[len] = '\0';
    return 1;
}

static int zt_bind_symbol_is_value_like(zt_symbol_kind kind) {
    return kind == ZT_SYMBOL_LOCAL;
}

static void zt_bind_seed_module_value_aliases(
        zt_scope *module_scope,
        zt_scope *target_scope,
        const char *decl_name) {
    char prefix[256];
    size_t prefix_len;
    size_t i;

    if (module_scope == NULL || target_scope == NULL) return;
    if (!zt_bind_split_prefix(decl_name, prefix, sizeof(prefix))) return;
    prefix_len = strlen(prefix);

    for (i = 0; i < module_scope->count; i += 1) {
        const zt_symbol *symbol = &module_scope->symbols[i];
        const char *short_name;
        const char *dot;

        if (symbol->name == NULL) continue;
        if (!zt_bind_symbol_is_value_like(symbol->kind)) continue;
        if (strncmp(symbol->name, prefix, prefix_len) != 0 || symbol->name[prefix_len] != '.') continue;

        dot = strrchr(symbol->name, '.');
        if (dot == NULL || dot[1] == '\0') continue;
        short_name = dot + 1;
        if (zt_scope_lookup_current(target_scope, short_name) != NULL) continue;
        zt_scope_declare(target_scope, symbol->kind, short_name, symbol->span, 1, NULL);
    }
}

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
            /* Try to resolve the WHOLE name as a qualified type name (e.g. format.BytesStyle) */
            if (zt_scope_lookup(scope, name) != NULL) {
                return; /* Satisfied */
            }
            zt_bind_emit_unresolved_with_suggestion(binder, qualifier, span, scope);
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
                zt_bind_emit_unresolved_with_suggestion(binder, node->as.ident_expr.name, node->span, scope);
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
                    zt_is_intrinsic_name(node->as.call_expr.callee->as.ident_expr.name))) {
                zt_bind_expression(binder, node->as.call_expr.callee, scope);
            }
            zt_bind_expr_list(binder, &node->as.call_expr.positional_args, scope);
            for (i = 0; i < node->as.call_expr.named_args.count; i++) {
                zt_bind_expression(binder, node->as.call_expr.named_args.items[i].value, scope);
            }
            break;
        case ZT_AST_FIELD_EXPR:
            if (node->as.field_expr.object != NULL &&
                node->as.field_expr.object->kind == ZT_AST_IDENT_EXPR &&
                node->as.field_expr.field_name != NULL &&
                strcmp(node->as.field_expr.object->as.ident_expr.name, "core") == 0 &&
                strcmp(node->as.field_expr.field_name, "Error") == 0) {
                return;
            }
            /* Try to resolve as a qualified name (e.g. bytes.empty) */
            if (node->as.field_expr.object != NULL && node->as.field_expr.object->kind == ZT_AST_IDENT_EXPR) {
                char qualified[512];
                const char *prefix = node->as.field_expr.object->as.ident_expr.name;
                const char *member = node->as.field_expr.field_name;
                if (prefix != NULL && member != NULL) {
                    snprintf(qualified, sizeof(qualified), "%s.%s", prefix, member);
                    symbol = zt_scope_lookup(scope, qualified);
                    if (symbol != NULL) {
                        /* Satisfied as a qualified name */
                        return;
                    }
                }
            }
            zt_bind_expression(binder, node->as.field_expr.object, scope);
            break;
        case ZT_AST_ENUM_DOT_EXPR:
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
        case ZT_AST_IF_EXPR:
            zt_bind_expression(binder, node->as.if_expr.condition, scope);
            zt_bind_expression(binder, node->as.if_expr.then_expr, scope);
            zt_bind_expression(binder, node->as.if_expr.else_expr, scope);
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
        case ZT_AST_STRUCT_LITERAL_EXPR:
            for (i = 0; i < node->as.struct_literal_expr.fields.count; i++) {
                zt_bind_expression(binder, node->as.struct_literal_expr.fields.items[i].value, scope);
            }
            break;
        case ZT_AST_FMT_EXPR:
            for (i = 0; i < node->as.fmt_expr.parts.count; i++) {
                zt_bind_expression(binder, node->as.fmt_expr.parts.items[i], scope);
            }
            break;
        default:
            break;
    }
}

static void zt_bind_expression(zt_binder *binder, const zt_ast_node *node, zt_scope *scope);

static void zt_bind_match_pattern(zt_binder *binder, const zt_ast_node *pattern, zt_scope *case_scope) {
    size_t i;
    const zt_ast_node *inner = NULL;
    if (pattern == NULL) return;

    if (pattern->kind == ZT_AST_SUCCESS_EXPR) {
        inner = pattern->as.success_expr.value;
    } else if (pattern->kind == ZT_AST_ERROR_EXPR) {
        inner = pattern->as.error_expr.value;
    }

    if (inner != NULL) {
        if (inner->kind == ZT_AST_IDENT_EXPR) {
            zt_bind_declare_name(binder, case_scope, ZT_SYMBOL_LOCAL, inner->as.ident_expr.name, inner->span, 0);
        } else if (inner->kind == ZT_AST_VALUE_BINDING) {
            zt_bind_declare_name(binder, case_scope, ZT_SYMBOL_LOCAL, inner->as.value_binding.name, inner->span, 0);
        } else {
            zt_bind_match_pattern(binder, inner, case_scope);
        }
        return;
    }

    if (pattern->kind == ZT_AST_CALL_EXPR) {
        /* VariantName(binding) pattern */
        /* Resolve callee */
        zt_bind_expression(binder, pattern->as.call_expr.callee, case_scope);
        
        /* Bind arguments as locals */
        for (i = 0; i < pattern->as.call_expr.positional_args.count; i++) {
            const zt_ast_node *arg = pattern->as.call_expr.positional_args.items[i];
            if (arg != NULL) {
                if (arg->kind == ZT_AST_IDENT_EXPR) {
                    zt_bind_declare_name(binder, case_scope, ZT_SYMBOL_LOCAL, arg->as.ident_expr.name, arg->span, 0);
                } else {
                    zt_bind_match_pattern(binder, arg, case_scope);
                }
            }
        }
        return;
    }

    if (pattern->kind == ZT_AST_FIELD_EXPR) {
        /* Enum.Variant pattern (no payload) -> resolve it */
        zt_bind_expression(binder, pattern, case_scope);
        return;
    }

    if (pattern->kind == ZT_AST_IDENT_EXPR) {
        zt_symbol *existing = zt_scope_lookup(case_scope, pattern->as.ident_expr.name);
        if (existing != NULL) {
            zt_bind_expression(binder, pattern, case_scope);
        } else {
            zt_bind_declare_name(binder, case_scope, ZT_SYMBOL_LOCAL, pattern->as.ident_expr.name, pattern->span, 0);
        }
        return;
    }

    if (pattern->kind == ZT_AST_VALUE_BINDING) {
        zt_bind_declare_name(binder, case_scope, ZT_SYMBOL_LOCAL, pattern->as.value_binding.name, pattern->span, 0);
        return;
    }

    /* Literals etc: check normally */
    zt_bind_expression(binder, pattern, case_scope);
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
        case ZT_AST_USING_STMT:
            zt_bind_expression(binder, node->as.using_stmt.init_value, scope);
            zt_scope_init(&child_scope, scope);
            zt_bind_declare_name(binder, &child_scope, ZT_SYMBOL_LOCAL, node->as.using_stmt.name, node->span, 0);
            if (node->as.using_stmt.cleanup_expr != NULL) {
                zt_bind_expression(binder, node->as.using_stmt.cleanup_expr, &child_scope);
            }
            if (node->as.using_stmt.body != NULL) {
                zt_bind_block(binder, node->as.using_stmt.body, &child_scope);
            } else {
                zt_bind_declare_name(binder, scope, ZT_SYMBOL_LOCAL, node->as.using_stmt.name, node->span, 0);
            }
            zt_scope_dispose(&child_scope);
            break;
        case ZT_AST_ASSIGN_STMT:
            if (zt_scope_lookup(scope, node->as.assign_stmt.name) == NULL) {
                zt_bind_emit_unresolved_with_suggestion(binder, node->as.assign_stmt.name, node->span, scope);
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
                
                zt_scope_init(&child_scope, scope);
                for (p = 0; p < case_node->as.match_case.patterns.count; p++) {
                    zt_bind_match_pattern(binder, case_node->as.match_case.patterns.items[p], &child_scope);
                }
                zt_bind_block(binder, case_node->as.match_case.body, &child_scope);
                zt_scope_dispose(&child_scope);
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

    binder->block_depth += 1;
    if (binder->block_depth == (size_t)ZT_BIND_MAX_READABLE_BLOCK_DEPTH + 1u) {
        zt_diag_list_add_severity(
            &binder->result->diagnostics,
            ZT_DIAG_BLOCK_TOO_DEEP,
            ZT_DIAG_SEVERITY_WARNING,
            block->span,
            "block nesting depth %zu is hard to scan; limit is %u",
            binder->block_depth,
            ZT_BIND_MAX_READABLE_BLOCK_DEPTH);
    }

    zt_scope_init(&block_scope, parent_scope);
    for (i = 0; i < block->as.block.statements.count; i++) {
        zt_bind_statement(binder, block->as.block.statements.items[i], &block_scope);
    }
    zt_scope_dispose(&block_scope);
    binder->block_depth -= 1;
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

static void zt_bind_where_clause(zt_binder *binder, const zt_ast_node *where_clause, zt_scope *scope);

static void zt_bind_param_list(zt_binder *binder, const zt_ast_node_list *params, zt_scope *scope) {
    size_t i;
    if (params == NULL) return;
    for (i = 0; i < params->count; i++) {
        const zt_ast_node *param = params->items[i];
        if (param == NULL || param->kind != ZT_AST_PARAM) continue;
        zt_bind_type_node(binder, param->as.param.type_node, scope);
        zt_bind_expression(binder, param->as.param.default_value, scope);
        zt_bind_declare_name(binder, scope, ZT_SYMBOL_PARAM, param->as.param.name, param->span, 0);
        zt_bind_where_clause(binder, param->as.param.where_clause, scope);
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

static size_t zt_bind_statement_count(const zt_ast_node *node);

static size_t zt_bind_block_statement_count(const zt_ast_node *block) {
    size_t count = 0;
    size_t i;

    if (block == NULL) return 0;
    if (block->kind != ZT_AST_BLOCK) return zt_bind_statement_count(block);

    for (i = 0; i < block->as.block.statements.count; i += 1) {
        count += zt_bind_statement_count(block->as.block.statements.items[i]);
    }
    return count;
}

static size_t zt_bind_statement_count(const zt_ast_node *node) {
    size_t count = 1;
    size_t i;

    if (node == NULL) return 0;

    switch (node->kind) {
        case ZT_AST_BLOCK:
            return zt_bind_block_statement_count(node);
        case ZT_AST_IF_STMT:
            count += zt_bind_block_statement_count(node->as.if_stmt.then_block);
            if (node->as.if_stmt.else_block != NULL) {
                count += node->as.if_stmt.else_block->kind == ZT_AST_IF_STMT
                    ? zt_bind_statement_count(node->as.if_stmt.else_block)
                    : zt_bind_block_statement_count(node->as.if_stmt.else_block);
            }
            return count;
        case ZT_AST_WHILE_STMT:
            return count + zt_bind_block_statement_count(node->as.while_stmt.body);
        case ZT_AST_FOR_STMT:
            return count + zt_bind_block_statement_count(node->as.for_stmt.body);
        case ZT_AST_REPEAT_STMT:
            return count + zt_bind_block_statement_count(node->as.repeat_stmt.body);
        case ZT_AST_USING_STMT:
            if (node->as.using_stmt.body != NULL)
                return count + zt_bind_block_statement_count(node->as.using_stmt.body);
            return count;
        case ZT_AST_MATCH_STMT:
            for (i = 0; i < node->as.match_stmt.cases.count; i += 1) {
                const zt_ast_node *case_node = node->as.match_stmt.cases.items[i];
                if (case_node != NULL && case_node->kind == ZT_AST_MATCH_CASE) {
                    count += zt_bind_block_statement_count(case_node->as.match_case.body);
                }
            }
            return count;
        default:
            return 1;
    }
}

static void zt_bind_warn_function_too_long(zt_binder *binder, const zt_ast_node *decl) {
    size_t statement_count;
    const char *name;

    if (binder == NULL || binder->result == NULL || decl == NULL || decl->kind != ZT_AST_FUNC_DECL) return;
    if (decl->as.func_decl.body == NULL) return;

    statement_count = zt_bind_block_statement_count(decl->as.func_decl.body);
    if (statement_count <= ZT_BIND_MAX_READABLE_FUNCTION_STATEMENTS) return;

    name = decl->as.func_decl.name != NULL ? decl->as.func_decl.name : "<anonymous>";
    zt_diag_list_add_severity(
        &binder->result->diagnostics,
        ZT_DIAG_FUNCTION_TOO_LONG,
        ZT_DIAG_SEVERITY_WARNING,
        decl->span,
        "function '%s' has %zu statements; limit is %u",
        name,
        statement_count,
        ZT_BIND_MAX_READABLE_FUNCTION_STATEMENTS);
}

static void zt_bind_decl(zt_binder *binder, const zt_ast_node *decl, zt_scope *module_scope) {
    zt_scope decl_scope;
    size_t i;

    if (decl == NULL) return;

    switch (decl->kind) {
        case ZT_AST_FUNC_DECL:
            zt_validate_parameter_ordering(decl->as.func_decl.params, &binder->result->diagnostics);
            zt_scope_init(&decl_scope, module_scope);
            zt_bind_type_param_defs(binder, &decl->as.func_decl.type_params, &decl_scope);
            zt_bind_param_list(binder, &decl->as.func_decl.params, &decl_scope);
            zt_bind_generic_constraints(binder, &decl->as.func_decl.constraints, &decl_scope);
            zt_bind_type_node(binder, decl->as.func_decl.return_type, &decl_scope);
            zt_bind_seed_module_value_aliases(module_scope, &decl_scope, decl->as.func_decl.name);
            zt_bind_warn_function_too_long(binder, decl);
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
                zt_validate_parameter_ordering(method->as.trait_method.params, &binder->result->diagnostics);
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
                zt_validate_parameter_ordering(method->as.func_decl.params, &binder->result->diagnostics);
                zt_bind_param_list(binder, &method->as.func_decl.params, &method_scope);
                zt_bind_type_node(binder, method->as.func_decl.return_type, &method_scope);
                zt_bind_warn_function_too_long(binder, method);
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
                zt_validate_parameter_ordering(func->as.func_decl.params, &binder->result->diagnostics);
                zt_bind_param_list(binder, &func->as.func_decl.params, &decl_scope);
                zt_bind_type_node(binder, func->as.func_decl.return_type, &decl_scope);
                zt_scope_dispose(&decl_scope);
            }
            break;
        case ZT_AST_TYPE_ALIAS_DECL:
            zt_scope_init(&decl_scope, module_scope);
            zt_bind_type_node(binder, decl->as.type_alias_decl.target_type, &decl_scope);
            zt_scope_dispose(&decl_scope);
            break;
        case ZT_AST_CONST_DECL:
            zt_scope_init(&decl_scope, module_scope);
            zt_bind_seed_module_value_aliases(module_scope, &decl_scope, decl->as.const_decl.name);
            zt_bind_type_node(binder, decl->as.const_decl.type_node, &decl_scope);
            zt_bind_expression(binder, decl->as.const_decl.init_value, &decl_scope);
            zt_scope_dispose(&decl_scope);
            break;
        case ZT_AST_VAR_DECL:
            zt_scope_init(&decl_scope, module_scope);
            zt_bind_seed_module_value_aliases(module_scope, &decl_scope, decl->as.var_decl.name);
            zt_bind_type_node(binder, decl->as.var_decl.type_node, &decl_scope);
            zt_bind_expression(binder, decl->as.var_decl.init_value, &decl_scope);
            zt_scope_dispose(&decl_scope);
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
        case ZT_AST_TYPE_ALIAS_DECL:
            zt_bind_declare_name(binder, module_scope, ZT_SYMBOL_TYPE_ALIAS, decl->as.type_alias_decl.name, decl->span, 0);
            break;
        case ZT_AST_EXTERN_DECL:
            for (i = 0; i < decl->as.extern_decl.functions.count; i++) {
                const zt_ast_node *func = decl->as.extern_decl.functions.items[i];
                if (func != NULL) {
                    zt_bind_declare_name(binder, module_scope, ZT_SYMBOL_EXTERN_FUNC, func->as.func_decl.name, func->span, 0);
                }
            }
            break;
        case ZT_AST_CONST_DECL:
            zt_bind_declare_name(binder, module_scope, ZT_SYMBOL_LOCAL, decl->as.const_decl.name, decl->span, 0);
            break;
        case ZT_AST_VAR_DECL:
            zt_bind_declare_name(binder, module_scope, ZT_SYMBOL_LOCAL, decl->as.var_decl.name, decl->span, 1);
            break;
        default:
            break;
    }
}

zt_bind_result zt_bind_file(const zt_ast_node *root) {
    static const char *core_traits[] = { "Equatable", "Hashable", "TextRepresentable", "Comparable", NULL };
    zt_bind_result result;
    zt_binder binder;
    size_t i;

    zt_scope_init(&result.module_scope, NULL);
    result.diagnostics = zt_diag_list_make();
    binder.result = &result;
    binder.block_depth = 0;

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







