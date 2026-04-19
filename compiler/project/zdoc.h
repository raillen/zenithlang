#ifndef ZENITH_NEXT_COMPILER_PROJECT_ZDOC_H
#define ZENITH_NEXT_COMPILER_PROJECT_ZDOC_H

#include "compiler/frontend/ast/model.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct zt_zdoc_source_unit {
    const char *source_path;
    const zt_ast_node *root;
} zt_zdoc_source_unit;

typedef enum zt_zdoc_severity {
    ZT_ZDOC_ERROR = 0,
    ZT_ZDOC_WARNING
} zt_zdoc_severity;

typedef enum zt_zdoc_code {
    ZT_ZDOC_MALFORMED_BLOCK = 0,
    ZT_ZDOC_MISSING_TARGET,
    ZT_ZDOC_MISSING_PAGE,
    ZT_ZDOC_UNRESOLVED_TARGET,
    ZT_ZDOC_UNRESOLVED_LINK,
    ZT_ZDOC_INVALID_GUIDE_TAG,
    ZT_ZDOC_INVALID_PAIRED_TAG,
    ZT_ZDOC_MISSING_PUBLIC_DOC
} zt_zdoc_code;

typedef struct zt_zdoc_diagnostic {
    zt_zdoc_severity severity;
    zt_zdoc_code code;
    char path[512];
    size_t line;
    size_t column;
    char message[256];
    char help[256];
} zt_zdoc_diagnostic;

typedef struct zt_zdoc_diagnostic_list {
    zt_zdoc_diagnostic *items;
    size_t count;
    size_t capacity;
} zt_zdoc_diagnostic_list;

void zt_zdoc_diagnostic_list_init(zt_zdoc_diagnostic_list *list);
void zt_zdoc_diagnostic_list_dispose(zt_zdoc_diagnostic_list *list);
size_t zt_zdoc_diagnostic_error_count(const zt_zdoc_diagnostic_list *list);
size_t zt_zdoc_diagnostic_warning_count(const zt_zdoc_diagnostic_list *list);

const char *zt_zdoc_severity_name(zt_zdoc_severity severity);
const char *zt_zdoc_code_name(zt_zdoc_code code);

int zt_zdoc_check_project(
    const char *project_root,
    const char *source_root,
    const char *zdoc_root,
    const zt_zdoc_source_unit *sources,
    size_t source_count,
    zt_zdoc_diagnostic_list *diagnostics
);

#ifdef __cplusplus
}
#endif

#endif
