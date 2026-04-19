#include "compiler/targets/c/legalization.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *c_legalize_safe_text(const char *text) {
    return text != NULL ? text : "";
}

static void c_legalize_set_result(c_legalize_result *result, c_legalize_error_code code, const char *format, ...) {
    va_list args;

    if (result == NULL) {
        return;
    }

    result->ok = (code == C_LEGALIZE_OK);
    result->code = code;
    result->message[0] = '\0';

    if (format == NULL || format[0] == '\0') {
        return;
    }

    va_start(args, format);
    vsnprintf(result->message, sizeof(result->message), format, args);
    va_end(args);
}

void c_legalize_result_init(c_legalize_result *result) {
    c_legalize_set_result(result, C_LEGALIZE_OK, "");
}

const char *c_legalize_error_code_name(c_legalize_error_code code) {
    switch (code) {
        case C_LEGALIZE_OK:
            return "c.legalize.ok";
        case C_LEGALIZE_INVALID_INPUT:
            return "c.legalize.invalid_input";
        case C_LEGALIZE_UNSUPPORTED_EXPR:
            return "c.legalize.unsupported_expr";
        case C_LEGALIZE_UNSUPPORTED_TYPE:
            return "c.legalize.unsupported_type";
        default:
            return "c.legalize.unknown_error";
    }
}

static int c_legalize_is_blank(const char *text) {
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

static int c_legalize_starts_with(const char *text, const char *prefix) {
    if (text == NULL || prefix == NULL) {
        return 0;
    }

    return strncmp(text, prefix, strlen(prefix)) == 0;
}

static int c_legalize_copy_trimmed(char *dest, size_t capacity, const char *text) {
    const char *start = text;
    const char *end;
    size_t length;

    if (capacity == 0) {
        return 0;
    }

    if (text == NULL) {
        dest[0] = '\0';
        return 1;
    }

    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') {
        start += 1;
    }

    end = start + strlen(start);
    while (end > start &&
            (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n' || end[-1] == '\r')) {
        end -= 1;
    }

    length = (size_t)(end - start);
    if (length + 1 > capacity) {
        return 0;
    }

    memcpy(dest, start, length);
    dest[length] = '\0';
    return 1;
}

static int c_legalize_copy_trimmed_segment(char *dest, size_t capacity, const char *start, const char *end) {
    while (start < end && (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')) {
        start += 1;
    }

    while (end > start &&
            (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n' || end[-1] == '\r')) {
        end -= 1;
    }

    if ((size_t)(end - start) + 1 > capacity) {
        return 0;
    }

    memcpy(dest, start, (size_t)(end - start));
    dest[end - start] = '\0';
    return 1;
}

static int c_legalize_split_two_operands(const char *text, char *left, size_t left_capacity, char *right, size_t right_capacity) {
    const char *comma = strchr(text, ',');

    if (comma == NULL) {
        return 0;
    }

    return c_legalize_copy_trimmed_segment(left, left_capacity, text, comma) &&
           c_legalize_copy_trimmed(right, right_capacity, comma + 1);
}

static int c_legalize_split_three_operands(
        const char *text,
        char *first,
        size_t first_capacity,
        char *second,
        size_t second_capacity,
        char *third,
        size_t third_capacity) {
    const char *comma1 = strchr(text, ',');
    const char *comma2;

    if (comma1 == NULL) {
        return 0;
    }

    comma2 = strchr(comma1 + 1, ',');
    if (comma2 == NULL) {
        return 0;
    }

    return c_legalize_copy_trimmed_segment(first, first_capacity, text, comma1) &&
           c_legalize_copy_trimmed_segment(second, second_capacity, comma1 + 1, comma2) &&
           c_legalize_copy_trimmed_segment(third, third_capacity, comma2 + 1, text + strlen(text));
}

static int c_legalize_is_identifier_start(char ch) {
    return ((ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            ch == '_');
}

static int c_legalize_is_identifier_char(char ch) {
    return c_legalize_is_identifier_start(ch) || (ch >= '0' && ch <= '9');
}

static int c_legalize_is_identifier_only(const char *text) {
    size_t index = 0;

    if (text == NULL || text[0] == '\0' || !c_legalize_is_identifier_start(text[0])) {
        return 0;
    }

    index = 1;
    while (text[index] != '\0') {
        if (!c_legalize_is_identifier_char(text[index])) {
            return 0;
        }
        index += 1;
    }

    return 1;
}

static const char *c_legalize_find_symbol_type(const zir_function *function_decl, const char *name) {
    size_t param_index;
    size_t block_index;

    if (function_decl == NULL || name == NULL) {
        return NULL;
    }

    for (param_index = 0; param_index < function_decl->param_count; param_index += 1) {
        if (strcmp(function_decl->params[param_index].name, name) == 0) {
            return function_decl->params[param_index].type_name;
        }
    }

    for (block_index = 0; block_index < function_decl->block_count; block_index += 1) {
        size_t instruction_index;
        const zir_block *block = &function_decl->blocks[block_index];

        for (instruction_index = 0; instruction_index < block->instruction_count; instruction_index += 1) {
            const zir_instruction *instruction = &block->instructions[instruction_index];

            if (instruction->kind == ZIR_INSTR_ASSIGN &&
                    instruction->dest_name != NULL &&
                    strcmp(instruction->dest_name, name) == 0) {
                return instruction->type_name;
            }
        }
    }

    return NULL;
}

static int c_legalize_resolve_sequence_type(const zir_function *function_decl, const char *expr_text, char *dest, size_t capacity) {
    char trimmed[256];
    const char *type_name;

    if (!c_legalize_copy_trimmed(trimmed, sizeof(trimmed), expr_text)) {
        return 0;
    }

    if (c_legalize_is_blank(trimmed)) {
        return 0;
    }

    if (strncmp(trimmed, "const \"", 7) == 0) {
        snprintf(dest, capacity, "text");
        return 1;
    }

    if (c_legalize_starts_with(trimmed, "make_list<int>")) {
        snprintf(dest, capacity, "list<int>");
        return 1;
    }

    if (c_legalize_starts_with(trimmed, "make_list<text>")) {
        snprintf(dest, capacity, "list<text>");
        return 1;
    }

    if (c_legalize_starts_with(trimmed, "make_map<text,text>")) {
        snprintf(dest, capacity, "map<text,text>");
        return 1;
    }

    if (!c_legalize_is_identifier_only(trimmed)) {
        return 0;
    }

    type_name = c_legalize_find_symbol_type(function_decl, trimmed);
    if (type_name == NULL) {
        return 0;
    }

    snprintf(dest, capacity, "%s", type_name);
    return 1;
}

static int c_legalize_expect_result_type(
        const char *op_name,
        const char *actual_type_name,
        const char *expected_type_name,
        c_legalize_result *result) {
    if (c_legalize_is_blank(expected_type_name) || strcmp(expected_type_name, actual_type_name) == 0) {
        return 1;
    }

    c_legalize_set_result(
        result,
        C_LEGALIZE_UNSUPPORTED_TYPE,
        "%s produces %s, but the expected result type is '%s'",
        op_name,
        actual_type_name,
        c_legalize_safe_text(expected_type_name)
    );
    return 0;
}

static int c_legalize_fill_expr(
        c_legalized_seq_expr *out,
        c_legalized_seq_kind kind,
        const char *runtime_name,
        const char *sequence_type_name,
        const char *sequence_expr,
        const char *arg1_expr,
        const char *arg2_expr) {
    if (out == NULL) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    out->kind = kind;
    snprintf(out->runtime_name, sizeof(out->runtime_name), "%s", c_legalize_safe_text(runtime_name));
    snprintf(out->sequence_type_name, sizeof(out->sequence_type_name), "%s", c_legalize_safe_text(sequence_type_name));
    snprintf(out->sequence_expr, sizeof(out->sequence_expr), "%s", c_legalize_safe_text(sequence_expr));
    snprintf(out->arg1_expr, sizeof(out->arg1_expr), "%s", c_legalize_safe_text(arg1_expr));
    snprintf(out->arg2_expr, sizeof(out->arg2_expr), "%s", c_legalize_safe_text(arg2_expr));
    return 1;
}

static int c_legalize_index_seq(
        const zir_function *function_decl,
        const char *expr_text,
        const char *expected_type_name,
        c_legalized_seq_expr *out,
        c_legalize_result *result) {
    char sequence_expr[128];
    char index_expr[64];
    char sequence_type[64];

    if (!c_legalize_split_two_operands(expr_text + 10, sequence_expr, sizeof(sequence_expr), index_expr, sizeof(index_expr))) {
        c_legalize_set_result(result, C_LEGALIZE_UNSUPPORTED_EXPR, "invalid index_seq expression '%s'", c_legalize_safe_text(expr_text));
        return 0;
    }

    if (!c_legalize_resolve_sequence_type(function_decl, sequence_expr, sequence_type, sizeof(sequence_type))) {
        c_legalize_set_result(result, C_LEGALIZE_UNSUPPORTED_EXPR, "unable to resolve sequence type in '%s'", c_legalize_safe_text(expr_text));
        return 0;
    }

    if (strcmp(sequence_type, "text") == 0) {
        if (!c_legalize_expect_result_type("index_seq", "text", expected_type_name, result)) {
            return 0;
        }

        c_legalize_fill_expr(out, C_LEGALIZED_SEQ_TEXT_INDEX, "zt_text_index", "text", sequence_expr, index_expr, "");
        return 1;
    }

    if (strcmp(sequence_type, "list<int>") == 0) {
        if (!c_legalize_expect_result_type("index_seq", "int", expected_type_name, result)) {
            return 0;
        }

        c_legalize_fill_expr(out, C_LEGALIZED_SEQ_LIST_I64_INDEX, "zt_list_i64_get", "list<int>", sequence_expr, index_expr, "");
        return 1;
    }

    if (strcmp(sequence_type, "list<text>") == 0) {
        if (!c_legalize_expect_result_type("index_seq", "text", expected_type_name, result)) {
            return 0;
        }

        c_legalize_fill_expr(out, C_LEGALIZED_SEQ_LIST_TEXT_INDEX, "zt_list_text_get", "list<text>", sequence_expr, index_expr, "");
        return 1;
    }

    if (strcmp(sequence_type, "map<text,text>") == 0) {
        if (!c_legalize_expect_result_type("index_seq", "text", expected_type_name, result)) {
            return 0;
        }

        c_legalize_fill_expr(out, C_LEGALIZED_SEQ_MAP_TEXT_TEXT_INDEX, "zt_map_text_text_get", "map<text,text>", sequence_expr, index_expr, "");
        return 1;
    }

    c_legalize_set_result(
        result,
        C_LEGALIZE_UNSUPPORTED_TYPE,
        "sequence type '%s' is not supported by the current C legalization for index_seq",
        sequence_type
    );
    return 0;
}

static int c_legalize_slice_seq(
        const zir_function *function_decl,
        const char *expr_text,
        const char *expected_type_name,
        c_legalized_seq_expr *out,
        c_legalize_result *result) {
    char sequence_expr[128];
    char start_expr[64];
    char end_expr[64];
    char sequence_type[64];

    if (!c_legalize_split_three_operands(expr_text + 10, sequence_expr, sizeof(sequence_expr), start_expr, sizeof(start_expr), end_expr, sizeof(end_expr))) {
        c_legalize_set_result(result, C_LEGALIZE_UNSUPPORTED_EXPR, "invalid slice_seq expression '%s'", c_legalize_safe_text(expr_text));
        return 0;
    }

    if (!c_legalize_resolve_sequence_type(function_decl, sequence_expr, sequence_type, sizeof(sequence_type))) {
        c_legalize_set_result(result, C_LEGALIZE_UNSUPPORTED_EXPR, "unable to resolve sequence type in '%s'", c_legalize_safe_text(expr_text));
        return 0;
    }

    if (strcmp(sequence_type, "text") == 0) {
        if (!c_legalize_expect_result_type("slice_seq", "text", expected_type_name, result)) {
            return 0;
        }

        c_legalize_fill_expr(out, C_LEGALIZED_SEQ_TEXT_SLICE, "zt_text_slice", "text", sequence_expr, start_expr, end_expr);
        return 1;
    }

    if (strcmp(sequence_type, "list<int>") == 0) {
        if (!c_legalize_expect_result_type("slice_seq", "list<int>", expected_type_name, result)) {
            return 0;
        }

        c_legalize_fill_expr(out, C_LEGALIZED_SEQ_LIST_I64_SLICE, "zt_list_i64_slice", "list<int>", sequence_expr, start_expr, end_expr);
        return 1;
    }

    if (strcmp(sequence_type, "list<text>") == 0) {
        if (!c_legalize_expect_result_type("slice_seq", "list<text>", expected_type_name, result)) {
            return 0;
        }

        c_legalize_fill_expr(out, C_LEGALIZED_SEQ_LIST_TEXT_SLICE, "zt_list_text_slice", "list<text>", sequence_expr, start_expr, end_expr);
        return 1;
    }

    c_legalize_set_result(
        result,
        C_LEGALIZE_UNSUPPORTED_TYPE,
        "sequence type '%s' is not supported by the current C legalization for slice_seq",
        sequence_type
    );
    return 0;
}

int c_legalize_seq_expr(
        const zir_function *function_decl,
        const char *expr_text,
        const char *expected_type_name,
        c_legalized_seq_expr *out,
        c_legalize_result *result) {
    char trimmed[256];

    c_legalize_result_init(result);

    if (out == NULL) {
        c_legalize_set_result(result, C_LEGALIZE_INVALID_INPUT, "legalized sequence output cannot be null");
        return 0;
    }

    memset(out, 0, sizeof(*out));

    if (!c_legalize_copy_trimmed(trimmed, sizeof(trimmed), expr_text)) {
        c_legalize_set_result(result, C_LEGALIZE_INVALID_INPUT, "sequence expression is too large for the current legalization subset");
        return 0;
    }

    if (c_legalize_starts_with(trimmed, "index_seq ")) {
        return c_legalize_index_seq(function_decl, trimmed, expected_type_name, out, result);
    }

    if (c_legalize_starts_with(trimmed, "slice_seq ")) {
        return c_legalize_slice_seq(function_decl, trimmed, expected_type_name, out, result);
    }

    c_legalize_set_result(
        result,
        C_LEGALIZE_UNSUPPORTED_EXPR,
        "expression '%s' is not a sequence operation supported by the current C legalization subset",
        c_legalize_safe_text(trimmed)
    );
    return 0;
}

int c_legalize_list_len_expr(
        const zir_function *function_decl,
        const char *expr_text,
        const char *expected_type_name,
        c_legalized_seq_expr *out,
        c_legalize_result *result) {
    char trimmed[256];
    char sequence_expr[128];
    char sequence_type[64];

    c_legalize_result_init(result);

    if (out == NULL) {
        c_legalize_set_result(result, C_LEGALIZE_INVALID_INPUT, "legalized list_len output cannot be null");
        return 0;
    }

    memset(out, 0, sizeof(*out));

    if (!c_legalize_copy_trimmed(trimmed, sizeof(trimmed), expr_text)) {
        c_legalize_set_result(result, C_LEGALIZE_INVALID_INPUT, "list_len expression is too large for the current legalization subset");
        return 0;
    }

    if (!c_legalize_starts_with(trimmed, "list_len ")) {
        c_legalize_set_result(
            result,
            C_LEGALIZE_UNSUPPORTED_EXPR,
            "expression '%s' is not list_len in the current C legalization subset",
            c_legalize_safe_text(trimmed)
        );
        return 0;
    }

    if (!c_legalize_copy_trimmed(sequence_expr, sizeof(sequence_expr), trimmed + 9)) {
        c_legalize_set_result(result, C_LEGALIZE_INVALID_INPUT, "list_len sequence expression is too large");
        return 0;
    }

    if (!c_legalize_resolve_sequence_type(function_decl, sequence_expr, sequence_type, sizeof(sequence_type))) {
        c_legalize_set_result(result, C_LEGALIZE_UNSUPPORTED_EXPR, "unable to resolve sequence type in '%s'", c_legalize_safe_text(trimmed));
        return 0;
    }

    if (strcmp(sequence_type, "list<int>") == 0) {
        if (!c_legalize_expect_result_type("list_len", "int", expected_type_name, result)) {
            return 0;
        }

        c_legalize_fill_expr(out, C_LEGALIZED_SEQ_LIST_I64_LEN, "zt_list_i64_len", "list<int>", sequence_expr, "", "");
        return 1;
    }

    if (strcmp(sequence_type, "list<text>") == 0) {
        if (!c_legalize_expect_result_type("list_len", "int", expected_type_name, result)) {
            return 0;
        }

        c_legalize_fill_expr(out, C_LEGALIZED_SEQ_LIST_TEXT_LEN, "zt_list_text_len", "list<text>", sequence_expr, "", "");
        return 1;
    }

    c_legalize_set_result(
        result,
        C_LEGALIZE_UNSUPPORTED_TYPE,
        "sequence type '%s' is not supported by the current C legalization for list_len",
        sequence_type
    );
    return 0;
}
static int c_legalize_render_zir_expr(const zir_expr *expr, char *dest, size_t capacity) {
    char *rendered;

    if (dest == NULL || capacity == 0 || expr == NULL) {
        return 0;
    }

    rendered = zir_expr_render_alloc(expr);
    if (rendered == NULL) {
        return 0;
    }

    if (strlen(rendered) + 1 > capacity) {
        free(rendered);
        return 0;
    }

    memcpy(dest, rendered, strlen(rendered) + 1);
    free(rendered);
    return 1;
}

static int c_legalize_resolve_zir_sequence_type(
        const zir_function *function_decl,
        const zir_expr *expr,
        char *dest,
        size_t capacity) {
    const char *type_name;

    if (expr == NULL || dest == NULL || capacity == 0) {
        return 0;
    }

    switch (expr->kind) {
        case ZIR_EXPR_NAME:
            type_name = c_legalize_find_symbol_type(function_decl, expr->as.text.text);
            if (type_name == NULL) return 0;
            snprintf(dest, capacity, "%s", type_name);
            return 1;

        case ZIR_EXPR_STRING:
            snprintf(dest, capacity, "text");
            return 1;

        case ZIR_EXPR_MAKE_LIST:
            snprintf(dest, capacity, "list<%s>", c_legalize_safe_text(expr->as.make_list.item_type_name));
            return 1;

        case ZIR_EXPR_MAKE_MAP:
            snprintf(
                dest,
                capacity,
                "map<%s,%s>",
                c_legalize_safe_text(expr->as.make_map.key_type_name),
                c_legalize_safe_text(expr->as.make_map.value_type_name)
            );
            return 1;

        default:
            return 0;
    }
}

int c_legalize_zir_seq_expr(
        const zir_function *function_decl,
        const zir_expr *expr,
        const char *expected_type_name,
        c_legalized_seq_expr *out,
        c_legalize_result *result) {
    char sequence_type[64];
    char sequence_expr[128];
    char arg1_expr[64];
    char arg2_expr[64];

    c_legalize_result_init(result);

    if (out == NULL) {
        c_legalize_set_result(result, C_LEGALIZE_INVALID_INPUT, "legalized sequence output cannot be null");
        return 0;
    }

    memset(out, 0, sizeof(*out));

    if (expr == NULL ||
            (expr->kind != ZIR_EXPR_INDEX_SEQ && expr->kind != ZIR_EXPR_SLICE_SEQ)) {
        c_legalize_set_result(result, C_LEGALIZE_UNSUPPORTED_EXPR, "structured expression is not index_seq or slice_seq");
        return 0;
    }

    if (!c_legalize_resolve_zir_sequence_type(function_decl, expr->as.sequence.first, sequence_type, sizeof(sequence_type)) ||
            !c_legalize_render_zir_expr(expr->as.sequence.first, sequence_expr, sizeof(sequence_expr)) ||
            !c_legalize_render_zir_expr(expr->as.sequence.second, arg1_expr, sizeof(arg1_expr))) {
        c_legalize_set_result(result, C_LEGALIZE_UNSUPPORTED_EXPR, "unable to legalize structured sequence operands");
        return 0;
    }

    if (expr->kind == ZIR_EXPR_SLICE_SEQ &&
            !c_legalize_render_zir_expr(expr->as.sequence.third, arg2_expr, sizeof(arg2_expr))) {
        c_legalize_set_result(result, C_LEGALIZE_UNSUPPORTED_EXPR, "unable to legalize structured slice end operand");
        return 0;
    }

    if (expr->kind == ZIR_EXPR_INDEX_SEQ) {
        if (strcmp(sequence_type, "text") == 0) {
            if (!c_legalize_expect_result_type("index_seq", "text", expected_type_name, result)) return 0;
            c_legalize_fill_expr(out, C_LEGALIZED_SEQ_TEXT_INDEX, "zt_text_index", "text", sequence_expr, arg1_expr, "");
            return 1;
        }

        if (strcmp(sequence_type, "list<int>") == 0) {
            if (!c_legalize_expect_result_type("index_seq", "int", expected_type_name, result)) return 0;
            c_legalize_fill_expr(out, C_LEGALIZED_SEQ_LIST_I64_INDEX, "zt_list_i64_get", "list<int>", sequence_expr, arg1_expr, "");
            return 1;
        }

        if (strcmp(sequence_type, "list<text>") == 0) {
            if (!c_legalize_expect_result_type("index_seq", "text", expected_type_name, result)) return 0;
            c_legalize_fill_expr(out, C_LEGALIZED_SEQ_LIST_TEXT_INDEX, "zt_list_text_get", "list<text>", sequence_expr, arg1_expr, "");
            return 1;
        }

        if (strcmp(sequence_type, "map<text,text>") == 0) {
            if (!c_legalize_expect_result_type("index_seq", "text", expected_type_name, result)) return 0;
            c_legalize_fill_expr(out, C_LEGALIZED_SEQ_MAP_TEXT_TEXT_INDEX, "zt_map_text_text_get", "map<text,text>", sequence_expr, arg1_expr, "");
            return 1;
        }
    }

    if (expr->kind == ZIR_EXPR_SLICE_SEQ) {
        if (strcmp(sequence_type, "text") == 0) {
            if (!c_legalize_expect_result_type("slice_seq", "text", expected_type_name, result)) return 0;
            c_legalize_fill_expr(out, C_LEGALIZED_SEQ_TEXT_SLICE, "zt_text_slice", "text", sequence_expr, arg1_expr, arg2_expr);
            return 1;
        }

        if (strcmp(sequence_type, "list<int>") == 0) {
            if (!c_legalize_expect_result_type("slice_seq", "list<int>", expected_type_name, result)) return 0;
            c_legalize_fill_expr(out, C_LEGALIZED_SEQ_LIST_I64_SLICE, "zt_list_i64_slice", "list<int>", sequence_expr, arg1_expr, arg2_expr);
            return 1;
        }

        if (strcmp(sequence_type, "list<text>") == 0) {
            if (!c_legalize_expect_result_type("slice_seq", "list<text>", expected_type_name, result)) return 0;
            c_legalize_fill_expr(out, C_LEGALIZED_SEQ_LIST_TEXT_SLICE, "zt_list_text_slice", "list<text>", sequence_expr, arg1_expr, arg2_expr);
            return 1;
        }
    }

    c_legalize_set_result(result, C_LEGALIZE_UNSUPPORTED_TYPE, "sequence type '%s' is not supported by structured legalization", sequence_type);
    return 0;
}

int c_legalize_zir_list_len_expr(
        const zir_function *function_decl,
        const zir_expr *expr,
        const char *expected_type_name,
        c_legalized_seq_expr *out,
        c_legalize_result *result) {
    char sequence_type[64];
    char sequence_expr[128];

    c_legalize_result_init(result);

    if (out == NULL) {
        c_legalize_set_result(result, C_LEGALIZE_INVALID_INPUT, "legalized list_len output cannot be null");
        return 0;
    }

    memset(out, 0, sizeof(*out));

    if (expr == NULL || expr->kind != ZIR_EXPR_LIST_LEN) {
        c_legalize_set_result(result, C_LEGALIZE_UNSUPPORTED_EXPR, "structured expression is not list_len");
        return 0;
    }

    if (!c_legalize_resolve_zir_sequence_type(function_decl, expr->as.single.value, sequence_type, sizeof(sequence_type)) ||
            !c_legalize_render_zir_expr(expr->as.single.value, sequence_expr, sizeof(sequence_expr))) {
        c_legalize_set_result(result, C_LEGALIZE_UNSUPPORTED_EXPR, "unable to legalize structured list_len operand");
        return 0;
    }

    if (strcmp(sequence_type, "list<int>") == 0) {
        if (!c_legalize_expect_result_type("list_len", "int", expected_type_name, result)) return 0;
        c_legalize_fill_expr(out, C_LEGALIZED_SEQ_LIST_I64_LEN, "zt_list_i64_len", "list<int>", sequence_expr, "", "");
        return 1;
    }

    if (strcmp(sequence_type, "list<text>") == 0) {
        if (!c_legalize_expect_result_type("list_len", "int", expected_type_name, result)) return 0;
        c_legalize_fill_expr(out, C_LEGALIZED_SEQ_LIST_TEXT_LEN, "zt_list_text_len", "list<text>", sequence_expr, "", "");
        return 1;
    }

    c_legalize_set_result(result, C_LEGALIZE_UNSUPPORTED_TYPE, "list_len is supported only for list<int> and list<text>, got '%s'", sequence_type);
    return 0;
}


