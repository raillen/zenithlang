#ifndef ZENITH_NEXT_COMPILER_HIR_NODES_MODEL_H
#define ZENITH_NEXT_COMPILER_HIR_NODES_MODEL_H

#include "compiler/frontend/lexer/token.h"
#include "compiler/semantic/types/types.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct zt_hir_decl zt_hir_decl;
typedef struct zt_hir_stmt zt_hir_stmt;
typedef struct zt_hir_expr zt_hir_expr;

typedef enum zt_hir_decl_kind {
    ZT_HIR_STRUCT_DECL,
    ZT_HIR_ENUM_DECL,
    ZT_HIR_FUNC_DECL
} zt_hir_decl_kind;

typedef enum zt_hir_stmt_kind {
    ZT_HIR_BLOCK_STMT,
    ZT_HIR_IF_STMT,
    ZT_HIR_WHILE_STMT,
    ZT_HIR_FOR_STMT,
    ZT_HIR_REPEAT_STMT,
    ZT_HIR_RETURN_STMT,
    ZT_HIR_CONST_STMT,
    ZT_HIR_VAR_STMT,
    ZT_HIR_ASSIGN_STMT,
    ZT_HIR_INDEX_ASSIGN_STMT,
    ZT_HIR_FIELD_ASSIGN_STMT,
    ZT_HIR_MATCH_STMT,
    ZT_HIR_BREAK_STMT,
    ZT_HIR_CONTINUE_STMT,
    ZT_HIR_USING_STMT,
    ZT_HIR_EXPR_STMT
} zt_hir_stmt_kind;

typedef enum zt_hir_expr_kind {
    ZT_HIR_IDENT_EXPR,
    ZT_HIR_INT_EXPR,
    ZT_HIR_FLOAT_EXPR,
    ZT_HIR_STRING_EXPR,
    ZT_HIR_BYTES_EXPR,
    ZT_HIR_BOOL_EXPR,
    ZT_HIR_NONE_EXPR,
    ZT_HIR_SUCCESS_EXPR,
    ZT_HIR_ERROR_EXPR,
    ZT_HIR_LIST_EXPR,
    ZT_HIR_MAP_EXPR,
    ZT_HIR_SET_EXPR,
    ZT_HIR_UNARY_EXPR,
    ZT_HIR_BINARY_EXPR,
    ZT_HIR_FIELD_EXPR,
    ZT_HIR_INDEX_EXPR,
    ZT_HIR_SLICE_EXPR,
    ZT_HIR_CALL_EXPR,
    ZT_HIR_METHOD_CALL_EXPR,
    ZT_HIR_DYN_METHOD_CALL_EXPR,
    ZT_HIR_FUNC_REF_EXPR,
    ZT_HIR_CALL_INDIRECT_EXPR,
    ZT_HIR_CONSTRUCT_EXPR,
    ZT_HIR_VALUE_BINDING_EXPR,
    ZT_HIR_CLOSURE_EXPR
} zt_hir_expr_kind;

typedef struct zt_hir_string_list {
    char **items;
    size_t count;
    size_t capacity;
} zt_hir_string_list;

typedef struct zt_hir_decl_list {
    zt_hir_decl **items;
    size_t count;
    size_t capacity;
} zt_hir_decl_list;

typedef struct zt_hir_stmt_list {
    zt_hir_stmt **items;
    size_t count;
    size_t capacity;
} zt_hir_stmt_list;

typedef struct zt_hir_expr_list {
    zt_hir_expr **items;
    size_t count;
    size_t capacity;
} zt_hir_expr_list;

typedef struct zt_hir_param {
    char *name;
    zt_type *type;
    zt_hir_expr *where_clause;
} zt_hir_param;

typedef struct zt_hir_param_list {
    zt_hir_param *items;
    size_t count;
    size_t capacity;
} zt_hir_param_list;

typedef struct zt_hir_capture {
    char *name;
    zt_type *type;
} zt_hir_capture;

typedef struct zt_hir_capture_list {
    zt_hir_capture *items;
    size_t count;
    size_t capacity;
} zt_hir_capture_list;


typedef struct zt_hir_field_decl {
    char *name;
    zt_type *type;
    zt_hir_expr *default_value;
    zt_hir_expr *where_clause;
} zt_hir_field_decl;

typedef struct zt_hir_field_decl_list {
    zt_hir_field_decl *items;
    size_t count;
    size_t capacity;
} zt_hir_field_decl_list;

typedef struct zt_hir_enum_variant_field {
    char *name;
    zt_type *type;
} zt_hir_enum_variant_field;

typedef struct zt_hir_enum_variant_field_list {
    zt_hir_enum_variant_field *items;
    size_t count;
    size_t capacity;
} zt_hir_enum_variant_field_list;

typedef struct zt_hir_enum_variant {
    char *name;
    zt_hir_enum_variant_field_list fields;
} zt_hir_enum_variant;

typedef struct zt_hir_enum_variant_list {
    zt_hir_enum_variant *items;
    size_t count;
    size_t capacity;
} zt_hir_enum_variant_list;

typedef struct zt_hir_field_init {
    zt_source_span span;
    char *name;
    zt_hir_expr *value;
} zt_hir_field_init;

typedef struct zt_hir_field_init_list {
    zt_hir_field_init *items;
    size_t count;
    size_t capacity;
} zt_hir_field_init_list;

typedef struct zt_hir_map_entry {
    zt_source_span span;
    zt_hir_expr *key;
    zt_hir_expr *value;
} zt_hir_map_entry;

typedef struct zt_hir_map_entry_list {
    zt_hir_map_entry *items;
    size_t count;
    size_t capacity;
} zt_hir_map_entry_list;

typedef struct zt_hir_match_case {
    zt_source_span span;
    int is_default;
    zt_hir_expr_list patterns;
    zt_hir_stmt *body;
} zt_hir_match_case;

typedef struct zt_hir_match_case_list {
    zt_hir_match_case *items;
    size_t count;
    size_t capacity;
} zt_hir_match_case_list;

typedef struct zt_hir_module_var {
    zt_source_span span;
    char *name;
    zt_type *type;
    zt_hir_expr *init_value;
} zt_hir_module_var;

typedef struct zt_hir_module_var_list {
    zt_hir_module_var *items;
    size_t count;
    size_t capacity;
} zt_hir_module_var_list;

typedef struct zt_hir_module {
    zt_source_span span;
    char *module_name;
    zt_hir_module_var_list module_vars;
    zt_hir_decl_list declarations;
} zt_hir_module;

struct zt_hir_decl {
    zt_hir_decl_kind kind;
    zt_source_span span;
    union {
        struct {
            char *name;
            int is_public;
            zt_hir_string_list type_params;
            zt_hir_field_decl_list fields;
        } struct_decl;
        struct {
            char *name;
            int is_public;
            zt_hir_string_list type_params;
            zt_hir_enum_variant_list variants;
        } enum_decl;
        struct {
            char *name;
            int is_public;
            int is_mutating;
            char *receiver_type_name;
            char *implemented_trait_name;
            zt_hir_string_list type_params;
            zt_hir_param_list params;
            zt_type *return_type;
            zt_hir_stmt *body;
        } func_decl;
    } as;
};

struct zt_hir_stmt {
    zt_hir_stmt_kind kind;
    zt_source_span span;
    union {
        struct {
            zt_hir_stmt_list statements;
        } block_stmt;
        struct {
            zt_hir_expr *condition;
            zt_hir_stmt *then_block;
            zt_hir_stmt *else_block;
        } if_stmt;
        struct {
            zt_hir_expr *condition;
            zt_hir_stmt *body;
        } while_stmt;
        struct {
            char *item_name;
            char *index_name;
            zt_hir_expr *iterable;
            zt_hir_stmt *body;
        } for_stmt;
        struct {
            zt_hir_expr *count;
            zt_hir_stmt *body;
        } repeat_stmt;
        struct {
            zt_hir_expr *value;
        } return_stmt;
        struct {
            char *name;
            zt_type *type;
            zt_hir_expr *init_value;
        } const_stmt;
        struct {
            char *name;
            zt_type *type;
            zt_hir_expr *init_value;
        } var_stmt;
        struct {
            char *name;
            zt_hir_expr *value;
        } assign_stmt;
        struct {
            zt_hir_expr *object;
            zt_hir_expr *index;
            zt_hir_expr *value;
        } index_assign_stmt;
        struct {
            zt_hir_expr *object;
            char *field_name;
            zt_hir_expr *value;
        } field_assign_stmt;
        struct {
            zt_hir_expr *subject;
            zt_hir_match_case_list cases;
        } match_stmt;
        struct {
            char *name;
            zt_type *type;
            zt_hir_expr *init_value;
            zt_hir_expr *cleanup_expr;
            zt_hir_stmt *body;
        } using_stmt;
        struct {
            zt_hir_expr *expr;
        } expr_stmt;
    } as;
};

struct zt_hir_expr {
    zt_hir_expr_kind kind;
    zt_source_span span;
    zt_type *type;
    union {
        struct { char *name; } ident_expr;
        struct { char *value; } int_expr;
        struct { char *value; } float_expr;
        struct { char *value; } string_expr;
        struct { char *value; } bytes_expr;
        struct { int value; } bool_expr;
        struct { zt_hir_expr *value; } success_expr;
        struct { zt_hir_expr *value; } error_expr;
        struct { zt_hir_expr_list elements; } list_expr;
        struct { zt_hir_map_entry_list entries; } map_expr;
        struct { zt_hir_expr_list elements; } set_expr;
        struct { zt_token_kind op; zt_hir_expr *operand; } unary_expr;
        struct { zt_token_kind op; zt_hir_expr *left; zt_hir_expr *right; } binary_expr;
        struct { zt_hir_expr *object; char *field_name; } field_expr;
        struct { zt_hir_expr *object; zt_hir_expr *index; } index_expr;
        struct { zt_hir_expr *object; zt_hir_expr *start; zt_hir_expr *end; } slice_expr;
        struct { char *callee_name; zt_hir_expr_list args; } call_expr;
        struct { zt_hir_expr *receiver; char *method_name; zt_hir_expr_list args; } method_call_expr;
        struct { zt_hir_expr *receiver; char *method_name; char *trait_name; zt_hir_expr_list args; } dyn_method_call_expr;
        struct { char *func_name; zt_type *callable_type; } func_ref_expr;
        struct { zt_hir_expr *callable; zt_hir_expr_list args; } call_indirect_expr;
        struct { char *type_name; zt_hir_field_init_list fields; } construct_expr;
        struct { char *name; } value_binding_expr;
        struct {
            zt_hir_param_list params;
            zt_type *return_type;
            zt_hir_stmt *body;
            zt_hir_capture_list captures;
        } closure_expr;
    } as;
};

const char *zt_hir_decl_kind_name(zt_hir_decl_kind kind);
const char *zt_hir_stmt_kind_name(zt_hir_stmt_kind kind);
const char *zt_hir_expr_kind_name(zt_hir_expr_kind kind);

zt_hir_module *zt_hir_module_make(zt_source_span span);
void zt_hir_module_dispose(zt_hir_module *module);
zt_hir_decl *zt_hir_decl_make(zt_hir_decl_kind kind, zt_source_span span);
void zt_hir_decl_dispose(zt_hir_decl *decl);
zt_hir_stmt *zt_hir_stmt_make(zt_hir_stmt_kind kind, zt_source_span span);
void zt_hir_stmt_dispose(zt_hir_stmt *stmt);
zt_hir_expr *zt_hir_expr_make(zt_hir_expr_kind kind, zt_source_span span);
void zt_hir_expr_dispose(zt_hir_expr *expr);

zt_hir_string_list zt_hir_string_list_make(void);
void zt_hir_string_list_push(zt_hir_string_list *list, char *value);
void zt_hir_string_list_dispose(zt_hir_string_list *list);
zt_hir_decl_list zt_hir_decl_list_make(void);
void zt_hir_decl_list_push(zt_hir_decl_list *list, zt_hir_decl *decl);
void zt_hir_decl_list_dispose(zt_hir_decl_list *list);
zt_hir_stmt_list zt_hir_stmt_list_make(void);
void zt_hir_stmt_list_push(zt_hir_stmt_list *list, zt_hir_stmt *stmt);
void zt_hir_stmt_list_dispose(zt_hir_stmt_list *list);
zt_hir_expr_list zt_hir_expr_list_make(void);
void zt_hir_expr_list_push(zt_hir_expr_list *list, zt_hir_expr *expr);
void zt_hir_expr_list_dispose(zt_hir_expr_list *list);
zt_hir_param_list zt_hir_param_list_make(void);
void zt_hir_param_list_push(zt_hir_param_list *list, zt_hir_param param);
void zt_hir_param_list_dispose(zt_hir_param_list *list);
zt_hir_field_decl_list zt_hir_field_decl_list_make(void);
void zt_hir_field_decl_list_push(zt_hir_field_decl_list *list, zt_hir_field_decl field);
void zt_hir_field_decl_list_dispose(zt_hir_field_decl_list *list);
zt_hir_enum_variant_field_list zt_hir_enum_variant_field_list_make(void);
void zt_hir_enum_variant_field_list_push(zt_hir_enum_variant_field_list *list, zt_hir_enum_variant_field field);
void zt_hir_enum_variant_field_list_dispose(zt_hir_enum_variant_field_list *list);
zt_hir_enum_variant_list zt_hir_enum_variant_list_make(void);
void zt_hir_enum_variant_list_push(zt_hir_enum_variant_list *list, zt_hir_enum_variant variant);
void zt_hir_enum_variant_list_dispose(zt_hir_enum_variant_list *list);
zt_hir_field_init_list zt_hir_field_init_list_make(void);
void zt_hir_field_init_list_push(zt_hir_field_init_list *list, zt_hir_field_init field);
void zt_hir_field_init_list_dispose(zt_hir_field_init_list *list);
zt_hir_map_entry_list zt_hir_map_entry_list_make(void);
void zt_hir_map_entry_list_push(zt_hir_map_entry_list *list, zt_hir_map_entry entry);
void zt_hir_map_entry_list_dispose(zt_hir_map_entry_list *list);
zt_hir_match_case_list zt_hir_match_case_list_make(void);
void zt_hir_match_case_list_push(zt_hir_match_case_list *list, zt_hir_match_case match_case);
void zt_hir_match_case_list_dispose(zt_hir_match_case_list *list);
zt_hir_module_var_list zt_hir_module_var_list_make(void);
zt_hir_capture_list zt_hir_capture_list_make(void);
void zt_hir_capture_list_push(zt_hir_capture_list *list, zt_hir_capture capture);
void zt_hir_capture_list_dispose(zt_hir_capture_list *list);
void zt_hir_module_var_list_push(zt_hir_module_var_list *list, zt_hir_module_var module_var);
void zt_hir_module_var_list_dispose(zt_hir_module_var_list *list);

#ifdef __cplusplus
}
#endif

#endif
