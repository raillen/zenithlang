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
    if ((result).error_count == 0) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: parse errors=%zu\n", msg, (result).error_count); } \
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

static void test_core_text_representable_constraint_ok(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func render<Item>(value: Item) -> text\n"
        "where Item is TextRepresentable<Item>\n"
        "    return value.to_text()\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "core_text_representable_constraint_ok parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "core_text_representable_constraint_ok type");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_non_trait_constraint_rejected(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "struct Player\n"
        "end\n"
        "func render<Item>(value: Item) -> text\n"
        "where Item is Player\n"
        "    return \"x\"\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "non_trait_constraint_rejected parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_EQ((int)checked.diagnostics.count, 1, "non_trait_constraint_rejected diag count");
    ASSERT_EQ((int)checked.diagnostics.items[0].code, ZT_DIAG_INVALID_TYPE, "non_trait_constraint_rejected diag code");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_user_type_map_key_with_explicit_core_traits_ok(void) {
    const char *src =
        "namespace app\n"
        "struct Player\n"
        "    id: int\n"
        "end\n"
        "apply Equatable<Player> to Player\n"
        "    func equals(other: Player) -> bool\n"
        "        return self.id == other.id\n"
        "    end\n"
        "end\n"
        "apply Hashable<Player> to Player\n"
        "    func hash() -> uint64\n"
        "        return uint64(1)\n"
        "    end\n"
        "end\n"
        "func demo() -> map<Player, text>\n"
        "    return {}\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "user_type_map_key_with_explicit_core_traits_ok parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "user_type_map_key_with_explicit_core_traits_ok type");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

int main(void) {
    test_core_text_representable_constraint_ok();
    test_non_trait_constraint_rejected();
    test_user_type_map_key_with_explicit_core_traits_ok();

    printf("Constraint tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}

