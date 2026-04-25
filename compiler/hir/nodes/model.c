#include "compiler/hir/nodes/model.h"

#include <stdlib.h>
#include <string.h>

const char *zt_hir_decl_kind_name(zt_hir_decl_kind kind) {
    switch (kind) {
        case ZT_HIR_STRUCT_DECL: return "struct_decl";
        case ZT_HIR_ENUM_DECL: return "enum_decl";
        case ZT_HIR_FUNC_DECL: return "func_decl";
        default: return "unknown";
    }
}

const char *zt_hir_stmt_kind_name(zt_hir_stmt_kind kind) {
    switch (kind) {
        case ZT_HIR_BLOCK_STMT: return "block_stmt";
        case ZT_HIR_IF_STMT: return "if_stmt";
        case ZT_HIR_WHILE_STMT: return "while_stmt";
        case ZT_HIR_FOR_STMT: return "for_stmt";
        case ZT_HIR_REPEAT_STMT: return "repeat_stmt";
        case ZT_HIR_RETURN_STMT: return "return_stmt";
        case ZT_HIR_CONST_STMT: return "const_stmt";
        case ZT_HIR_VAR_STMT: return "var_stmt";
        case ZT_HIR_ASSIGN_STMT: return "assign_stmt";
        case ZT_HIR_INDEX_ASSIGN_STMT: return "index_assign_stmt";
        case ZT_HIR_FIELD_ASSIGN_STMT: return "field_assign_stmt";
        case ZT_HIR_MATCH_STMT: return "match_stmt";
        case ZT_HIR_BREAK_STMT: return "break_stmt";
        case ZT_HIR_CONTINUE_STMT: return "continue_stmt";
        case ZT_HIR_EXPR_STMT: return "expr_stmt";
        default: return "unknown";
    }
}

const char *zt_hir_expr_kind_name(zt_hir_expr_kind kind) {
    switch (kind) {
        case ZT_HIR_IDENT_EXPR: return "ident_expr";
        case ZT_HIR_INT_EXPR: return "int_expr";
        case ZT_HIR_FLOAT_EXPR: return "float_expr";
        case ZT_HIR_STRING_EXPR: return "string_expr";
        case ZT_HIR_BYTES_EXPR: return "bytes_expr";
        case ZT_HIR_BOOL_EXPR: return "bool_expr";
        case ZT_HIR_NONE_EXPR: return "none_expr";
        case ZT_HIR_SUCCESS_EXPR: return "success_expr";
        case ZT_HIR_ERROR_EXPR: return "error_expr";
        case ZT_HIR_LIST_EXPR: return "list_expr";
        case ZT_HIR_MAP_EXPR: return "map_expr";
        case ZT_HIR_UNARY_EXPR: return "unary_expr";
        case ZT_HIR_BINARY_EXPR: return "binary_expr";
        case ZT_HIR_FIELD_EXPR: return "field_expr";
        case ZT_HIR_INDEX_EXPR: return "index_expr";
        case ZT_HIR_SLICE_EXPR: return "slice_expr";
        case ZT_HIR_CALL_EXPR: return "call_expr";
        case ZT_HIR_METHOD_CALL_EXPR: return "method_call_expr";
        case ZT_HIR_DYN_METHOD_CALL_EXPR: return "dyn_method_call_expr";
        case ZT_HIR_FUNC_REF_EXPR: return "func_ref_expr";
        case ZT_HIR_CALL_INDIRECT_EXPR: return "call_indirect_expr";
        case ZT_HIR_CONSTRUCT_EXPR: return "construct_expr";
        case ZT_HIR_VALUE_BINDING_EXPR: return "value_binding_expr";
        case ZT_HIR_CLOSURE_EXPR: return "closure_expr";
        default: return "unknown";
    }
}

static void *zt_hir_realloc_array(void *items, size_t *capacity, size_t item_size) {
    size_t new_capacity;
    void *new_items;
    if (capacity == NULL) return NULL;
    new_capacity = *capacity == 0 ? 4 : (*capacity * 2);
    new_items = realloc(items, new_capacity * item_size);
    if (new_items != NULL) *capacity = new_capacity;
    return new_items;
}

zt_hir_module *zt_hir_module_make(zt_source_span span) {
    zt_hir_module *module = (zt_hir_module *)calloc(1, sizeof(zt_hir_module));
    if (module == NULL) return NULL;
    module->span = span;
    module->module_vars = zt_hir_module_var_list_make();
    module->declarations = zt_hir_decl_list_make();
    return module;
}

zt_hir_decl *zt_hir_decl_make(zt_hir_decl_kind kind, zt_source_span span) {
    zt_hir_decl *decl = (zt_hir_decl *)calloc(1, sizeof(zt_hir_decl));
    if (decl == NULL) return NULL;
    decl->kind = kind;
    decl->span = span;
    return decl;
}

zt_hir_stmt *zt_hir_stmt_make(zt_hir_stmt_kind kind, zt_source_span span) {
    zt_hir_stmt *stmt = (zt_hir_stmt *)calloc(1, sizeof(zt_hir_stmt));
    if (stmt == NULL) return NULL;
    stmt->kind = kind;
    stmt->span = span;
    return stmt;
}

zt_hir_expr *zt_hir_expr_make(zt_hir_expr_kind kind, zt_source_span span) {
    zt_hir_expr *expr = (zt_hir_expr *)calloc(1, sizeof(zt_hir_expr));
    if (expr == NULL) return NULL;
    expr->kind = kind;
    expr->span = span;
    expr->type = zt_type_make(ZT_TYPE_UNKNOWN);
    return expr;
}

zt_hir_string_list zt_hir_string_list_make(void) { zt_hir_string_list list = {0}; return list; }
zt_hir_decl_list zt_hir_decl_list_make(void) { zt_hir_decl_list list = {0}; return list; }
zt_hir_stmt_list zt_hir_stmt_list_make(void) { zt_hir_stmt_list list = {0}; return list; }
zt_hir_expr_list zt_hir_expr_list_make(void) { zt_hir_expr_list list = {0}; return list; }
zt_hir_param_list zt_hir_param_list_make(void) { zt_hir_param_list list = {0}; return list; }
zt_hir_field_decl_list zt_hir_field_decl_list_make(void) { zt_hir_field_decl_list list = {0}; return list; }
zt_hir_enum_variant_field_list zt_hir_enum_variant_field_list_make(void) { zt_hir_enum_variant_field_list list = {0}; return list; }
zt_hir_enum_variant_list zt_hir_enum_variant_list_make(void) { zt_hir_enum_variant_list list = {0}; return list; }
zt_hir_field_init_list zt_hir_field_init_list_make(void) { zt_hir_field_init_list list = {0}; return list; }
zt_hir_map_entry_list zt_hir_map_entry_list_make(void) { zt_hir_map_entry_list list = {0}; return list; }
zt_hir_match_case_list zt_hir_match_case_list_make(void) { zt_hir_match_case_list list = {0}; return list; }
zt_hir_module_var_list zt_hir_module_var_list_make(void) { zt_hir_module_var_list list = {0}; return list; }
zt_hir_capture_list zt_hir_capture_list_make(void) { zt_hir_capture_list list = {0}; return list; }

void zt_hir_string_list_push(zt_hir_string_list *list, char *value) {
    char **new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (char **)zt_hir_realloc_array(list->items, &list->capacity, sizeof(char *));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = value;
}

void zt_hir_decl_list_push(zt_hir_decl_list *list, zt_hir_decl *decl) {
    zt_hir_decl **new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (zt_hir_decl **)zt_hir_realloc_array(list->items, &list->capacity, sizeof(zt_hir_decl *));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = decl;
}

void zt_hir_stmt_list_push(zt_hir_stmt_list *list, zt_hir_stmt *stmt) {
    zt_hir_stmt **new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (zt_hir_stmt **)zt_hir_realloc_array(list->items, &list->capacity, sizeof(zt_hir_stmt *));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = stmt;
}

void zt_hir_expr_list_push(zt_hir_expr_list *list, zt_hir_expr *expr) {
    zt_hir_expr **new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (zt_hir_expr **)zt_hir_realloc_array(list->items, &list->capacity, sizeof(zt_hir_expr *));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = expr;
}

void zt_hir_param_list_push(zt_hir_param_list *list, zt_hir_param param) {
    zt_hir_param *new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (zt_hir_param *)zt_hir_realloc_array(list->items, &list->capacity, sizeof(zt_hir_param));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = param;
}

void zt_hir_field_decl_list_push(zt_hir_field_decl_list *list, zt_hir_field_decl field) {
    zt_hir_field_decl *new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (zt_hir_field_decl *)zt_hir_realloc_array(list->items, &list->capacity, sizeof(zt_hir_field_decl));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = field;
}

void zt_hir_enum_variant_field_list_push(zt_hir_enum_variant_field_list *list, zt_hir_enum_variant_field field) {
    zt_hir_enum_variant_field *new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (zt_hir_enum_variant_field *)zt_hir_realloc_array(
            list->items,
            &list->capacity,
            sizeof(zt_hir_enum_variant_field));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = field;
}

void zt_hir_enum_variant_list_push(zt_hir_enum_variant_list *list, zt_hir_enum_variant variant) {
    zt_hir_enum_variant *new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (zt_hir_enum_variant *)zt_hir_realloc_array(
            list->items,
            &list->capacity,
            sizeof(zt_hir_enum_variant));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = variant;
}

void zt_hir_field_init_list_push(zt_hir_field_init_list *list, zt_hir_field_init field) {
    zt_hir_field_init *new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (zt_hir_field_init *)zt_hir_realloc_array(list->items, &list->capacity, sizeof(zt_hir_field_init));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = field;
}

void zt_hir_map_entry_list_push(zt_hir_map_entry_list *list, zt_hir_map_entry entry) {
    zt_hir_map_entry *new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (zt_hir_map_entry *)zt_hir_realloc_array(list->items, &list->capacity, sizeof(zt_hir_map_entry));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = entry;
}

void zt_hir_match_case_list_push(zt_hir_match_case_list *list, zt_hir_match_case match_case) {
    zt_hir_match_case *new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (zt_hir_match_case *)zt_hir_realloc_array(list->items, &list->capacity, sizeof(zt_hir_match_case));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = match_case;
}


void zt_hir_capture_list_push(zt_hir_capture_list *list, zt_hir_capture capture) {
    zt_hir_capture *new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (zt_hir_capture *)zt_hir_realloc_array(list->items, &list->capacity, sizeof(zt_hir_capture));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = capture;
}

void zt_hir_module_var_list_push(zt_hir_module_var_list *list, zt_hir_module_var module_var) {
    zt_hir_module_var *new_items;
    if (list == NULL) return;
    if (list->count >= list->capacity) {
        new_items = (zt_hir_module_var *)zt_hir_realloc_array(list->items, &list->capacity, sizeof(zt_hir_module_var));
        if (new_items == NULL) return;
        list->items = new_items;
    }
    list->items[list->count++] = module_var;
}

void zt_hir_string_list_dispose(zt_hir_string_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) free(list->items[i]);
    free(list->items);
    memset(list, 0, sizeof(*list));
}

void zt_hir_expr_dispose(zt_hir_expr *expr) {
    if (expr == NULL) return;
    zt_type_dispose(expr->type);
    switch (expr->kind) {
        case ZT_HIR_IDENT_EXPR: free(expr->as.ident_expr.name); break;
        case ZT_HIR_INT_EXPR: free(expr->as.int_expr.value); break;
        case ZT_HIR_FLOAT_EXPR: free(expr->as.float_expr.value); break;
        case ZT_HIR_STRING_EXPR: free(expr->as.string_expr.value); break;
        case ZT_HIR_BYTES_EXPR: free(expr->as.bytes_expr.value); break;
        case ZT_HIR_SUCCESS_EXPR: zt_hir_expr_dispose(expr->as.success_expr.value); break;
        case ZT_HIR_ERROR_EXPR: zt_hir_expr_dispose(expr->as.error_expr.value); break;
        case ZT_HIR_LIST_EXPR: zt_hir_expr_list_dispose(&expr->as.list_expr.elements); break;
        case ZT_HIR_MAP_EXPR: zt_hir_map_entry_list_dispose(&expr->as.map_expr.entries); break;
        case ZT_HIR_UNARY_EXPR: zt_hir_expr_dispose(expr->as.unary_expr.operand); break;
        case ZT_HIR_BINARY_EXPR:
            zt_hir_expr_dispose(expr->as.binary_expr.left);
            zt_hir_expr_dispose(expr->as.binary_expr.right);
            break;
        case ZT_HIR_FIELD_EXPR:
            zt_hir_expr_dispose(expr->as.field_expr.object);
            free(expr->as.field_expr.field_name);
            break;
        case ZT_HIR_INDEX_EXPR:
            zt_hir_expr_dispose(expr->as.index_expr.object);
            zt_hir_expr_dispose(expr->as.index_expr.index);
            break;
        case ZT_HIR_SLICE_EXPR:
            zt_hir_expr_dispose(expr->as.slice_expr.object);
            zt_hir_expr_dispose(expr->as.slice_expr.start);
            zt_hir_expr_dispose(expr->as.slice_expr.end);
            break;
        case ZT_HIR_CALL_EXPR:
            free(expr->as.call_expr.callee_name);
            zt_hir_expr_list_dispose(&expr->as.call_expr.args);
            break;
        case ZT_HIR_METHOD_CALL_EXPR:
            zt_hir_expr_dispose(expr->as.method_call_expr.receiver);
            free(expr->as.method_call_expr.method_name);
            zt_hir_expr_list_dispose(&expr->as.method_call_expr.args);
            break;
        case ZT_HIR_DYN_METHOD_CALL_EXPR:
            zt_hir_expr_dispose(expr->as.dyn_method_call_expr.receiver);
            free(expr->as.dyn_method_call_expr.method_name);
            free(expr->as.dyn_method_call_expr.trait_name);
            zt_hir_expr_list_dispose(&expr->as.dyn_method_call_expr.args);
            break;
        case ZT_HIR_FUNC_REF_EXPR:
            free(expr->as.func_ref_expr.func_name);
            zt_type_dispose(expr->as.func_ref_expr.callable_type);
            break;
        case ZT_HIR_CALL_INDIRECT_EXPR:
            zt_hir_expr_dispose(expr->as.call_indirect_expr.callable);
            zt_hir_expr_list_dispose(&expr->as.call_indirect_expr.args);
            break;
        case ZT_HIR_CONSTRUCT_EXPR:
            free(expr->as.construct_expr.type_name);
            zt_hir_field_init_list_dispose(&expr->as.construct_expr.fields);
            break;
        case ZT_HIR_VALUE_BINDING_EXPR:
            free(expr->as.value_binding_expr.name);
            break;
        case ZT_HIR_CLOSURE_EXPR:
            zt_hir_param_list_dispose(&expr->as.closure_expr.params);
            zt_type_dispose(expr->as.closure_expr.return_type);
            zt_hir_stmt_dispose(expr->as.closure_expr.body);
            zt_hir_capture_list_dispose(&expr->as.closure_expr.captures);
            break;
        default: break;
    }
    free(expr);
}

void zt_hir_expr_list_dispose(zt_hir_expr_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) zt_hir_expr_dispose(list->items[i]);
    free(list->items);
    memset(list, 0, sizeof(*list));
}

void zt_hir_map_entry_list_dispose(zt_hir_map_entry_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) {
        zt_hir_expr_dispose(list->items[i].key);
        zt_hir_expr_dispose(list->items[i].value);
    }
    free(list->items);
    memset(list, 0, sizeof(*list));
}

void zt_hir_field_init_list_dispose(zt_hir_field_init_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) {
        free(list->items[i].name);
        zt_hir_expr_dispose(list->items[i].value);
    }
    free(list->items);
    memset(list, 0, sizeof(*list));
}

void zt_hir_stmt_dispose(zt_hir_stmt *stmt) {
    if (stmt == NULL) return;
    switch (stmt->kind) {
        case ZT_HIR_BLOCK_STMT: zt_hir_stmt_list_dispose(&stmt->as.block_stmt.statements); break;
        case ZT_HIR_IF_STMT:
            zt_hir_expr_dispose(stmt->as.if_stmt.condition);
            zt_hir_stmt_dispose(stmt->as.if_stmt.then_block);
            zt_hir_stmt_dispose(stmt->as.if_stmt.else_block);
            break;
        case ZT_HIR_WHILE_STMT:
            zt_hir_expr_dispose(stmt->as.while_stmt.condition);
            zt_hir_stmt_dispose(stmt->as.while_stmt.body);
            break;
        case ZT_HIR_FOR_STMT:
            free(stmt->as.for_stmt.item_name);
            free(stmt->as.for_stmt.index_name);
            zt_hir_expr_dispose(stmt->as.for_stmt.iterable);
            zt_hir_stmt_dispose(stmt->as.for_stmt.body);
            break;
        case ZT_HIR_REPEAT_STMT:
            zt_hir_expr_dispose(stmt->as.repeat_stmt.count);
            zt_hir_stmt_dispose(stmt->as.repeat_stmt.body);
            break;
        case ZT_HIR_RETURN_STMT: zt_hir_expr_dispose(stmt->as.return_stmt.value); break;
        case ZT_HIR_CONST_STMT:
            free(stmt->as.const_stmt.name);
            zt_type_dispose(stmt->as.const_stmt.type);
            zt_hir_expr_dispose(stmt->as.const_stmt.init_value);
            break;
        case ZT_HIR_VAR_STMT:
            free(stmt->as.var_stmt.name);
            zt_type_dispose(stmt->as.var_stmt.type);
            zt_hir_expr_dispose(stmt->as.var_stmt.init_value);
            break;
        case ZT_HIR_ASSIGN_STMT:
            free(stmt->as.assign_stmt.name);
            zt_hir_expr_dispose(stmt->as.assign_stmt.value);
            break;
        case ZT_HIR_INDEX_ASSIGN_STMT:
            zt_hir_expr_dispose(stmt->as.index_assign_stmt.object);
            zt_hir_expr_dispose(stmt->as.index_assign_stmt.index);
            zt_hir_expr_dispose(stmt->as.index_assign_stmt.value);
            break;
        case ZT_HIR_FIELD_ASSIGN_STMT:
            zt_hir_expr_dispose(stmt->as.field_assign_stmt.object);
            free(stmt->as.field_assign_stmt.field_name);
            zt_hir_expr_dispose(stmt->as.field_assign_stmt.value);
            break;
        case ZT_HIR_MATCH_STMT:
            zt_hir_expr_dispose(stmt->as.match_stmt.subject);
            zt_hir_match_case_list_dispose(&stmt->as.match_stmt.cases);
            break;
        case ZT_HIR_EXPR_STMT: zt_hir_expr_dispose(stmt->as.expr_stmt.expr); break;
        default: break;
    }
    free(stmt);
}

void zt_hir_stmt_list_dispose(zt_hir_stmt_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) zt_hir_stmt_dispose(list->items[i]);
    free(list->items);
    memset(list, 0, sizeof(*list));
}

void zt_hir_param_list_dispose(zt_hir_param_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) {
        free(list->items[i].name);
        zt_type_dispose(list->items[i].type);
        zt_hir_expr_dispose(list->items[i].where_clause);
    }
    free(list->items);
    memset(list, 0, sizeof(*list));
}

void zt_hir_field_decl_list_dispose(zt_hir_field_decl_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) {
        free(list->items[i].name);
        zt_type_dispose(list->items[i].type);
        zt_hir_expr_dispose(list->items[i].default_value);
        zt_hir_expr_dispose(list->items[i].where_clause);
    }
    free(list->items);
    memset(list, 0, sizeof(*list));
}

void zt_hir_enum_variant_field_list_dispose(zt_hir_enum_variant_field_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) {
        free(list->items[i].name);
        zt_type_dispose(list->items[i].type);
    }
    free(list->items);
    memset(list, 0, sizeof(*list));
}

void zt_hir_enum_variant_list_dispose(zt_hir_enum_variant_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) {
        free(list->items[i].name);
        zt_hir_enum_variant_field_list_dispose(&list->items[i].fields);
    }
    free(list->items);
    memset(list, 0, sizeof(*list));
}

void zt_hir_match_case_list_dispose(zt_hir_match_case_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) {
        zt_hir_expr_list_dispose(&list->items[i].patterns);
        zt_hir_stmt_dispose(list->items[i].body);
    }
    free(list->items);
    memset(list, 0, sizeof(*list));
}


void zt_hir_capture_list_dispose(zt_hir_capture_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) {
        free(list->items[i].name);
        zt_type_dispose(list->items[i].type);
    }
    free(list->items);
    memset(list, 0, sizeof(*list));
}

void zt_hir_module_var_list_dispose(zt_hir_module_var_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) {
        free(list->items[i].name);
        zt_type_dispose(list->items[i].type);
        zt_hir_expr_dispose(list->items[i].init_value);
    }
    free(list->items);
    memset(list, 0, sizeof(*list));
}

void zt_hir_decl_dispose(zt_hir_decl *decl) {
    if (decl == NULL) return;
    switch (decl->kind) {
        case ZT_HIR_STRUCT_DECL:
            free(decl->as.struct_decl.name);
            zt_hir_string_list_dispose(&decl->as.struct_decl.type_params);
            zt_hir_field_decl_list_dispose(&decl->as.struct_decl.fields);
            break;
        case ZT_HIR_ENUM_DECL:
            free(decl->as.enum_decl.name);
            zt_hir_string_list_dispose(&decl->as.enum_decl.type_params);
            zt_hir_enum_variant_list_dispose(&decl->as.enum_decl.variants);
            break;
        case ZT_HIR_FUNC_DECL:
            free(decl->as.func_decl.name);
            free(decl->as.func_decl.receiver_type_name);
            free(decl->as.func_decl.implemented_trait_name);
            zt_hir_string_list_dispose(&decl->as.func_decl.type_params);
            zt_hir_param_list_dispose(&decl->as.func_decl.params);
            zt_type_dispose(decl->as.func_decl.return_type);
            zt_hir_stmt_dispose(decl->as.func_decl.body);
            break;
        default: break;
    }
    free(decl);
}

void zt_hir_decl_list_dispose(zt_hir_decl_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i++) zt_hir_decl_dispose(list->items[i]);
    free(list->items);
    memset(list, 0, sizeof(*list));
}

void zt_hir_module_dispose(zt_hir_module *module) {
    if (module == NULL) return;
    free(module->module_name);
    zt_hir_module_var_list_dispose(&module->module_vars);
    zt_hir_decl_list_dispose(&module->declarations);
    free(module);
}

