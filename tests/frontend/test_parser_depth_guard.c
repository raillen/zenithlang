#include "compiler/frontend/parser/parser.h"
#include "compiler/utils/arena.h"
#include "compiler/utils/string_pool.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT_TRUE(condition, msg) do { \
    tests_run++; \
    if (condition) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s\n", msg); } \
} while (0)

static int has_diag_code(const zt_parser_result *result, zt_diag_code code) {
    size_t i;
    if (result == NULL) return 0;
    for (i = 0; i < result->diagnostics.count; i += 1) {
        if (result->diagnostics.items[i].code == code) {
            return 1;
        }
    }
    return 0;
}

static void test_parser_reports_deep_expression_limit(void) {
    enum { DEPTH = 700 };
    char *source;
    size_t capacity = 4096 + (DEPTH * 2);
    size_t offset = 0;
    size_t i;
    zt_arena arena;
    zt_string_pool pool;
    zt_parser_result parsed;

    source = (char *)calloc(capacity, 1);
    ASSERT_TRUE(source != NULL, "parser_depth_alloc");
    if (source == NULL) {
        return;
    }

    offset += (size_t)snprintf(
        source + offset,
        capacity - offset,
        "namespace app\nfunc demo() -> int\n    return ");
    for (i = 0; i < DEPTH; i += 1) {
        offset += (size_t)snprintf(source + offset, capacity - offset, "(");
    }
    offset += (size_t)snprintf(source + offset, capacity - offset, "1");
    for (i = 0; i < DEPTH; i += 1) {
        offset += (size_t)snprintf(source + offset, capacity - offset, ")");
    }
    offset += (size_t)snprintf(source + offset, capacity - offset, "\nend\n");

    zt_arena_init(&arena, 1 << 16);
    zt_string_pool_init(&pool, &arena);
    parsed = zt_parse(&arena, &pool, "deep_expr.zt", source, strlen(source));

    ASSERT_TRUE(
        has_diag_code(&parsed, ZT_DIAG_STRUCTURE_LIMIT_EXCEEDED),
        "parser_depth_guard_code");

    zt_parser_result_dispose(&parsed);
    free(source);
}

int main(void) {
    test_parser_reports_deep_expression_limit();
    printf("Parser depth guard tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
