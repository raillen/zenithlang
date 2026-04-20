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

typedef struct zir_function_ctx {
    const zt_hir_module *module_hir;
    zt_diag_list *diagnostics;
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
    size_t function_count;
} zir_decl_counts;

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

static const char *zir_prev_segment(const char *name) {
    const char *last;
    const char *cursor;
    if (name == NULL) return NULL;
    last = strrchr(name, '.');
    if (last == NULL || last == name) return NULL;
    cursor = last - 1;
    while (cursor > name && *cursor != '.') cursor -= 1;
    if (*cursor == '.') return cursor + 1;
    return name;
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
    if (last_dot == NULL) return 0;
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

static int zir_type_name_is_optional(const char *type_name) {
    return zir_starts_with(type_name, "optional<");
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

static const zt_hir_decl *zir_find_struct_decl_hir(const zt_hir_module *module_decl, const char *name) {
    size_t i;
    if (module_decl == NULL || name == NULL) return NULL;
    for (i = 0; i < module_decl->declarations.count; i += 1) {
        const zt_hir_decl *decl = module_decl->declarations.items[i];
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
        const zt_hir_module *module_decl,
        const char *enum_name,
        const char *variant_name,
        size_t *out_index) {
    size_t i;
    if (out_index != NULL) *out_index = 0;
    if (module_decl == NULL || enum_name == NULL || variant_name == NULL) return 0;
    for (i = 0; i < module_decl->declarations.count; i += 1) {
        const zt_hir_decl *decl = module_decl->declarations.items[i];
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

static zir_expr *zir_lower_hir_expr(
        const zt_hir_module *module_decl,
        const zt_hir_expr *expr,
        const char *replace_ident_from,
        const char *replace_ident_to,
        const char *replace_it_to);

static void zir_call_add_lowered_args(
        zir_expr *call,
        const zt_hir_module *module_decl,
        const zt_hir_expr_list *args,
        const char *replace_ident_from,
        const char *replace_ident_to,
        const char *replace_it_to) {
    size_t i;
    if (call == NULL || args == NULL) return;
    for (i = 0; i < args->count; i += 1) {
        zir_expr_call_add_arg(
            call,
            zir_lower_hir_expr(module_decl, args->items[i], replace_ident_from, replace_ident_to, replace_it_to));
    }
}

static zir_expr *zir_lower_len_call(
        const zt_hir_module *module_decl,
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
    arg_expr = zir_lower_hir_expr(module_decl, arg, replace_ident_from, replace_ident_to, replace_it_to);
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
        const zt_hir_module *module_decl,
        const zt_hir_expr *expr,
        const char *replace_ident_from,
        const char *replace_ident_to,
        const char *replace_it_to) {
    const char *callee_name = expr->as.call_expr.callee_name != NULL ? expr->as.call_expr.callee_name : "";
    zir_expr *call = NULL;

    if (zir_text_eq(callee_name, "len")) {
        zir_expr *len_expr = zir_lower_len_call(module_decl, expr, replace_ident_from, replace_ident_to, replace_it_to);
        if (len_expr != NULL) return len_expr;
    }

    if (zir_name_matches(callee_name, "core.list_get_i64")) {
        call = zir_expr_make_call_extern("c.zt_list_i64_get_optional");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_name_matches(callee_name, "core.list_get_text")) {
        call = zir_expr_make_call_extern("c.zt_list_text_get_optional");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_name_matches(callee_name, "core.map_get")) {
        call = zir_expr_make_call_extern("c.zt_map_text_text_get_optional");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }

    if (zir_call_is_module_func(callee_name, "bytes", "from_list")) {
        call = zir_expr_make_call_extern("c.zt_bytes_from_list_i64");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "to_list")) {
        call = zir_expr_make_call_extern("c.zt_bytes_to_list_i64");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "join")) {
        call = zir_expr_make_call_extern("c.zt_bytes_join");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "starts_with")) {
        call = zir_expr_make_call_extern("c.zt_bytes_starts_with");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "ends_with")) {
        call = zir_expr_make_call_extern("c.zt_bytes_ends_with");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "contains")) {
        call = zir_expr_make_call_extern("c.zt_bytes_contains");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "bytes", "empty")) {
        call = zir_expr_make_call_extern("c.zt_bytes_empty");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "text", "to_utf8")) {
        call = zir_expr_make_call_extern("c.zt_text_to_utf8_bytes");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "text", "from_utf8")) {
        call = zir_expr_make_call_extern("c.zt_text_from_utf8_bytes");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "io", "zt_host_write_stdout")) {
        call = zir_expr_make_call_extern("c.zt_host_write_stdout");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "io", "zt_host_write_stderr")) {
        call = zir_expr_make_call_extern("c.zt_host_write_stderr");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "io", "zt_host_read_line_stdin")) {
        call = zir_expr_make_call_extern("c.zt_host_read_line_stdin");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "io", "zt_host_read_all_stdin")) {
        call = zir_expr_make_call_extern("c.zt_host_read_all_stdin");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "fs", "zt_host_read_file")) {
        call = zir_expr_make_call_extern("c.zt_host_read_file");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "fs", "zt_host_write_file")) {
        call = zir_expr_make_call_extern("c.zt_host_write_file");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "fs", "zt_host_path_exists")) {
        call = zir_expr_make_call_extern("c.zt_host_path_exists");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "time", "zt_host_time_now_unix_ms")) {
        call = zir_expr_make_call_extern("c.zt_host_time_now_unix_ms");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "time", "zt_host_time_sleep_ms")) {
        call = zir_expr_make_call_extern("c.zt_host_time_sleep_ms");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "os", "zt_host_os_current_dir")) {
        call = zir_expr_make_call_extern("c.zt_host_os_current_dir");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "os", "zt_host_os_change_dir")) {
        call = zir_expr_make_call_extern("c.zt_host_os_change_dir");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "os", "zt_host_os_env")) {
        call = zir_expr_make_call_extern("c.zt_host_os_env");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "os", "zt_host_os_pid")) {
        call = zir_expr_make_call_extern("c.zt_host_os_pid");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "os", "zt_host_os_platform")) {
        call = zir_expr_make_call_extern("c.zt_host_os_platform");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "os", "zt_host_os_arch")) {
        call = zir_expr_make_call_extern("c.zt_host_os_arch");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "process", "zt_host_process_run")) {
        call = zir_expr_make_call_extern("c.zt_host_process_run");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "path", "zt_text_concat")) {
        call = zir_expr_make_call_extern("c.zt_text_concat");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "json", "zt_json_parse_map_text_text")) {
        call = zir_expr_make_call_extern("c.zt_json_parse_map_text_text");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "json", "zt_json_stringify_map_text_text")) {
        call = zir_expr_make_call_extern("c.zt_json_stringify_map_text_text");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "json", "zt_json_pretty_map_text_text")) {
        call = zir_expr_make_call_extern("c.zt_json_pretty_map_text_text");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "format", "zt_format_hex_i64")) {
        call = zir_expr_make_call_extern("c.zt_format_hex_i64");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "format", "zt_format_bin_i64")) {
        call = zir_expr_make_call_extern("c.zt_format_bin_i64");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "format", "zt_format_bytes_binary")) {
        call = zir_expr_make_call_extern("c.zt_format_bytes_binary");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_call_is_module_func(callee_name, "format", "zt_format_bytes_decimal")) {
        call = zir_expr_make_call_extern("c.zt_format_bytes_decimal");
        zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
        return call;
    }
    if (zir_starts_with(callee_name, "c.")) {
        call = zir_expr_make_call_extern(callee_name);
    } else {
        call = zir_expr_make_call_direct(callee_name);
    }
    zir_call_add_lowered_args(call, module_decl, &expr->as.call_expr.args, replace_ident_from, replace_ident_to, replace_it_to);
    return call;
}

static zir_expr *zir_lower_hir_expr(
        const zt_hir_module *module_decl,
        const zt_hir_expr *expr,
        const char *replace_ident_from,
        const char *replace_ident_to,
        const char *replace_it_to) {
    zir_expr *out;
    size_t i;

    (void)module_decl;
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
            zir_expr *value = zir_lower_hir_expr(
                module_decl,
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
            return zir_expr_make_outcome_failure(zir_lower_hir_expr(
                module_decl,
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
                    zir_lower_hir_expr(
                        module_decl,
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
                    zir_lower_hir_expr(
                        module_decl,
                        expr->as.map_expr.entries.items[i].key,
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to),
                    zir_lower_hir_expr(
                        module_decl,
                        expr->as.map_expr.entries.items[i].value,
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            }
            return out;
        }

        case ZT_HIR_UNARY_EXPR:
            if (expr->as.unary_expr.op == ZT_TOKEN_QUESTION) {
                return zir_expr_make_try_propagate(zir_lower_hir_expr(
                    module_decl,
                    expr->as.unary_expr.operand,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to));
            }
            return zir_expr_make_unary(
                zir_unary_op_name(expr->as.unary_expr.op),
                zir_lower_hir_expr(
                    module_decl,
                    expr->as.unary_expr.operand,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to));

        case ZT_HIR_BINARY_EXPR:
            return zir_expr_make_binary(
                zir_binary_op_name(expr->as.binary_expr.op),
                zir_lower_hir_expr(
                    module_decl,
                    expr->as.binary_expr.left,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to),
                zir_lower_hir_expr(
                    module_decl,
                    expr->as.binary_expr.right,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to));

        case ZT_HIR_FIELD_EXPR:
            if (expr->as.field_expr.object != NULL &&
                    expr->as.field_expr.object->kind == ZT_HIR_IDENT_EXPR &&
                    expr->as.field_expr.field_name != NULL) {
                size_t variant_index = 0;
                if (zir_find_enum_variant_index_hir(
                        module_decl,
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
                zir_lower_hir_expr(
                    module_decl,
                    expr->as.field_expr.object,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to),
                expr->as.field_expr.field_name);

        case ZT_HIR_INDEX_EXPR:
            return zir_expr_make_index_seq(
                zir_lower_hir_expr(
                    module_decl,
                    expr->as.index_expr.object,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to),
                zir_lower_hir_expr(
                    module_decl,
                    expr->as.index_expr.index,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to));

        case ZT_HIR_SLICE_EXPR:
        {
            zir_expr *object_expr = zir_lower_hir_expr(
                module_decl,
                expr->as.slice_expr.object,
                replace_ident_from,
                replace_ident_to,
                replace_it_to);
            zir_expr *start_expr = NULL;
            zir_expr *end_expr = NULL;
            if (expr->as.slice_expr.start != NULL) {
                start_expr = zir_lower_hir_expr(
                    module_decl,
                    expr->as.slice_expr.start,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to);
            } else {
                start_expr = zir_expr_make_int("0");
            }

            if (expr->as.slice_expr.end != NULL) {
                end_expr = zir_lower_hir_expr(
                    module_decl,
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
                    zir_lower_hir_expr(
                        module_decl,
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
                    zir_lower_hir_expr(
                        module_decl,
                        expr->as.slice_expr.object,
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            } else if (expr->as.slice_expr.object != NULL &&
                    expr->as.slice_expr.object->type != NULL &&
                    expr->as.slice_expr.object->type->kind == ZT_TYPE_MAP) {
                end_expr = zir_expr_make_map_len(
                    zir_lower_hir_expr(
                        module_decl,
                        expr->as.slice_expr.object,
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            } else {
                end_expr = zir_expr_make_list_len(
                    zir_lower_hir_expr(
                        module_decl,
                        expr->as.slice_expr.object,
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            }

            return zir_expr_make_slice_seq(object_expr, start_expr, end_expr);
        }

        case ZT_HIR_CALL_EXPR:
            return zir_lower_call_expr(module_decl, expr, replace_ident_from, replace_ident_to, replace_it_to);

        case ZT_HIR_METHOD_CALL_EXPR: {
            zir_expr *call = zir_expr_make_call_direct(expr->as.method_call_expr.method_name);
            zir_expr_call_add_arg(
                call,
                zir_lower_hir_expr(
                    module_decl,
                    expr->as.method_call_expr.receiver,
                    replace_ident_from,
                    replace_ident_to,
                    replace_it_to));
            for (i = 0; i < expr->as.method_call_expr.args.count; i += 1) {
                zir_expr_call_add_arg(
                    call,
                    zir_lower_hir_expr(
                        module_decl,
                        expr->as.method_call_expr.args.items[i],
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            }
            return call;
        }

        case ZT_HIR_CONSTRUCT_EXPR: {
            out = zir_expr_make_make_struct(expr->as.construct_expr.type_name);
            for (i = 0; i < expr->as.construct_expr.fields.count; i += 1) {
                const zt_hir_field_init *field = &expr->as.construct_expr.fields.items[i];
                zir_expr_make_struct_add_field(
                    out,
                    field->name,
                    zir_lower_hir_expr(
                        module_decl,
                        field->value,
                        replace_ident_from,
                        replace_ident_to,
                        replace_it_to));
            }
            return out;
        }

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

    struct_decl = zir_find_struct_decl_hir(ctx->module_hir, init_value->as.construct_expr.type_name);
    if (struct_decl == NULL) return 0;

    construct_expr = zir_expr_make_make_struct(init_value->as.construct_expr.type_name);
    if (construct_expr == NULL) return 0;

    for (i = 0; i < init_value->as.construct_expr.fields.count; i += 1) {
        const zt_hir_field_init *field_init = &init_value->as.construct_expr.fields.items[i];
        const zt_hir_field_decl *field_decl = zir_find_struct_field_hir(struct_decl, field_init->name);

        if (field_decl != NULL && field_decl->where_clause != NULL) {
            char *temp_name = zir_lower_format("__zt_ctor_field_", ctx->generated_value_counter++);
            char *temp_type = zir_type_name_owned(field_decl->type);
            zir_expr *value_expr = zir_lower_hir_expr(ctx->module_hir, field_init->value, NULL, NULL, NULL);
            zir_expr *message_expr;
            zir_expr *eval_expr;
            char *message_text;

            zir_emit_assign_expr(ctx, temp_name, temp_type, value_expr, zir_span_from_source(field_init->span));

            message_expr = zir_lower_hir_expr(
                ctx->module_hir,
                field_decl->where_clause,
                NULL,
                NULL,
                field_decl->name);
            eval_expr = zir_lower_hir_expr(
                ctx->module_hir,
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
            zir_lower_hir_expr(ctx->module_hir, field_init->value, NULL, NULL, NULL));
    }

    zir_emit_assign_expr(ctx, target_name, target_type_name, construct_expr, span);
    return 1;
}

static void zir_lower_try_assignment(
        zir_function_ctx *ctx,
        const char *target_name,
        const char *target_type_name,
        const zt_hir_expr *try_expr,
        zir_span span) {
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

    temp_name = zir_lower_format("__zt_try_result_", ctx->try_temp_counter++);
    temp_type = zir_type_name_owned(try_expr->as.unary_expr.operand->type);
    operand_expr = zir_lower_hir_expr(ctx->module_hir, try_expr->as.unary_expr.operand, NULL, NULL, NULL);
    zir_emit_assign_expr(ctx, temp_name, temp_type, operand_expr, span);

    success_label = zir_lower_format("try_success_", ctx->try_label_counter++);
    failure_label = zir_lower_format("try_failure_", ctx->try_label_counter++);
    after_label = zir_lower_format("try_after_", ctx->try_label_counter++);

    cond_expr = zir_expr_make_outcome_is_success(zir_expr_make_name(temp_name));
    zir_set_terminator(
        ctx,
        zir_make_branch_if_terminator_expr(cond_expr, zir_lower_strdup(success_label), zir_lower_strdup(failure_label)));

    failure_block = zir_add_block(ctx, failure_label, span);
    ctx->current_block = failure_block;
    zir_set_terminator(
        ctx,
        zir_make_return_terminator_expr(
            zir_expr_make_try_propagate(zir_expr_make_name(temp_name))));

    success_block = zir_add_block(ctx, success_label, span);
    ctx->current_block = success_block;
    zir_emit_assign_expr(
        ctx,
        target_name,
        target_type_name,
        zir_expr_make_outcome_value(zir_expr_make_name(temp_name)),
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

static void zir_lower_try_effect(zir_function_ctx *ctx, const zt_hir_expr *try_expr, zir_span span) {
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

    if (ctx == NULL || try_expr == NULL || !zir_is_try_expr(try_expr)) return;

    temp_name = zir_lower_format("__zt_try_result_", ctx->try_temp_counter++);
    temp_type = zir_type_name_owned(try_expr->as.unary_expr.operand->type);
    operand_expr = zir_lower_hir_expr(ctx->module_hir, try_expr->as.unary_expr.operand, NULL, NULL, NULL);
    zir_emit_assign_expr(ctx, temp_name, temp_type, operand_expr, span);

    success_label = zir_lower_format("try_success_", ctx->try_label_counter++);
    failure_label = zir_lower_format("try_failure_", ctx->try_label_counter++);
    after_label = zir_lower_format("try_after_", ctx->try_label_counter++);

    cond_expr = zir_expr_make_outcome_is_success(zir_expr_make_name(temp_name));
    zir_set_terminator(
        ctx,
        zir_make_branch_if_terminator_expr(cond_expr, zir_lower_strdup(success_label), zir_lower_strdup(failure_label)));

    failure_block = zir_add_block(ctx, failure_label, span);
    ctx->current_block = failure_block;
    zir_set_terminator(
        ctx,
        zir_make_return_terminator_expr(
            zir_expr_make_try_propagate(zir_expr_make_name(temp_name))));

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
        zt_source_span span_source) {
    char *type_name = zir_type_name_owned(type);
    zir_span span = zir_span_from_source(span_source);

    if (zir_is_try_expr(init_value)) {
        zir_lower_try_assignment(ctx, name, type_name, init_value, span);
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
        zir_lower_hir_expr(ctx->module_hir, init_value, NULL, NULL, NULL),
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

    if (pattern->kind == ZT_HIR_FIELD_EXPR &&
            pattern->as.field_expr.object != NULL &&
            pattern->as.field_expr.object->kind == ZT_HIR_IDENT_EXPR &&
            pattern->as.field_expr.field_name != NULL) {
        size_t variant_index = 0;
        if (zir_find_enum_variant_index_hir(
                ctx->module_hir,
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
            zir_lower_hir_expr(ctx->module_hir, pattern, NULL, NULL, NULL));
    }

    {
        const zt_hir_field_init *tag_field = zir_find_construct_field(pattern, "__zt_enum_tag");
        size_t i;
        condition = zir_expr_make_binary(
            "eq",
            zir_expr_make_get_field(zir_expr_make_name(subject_name), "__zt_enum_tag"),
            zir_lower_hir_expr(ctx->module_hir, tag_field != NULL ? tag_field->value : NULL, NULL, NULL, NULL));

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
                zir_lower_hir_expr(ctx->module_hir, field->value, NULL, NULL, NULL));
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
            zir_lower_hir_expr(ctx->module_hir, stmt->as.match_stmt.subject, NULL, NULL, NULL),
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
                    zir_lower_hir_expr(ctx->module_hir, stmt->as.if_stmt.condition, NULL, NULL, NULL),
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
                    zir_lower_hir_expr(ctx->module_hir, stmt->as.while_stmt.condition, NULL, NULL, NULL),
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
                    ctx->diagnostics,
                    stmt->span,
                    "for-in currently supports list, map<text,text> and text");
            }

            zir_emit_assign_expr(
                ctx,
                iter_temp,
                iter_type_name,
                zir_lower_hir_expr(ctx->module_hir, stmt->as.for_stmt.iterable, NULL, NULL, NULL),
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
                zir_lower_hir_expr(ctx->module_hir, stmt->as.repeat_stmt.count, NULL, NULL, NULL),
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
                zir_expr *return_expr = zir_lower_hir_expr(
                    ctx->module_hir,
                    stmt->as.return_stmt.value,
                    NULL,
                    NULL,
                    NULL);
                if (zir_type_name_is_optional(fn_return_type) &&
                        (stmt->as.return_stmt.value->type == NULL ||
                         stmt->as.return_stmt.value->type->kind != ZT_TYPE_OPTIONAL)) {
                    return_expr = zir_expr_make_optional_present(return_expr);
                }
                zir_set_terminator(
                    ctx,
                    zir_make_return_terminator_expr(return_expr));
            }
            return;

        case ZT_HIR_CONST_STMT:
            zir_lower_var_like_statement(
                ctx,
                stmt->as.const_stmt.name,
                stmt->as.const_stmt.type,
                stmt->as.const_stmt.init_value,
                stmt->span);
            return;

        case ZT_HIR_VAR_STMT:
            zir_lower_var_like_statement(
                ctx,
                stmt->as.var_stmt.name,
                stmt->as.var_stmt.type,
                stmt->as.var_stmt.init_value,
                stmt->span);
            return;

        case ZT_HIR_ASSIGN_STMT: {
            char *type_name = zir_type_name_owned(
                stmt->as.assign_stmt.value != NULL ? stmt->as.assign_stmt.value->type : NULL);
            if (zir_is_try_expr(stmt->as.assign_stmt.value)) {
                zir_lower_try_assignment(ctx, stmt->as.assign_stmt.name, type_name, stmt->as.assign_stmt.value, span);
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
                zir_lower_hir_expr(ctx->module_hir, stmt->as.assign_stmt.value, NULL, NULL, NULL),
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
                    zir_lower_hir_expr(ctx->module_hir, stmt->as.index_assign_stmt.object, NULL, NULL, NULL),
                    zir_lower_hir_expr(ctx->module_hir, stmt->as.index_assign_stmt.index, NULL, NULL, NULL),
                    zir_lower_hir_expr(ctx->module_hir, stmt->as.index_assign_stmt.value, NULL, NULL, NULL));
            } else {
                effect_expr = zir_expr_make_list_set(
                    zir_lower_hir_expr(ctx->module_hir, stmt->as.index_assign_stmt.object, NULL, NULL, NULL),
                    zir_lower_hir_expr(ctx->module_hir, stmt->as.index_assign_stmt.index, NULL, NULL, NULL),
                    zir_lower_hir_expr(ctx->module_hir, stmt->as.index_assign_stmt.value, NULL, NULL, NULL));
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
                struct_decl = zir_find_struct_decl_hir(ctx->module_hir, object_type->name);
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
                    zir_lower_hir_expr(ctx->module_hir, stmt->as.field_assign_stmt.value, NULL, NULL, NULL),
                    span);

                zir_emit_effect_expr(
                    ctx,
                    zir_expr_make_set_field(
                        zir_lower_hir_expr(ctx->module_hir, stmt->as.field_assign_stmt.object, NULL, NULL, NULL),
                        stmt->as.field_assign_stmt.field_name,
                        zir_expr_make_name(temp_name)),
                    span);

                message_expr = zir_lower_hir_expr(
                    ctx->module_hir,
                    field_decl->where_clause,
                    NULL,
                    NULL,
                    stmt->as.field_assign_stmt.field_name);
                eval_expr = zir_lower_hir_expr(
                    ctx->module_hir,
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
                    zir_lower_hir_expr(ctx->module_hir, stmt->as.field_assign_stmt.object, NULL, NULL, NULL),
                    stmt->as.field_assign_stmt.field_name,
                    zir_lower_hir_expr(ctx->module_hir, stmt->as.field_assign_stmt.value, NULL, NULL, NULL)),
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
                zir_lower_try_effect(ctx, stmt->as.expr_stmt.expr, span);
                return;
            }
            zir_emit_effect_expr(
                ctx,
                zir_lower_hir_expr(ctx->module_hir, stmt->as.expr_stmt.expr, NULL, NULL, NULL),
                span);
            return;

        default:
            zir_add_lower_diag(ctx->diagnostics, stmt->span, "unsupported statement during HIR->ZIR lowering");
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

static zir_struct_decl zir_lower_struct_decl(const zt_hir_decl *decl, const zt_hir_module *module_hir) {
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
            where_expr = zir_lower_hir_expr(module_hir, hir_field->where_clause, NULL, NULL, hir_field->name);
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

static zir_function zir_lower_function_decl(
        const zt_hir_module *module_hir,
        zt_diag_list *diagnostics,
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
    ctx.module_hir = module_hir;
    ctx.diagnostics = diagnostics;
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
            where_expr = zir_lower_hir_expr(module_hir, hir_param->where_clause, NULL, NULL, hir_param->name);
        }

        params[next_param] = zir_make_param(
            zir_lower_strdup(hir_param->name),
            zir_type_name_owned(hir_param->type),
            where_expr);
        param_span = zir_span_from_source(
            hir_param->where_clause != NULL ? hir_param->where_clause->span : decl->span);
        params[next_param].span = param_span;

        if (hir_param->where_clause != NULL) {
            message_expr = zir_lower_hir_expr(module_hir, hir_param->where_clause, NULL, NULL, hir_param->name);
            eval_expr = zir_lower_hir_expr(module_hir, hir_param->where_clause, NULL, NULL, hir_param->name);
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
        zir_add_lower_diag(diagnostics, decl->span, "out of memory materializing lowered blocks");
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

static zir_decl_counts zir_count_decls(const zt_hir_module *module_hir) {
    zir_decl_counts counts;
    size_t i;
    memset(&counts, 0, sizeof(counts));
    if (module_hir == NULL) return counts;
    for (i = 0; i < module_hir->declarations.count; i += 1) {
        const zt_hir_decl *decl = module_hir->declarations.items[i];
        if (decl == NULL) continue;
        if (decl->kind == ZT_HIR_STRUCT_DECL) counts.struct_count += 1;
        if (decl->kind == ZT_HIR_ENUM_DECL) counts.enum_count += 1;
        if (decl->kind == ZT_HIR_FUNC_DECL) counts.function_count += 1;
    }
    return counts;
}

zir_lower_result zir_lower_hir_to_zir(const zt_hir_module *module_decl) {
    zir_lower_result out;
    zir_decl_counts counts;
    zir_struct_decl *structs = NULL;
    zir_enum_decl *enums = NULL;
    zir_function *functions = NULL;
    size_t struct_index = 0;
    size_t enum_index = 0;
    size_t fn_index = 0;
    size_t i;

    memset(&out, 0, sizeof(out));
    out.diagnostics = zt_diag_list_make();

    if (module_decl == NULL) {
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

    counts = zir_count_decls(module_decl);
    if (counts.struct_count > 0) {
        structs = (zir_struct_decl *)calloc(counts.struct_count, sizeof(zir_struct_decl));
    }
    if (counts.enum_count > 0) {
        enums = (zir_enum_decl *)calloc(counts.enum_count, sizeof(zir_enum_decl));
    }
    if (counts.function_count > 0) {
        functions = (zir_function *)calloc(counts.function_count, sizeof(zir_function));
    }

    for (i = 0; i < module_decl->declarations.count; i += 1) {
        const zt_hir_decl *decl = module_decl->declarations.items[i];
        if (decl == NULL) continue;

        if (decl->kind == ZT_HIR_STRUCT_DECL && struct_index < counts.struct_count) {
            structs[struct_index++] = zir_lower_struct_decl(decl, module_decl);
            continue;
        }

        if (decl->kind == ZT_HIR_ENUM_DECL && enum_index < counts.enum_count) {
            enums[enum_index++] = zir_lower_enum_decl(decl);
            continue;
        }

        if (decl->kind == ZT_HIR_FUNC_DECL && fn_index < counts.function_count) {
            functions[fn_index++] = zir_lower_function_decl(module_decl, &out.diagnostics, decl);
            continue;
        }
    }

    out.module = zir_make_module_with_decls(
        zir_lower_strdup(module_decl->module_name != NULL ? module_decl->module_name : "main"),
        structs,
        counts.struct_count,
        enums,
        counts.enum_count,
        functions,
        counts.function_count);
    out.module.span = zir_span_from_source(module_decl->span);
    return out;
}

void zir_lower_result_dispose(zir_lower_result *result) {
    if (result == NULL) return;
    zir_module_dispose_owned(&result->module);
    zt_diag_list_dispose(&result->diagnostics);
    memset(result, 0, sizeof(*result));
}

