/*
 * compiler/zir/lowering/from_hir.c
 *
 * Functional HIR -> ZIR lowering used by the source build.
 */
#include "compiler/zir/lowering/from_hir.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct zir_instruction_buffer {
    zir_instruction *items;
    size_t count;
    size_t capacity;
} zir_instruction_buffer;

typedef struct zir_block_state {
    char *label;
    zir_instruction_buffer instructions;
    zir_terminator terminator;
    int has_terminator;
    zir_span span;
} zir_block_state;

typedef struct zir_block_state_buffer {
    zir_block_state *items;
    size_t count;
    size_t capacity;
} zir_block_state_buffer;

typedef struct zir_loop_target {
    const char *continue_label;
    const char *break_label;
} zir_loop_target;

typedef struct zir_function_buffer {
    zir_function *items;
    size_t count;
    size_t capacity;
} zir_function_buffer;

typedef struct zir_lower_env {
    const zt_hir_module *module_hir;
    zt_diag_list *diagnostics;
    zir_function_buffer *hoisted_functions;
    size_t *closure_counter;
} zir_lower_env;

typedef struct zir_function_ctx {
    zir_lower_env *env;
    zir_block_state_buffer blocks;
    size_t current_block;
    zir_loop_target loop_stack[32];
    size_t loop_depth;
    size_t if_label_counter;
    size_t while_label_counter;
    size_t repeat_label_counter;
    size_t for_label_counter;
    size_t match_label_counter;
    size_t repeat_temp_counter;
    size_t for_temp_counter;
    size_t match_subject_counter;
    size_t try_temp_counter;
    size_t try_label_counter;
    size_t generated_value_counter;
} zir_function_ctx;

typedef struct zir_decl_counts {
    size_t struct_count;
    size_t enum_count;
    size_t module_var_count;
    size_t function_count;
} zir_decl_counts;

typedef struct zir_nesting_guard {
    zt_diag_list *diagnostics;
    size_t depth;
    int limit_hit;
} zir_nesting_guard;

#define ZIR_LOWER_MAX_NESTING_DEPTH 128u

static void zir_add_lower_diag(
        zt_diag_list *diagnostics,
        zt_source_span span,
        const char *message) {
    if (diagnostics == NULL) return;
    zt_diag_list_add(
        diagnostics,
        ZT_DIAG_INVALID_TYPE,
        span,
        "%s",
        message != NULL ? message : "lowering error");
}

static int zir_nesting_push(zir_nesting_guard *guard, zt_source_span span, const char *label) {
    if (guard == NULL) return 1;
    if (guard->depth >= ZIR_LOWER_MAX_NESTING_DEPTH) {
        if (!guard->limit_hit && guard->diagnostics != NULL) {
            zt_diag_list_add(
                guard->diagnostics,
                ZT_DIAG_STRUCTURE_LIMIT_EXCEEDED,
                span,
                "%s nesting exceeds ZIR lowering limit (%u levels)",
                label != NULL ? label : "HIR",
                (unsigned)ZIR_LOWER_MAX_NESTING_DEPTH);
        }
        guard->limit_hit = 1;
        return 0;
    }
    guard->depth += 1;
    return 1;
}

static void zir_nesting_pop(zir_nesting_guard *guard) {
    if (guard != NULL && guard->depth > 0) {
        guard->depth -= 1;
    }
}

static int zir_validate_hir_expr_nesting(zir_nesting_guard *guard, const zt_hir_expr *expr);
static int zir_validate_hir_stmt_nesting(zir_nesting_guard *guard, const zt_hir_stmt *stmt);

static int zir_validate_hir_expr_list_nesting(zir_nesting_guard *guard, const zt_hir_expr_list *list) {
    size_t i;
    if (list == NULL) return 1;
    for (i = 0; i < list->count; i += 1) {
        if (!zir_validate_hir_expr_nesting(guard, list->items[i])) {
            return 0;
        }
    }
    return 1;
}

static int zir_validate_hir_expr_nesting(zir_nesting_guard *guard, const zt_hir_expr *expr) {
    size_t i;
    if (expr == NULL) return 1;
    if (!zir_nesting_push(guard, expr->span, "expression")) {
        return 0;
    }

    switch (expr->kind) {
        case ZT_HIR_SUCCESS_EXPR:
            if (!zir_validate_hir_expr_nesting(guard, expr->as.success_expr.value)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_ERROR_EXPR:
            if (!zir_validate_hir_expr_nesting(guard, expr->as.error_expr.value)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_LIST_EXPR:
            if (!zir_validate_hir_expr_list_nesting(guard, &expr->as.list_expr.elements)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_MAP_EXPR:
            for (i = 0; i < expr->as.map_expr.entries.count; i += 1) {
                const zt_hir_map_entry *entry = &expr->as.map_expr.entries.items[i];
                if (!zir_validate_hir_expr_nesting(guard, entry->key) ||
                        !zir_validate_hir_expr_nesting(guard, entry->value)) {
                    zir_nesting_pop(guard);
                    return 0;
                }
            }
            break;
        case ZT_HIR_UNARY_EXPR:
            if (!zir_validate_hir_expr_nesting(guard, expr->as.unary_expr.operand)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_BINARY_EXPR:
            if (!zir_validate_hir_expr_nesting(guard, expr->as.binary_expr.left) ||
                    !zir_validate_hir_expr_nesting(guard, expr->as.binary_expr.right)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_FIELD_EXPR:
            if (!zir_validate_hir_expr_nesting(guard, expr->as.field_expr.object)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_INDEX_EXPR:
            if (!zir_validate_hir_expr_nesting(guard, expr->as.index_expr.object) ||
                    !zir_validate_hir_expr_nesting(guard, expr->as.index_expr.index)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_SLICE_EXPR:
            if (!zir_validate_hir_expr_nesting(guard, expr->as.slice_expr.object) ||
                    !zir_validate_hir_expr_nesting(guard, expr->as.slice_expr.start) ||
                    !zir_validate_hir_expr_nesting(guard, expr->as.slice_expr.end)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_CALL_EXPR:
            if (!zir_validate_hir_expr_list_nesting(guard, &expr->as.call_expr.args)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_METHOD_CALL_EXPR:
            if (!zir_validate_hir_expr_nesting(guard, expr->as.method_call_expr.receiver) ||
                    !zir_validate_hir_expr_list_nesting(guard, &expr->as.method_call_expr.args)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_CLOSURE_EXPR:
            if (!zir_validate_hir_stmt_nesting(guard, expr->as.closure_expr.body)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_CONSTRUCT_EXPR:
            for (i = 0; i < expr->as.construct_expr.fields.count; i += 1) {
                if (!zir_validate_hir_expr_nesting(guard, expr->as.construct_expr.fields.items[i].value)) {
                    zir_nesting_pop(guard);
                    return 0;
                }
            }
            break;
        default:
            break;
    }

    zir_nesting_pop(guard);
    return 1;
}

static int zir_validate_hir_stmt_nesting(zir_nesting_guard *guard, const zt_hir_stmt *stmt) {
    size_t i;
    if (stmt == NULL) return 1;
    if (!zir_nesting_push(guard, stmt->span, "statement")) {
        return 0;
    }

    switch (stmt->kind) {
        case ZT_HIR_BLOCK_STMT:
            for (i = 0; i < stmt->as.block_stmt.statements.count; i += 1) {
                if (!zir_validate_hir_stmt_nesting(guard, stmt->as.block_stmt.statements.items[i])) {
                    zir_nesting_pop(guard);
                    return 0;
                }
            }
            break;
        case ZT_HIR_IF_STMT:
            if (!zir_validate_hir_expr_nesting(guard, stmt->as.if_stmt.condition) ||
                    !zir_validate_hir_stmt_nesting(guard, stmt->as.if_stmt.then_block) ||
                    !zir_validate_hir_stmt_nesting(guard, stmt->as.if_stmt.else_block)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_WHILE_STMT:
            if (!zir_validate_hir_expr_nesting(guard, stmt->as.while_stmt.condition) ||
                    !zir_validate_hir_stmt_nesting(guard, stmt->as.while_stmt.body)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_FOR_STMT:
            if (!zir_validate_hir_expr_nesting(guard, stmt->as.for_stmt.iterable) ||
                    !zir_validate_hir_stmt_nesting(guard, stmt->as.for_stmt.body)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_REPEAT_STMT:
            if (!zir_validate_hir_expr_nesting(guard, stmt->as.repeat_stmt.count) ||
                    !zir_validate_hir_stmt_nesting(guard, stmt->as.repeat_stmt.body)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_RETURN_STMT:
            if (!zir_validate_hir_expr_nesting(guard, stmt->as.return_stmt.value)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_CONST_STMT:
            if (!zir_validate_hir_expr_nesting(guard, stmt->as.const_stmt.init_value)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_VAR_STMT:
            if (!zir_validate_hir_expr_nesting(guard, stmt->as.var_stmt.init_value)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_ASSIGN_STMT:
            if (!zir_validate_hir_expr_nesting(guard, stmt->as.assign_stmt.value)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_INDEX_ASSIGN_STMT:
            if (!zir_validate_hir_expr_nesting(guard, stmt->as.index_assign_stmt.object) ||
                    !zir_validate_hir_expr_nesting(guard, stmt->as.index_assign_stmt.index) ||
                    !zir_validate_hir_expr_nesting(guard, stmt->as.index_assign_stmt.value)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_FIELD_ASSIGN_STMT:
            if (!zir_validate_hir_expr_nesting(guard, stmt->as.field_assign_stmt.object) ||
                    !zir_validate_hir_expr_nesting(guard, stmt->as.field_assign_stmt.value)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        case ZT_HIR_MATCH_STMT:
            if (!zir_validate_hir_expr_nesting(guard, stmt->as.match_stmt.subject)) {
                zir_nesting_pop(guard);
                return 0;
            }
            for (i = 0; i < stmt->as.match_stmt.cases.count; i += 1) {
                const zt_hir_match_case *match_case = &stmt->as.match_stmt.cases.items[i];
                if (!zir_validate_hir_expr_list_nesting(guard, &match_case->patterns) ||
                        !zir_validate_hir_stmt_nesting(guard, match_case->body)) {
                    zir_nesting_pop(guard);
                    return 0;
                }
            }
            break;
        case ZT_HIR_EXPR_STMT:
            if (!zir_validate_hir_expr_nesting(guard, stmt->as.expr_stmt.expr)) {
                zir_nesting_pop(guard);
                return 0;
            }
            break;
        default:
            break;
    }

    zir_nesting_pop(guard);
    return 1;
}

static int zir_validate_module_nesting(zir_lower_env *env, zt_diag_list *diagnostics) {
    zir_nesting_guard guard;
    size_t i;

    guard.diagnostics = diagnostics;
    guard.depth = 0;
    guard.limit_hit = 0;

    if (env->module_hir == NULL) return 0;

    for (i = 0; i < env->module_hir->module_vars.count; i += 1) {
        if (!zir_validate_hir_expr_nesting(&guard, env->module_hir->module_vars.items[i].init_value)) {
            return 0;
        }
    }

    for (i = 0; i < env->module_hir->declarations.count; i += 1) {
        const zt_hir_decl *decl = env->module_hir->declarations.items[i];
        size_t j;
        if (decl == NULL) continue;
        switch (decl->kind) {
            case ZT_HIR_STRUCT_DECL:
                for (j = 0; j < decl->as.struct_decl.fields.count; j += 1) {
                    const zt_hir_field_decl *field = &decl->as.struct_decl.fields.items[j];
                    if (!zir_validate_hir_expr_nesting(&guard, field->default_value) ||
                            !zir_validate_hir_expr_nesting(&guard, field->where_clause)) {
                        return 0;
                    }
                }
                break;
            case ZT_HIR_FUNC_DECL:
                for (j = 0; j < decl->as.func_decl.params.count; j += 1) {
                    if (!zir_validate_hir_expr_nesting(&guard, decl->as.func_decl.params.items[j].where_clause)) {
                        return 0;
                    }
                }
                if (!zir_validate_hir_stmt_nesting(&guard, decl->as.func_decl.body)) {
                    return 0;
                }
                break;
            default:
                break;
        }
    }

    return 1;
}

static void zir_function_buffer_init(zir_function_buffer *buffer) {
    buffer->items = NULL;
    buffer->count = 0;
    buffer->capacity = 0;
}

static void zir_function_buffer_push(zir_function_buffer *buffer, zir_function func) {
    if (buffer->count >= buffer->capacity) {
        size_t new_cap = buffer->capacity == 0 ? 4 : buffer->capacity * 2;
        zir_function *new_items = (zir_function *)realloc(buffer->items, new_cap * sizeof(zir_function));
        if (new_items == NULL) return;
        buffer->items = new_items;
        buffer->capacity = new_cap;
    }
    buffer->items[buffer->count++] = func;
}

static char *zir_lower_strdup(const char *text) {
    size_t len;
    char *copy;
    if (text == NULL) return NULL;
    len = strlen(text);
    copy = (char *)malloc(len + 1);
    if (copy == NULL) return NULL;
    memcpy(copy, text, len + 1);
    return copy;
}

static char *zir_lower_format(const char *prefix, size_t index) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%s%zu", prefix, index);
    return zir_lower_strdup(buffer);
}

static int zir_text_eq(const char *left, const char *right) {
    if (left == right) return 1;
    if (left == NULL || right == NULL) return 0;
    return strcmp(left, right) == 0;
}

static const char *zir_last_segment(const char *name) {
    const char *dot;
    if (name == NULL) return NULL;
    dot = strrchr(name, '.');
    return dot != NULL ? dot + 1 : name;
}

static int zir_name_matches(const char *left, const char *right) {
    if (zir_text_eq(left, right)) return 1;
    return zir_text_eq(zir_last_segment(left), zir_last_segment(right));
}

static int zir_call_is_module_func(const char *name, const char *module_name, const char *func_name) {
    const char *last_dot;
    const char *module_start;
    size_t module_len;
    if (name == NULL || module_name == NULL || func_name == NULL) return 0;
    last_dot = strrchr(name, '.');
    if (last_dot == NULL) {
        return zir_text_eq(name, func_name);
    }
    if (!zir_text_eq(last_dot + 1, func_name)) return 0;

    module_start = last_dot;
    while (module_start > name && module_start[-1] != '.') {
        module_start -= 1;
    }
    module_len = (size_t)(last_dot - module_start);
    return strlen(module_name) == module_len &&
           strncmp(module_start, module_name, module_len) == 0;
}

static int zir_starts_with(const char *text, const char *prefix) {
    size_t prefix_len;
    if (text == NULL || prefix == NULL) return 0;
    prefix_len = strlen(prefix);
    return strncmp(text, prefix, prefix_len) == 0;
}

static int zir_call_is_intrinsic(const zt_hir_expr *expr, const char *name) {
    if (expr == NULL || expr->kind != ZT_HIR_CALL_EXPR || name == NULL) return 0;
    return zir_name_matches(expr->as.call_expr.callee_name, name);
}

static int zir_type_name_is_optional(const char *type_name) {
    return zir_starts_with(type_name, "optional<");
}

static char *zir_type_name_owned(const zt_type *type);

static void zir_copy_sanitized(char *dest, size_t capacity, const char *source) {
    size_t out_index = 0;
    size_t index = 0;

    if (dest == NULL || capacity == 0) {
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

static void zir_strip_spaces_in_place(char *text) {
    size_t read_index = 0;
    size_t write_index = 0;

    if (text == NULL) {
        return;
    }

    while (text[read_index] != '\0') {
        if (text[read_index] != ' ') {
            text[write_index++] = text[read_index];
        }
        read_index += 1;
    }

    text[write_index] = '\0';
}

static int zir_build_map_runtime_name_from_type(
        const zt_type *map_type,
        const char *suffix,
        char *dest,
        size_t capacity) {
    char *type_name = NULL;
    char *key_type_name = NULL;
    char *value_type_name = NULL;
    char canonical_name[256];
    char sanitized[192];

    if (map_type == NULL || suffix == NULL || dest == NULL || capacity == 0) {
        return 0;
    }

    if (map_type->kind != ZT_TYPE_MAP || map_type->args.count < 2) {
        return 0;
    }

    key_type_name = zir_type_name_owned(map_type->args.items[0]);
    value_type_name = zir_type_name_owned(map_type->args.items[1]);
    if (key_type_name == NULL || value_type_name == NULL) {
        free(key_type_name);
        free(value_type_name);
        return 0;
    }

    zir_strip_spaces_in_place(key_type_name);
    zir_strip_spaces_in_place(value_type_name);

    snprintf(canonical_name, sizeof(canonical_name), "map<%s,%s>", key_type_name, value_type_name);
    type_name = canonical_name;

    if (strcmp(type_name, "map<text,text>") == 0) {
        snprintf(dest, capacity, "zt_map_text_text_%s", suffix);
        free(key_type_name);
        free(value_type_name);
        return 1;
    }

    zir_copy_sanitized(sanitized, sizeof(sanitized), type_name);
    snprintf(dest, capacity, "zt_map_generated_%s_%s", sanitized, suffix);
    free(key_type_name);
    free(value_type_name);
    return 1;
}

static char *zir_optional_inner_type_owned(const char *type_name) {
    const char *start;
    const char *cursor;
    size_t len;
    int depth = 1;
    char *inner;

    if (!zir_type_name_is_optional(type_name)) return NULL;

    start = type_name + strlen("optional<");
    cursor = start;
    while (*cursor != '\0') {
        if (*cursor == '<') {
            depth += 1;
        } else if (*cursor == '>') {
            depth -= 1;
            if (depth == 0) break;
        }
        cursor += 1;
    }

    if (depth != 0 || cursor <= start) return NULL;

    len = (size_t)(cursor - start);
    inner = (char *)malloc(len + 1);
    if (inner == NULL) return NULL;
    memcpy(inner, start, len);
    inner[len] = '\0';
    return inner;
}

static char *zir_result_branch_type_owned(const zt_type *type, size_t branch_index) {
    if (type != NULL &&
            type->kind == ZT_TYPE_RESULT &&
            type->args.count > branch_index &&
            type->args.items[branch_index] != NULL) {
        return zir_type_name_owned(type->args.items[branch_index]);
    }
    return zir_lower_strdup(branch_index == 0 ? "int" : "text");
}

static zir_span zir_span_from_source(zt_source_span span) {
    return zir_make_span(span.source_name, span.line, span.column_start);
}

static char *zir_type_name_owned(const zt_type *type) {
    char buffer[256];
    if (type == NULL) return zir_lower_strdup("void");
    zt_type_format(type, buffer, sizeof(buffer));
    return zir_lower_strdup(buffer);
}

static void zir_instruction_buffer_init(zir_instruction_buffer *buffer) {
    if (buffer == NULL) return;
    memset(buffer, 0, sizeof(*buffer));
}

static void zir_block_state_buffer_init(zir_block_state_buffer *buffer) {
    if (buffer == NULL) return;
    memset(buffer, 0, sizeof(*buffer));
}

static void *zir_lower_grow(void *items, size_t *capacity, size_t item_size) {
    size_t new_capacity;
    void *grown;
    if (capacity == NULL) return NULL;
    new_capacity = *capacity == 0 ? 4 : (*capacity * 2);
    grown = realloc(items, new_capacity * item_size);
    if (grown == NULL) return NULL;
    *capacity = new_capacity;
    return grown;
}

static int zir_instruction_buffer_push(zir_instruction_buffer *buffer, zir_instruction instruction) {
    zir_instruction *grown;
    if (buffer == NULL) return 0;
    if (buffer->count >= buffer->capacity) {
        grown = (zir_instruction *)zir_lower_grow(buffer->items, &buffer->capacity, sizeof(zir_instruction));
        if (grown == NULL) return 0;
        buffer->items = grown;
    }
    buffer->items[buffer->count++] = instruction;
    return 1;
}

static size_t zir_block_state_buffer_push(zir_block_state_buffer *buffer, zir_block_state block_state) {
    zir_block_state *grown;
    if (buffer == NULL) return 0;
    if (buffer->count >= buffer->capacity) {
        grown = (zir_block_state *)zir_lower_grow(buffer->items, &buffer->capacity, sizeof(zir_block_state));
        if (grown == NULL) return 0;
        buffer->items = grown;
    }
    buffer->items[buffer->count] = block_state;
    buffer->count += 1;
    return buffer->count - 1;
}

static zir_block_state *zir_current_block(zir_function_ctx *ctx) {
    if (ctx == NULL || ctx->blocks.count == 0 || ctx->current_block >= ctx->blocks.count) return NULL;
    return &ctx->blocks.items[ctx->current_block];
}

static size_t zir_add_block(zir_function_ctx *ctx, const char *label_text, zir_span span) {
    zir_block_state state;
    memset(&state, 0, sizeof(state));
    state.label = zir_lower_strdup(label_text);
    state.span = span;
    zir_instruction_buffer_init(&state.instructions);
    state.has_terminator = 0;
    return zir_block_state_buffer_push(&ctx->blocks, state);
}

static void zir_emit_instruction(zir_function_ctx *ctx, zir_instruction instruction) {
    zir_block_state *block = zir_current_block(ctx);
    if (block == NULL || block->has_terminator) {
        zir_instruction_dispose_owned(&instruction);
        return;
    }
    if (!zir_instruction_buffer_push(&block->instructions, instruction)) {
        zir_instruction_dispose_owned(&instruction);
    }
}

static void zir_set_terminator(zir_function_ctx *ctx, zir_terminator terminator) {
    zir_block_state *block = zir_current_block(ctx);
    if (block == NULL) {
        zir_terminator_dispose_owned(&terminator);
        return;
    }
    if (block->has_terminator) {
        zir_terminator_dispose_owned(&terminator);
        return;
    }
    block->terminator = terminator;
    block->has_terminator = 1;
}

static const zt_hir_decl *zir_find_struct_decl_hir(zir_lower_env *env, const char *name) {
    size_t i;
    if (env->module_hir == NULL || name == NULL) return NULL;
    for (i = 0; i < env->module_hir->declarations.count; i += 1) {
        const zt_hir_decl *decl = env->module_hir->declarations.items[i];
        if (decl == NULL || decl->kind != ZT_HIR_STRUCT_DECL) continue;
        if (zir_name_matches(decl->as.struct_decl.name, name)) return decl;
    }
    return NULL;
}

static const zt_hir_field_decl *zir_find_struct_field_hir(const zt_hir_decl *struct_decl, const char *field_name) {
    size_t i;
    if (struct_decl == NULL || struct_decl->kind != ZT_HIR_STRUCT_DECL || field_name == NULL) return NULL;
    for (i = 0; i < struct_decl->as.struct_decl.fields.count; i += 1) {
        const zt_hir_field_decl *field = &struct_decl->as.struct_decl.fields.items[i];
        if (zir_text_eq(field->name, field_name)) return field;
    }
    return NULL;
}

static int zir_find_enum_variant_index_hir(
        zir_lower_env *env,
        const char *enum_name,
        const char *variant_name,
        size_t *out_index) {
    size_t i;
    if (out_index != NULL) *out_index = 0;
    if (env->module_hir == NULL || enum_name == NULL || variant_name == NULL) return 0;
    for (i = 0; i < env->module_hir->declarations.count; i += 1) {
        const zt_hir_decl *decl = env->module_hir->declarations.items[i];
        size_t v;
        if (decl == NULL || decl->kind != ZT_HIR_ENUM_DECL) continue;
        if (!zir_name_matches(decl->as.enum_decl.name, enum_name)) continue;
        for (v = 0; v < decl->as.enum_decl.variants.count; v += 1) {
            const zt_hir_enum_variant *variant = &decl->as.enum_decl.variants.items[v];
            if (zir_text_eq(variant->name, variant_name)) {
                if (out_index != NULL) *out_index = v;
                return 1;
            }
        }
        return 0;
    }
    return 0;
}

static const char *zir_binary_op_name(zt_token_kind op) {
    switch (op) {
        case ZT_TOKEN_PLUS: return "add";
        case ZT_TOKEN_MINUS: return "sub";
        case ZT_TOKEN_STAR: return "mul";
        case ZT_TOKEN_SLASH: return "div";
        case ZT_TOKEN_PERCENT: return "mod";
        case ZT_TOKEN_EQEQ: return "eq";
        case ZT_TOKEN_NEQ: return "ne";
        case ZT_TOKEN_LT: return "lt";
        case ZT_TOKEN_LTE: return "le";
        case ZT_TOKEN_GT: return "gt";
        case ZT_TOKEN_GTE: return "ge";
        case ZT_TOKEN_AND: return "and";
        case ZT_TOKEN_OR: return "or";
        default: return "unknown";
    }
}

static const char *zir_unary_op_name(zt_token_kind op) {
    switch (op) {
        case ZT_TOKEN_MINUS: return "neg";
        case ZT_TOKEN_NOT:
        case ZT_TOKEN_BANG: return "not";
        default: return "unknown";
    }
}


static zir_function zir_lower_closure_as_function(
        zir_lower_env *env,
        const char *name,
        const zt_hir_expr *closure);
static zir_expr *zir_lower_hir_expr(
        zir_lower_env *env,
        const zt_hir_expr *expr,
        const char *replace_ident_from,
        const char *replace_ident_to,
        const char *replace_it_to);

static void zir_call_add_lowered_args(
        zir_expr *call,
        zir_lower_env *env,
        const zt_hir_expr_list *args,
        const char *replace_ident_from,
        const char *replace_ident_to,
        const char *replace_it_to) {
    size_t i;
    if (call == NULL || args == NULL) return;
    for (i = 0; i < args->count; i += 1) {
        zir_expr_call_add_arg(
            call,
            zir_lower_hir_expr(env, args->items[i], replace_ident_from, replace_ident_to, replace_it_to));
    }
}

static zir_expr *zir_lower_len_call(
        zir_lower_env *env,
        const zt_hir_expr *call_expr,
        const char *replace_ident_from,
        const char *replace_ident_to,
        const char *replace_it_to) {
    const zt_hir_expr *arg;
    zir_expr *arg_expr;
    zir_expr *call;

    if (call_expr->as.call_expr.args.count != 1) {
        return NULL;
    }

    arg = call_expr->as.call_expr.args.items[0];
    arg_expr = zir_lower_hir_expr(env, arg, replace_ident_from, replace_ident_to, replace_it_to);
    if (arg == NULL || arg->type == NULL) {
        return zir_expr_make_list_len(arg_expr);
    }

    if (arg->type->kind == ZT_TYPE_LIST) {
        return zir_expr_make_list_len(arg_expr);
    }

    if (arg->type->kind == ZT_TYPE_MAP) {
        return zir_expr_make_map_len(arg_expr);
    }
    if (arg->type->kind == ZT_TYPE_TEXT) {
        if (arg->kind == ZT_HIR_CALL_EXPR &&
                zir_name_matches(arg->as.call_expr.callee_name, "core.dyn_text_repr_to_text") &&
                arg->as.call_expr.args.count == 1) {
            call = zir_expr_make_call_extern("c.zt_dyn_text_repr_text_len");
            zir_expr_call_add_arg(
                call,
                zir_lower_hir_expr(env,
                    arg->as.call_expr.args.items[0],
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to));
            return call;
        }

        call = zir_expr_make_call_extern("c.zt_text_len");
        zir_expr_call_add_arg(call, arg_expr);
        return call;
    }

    if (arg->type->kind == ZT_TYPE_BYTES) {
        call = zir_expr_make_call_extern("c.zt_bytes_len");
        zir_expr_call_add_arg(call, arg_expr);
        return call;
    }

    return zir_expr_make_list_len(arg_expr);
}

static zir_expr *zir_lower_call_expr(
        zir_lower_env *env,
        const zt_hir_expr *expr,
        const char *replace_ident_from,
        const char *replace_ident_to,
        const char *replace_it_to) {
    const char *callee_name = expr->as.call_expr.callee_name != NULL ? expr->as.call_expr.callee_name : "";
    zir_expr *call = NULL;

    if (zir_text_eq(callee_name, "len")) {
        zir_expr *len_expr = zir_lower_len_call(env, expr, replace_ident_from, replace_ident_to, replace_it_to);
        if (len_expr != NULL) return len_expr;
    }

    if (zir_text_eq(callee_name, "check")) {
        call = zir_expr_make_call_extern("c.zt_check");
        if (expr->as.call_expr.args.count > 0) {
            zir_expr_call_add_arg(
                call,
                zir_lower_hir_expr(env,
                    expr->as.call_expr.args.items[0],
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to));
        } else {
            zir_expr_call_add_arg(call, zir_expr_make_bool(0));
        }
        zir_expr_call_add_arg(call, zir_expr_make_string("check failed"));
        return call;
    }

    if (zir_name_matches(callee_name, "core.Error")) {
        call = zir_expr_make_call_extern("c.zt_core_error_make");
        if (expr->as.call_expr.args.count > 0) {
            zir_expr_call_add_arg(
                call,
                zir_lower_hir_expr(env,
                    expr->as.call_expr.args.items[0],
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to));
        } else {
            zir_expr_call_add_arg(call, zir_expr_make_string(""));
        }

        if (expr->as.call_expr.args.count > 1) {
            zir_expr_call_add_arg(
                call,
                zir_lower_hir_expr(env,
                    expr->as.call_expr.args.items[1],
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to));
        } else {
            zir_expr_call_add_arg(call, zir_expr_make_string(""));
        }

        zir_expr_call_add_arg(call, zir_expr_make_optional_empty("text"));
        return call;
    }

    if (zir_name_matches(callee_name, "core.list_get_i64")) {
        call = zir_expr_make_call_extern("c.zt_list_i64_get_optional");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_name_matches(callee_name, "core.list_get_text")) {
        call = zir_expr_make_call_extern("c.zt_list_text_get_optional");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_name_matches(callee_name, "core.map_get")) {
        char runtime_name[224];
        char callee_buffer[256];
        const zt_type *map_type = NULL;

        if (expr->as.call_expr.args.count > 0 &&
                expr->as.call_expr.args.items[0] != NULL) {
            map_type = expr->as.call_expr.args.items[0]->type;
        }

        if (!zir_build_map_runtime_name_from_type(
                map_type,
                "get_optional",
                runtime_name,
                sizeof(runtime_name))) {
            snprintf(runtime_name, sizeof(runtime_name), "zt_map_text_text_get_optional");
        }

        snprintf(callee_buffer, sizeof(callee_buffer), "c.%s", runtime_name);
        call = zir_expr_make_call_extern(callee_buffer);
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_name_matches(callee_name, "core.fmt_box_i64")) {
        call = zir_expr_make_call_extern("c.zt_dyn_text_repr_from_i64");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_name_matches(callee_name, "core.fmt_box_float")) {
        call = zir_expr_make_call_extern("c.zt_dyn_text_repr_from_float");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_name_matches(callee_name, "core.fmt_box_bool")) {
        call = zir_expr_make_call_extern("c.zt_dyn_text_repr_from_bool");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_name_matches(callee_name, "core.fmt_box_text")) {
        call = zir_expr_make_call_extern("c.zt_dyn_text_repr_from_text");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_name_matches(callee_name, "core.dyn_text_repr_to_text")) {
        call = zir_expr_make_call_extern("c.zt_dyn_text_repr_to_text");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }

    if (zir_call_is_module_func(callee_name, "bytes", "zt_bytes_empty")) {
        call = zir_expr_make_call_extern("c.zt_bytes_empty");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "zt_bytes_from_list_i64")) {
        call = zir_expr_make_call_extern("c.zt_bytes_from_list_i64");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "zt_bytes_to_list_i64")) {
        call = zir_expr_make_call_extern("c.zt_bytes_to_list_i64");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "zt_bytes_join")) {
        call = zir_expr_make_call_extern("c.zt_bytes_join");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "zt_bytes_starts_with")) {
        call = zir_expr_make_call_extern("c.zt_bytes_starts_with");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "zt_bytes_ends_with")) {
        call = zir_expr_make_call_extern("c.zt_bytes_ends_with");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "zt_bytes_contains")) {
        call = zir_expr_make_call_extern("c.zt_bytes_contains");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "from_list")) {
        call = zir_expr_make_call_extern("c.zt_bytes_from_list_i64");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "to_list")) {
        call = zir_expr_make_call_extern("c.zt_bytes_to_list_i64");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "join")) {
        call = zir_expr_make_call_extern("c.zt_bytes_join");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "starts_with")) {
        call = zir_expr_make_call_extern("c.zt_bytes_starts_with");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "ends_with")) {
        call = zir_expr_make_call_extern("c.zt_bytes_ends_with");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "contains")) {
        call = zir_expr_make_call_extern("c.zt_bytes_contains");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "empty")) {
        call = zir_expr_make_call_extern("c.zt_bytes_empty");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "text", "to_utf8")) {
        call = zir_expr_make_call_extern("c.zt_text_to_utf8_bytes");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "text", "from_utf8")) {
        call = zir_expr_make_call_extern("c.zt_text_from_utf8_bytes");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "concurrent", "zt_thread_boundary_copy_text")) {
        call = zir_expr_make_call_extern("c.zt_thread_boundary_copy_text");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "concurrent", "zt_thread_boundary_copy_bytes")) {
        call = zir_expr_make_call_extern("c.zt_thread_boundary_copy_bytes");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "concurrent", "zt_thread_boundary_copy_list_i64")) {
        call = zir_expr_make_call_extern("c.zt_thread_boundary_copy_list_i64");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "concurrent", "zt_thread_boundary_copy_list_text")) {
        call = zir_expr_make_call_extern("c.zt_thread_boundary_copy_list_text");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "concurrent", "zt_thread_boundary_copy_map_text_text")) {
        call = zir_expr_make_call_extern("c.zt_thread_boundary_copy_map_text_text");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "io", "zt_host_write_stdout")) {
        call = zir_expr_make_call_extern("c.zt_host_write_stdout");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "io", "zt_host_write_stderr")) {
        call = zir_expr_make_call_extern("c.zt_host_write_stderr");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "io", "zt_host_read_line_stdin")) {
        call = zir_expr_make_call_extern("c.zt_host_read_line_stdin");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "io", "zt_host_read_all_stdin")) {
        call = zir_expr_make_call_extern("c.zt_host_read_all_stdin");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "test", "zt_test_fail")) {
        call = zir_expr_make_call_extern("c.zt_test_fail");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "test", "zt_test_skip")) {
        call = zir_expr_make_call_extern("c.zt_test_skip");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "fs", "zt_host_read_file")) {
        call = zir_expr_make_call_extern("c.zt_host_read_file");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "fs", "zt_host_write_file")) {
        call = zir_expr_make_call_extern("c.zt_host_write_file");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "fs", "zt_host_path_exists")) {
        call = zir_expr_make_call_extern("c.zt_host_path_exists");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "time", "zt_host_time_now_unix_ms")) {
        call = zir_expr_make_call_extern("c.zt_host_time_now_unix_ms");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "time", "zt_host_time_sleep_ms")) {
        call = zir_expr_make_call_extern("c.zt_host_time_sleep_ms");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "random", "zt_host_random_seed")) {
        call = zir_expr_make_call_extern("c.zt_host_random_seed");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "random", "zt_host_random_next_i64")) {
        call = zir_expr_make_call_extern("c.zt_host_random_next_i64");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "lazy", "zt_lazy_i64_once")) {
        call = zir_expr_make_call_extern("c.zt_lazy_i64_once");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "lazy", "zt_lazy_i64_force")) {
        call = zir_expr_make_call_extern("c.zt_lazy_i64_force");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "lazy", "zt_lazy_i64_is_consumed")) {
        call = zir_expr_make_call_extern("c.zt_lazy_i64_is_consumed");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "os", "zt_host_os_current_dir")) {
        call = zir_expr_make_call_extern("c.zt_host_os_current_dir");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "os", "zt_host_os_change_dir")) {
        call = zir_expr_make_call_extern("c.zt_host_os_change_dir");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "os", "zt_host_os_env")) {
        call = zir_expr_make_call_extern("c.zt_host_os_env");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "os", "zt_host_os_pid")) {
        call = zir_expr_make_call_extern("c.zt_host_os_pid");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "os", "zt_host_os_platform")) {
        call = zir_expr_make_call_extern("c.zt_host_os_platform");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "os", "zt_host_os_arch")) {
        call = zir_expr_make_call_extern("c.zt_host_os_arch");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "process", "zt_host_process_run")) {
        call = zir_expr_make_call_extern("c.zt_host_process_run");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "net", "zt_text_eq")) {
        call = zir_expr_make_call_extern("c.zt_text_eq");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "net", "zt_net_connect")) {
        call = zir_expr_make_call_extern("c.zt_net_connect");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "net", "zt_net_read_some")) {
        call = zir_expr_make_call_extern("c.zt_net_read_some");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "net", "zt_net_write_all")) {
        call = zir_expr_make_call_extern("c.zt_net_write_all");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "net", "zt_net_close")) {
        call = zir_expr_make_call_extern("c.zt_net_close");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "net", "zt_net_is_closed")) {
        call = zir_expr_make_call_extern("c.zt_net_is_closed");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "net", "zt_net_error_kind_index")) {
        call = zir_expr_make_call_extern("c.zt_net_error_kind_index");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "path", "zt_text_concat") ||
            zir_call_is_module_func(callee_name, "text", "zt_text_concat")) {
        call = zir_expr_make_call_extern("c.zt_text_concat");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "path", "zt_text_eq")) {
        call = zir_expr_make_call_extern("c.zt_text_eq");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "path", "zt_path_normalize")) {
        call = zir_expr_make_call_extern("c.zt_path_normalize");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "path", "zt_path_is_absolute")) {
        call = zir_expr_make_call_extern("c.zt_path_is_absolute");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "path", "zt_path_absolute")) {
        call = zir_expr_make_call_extern("c.zt_path_absolute");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "path", "zt_path_relative")) {
        call = zir_expr_make_call_extern("c.zt_path_relative");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "json", "zt_json_parse_map_text_text")) {
        call = zir_expr_make_call_extern("c.zt_json_parse_map_text_text");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "json", "zt_json_stringify_map_text_text")) {
        call = zir_expr_make_call_extern("c.zt_json_stringify_map_text_text");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "json", "zt_json_pretty_map_text_text")) {
        call = zir_expr_make_call_extern("c.zt_json_pretty_map_text_text");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "format", "zt_format_hex_i64")) {
        call = zir_expr_make_call_extern("c.zt_format_hex_i64");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "format", "zt_format_number")) {
        call = zir_expr_make_call_extern("c.zt_format_number");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "format", "zt_format_percent")) {
        call = zir_expr_make_call_extern("c.zt_format_percent");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "format", "zt_format_date")) {
        call = zir_expr_make_call_extern("c.zt_format_date");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "format", "zt_format_datetime")) {
        call = zir_expr_make_call_extern("c.zt_format_datetime");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "format", "zt_format_date_pattern")) {
        call = zir_expr_make_call_extern("c.zt_format_date_pattern");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "format", "zt_format_datetime_pattern")) {
        call = zir_expr_make_call_extern("c.zt_format_datetime_pattern");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "format", "zt_format_bin_i64")) {
        call = zir_expr_make_call_extern("c.zt_format_bin_i64");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "format", "zt_format_bytes_binary")) {
        call = zir_expr_make_call_extern("c.zt_format_bytes_binary");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "format", "zt_format_bytes_decimal")) {
        call = zir_expr_make_call_extern("c.zt_format_bytes_decimal");
        zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_pow")) { call = zir_expr_make_call_extern("c.zt_math_pow"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_sqrt")) { call = zir_expr_make_call_extern("c.zt_math_sqrt"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_floor")) { call = zir_expr_make_call_extern("c.zt_math_floor"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_ceil")) { call = zir_expr_make_call_extern("c.zt_math_ceil"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_round_half_away_from_zero")) { call = zir_expr_make_call_extern("c.zt_math_round_half_away_from_zero"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_trunc")) { call = zir_expr_make_call_extern("c.zt_math_trunc"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_sin")) { call = zir_expr_make_call_extern("c.zt_math_sin"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_cos")) { call = zir_expr_make_call_extern("c.zt_math_cos"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_tan")) { call = zir_expr_make_call_extern("c.zt_math_tan"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_asin")) { call = zir_expr_make_call_extern("c.zt_math_asin"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_acos")) { call = zir_expr_make_call_extern("c.zt_math_acos"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_atan")) { call = zir_expr_make_call_extern("c.zt_math_atan"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_atan2")) { call = zir_expr_make_call_extern("c.zt_math_atan2"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_ln")) { call = zir_expr_make_call_extern("c.zt_math_ln"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_log_ten")) { call = zir_expr_make_call_extern("c.zt_math_log_ten"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_log10")) { call = zir_expr_make_call_extern("c.zt_math_log10"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_log2")) { call = zir_expr_make_call_extern("c.zt_math_log2"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_log")) { call = zir_expr_make_call_extern("c.zt_math_log"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_exp")) { call = zir_expr_make_call_extern("c.zt_math_exp"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_is_nan")) { call = zir_expr_make_call_extern("c.zt_math_is_nan"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_is_infinite")) { call = zir_expr_make_call_extern("c.zt_math_is_infinite"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "math", "zt_math_is_finite")) { call = zir_expr_make_call_extern("c.zt_math_is_finite"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_queue_i64_new")) { call = zir_expr_make_call_extern("c.zt_queue_i64_new"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_queue_i64_enqueue")) { call = zir_expr_make_call_extern("c.zt_queue_i64_enqueue"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_queue_i64_dequeue")) { call = zir_expr_make_call_extern("c.zt_queue_i64_dequeue"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_queue_i64_peek")) { call = zir_expr_make_call_extern("c.zt_queue_i64_peek"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_queue_text_new")) { call = zir_expr_make_call_extern("c.zt_queue_text_new"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_queue_text_enqueue")) { call = zir_expr_make_call_extern("c.zt_queue_text_enqueue"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_queue_text_dequeue")) { call = zir_expr_make_call_extern("c.zt_queue_text_dequeue"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_queue_text_peek")) { call = zir_expr_make_call_extern("c.zt_queue_text_peek"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_stack_i64_new")) { call = zir_expr_make_call_extern("c.zt_stack_i64_new"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_stack_i64_push")) { call = zir_expr_make_call_extern("c.zt_stack_i64_push"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_stack_i64_pop")) { call = zir_expr_make_call_extern("c.zt_stack_i64_pop"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_stack_i64_peek")) { call = zir_expr_make_call_extern("c.zt_stack_i64_peek"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_stack_text_new")) { call = zir_expr_make_call_extern("c.zt_stack_text_new"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_stack_text_push")) { call = zir_expr_make_call_extern("c.zt_stack_text_push"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_stack_text_pop")) { call = zir_expr_make_call_extern("c.zt_stack_text_pop"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_stack_text_peek")) { call = zir_expr_make_call_extern("c.zt_stack_text_peek"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_i64_new")) { call = zir_expr_make_call_extern("c.zt_grid2d_i64_new"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_i64_get")) { call = zir_expr_make_call_extern("c.zt_grid2d_i64_get"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_i64_set")) { call = zir_expr_make_call_extern("c.zt_grid2d_i64_set"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_i64_set_owned")) { call = zir_expr_make_call_extern("c.zt_grid2d_i64_set_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_i64_fill")) { call = zir_expr_make_call_extern("c.zt_grid2d_i64_fill"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_i64_fill_owned")) { call = zir_expr_make_call_extern("c.zt_grid2d_i64_fill_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_i64_rows")) { call = zir_expr_make_call_extern("c.zt_grid2d_i64_rows"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_i64_cols")) { call = zir_expr_make_call_extern("c.zt_grid2d_i64_cols"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_text_new")) { call = zir_expr_make_call_extern("c.zt_grid2d_text_new"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_text_get")) { call = zir_expr_make_call_extern("c.zt_grid2d_text_get"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_text_set")) { call = zir_expr_make_call_extern("c.zt_grid2d_text_set"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_text_set_owned")) { call = zir_expr_make_call_extern("c.zt_grid2d_text_set_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_text_fill")) { call = zir_expr_make_call_extern("c.zt_grid2d_text_fill"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_text_fill_owned")) { call = zir_expr_make_call_extern("c.zt_grid2d_text_fill_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_text_rows")) { call = zir_expr_make_call_extern("c.zt_grid2d_text_rows"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid2d_text_cols")) { call = zir_expr_make_call_extern("c.zt_grid2d_text_cols"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_pqueue_i64_new")) { call = zir_expr_make_call_extern("c.zt_pqueue_i64_new"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_pqueue_i64_push")) { call = zir_expr_make_call_extern("c.zt_pqueue_i64_push"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_pqueue_i64_push_owned")) { call = zir_expr_make_call_extern("c.zt_pqueue_i64_push_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_pqueue_i64_pop")) { call = zir_expr_make_call_extern("c.zt_pqueue_i64_pop"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_pqueue_i64_peek")) { call = zir_expr_make_call_extern("c.zt_pqueue_i64_peek"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_pqueue_i64_len")) { call = zir_expr_make_call_extern("c.zt_pqueue_i64_len"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_pqueue_text_new")) { call = zir_expr_make_call_extern("c.zt_pqueue_text_new"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_pqueue_text_push")) { call = zir_expr_make_call_extern("c.zt_pqueue_text_push"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_pqueue_text_push_owned")) { call = zir_expr_make_call_extern("c.zt_pqueue_text_push_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_pqueue_text_pop")) { call = zir_expr_make_call_extern("c.zt_pqueue_text_pop"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_pqueue_text_peek")) { call = zir_expr_make_call_extern("c.zt_pqueue_text_peek"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_pqueue_text_len")) { call = zir_expr_make_call_extern("c.zt_pqueue_text_len"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_i64_new")) { call = zir_expr_make_call_extern("c.zt_circbuf_i64_new"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_i64_push")) { call = zir_expr_make_call_extern("c.zt_circbuf_i64_push"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_i64_push_owned")) { call = zir_expr_make_call_extern("c.zt_circbuf_i64_push_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_i64_pop")) { call = zir_expr_make_call_extern("c.zt_circbuf_i64_pop"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_i64_peek")) { call = zir_expr_make_call_extern("c.zt_circbuf_i64_peek"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_i64_len")) { call = zir_expr_make_call_extern("c.zt_circbuf_i64_len"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_i64_capacity")) { call = zir_expr_make_call_extern("c.zt_circbuf_i64_capacity"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_i64_is_full")) { call = zir_expr_make_call_extern("c.zt_circbuf_i64_is_full"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_text_new")) { call = zir_expr_make_call_extern("c.zt_circbuf_text_new"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_text_push")) { call = zir_expr_make_call_extern("c.zt_circbuf_text_push"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_text_push_owned")) { call = zir_expr_make_call_extern("c.zt_circbuf_text_push_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_text_pop")) { call = zir_expr_make_call_extern("c.zt_circbuf_text_pop"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_text_peek")) { call = zir_expr_make_call_extern("c.zt_circbuf_text_peek"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_text_len")) { call = zir_expr_make_call_extern("c.zt_circbuf_text_len"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_text_capacity")) { call = zir_expr_make_call_extern("c.zt_circbuf_text_capacity"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_circbuf_text_is_full")) { call = zir_expr_make_call_extern("c.zt_circbuf_text_is_full"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreemap_text_text_new")) { call = zir_expr_make_call_extern("c.zt_btreemap_text_text_new"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreemap_text_text_set")) { call = zir_expr_make_call_extern("c.zt_btreemap_text_text_set"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreemap_text_text_set_owned")) { call = zir_expr_make_call_extern("c.zt_btreemap_text_text_set_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreemap_text_text_get")) { call = zir_expr_make_call_extern("c.zt_btreemap_text_text_get"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreemap_text_text_get_optional")) { call = zir_expr_make_call_extern("c.zt_btreemap_text_text_get_optional"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreemap_text_text_contains")) { call = zir_expr_make_call_extern("c.zt_btreemap_text_text_contains"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreemap_text_text_remove")) { call = zir_expr_make_call_extern("c.zt_btreemap_text_text_remove"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreemap_text_text_remove_owned")) { call = zir_expr_make_call_extern("c.zt_btreemap_text_text_remove_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreemap_text_text_len")) { call = zir_expr_make_call_extern("c.zt_btreemap_text_text_len"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreeset_text_new")) { call = zir_expr_make_call_extern("c.zt_btreeset_text_new"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreeset_text_insert")) { call = zir_expr_make_call_extern("c.zt_btreeset_text_insert"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreeset_text_insert_owned")) { call = zir_expr_make_call_extern("c.zt_btreeset_text_insert_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreeset_text_contains")) { call = zir_expr_make_call_extern("c.zt_btreeset_text_contains"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreeset_text_remove")) { call = zir_expr_make_call_extern("c.zt_btreeset_text_remove"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreeset_text_remove_owned")) { call = zir_expr_make_call_extern("c.zt_btreeset_text_remove_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_btreeset_text_len")) { call = zir_expr_make_call_extern("c.zt_btreeset_text_len"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_i64_new")) { call = zir_expr_make_call_extern("c.zt_grid3d_i64_new"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_i64_get")) { call = zir_expr_make_call_extern("c.zt_grid3d_i64_get"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_i64_set")) { call = zir_expr_make_call_extern("c.zt_grid3d_i64_set"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_i64_set_owned")) { call = zir_expr_make_call_extern("c.zt_grid3d_i64_set_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_i64_fill")) { call = zir_expr_make_call_extern("c.zt_grid3d_i64_fill"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_i64_fill_owned")) { call = zir_expr_make_call_extern("c.zt_grid3d_i64_fill_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_i64_depth")) { call = zir_expr_make_call_extern("c.zt_grid3d_i64_depth"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_i64_rows")) { call = zir_expr_make_call_extern("c.zt_grid3d_i64_rows"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_i64_cols")) { call = zir_expr_make_call_extern("c.zt_grid3d_i64_cols"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_text_new")) { call = zir_expr_make_call_extern("c.zt_grid3d_text_new"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_text_get")) { call = zir_expr_make_call_extern("c.zt_grid3d_text_get"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_text_set")) { call = zir_expr_make_call_extern("c.zt_grid3d_text_set"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_text_set_owned")) { call = zir_expr_make_call_extern("c.zt_grid3d_text_set_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_text_fill")) { call = zir_expr_make_call_extern("c.zt_grid3d_text_fill"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_text_fill_owned")) { call = zir_expr_make_call_extern("c.zt_grid3d_text_fill_owned"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_text_depth")) { call = zir_expr_make_call_extern("c.zt_grid3d_text_depth"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_text_rows")) { call = zir_expr_make_call_extern("c.zt_grid3d_text_rows"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_call_is_module_func(callee_name, "collections", "zt_grid3d_text_cols")) { call = zir_expr_make_call_extern("c.zt_grid3d_text_cols"); zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to); return call; }
    if (zir_starts_with(callee_name, "c.")) {
        call = zir_expr_make_call_extern(callee_name);
    } else {
        call = zir_expr_make_call_direct(callee_name);
    }
    zir_call_add_lowered_args(call, env, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
    return call;
}


static zir_function zir_lower_closure_as_function(
        zir_lower_env *env,
        const char *name,
        const zt_hir_expr *closure);
static zir_expr *zir_lower_hir_expr(
        zir_lower_env *env,
        const zt_hir_expr *expr,
        const char *replace_ident_from,
        const char *replace_ident_to,
        const char *replace_it_to) {
    zir_expr *out;
    size_t i;

    (void)env->module_hir;
    if (expr == NULL) return NULL;

    switch (expr->kind) {
        case ZT_HIR_IDENT_EXPR: {
            const char *name = expr->as.ident_expr.name;
            if (name == NULL) return zir_expr_make_name("<null>");
            if (replace_ident_from != NULL && replace_ident_to != NULL && zir_text_eq(name, replace_ident_from)) {
                return zir_expr_make_name(replace_ident_to);
            }
            if (replace_it_to != NULL && zir_text_eq(name, "it")) {
                return zir_expr_make_name(replace_it_to);
            }
            return zir_expr_make_name(name);
        }

        case ZT_HIR_INT_EXPR:
            return zir_expr_make_int(expr->as.int_expr.value);

        case ZT_HIR_FLOAT_EXPR:
            return zir_expr_make_float(expr->as.float_expr.value);

        case ZT_HIR_STRING_EXPR:
            return zir_expr_make_string(expr->as.string_expr.value);

        case ZT_HIR_BYTES_EXPR:
            return zir_expr_make_bytes(expr->as.bytes_expr.value);

        case ZT_HIR_BOOL_EXPR:
            return zir_expr_make_bool(expr->as.bool_expr.value);

        case ZT_HIR_NONE_EXPR: {
            char *inner = NULL;
            if (expr->type != NULL && expr->type->kind == ZT_TYPE_OPTIONAL && expr->type->args.count > 0) {
                inner = zir_type_name_owned(expr->type->args.items[0]);
            }
            if (inner == NULL) inner = zir_lower_strdup("int");
            out = zir_expr_make_optional_empty(inner);
            free(inner);
            return out;
        }

        case ZT_HIR_SUCCESS_EXPR: {
            zir_expr *value = zir_lower_hir_expr(env,
                expr->as.success_expr.value,
                replace_ident_from,
                replace_ident_to,
                replace_it_to);
            if (expr->type != NULL &&
                    expr->type->kind == ZT_TYPE_RESULT &&
                    expr->type->args.count > 0 &&
                    expr->type->args.items[0] != NULL &&
                    expr->type->args.items[0]->kind == ZT_TYPE_VOID) {
                if (value != NULL) zir_expr_dispose(value);
                return zir_expr_make_outcome_success(NULL);
            }
            return zir_expr_make_outcome_success(value);
        }

        case ZT_HIR_ERROR_EXPR:
            return zir_expr_make_outcome_failure(zir_lower_hir_expr(env,
                expr->as.error_expr.value,
                replace_ident_from,
                replace_ident_to,
                replace_it_to));

        case ZT_HIR_LIST_EXPR: {
            char *item_type = zir_lower_strdup("unknown");
            if (expr->type != NULL && expr->type->kind == ZT_TYPE_LIST && expr->type->args.count > 0) {
                free(item_type);
                item_type = zir_type_name_owned(expr->type->args.items[0]);
            }
            out = zir_expr_make_make_list(item_type);
            free(item_type);
            for (i = 0; i < expr->as.list_expr.elements.count; i += 1) {
                zir_expr_make_list_add_item(
                    out,
                    zir_lower_hir_expr(env,
                        expr->as.list_expr.elements.items[i],
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            }
            return out;
        }

        case ZT_HIR_MAP_EXPR: {
            char *key_type = zir_lower_strdup("unknown");
            char *value_type = zir_lower_strdup("unknown");
            if (expr->type != NULL && expr->type->kind == ZT_TYPE_MAP && expr->type->args.count >= 2) {
                free(key_type);
                free(value_type);
                key_type = zir_type_name_owned(expr->type->args.items[0]);
                value_type = zir_type_name_owned(expr->type->args.items[1]);
            }
            out = zir_expr_make_make_map(key_type, value_type);
            free(key_type);
            free(value_type);
            for (i = 0; i < expr->as.map_expr.entries.count; i += 1) {
                zir_expr_make_map_add_entry(
                    out,
                    zir_lower_hir_expr(env,
                        expr->as.map_expr.entries.items[i].key,
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to),
                    zir_lower_hir_expr(env,
                        expr->as.map_expr.entries.items[i].value,
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            }
            return out;
        }

        case ZT_HIR_UNARY_EXPR:
            if (expr->as.unary_expr.op == ZT_TOKEN_QUESTION) {
                return zir_expr_make_try_propagate(zir_lower_hir_expr(env,
                    expr->as.unary_expr.operand,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to));
            }
            return zir_expr_make_unary(
                zir_unary_op_name(expr->as.unary_expr.op),
                zir_lower_hir_expr(env,
                    expr->as.unary_expr.operand,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to));

        case ZT_HIR_BINARY_EXPR:
            return zir_expr_make_binary(
                zir_binary_op_name(expr->as.binary_expr.op),
                zir_lower_hir_expr(env,
                    expr->as.binary_expr.left,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to),
                zir_lower_hir_expr(env,
                    expr->as.binary_expr.right,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to));

        case ZT_HIR_FIELD_EXPR:
            if (expr->as.field_expr.object != NULL &&
                    expr->as.field_expr.object->kind == ZT_HIR_IDENT_EXPR &&
                    expr->as.field_expr.field_name != NULL) {
                size_t variant_index = 0;
                if (zir_find_enum_variant_index_hir(env,
                        expr->as.field_expr.object->as.ident_expr.name,
                        expr->as.field_expr.field_name,
                        &variant_index)) {
                    char tag_buffer[32];
                    zir_expr *enum_construct;
                    zir_expr *tag_value;
                    snprintf(tag_buffer, sizeof(tag_buffer), "%zu", variant_index);
                    enum_construct = zir_expr_make_make_struct(expr->as.field_expr.object->as.ident_expr.name);
                    tag_value = zir_expr_make_int(tag_buffer);
                    zir_expr_make_struct_add_field(enum_construct, "__zt_enum_tag", tag_value);
                    return enum_construct;
                }
            }
            return zir_expr_make_get_field(
                zir_lower_hir_expr(env,
                    expr->as.field_expr.object,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to),
                expr->as.field_expr.field_name);

        case ZT_HIR_INDEX_EXPR:
            return zir_expr_make_index_seq(
                zir_lower_hir_expr(env,
                    expr->as.index_expr.object,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to),
                zir_lower_hir_expr(env,
                    expr->as.index_expr.index,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to));

        case ZT_HIR_SLICE_EXPR:
        {
            zir_expr *object_expr = zir_lower_hir_expr(env,
                expr->as.slice_expr.object,
                replace_ident_from,
                replace_ident_to,
                replace_it_to);
            zir_expr *start_expr = NULL;
            zir_expr *end_expr = NULL;
            if (expr->as.slice_expr.start != NULL) {
                start_expr = zir_lower_hir_expr(env,
                    expr->as.slice_expr.start,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to);
            } else {
                start_expr = zir_expr_make_int("0");
            }

            if (expr->as.slice_expr.end != NULL) {
                end_expr = zir_lower_hir_expr(env,
                    expr->as.slice_expr.end,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to);
            } else if (expr->as.slice_expr.object != NULL &&
                    expr->as.slice_expr.object->type != NULL &&
                    expr->as.slice_expr.object->type->kind == ZT_TYPE_TEXT) {
                end_expr = zir_expr_make_call_extern("c.zt_text_len");
                zir_expr_call_add_arg(
                    end_expr,
                    zir_lower_hir_expr(env,
                        expr->as.slice_expr.object,
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            } else if (expr->as.slice_expr.object != NULL &&
                    expr->as.slice_expr.object->type != NULL &&
                    expr->as.slice_expr.object->type->kind == ZT_TYPE_BYTES) {
                end_expr = zir_expr_make_call_extern("c.zt_bytes_len");
                zir_expr_call_add_arg(
                    end_expr,
                    zir_lower_hir_expr(env,
                        expr->as.slice_expr.object,
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            } else if (expr->as.slice_expr.object != NULL &&
                    expr->as.slice_expr.object->type != NULL &&
                    expr->as.slice_expr.object->type->kind == ZT_TYPE_MAP) {
                end_expr = zir_expr_make_map_len(
                    zir_lower_hir_expr(env,
                        expr->as.slice_expr.object,
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            } else {
                end_expr = zir_expr_make_list_len(
                    zir_lower_hir_expr(env,
                        expr->as.slice_expr.object,
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            }

            return zir_expr_make_slice_seq(object_expr, start_expr, end_expr);
        }

        case ZT_HIR_CALL_EXPR:
            return zir_lower_call_expr(env, expr, replace_ident_from, replace_ident_to, replace_it_to);

        case ZT_HIR_METHOD_CALL_EXPR: {
            zir_expr *call = zir_expr_make_call_direct(expr->as.method_call_expr.method_name);
            zir_expr_call_add_arg(
                call,
                zir_lower_hir_expr(env,
                    expr->as.method_call_expr.receiver,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to));
            for (i = 0; i < expr->as.method_call_expr.args.count; i += 1) {
                zir_expr_call_add_arg(
                    call,
                    zir_lower_hir_expr(env,
                        expr->as.method_call_expr.args.items[i],
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            }
            return call;
        }

        case ZT_HIR_DYN_METHOD_CALL_EXPR: {
            zir_expr *receiver = zir_lower_hir_expr(env,
                expr->as.dyn_method_call_expr.receiver,
                replace_ident_from,
                replace_ident_to,
                replace_it_to);
            zir_expr *call = zir_expr_make_call_dyn(
                receiver,
                expr->as.dyn_method_call_expr.method_name,
                expr->as.dyn_method_call_expr.trait_name);
            for (i = 0; i < expr->as.dyn_method_call_expr.args.count; i += 1) {
                zir_expr_call_add_arg(
                    call,
                    zir_lower_hir_expr(env,
                        expr->as.dyn_method_call_expr.args.items[i],
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            }
            return call;
        }


        case ZT_HIR_CLOSURE_EXPR: {
            char name_buf[128];
            size_t i;
            zir_expr *out;
            size_t next_closure_index = 0;
            if (env->closure_counter != NULL) {
                next_closure_index = (*env->closure_counter)++;
            }
            snprintf(name_buf, sizeof(name_buf), "__zt_closure_%zu", next_closure_index);
            
            zir_function_buffer_push(env->hoisted_functions, 
                zir_lower_closure_as_function(env, name_buf, expr));
            
            out = zir_expr_make_make_closure(name_buf);
            for (i = 0; i < expr->as.closure_expr.captures.count; i++) {
                zir_expr_list_push(&out->as.make_closure.captures, 
                    zir_expr_make_name(expr->as.closure_expr.captures.items[i].name));
            }
            return out;
        }
        case ZT_HIR_CONSTRUCT_EXPR: {
            out = zir_expr_make_make_struct(expr->as.construct_expr.type_name);
            for (i = 0; i < expr->as.construct_expr.fields.count; i += 1) {
                const zt_hir_field_init *field = &expr->as.construct_expr.fields.items[i];
                zir_expr_make_struct_add_field(
                    out,
                    field->name,
                    zir_lower_hir_expr(env,
                        field->value,
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            }
            return out;
        }

        case ZT_HIR_FUNC_REF_EXPR: {
            char type_buffer[256];
            const char *type_name = "func() -> void";
            if (expr->as.func_ref_expr.callable_type != NULL) {
                zt_type_format(expr->as.func_ref_expr.callable_type, type_buffer, sizeof(type_buffer));
                type_name = type_buffer;
            }
            return zir_expr_make_func_ref(expr->as.func_ref_expr.func_name, type_name);
        }

        case ZT_HIR_CALL_INDIRECT_EXPR: {
            zir_expr *callable_expr = zir_lower_hir_expr(env,
                expr->as.call_indirect_expr.callable,
                replace_ident_from,
                replace_ident_to,
                replace_it_to);
            zir_expr *call = zir_expr_make_call_indirect(callable_expr);
            for (i = 0; i < expr->as.call_indirect_expr.args.count; i += 1) {
                zir_expr_call_add_arg(
                    call,
                    zir_lower_hir_expr(env,
                        expr->as.call_indirect_expr.args.items[i],
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            }
            return call;
        }

        case ZT_HIR_VALUE_BINDING_EXPR:
            return zir_expr_make_name(expr->as.value_binding_expr.name);

        default:
            return zir_expr_make_name("<unsupported>");
    }
}

static void zir_emit_assign_expr(
        zir_function_ctx *ctx,
        const char *dest_name,
        const char *type_name,
        zir_expr *expr,
        zir_span span) {
    zir_instruction instruction = zir_make_assign_instruction_expr(
        zir_lower_strdup(dest_name),
        zir_lower_strdup(type_name),
        expr);
    instruction.span = span;
    zir_emit_instruction(ctx, instruction);
}

static void zir_emit_effect_expr(zir_function_ctx *ctx, zir_expr *expr, zir_span span) {
    zir_instruction instruction = zir_make_effect_instruction_expr(expr);
    instruction.span = span;
    zir_emit_instruction(ctx, instruction);
}

static void zir_emit_check_contract(
        zir_function_ctx *ctx,
        const char *subject_name,
        const char *subject_type_name,
        zir_expr *predicate_eval_expr,
        const char *message_predicate_text,
        zir_span span) {
    zir_instruction instruction = zir_make_check_contract_instruction_expr(predicate_eval_expr);
    if (instruction.expr_text != NULL) free((void *)instruction.expr_text);
    instruction.expr_text = zir_lower_strdup(message_predicate_text != NULL ? message_predicate_text : "");
    instruction.dest_name = zir_lower_strdup(subject_name);
    instruction.type_name = zir_lower_strdup(subject_type_name);
    instruction.span = span;
    zir_emit_instruction(ctx, instruction);
}

static void zir_loop_push(zir_function_ctx *ctx, const char *continue_label, const char *break_label) {
    if (ctx == NULL || ctx->loop_depth >= 32) return;
    ctx->loop_stack[ctx->loop_depth].continue_label = continue_label;
    ctx->loop_stack[ctx->loop_depth].break_label = break_label;
    ctx->loop_depth += 1;
}

static void zir_loop_pop(zir_function_ctx *ctx) {
    if (ctx == NULL || ctx->loop_depth == 0) return;
    ctx->loop_depth -= 1;
}

static const zir_loop_target *zir_loop_current(const zir_function_ctx *ctx) {
    if (ctx == NULL || ctx->loop_depth == 0) return NULL;
    return &ctx->loop_stack[ctx->loop_depth - 1];
}

static int zir_is_try_expr(const zt_hir_expr *expr) {
    return expr != NULL &&
           expr->kind == ZT_HIR_UNARY_EXPR &&
           expr->as.unary_expr.op == ZT_TOKEN_QUESTION &&
           expr->as.unary_expr.operand != NULL;
}

static void zir_lower_stmt(zir_function_ctx *ctx, const zt_hir_stmt *stmt, const char *fn_return_type);

static int zir_try_lower_constructor_contract_assignment(
        zir_function_ctx *ctx,
        const char *target_name,
        const char *target_type_name,
        const zt_hir_expr *init_value,
        zir_span span) {
    const zt_hir_decl *struct_decl;
    zir_expr *construct_expr;
    size_t i;

    if (ctx == NULL || init_value == NULL || init_value->kind != ZT_HIR_CONSTRUCT_EXPR) return 0;

    struct_decl = zir_find_struct_decl_hir(ctx->env, init_value->as.construct_expr.type_name);
    if (struct_decl == NULL) return 0;

    construct_expr = zir_expr_make_make_struct(init_value->as.construct_expr.type_name);
    if (construct_expr == NULL) return 0;

    for (i = 0; i < init_value->as.construct_expr.fields.count; i += 1) {
        const zt_hir_field_init *field_init = &init_value->as.construct_expr.fields.items[i];
        const zt_hir_field_decl *field_decl = zir_find_struct_field_hir(struct_decl, field_init->name);

        if (field_decl != NULL && field_decl->where_clause != NULL) {
            char *temp_name = zir_lower_format("__zt_ctor_field_", ctx->generated_value_counter++);
            char *temp_type = zir_type_name_owned(field_decl->type);
            zir_expr *value_expr = zir_lower_hir_expr(ctx->env, field_init->value, NULL, NULL, NULL);
            zir_expr *message_expr;
            zir_expr *eval_expr;
            char *message_text;

            zir_emit_assign_expr(ctx, temp_name, temp_type, value_expr, zir_span_from_source(field_init->span));

            message_expr = zir_lower_hir_expr(ctx->env,
                field_decl->where_clause,
                NULL,
                NULL,
                field_decl->name);
            eval_expr = zir_lower_hir_expr(ctx->env,
                field_decl->where_clause,
                field_decl->name,
                temp_name,
                temp_name);
            message_text = zir_expr_render_alloc(message_expr);
            zir_emit_check_contract(
                ctx,
                temp_name,
                temp_type,
                eval_expr,
                message_text,
                zir_span_from_source(field_init->span));
            zir_expr_dispose(message_expr);
            free(message_text);

            zir_expr_make_struct_add_field(construct_expr, field_init->name, zir_expr_make_name(temp_name));
            free(temp_name);
            free(temp_type);
            continue;
        }

        zir_expr_make_struct_add_field(
            construct_expr,
            field_init->name,
            zir_lower_hir_expr(ctx->env, field_init->value, NULL, NULL, NULL));
    }

    zir_emit_assign_expr(ctx, target_name, target_type_name, construct_expr, span);
    return 1;
}

static void zir_lower_try_assignment(
        zir_function_ctx *ctx,
        const char *target_name,
        const char *target_type_name,
        const zt_hir_expr *try_expr,
        zir_span span,
        const char *fn_return_type) {
    char *temp_name;
    char *temp_type;
    char *success_label;
    char *failure_label;
    char *after_label;
    zir_expr *operand_expr;
    zir_expr *cond_expr;
    size_t success_block;
    size_t failure_block;
    size_t after_block;
    const zt_type *operand_type;
    int is_optional_try;

    temp_name = zir_lower_format("__zt_try_result_", ctx->try_temp_counter++);
    temp_type = zir_type_name_owned(try_expr->as.unary_expr.operand->type);
    operand_expr = zir_lower_hir_expr(ctx->env, try_expr->as.unary_expr.operand, NULL, NULL, NULL);
    zir_emit_assign_expr(ctx, temp_name, temp_type, operand_expr, span);

    success_label = zir_lower_format("try_success_", ctx->try_label_counter++);
    failure_label = zir_lower_format("try_failure_", ctx->try_label_counter++);
    after_label = zir_lower_format("try_after_", ctx->try_label_counter++);

    operand_type = try_expr->as.unary_expr.operand->type;
    is_optional_try =
        operand_type != NULL &&
        operand_type->kind == ZT_TYPE_OPTIONAL &&
        operand_type->args.count == 1;

    if (is_optional_try) {
        cond_expr = zir_expr_make_optional_is_present(zir_expr_make_name(temp_name));
    } else {
        cond_expr = zir_expr_make_outcome_is_success(zir_expr_make_name(temp_name));
    }
    zir_set_terminator(
        ctx,
        zir_make_branch_if_terminator_expr(cond_expr, zir_lower_strdup(success_label), zir_lower_strdup(failure_label)));

    failure_block = zir_add_block(ctx, failure_label, span);
    ctx->current_block = failure_block;
    if (is_optional_try) {
        char *optional_inner = zir_optional_inner_type_owned(fn_return_type);
        if (optional_inner == NULL) optional_inner = zir_lower_strdup("int");
        zir_set_terminator(
            ctx,
            zir_make_return_terminator_expr(zir_expr_make_optional_empty(optional_inner)));
        free(optional_inner);
    } else {
        zir_set_terminator(
            ctx,
            zir_make_return_terminator_expr(
                zir_expr_make_try_propagate(zir_expr_make_name(temp_name))));
    }

    success_block = zir_add_block(ctx, success_label, span);
    ctx->current_block = success_block;
    zir_emit_assign_expr(
        ctx,
        target_name,
        target_type_name,
        is_optional_try
            ? zir_expr_make_optional_value(zir_expr_make_name(temp_name))
            : zir_expr_make_outcome_value(zir_expr_make_name(temp_name)),
        span);
    zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(after_label)));

    after_block = zir_add_block(ctx, after_label, span);
    ctx->current_block = after_block;

    free(temp_name);
    free(temp_type);
    free(success_label);
    free(failure_label);
    free(after_label);
}

static void zir_lower_try_effect(
        zir_function_ctx *ctx,
        const zt_hir_expr *try_expr,
        zir_span span,
        const char *fn_return_type) {
    char *temp_name;
    char *temp_type;
    char *success_label;
    char *failure_label;
    char *after_label;
    zir_expr *operand_expr;
    zir_expr *cond_expr;
    size_t success_block;
    size_t failure_block;
    size_t after_block;
    const zt_type *operand_type;
    int is_optional_try;

    if (ctx == NULL || try_expr == NULL || !zir_is_try_expr(try_expr)) return;

    temp_name = zir_lower_format("__zt_try_result_", ctx->try_temp_counter++);
    temp_type = zir_type_name_owned(try_expr->as.unary_expr.operand->type);
    operand_expr = zir_lower_hir_expr(ctx->env, try_expr->as.unary_expr.operand, NULL, NULL, NULL);
    zir_emit_assign_expr(ctx, temp_name, temp_type, operand_expr, span);

    success_label = zir_lower_format("try_success_", ctx->try_label_counter++);
    failure_label = zir_lower_format("try_failure_", ctx->try_label_counter++);
    after_label = zir_lower_format("try_after_", ctx->try_label_counter++);

    operand_type = try_expr->as.unary_expr.operand->type;
    is_optional_try =
        operand_type != NULL &&
        operand_type->kind == ZT_TYPE_OPTIONAL &&
        operand_type->args.count == 1;

    if (is_optional_try) {
        cond_expr = zir_expr_make_optional_is_present(zir_expr_make_name(temp_name));
    } else {
        cond_expr = zir_expr_make_outcome_is_success(zir_expr_make_name(temp_name));
    }
    zir_set_terminator(
        ctx,
        zir_make_branch_if_terminator_expr(cond_expr, zir_lower_strdup(success_label), zir_lower_strdup(failure_label)));

    failure_block = zir_add_block(ctx, failure_label, span);
    ctx->current_block = failure_block;
    if (is_optional_try) {
        char *optional_inner = zir_optional_inner_type_owned(fn_return_type);
        if (optional_inner == NULL) optional_inner = zir_lower_strdup("int");
        zir_set_terminator(
            ctx,
            zir_make_return_terminator_expr(zir_expr_make_optional_empty(optional_inner)));
        free(optional_inner);
    } else {
        zir_set_terminator(
            ctx,
            zir_make_return_terminator_expr(
                zir_expr_make_try_propagate(zir_expr_make_name(temp_name))));
    }

    success_block = zir_add_block(ctx, success_label, span);
    ctx->current_block = success_block;
    zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(after_label)));

    after_block = zir_add_block(ctx, after_label, span);
    ctx->current_block = after_block;

    free(temp_name);
    free(temp_type);
    free(success_label);
    free(failure_label);
    free(after_label);
}

static void zir_lower_var_like_statement(
        zir_function_ctx *ctx,
        const char *name,
        const zt_type *type,
        const zt_hir_expr *init_value,
        zt_source_span span_source,
        const char *fn_return_type) {
    char *type_name = zir_type_name_owned(type);
    zir_span span = zir_span_from_source(span_source);

    if (zir_is_try_expr(init_value)) {
        zir_lower_try_assignment(ctx, name, type_name, init_value, span, fn_return_type);
        free(type_name);
        return;
    }

    if (zir_try_lower_constructor_contract_assignment(ctx, name, type_name, init_value, span)) {
        free(type_name);
        return;
    }

    zir_emit_assign_expr(
        ctx,
        name,
        type_name,
        zir_lower_hir_expr(ctx->env, init_value, NULL, NULL, NULL),
        span);
    free(type_name);
}

static const zt_hir_field_init *zir_find_construct_field(
        const zt_hir_expr *pattern,
        const char *field_name) {
    size_t i;
    if (pattern == NULL || pattern->kind != ZT_HIR_CONSTRUCT_EXPR || field_name == NULL) return NULL;
    for (i = 0; i < pattern->as.construct_expr.fields.count; i += 1) {
        const zt_hir_field_init *field = &pattern->as.construct_expr.fields.items[i];
        if (zir_text_eq(field->name, field_name)) return field;
    }
    return NULL;
}

static int zir_pattern_is_enum_construct(const zt_hir_expr *pattern) {
    return zir_find_construct_field(pattern, "__zt_enum_tag") != NULL;
}

static zir_expr *zir_match_single_pattern_condition(
        zir_function_ctx *ctx,
        const char *subject_name,
        const zt_hir_expr *pattern) {
    zir_expr *condition;

    if (ctx == NULL || subject_name == NULL || pattern == NULL) return zir_expr_make_bool(0);

    if (pattern->kind == ZT_HIR_VALUE_BINDING_EXPR) {
        return zir_expr_make_optional_is_present(zir_expr_make_name(subject_name));
    }

    if (pattern->kind == ZT_HIR_SUCCESS_EXPR) {
        condition = zir_expr_make_outcome_is_success(zir_expr_make_name(subject_name));
        if (pattern->as.success_expr.value == NULL ||
                pattern->as.success_expr.value->kind == ZT_HIR_IDENT_EXPR ||
                pattern->as.success_expr.value->kind == ZT_HIR_NONE_EXPR) {
            return condition;
        }
        return zir_expr_make_binary(
            "and",
            condition,
            zir_expr_make_binary(
                "eq",
                zir_expr_make_outcome_value(zir_expr_make_name(subject_name)),
                zir_lower_hir_expr(ctx->env, pattern->as.success_expr.value, NULL, NULL, NULL)));
    }

    if (pattern->kind == ZT_HIR_ERROR_EXPR) {
        condition = zir_expr_make_binary(
            "eq",
            zir_expr_make_outcome_is_success(zir_expr_make_name(subject_name)),
            zir_expr_make_bool(0));
        if (pattern->as.error_expr.value == NULL ||
                pattern->as.error_expr.value->kind == ZT_HIR_IDENT_EXPR) {
            return condition;
        }
        return zir_expr_make_binary(
            "and",
            condition,
            zir_expr_make_binary(
                "eq",
                zir_expr_make_get_field(zir_expr_make_name(subject_name), "error"),
                zir_lower_hir_expr(ctx->env, pattern->as.error_expr.value, NULL, NULL, NULL)));
    }

    if (pattern->kind == ZT_HIR_NONE_EXPR) {
        return zir_expr_make_binary("eq", zir_expr_make_optional_is_present(zir_expr_make_name(subject_name)), zir_expr_make_bool(0));
    }

    if (pattern->kind == ZT_HIR_FIELD_EXPR &&
            pattern->as.field_expr.object != NULL &&
            pattern->as.field_expr.object->kind == ZT_HIR_IDENT_EXPR &&
            pattern->as.field_expr.field_name != NULL) {
        size_t variant_index = 0;
        if (zir_find_enum_variant_index_hir(ctx->env,
                pattern->as.field_expr.object->as.ident_expr.name,
                pattern->as.field_expr.field_name,
                &variant_index)) {
            char tag_buffer[32];
            snprintf(tag_buffer, sizeof(tag_buffer), "%zu", variant_index);
            return zir_expr_make_binary(
                "eq",
                zir_expr_make_get_field(zir_expr_make_name(subject_name), "__zt_enum_tag"),
                zir_expr_make_int(tag_buffer));
        }
    }

    if (!zir_pattern_is_enum_construct(pattern)) {
        return zir_expr_make_binary(
            "eq",
            zir_expr_make_name(subject_name),
            zir_lower_hir_expr(ctx->env, pattern, NULL, NULL, NULL));
    }

    {
        const zt_hir_field_init *tag_field = zir_find_construct_field(pattern, "__zt_enum_tag");
        size_t i;
        condition = zir_expr_make_binary(
            "eq",
            zir_expr_make_get_field(zir_expr_make_name(subject_name), "__zt_enum_tag"),
            zir_lower_hir_expr(ctx->env, tag_field != NULL ? tag_field->value : NULL, NULL, NULL, NULL));

        for (i = 0; i < pattern->as.construct_expr.fields.count; i += 1) {
            const zt_hir_field_init *field = &pattern->as.construct_expr.fields.items[i];
            zir_expr *field_cond;
            if (zir_text_eq(field->name, "__zt_enum_tag")) continue;
            if (field->value != NULL && field->value->kind == ZT_HIR_IDENT_EXPR) {
                continue;
            }
            field_cond = zir_expr_make_binary(
                "eq",
                zir_expr_make_get_field(zir_expr_make_name(subject_name), field->name),
                zir_lower_hir_expr(ctx->env, field->value, NULL, NULL, NULL));
            condition = zir_expr_make_binary("and", condition, field_cond);
        }
    }

    return condition;
}

static zir_expr *zir_match_case_condition(
        zir_function_ctx *ctx,
        const char *subject_name,
        const zt_hir_expr_list *patterns) {
    zir_expr *condition = NULL;
    size_t i;

    if (ctx == NULL || subject_name == NULL || patterns == NULL || patterns->count == 0) return NULL;

    for (i = 0; i < patterns->count; i += 1) {
        zir_expr *single = zir_match_single_pattern_condition(ctx, subject_name, patterns->items[i]);
        if (condition == NULL) {
            condition = single;
        } else {
            condition = zir_expr_make_binary("or", condition, single);
        }
    }

    return condition;
}

static void zir_emit_match_case_bindings(
        zir_function_ctx *ctx,
        const char *subject_name,
        const zt_hir_match_case *match_case,
        zir_span span) {
    const zt_hir_expr *pattern;
    size_t i;

    if (ctx == NULL || subject_name == NULL || match_case == NULL || match_case->patterns.count == 0) return;
    pattern = match_case->patterns.items[0];

    if (pattern->kind == ZT_HIR_VALUE_BINDING_EXPR) {
        char *type_name;
        zir_expr *value_expr;
        type_name = pattern->type != NULL ? zir_type_name_owned(pattern->type) : zir_lower_strdup("int");
        value_expr = zir_expr_make_optional_value(zir_expr_make_name(subject_name));
        zir_emit_assign_expr(ctx, pattern->as.value_binding_expr.name, type_name, value_expr, span);
        free(type_name);
        return;
    }

    if (pattern->kind == ZT_HIR_SUCCESS_EXPR && pattern->as.success_expr.value != NULL) {
        const zt_hir_expr *value_pattern = pattern->as.success_expr.value;
        if (value_pattern->kind == ZT_HIR_IDENT_EXPR &&
                !zir_text_eq(value_pattern->as.ident_expr.name, "_")) {
            char *type_name = zir_result_branch_type_owned(pattern->type, 0);
            zir_emit_assign_expr(
                ctx,
                value_pattern->as.ident_expr.name,
                type_name,
                zir_expr_make_outcome_value(zir_expr_make_name(subject_name)),
                span);
            free(type_name);
        }
        return;
    }

    if (pattern->kind == ZT_HIR_ERROR_EXPR && pattern->as.error_expr.value != NULL) {
        const zt_hir_expr *error_pattern = pattern->as.error_expr.value;
        if (error_pattern->kind == ZT_HIR_IDENT_EXPR &&
                !zir_text_eq(error_pattern->as.ident_expr.name, "_")) {
            char *type_name = zir_result_branch_type_owned(pattern->type, 1);
            zir_emit_assign_expr(
                ctx,
                error_pattern->as.ident_expr.name,
                type_name,
                zir_expr_make_get_field(zir_expr_make_name(subject_name), "error"),
                span);
            free(type_name);
        }
        return;
    }

    if (!zir_pattern_is_enum_construct(pattern)) return;

    for (i = 0; i < pattern->as.construct_expr.fields.count; i += 1) {
        const zt_hir_field_init *field = &pattern->as.construct_expr.fields.items[i];
        const zt_hir_expr *payload_expr = field->value;
        char *type_name;
        if (zir_text_eq(field->name, "__zt_enum_tag")) continue;
        if (payload_expr == NULL || payload_expr->kind != ZT_HIR_IDENT_EXPR) continue;
        if (zir_text_eq(payload_expr->as.ident_expr.name, "_")) continue;

        type_name = zir_type_name_owned(payload_expr->type);
        zir_emit_assign_expr(
            ctx,
            payload_expr->as.ident_expr.name,
            type_name,
            zir_expr_make_get_field(zir_expr_make_name(subject_name), field->name),
            span);
        free(type_name);
    }
}

static void zir_lower_match_statement(zir_function_ctx *ctx, const zt_hir_stmt *stmt, const char *fn_return_type) {
    char *subject_temp;
    char *after_label;
    char *next_case_label;
    size_t after_block;
    size_t i;
    zir_span span;

    if (ctx == NULL || stmt == NULL) return;
    span = zir_span_from_source(stmt->span);
    subject_temp = zir_lower_format("__zt_match_subject_", ctx->match_subject_counter++);
    {
        char *subject_type_name = zir_type_name_owned(
            stmt->as.match_stmt.subject != NULL ? stmt->as.match_stmt.subject->type : NULL);
        zir_emit_assign_expr(
            ctx,
            subject_temp,
            subject_type_name,
            zir_lower_hir_expr(ctx->env, stmt->as.match_stmt.subject, NULL, NULL, NULL),
            span);
        free(subject_type_name);
    }

    after_label = zir_lower_format("match_after_", ctx->match_label_counter++);
    next_case_label = zir_lower_format("match_case_", ctx->match_label_counter++);
    zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(next_case_label)));

    for (i = 0; i < stmt->as.match_stmt.cases.count; i += 1) {
        const zt_hir_match_case *match_case = &stmt->as.match_stmt.cases.items[i];
        size_t case_block;
        size_t body_block;
        char *body_label = NULL;
        char *fallback_label = NULL;

        case_block = zir_add_block(ctx, next_case_label, zir_span_from_source(match_case->span));
        ctx->current_block = case_block;
        free(next_case_label);
        next_case_label = NULL;

        if (match_case->is_default) {
            zir_lower_stmt(ctx, match_case->body, fn_return_type);
            if (zir_current_block(ctx) != NULL && !zir_current_block(ctx)->has_terminator) {
                zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(after_label)));
            }
            continue;
        }

        body_label = zir_lower_format("match_body_", ctx->match_label_counter++);
        if (i + 1 < stmt->as.match_stmt.cases.count) {
            fallback_label = zir_lower_format("match_case_", ctx->match_label_counter++);
        } else {
            fallback_label = zir_lower_strdup(after_label);
        }

        zir_set_terminator(
            ctx,
            zir_make_branch_if_terminator_expr(
                zir_match_case_condition(ctx, subject_temp, &match_case->patterns),
                zir_lower_strdup(body_label),
                zir_lower_strdup(fallback_label)));

        body_block = zir_add_block(ctx, body_label, zir_span_from_source(match_case->span));
        ctx->current_block = body_block;
        zir_emit_match_case_bindings(ctx, subject_temp, match_case, zir_span_from_source(match_case->span));
        zir_lower_stmt(ctx, match_case->body, fn_return_type);
        if (zir_current_block(ctx) != NULL && !zir_current_block(ctx)->has_terminator) {
            zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(after_label)));
        }

        next_case_label = fallback_label;
        free(body_label);
    }

    if (next_case_label != NULL) free(next_case_label);
    after_block = zir_add_block(ctx, after_label, span);
    ctx->current_block = after_block;
    free(subject_temp);
    free(after_label);
}

static void zir_lower_stmt(zir_function_ctx *ctx, const zt_hir_stmt *stmt, const char *fn_return_type) {
    zir_span span;
    if (ctx == NULL || stmt == NULL) return;
    if (zir_current_block(ctx) != NULL && zir_current_block(ctx)->has_terminator) return;
    span = zir_span_from_source(stmt->span);

    switch (stmt->kind) {
        case ZT_HIR_BLOCK_STMT: {
            size_t i;
            for (i = 0; i < stmt->as.block_stmt.statements.count; i += 1) {
                zir_lower_stmt(ctx, stmt->as.block_stmt.statements.items[i], fn_return_type);
                if (zir_current_block(ctx) != NULL && zir_current_block(ctx)->has_terminator) break;
            }
            return;
        }

        case ZT_HIR_IF_STMT: {
            char *then_label = zir_lower_format("if_then_", ctx->if_label_counter++);
            char *else_label = zir_lower_format("if_else_", ctx->if_label_counter++);
            char *join_label = zir_lower_format("if_join_", ctx->if_label_counter++);
            size_t then_block;
            size_t else_block;
            size_t join_block;

            zir_set_terminator(
                ctx,
                zir_make_branch_if_terminator_expr(
                    zir_lower_hir_expr(ctx->env, stmt->as.if_stmt.condition, NULL, NULL, NULL),
                    zir_lower_strdup(then_label),
                    zir_lower_strdup(else_label)));

            then_block = zir_add_block(ctx, then_label, span);
            ctx->current_block = then_block;
            zir_lower_stmt(ctx, stmt->as.if_stmt.then_block, fn_return_type);
            if (zir_current_block(ctx) != NULL && !zir_current_block(ctx)->has_terminator) {
                zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(join_label)));
            }

            else_block = zir_add_block(ctx, else_label, span);
            ctx->current_block = else_block;
            zir_lower_stmt(ctx, stmt->as.if_stmt.else_block, fn_return_type);
            if (zir_current_block(ctx) != NULL && !zir_current_block(ctx)->has_terminator) {
                zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(join_label)));
            }

            join_block = zir_add_block(ctx, join_label, span);
            ctx->current_block = join_block;

            free(then_label);
            free(else_label);
            free(join_label);
            return;
        }

        case ZT_HIR_WHILE_STMT: {
            char *cond_label = zir_lower_format("while_cond_", ctx->while_label_counter++);
            char *body_label = zir_lower_format("while_body_", ctx->while_label_counter++);
            char *exit_label = zir_lower_format("while_exit_", ctx->while_label_counter++);
            size_t cond_block;
            size_t body_block;
            size_t exit_block;

            zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(cond_label)));

            cond_block = zir_add_block(ctx, cond_label, span);
            ctx->current_block = cond_block;
            zir_set_terminator(
                ctx,
                zir_make_branch_if_terminator_expr(
                    zir_lower_hir_expr(ctx->env, stmt->as.while_stmt.condition, NULL, NULL, NULL),
                    zir_lower_strdup(body_label),
                    zir_lower_strdup(exit_label)));

            body_block = zir_add_block(ctx, body_label, span);
            ctx->current_block = body_block;
            zir_loop_push(ctx, cond_label, exit_label);
            zir_lower_stmt(ctx, stmt->as.while_stmt.body, fn_return_type);
            if (zir_current_block(ctx) != NULL && !zir_current_block(ctx)->has_terminator) {
                zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(cond_label)));
            }
            zir_loop_pop(ctx);

            exit_block = zir_add_block(ctx, exit_label, span);
            ctx->current_block = exit_block;

            free(cond_label);
            free(body_label);
            free(exit_label);
            return;
        }

        case ZT_HIR_FOR_STMT: {
            const zt_type *iter_type = NULL;
            zt_type_kind iter_kind = ZT_TYPE_UNKNOWN;
            char *iter_temp;
            char *index_temp;
            char *len_temp;
            char *iter_type_name;
            char *item_type_name;
            char *second_type_name;
            char *cond_label;
            char *body_label;
            char *step_label;
            char *exit_label;
            size_t cond_block;
            size_t body_block;
            size_t step_block;
            size_t exit_block;
            zir_expr *len_expr;
            zir_expr *item_expr;
            zir_expr *second_expr;

            iter_type = stmt->as.for_stmt.iterable != NULL ? stmt->as.for_stmt.iterable->type : NULL;
            if (iter_type != NULL) {
                iter_kind = iter_type->kind;
            }

            iter_temp = zir_lower_format("__zt_for_iter_", ctx->for_temp_counter++);
            index_temp = zir_lower_format("__zt_for_index_", ctx->for_temp_counter++);
            len_temp = zir_lower_format("__zt_for_len_", ctx->for_temp_counter++);
            cond_label = zir_lower_format("for_cond_", ctx->for_label_counter++);
            body_label = zir_lower_format("for_body_", ctx->for_label_counter++);
            step_label = zir_lower_format("for_step_", ctx->for_label_counter++);
            exit_label = zir_lower_format("for_exit_", ctx->for_label_counter++);

            iter_type_name = zir_type_name_owned(iter_type);
            item_type_name = zir_lower_strdup("unknown");
            second_type_name = zir_lower_strdup("int");

            if (iter_type != NULL && iter_kind == ZT_TYPE_LIST && iter_type->args.count > 0) {
                free(item_type_name);
                item_type_name = zir_type_name_owned(iter_type->args.items[0]);
            } else if (iter_type != NULL && iter_kind == ZT_TYPE_MAP && iter_type->args.count >= 2) {
                free(item_type_name);
                free(second_type_name);
                item_type_name = zir_type_name_owned(iter_type->args.items[0]);
                second_type_name = zir_type_name_owned(iter_type->args.items[1]);
            } else if (iter_type != NULL && iter_kind == ZT_TYPE_TEXT) {
                free(item_type_name);
                item_type_name = zir_lower_strdup("text");
            } else if (iter_type != NULL) {
                zir_add_lower_diag(
                    ctx->env->diagnostics,
                    stmt->span,
                    "for-in currently supports list, map<text,text> and text");
            }

            zir_emit_assign_expr(
                ctx,
                iter_temp,
                iter_type_name,
                zir_lower_hir_expr(ctx->env, stmt->as.for_stmt.iterable, NULL, NULL, NULL),
                span);
            zir_emit_assign_expr(ctx, index_temp, "int", zir_expr_make_int("0"), span);

            if (iter_kind == ZT_TYPE_MAP) {
                len_expr = zir_expr_make_map_len(zir_expr_make_name(iter_temp));
            } else if (iter_kind == ZT_TYPE_TEXT) {
                len_expr = zir_expr_make_call_extern("c.zt_text_len");
                zir_expr_call_add_arg(len_expr, zir_expr_make_name(iter_temp));
            } else if (iter_kind == ZT_TYPE_BYTES) {
                len_expr = zir_expr_make_call_extern("c.zt_bytes_len");
                zir_expr_call_add_arg(len_expr, zir_expr_make_name(iter_temp));
            } else if (iter_kind == ZT_TYPE_LIST) {
                len_expr = zir_expr_make_list_len(zir_expr_make_name(iter_temp));
            } else {
                len_expr = zir_expr_make_int("0");
            }
            zir_emit_assign_expr(ctx, len_temp, "int", len_expr, span);
            zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(cond_label)));

            cond_block = zir_add_block(ctx, cond_label, span);
            ctx->current_block = cond_block;
            zir_set_terminator(
                ctx,
                zir_make_branch_if_terminator_expr(
                    zir_expr_make_binary(
                        "lt",
                        zir_expr_make_name(index_temp),
                        zir_expr_make_name(len_temp)),
                    zir_lower_strdup(body_label),
                    zir_lower_strdup(exit_label)));

            body_block = zir_add_block(ctx, body_label, span);
            ctx->current_block = body_block;

            if (iter_kind == ZT_TYPE_MAP) {
                item_expr = zir_expr_make_call_extern("c.zt_map_text_text_key_at");
                zir_expr_call_add_arg(item_expr, zir_expr_make_name(iter_temp));
                zir_expr_call_add_arg(item_expr, zir_expr_make_name(index_temp));
            } else if (iter_kind == ZT_TYPE_TEXT) {
                item_expr = zir_expr_make_call_extern("c.zt_text_index");
                zir_expr_call_add_arg(item_expr, zir_expr_make_name(iter_temp));
                zir_expr_call_add_arg(item_expr, zir_expr_make_name(index_temp));
            } else {
                item_expr = zir_expr_make_index_seq(
                    zir_expr_make_name(iter_temp),
                    zir_expr_make_name(index_temp));
            }
            zir_emit_assign_expr(ctx, stmt->as.for_stmt.item_name, item_type_name, item_expr, span);

            if (stmt->as.for_stmt.index_name != NULL) {
                if (iter_kind == ZT_TYPE_MAP) {
                    second_expr = zir_expr_make_call_extern("c.zt_map_text_text_value_at");
                    zir_expr_call_add_arg(second_expr, zir_expr_make_name(iter_temp));
                    zir_expr_call_add_arg(second_expr, zir_expr_make_name(index_temp));
                } else {
                    second_expr = zir_expr_make_name(index_temp);
                }
                zir_emit_assign_expr(
                    ctx,
                    stmt->as.for_stmt.index_name,
                    second_type_name,
                    second_expr,
                    span);
            }

            zir_loop_push(ctx, step_label, exit_label);
            zir_lower_stmt(ctx, stmt->as.for_stmt.body, fn_return_type);
            if (zir_current_block(ctx) != NULL && !zir_current_block(ctx)->has_terminator) {
                zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(step_label)));
            }
            zir_loop_pop(ctx);

            step_block = zir_add_block(ctx, step_label, span);
            ctx->current_block = step_block;
            zir_emit_assign_expr(
                ctx,
                index_temp,
                "int",
                zir_expr_make_binary("add", zir_expr_make_name(index_temp), zir_expr_make_int("1")),
                span);
            zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(cond_label)));

            exit_block = zir_add_block(ctx, exit_label, span);
            ctx->current_block = exit_block;

            free(iter_temp);
            free(index_temp);
            free(len_temp);
            free(iter_type_name);
            free(item_type_name);
            free(second_type_name);
            free(cond_label);
            free(body_label);
            free(step_label);
            free(exit_label);
            return;
        }

        case ZT_HIR_REPEAT_STMT: {
            char *count_temp = zir_lower_format("__zt_repeat_count_", ctx->repeat_temp_counter++);
            char *index_temp = zir_lower_format("__zt_repeat_index_", ctx->repeat_temp_counter++);
            char *cond_label = zir_lower_format("repeat_cond_", ctx->repeat_label_counter++);
            char *body_label = zir_lower_format("repeat_body_", ctx->repeat_label_counter++);
            char *step_label = zir_lower_format("repeat_step_", ctx->repeat_label_counter++);
            char *exit_label = zir_lower_format("repeat_exit_", ctx->repeat_label_counter++);
            size_t cond_block;
            size_t body_block;
            size_t step_block;
            size_t exit_block;

            zir_emit_assign_expr(
                ctx,
                count_temp,
                "int",
                zir_lower_hir_expr(ctx->env, stmt->as.repeat_stmt.count, NULL, NULL, NULL),
                span);
            zir_emit_assign_expr(ctx, index_temp, "int", zir_expr_make_int("0"), span);
            zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(cond_label)));

            cond_block = zir_add_block(ctx, cond_label, span);
            ctx->current_block = cond_block;
            zir_set_terminator(
                ctx,
                zir_make_branch_if_terminator_expr(
                    zir_expr_make_binary(
                        "lt",
                        zir_expr_make_name(index_temp),
                        zir_expr_make_name(count_temp)),
                    zir_lower_strdup(body_label),
                    zir_lower_strdup(exit_label)));

            body_block = zir_add_block(ctx, body_label, span);
            ctx->current_block = body_block;
            zir_loop_push(ctx, step_label, exit_label);
            zir_lower_stmt(ctx, stmt->as.repeat_stmt.body, fn_return_type);
            if (zir_current_block(ctx) != NULL && !zir_current_block(ctx)->has_terminator) {
                zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(step_label)));
            }
            zir_loop_pop(ctx);

            step_block = zir_add_block(ctx, step_label, span);
            ctx->current_block = step_block;
            zir_emit_assign_expr(
                ctx,
                index_temp,
                "int",
                zir_expr_make_binary("add", zir_expr_make_name(index_temp), zir_expr_make_int("1")),
                span);
            zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(cond_label)));

            exit_block = zir_add_block(ctx, exit_label, span);
            ctx->current_block = exit_block;

            free(count_temp);
            free(index_temp);
            free(cond_label);
            free(body_label);
            free(step_label);
            free(exit_label);
            return;
        }

        case ZT_HIR_RETURN_STMT:
            if (stmt->as.return_stmt.value == NULL || zir_text_eq(fn_return_type, "void")) {
                zir_set_terminator(ctx, zir_make_return_terminator(""));
            } else {
                zir_expr *return_expr = zir_lower_hir_expr(ctx->env,
                    stmt->as.return_stmt.value,
                    NULL,
                    NULL,
                    NULL);
                char *value_type_name = zir_type_name_owned(stmt->as.return_stmt.value->type);
                if (zir_type_name_is_optional(fn_return_type) &&
                        stmt->as.return_stmt.value->kind != ZT_HIR_NONE_EXPR &&
                        (value_type_name == NULL || !zir_text_eq(value_type_name, fn_return_type))) {
                    return_expr = zir_expr_make_optional_present(return_expr);
                }
                zir_set_terminator(
                    ctx,
                    zir_make_return_terminator_expr(return_expr));
                free(value_type_name);
            }
            return;

        case ZT_HIR_CONST_STMT:
            zir_lower_var_like_statement(
                ctx,
                stmt->as.const_stmt.name,
                stmt->as.const_stmt.type,
                stmt->as.const_stmt.init_value,
                stmt->span,
                fn_return_type);
            return;

        case ZT_HIR_VAR_STMT:
            zir_lower_var_like_statement(
                ctx,
                stmt->as.var_stmt.name,
                stmt->as.var_stmt.type,
                stmt->as.var_stmt.init_value,
                stmt->span,
                fn_return_type);
            return;

        case ZT_HIR_ASSIGN_STMT: {
            char *type_name = zir_type_name_owned(
                stmt->as.assign_stmt.value != NULL ? stmt->as.assign_stmt.value->type : NULL);
            if (zir_is_try_expr(stmt->as.assign_stmt.value)) {
                zir_lower_try_assignment(
                    ctx,
                    stmt->as.assign_stmt.name,
                    type_name,
                    stmt->as.assign_stmt.value,
                    span,
                    fn_return_type);
                free(type_name);
                return;
            }

            if (zir_try_lower_constructor_contract_assignment(
                    ctx,
                    stmt->as.assign_stmt.name,
                    type_name,
                    stmt->as.assign_stmt.value,
                    span)) {
                free(type_name);
                return;
            }

            zir_emit_assign_expr(
                ctx,
                stmt->as.assign_stmt.name,
                type_name,
                zir_lower_hir_expr(ctx->env, stmt->as.assign_stmt.value, NULL, NULL, NULL),
                span);
            free(type_name);
            return;
        }

        case ZT_HIR_INDEX_ASSIGN_STMT: {
            const zt_type *object_type = NULL;
            zir_expr *effect_expr;

            if (stmt->as.index_assign_stmt.object != NULL) {
                object_type = stmt->as.index_assign_stmt.object->type;
            }

            if (object_type != NULL && object_type->kind == ZT_TYPE_MAP) {
                effect_expr = zir_expr_make_map_set(
                    zir_lower_hir_expr(ctx->env, stmt->as.index_assign_stmt.object, NULL, NULL, NULL),
                    zir_lower_hir_expr(ctx->env, stmt->as.index_assign_stmt.index, NULL, NULL, NULL),
                    zir_lower_hir_expr(ctx->env, stmt->as.index_assign_stmt.value, NULL, NULL, NULL));
            } else {
                effect_expr = zir_expr_make_list_set(
                    zir_lower_hir_expr(ctx->env, stmt->as.index_assign_stmt.object, NULL, NULL, NULL),
                    zir_lower_hir_expr(ctx->env, stmt->as.index_assign_stmt.index, NULL, NULL, NULL),
                    zir_lower_hir_expr(ctx->env, stmt->as.index_assign_stmt.value, NULL, NULL, NULL));
            }

            zir_emit_effect_expr(ctx, effect_expr, span);
            return;
        }

        case ZT_HIR_FIELD_ASSIGN_STMT: {
            const zt_hir_decl *struct_decl = NULL;
            const zt_hir_field_decl *field_decl = NULL;
            const zt_type *object_type = NULL;

            if (stmt->as.field_assign_stmt.object != NULL) {
                object_type = stmt->as.field_assign_stmt.object->type;
            }
            if (object_type != NULL && object_type->kind == ZT_TYPE_USER) {
                struct_decl = zir_find_struct_decl_hir(ctx->env, object_type->name);
                field_decl = zir_find_struct_field_hir(struct_decl, stmt->as.field_assign_stmt.field_name);
            }

            if (field_decl != NULL && field_decl->where_clause != NULL) {
                char *temp_name = zir_lower_format("__zt_field_value_", ctx->generated_value_counter++);
                char *temp_type = zir_type_name_owned(field_decl->type);
                zir_expr *message_expr;
                zir_expr *eval_expr;
                char *message_text;

                zir_emit_assign_expr(
                    ctx,
                    temp_name,
                    temp_type,
                    zir_lower_hir_expr(ctx->env, stmt->as.field_assign_stmt.value, NULL, NULL, NULL),
                    span);

                zir_emit_effect_expr(
                    ctx,
                    zir_expr_make_set_field(
                        zir_lower_hir_expr(ctx->env, stmt->as.field_assign_stmt.object, NULL, NULL, NULL),
                        stmt->as.field_assign_stmt.field_name,
                        zir_expr_make_name(temp_name)),
                    span);

                message_expr = zir_lower_hir_expr(ctx->env,
                    field_decl->where_clause,
                    NULL,
                    NULL,
                    stmt->as.field_assign_stmt.field_name);
                eval_expr = zir_lower_hir_expr(ctx->env,
                    field_decl->where_clause,
                    stmt->as.field_assign_stmt.field_name,
                    temp_name,
                    temp_name);
                message_text = zir_expr_render_alloc(message_expr);
                zir_emit_check_contract(
                    ctx,
                    temp_name,
                    temp_type,
                    eval_expr,
                    message_text,
                    span);
                zir_expr_dispose(message_expr);
                free(message_text);
                free(temp_name);
                free(temp_type);
                return;
            }

            zir_emit_effect_expr(
                ctx,
                zir_expr_make_set_field(
                    zir_lower_hir_expr(ctx->env, stmt->as.field_assign_stmt.object, NULL, NULL, NULL),
                    stmt->as.field_assign_stmt.field_name,
                    zir_lower_hir_expr(ctx->env, stmt->as.field_assign_stmt.value, NULL, NULL, NULL)),
                span);
            return;
        }

        case ZT_HIR_MATCH_STMT:
            zir_lower_match_statement(ctx, stmt, fn_return_type);
            return;

        case ZT_HIR_BREAK_STMT: {
            const zir_loop_target *target = zir_loop_current(ctx);
            if (target != NULL) {
                zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(target->break_label)));
            } else {
                zir_set_terminator(ctx, zir_make_unreachable_terminator());
            }
            return;
        }

        case ZT_HIR_CONTINUE_STMT: {
            const zir_loop_target *target = zir_loop_current(ctx);
            if (target != NULL) {
                zir_set_terminator(ctx, zir_make_jump_terminator(zir_lower_strdup(target->continue_label)));
            } else {
                zir_set_terminator(ctx, zir_make_unreachable_terminator());
            }
            return;
        }

        case ZT_HIR_EXPR_STMT:
            if (zir_is_try_expr(stmt->as.expr_stmt.expr)) {
                zir_lower_try_effect(ctx, stmt->as.expr_stmt.expr, span, fn_return_type);
                return;
            }
            if (zir_call_is_intrinsic(stmt->as.expr_stmt.expr, "panic")) {
                zir_expr *message_expr = NULL;
                if (stmt->as.expr_stmt.expr->as.call_expr.args.count > 0) {
                    message_expr = zir_lower_hir_expr(ctx->env,
                        stmt->as.expr_stmt.expr->as.call_expr.args.items[0],
                        NULL,
                        NULL,
                        NULL);
                }
                if (message_expr == NULL) {
                    message_expr = zir_expr_make_string("panic");
                }
                zir_set_terminator(ctx, zir_make_panic_terminator_expr(message_expr));
                return;
            }
            zir_emit_effect_expr(
                ctx,
                zir_lower_hir_expr(ctx->env, stmt->as.expr_stmt.expr, NULL, NULL, NULL),
                span);
            return;

        default:
            zir_add_lower_diag(ctx->env->diagnostics, stmt->span, "unsupported statement during HIR->ZIR lowering");
            return;
    }
}

static void zir_dispose_block_states(zir_function_ctx *ctx) {
    size_t i;
    if (ctx == NULL) return;
    for (i = 0; i < ctx->blocks.count; i += 1) {
        zir_block_state *state = &ctx->blocks.items[i];
        size_t j;
        free(state->label);
        for (j = 0; j < state->instructions.count; j += 1) {
            zir_instruction_dispose_owned(&state->instructions.items[j]);
        }
        free(state->instructions.items);
        if (state->has_terminator) {
            zir_terminator_dispose_owned(&state->terminator);
        }
        memset(state, 0, sizeof(*state));
    }
    free(ctx->blocks.items);
    memset(&ctx->blocks, 0, sizeof(ctx->blocks));
}

static zir_block *zir_materialize_blocks(zir_function_ctx *ctx, const char *fn_return_type, size_t *out_count) {
    zir_block *blocks;
    size_t i;
    size_t count = 0;

    if (out_count != NULL) *out_count = 0;
    if (ctx == NULL) return NULL;

    count = ctx->blocks.count;
    if (count == 0) return NULL;
    blocks = (zir_block *)calloc(count, sizeof(zir_block));
    if (blocks == NULL) return NULL;

    for (i = 0; i < count; i += 1) {
        zir_block_state *state = &ctx->blocks.items[i];
        zir_terminator terminator;

        if (!state->has_terminator) {
            if (zir_text_eq(fn_return_type, "void")) {
                state->terminator = zir_make_return_terminator("");
            } else {
                state->terminator = zir_make_unreachable_terminator();
            }
            state->has_terminator = 1;
        }

        terminator = state->terminator;
        blocks[i] = zir_make_block(
            state->label,
            state->instructions.items,
            state->instructions.count,
            terminator);
        blocks[i].span = state->span;

        state->label = NULL;
        state->instructions.items = NULL;
        state->instructions.count = 0;
        state->instructions.capacity = 0;
        state->has_terminator = 0;
        memset(&state->terminator, 0, sizeof(state->terminator));
    }

    free(ctx->blocks.items);
    ctx->blocks.items = NULL;
    ctx->blocks.count = 0;
    ctx->blocks.capacity = 0;
    if (out_count != NULL) *out_count = count;
    return blocks;
}

static zir_struct_decl zir_lower_struct_decl(zir_lower_env *env, const zt_hir_decl *decl) {
    zir_struct_decl out;
    zir_field_decl *fields = NULL;
    size_t i;

    memset(&out, 0, sizeof(out));
    if (decl == NULL || decl->kind != ZT_HIR_STRUCT_DECL) return out;

    if (decl->as.struct_decl.fields.count > 0) {
        fields = (zir_field_decl *)calloc(decl->as.struct_decl.fields.count, sizeof(zir_field_decl));
    }

    for (i = 0; i < decl->as.struct_decl.fields.count; i += 1) {
        const zt_hir_field_decl *hir_field = &decl->as.struct_decl.fields.items[i];
        zir_expr *where_expr = NULL;
        if (hir_field->where_clause != NULL) {
            where_expr = zir_lower_hir_expr(env, hir_field->where_clause, NULL, NULL, hir_field->name);
        }
        fields[i] = zir_make_field_decl(
            zir_lower_strdup(hir_field->name),
            zir_type_name_owned(hir_field->type),
            where_expr);
        fields[i].span = zir_span_from_source(
            hir_field->where_clause != NULL ? hir_field->where_clause->span : decl->span);
    }

    out = zir_make_struct_decl(
        zir_lower_strdup(decl->as.struct_decl.name),
        fields,
        decl->as.struct_decl.fields.count);
    out.span = zir_span_from_source(decl->span);
    return out;
}

static zir_enum_decl zir_lower_enum_decl(const zt_hir_decl *decl) {
    zir_enum_decl out;
    zir_enum_variant_decl *variants = NULL;
    size_t i;

    memset(&out, 0, sizeof(out));
    if (decl == NULL || decl->kind != ZT_HIR_ENUM_DECL) return out;

    if (decl->as.enum_decl.variants.count > 0) {
        variants = (zir_enum_variant_decl *)calloc(
            decl->as.enum_decl.variants.count,
            sizeof(zir_enum_variant_decl));
    }

    for (i = 0; i < decl->as.enum_decl.variants.count; i += 1) {
        const zt_hir_enum_variant *hir_variant = &decl->as.enum_decl.variants.items[i];
        zir_enum_variant_field_decl *fields = NULL;
        size_t f;

        if (hir_variant->fields.count > 0) {
            fields = (zir_enum_variant_field_decl *)calloc(
                hir_variant->fields.count,
                sizeof(zir_enum_variant_field_decl));
        }

        for (f = 0; f < hir_variant->fields.count; f += 1) {
            const zt_hir_enum_variant_field *hir_field = &hir_variant->fields.items[f];
            fields[f] = zir_make_enum_variant_field_decl(
                zir_lower_strdup(hir_field->name != NULL ? hir_field->name : "<field>"),
                zir_type_name_owned(hir_field->type));
            fields[f].span = zir_span_from_source(decl->span);
        }

        variants[i] = zir_make_enum_variant_decl(
            zir_lower_strdup(hir_variant->name),
            fields,
            hir_variant->fields.count);
        variants[i].span = zir_span_from_source(decl->span);
    }

    out = zir_make_enum_decl(
        zir_lower_strdup(decl->as.enum_decl.name),
        variants,
        decl->as.enum_decl.variants.count);
    out.span = zir_span_from_source(decl->span);
    return out;
}


static zir_function zir_lower_closure_as_function(
        zir_lower_env *env,
        const char *name,
        const zt_hir_expr *closure) {
    zir_function out;
    zir_function_ctx ctx;
    zir_param *params = NULL;
    size_t i;
    char *return_type = NULL;
    zir_block *blocks = NULL;
    size_t block_count = 0;

    memset(&out, 0, sizeof(out));
    memset(&ctx, 0, sizeof(ctx));
    ctx.env = env;
    zir_block_state_buffer_init(&ctx.blocks);
    ctx.current_block = zir_add_block(&ctx, "entry", zir_span_from_source(closure->span));

    if (closure->as.closure_expr.params.count > 0) {
        params = (zir_param *)calloc(closure->as.closure_expr.params.count, sizeof(zir_param));
        for (i = 0; i < closure->as.closure_expr.params.count; i++) {
            const zt_hir_param *hp = &closure->as.closure_expr.params.items[i];
            params[i] = zir_make_param(
                zir_lower_strdup(hp->name),
                zir_type_name_owned(hp->type),
                NULL);
            params[i].span = zir_span_from_source(closure->span);
        }
    }

    return_type = zir_type_name_owned(closure->as.closure_expr.return_type);
    zir_lower_stmt(&ctx, closure->as.closure_expr.body, return_type);
    blocks = zir_materialize_blocks(&ctx, return_type, &block_count);

    out = zir_make_function(
        zir_lower_strdup(name),
        params,
        closure->as.closure_expr.params.count,
        return_type,
        blocks,
        block_count);
    out.span = zir_span_from_source(closure->span);
    out.is_closure = 1;

    {
        char ctx_name[128];
        snprintf(ctx_name, sizeof(ctx_name), "ctx_%s", name);
        out.closure_ctx_type_name = zir_lower_strdup(ctx_name);
    }

    for (i = 0; i < closure->as.closure_expr.captures.count; i++) {
        const zt_hir_capture *hir_cap = &closure->as.closure_expr.captures.items[i];
        zir_capture zir_cap;
        zir_cap.name = zir_lower_strdup(hir_cap->name);
        zir_cap.type_name = zir_type_name_owned(hir_cap->type);
        zir_cap.span = zir_span_from_source(closure->span);
        zir_capture_list_push(&out.context_captures, zir_cap);
    }

    return out;
}

static zir_function zir_lower_function_decl(
        zir_lower_env *env,
        const zt_hir_decl *decl) {
    zir_function out;
    zir_function_ctx ctx;
    zir_param *params = NULL;
    size_t param_count = 0;
    size_t next_param = 0;
    size_t i;
    char *return_type = NULL;
    zir_block *blocks = NULL;
    size_t block_count = 0;

    memset(&out, 0, sizeof(out));
    if (decl == NULL || decl->kind != ZT_HIR_FUNC_DECL) return out;

    memset(&ctx, 0, sizeof(ctx));
    ctx.env = env;
    zir_block_state_buffer_init(&ctx.blocks);
    ctx.current_block = zir_add_block(&ctx, "entry", zir_span_from_source(decl->span));

    param_count = decl->as.func_decl.params.count;
    if (decl->as.func_decl.receiver_type_name != NULL) {
        param_count += 1;
    }
    if (param_count > 0) {
        params = (zir_param *)calloc(param_count, sizeof(zir_param));
    }

    if (decl->as.func_decl.receiver_type_name != NULL && next_param < param_count) {
        params[next_param] = zir_make_param(
            zir_lower_strdup("self"),
            zir_lower_strdup(decl->as.func_decl.receiver_type_name),
            NULL);
        params[next_param].span = zir_span_from_source(decl->span);
        next_param += 1;
    }

    for (i = 0; i < decl->as.func_decl.params.count && next_param < param_count; i += 1) {
        const zt_hir_param *hir_param = &decl->as.func_decl.params.items[i];
        zir_expr *where_expr = NULL;
        zir_expr *message_expr;
        zir_expr *eval_expr;
        char *message_text;
        zir_span param_span;

        if (hir_param->where_clause != NULL) {
            where_expr = zir_lower_hir_expr(env, hir_param->where_clause, NULL, NULL, hir_param->name);
        }

        params[next_param] = zir_make_param(
            zir_lower_strdup(hir_param->name),
            zir_type_name_owned(hir_param->type),
            where_expr);
        param_span = zir_span_from_source(
            hir_param->where_clause != NULL ? hir_param->where_clause->span : decl->span);
        params[next_param].span = param_span;

        if (hir_param->where_clause != NULL) {
            message_expr = zir_lower_hir_expr(env, hir_param->where_clause, NULL, NULL, hir_param->name);
            eval_expr = zir_lower_hir_expr(env, hir_param->where_clause, NULL, NULL, hir_param->name);
            message_text = zir_expr_render_alloc(message_expr);
            zir_emit_check_contract(
                &ctx,
                params[next_param].name,
                params[next_param].type_name,
                eval_expr,
                message_text,
                param_span);
            zir_expr_dispose(message_expr);
            free(message_text);
        }

        next_param += 1;
    }

    return_type = zir_type_name_owned(decl->as.func_decl.return_type);
    zir_lower_stmt(&ctx, decl->as.func_decl.body, return_type);
    blocks = zir_materialize_blocks(&ctx, return_type, &block_count);
    if (blocks == NULL && block_count == 0 && ctx.blocks.count != 0) {
        zir_add_lower_diag(env->diagnostics, decl->span, "out of memory materializing lowered blocks");
        zir_dispose_block_states(&ctx);
    }

    out = zir_make_function(
        zir_lower_strdup(decl->as.func_decl.name),
        params,
        param_count,
        return_type,
        blocks,
        block_count);
    out.span = zir_span_from_source(decl->span);
    out.receiver_type_name = zir_lower_strdup(decl->as.func_decl.receiver_type_name);
    out.implemented_trait_name = zir_lower_strdup(decl->as.func_decl.implemented_trait_name);
    out.is_mutating = decl->as.func_decl.is_mutating;
    return out;
}

static zir_module_var zir_lower_module_var_decl(
        zir_lower_env *env,
        const zt_hir_module_var *module_var) {
    zir_module_var out;
    zir_expr *init_expr = NULL;

    if (module_var != NULL && module_var->init_value != NULL) {
        init_expr = zir_lower_hir_expr(env, module_var->init_value, NULL, NULL, NULL);
    }

    if (init_expr != NULL) {
        out = zir_make_module_var_expr(
            zir_lower_strdup(module_var->name),
            zir_type_name_owned(module_var->type),
            init_expr);
    } else {
        out = zir_make_module_var(
            zir_lower_strdup(module_var != NULL ? module_var->name : ""),
            zir_type_name_owned(module_var != NULL ? module_var->type : NULL),
            NULL);
    }

    out.span = zir_span_from_source(module_var != NULL ? module_var->span : zt_source_span_unknown());
    return out;
}

static zir_decl_counts zir_count_decls(const zt_hir_module *module_hir) {
    zir_decl_counts counts;
    size_t i;
    memset(&counts, 0, sizeof(counts));
    if (module_hir == NULL) return counts;
    counts.module_var_count = module_hir->module_vars.count;
    for (i = 0; i < module_hir->declarations.count; i += 1) {
        const zt_hir_decl *decl = module_hir->declarations.items[i];
        if (decl == NULL) continue;
        if (decl->kind == ZT_HIR_STRUCT_DECL) counts.struct_count += 1;
        if (decl->kind == ZT_HIR_ENUM_DECL) counts.enum_count += 1;
        if (decl->kind == ZT_HIR_FUNC_DECL) counts.function_count += 1;
    }
    return counts;
}

zir_lower_result zir_lower_hir_to_zir(const zt_hir_module *module_hir) {
    zir_lower_env env_ctx;
    zir_lower_env *env = &env_ctx;
    zir_lower_result out;
    zir_decl_counts counts;
    zir_struct_decl *structs = NULL;
    zir_enum_decl *enums = NULL;
    zir_module_var *module_vars = NULL;
    zir_function *functions = NULL;
    size_t struct_index = 0;
    size_t enum_index = 0;
    size_t module_var_index = 0;
    size_t fn_index = 0;
    size_t i;

    memset(&out, 0, sizeof(out));
    out.diagnostics = zt_diag_list_make();

    memset(&env_ctx, 0, sizeof(env_ctx));
    env_ctx.module_hir = module_hir;
    env_ctx.diagnostics = &out.diagnostics;
    
    {
        size_t *closure_counter = (size_t *)malloc(sizeof(size_t));
        if (closure_counter != NULL) {
            *closure_counter = 0;
        }
        env_ctx.closure_counter = closure_counter;
    }
    
    {
        zir_function_buffer hoisted;
        zir_function_buffer_init(&hoisted);
        env_ctx.hoisted_functions = (zir_function_buffer*)malloc(sizeof(zir_function_buffer));
        *env_ctx.hoisted_functions = hoisted;
    }

    if (env->module_hir == NULL) {
        zt_diag_list_add(
            &out.diagnostics,
            ZT_DIAG_INVALID_TYPE,
            zt_source_span_unknown(),
            "HIR module cannot be null");
        out.module = zir_make_module_with_decls(
            zir_lower_strdup("unknown"),
            NULL,
            0,
            NULL,
            0,
            NULL,
            0);
        return out;
    }

    if (!zir_validate_module_nesting(env, &out.diagnostics)) {
        out.module = zir_make_module_with_decls_and_vars(
            zir_lower_strdup(env->module_hir->module_name != NULL ? env->module_hir->module_name : "main"),
            NULL,
            0,
            NULL,
            0,
            NULL,
            0,
            NULL,
            0);
        out.module.span = zir_span_from_source(env->module_hir->span);
        return out;
    }

    counts = zir_count_decls(env->module_hir);
    if (counts.struct_count > 0) {
        structs = (zir_struct_decl *)calloc(counts.struct_count, sizeof(zir_struct_decl));
    }
    if (counts.enum_count > 0) {
        enums = (zir_enum_decl *)calloc(counts.enum_count, sizeof(zir_enum_decl));
    }
    if (counts.module_var_count > 0) {
        module_vars = (zir_module_var *)calloc(counts.module_var_count, sizeof(zir_module_var));
    }
    if (counts.function_count > 0) {
        functions = (zir_function *)calloc(counts.function_count, sizeof(zir_function));
    }

    for (i = 0; i < env->module_hir->module_vars.count; i += 1) {
        const zt_hir_module_var *module_var = &env->module_hir->module_vars.items[i];
        if (module_var_index < counts.module_var_count) {
            module_vars[module_var_index++] = zir_lower_module_var_decl(env, module_var);
        }
    }

    for (i = 0; i < env->module_hir->declarations.count; i += 1) {
        const zt_hir_decl *decl = env->module_hir->declarations.items[i];
        if (decl == NULL) continue;

        if (decl->kind == ZT_HIR_STRUCT_DECL && struct_index < counts.struct_count) {
            structs[struct_index++] = zir_lower_struct_decl(env, decl);
            continue;
        }

        if (decl->kind == ZT_HIR_ENUM_DECL && enum_index < counts.enum_count) {
            enums[enum_index++] = zir_lower_enum_decl(decl);
            continue;
        }

        if (decl->kind == ZT_HIR_FUNC_DECL && fn_index < counts.function_count) {
            functions[fn_index++] = zir_lower_function_decl(env, decl);
            continue;
        }
    }

    if (env_ctx.hoisted_functions->count > 0) {
        size_t total_fns = counts.function_count + env_ctx.hoisted_functions->count;
        zir_function *all_fns = (zir_function *)calloc(total_fns, sizeof(zir_function));
        if (functions != NULL) {
            memcpy(all_fns, functions, counts.function_count * sizeof(zir_function));
            free(functions);
        }
        for (i = 0; i < env_ctx.hoisted_functions->count; i++) {
            all_fns[counts.function_count + i] = env_ctx.hoisted_functions->items[i];
        }
        functions = all_fns;
        counts.function_count = total_fns;
    }

    /* Free the buffer but not the items (they are now in 'functions') */
    free(env_ctx.hoisted_functions->items);
    free(env_ctx.hoisted_functions);
    free(env_ctx.closure_counter);

    out.module = zir_make_module_with_decls_and_vars(
        zir_lower_strdup(env->module_hir->module_name != NULL ? env->module_hir->module_name : "main"),
        structs,
        counts.struct_count,
        enums,
        counts.enum_count,
        module_vars,
        counts.module_var_count,
        functions,
        counts.function_count);
    out.module.span = zir_span_from_source(env->module_hir->span);
    return out;
}

void zir_lower_result_dispose(zir_lower_result *result) {
    if (result == NULL) return;
    zir_module_dispose_owned(&result->module);
    zt_diag_list_dispose(&result->diagnostics);
    memset(result, 0, sizeof(*result));
}
