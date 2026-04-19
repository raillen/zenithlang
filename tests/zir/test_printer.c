#include "compiler/zir/model.h"
#include "compiler/zir/printer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_COUNT(items) (sizeof(items) / sizeof((items)[0]))

static void assert_rendered(const char *name, const zir_module *module_decl, const char *expected) {
    zir_printer printer;
    zir_printer_init(&printer);

    if (!zir_printer_print_module(&printer, module_decl)) {
        fprintf(stderr, "falha ao renderizar %s\n", name);
        zir_printer_dispose(&printer);
        exit(1);
    }

    const char *actual = zir_printer_text(&printer);
    if (strcmp(actual, expected) != 0) {
        fprintf(stderr, "falha no teste %s\n", name);
        fprintf(stderr, "esperado:\n%s\n", expected);
        fprintf(stderr, "recebido:\n%s\n", actual);
        zir_printer_dispose(&printer);
        exit(1);
    }

    zir_printer_dispose(&printer);
}

static void test_add(void) {
    const zir_param params[] = {
        zir_make_param("a", "int"),
        zir_make_param("b", "int"),
    };
    const zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "int", "binary.add a, b"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t0")),
    };
    const zir_function functions[] = {
        zir_make_function("add", params, ARRAY_COUNT(params), "int", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_rendered(
        "add.zir",
        &module_decl,
        "module main\n\nfunc add(a: int, b: int) -> int\n\nblock entry:\n  t0: int = binary.add a, b\n  return t0"
    );
}

static void test_if_else(void) {
    const zir_param params[] = {
        zir_make_param("x", "int"),
    };
    const zir_instruction entry_instructions[] = {
        zir_make_assign_instruction("t0", "bool", "binary.lt x, 0"),
    };
    const zir_instruction negative_instructions[] = {
        zir_make_assign_instruction("t1", "int", "unary.neg x"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", entry_instructions, ARRAY_COUNT(entry_instructions), zir_make_branch_if_terminator("t0", "negative", "positive")),
        zir_make_block("negative", negative_instructions, ARRAY_COUNT(negative_instructions), zir_make_return_terminator("t1")),
        zir_make_block("positive", NULL, 0, zir_make_return_terminator("x")),
    };
    const zir_function functions[] = {
        zir_make_function("abs", params, ARRAY_COUNT(params), "int", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_rendered(
        "if_else.zir",
        &module_decl,
        "module main\n\nfunc abs(x: int) -> int\n\nblock entry:\n  t0: bool = binary.lt x, 0\n  branch_if t0, negative, positive\n\nblock negative:\n  t1: int = unary.neg x\n  return t1\n\nblock positive:\n  return x"
    );
}

static void test_extern_call(void) {
    const zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "text", "const \"hello\""),
        zir_make_assign_instruction("t1", "int", "call_extern c.puts(t0)"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t1")),
    };
    const zir_function functions[] = {
        zir_make_function("main", NULL, 0, "int", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_rendered(
        "extern_c_call.zir",
        &module_decl,
        "module main\n\nfunc main() -> int\n\nblock entry:\n  t0: text = const \"hello\"\n  t1: int = call_extern c.puts(t0)\n  return t1"
    );
}

static void test_effect_and_jump(void) {
    const zir_param params[] = {
        zir_make_param("point", "struct Point"),
        zir_make_param("value", "int"),
    };
    const zir_instruction entry_instructions[] = {
        zir_make_effect_instruction("set_field point, x, value"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", entry_instructions, ARRAY_COUNT(entry_instructions), zir_make_jump_terminator("exit")),
        zir_make_block("exit", NULL, 0, zir_make_unreachable_terminator()),
    };
    const zir_function functions[] = {
        zir_make_function("mutate", params, ARRAY_COUNT(params), "void", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_rendered(
        "effect_and_jump.zir",
        &module_decl,
        "module main\n\nfunc mutate(point: struct Point, value: int) -> void\n\nblock entry:\n  set_field point, x, value\n  jump exit\n\nblock exit:\n  unreachable"
    );
}


static void test_structured_expression_printer(void) {
    const zir_param params[] = {
        zir_make_param("a", "int"),
        zir_make_param("b", "int"),
    };
    const zir_instruction instructions[] = {
        zir_make_assign_instruction_expr(
            "t0",
            "int",
            zir_expr_make_binary("add", zir_expr_make_name("a"), zir_expr_make_name("b"))
        ),
    };
    const zir_block blocks[] = {
        zir_make_block(
            "entry",
            instructions,
            ARRAY_COUNT(instructions),
            zir_make_return_terminator_expr(zir_expr_make_name("t0"))
        ),
    };
    const zir_function functions[] = {
        zir_make_function("structured_add", params, ARRAY_COUNT(params), "int", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_rendered(
        "structured_expression.zir",
        &module_decl,
        "module main\n\nfunc structured_add(a: int, b: int) -> int\n\nblock entry:\n  t0: int = binary.add a, b\n  return t0"
    );
}

static void test_spans(void) {
    zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "int", "copy value"),
    };
    zir_terminator terminator = zir_make_return_terminator("t0");
    zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), terminator),
    };
    zir_param params[] = {
        zir_make_param("value", "int"),
    };
    zir_function functions[] = {
        zir_make_function("echo", params, ARRAY_COUNT(params), "int", blocks, ARRAY_COUNT(blocks)),
    };
    zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    instructions[0].span = zir_make_span("sample.zt", 3, 3);
    blocks[0].terminator.span = zir_make_span("sample.zt", 4, 3);
    blocks[0].span = zir_make_span("sample.zt", 2, 1);
    functions[0].span = zir_make_span("sample.zt", 1, 1);

    assert_rendered(
        "spans.zir",
        &module_decl,
        "module main\n\n@span sample.zt:1:1\nfunc echo(value: int) -> int\n\n@span sample.zt:2:1\nblock entry:\n  @span sample.zt:3:3\n  t0: int = copy value\n  @span sample.zt:4:3\n  return t0"
    );
}

static void test_enum_payload_print(void) {
    const zir_enum_variant_field_decl not_found_fields[] = {
        zir_make_enum_variant_field_decl("id", "int"),
    };
    const zir_enum_variant_field_decl invalid_name_fields[] = {
        zir_make_enum_variant_field_decl("message", "text"),
    };
    const zir_enum_variant_decl variants[] = {
        zir_make_enum_variant_decl("NotFound", not_found_fields, ARRAY_COUNT(not_found_fields)),
        zir_make_enum_variant_decl("InvalidName", invalid_name_fields, ARRAY_COUNT(invalid_name_fields)),
        zir_make_enum_variant_decl("Timeout", NULL, 0),
    };
    const zir_enum_decl enums[] = {
        zir_make_enum_decl("LoadUserError", variants, ARRAY_COUNT(variants)),
    };
    const zir_module module_decl = zir_make_module_with_decls("main", NULL, 0, enums, ARRAY_COUNT(enums), NULL, 0);

    assert_rendered(
        "enum_payload_print.zir",
        &module_decl,
        "module main\n\nenum LoadUserError\n  NotFound(id: int)\n  InvalidName(message: text)\n  Timeout"
    );
}
int main(void) {
    test_add();
    test_if_else();
    test_extern_call();
    test_effect_and_jump();
    test_structured_expression_printer();
    test_spans();
    test_enum_payload_print();
    puts("ZIR printer C tests OK");
    return 0;
}

