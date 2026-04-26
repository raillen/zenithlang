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
        case ZT_DIAG_STRUCTURE_LIMIT_EXCEEDED: return "structure_limit_exceeded";
        case ZT_DIAG_DUPLICATE_NAME: return "duplicate_name";
        case ZT_DIAG_SHADOWING: return "shadowing";
        case ZT_DIAG_UNRESOLVED_NAME: return "unresolved_name";
        case ZT_DIAG_CONFUSING_NAME: return "confusing_name";
        case ZT_DIAG_SIMILAR_NAME: return "similar_name";
        case ZT_DIAG_BLOCK_TOO_DEEP: return "block_too_deep";
        case ZT_DIAG_FUNCTION_TOO_LONG: return "function_too_long";
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
        case ZT_DIAG_ENUM_DEFAULT_CASE: return "enum_default_case";
        case ZT_DIAG_TOKEN_TOO_LONG: return "token_too_long";
        case ZT_DIAG_LEXER_UNTERMINATED_STRING: return "lexer_unterminated_string";
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
        case ZT_DIAG_DYN_MUT_METHOD: return "dyn.mut_method";
        case ZT_DIAG_DYN_GENERIC_TRAIT: return "dyn.generic_trait";
        case ZT_DIAG_DYN_TOO_MANY_METHODS: return "dyn.too_many_methods";
        case ZT_DIAG_DYN_UNCOPYABLE: return "dyn.uncopyable";
        case ZT_DIAG_DYN_NO_APPLY: return "dyn.no_apply";
        case ZT_DIAG_DYN_FFI_UNSAFE: return "dyn.ffi_unsafe";
        case ZT_DIAG_CALLABLE_SIGNATURE_MISMATCH: return "callable.signature_mismatch";
        case ZT_DIAG_CALLABLE_ESCAPE_PUBLIC_VAR: return "callable.escape_public_var";
        case ZT_DIAG_CALLABLE_ESCAPE_STRUCT_FIELD: return "callable.escape_struct_field";
        case ZT_DIAG_CALLABLE_ESCAPE_CONTAINER: return "callable.escape_container";
        case ZT_DIAG_CALLABLE_EXTERN_C_SIGNATURE: return "callable.extern_c_signature";
        case ZT_DIAG_CALLABLE_EXTERN_C_CLOSURE_UNSUPPORTED: return "callable.extern_c_closure_unsupported";
        case ZT_DIAG_CALLABLE_INVALID_FUNC_REF: return "callable.invalid_func_ref";
        case ZT_DIAG_CLOSURE_MUT_CAPTURE_UNSUPPORTED: return "closure.mut_capture_unsupported";
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
        case ZT_DIAG_STRUCTURE_LIMIT_EXCEEDED: return "compiler.limit_exceeded";
        case ZT_DIAG_DUPLICATE_NAME: return "name.duplicate";
        case ZT_DIAG_SHADOWING: return "name.shadowing";
        case ZT_DIAG_UNRESOLVED_NAME: return "name.unresolved";
        case ZT_DIAG_CONFUSING_NAME: return "name.confusing";
        case ZT_DIAG_SIMILAR_NAME: return "name.similar";
        case ZT_DIAG_BLOCK_TOO_DEEP: return "style.block_too_deep";
        case ZT_DIAG_FUNCTION_TOO_LONG: return "style.function_too_long";
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
        case ZT_DIAG_ENUM_DEFAULT_CASE: return "control_flow.enum_default_case";
        case ZT_DIAG_TOKEN_TOO_LONG: return "lexer.token_too_long";
        case ZT_DIAG_LEXER_UNTERMINATED_STRING: return "lexer.unterminated_string";
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
        case ZT_DIAG_DYN_MUT_METHOD: return "dyn.mut_method";
        case ZT_DIAG_DYN_GENERIC_TRAIT: return "dyn.generic_trait";
        case ZT_DIAG_DYN_TOO_MANY_METHODS: return "dyn.too_many_methods";
        case ZT_DIAG_DYN_UNCOPYABLE: return "dyn.uncopyable";
        case ZT_DIAG_DYN_NO_APPLY: return "dyn.no_apply";
        case ZT_DIAG_DYN_FFI_UNSAFE: return "dyn.ffi_unsafe";
        case ZT_DIAG_CALLABLE_SIGNATURE_MISMATCH: return "callable.signature_mismatch";
        case ZT_DIAG_CALLABLE_ESCAPE_PUBLIC_VAR: return "callable.escape_public_var";
        case ZT_DIAG_CALLABLE_ESCAPE_STRUCT_FIELD: return "callable.escape_struct_field";
        case ZT_DIAG_CALLABLE_ESCAPE_CONTAINER: return "callable.escape_container";
        case ZT_DIAG_CALLABLE_EXTERN_C_SIGNATURE: return "callable.extern_c_signature";
        case ZT_DIAG_CALLABLE_EXTERN_C_CLOSURE_UNSUPPORTED: return "callable.extern_c_closure_unsupported";
        case ZT_DIAG_CALLABLE_INVALID_FUNC_REF: return "callable.invalid_func_ref";
        case ZT_DIAG_CLOSURE_MUT_CAPTURE_UNSUPPORTED: return "closure.mut_capture_unsupported";
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
        case ZT_DIAG_STRUCTURE_LIMIT_EXCEEDED: return "Reduce nesting depth or split the construct into smaller parts.";
        case ZT_DIAG_DUPLICATE_NAME: return "Rename one of the declarations in this scope.";
        case ZT_DIAG_SHADOWING: return "Use a different local name to avoid shadowing an outer declaration.";
        case ZT_DIAG_UNRESOLVED_NAME: return "Declare or import this name before using it.";
        case ZT_DIAG_CONFUSING_NAME: return "Rename identifiers that mix confusable characters like l/I/1 or O/0.";
        case ZT_DIAG_SIMILAR_NAME: return "Rename one identifier so names in the same scope are easier to tell apart.";
        case ZT_DIAG_BLOCK_TOO_DEEP: return "Split nested logic into a smaller helper function or return early.";
        case ZT_DIAG_FUNCTION_TOO_LONG: return "Split a long function into smaller named helpers.";
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
        case ZT_DIAG_NON_EXHAUSTIVE_MATCH: return "Add the listed missing cases or a final `case default` branch.";
        case ZT_DIAG_ENUM_DEFAULT_CASE: return "List enum variants explicitly when the enum is known.";
        case ZT_DIAG_TOKEN_TOO_LONG: return "Reduce the token length to fit within the lexer limit.";
        case ZT_DIAG_LEXER_UNTERMINATED_STRING: return "Close the string literal with a matching delimiter.";
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
        case ZT_DIAG_DYN_MUT_METHOD: return "Use a concrete generic parameter with a where constraint, or remove mut from the dyn trait.";
        case ZT_DIAG_DYN_GENERIC_TRAIT: return "Use a generic parameter with a where constraint instead of dyn for generic traits.";
        case ZT_DIAG_DYN_TOO_MANY_METHODS: return "Split the trait or use a generic parameter with a where constraint instead of dyn.";
        case ZT_DIAG_DYN_UNCOPYABLE: return "Use copyable parameter/return types, or keep the concrete type through generics.";
        case ZT_DIAG_DYN_NO_APPLY: return "Implement the trait for this type using apply Trait to Type.";
        case ZT_DIAG_DYN_FFI_UNSAFE: return "dyn types cannot be used in extern c signatures.";
        case ZT_DIAG_CALLABLE_SIGNATURE_MISMATCH: return "Callable values must match the expected signature exactly; compare the expected and received signatures.";
        case ZT_DIAG_CALLABLE_ESCAPE_PUBLIC_VAR: return "A callable cannot be stored in a public namespace variable in v1; only local bindings and parameters are allowed.";
        case ZT_DIAG_CALLABLE_ESCAPE_STRUCT_FIELD: return "A callable cannot be stored as a struct field in v1; closures with captures are deferred to R3.M6.";
        case ZT_DIAG_CALLABLE_ESCAPE_CONTAINER: return "Callables cannot appear inside list, map, optional, or result element types in v1.";
        case ZT_DIAG_CALLABLE_EXTERN_C_SIGNATURE: return "Only primitive, text, and bytes shapes are allowed in callables that cross the extern c boundary.";
        case ZT_DIAG_CALLABLE_EXTERN_C_CLOSURE_UNSUPPORTED: return "Closures capturing variables cannot be passed to an extern c boundary. Use a plain non-capturing function.";
        case ZT_DIAG_CALLABLE_INVALID_FUNC_REF: return "Use a non-generic top-level wrapper whose signature matches the expected callable type.";
        case ZT_DIAG_CLOSURE_MUT_CAPTURE_UNSUPPORTED: return "Captured scoped variables in Zenith are immutable snapshots (by value). You cannot assign a new value to a captured variable.";
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

size_t zt_diag_list_error_count(const zt_diag_list *list) {
    size_t count = 0;
    size_t i;

    if (list == NULL) return 0;
    for (i = 0; i < list->count; i += 1) {
        if (list->items[i].severity == ZT_DIAG_SEVERITY_ERROR) {
            count += 1;
        }
    }
    return count;
}

int zt_diag_list_has_errors(const zt_diag_list *list) {
    return zt_diag_list_error_count(list) > 0;
}

void zt_diag_list_promote_warnings(zt_diag_list *list) {
    size_t i;

    if (list == NULL) return;
    for (i = 0; i < list->count; i += 1) {
        if (list->items[i].severity == ZT_DIAG_SEVERITY_WARNING) {
            list->items[i].severity = ZT_DIAG_SEVERITY_ERROR;
        }
    }
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
        case ZT_DIAG_STRUCTURE_LIMIT_EXCEEDED:
        case ZT_DIAG_UNRESOLVED_NAME:
        case ZT_DIAG_CONFUSING_NAME:
        case ZT_DIAG_SIMILAR_NAME:
        case ZT_DIAG_BLOCK_TOO_DEEP:
        case ZT_DIAG_FUNCTION_TOO_LONG:
        case ZT_DIAG_DUPLICATE_NAME:
        case ZT_DIAG_SHADOWING:
        case ZT_DIAG_CONST_REASSIGNMENT:
        case ZT_DIAG_PARAM_ORDERING:
        case ZT_DIAG_NAMED_ARG_AFTER_POSITIONAL:
        case ZT_DIAG_TOKEN_TOO_LONG:
        case ZT_DIAG_LEXER_UNTERMINATED_STRING:
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
        case ZT_DIAG_DYN_MUT_METHOD:
        case ZT_DIAG_DYN_GENERIC_TRAIT:
        case ZT_DIAG_DYN_TOO_MANY_METHODS:
        case ZT_DIAG_DYN_UNCOPYABLE:
        case ZT_DIAG_DYN_NO_APPLY:
        case ZT_DIAG_DYN_FFI_UNSAFE:
        case ZT_DIAG_CALLABLE_SIGNATURE_MISMATCH:
        case ZT_DIAG_CALLABLE_ESCAPE_PUBLIC_VAR:
        case ZT_DIAG_CALLABLE_ESCAPE_STRUCT_FIELD:
        case ZT_DIAG_CALLABLE_ESCAPE_CONTAINER:
        case ZT_DIAG_CALLABLE_EXTERN_C_SIGNATURE:
        case ZT_DIAG_CALLABLE_EXTERN_C_CLOSURE_UNSUPPORTED:
        case ZT_DIAG_CALLABLE_INVALID_FUNC_REF:
        case ZT_DIAG_CLOSURE_MUT_CAPTURE_UNSUPPORTED:
            return ZT_DIAG_EFFORT_MODERATE;

        case ZT_DIAG_NON_EXHAUSTIVE_MATCH:
        case ZT_DIAG_ENUM_DEFAULT_CASE:
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
        case ZT_DIAG_CONFUSING_NAME: return "Rename this identifier to reduce visual confusion.";
        case ZT_DIAG_SIMILAR_NAME: return "Rename one of these similar identifiers.";
        case ZT_DIAG_BLOCK_TOO_DEEP: return "Split this nested block into smaller steps.";
        case ZT_DIAG_FUNCTION_TOO_LONG: return "Extract a smaller helper function.";
        case ZT_DIAG_SYNTAX_ERROR: return "Fix the syntax near the reported location.";
        case ZT_DIAG_UNEXPECTED_TOKEN: return "Replace or remove the unexpected token.";
        case ZT_DIAG_STRUCTURE_LIMIT_EXCEEDED: return "Split the construct into smaller nested pieces.";
        case ZT_DIAG_TYPE_MISMATCH: return "Convert the value type or change the expected type.";
        case ZT_DIAG_CONST_REASSIGNMENT: return "Use var if the binding must be reassigned.";
        case ZT_DIAG_PARAM_ORDERING: return "Move required parameters before parameters with default values (or add defaults).";
        case ZT_DIAG_NON_EXHAUSTIVE_MATCH: return "Add the listed missing match cases or a final default branch.";
        case ZT_DIAG_ENUM_DEFAULT_CASE: return "Replace `case default` with explicit enum variants.";
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
        case ZT_DIAG_DYN_MUT_METHOD: return "Remove mut from the trait method or use generics with a where constraint.";
        case ZT_DIAG_DYN_GENERIC_TRAIT: return "Keep the concrete type through generics with a where constraint.";
        case ZT_DIAG_DYN_TOO_MANY_METHODS: return "Split the trait or use generics with a where constraint.";
        case ZT_DIAG_DYN_UNCOPYABLE: return "Use copyable method types or keep the concrete type through generics.";
        case ZT_DIAG_DYN_NO_APPLY: return "Add an apply Trait to Type block for this type.";
        case ZT_DIAG_DYN_FFI_UNSAFE: return "Use a concrete type instead of dyn in extern c signatures.";
        case ZT_DIAG_CALLABLE_SIGNATURE_MISMATCH: return "Make the received callable signature match the expected signature.";
        case ZT_DIAG_CALLABLE_ESCAPE_PUBLIC_VAR: return "Keep the callable in a local binding or pass it as a function parameter.";
        case ZT_DIAG_CALLABLE_ESCAPE_STRUCT_FIELD: return "Store plain data in the struct and pass the callable through a function parameter instead.";
        case ZT_DIAG_CALLABLE_ESCAPE_CONTAINER: return "Pass callables individually as function arguments rather than collecting them in a container.";
        case ZT_DIAG_CALLABLE_EXTERN_C_SIGNATURE: return "Use only int, float, bool, text, or bytes in callables that are passed to extern c.";
        case ZT_DIAG_CALLABLE_INVALID_FUNC_REF: return "Reference a non-generic top-level wrapper with the expected signature.";
        default: return NULL;
    }
}

const char *zt_diag_next_text(zt_diag_code code) {
    switch (code) {
        case ZT_DIAG_UNRESOLVED_NAME:
            return "Re-run `zt check` after adding the declaration or the correct `import`.";
        case ZT_DIAG_CONFUSING_NAME:
        case ZT_DIAG_SIMILAR_NAME:
            return "Re-run `zt check` after renaming the identifier.";
        case ZT_DIAG_BLOCK_TOO_DEEP:
            return "Re-run `zt check` after reducing the nesting depth.";
        case ZT_DIAG_FUNCTION_TOO_LONG:
            return "Re-run `zt check` after splitting the function.";
        case ZT_DIAG_SYNTAX_ERROR:
            return "Re-run `zt check` after fixing the syntax.";
        case ZT_DIAG_UNEXPECTED_TOKEN:
            return "Re-run `zt check` after adjusting the offending token.";
        case ZT_DIAG_STRUCTURE_LIMIT_EXCEEDED:
            return "Re-run `zt check` after splitting the construct.";
        case ZT_DIAG_TYPE_MISMATCH:
            return "Re-run `zt check` to confirm the types align on both sides.";
        case ZT_DIAG_CONST_REASSIGNMENT:
            return "Re-run `zt check` after switching `const` to `var` (or removing the reassignment).";
        case ZT_DIAG_PARAM_ORDERING:
            return "Re-run `zt check` after reordering parameters or adding default values.";
        case ZT_DIAG_NON_EXHAUSTIVE_MATCH:
            return "Re-run `zt check` after adding the listed missing cases or a `default` branch.";
        case ZT_DIAG_ENUM_DEFAULT_CASE:
            return "Re-run `zt check` after listing the enum variants.";
        case ZT_DIAG_DUPLICATE_NAME:
            return "Re-run `zt check` after renaming one of the duplicate declarations.";
        case ZT_DIAG_SHADOWING:
            return "Re-run `zt check` after choosing a non-shadowing name.";
        case ZT_DIAG_INVALID_CALL:
            return "Re-run `zt check` after fixing the callee or argument list.";
        case ZT_DIAG_INVALID_ARGUMENT:
            return "Re-run `zt check` after fixing argument names, order, or types.";
        case ZT_DIAG_INVALID_OPERATOR:
            return "Re-run `zt check` after aligning operand types.";
        case ZT_DIAG_INVALID_ASSIGNMENT:
            return "Re-run `zt check` after making the target mutable and type-compatible.";
        case ZT_DIAG_INVALID_MUTATION:
            return "Re-run `zt check` after marking the receiver or binding mutable.";
        case ZT_DIAG_INVALID_CONDITION_TYPE:
            return "Re-run `zt check` after converting the condition to a `bool` expression.";
        case ZT_DIAG_INVALID_RETURN:
            return "Re-run `zt check` after aligning return values with the declared return type.";
        case ZT_DIAG_PROJECT_IMPORT_CYCLE:
            return "Re-run `zt check` after breaking the cycle between the namespaces.";
        case ZT_DIAG_PROJECT_MISSING_ENTRY:
            return "Re-run `zt check` after setting `app.entry` to a valid namespace inside `source.root`.";
        case ZT_DIAG_PROJECT_UNRESOLVED_IMPORT:
            return "Re-run `zt check` after adding the missing namespace under `source.root`.";
        case ZT_DIAG_PROJECT_INVALID_NAMESPACE:
            return "Re-run `zt check` after renaming the file/folder to match the declared namespace.";
        case ZT_DIAG_PROJECT_MONOMORPHIZATION_LIMIT_EXCEEDED:
            return "Re-run `zt check` after reducing generic instances or raising `build.monomorphization_limit`.";
        case ZT_DIAG_DOC_UNRESOLVED_TARGET:
            return "Re-run `zt doc check` after pointing the doc target to an existing symbol.";
        case ZT_DIAG_DOC_UNRESOLVED_LINK:
            return "Re-run `zt doc check` after fixing the link target name.";
        case ZT_DIAG_DOC_MISSING_PUBLIC_DOC:
            return "Re-run `zt doc check` after adding the paired ZDoc block.";
        case ZT_DIAG_DYN_MUT_METHOD:
            return "Re-run `zt check` after removing mut or switching to generics with a where constraint.";
        case ZT_DIAG_DYN_GENERIC_TRAIT:
            return "Re-run `zt check` after replacing dyn with a generic parameter and where constraint.";
        case ZT_DIAG_DYN_TOO_MANY_METHODS:
            return "Re-run `zt check` after splitting the trait into smaller traits.";
        case ZT_DIAG_DYN_UNCOPYABLE:
            return "Re-run `zt check` after changing method signatures or replacing dyn with generics.";
        case ZT_DIAG_DYN_NO_APPLY:
            return "Re-run `zt check` after adding an apply Trait to Type block.";
        case ZT_DIAG_DYN_FFI_UNSAFE:
            return "Re-run `zt check` after using a concrete type in the extern c signature.";
        case ZT_DIAG_CALLABLE_SIGNATURE_MISMATCH:
            return "Re-run `zt check` after aligning the callable signature shown in the diagnostic.";
        case ZT_DIAG_CALLABLE_ESCAPE_PUBLIC_VAR:
        case ZT_DIAG_CALLABLE_ESCAPE_STRUCT_FIELD:
        case ZT_DIAG_CALLABLE_ESCAPE_CONTAINER:
            return "Re-run `zt check` after moving the callable to an allowed position.";
        case ZT_DIAG_CALLABLE_EXTERN_C_SIGNATURE:
            return "Re-run `zt check` after narrowing the callable to boundary-safe shapes.";
        case ZT_DIAG_CALLABLE_EXTERN_C_CLOSURE_UNSUPPORTED:
            return "Re-run `zt check` after passing a pure named function without captures instead.";
        case ZT_DIAG_CALLABLE_INVALID_FUNC_REF:
            return "Re-run `zt check` after referencing a non-generic wrapper with the expected signature.";
        case ZT_DIAG_CLOSURE_MUT_CAPTURE_UNSUPPORTED:
            return "Re-run `zt check` after removing assignments to outer variables from the closure body.";
        default:
            return NULL;
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

zt_cog_profile zt_cog_profile_from_text(const char *text) {
    if (text == NULL) return ZT_COG_PROFILE_BALANCED;
    if (strcmp(text, "beginner") == 0) return ZT_COG_PROFILE_BEGINNER;
    if (strcmp(text, "balanced") == 0) return ZT_COG_PROFILE_BALANCED;
    if (strcmp(text, "full") == 0) return ZT_COG_PROFILE_FULL;
    return ZT_COG_PROFILE_BALANCED;
}

void zt_diag_telemetry_log(
        const char *project_root,
        const char *profile_name,
        const char *stage,
        const zt_diag *diag) {
    FILE *log_file;
    char log_path[512];

    if (project_root == NULL || diag == NULL) return;

    snprintf(log_path, sizeof(log_path), "%s/.ztc-tmp/accessibility_metrics.jsonl", project_root);
    
    log_file = fopen(log_path, "a");
    if (log_file == NULL) return;

    /* Very simple JSONL record. In a real system we'd escape strings properly. */
    fprintf(log_file, "{\"timestamp\":\"2026-04-20T12:00:00Z\",\"event\":\"diagnostic_reported\",\"profile\":\"%s\",\"code\":\"%s\",\"severity\":\"%s\",\"effort\":\"%s\",\"stage\":\"%s\",\"file\":\"%s\",\"line\":%zu,\"col\":%zu}\n",
        profile_name != NULL ? profile_name : "unknown",
        zt_diag_code_stable(diag->code),
        zt_diag_severity_name(diag->severity),
        zt_diag_effort_label(diag->effort),
        stage != NULL ? stage : "unknown",
        diag->span.source_name != NULL ? diag->span.source_name : "unknown",
        diag->span.line,
        diag->span.column_start);

    fclose(log_file);
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
    const char *next_step;
    const char *effort_label;
    char source_line[1024];
    int use_color = 1;

    if (stream == NULL || diag == NULL) return;

    action = zt_diag_action_text(diag->code);
    next_step = zt_diag_next_text(diag->code);
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

    if (next_step != NULL && next_step[0] != '\0') {
        if (use_color) fprintf(stream, "\xe2\x9e\xa1\xef\xb8\x8f ");
        if (use_color) fprintf(stream, ANSI_CYAN "NEXT:" ANSI_RESET " %s\n", next_step);
        else fprintf(stream, "NEXT: %s\n", next_step);
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
    const char *action;
    const char *next_step;

    if (stream == NULL || diag == NULL) return;

    fprintf(stream, "%s[%s] ", zt_diag_severity_name(diag->severity), zt_diag_code_stable(diag->code));
    zt_diag_print_source_span(stream, diag->span);
    if (stage != NULL && stage[0] != '\0') {
        fprintf(stream, " stage=%s", stage);
    }
    fprintf(stream, " effort=%s", zt_diag_effort_label(diag->effort));
    fprintf(stream, " %s", diag->message);

    action = zt_diag_action_text(diag->code);
    if (action != NULL && action[0] != '\0') {
        fprintf(stream, " action=\"%s\"", action);
    }

    if (diag->suggestion[0] != '\0') {
        fprintf(stream, " suggestion=\"%s\"", diag->suggestion);
    }

    next_step = zt_diag_next_text(diag->code);
    if (next_step != NULL && next_step[0] != '\0') {
        fprintf(stream, " next=\"%s\"", next_step);
    }

    fprintf(stream, "\n");
}

void zt_diag_render_ci_list(FILE *stream, const char *stage, const zt_diag_list *diagnostics) {
    size_t i;

    if (stream == NULL || diagnostics == NULL) return;

    for (i = 0; i < diagnostics->count; i += 1) {
        zt_diag_render_ci(stream, stage, &diagnostics->items[i]);
    }
}


