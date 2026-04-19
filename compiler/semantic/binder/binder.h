#ifndef ZENITH_NEXT_COMPILER_SEMANTIC_BINDER_H
#define ZENITH_NEXT_COMPILER_SEMANTIC_BINDER_H

#include "compiler/frontend/ast/model.h"
#include "compiler/semantic/diagnostics/diagnostics.h"
#include "compiler/semantic/symbols/symbols.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct zt_bind_result {
    zt_scope module_scope;
    zt_diag_list diagnostics;
} zt_bind_result;

zt_bind_result zt_bind_file(const zt_ast_node *root);
void zt_bind_result_dispose(zt_bind_result *result);

#ifdef __cplusplus
}
#endif

#endif

