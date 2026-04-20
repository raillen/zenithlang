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

static int has_diag_code(const zt_check_result *checked, zt_diag_code code) {
    size_t i;
    if (checked == NULL) return 0;
    for (i = 0; i < checked->diagnostics.count; i++) {
        if (checked->diagnostics.items[i].code == code) return 1;
    }
    return 0;
}

static void test_const_reassignment_rejected(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo()\n"
        "    const value: int = 1\n"
        "    value = 2\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "const_reassignment parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_EQ((int)checked.diagnostics.count, 1, "const_reassignment diag count");
    ASSERT_EQ((int)checked.diagnostics.items[0].code, ZT_DIAG_CONST_REASSIGNMENT, "const_reassignment diag code");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_optional_none_return_ok(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> optional<text>\n"
        "    return none\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "optional_none_return_ok parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "optional_none_return_ok type");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_result_success_error_ok(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func ok() -> result<int, text>\n"
        "    return success(1)\n"
        "end\n"
        "func fail() -> result<void, text>\n"
        "    return error(\"bad\")\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "result_success_error_ok parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "result_success_error_ok type");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_named_args_and_defaults_ok(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func sum(a: int, b: int = 1) -> int\n"
        "    return a + b\n"
        "end\n"
        "func demo() -> int\n"
        "    return sum(1, b: 2)\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "named_args_and_defaults_ok parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "named_args_and_defaults_ok type");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_named_args_out_of_order_rejected(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func sum(a: int, b: int = 1) -> int\n"
        "    return a + b\n"
        "end\n"
        "func demo() -> int\n"
        "    return sum(b: 2, a: 1)\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "named_args_out_of_order_rejected parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_EQ((int)checked.diagnostics.count, 1, "named_args_out_of_order_rejected diag count");
    ASSERT_EQ((int)checked.diagnostics.items[0].code, ZT_DIAG_INVALID_ARGUMENT, "named_args_out_of_order_rejected diag code");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_invalid_map_key_type_rejected(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> map<float, text>\n"
        "    return {}\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "invalid_map_key_type_rejected parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_EQ((int)checked.diagnostics.count, 1, "invalid_map_key_type_rejected diag count");
    ASSERT_EQ((int)checked.diagnostics.items[0].code, ZT_DIAG_INVALID_MAP_KEY_TYPE, "invalid_map_key_type_rejected diag code");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_explicit_conversion_rules(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src_ok =
        "namespace app\n"
        "func ok() -> int8\n"
        "    return int8(10)\n"
        "end";
    const char *src_bad =
        "namespace app\n"
        "func bad() -> int\n"
        "    return int(10.5)\n"
        "end";
    zt_parser_result parsed_ok = zt_parse(&test_arena, &test_pool, "test", src_ok, strlen(src_ok));
    zt_parser_result parsed_bad = zt_parse(&test_arena, &test_pool, "test", src_bad, strlen(src_bad));
    ASSERT_NO_PARSE_ERRORS(parsed_ok, "explicit_conversion_rules ok parse");
    ASSERT_NO_PARSE_ERRORS(parsed_bad, "explicit_conversion_rules bad parse");
    zt_check_result checked_ok = zt_check_file(parsed_ok.root);
    zt_check_result checked_bad = zt_check_file(parsed_bad.root);
    ASSERT_NO_TYPE_ERRORS(checked_ok, "explicit_conversion_rules ok type");
    ASSERT_EQ((int)checked_bad.diagnostics.count, 1, "explicit_conversion_rules bad diag count");
    ASSERT_EQ((int)checked_bad.diagnostics.items[0].code, ZT_DIAG_INVALID_CONVERSION, "explicit_conversion_rules bad diag code");
    zt_check_result_dispose(&checked_ok);
    zt_check_result_dispose(&checked_bad);
    zt_parser_result_dispose(&parsed_ok);
    zt_parser_result_dispose(&parsed_bad);
}

static void test_non_mutating_self_assignment_rejected(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "struct Player\n"
        "    hp: int\n"
        "end\n"
        "apply Player\n"
        "    func heal(amount: int)\n"
        "        self.hp = self.hp + amount\n"
        "    end\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "non_mutating_self_assignment_rejected parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_EQ((int)checked.diagnostics.count, 1, "non_mutating_self_assignment_rejected diag count");
    ASSERT_EQ((int)checked.diagnostics.items[0].code, ZT_DIAG_INVALID_MUTATION, "non_mutating_self_assignment_rejected diag code");
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
    ASSERT_EQ((int)checked.diagnostics.count, 1, "integer_overflow_rejected diag count");
    ASSERT_EQ((int)checked.diagnostics.items[0].code, ZT_DIAG_INTEGER_OVERFLOW, "integer_overflow_rejected diag code");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_contextual_integer_literal_arithmetic_ok(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo(value: uint8) -> uint8\n"
        "    return value + 1\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "contextual_integer_literal_arithmetic_ok parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "contextual_integer_literal_arithmetic_ok type");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_bytes_literal_and_len_ok(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> int\n"
        "    const data: bytes = hex bytes \"DE AD BE EF\"\n"
        "    const part: bytes = data[1..2]\n"
        "    const byte: uint8 = data[2]\n"
        "    return len(data) + len(part)\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "bytes_literal_and_len_ok parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "bytes_literal_and_len_ok type");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_std_bytes_ops_ok(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "import std.bytes as bytes\n"
        "func demo() -> int\n"
        "    const values: list<int> = [1, 2, 3]\n"
        "    const data: bytes = bytes.from_list(values)\n"
        "    const roundtrip: list<int> = bytes.to_list(data)\n"
        "    const joined: bytes = bytes.join(data, hex bytes \"04\")\n"
        "    const has_prefix: bool = bytes.starts_with(joined, hex bytes \"01\")\n"
        "    const has_suffix: bool = bytes.ends_with(joined, hex bytes \"04\")\n"
        "    const has_middle: bool = bytes.contains(joined, hex bytes \"02 03\")\n"
        "    if has_prefix and has_suffix and has_middle\n"
        "        return roundtrip[0]\n"
        "    end\n"
        "    return 0\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "std_bytes_ops_ok parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "std_bytes_ops_ok type");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_for_and_match_type_checking(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src_ok =
        "namespace app\n"
        "func sum(scores: map<text, int>) -> int\n"
        "    var total: int = 0\n"
        "    for key, value in scores\n"
        "        total = total + value\n"
        "    end\n"
        "    return total\n"
        "end";
    const char *src_bad =
        "namespace app\n"
        "func demo(value: int) -> int\n"
        "    match value\n"
        "        case \"x\"\n"
        "            return 1\n"
        "    end\n"
        "    return 0\n"
        "end";
    zt_parser_result parsed_ok = zt_parse(&test_arena, &test_pool, "test", src_ok, strlen(src_ok));
    zt_parser_result parsed_bad = zt_parse(&test_arena, &test_pool, "test", src_bad, strlen(src_bad));
    ASSERT_NO_PARSE_ERRORS(parsed_ok, "for_and_match_type_checking ok parse");
    ASSERT_NO_PARSE_ERRORS(parsed_bad, "for_and_match_type_checking bad parse");
    zt_check_result checked_ok = zt_check_file(parsed_ok.root);
    zt_check_result checked_bad = zt_check_file(parsed_bad.root);
    ASSERT_NO_TYPE_ERRORS(checked_ok, "for_and_match_type_checking ok type");
    ASSERT_EQ((int)checked_bad.diagnostics.count, 1, "for_and_match_type_checking bad diag count");
    ASSERT_EQ((int)checked_bad.diagnostics.items[0].code, ZT_DIAG_TYPE_MISMATCH, "for_and_match_type_checking bad diag code");
    zt_check_result_dispose(&checked_ok);
    zt_check_result_dispose(&checked_bad);
    zt_parser_result_dispose(&parsed_ok);
    zt_parser_result_dispose(&parsed_bad);
}

static void test_const_collection_index_mutation_rejected(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo()\n"
        "    const values: list<int> = [1, 2, 3]\n"
        "    values[0] = 9\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "const_collection_index_mutation_rejected parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_EQ((int)checked.diagnostics.count, 1, "const_collection_index_mutation_rejected diag count");
    ASSERT_EQ((int)checked.diagnostics.items[0].code, ZT_DIAG_INVALID_MUTATION, "const_collection_index_mutation_rejected diag code");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_enum_constructor_and_match_binding_ok(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "enum Shape\n"
        "    Circle(radius: int)\n"
        "    Rectangle(width: int, height: int)\n"
        "    Point\n"
        "end\n"
        "func area(shape: Shape) -> int\n"
        "    match shape\n"
        "        case Shape.Circle(r)\n"
        "            return r * r\n"
        "        case Shape.Rectangle(w, h)\n"
        "            return w * h\n"
        "        case Shape.Point\n"
        "            return 0\n"
        "    end\n"
        "end\n"
        "func make() -> Shape\n"
        "    return Shape.Circle(radius: 10)\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "enum_constructor_and_match_binding_ok parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "enum_constructor_and_match_binding_ok type");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_enum_match_duplicate_default_rejected(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "enum Status\n"
        "    Ok\n"
        "    Error\n"
        "end\n"
        "func demo(status: Status) -> int\n"
        "    match status\n"
        "        case default\n"
        "            return 1\n"
        "        case default\n"
        "            return 2\n"
        "    end\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "enum_match_duplicate_default_rejected parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_EQ(has_diag_code(&checked, ZT_DIAG_INVALID_ARGUMENT), 1, "enum_match_duplicate_default_rejected code");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_enum_match_default_must_be_last_rejected(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "enum Status\n"
        "    Ok\n"
        "    Error\n"
        "end\n"
        "func demo(status: Status) -> int\n"
        "    match status\n"
        "        case default\n"
        "            return 1\n"
        "        case Status.Ok\n"
        "            return 2\n"
        "    end\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "enum_match_default_must_be_last_rejected parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_EQ(has_diag_code(&checked, ZT_DIAG_INVALID_ARGUMENT), 1, "enum_match_default_must_be_last_rejected code");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_enum_match_non_exhaustive_rejected(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "enum Status\n"
        "    Ok\n"
        "    Error(message: text)\n"
        "end\n"
        "func demo(status: Status) -> int\n"
        "    match status\n"
        "        case Status.Ok\n"
        "            return 1\n"
        "    end\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "enum_match_non_exhaustive_rejected parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_EQ(has_diag_code(&checked, ZT_DIAG_NON_EXHAUSTIVE_MATCH), 1, "enum_match_non_exhaustive_rejected code");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_core_error_canonical_ok(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func ok() -> result<int, core.Error>\n"
        "    return success(1)\n"
        "end\n"
        "func fail() -> result<void, core.Error>\n"
        "    return error(\"bad\")\n"
        "end\n"
        "func forward() -> result<int, core.Error>\n"
        "    const value: int = ok()?\n"
        "    return success(value)\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "core_error_canonical_ok parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "core_error_canonical_ok type");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}
int main(void) {
    test_const_reassignment_rejected();
    test_optional_none_return_ok();
    test_result_success_error_ok();
    test_core_error_canonical_ok();
    test_named_args_and_defaults_ok();
    test_named_args_out_of_order_rejected();
    test_invalid_map_key_type_rejected();
    test_explicit_conversion_rules();
    test_non_mutating_self_assignment_rejected();
    test_integer_overflow_rejected();
    test_contextual_integer_literal_arithmetic_ok();
    test_bytes_literal_and_len_ok();
    test_std_bytes_ops_ok();
    test_for_and_match_type_checking();
    test_const_collection_index_mutation_rejected();
    test_enum_constructor_and_match_binding_ok();
    test_enum_match_duplicate_default_rejected();
    test_enum_match_default_must_be_last_rejected();
    test_enum_match_non_exhaustive_rejected();

    printf("Type tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}