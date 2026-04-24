/*
 * Regression coverage for PLI-02 (C-symbol collision guard).
 *
 * The emitter's sanitizer flattens every character outside
 * `[A-Za-z0-9_]` to `_`, which means two distinct Zenith declarations
 * can mangle to the same C identifier. Before the fix, the backend
 * silently emitted invalid C; now `c_emitter_emit_module` must refuse
 * and produce a readable diagnostic mentioning both source names and
 * the colliding C symbol.
 */

#include "compiler/targets/c/emitter.h"
#include "compiler/zir/model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_COUNT(items) (sizeof(items) / sizeof((items)[0]))

static void fail_now(const char *message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

/*
 * Case 1: a function and a struct share the same mangled tail
 * inside the same module. Both become `zt_app__foo`.
 */
static void test_function_and_struct_collision(void) {
    const zir_block func_blocks[] = {
        zir_make_block(
            "entry",
            NULL, 0,
            zir_make_return_terminator("0")),
    };
    const zir_function functions[] = {
        zir_make_function("foo", NULL, 0, "int", func_blocks, ARRAY_COUNT(func_blocks)),
    };
    const zir_field_decl fields[] = {
        zir_make_field_decl("value", "int", NULL),
    };
    const zir_struct_decl structs[] = {
        zir_make_struct_decl("foo", fields, ARRAY_COUNT(fields)),
    };
    const zir_module module_decl = zir_make_module_with_structs(
        "app",
        structs, ARRAY_COUNT(structs),
        functions, ARRAY_COUNT(functions));

    c_emitter emitter;
    c_emit_result result;

    c_emitter_init(&emitter);
    if (c_emitter_emit_module(&emitter, &module_decl, &result)) {
        c_emitter_dispose(&emitter);
        fail_now("expected collision between struct 'foo' and function 'foo', but emitter succeeded");
    }

    if (strstr(result.message, "collision") == NULL) {
        fprintf(stderr, "expected 'collision' in message, got: %s\n", result.message);
        c_emitter_dispose(&emitter);
        exit(1);
    }
    if (strstr(result.message, "zt_app__foo") == NULL) {
        fprintf(stderr, "expected mangled symbol 'zt_app__foo' in message, got: %s\n", result.message);
        c_emitter_dispose(&emitter);
        exit(1);
    }

    c_emitter_dispose(&emitter);
}

/*
 * Case 2: two distinct source names that sanitize to the same symbol.
 * The struct `a.b` uses a dot segment that the current sanitizer
 * flattens to `_`, clashing with a struct literally named `a_b`.
 * Note: Zenith's frontend normally rejects dotted struct names, but
 * the emitter must still refuse this ZIR-level pathology instead of
 * emitting duplicate typedefs.
 */
static void test_dot_vs_underscore_struct_collision(void) {
    const zir_field_decl fields_left[] = {
        zir_make_field_decl("x", "int", NULL),
    };
    const zir_field_decl fields_right[] = {
        zir_make_field_decl("y", "int", NULL),
    };
    const zir_struct_decl structs[] = {
        zir_make_struct_decl("a.b", fields_left, ARRAY_COUNT(fields_left)),
        zir_make_struct_decl("a_b", fields_right, ARRAY_COUNT(fields_right)),
    };
    const zir_module module_decl = zir_make_module_with_structs(
        "app",
        structs, ARRAY_COUNT(structs),
        NULL, 0);

    c_emitter emitter;
    c_emit_result result;

    c_emitter_init(&emitter);
    if (c_emitter_emit_module(&emitter, &module_decl, &result)) {
        c_emitter_dispose(&emitter);
        fail_now("expected collision between structs 'a.b' and 'a_b', but emitter succeeded");
    }

    if (strstr(result.message, "collision") == NULL) {
        fprintf(stderr, "expected 'collision' in message, got: %s\n", result.message);
        c_emitter_dispose(&emitter);
        exit(1);
    }

    c_emitter_dispose(&emitter);
}

/*
 * Case 3: non-colliding module still emits successfully. Guards
 * against a false-positive regression in the check.
 */
static void test_no_collision_emits_ok(void) {
    const zir_block func_blocks[] = {
        zir_make_block(
            "entry",
            NULL, 0,
            zir_make_return_terminator("0")),
    };
    const zir_function functions[] = {
        zir_make_function("alpha", NULL, 0, "int", func_blocks, ARRAY_COUNT(func_blocks)),
    };
    const zir_field_decl fields[] = {
        zir_make_field_decl("value", "int", NULL),
    };
    const zir_struct_decl structs[] = {
        zir_make_struct_decl("beta", fields, ARRAY_COUNT(fields)),
    };
    const zir_module module_decl = zir_make_module_with_structs(
        "ok",
        structs, ARRAY_COUNT(structs),
        functions, ARRAY_COUNT(functions));

    c_emitter emitter;
    c_emit_result result;

    c_emitter_init(&emitter);
    if (!c_emitter_emit_module(&emitter, &module_decl, &result)) {
        fprintf(stderr, "unexpected emitter failure for non-colliding module: %s\n", result.message);
        c_emitter_dispose(&emitter);
        exit(1);
    }
    c_emitter_dispose(&emitter);
}

int main(void) {
    test_function_and_struct_collision();
    test_dot_vs_underscore_struct_collision();
    test_no_collision_emits_ok();
    puts("C emitter symbol collision tests OK");
    return 0;
}
