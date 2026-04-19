#ifndef ZENITH_NEXT_COMPILER_ZIR_VERIFIER_H
#define ZENITH_NEXT_COMPILER_ZIR_VERIFIER_H

#include "compiler/zir/model.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum zir_verifier_code {
    ZIR_VERIFIER_OK = 0,
    ZIR_VERIFIER_INVALID_INPUT,
    ZIR_VERIFIER_INVALID_TYPE_ANY,
    ZIR_VERIFIER_INVALID_TARGET_LEAK,
    ZIR_VERIFIER_USE_BEFORE_DEFINITION,
    ZIR_VERIFIER_UNKNOWN_BLOCK,
    ZIR_VERIFIER_INVALID_TERMINATOR,
    ZIR_VERIFIER_DUPLICATE_BLOCK_LABEL,
    ZIR_VERIFIER_INVALID_RETURN
} zir_verifier_code;

typedef struct zir_verifier_result {
    int ok;
    zir_verifier_code code;
    const char *source_name;
    size_t line;
    size_t column;
    int has_span;
    char message[256];
} zir_verifier_result;

void zir_verifier_result_init(zir_verifier_result *result);
const char *zir_verifier_code_name(zir_verifier_code code);
int zir_verify_module(const zir_module *module_decl, zir_verifier_result *result);

#ifdef __cplusplus
}
#endif

#endif

