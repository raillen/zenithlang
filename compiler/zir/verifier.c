#include "compiler/zir/verifier.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_COUNT(items) (sizeof(items) / sizeof((items)[0]))

static const char *zir_safe_text(const char *text) {
    return text != NULL ? text : "";
}

typedef struct zir_symbol_table {
    const char **names;
    size_t count;
    size_t capacity;
} zir_symbol_table;

static int zir_span_ptr_is_known(const zir_span *span) {
    return span != NULL && zir_span_is_known(*span);
}

static const zir_span *zir_prefer_span(const zir_span *primary, const zir_span *fallback) {
    if (zir_span_ptr_is_known(primary)) {
        return primary;
    }
    if (zir_span_ptr_is_known(fallback)) {
        return fallback;
    }
    return NULL;
}

static void zir_verifier_set_result_at(zir_verifier_result *result, zir_verifier_code code, const zir_span *span, const char *format, ...) {
    va_list args;
    char base_message[256];

    if (result == NULL) {
        return;
    }

    result->ok = (code == ZIR_VERIFIER_OK);
    result->code = code;
    result->source_name = NULL;
    result->line = 0;
    result->column = 0;
    result->has_span = 0;
    result->message[0] = '\0';

    if (format == NULL || format[0] == '\0') {
        return;
    }

    va_start(args, format);
    vsnprintf(base_message, sizeof(base_message), format, args);
    va_end(args);

    if (zir_span_ptr_is_known(span)) {
        result->source_name = span->source_name;
        result->line = span->line;
        result->column = span->column;
        result->has_span = 1;
        snprintf(result->message, sizeof(result->message), "%.160s", base_message);
        {
            size_t used = strlen(result->message);
            if (used < sizeof(result->message) - 1) {
                snprintf(
                    result->message + used,
                    sizeof(result->message) - used,
                    " at %s:%zu:%zu",
                    span->source_name,
                    span->line,
                    span->column
                );
            }
        }
        return;
    }

    snprintf(result->message, sizeof(result->message), "%s", base_message);
}

static void zir_verifier_set_result(zir_verifier_result *result, zir_verifier_code code, const char *format, ...) {
    va_list args;
    char base_message[256];

    if (result == NULL) {
        return;
    }

    if (format == NULL || format[0] == '\0') {
        zir_verifier_set_result_at(result, code, NULL, "");
        return;
    }

    va_start(args, format);
    vsnprintf(base_message, sizeof(base_message), format, args);
    va_end(args);
    zir_verifier_set_result_at(result, code, NULL, "%s", base_message);
}

void zir_verifier_result_init(zir_verifier_result *result) {
    zir_verifier_set_result(result, ZIR_VERIFIER_OK, "");
}

const char *zir_verifier_code_name(zir_verifier_code code) {
    switch (code) {
        case ZIR_VERIFIER_OK:
            return "zir.ok";
        case ZIR_VERIFIER_INVALID_INPUT:
            return "zir.invalid_input";
        case ZIR_VERIFIER_INVALID_TYPE_ANY:
            return "zir.invalid_type.any";
        case ZIR_VERIFIER_INVALID_TARGET_LEAK:
            return "zir.invalid_target_leak";
        case ZIR_VERIFIER_USE_BEFORE_DEFINITION:
            return "zir.use_before_definition";
        case ZIR_VERIFIER_UNKNOWN_BLOCK:
            return "zir.unknown_block";
        case ZIR_VERIFIER_INVALID_TERMINATOR:
            return "zir.invalid_terminator";
        case ZIR_VERIFIER_DUPLICATE_BLOCK_LABEL:
            return "zir.duplicate_block_label";
        case ZIR_VERIFIER_INVALID_RETURN:
            return "zir.invalid_return";
        default:
            return "zir.unknown_error";
    }
}

static int zir_is_identifier_start(char ch) {
    return ((ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            ch == '_');
}

static int zir_is_identifier_char(char ch) {
    return zir_is_identifier_start(ch) || (ch >= '0' && ch <= '9');
}

static int zir_text_is_blank(const char *text) {
    size_t index = 0;

    if (text == NULL) {
        return 1;
    }

    while (text[index] != '\0') {
        if (text[index] != ' ' && text[index] != '\t' && text[index] != '\n' && text[index] != '\r') {
            return 0;
        }
        index += 1;
    }

    return 1;
}

static int zir_starts_with(const char *text, const char *prefix) {
    if (text == NULL || prefix == NULL) {
        return 0;
    }
    return strncmp(text, prefix, strlen(prefix)) == 0;
}

static int zir_find_token(const char *text, const char *needle) {
    size_t index = 0;
    const size_t needle_len = strlen(needle);

    if (text == NULL || needle == NULL || needle_len == 0) {
        return 0;
    }

    while (text[index] != '\0') {
        size_t start = index;

        if (!zir_is_identifier_start(text[index])) {
            index += 1;
            continue;
        }

        index += 1;
        while (zir_is_identifier_char(text[index])) {
            index += 1;
        }

        if ((size_t)(index - start) == needle_len &&
                strncmp(text + start, needle, needle_len) == 0) {
            return 1;
        }
    }

    return 0;
}

static int zir_contains_any_type(const char *type_name) {
    return zir_find_token(type_name, "any");
}

static int zir_contains_target_leak(const char *text) {
    if (text == NULL) {
        return 0;
    }
    if (zir_find_token(text, "lua_call")) {
        return 1;
    }
    if (strstr(text, "lua.") != NULL) {
        return 1;
    }
    if (strstr(text, "zt.") != NULL) {
        return 1;
    }
    if (zir_find_token(text, "ffi")) {
        return 1;
    }
    if (zir_find_token(text, "metatable")) {
        return 1;
    }
    return 0;
}

static int zir_verify_type_name(const char *type_name, const char *context, const zir_span *span, zir_verifier_result *result) {
    if (zir_text_is_blank(type_name)) {
        return 1;
    }

    if (zir_contains_any_type(type_name)) {
        zir_verifier_set_result_at(
            result,
            ZIR_VERIFIER_INVALID_TYPE_ANY,
            span,
            "any is not part of the Zenith Next core (%s)",
            context
        );
        return 0;
    }

    if (zir_contains_target_leak(type_name)) {
        zir_verifier_set_result_at(
            result,
            ZIR_VERIFIER_INVALID_TARGET_LEAK,
            span,
            "target-specific names and legacy runtime helpers cannot appear in ZIR (%s)",
            context
        );
        return 0;
    }

    return 1;
}

static int zir_symbol_table_contains(const zir_symbol_table *table, const char *name) {
    size_t index;

    if (table == NULL || name == NULL) {
        return 0;
    }

    for (index = 0; index < table->count; index += 1) {
        if (strcmp(table->names[index], name) == 0) {
            return 1;
        }
    }

    return 0;
}

static int zir_symbol_table_add(zir_symbol_table *table, const char *name) {
    if (table == NULL || name == NULL || table->count >= table->capacity) {
        return 0;
    }

    table->names[table->count] = name;
    table->count += 1;
    return 1;
}

static int zir_symbol_table_add_unique(zir_symbol_table *table, const char *name) {
    if (zir_symbol_table_contains(table, name)) {
        return 1;
    }
    return zir_symbol_table_add(table, name);
}

static size_t zir_count_function_definitions(const zir_module *module_decl, const zir_function *function_decl) {
    size_t count = 0;
    size_t block_index;
    if (function_decl == NULL) {
        return 0;
    }

    if (module_decl != NULL) {
        count += module_decl->module_var_count;
        count += module_decl->function_count;
    }

    count += function_decl->param_count;
    for (block_index = 0; block_index < function_decl->block_count; block_index += 1) {
        size_t instruction_index;
        const zir_block *block = &function_decl->blocks[block_index];
        for (instruction_index = 0; instruction_index < block->instruction_count; instruction_index += 1) {
            if (block->instructions[instruction_index].kind == ZIR_INSTR_ASSIGN) {
                count += 1;
            }
        }
    }

    return count;
}

static int zir_collect_function_definitions(
        const zir_module *module_decl,
        const zir_function *function_decl,
        zir_symbol_table *defined,
        const zir_span *span,
        zir_verifier_result *result) {
    size_t module_var_index;
    size_t param_index;
    size_t block_index;

    if (function_decl == NULL || defined == NULL) {
        return 0;
    }

    if (module_decl != NULL) {
        for (module_var_index = 0; module_var_index < module_decl->module_var_count; module_var_index += 1) {
            if (!zir_symbol_table_add_unique(defined, module_decl->module_vars[module_var_index].name)) {
                zir_verifier_set_result_at(result, ZIR_VERIFIER_INVALID_INPUT, span, "unable to register module variable");
                return 0;
            }
        }
        for (size_t fn_index = 0; fn_index < module_decl->function_count; fn_index += 1) {
            if (!zir_symbol_table_add_unique(defined, module_decl->functions[fn_index].name)) {
                zir_verifier_set_result_at(result, ZIR_VERIFIER_INVALID_INPUT, span, "unable to register function");
                return 0;
            }
        }
    }

    for (param_index = 0; param_index < function_decl->param_count; param_index += 1) {
        if (!zir_symbol_table_add_unique(defined, function_decl->params[param_index].name)) {
            zir_verifier_set_result_at(result, ZIR_VERIFIER_INVALID_INPUT, span, "unable to register function parameter");
            return 0;
        }
    }

    for (block_index = 0; block_index < function_decl->block_count; block_index += 1) {
        size_t instruction_index;
        const zir_block *block = &function_decl->blocks[block_index];
        for (instruction_index = 0; instruction_index < block->instruction_count; instruction_index += 1) {
            const zir_instruction *instruction = &block->instructions[instruction_index];
            if (instruction->kind != ZIR_INSTR_ASSIGN) {
                continue;
            }
            if (!zir_symbol_table_add_unique(defined, instruction->dest_name)) {
                zir_verifier_set_result_at(result, ZIR_VERIFIER_INVALID_INPUT, span, "unable to register definition");
                return 0;
            }
        }
    }

    return 1;
}

static int zir_is_reserved_identifier(const char *token, size_t token_len, char prev_char, char next_char) {
    static const char *const reserved[] = {
        "binary",
        "unary",
        "logical",
        "call_direct",
        "call_extern",
        "call_runtime_intrinsic",
        "make_struct",
        "make_list",
        "make_map",
        "list_len",
        "map_len",
        "list_push",
        "list_set",
        "map_set",
        "index_seq",
        "slice_seq",
        "coalesce",
        "optional_present",
        "optional_empty",
        "optional_is_present",
        "optional_value",
        "outcome_success",
        "outcome_failure",
        "outcome_is_success",
        "outcome_value",
        "try_propagate",
        "get_field",
        "set_field",
        "const",
        "int",
        "float",
        "bool",
        "text",
        "void",
        "never",
        "list",
        "map",
        "grid2d",
        "pqueue",
        "circbuf",
        "btreemap",
        "btreeset",
        "grid3d",
        "struct",
        "enum",
        "union",
        "func",
        "opaque",
        "true",
        "false"
    };
    size_t index;

    if (prev_char == '.' || next_char == '.' || next_char == ':') {
        return 1;
    }

    if (token_len > 0 && token[0] >= 'A' && token[0] <= 'Z') {
        return 1;
    }

    for (index = 0; index < ARRAY_COUNT(reserved); index += 1) {
        if (strlen(reserved[index]) == token_len &&
                strncmp(token, reserved[index], token_len) == 0) {
            return 1;
        }
    }

    return 0;
}

static int zir_verify_identifier_usage_tokens(
        const char *text,
        const zir_symbol_table *defined,
        const char *context,
        const zir_span *span,
        zir_verifier_result *result) {
    size_t index = 0;

    if (zir_text_is_blank(text)) {
        return 1;
    }

    if (zir_contains_target_leak(text)) {
        zir_verifier_set_result_at(
            result,
            ZIR_VERIFIER_INVALID_TARGET_LEAK,
            span,
            "target-specific names and legacy runtime helpers cannot appear in ZIR (%s)",
            context
        );
        return 0;
    }

    while (text[index] != '\0') {
        size_t start;
        size_t token_len;
        char prev_char = '\0';
        char next_char = '\0';

        if (text[index] == '"') {
            index += 1;
            while (text[index] != '\0') {
                if (text[index] == '\\' && text[index + 1] != '\0') {
                    index += 2;
                    continue;
                }
                if (text[index] == '"') {
                    index += 1;
                    break;
                }
                index += 1;
            }
            continue;
        }

        if (!zir_is_identifier_start(text[index])) {
            index += 1;
            continue;
        }

        start = index;
        if (start > 0) {
            prev_char = text[start - 1];
        }

        index += 1;
        while (zir_is_identifier_char(text[index])) {
            index += 1;
        }

        token_len = index - start;
        next_char = text[index];

        if (zir_is_reserved_identifier(text + start, token_len, prev_char, next_char)) {
            continue;
        }

        {
            char token[64];
            const size_t copy_len = token_len < sizeof(token) - 1 ? token_len : sizeof(token) - 1;

            memcpy(token, text + start, copy_len);
            token[copy_len] = '\0';

            if (!zir_symbol_table_contains(defined, token)) {
                zir_verifier_set_result_at(
                    result,
                    ZIR_VERIFIER_USE_BEFORE_DEFINITION,
                    span,
                    "value '%s' is used before definition (%s)",
                    token,
                    context
                );
                return 0;
            }
        }
    }

    return 1;
}

static int zir_copy_trimmed_segment(char *dest, size_t capacity, const char *start, const char *end) {
    while (start < end && (*start == ' ' || *start == '\t')) {
        start += 1;
    }

    while (end > start && (end[-1] == ' ' || end[-1] == '\t')) {
        end -= 1;
    }

    if ((size_t)(end - start) >= capacity) {
        return 0;
    }

    memcpy(dest, start, (size_t)(end - start));
    dest[end - start] = '\0';
    return 1;
}

static int zir_verify_identifier_usage(
        const char *text,
        const zir_symbol_table *defined,
        const char *context,
        const zir_span *span,
        zir_verifier_result *result) {
    if (zir_starts_with(text, "set_field ")) {
        const char *args = text + strlen("set_field ");
        const char *first_comma = strchr(args, ',');
        const char *second_comma = first_comma != NULL ? strchr(first_comma + 1, ',') : NULL;
        char target[128];
        char value[128];

        if (first_comma != NULL && second_comma != NULL &&
                zir_copy_trimmed_segment(target, sizeof(target), args, first_comma) &&
                zir_copy_trimmed_segment(value, sizeof(value), second_comma + 1, args + strlen(args))) {
            return zir_verify_identifier_usage_tokens(target, defined, context, span, result) &&
                   zir_verify_identifier_usage_tokens(value, defined, context, span, result);
        }
    }

    if (zir_starts_with(text, "get_field ")) {
        const char *args = text + strlen("get_field ");
        const char *first_comma = strchr(args, ',');
        char target[128];

        if (first_comma != NULL &&
                zir_copy_trimmed_segment(target, sizeof(target), args, first_comma)) {
            return zir_verify_identifier_usage_tokens(target, defined, context, span, result);
        }
    }

    return zir_verify_identifier_usage_tokens(text, defined, context, span, result);
}

static int zir_verify_named_reference(
        const char *name,
        const zir_symbol_table *defined,
        const char *context,
        const zir_span *span,
        zir_verifier_result *result) {
    if (name == NULL || name[0] == '\0') {
        zir_verifier_set_result_at(
            result,
            ZIR_VERIFIER_INVALID_INPUT,
            span,
            "missing value reference (%s)",
            context
        );
        return 0;
    }

    if (!zir_symbol_table_contains(defined, name)) {
        zir_verifier_set_result_at(
            result,
            ZIR_VERIFIER_USE_BEFORE_DEFINITION,
            span,
            "value '%s' is used before definition (%s)",
            name,
            context
        );
        return 0;
    }

    return 1;
}

static int zir_verify_metadata_text(
        const char *text,
        const char *context,
        const zir_span *span,
        zir_verifier_result *result) {
    if (text == NULL || text[0] == '\0') {
        return 1;
    }

    if (zir_contains_target_leak(text)) {
        zir_verifier_set_result_at(
            result,
            ZIR_VERIFIER_INVALID_TARGET_LEAK,
            span,
            "target-specific names and legacy runtime helpers cannot appear in ZIR (%s)",
            context
        );
        return 0;
    }

    return 1;
}

static int zir_verify_expr(
        const zir_expr *expr,
        const zir_symbol_table *defined,
        const char *context,
        const zir_span *fallback_span,
        zir_verifier_result *result) {
    const zir_span *span;

    if (expr == NULL) return 1;
    span = zir_prefer_span(&expr->span, fallback_span);

    switch (expr->kind) {
        case ZIR_EXPR_NAME:
            return zir_verify_named_reference(expr->as.text.text, defined, context, span, result);

        case ZIR_EXPR_INT:
        case ZIR_EXPR_FLOAT:
        case ZIR_EXPR_BOOL:
        case ZIR_EXPR_STRING:
        case ZIR_EXPR_BYTES:
            return 1;

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
        case ZIR_EXPR_OPTIONAL_VALUE:
            return zir_verify_expr(expr->as.single.value, defined, context, span, result);

        case ZIR_EXPR_UNARY:
            return zir_verify_metadata_text(expr->as.unary.op_name, context, span, result) &&
                   zir_verify_expr(expr->as.unary.operand, defined, context, span, result);

        case ZIR_EXPR_BINARY:
            return zir_verify_metadata_text(expr->as.binary.op_name, context, span, result) &&
                   zir_verify_expr(expr->as.binary.left, defined, context, span, result) &&
                   zir_verify_expr(expr->as.binary.right, defined, context, span, result);

        case ZIR_EXPR_CALL_DIRECT:
        case ZIR_EXPR_CALL_EXTERN:
        case ZIR_EXPR_CALL_RUNTIME_INTRINSIC: {
            size_t i;
            if (!zir_verify_metadata_text(expr->as.call.callee_name, context, span, result)) return 0;
            for (i = 0; i < expr->as.call.args.count; i++) {
                if (!zir_verify_expr(expr->as.call.args.items[i], defined, context, span, result)) return 0;
            }
            return 1;
        }

        case ZIR_EXPR_CALL_DYN: {
            size_t i;
            if (!zir_verify_expr(expr->as.dyn_call.receiver, defined, context, span, result)) return 0;
            if (!zir_verify_metadata_text(expr->as.dyn_call.method_name, context, span, result)) return 0;
            if (!zir_verify_metadata_text(expr->as.dyn_call.trait_name, context, span, result)) return 0;
            for (i = 0; i < expr->as.dyn_call.args.count; i++) {
                if (!zir_verify_expr(expr->as.dyn_call.args.items[i], defined, context, span, result)) return 0;
            }
            return 1;
        }

        case ZIR_EXPR_FUNC_REF:
            return zir_verify_metadata_text(expr->as.func_ref.func_name, context, span, result) &&
                   zir_verify_type_name(expr->as.func_ref.callable_type_name, context, span, result);

        case ZIR_EXPR_CALL_INDIRECT: {
            size_t i;
            if (!zir_verify_expr(expr->as.call_indirect.callable, defined, context, span, result)) return 0;
            for (i = 0; i < expr->as.call_indirect.args.count; i++) {
                if (!zir_verify_expr(expr->as.call_indirect.args.items[i], defined, context, span, result)) return 0;
            }
            return 1;
        }

        case ZIR_EXPR_MAKE_STRUCT: {
            size_t i;
            if (!zir_verify_type_name(expr->as.make_struct.type_name, context, span, result)) return 0;
            for (i = 0; i < expr->as.make_struct.fields.count; i++) {
                if (!zir_verify_expr(expr->as.make_struct.fields.items[i].value, defined, context, span, result)) return 0;
            }
            return 1;
        }

        case ZIR_EXPR_MAKE_LIST: {
            size_t i;
            if (!zir_verify_type_name(expr->as.make_list.item_type_name, context, span, result)) return 0;
            for (i = 0; i < expr->as.make_list.items.count; i++) {
                if (!zir_verify_expr(expr->as.make_list.items.items[i], defined, context, span, result)) return 0;
            }
            return 1;
        }

        case ZIR_EXPR_MAKE_MAP: {
            size_t i;
            if (!zir_verify_type_name(expr->as.make_map.key_type_name, context, span, result)) return 0;
            if (!zir_verify_type_name(expr->as.make_map.value_type_name, context, span, result)) return 0;
            for (i = 0; i < expr->as.make_map.entries.count; i++) {
                if (!zir_verify_expr(expr->as.make_map.entries.items[i].key, defined, context, span, result)) return 0;
                if (!zir_verify_expr(expr->as.make_map.entries.items[i].value, defined, context, span, result)) return 0;
            }
            return 1;
        }

        case ZIR_EXPR_GET_FIELD:
            return zir_verify_expr(expr->as.field.object, defined, context, span, result);

        case ZIR_EXPR_SET_FIELD:
            return zir_verify_expr(expr->as.field.object, defined, context, span, result) &&
                   zir_verify_expr(expr->as.field.value, defined, context, span, result);

        case ZIR_EXPR_INDEX_SEQ:
        case ZIR_EXPR_COALESCE:
        case ZIR_EXPR_LIST_PUSH:
            return zir_verify_expr(expr->as.sequence.first, defined, context, span, result) &&
                   zir_verify_expr(expr->as.sequence.second, defined, context, span, result);

        case ZIR_EXPR_SLICE_SEQ:
        case ZIR_EXPR_LIST_SET:
        case ZIR_EXPR_MAP_SET:
            return zir_verify_expr(expr->as.sequence.first, defined, context, span, result) &&
                   zir_verify_expr(expr->as.sequence.second, defined, context, span, result) &&
                   zir_verify_expr(expr->as.sequence.third, defined, context, span, result);

        case ZIR_EXPR_OPTIONAL_EMPTY:
            return zir_verify_type_name(expr->as.type_only.type_name, context, span, result);

        default:
            zir_verifier_set_result_at(
                result,
                ZIR_VERIFIER_INVALID_INPUT,
                span,
                "unknown expression kind (%s)",
                context
            );
            return 0;
    }
}

static int zir_verify_instruction(
        const zir_instruction *instruction,
        const zir_symbol_table *defined,
        const char *function_name,
        const char *block_label,
        size_t instruction_index,
        const zir_span *fallback_span,
        zir_verifier_result *result) {
    char context[128];
    const zir_span *span = zir_prefer_span(&instruction->span, fallback_span);

    snprintf(
        context,
        sizeof(context),
        "func %s block %s instruction %zu",
        function_name,
        block_label,
        instruction_index
    );

    switch (instruction->kind) {
        case ZIR_INSTR_ASSIGN:
            if (!zir_verify_type_name(instruction->type_name, context, span, result)) {
                return 0;
            }

            if (instruction->expr != NULL) {
                if (!zir_verify_expr(instruction->expr, defined, context, span, result)) {
                    return 0;
                }
            } else if (!zir_verify_identifier_usage(instruction->expr_text, defined, context, span, result)) {
                return 0;
            }

            return 1;

        case ZIR_INSTR_EFFECT:
            if (instruction->expr != NULL) {
                return zir_verify_expr(instruction->expr, defined, context, span, result);
            }
            return zir_verify_identifier_usage(instruction->expr_text, defined, context, span, result);

        case ZIR_INSTR_CHECK_CONTRACT:
            if (instruction->expr != NULL) {
                return zir_verify_expr(instruction->expr, defined, context, span, result);
            }
            return zir_verify_identifier_usage(instruction->expr_text, defined, context, span, result);

        default:
            zir_verifier_set_result_at(
                result,
                ZIR_VERIFIER_INVALID_INPUT,
                span,
                "unknown instruction kind in %s",
                context
            );
            return 0;
    }
}

static int zir_block_label_exists(const zir_function *function_decl, const char *label) {
    size_t block_index;

    if (function_decl == NULL || label == NULL) {
        return 0;
    }

    for (block_index = 0; block_index < function_decl->block_count; block_index += 1) {
        if (function_decl->blocks[block_index].label != NULL &&
                strcmp(function_decl->blocks[block_index].label, label) == 0) {
            return 1;
        }
    }

    return 0;
}

static int zir_verify_terminator(
        const zir_function *function_decl,
        const zir_block *block,
        const zir_symbol_table *defined,
        zir_verifier_result *result) {
    char context[128];
    const zir_span *span = zir_prefer_span(&block->terminator.span, zir_prefer_span(&block->span, &function_decl->span));

    snprintf(
        context,
        sizeof(context),
        "func %s block %s terminator",
        function_decl->name,
        block->label
    );

    switch (block->terminator.kind) {
        case ZIR_TERM_RETURN:
            if (strcmp(function_decl->return_type, "void") == 0) {
                if (block->terminator.value != NULL || !zir_text_is_blank(block->terminator.value_text)) {
                    zir_verifier_set_result_at(
                        result,
                        ZIR_VERIFIER_INVALID_RETURN,
                        span,
                        "void function cannot return a value (%s)",
                        context
                    );
                    return 0;
                }
                return 1;
            }

            if (block->terminator.value == NULL && zir_text_is_blank(block->terminator.value_text)) {
                zir_verifier_set_result_at(
                    result,
                    ZIR_VERIFIER_INVALID_RETURN,
                    span,
                    "non-void function must return a value (%s)",
                    context
                );
                return 0;
            }

            if (block->terminator.value != NULL) {
                return zir_verify_expr(block->terminator.value, defined, context, span, result);
            }

            return zir_verify_identifier_usage(block->terminator.value_text, defined, context, span, result);

        case ZIR_TERM_JUMP:
            if (!zir_block_label_exists(function_decl, block->terminator.target_label)) {
                zir_verifier_set_result_at(
                    result,
                    ZIR_VERIFIER_UNKNOWN_BLOCK,
                    span,
                    "terminator references unknown block '%s' (%s)",
                    block->terminator.target_label,
                    context
                );
                return 0;
            }
            return 1;

        case ZIR_TERM_BRANCH_IF:
            if (block->terminator.condition != NULL) {
                if (!zir_verify_expr(block->terminator.condition, defined, context, span, result)) {
                    return 0;
                }
            } else if (!zir_verify_identifier_usage(block->terminator.condition_text, defined, context, span, result)) {
                return 0;
            }

            if (!zir_block_label_exists(function_decl, block->terminator.then_label)) {
                zir_verifier_set_result_at(
                    result,
                    ZIR_VERIFIER_UNKNOWN_BLOCK,
                    span,
                    "terminator references unknown block '%s' (%s)",
                    block->terminator.then_label,
                    context
                );
                return 0;
            }

            if (!zir_block_label_exists(function_decl, block->terminator.else_label)) {
                zir_verifier_set_result_at(
                    result,
                    ZIR_VERIFIER_UNKNOWN_BLOCK,
                    span,
                    "terminator references unknown block '%s' (%s)",
                    block->terminator.else_label,
                    context
                );
                return 0;
            }
            return 1;

        case ZIR_TERM_PANIC:
        case ZIR_TERM_UNREACHABLE:
            return 1;

        default:
            zir_verifier_set_result_at(
                result,
                ZIR_VERIFIER_INVALID_TERMINATOR,
                span,
                "unknown terminator kind (%s)",
                context
            );
            return 0;
    }
}

static int zir_verify_function(const zir_module *module_decl, const zir_function *function_decl, zir_verifier_result *result) {
    size_t param_index;
    size_t block_index;
    const char **block_labels = NULL;
    zir_symbol_table labels;
    const char **defined_names = NULL;
    zir_symbol_table defined;
    size_t definition_capacity;

    if (!zir_verify_type_name(function_decl->return_type, function_decl->name, &function_decl->span, result)) {
        return 0;
    }

    block_labels = (const char **)calloc(function_decl->block_count > 0 ? function_decl->block_count : 1, sizeof(const char *));
    if (block_labels == NULL) {
        zir_verifier_set_result(result, ZIR_VERIFIER_INVALID_INPUT, "unable to allocate verifier state");
        return 0;
    }

    labels.names = block_labels;
    labels.count = 0;
    labels.capacity = function_decl->block_count;

    for (param_index = 0; param_index < function_decl->param_count; param_index += 1) {
        char context[128];
        const zir_span *span = zir_prefer_span(&function_decl->params[param_index].span, &function_decl->span);

        snprintf(context, sizeof(context), "func %s param %s", function_decl->name, function_decl->params[param_index].name);
        if (!zir_verify_type_name(function_decl->params[param_index].type_name, context, span, result)) {
            free(block_labels);
            return 0;
        }
    }

    for (block_index = 0; block_index < function_decl->block_count; block_index += 1) {
        const zir_block *block = &function_decl->blocks[block_index];
        const zir_span *span = zir_prefer_span(&block->span, &function_decl->span);

        if (zir_symbol_table_contains(&labels, block->label)) {
            zir_verifier_set_result_at(
                result,
                ZIR_VERIFIER_DUPLICATE_BLOCK_LABEL,
                span,
                "block label '%s' appears more than once in func %s",
                block->label,
                function_decl->name
            );
            free(block_labels);
            return 0;
        }
        if (!zir_symbol_table_add(&labels, block->label)) {
            zir_verifier_set_result_at(result, ZIR_VERIFIER_INVALID_INPUT, span, "unable to register block label");
            free(block_labels);
            return 0;
        }
    }

    definition_capacity = zir_count_function_definitions(module_decl, function_decl);
    if (definition_capacity == 0) {
        definition_capacity = 1;
    }

    defined_names = (const char **)calloc(definition_capacity, sizeof(const char *));
    if (defined_names == NULL) {
        zir_verifier_set_result(result, ZIR_VERIFIER_INVALID_INPUT, "unable to allocate verifier state");
        free(block_labels);
        return 0;
    }

    defined.names = defined_names;
    defined.count = 0;
    defined.capacity = definition_capacity;

    if (!zir_collect_function_definitions(module_decl, function_decl, &defined, &function_decl->span, result)) {
        free(defined_names);
        free(block_labels);
        return 0;
    }

    for (block_index = 0; block_index < function_decl->block_count; block_index += 1) {
        const zir_block *block = &function_decl->blocks[block_index];
        const zir_span *block_span = zir_prefer_span(&block->span, &function_decl->span);
        size_t instruction_index;

        for (instruction_index = 0; instruction_index < block->instruction_count; instruction_index += 1) {
            if (!zir_verify_instruction(
                    &block->instructions[instruction_index],
                    &defined,
                    function_decl->name,
                    block->label,
                    instruction_index,
                    block_span,
                    result)) {
                free(defined_names);
                free(block_labels);
                return 0;
            }
        }

        if (!zir_verify_terminator(function_decl, block, &defined, result)) {
            free(defined_names);
            free(block_labels);
            return 0;
        }
    }

    free(defined_names);
    free(block_labels);
    return 1;
}

int zir_verify_module(const zir_module *module_decl, zir_verifier_result *result) {
    size_t module_var_index;
    size_t struct_index;
    size_t enum_index;
    size_t function_index;

    zir_verifier_result_init(result);

    if (module_decl == NULL) {
        zir_verifier_set_result(result, ZIR_VERIFIER_INVALID_INPUT, "module pointer cannot be null");
        return 0;
    }

    for (struct_index = 0; struct_index < module_decl->struct_count; struct_index += 1) {
        const zir_struct_decl *struct_decl = &module_decl->structs[struct_index];
        size_t field_index;

        for (field_index = 0; field_index < struct_decl->field_count; field_index += 1) {
            const zir_field_decl *field = &struct_decl->fields[field_index];
            const zir_span *span = zir_prefer_span(&field->span, &struct_decl->span);
            char context[192];
            snprintf(context, sizeof(context), "struct %s field %s", zir_safe_text(struct_decl->name), zir_safe_text(field->name));
            if (!zir_verify_type_name(field->type_name, context, span, result)) {
                return 0;
            }
        }
    }

    for (enum_index = 0; enum_index < module_decl->enum_count; enum_index += 1) {
        const zir_enum_decl *enum_decl = &module_decl->enums[enum_index];
        size_t variant_index;

        for (variant_index = 0; variant_index < enum_decl->variant_count; variant_index += 1) {
            const zir_enum_variant_decl *variant = &enum_decl->variants[variant_index];
            size_t field_index;

            for (field_index = 0; field_index < variant->field_count; field_index += 1) {
                const zir_enum_variant_field_decl *field = &variant->fields[field_index];
                const zir_span *span = zir_prefer_span(&field->span, zir_prefer_span(&variant->span, &enum_decl->span));
                char context[256];
                snprintf(
                    context,
                    sizeof(context),
                    "enum %s variant %s field %s",
                    zir_safe_text(enum_decl->name),
                    zir_safe_text(variant->name),
                    zir_safe_text(field->name));
                if (!zir_verify_type_name(field->type_name, context, span, result)) {
                    return 0;
                }
            }
        }
    }

    if (module_decl->module_var_count > 0) {
        const char **module_var_names = (const char **)calloc(
            module_decl->module_var_count,
            sizeof(const char *));
        zir_symbol_table module_var_defined;

        if (module_var_names == NULL) {
            zir_verifier_set_result(result, ZIR_VERIFIER_INVALID_INPUT, "unable to allocate verifier state");
            return 0;
        }

        module_var_defined.names = module_var_names;
        module_var_defined.count = 0;
        module_var_defined.capacity = module_decl->module_var_count;

        for (module_var_index = 0; module_var_index < module_decl->module_var_count; module_var_index += 1) {
            const zir_module_var *module_var = &module_decl->module_vars[module_var_index];
            const zir_span *span = zir_prefer_span(&module_var->span, &module_decl->span);
            char context[192];

            snprintf(context, sizeof(context), "module var %s", zir_safe_text(module_var->name));
            if (!zir_verify_type_name(module_var->type_name, context, span, result)) {
                free(module_var_names);
                return 0;
            }

            if (!zir_symbol_table_add_unique(&module_var_defined, module_var->name)) {
                zir_verifier_set_result_at(result, ZIR_VERIFIER_INVALID_INPUT, span, "unable to register module variable");
                free(module_var_names);
                return 0;
            }
        }

        for (module_var_index = 0; module_var_index < module_decl->module_var_count; module_var_index += 1) {
            const zir_module_var *module_var = &module_decl->module_vars[module_var_index];
            const zir_span *span = zir_prefer_span(&module_var->span, &module_decl->span);
            char context[192];

            snprintf(context, sizeof(context), "module var %s init", zir_safe_text(module_var->name));
            if (module_var->init_expr != NULL) {
                if (!zir_verify_expr(module_var->init_expr, &module_var_defined, context, span, result)) {
                    free(module_var_names);
                    return 0;
                }
            } else if (!zir_verify_identifier_usage(module_var->init_expr_text, &module_var_defined, context, span, result)) {
                free(module_var_names);
                return 0;
            }
        }

        free(module_var_names);
    }

    for (function_index = 0; function_index < module_decl->function_count; function_index += 1) {
        if (!zir_verify_function(module_decl, &module_decl->functions[function_index], result)) {
            return 0;
        }
    }

    zir_verifier_result_init(result);
    return 1;
}
