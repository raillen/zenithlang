#ifndef ZENITH_NEXT_COMPILER_SEMANTIC_DIAGNOSTICS_H
#define ZENITH_NEXT_COMPILER_SEMANTIC_DIAGNOSTICS_H

#include "compiler/frontend/lexer/token.h"

#include <stdio.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum zt_diag_code {
    ZT_DIAG_PROJECT_ERROR,
    ZT_DIAG_PROJECT_IMPORT_CYCLE,
    ZT_DIAG_PROJECT_MISSING_ENTRY,
    ZT_DIAG_PROJECT_INVALID_NAMESPACE,
    ZT_DIAG_PROJECT_UNRESOLVED_IMPORT,
    ZT_DIAG_PROJECT_INVALID_INPUT,
    ZT_DIAG_PROJECT_INVALID_SECTION,
    ZT_DIAG_PROJECT_UNKNOWN_KEY,
    ZT_DIAG_PROJECT_INVALID_ASSIGNMENT,
    ZT_DIAG_PROJECT_INVALID_STRING,
    ZT_DIAG_PROJECT_MISSING_FIELD,
    ZT_DIAG_PROJECT_INVALID_KIND,
    ZT_DIAG_PROJECT_INVALID_TARGET,
    ZT_DIAG_PROJECT_INVALID_PROFILE,
    ZT_DIAG_PROJECT_PATH_TOO_LONG,
    ZT_DIAG_PROJECT_TOO_MANY_DEPENDENCIES,
    ZT_DIAG_ZIR_PARSE_ERROR,
    ZT_DIAG_ZIR_VERIFY_ERROR,
    ZT_DIAG_BACKEND_C_EMIT_ERROR,
    ZT_DIAG_BACKEND_C_LEGALIZE_ERROR,
    ZT_DIAG_SYNTAX_ERROR,
    ZT_DIAG_UNEXPECTED_TOKEN,
    ZT_DIAG_DUPLICATE_NAME,
    ZT_DIAG_SHADOWING,
    ZT_DIAG_UNRESOLVED_NAME,
    ZT_DIAG_INVALID_CONSTRAINT_TARGET,
    ZT_DIAG_INVALID_TYPE,
    ZT_DIAG_TYPE_MISMATCH,
    ZT_DIAG_INVALID_ASSIGNMENT,
    ZT_DIAG_CONST_REASSIGNMENT,
    ZT_DIAG_INVALID_RETURN,
    ZT_DIAG_INVALID_CONDITION_TYPE,
    ZT_DIAG_INVALID_CALL,
    ZT_DIAG_INVALID_ARGUMENT,
    ZT_DIAG_INVALID_OPERATOR,
    ZT_DIAG_INVALID_MAP_KEY_TYPE,
    ZT_DIAG_INVALID_MUTATION,
    ZT_DIAG_INVALID_CONVERSION,
    ZT_DIAG_INTEGER_OVERFLOW,
    ZT_DIAG_TOKEN_TOO_LONG,
    ZT_DIAG_PARAM_ORDERING,
    ZT_DIAG_NAMED_ARG_AFTER_POSITIONAL,
    ZT_DIAG_DOC_MALFORMED_BLOCK,
    ZT_DIAG_DOC_MISSING_TARGET,
    ZT_DIAG_DOC_MISSING_PAGE,
    ZT_DIAG_DOC_UNRESOLVED_TARGET,
    ZT_DIAG_DOC_UNRESOLVED_LINK,
    ZT_DIAG_DOC_INVALID_GUIDE_TAG,
    ZT_DIAG_DOC_INVALID_PAIRED_TAG,
    ZT_DIAG_DOC_MISSING_PUBLIC_DOC
} zt_diag_code;

typedef enum zt_diag_severity {
    ZT_DIAG_SEVERITY_ERROR,
    ZT_DIAG_SEVERITY_WARNING,
    ZT_DIAG_SEVERITY_NOTE,
    ZT_DIAG_SEVERITY_HELP
} zt_diag_severity;

typedef struct zt_diag {
    zt_diag_code code;
    zt_diag_severity severity;
    zt_source_span span;
    char message[512];
} zt_diag;

typedef struct zt_diag_list {
    zt_diag *items;
    size_t count;
    size_t capacity;
} zt_diag_list;

const char *zt_diag_code_name(zt_diag_code code);
const char *zt_diag_code_stable(zt_diag_code code);
const char *zt_diag_default_help(zt_diag_code code);
const char *zt_diag_severity_name(zt_diag_severity severity);

zt_diag_list zt_diag_list_make(void);
void zt_diag_list_dispose(zt_diag_list *list);
void zt_diag_list_add(zt_diag_list *list, zt_diag_code code, zt_source_span span, const char *format, ...);
void zt_diag_list_add_severity(
    zt_diag_list *list,
    zt_diag_code code,
    zt_diag_severity severity,
    zt_source_span span,
    const char *format,
    ...);
void zt_diag_render_detailed(FILE *stream, const char *stage, const zt_diag *diag);
void zt_diag_render_detailed_list(FILE *stream, const char *stage, const zt_diag_list *diagnostics);
void zt_diag_render_ci(FILE *stream, const char *stage, const zt_diag *diag);
void zt_diag_render_ci_list(FILE *stream, const char *stage, const zt_diag_list *diagnostics);

#ifdef __cplusplus
}
#endif

#endif
