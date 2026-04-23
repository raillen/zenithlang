#include "compiler/utils/string_pool.h"
#include "compiler/frontend/parser/parser.h"
#include "compiler/semantic/types/checker.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT_EQ(actual, expected, msg) do { \
    tests_run++; \
    if ((actual) == (expected)) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected %d, got %d\n", msg, (int)(expected), (int)(actual)); } \
} while(0)

#define ASSERT_NO_PARSE_ERRORS(result, msg) do { \
    tests_run++; \
    if ((result).diagnostics.count == 0) { tests_passed++; } \
    else { \
        fprintf(stderr, "FAIL: %s: parse errors=%zu\n", msg, (result).diagnostics.count); \
        for (size_t _i = 0; _i < (result).diagnostics.count; _i++) { \
            fprintf(stderr, "  parse diag: %s\n", (result).diagnostics.items[_i].message); \
        } \
    } \
} while(0)

static int has_diag_code(const zt_check_result *checked, zt_diag_code code) {
    size_t i;

    if (checked == NULL) return 0;
    for (i = 0; i < checked->diagnostics.count; i++) {
        if (checked->diagnostics.items[i].code == code) return 1;
    }
    return 0;
}

static void test_integer_overflow_rejected(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    const char *src =
        "namespace app\n"
        "func demo() -> int64\n"
        "    return 9223372036854775807 + 1\n"
        "end";
    zt_parser_result parsed;
    zt_check_result checked;

    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "integer_overflow parse");

    checked = zt_check_file(parsed.root);
    ASSERT_EQ(has_diag_code(&checked, ZT_DIAG_INTEGER_OVERFLOW), 1, "integer_overflow diag");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_float_out_of_range_rejected(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    const char *src =
        "namespace app\n"
        "func demo() -> float\n"
        "    return 1e309\n"
        "end";
    zt_parser_result parsed;
    zt_check_result checked;

    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "float_out_of_range parse");

    checked = zt_check_file(parsed.root);
    ASSERT_EQ(has_diag_code(&checked, ZT_DIAG_INVALID_CONVERSION), 1, "float_out_of_range diag");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

int main(void) {
    test_integer_overflow_rejected();
    test_float_out_of_range_rejected();
    printf("Numeric literal guardrail tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
