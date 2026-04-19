/*
 * compiler/zir/lowering/from_hir.c
 * Stub implementation - the original was not recoverable from git blobs.
 * This provides the entry points declared in from_hir.h.
 */
#include "compiler/zir/lowering/from_hir.h"

#include <string.h>

zir_lower_result zir_lower_hir_to_zir(const zt_hir_module *module_decl) {
    zir_lower_result result;
    memset(&result, 0, sizeof(result));
    result.diagnostics = zt_diag_list_make();

    if (module_decl == NULL) {
        return result;
    }

    /* Stub: produce an empty ZIR module.
       The zt-next-v2.exe binary has the full implementation. */
    result.module.functions = NULL;
    result.module.function_count = 0;

    return result;
}

void zir_lower_result_dispose(zir_lower_result *result) {
    if (result == NULL) return;
    zt_diag_list_dispose(&result->diagnostics);
}
