#ifndef ZENITH_NEXT_COMPILER_FRONTEND_AST_MODEL_H
#define ZENITH_NEXT_COMPILER_FRONTEND_AST_MODEL_H

#include "compiler/frontend/lexer/token.h"

#include <stddef.h>
#include "compiler/utils/arena.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum zt_ast_kind {
    ZT_AST_FILE,
    ZT_AST_NAMESPACE_DECL,
    ZT_AST_IMPORT_DECL,
    ZT_AST_FUNC_DECL,
    ZT_AST_STRUCT_DECL,
    ZT_AST_TRAIT_DECL,
    ZT_AST_APPLY_DECL,
    ZT_AST_ENUM_DECL,
    ZT_AST_EXTERN_DECL,
    ZT_AST_STRUCT_FIELD,
    ZT_AST_TRAIT_METHOD,
    ZT_AST_ENUM_VARIANT,
    ZT_AST_GENERIC_CONSTRAINT,
    ZT_AST_PARAM,
    ZT_AST_TYPE_SIMPLE,
    ZT_AST_TYPE_GENERIC,
    ZT_AST_TYPE_DYN,
    ZT_AST_BLOCK,
    ZT_AST_IF_STMT,
    ZT_AST_WHILE_STMT,
    ZT_AST_FOR_STMT,
    ZT_AST_REPEAT_STMT,
    ZT_AST_RETURN_STMT,
    ZT_AST_VAR_DECL,
    ZT_AST_CONST_DECL,
    ZT_AST_ASSIGN_STMT,
    ZT_AST_INDEX_ASSIGN_STMT,
    ZT_AST_FIELD_ASSIGN_STMT,
    ZT_AST_MATCH_STMT,
    ZT_AST_MATCH_CASE,
    ZT_AST_BREAK_STMT,
    ZT_AST_CONTINUE_STMT,
    ZT_AST_EXPR_STMT,
    ZT_AST_BINARY_EXPR,
    ZT_AST_UNARY_EXPR,
    ZT_AST_CALL_EXPR,
    ZT_AST_FIELD_EXPR,
    ZT_AST_INDEX_EXPR,
    ZT_AST_SLICE_EXPR,
    ZT_AST_INT_EXPR,
    ZT_AST_FLOAT_EXPR,
    ZT_AST_STRING_EXPR,
    ZT_AST_BYTES_EXPR,
    ZT_AST_BOOL_EXPR,
    ZT_AST_NONE_EXPR,
    ZT_AST_SUCCESS_EXPR,
    ZT_AST_ERROR_EXPR,
    ZT_AST_LIST_EXPR,
    ZT_AST_MAP_EXPR,
    ZT_AST_IDENT_EXPR,
    ZT_AST_FMT_EXPR,
    ZT_AST_GROUPED_EXPR,
    ZT_AST_WHERE_CLAUSE,
    ZT_AST_MATCH_BINDING,
    ZT_AST_VALUE_BINDING
} zt_ast_kind;

typedef struct zt_ast_node zt_ast_node;

typedef struct zt_ast_node_list {
    zt_ast_node **items;
    size_t count;
    size_t capacity;
} zt_ast_node_list;

typedef struct zt_ast_named_arg {
    zt_source_span span;
    const char *name;
    zt_ast_node *value;
} zt_ast_named_arg;

typedef struct zt_ast_named_arg_list {
    zt_ast_named_arg *items;
    size_t count;
    size_t capacity;
} zt_ast_named_arg_list;

typedef struct zt_ast_map_entry {
    zt_source_span span;
    zt_ast_node *key;
    zt_ast_node *value;
} zt_ast_map_entry;

typedef struct zt_ast_map_entry_list {
    zt_ast_map_entry *items;
    size_t count;
    size_t capacity;
} zt_ast_map_entry_list;

struct zt_ast_node {
    zt_ast_kind kind;
    zt_source_span span;
    
    zt_token *comments;
    size_t comment_count;

    union {
        struct {
            const char *module_name;
            zt_ast_node_list imports;
            zt_ast_node_list declarations;
        } file;

        struct {
            const char *name;
        } namespace_decl;

        struct {
            const char *path;
            const char *alias;
        } import_decl;

        struct {
            const char *name;
            zt_ast_node_list type_params;
            zt_ast_node_list params;
            zt_ast_node_list constraints;
            zt_ast_node *return_type;
            zt_ast_node *body;
            int is_public;
            int is_mutating;
            int is_test;
        } func_decl;

        struct {
            const char *name;
            zt_ast_node_list type_params;
            zt_ast_node_list constraints;
            zt_ast_node_list fields;
            int is_public;
        } struct_decl;

        struct {
            const char *name;
            zt_ast_node_list type_params;
            zt_ast_node_list constraints;
            zt_ast_node_list methods;
            int is_public;
        } trait_decl;

        struct {
            const char *trait_name;
            zt_ast_node_list trait_type_params;
            const char *target_name;
            zt_ast_node_list target_type_params;
            zt_ast_node_list constraints;
            zt_ast_node_list methods;
        } apply_decl;

        struct {
            const char *name;
            zt_ast_node_list type_params;
            zt_ast_node_list constraints;
            zt_ast_node_list variants;
            int is_public;
        } enum_decl;

        struct {
            const char *binding;
            zt_ast_node_list functions;
            int is_public;
        } extern_decl;

        struct {
            const char *name;
            zt_ast_node *type_node;
            zt_ast_node *default_value;
            zt_ast_node *where_clause;
        } struct_field;

        struct {
            const char *name;
            zt_ast_node_list params;
            zt_ast_node *return_type;
            int is_mutating;
        } trait_method;

        struct {
            const char *name;
            zt_ast_node_list fields;
        } enum_variant;

        struct {
            const char *type_param_name;
            zt_ast_node *trait_type;
        } generic_constraint;

        struct {
            const char *name;
            zt_ast_node *type_node;
            zt_ast_node *default_value;
            zt_ast_node *where_clause;
        } param;

        struct {
            const char *name;
        } type_simple;

        struct {
            const char *name;
            zt_ast_node_list type_args;
        } type_generic;

        struct {
            zt_ast_node *inner_type;
        } type_dyn;

        struct {
            zt_ast_node_list statements;
        } block;

        struct {
            zt_ast_node *condition;
            zt_ast_node *then_block;
            zt_ast_node *else_block;
        } if_stmt;

        struct {
            zt_ast_node *condition;
            zt_ast_node *body;
        } while_stmt;

        struct {
            const char *item_name;
            const char *index_name;
            zt_ast_node *iterable;
            zt_ast_node *body;
        } for_stmt;

        struct {
            zt_ast_node *count;
            zt_ast_node *body;
        } repeat_stmt;

        struct {
            zt_ast_node *value;
        } return_stmt;

        struct {
            const char *name;
            zt_ast_node *type_node;
            zt_ast_node *init_value;
            int is_public;
            int is_module_level;
        } var_decl;

        struct {
            const char *name;
            zt_ast_node *type_node;
            zt_ast_node *init_value;
            int is_public;
            int is_module_level;
        } const_decl;

        struct {
            const char *name;
            zt_ast_node *value;
        } assign_stmt;

        struct {
            zt_ast_node *object;
            zt_ast_node *index;
            zt_ast_node *value;
        } index_assign_stmt;

        struct {
            zt_ast_node *object;
            const char *field_name;
            zt_ast_node *value;
        } field_assign_stmt;

        struct {
            zt_ast_node *subject;
            zt_ast_node_list cases;
        } match_stmt;

        struct {
            zt_ast_node_list patterns;
            zt_ast_node *body;
            int is_default;
        } match_case;

        struct {
            zt_ast_node *expr;
        } expr_stmt;

        struct {
            zt_ast_node *left;
            zt_ast_node *right;
            zt_token_kind op;
        } binary_expr;

        struct {
            zt_token_kind op;
            zt_ast_node *operand;
        } unary_expr;

        struct {
            zt_ast_node *callee;
            zt_ast_node_list positional_args;
            zt_ast_named_arg_list named_args;
        } call_expr;

        struct {
            zt_ast_node *object;
            const char *field_name;
        } field_expr;

        struct {
            zt_ast_node *object;
            zt_ast_node *index;
        } index_expr;

        struct {
            zt_ast_node *object;
            zt_ast_node *start;
            zt_ast_node *end;
        } slice_expr;

        struct {
            const char *value;
        } int_expr;

        struct {
            const char *value;
        } float_expr;

        struct {
            const char *value;
        } string_expr;

        struct {
            const char *value;
        } bytes_expr;

        struct {
            int value;
        } bool_expr;

        struct {
            int dummy;
        } none_expr;

        struct {
            zt_ast_node *value;
        } success_expr;

        struct {
            zt_ast_node *value;
        } error_expr;

        struct {
            zt_ast_node_list elements;
        } list_expr;

        struct {
            zt_ast_map_entry_list entries;
        } map_expr;

        struct {
            const char *name;
        } ident_expr;

        struct {
            zt_ast_node_list parts;
        } fmt_expr;

        struct {
            zt_ast_node *inner;
        } grouped_expr;

        struct {
            const char *param_name;
            zt_ast_node *condition;
        } where_clause;

        struct {
            const char *param_name;
        } match_binding;

        struct {
            const char *name;
            zt_ast_node *type_node;
        } value_binding;
    } as;
};

zt_ast_node *zt_ast_make(zt_arena *arena, zt_ast_kind kind, zt_source_span span);

zt_ast_node_list zt_ast_node_list_make(void);
void zt_ast_node_list_push(zt_arena *arena, zt_ast_node_list *list, zt_ast_node *node);

zt_ast_named_arg_list zt_ast_named_arg_list_make(void);
void zt_ast_named_arg_list_push(zt_arena *arena, zt_ast_named_arg_list *list, zt_ast_named_arg arg);

zt_ast_map_entry_list zt_ast_map_entry_list_make(void);
void zt_ast_map_entry_list_push(zt_arena *arena, zt_ast_map_entry_list *list, zt_ast_map_entry entry);

const char *zt_ast_kind_name(zt_ast_kind kind);

#ifdef __cplusplus
}
#endif

#endif
