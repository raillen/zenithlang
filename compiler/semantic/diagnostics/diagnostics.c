#include "compiler/semantic/diagnostics/diagnostics.h"
#include "compiler/utils/l10n.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ZT_DIAG_MAX_TOTAL 256u
#define ZT_DIAG_RECENT_DEDUP_WINDOW 64u

static int zt_diag_same_text(const char *left, const char *right) {
    if (left == right) return 1;
    if (left == NULL || right == NULL) return 0;
    return strcmp(left, right) == 0;
}

static int zt_diag_same_span(zt_source_span left, zt_source_span right) {
    return left.line == right.line &&
           left.column_start == right.column_start &&
           left.column_end == right.column_end &&
           zt_diag_same_text(left.source_name, right.source_name);
}

static int zt_diag_is_duplicate_recent(
        const zt_diag_list *list,
        zt_diag_code code,
        zt_source_span span,
        const char *message) {
    size_t start;
    size_t i;

    if (list == NULL || list->count == 0) return 0;

    start = list->count > ZT_DIAG_RECENT_DEDUP_WINDOW
        ? list->count - ZT_DIAG_RECENT_DEDUP_WINDOW
        : 0;

    for (i = list->count; i > start; i -= 1) {
        const zt_diag *existing = &list->items[i - 1];
        if (existing->code != code) continue;
        if (!zt_diag_same_span(existing->span, span)) continue;
        if (strcmp(existing->message, message) != 0) continue;
        return 1;
    }

    return 0;
}
static void zt_diag_print_source_span(FILE *stream, zt_source_span span) {
    const char *source_name = (span.source_name != NULL && span.source_name[0] != '\0')
        ? span.source_name
        : "<unknown>";
    size_t line = span.line > 0 ? span.line : 1;
    size_t column_start = span.column_start > 0 ? span.column_start : 1;
    size_t column_end = span.column_end > 0 ? span.column_end : column_start;

    fprintf(stream, "%s:%zu:%zu", source_name, line, column_start);
    if (column_end > column_start) {
        fprintf(stream, "-%zu", column_end);
    }
}

static int zt_diag_read_source_line(const char *path, size_t target_line, char *line, size_t capacity) {
    FILE *file;
    int ch;
    size_t current_line = 1;
    size_t out_len = 0;
    int found = 0;

    if (path == NULL || path[0] == '\0' || line == NULL || capacity == 0 || target_line == 0) return 0;

    file = fopen(path, "rb");
    if (file == NULL) return 0;

    while ((ch = fgetc(file)) != EOF) {
        if (current_line == target_line) {
            found = 1;
            if (ch == '\r') continue;
            if (ch == '\n') break;
            if (out_len + 1 < capacity) {
                line[out_len++] = (char)ch;
            }
        }

        if (ch == '\n') {
            if (current_line == target_line) break;
            current_line += 1;
        }
    }

    fclose(file);

    if (!found && current_line == target_line) {
        found = 1;
    }

    if (!found) return 0;

    line[out_len] = '\0';
    return 1;
}

static void zt_diag_render_caret(FILE *stream, zt_source_span span) {
    size_t start = span.column_start > 0 ? span.column_start : 1;
    size_t end = span.column_end >= start ? span.column_end : start;
    size_t i;

    fprintf(stream, "    | ");
    for (i = 1; i < start; i += 1) {
        fputc(' ', stream);
    }
    for (i = start; i <= end; i += 1) {
        fputc('^', stream);
    }
    fputc('\n', stream);
}

const char *zt_diag_code_name(zt_diag_code code) {
    switch (code) {
        case ZT_DIAG_PROJECT_ERROR: return "project_error";
        case ZT_DIAG_PROJECT_IMPORT_CYCLE: return "project_import_cycle";
        case ZT_DIAG_PROJECT_MISSING_ENTRY: return "project_missing_entry";
        case ZT_DIAG_PROJECT_INVALID_NAMESPACE: return "project_invalid_namespace";
        case ZT_DIAG_PROJECT_UNRESOLVED_IMPORT: return "project_unresolved_import";
        case ZT_DIAG_PROJECT_INVALID_INPUT: return "project_invalid_input";
        case ZT_DIAG_PROJECT_INVALID_SECTION: return "project_invalid_section";
        case ZT_DIAG_PROJECT_UNKNOWN_KEY: return "project_unknown_key";
        case ZT_DIAG_PROJECT_INVALID_ASSIGNMENT: return "project_invalid_assignment";
        case ZT_DIAG_PROJECT_INVALID_STRING: return "project_invalid_string";
        case ZT_DIAG_PROJECT_MISSING_FIELD: return "project_missing_field";
        case ZT_DIAG_PROJECT_INVALID_KIND: return "project_invalid_kind";
        case ZT_DIAG_PROJECT_INVALID_TARGET: return "project_invalid_target";
        case ZT_DIAG_PROJECT_INVALID_PROFILE: return "project_invalid_profile";
        case ZT_DIAG_PROJECT_PATH_TOO_LONG: return "project_path_too_long";
        case ZT_DIAG_PROJECT_TOO_MANY_DEPENDENCIES: return "project_too_many_dependencies";
        case ZT_DIAG_PROJECT_INVALID_MONOMORPHIZATION_LIMIT: return "project_invalid_monomorphization_limit";
        case ZT_DIAG_PROJECT_MONOMORPHIZATION_LIMIT_EXCEEDED: return "project_monomorphization_limit_exceeded";
        case ZT_DIAG_ZIR_PARSE_ERROR: return "zir_parse_error";
        case ZT_DIAG_ZIR_VERIFY_ERROR: return "zir_verify_error";
        case ZT_DIAG_BACKEND_C_EMIT_ERROR: return "backend_c_emit_error";
        case ZT_DIAG_BACKEND_C_LEGALIZE_ERROR: return "backend_c_legalize_error";
        case ZT_DIAG_SYNTAX_ERROR: return "syntax_error";
        case ZT_DIAG_UNEXPECTED_TOKEN: return "unexpected_token";
        case ZT_DIAG_DUPLICATE_NAME: return "duplicate_name";
        case ZT_DIAG_SHADOWING: return "shadowing";
        case ZT_DIAG_UNRESOLVED_NAME: return "unresolved_name";
        case ZT_DIAG_INVALID_CONSTRAINT_TARGET: return "invalid_constraint_target";
        case ZT_DIAG_INVALID_TYPE: return "invalid_type";
        case ZT_DIAG_TYPE_MISMATCH: return "type_mismatch";
        case ZT_DIAG_INVALID_ASSIGNMENT: return "invalid_assignment";
        case ZT_DIAG_CONST_REASSIGNMENT: return "const_reassignment";
        case ZT_DIAG_INVALID_RETURN: return "invalid_return";
        case ZT_DIAG_INVALID_CONDITION_TYPE: return "invalid_condition_type";
        case ZT_DIAG_INVALID_CALL: return "invalid_call";
        case ZT_DIAG_INVALID_ARGUMENT: return "invalid_argument";
        case ZT_DIAG_INVALID_OPERATOR: return "invalid_operator";
        case ZT_DIAG_INVALID_MAP_KEY_TYPE: return "invalid_map_key_type";
        case ZT_DIAG_INVALID_MUTATION: return "invalid_mutation";
        case ZT_DIAG_INVALID_CONVERSION: return "invalid_conversion";
        case ZT_DIAG_INTEGER_OVERFLOW: return "integer_overflow";
        case ZT_DIAG_TOKEN_TOO_LONG: return "token_too_long";
        case ZT_DIAG_PARAM_ORDERING: return "param_ordering";
        case ZT_DIAG_NAMED_ARG_AFTER_POSITIONAL: return "named_arg_after_positional";
        case ZT_DIAG_DOC_MALFORMED_BLOCK: return "doc_malformed_block";
        case ZT_DIAG_DOC_MISSING_TARGET: return "doc_missing_target";
        case ZT_DIAG_DOC_MISSING_PAGE: return "doc_missing_page";
        case ZT_DIAG_DOC_UNRESOLVED_TARGET: return "doc_unresolved_target";
        case ZT_DIAG_DOC_UNRESOLVED_LINK: return "doc_unresolved_link";
        case ZT_DIAG_DOC_INVALID_GUIDE_TAG: return "doc_invalid_guide_tag";
        case ZT_DIAG_DOC_INVALID_PAIRED_TAG: return "doc_invalid_paired_tag";
        case ZT_DIAG_DOC_MISSING_PUBLIC_DOC: return "doc_missing_public_doc";
        default: return "unknown";
    }
}

const char *zt_diag_code_stable(zt_diag_code code) {
    switch (code) {
        case ZT_DIAG_PROJECT_ERROR: return "project.error";
        case ZT_DIAG_PROJECT_IMPORT_CYCLE: return "project.import_cycle";
        case ZT_DIAG_PROJECT_MISSING_ENTRY: return "project.missing_entry";
        case ZT_DIAG_PROJECT_INVALID_NAMESPACE: return "project.invalid_namespace";
        case ZT_DIAG_PROJECT_UNRESOLVED_IMPORT: return "project.unresolved_import";
        case ZT_DIAG_PROJECT_INVALID_INPUT: return "project.invalid_input";
        case ZT_DIAG_PROJECT_INVALID_SECTION: return "project.invalid_section";
        case ZT_DIAG_PROJECT_UNKNOWN_KEY: return "project.unknown_key";
        case ZT_DIAG_PROJECT_INVALID_ASSIGNMENT: return "project.invalid_assignment";
        case ZT_DIAG_PROJECT_INVALID_STRING: return "project.invalid_string";
        case ZT_DIAG_PROJECT_MISSING_FIELD: return "project.missing_field";
        case ZT_DIAG_PROJECT_INVALID_KIND: return "project.invalid_kind";
        case ZT_DIAG_PROJECT_INVALID_TARGET: return "project.invalid_target";
        case ZT_DIAG_PROJECT_INVALID_PROFILE: return "project.invalid_profile";
        case ZT_DIAG_PROJECT_PATH_TOO_LONG: return "project.path_too_long";
        case ZT_DIAG_PROJECT_TOO_MANY_DEPENDENCIES: return "project.too_many_dependencies";
        case ZT_DIAG_PROJECT_INVALID_MONOMORPHIZATION_LIMIT: return "project.invalid_monomorphization_limit";
        case ZT_DIAG_PROJECT_MONOMORPHIZATION_LIMIT_EXCEEDED: return "project.monomorphization_limit_exceeded";
        case ZT_DIAG_ZIR_PARSE_ERROR: return "zir.parse";
        case ZT_DIAG_ZIR_VERIFY_ERROR: return "zir.verify";
        case ZT_DIAG_BACKEND_C_EMIT_ERROR: return "backend.c.emit";
        case ZT_DIAG_BACKEND_C_LEGALIZE_ERROR: return "backend.c.legalize";
        case ZT_DIAG_SYNTAX_ERROR: return "syntax.error";
        case ZT_DIAG_UNEXPECTED_TOKEN: return "syntax.unexpected_token";
        case ZT_DIAG_DUPLICATE_NAME: return "name.duplicate";
        case ZT_DIAG_SHADOWING: return "name.shadowing";
        case ZT_DIAG_UNRESOLVED_NAME: return "name.unresolved";
        case ZT_DIAG_INVALID_CONSTRAINT_TARGET: return "generic.constraint_target";
        case ZT_DIAG_INVALID_TYPE: return "type.invalid";
        case ZT_DIAG_TYPE_MISMATCH: return "type.mismatch";
        case ZT_DIAG_INVALID_ASSIGNMENT: return "mutability.invalid_assignment";
        case ZT_DIAG_CONST_REASSIGNMENT: return "mutability.const_update";
        case ZT_DIAG_INVALID_RETURN: return "control_flow.invalid_return";
        case ZT_DIAG_INVALID_CONDITION_TYPE: return "control_flow.invalid_condition";
        case ZT_DIAG_INVALID_CALL: return "type.invalid_call";
        case ZT_DIAG_INVALID_ARGUMENT: return "type.invalid_argument";
        case ZT_DIAG_INVALID_OPERATOR: return "type.invalid_operator";
        case ZT_DIAG_INVALID_MAP_KEY_TYPE: return "type.invalid_map_key";
        case ZT_DIAG_INVALID_MUTATION: return "mutability.invalid_update";
        case ZT_DIAG_INVALID_CONVERSION: return "type.invalid_conversion";
        case ZT_DIAG_INTEGER_OVERFLOW: return "type.integer_overflow";
        case ZT_DIAG_NON_EXHAUSTIVE_MATCH: return "control_flow.non_exhaustive_match";
        case ZT_DIAG_TOKEN_TOO_LONG: return "lexer.token_too_long";
        case ZT_DIAG_PARAM_ORDERING: return "semantic.param_ordering";
        case ZT_DIAG_NAMED_ARG_AFTER_POSITIONAL: return "semantic.named_arg_after_positional";
        case ZT_DIAG_DOC_MALFORMED_BLOCK: return "doc.malformed_block";
        case ZT_DIAG_DOC_MISSING_TARGET: return "doc.missing_target";
        case ZT_DIAG_DOC_MISSING_PAGE: return "doc.missing_page";
        case ZT_DIAG_DOC_UNRESOLVED_TARGET: return "doc.unresolved_target";
        case ZT_DIAG_DOC_UNRESOLVED_LINK: return "doc.unresolved_link";
        case ZT_DIAG_DOC_INVALID_GUIDE_TAG: return "doc.invalid_guide_tag";
        case ZT_DIAG_DOC_INVALID_PAIRED_TAG: return "doc.invalid_paired_tag";
        case ZT_DIAG_DOC_MISSING_PUBLIC_DOC: return "doc.missing_public_doc";
        default: return "internal.unknown";
    }
}

const char *zt_diag_default_help(zt_diag_code code) {
    const char *localized = zt_l10n_default_help(code);
    if (localized != NULL) return localized;

    switch (code) {
        case ZT_DIAG_PROJECT_ERROR: return "Check the zenith.ztproj file for errors.";
        case ZT_DIAG_PROJECT_IMPORT_CYCLE: return "Refactor dependencies to eliminate circular imports.";
        case ZT_DIAG_PROJECT_MISSING_ENTRY: return "Ensure the app.entry points to a valid namespace inside source.root.";
        case ZT_DIAG_PROJECT_INVALID_NAMESPACE: return "Ensure the file directory layout matches the namespace exactly.";
        case ZT_DIAG_PROJECT_UNRESOLVED_IMPORT: return "Ensure the imported namespace exists in the source directory.";
        case ZT_DIAG_PROJECT_INVALID_INPUT: return "Check manifest path and file readability.";
        case ZT_DIAG_PROJECT_INVALID_SECTION: return "Use only supported manifest sections.";
        case ZT_DIAG_PROJECT_UNKNOWN_KEY: return "Use only supported keys for this section.";
        case ZT_DIAG_PROJECT_INVALID_ASSIGNMENT: return "Use key = value assignments with valid TOML-like syntax.";
        case ZT_DIAG_PROJECT_INVALID_STRING: return "Use quoted string values and close all quotes.";
        case ZT_DIAG_PROJECT_MISSING_FIELD: return "Provide all required fields for this project kind.";
        case ZT_DIAG_PROJECT_INVALID_KIND: return "Set project.kind to app or lib.";
        case ZT_DIAG_PROJECT_INVALID_TARGET: return "Set build.target to native in the bootstrap driver.";
        case ZT_DIAG_PROJECT_INVALID_PROFILE: return "Set build.profile to debug or release.";
        case ZT_DIAG_PROJECT_PATH_TOO_LONG: return "Shorten manifest paths or project root path length.";
        case ZT_DIAG_PROJECT_TOO_MANY_DEPENDENCIES: return "Reduce dependencies or increase parser limits.";
        case ZT_DIAG_PROJECT_INVALID_MONOMORPHIZATION_LIMIT: return "Set build.monomorphization_limit to a positive integer.";
        case ZT_DIAG_PROJECT_MONOMORPHIZATION_LIMIT_EXCEEDED: return "Reduce generic combinations or raise build.monomorphization_limit.";
        case ZT_DIAG_ZIR_PARSE_ERROR: return "Check ZIR syntax and block/function structure around the reported line.";
        case ZT_DIAG_ZIR_VERIFY_ERROR: return "Fix invalid SSA/control-flow invariants in the generated or handwritten ZIR.";
        case ZT_DIAG_BACKEND_C_EMIT_ERROR: return "Adjust unsupported constructs or types before C emission.";
        case ZT_DIAG_BACKEND_C_LEGALIZE_ERROR: return "Adjust sequence expressions to forms supported by C legalization.";
        case ZT_DIAG_SYNTAX_ERROR: return "Review the syntax near the error.";
        case ZT_DIAG_UNEXPECTED_TOKEN: return "Check for missing separators or invalid characters.";
        case ZT_DIAG_DUPLICATE_NAME: return "Rename one of the declarations in this scope.";
        case ZT_DIAG_SHADOWING: return "Use a different local name to avoid shadowing an outer declaration.";
        case ZT_DIAG_UNRESOLVED_NAME: return "Declare or import this name before using it.";
        case ZT_DIAG_INVALID_CONSTRAINT_TARGET: return "Apply constraints only to generic type parameters.";
        case ZT_DIAG_INVALID_TYPE: return "Check the type name and generic arguments.";
        case ZT_DIAG_TYPE_MISMATCH: return "Convert the value explicitly or change the expected type.";
        case ZT_DIAG_INVALID_ASSIGNMENT: return "Ensure the assignment target is mutable and type-compatible.";
        case ZT_DIAG_CONST_REASSIGNMENT: return "Use var if this binding must be reassigned.";
        case ZT_DIAG_INVALID_RETURN: return "Match return values to the function return type on every path.";
        case ZT_DIAG_INVALID_CONDITION_TYPE: return "Use a bool expression in this condition.";
        case ZT_DIAG_INVALID_CALL: return "Call a function value and provide the required arguments.";
        case ZT_DIAG_INVALID_ARGUMENT: return "Check argument names, order and arity for this call.";
        case ZT_DIAG_INVALID_OPERATOR: return "Use operands compatible with this operator.";
        case ZT_DIAG_INVALID_MAP_KEY_TYPE: return "Use a key type that implements Equatable and Hashable.";
        case ZT_DIAG_INVALID_MUTATION: return "Mark the receiver or binding as mutable before mutating.";
        case ZT_DIAG_INVALID_CONVERSION: return "Use a supported explicit conversion for this source type.";
        case ZT_DIAG_INTEGER_OVERFLOW: return "Use a wider numeric type or reduce the arithmetic range.";
        case ZT_DIAG_NON_EXHAUSTIVE_MATCH: return "Add missing cases or a default -> case to cover all variants.";
        case ZT_DIAG_TOKEN_TOO_LONG: return "Reduce the token length to fit within the 1024 character limit.";
        case ZT_DIAG_PARAM_ORDERING: return "Required parameters must come before parameters with default values.";
        case ZT_DIAG_NAMED_ARG_AFTER_POSITIONAL: return "After a named argument appears, all remaining arguments must be named.";
        case ZT_DIAG_DOC_MALFORMED_BLOCK: return "Wrap text in --- blocks and close each block.";
        case ZT_DIAG_DOC_MISSING_TARGET: return "Add @target: <symbol> in paired API docs.";
        case ZT_DIAG_DOC_MISSING_PAGE: return "Add @page: <slug> in guide docs under zdoc/guides.";
        case ZT_DIAG_DOC_UNRESOLVED_TARGET: return "Use namespace, local symbol name, or a qualified symbol that exists.";
        case ZT_DIAG_DOC_UNRESOLVED_LINK: return "Check the link target name or document the referenced symbol.";
        case ZT_DIAG_DOC_INVALID_GUIDE_TAG: return "Guide docs accept @page and do not accept @target.";
        case ZT_DIAG_DOC_INVALID_PAIRED_TAG: return "Paired API docs accept @target and do not accept @page.";
        case ZT_DIAG_DOC_MISSING_PUBLIC_DOC: return "Add a paired ZDoc block for this public symbol.";
        default: return NULL;
    }
}

const char *zt_diag_severity_name(zt_diag_severity severity) {
    return zt_l10n_severity_name(severity);
}

zt_diag_list zt_diag_list_make(void) {
    zt_diag_list list;
    list.items = NULL;
    list.count = 0;
    list.capacity = 0;
    return list;
}

void zt_diag_list_dispose(zt_diag_list *list) {
    if (list == NULL) return;
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

static void zt_diag_list_add_va(
        zt_diag_list *list,
        zt_diag_code code,
        zt_diag_severity severity,
        zt_source_span span,
        const char *format,
        va_list args) {
    char message[512];

    if (list == NULL) return;
    if (format == NULL) return;

    vsnprintf(message, sizeof(message), format, args);

    if (zt_diag_is_duplicate_recent(list, code, span, message)) return;
    if (list->count >= ZT_DIAG_MAX_TOTAL) return;

    if (list->count >= list->capacity) {
        size_t new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        zt_diag *new_items = (zt_diag *)realloc(list->items, new_capacity * sizeof(zt_diag));
        if (new_items == NULL) return;
        list->items = new_items;
        list->capacity = new_capacity;
    }

list->items[list->count].code = code;
    list->items[list->count].severity = severity;
    list->items[list->count].effort = zt_diag_code_effort(code);
    list->items[list->count].span = span;
    memcpy(list->items[list->count].message, message, sizeof(message));
    list->items[list->count].suggestion[0] = '\0';

    list->count += 1;
}

void zt_diag_list_add(zt_diag_list *list, zt_diag_code code, zt_source_span span, const char *format, ...) {
    va_list args;

    va_start(args, format);
    zt_diag_list_add_va(list, code, ZT_DIAG_SEVERITY_ERROR, span, format, args);
    va_end(args);
}

void zt_diag_list_add_severity(
        zt_diag_list *list,
        zt_diag_code code,
        zt_diag_severity severity,
        zt_source_span span,
        const char *format,
        ...) {
    va_list args;

    va_start(args, format);
    zt_diag_list_add_va(list, code, severity, span, format, args);
    va_end(args);
}

void zt_diag_list_add_suggestion(
        zt_diag_list *list,
        zt_diag_code code,
        zt_source_span span,
        const char *suggestion,
        const char *format,
        ...) {
    va_list args;
    size_t idx;

    if (list == NULL || format == NULL) return;

    va_start(args, format);

    zt_diag_list_add_va(list, code, ZT_DIAG_SEVERITY_ERROR, span, format, args);

    va_end(args);

    if (list->count == 0) return;
    idx = list->count - 1;
    if (suggestion != NULL && suggestion[0] != '\0') {
        snprintf(list->items[idx].suggestion, sizeof(list->items[idx].suggestion), "%s", suggestion);
    }
}
static int zt_name_edit_distance(const char *a, const char *b) {
    size_t la, lb;
    size_t i, j;
    size_t *dp;
    size_t result;

    if (a == NULL || b == NULL) return 999;
    la = strlen(a);
    lb = strlen(b);
    if (la == 0) return (int)lb;
    if (lb == 0) return (int)la;

    dp = (size_t *)malloc((la + 1) * (lb + 1) * sizeof(size_t));
    if (dp == NULL) return 999;

    for (i = 0; i <= la; i += 1) dp[i * (lb + 1)] = i;
    for (j = 0; j <= lb; j += 1) dp[j] = j;

    for (i = 1; i <= la; i += 1) {
        for (j = 1; j <= lb; j += 1) {
            size_t cost = (a[i - 1] != b[j - 1]) ? 1 : 0;
            size_t del = dp[(i - 1) * (lb + 1) + j] + 1;
            size_t ins = dp[i * (lb + 1) + (j - 1)] + 1;
            size_t sub = dp[(i - 1) * (lb + 1) + (j - 1)] + cost;
            dp[i * (lb + 1) + j] = del < ins ? (del < sub ? del : sub) : (ins < sub ? ins : sub);
        }
    }

    result = dp[la * (lb + 1) + lb];
    free(dp);
    return (int)result;
}

int zt_name_suggest(const char *unknown, const char **candidates, size_t candidate_count, char *out, size_t out_capacity) {
    size_t i;
    int best_dist;
    size_t best_idx;
    int threshold;

    if (unknown == NULL || candidates == NULL || out == NULL || out_capacity == 0) return 0;

    threshold = (int)strlen(unknown);
    if (threshold > 3) threshold = threshold / 2;
    if (threshold < 1) threshold = 1;

    best_dist = 999;
    best_idx = 0;

    for (i = 0; i < candidate_count; i += 1) {
        int dist = zt_name_edit_distance(unknown, candidates[i]);
        if (dist < best_dist) {
            best_dist = dist;
            best_idx = i;
        }
    }

    if (best_dist <= threshold) {
        snprintf(out, out_capacity, "%s", candidates[best_idx]);
        return 1;
    }

    return 0;
}

zt_diag_effort zt_diag_code_effort(zt_diag_code code) {
    switch (code) {
        case ZT_DIAG_SYNTAX_ERROR:
        case ZT_DIAG_UNEXPECTED_TOKEN:
        case ZT_DIAG_UNRESOLVED_NAME:
        case ZT_DIAG_DUPLICATE_NAME:
        case ZT_DIAG_SHADOWING:
        case ZT_DIAG_CONST_REASSIGNMENT:
        case ZT_DIAG_PARAM_ORDERING:
        case ZT_DIAG_NAMED_ARG_AFTER_POSITIONAL:
        case ZT_DIAG_TOKEN_TOO_LONG:
            return ZT_DIAG_EFFORT_QUICK_FIX;

        case ZT_DIAG_TYPE_MISMATCH:
        case ZT_DIAG_INVALID_TYPE:
        case ZT_DIAG_INVALID_CALL:
        case ZT_DIAG_INVALID_ARGUMENT:
        case ZT_DIAG_INVALID_OPERATOR:
        case ZT_DIAG_INVALID_ASSIGNMENT:
        case ZT_DIAG_INVALID_MUTATION:
        case ZT_DIAG_INVALID_CONVERSION:
        case ZT_DIAG_INVALID_MAP_KEY_TYPE:
        case ZT_DIAG_INVALID_CONDITION_TYPE:
        case ZT_DIAG_INVALID_RETURN:
        case ZT_DIAG_PROJECT_UNRESOLVED_IMPORT:
        case ZT_DIAG_PROJECT_MISSING_ENTRY:
        case ZT_DIAG_PROJECT_INVALID_NAMESPACE:
        case ZT_DIAG_DOC_UNRESOLVED_TARGET:
        case ZT_DIAG_DOC_UNRESOLVED_LINK:
        case ZT_DIAG_DOC_MISSING_PUBLIC_DOC:
            return ZT_DIAG_EFFORT_MODERATE;

        case ZT_DIAG_NON_EXHAUSTIVE_MATCH:
        case ZT_DIAG_PROJECT_IMPORT_CYCLE:
        case ZT_DIAG_INTEGER_OVERFLOW:
        case ZT_DIAG_INVALID_CONSTRAINT_TARGET:
        case ZT_DIAG_PROJECT_MONOMORPHIZATION_LIMIT_EXCEEDED:
        case ZT_DIAG_ZIR_PARSE_ERROR:
        case ZT_DIAG_ZIR_VERIFY_ERROR:
        case ZT_DIAG_BACKEND_C_EMIT_ERROR:
        case ZT_DIAG_BACKEND_C_LEGALIZE_ERROR:
            return ZT_DIAG_EFFORT_REQUIRES_THINKING;

        default:
            return ZT_DIAG_EFFORT_MODERATE;
    }
}

const char *zt_diag_effort_label(zt_diag_effort effort) {
    switch (effort) {
        case ZT_DIAG_EFFORT_QUICK_FIX: return "\xe2\x9a\xa1 quick fix";
        case ZT_DIAG_EFFORT_MODERATE: return "\xf0\x9f\x94\xa7 moderate";
        case ZT_DIAG_EFFORT_REQUIRES_THINKING: return "\xf0\x9f\xa7\xa9 requires thinking";
        default: return "moderate";
    }
}

const char *zt_diag_action_text(zt_diag_code code) {
    switch (code) {
        case ZT_DIAG_UNRESOLVED_NAME: return "Declare or import the name before using it.";
        case ZT_DIAG_SYNTAX_ERROR: return "Fix the syntax near the reported location.";
        case ZT_DIAG_UNEXPECTED_TOKEN: return "Replace or remove the unexpected token.";
        case ZT_DIAG_TYPE_MISMATCH: return "Convert the value type or change the expected type.";
        case ZT_DIAG_CONST_REASSIGNMENT: return "Use var if the binding must be reassigned.";
        case ZT_DIAG_NON_EXHAUSTIVE_MATCH: return "Add missing match cases or a default branch.";
        case ZT_DIAG_DUPLICATE_NAME: return "Rename one of the duplicate declarations.";
        case ZT_DIAG_SHADOWING: return "Use a different name to avoid shadowing the outer declaration.";
        case ZT_DIAG_INVALID_CALL: return "Check the function name and argument count.";
        case ZT_DIAG_INVALID_ARGUMENT: return "Check argument names, order, and types.";
        case ZT_DIAG_INVALID_OPERATOR: return "Use compatible operand types for this operator.";
        case ZT_DIAG_INVALID_ASSIGNMENT: return "Ensure the target is mutable and types match.";
        case ZT_DIAG_INVALID_MUTATION: return "Mark the receiver or binding as mutable.";
        case ZT_DIAG_INVALID_CONDITION_TYPE: return "Use a bool expression in the condition.";
        case ZT_DIAG_INVALID_RETURN: return "Ensure all code paths return the correct type.";
        case ZT_DIAG_PROJECT_IMPORT_CYCLE: return "Refactor imports to break the cycle.";
        case ZT_DIAG_PROJECT_MISSING_ENTRY: return "Point app.entry to a valid namespace.";
        default: return NULL;
    }
}

size_t zt_cog_profile_error_limit(zt_cog_profile profile) {
    switch (profile) {
        case ZT_COG_PROFILE_BEGINNER: return 3;
        case ZT_COG_PROFILE_BALANCED: return 5;
        case ZT_COG_PROFILE_FULL: return (size_t)-1;
        default: return 5;
    }
}

#define ANSI_RED     "\x1b[1;31m"
#define ANSI_GREEN   "\x1b[1;32m"
#define ANSI_YELLOW  "\x1b[1;33m"
#define ANSI_BLUE    "\x1b[1;34m"
#define ANSI_CYAN    "\x1b[1;36m"
#define ANSI_RESET   "\x1b[0m"

void zt_diag_render_detailed(FILE *stream, const char *stage, const zt_diag *diag) {
    const char *help;
    char source_line[1024];
    int use_color = 1; /* For simple MVP, enable colors. In production we'd check isatty. */

    if (stream == NULL || diag == NULL) return;

    /* Header: Severity[code] */
    const char *sev_color = (diag->severity == ZT_DIAG_SEVERITY_ERROR) ? ANSI_RED : ANSI_YELLOW;
    if (use_color) fprintf(stream, "%s", sev_color);
    fprintf(stream, "%s", zt_diag_severity_name(diag->severity));
    if (use_color) fprintf(stream, ANSI_CYAN);
    fprintf(stream, "[%s]\n", zt_diag_code_stable(diag->code));
    if (use_color) fprintf(stream, ANSI_RESET);

    /* Main message */
    fprintf(stream, "%s\n", diag->message);

    /* Where section */
    if (use_color) fprintf(stream, "\n" ANSI_BLUE "%s" ANSI_RESET "\n  ", zt_l10n_label_where());
    else fprintf(stream, "\n%s\n  ", zt_l10n_label_where());
    zt_diag_print_source_span(stream, diag->span);

    /* Source code and Caret section */
    if (diag->span.source_name != NULL && zt_diag_read_source_line(diag->span.source_name, diag->span.line, source_line, sizeof(source_line))) {
        if (use_color) fprintf(stream, "\n\n" ANSI_BLUE "%s" ANSI_RESET "\n  %zu | %s\n", zt_l10n_label_code(), diag->span.line, source_line);
        else fprintf(stream, "\n\n%s\n  %zu | %s\n", zt_l10n_label_code(), diag->span.line, source_line);
        
        if (use_color) fprintf(stream, ANSI_RED);
        zt_diag_render_caret(stream, diag->span);
        if (use_color) fprintf(stream, ANSI_RESET);
    }

    /* Note section */
    if (stage != NULL && stage[0] != '\0') {
        if (use_color) fprintf(stream, "\n" ANSI_CYAN "%s" ANSI_RESET "\n  stage: %s\n", zt_l10n_label_note(), stage);
        else fprintf(stream, "\n%s\n  stage: %s\n", zt_l10n_label_note(), stage);
    }

    /* Help section */
    help = zt_diag_default_help(diag->code);
    if (help != NULL && help[0] != '\0') {
        if (use_color) fprintf(stream, "\n" ANSI_GREEN "%s" ANSI_RESET "\n  %s\n", zt_l10n_label_help(), help);
        else fprintf(stream, "\n%s\n  %s\n", zt_l10n_label_help(), help);
    }

    fprintf(stream, "\n");
}

void zt_diag_render_detailed_list(FILE *stream, const char *stage, const zt_diag_list *diagnostics) {
    size_t i;

    if (stream == NULL || diagnostics == NULL) return;

    for (i = 0; i < diagnostics->count; i += 1) {
        zt_diag_render_detailed(stream, stage, &diagnostics->items[i]);
    }
}

void zt_diag_render_action_first(FILE *stream, const char *stage, const zt_diag *diag) {
    const char *action;
    const char *effort_label;
    char source_line[1024];
    int use_color = 1;

    if (stream == NULL || diag == NULL) return;

    action = zt_diag_action_text(diag->code);
    effort_label = zt_diag_effort_label(diag->effort);

    fprintf(stream, "\n");
    if (use_color) fprintf(stream, "\xf0\x9f\x93\x8c ");
    if (use_color) fprintf(stream, ANSI_GREEN "ACTION:" ANSI_RESET " ");
    if (action) {
        fprintf(stream, "%s\n", action);
    } else {
        fprintf(stream, "%s\n", diag->message);
    }

    if (use_color) fprintf(stream, "\xe2\x84\xb9\xef\xb8\x8f ");
    if (use_color) fprintf(stream, ANSI_BLUE "WHY:" ANSI_RESET " %s\n", diag->message);

    if (diag->suggestion[0] != '\0') {
        if (use_color) fprintf(stream, "\xf0\x9f\x92\xa1 ");
        if (use_color) fprintf(stream, ANSI_CYAN "SUGGESTION:" ANSI_RESET " Did you mean `%s`?\n", diag->suggestion);
    }

    if (use_color) fprintf(stream, "\xe2\x9a\xa0\xef\xb8\x8f ");
    if (use_color) fprintf(stream, ANSI_RED "%s" ANSI_RESET "[%s] ", zt_diag_severity_name(diag->severity), zt_diag_code_stable(diag->code));
    else fprintf(stream, "%s[%s] ", zt_diag_severity_name(diag->severity), zt_diag_code_stable(diag->code));
    if (use_color) fprintf(stream, ANSI_YELLOW "%s" ANSI_RESET "\n", effort_label);
    else fprintf(stream, "%s\n", effort_label);

    if (use_color) fprintf(stream, "\n" ANSI_BLUE "%s" ANSI_RESET "\n  ", zt_l10n_label_where());
    else fprintf(stream, "\n%s\n  ", zt_l10n_label_where());
    zt_diag_print_source_span(stream, diag->span);

    if (diag->span.source_name != NULL && zt_diag_read_source_line(diag->span.source_name, diag->span.line, source_line, sizeof(source_line))) {
        if (use_color) fprintf(stream, "\n\n" ANSI_BLUE "%s" ANSI_RESET "\n  %zu | %s\n", zt_l10n_label_code(), diag->span.line, source_line);
        else fprintf(stream, "\n\n%s\n  %zu | %s\n", zt_l10n_label_code(), diag->span.line, source_line);
        if (use_color) fprintf(stream, ANSI_RED);
        zt_diag_render_caret(stream, diag->span);
        if (use_color) fprintf(stream, ANSI_RESET);
    }

    {
        const char *help = zt_diag_default_help(diag->code);
        if (help != NULL && help[0] != '\0') {
            if (use_color) fprintf(stream, "\n" ANSI_GREEN "%s" ANSI_RESET "\n  %s\n", zt_l10n_label_help(), help);
            else fprintf(stream, "\n%s\n  %s\n", zt_l10n_label_help(), help);
        }
    }

    if (stage != NULL && stage[0] != '\0') {
        if (use_color) fprintf(stream, "\n" ANSI_CYAN "%s" ANSI_RESET "\n  stage: %s\n", zt_l10n_label_note(), stage);
        else fprintf(stream, "\n%s\n  stage: %s\n", zt_l10n_label_note(), stage);
    }

    fprintf(stream, "\n");
}

void zt_diag_render_action_first_list(FILE *stream, const char *stage, const zt_diag_list *diagnostics, size_t max_errors) {
    size_t i;
    size_t error_count;
    size_t suppressed;

    if (stream == NULL || diagnostics == NULL) return;

    error_count = 0;
    for (i = 0; i < diagnostics->count; i += 1) {
        if (diagnostics->items[i].severity == ZT_DIAG_SEVERITY_ERROR) {
            error_count += 1;
        }
    }

    suppressed = 0;
    for (i = 0; i < diagnostics->count; i += 1) {
        if (max_errors != (size_t)-1 && i >= max_errors) {
            suppressed = diagnostics->count - max_errors;
            break;
        }
        zt_diag_render_action_first(stream, stage, &diagnostics->items[i]);
    }

    if (suppressed > 0) {
        if (max_errors == 3) {
            fprintf(stream, "\n... and %zu more error(s). Run `zt check --all` to show everything.\n\n", suppressed);
        } else {
            fprintf(stream, "\n... and %zu more error(s). Run `zt check --all` to show everything.\n\n", suppressed);
        }
    }
}

void zt_diag_render_ci(FILE *stream, const char *stage, const zt_diag *diag) {
    if (stream == NULL || diag == NULL) return;

    fprintf(stream, "%s[%s] ", zt_diag_severity_name(diag->severity), zt_diag_code_stable(diag->code));
    zt_diag_print_source_span(stream, diag->span);
    if (stage != NULL && stage[0] != '\0') {
        fprintf(stream, " stage=%s", stage);
    }
    fprintf(stream, " %s\n", diag->message);
}

void zt_diag_render_ci_list(FILE *stream, const char *stage, const zt_diag_list *diagnostics) {
    size_t i;

    if (stream == NULL || diagnostics == NULL) return;

    for (i = 0; i < diagnostics->count; i += 1) {
        zt_diag_render_ci(stream, stage, &diagnostics->items[i]);
    }
}
