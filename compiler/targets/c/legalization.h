#ifndef ZENITH_NEXT_COMPILER_TARGETS_C_LEGALIZATION_H
#define ZENITH_NEXT_COMPILER_TARGETS_C_LEGALIZATION_H

#include "compiler/zir/model.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum c_legalize_error_code {
    C_LEGALIZE_OK = 0,
    C_LEGALIZE_INVALID_INPUT,
    C_LEGALIZE_UNSUPPORTED_EXPR,
    C_LEGALIZE_UNSUPPORTED_TYPE
} c_legalize_error_code;

typedef enum c_legalized_seq_kind {
    C_LEGALIZED_SEQ_NONE = 0,
    C_LEGALIZED_SEQ_TEXT_INDEX,
    C_LEGALIZED_SEQ_TEXT_SLICE,
    C_LEGALIZED_SEQ_LIST_I64_INDEX,
    C_LEGALIZED_SEQ_LIST_I64_SLICE,
    C_LEGALIZED_SEQ_LIST_I64_LEN,
    C_LEGALIZED_SEQ_LIST_TEXT_INDEX,
    C_LEGALIZED_SEQ_LIST_TEXT_SLICE,
    C_LEGALIZED_SEQ_LIST_TEXT_LEN,
    C_LEGALIZED_SEQ_MAP_TEXT_TEXT_INDEX,
    C_LEGALIZED_SEQ_MAP_INDEX,
    C_LEGALIZED_SEQ_MAP_LEN
} c_legalized_seq_kind;

typedef struct c_legalize_result {
    int ok;
    c_legalize_error_code code;
    char message[256];
} c_legalize_result;

typedef struct c_legalized_seq_expr {
    c_legalized_seq_kind kind;
    char runtime_name[160];
    char sequence_type_name[96];
    char sequence_expr[128];
    char arg1_expr[64];
    char arg2_expr[64];
} c_legalized_seq_expr;

void c_legalize_result_init(c_legalize_result *result);
const char *c_legalize_error_code_name(c_legalize_error_code code);

int c_legalize_seq_expr(
    const zir_function *function_decl,
    const char *expr_text,
    const char *expected_type_name,
    c_legalized_seq_expr *out,
    c_legalize_result *result
);

int c_legalize_list_len_expr(
    const zir_function *function_decl,
    const char *expr_text,
    const char *expected_type_name,
    c_legalized_seq_expr *out,
    c_legalize_result *result
);

int c_legalize_zir_seq_expr(
    const zir_function *function_decl,
    const zir_expr *expr,
    const char *expected_type_name,
    c_legalized_seq_expr *out,
    c_legalize_result *result
);

int c_legalize_zir_list_len_expr(
    const zir_function *function_decl,
    const zir_expr *expr,
    const char *expected_type_name,
    c_legalized_seq_expr *out,
    c_legalize_result *result
);

#ifdef __cplusplus
}
#endif

#endif
