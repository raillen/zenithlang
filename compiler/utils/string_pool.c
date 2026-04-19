#include "compiler/utils/string_pool.h"
#include <string.h>

#define ZT_STRING_POOL_INITIAL_CAPACITY 1024

// FNV-1a hash function
static uint32_t zt_hash_string(const char *text, size_t length) {
    uint32_t hash = 2166136261u;
    size_t i;
    for (i = 0; i < length; i++) {
        hash ^= (uint8_t)text[i];
        hash *= 16777619u;
    }
    return hash;
}

void zt_string_pool_init(zt_string_pool *pool, zt_arena *arena) {
    if (pool == NULL || arena == NULL) return;
    pool->capacity = ZT_STRING_POOL_INITIAL_CAPACITY;
    pool->count = 0;
    pool->arena = arena;
    pool->buckets = (zt_string_entry **)zt_arena_alloc(arena, sizeof(zt_string_entry *) * pool->capacity);
    if (pool->buckets != NULL) {
        memset(pool->buckets, 0, sizeof(zt_string_entry *) * pool->capacity);
    }
}

static void zt_string_pool_grow(zt_string_pool *pool) {
    size_t new_capacity = pool->capacity * 2;
    zt_string_entry **new_buckets = (zt_string_entry **)zt_arena_alloc(pool->arena, sizeof(zt_string_entry *) * new_capacity);
    size_t i;

    if (new_buckets == NULL) return; /* Allocation failed, continue with old buckets (degraded performance) */
    memset(new_buckets, 0, sizeof(zt_string_entry *) * new_capacity);

    for (i = 0; i < pool->capacity; i++) {
        zt_string_entry *entry = pool->buckets[i];
        while (entry != NULL) {
            zt_string_entry *next = entry->next;
            size_t new_index = entry->hash & (new_capacity - 1);
            entry->next = new_buckets[new_index];
            new_buckets[new_index] = entry;
            entry = next;
        }
    }

    pool->buckets = new_buckets;
    pool->capacity = new_capacity;
}

const char *zt_string_pool_intern_len(zt_string_pool *pool, const char *text, size_t length) {
    uint32_t hash;
    size_t index;
    zt_string_entry *entry;
    char *copy;

    if (pool == NULL || pool->buckets == NULL || text == NULL) return NULL;

    hash = zt_hash_string(text, length);
    index = hash & (pool->capacity - 1);

    entry = pool->buckets[index];
    while (entry != NULL) {
        if (entry->hash == hash && entry->length == length && memcmp(entry->text, text, length) == 0) {
            return entry->text;
        }
        entry = entry->next;
    }

    if (pool->count >= pool->capacity * 0.75) {
        zt_string_pool_grow(pool);
        index = hash & (pool->capacity - 1); /* Recompute index */
    }

    /* Allocate and copy the string, adding null terminator */
    copy = (char *)zt_arena_alloc(pool->arena, length + 1);
    if (copy == NULL) return NULL;
    memcpy(copy, text, length);
    copy[length] = '\0';

    /* Create new entry */
    entry = (zt_string_entry *)zt_arena_alloc(pool->arena, sizeof(zt_string_entry));
    if (entry == NULL) return NULL;
    entry->text = copy;
    entry->length = length;
    entry->hash = hash;
    entry->next = pool->buckets[index];

    pool->buckets[index] = entry;
    pool->count++;

    return entry->text;
}

const char *zt_string_pool_intern(zt_string_pool *pool, const char *text) {
    if (text == NULL) return NULL;
    return zt_string_pool_intern_len(pool, text, strlen(text));
}
