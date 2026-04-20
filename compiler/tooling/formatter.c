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

static void sb_free(sb_t *sb) {
    if (sb->data) free(sb->data);
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

static void sb_indent(sb_t *sb) {
    for (int i = 0; i < sb->indent_level; i++) {
        sb_append(sb, "    ");
    }
}

static void format_node(sb_t *sb, const zt_ast_node *node);

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
            if (node->as.func_decl.is_public) sb_append(sb, "public ");
            sb_append(sb, "func ");
            if (node->as.func_decl.is_mutating) sb_append(sb, "mut ");
            sb_append(sb, node->as.func_decl.name);
            if (node->as.func_decl.type_params.count > 0) {
                sb_append(sb, "[");
                format_node_list_comma(sb, node->as.func_decl.type_params);
                sb_append(sb, "]");
            }
            sb_append(sb, "(");
            format_node_list_comma(sb, node->as.func_decl.params);
            sb_append(sb, ")");
            if (node->as.func_decl.return_type) {
                sb_append(sb, " -> ");
                format_node(sb, node->as.func_decl.return_type);
            }
            if (node->as.func_decl.constraints.count > 0) {
                sb_append(sb, " where ");
                format_node_list_comma(sb, node->as.func_decl.constraints);
            }
            if (node->as.func_decl.body) {
                sb_append(sb, " ");
                format_node(sb, node->as.func_decl.body);
            }
            break;
        case ZT_AST_STRUCT_DECL:
            if (node->as.struct_decl.is_public) sb_append(sb, "public ");
            sb_append(sb, "struct ");
            sb_append(sb, node->as.struct_decl.name);
            if (node->as.struct_decl.type_params.count > 0) {
                sb_append(sb, "[");
                format_node_list_comma(sb, node->as.struct_decl.type_params);
                sb_append(sb, "]");
            }
            if (node->as.struct_decl.constraints.count > 0) {
                sb_append(sb, " where ");
                format_node_list_comma(sb, node->as.struct_decl.constraints);
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
                sb_append(sb, "[");
                format_node_list_comma(sb, node->as.trait_decl.type_params);
                sb_append(sb, "]");
            }
            if (node->as.trait_decl.constraints.count > 0) {
                sb_append(sb, " where ");
                format_node_list_comma(sb, node->as.trait_decl.constraints);
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
                sb_append(sb, "[");
                format_node_list_comma(sb, node->as.apply_decl.trait_type_params);
                sb_append(sb, "]");
            }
            sb_append(sb, " to ");
            sb_append(sb, node->as.apply_decl.target_name);
            if (node->as.apply_decl.target_type_params.count > 0) {
                sb_append(sb, "[");
                format_node_list_comma(sb, node->as.apply_decl.target_type_params);
                sb_append(sb, "]");
            }
            if (node->as.apply_decl.constraints.count > 0) {
                sb_append(sb, " where ");
                format_node_list_comma(sb, node->as.apply_decl.constraints);
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
                sb_append(sb, "[");
                format_node_list_comma(sb, node->as.enum_decl.type_params);
                sb_append(sb, "]");
            }
            if (node->as.enum_decl.constraints.count > 0) {
                sb_append(sb, " where ");
                format_node_list_comma(sb, node->as.enum_decl.constraints);
            }
            sb_append(sb, "\n");
            sb->indent_level++;
            for (size_t i = 0; i < node->as.enum_decl.variants.count; i++) {
                sb_indent(sb);
                format_node(sb, node->as.enum_decl.variants.items[i]);
                if (i < node->as.enum_decl.variants.count - 1) sb_append(sb, ",\n");
                else sb_append(sb, "\n");
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
            sb_append(sb, "func ");
            if (node->as.trait_method.is_mutating) sb_append(sb, "mut ");
            sb_append(sb, node->as.trait_method.name);
            sb_append(sb, "(");
            format_node_list_comma(sb, node->as.trait_method.params);
            sb_append(sb, ")");
            if (node->as.trait_method.return_type) {
                sb_append(sb, " -> ");
                format_node(sb, node->as.trait_method.return_type);
            }
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
            sb_append(sb, "var ");
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
                sb_append(sb, "default -> ");
            } else {
                sb_append(sb, "case ");
                format_node_list_comma(sb, node->as.match_case.patterns);
                sb_append(sb, " -> ");
            }
            format_node(sb, node->as.match_case.body);
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
            sb_append(sb, node->as.string_expr.value);
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
        case ZT_AST_IDENT_EXPR:
            sb_append(sb, node->as.ident_expr.name);
            break;
        case ZT_AST_GROUPED_EXPR:
            sb_append(sb, "(");
            format_node(sb, node->as.grouped_expr.inner);
            sb_append(sb, ")");
            break;
        case ZT_AST_WHERE_CLAUSE:
            sb_append(sb, "where ");
            sb_append(sb, node->as.where_clause.param_name);
            sb_append(sb, " -> ");
            format_node(sb, node->as.where_clause.condition);
            break;
        case ZT_AST_FORMAT_STRING_EXPR:
            sb_append(sb, "f\"format_string\""); // Simplified
            break;
        case ZT_AST_MATCH_BINDING:
            sb_append(sb, "todo_match_binding");
            break;
    }
}

char *zt_format_node_to_string(const zt_ast_node *node) {
    sb_t sb;
    sb_init(&sb);
    format_node(&sb, node);
    return sb.data;
}
