#include "compiler/utils/arena.h"
#include "compiler/utils/string_pool.h"
#include "compiler/frontend/parser/parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT_EQ(actual, expected, msg) do { \
    tests_run++; \
    if ((actual) == (expected)) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected %d, got %d\n", msg, (int)(expected), (int)(actual)); } \
} while(0)

#define ASSERT_STR_EQ(actual, expected, msg) do { \
    tests_run++; \
    if ((actual) != NULL && strcmp((actual), (expected)) == 0) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected \"%s\", got \"%s\"\n", msg, (expected), (actual) ? (actual) : "(null)"); } \
} while(0)

#define ASSERT_NULL(actual, msg) do { \
    tests_run++; \
    if ((actual) == NULL) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected NULL, got %p\n", msg, (void*)(actual)); } \
} while(0)

#define ASSERT_NOT_NULL(actual, msg) do { \
    tests_run++; \
    if ((actual) != NULL) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected non-NULL\n", msg); } \
} while(0)

#define ASSERT_NO_ERRORS(result, msg) do { \
    tests_run++; \
    if ((result).diagnostics.count == 0) { tests_passed++; } \
    else { \
        fprintf(stderr, "FAIL: %s: expected 0 errors, got %zu\n", msg, (result).diagnostics.count); \
        for (size_t _i = 0; _i < (result).diagnostics.count; _i++) { \
            fprintf(stderr, "  error: %s\n", (result).diagnostics.items[_i].message); \
        } \
    } \
} while(0)

static void test_namespace_only(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app.main";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "namespace_only");
    ASSERT_NOT_NULL(r.root, "root not null");
    ASSERT_EQ(r.root->kind, ZT_AST_FILE, "root is file");
    ASSERT_STR_EQ(r.root->as.file.module_name, "app.main", "module name");
    ASSERT_EQ(r.root->as.file.imports.count, 0, "no imports");
    ASSERT_EQ(r.root->as.file.declarations.count, 0, "no decls");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_namespace_and_import(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app.user\nimport std.text\nimport app.types as t";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "namespace_and_import");
    ASSERT_NOT_NULL(r.root, "root");
    ASSERT_EQ(r.root->as.file.imports.count, 2, "2 imports");
    zt_ast_node *imp1 = r.root->as.file.imports.items[0];
    ASSERT_EQ(imp1->kind, ZT_AST_IMPORT_DECL, "import decl 1");
    ASSERT_STR_EQ(imp1->as.import_decl.path, "std.text", "import path 1");
    ASSERT_NULL(imp1->as.import_decl.alias, "no alias 1");
    zt_ast_node *imp2 = r.root->as.file.imports.items[1];
    ASSERT_STR_EQ(imp2->as.import_decl.path, "app.types", "import path 2");
    ASSERT_STR_EQ(imp2->as.import_decl.alias, "t", "import alias 2");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_simple_func(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc greet(name: text) -> text\n    return name\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "simple_func");
    ASSERT_EQ(r.root->as.file.declarations.count, 1, "1 decl");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    ASSERT_EQ(func->kind, ZT_AST_FUNC_DECL, "func decl");
    ASSERT_STR_EQ(func->as.func_decl.name, "greet", "func name");
    ASSERT_EQ(func->as.func_decl.params.count, 1, "1 param");
    ASSERT_NOT_NULL(func->as.func_decl.return_type, "return type");
    ASSERT_EQ(func->as.func_decl.return_type->kind, ZT_AST_TYPE_SIMPLE, "return type simple");
    ASSERT_STR_EQ(func->as.func_decl.return_type->as.type_simple.name, "text", "return type name");
    ASSERT_NOT_NULL(func->as.func_decl.body, "body");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_func_void(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc hello()\n    return\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "func_void");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    ASSERT_STR_EQ(func->as.func_decl.name, "hello", "func name");
    ASSERT_NULL(func->as.func_decl.return_type, "no return type");
    ASSERT_EQ(func->as.func_decl.params.count, 0, "0 params");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_public_func(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\npublic func add(a: int, b: int) -> int\n    return a + b\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "public_func");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    ASSERT_EQ(func->as.func_decl.is_public, 1, "is public");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_public_module_const(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\npublic const PI: float = 3.14";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "public_module_const");
    ASSERT_EQ(r.root->as.file.declarations.count, 1, "1 decl");
    zt_ast_node *decl = r.root->as.file.declarations.items[0];
    ASSERT_EQ(decl->kind, ZT_AST_CONST_DECL, "const decl");
    ASSERT_EQ(decl->as.const_decl.is_public, 1, "const is public");
    ASSERT_EQ(decl->as.const_decl.is_module_level, 1, "const is module-level");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_public_module_var(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\npublic var counter: int = 0";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "public_module_var");
    ASSERT_EQ(r.root->as.file.declarations.count, 1, "1 decl");
    zt_ast_node *decl = r.root->as.file.declarations.items[0];
    ASSERT_EQ(decl->kind, ZT_AST_VAR_DECL, "var decl");
    ASSERT_EQ(decl->as.var_decl.is_public, 1, "var is public");
    ASSERT_EQ(decl->as.var_decl.is_module_level, 1, "var is module-level");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_const_and_var(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc main()\n    const x: int = 10\n    var y: int = 20\n    y = y + 1\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "const_and_var");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    zt_ast_node *body = func->as.func_decl.body;
    ASSERT_EQ(body->kind, ZT_AST_BLOCK, "body is block");
    ASSERT_EQ(body->as.block.statements.count, 3, "3 stmts");
    ASSERT_EQ(body->as.block.statements.items[0]->kind, ZT_AST_CONST_DECL, "const decl");
    ASSERT_EQ(body->as.block.statements.items[1]->kind, ZT_AST_VAR_DECL, "var decl");
    ASSERT_EQ(body->as.block.statements.items[2]->kind, ZT_AST_ASSIGN_STMT, "assign stmt");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_if_else(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc check(x: int)\n    if x > 0\n        return x\n    else\n        return 0\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "if_else");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    zt_ast_node *body = func->as.func_decl.body;
    ASSERT_EQ(body->as.block.statements.count, 1, "1 stmt");
    ASSERT_EQ(body->as.block.statements.items[0]->kind, ZT_AST_IF_STMT, "if stmt");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_while_loop(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc count(n: int)\n    while n > 0\n        n = n - 1\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "while_loop");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    zt_ast_node *body = func->as.func_decl.body;
    ASSERT_EQ(body->as.block.statements.count, 1, "1 stmt");
    ASSERT_EQ(body->as.block.statements.items[0]->kind, ZT_AST_WHILE_STMT, "while stmt");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_for_loop(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc each(items: list<int>)\n    for item in items\n        return item\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "for_loop");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    zt_ast_node *body = func->as.func_decl.body;
    ASSERT_EQ(body->as.block.statements.items[0]->kind, ZT_AST_FOR_STMT, "for stmt");
    zt_ast_node *for_stmt = body->as.block.statements.items[0];
    ASSERT_STR_EQ(for_stmt->as.for_stmt.item_name, "item", "item name");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_repeat_times(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc greet()\n    repeat 5 times\n        return\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "repeat_times");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    zt_ast_node *body = func->as.func_decl.body;
    ASSERT_EQ(body->as.block.statements.items[0]->kind, ZT_AST_REPEAT_STMT, "repeat stmt");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_struct_decl(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\npublic struct Player\n    name: text\n    hp: int where it >= 0\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "struct_decl");
    zt_ast_node *s = r.root->as.file.declarations.items[0];
    ASSERT_EQ(s->kind, ZT_AST_STRUCT_DECL, "struct decl");
    ASSERT_STR_EQ(s->as.struct_decl.name, "Player", "struct name");
    ASSERT_EQ(s->as.struct_decl.fields.count, 2, "2 fields");
    ASSERT_EQ(s->as.struct_decl.is_public, 1, "is public");
    zt_ast_node *field2 = s->as.struct_decl.fields.items[1];
    ASSERT_STR_EQ(field2->as.struct_field.name, "hp", "field name");
    ASSERT_NOT_NULL(field2->as.struct_field.where_clause, "where clause present");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_trait_apply(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\npublic trait Healable<Item>\n    mut func heal(amount: int)\nend\napply Healable to Player\n    mut func heal(amount: int)\n        self.hp = self.hp + amount\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "trait_apply");
    ASSERT_EQ(r.root->as.file.declarations.count, 2, "2 decls");
    zt_ast_node *trait = r.root->as.file.declarations.items[0];
    ASSERT_EQ(trait->kind, ZT_AST_TRAIT_DECL, "trait decl");
    ASSERT_STR_EQ(trait->as.trait_decl.name, "Healable", "trait name");
    zt_ast_node *apply = r.root->as.file.declarations.items[1];
    ASSERT_EQ(apply->kind, ZT_AST_APPLY_DECL, "apply decl");
    ASSERT_STR_EQ(apply->as.apply_decl.trait_name, "Healable", "trait name in apply");
    ASSERT_STR_EQ(apply->as.apply_decl.target_name, "Player", "target name in apply");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_enum_decl(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\npublic enum ReadResult\n    Success(content: text)\n    NotFound\n    InvalidEncoding(message: text)\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "enum_decl");
    zt_ast_node *e = r.root->as.file.declarations.items[0];
    ASSERT_EQ(e->kind, ZT_AST_ENUM_DECL, "enum decl");
    ASSERT_STR_EQ(e->as.enum_decl.name, "ReadResult", "enum name");
    ASSERT_EQ(e->as.enum_decl.variants.count, 3, "3 variants");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_extern_decl(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nextern c\n    public func puts(message: text) -> int32\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "extern_decl");
    zt_ast_node *ext = r.root->as.file.declarations.items[0];
    ASSERT_EQ(ext->kind, ZT_AST_EXTERN_DECL, "extern decl");
    ASSERT_STR_EQ(ext->as.extern_decl.binding, "c", "extern binding");
    ASSERT_EQ(ext->as.extern_decl.functions.count, 1, "1 func");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_expressions(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc calc(x: int)\n    const a: int = x + 1\n    const b: int = x * 2\n    const c: int = -a\n    const d: bool = x == 1 and b != 3\n    const e: bool = not d\n    const f: text = \"hello\"\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "expressions");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    zt_ast_node *body = func->as.func_decl.body;
    ASSERT_EQ(body->as.block.statements.count, 6, "6 stmts");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_call_and_field(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc demo()\n    foo(1, 2)\n    const x: int = foo(1, 2)\n    const y: text = obj.field\n    const z: int = arr[0]\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "call_and_field");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    zt_ast_node *body = func->as.func_decl.body;
    zt_ast_node *stmt0 = body->as.block.statements.items[0];
    ASSERT_EQ(stmt0->kind, ZT_AST_EXPR_STMT, "expr stmt");
    ASSERT_EQ(stmt0->as.expr_stmt.expr->kind, ZT_AST_CALL_EXPR, "expr stmt call expr");
    zt_ast_node *stmt1 = body->as.block.statements.items[1];
    ASSERT_EQ(stmt1->kind, ZT_AST_CONST_DECL, "const 0");
    ASSERT_EQ(stmt1->as.const_decl.init_value->kind, ZT_AST_CALL_EXPR, "call expr");
    zt_ast_node *stmt2 = body->as.block.statements.items[2];
    ASSERT_EQ(stmt2->as.const_decl.init_value->kind, ZT_AST_FIELD_EXPR, "field expr");
    zt_ast_node *stmt3 = body->as.block.statements.items[3];
    ASSERT_EQ(stmt3->as.const_decl.init_value->kind, ZT_AST_INDEX_EXPR, "index expr");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_binary_precedence(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc calc()\n    const x: int = 1 + 2 * 3\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "binary_precedence");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    zt_ast_node *body = func->as.func_decl.body;
    zt_ast_node *stmt = body->as.block.statements.items[0];
    ASSERT_EQ(stmt->kind, ZT_AST_CONST_DECL, "const decl");
    zt_ast_node *expr = stmt->as.const_decl.init_value;
    ASSERT_EQ(expr->kind, ZT_AST_BINARY_EXPR, "binary expr");
    ASSERT_EQ(expr->as.binary_expr.op, ZT_TOKEN_PLUS, "top is +");
    ASSERT_NOT_NULL(expr->as.binary_expr.left, "left not null");
    ASSERT_NOT_NULL(expr->as.binary_expr.right, "right not null");
    ASSERT_EQ(expr->as.binary_expr.right->kind, ZT_AST_BINARY_EXPR, "right is binary");
    ASSERT_EQ(expr->as.binary_expr.right->as.binary_expr.op, ZT_TOKEN_STAR, "right op is *");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_match_statement(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc classify(x: int)\n    match x\n        case 0:\n            return 0\n        case else:\n            return 1\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "match_statement");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    zt_ast_node *match_stmt = func->as.func_decl.body->as.block.statements.items[0];
    ASSERT_EQ(match_stmt->kind, ZT_AST_MATCH_STMT, "match stmt kind");
    ASSERT_EQ(match_stmt->as.match_stmt.cases.count, 2, "match case count");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_optional_result(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc find(id: int) -> optional<text>\n    if id == 0\n        return none\n    end\n    return \"found\"\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "optional_result");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    ASSERT_NOT_NULL(func->as.func_decl.return_type, "return type exists");
    ASSERT_EQ(func->as.func_decl.return_type->kind, ZT_AST_TYPE_GENERIC, "generic return type");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_success_error(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc parse_port(source: text) -> result<int, text>\n    if source == \"\"\n        return error(\"missing\")\n    end\n    return success(8080)\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "success_error");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    zt_ast_node *if_stmt = func->as.func_decl.body->as.block.statements.items[0];
    zt_ast_node *if_return = if_stmt->as.if_stmt.then_block->as.block.statements.items[0];
    zt_ast_node *final_return = func->as.func_decl.body->as.block.statements.items[1];
    ASSERT_EQ(if_return->as.return_stmt.value->kind, ZT_AST_ERROR_EXPR, "error expr preserved");
    ASSERT_EQ(final_return->as.return_stmt.value->kind, ZT_AST_SUCCESS_EXPR, "success expr preserved");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_list_literal(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc main()\n    const ports: list<int> = [3000, 3001]\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "list_literal");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    zt_ast_node *body = func->as.func_decl.body;
    zt_ast_node *stmt = body->as.block.statements.items[0];
    ASSERT_EQ(stmt->as.const_decl.init_value->kind, ZT_AST_LIST_EXPR, "list expr");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_map_literal(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc main()\n    const m: map<text, text> = {\"name\": \"Zenith\"}\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "map_literal");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    zt_ast_node *body = func->as.func_decl.body;
    zt_ast_node *stmt = body->as.block.statements.items[0];
    ASSERT_EQ(stmt->as.const_decl.init_value->kind, ZT_AST_MAP_EXPR, "map expr");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_generic_type(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc example(x: result<int, text>, y: optional<text>)\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "generic_type");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    ASSERT_EQ(func->as.func_decl.params.count, 2, "2 params");
    zt_ast_node *p1 = func->as.func_decl.params.items[0];
    ASSERT_EQ(p1->as.param.type_node->kind, ZT_AST_TYPE_GENERIC, "param 1 generic");
    ASSERT_STR_EQ(p1->as.param.type_node->as.type_generic.name, "result", "result type");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_for_with_index(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc each(items: list<int>)\n    for key, value in items\n        return\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "for_with_index");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    zt_ast_node *for_stmt = func->as.func_decl.body->as.block.statements.items[0];
    ASSERT_STR_EQ(for_stmt->as.for_stmt.item_name, "key", "item name");
    ASSERT_NOT_NULL(for_stmt->as.for_stmt.index_name, "index name present");
    ASSERT_STR_EQ(for_stmt->as.for_stmt.index_name, "value", "index name value");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_else_if(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc classify(x: int) -> text\n    if x > 0\n        return \"positive\"\n    else if x < 0\n        return \"negative\"\n    else\n        return \"zero\"\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "else_if");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_void_type(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nextern host\n    public func read_file(path: text) -> result<void, text>\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "void_type");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_func_with_default_param(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc open_file(path: text, mode: text = \"read\") -> result<void, text>\n    return success(none)\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "func_with_default_param");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    ASSERT_EQ(func->as.func_decl.params.count, 2, "2 params");
    ASSERT_NULL(func->as.func_decl.params.items[0]->as.param.default_value, "param 0 no default");
    ASSERT_NOT_NULL(func->as.func_decl.params.items[1]->as.param.default_value, "param 1 has default");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}


static void test_param_and_named_arg_with_to_keyword(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc emit(value: text, to: bool = false)\n    if to\n        return\n    end\n    return\nend\nfunc main()\n    emit(\"ok\", to: true)\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "param_named_arg_to_keyword");

    zt_ast_node *emit_func = r.root->as.file.declarations.items[0];
    ASSERT_EQ(emit_func->as.func_decl.params.count, 2, "emit has 2 params");
    ASSERT_STR_EQ(emit_func->as.func_decl.params.items[1]->as.param.name, "to", "param name is to");

    zt_ast_node *main_func = r.root->as.file.declarations.items[1];
    zt_ast_node *stmt = main_func->as.func_decl.body->as.block.statements.items[0];
    ASSERT_EQ(stmt->kind, ZT_AST_EXPR_STMT, "main statement is expr");
    ASSERT_EQ(stmt->as.expr_stmt.expr->kind, ZT_AST_CALL_EXPR, "main expression is call");
    ASSERT_EQ(stmt->as.expr_stmt.expr->as.call_expr.named_args.count, 1, "call has 1 named arg");
    ASSERT_STR_EQ(stmt->as.expr_stmt.expr->as.call_expr.named_args.items[0].name, "to", "named arg label is to");

    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_question_propagation(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc demo(x: optional<text>)\n    const y: text = x?\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "question_propagation");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_qualified_type_name(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc demo(user: user_types.User) -> user_types.Profile\n    return user\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "qualified_type_name");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    ASSERT_STR_EQ(func->as.func_decl.params.items[0]->as.param.type_node->as.type_simple.name, "user_types.User", "qualified param type");
    ASSERT_STR_EQ(func->as.func_decl.return_type->as.type_simple.name, "user_types.Profile", "qualified return type");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_inherent_apply(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\napply Player\n    func is_alive() -> bool\n        return self.hp > 0\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "inherent_apply");
    zt_ast_node *apply = r.root->as.file.declarations.items[0];
    ASSERT_EQ(apply->kind, ZT_AST_APPLY_DECL, "apply decl");
    ASSERT_NULL(apply->as.apply_decl.trait_name, "no trait name for inherent apply");
    ASSERT_STR_EQ(apply->as.apply_decl.target_name, "Player", "target name for inherent apply");
    ASSERT_EQ(apply->as.apply_decl.methods.count, 1, "1 inherent method");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_func_generic_constraints(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\npublic func render<Item>(value: Item) -> text\nwhere Item is TextRepresentable<Item>\n    return value.to_text()\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "func_generic_constraints");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    ASSERT_EQ(func->as.func_decl.type_params.count, 1, "func has 1 type param");
    ASSERT_EQ(func->as.func_decl.constraints.count, 1, "func has 1 constraint");
    ASSERT_EQ(func->as.func_decl.constraints.items[0]->kind, ZT_AST_GENERIC_CONSTRAINT, "func constraint kind");
    ASSERT_STR_EQ(func->as.func_decl.constraints.items[0]->as.generic_constraint.type_param_name, "Item", "func constraint lhs");
    ASSERT_EQ(func->as.func_decl.constraints.items[0]->as.generic_constraint.trait_type->kind, ZT_AST_TYPE_GENERIC, "func constraint rhs kind");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_struct_generic_constraints(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\npublic struct Cache<Key, Value>\nwhere Key is Hashable<Key> and Key is Equatable<Key>\n    items: map<Key, Value>\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "struct_generic_constraints");
    zt_ast_node *decl = r.root->as.file.declarations.items[0];
    ASSERT_EQ(decl->as.struct_decl.constraints.count, 2, "struct has 2 constraints");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_trait_generic_constraints(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\npublic trait Reader<Item>\nwhere Item is TextRepresentable<Item>\n    func render(value: Item) -> text\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "trait_generic_constraints");
    zt_ast_node *decl = r.root->as.file.declarations.items[0];
    ASSERT_EQ(decl->as.trait_decl.constraints.count, 1, "trait has 1 constraint");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_enum_generic_constraints(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\npublic enum Boxed<Item>\nwhere Item is TextRepresentable<Item>\n    Some(value: Item)\n    None\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "enum_generic_constraints");
    zt_ast_node *decl = r.root->as.file.declarations.items[0];
    ASSERT_EQ(decl->as.enum_decl.constraints.count, 1, "enum has 1 constraint");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_apply_generic_constraints(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\napply Box<Item>\nwhere Item is TextRepresentable<Item>\n    public func render() -> text\n        return self.value.to_text()\n    end\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "apply_generic_constraints");
    zt_ast_node *decl = r.root->as.file.declarations.items[0];
    ASSERT_NULL(decl->as.apply_decl.trait_name, "apply has no trait");
    ASSERT_EQ(decl->as.apply_decl.target_type_params.count, 1, "apply has 1 target type param");
    ASSERT_EQ(decl->as.apply_decl.constraints.count, 1, "apply has 1 constraint");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_hex_bytes_literal(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc demo()\n    const data: bytes = hex bytes \"DE AD_be ef\"\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "hex_bytes_literal");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    zt_ast_node *stmt = func->as.func_decl.body->as.block.statements.items[0];
    ASSERT_EQ(stmt->as.const_decl.init_value->kind, ZT_AST_BYTES_EXPR, "bytes expr kind");
    ASSERT_STR_EQ(stmt->as.const_decl.init_value->as.bytes_expr.value, "DEADBEEF", "normalized hex bytes");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_hex_bytes_literal_rejects_odd_digits(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nfunc demo()\n    const data: bytes = hex bytes \"ABC\"\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_EQ((int)r.diagnostics.count, 1, "hex bytes odd digit error");
    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

static void test_attr_test_on_func(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src = "namespace app\nattr test\nfunc smoke()\n    return\nend";
    zt_parser_result r = zt_parse(&test_arena, &test_pool, "test", src, strlen(src));
    ASSERT_NO_ERRORS(r, "attr_test_on_func");
    ASSERT_EQ(r.root->as.file.declarations.count, 1, "1 decl");
    zt_ast_node *func = r.root->as.file.declarations.items[0];
    ASSERT_EQ(func->kind, ZT_AST_FUNC_DECL, "func decl");
    ASSERT_EQ(func->as.func_decl.is_test, 1, "func marked as test");

    zt_parser_result_dispose(&r);
    zt_arena_dispose(&test_arena);
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    printf("Running Zenith Parser Tests...\n");

    test_namespace_only();
    test_namespace_and_import();
    test_simple_func();
    test_func_void();
    test_public_func();
    test_public_module_const();
    test_public_module_var();
    test_const_and_var();
    test_if_else();
    test_while_loop();
    test_for_loop();
    test_repeat_times();
    test_struct_decl();
    test_trait_apply();
    test_enum_decl();
    test_extern_decl();
    test_expressions();
    test_call_and_field();
    test_binary_precedence();
    test_match_statement();
    test_optional_result();
    test_success_error();
    test_list_literal();
    test_map_literal();
    test_generic_type();
    test_for_with_index();
    test_else_if();
    test_void_type();
    test_func_with_default_param();
    test_param_and_named_arg_with_to_keyword();
    test_attr_test_on_func();
    test_question_propagation();
    test_qualified_type_name();
    test_inherent_apply();
    test_func_generic_constraints();
    test_struct_generic_constraints();
    test_trait_generic_constraints();
    test_enum_generic_constraints();
    test_apply_generic_constraints();
    test_hex_bytes_literal();
    test_hex_bytes_literal_rejects_odd_digits();

    printf("Parser tests: %d/%d passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}


