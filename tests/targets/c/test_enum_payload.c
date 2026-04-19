#include "compiler/zir/model.h"
#include "compiler/zir/verifier.h"
#include "compiler/targets/c/emitter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_COUNT(items) (sizeof(items) / sizeof((items)[0]))

static void fail_now(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

static void assert_contains(const char *text, const char *fragment) {
    if (strstr(text, fragment) == NULL) {
        fprintf(stderr, "fragmento ausente:\n%s\n", fragment);
        fprintf(stderr, "saida:\n%s\n", text);
        exit(1);
    }
}

int main(void) {
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
    const zir_block blocks[] = {
        zir_make_block("entry", NULL, 0, zir_make_return_terminator("0")),
    };
    const zir_function functions[] = {
        zir_make_function("main", NULL, 0, "int", blocks, ARRAY_COUNT(blocks)),
    };
    const zir_module module_decl = zir_make_module_with_decls(
        "app",
        NULL,
        0,
        enums,
        ARRAY_COUNT(enums),
        functions,
        ARRAY_COUNT(functions));
    zir_verifier_result verify;
    c_emitter emitter;
    c_emit_result emitted;
    const char *output;

    if (!zir_verify_module(&module_decl, &verify)) {
        fail_now("falha: verifier rejeitou enum payload para emitter");
    }

    c_emitter_init(&emitter);
    if (!c_emitter_emit_module(&emitter, &module_decl, &emitted)) {
        fprintf(stderr, "falha emitter: %s\n", emitted.message);
        c_emitter_dispose(&emitter);
        exit(1);
    }

    output = c_emitter_text(&emitter);
    assert_contains(output, "typedef enum zt_app__LoadUserError_tag {");
    assert_contains(output, "zt_app__LoadUserError__NotFound,");
    assert_contains(output, "zt_app__LoadUserError__InvalidName,");
    assert_contains(output, "zt_app__LoadUserError__Timeout");
    assert_contains(output, "typedef struct zt_app__LoadUserError {");
    assert_contains(output, "zt_app__LoadUserError_tag tag;");
    assert_contains(output, "union {");
    assert_contains(output, "zt_int id;");
    assert_contains(output, "zt_text *message;");
    assert_contains(output, "char _unused;");
    assert_contains(output, "} as;");
    assert_contains(output, "} zt_app__LoadUserError;");

    c_emitter_dispose(&emitter);

    puts("C enum payload emitter tests OK");
    return 0;
}
