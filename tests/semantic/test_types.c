#include "compiler/utils/arena.h"
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

static void test_to_text_builtin_rules(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src_ok =
        "namespace app\n"
        "func ok() -> text\n"
        "    return to_text(42)\n"
        "end";
    const char *src_bad =
        "namespace app\n"
        "func bad() -> text\n"
        "    const data: bytes = hex bytes \"DE AD\"\n"
        "    return to_text(data)\n"
        "end";
    zt_parser_result parsed_ok = zt_parse(&test_arena, &test_pool, "test", src_ok, strlen(src_ok));
    zt_parser_result parsed_bad = zt_parse(&test_arena, &test_pool, "test", src_bad, strlen(src_bad));
    ASSERT_NO_PARSE_ERRORS(parsed_ok, "to_text_builtin_rules ok parse");
    ASSERT_NO_PARSE_ERRORS(parsed_bad, "to_text_builtin_rules bad parse");
    zt_check_result checked_ok = zt_check_file(parsed_ok.root);
    zt_check_result checked_bad = zt_check_file(parsed_bad.root);
    ASSERT_NO_TYPE_ERRORS(checked_ok, "to_text_builtin_rules ok type");
    ASSERT_EQ((int)checked_bad.diagnostics.count, 1, "to_text_builtin_rules bad diag count");
    ASSERT_EQ((int)checked_bad.diagnostics.items[0].code, ZT_DIAG_INVALID_TYPE, "to_text_builtin_rules bad diag code");
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
        "        case \"x\" ->\n"
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

static void test_module_var_assignment_ok(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "public var counter: int = 0\n"
        "func bump() -> int\n"
        "    counter = counter + 1\n"
        "    return counter\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "module_var_assignment_ok parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "module_var_assignment_ok type");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_module_var_assignment_type_mismatch_rejected(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "public var counter: int = 0\n"
        "func bad()\n"
        "    counter = \"wrong\"\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "module_var_assignment_type_mismatch_rejected parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_EQ(has_diag_code(&checked, ZT_DIAG_TYPE_MISMATCH), 1, "module_var_assignment_type_mismatch_rejected code");
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
        "        case Shape.Circle(r) ->\n"
        "            return r * r\n"
        "        case Shape.Rectangle(w, h) ->\n"
        "            return w * h\n"
        "        case Shape.Point ->\n"
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
        "        case default ->\n"
        "            return 1\n"
        "        case default ->\n"
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
        "        case default ->\n"
        "            return 1\n"
        "        case Status.Ok ->\n"
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

static void test_enum_match_default_warning(void) {
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
        "        case Status.Ok ->\n"
        "            return 1\n"
        "        case default ->\n"
        "            return 2\n"
        "    end\n"
        "end";

    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "enum_match_default_warning parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_EQ((int)checked.diagnostics.count, 1, "enum_match_default_warning diag count");
    ASSERT_EQ(checked.diagnostics.items[0].code, ZT_DIAG_ENUM_DEFAULT_CASE, "enum_match_default_warning code");
    ASSERT_EQ(checked.diagnostics.items[0].severity, ZT_DIAG_SEVERITY_WARNING, "enum_match_default_warning severity");
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
        "        case Status.Ok ->\n"
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

static void test_transferable_builtin_shapes(void) {
    zt_type *int_type = zt_type_make(ZT_TYPE_INT);
    zt_type *text_type = zt_type_make(ZT_TYPE_TEXT);
    zt_type *bytes_type = zt_type_make(ZT_TYPE_BYTES);
    zt_type *core_error_type = zt_type_make(ZT_TYPE_CORE_ERROR);
    zt_type *dyn_text_type;
    zt_type *list_text_type;
    zt_type *optional_list_text_type;
    zt_type *result_int_error_type;
    zt_type_list args = zt_type_list_make();
    zt_type_list nested_args = zt_type_list_make();

    ASSERT_EQ(zt_checker_type_is_transferable(NULL, int_type), 1, "transferable int");
    ASSERT_EQ(zt_checker_type_is_transferable(NULL, text_type), 1, "transferable text");
    ASSERT_EQ(zt_checker_type_is_transferable(NULL, bytes_type), 1, "transferable bytes");
    ASSERT_EQ(zt_checker_type_is_transferable(NULL, core_error_type), 1, "transferable core_error");

    zt_type_list_push(&args, zt_type_make(ZT_TYPE_TEXT));
    list_text_type = zt_type_make_with_args(ZT_TYPE_LIST, NULL, args);
    ASSERT_EQ(zt_checker_type_is_transferable(NULL, list_text_type), 1, "transferable list_text");

    zt_type_list_push(&nested_args, zt_type_clone(list_text_type));
    optional_list_text_type = zt_type_make_with_args(ZT_TYPE_OPTIONAL, NULL, nested_args);
    ASSERT_EQ(zt_checker_type_is_transferable(NULL, optional_list_text_type), 1, "transferable optional_list_text");

    args = zt_type_list_make();
    zt_type_list_push(&args, zt_type_make(ZT_TYPE_INT));
    zt_type_list_push(&args, zt_type_make(ZT_TYPE_CORE_ERROR));
    result_int_error_type = zt_type_make_with_args(ZT_TYPE_RESULT, NULL, args);
    ASSERT_EQ(zt_checker_type_is_transferable(NULL, result_int_error_type), 1, "transferable result_int_core_error");

    args = zt_type_list_make();
    zt_type_list_push(&args, zt_type_make(ZT_TYPE_TEXT));
    dyn_text_type = zt_type_make_with_args(ZT_TYPE_DYN, NULL, args);
    ASSERT_EQ(zt_checker_type_is_transferable(NULL, dyn_text_type), 0, "dyn_text_not_transferable");

    zt_type_dispose(int_type);
    zt_type_dispose(text_type);
    zt_type_dispose(bytes_type);
    zt_type_dispose(core_error_type);
    zt_type_dispose(list_text_type);
    zt_type_dispose(optional_list_text_type);
    zt_type_dispose(result_int_error_type);
    zt_type_dispose(dyn_text_type);
}

static void test_transferable_user_types(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_parser_result parsed;
    zt_check_result checked;
    zt_type *snapshot_type = zt_type_make_named(ZT_TYPE_USER, "Snapshot");
    zt_type *node_type = zt_type_make_named(ZT_TYPE_USER, "Node");
    zt_type *message_type = zt_type_make_named(ZT_TYPE_USER, "Message");
    zt_type *packet_type = zt_type_make_named(ZT_TYPE_USER, "Packet");
    zt_type *dynamic_holder_type = zt_type_make_named(ZT_TYPE_USER, "DynamicHolder");
    zt_type *job_payload_type = zt_type_make_named(ZT_TYPE_USER, "JobPayload");
    zt_type *dynamic_box_type = zt_type_make_named(ZT_TYPE_USER, "DynamicBox");
    const char *src =
        "namespace app\n"
        "struct Snapshot\n"
        "    title: text\n"
        "    items: list<int>\n"
        "end\n"
        "struct Node\n"
        "    name: text\n"
        "    next: optional<Node>\n"
        "end\n"
        "enum Message\n"
        "    Ready(snapshot: Snapshot)\n"
        "    Failed(err: core.Error)\n"
        "end\n"
        "struct Box<T>\n"
        "    value: T\n"
        "end\n"
        "struct Packet\n"
        "    payload: Box<text>\n"
        "end\n"
        "struct DynamicHolder\n"
        "    value: dyn<text>\n"
        "end\n"
        "enum JobPayload\n"
        "    Snapshot(snapshot: Snapshot)\n"
        "    Dynamic(value: dyn<text>)\n"
        "end\n"
        "struct DynamicBox\n"
        "    payload: Box<dyn<text>>\n"
        "end\n";

    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "transferable_user_types parse");
    checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "transferable_user_types type");

    ASSERT_EQ(zt_checker_type_is_transferable(parsed.root, snapshot_type), 1, "transferable Snapshot");
    ASSERT_EQ(zt_checker_type_is_transferable(parsed.root, node_type), 1, "transferable recursive Node");
    ASSERT_EQ(zt_checker_type_is_transferable(parsed.root, message_type), 1, "transferable Message");
    ASSERT_EQ(zt_checker_type_is_transferable(parsed.root, packet_type), 1, "transferable Packet");
    ASSERT_EQ(zt_checker_type_is_transferable(parsed.root, dynamic_holder_type), 0, "DynamicHolder_not_transferable");
    ASSERT_EQ(zt_checker_type_is_transferable(parsed.root, job_payload_type), 0, "JobPayload_not_transferable");
    ASSERT_EQ(zt_checker_type_is_transferable(parsed.root, dynamic_box_type), 0, "DynamicBox_not_transferable");

    zt_type_dispose(snapshot_type);
    zt_type_dispose(node_type);
    zt_type_dispose(message_type);
    zt_type_dispose(packet_type);
    zt_type_dispose(dynamic_holder_type);
    zt_type_dispose(job_payload_type);
    zt_type_dispose(dynamic_box_type);
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_boundary_copy_alpha_subset_ok(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    const char *src =
        "namespace app\n"
        "import std.concurrent as concurrent\n"
        "func demo() -> int\n"
        "    const score: int = concurrent.copy_int(7)\n"
        "    const label: text = concurrent.copy_text(\"ready\")\n"
        "    const raw: bytes = concurrent.copy_bytes(hex bytes \"41 42\")\n"
        "    const ids: list<int> = concurrent.copy_list_int([1, 2, 3])\n"
        "    const names: list<text> = concurrent.copy_list_text([\"ada\", \"bob\"])\n"
        "    const tags: map<text,text> = concurrent.copy_map_text_text({\"role\": \"engine\"})\n"
        "    if label == \"ready\" and len(raw) == 2 and len(ids) == 3 and len(names) == 2 and len(tags) == 1\n"
        "        return score\n"
        "    end\n"
        "    return 0\n"
        "end";
    zt_parser_result parsed;
    zt_check_result checked;

    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "boundary_copy_alpha_subset parse");
    checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "boundary_copy_alpha_subset type");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

static void test_boundary_copy_rejects_unsupported_transferable_shape(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    const char *src =
        "namespace app\n"
        "import std.concurrent as concurrent\n"
        "struct Snapshot\n"
        "    title: text\n"
        "end\n"
        "func demo() -> Snapshot\n"
        "    const value: Snapshot = Snapshot(title: \"alpha\")\n"
        "    return concurrent.copy_text(value)\n"
        "end";
    zt_parser_result parsed;
    zt_check_result checked;

    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "boundary_copy_unsupported parse");
    checked = zt_check_file(parsed.root);
    ASSERT_EQ((int)checked.diagnostics.count, 1, "boundary_copy_unsupported diag count");
    ASSERT_EQ((int)checked.diagnostics.items[0].code, ZT_DIAG_INVALID_CALL, "boundary_copy_unsupported diag code");
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

/* **Validates: Requirements 1.1, 1.2**
 * Bug Condition Exploration Test - Stdlib Function Type Mismatch Not Detected
 *
 * CRITICAL: This test MUST FAIL on unfixed code - failure confirms the bug exists.
 *
 * Property: When concurrent.copy_text(value) is called where value is of type Snapshot (struct),
 * the type checker MUST generate a diagnostic error (ZT_DIAG_INVALID_CALL or ZT_DIAG_TYPE_MISMATCH).
 *
 * Expected outcome on UNFIXED code: Test FAILS (0 diagnostics generated instead of 1)
 * This failure proves the bug exists: stdlib function type validation is not working.
 */
static void test_bug_condition_stdlib_type_mismatch_not_detected(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    const char *src =
        "namespace app\n"
        "import std.concurrent as concurrent\n"
        "struct Snapshot\n"
        "    title: text\n"
        "end\n"
        "func demo() -> text\n"
        "    const value: Snapshot = Snapshot(title: \"test\")\n"
        "    return concurrent.copy_text(value)\n"
        "end";
    zt_parser_result parsed;
    zt_check_result checked;

    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "bug_condition_stdlib_type_mismatch parse");
    checked = zt_check_file(parsed.root);

    /* This assertion SHOULD FAIL on unfixed code (actual=0, expected=1)
     * When it fails, it proves the bug exists: type checker is not validating
     * stdlib function parameter types correctly. */
    ASSERT_EQ((int)checked.diagnostics.count, 1, "bug_condition_stdlib_type_mismatch diag count");

    /* If we get here (test passed), verify it's the right diagnostic */
    if (checked.diagnostics.count > 0) {
        int has_type_error = (checked.diagnostics.items[0].code == ZT_DIAG_INVALID_CALL ||
                              checked.diagnostics.items[0].code == ZT_DIAG_TYPE_MISMATCH);
        ASSERT_EQ(has_type_error, 1, "bug_condition_stdlib_type_mismatch diag code");
    }

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
    test_high_arity_missing_argument_rejected();
    test_invalid_map_key_type_rejected();
    test_explicit_conversion_rules();
    test_to_text_builtin_rules();
    test_non_mutating_self_assignment_rejected();
    test_integer_overflow_rejected();
    test_contextual_integer_literal_arithmetic_ok();
    test_bytes_literal_and_len_ok();
    test_std_bytes_ops_ok();
    test_for_and_match_type_checking();
    test_const_collection_index_mutation_rejected();
    test_module_var_assignment_ok();
    test_module_var_assignment_type_mismatch_rejected();
    test_enum_constructor_and_match_binding_ok();
    test_enum_match_duplicate_default_rejected();
    test_enum_match_default_must_be_last_rejected();
    test_enum_match_default_warning();
    test_enum_match_non_exhaustive_rejected();
    test_transferable_builtin_shapes();
    test_transferable_user_types();
    test_boundary_copy_alpha_subset_ok();
    test_boundary_copy_rejects_unsupported_transferable_shape();
    test_bug_condition_stdlib_type_mismatch_not_detected();

    printf("Type tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
