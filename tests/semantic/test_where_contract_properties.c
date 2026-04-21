/*
 * R2.M3 - Property tests for where clauses and contracts.
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

static void test_where_trait_constraint_ok(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func process<T>(value: T) -> text\n"
        "where T is TextRepresentable<T>\n"
        "    return value.to_text()\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "where_trait_constraint_ok parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "where_trait_constraint_ok type");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_where_struct_constraint_fail(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "struct Player\n"
        "    name: text\n"
        "end\n"
        "func process<T>(value: T) -> text\n"
        "where T is Player\n"
        "    return \"x\"\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "where_struct_constraint_fail parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_HAS_ERROR(checked, ZT_DIAG_INVALID_TYPE, "where_struct_constraint_fail should reject struct");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_where_multiple_constraints(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func render<T>(value: T) -> text\n"
        "where T is TextRepresentable<T> and T is TextRepresentable<T>\n"
        "    return value.to_text()\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "where_multiple_constraints parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "where_multiple_constraints type");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_generic_no_constraint(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func identity<T>(value: T) -> T\n"
        "    return value\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "generic_no_constraint parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "generic_no_constraint type");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_where_on_struct_method(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "struct Container<T>\n"
        "    value: T\n"
        "end\n"
        "func display<T>(c: Container<T>) -> text\n"
        "where T is TextRepresentable<T>\n"
        "    return c.value.to_text()\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "where_on_struct_method parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "where_on_struct_method type");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_contract_violation_type_mismatch(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "struct Player\n"
        "    name: text\n"
        "end\n"
        "func render<T>(value: T) -> text\n"
        "where T is TextRepresentable<T>\n"
        "    return value.to_text()\n"
        "end\n"
        "func demo() -> text\n"
        "    const p: Player = Player(name: \"zen\")\n"
        "    return render(p)\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "contract_violation parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_EQ((int)checked.diagnostics.count >= 1, 1, "contract_violation should have errors");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

int main(void) {
    printf("Running where/contracts property tests...\n\n");

    test_where_trait_constraint_ok();
    test_where_struct_constraint_fail();
    test_where_multiple_constraints();
    test_generic_no_constraint();
    test_where_on_struct_method();
    test_contract_violation_type_mismatch();

    printf("\nWhere/Contracts property tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}

