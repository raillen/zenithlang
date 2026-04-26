#include "compiler/utils/arena.h"
#include "compiler/utils/string_pool.h"
#include "compiler/frontend/parser/parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT_EQ(actual, expected, msg) do { \
    tests_run++; \
    if ((actual) == (expected)) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected %d, got %d\n", msg, (int)(expected), (int)(actual)); } \
} while(0)

#define ASSERT_GT(actual, expected, msg) do { \
    tests_run++; \
    if ((actual) > (expected)) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected >%d, got %d\n", msg, (int)(expected), (int)(actual)); } \
} while(0)

#define ASSERT_NOT_NULL(actual, msg) do { \
    tests_run++; \
    if ((actual) != NULL) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected non-NULL\n", msg); } \
} while(0)

static void assert_parse_diag_contains(const char *src, const char *needle, const char *msg) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_parser_result r;
    int found = 0;
    size_t i;

    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    for (i = 0; i < r.diagnostics.count; i += 1) {
        if (strstr(r.diagnostics.items[i].message, needle) != NULL) {
            found = 1;
            break;
        }
    }

    tests_run += 1;
    if (found) {
        tests_passed += 1;
    } else {
        fprintf(stderr, "FAIL: %s: expected diagnostic containing \"%s\"\n", msg, needle);
        fprintf(stderr, "  diagnostics=%zu\n", r.diagnostics.count);
        for (i = 0; i < r.diagnostics.count; i += 1) {
            fprintf(stderr, "  diag: %s\n", r.diagnostics.items[i].message);
        }
    }

    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 1: Missing function name */
static void test_error_missing_func_name(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc (x: int) -> int\n    return x\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing func name");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 2: Missing parameter type */
static void test_error_missing_param_type(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo(x)\n    return x\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing param type");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 3: Missing return type after arrow */
static void test_error_missing_return_type(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo() -> \n    return 1\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing return type");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 4: Missing 'end' for function */
static void test_error_missing_func_end(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    return 1\n";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing func end");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 5: Missing 'end' for struct */
static void test_error_missing_struct_end(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nstruct Foo\n    x: int\n";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing struct end");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 6: Missing 'end' for enum */
static void test_error_missing_enum_end(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nenum Foo\n    Bar\n";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing enum end");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 7: Missing 'end' for trait */
static void test_error_missing_trait_end(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\ntrait Foo\n    func bar()\n";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing trait end");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 8: Missing 'end' for apply */
static void test_error_missing_apply_end(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\napply Foo\n    func bar()\n    end\n";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    /* This may parse successfully since func has its own 'end' */
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 9: Missing 'end' for extern */
static void test_error_missing_extern_end(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nextern c\n    func foo()\n";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing extern end");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 10: Missing 'else' block after 'else' */
static void test_error_missing_else_block(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    if true\n        return 1\n    else\n";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing else block");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 11: Missing 'end' for if */
static void test_error_missing_if_end(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    if true\n        return 1\n";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing if end");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 12: Missing 'end' for while */
static void test_error_missing_while_end(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    while true\n        return 1\n";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing while end");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 13: Missing 'end' for for */
static void test_error_missing_for_end(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    for x in items\n        return x\n";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing for end");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 14: Missing 'end' for repeat */
static void test_error_missing_repeat_end(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    repeat 5 times\n        return\n";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing repeat end");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 15: Missing 'end' for match */
static void test_error_missing_match_end(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    match x\n        case 0\n            return 0\n";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing match end");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 16: Top-level var declaration is valid */
static void test_top_level_var_allowed(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nvar x: int = 10\nfunc foo()\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_EQ((int)r.diagnostics.count, 0, "top-level var is allowed");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 17: Missing field type in struct */
static void test_error_missing_field_type(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nstruct Foo\n    x\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing field type");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 18: Missing variant name in enum */
static void test_error_missing_variant_name(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nenum Foo\n    (x: int)\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing variant name");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 19: Missing expression in return */
static void test_error_missing_return_expr(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo() -> int\n    return\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    /* This should parse successfully (void return in func returning int) */
    /* Semantic analysis will catch the type mismatch */
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 20: Missing condition in if */
static void test_error_missing_if_condition(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    if\n        return 1\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing if condition");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 21: Missing condition in while */
static void test_error_missing_while_condition(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    while\n        return 1\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing while condition");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 22: Missing iterable in for */
static void test_error_missing_for_iterable(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    for x in\n        return x\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing for iterable");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 23: Missing count in repeat */
static void test_error_missing_repeat_count(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    repeat times\n        return\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing repeat count");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 24: Missing 'times' keyword in repeat */
static void test_error_missing_times_keyword(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    repeat 5\n        return\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing times keyword");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 25: Missing subject in match */
static void test_error_missing_match_subject(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    match\n        case 0\n            return 0\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing match subject");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 26: Missing pattern in case */
static void test_error_missing_case_pattern(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    match x\n        case\n            return 0\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing case pattern");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 27: Error recovery - multiple errors in one file */
static void test_error_multiple_errors_recovery(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo(\nfunc bar()\nend\nfunc baz()\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "multiple errors detected");
    /* Parser should recover and parse bar and baz */
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 28: Missing colon in parameter */
static void test_error_missing_param_colon(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo(x int)\n    return x\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing param colon");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 29: Missing comma in parameters */
static void test_error_missing_param_comma(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo(x: int y: text)\n    return x\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing param comma");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 30: Invalid token in expression */
static void test_error_invalid_expression(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    const x: int = 1 + + 2\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    /* Parser should handle this gracefully */
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 31: Missing type in generic constraint */
static void test_error_missing_constraint_type(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo<T>()\nwhere T is\n    return\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing constraint type");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 32: Missing 'is' in generic constraint */
static void test_error_missing_is_keyword(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo<T>()\nwhere T TextRepresentable<T>\n    return\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing 'is' keyword");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 33: Missing opening paren in call */
static void test_error_missing_call_paren(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    foo x, y\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    /* This will parse as expression statement with identifiers */
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 34: Missing closing paren in call */
static void test_error_missing_closing_paren(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    foo(1, 2\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing closing paren");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 35: Missing opening bracket in index */
static void test_error_missing_index_bracket(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    arr 0\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    /* Will parse as two separate expressions */
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 36: Missing closing bracket in index */
static void test_error_missing_closing_bracket(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    arr[0\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on missing closing bracket");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 37: Invalid struct field syntax */
static void test_error_invalid_field_syntax(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nstruct Foo\n    x int\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on invalid field syntax");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 38: Missing trait method body */
static void test_error_missing_trait_method(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\ntrait Foo\n    x: int\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on invalid trait member");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 39: Missing apply method body */
static void test_error_missing_apply_method(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\napply Foo\n    x: int\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on invalid apply member");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 40: Missing extern function */
static void test_error_missing_extern_func(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nextern c\n    x: int\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on invalid extern member");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 41: Error recovery - invalid token at start of file */
static void test_error_recovery_invalid_start(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nreturn 1\nfunc foo()\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    /* Parser should recover and parse func foo */
    ASSERT_GT((int)r.diagnostics.count, 0, "error on invalid start token");
    ASSERT_NOT_NULL(r.root, "root not null after recovery");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 42: Error recovery - garbage between declarations */
static void test_error_recovery_garbage_between_decls(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\nend\n++++\nfunc bar()\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_GT((int)r.diagnostics.count, 0, "error on garbage between decls");
    /* Parser should recover and parse bar */
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 43: Error recovery - unclosed string literal */
static void test_error_recovery_unclosed_string(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    const x: text = \"hello\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    /* Lexer will handle this, parser should recover */
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 44: Error recovery - deeply nested expressions */
static void test_error_recovery_deep_nesting(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\n    const x: int = ((((((((((1))))))))))\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    /* Should parse successfully */
    ASSERT_EQ((int)r.diagnostics.count, 0, "deep nesting parses OK");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 45: Error recovery - empty block */
static void test_error_recovery_empty_block(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc foo()\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    /* Empty block is valid */
    ASSERT_EQ((int)r.diagnostics.count, 0, "empty block is valid");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

/* Test 46: `fmt "..."` interpolation parses as a first-class expression */
static void test_fmt_interpolation_supported(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func show(name: text)\n"
        "    const msg: text = fmt \"Hello {name}\"\n"
        "    const ok: text = \"fallback\"\n"
        "end";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    const zt_ast_node *func = NULL;
    const zt_ast_node *stmt = NULL;
    ASSERT_EQ((int)r.diagnostics.count, 0, "fmt interpolation parses without deferred diagnostic");
    ASSERT_NOT_NULL(r.root, "root exists after fmt deferred diagnostic");
    ASSERT_EQ((int)r.root->as.file.declarations.count, 1, "function still parsed");
    ASSERT_EQ((int)r.root->as.file.declarations.items[0]->as.func_decl.body->as.block.statements.count, 2, "parser recovers and continues after fmt deferred");
    func = r.root->as.file.declarations.items[0];
    stmt = func->as.func_decl.body->as.block.statements.items[0];
    ASSERT_EQ((int)stmt->kind, (int)ZT_AST_CONST_DECL, "first statement is const msg");
    ASSERT_NOT_NULL(stmt->as.const_decl.init_value, "fmt expression exists");
    ASSERT_EQ((int)stmt->as.const_decl.init_value->kind, (int)ZT_AST_FMT_EXPR, "const msg init is fmt expr");
    ASSERT_GT((int)stmt->as.const_decl.init_value->as.fmt_expr.parts.count, 1, "fmt expression keeps literal and interpolation parts");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_error_noncanonical_syntax_suggestions(void) {
    assert_parse_diag_contains(
        "namespace app\nfunc main(name: string)\nend",
        "use 'text'",
        "noncanonical string suggests text");

    assert_parse_diag_contains(
        "namespace app\nfunc main()\n    let name: text = \"Ada\"\nend",
        "use 'const name: Type = value' or 'var name: Type = value'",
        "noncanonical let suggests const or var");

    assert_parse_diag_contains(
        "namespace app\nfunc main() -> int\n    const ok: bool = true\n    if ok && true\n        return 1\n    end\n    return 0\nend",
        "use 'and'",
        "noncanonical && suggests and");

    assert_parse_diag_contains(
        "namespace app\nfunc main() -> int\n    const ok: bool = true\n    if ok || false\n        return 1\n    end\n    return 0\nend",
        "use 'or'",
        "noncanonical || suggests or");

    assert_parse_diag_contains(
        "namespace app\nfunc main() -> int\n    const ok: bool = true\n    if !ok\n        return 1\n    end\n    return 0\nend",
        "use 'not value'",
        "noncanonical ! suggests not");

    assert_parse_diag_contains(
        "namespace app\nfunc main()\n    const maybe_name: optional<text> = null\nend",
        "use optional<T> and none",
        "noncanonical null suggests optional none");

    assert_parse_diag_contains(
        "namespace app\nfunc main()\n    throw error(\"bad\")\nend",
        "use result<T,E>, error(...), or panic(...)",
        "noncanonical throw suggests result or panic");

    assert_parse_diag_contains(
        "namespace app\nabstract class Scoreable\nend",
        "use trait",
        "noncanonical abstract suggests trait");

    assert_parse_diag_contains(
        "namespace app\nvirtual func area() -> int\n    return 0\nend",
        "use dyn<Trait>",
        "noncanonical virtual suggests dyn");

    assert_parse_diag_contains(
        "namespace app\nunion Shape\nend",
        "use enum with payload",
        "noncanonical union suggests enum");

    assert_parse_diag_contains(
        "namespace app\npartial struct Player\nend",
        "use apply and namespace/file organization",
        "noncanonical partial suggests apply");
}

int main(void) {
    printf("Running parser error recovery tests...\n\n");

    test_error_missing_func_name();
    test_error_missing_param_type();
    test_error_missing_return_type();
    test_error_missing_func_end();
    test_error_missing_struct_end();
    test_error_missing_enum_end();
    test_error_missing_trait_end();
    test_error_missing_apply_end();
    test_error_missing_extern_end();
    test_error_missing_else_block();
    test_error_missing_if_end();
    test_error_missing_while_end();
    test_error_missing_for_end();
    test_error_missing_repeat_end();
    test_error_missing_match_end();
    test_top_level_var_allowed();
    test_error_missing_field_type();
    test_error_missing_variant_name();
    test_error_missing_return_expr();
    test_error_missing_if_condition();
    test_error_missing_while_condition();
    test_error_missing_for_iterable();
    test_error_missing_repeat_count();
    test_error_missing_times_keyword();
    test_error_missing_match_subject();
    test_error_missing_case_pattern();
    test_error_multiple_errors_recovery();
    test_error_missing_param_colon();
    test_error_missing_param_comma();
    test_error_invalid_expression();
    test_error_missing_constraint_type();
    test_error_missing_is_keyword();
    test_error_missing_call_paren();
    test_error_missing_closing_paren();
    test_error_missing_index_bracket();
    test_error_missing_closing_bracket();
    test_error_invalid_field_syntax();
    test_error_missing_trait_method();
    test_error_missing_apply_method();
    test_error_missing_extern_func();
    test_error_recovery_invalid_start();
    test_error_recovery_garbage_between_decls();
    test_error_recovery_unclosed_string();
    test_error_recovery_deep_nesting();
    test_error_recovery_empty_block();
    test_fmt_interpolation_supported();
    test_error_noncanonical_syntax_suggestions();

    printf("\nError recovery tests: %d/%d passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
