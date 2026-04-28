#ifndef ZENITH_NEXT_COMPILER_ZIR_MODEL_H
#define ZENITH_NEXT_COMPILER_ZIR_MODEL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct zir_expr zir_expr;

typedef struct zir_span {
    const char *source_name;
    size_t line;
    size_t column;
} zir_span;

typedef struct zir_param {
    const char *name;
    const char *type_name;
    zir_span span;
    zir_expr *where_clause;
} zir_param;

typedef struct zir_field_decl {
    const char *name;
    const char *type_name;
    zir_span span;
    zir_expr *where_clause;
} zir_field_decl;

typedef struct zir_expr_list {
    zir_expr **items;
    size_t count;
    size_t capacity;
} zir_expr_list;

typedef struct zir_named_expr {
    const char *name;
    zir_expr *value;
    zir_span span;
} zir_named_expr;

typedef struct zir_named_expr_list {
    zir_named_expr *items;
    size_t count;
    size_t capacity;
} zir_named_expr_list;

typedef struct zir_map_entry {
    zir_expr *key;
    zir_expr *value;
    zir_span span;
} zir_map_entry;

typedef struct zir_map_entry_list {
    zir_map_entry *items;
    size_t count;
    size_t capacity;
} zir_map_entry_list;

typedef struct zir_capture {
    const char *name;
    const char *type_name;
    zir_span span;
} zir_capture;

typedef struct zir_capture_list {
    zir_capture *items;
    size_t count;
    size_t capacity;
} zir_capture_list;

typedef enum zir_expr_kind {
    ZIR_EXPR_NAME,
    ZIR_EXPR_INT,
    ZIR_EXPR_FLOAT,
    ZIR_EXPR_BOOL,
    ZIR_EXPR_STRING,
    ZIR_EXPR_BYTES,
    ZIR_EXPR_COPY,
    ZIR_EXPR_UNARY,
    ZIR_EXPR_BINARY,
    ZIR_EXPR_CALL_DIRECT,
    ZIR_EXPR_CALL_EXTERN,
    ZIR_EXPR_CALL_RUNTIME_INTRINSIC,
    ZIR_EXPR_CALL_DYN,
    ZIR_EXPR_MAKE_STRUCT,
    ZIR_EXPR_MAKE_LIST,
    ZIR_EXPR_MAKE_MAP,
    ZIR_EXPR_MAKE_SET,
    ZIR_EXPR_GET_FIELD,
    ZIR_EXPR_SET_FIELD,
    ZIR_EXPR_INDEX_SEQ,
    ZIR_EXPR_SLICE_SEQ,
    ZIR_EXPR_LIST_LEN,
    ZIR_EXPR_MAP_LEN,
    ZIR_EXPR_SET_LEN,
    ZIR_EXPR_LIST_PUSH,
    ZIR_EXPR_LIST_SET,
    ZIR_EXPR_MAP_SET,
    ZIR_EXPR_SET_ADD,
    ZIR_EXPR_SET_REMOVE,
    ZIR_EXPR_SET_HAS,
    ZIR_EXPR_OPTIONAL_PRESENT,
    ZIR_EXPR_OPTIONAL_EMPTY,
    ZIR_EXPR_OPTIONAL_IS_PRESENT,
    ZIR_EXPR_COALESCE,
    ZIR_EXPR_OUTCOME_SUCCESS,
    ZIR_EXPR_OUTCOME_FAILURE,
    ZIR_EXPR_OUTCOME_IS_SUCCESS,
    ZIR_EXPR_OUTCOME_VALUE,
    ZIR_EXPR_TRY_PROPAGATE,
    ZIR_EXPR_OUTCOME_WRAP_CONTEXT,
    ZIR_EXPR_OPTIONAL_VALUE,
    ZIR_EXPR_FUNC_REF,
    ZIR_EXPR_CALL_INDIRECT,
    ZIR_EXPR_IF,
    ZIR_EXPR_MAKE_CLOSURE
} zir_expr_kind;

struct zir_expr {
    zir_expr_kind kind;
    zir_span span;
    union {
        struct {
            const char *text;
        } text;
        struct {
            int value;
        } bool_literal;
        struct {
            zir_expr *value;
        } single;
        struct {
            const char *op_name;
            zir_expr *operand;
        } unary;
        struct {
            const char *op_name;
            zir_expr *left;
            zir_expr *right;
        } binary;
        struct {
            const char *callee_name;
            zir_expr_list args;
        } call;
        struct {
            zir_expr *receiver;
            const char *method_name;
            const char *trait_name;
            zir_expr_list args;
        } dyn_call;
        struct {
            const char *type_name;
            zir_named_expr_list fields;
        } make_struct;
        struct {
            const char *item_type_name;
            zir_expr_list items;
        } make_list;
        struct {
            const char *key_type_name;
            const char *value_type_name;
            zir_map_entry_list entries;
        } make_map;
        struct {
            const char *elem_type_name;
            zir_expr_list items;
        } make_set;
        struct {
            zir_expr *object;
            const char *field_name;
            zir_expr *value;
        } field;
        struct {
            zir_expr *first;
            zir_expr *second;
            zir_expr *third;
        } sequence;
        struct {
            const char *type_name;
        } type_only;
        struct {
            const char *func_name;
            const char *callable_type_name;
        } func_ref;
        struct {
            zir_expr *callable;
            zir_expr_list args;
        } call_indirect;
        struct {
            const char *func_name;
            zir_expr_list captures;
        } make_closure;
    } as;
};

typedef enum zir_instruction_kind {
    ZIR_INSTR_ASSIGN,
    ZIR_INSTR_EFFECT,
    ZIR_INSTR_CHECK_CONTRACT
} zir_instruction_kind;

typedef struct zir_instruction {
    zir_instruction_kind kind;
    const char *dest_name;
    const char *type_name;
    const char *expr_text;
    zir_span span;
    zir_expr *expr;
} zir_instruction;

typedef enum zir_terminator_kind {
    ZIR_TERM_RETURN,
    ZIR_TERM_JUMP,
    ZIR_TERM_BRANCH_IF,
    ZIR_TERM_PANIC,
    ZIR_TERM_UNREACHABLE
} zir_terminator_kind;

typedef struct zir_terminator {
    zir_terminator_kind kind;
    const char *value_text;
    const char *target_label;
    const char *condition_text;
    const char *then_label;
    const char *else_label;
    const char *message_text;
    zir_span span;
    zir_expr *value;
    zir_expr *condition;
    zir_expr *message;
} zir_terminator;

typedef struct zir_block {
    const char *label;
    const zir_instruction *instructions;
    size_t instruction_count;
    zir_terminator terminator;
    zir_span span;
} zir_block;

typedef struct zir_struct_decl {
    const char *name;
    const zir_field_decl *fields;
    size_t field_count;
    zir_span span;
} zir_struct_decl;

typedef struct zir_enum_variant_field_decl {
    const char *name;
    const char *type_name;
    zir_span span;
} zir_enum_variant_field_decl;

typedef struct zir_enum_variant_decl {
    const char *name;
    const zir_enum_variant_field_decl *fields;
    size_t field_count;
    zir_span span;
} zir_enum_variant_decl;

typedef struct zir_enum_decl {
    const char *name;
    const zir_enum_variant_decl *variants;
    size_t variant_count;
    zir_span span;
} zir_enum_decl;

typedef struct zir_module_var {
    const char *name;
    const char *type_name;
    const char *init_expr_text;
    zir_span span;
    zir_expr *init_expr;
} zir_module_var;

typedef struct zir_function {
    const char *name;
    const zir_param *params;
    size_t param_count;
    const char *return_type;
    const zir_block *blocks;
    size_t block_count;
    zir_span span;
    const char *receiver_type_name;
    const char *implemented_trait_name;
    int is_mutating;
    int is_closure;
    const char *closure_ctx_type_name;
    zir_capture_list context_captures;
} zir_function;

typedef struct zir_module {
    const char *name;
    const zir_module_var *module_vars;
    size_t module_var_count;
    const zir_function *functions;
    size_t function_count;
    zir_span span;
    const zir_struct_decl *structs;
    size_t struct_count;
    const zir_enum_decl *enums;
    size_t enum_count;
} zir_module;

zir_span zir_make_span(const char *source_name, size_t line, size_t column);
int zir_span_is_known(zir_span span);

const char *zir_expr_kind_name(zir_expr_kind kind);

zir_expr_list zir_expr_list_make(void);
void zir_expr_list_push(zir_expr_list *list, zir_expr *expr);
void zir_expr_list_dispose(zir_expr_list *list);

zir_named_expr_list zir_named_expr_list_make(void);
void zir_named_expr_list_push(zir_named_expr_list *list, zir_named_expr entry);
void zir_named_expr_list_dispose(zir_named_expr_list *list);

zir_map_entry_list zir_map_entry_list_make(void);
void zir_map_entry_list_push(zir_map_entry_list *list, zir_map_entry entry);
void zir_map_entry_list_dispose(zir_map_entry_list *list);

zir_capture_list zir_capture_list_make(void);
void zir_capture_list_push(zir_capture_list *list, zir_capture capture);
void zir_capture_list_dispose(zir_capture_list *list);

zir_expr *zir_expr_make_name(const char *text);
zir_expr *zir_expr_make_int(const char *text);
zir_expr *zir_expr_make_float(const char *text);
zir_expr *zir_expr_make_bool(int value);
zir_expr *zir_expr_make_string(const char *text);
zir_expr *zir_expr_make_bytes(const char *hex_text);
zir_expr *zir_expr_make_copy(zir_expr *value);
zir_expr *zir_expr_make_unary(const char *op_name, zir_expr *operand);
zir_expr *zir_expr_make_binary(const char *op_name, zir_expr *left, zir_expr *right);
zir_expr *zir_expr_make_call_direct(const char *callee_name);
zir_expr *zir_expr_make_call_extern(const char *callee_name);
zir_expr *zir_expr_make_call_dyn(zir_expr *receiver, const char *method_name, const char *trait_name);
zir_expr *zir_expr_make_call_runtime_intrinsic(const char *callee_name);
zir_expr *zir_expr_make_make_struct(const char *type_name);
zir_expr *zir_expr_make_make_list(const char *item_type_name);
zir_expr *zir_expr_make_make_map(const char *key_type_name, const char *value_type_name);
zir_expr *zir_expr_make_make_set(const char *elem_type_name);
zir_expr *zir_expr_make_get_field(zir_expr *object, const char *field_name);
zir_expr *zir_expr_make_set_field(zir_expr *object, const char *field_name, zir_expr *value);
zir_expr *zir_expr_make_index_seq(zir_expr *sequence, zir_expr *index);
zir_expr *zir_expr_make_slice_seq(zir_expr *sequence, zir_expr *start, zir_expr *end);
zir_expr *zir_expr_make_list_len(zir_expr *sequence);
zir_expr *zir_expr_make_map_len(zir_expr *map);
zir_expr *zir_expr_make_set_len(zir_expr *set);
zir_expr *zir_expr_make_list_push(zir_expr *target, zir_expr *value);
zir_expr *zir_expr_make_list_set(zir_expr *target, zir_expr *index, zir_expr *value);
zir_expr *zir_expr_make_map_set(zir_expr *target, zir_expr *key, zir_expr *value);
zir_expr *zir_expr_make_set_add(zir_expr *target, zir_expr *value);
zir_expr *zir_expr_make_set_remove(zir_expr *target, zir_expr *value);
zir_expr *zir_expr_make_set_has(zir_expr *target, zir_expr *value);
zir_expr *zir_expr_make_optional_present(zir_expr *value);
zir_expr *zir_expr_make_optional_empty(const char *type_name);
zir_expr *zir_expr_make_optional_is_present(zir_expr *value);
zir_expr *zir_expr_make_coalesce(zir_expr *value, zir_expr *fallback);
zir_expr *zir_expr_make_outcome_success(zir_expr *value);
zir_expr *zir_expr_make_outcome_failure(zir_expr *value);
zir_expr *zir_expr_make_outcome_is_success(zir_expr *value);
zir_expr *zir_expr_make_outcome_value(zir_expr *value);
zir_expr *zir_expr_make_try_propagate(zir_expr *value);
zir_expr *zir_expr_make_outcome_wrap_context(zir_expr *value, zir_expr *context);
zir_expr *zir_expr_make_optional_value(zir_expr *value);
zir_expr *zir_expr_make_func_ref(const char *func_name, const char *callable_type_name);
zir_expr *zir_expr_make_call_indirect(zir_expr *callable);
zir_expr *zir_expr_make_if(zir_expr *condition, zir_expr *then_expr, zir_expr *else_expr);
zir_expr *zir_expr_make_make_closure(const char *func_name);
void zir_expr_call_add_arg(zir_expr *expr, zir_expr *arg);
void zir_expr_make_struct_add_field(zir_expr *expr, const char *name, zir_expr *value);
void zir_expr_make_list_add_item(zir_expr *expr, zir_expr *item);
void zir_expr_make_map_add_entry(zir_expr *expr, zir_expr *key, zir_expr *value);
void zir_expr_make_set_add_item(zir_expr *expr, zir_expr *item);
void zir_expr_dispose(zir_expr *expr);
char *zir_expr_render_alloc(const zir_expr *expr);

zir_param zir_make_param(const char *name, const char *type_name, zir_expr *where_clause);
zir_field_decl zir_make_field_decl(const char *name, const char *type_name, zir_expr *where_clause);
zir_instruction zir_make_assign_instruction(const char *dest_name, const char *type_name, const char *expr_text);
zir_instruction zir_make_assign_instruction_expr(const char *dest_name, const char *type_name, zir_expr *expr);
zir_instruction zir_make_effect_instruction(const char *expr_text);
zir_instruction zir_make_effect_instruction_expr(zir_expr *expr);
zir_instruction zir_make_check_contract_instruction_expr(zir_expr *expr);
zir_terminator zir_make_return_terminator(const char *value_text);
zir_terminator zir_make_return_terminator_expr(zir_expr *value);
zir_terminator zir_make_jump_terminator(const char *target_label);
zir_terminator zir_make_branch_if_terminator(const char *condition_text, const char *then_label, const char *else_label);
zir_terminator zir_make_branch_if_terminator_expr(zir_expr *condition, const char *then_label, const char *else_label);
zir_terminator zir_make_panic_terminator(const char *message_text);
zir_terminator zir_make_panic_terminator_expr(zir_expr *message);
zir_terminator zir_make_unreachable_terminator(void);
zir_block zir_make_block(const char *label, const zir_instruction *instructions, size_t instruction_count, zir_terminator terminator);
zir_struct_decl zir_make_struct_decl(const char *name, const zir_field_decl *fields, size_t field_count);
zir_enum_variant_field_decl zir_make_enum_variant_field_decl(const char *name, const char *type_name);
zir_enum_variant_decl zir_make_enum_variant_decl(const char *name, const zir_enum_variant_field_decl *fields, size_t field_count);
zir_enum_decl zir_make_enum_decl(const char *name, const zir_enum_variant_decl *variants, size_t variant_count);
zir_module_var zir_make_module_var(const char *name, const char *type_name, const char *init_expr_text);
zir_module_var zir_make_module_var_expr(const char *name, const char *type_name, zir_expr *init_expr);
zir_function zir_make_function(const char *name, const zir_param *params, size_t param_count, const char *return_type, const zir_block *blocks, size_t block_count);
zir_module zir_make_module(const char *name, const zir_function *functions, size_t function_count);
zir_module zir_make_module_with_structs(const char *name, const zir_struct_decl *structs, size_t struct_count, const zir_function *functions, size_t function_count);
zir_module zir_make_module_with_decls(const char *name, const zir_struct_decl *structs, size_t struct_count, const zir_enum_decl *enums, size_t enum_count, const zir_function *functions, size_t function_count);
zir_module zir_make_module_with_decls_and_vars(
        const char *name,
        const zir_struct_decl *structs,
        size_t struct_count,
        const zir_enum_decl *enums,
        size_t enum_count,
        const zir_module_var *module_vars,
        size_t module_var_count,
        const zir_function *functions,
        size_t function_count);

void zir_instruction_dispose_owned(zir_instruction *instruction);
void zir_terminator_dispose_owned(zir_terminator *terminator);
void zir_block_dispose_owned(zir_block *block);
void zir_struct_decl_dispose_owned(zir_struct_decl *struct_decl);
void zir_enum_decl_dispose_owned(zir_enum_decl *enum_decl);
void zir_module_var_dispose_owned(zir_module_var *module_var);
void zir_function_dispose_owned(zir_function *function_decl);
void zir_module_dispose_owned(zir_module *module_decl);

#ifdef __cplusplus
}
#endif

#endif
