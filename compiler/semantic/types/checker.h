#ifndef ZENITH_NEXT_COMPILER_SEMANTIC_TYPES_CHECKER_H
#define ZENITH_NEXT_COMPILER_SEMANTIC_TYPES_CHECKER_H

#include "compiler/frontend/ast/model.h"
#include "compiler/semantic/diagnostics/diagnostics.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct zt_check_result {
    zt_diag_list diagnostics;
} zt_check_result;

zt_check_result zt_check_file(const zt_ast_node *root);
void zt_check_result_dispose(zt_check_result *result);

#ifdef __cplusplus
}
#endif

#endif

