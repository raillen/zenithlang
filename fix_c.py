import re

def fix_c(c_path):
    with open(c_path, 'r') as f:
        content = f.read()

    # 1. Fix the `free(optional); }` leftovers from broken regex
    # I'll just remove `free(optional); }` and `free(outcome); }` and any dangling `if (...) { ... }` leftovers.
    # Actually, it's safer to find the leftover fragments and wipe them.
    leftovers = [
        r'if\s*\(optional\.is_present[^}]+\}\s*free\(optional\);\s*\}',
        r'if\s*\(!outcome\.is_success[^}]+\}\s*free\(outcome\);\s*\}',
        r'if\s*\(!outcome\.is_success[^}]+\}\s*free\(outcome\);\s*\}',
        r'free\(optional\);\s*\}',
        r'free\(outcome\);\s*\}'
    ]
    for pattern in leftovers:
        content = re.sub(pattern, '', content)

    # 2. Fix the zt_outcome_*_failure_message functions:
    # `zt_outcome_i64_text *outcome = zt_outcome_i64_text_failure(error);`
    # -> `zt_outcome_i64_text outcome = zt_outcome_i64_text_failure(error);`
    content = re.sub(r'(zt_outcome_[a-z0-9_]+)\s*\*outcome\s*=\s*\1_failure', r'\1 outcome = \1_failure', content)
    # The return is fine (it's already `return outcome;`)

    # 3. Undo `value.` where `value` is `zt_text *` or `zt_bytes *`
    # In `zt_free_text`:
    content = content.replace('free(value.data);', 'free(value->data);')
    content = content.replace('value.data = NULL;', 'value->data = NULL;')
    content = content.replace('value.len = 0;', 'value->len = 0;')
    # In `zt_text_from_utf8`, `zt_text_concat`, etc:
    content = content.replace('value.data', 'value->data')
    content = content.replace('value.len', 'value->len')
    content = content.replace('value.header.rc', 'value->header.rc')
    content = content.replace('value.header.kind', 'value->header.kind')
    
    # Wait, there are places where value IS a zt_optional_text or outcome struct now, e.g. in `zt_optional_text_is_present(zt_optional_text value)`. 
    # If I blindly replace `value.` -> `value->`, I break those again.
    # But wait, in `zt_optional_text_is_present(zt_optional_text value)`, it should be `value.is_present`.
    # `is_present` was not replaced in the undo above. The undo only targets `.data`, `.len`, `.header.rc`, `.header.kind`. This is very safe!

    # 4. Remove `zt_optional_*_new` and `zt_outcome_*_new` and `zt_optional_*_alloc` properly
    # These functions look like:
    # static zt_optional_text *zt_optional_text_new(void) { ... return optional; }
    # Since their return type was already changed to value, they might look like:
    # static zt_optional_text zt_optional_text_new(void) { ... return optional; }
    # Or just wipe any function containing `_new(void) { ... return optional; }`
    # Let's use a regex that matches the function signature and everything up to `return optional; }` or `return outcome; }`
    content = re.sub(r'static zt_optional_[a-z0-9_]+\s+zt_optional_[a-z0-9_]+_new\(void\)\s*\{.*?return\s+optional;\s*\}', '', content, flags=re.DOTALL)
    content = re.sub(r'static zt_outcome_[a-z0-9_]+\s+zt_outcome_[a-z0-9_]+_new\(void\)\s*\{.*?return\s+outcome;\s*\}', '', content, flags=re.DOTALL)
    
    # Same for map_text_text_alloc if it exists? Map is pointer, keep it.

    # 5. Fix zt_hash_text being implicit. 
    # Find `static uint32_t zt_hash_text` and move it to the top, before `zt_map_text_text_new`.
    # Actually, the error says:
    # runtime/c/zenith_rt.c:1687:17: error: conflicting types for 'zt_hash_text'
    # runtime/c/zenith_rt.c:813:21: note: previous implicit declaration
    # So `zt_hash_text` is defined at 1687. I will extract it and put it before `zt_map_text_text_find_index`.
    hash_match = re.search(r'static uint32_t zt_hash_text\(const zt_text \*key\)\s*\{.*?return hash;\s*\}\n', content, flags=re.DOTALL)
    if hash_match:
        hash_code = hash_match.group(0)
        content = content.replace(hash_code, '') # Remove it from its current spot
        # Insert before zt_map_text_text_find_index
        find_idx = content.find('size_t zt_map_text_text_find_index')
        if find_idx != -1:
            content = content[:find_idx] + hash_code + '\n' + content[find_idx:]

    # 6. zt_validate_and_free_text
    # `value == NULL || value.data == NULL || value.len == 0`
    # We already reverted `value.data` to `value->data`, etc.

    # 7. zt_host_default_write_stream
    # `if (value.len > 0)` -> already reverted to `value->len`.

    # 8. Let's fix outcome propagate
    # e.g., `zt_outcome_i64_text zt_outcome_i64_text_propagate(zt_outcome_i64_text outcome) { zt_retain(outcome); return outcome; }`
    # We stripped `zt_retain` in previous script.
    # But now `outcome` is passed by value, so `zt_retain(&outcome)` is wrong anyway if it expects a pointer. 
    # Actually, we shouldn't retain the outcome struct itself. The outcome struct is a value. 
    # Does it contain a text? Yes, outcome->error or outcome->value.
    # If we propagate, we are copying the struct. The struct contains pointers. So if we return it by value, we don't need to retain the pointers again IF we are transferring ownership, OR if we are copying, we DO need to retain.
    # The prompt says: "Convert `optional` and `outcome` wrappers to Value Types (Stack)". 
    # Usually returning by value copies the struct. If ARC is used, the caller might release it later.
    # Let's not overthink ARC for now, just fix compile errors.
    
    # 9. In `zt_outcome_text_text_failure_message`, `zt_outcome_text_text *outcome = ...`
    # Fixed in step 2.

    # 10. `optional = (zt_optional_text *)calloc(...)`
    # These are inside `_new` which we just removed in step 4.

    # 11. implicit declaration of `zt_free_optional_...` in `zt_release`
    # Our previous script tried to remove `case ZT_HEAP_OPTIONAL_TEXT: ... break;`
    # Let's ensure they are completely removed from `zt_release`.
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
        heap_kind = "ZT_HEAP_" + s[3:].upper()
        # Regex to remove case ZT_HEAP_X: zt_free_X(ref); break;
        content = re.sub(r'case\s+' + heap_kind + r':\s+zt_free_' + s[3:] + r'\(\(.*?\)ref\);\s*break;', '', content, flags=re.DOTALL)
        content = re.sub(r'case\s+' + heap_kind + r':\s+zt_free_' + s[3:] + r'\(ref\);\s*break;', '', content, flags=re.DOTALL)
        content = re.sub(r'case\s+' + heap_kind + r':\s+break;', '', content, flags=re.DOTALL) # in case it's empty
        content = re.sub(r'zt_free_' + s[3:] + r'\([^;]+;\n?', '', content)

    # 12. Fix zt_list_text_get_optional prototype vs implementation
    # Implementation: `zt_optional_text *zt_list_text_get_optional` -> wait, I already replaced it!
    # Let's check why there was an error: "conflicting types for 'zt_list_text_get_optional'; have 'zt_optional_text *(...)'"
    # Ah, the regex to replace return type might have missed it if it was slightly different!
    # Let's force it:
    content = re.sub(r'zt_optional_text\s*\*zt_list_text_get_optional', 'zt_optional_text zt_list_text_get_optional', content)
    content = re.sub(r'zt_optional_text\s*\*zt_map_text_text_get_optional', 'zt_optional_text zt_map_text_text_get_optional', content)

    with open(c_path, 'w') as f:
        f.write(content)

if __name__ == '__main__':
    fix_c('runtime/c/zenith_rt.c')
