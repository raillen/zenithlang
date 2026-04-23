#ifndef ZENITH_NEXT_COMPILER_TARGETS_C_EMITTER_H
#define ZENITH_NEXT_COMPILER_TARGETS_C_EMITTER_H

#include "compiler/zir/model.h"

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum c_emit_error_code {
    C_EMIT_OK = 0,
    C_EMIT_INVALID_INPUT,
    C_EMIT_UNSUPPORTED_TYPE,
    C_EMIT_UNSUPPORTED_EXPR,
    C_EMIT_INVALID_MAIN_SIGNATURE
} c_emit_error_code;

typedef struct c_emit_result {
    int ok;
    c_emit_error_code code;
    char message[256];
} c_emit_result;

typedef struct c_string_buffer {
    char *data;
    size_t length;
    size_t resident_length;
    size_t capacity;
    FILE *spill_file;
    size_t spill_threshold;
    int spilled;
} c_string_buffer;

typedef struct c_emitter {
    c_string_buffer buffer;
    const char *newline;
} c_emitter;

void c_emit_result_init(c_emit_result *result);
const char *c_emit_error_code_name(c_emit_error_code code);

void c_emitter_init(c_emitter *emitter);
void c_emitter_reset(c_emitter *emitter);
void c_emitter_dispose(c_emitter *emitter);
const char *c_emitter_text(c_emitter *emitter);
int c_emitter_write_file(c_emitter *emitter, const char *path);
int c_emitter_emit_module(c_emitter *emitter, const zir_module *module_decl, c_emit_result *result);

#ifdef __cplusplus
}
#endif

#endif
