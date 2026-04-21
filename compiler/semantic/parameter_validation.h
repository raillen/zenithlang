#ifndef ZENITH_NEXT_COMPILER_SEMANTIC_PARAMETER_VALIDATION_H
#define ZENITH_NEXT_COMPILER_SEMANTIC_PARAMETER_VALIDATION_H

#include "compiler/frontend/ast/model.h"
#include "compiler/semantic/diagnostics/diagnostics.h"

#ifdef __cplusplus
extern "C" {
#endif

void zt_validate_parameter_ordering(zt_ast_node_list params, zt_diag_list *diagnostics);

void zt_validate_argument_ordering(
    zt_ast_node_list positional_args,
    zt_ast_named_arg_list named_args,
    zt_diag_list *diagnostics);

#ifdef __cplusplus
}
#endif

#endif

