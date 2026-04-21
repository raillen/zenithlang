#include "compiler/targets/c/legalization.h"
#include "compiler/zir/model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_COUNT(items) (sizeof(items) / sizeof((items)[0]))

static void assert_true(const char *name, int condition) {
    if (!condition) {
        fprintf(stderr, "falha no teste %s\n", name);
        exit(1);
    }
}

static zir_param test_param(const char *name, const char *type_name) {
    return zir_make_param(name, type_name, NULL);
}

static void assert_ok(
        const char *name,
        const zir_function *function_decl,
        const char *expr_text,
        const char *expected_type_name,
        c_legalized_seq_kind expected_kind,
        const char *expected_runtime_name,
        const char *expected_sequence_type_name,
        const char *expected_sequence_expr,
        const char *expected_arg1_expr,
        const char *expected_arg2_expr) {
    c_legalized_seq_expr legalized;
    c_legalize_result result;

    c_legalize_result_init(&result);
    if (!c_legalize_seq_expr(function_decl, expr_text, expected_type_name, &legalized, &result)) {
        fprintf(stderr, "falha ao legalizar %s\n", name);
        fprintf(stderr, "codigo: %s\n", c_legalize_error_code_name(result.code));
        fprintf(stderr, "mensagem: %s\n", result.message);
        exit(1);
    }

    assert_true(name, legalized.kind == expected_kind);
    assert_true(name, strcmp(legalized.runtime_name, expected_runtime_name) == 0);
    assert_true(name, strcmp(legalized.sequence_type_name, expected_sequence_type_name) == 0);
    assert_true(name, strcmp(legalized.sequence_expr, expected_sequence_expr) == 0);
    assert_true(name, strcmp(legalized.arg1_expr, expected_arg1_expr) == 0);
    assert_true(name, strcmp(legalized.arg2_expr, expected_arg2_expr) == 0);
}

static void assert_len_ok(
        const char *name,
        const zir_function *function_decl,
        const char *expr_text,
        const char *expected_type_name,
        c_legalized_seq_kind expected_kind,
        const char *expected_runtime_name,
        const char *expected_sequence_type_name,
        const char *expected_sequence_expr) {
    c_legalized_seq_expr legalized;
    c_legalize_result result;

    c_legalize_result_init(&result);
    if (!c_legalize_list_len_expr(function_decl, expr_text, expected_type_name, &legalized, &result)) {
        fprintf(stderr, "falha ao legalizar %s\n", name);
        fprintf(stderr, "codigo: %s\n", c_legalize_error_code_name(result.code));
        fprintf(stderr, "mensagem: %s\n", result.message);
        exit(1);
    }

    assert_true(name, legalized.kind == expected_kind);
    assert_true(name, strcmp(legalized.runtime_name, expected_runtime_name) == 0);
    assert_true(name, strcmp(legalized.sequence_type_name, expected_sequence_type_name) == 0);
    assert_true(name, strcmp(legalized.sequence_expr, expected_sequence_expr) == 0);
    assert_true(name, legalized.arg1_expr[0] == '\0');
    assert_true(name, legalized.arg2_expr[0] == '\0');
}

static void assert_fails(
        const char *name,
        const zir_function *function_decl,
        const char *expr_text,
        const char *expected_type_name,
        c_legalize_error_code expected_code,
        const char *expected_message_fragment) {
    c_legalized_seq_expr legalized;
    c_legalize_result result;

    c_legalize_result_init(&result);
    if (c_legalize_seq_expr(function_decl, expr_text, expected_type_name, &legalized, &result)) {
        fprintf(stderr, "era esperado erro em %s, mas legalizacao foi aceita\n", name);
        exit(1);
    }

    assert_true(name, result.code == expected_code);
    assert_true(name, strstr(result.message, expected_message_fragment) != NULL);
}

static void assert_len_fails(
        const char *name,
        const zir_function *function_decl,
        const char *expr_text,
        const char *expected_type_name,
        c_legalize_error_code expected_code,
        const char *expected_message_fragment) {
    c_legalized_seq_expr legalized;
    c_legalize_result result;

    c_legalize_result_init(&result);
    if (c_legalize_list_len_expr(function_decl, expr_text, expected_type_name, &legalized, &result)) {
        fprintf(stderr, "era esperado erro em %s, mas list_len foi legalizado\n", name);
        exit(1);
    }

    assert_true(name, result.code == expected_code);
    assert_true(name, strstr(result.message, expected_message_fragment) != NULL);
}

static void test_text_index_seq(void) {
    const zir_param params[] = {
        test_param("name", "text"),
    };
    const zir_function function_decl = zir_make_function("char_at", params, ARRAY_COUNT(params), "text", NULL, 0);

    assert_ok(
        "text_index_seq",
        &function_decl,
        "index_seq name, 2",
        "text",
        C_LEGALIZED_SEQ_TEXT_INDEX,
        "zt_text_index",
        "text",
        "name",
        "2",
        ""
    );
}

static void test_text_slice_seq(void) {
    const zir_param params[] = {
        test_param("name", "text"),
    };
    const zir_function function_decl = zir_make_function("slice", params, ARRAY_COUNT(params), "text", NULL, 0);

    assert_ok(
        "text_slice_seq",
        &function_decl,
        "slice_seq name, 2, 4",
        "text",
        C_LEGALIZED_SEQ_TEXT_SLICE,
        "zt_text_slice",
        "text",
        "name",
        "2",
        "4"
    );
}

static void test_list_index_seq(void) {
    const zir_param params[] = {
        test_param("xs", "list<int>"),
    };
    const zir_function function_decl = zir_make_function("first", params, ARRAY_COUNT(params), "int", NULL, 0);

    assert_ok(
        "list_index_seq",
        &function_decl,
        "index_seq xs, 1",
        "int",
        C_LEGALIZED_SEQ_LIST_I64_INDEX,
        "zt_list_i64_get",
        "list<int>",
        "xs",
        "1",
        ""
    );
}

static void test_list_slice_seq(void) {
    const zir_param params[] = {
        test_param("xs", "list<int>"),
    };
    const zir_function function_decl = zir_make_function("window", params, ARRAY_COUNT(params), "list<int>", NULL, 0);

    assert_ok(
        "list_slice_seq",
        &function_decl,
        "slice_seq xs, 2, 4",
        "list<int>",
        C_LEGALIZED_SEQ_LIST_I64_SLICE,
        "zt_list_i64_slice",
        "list<int>",
        "xs",
        "2",
        "4"
    );
}

static void test_list_text_index_seq(void) {
    const zir_param params[] = {
        test_param("xs", "list<text>"),
    };
    const zir_function function_decl = zir_make_function("first_text", params, ARRAY_COUNT(params), "text", NULL, 0);

    assert_ok(
        "list_text_index_seq",
        &function_decl,
        "index_seq xs, 1",
        "text",
        C_LEGALIZED_SEQ_LIST_TEXT_INDEX,
        "zt_list_text_get",
        "list<text>",
        "xs",
        "1",
        ""
    );
}

static void test_list_text_slice_seq(void) {
    const zir_param params[] = {
        test_param("xs", "list<text>"),
    };
    const zir_function function_decl = zir_make_function("window_text", params, ARRAY_COUNT(params), "list<text>", NULL, 0);

    assert_ok(
        "list_text_slice_seq",
        &function_decl,
        "slice_seq xs, 2, 4",
        "list<text>",
        C_LEGALIZED_SEQ_LIST_TEXT_SLICE,
        "zt_list_text_slice",
        "list<text>",
        "xs",
        "2",
        "4"
    );
}

static void test_map_text_text_index_seq(void) {
    const zir_param params[] = {
        test_param("cfg", "map<text,text>"),
        test_param("key", "text"),
    };
    const zir_function function_decl = zir_make_function("lookup", params, ARRAY_COUNT(params), "text", NULL, 0);

    assert_ok(
        "map_text_text_index_seq",
        &function_decl,
        "index_seq cfg, key",
        "text",
        C_LEGALIZED_SEQ_MAP_TEXT_TEXT_INDEX,
        "zt_map_text_text_get",
        "map<text,text>",
        "cfg",
        "key",
        ""
    );
}

static void test_map_int_text_index_seq(void) {
    const zir_param params[] = {
        test_param("cfg", "map<int,text>"),
        test_param("key", "int"),
    };
    const zir_function function_decl = zir_make_function("lookup_int_key", params, ARRAY_COUNT(params), "text", NULL, 0);

    assert_ok(
        "map_int_text_index_seq",
        &function_decl,
        "index_seq cfg, key",
        "text",
        C_LEGALIZED_SEQ_MAP_INDEX,
        "zt_map_generated_map_int_text__get",
        "map<int,text>",
        "cfg",
        "key",
        ""
    );
}

static void test_list_len(void) {
    const zir_param params[] = {
        test_param("xs", "list<int>"),
    };
    const zir_function function_decl = zir_make_function("count", params, ARRAY_COUNT(params), "int", NULL, 0);

    assert_len_ok(
        "list_len",
        &function_decl,
        "list_len xs",
        "int",
        C_LEGALIZED_SEQ_LIST_I64_LEN,
        "zt_list_i64_len",
        "list<int>",
        "xs"
    );
}

static void test_list_text_len(void) {
    const zir_param params[] = {
        test_param("xs", "list<text>"),
    };
    const zir_function function_decl = zir_make_function("count_text", params, ARRAY_COUNT(params), "int", NULL, 0);

    assert_len_ok(
        "list_text_len",
        &function_decl,
        "list_len xs",
        "int",
        C_LEGALIZED_SEQ_LIST_TEXT_LEN,
        "zt_list_text_len",
        "list<text>",
        "xs"
    );
}

static void test_map_text_text_len(void) {
    const zir_param params[] = {
        test_param("cfg", "map<text,text>"),
    };
    const zir_function function_decl = zir_make_function("count_map", params, ARRAY_COUNT(params), "int", NULL, 0);

    assert_len_ok(
        "map_text_text_len",
        &function_decl,
        "map_len cfg",
        "int",
        C_LEGALIZED_SEQ_MAP_LEN,
        "zt_map_text_text_len",
        "map<text,text>",
        "cfg"
    );
}

static void test_map_int_text_len(void) {
    const zir_param params[] = {
        test_param("cfg", "map<int,text>"),
    };
    const zir_function function_decl = zir_make_function("count_map_int_key", params, ARRAY_COUNT(params), "int", NULL, 0);

    assert_len_ok(
        "map_int_text_len",
        &function_decl,
        "map_len cfg",
        "int",
        C_LEGALIZED_SEQ_MAP_LEN,
        "zt_map_generated_map_int_text__len",
        "map<int,text>",
        "cfg"
    );
}


static void test_structured_index_seq(void) {
    const zir_param params[] = {
        test_param("xs", "list<int>"),
    };
    const zir_function function_decl = zir_make_function("structured_index", params, ARRAY_COUNT(params), "int", NULL, 0);
    zir_expr *expr = zir_expr_make_index_seq(zir_expr_make_name("xs"), zir_expr_make_int("1"));
    c_legalized_seq_expr legalized;
    c_legalize_result result;

    c_legalize_result_init(&result);
    if (!c_legalize_zir_seq_expr(&function_decl, expr, "int", &legalized, &result)) {
        fprintf(stderr, "falha ao legalizar structured_index_seq\n");
        fprintf(stderr, "codigo: %s\n", c_legalize_error_code_name(result.code));
        fprintf(stderr, "mensagem: %s\n", result.message);
        exit(1);
    }

    assert_true("structured_index_kind", legalized.kind == C_LEGALIZED_SEQ_LIST_I64_INDEX);
    assert_true("structured_index_runtime", strcmp(legalized.runtime_name, "zt_list_i64_get") == 0);
    assert_true("structured_index_sequence", strcmp(legalized.sequence_expr, "xs") == 0);
    assert_true("structured_index_arg", strcmp(legalized.arg1_expr, "1") == 0);
}

static void test_structured_map_int_text_index_seq(void) {
    const zir_param params[] = {
        test_param("cfg", "map<int,text>"),
        test_param("key", "int"),
    };
    const zir_function function_decl = zir_make_function("structured_map_index", params, ARRAY_COUNT(params), "text", NULL, 0);
    zir_expr *expr = zir_expr_make_index_seq(zir_expr_make_name("cfg"), zir_expr_make_name("key"));
    c_legalized_seq_expr legalized;
    c_legalize_result result;

    c_legalize_result_init(&result);
    if (!c_legalize_zir_seq_expr(&function_decl, expr, "text", &legalized, &result)) {
        fprintf(stderr, "falha ao legalizar structured_map_int_text_index_seq\n");
        fprintf(stderr, "codigo: %s\n", c_legalize_error_code_name(result.code));
        fprintf(stderr, "mensagem: %s\n", result.message);
        exit(1);
    }

    assert_true("structured_map_index_kind", legalized.kind == C_LEGALIZED_SEQ_MAP_INDEX);
    assert_true("structured_map_index_runtime", strcmp(legalized.runtime_name, "zt_map_generated_map_int_text__get") == 0);
    assert_true("structured_map_index_sequence_type", strcmp(legalized.sequence_type_name, "map<int,text>") == 0);
    assert_true("structured_map_index_sequence", strcmp(legalized.sequence_expr, "cfg") == 0);
    assert_true("structured_map_index_arg", strcmp(legalized.arg1_expr, "key") == 0);
}

static void test_structured_list_len(void) {
    const zir_param params[] = {
        test_param("xs", "list<text>"),
    };
    const zir_function function_decl = zir_make_function("structured_len", params, ARRAY_COUNT(params), "int", NULL, 0);
    zir_expr *expr = zir_expr_make_list_len(zir_expr_make_name("xs"));
    c_legalized_seq_expr legalized;
    c_legalize_result result;

    c_legalize_result_init(&result);
    if (!c_legalize_zir_list_len_expr(&function_decl, expr, "int", &legalized, &result)) {
        fprintf(stderr, "falha ao legalizar structured_list_len\n");
        fprintf(stderr, "codigo: %s\n", c_legalize_error_code_name(result.code));
        fprintf(stderr, "mensagem: %s\n", result.message);
        exit(1);
    }

    assert_true("structured_len_kind", legalized.kind == C_LEGALIZED_SEQ_LIST_TEXT_LEN);
    assert_true("structured_len_runtime", strcmp(legalized.runtime_name, "zt_list_text_len") == 0);
    assert_true("structured_len_sequence", strcmp(legalized.sequence_expr, "xs") == 0);
}

static void test_structured_map_int_text_len(void) {
    const zir_param params[] = {
        test_param("cfg", "map<int,text>"),
    };
    const zir_function function_decl = zir_make_function("structured_map_len", params, ARRAY_COUNT(params), "int", NULL, 0);
    zir_expr *expr = zir_expr_make_map_len(zir_expr_make_name("cfg"));
    c_legalized_seq_expr legalized;
    c_legalize_result result;

    c_legalize_result_init(&result);
    if (!c_legalize_zir_list_len_expr(&function_decl, expr, "int", &legalized, &result)) {
        fprintf(stderr, "falha ao legalizar structured_map_int_text_len\n");
        fprintf(stderr, "codigo: %s\n", c_legalize_error_code_name(result.code));
        fprintf(stderr, "mensagem: %s\n", result.message);
        exit(1);
    }

    assert_true("structured_map_len_kind", legalized.kind == C_LEGALIZED_SEQ_MAP_LEN);
    assert_true("structured_map_len_runtime", strcmp(legalized.runtime_name, "zt_map_generated_map_int_text__len") == 0);
    assert_true("structured_map_len_sequence_type", strcmp(legalized.sequence_type_name, "map<int,text>") == 0);
    assert_true("structured_map_len_sequence", strcmp(legalized.sequence_expr, "cfg") == 0);
}

static void test_text_index_seq_rejects_wrong_result_type(void) {
    const zir_param params[] = {
        test_param("name", "text"),
    };
    const zir_function function_decl = zir_make_function("bad", params, ARRAY_COUNT(params), "int", NULL, 0);

    assert_fails(
        "text_index_seq_wrong_result_type",
        &function_decl,
        "index_seq name, 1",
        "int",
        C_LEGALIZE_UNSUPPORTED_TYPE,
        "expected result type is 'int'"
    );
}

static void test_list_len_rejects_text(void) {
    const zir_param params[] = {
        test_param("name", "text"),
    };
    const zir_function function_decl = zir_make_function("bad_len", params, ARRAY_COUNT(params), "int", NULL, 0);

    assert_len_fails(
        "list_len_rejects_text",
        &function_decl,
        "list_len name",
        "int",
        C_LEGALIZE_UNSUPPORTED_TYPE,
        "text"
    );
}

static void test_unknown_sequence_type_fails(void) {
    const zir_param params[] = {
        test_param("value", "Optional<int>"),
    };
    const zir_function function_decl = zir_make_function("bad_seq", params, ARRAY_COUNT(params), "int", NULL, 0);

    assert_fails(
        "unknown_sequence_type",
        &function_decl,
        "index_seq value, 1",
        "int",
        C_LEGALIZE_UNSUPPORTED_TYPE,
        "Optional<int>"
    );
}

int main(void) {
    test_text_index_seq();
    test_text_slice_seq();
    test_list_index_seq();
    test_list_slice_seq();
    test_list_text_index_seq();
    test_list_text_slice_seq();
    test_map_text_text_index_seq();
    test_map_int_text_index_seq();
    test_list_len();
    test_list_text_len();
    test_map_text_text_len();
    test_map_int_text_len();
    test_structured_index_seq();
    test_structured_map_int_text_index_seq();
    test_structured_list_len();
    test_structured_map_int_text_len();
    test_text_index_seq_rejects_wrong_result_type();
    test_list_len_rejects_text();
    test_unknown_sequence_type_fails();
    puts("C legalization tests OK");
    return 0;
}
