#include "compiler/frontend/ast/model.h"

#include <stdlib.h>
#include <string.h>

const char *zt_ast_kind_name(zt_ast_kind kind) {
    switch (kind) {
        case ZT_AST_FILE: return "file";
        case ZT_AST_NAMESPACE_DECL: return "namespace_decl";
        case ZT_AST_IMPORT_DECL: return "import_decl";
        case ZT_AST_FUNC_DECL: return "func_decl";
        case ZT_AST_STRUCT_DECL: return "struct_decl";
        case ZT_AST_TRAIT_DECL: return "trait_decl";
        case ZT_AST_APPLY_DECL: return "apply_decl";
        case ZT_AST_ENUM_DECL: return "enum_decl";
        case ZT_AST_EXTERN_DECL: return "extern_decl";
        case ZT_AST_STRUCT_FIELD: return "struct_field";
        case ZT_AST_TRAIT_METHOD: return "trait_method";
        case ZT_AST_ENUM_VARIANT: return "enum_variant";
        case ZT_AST_GENERIC_CONSTRAINT: return "generic_constraint";
        case ZT_AST_PARAM: return "param";
        case ZT_AST_TYPE_SIMPLE: return "type_simple";
        case ZT_AST_TYPE_GENERIC: return "type_generic";
        case ZT_AST_BLOCK: return "block";
        case ZT_AST_IF_STMT: return "if_stmt";
        case ZT_AST_WHILE_STMT: return "while_stmt";
        case ZT_AST_FOR_STMT: return "for_stmt";
        case ZT_AST_REPEAT_STMT: return "repeat_stmt";
        case ZT_AST_RETURN_STMT: return "return_stmt";
        case ZT_AST_VAR_DECL: return "var_decl";
        case ZT_AST_CONST_DECL: return "const_decl";
        case ZT_AST_ASSIGN_STMT: return "assign_stmt";
        case ZT_AST_INDEX_ASSIGN_STMT: return "index_assign_stmt";
        case ZT_AST_FIELD_ASSIGN_STMT: return "field_assign_stmt";
        case ZT_AST_MATCH_STMT: return "match_stmt";
        case ZT_AST_MATCH_CASE: return "match_case";
        case ZT_AST_BREAK_STMT: return "break_stmt";
        case ZT_AST_CONTINUE_STMT: return "continue_stmt";
        case ZT_AST_EXPR_STMT: return "expr_stmt";
        case ZT_AST_BINARY_EXPR: return "binary_expr";
        case ZT_AST_UNARY_EXPR: return "unary_expr";
        case ZT_AST_CALL_EXPR: return "call_expr";
        case ZT_AST_FIELD_EXPR: return "field_expr";
        case ZT_AST_INDEX_EXPR: return "index_expr";
        case ZT_AST_SLICE_EXPR: return "slice_expr";
        case ZT_AST_INT_EXPR: return "int_expr";
        case ZT_AST_FLOAT_EXPR: return "float_expr";
        case ZT_AST_STRING_EXPR: return "string_expr";
        case ZT_AST_BYTES_EXPR: return "bytes_expr";
        case ZT_AST_BOOL_EXPR: return "bool_expr";
        case ZT_AST_NONE_EXPR: return "none_expr";
        case ZT_AST_SUCCESS_EXPR: return "success_expr";
        case ZT_AST_ERROR_EXPR: return "error_expr";
        case ZT_AST_LIST_EXPR: return "list_expr";
        case ZT_AST_MAP_EXPR: return "map_expr";
        case ZT_AST_IDENT_EXPR: return "ident_expr";
        case ZT_AST_FMT_EXPR: return "fmt_expr";
        case ZT_AST_GROUPED_EXPR: return "grouped_expr";
        case ZT_AST_WHERE_CLAUSE: return "where_clause";
        case ZT_AST_MATCH_BINDING: return "match_binding";
        case ZT_AST_VALUE_BINDING: return "value_binding";
        default: return "unknown";
    }
}


zt_ast_node *zt_ast_make(zt_arena *arena, zt_ast_kind kind, zt_source_span span) {
    zt_ast_node *node = (zt_ast_node *)zt_arena_alloc(arena, sizeof(zt_ast_node));
    if (node == NULL) return NULL;
    memset(node, 0, sizeof(zt_ast_node));
    node->kind = kind;
    node->span = span;
    return node;
}

void zt_ast_node_list_push(zt_arena *arena, zt_ast_node_list *list, zt_ast_node *node) {
    if (list->count >= list->capacity) {
        size_t new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        zt_ast_node **new_items = (zt_ast_node **)zt_arena_alloc(arena, new_capacity * sizeof(zt_ast_node *));
        if (list->items != NULL) {
            memcpy(new_items, list->items, list->count * sizeof(zt_ast_node *));
        }
        list->items = new_items;
        list->capacity = new_capacity;
    }
    list->items[list->count++] = node;
}

void zt_ast_named_arg_list_push(zt_arena *arena, zt_ast_named_arg_list *list, zt_ast_named_arg arg) {
    if (list->count >= list->capacity) {
        size_t new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        zt_ast_named_arg *new_items = (zt_ast_named_arg *)zt_arena_alloc(arena, new_capacity * sizeof(zt_ast_named_arg));
        if (list->items != NULL) {
            memcpy(new_items, list->items, list->count * sizeof(zt_ast_named_arg));
        }
        list->items = new_items;
        list->capacity = new_capacity;
    }
    list->items[list->count++] = arg;
}

void zt_ast_map_entry_list_push(zt_arena *arena, zt_ast_map_entry_list *list, zt_ast_map_entry entry) {
    if (list->count >= list->capacity) {
        size_t new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        zt_ast_map_entry *new_items = (zt_ast_map_entry *)zt_arena_alloc(arena, new_capacity * sizeof(zt_ast_map_entry));
        if (list->items != NULL) {
            memcpy(new_items, list->items, list->count * sizeof(zt_ast_map_entry));
        }
        list->items = new_items;
        list->capacity = new_capacity;
    }
    list->items[list->count++] = entry;
}

zt_ast_node_list zt_ast_node_list_make(void) {
    zt_ast_node_list list;
    list.items = NULL;
    list.count = 0;
    list.capacity = 0;
    return list;
}

zt_ast_map_entry_list zt_ast_map_entry_list_make(void) {
    zt_ast_map_entry_list list;
    list.items = NULL;
    list.count = 0;
    list.capacity = 0;
    return list;
}


zt_ast_named_arg_list zt_ast_named_arg_list_make(void) {
    zt_ast_named_arg_list list;
    list.items = NULL;
    list.count = 0;
    list.capacity = 0;
    return list;
}

