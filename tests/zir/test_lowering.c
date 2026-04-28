#include "compiler/utils/arena.h"
#include "compiler/utils/string_pool.h"
#include "compiler/frontend/parser/parser.h"
#include "compiler/hir/lowering/from_ast.h"
#include "compiler/zir/lowering/from_hir.h"
#include "compiler/zir/printer.h"
#include "compiler/zir/verifier.h"

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

#define ASSERT_EQ_INT(actual, expected, msg) do { \
    tests_run++; \
    if ((int)(actual) == (int)(expected)) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected %d, got %d\n", msg, (int)(expected), (int)(actual)); } \
} while (0)

#define ASSERT_EQ_SIZE(actual, expected, msg) do { \
    tests_run++; \
    if ((size_t)(actual) == (size_t)(expected)) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected %zu, got %zu\n", msg, (size_t)(expected), (size_t)(actual)); } \
} while (0)

#define ASSERT_STR_EQ(actual, expected, msg) do { \
    tests_run++; \
    if ((actual) != NULL && strcmp((actual), (expected)) == 0) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected %s, got %s\n", msg, (expected), (actual) != NULL ? (actual) : "<null>"); } \
} while (0)

#define ASSERT_NOT_NULL(actual, msg) do { \
    tests_run++; \
    if ((actual) != NULL) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected non-NULL\n", msg); } \
} while (0)

static int assert_no_parse_errors(const char *name, const zt_parser_result *result) {
    size_t i;
    tests_run++;
    if (result->error_count == 0) {
        tests_passed++;
        return 1;
    }

    fprintf(stderr, "FAIL: %s: parse errors=%zu\n", name, result->error_count);
    for (i = 0; i < result->error_count; i++) {
        fprintf(stderr, "  parse: %s\n", result->errors[i].message);
    }
    return 0;
}

static int assert_no_hir_errors(const char *name, const zt_hir_lower_result *result) {
    size_t i;
    tests_run++;
    if (result->diagnostics.count == 0) {
        tests_passed++;
        return 1;
    }

    fprintf(stderr, "FAIL: %s: HIR diagnostics=%zu\n", name, result->diagnostics.count);
    for (i = 0; i < result->diagnostics.count; i++) {
        fprintf(stderr, "  hir: %s\n", result->diagnostics.items[i].message);
    }
    return 0;
}

static int assert_no_zir_errors(const char *name, const zir_lower_result *result) {
    size_t i;
    tests_run++;
    if (result->diagnostics.count == 0) {
        tests_passed++;
        return 1;
    }

    fprintf(stderr, "FAIL: %s: ZIR diagnostics=%zu\n", name, result->diagnostics.count);
    for (i = 0; i < result->diagnostics.count; i++) {
        fprintf(stderr, "  zir: %s\n", result->diagnostics.items[i].message);
    }
    return 0;
}

static int assert_zir_verifies(const char *name, const zir_module *module_decl) {
    zir_verifier_result verifier;
    tests_run++;
    if (zir_verify_module(module_decl, &verifier)) {
        tests_passed++;
        return 1;
    }

    fprintf(stderr, "FAIL: %s: verifier=%s: %s\n", name, zir_verifier_code_name(verifier.code), verifier.message);
    return 0;
}

static int assert_print_contains(const char *name, const zir_module *module_decl, const char *fragment) {
    zir_printer printer;
    const char *actual;
    int ok;

    zir_printer_init(&printer);
    tests_run++;
    ok = zir_printer_print_module(&printer, module_decl);
    actual = zir_printer_text(&printer);
    if (ok && actual != NULL && strstr(actual, fragment) != NULL) {
        tests_passed++;
        zir_printer_dispose(&printer);
        return 1;
    }

    fprintf(stderr, "FAIL: %s: printer output missing fragment: %s\n", name, fragment);
    fprintf(stderr, "received:\n%s\n", actual != NULL ? actual : "<null>");
    zir_printer_dispose(&printer);
    return 0;
}

static void test_binary_expression_lowering(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func add(a: int, b: int) -> int\n"
        "    const total: int = a + b\n"
        "    return total\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test.zt", src, strlen(src));
    zt_hir_lower_result hir;
    zir_lower_result zir;
    const zir_function *function_decl;
    const zir_block *entry;
    const zir_instruction *instruction;

    if (!assert_no_parse_errors("zir_binary parse", &parsed)) {
        zt_parser_result_dispose(&parsed);
        return;
    }

    hir = zt_lower_ast_to_hir(parsed.root);
    if (!assert_no_hir_errors("zir_binary hir", &hir)) {
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        return;
    }

    zir = zir_lower_hir_to_zir(hir.module);
    if (!assert_no_zir_errors("zir_binary lower", &zir)) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        return;
    }

    ASSERT_STR_EQ(zir.module.name, "app", "zir_binary module name");
    ASSERT_EQ_SIZE(zir.module.function_count, 1, "zir_binary function count");
    function_decl = &zir.module.functions[0];
    ASSERT_STR_EQ(function_decl->name, "add", "zir_binary function name");
    ASSERT_EQ_SIZE(function_decl->block_count, 1, "zir_binary block count");

    entry = &function_decl->blocks[0];
    ASSERT_STR_EQ(entry->label, "entry", "zir_binary block label");
    ASSERT_EQ_SIZE(entry->instruction_count, 1, "zir_binary instruction count");

    instruction = &entry->instructions[0];
    ASSERT_EQ_INT(instruction->kind, ZIR_INSTR_ASSIGN, "zir_binary instruction kind");
    ASSERT_STR_EQ(instruction->dest_name, "total", "zir_binary destination");
    ASSERT_NOT_NULL(instruction->expr, "zir_binary structured expr");
    if (instruction->expr != NULL) {
        ASSERT_EQ_INT(instruction->expr->kind, ZIR_EXPR_BINARY, "zir_binary expr kind");
        ASSERT_STR_EQ(instruction->expr->as.binary.op_name, "add", "zir_binary op");
        ASSERT_EQ_INT(instruction->expr->as.binary.left->kind, ZIR_EXPR_NAME, "zir_binary left kind");
        ASSERT_STR_EQ(instruction->expr->as.binary.left->as.text.text, "a", "zir_binary left");
        ASSERT_STR_EQ(instruction->expr->as.binary.right->as.text.text, "b", "zir_binary right");
    }

    ASSERT_EQ_INT(entry->terminator.kind, ZIR_TERM_RETURN, "zir_binary terminator kind");
    ASSERT_NOT_NULL(entry->terminator.value, "zir_binary return expr");
    if (entry->terminator.value != NULL) {
        ASSERT_EQ_INT(entry->terminator.value->kind, ZIR_EXPR_NAME, "zir_binary return kind");
        ASSERT_STR_EQ(entry->terminator.value->as.text.text, "total", "zir_binary return value");
    }

    assert_zir_verifies("zir_binary verify", &zir.module);
    assert_print_contains("zir_binary print assign", &zir.module, "total: int = binary.add a, b");
    assert_print_contains("zir_binary print return", &zir.module, "return total");

    zir_lower_result_dispose(&zir);
    zt_hir_lower_result_dispose(&hir);
    zt_parser_result_dispose(&parsed);
}

static void test_if_else_lowering(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func choose(flag: bool) -> int\n"
        "    if flag\n"
        "        return 1\n"
        "    else\n"
        "        return 2\n"
        "    end\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test.zt", src, strlen(src));
    zt_hir_lower_result hir;
    zir_lower_result zir;
    const zir_function *function_decl;
    const zir_block *entry;

    if (!assert_no_parse_errors("zir_if parse", &parsed)) {
        zt_parser_result_dispose(&parsed);
        return;
    }

    hir = zt_lower_ast_to_hir(parsed.root);
    if (!assert_no_hir_errors("zir_if hir", &hir)) {
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        return;
    }

    zir = zir_lower_hir_to_zir(hir.module);
    if (!assert_no_zir_errors("zir_if lower", &zir)) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        return;
    }

    function_decl = &zir.module.functions[0];
    ASSERT_EQ_SIZE(function_decl->block_count, 4, "zir_if block count");
    entry = &function_decl->blocks[0];
    ASSERT_EQ_INT(entry->terminator.kind, ZIR_TERM_BRANCH_IF, "zir_if entry terminator");
    ASSERT_NOT_NULL(entry->terminator.condition, "zir_if structured condition");
    if (entry->terminator.condition != NULL) {
        ASSERT_EQ_INT(entry->terminator.condition->kind, ZIR_EXPR_NAME, "zir_if condition kind");
        ASSERT_STR_EQ(entry->terminator.condition->as.text.text, "flag", "zir_if condition name");
    }
    ASSERT_STR_EQ(entry->terminator.then_label, "if_then_0", "zir_if then label");
    ASSERT_STR_EQ(entry->terminator.else_label, "if_else_1", "zir_if else label");
    ASSERT_EQ_INT(function_decl->blocks[1].terminator.kind, ZIR_TERM_RETURN, "zir_if then returns");
    ASSERT_EQ_INT(function_decl->blocks[2].terminator.kind, ZIR_TERM_RETURN, "zir_if else returns");
    ASSERT_EQ_INT(function_decl->blocks[3].terminator.kind, ZIR_TERM_UNREACHABLE, "zir_if join unreachable");

    assert_zir_verifies("zir_if verify", &zir.module);
    assert_print_contains("zir_if print branch", &zir.module, "branch_if flag, if_then_0, if_else_1");

    zir_lower_result_dispose(&zir);
    zt_hir_lower_result_dispose(&hir);
    zt_parser_result_dispose(&parsed);
}

static void test_struct_constructor_lowering(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "struct Player\n"
        "    name: text\n"
        "    hp: int = 100\n"
        "end\n"
        "func make_player() -> Player\n"
        "    return Player(name: \"Ayla\")\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test.zt", src, strlen(src));
    zt_hir_lower_result hir;
    zir_lower_result zir;
    const zir_expr *expr;

    if (!assert_no_parse_errors("zir_construct parse", &parsed)) {
        zt_parser_result_dispose(&parsed);
        return;
    }

    hir = zt_lower_ast_to_hir(parsed.root);
    if (!assert_no_hir_errors("zir_construct hir", &hir)) {
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        return;
    }

    zir = zir_lower_hir_to_zir(hir.module);
    if (!assert_no_zir_errors("zir_construct lower", &zir)) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        return;
    }

    ASSERT_EQ_SIZE(zir.module.function_count, 1, "zir_construct function count");
    expr = zir.module.functions[0].blocks[0].terminator.value;
    ASSERT_NOT_NULL(expr, "zir_construct return expr");
    if (expr != NULL) {
        ASSERT_EQ_INT(expr->kind, ZIR_EXPR_MAKE_STRUCT, "zir_construct expr kind");
        ASSERT_STR_EQ(expr->as.make_struct.type_name, "Player", "zir_construct type");
        ASSERT_EQ_SIZE(expr->as.make_struct.fields.count, 2, "zir_construct field count");
        ASSERT_STR_EQ(expr->as.make_struct.fields.items[0].name, "name", "zir_construct field0");
        ASSERT_EQ_INT(expr->as.make_struct.fields.items[0].value->kind, ZIR_EXPR_STRING, "zir_construct field0 kind");
        ASSERT_STR_EQ(expr->as.make_struct.fields.items[0].value->as.text.text, "Ayla", "zir_construct field0 value");
        ASSERT_STR_EQ(expr->as.make_struct.fields.items[1].name, "hp", "zir_construct field1");
        ASSERT_EQ_INT(expr->as.make_struct.fields.items[1].value->kind, ZIR_EXPR_INT, "zir_construct field1 kind");
        ASSERT_STR_EQ(expr->as.make_struct.fields.items[1].value->as.text.text, "100", "zir_construct field1 value");
    }

    assert_zir_verifies("zir_construct verify", &zir.module);
    assert_print_contains("zir_construct print", &zir.module, "return make_struct Player { name: const \"Ayla\", hp: 100 }");

    zir_lower_result_dispose(&zir);
    zt_hir_lower_result_dispose(&hir);
    zt_parser_result_dispose(&parsed);
}

static void test_while_lowering(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func countdown(n: int) -> int\n"
        "    var current: int = n\n"
        "    while current > 0\n"
        "        current = current - 1\n"
        "    end\n"
        "    return current\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test.zt", src, strlen(src));
    zt_hir_lower_result hir;
    zir_lower_result zir;
    const zir_function *function_decl;

    if (!assert_no_parse_errors("zir_while parse", &parsed)) {
        zt_parser_result_dispose(&parsed);
        return;
    }

    hir = zt_lower_ast_to_hir(parsed.root);
    if (!assert_no_hir_errors("zir_while hir", &hir)) {
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        return;
    }

    zir = zir_lower_hir_to_zir(hir.module);
    if (!assert_no_zir_errors("zir_while lower", &zir)) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        return;
    }

    function_decl = &zir.module.functions[0];
    ASSERT_EQ_SIZE(function_decl->block_count, 4, "zir_while block count");
    ASSERT_EQ_INT(function_decl->blocks[0].terminator.kind, ZIR_TERM_JUMP, "zir_while entry jumps");
    ASSERT_STR_EQ(function_decl->blocks[1].label, "while_cond_0", "zir_while cond label");
    ASSERT_EQ_INT(function_decl->blocks[1].terminator.kind, ZIR_TERM_BRANCH_IF, "zir_while cond branch");
    ASSERT_STR_EQ(function_decl->blocks[2].label, "while_body_1", "zir_while body label");
    ASSERT_EQ_INT(function_decl->blocks[2].terminator.kind, ZIR_TERM_JUMP, "zir_while body jumps");
    ASSERT_STR_EQ(function_decl->blocks[3].label, "while_exit_2", "zir_while exit label");
    ASSERT_EQ_INT(function_decl->blocks[3].terminator.kind, ZIR_TERM_RETURN, "zir_while exit returns");

    assert_zir_verifies("zir_while verify", &zir.module);
    assert_print_contains("zir_while print", &zir.module, "branch_if binary.gt current, 0, while_body_1, while_exit_2");

    zir_lower_result_dispose(&zir);
    zt_hir_lower_result_dispose(&hir);
    zt_parser_result_dispose(&parsed);
}

static void test_repeat_lowering(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func accumulate() -> int\n"
        "    var total: int = 0\n"
        "    repeat 3 times\n"
        "        total = total + 2\n"
        "    end\n"
        "    return total\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test.zt", src, strlen(src));
    zt_hir_lower_result hir;
    zir_lower_result zir;
    const zir_function *function_decl;

    if (!assert_no_parse_errors("zir_repeat parse", &parsed)) {
        zt_parser_result_dispose(&parsed);
        return;
    }

    hir = zt_lower_ast_to_hir(parsed.root);
    if (!assert_no_hir_errors("zir_repeat hir", &hir)) {
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        return;
    }

    zir = zir_lower_hir_to_zir(hir.module);
    if (!assert_no_zir_errors("zir_repeat lower", &zir)) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        return;
    }

    function_decl = &zir.module.functions[0];
    ASSERT_EQ_SIZE(function_decl->block_count, 5, "zir_repeat block count");
    ASSERT_EQ_SIZE(function_decl->blocks[0].instruction_count, 3, "zir_repeat entry instruction count");
    ASSERT_EQ_INT(function_decl->blocks[0].terminator.kind, ZIR_TERM_JUMP, "zir_repeat entry jumps");
    ASSERT_EQ_INT(function_decl->blocks[1].terminator.kind, ZIR_TERM_BRANCH_IF, "zir_repeat cond branch");
    ASSERT_EQ_INT(function_decl->blocks[2].terminator.kind, ZIR_TERM_JUMP, "zir_repeat body jumps");
    ASSERT_EQ_INT(function_decl->blocks[3].terminator.kind, ZIR_TERM_JUMP, "zir_repeat step jumps");
    ASSERT_EQ_INT(function_decl->blocks[4].terminator.kind, ZIR_TERM_RETURN, "zir_repeat exit returns");

    assert_zir_verifies("zir_repeat verify", &zir.module);
    assert_print_contains("zir_repeat print", &zir.module, "__zt_repeat_count_0: int = 3");

    zir_lower_result_dispose(&zir);
    zt_hir_lower_result_dispose(&hir);
    zt_parser_result_dispose(&parsed);
}

static void test_match_lowering(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func classify(value: int) -> int\n"
        "    match value\n"
        "        case 10, 20, 30:\n"
        "            return 1\n"
        "        case else:\n"
        "            return 0\n"
        "    end\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test.zt", src, strlen(src));
    zt_hir_lower_result hir;
    zir_lower_result zir;
    const zir_function *function_decl;

    if (!assert_no_parse_errors("zir_match parse", &parsed)) {
        zt_parser_result_dispose(&parsed);
        return;
    }

    hir = zt_lower_ast_to_hir(parsed.root);
    if (!assert_no_hir_errors("zir_match hir", &hir)) {
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        return;
    }

    zir = zir_lower_hir_to_zir(hir.module);
    if (!assert_no_zir_errors("zir_match lower", &zir)) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        return;
    }

    function_decl = &zir.module.functions[0];
    ASSERT_EQ_SIZE(function_decl->block_count, 5, "zir_match block count");
    ASSERT_EQ_SIZE(function_decl->blocks[0].instruction_count, 1, "zir_match subject temp");
    ASSERT_EQ_INT(function_decl->blocks[1].terminator.kind, ZIR_TERM_BRANCH_IF, "zir_match first case branches");
    ASSERT_EQ_INT(function_decl->blocks[2].terminator.kind, ZIR_TERM_RETURN, "zir_match first case returns");
    ASSERT_EQ_INT(function_decl->blocks[3].terminator.kind, ZIR_TERM_RETURN, "zir_match default returns");

    assert_zir_verifies("zir_match verify", &zir.module);
    assert_print_contains("zir_match print", &zir.module, "binary.or binary.or binary.eq __zt_match_subject_0, 10, binary.eq __zt_match_subject_0, 20, binary.eq __zt_match_subject_0, 30");

    zir_lower_result_dispose(&zir);
    zt_hir_lower_result_dispose(&hir);
    zt_parser_result_dispose(&parsed);
}

int main(void) {
    test_binary_expression_lowering();
    test_if_else_lowering();
    test_struct_constructor_lowering();
    test_while_lowering();
    test_repeat_lowering();
    test_match_lowering();

    printf("ZIR lowering tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}

