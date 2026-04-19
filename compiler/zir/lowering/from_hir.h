#ifndef ZENITH_NEXT_COMPILER_ZIR_LOWERING_FROM_HIR_H
#define ZENITH_NEXT_COMPILER_ZIR_LOWERING_FROM_HIR_H

#include "compiler/hir/nodes/model.h"
#include "compiler/semantic/diagnostics/diagnostics.h"
#include "compiler/zir/model.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct zir_lower_result {
    zir_module module;
    zt_diag_list diagnostics;
} zir_lower_result;

zir_lower_result zir_lower_hir_to_zir(const zt_hir_module *module_decl);
void zir_lower_result_dispose(zir_lower_result *result);

#ifdef __cplusplus
}
#endif

#endif

