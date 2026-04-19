#ifndef ZENITH_TOOLING_FORMATTER_H
#define ZENITH_TOOLING_FORMATTER_H

#include "compiler/frontend/ast/model.h"

#ifdef __cplusplus
extern "C" {
#endif

char *zt_format_node_to_string(const zt_ast_node *node);

#ifdef __cplusplus
}
#endif

#endif
