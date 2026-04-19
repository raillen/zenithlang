#ifndef ZENITH_NEXT_COMPILER_FRONTEND_LEXER_LEXER_H
#define ZENITH_NEXT_COMPILER_FRONTEND_LEXER_LEXER_H

#include "compiler/frontend/lexer/token.h"

/* Forward declaration to avoid circular dependency */
typedef struct zt_diag_list zt_diag_list;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct zt_lexer zt_lexer;

zt_lexer *zt_lexer_make(const char *source_name, const char *source_text, size_t source_length);
void zt_lexer_dispose(zt_lexer *lexer);

/* Optional: register diagnostics list for lexer errors */
void zt_lexer_set_diagnostics(zt_lexer *lexer, zt_diag_list *diagnostics);

zt_token zt_lexer_next_token(zt_lexer *lexer);

zt_token zt_lexer_resume_string(zt_lexer *lexer);

zt_source_span zt_lexer_current_span(const zt_lexer *lexer);

int zt_lexer_is_at_end(const zt_lexer *lexer);

#ifdef __cplusplus
}
#endif

#endif
