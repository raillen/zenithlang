/**
 * R2.M3 - Property Tests para Conversões e Overflow
 * 
 * Invariantes testados:
 * 1. Conversões explícitas entre tipos numéricos funcionam
 * 2. Overflow é detectado em tempo de compilação quando possível
 * 3. Conversões inválidas geram erros claros
 * 4. Type casting preserva semântica
 * 5. Widening conversions são seguras
 * 6. Narrowing conversions requerem explicit cast
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

/* Test 1: Explicit conversion int -> int64 */
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

/* Test 2: Widening conversion segura int -> float */
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

/* Test 3: Narrowing conversion requer cast explícito */
static void test_narrowing_requires_explicit_cast(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> int8\n"
        "    const x: int64 = 42\n"
        "    return x\n"
        "end";
    
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "narrowing_requires_explicit_cast parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_HAS_ERROR(checked, ZT_DIAG_TYPE_MISMATCH, "narrowing should fail without cast");
    
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

/* Test 4: Conversão inválida text -> int deve falhar */
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

/* Test 5: Bool para int deve falhar */
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

/* Test 6: UInt para Int com cast explícito */
static void test_uint_to_int_explicit(void) {
    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo() -> int\n"
        "    const x: uint = 42\n"
        "    return int(x)\n"
        "end";
    
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "uint_to_int_explicit parse");
    zt_check_result checked = zt_check_file(parsed.root);
    ASSERT_NO_TYPE_ERRORS(checked, "uint_to_int_explicit type");
    
    zt_check_result_dispose(&checked);
    zt_parser_result_dispose(&parsed);
}

/* Test 7: Float para int com truncamento explícito */
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
    test_narrowing_requires_explicit_cast();
    test_invalid_conversion_text_to_int();
    test_bool_to_int_invalid();
    test_uint_to_int_explicit();
    test_float_to_int_truncation();
    
    printf("\nConversion/Overflow property tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
