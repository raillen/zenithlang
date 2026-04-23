#include "compiler/utils/string_pool.h"
#include "compiler/frontend/parser/parser.h"
#include "compiler/semantic/types/checker.h"

#include <stdio.h>
#include <stdarg.h>
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

static int appendf(char *buffer, size_t capacity, size_t *offset, const char *fmt, ...) {
    int written;
    va_list args;

    if (buffer == NULL || offset == NULL || fmt == NULL || *offset >= capacity) {
        return 0;
    }

    va_start(args, fmt);
    written = vsnprintf(buffer + *offset, capacity - *offset, fmt, args);
    va_end(args);

    if (written < 0 || (size_t)written >= capacity - *offset) {
        return 0;
    }

    *offset += (size_t)written;
    return 1;
}

static void test_catalog_lookup_scale_ok(void) {
    enum {
        CONST_COUNT = 2000,
        FUNC_COUNT = 1500,
        BUFFER_CAPACITY = 1024 * 1024
    };
    static char src[BUFFER_CAPACITY];
    size_t offset = 0;
    size_t i;
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_parser_result parsed;
    zt_check_result checked;

    zt_arena_init(&test_arena, 1024 * 1024);
    zt_string_pool_init(&test_pool, &test_arena);

    ASSERT_EQ(appendf(src, sizeof(src), &offset, "namespace app\n"), 1, "catalog_scale namespace");

    for (i = 0; i < CONST_COUNT; i++) {
        ASSERT_EQ(
            appendf(src, sizeof(src), &offset, "const c%04zu: int = %zu\n", i, i),
            1,
            "catalog_scale append const");
    }

    for (i = 0; i < FUNC_COUNT; i++) {
        ASSERT_EQ(
            appendf(
                src,
                sizeof(src),
                &offset,
                "func use%04zu() -> int\n    return c%04zu\nend\n",
                i,
                CONST_COUNT - i - 1),
            1,
            "catalog_scale append func");
    }

    ASSERT_EQ(
        appendf(
            src,
            sizeof(src),
            &offset,
            "func main() -> int\n    return use%04d()\nend\n",
            FUNC_COUNT - 1),
        1,
        "catalog_scale append main");

    parsed = zt_parse(&test_arena, &test_pool, "catalog_scale", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "catalog_scale parse");

    checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "catalog_scale type");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

int main(void) {
    test_catalog_lookup_scale_ok();
    printf("Catalog lookup scale tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
