#include "compiler/utils/arena.h"
#include "compiler/utils/string_pool.h"
#include "compiler/frontend/parser/parser.h"
#include "compiler/hir/lowering/from_ast.h"
#include "compiler/targets/c/emitter.h"
#include "compiler/zir/lowering/from_hir.h"
#include "compiler/zir/model.h"
#include "compiler/zir/verifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#define ARRAY_COUNT(items) (sizeof(items) / sizeof((items)[0]))

static void ensure_generated_output_dir(void) {
#ifdef _WIN32
    (void)_mkdir(".ztc-tmp");
    (void)_mkdir(".ztc-tmp/tests");
    (void)_mkdir(".ztc-tmp/tests/targets");
    (void)_mkdir(".ztc-tmp/tests/targets/c");
#else
    (void)mkdir(".ztc-tmp", 0777);
    (void)mkdir(".ztc-tmp/tests", 0777);
    (void)mkdir(".ztc-tmp/tests/targets", 0777);
    (void)mkdir(".ztc-tmp/tests/targets/c", 0777);
#endif
}
static void verify_or_fail(const char *name, const zir_module *module_decl) {
    zir_verifier_result result;

    if (!zir_verify_module(module_decl, &result)) {
        fprintf(stderr, "ZIR invalida em %s\n", name);
        fprintf(stderr, "codigo: %s\n", zir_verifier_code_name(result.code));
        fprintf(stderr, "mensagem: %s\n", result.message);
        exit(1);
    }
}

static void assert_rendered(const char *name, const zir_module *module_decl, const char *expected) {
    c_emitter emitter;
    c_emit_result result;
    const char *actual;

    verify_or_fail(name, module_decl);

    c_emitter_init(&emitter);
    if (!c_emitter_emit_module(&emitter, module_decl, &result)) {
        fprintf(stderr, "falha ao emitir C para %s\n", name);
        fprintf(stderr, "codigo: %s\n", c_emit_error_code_name(result.code));
        fprintf(stderr, "mensagem: %s\n", result.message);
        c_emitter_dispose(&emitter);
        exit(1);
    }

    actual = c_emitter_text(&emitter);
    if (strcmp(actual, expected) != 0) {
        fprintf(stderr, "falha no teste %s\n", name);
        fprintf(stderr, "esperado:\n%s\n", expected);
        fprintf(stderr, "recebido:\n%s\n", actual);
        c_emitter_dispose(&emitter);
        exit(1);
    }

    c_emitter_dispose(&emitter);
}

static void assert_rendered_contains_all(
        const char *name,
        const zir_module *module_decl,
        const char *const *fragments,
        size_t fragment_count) {
    c_emitter emitter;
    c_emit_result result;
    const char *actual;
    size_t index;

    verify_or_fail(name, module_decl);

    c_emitter_init(&emitter);
    if (!c_emitter_emit_module(&emitter, module_decl, &result)) {
        fprintf(stderr, "falha ao emitir C para %s\n", name);
        fprintf(stderr, "codigo: %s\n", c_emit_error_code_name(result.code));
        fprintf(stderr, "mensagem: %s\n", result.message);
        c_emitter_dispose(&emitter);
        exit(1);
    }

    actual = c_emitter_text(&emitter);
    for (index = 0; index < fragment_count; index += 1) {
        if (strstr(actual, fragments[index]) == NULL) {
            fprintf(stderr, "falha no teste %s: fragmento ausente\n", name);
            fprintf(stderr, "fragmento: %s\n", fragments[index]);
            fprintf(stderr, "recebido:\n%s\n", actual);
            c_emitter_dispose(&emitter);
            exit(1);
        }
    }

    c_emitter_dispose(&emitter);
}

static void write_rendered_file(const char *name, const zir_module *module_decl, const char *path) {
    c_emitter emitter;
    c_emit_result result;
    FILE *file;

    verify_or_fail(name, module_decl);

    c_emitter_init(&emitter);
    if (!c_emitter_emit_module(&emitter, module_decl, &result)) {
        fprintf(stderr, "falha ao emitir C para %s\n", name);
        fprintf(stderr, "codigo: %s\n", c_emit_error_code_name(result.code));
        fprintf(stderr, "mensagem: %s\n", result.message);
        c_emitter_dispose(&emitter);
        exit(1);
    }

    file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "falha ao abrir %s\n", path);
        c_emitter_dispose(&emitter);
        exit(1);
    }

    fwrite(c_emitter_text(&emitter), 1, strlen(c_emitter_text(&emitter)), file);
    fclose(file);
    c_emitter_dispose(&emitter);
}

static void test_add(void) {
    const zir_param params[] = {
        zir_make_param("a", "int", NULL),
        zir_make_param("b", "int", NULL),
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
        "emit_add",
        &module_decl,
        "#include \"runtime/c/zenith_rt.h\"\n#include <stdio.h>\n\nstatic zt_int zt_main__add(zt_int a, zt_int b);\n\nstatic zt_int zt_main__add(zt_int a, zt_int b) {\n    zt_int t0;\n    goto zt_block_entry;\n\nzt_block_entry:\n    t0 = zt_add_i64(a, b);\n    return t0;\n}"
    );
}

static void test_if_else(void) {
    const zir_param params[] = {
        zir_make_param("x", "int", NULL),
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
        "emit_if_else",
        &module_decl,
        "#include \"runtime/c/zenith_rt.h\"\n#include <stdio.h>\n\nstatic zt_int zt_main__abs(zt_int x);\n\nstatic zt_int zt_main__abs(zt_int x) {\n    zt_bool t0;\n    zt_int t1;\n    goto zt_block_entry;\n\nzt_block_entry:\n    t0 = (x < 0);\n    if (t0) goto zt_block_negative;\n    goto zt_block_positive;\n\nzt_block_negative:\n    t1 = (-x);\n    return t1;\n\nzt_block_positive:\n    return x;\n}"
    );
}

static void test_text_return_transfer(void) {
    const zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "text", "const \"hi\""),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t0")),
    };
    const zir_function functions[] = {
        zir_make_function("greet", NULL, 0, "text", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_rendered(
        "emit_text_return",
        &module_decl,
        "#include \"runtime/c/zenith_rt.h\"\n#include <stdio.h>\n\nstatic zt_text *zt_main__greet(void);\n\nstatic zt_text *zt_main__greet(void) {\n    zt_text *t0 = NULL;\n    zt_text *zt_return_value = NULL;\n    goto zt_block_entry;\n\nzt_block_entry:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    t0 = zt_text_from_utf8_literal(\"hi\");\n    zt_return_value = t0;\n    t0 = NULL;\n    goto zt_cleanup;\n\nzt_cleanup:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    return zt_return_value;\n}"
    );
}

static void test_text_index_seq(void) {
    const zir_param params[] = {
        zir_make_param("name", "text", NULL),
    };
    const zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "text", "index_seq name, 2"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t0")),
    };
    const zir_function functions[] = {
        zir_make_function("char_at", params, ARRAY_COUNT(params), "text", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_rendered(
        "emit_text_index_seq",
        &module_decl,
        "#include \"runtime/c/zenith_rt.h\"\n#include <stdio.h>\n\nstatic zt_text *zt_main__char_at(zt_text *name);\n\nstatic zt_text *zt_main__char_at(zt_text *name) {\n    zt_text *t0 = NULL;\n    zt_text *zt_return_value = NULL;\n    goto zt_block_entry;\n\nzt_block_entry:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    t0 = zt_text_index(name, 2);\n    zt_return_value = t0;\n    t0 = NULL;\n    goto zt_cleanup;\n\nzt_cleanup:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    return zt_return_value;\n}"
    );
}

static void test_text_slice_seq(void) {
    const zir_param params[] = {
        zir_make_param("name", "text", NULL),
    };
    const zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "text", "slice_seq name, 2, 4"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t0")),
    };
    const zir_function functions[] = {
        zir_make_function("window", params, ARRAY_COUNT(params), "text", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_rendered(
        "emit_text_slice_seq",
        &module_decl,
        "#include \"runtime/c/zenith_rt.h\"\n#include <stdio.h>\n\nstatic zt_text *zt_main__window(zt_text *name);\n\nstatic zt_text *zt_main__window(zt_text *name) {\n    zt_text *t0 = NULL;\n    zt_text *zt_return_value = NULL;\n    goto zt_block_entry;\n\nzt_block_entry:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    t0 = zt_text_slice(name, 2, 4);\n    zt_return_value = t0;\n    t0 = NULL;\n    goto zt_cleanup;\n\nzt_cleanup:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    return zt_return_value;\n}"
    );
}

static void test_list_index_seq(void) {
    const zir_param params[] = {
        zir_make_param("xs", "list<int>", NULL),
    };
    const zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "int", "index_seq xs, 1"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t0")),
    };
    const zir_function functions[] = {
        zir_make_function("first", params, ARRAY_COUNT(params), "int", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_rendered(
        "emit_list_index_seq",
        &module_decl,
        "#include \"runtime/c/zenith_rt.h\"\n#include <stdio.h>\n\nstatic zt_int zt_main__first(zt_list_i64 *xs);\n\nstatic zt_int zt_main__first(zt_list_i64 *xs) {\n    zt_int t0;\n    goto zt_block_entry;\n\nzt_block_entry:\n    t0 = zt_list_i64_get(xs, 1);\n    return t0;\n}"
    );
}

static void test_list_slice_seq(void) {
    const zir_param params[] = {
        zir_make_param("xs", "list<int>", NULL),
    };
    const zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "list<int>", "slice_seq xs, 2, 4"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t0")),
    };
    const zir_function functions[] = {
        zir_make_function("window_ints", params, ARRAY_COUNT(params), "list<int>", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_rendered(
        "emit_list_slice_seq",
        &module_decl,
        "#include \"runtime/c/zenith_rt.h\"\n#include <stdio.h>\n\nstatic zt_list_i64 *zt_main__window_ints(zt_list_i64 *xs);\n\nstatic zt_list_i64 *zt_main__window_ints(zt_list_i64 *xs) {\n    zt_list_i64 *t0 = NULL;\n    zt_list_i64 *zt_return_value = NULL;\n    goto zt_block_entry;\n\nzt_block_entry:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    t0 = zt_list_i64_slice(xs, 2, 4);\n    zt_return_value = t0;\n    t0 = NULL;\n    goto zt_cleanup;\n\nzt_cleanup:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    return zt_return_value;\n}"
    );
}

static void test_list_len_expr(void) {
    const zir_param params[] = {
        zir_make_param("xs", "list<int>", NULL),
    };
    const zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "int", "list_len xs"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t0")),
    };
    const zir_function functions[] = {
        zir_make_function("count", params, ARRAY_COUNT(params), "int", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));

    assert_rendered(
        "emit_list_len",
        &module_decl,
        "#include \"runtime/c/zenith_rt.h\"\n#include <stdio.h>\n\nstatic zt_int zt_main__count(zt_list_i64 *xs);\n\nstatic zt_int zt_main__count(zt_list_i64 *xs) {\n    zt_int t0;\n    goto zt_block_entry;\n\nzt_block_entry:\n    t0 = zt_list_i64_len(xs);\n    return t0;\n}"
    );
}

static void test_make_list_text_expr(void) {
    const zir_param params[] = {
        zir_make_param("a", "text", NULL),
        zir_make_param("b", "text", NULL),
    };
    const zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "list<text>", "make_list<text> [a, b]"),
        zir_make_assign_instruction("t1", "text", "index_seq t0, 2"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t1")),
    };
    const zir_function functions[] = {
        zir_make_function("pick", params, ARRAY_COUNT(params), "text", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "static zt_text *zt_main__pick(zt_text *a, zt_text *b);",
        "zt_list_text *t0 = NULL;",
        "t0 = zt_list_text_from_array(((zt_text *[]){a, b}), 2);",
        "t1 = zt_list_text_get(t0, 2);",
        "return zt_return_value;",
    };

    assert_rendered_contains_all("emit_make_list_text", &module_decl, fragments, ARRAY_COUNT(fragments));
}

static void test_make_map_text_text_expr(void) {
    const zir_param params[] = {
        zir_make_param("k1", "text", NULL),
        zir_make_param("v1", "text", NULL),
        zir_make_param("k2", "text", NULL),
        zir_make_param("v2", "text", NULL),
    };
    const zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "map<text,text>", "make_map<text,text> [k1: v1, k2: v2]"),
        zir_make_assign_instruction("t1", "text", "index_seq t0, k2"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t1")),
    };
    const zir_function functions[] = {
        zir_make_function("pick_map", params, ARRAY_COUNT(params), "text", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "static zt_text *zt_main__pick_map(zt_text *k1, zt_text *v1, zt_text *k2, zt_text *v2);",
        "zt_map_text_text *t0 = NULL;",
        "t0 = zt_map_text_text_from_arrays(((zt_text *[]){k1, k2}), ((zt_text *[]){v1, v2}), 2);",
        "t1 = zt_map_text_text_get(t0, k2);",
    };

    assert_rendered_contains_all("emit_make_map_text_text", &module_decl, fragments, ARRAY_COUNT(fragments));
}

static void test_optional_present_empty_exprs(void) {
    const zir_param params[] = {
        zir_make_param("flag", "bool", NULL),
    };
    const zir_instruction some_instructions[] = {
        zir_make_assign_instruction("t0", "Optional<int>", "optional_present 7"),
    };
    const zir_instruction none_instructions[] = {
        zir_make_assign_instruction("t1", "Optional<int>", "optional_empty<int>"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", NULL, 0, zir_make_branch_if_terminator("flag", "some_bb", "none_bb")),
        zir_make_block("some_bb", some_instructions, ARRAY_COUNT(some_instructions), zir_make_return_terminator("t0")),
        zir_make_block("none_bb", none_instructions, ARRAY_COUNT(none_instructions), zir_make_return_terminator("t1")),
    };
    const zir_function functions[] = {
        zir_make_function("maybe", params, ARRAY_COUNT(params), "Optional<int>", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "static zt_optional_i64 zt_main__maybe(zt_bool flag);",
        "t0 = zt_optional_i64_present(7);",
        "t1 = zt_optional_i64_empty();",
    };

    assert_rendered_contains_all("emit_optional_present_empty", &module_decl, fragments, ARRAY_COUNT(fragments));
}

static void test_optional_coalesce_exprs(void) {
    const zir_param params[] = {
        zir_make_param("v", "Optional<int>", NULL),
        zir_make_param("fallback", "int", NULL),
    };
    const zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "bool", "optional_is_present v"),
        zir_make_assign_instruction("t1", "int", "coalesce v, fallback"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t1")),
    };
    const zir_function functions[] = {
        zir_make_function("with_default", params, ARRAY_COUNT(params), "int", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "static zt_int zt_main__with_default(zt_optional_i64 v, zt_int fallback);",
        "t0 = zt_optional_i64_is_present(v);",
        "t1 = zt_optional_i64_coalesce(v, fallback);",
    };

    assert_rendered_contains_all("emit_optional_coalesce", &module_decl, fragments, ARRAY_COUNT(fragments));
}

static void test_outcome_exprs(void) {
    const zir_param params[] = {
        zir_make_param("value", "Outcome<int,text>", NULL),
    };
    const zir_instruction entry_instructions[] = {
        zir_make_assign_instruction("t0", "bool", "outcome_is_success value"),
    };
    const zir_instruction ok_instructions[] = {
        zir_make_assign_instruction("t1", "int", "outcome_value value"),
        zir_make_assign_instruction("t2", "Outcome<int,text>", "outcome_success t1"),
    };
    const zir_instruction fail_instructions[] = {
        zir_make_assign_instruction("t3", "Outcome<int,text>", "try_propagate value"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", entry_instructions, ARRAY_COUNT(entry_instructions), zir_make_branch_if_terminator("t0", "ok_bb", "fail_bb")),
        zir_make_block("ok_bb", ok_instructions, ARRAY_COUNT(ok_instructions), zir_make_return_terminator("t2")),
        zir_make_block("fail_bb", fail_instructions, ARRAY_COUNT(fail_instructions), zir_make_return_terminator("t3")),
    };
    const zir_function functions[] = {
        zir_make_function("read_value", params, ARRAY_COUNT(params), "Outcome<int,text>", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "static zt_outcome_i64_text zt_main__read_value(zt_outcome_i64_text value);",
        "t0 = zt_outcome_i64_text_is_success(value);",
        "t1 = zt_outcome_i64_text_value(value);",
        "t2 = zt_outcome_i64_text_success(t1);",
        "t3 = zt_outcome_i64_text_propagate(value);",
    };

    assert_rendered_contains_all("emit_outcome_exprs", &module_decl, fragments, ARRAY_COUNT(fragments));
}

static void test_outcome_failure_expr(void) {
    const zir_param params[] = {
        zir_make_param("err", "text", NULL),
    };
    const zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "Outcome<int,text>", "outcome_failure err"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t0")),
    };
    const zir_function functions[] = {
        zir_make_function("fail", params, ARRAY_COUNT(params), "Outcome<int,text>", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "static zt_outcome_i64_text zt_main__fail(zt_text *err);",
        "t0 = zt_outcome_i64_text_failure(err);",
    };

    assert_rendered_contains_all("emit_outcome_failure", &module_decl, fragments, ARRAY_COUNT(fragments));
}

static void test_optional_text_exprs(void) {
    const zir_param params[] = {
        zir_make_param("flag", "bool", NULL),
        zir_make_param("value", "text", NULL),
        zir_make_param("fallback", "text", NULL),
    };
    const zir_instruction some_instructions[] = {
        zir_make_assign_instruction("t0", "Optional<text>", "optional_present value"),
        zir_make_assign_instruction("t1", "bool", "optional_is_present t0"),
        zir_make_assign_instruction("t2", "text", "coalesce t0, fallback"),
    };
    const zir_instruction none_instructions[] = {
        zir_make_assign_instruction("t3", "Optional<text>", "optional_empty<text>"),
        zir_make_assign_instruction("t4", "text", "coalesce t3, fallback"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", NULL, 0, zir_make_branch_if_terminator("flag", "some_bb", "none_bb")),
        zir_make_block("some_bb", some_instructions, ARRAY_COUNT(some_instructions), zir_make_return_terminator("t2")),
        zir_make_block("none_bb", none_instructions, ARRAY_COUNT(none_instructions), zir_make_return_terminator("t4")),
    };
    const zir_function functions[] = {
        zir_make_function("with_text", params, ARRAY_COUNT(params), "text", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "static zt_text *zt_main__with_text(zt_bool flag, zt_text *value, zt_text *fallback);",
        "zt_optional_text t0;",
        "t0 = zt_optional_text_present(value);",
        "t1 = zt_optional_text_is_present(t0);",
        "t2 = zt_optional_text_coalesce(t0, fallback);",
        "t3 = zt_optional_text_empty();",
        "t4 = zt_optional_text_coalesce(t3, fallback);",
    };

    assert_rendered_contains_all("emit_optional_text", &module_decl, fragments, ARRAY_COUNT(fragments));
}

static void test_optional_list_i64_exprs(void) {
    const zir_param params[] = {
        zir_make_param("flag", "bool", NULL),
        zir_make_param("value", "list<int>", NULL),
        zir_make_param("fallback", "list<int>", NULL),
    };
    const zir_instruction some_instructions[] = {
        zir_make_assign_instruction("t0", "Optional<list<int>>", "optional_present value"),
        zir_make_assign_instruction("t1", "bool", "optional_is_present t0"),
        zir_make_assign_instruction("t2", "list<int>", "coalesce t0, fallback"),
    };
    const zir_instruction none_instructions[] = {
        zir_make_assign_instruction("t3", "Optional<list<int>>", "optional_empty<list<int>>"),
        zir_make_assign_instruction("t4", "list<int>", "coalesce t3, fallback"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", NULL, 0, zir_make_branch_if_terminator("flag", "some_bb", "none_bb")),
        zir_make_block("some_bb", some_instructions, ARRAY_COUNT(some_instructions), zir_make_return_terminator("t2")),
        zir_make_block("none_bb", none_instructions, ARRAY_COUNT(none_instructions), zir_make_return_terminator("t4")),
    };
    const zir_function functions[] = {
        zir_make_function("with_list", params, ARRAY_COUNT(params), "list<int>", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "static zt_list_i64 *zt_main__with_list(zt_bool flag, zt_list_i64 *value, zt_list_i64 *fallback);",
        "zt_optional_list_i64 t0;",
        "t0 = zt_optional_list_i64_present(value);",
        "t1 = zt_optional_list_i64_is_present(t0);",
        "t2 = zt_optional_list_i64_coalesce(t0, fallback);",
        "t3 = zt_optional_list_i64_empty();",
        "t4 = zt_optional_list_i64_coalesce(t3, fallback);",
    };

    assert_rendered_contains_all("emit_optional_list_i64", &module_decl, fragments, ARRAY_COUNT(fragments));
}

static void test_outcome_void_exprs(void) {
    const zir_param params[] = {
        zir_make_param("value", "Outcome<void,text>", NULL),
    };
    const zir_instruction entry_instructions[] = {
        zir_make_assign_instruction("t0", "bool", "outcome_is_success value"),
    };
    const zir_instruction ok_instructions[] = {
        zir_make_assign_instruction("t1", "Outcome<void,text>", "outcome_success"),
    };
    const zir_instruction fail_instructions[] = {
        zir_make_assign_instruction("t2", "Outcome<void,text>", "try_propagate value"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", entry_instructions, ARRAY_COUNT(entry_instructions), zir_make_branch_if_terminator("t0", "ok_bb", "fail_bb")),
        zir_make_block("ok_bb", ok_instructions, ARRAY_COUNT(ok_instructions), zir_make_return_terminator("t1")),
        zir_make_block("fail_bb", fail_instructions, ARRAY_COUNT(fail_instructions), zir_make_return_terminator("t2")),
    };
    const zir_function functions[] = {
        zir_make_function("run", params, ARRAY_COUNT(params), "Outcome<void,text>", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "static zt_outcome_void_text zt_main__run(zt_outcome_void_text value);",
        "t0 = zt_outcome_void_text_is_success(value);",
        "t1 = zt_outcome_void_text_success();",
        "t2 = zt_outcome_void_text_propagate(value);",
    };

    assert_rendered_contains_all("emit_outcome_void", &module_decl, fragments, ARRAY_COUNT(fragments));
}

static void test_outcome_void_failure_expr(void) {
    const zir_param params[] = {
        zir_make_param("err", "text", NULL),
    };
    const zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "Outcome<void,text>", "outcome_failure err"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t0")),
    };
    const zir_function functions[] = {
        zir_make_function("fail_void", params, ARRAY_COUNT(params), "Outcome<void,text>", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "static zt_outcome_void_text zt_main__fail_void(zt_text *err);",
        "t0 = zt_outcome_void_text_failure(err);",
    };

    assert_rendered_contains_all("emit_outcome_void_failure", &module_decl, fragments, ARRAY_COUNT(fragments));
}

static void test_outcome_text_text_exprs(void) {
    const zir_param params[] = {
        zir_make_param("value", "Outcome<text,text>", NULL),
    };
    const zir_instruction entry_instructions[] = {
        zir_make_assign_instruction("t0", "bool", "outcome_is_success value"),
    };
    const zir_instruction ok_instructions[] = {
        zir_make_assign_instruction("t1", "text", "outcome_value value"),
        zir_make_assign_instruction("t2", "Outcome<text,text>", "outcome_success t1"),
    };
    const zir_instruction fail_instructions[] = {
        zir_make_assign_instruction("t3", "Outcome<text,text>", "try_propagate value"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", entry_instructions, ARRAY_COUNT(entry_instructions), zir_make_branch_if_terminator("t0", "ok_bb", "fail_bb")),
        zir_make_block("ok_bb", ok_instructions, ARRAY_COUNT(ok_instructions), zir_make_return_terminator("t2")),
        zir_make_block("fail_bb", fail_instructions, ARRAY_COUNT(fail_instructions), zir_make_return_terminator("t3")),
    };
    const zir_function functions[] = {
        zir_make_function("read_text", params, ARRAY_COUNT(params), "Outcome<text,text>", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "static zt_outcome_text_text zt_main__read_text(zt_outcome_text_text value);",
        "t0 = zt_outcome_text_text_is_success(value);",
        "t1 = zt_outcome_text_text_value(value);",
        "t2 = zt_outcome_text_text_success(t1);",
        "t3 = zt_outcome_text_text_propagate(value);",
    };

    assert_rendered_contains_all("emit_outcome_text_text", &module_decl, fragments, ARRAY_COUNT(fragments));
}

static void test_outcome_text_text_failure_expr(void) {
    const zir_param params[] = {
        zir_make_param("err", "text", NULL),
    };
    const zir_instruction instructions[] = {
        zir_make_assign_instruction("t0", "Outcome<text,text>", "outcome_failure err"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t0")),
    };
    const zir_function functions[] = {
        zir_make_function("fail_text", params, ARRAY_COUNT(params), "Outcome<text,text>", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "static zt_outcome_text_text zt_main__fail_text(zt_text *err);",
        "t0 = zt_outcome_text_text_failure(err);",
    };

    assert_rendered_contains_all("emit_outcome_text_text_failure", &module_decl, fragments, ARRAY_COUNT(fragments));
}


static void test_generic_outcome_bool_core_error_exprs(void) {
    const zir_param params[] = {
        zir_make_param("value", "Outcome<bool,core.Error>", NULL),
    };
    const zir_instruction entry_instructions[] = {
        zir_make_assign_instruction("t0", "bool", "outcome_is_success value"),
    };
    const zir_instruction ok_instructions[] = {
        zir_make_assign_instruction("t1", "bool", "outcome_value value"),
        zir_make_assign_instruction("t2", "Outcome<bool,core.Error>", "outcome_success t1"),
    };
    const zir_instruction fail_instructions[] = {
        zir_make_assign_instruction("t3", "Outcome<bool,core.Error>", "try_propagate value"),
    };
    const zir_instruction fail_literal_instructions[] = {
        zir_make_assign_instruction("t0", "Outcome<bool,core.Error>", "outcome_failure const \"boom\""),
    };
    const zir_block read_blocks[] = {
        zir_make_block("entry", entry_instructions, ARRAY_COUNT(entry_instructions), zir_make_branch_if_terminator("t0", "ok_bb", "fail_bb")),
        zir_make_block("ok_bb", ok_instructions, ARRAY_COUNT(ok_instructions), zir_make_return_terminator("t2")),
        zir_make_block("fail_bb", fail_instructions, ARRAY_COUNT(fail_instructions), zir_make_return_terminator("t3")),
    };
    const zir_block fail_blocks[] = {
        zir_make_block("entry", fail_literal_instructions, ARRAY_COUNT(fail_literal_instructions), zir_make_return_terminator("t0")),
    };
    const zir_function functions[] = {
        zir_make_function("read_flag", params, ARRAY_COUNT(params), "Outcome<bool,core.Error>", read_blocks, ARRAY_COUNT(read_blocks)),
        zir_make_function("fail_flag", NULL, 0, "Outcome<bool,core.Error>", fail_blocks, ARRAY_COUNT(fail_blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "#include <string.h>",
        "typedef struct zt_generated_outcome_bool_core_error_ {",
        "static zt_generated_outcome_bool_core_error_ zt_generated_outcome_bool_core_error__success(zt_bool value) {",
        "outcome.error = zt_core_error_clone(outcome.error);",
        "zt_core_error_dispose(&outcome->error);",
        "static zt_generated_outcome_bool_core_error_ zt_generated_outcome_bool_core_error__failure_message(const char *message) {",
        "static zt_generated_outcome_bool_core_error_ zt_main__read_flag(zt_generated_outcome_bool_core_error_ value);",
        "t0 = zt_generated_outcome_bool_core_error__is_success(value);",
        "t1 = zt_generated_outcome_bool_core_error__value(value);",
        "t2 = zt_generated_outcome_bool_core_error__success(t1);",
        "t3 = zt_generated_outcome_bool_core_error__propagate(value);",
        "t0 = zt_generated_outcome_bool_core_error__failure_message(\"boom\");",
    };

    assert_rendered_contains_all("emit_generic_outcome_bool_core_error", &module_decl, fragments, ARRAY_COUNT(fragments));
}

static void test_list_set_effects(void) {
    const zir_param int_params[] = {
        zir_make_param("xs", "list<int>", NULL),
        zir_make_param("v", "int", NULL),
    };
    const zir_instruction int_instructions[] = {
        zir_make_effect_instruction("list_set xs, 2, v"),
        zir_make_assign_instruction("t0", "int", "index_seq xs, 2"),
    };
    const zir_block int_blocks[] = {
        zir_make_block("entry", int_instructions, ARRAY_COUNT(int_instructions), zir_make_return_terminator("t0")),
    };
    const zir_function int_functions[] = {
        zir_make_function("replace_int", int_params, ARRAY_COUNT(int_params), "int", int_blocks, ARRAY_COUNT(int_blocks)),
    };
    const zir_module int_module = zir_make_module("main", int_functions, ARRAY_COUNT(int_functions));
    static const char *const int_fragments[] = {
        "xs = zt_list_i64_set_owned(xs, 2, v);",
        "t0 = zt_list_i64_get(xs, 2);",
    };

    const zir_param text_params[] = {
        zir_make_param("xs", "list<text>", NULL),
        zir_make_param("value", "text", NULL),
    };
    const zir_instruction text_instructions[] = {
        zir_make_effect_instruction("list_set xs, 1, value"),
        zir_make_assign_instruction("t0", "text", "index_seq xs, 1"),
    };
    const zir_block text_blocks[] = {
        zir_make_block("entry", text_instructions, ARRAY_COUNT(text_instructions), zir_make_return_terminator("t0")),
    };
    const zir_function text_functions[] = {
        zir_make_function("replace_text", text_params, ARRAY_COUNT(text_params), "text", text_blocks, ARRAY_COUNT(text_blocks)),
    };
    const zir_module text_module = zir_make_module("main", text_functions, ARRAY_COUNT(text_functions));
    static const char *const text_fragments[] = {
        "xs = zt_list_text_set_owned(xs, 1, value);",
        "t0 = zt_list_text_get(xs, 1);",
    };

    assert_rendered_contains_all("emit_list_set_int", &int_module, int_fragments, ARRAY_COUNT(int_fragments));
    assert_rendered_contains_all("emit_list_set_text", &text_module, text_fragments, ARRAY_COUNT(text_fragments));
}

static void test_map_set_and_len_effects(void) {
    const zir_param params[] = {
        zir_make_param("cfg", "map<text,text>", NULL),
        zir_make_param("key", "text", NULL),
        zir_make_param("value", "text", NULL),
    };
    const zir_instruction instructions[] = {
        zir_make_effect_instruction("map_set cfg, key, value"),
        zir_make_assign_instruction("t0", "int", "map_len cfg"),
        zir_make_assign_instruction("t1", "text", "index_seq cfg, key"),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", instructions, ARRAY_COUNT(instructions), zir_make_return_terminator("t1")),
    };
    const zir_function functions[] = {
        zir_make_function("update_map", params, ARRAY_COUNT(params), "text", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "cfg = zt_map_text_text_set_owned(cfg, key, value);",
        "t0 = zt_map_text_text_len(cfg);",
        "t1 = zt_map_text_text_get(cfg, key);",
    };

    assert_rendered_contains_all("emit_map_set_and_len", &module_decl, fragments, ARRAY_COUNT(fragments));
}

static void test_managed_param_return_retain(void) {
    const zir_param params[] = {
        zir_make_param("name", "text", NULL),
    };
    const zir_block blocks[] = {
        zir_make_block("entry", NULL, 0, zir_make_return_terminator("name")),
    };
    const zir_function functions[] = {
        zir_make_function("echo", params, ARRAY_COUNT(params), "text", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "return (zt_retain(name), name);",
    };

    assert_rendered_contains_all("emit_managed_param_return", &module_decl, fragments, ARRAY_COUNT(fragments));
}

static zir_module make_branch_main_module(void) {
    static const zir_instruction instructions[] = {
        { ZIR_INSTR_ASSIGN, "t0", "bool", "binary.lt 1, 2" },
    };
    static const zir_block blocks[] = {
        { "entry", instructions, ARRAY_COUNT(instructions), { ZIR_TERM_BRANCH_IF, NULL, NULL, "t0", "then_block", "else_block", NULL } },
        { "then_block", NULL, 0, { ZIR_TERM_RETURN, "7", NULL, NULL, NULL, NULL, NULL } },
        { "else_block", NULL, 0, { ZIR_TERM_RETURN, "9", NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "main", NULL, 0, "int", blocks, ARRAY_COUNT(blocks) },
    };

    return zir_make_module("main", functions, ARRAY_COUNT(functions));
}

static zir_module make_puts_main_module(void) {
    static const zir_instruction instructions[] = {
        { ZIR_INSTR_ASSIGN, "t0", "text", "const \"hello from zenith\"" },
        { ZIR_INSTR_EFFECT, NULL, NULL, "call_extern c.puts(t0)" },
    };
    static const zir_block blocks[] = {
        { "entry", instructions, ARRAY_COUNT(instructions), { ZIR_TERM_RETURN, "0", NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "main", NULL, 0, "int", blocks, ARRAY_COUNT(blocks) },
    };

    return zir_make_module("main", functions, ARRAY_COUNT(functions));
}

static zir_module make_text_index_main_module(void) {
    static const zir_instruction instructions[] = {
        { ZIR_INSTR_ASSIGN, "t0", "text", "const \"Zenith\"" },
        { ZIR_INSTR_ASSIGN, "t1", "text", "index_seq t0, 2" },
        { ZIR_INSTR_EFFECT, NULL, NULL, "call_extern c.puts(t1)" },
    };
    static const zir_block blocks[] = {
        { "entry", instructions, ARRAY_COUNT(instructions), { ZIR_TERM_RETURN, "0", NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "main", NULL, 0, "int", blocks, ARRAY_COUNT(blocks) },
    };

    return zir_make_module("main", functions, ARRAY_COUNT(functions));
}

static zir_module make_text_slice_main_module(void) {
    static const zir_instruction instructions[] = {
        { ZIR_INSTR_ASSIGN, "t0", "text", "const \"Zenith\"" },
        { ZIR_INSTR_ASSIGN, "t1", "text", "slice_seq t0, 2, 4" },
        { ZIR_INSTR_EFFECT, NULL, NULL, "call_extern c.puts(t1)" },
    };
    static const zir_block blocks[] = {
        { "entry", instructions, ARRAY_COUNT(instructions), { ZIR_TERM_RETURN, "0", NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "main", NULL, 0, "int", blocks, ARRAY_COUNT(blocks) },
    };

    return zir_make_module("main", functions, ARRAY_COUNT(functions));
}

static zir_module make_list_index_main_module(void) {
    static const zir_instruction instructions[] = {
        { ZIR_INSTR_ASSIGN, "t0", "list<int>", "make_list<int> [4, 7, 9]" },
        { ZIR_INSTR_ASSIGN, "t1", "int", "index_seq t0, 2" },
    };
    static const zir_block blocks[] = {
        { "entry", instructions, ARRAY_COUNT(instructions), { ZIR_TERM_RETURN, "t1", NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "main", NULL, 0, "int", blocks, ARRAY_COUNT(blocks) },
    };

    return zir_make_module("main", functions, ARRAY_COUNT(functions));
}

static zir_module make_list_slice_len_main_module(void) {
    static const zir_instruction instructions[] = {
        { ZIR_INSTR_ASSIGN, "t0", "list<int>", "make_list<int> [4, 7, 9, 11]" },
        { ZIR_INSTR_ASSIGN, "t1", "list<int>", "slice_seq t0, 2, 3" },
        { ZIR_INSTR_ASSIGN, "t2", "int", "list_len t1" },
    };
    static const zir_block blocks[] = {
        { "entry", instructions, ARRAY_COUNT(instructions), { ZIR_TERM_RETURN, "t2", NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "main", NULL, 0, "int", blocks, ARRAY_COUNT(blocks) },
    };

    return zir_make_module("main", functions, ARRAY_COUNT(functions));
}



static zir_module make_result_bool_main_module(void) {
    static const zir_instruction instructions[] = {
        { ZIR_INSTR_ASSIGN, "t0", "Outcome<void,bool>", "outcome_failure false" },
    };
    static const zir_block blocks[] = {
        { "entry", instructions, ARRAY_COUNT(instructions), { ZIR_TERM_RETURN, "t0", NULL, NULL, NULL, NULL, NULL } },
    };
    static const zir_function functions[] = {
        { "main", NULL, 0, "Outcome<void,bool>", blocks, ARRAY_COUNT(blocks) },
    };

    return zir_make_module("main", functions, ARRAY_COUNT(functions));
}

static void test_structured_zir_emission(void) {
    zir_expr *list_expr = zir_expr_make_make_list("int");
    const zir_instruction instructions[] = {
        zir_make_assign_instruction_expr(
            "t0",
            "int",
            zir_expr_make_binary("add", zir_expr_make_int("1"), zir_expr_make_int("2"))
        ),
        zir_make_assign_instruction_expr(
            "nums",
            "list<int>",
            list_expr
        ),
        zir_make_assign_instruction_expr(
            "t1",
            "int",
            zir_expr_make_index_seq(zir_expr_make_name("nums"), zir_expr_make_int("2"))
        ),
    };
    const zir_block blocks[] = {
        zir_make_block(
            "entry",
            instructions,
            ARRAY_COUNT(instructions),
            zir_make_return_terminator_expr(
                zir_expr_make_binary("add", zir_expr_make_name("t0"), zir_expr_make_name("t1"))
            )
        ),
    };
    const zir_function functions[] = {
        zir_make_function("main", NULL, 0, "int", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module("main", functions, ARRAY_COUNT(functions));
    static const char *const fragments[] = {
        "t0 = zt_add_i64(1, 2);",
        "nums = zt_list_i64_from_array(((zt_int[]){4, 7, 9}), 3);",
        "t1 = zt_list_i64_get(nums, 2);",
        "zt_return_value = zt_add_i64(t0, t1);",
    };

    zir_expr_make_list_add_item(list_expr, zir_expr_make_int("4"));
    zir_expr_make_list_add_item(list_expr, zir_expr_make_int("7"));
    zir_expr_make_list_add_item(list_expr, zir_expr_make_int("9"));

    assert_rendered_contains_all("emit_structured_zir", &module_decl, fragments, ARRAY_COUNT(fragments));
}


static void test_real_lowering_to_c(void) {    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena, 65536);
    zt_string_pool_init(&test_pool, &test_arena);

    const char *src =
        "namespace main\n"
        "func main() -> int\n"
        "    const a: int = 1\n"
        "    const b: int = 2\n"
        "    return a + b\n"
        "end";
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, "emitter.zt", src, strlen(src));
    zt_hir_lower_result hir;
    zir_lower_result zir;
    static const char *const fragments[] = {
        "a = 1;",
        "b = 2;",
        "return zt_add_i64(a, b);",
        "int main(void) {",
    };

    if (parsed.diagnostics.count != 0) {
        fprintf(stderr, "falha no teste real_lowering_to_c: parse errors=%zu\n", parsed.diagnostics.count);
        zt_parser_result_dispose(&parsed);
        exit(1);
    }

    hir = zt_lower_ast_to_hir(parsed.root);
    if (hir.diagnostics.count != 0) {
        fprintf(stderr, "falha no teste real_lowering_to_c: HIR diagnostics=%zu\n", hir.diagnostics.count);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        exit(1);
    }

    zir = zir_lower_hir_to_zir(hir.module);
    if (zir.diagnostics.count != 0) {
        fprintf(stderr, "falha no teste real_lowering_to_c: ZIR diagnostics=%zu\n", zir.diagnostics.count);
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        exit(1);
    }

    assert_rendered_contains_all("emit_real_lowering_to_c", &zir.module, fragments, ARRAY_COUNT(fragments));

    zir_lower_result_dispose(&zir);
    zt_hir_lower_result_dispose(&hir);
    zt_parser_result_dispose(&parsed);
}

static void test_main_wrapper_and_outputs(void) {
    const zir_module branch_main = make_branch_main_module();
    const zir_module puts_main = make_puts_main_module();
    const zir_module text_index_main = make_text_index_main_module();
    const zir_module text_slice_main = make_text_slice_main_module();
    const zir_module list_index_main = make_list_index_main_module();
    const zir_module list_slice_len_main = make_list_slice_len_main_module();
    const zir_module result_bool_main = make_result_bool_main_module();

    assert_rendered(
        "emit_branch_main",
        &branch_main,
        "#include \"runtime/c/zenith_rt.h\"\n#include <stdio.h>\n\nstatic zt_int zt_main__main(void);\n\nstatic zt_int zt_main__main(void) {\n    zt_bool t0;\n    goto zt_block_entry;\n\nzt_block_entry:\n    t0 = (1 < 2);\n    if (t0) goto zt_block_then_block;\n    goto zt_block_else_block;\n\nzt_block_then_block:\n    return 7;\n\nzt_block_else_block:\n    return 9;\n}\n\nint main(void) {\n    return (int)zt_main__main();\n}"
    );

    assert_rendered(
        "emit_puts_main",
        &puts_main,
        "#include \"runtime/c/zenith_rt.h\"\n#include <stdio.h>\n\nstatic zt_int zt_main__main(void);\n\nstatic zt_int zt_main__main(void) {\n    zt_text *t0 = NULL;\n    zt_int zt_return_value;\n    goto zt_block_entry;\n\nzt_block_entry:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    t0 = zt_text_from_utf8_literal(\"hello from zenith\");\n    puts(zt_text_data(t0));\n    zt_return_value = 0;\n    goto zt_cleanup;\n\nzt_cleanup:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    return zt_return_value;\n}\n\nint main(void) {\n    return (int)zt_main__main();\n}"
    );

    assert_rendered(
        "emit_text_index_main",
        &text_index_main,
        "#include \"runtime/c/zenith_rt.h\"\n#include <stdio.h>\n\nstatic zt_int zt_main__main(void);\n\nstatic zt_int zt_main__main(void) {\n    zt_text *t0 = NULL;\n    zt_text *t1 = NULL;\n    zt_int zt_return_value;\n    goto zt_block_entry;\n\nzt_block_entry:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    t0 = zt_text_from_utf8_literal(\"Zenith\");\n    if (t1 != NULL) { zt_release(t1); t1 = NULL; }\n    t1 = zt_text_index(t0, 2);\n    puts(zt_text_data(t1));\n    zt_return_value = 0;\n    goto zt_cleanup;\n\nzt_cleanup:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    if (t1 != NULL) { zt_release(t1); t1 = NULL; }\n    return zt_return_value;\n}\n\nint main(void) {\n    return (int)zt_main__main();\n}"
    );

    assert_rendered(
        "emit_text_slice_main",
        &text_slice_main,
        "#include \"runtime/c/zenith_rt.h\"\n#include <stdio.h>\n\nstatic zt_int zt_main__main(void);\n\nstatic zt_int zt_main__main(void) {\n    zt_text *t0 = NULL;\n    zt_text *t1 = NULL;\n    zt_int zt_return_value;\n    goto zt_block_entry;\n\nzt_block_entry:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    t0 = zt_text_from_utf8_literal(\"Zenith\");\n    if (t1 != NULL) { zt_release(t1); t1 = NULL; }\n    t1 = zt_text_slice(t0, 2, 4);\n    puts(zt_text_data(t1));\n    zt_return_value = 0;\n    goto zt_cleanup;\n\nzt_cleanup:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    if (t1 != NULL) { zt_release(t1); t1 = NULL; }\n    return zt_return_value;\n}\n\nint main(void) {\n    return (int)zt_main__main();\n}"
    );

    assert_rendered(
        "emit_list_index_main",
        &list_index_main,
        "#include \"runtime/c/zenith_rt.h\"\n#include <stdio.h>\n\nstatic zt_int zt_main__main(void);\n\nstatic zt_int zt_main__main(void) {\n    zt_list_i64 *t0 = NULL;\n    zt_int t1;\n    zt_int zt_return_value;\n    goto zt_block_entry;\n\nzt_block_entry:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    t0 = zt_list_i64_from_array(((zt_int[]){4, 7, 9}), 3);\n    t1 = zt_list_i64_get(t0, 2);\n    zt_return_value = t1;\n    goto zt_cleanup;\n\nzt_cleanup:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    return zt_return_value;\n}\n\nint main(void) {\n    return (int)zt_main__main();\n}"
    );

    assert_rendered(
        "emit_list_slice_len_main",
        &list_slice_len_main,
        "#include \"runtime/c/zenith_rt.h\"\n#include <stdio.h>\n\nstatic zt_int zt_main__main(void);\n\nstatic zt_int zt_main__main(void) {\n    zt_list_i64 *t0 = NULL;\n    zt_list_i64 *t1 = NULL;\n    zt_int t2;\n    zt_int zt_return_value;\n    goto zt_block_entry;\n\nzt_block_entry:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    t0 = zt_list_i64_from_array(((zt_int[]){4, 7, 9, 11}), 4);\n    if (t1 != NULL) { zt_release(t1); t1 = NULL; }\n    t1 = zt_list_i64_slice(t0, 2, 3);\n    t2 = zt_list_i64_len(t1);\n    zt_return_value = t2;\n    goto zt_cleanup;\n\nzt_cleanup:\n    if (t0 != NULL) { zt_release(t0); t0 = NULL; }\n    if (t1 != NULL) { zt_release(t1); t1 = NULL; }\n    return zt_return_value;\n}\n\nint main(void) {\n    return (int)zt_main__main();\n}"
    );


    {
        static const char *const fragments[] = {
            "static zt_generated_outcome_void_bool_ zt_main__main(void);",
            "t0 = zt_generated_outcome_void_bool__failure(false);",
            "zt_generated_outcome_void_bool_ __zt_main_result = zt_main__main();",
            "zt_generated_outcome_void_bool__dispose(&__zt_main_result);",
        };

        assert_rendered_contains_all("emit_result_bool_main", &result_bool_main, fragments, ARRAY_COUNT(fragments));
    }

    write_rendered_file("branch_main_out", &branch_main, ".ztc-tmp/tests/targets/c/generated_branch_main.c");
    write_rendered_file("puts_main_out", &puts_main, ".ztc-tmp/tests/targets/c/generated_puts_main.c");
    write_rendered_file("text_index_main_out", &text_index_main, ".ztc-tmp/tests/targets/c/generated_text_index_main.c");
    write_rendered_file("text_slice_main_out", &text_slice_main, ".ztc-tmp/tests/targets/c/generated_text_slice_main.c");
    write_rendered_file("list_index_main_out", &list_index_main, ".ztc-tmp/tests/targets/c/generated_list_index_main.c");
    write_rendered_file("list_slice_len_main_out", &list_slice_len_main, ".ztc-tmp/tests/targets/c/generated_list_slice_len_main.c");
}

int main(void) {
    ensure_generated_output_dir();
    test_add();
    test_if_else();
    test_text_return_transfer();
    test_text_index_seq();
    test_text_slice_seq();
    test_list_index_seq();
    test_list_slice_seq();
    test_list_len_expr();
    test_make_list_text_expr();
    test_make_map_text_text_expr();
    test_optional_present_empty_exprs();
    test_optional_coalesce_exprs();
    test_optional_text_exprs();
    test_optional_list_i64_exprs();
    test_outcome_exprs();
    test_outcome_failure_expr();
    test_outcome_void_exprs();
    test_outcome_void_failure_expr();
    test_outcome_text_text_exprs();
    test_outcome_text_text_failure_expr();
    test_generic_outcome_bool_core_error_exprs();
    test_list_set_effects();
    test_map_set_and_len_effects();
    test_managed_param_return_retain();
    test_structured_zir_emission();
    test_real_lowering_to_c();
    test_main_wrapper_and_outputs();
    puts("C emitter tests OK");
    return 0;
}



