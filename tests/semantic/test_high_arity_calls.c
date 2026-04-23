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

static int has_diag_code(const zt_check_result *checked, zt_diag_code code) {
    size_t i;
    if (checked == NULL) return 0;
    for (i = 0; i < checked->diagnostics.count; i++) {
        if (checked->diagnostics.items[i].code == code) return 1;
    }
    return 0;
}

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

static void test_high_arity_missing_argument_rejected(void) {
    enum { PARAM_COUNT = 129, PROVIDED_ARGS = 128 };
    char src[32768];
    size_t offset = 0;
    size_t i;
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_parser_result parsed;
    zt_check_result checked;

    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    ASSERT_EQ(appendf(src, sizeof(src), &offset, "namespace app\nfunc giant("), 1, "high_arity append header");
    for (i = 1; i <= PARAM_COUNT; i++) {
        ASSERT_EQ(appendf(src, sizeof(src), &offset, "p%03zu: int", i), 1, "high_arity append param");
        if (i < PARAM_COUNT) {
            ASSERT_EQ(appendf(src, sizeof(src), &offset, ", "), 1, "high_arity append param comma");
        }
    }
    ASSERT_EQ(appendf(src, sizeof(src), &offset, ") -> int\n    return p001\nend\nfunc demo() -> int\n    return giant("), 1, "high_arity append call header");
    for (i = 1; i <= PROVIDED_ARGS; i++) {
        ASSERT_EQ(appendf(src, sizeof(src), &offset, "%zu", i), 1, "high_arity append arg");
        if (i < PROVIDED_ARGS) {
            ASSERT_EQ(appendf(src, sizeof(src), &offset, ", "), 1, "high_arity append arg comma");
        }
    }
    ASSERT_EQ(appendf(src, sizeof(src), &offset, ")\nend"), 1, "high_arity append footer");

    parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "high_arity_missing_argument_rejected parse");

    checked = zt_check_file(parsed.root);
    ASSERT_EQ(has_diag_code(&checked, ZT_DIAG_INVALID_ARGUMENT), 1, "high_arity_missing_argument_rejected code");

    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

int main(void) {
    test_high_arity_missing_argument_rejected();
    printf("High arity semantic tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
