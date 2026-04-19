#ifndef ZENITH_NEXT_COMPILER_FRONTEND_PARSER_PARSER_H
#define ZENITH_NEXT_COMPILER_FRONTEND_PARSER_PARSER_H

#include "compiler/frontend/ast/model.h"
#include "compiler/frontend/lexer/lexer.h"
#include "compiler/utils/string_pool.h"
#include "compiler/utils/arena.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "compiler/semantic/diagnostics/diagnostics.h"

typedef struct zt_parser_result {
    zt_ast_node *root;
    zt_diag_list diagnostics;
} zt_parser_result;

void zt_parser_result_dispose(zt_parser_result *result);

zt_parser_result zt_parse(zt_arena *arena, zt_string_pool *pool, const char *source_name, const char *source_text, size_t source_length);

#ifdef __cplusplus
}
#endif

#endif
