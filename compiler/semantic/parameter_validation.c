/*
 * parameter_validation.c
 * 
 * Validation functions for function parameters and arguments.
 * These functions are designed to be called by the semantic binder
 * to validate parameter ordering and argument usage.
 */

#include "compiler/semantic/diagnostics/diagnostics.h"
#include "compiler/frontend/ast/model.h"

#include <stddef.h>

/*
 * Validate that required parameters come before defaulted parameters.
 * 
 * Spec: "required parameters come before defaulted parameters"
 *       "defaulted parameters form a trailing suffix"
 * 
 * Example of INVALID code:
 *   func foo(a: int = 1, b: int)  -- ERROR: required after default
 * 
 * Example of VALID code:
 *   func foo(a: int, b: int = 1)  -- OK: required before default
 */
void zt_validate_parameter_ordering(zt_ast_node_list params, zt_diag_list *diagnostics) {
    if (params.items == NULL || diagnostics == NULL) {
        return;
    }
    
    int seen_default = 0;
    
    for (size_t i = 0; i < params.count; i++) {
        zt_ast_node *param = params.items[i];
        if (param == NULL || param->kind != ZT_AST_PARAM) {
            continue;
        }
        
        if (param->as.param.default_value != NULL) {
            /* This parameter has a default value */
            seen_default = 1;
        } else if (seen_default) {
            /* ERROR: Required parameter after a defaulted parameter */
            zt_diag_list_add(
                diagnostics,
                ZT_DIAG_PARAM_ORDERING,
                param->span,
                "required parameter '%s' cannot follow defaulted parameter",
                param->as.param.name
            );
        }
    }
}

/*
 * Validate that named arguments don't appear before positional arguments.
 * 
 * Spec: "after a named argument appears, remaining provided arguments are named"
 * 
 * Example of INVALID code:
 *   foo(1, name: "value", 2)  -- ERROR: positional after named
 * 
 * Example of VALID code:
 *   foo(1, 2, name: "value")  -- OK: all positionals before named
 *   foo(name: "value", other: 1)  -- OK: all named
 */
void zt_validate_argument_ordering(
    zt_ast_node_list positional_args,
    zt_ast_named_arg_list named_args,
    zt_diag_list *diagnostics) {
    
    if (diagnostics == NULL) {
        return;
    }
    
    /* 
     * This validation would check that in the original source,
     * positional arguments don't appear after named arguments.
     * 
     * However, since the parser already separates them into different
     * lists, we can't detect the original ordering here.
     * 
     * This validation should be done during parsing if we want to
     * catch this error, or we need to track the original order.
     * 
     * For now, this is a placeholder for future implementation.
     */
    (void)positional_args;
    (void)named_args;
}
