/*
 * R2.M3 - Property tests for Optional and Result.
 */

#include "compiler/utils/arena.h"
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
    else { fprintf(stderr, "FAIL: %s: parse errors=%zu\n", msg, (result).diagnostics.count); } \
} while(0)

#define ASSERT_NO_TYPE_ERRORS(result, msg) do { \
    tests_run++; \
    if ((result).diagnostics.count == 0) { tests_passed++; } \
    else { \
        fprintf(stderr, "FAIL: %s: diagnostics=%zu\n", msg, (result).diagnostics.count); \
        for (size_t _i = 0; _i < (result).diagnostics.count; _i++) { \
            fprintf(stderr, "  diag: %s\n", (result).diagnostics.items[_i].message); \
        } \
    } \
} while(0)

#define ASSERT_HAS_ERROR(result, expected_code, msg) do { \
    tests_run++; \
    int _found = 0; \
    for (size_t _i = 0; _i < (result).diagnostics.count; _i++) { \
        if ((result).diagnostics.items[_i].code == (expected_code)) { \
            _found = 1; \
            break; \
        } \
    } \
    if (_found) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected diag code %d, not found\n", msg, (int)(expected_code)); } \
} while(0)

static void test_optional_none_assignment(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> optional<int>\n"
        "    return none\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "optional_none_assignment parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "optional_none_assignment type");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_optional_with_value(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> optional<int>\n"
        "    return 42\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "optional_with_value parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "optional_with_value type");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_result_success(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> result<int, text>\n"
        "    return success(42)\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "result_success parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "result_success type");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_result_error(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> result<int, text>\n"
        "    return error(\"failed\")\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "result_error parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "result_error type");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_try_operator_propagation(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func parse_int(s: text) -> result<int, core.Error>\n"
        "    return success(42)\n"
        "end\n"
        "func demo() -> result<int, core.Error>\n"
        "    const value: int = parse_int(\"42\")?\n"
        "    return success(value)\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "try_operator_propagation parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "try_operator_propagation type");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_optional_type_mismatch(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> optional<int>\n"
        "    return \"not an int\"\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "optional_type_mismatch parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_EQ((int)checked.diagnostics.count >= 1, 1, "optional_type_mismatch should have type error");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_none_to_non_optional(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> int\n"
        "    return none\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "none_to_non_optional parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_EQ((int)checked.diagnostics.count >= 1, 1, "none_to_non_optional should have type error");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

int main(void) {
    printf("Running optional/result property tests...\n\n");

    test_optional_none_assignment();
    test_optional_with_value();
    test_result_success();
    test_result_error();
    test_try_operator_propagation();
    test_optional_type_mismatch();
    test_none_to_non_optional();

    printf("\nOptional/Result property tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}

