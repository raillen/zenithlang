#ifndef ZENITH_NEXT_COMPILER_HIR_LOWERING_FROM_AST_H
#define ZENITH_NEXT_COMPILER_HIR_LOWERING_FROM_AST_H

#include "compiler/frontend/ast/model.h"
#include "compiler/hir/nodes/model.h"
#include "compiler/semantic/diagnostics/diagnostics.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct zt_hir_lower_result {
    zt_hir_module *module;
    zt_diag_list diagnostics;
} zt_hir_lower_result;

zt_hir_lower_result zt_lower_ast_to_hir(const zt_ast_node *root);
void zt_hir_lower_result_dispose(zt_hir_lower_result *result);

#ifdef __cplusplus
}
#endif

#endif

