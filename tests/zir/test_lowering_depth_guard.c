#include "compiler/frontend/parser/parser.h"
#include "compiler/hir/lowering/from_ast.h"
#include "compiler/utils/arena.h"
#include "compiler/utils/string_pool.h"
#include "compiler/zir/lowering/from_hir.h"

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

static int has_diag_code(const zt_diag_list *diagnostics, zt_diag_code code) {
    size_t i;
    if (diagnostics == NULL) return 0;
    for (i = 0; i < diagnostics->count; i += 1) {
        if (diagnostics->items[i].code == code) {
            return 1;
        }
    }
    return 0;
}

static void test_lowering_reports_deep_expression_limit(void) {
    enum { DEPTH = 150 };
    char *source;
    size_t capacity = 16384 + (DEPTH * 32);
    size_t offset = 0;
    size_t i;
    zt_arena arena;
    zt_string_pool pool;
    zt_parser_result parsed;
    zt_hir_lower_result hir;
    zir_lower_result zir;

    source = (char *)calloc(capacity, 1);
    ASSERT_TRUE(source != NULL, "zir_depth_alloc");
    if (source == NULL) {
        return;
    }

    offset += (size_t)snprintf(
        source + offset,
        capacity - offset,
        "namespace app\nfunc demo() -> int\n");
    for (i = 0; i < DEPTH; i += 1) {
        offset += (size_t)snprintf(source + offset, capacity - offset, "    if true\n");
    }
    offset += (size_t)snprintf(source + offset, capacity - offset, "    return 1\n");
    for (i = 0; i < DEPTH; i += 1) {
        offset += (size_t)snprintf(source + offset, capacity - offset, "    end\n");
    }
    offset += (size_t)snprintf(source + offset, capacity - offset, "    return 0\nend\n");

    zt_arena_init(&arena, 1 << 16);
    zt_string_pool_init(&pool, &arena);
    parsed = zt_parse(&arena, &pool, "deep_lower.zt", source, strlen(source));
    ASSERT_TRUE(parsed.diagnostics.count == 0, "zir_depth_parse_ok");
    if (parsed.diagnostics.count != 0) {
        zt_parser_result_dispose(&parsed);
        free(source);
        return;
    }

    hir = zt_lower_ast_to_hir(parsed.root);
    ASSERT_TRUE(hir.diagnostics.count == 0, "zir_depth_hir_ok");
    if (hir.diagnostics.count != 0) {
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        free(source);
        return;
    }

    zir = zir_lower_hir_to_zir(hir.module);
    ASSERT_TRUE(
        has_diag_code(&zir.diagnostics, ZT_DIAG_STRUCTURE_LIMIT_EXCEEDED),
        "zir_depth_guard_code");

    zir_lower_result_dispose(&zir);
    zt_hir_lower_result_dispose(&hir);
    zt_parser_result_dispose(&parsed);
    free(source);
}

int main(void) {
    test_lowering_reports_deep_expression_limit();
    printf("ZIR lowering depth guard tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
