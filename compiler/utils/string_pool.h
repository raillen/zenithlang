#ifndef ZT_STRING_POOL_H
#define ZT_STRING_POOL_H

#include "compiler/utils/arena.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct zt_string_entry {
    const char *text;
    size_t length;
    uint32_t hash;
    struct zt_string_entry *next;
} zt_string_entry;

typedef struct zt_string_pool {
    zt_string_entry **buckets;
    size_t capacity;
    size_t count;
    zt_arena *arena;
} zt_string_pool;

/**
 * Initializes a new string pool.
 * @param pool The pool to initialize.
 * @param arena The arena to allocate string copies and string entries from.
 */
void zt_string_pool_init(zt_string_pool *pool, zt_arena *arena);

/**
 * Interns a null-terminated string.
 * @param pool The pool to intern the string into.
 * @param text The string to intern.
 * @return A pointer to the interned string, guaranteed to be stable and single-instance. NULL on allocation failure.
 */
const char *zt_string_pool_intern(zt_string_pool *pool, const char *text);

/**
 * Interns a string with a specific length.
 * @param pool The pool to intern the string into.
 * @param text The string to intern.
 * @param length The length of the string to intern.
 * @return A pointer to the interned null-terminated string, guaranteed to be stable and single-instance. NULL on allocation failure.
 */
const char *zt_string_pool_intern_len(zt_string_pool *pool, const char *text, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* ZT_STRING_POOL_H */
