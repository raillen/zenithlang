#include "compiler/tooling/formatter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
    int indent_level;
} sb_t;

static void sb_init(sb_t *sb) {
    sb->capacity = 1024;
    sb->length = 0;
    sb->data = malloc(sb->capacity);
    if (sb->data) sb->data[0] = '\0';
    sb->indent_level = 0;
}

static void sb_append_len(sb_t *sb, const char *str, size_t len) {
    if (!sb->data || !str || len == 0) return;
    if (sb->length + len + 1 > sb->capacity) {
        size_t new_cap = sb->capacity * 2;
        while (sb->length + len + 1 > new_cap) new_cap *= 2;
        sb->capacity = new_cap;
        char *new_data = realloc(sb->data, sb->capacity);
        if (!new_data) return;
        sb->data = new_data;
    }
    memcpy(sb->data + sb->length, str, len);
    sb->length += len;
    sb->data[sb->length] = '\0';
}

static void sb_append(sb_t *sb, const char *str) {
    if (str) sb_append_len(sb, str, strlen(str));
}

static void sb_append_string_value(sb_t *sb, const char *value) {
    if (!value) return;

    /* String literals always require surrounding double quotes. The
     * previous version skipped them when the payload had no escapable
     * characters, which made `fmt(fmt(x))` re-parse as a bare
     * identifier (`"Ada"` becoming `Ada`). Always quote and always
     * escape the control characters that would otherwise break the
     * literal. */
    sb_append(sb, "\"");
    for (const unsigned char *p = (const unsigned char *)value; *p; ++p) {
        switch (*p) {
            case '\n': sb_append(sb, "\\n"); break;
            case '\r': sb_append(sb, "\\r"); break;
            case '\t': sb_append(sb, "\\t"); break;
            case '"': sb_append(sb, "\\\""); break;
            case '\\': sb_append(sb, "\\\\"); break;
            default: {
                char ch[2] = {(char)*p, '\0'};
                sb_append(sb, ch);
                break;
            }
        }
    }
    sb_append(sb, "\"");
}

static void sb_append_fmt_literal_value(sb_t *sb, const char *value) {
    if (!value) return;
    for (const unsigned char *p = (const unsigned char *)value; *p; ++p) {
        switch (*p) {
            case '\n': sb_append(sb, "\\n"); break;
            case '\r': sb_append(sb, "\\r"); break;
            case '\t': sb_append(sb, "\\t"); break;
            case '"': sb_append(sb, "\\\""); break;
            case '\\': sb_append(sb, "\\\\"); break;
            case '{': sb_append(sb, "{{"); break;
            case '}': sb_append(sb, "}}"); break;
            default: {
                char ch[2] = {(char)*p, '\0'};
                sb_append(sb, ch);
                break;
            }
        }
    }
}

static void sb_indent(sb_t *sb) {
    for (int i = 0; i < sb->indent_level; i++) {
        sb_append(sb, "    ");
    }
}

static void format_node(sb_t *sb, const zt_ast_node *node);

#define ZT_FORMATTER_TARGET_WIDTH 100u

static void format_comments(sb_t *sb, const zt_ast_node *node) {
    if (!node || node->comment_count == 0) return;
    for (size_t i = 0; i < node->comment_count; i++) {
        sb_append_len(sb, node->comments[i].text, node->comments[i].length);
        sb_append(sb, "\n");
        sb_indent(sb);
    }
}

static const char *op_to_str(zt_token_kind op) {
    switch (op) {
        case ZT_TOKEN_PLUS: return "+";
        case ZT_TOKEN_MINUS: return "-";
        case ZT_TOKEN_STAR: return "*";
        case ZT_TOKEN_SLASH: return "/";
        case ZT_TOKEN_PERCENT: return "%";
        case ZT_TOKEN_EQEQ: return "==";
        case ZT_TOKEN_NEQ: return "!=";
        case ZT_TOKEN_LT: return "<";
        case ZT_TOKEN_LTE: return "<=";
        case ZT_TOKEN_GT: return ">";
        case ZT_TOKEN_GTE: return ">=";
        case ZT_TOKEN_AND: return "and";
        case ZT_TOKEN_OR: return "or";
        case ZT_TOKEN_NOT: return "not";
        case ZT_TOKEN_BANG: return "!";
        default: return "?";
    }
}

static void format_node_list_comma(sb_t *sb, zt_ast_node_list list) {
    for (size_t i = 0; i < list.count; i++) {
        format_node(sb, list.items[i]);
        if (i < list.count - 1) sb_append(sb, ", ");
    }
}

static void format_node_list_separator(sb_t *sb, zt_ast_node_list list, const char *separator) {
    for (size_t i = 0; i < list.count; i++) {
        format_node(sb, list.items[i]);
        if (i < list.count - 1) sb_append(sb, separator);
    }
}

static void format_node_list_and(sb_t *sb, zt_ast_node_list list) {
    format_node_list_separator(sb, list, " and ");
}

static char *format_node_to_owned_text(const zt_ast_node *node) {
    sb_t tmp;
    sb_init(&tmp);
    format_node(&tmp, node);
    return tmp.data;
}

static size_t formatted_node_length(const zt_ast_node *node) {
    char *text = format_node_to_owned_text(node);
    size_t length = text != NULL ? strlen(text) : 0;
    free(text);
    return length;
}

static size_t formatted_node_list_length(zt_ast_node_list list, const char *separator) {
    size_t length = 0;
    size_t separator_length = separator != NULL ? strlen(separator) : 0;
    for (size_t i = 0; i < list.count; i++) {
        length += formatted_node_length(list.items[i]);
        if (i < list.count - 1) length += separator_length;
    }
    return length;
}

static size_t formatted_type_params_length(zt_ast_node_list list) {
    if (list.count == 0) return 0;
    return 2 + formatted_node_list_length(list, ", ");
}

static size_t formatted_func_decl_signature_length(const zt_ast_node *node) {
    size_t length;
    if (node == NULL || node->kind != ZT_AST_FUNC_DECL) return 0;

    length = node->as.func_decl.is_public ? strlen("public ") : 0;
    length += strlen("func ");
    if (node->as.func_decl.is_mutating) length += strlen("mut ");
    length += node->as.func_decl.name != NULL ? strlen(node->as.func_decl.name) : 0;
    length += formatted_type_params_length(node->as.func_decl.type_params);
    length += 2 + formatted_node_list_length(node->as.func_decl.params, ", ");
    if (node->as.func_decl.return_type != NULL) {
        length += strlen(" -> ");
        length += formatted_node_length(node->as.func_decl.return_type);
    }
    if (node->as.func_decl.constraints.count > 0) {
        length += strlen(" where ");
        length += formatted_node_list_length(node->as.func_decl.constraints, " and ");
    }
    return length;
}

static size_t formatted_trait_method_signature_length(const zt_ast_node *node) {
    size_t length;
    if (node == NULL || node->kind != ZT_AST_TRAIT_METHOD) return 0;

    length = strlen("func ");
    if (node->as.trait_method.is_mutating) length += strlen("mut ");
    length += node->as.trait_method.name != NULL ? strlen(node->as.trait_method.name) : 0;
    length += 2 + formatted_node_list_length(node->as.trait_method.params, ", ");
    if (node->as.trait_method.return_type != NULL) {
        length += strlen(" -> ");
        length += formatted_node_length(node->as.trait_method.return_type);
    }
    return length;
}

static size_t param_where_count(zt_ast_node_list params) {
    size_t count = 0;
    for (size_t i = 0; i < params.count; i++) {
        const zt_ast_node *param = params.items[i];
        if (param != NULL &&
                param->kind == ZT_AST_PARAM &&
                param->as.param.where_clause != NULL) {
            count++;
        }
    }
    return count;
}

static size_t param_default_count(zt_ast_node_list params) {
    size_t count = 0;
    for (size_t i = 0; i < params.count; i++) {
        const zt_ast_node *param = params.items[i];
        if (param != NULL &&
                param->kind == ZT_AST_PARAM &&
                param->as.param.default_value != NULL) {
            count++;
        }
    }
    return count;
}

static int should_format_param_list_multiline(zt_ast_node_list params, size_t inline_length) {
    size_t where_count;
    size_t default_count;

    if (params.count == 0) return 0;
    if (inline_length > ZT_FORMATTER_TARGET_WIDTH) return 1;

    where_count = param_where_count(params);
    if (params.count > 1 && where_count > 1) return 1;
    if (params.count > 1 && where_count > 0 && inline_length > 80u) return 1;

    default_count = param_default_count(params);
    if (params.count > 1 && default_count > 0 && inline_length > 80u) return 1;

    return 0;
}

static void format_param_list_multiline(sb_t *sb, zt_ast_node_list params) {
    sb_append(sb, "(\n");
    sb->indent_level++;
    for (size_t i = 0; i < params.count; i++) {
        sb_indent(sb);
        format_node(sb, params.items[i]);
        if (i < params.count - 1) sb_append(sb, ",");
        sb_append(sb, "\n");
    }
    sb->indent_level--;
    sb_indent(sb);
    sb_append(sb, ")");
}

static void format_func_decl_signature(sb_t *sb, const zt_ast_node *node, int *out_multiline, int *out_where_multiline) {
    size_t inline_length = formatted_func_decl_signature_length(node);
    int params_multiline = should_format_param_list_multiline(node->as.func_decl.params, inline_length);
    int where_multiline = node->as.func_decl.constraints.count > 0 &&
        (params_multiline ||
         node->as.func_decl.constraints.count > 1 ||
         inline_length > ZT_FORMATTER_TARGET_WIDTH);

    if (out_multiline != NULL) *out_multiline = params_multiline;
    if (out_where_multiline != NULL) *out_where_multiline = where_multiline;

    if (node->as.func_decl.is_public) sb_append(sb, "public ");
    sb_append(sb, "func ");
    if (node->as.func_decl.is_mutating) sb_append(sb, "mut ");
    sb_append(sb, node->as.func_decl.name);
    if (node->as.func_decl.type_params.count > 0) {
        sb_append(sb, "<");
        format_node_list_comma(sb, node->as.func_decl.type_params);
        sb_append(sb, ">");
    }
    if (params_multiline) {
        format_param_list_multiline(sb, node->as.func_decl.params);
    } else {
        sb_append(sb, "(");
        format_node_list_comma(sb, node->as.func_decl.params);
        sb_append(sb, ")");
    }
    if (node->as.func_decl.return_type) {
        sb_append(sb, " -> ");
        format_node(sb, node->as.func_decl.return_type);
    }
    if (node->as.func_decl.constraints.count > 0) {
        if (where_multiline) {
            sb_append(sb, "\n");
            sb_indent(sb);
            sb_append(sb, "where ");
        } else {
            sb_append(sb, " where ");
        }
        format_node_list_and(sb, node->as.func_decl.constraints);
    }
}

static void format_trait_method_signature(sb_t *sb, const zt_ast_node *node) {
    size_t inline_length = formatted_trait_method_signature_length(node);
    int params_multiline = should_format_param_list_multiline(node->as.trait_method.params, inline_length);

    sb_append(sb, "func ");
    if (node->as.trait_method.is_mutating) sb_append(sb, "mut ");
    sb_append(sb, node->as.trait_method.name);
    if (params_multiline) {
        format_param_list_multiline(sb, node->as.trait_method.params);
    } else {
        sb_append(sb, "(");
        format_node_list_comma(sb, node->as.trait_method.params);
        sb_append(sb, ")");
    }
    if (node->as.trait_method.return_type) {
        sb_append(sb, " -> ");
        format_node(sb, node->as.trait_method.return_type);
    }
}

static void format_node(sb_t *sb, const zt_ast_node *node) {
    if (!node) return;
    format_comments(sb, node);
    switch (node->kind) {
        case ZT_AST_FILE:
            if (node->as.file.module_name) {
                sb_append(sb, "namespace ");
                sb_append(sb, node->as.file.module_name);
                sb_append(sb, "\n\n");
            }
            for (size_t i = 0; i < node->as.file.imports.count; i++) {
                format_node(sb, node->as.file.imports.items[i]);
                sb_append(sb, "\n");
            }
            if (node->as.file.imports.count > 0 && node->as.file.declarations.count > 0) sb_append(sb, "\n");
            for (size_t i = 0; i < node->as.file.declarations.count; i++) {
                format_node(sb, node->as.file.declarations.items[i]);
                if (i < node->as.file.declarations.count - 1) sb_append(sb, "\n\n");
            }
            break;
        case ZT_AST_NAMESPACE_DECL:
            sb_append(sb, "namespace ");
            sb_append(sb, node->as.namespace_decl.name);
            break;
        case ZT_AST_IMPORT_DECL:
            sb_append(sb, "import ");
            sb_append(sb, node->as.import_decl.path);
            if (node->as.import_decl.alias) {
                sb_append(sb, " as ");
                sb_append(sb, node->as.import_decl.alias);
            }
            break;
        case ZT_AST_FUNC_DECL:
        {
            int params_multiline = 0;
            int where_multiline = 0;
            format_func_decl_signature(sb, node, &params_multiline, &where_multiline);
            if (node->as.func_decl.body) {
                if (!params_multiline && !where_multiline) sb_append(sb, " ");
                format_node(sb, node->as.func_decl.body);
            }
            break;
        }
        case ZT_AST_STRUCT_DECL:
            if (node->as.struct_decl.is_public) sb_append(sb, "public ");
            sb_append(sb, "struct ");
            sb_append(sb, node->as.struct_decl.name);
            if (node->as.struct_decl.type_params.count > 0) {
                sb_append(sb, "<");
                format_node_list_comma(sb, node->as.struct_decl.type_params);
                sb_append(sb, ">");
            }
            if (node->as.struct_decl.constraints.count > 0) {
                sb_append(sb, " where ");
                format_node_list_and(sb, node->as.struct_decl.constraints);
            }
            sb_append(sb, "\n");
            sb->indent_level++;
            for (size_t i = 0; i < node->as.struct_decl.fields.count; i++) {
                sb_indent(sb);
                format_node(sb, node->as.struct_decl.fields.items[i]);
                sb_append(sb, "\n");
            }
            sb->indent_level--;
            sb_indent(sb);
            sb_append(sb, "end");
            break;
        case ZT_AST_TRAIT_DECL:
            if (node->as.trait_decl.is_public) sb_append(sb, "public ");
            sb_append(sb, "trait ");
            sb_append(sb, node->as.trait_decl.name);
            if (node->as.trait_decl.type_params.count > 0) {
                sb_append(sb, "<");
                format_node_list_comma(sb, node->as.trait_decl.type_params);
                sb_append(sb, ">");
            }
            if (node->as.trait_decl.constraints.count > 0) {
                sb_append(sb, " where ");
                format_node_list_and(sb, node->as.trait_decl.constraints);
            }
            sb_append(sb, "\n");
            sb->indent_level++;
            for (size_t i = 0; i < node->as.trait_decl.methods.count; i++) {
                sb_indent(sb);
                format_node(sb, node->as.trait_decl.methods.items[i]);
                sb_append(sb, "\n");
            }
            sb->indent_level--;
            sb_indent(sb);
            sb_append(sb, "end");
            break;
        case ZT_AST_APPLY_DECL:
            sb_append(sb, "apply ");
            sb_append(sb, node->as.apply_decl.trait_name);
            if (node->as.apply_decl.trait_type_params.count > 0) {
                sb_append(sb, "<");
                format_node_list_comma(sb, node->as.apply_decl.trait_type_params);
                sb_append(sb, ">");
            }
            sb_append(sb, " to ");
            sb_append(sb, node->as.apply_decl.target_name);
            if (node->as.apply_decl.target_type_params.count > 0) {
                sb_append(sb, "<");
                format_node_list_comma(sb, node->as.apply_decl.target_type_params);
                sb_append(sb, ">");
            }
            if (node->as.apply_decl.constraints.count > 0) {
                sb_append(sb, " where ");
                format_node_list_and(sb, node->as.apply_decl.constraints);
            }
            sb_append(sb, "\n");
            sb->indent_level++;
            for (size_t i = 0; i < node->as.apply_decl.methods.count; i++) {
                sb_indent(sb);
                format_node(sb, node->as.apply_decl.methods.items[i]);
                sb_append(sb, "\n");
            }
            sb->indent_level--;
            sb_indent(sb);
            sb_append(sb, "end");
            break;
        case ZT_AST_ENUM_DECL:
            if (node->as.enum_decl.is_public) sb_append(sb, "public ");
            sb_append(sb, "enum ");
            sb_append(sb, node->as.enum_decl.name);
            if (node->as.enum_decl.type_params.count > 0) {
                sb_append(sb, "<");
                format_node_list_comma(sb, node->as.enum_decl.type_params);
                sb_append(sb, ">");
            }
            if (node->as.enum_decl.constraints.count > 0) {
                sb_append(sb, " where ");
                format_node_list_and(sb, node->as.enum_decl.constraints);
            }
            sb_append(sb, "\n");
            sb->indent_level++;
            for (size_t i = 0; i < node->as.enum_decl.variants.count; i++) {
                sb_indent(sb);
                format_node(sb, node->as.enum_decl.variants.items[i]);
                /* The parser does NOT consume commas between variants;
                 * they are separated purely by newlines. Emitting a
                 * trailing `,` here made `fmt(fmt(x))` fail to
                 * re-parse. */
                sb_append(sb, "\n");
            }
            sb->indent_level--;
            sb_indent(sb);
            sb_append(sb, "end");
            break;
        case ZT_AST_EXTERN_DECL:
            if (node->as.extern_decl.is_public) sb_append(sb, "public ");
            sb_append(sb, "extern ");
            if (node->as.extern_decl.binding) {
                sb_append(sb, "\"");
                sb_append(sb, node->as.extern_decl.binding);
                sb_append(sb, "\" ");
            }
            sb_append(sb, "\n");
            sb->indent_level++;
            for (size_t i = 0; i < node->as.extern_decl.functions.count; i++) {
                sb_indent(sb);
                format_node(sb, node->as.extern_decl.functions.items[i]);
                sb_append(sb, "\n");
            }
            sb->indent_level--;
            sb_indent(sb);
            sb_append(sb, "end");
            break;
        case ZT_AST_TYPE_ALIAS_DECL:
            if (node->as.type_alias_decl.is_public) sb_append(sb, "public ");
            sb_append(sb, "type ");
            sb_append(sb, node->as.type_alias_decl.name);
            sb_append(sb, " = ");
            format_node(sb, node->as.type_alias_decl.target_type);
            break;
        case ZT_AST_STRUCT_FIELD:
            sb_append(sb, node->as.struct_field.name);
            if (node->as.struct_field.type_node) {
                sb_append(sb, ": ");
                format_node(sb, node->as.struct_field.type_node);
            }
            if (node->as.struct_field.default_value) {
                sb_append(sb, " = ");
                format_node(sb, node->as.struct_field.default_value);
            }
            if (node->as.struct_field.where_clause) {
                sb_append(sb, " ");
                format_node(sb, node->as.struct_field.where_clause);
            }
            break;
        case ZT_AST_TRAIT_METHOD:
            format_trait_method_signature(sb, node);
            break;
        case ZT_AST_ENUM_VARIANT:
            sb_append(sb, node->as.enum_variant.name);
            if (node->as.enum_variant.fields.count > 0) {
                sb_append(sb, "(");
                format_node_list_comma(sb, node->as.enum_variant.fields);
                sb_append(sb, ")");
            }
            break;
        case ZT_AST_GENERIC_CONSTRAINT:
            sb_append(sb, node->as.generic_constraint.type_param_name);
            sb_append(sb, " is ");
            format_node(sb, node->as.generic_constraint.trait_type);
            break;
        case ZT_AST_PARAM:
            sb_append(sb, node->as.param.name);
            if (node->as.param.type_node) {
                sb_append(sb, ": ");
                format_node(sb, node->as.param.type_node);
            }
            if (node->as.param.default_value) {
                sb_append(sb, " = ");
                format_node(sb, node->as.param.default_value);
            }
            if (node->as.param.where_clause) {
                sb_append(sb, " ");
                format_node(sb, node->as.param.where_clause);
            }
            break;
        case ZT_AST_TYPE_SIMPLE:
            sb_append(sb, node->as.type_simple.name);
            break;
        case ZT_AST_TYPE_GENERIC:
            sb_append(sb, node->as.type_generic.name);
            if (node->as.type_generic.type_args.count > 0) {
                sb_append(sb, "<");
                format_node_list_comma(sb, node->as.type_generic.type_args);
                sb_append(sb, ">");
            }
            break;
        case ZT_AST_TYPE_DYN:
            sb_append(sb, "any<");
            format_node(sb, node->as.type_dyn.inner_type);
            sb_append(sb, ">");
            break;
        case ZT_AST_TYPE_CALLABLE:
            sb_append(sb, "func(");
            format_node_list_comma(sb, node->as.type_callable.params);
            sb_append(sb, ")");
            if (node->as.type_callable.return_type) {
                sb_append(sb, " -> ");
                format_node(sb, node->as.type_callable.return_type);
            }
            break;
        case ZT_AST_BLOCK:
            sb_append(sb, "\n");
            sb->indent_level++;
            for (size_t i = 0; i < node->as.block.statements.count; i++) {
                sb_indent(sb);
                format_node(sb, node->as.block.statements.items[i]);
                sb_append(sb, "\n");
            }
            sb->indent_level--;
            sb_indent(sb);
            sb_append(sb, "end");
            break;
        case ZT_AST_IF_STMT:
            sb_append(sb, "if ");
            format_node(sb, node->as.if_stmt.condition);
            sb_append(sb, " ");
            format_node(sb, node->as.if_stmt.then_block);
            if (node->as.if_stmt.else_block) {
                sb_append(sb, " else ");
                format_node(sb, node->as.if_stmt.else_block);
            }
            break;
        case ZT_AST_WHILE_STMT:
            sb_append(sb, "while ");
            format_node(sb, node->as.while_stmt.condition);
            sb_append(sb, " ");
            format_node(sb, node->as.while_stmt.body);
            break;
        case ZT_AST_FOR_STMT:
            sb_append(sb, "for ");
            sb_append(sb, node->as.for_stmt.item_name);
            if (node->as.for_stmt.index_name) {
                sb_append(sb, ", ");
                sb_append(sb, node->as.for_stmt.index_name);
            }
            sb_append(sb, " in ");
            format_node(sb, node->as.for_stmt.iterable);
            sb_append(sb, " ");
            format_node(sb, node->as.for_stmt.body);
            break;
        case ZT_AST_REPEAT_STMT:
            sb_append(sb, "repeat ");
            format_node(sb, node->as.repeat_stmt.count);
            sb_append(sb, " times ");
            format_node(sb, node->as.repeat_stmt.body);
            break;
        case ZT_AST_RETURN_STMT:
            sb_append(sb, "return");
            if (node->as.return_stmt.value) {
                sb_append(sb, " ");
                format_node(sb, node->as.return_stmt.value);
            }
            break;
        case ZT_AST_VAR_DECL:
            if (node->as.var_decl.is_module_level && node->as.var_decl.is_public) sb_append(sb, "public ");
            sb_append(sb, node->as.var_decl.is_capture ? "capture " : "var ");
            sb_append(sb, node->as.var_decl.name);
            if (node->as.var_decl.type_node) {
                sb_append(sb, ": ");
                format_node(sb, node->as.var_decl.type_node);
            }
            if (node->as.var_decl.init_value) {
                sb_append(sb, " = ");
                format_node(sb, node->as.var_decl.init_value);
            }
            break;
        case ZT_AST_CONST_DECL:
            if (node->as.const_decl.is_module_level && node->as.const_decl.is_public) sb_append(sb, "public ");
            sb_append(sb, "const ");
            sb_append(sb, node->as.const_decl.name);
            if (node->as.const_decl.type_node) {
                sb_append(sb, ": ");
                format_node(sb, node->as.const_decl.type_node);
            }
            if (node->as.const_decl.init_value) {
                sb_append(sb, " = ");
                format_node(sb, node->as.const_decl.init_value);
            }
            break;
        case ZT_AST_ASSIGN_STMT:
            sb_append(sb, node->as.assign_stmt.name);
            sb_append(sb, " = ");
            format_node(sb, node->as.assign_stmt.value);
            break;
        case ZT_AST_INDEX_ASSIGN_STMT:
            format_node(sb, node->as.index_assign_stmt.object);
            sb_append(sb, "[");
            format_node(sb, node->as.index_assign_stmt.index);
            sb_append(sb, "] = ");
            format_node(sb, node->as.index_assign_stmt.value);
            break;
        case ZT_AST_FIELD_ASSIGN_STMT:
            format_node(sb, node->as.field_assign_stmt.object);
            sb_append(sb, ".");
            sb_append(sb, node->as.field_assign_stmt.field_name);
            sb_append(sb, " = ");
            format_node(sb, node->as.field_assign_stmt.value);
            break;
        case ZT_AST_MATCH_STMT:
            sb_append(sb, "match ");
            format_node(sb, node->as.match_stmt.subject);
            sb_append(sb, "\n");
            sb->indent_level++;
            for (size_t i = 0; i < node->as.match_stmt.cases.count; i++) {
                sb_indent(sb);
                format_node(sb, node->as.match_stmt.cases.items[i]);
                sb_append(sb, "\n");
            }
            sb->indent_level--;
            sb_indent(sb);
            sb_append(sb, "end");
            break;
        case ZT_AST_MATCH_CASE:
            if (node->as.match_case.is_default) {
                sb_append(sb, "case else:");
            } else {
                sb_append(sb, "case ");
                format_node_list_comma(sb, node->as.match_case.patterns);
                sb_append(sb, ":");
            }
            /*
             * A case body parsed as a block must NOT be emitted with
             * its own `end` delimiter here — case boundaries inside a
             * `match` are closed by the next `case`/`default`/`end` of
             * the enclosing match. Emitting a block's `end` re-parses
             * as `end of match`, which either truncates the match or
             * breaks the parser on a subsequent pass.
             *
             * Each statement is preceded by a newline + indent so the
             * enclosing `match` owns the trailing newline between
             * cases without producing blank lines.
             */
            if (node->as.match_case.body != NULL &&
                    node->as.match_case.body->kind == ZT_AST_BLOCK) {
                const zt_ast_node *body = node->as.match_case.body;
                size_t i;
                sb->indent_level++;
                for (i = 0; i < body->as.block.statements.count; i++) {
                    sb_append(sb, "\n");
                    sb_indent(sb);
                    format_node(sb, body->as.block.statements.items[i]);
                }
                sb->indent_level--;
            } else if (node->as.match_case.body != NULL) {
                sb_append(sb, " ");
                format_node(sb, node->as.match_case.body);
            }
            break;
        case ZT_AST_BREAK_STMT:
            sb_append(sb, "break");
            break;
        case ZT_AST_CONTINUE_STMT:
            sb_append(sb, "continue");
            break;
        case ZT_AST_EXPR_STMT:
            format_node(sb, node->as.expr_stmt.expr);
            break;
        case ZT_AST_BINARY_EXPR:
            format_node(sb, node->as.binary_expr.left);
            sb_append(sb, " ");
            sb_append(sb, op_to_str(node->as.binary_expr.op));
            sb_append(sb, " ");
            format_node(sb, node->as.binary_expr.right);
            break;
        case ZT_AST_UNARY_EXPR:
            sb_append(sb, op_to_str(node->as.unary_expr.op));
            if (isalpha(op_to_str(node->as.unary_expr.op)[0])) sb_append(sb, " ");
            format_node(sb, node->as.unary_expr.operand);
            break;
        case ZT_AST_CALL_EXPR:
            format_node(sb, node->as.call_expr.callee);
            sb_append(sb, "(");
            for (size_t i = 0; i < node->as.call_expr.positional_args.count; i++) {
                format_node(sb, node->as.call_expr.positional_args.items[i]);
                if (i < node->as.call_expr.positional_args.count - 1 || node->as.call_expr.named_args.count > 0) {
                    sb_append(sb, ", ");
                }
            }
            for (size_t i = 0; i < node->as.call_expr.named_args.count; i++) {
                sb_append(sb, node->as.call_expr.named_args.items[i].name);
                sb_append(sb, ": ");
                format_node(sb, node->as.call_expr.named_args.items[i].value);
                if (i < node->as.call_expr.named_args.count - 1) sb_append(sb, ", ");
            }
            sb_append(sb, ")");
            break;
        case ZT_AST_FIELD_EXPR:
            format_node(sb, node->as.field_expr.object);
            sb_append(sb, ".");
            sb_append(sb, node->as.field_expr.field_name);
            break;
        case ZT_AST_ENUM_DOT_EXPR:
            sb_append(sb, ".");
            sb_append(sb, node->as.enum_dot_expr.variant_name);
            break;
        case ZT_AST_INDEX_EXPR:
            format_node(sb, node->as.index_expr.object);
            sb_append(sb, "[");
            format_node(sb, node->as.index_expr.index);
            sb_append(sb, "]");
            break;
        case ZT_AST_SLICE_EXPR:
            format_node(sb, node->as.slice_expr.object);
            sb_append(sb, "[");
            if (node->as.slice_expr.start) format_node(sb, node->as.slice_expr.start);
            sb_append(sb, "..");
            if (node->as.slice_expr.end) format_node(sb, node->as.slice_expr.end);
            sb_append(sb, "]");
            break;
        case ZT_AST_INT_EXPR:
            sb_append(sb, node->as.int_expr.value);
            break;
        case ZT_AST_FLOAT_EXPR:
            sb_append(sb, node->as.float_expr.value);
            break;
        case ZT_AST_STRING_EXPR:
            sb_append_string_value(sb, node->as.string_expr.value);
            break;
        case ZT_AST_BYTES_EXPR:
            sb_append(sb, node->as.bytes_expr.value);
            break;
        case ZT_AST_BOOL_EXPR:
            if (node->as.bool_expr.value) sb_append(sb, "true");
            else sb_append(sb, "false");
            break;
        case ZT_AST_NONE_EXPR:
            sb_append(sb, "none");
            break;
        case ZT_AST_SUCCESS_EXPR:
            sb_append(sb, "success(");
            if (node->as.success_expr.value) format_node(sb, node->as.success_expr.value);
            sb_append(sb, ")");
            break;
        case ZT_AST_ERROR_EXPR:
            sb_append(sb, "error(");
            if (node->as.error_expr.value) format_node(sb, node->as.error_expr.value);
            sb_append(sb, ")");
            break;
        case ZT_AST_LIST_EXPR:
            sb_append(sb, "[");
            format_node_list_comma(sb, node->as.list_expr.elements);
            sb_append(sb, "]");
            break;
        case ZT_AST_MAP_EXPR:
            sb_append(sb, "{");
            for (size_t i = 0; i < node->as.map_expr.entries.count; i++) {
                format_node(sb, node->as.map_expr.entries.items[i].key);
                sb_append(sb, ": ");
                format_node(sb, node->as.map_expr.entries.items[i].value);
                if (i < node->as.map_expr.entries.count - 1) sb_append(sb, ", ");
            }
            sb_append(sb, "}");
            break;
        case ZT_AST_SET_EXPR:
            sb_append(sb, "{");
            format_node_list_comma(sb, node->as.set_expr.elements);
            sb_append(sb, "}");
            break;
        case ZT_AST_STRUCT_LITERAL_EXPR:
            sb_append(sb, "{");
            for (size_t i = 0; i < node->as.struct_literal_expr.fields.count; i++) {
                sb_append(sb, node->as.struct_literal_expr.fields.items[i].name);
                sb_append(sb, ": ");
                format_node(sb, node->as.struct_literal_expr.fields.items[i].value);
                if (i < node->as.struct_literal_expr.fields.count - 1) sb_append(sb, ", ");
            }
            sb_append(sb, "}");
            break;
        case ZT_AST_IDENT_EXPR:
            sb_append(sb, node->as.ident_expr.name);
            break;
        case ZT_AST_FMT_EXPR:
            sb_append(sb, "f\"");
            for (size_t i = 0; i < node->as.fmt_expr.parts.count; i++) {
                const zt_ast_node *part = node->as.fmt_expr.parts.items[i];
                if (part == NULL) continue;
                if (part->kind == ZT_AST_STRING_EXPR) {
                    sb_append_fmt_literal_value(sb, part->as.string_expr.value);
                } else {
                    sb_append(sb, "{");
                    format_node(sb, part);
                    sb_append(sb, "}");
                }
            }
            sb_append(sb, "\"");
            break;
        case ZT_AST_GROUPED_EXPR:
            sb_append(sb, "(");
            format_node(sb, node->as.grouped_expr.inner);
            sb_append(sb, ")");
            break;
        case ZT_AST_IF_EXPR:
            sb_append(sb, "if ");
            format_node(sb, node->as.if_expr.condition);
            if (node->as.if_expr.uses_then) {
                sb_append(sb, " then ");
                format_node(sb, node->as.if_expr.then_expr);
                sb_append(sb, " else ");
                format_node(sb, node->as.if_expr.else_expr);
            } else {
                sb_append(sb, "\n");
                sb->indent_level++;
                sb_indent(sb);
                format_node(sb, node->as.if_expr.then_expr);
                sb_append(sb, "\n");
                sb->indent_level--;
                sb_indent(sb);
                sb_append(sb, "else\n");
                sb->indent_level++;
                sb_indent(sb);
                format_node(sb, node->as.if_expr.else_expr);
                sb_append(sb, "\n");
                sb->indent_level--;
                sb_indent(sb);
                sb_append(sb, "end");
            }
            break;
        case ZT_AST_WHERE_CLAUSE:
            /* The parser reads `where <expression>` and recovers
             * `param_name` from the enclosing declaration (struct
             * field name, function parameter name). Emitting
             * `where NAME -> COND` produced output that the parser
             * rejected on the second pass. */
            sb_append(sb, "where ");
            format_node(sb, node->as.where_clause.condition);
            break;
        case ZT_AST_MATCH_BINDING:
            sb_append(sb, node->as.match_binding.param_name);
            break;
        case ZT_AST_VALUE_BINDING:
            sb_append(sb, "some(");
            sb_append(sb, node->as.value_binding.name);
            sb_append(sb, ")");
            break;
        case ZT_AST_CLOSURE_EXPR:
            sb_append(sb, "func(");
            format_node_list_comma(sb, node->as.closure_expr.params);
            sb_append(sb, ")");
            if (node->as.closure_expr.return_type) {
                sb_append(sb, " -> ");
                format_node(sb, node->as.closure_expr.return_type);
            }
            if (node->as.closure_expr.is_lambda &&
                    node->as.closure_expr.body != NULL &&
                    node->as.closure_expr.body->kind == ZT_AST_BLOCK &&
                    node->as.closure_expr.body->as.block.statements.count == 1 &&
                    node->as.closure_expr.body->as.block.statements.items[0] != NULL &&
                    node->as.closure_expr.body->as.block.statements.items[0]->kind == ZT_AST_RETURN_STMT) {
                sb_append(sb, " ");
                format_node(sb, node->as.closure_expr.body->as.block.statements.items[0]->as.return_stmt.value);
                break;
            }
            sb_append(sb, " ");
            format_node(sb, node->as.closure_expr.body);
            break;
        case ZT_AST_USING_STMT:
            sb_append(sb, "using ");
            sb_append(sb, node->as.using_stmt.name);
            sb_append(sb, " = ");
            format_node(sb, node->as.using_stmt.init_value);
            if (node->as.using_stmt.cleanup_expr != NULL) {
                sb_append(sb, " then ");
                format_node(sb, node->as.using_stmt.cleanup_expr);
            } else if (node->as.using_stmt.body != NULL) {
                sb_append(sb, "\n");
                format_node(sb, node->as.using_stmt.body);
                sb_append(sb, "end");
            }
            break;
    }
}

char *zt_format_node_to_string(const zt_ast_node *node) {
    sb_t sb;
    sb_init(&sb);
    format_node(&sb, node);
    return sb.data;
}
