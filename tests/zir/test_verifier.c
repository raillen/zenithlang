#include "compiler/zir/model.h"
#include "compiler/zir/verifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#define ARRAY_COUNT(items) (sizeof(items) / sizeof((items)[0]))

static void assert_ok(const char *name, const zir_module *module_decl) {
    zir_verifier_result result;

    if (!zir_verify_module(module_decl, &result)) {
        fprintf(stderr, "falha no teste %s\n", name);
        fprintf(stderr, "codigo: %s\n", zir_verifier_code_name(result.code));
        fprintf(stderr, "mensagem: %s\n", result.message);
        exit(1);
    }
}

static zir_verifier_result assert_failure(
        const char *name,
        const zir_module *module_decl,
        zir_verifier_code expected_code,
        const char *expected_message_fragment) {
    zir_verifier_result result;

    if (zir_verify_module(module_decl, &result)) {
        fprintf(stderr, "falha no teste %s: era esperado erro\n", name);
        exit(1);
    }

    if (result.code != expected_code) {
        fprintf(stderr, "falha no teste %s: codigo inesperado\n", name);
        fprintf(stderr, "esperado: %s\n", zir_verifier_code_name(expected_code));
        fprintf(stderr, "recebido: %s\n", zir_verifier_code_name(result.code));
        fprintf(stderr, "mensagem: %s\n", result.message);
        exit(1);
    }

    if (expected_message_fragment != NULL && strstr(result.message, expected_message_fragment) == NULL) {
        fprintf(stderr, "falha no teste %s: mensagem inesperada\n", name);
        fprintf(stderr, "fragmento esperado: %s\n", expected_message_fragment);
        fprintf(stderr, "recebido: %s\n", result.message);
        exit(1);
    }

    return result;
}

static zir_module make_valid_module(void) {
    static const zir_param params[] = {
        { "a", "int" },
        { "b", "int" },
    };
    static const zir_instruction instructions[] = {
        { ZIR_INSTR_ASSIGN, "t0", "int", "binary.add a, b" },
    };
    static const zir_block blocks[] = {
        { "entry", instructions, ARRAY_COUNT(instructions), { ZIR_TERM_RETURN, "t0", NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "add", params, ARRAY_COUNT(params), "int", blocks, ARRAY_COUNT(blocks) },
    };
    return zir_make_module("main", functions, ARRAY_COUNT(functions));
}

static void test_valid_module(void) {
    const zir_module module_decl = make_valid_module();
    assert_ok("valid_module", &module_decl);
}

static void test_effect_and_jump(void) {
    static const zir_param params[] = {
        { "point", "struct Point" },
        { "value", "int" },
    };
    static const zir_instruction instructions[] = {
        { ZIR_INSTR_EFFECT, NULL, NULL, "set_field point, x, value" },
    };
    static const zir_block blocks[] = {
        { "entry", instructions, ARRAY_COUNT(instructions), { ZIR_TERM_JUMP, NULL, "exit", NULL, NULL, NULL, NULL } },
        { "exit", NULL, 0, { ZIR_TERM_UNREACHABLE, NULL, NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "mutate", params, ARRAY_COUNT(params), "void", blocks, ARRAY_COUNT(blocks) },
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_ok("effect_and_jump", &module_decl);
}

static void test_invalid_any(void) {
    static const zir_param params[] = {
        { "x", "any" },
    };
    static const zir_block blocks[] = {
        { "entry", NULL, 0, { ZIR_TERM_RETURN, "x", NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "f", params, ARRAY_COUNT(params), "any", blocks, ARRAY_COUNT(blocks) },
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_failure(
        "invalid_any",
        &module_decl,
        ZIR_VERIFIER_INVALID_TYPE_ANY,
        "any is not part of the Zenith Next core"
    );
}

static void test_invalid_target_leak(void) {
    static const zir_param params[] = {
        { "a", "Optional<int>" },
        { "b", "int" },
    };
    static const zir_instruction instructions[] = {
        { ZIR_INSTR_ASSIGN, "t0", "int", "lua_call zt.unwrap_or(a, b)" },
    };
    static const zir_block blocks[] = {
        { "entry", instructions, ARRAY_COUNT(instructions), { ZIR_TERM_RETURN, "t0", NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "f", params, ARRAY_COUNT(params), "int", blocks, ARRAY_COUNT(blocks) },
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_failure(
        "invalid_target_leak",
        &module_decl,
        ZIR_VERIFIER_INVALID_TARGET_LEAK,
        "target-specific names and legacy runtime helpers cannot appear in ZIR"
    );
}

static void test_identifier_with_ffi_substring_is_valid(void) {
    static const zir_param params[] = {
        { "suffix", "text" },
    };
    static const zir_block blocks[] = {
        { "entry", NULL, 0, { ZIR_TERM_RETURN, "suffix", NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "keep_suffix", params, ARRAY_COUNT(params), "text", blocks, ARRAY_COUNT(blocks) },
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_ok("identifier_with_ffi_substring_is_valid", &module_decl);
}

static void test_use_before_definition(void) {
    static const zir_param params[] = {
        { "a", "int" },
    };
    static const zir_instruction instructions[] = {
        { ZIR_INSTR_ASSIGN, "t0", "int", "binary.add a, t1" },
    };
    static const zir_block blocks[] = {
        { "entry", instructions, ARRAY_COUNT(instructions), { ZIR_TERM_RETURN, "t0", NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "f", params, ARRAY_COUNT(params), "int", blocks, ARRAY_COUNT(blocks) },
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_failure(
        "use_before_definition",
        &module_decl,
        ZIR_VERIFIER_USE_BEFORE_DEFINITION,
        "t1"
    );
}

static void test_unknown_block(void) {
    static const zir_param params[] = {
        { "flag", "bool" },
    };
    static const zir_block blocks[] = {
        { "entry", NULL, 0, { ZIR_TERM_BRANCH_IF, NULL, NULL, "flag", "then", "missing", NULL } },
        { "then", NULL, 0, { ZIR_TERM_RETURN, "flag", NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "f", params, ARRAY_COUNT(params), "bool", blocks, ARRAY_COUNT(blocks) },
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_failure(
        "unknown_block",
        &module_decl,
        ZIR_VERIFIER_UNKNOWN_BLOCK,
        "missing"
    );
}

static void test_invalid_terminator(void) {
    static const zir_block blocks[] = {
        { "entry", NULL, 0, { ZIR_TERM_UNREACHABLE, NULL, NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "f", NULL, 0, "void", blocks, ARRAY_COUNT(blocks) },
    };
    zir_block mutable_blocks[ARRAY_COUNT(blocks)];
    zir_function mutable_functions[ARRAY_COUNT(functions)];
    zir_module module_decl;

    memcpy(mutable_blocks, blocks, sizeof(blocks));
    mutable_blocks[0].terminator.kind = (zir_terminator_kind)999;
    memcpy(mutable_functions, functions, sizeof(functions));
    mutable_functions[0].blocks = mutable_blocks;
    module_decl = zir_make_module("main", mutable_functions, ARRAY_COUNT(mutable_functions));

    assert_failure(
        "invalid_terminator",
        &module_decl,
        ZIR_VERIFIER_INVALID_TERMINATOR,
        "unknown terminator kind"
    );
}

static void test_invalid_return(void) {
    static const zir_block blocks[] = {
        { "entry", NULL, 0, { ZIR_TERM_RETURN, NULL, NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "f", NULL, 0, "int", blocks, ARRAY_COUNT(blocks) },
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_failure(
        "invalid_return",
        &module_decl,
        ZIR_VERIFIER_INVALID_RETURN,
        "must return a value"
    );
}

static void test_duplicate_block_label(void) {
    static const zir_block blocks[] = {
        { "entry", NULL, 0, { ZIR_TERM_UNREACHABLE, NULL, NULL, NULL, NULL, NULL, NULL } },
        { "entry", NULL, 0, { ZIR_TERM_UNREACHABLE, NULL, NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "f", NULL, 0, "void", blocks, ARRAY_COUNT(blocks) },
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_failure(
        "duplicate_block_label",
        &module_decl,
        ZIR_VERIFIER_DUPLICATE_BLOCK_LABEL,
        "entry"
    );
}


static void test_structured_expr_ok(void) {
    const zir_param params[] = {
        zir_make_param("a", "int", NULL),
        zir_make_param("b", "int", NULL),
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
        zir_make_function("f", params, ARRAY_COUNT(params), "int", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_ok("structured_expr_ok", &module_decl);
}

static void test_structured_use_before_definition(void) {
    const zir_param params[] = {
        zir_make_param("a", "int", NULL),
    };
    const zir_instruction instructions[] = {
        zir_make_assign_instruction_expr(
            "t0",
            "int",
            zir_expr_make_binary("add", zir_expr_make_name("a"), zir_expr_make_name("missing"))
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
        zir_make_function("f", params, ARRAY_COUNT(params), "int", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_failure(
        "structured_use_before_definition",
        &module_decl,
        ZIR_VERIFIER_USE_BEFORE_DEFINITION,
        "missing"
    );
}

static void test_error_span(void) {
    zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "int", "binary.add a, t1"),
    };
    zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t0")),
    };
    zir_param params[] = {
        zir_make_param("a", "int", NULL),
    };
    zir_function functions[] = {
        zir_make_function("f", params, ARRAY_COUNT(params), "int", blocks, ARRAY_COUNT(blocks)),
    };
    zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    zir_verifier_result result;

    instructions[0].span = zir_make_span("broken.zt", 7, 15);

    result = assert_failure(
        "error_span",
        &module_decl,
        ZIR_VERIFIER_USE_BEFORE_DEFINITION,
        "broken.zt:7:15"
    );

    if (!result.has_span) {
        fprintf(stderr, "falha no teste error_span: span ausente\n");
        exit(1);
    }

    if (strcmp(result.source_name, "broken.zt") != 0 || result.line != 7 || result.column != 15) {
        fprintf(stderr, "falha no teste error_span: span inesperado\n");
        fprintf(stderr, "recebido: %s:%zu:%zu\n", result.source_name, result.line, result.column);
        exit(1);
    }
}

static void test_cross_block_reference_ok(void) {
    const zir_param params[] = {
        zir_make_param("a", "int", NULL),
    };
    const zir_instruction entry_instructions[] = {
        zir_make_assign_instruction_expr("t0", "int", zir_expr_make_name("a")),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", entry_instructions, ARRAY_COUNT(entry_instructions), zir_make_jump_terminator("body")),
        zir_make_block("body", NULL, 0, zir_make_return_terminator_expr(zir_expr_make_name("t0"))),
    };
    const zir_function functions[] = {
        zir_make_function("f", params, ARRAY_COUNT(params), "int", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_ok("cross_block_reference_ok", &module_decl);
}

static void test_enum_payload_ok(void) {
    const zir_enum_variant_field_decl ok_fields[] = {
        zir_make_enum_variant_field_decl("id", "int"),
    };
    const zir_enum_variant_decl variants[] = {
        zir_make_enum_variant_decl("NotFound", ok_fields, ARRAY_COUNT(ok_fields)),
        zir_make_enum_variant_decl("Timeout", NULL, 0),
    };
    const zir_enum_decl enums[] = {
        zir_make_enum_decl("LoadUserError", variants, ARRAY_COUNT(variants)),
    };
    const zir_module module_decl = zir_make_module_with_decls("main", NULL, 0, enums, ARRAY_COUNT(enums), NULL, 0);

    assert_ok("enum_payload_ok", &module_decl);
}

static void test_enum_payload_invalid_any_type(void) {
    const zir_enum_variant_field_decl bad_fields[] = {
        zir_make_enum_variant_field_decl("payload", "any"),
    };
    const zir_enum_variant_decl variants[] = {
        zir_make_enum_variant_decl("Bad", bad_fields, ARRAY_COUNT(bad_fields)),
    };
    const zir_enum_decl enums[] = {
        zir_make_enum_decl("Broken", variants, ARRAY_COUNT(variants)),
    };
    const zir_module module_decl = zir_make_module_with_decls("main", NULL, 0, enums, ARRAY_COUNT(enums), NULL, 0);

    assert_failure(
        "enum_payload_invalid_any_type",
        &module_decl,
        ZIR_VERIFIER_INVALID_TYPE_ANY,
        "enum Broken variant Bad field payload"
    );
}
int main(void) {
    test_valid_module();
    test_effect_and_jump();
    test_invalid_any();
    test_invalid_target_leak();
    test_identifier_with_ffi_substring_is_valid();
    test_use_before_definition();
    test_unknown_block();
    test_invalid_terminator();
    test_invalid_return();
    test_duplicate_block_label();
    test_structured_expr_ok();
    test_structured_use_before_definition();
    test_error_span();
    test_cross_block_reference_ok();
    test_enum_payload_ok();
    test_enum_payload_invalid_any_type();
    puts("ZIR verifier C tests OK");
    return 0;
}

