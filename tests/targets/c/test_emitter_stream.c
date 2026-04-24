#include "compiler/targets/c/emitter.h"
#include "compiler/zir/model.h"
#include "compiler/zir/verifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_COUNT(items) (sizeof(items) / sizeof((items)[0]))

static void fail_now(const char *message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

static void verify_or_fail(const zir_module *module_decl) {
    zir_verifier_result result;

    if (!zir_verify_module(module_decl, &result)) {
        fprintf(stderr, "verifier rejeitou modulo stream emitter\n");
        fprintf(stderr, "codigo: %s\n", zir_verifier_code_name(result.code));
        fprintf(stderr, "mensagem: %s\n", result.message);
        exit(1);
    }
}

static char *read_file_text(FILE *file) {
    long length;
    char *text;

    if (fflush(file) != 0 || fseek(file, 0, SEEK_END) != 0) {
        return NULL;
    }

    length = ftell(file);
    if (length < 0 || fseek(file, 0, SEEK_SET) != 0) {
        return NULL;
    }

    text = (char *)malloc((size_t)length + 1);
    if (text == NULL) {
        return NULL;
    }

    if (length > 0 && fread(text, 1, (size_t)length, file) != (size_t)length) {
        free(text);
        return NULL;
    }

    text[length] = '\0';
    return text;
}

static void test_stream_matches_materialized_when_spilled(void) {
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
    c_emitter emitter;
    c_emit_result result;
    FILE *stream_file;
    char *stream_text;
    const char *materialized;

    verify_or_fail(&module_decl);

    c_emitter_init(&emitter);
    emitter.buffer.spill_threshold = 64;
    if (!c_emitter_emit_module(&emitter, &module_decl, &result)) {
        fprintf(stderr, "falha emitter: %s\n", result.message);
        c_emitter_dispose(&emitter);
        exit(1);
    }

    if (!emitter.buffer.spilled) {
        c_emitter_dispose(&emitter);
        fail_now("falha: emitter nao ativou spill no teste focado");
    }

    stream_file = tmpfile();
    if (stream_file == NULL) {
        c_emitter_dispose(&emitter);
        fail_now("falha: tmpfile indisponivel para stream emitter");
    }

    if (!c_emitter_write_stream(&emitter, stream_file)) {
        fclose(stream_file);
        c_emitter_dispose(&emitter);
        fail_now("falha: c_emitter_write_stream nao conseguiu persistir o modulo");
    }

    stream_text = read_file_text(stream_file);
    fclose(stream_file);
    if (stream_text == NULL) {
        c_emitter_dispose(&emitter);
        fail_now("falha: nao foi possivel reler a saida em stream");
    }

    materialized = c_emitter_text(&emitter);
    if (strcmp(stream_text, materialized) != 0) {
        fprintf(stderr, "falha: stream divergiu da materializacao\n");
        fprintf(stderr, "stream:\n%s\n", stream_text);
        fprintf(stderr, "materializado:\n%s\n", materialized);
        free(stream_text);
        c_emitter_dispose(&emitter);
        exit(1);
    }

    free(stream_text);
    c_emitter_dispose(&emitter);
}

int main(void) {
    test_stream_matches_materialized_when_spilled();
    puts("C emitter stream tests OK");
    return 0;
}
