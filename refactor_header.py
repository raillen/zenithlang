import re

def process_header(h_path):
    with open(h_path, 'r') as f:
        content = f.read()

    # 1. Remove zt_header header; from optional and outcome
    content = re.sub(r'typedef struct (zt_optional_[a-z0-9_]+|zt_outcome_[a-z0-9_]+)\s*\{\s*zt_header header;\s*', r'typedef struct \1 {\n    ', content)

    # 2. Add `_owned` variants for push
    content = content.replace('void zt_list_i64_push(zt_list_i64 *list, zt_int value);',
                              'void zt_list_i64_push(zt_list_i64 *list, zt_int value);\nzt_list_i64 *zt_list_i64_push_owned(zt_list_i64 *list, zt_int value);')
    content = content.replace('void zt_list_text_push(zt_list_text *list, zt_text *value);',
                              'void zt_list_text_push(zt_list_text *list, zt_text *value);\nzt_list_text *zt_list_text_push_owned(zt_list_text *list, zt_text *value);')

    # 3. Hash map struct
    map_struct_old = """typedef struct zt_map_text_text {
    zt_header header;
    size_t len;
    size_t capacity;
    zt_text **keys;
    zt_text **values;
} zt_map_text_text;"""
    map_struct_new = """typedef struct zt_map_text_text {
    zt_header header;
    size_t len;
    size_t capacity;
    zt_text **keys;
    zt_text **values;
    size_t hash_capacity;
    size_t *hash_indices;
} zt_map_text_text;"""
    content = content.replace(map_struct_old, map_struct_new)

    # 4. Change pointers to values for outcome and optional
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
        # `const zt_optional_text *` -> `zt_optional_text `
        content = re.sub(r'const\s+' + s + r'\s*\*', s + r' ', content)
        # `zt_optional_text *(*` -> `zt_optional_text (*`
        content = re.sub(r'\b' + s + r'\s*\*\s*\(\*', s + r' (*', content)
        # `zt_optional_text *` -> `zt_optional_text `
        content = re.sub(r'\b' + s + r'\s*\*', s + r' ', content)

    with open(h_path, 'w') as f:
        f.write(content)

if __name__ == '__main__':
    process_header('runtime/c/zenith_rt.h')
