#ifndef ZENITH_NEXT_COMPILER_SEMANTIC_TYPES_CHECKER_H
#define ZENITH_NEXT_COMPILER_SEMANTIC_TYPES_CHECKER_H

#include "compiler/frontend/ast/model.h"
#include "compiler/semantic/diagnostics/diagnostics.h"
#include "compiler/semantic/types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct zt_check_result {
    zt_diag_list diagnostics;
} zt_check_result;

zt_check_result zt_check_file(const zt_ast_node *root);
void zt_check_result_dispose(zt_check_result *result);
int zt_checker_type_is_transferable(const zt_ast_node *root, const zt_type *type);
zt_type *zt_checker_resolve_type_node(const zt_ast_node *root, const zt_ast_node *type_node);

#ifdef __cplusplus
}
#endif

#endif
