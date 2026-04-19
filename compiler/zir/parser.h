#ifndef ZENITH_NEXT_COMPILER_ZIR_PARSER_H
#define ZENITH_NEXT_COMPILER_ZIR_PARSER_H

#include "compiler/zir/model.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum zir_parse_error_code {
    ZIR_PARSE_OK = 0,
    ZIR_PARSE_INVALID_INPUT,
    ZIR_PARSE_EXPECTED_MODULE,
    ZIR_PARSE_EXPECTED_FUNC,
    ZIR_PARSE_EXPECTED_BLOCK,
    ZIR_PARSE_EXPECTED_ARROW,
    ZIR_PARSE_EXPECTED_COLON,
    ZIR_PARSE_EXPECTED_LPAREN,
    ZIR_PARSE_EXPECTED_RPAREN,
    ZIR_PARSE_EXPECTED_IDENTIFIER,
    ZIR_PARSE_TOO_MANY_PARAMS,
    ZIR_PARSE_TOO_MANY_BLOCKS,
    ZIR_PARSE_TOO_MANY_INSTRUCTIONS,
    ZIR_PARSE_TOO_MANY_FUNCTIONS,
    ZIR_PARSE_INVALID_TERMINATOR,
    ZIR_PARSE_UNKNOWN_LINE,
    ZIR_PARSE_OUT_OF_MEMORY
} zir_parse_error_code;

typedef struct zir_parse_result {
    int ok;
    zir_parse_error_code code;
    int line_number;
    char message[256];
    zir_module module;
    zir_param param_storage[256];
    zir_block block_storage[512];
    zir_function function_storage[64];
    zir_instruction instruction_storage[2048];
    zir_terminator terminator_storage[512];
} zir_parse_result;

void zir_parse_result_init(zir_parse_result *result);
void zir_parse_result_dispose(zir_parse_result *result);
const char *zir_parse_error_code_name(zir_parse_error_code code);

int zir_parse_module(const char *text, size_t length, zir_parse_result *result);

#ifdef __cplusplus
}
#endif

#endif
