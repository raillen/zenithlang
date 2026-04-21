/*
 * R2.M3 - Property tests for conversions and overflow.
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

static void test_explicit_conversion_int_to_int64(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> int64\n"
        "    const x: int = 42\n"
        "    return int64(x)\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "explicit_conversion_int_to_int64 parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "explicit_conversion_int_to_int64 type");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_widening_conversion_int_to_float(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> float\n"
        "    const x: int = 42\n"
        "    return float(x)\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "widening_conversion_int_to_float parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "widening_conversion_int_to_float type");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_integer_overflow_rejected(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> int8\n"
        "    return int8(120) + int8(10)\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "integer_overflow_rejected parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_HAS_ERROR(checked, ZT_DIAG_INTEGER_OVERFLOW, "integer_overflow_rejected code");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_invalid_conversion_text_to_int(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> int\n"
        "    const s: text = \"42\"\n"
        "    return int(s)\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "invalid_conversion_text_to_int parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_HAS_ERROR(checked, ZT_DIAG_INVALID_CONVERSION, "text->int should be invalid");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_bool_to_int_invalid(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> int\n"
        "    const b: bool = true\n"
        "    return int(b)\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "bool_to_int_invalid parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_HAS_ERROR(checked, ZT_DIAG_INVALID_CONVERSION, "bool->int should be invalid");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_uint8_to_int_explicit(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> int\n"
        "    const x: uint8 = 42\n"
        "    return int(x)\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "uint8_to_int_explicit parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "uint8_to_int_explicit type");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_float_to_int_truncation(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> int\n"
        "    const f: float = 3.14\n"
        "    return int(f)\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "float_to_int_truncation parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "float_to_int_truncation type");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

int main(void) {
    printf("Running conversion/overflow property tests...\n\n");

    test_explicit_conversion_int_to_int64();
    test_widening_conversion_int_to_float();
    test_integer_overflow_rejected();
    test_invalid_conversion_text_to_int();
    test_bool_to_int_invalid();
    test_uint8_to_int_explicit();
    test_float_to_int_truncation();

    printf("\nConversion/Overflow property tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
