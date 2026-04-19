#include "compiler/semantic/diagnostics/diagnostics.h"

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
    switch (severity) {
        case ZT_DIAG_SEVERITY_ERROR: return "error";
        case ZT_DIAG_SEVERITY_WARNING: return "warning";
        case ZT_DIAG_SEVERITY_NOTE: return "note";
        case ZT_DIAG_SEVERITY_HELP: return "help";
        default: return "error";
    }
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
    list->items[list->count].span = span;
    memcpy(list->items[list->count].message, message, sizeof(message));

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
void zt_diag_render_detailed(FILE *stream, const char *stage, const zt_diag *diag) {
    const char *help;
    char source_line[1024];

    if (stream == NULL || diag == NULL) return;

    fprintf(stream, "%s[%s]\n", zt_diag_severity_name(diag->severity), zt_diag_code_stable(diag->code));
    fprintf(stream, "%s\n", diag->message);

    fprintf(stream, "\nwhere\n  ");
    zt_diag_print_source_span(stream, diag->span);

    if (diag->span.source_name != NULL && zt_diag_read_source_line(diag->span.source_name, diag->span.line, source_line, sizeof(source_line))) {
        fprintf(stream, "\n\ncode\n  %zu | %s\n", diag->span.line, source_line);
        zt_diag_render_caret(stream, diag->span);
    }

    if (stage != NULL && stage[0] != '\0') {
        fprintf(stream, "\nnote\n  stage: %s\n", stage);
    }

    help = zt_diag_default_help(diag->code);
    if (help != NULL && help[0] != '\0') {
        fprintf(stream, "\nhelp\n  %s\n", help);
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
