#ifndef ZT_ARENA_H
#define ZT_ARENA_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct zt_arena_chunk {
    size_t size;
    size_t used;
    struct zt_arena_chunk *next;
    uint8_t data[];
} zt_arena_chunk;

typedef struct zt_arena {
    zt_arena_chunk *first;
    zt_arena_chunk *current;
    size_t default_chunk_size;
} zt_arena;

/**
 * Initializes a new arena.
 * @param arena The arena to initialize.
 * @param default_chunk_size The default allocation chunk size (usually 64KB or 1MB).
 */
void zt_arena_init(zt_arena *arena, size_t default_chunk_size);

/**
 * Disposes all memory chunks associated with the arena.
 * @param arena The arena to dispose.
 */
void zt_arena_dispose(zt_arena *arena);

/**
 * Allocates a block of memory from the arena.
 * @param arena The arena to allocate from.
 * @param size The amount of bytes to allocate.
 * @return A pointer to the newly allocated block, or NULL on failure.
 */
void *zt_arena_alloc(zt_arena *arena, size_t size);

/**
 * Allocates a string from the arena and copies the given text.
 * @param arena The arena to allocate from.
 * @param text The string to duplicate.
 * @return A pointer to the duplicated string, or NULL on failure.
 */
char *zt_arena_strdup(zt_arena *arena, const char *text);

#ifdef __cplusplus
}
#endif

#endif /* ZT_ARENA_H */
