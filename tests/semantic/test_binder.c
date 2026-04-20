#include "compiler/utils/arena.h"
#include "compiler/utils/string_pool.h"
#include "compiler/frontend/parser/parser.h"
#include "compiler/semantic/binder/binder.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT_EQ(actual, expected, msg) do { \
    tests_run++; \
    if ((actual) == (expected)) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected %d, got %d\n", msg, (int)(expected), (int)(actual)); } \
} while(0)

#define ASSERT_NOT_NULL(actual, msg) do { \
    tests_run++; \
    if ((actual) != NULL) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected non-NULL\n", msg); } \
} while(0)

#define ASSERT_NO_PARSE_ERRORS(result, msg) do { \
    tests_run++; \
    if ((result).diagnostics.count == 0) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: parse errors=%zu\n", msg, (result).diagnostics.count); } \
} while(0)

#define ASSERT_NO_BIND_ERRORS(result, msg) do { \
    tests_run++; \
    if ((result).diagnostics.count == 0) { tests_passed++; } \
    else { \
        fprintf(stderr, "FAIL: %s: diagnostics=%zu\n", msg, (result).diagnostics.count); \
        for (size_t _i = 0; _i < (result).diagnostics.count; _i++) { \
            fprintf(stderr, "  diag: %s\n", (result).diagnostics.items[_i].message); \
        } \
    } \
} while(0)

static void test_simple_bind_ok(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "import std.text\n"
        "func greet(name: text) -> text\n"
        "    return text.trim(name)\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "simple_bind_ok parse");
    zt_bind_result bound = zt_bind_file(parsed.root);
    ASSERT_NO_BIND_ERRORS(bound, "simple_bind_ok bind");
    ASSERT_NOT_NULL(zt_scope_lookup(&bound.module_scope, "text"), "import alias text in module scope");
    zt_bind_result_dispose(&bound);
    zt_parser_result_dispose(&parsed);
}

static void test_duplicate_top_level(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "struct Player\n"
        "end\n"
        "func Player()\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "duplicate_top_level parse");
    zt_bind_result bound = zt_bind_file(parsed.root);
    ASSERT_EQ((int)bound.diagnostics.count, 1, "duplicate_top_level diag count");
    ASSERT_EQ(bound.diagnostics.items[0].code, ZT_DIAG_DUPLICATE_NAME, "duplicate_top_level diag code");
    zt_bind_result_dispose(&bound);
    zt_parser_result_dispose(&parsed);
}

static void test_shadowing_local_over_param(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo(value: int)\n"
        "    const value: int = 1\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "shadowing_local_over_param parse");
    zt_bind_result bound = zt_bind_file(parsed.root);
    ASSERT_EQ((int)bound.diagnostics.count, 1, "shadowing_local_over_param diag count");
    ASSERT_EQ(bound.diagnostics.items[0].code, ZT_DIAG_SHADOWING, "shadowing_local_over_param code");
    zt_bind_result_dispose(&bound);
    zt_parser_result_dispose(&parsed);
}

static void test_unresolved_identifier(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> int\n"
        "    return missing\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "unresolved_identifier parse");
    zt_bind_result bound = zt_bind_file(parsed.root);
    ASSERT_EQ((int)bound.diagnostics.count, 1, "unresolved_identifier diag count");
    ASSERT_EQ(bound.diagnostics.items[0].code, ZT_DIAG_UNRESOLVED_NAME, "unresolved_identifier code");
    zt_bind_result_dispose(&bound);
    zt_parser_result_dispose(&parsed);
}

static void test_core_prelude_generic_constraint(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "public func render<Item>(value: Item) -> text\n"
        "where Item is TextRepresentable<Item>\n"
        "    return value.to_text()\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "core_prelude_generic_constraint parse");
    zt_bind_result bound = zt_bind_file(parsed.root);
    ASSERT_NO_BIND_ERRORS(bound, "core_prelude_generic_constraint bind");
    ASSERT_NOT_NULL(zt_scope_lookup(&bound.module_scope, "TextRepresentable"), "implicit core trait available");
    zt_bind_result_dispose(&bound);
    zt_parser_result_dispose(&parsed);
}

static void test_self_resolution_in_apply(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "struct Player\n"
        "    hp: int\n"
        "end\n"
        "apply Player\n"
        "    mut func heal()\n"
        "        self.hp = 1\n"
        "    end\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "self_resolution_in_apply parse");
    zt_bind_result bound = zt_bind_file(parsed.root);
    ASSERT_NO_BIND_ERRORS(bound, "self_resolution_in_apply bind");
    zt_bind_result_dispose(&bound);
    zt_parser_result_dispose(&parsed);
}

static void test_it_resolution_in_where(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "struct Player\n"
        "    hp: int where it >= 0\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "it_resolution_in_where parse");
    zt_bind_result bound = zt_bind_file(parsed.root);
    ASSERT_NO_BIND_ERRORS(bound, "it_resolution_in_where bind");
    zt_bind_result_dispose(&bound);
    zt_parser_result_dispose(&parsed);
}

static void test_duplicate_import_alias(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "import std.text as util\n"
        "import app.util as util";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "duplicate_import_alias parse");
    zt_bind_result bound = zt_bind_file(parsed.root);
    ASSERT_EQ((int)bound.diagnostics.count, 1, "duplicate_import_alias diag count");
    ASSERT_EQ(bound.diagnostics.items[0].code, ZT_DIAG_DUPLICATE_NAME, "duplicate_import_alias code");
    zt_bind_result_dispose(&bound);
    zt_parser_result_dispose(&parsed);
}

static void test_unresolved_qualified_type_prefix(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo(user: unknown_alias.User)\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "unresolved_qualified_type_prefix parse");
    zt_bind_result bound = zt_bind_file(parsed.root);
    ASSERT_EQ((int)bound.diagnostics.count, 1, "unresolved_qualified_type_prefix diag count");
    ASSERT_EQ(bound.diagnostics.items[0].code, ZT_DIAG_UNRESOLVED_NAME, "unresolved_qualified_type_prefix code");
    zt_bind_result_dispose(&bound);
    zt_parser_result_dispose(&parsed);
}

static void test_match_payload_bindings_resolved(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "enum Shape\n"
        "    Circle(radius: int)\n"
        "    Point\n"
        "end\n"
        "func demo(shape: Shape) -> int\n"
        "    match shape\n"
        "        case Shape.Circle(r)\n"
        "            return r\n"
        "        case Shape.Point\n"
        "            return 0\n"
        "    end\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "match_payload_bindings_resolved parse");
    zt_bind_result bound = zt_bind_file(parsed.root);
    ASSERT_NO_BIND_ERRORS(bound, "match_payload_bindings_resolved bind");
    zt_bind_result_dispose(&bound);
    zt_parser_result_dispose(&parsed);
}
static void test_confusing_name_warning(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> int\n"
        "    const I1: int = 1\n"
        "    return I1\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "confusing_name_warning parse");

    zt_bind_result bound = zt_bind_file(parsed.root);
    ASSERT_EQ((int)bound.diagnostics.count, 1, "confusing_name_warning diag count");
    ASSERT_EQ(bound.diagnostics.items[0].code, ZT_DIAG_CONFUSING_NAME, "confusing_name_warning code");
    ASSERT_EQ(bound.diagnostics.items[0].severity, ZT_DIAG_SEVERITY_WARNING, "confusing_name_warning severity");

    zt_bind_result_dispose(&bound);
    zt_parser_result_dispose(&parsed);
}

int main(void) {
    test_simple_bind_ok();
    test_duplicate_top_level();
    test_shadowing_local_over_param();
    test_unresolved_identifier();
    test_core_prelude_generic_constraint();
    test_self_resolution_in_apply();
    test_it_resolution_in_where();
    test_duplicate_import_alias();
    test_unresolved_qualified_type_prefix();
    test_match_payload_bindings_resolved();
    test_confusing_name_warning();

    printf("Binder tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}



