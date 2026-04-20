#ifndef ZENITH_NEXT_COMPILER_SEMANTIC_TYPES_TYPES_H
#define ZENITH_NEXT_COMPILER_SEMANTIC_TYPES_TYPES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum zt_type_kind {
    ZT_TYPE_UNKNOWN,
    ZT_TYPE_NONE_LITERAL,
    ZT_TYPE_SUCCESS_WRAPPER,
    ZT_TYPE_ERROR_WRAPPER,
    ZT_TYPE_BOOL,
    ZT_TYPE_INT,
    ZT_TYPE_INT8,
    ZT_TYPE_INT16,
    ZT_TYPE_INT32,
    ZT_TYPE_INT64,
    ZT_TYPE_UINT8,
    ZT_TYPE_UINT16,
    ZT_TYPE_UINT32,
    ZT_TYPE_UINT64,
    ZT_TYPE_FLOAT,
    ZT_TYPE_FLOAT32,
    ZT_TYPE_FLOAT64,
    ZT_TYPE_TEXT,
    ZT_TYPE_CORE_ERROR,
    ZT_TYPE_BYTES,
    ZT_TYPE_VOID,
    ZT_TYPE_USER,
    ZT_TYPE_TYPE_PARAM,
    ZT_TYPE_OPTIONAL,
    ZT_TYPE_RESULT,
    ZT_TYPE_LIST,
    ZT_TYPE_MAP,
    ZT_TYPE_GRID2D,
    ZT_TYPE_PQUEUE,
    ZT_TYPE_CIRCBUF,
    ZT_TYPE_BTREEMAP,
    ZT_TYPE_BTREESET,
    ZT_TYPE_GRID3D,
} zt_type_kind;

typedef struct zt_type zt_type;

typedef struct zt_type_list {
    zt_type **items;
    size_t count;
    size_t capacity;
} zt_type_list;

struct zt_type {
    zt_type_kind kind;
    char *name;
    zt_type_list args;
};

const char *zt_type_kind_name(zt_type_kind kind);

zt_type *zt_type_make(zt_type_kind kind);
zt_type *zt_type_make_named(zt_type_kind kind, const char *name);
zt_type *zt_type_make_with_args(zt_type_kind kind, const char *name, zt_type_list args);
zt_type *zt_type_clone(const zt_type *type);
void zt_type_dispose(zt_type *type);

zt_type_list zt_type_list_make(void);
void zt_type_list_push(zt_type_list *list, zt_type *type);
void zt_type_list_dispose(zt_type_list *list);

int zt_type_equals(const zt_type *left, const zt_type *right);
void zt_type_format(const zt_type *type, char *buffer, size_t buffer_size);

int zt_type_is_integral(const zt_type *type);
int zt_type_is_signed_integral(const zt_type *type);
int zt_type_is_unsigned_integral(const zt_type *type);
int zt_type_is_float(const zt_type *type);
int zt_type_is_numeric(const zt_type *type);

#ifdef __cplusplus
}
#endif

#endif
