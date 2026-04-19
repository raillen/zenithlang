import re

def process_c(c_path):
    with open(c_path, 'r') as f:
        content = f.read()

    # 1. Pointers to values
    structs = [
        'zt_optional_text',
        'zt_optional_list_i64',
        'zt_optional_list_text',
        'zt_optional_map_text_text',
        'zt_outcome_i64_text',
        'zt_outcome_void_text',
        'zt_outcome_text_text',
        'zt_outcome_list_i64_text',
        'zt_outcome_list_text_text',
        'zt_outcome_map_text_text'
    ]

    for s in structs:
        # replace return types in implementation
        content = re.sub(r'^' + s + r'\s*\*\s*(' + s + r'_[a-zA-Z0-9_]+)\(', s + r' \1(', content, flags=re.MULTILINE)
        content = re.sub(r'^static\s+' + s + r'\s*\*\s*(' + s + r'_[a-zA-Z0-9_]+|zt_host_[a-zA-Z0-9_]+)\(', r'static ' + s + r' \1(', content, flags=re.MULTILINE)
        content = re.sub(r'^' + s + r'\s*\*\s*(zt_host_[a-zA-Z0-9_]+)\(', s + r' \1(', content, flags=re.MULTILINE)

        # replace argument types
        content = re.sub(r'const\s+' + s + r'\s*\*', s + r' ', content)
        content = re.sub(r'\b' + s + r'\s*\*\s*([a-zA-Z0-9_]+)\s*([,)])', s + r' \1\2', content)

        # Remove `_new` and `_alloc` and `_free` functions completely
        content = re.sub(r'static ' + s + r' \*(?:' + s + r'_new|' + s + r'_alloc)\(void\)\s*\{[^}]+\}\n?', '', content)
        content = re.sub(r'static void zt_free_' + s[3:] + r'\([^)]+\)\s*\{[^}]+\}\n?', '', content)

        # Replace `*outcome` and `*optional` variable declarations
        content = re.sub(r'\b' + s + r'\s+\*(outcome|optional);', s + r' \1;', content)
        
        # Remove calls to `_new()`
        content = re.sub(r'\b(outcome|optional)\s*=\s*' + s + r'_new\(\);', '', content)

        # Change `->` to `.` for outcome and optional variables
        # This is a bit tricky, let's just do it directly for those identifiers
        content = re.sub(r'\b(outcome|optional|value|outcome_val)\->', r'\1.', content)

        # Remove `zt_runtime_require_` calls for value types
        content = re.sub(r'\s*zt_runtime_require_(optional_[a-z0-9_]+|outcome_[a-z0-9_]+)\([^;]+;\n', '\n', content)

    # Fix zt_host_default_read_file return type and internal outcome
    content = re.sub(r'zt_outcome_text_text\s+\*outcome;', r'zt_outcome_text_text outcome;', content)
    
    # 2. Add `_owned` push functions
    push_owned_i64 = """void zt_list_i64_push(zt_list_i64 *list, zt_int value) {"""
    push_owned_i64_new = """zt_list_i64 *zt_list_i64_push_owned(zt_list_i64 *list, zt_int value) {
    zt_runtime_require_list_i64(list, "zt_list_i64_push_owned requires list");
    if (list->header.rc > 1u) {
        zt_list_i64 *clone = zt_list_i64_from_array(list->data, list->len);
        zt_release(list);
        list = clone;
    }
    zt_list_i64_push(list, value);
    return list;
}

void zt_list_i64_push(zt_list_i64 *list, zt_int value) {"""
    content = content.replace(push_owned_i64, push_owned_i64_new)

    push_owned_text = """void zt_list_text_push(zt_list_text *list, zt_text *value) {"""
    push_owned_text_new = """zt_list_text *zt_list_text_push_owned(zt_list_text *list, zt_text *value) {
    zt_runtime_require_list_text(list, "zt_list_text_push_owned requires list");
    if (list->header.rc > 1u) {
        zt_list_text *clone = zt_list_text_from_array(list->data, list->len);
        zt_release(list);
        list = clone;
    }
    zt_list_text_push(list, value);
    return list;
}

void zt_list_text_push(zt_list_text *list, zt_text *value) {"""
    content = content.replace(push_owned_text, push_owned_text_new)

    # 3. Hash map text-text rewrite
    # Since map uses arrays, we change `zt_map_text_text_find_index`
    # and `zt_map_text_text_reserve` and `zt_map_text_text_new`
    # First, hash func:
    hash_func = """static uint32_t zt_hash_text(const zt_text *key) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < key->len; i++) {
        hash ^= (uint8_t)key->data[i];
        hash *= 16777619u;
    }
    return hash;
}
"""
    content = content.replace("zt_map_text_text *zt_map_text_text_new(void) {", hash_func + "\nzt_map_text_text *zt_map_text_text_new(void) {")

    # Rewrite zt_map_text_text_find_index
    old_find_index = re.search(r'size_t zt_map_text_text_find_index\(const zt_map_text_text \*map, const zt_text \*key, zt_bool \*found\) \{.*?\n\}', content, flags=re.DOTALL)
    if old_find_index:
        new_find_index = """size_t zt_map_text_text_find_index(const zt_map_text_text *map, const zt_text *key, zt_bool *found) {
    if (map->hash_capacity == 0) {
        *found = false;
        return 0;
    }
    uint32_t hash = zt_hash_text(key);
    size_t mask = map->hash_capacity - 1;
    size_t index = hash & mask;
    
    while (true) {
        size_t entry_idx = map->hash_indices[index];
        if (entry_idx == SIZE_MAX) {
            *found = false;
            return index; // Return the empty hash slot
        }
        if (zt_text_eq(map->keys[entry_idx], key)) {
            *found = true;
            return entry_idx; // Return the actual key/value index
        }
        index = (index + 1) & mask;
    }
}"""
        content = content.replace(old_find_index.group(0), new_find_index)

    # We also need to rewrite reserve/rehash
    old_reserve = re.search(r'static void zt_map_text_text_reserve\(zt_map_text_text \*map, size_t new_capacity\) \{.*?\n\}', content, flags=re.DOTALL)
    if old_reserve:
        new_reserve = """static void zt_map_text_text_rehash(zt_map_text_text *map, size_t new_hash_capacity) {
    size_t *new_hash_indices = (size_t *)malloc(new_hash_capacity * sizeof(size_t));
    for (size_t i = 0; i < new_hash_capacity; i++) {
        new_hash_indices[i] = SIZE_MAX;
    }
    size_t mask = new_hash_capacity - 1;
    for (size_t i = 0; i < map->len; i++) {
        uint32_t hash = zt_hash_text(map->keys[i]);
        size_t index = hash & mask;
        while (new_hash_indices[index] != SIZE_MAX) {
            index = (index + 1) & mask;
        }
        new_hash_indices[index] = i;
    }
    free(map->hash_indices);
    map->hash_indices = new_hash_indices;
    map->hash_capacity = new_hash_capacity;
}

static void zt_map_text_text_reserve(zt_map_text_text *map, size_t new_capacity) {
    if (new_capacity <= map->capacity) {
        return;
    }
    size_t cap = map->capacity == 0 ? 8 : map->capacity * 2;
    while (cap < new_capacity) {
        cap *= 2;
    }
    zt_text **new_keys = (zt_text **)realloc(map->keys, cap * sizeof(zt_text *));
    zt_text **new_values = (zt_text **)realloc(map->values, cap * sizeof(zt_text *));
    if (new_keys == NULL || new_values == NULL) {
        zt_runtime_error(ZT_ERR_PLATFORM, "failed to reallocate map arrays");
    }
    map->keys = new_keys;
    map->values = new_values;
    map->capacity = cap;
    
    if (map->hash_capacity < cap * 2) {
        size_t new_hash_cap = map->hash_capacity == 0 ? 16 : map->hash_capacity * 2;
        while (new_hash_cap < cap * 2) new_hash_cap *= 2;
        zt_map_text_text_rehash(map, new_hash_cap);
    }
}"""
        content = content.replace(old_reserve.group(0), new_reserve)

    # In map_set, update the hash slot when appending a new element
    # Look for map->keys[map->len] = key;
    old_set = """    map->keys[map->len] = key;
    map->values[map->len] = value;
    map->len += 1;"""
    new_set = """    map->keys[map->len] = key;
    map->values[map->len] = value;
    // We already found the empty hash slot in zt_map_text_text_find_index, 
    // but reserve might have rehashed! So we must re-find the slot.
    zt_bool dummy;
    size_t slot = zt_map_text_text_find_index(map, key, &dummy);
    map->hash_indices[slot] = map->len;
    map->len += 1;"""
    content = content.replace(old_set, new_set)

    # In map free, also free hash_indices
    content = content.replace("free(map->values);", "free(map->values);\n    free(map->hash_indices);")

    # Clean zt_release cases for Outcome/Optional
    for s in structs:
        heap_kind = "ZT_HEAP_" + s[3:].upper()
        # Find case ZT_HEAP_OPTIONAL_TEXT: ... break;
        content = re.sub(r'case\s+' + heap_kind + r':\s+zt_free_' + s[3:] + r'\(.*?break;', '', content, flags=re.DOTALL)

    # Also fix some propagate functions
    content = re.sub(r'(zt_outcome_[a-z0-9_]+_propagate[^}]+)zt_retain\([^)]+\);\s*', r'\1', content)

    # Special outcome return void
    content = content.replace("""zt_outcome_void_text zt_outcome_void_text_success(void) {
    static zt_outcome_void_text singleton = {
        { 1u, (uint32_t)ZT_HEAP_IMMORTAL_OUTCOME_VOID_TEXT },
        true,
        NULL
    };

    return &singleton;
}""", """zt_outcome_void_text zt_outcome_void_text_success(void) {
    zt_outcome_void_text outcome;
    outcome.is_success = true;
    outcome.error = NULL;
    return outcome;
}""")

    # Any returned &singleton ?
    content = re.sub(r'return\s+&singleton;', 'return singleton;', content)

    with open(c_path, 'w') as f:
        f.write(content)

if __name__ == '__main__':
    process_c('runtime/c/zenith_rt.c')
