#include "compiler/utils/arena.h"
#include "compiler/utils/string_pool.h"
#include "compiler/frontend/parser/parser.h"
#include "compiler/hir/lowering/from_ast.h"

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

#define ASSERT_STR_EQ(actual, expected, msg) do { \
    tests_run++; \
    if ((actual) != NULL && strcmp((actual), (expected)) == 0) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected %s, got %s\n", msg, (expected), (actual) != NULL ? (actual) : "<null>"); } \
} while(0)

#define ASSERT_NO_PARSE_ERRORS(result, msg) do { \
    tests_run++; \
    if ((result).error_count == 0) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: parse errors=%zu\n", msg, (result).error_count); } \
} while(0)

#define ASSERT_NO_LOWER_ERRORS(result, msg) do { \
    tests_run++; \
    if ((result).diagnostics.count == 0) { tests_passed++; } \
    else { \
        fprintf(stderr, "FAIL: %s: diagnostics=%zu\n", msg, (result).diagnostics.count); \
        for (size_t _i = 0; _i < (result).diagnostics.count; _i++) { \
            fprintf(stderr, "  diag: %s\n", (result).diagnostics.items[_i].message); \
        } \
    } \
} while(0)

static zt_hir_decl *find_func(zt_hir_module *module, const char *name) {
    size_t i;
    if (module == NULL || name == NULL) return NULL;
    for (i = 0; i < module->declarations.count; i++) {
        zt_hir_decl *decl = module->declarations.items[i];
        if (decl != NULL && decl->kind == ZT_HIR_FUNC_DECL && decl->as.func_decl.name != NULL && strcmp(decl->as.func_decl.name, name) == 0) {
            return decl;
        }
    }
    return NULL;
}

static zt_hir_stmt *first_func_stmt(zt_hir_decl *func) {
    if (func == NULL || func->kind != ZT_HIR_FUNC_DECL || func->as.func_decl.body == NULL) return NULL;
    if (func->as.func_decl.body->kind != ZT_HIR_BLOCK_STMT) return NULL;
    if (func->as.func_decl.body->as.block_stmt.statements.count == 0) return NULL;
    return func->as.func_decl.body->as.block_stmt.statements.items[0];
}

static void test_named_args_and_defaults_lower_to_ordered_call(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func sum(a: int, b: int = 2, c: int = 3) -> int\n"
        "    return a + b + c\n"
        "end\n"
        "func demo() -> int\n"
        "    return sum(1, c: 4)\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "hir_named_args parse");
    zt_hir_lower_result lowered = zt_lower_ast_to_hir(parsed.root);
    ASSERT_NO_LOWER_ERRORS(lowered, "hir_named_args lower");

    zt_hir_decl *demo = find_func(lowered.module, "demo");
    ASSERT_NOT_NULL(demo, "hir_named_args demo func");
    if (demo != NULL) {
        zt_hir_stmt *stmt = first_func_stmt(demo);
        ASSERT_NOT_NULL(stmt, "hir_named_args return stmt");
        if (stmt != NULL) {
            ASSERT_EQ(stmt->kind, ZT_HIR_RETURN_STMT, "hir_named_args stmt kind");
            ASSERT_EQ(stmt->as.return_stmt.value->kind, ZT_HIR_CALL_EXPR, "hir_named_args call kind");
            ASSERT_STR_EQ(stmt->as.return_stmt.value->as.call_expr.callee_name, "sum", "hir_named_args callee");
            ASSERT_EQ((int)stmt->as.return_stmt.value->as.call_expr.args.count, 3, "hir_named_args arg count");
            ASSERT_STR_EQ(stmt->as.return_stmt.value->as.call_expr.args.items[0]->as.int_expr.value, "1", "hir_named_args arg0");
            ASSERT_STR_EQ(stmt->as.return_stmt.value->as.call_expr.args.items[1]->as.int_expr.value, "2", "hir_named_args arg1 default");
            ASSERT_STR_EQ(stmt->as.return_stmt.value->as.call_expr.args.items[2]->as.int_expr.value, "4", "hir_named_args arg2 named");
        }
    }

    zt_hir_lower_result_dispose(&lowered);
    zt_parser_result_dispose(&parsed);
}

static void test_struct_constructor_defaults_lower_to_field_inits(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "struct Player\n"
        "    name: text\n"
        "    hp: int = 100\n"
        "end\n"
        "func demo() -> Player\n"
        "    return Player(name: \"Ayla\")\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "hir_construct parse");
    zt_hir_lower_result lowered = zt_lower_ast_to_hir(parsed.root);
    ASSERT_NO_LOWER_ERRORS(lowered, "hir_construct lower");

    zt_hir_decl *demo = find_func(lowered.module, "demo");
    ASSERT_NOT_NULL(demo, "hir_construct demo func");
    if (demo != NULL) {
        zt_hir_stmt *stmt = first_func_stmt(demo);
        ASSERT_NOT_NULL(stmt, "hir_construct return stmt");
        if (stmt != NULL) {
            zt_hir_expr *expr = stmt->as.return_stmt.value;
            ASSERT_EQ(stmt->kind, ZT_HIR_RETURN_STMT, "hir_construct stmt kind");
            ASSERT_EQ(expr->kind, ZT_HIR_CONSTRUCT_EXPR, "hir_construct expr kind");
            ASSERT_STR_EQ(expr->as.construct_expr.type_name, "Player", "hir_construct type");
            ASSERT_EQ((int)expr->as.construct_expr.fields.count, 2, "hir_construct field count");
            ASSERT_STR_EQ(expr->as.construct_expr.fields.items[0].name, "name", "hir_construct field0 name");
            ASSERT_EQ(expr->as.construct_expr.fields.items[0].value->kind, ZT_HIR_STRING_EXPR, "hir_construct field0 kind");
            ASSERT_STR_EQ(expr->as.construct_expr.fields.items[0].value->as.string_expr.value, "Ayla", "hir_construct field0 value");
            ASSERT_STR_EQ(expr->as.construct_expr.fields.items[1].name, "hp", "hir_construct field1 name");
            ASSERT_EQ(expr->as.construct_expr.fields.items[1].value->kind, ZT_HIR_INT_EXPR, "hir_construct field1 kind");
            ASSERT_STR_EQ(expr->as.construct_expr.fields.items[1].value->as.int_expr.value, "100", "hir_construct field1 default");
        }
    }

    zt_hir_lower_result_dispose(&lowered);
    zt_parser_result_dispose(&parsed);
}

static void test_apply_method_decl_captures_receiver_and_trait(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "struct Player\n"
        "    hp: int\n"
        "end\n"
        "trait Healable\n"
        "    mut func heal(amount: int)\n"
        "end\n"
        "apply Healable to Player\n"
        "    mut func heal(amount: int)\n"
        "        self.hp = self.hp + amount\n"
        "    end\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "hir_apply_decl parse");
    zt_hir_lower_result lowered = zt_lower_ast_to_hir(parsed.root);
    ASSERT_NO_LOWER_ERRORS(lowered, "hir_apply_decl lower");

    zt_hir_decl *heal = find_func(lowered.module, "Player__Healable__heal");
    ASSERT_NOT_NULL(heal, "hir_apply_decl method func");
    if (heal != NULL) {
        ASSERT_EQ(heal->kind, ZT_HIR_FUNC_DECL, "hir_apply_decl kind");
        ASSERT_EQ(heal->as.func_decl.is_mutating, 1, "hir_apply_decl mutating");
        ASSERT_STR_EQ(heal->as.func_decl.receiver_type_name, "Player", "hir_apply_decl receiver");
        ASSERT_STR_EQ(heal->as.func_decl.implemented_trait_name, "Healable", "hir_apply_decl trait");
    }

    zt_hir_lower_result_dispose(&lowered);
    zt_parser_result_dispose(&parsed);
}

static void test_method_call_and_import_call_lower_differently(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "import std.text\n"
        "struct Player\n"
        "    hp: int\n"
        "end\n"
        "apply Player\n"
        "    func heal(amount: int)\n"
        "        self.hp = self.hp + amount\n"
        "    end\n"
        "end\n"
        "func act(player: Player)\n"
        "    player.heal(10)\n"
        "end\n"
        "func trim_name(name: text) -> text\n"
        "    return text.trim(name)\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "hir_calls parse");
    zt_hir_lower_result lowered = zt_lower_ast_to_hir(parsed.root);
    ASSERT_NO_LOWER_ERRORS(lowered, "hir_calls lower");

    zt_hir_decl *act = find_func(lowered.module, "act");
    ASSERT_NOT_NULL(act, "hir_calls act func");
    if (act != NULL) {
        zt_hir_stmt *stmt = first_func_stmt(act);
        ASSERT_NOT_NULL(stmt, "hir_calls method stmt");
        if (stmt != NULL) {
            ASSERT_EQ(stmt->kind, ZT_HIR_EXPR_STMT, "hir_calls method stmt kind");
            ASSERT_EQ(stmt->as.expr_stmt.expr->kind, ZT_HIR_METHOD_CALL_EXPR, "hir_calls method expr kind");
            ASSERT_STR_EQ(stmt->as.expr_stmt.expr->as.method_call_expr.method_name, "Player__heal", "hir_calls method name");
            ASSERT_EQ(stmt->as.expr_stmt.expr->as.method_call_expr.receiver->kind, ZT_HIR_IDENT_EXPR, "hir_calls receiver kind");
            ASSERT_STR_EQ(stmt->as.expr_stmt.expr->as.method_call_expr.receiver->as.ident_expr.name, "player", "hir_calls receiver name");
            ASSERT_EQ((int)stmt->as.expr_stmt.expr->as.method_call_expr.args.count, 1, "hir_calls method arg count");
        }
    }

    zt_hir_decl *trim_name = find_func(lowered.module, "trim_name");
    ASSERT_NOT_NULL(trim_name, "hir_calls trim_name func");
    if (trim_name != NULL) {
        zt_hir_stmt *stmt = first_func_stmt(trim_name);
        ASSERT_NOT_NULL(stmt, "hir_calls import stmt");
        if (stmt != NULL) {
            ASSERT_EQ(stmt->kind, ZT_HIR_RETURN_STMT, "hir_calls import stmt kind");
            ASSERT_EQ(stmt->as.return_stmt.value->kind, ZT_HIR_CALL_EXPR, "hir_calls import expr kind");
            ASSERT_STR_EQ(stmt->as.return_stmt.value->as.call_expr.callee_name, "text.trim", "hir_calls import callee");
            ASSERT_EQ((int)stmt->as.return_stmt.value->as.call_expr.args.count, 1, "hir_calls import arg count");
        }
    }

    zt_hir_lower_result_dispose(&lowered);
    zt_parser_result_dispose(&parsed);
}

static void test_match_lowered_with_pattern_groups_and_default(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace app\n"
        "func demo(value: int) -> int\n"
        "    match value\n"
        "        case 10, 20, 30\n"
        "            return 1\n"
        "        case default\n"
        "            return 0\n"
        "    end\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_PARSE_ERRORS(parsed, "hir_match parse");
    zt_hir_lower_result lowered = zt_lower_ast_to_hir(parsed.root);
    ASSERT_NO_LOWER_ERRORS(lowered, "hir_match lower");

    zt_hir_decl *demo = find_func(lowered.module, "demo");
    ASSERT_NOT_NULL(demo, "hir_match demo func");
    if (demo != NULL) {
        zt_hir_stmt *stmt = first_func_stmt(demo);
        ASSERT_NOT_NULL(stmt, "hir_match stmt");
        if (stmt != NULL) {
            ASSERT_EQ(stmt->kind, ZT_HIR_MATCH_STMT, "hir_match stmt kind");
            ASSERT_EQ((int)stmt->as.match_stmt.cases.count, 2, "hir_match case count");
            ASSERT_EQ((int)stmt->as.match_stmt.cases.items[0].patterns.count, 3, "hir_match first case patterns");
            ASSERT_EQ(stmt->as.match_stmt.cases.items[1].is_default, 1, "hir_match default case");
        }
    }

    zt_hir_lower_result_dispose(&lowered);
    zt_parser_result_dispose(&parsed);
}

int main(void) {
    test_named_args_and_defaults_lower_to_ordered_call();
    test_struct_constructor_defaults_lower_to_field_inits();
    test_apply_method_decl_captures_receiver_and_trait();
    test_method_call_and_import_call_lower_differently();
    test_match_lowered_with_pattern_groups_and_default();

    printf("HIR lowering tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}


