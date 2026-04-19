#include "compiler/utils/arena.h"
#include <stdlib.h>
#include <string.h>

void zt_arena_init(zt_arena *arena, size_t default_chunk_size) {
    if (arena == NULL) return;
    arena->first = NULL;
    arena->current = NULL;
    arena->default_chunk_size = default_chunk_size > 0 ? default_chunk_size : (64 * 1024);
}

void zt_arena_dispose(zt_arena *arena) {
    zt_arena_chunk *chunk;
    zt_arena_chunk *next;

    if (arena == NULL) return;

    chunk = arena->first;
    while (chunk != NULL) {
        next = chunk->next;
        free(chunk);
        chunk = next;
    }

    arena->first = NULL;
    arena->current = NULL;
}

static zt_arena_chunk *zt_arena_alloc_chunk(size_t size) {
    zt_arena_chunk *chunk = (zt_arena_chunk *)malloc(sizeof(zt_arena_chunk) + size);
    if (chunk == NULL) return NULL;
    chunk->size = size;
    chunk->used = 0;
    chunk->next = NULL;
    return chunk;
}

void *zt_arena_alloc(zt_arena *arena, size_t size) {
    size_t alloc_size;
    uint8_t *ptr;
    zt_arena_chunk *chunk;

    if (arena == NULL || size == 0) return NULL;

    /* Align size to 8 bytes */
    if (size % 8 != 0) {
        size += 8 - (size % 8);
    }

    if (arena->current != NULL && arena->current->used + size <= arena->current->size) {
        ptr = arena->current->data + arena->current->used;
        arena->current->used += size;
        return ptr;
    }

    alloc_size = size > arena->default_chunk_size ? size : arena->default_chunk_size;
    chunk = zt_arena_alloc_chunk(alloc_size);
    if (chunk == NULL) return NULL;

    if (arena->current != NULL) {
        arena->current->next = chunk;
    } else {
        arena->first = chunk;
    }
    
    arena->current = chunk;
    ptr = chunk->data + chunk->used;
    chunk->used += size;
    return ptr;
}

char *zt_arena_strdup(zt_arena *arena, const char *text) {
    size_t len;
    char *copy;

    if (arena == NULL || text == NULL) return NULL;
    
    len = strlen(text);
    copy = (char *)zt_arena_alloc(arena, len + 1);
    if (copy == NULL) return NULL;
    
    memcpy(copy, text, len + 1);
    return copy;
}
