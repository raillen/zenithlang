import re

def fix_c(c_path):
    with open(c_path, 'r') as f:
        content = f.read()

    # The compiler errors give us exact line numbers and patterns to fix!
    # Instead of relying on what I think I broke, I'll fix the exact code from the compiler errors.

    # 1. zt_outcome_failure_message returning pointers instead of values
    content = re.sub(r'zt_outcome_([a-z0-9_]+)_text\s+\*outcome\s*=\s*zt_outcome_\1_text_failure\(error\);',
                     r'zt_outcome_\1_text outcome = zt_outcome_\1_text_failure(error);', content)

    # 2. value.data -> value->data in all these specific locations
    # Rather than guessing, let's just globally replace value.data with value->data 
    # EXCEPT where value is a struct. In zenith_rt.c, `value` is usually a pointer to `zt_text`, `zt_bytes`, `zt_list_*`.
    # Only in `zt_optional_*_is_present` and `zt_optional_*_coalesce` is `value` a struct.
    # Let's fix those specifically.
    content = re.sub(r'\bvalue\.data\b', 'value->data', content)
    content = re.sub(r'\bvalue\.len\b', 'value->len', content)
    content = re.sub(r'\bvalue\.header\.', 'value->header.', content)

    # For optional/outcome where value is a value type, it's called `value` and has `is_present` / `is_success`.
    # Wait, in `zt_optional_i64_coalesce(zt_optional_i64 value, ...)` we need `value.is_present`. My script didn't touch `.is_present`!
    # But wait, in coalesce: `selected = value->is_present ? value->value : fallback;` -> `selected = value.is_present ? value.value : fallback;`
    # Let's explicitly replace `value->is_present` with `value.is_present`, `value->value` with `value.value` in those functions.
    # Actually, the compiler only complained about `value.data`, `value.len`, `value.header`!
    # It didn't complain about `value.is_present`!
    
    # 3. Leftover `free(optional);` and `free(outcome);` inside empty `{}` or mixed.
    # The compiler error shows:
    # if (optional.is_present && optional.value != NULL) {
    # free(optional);
    # }
    content = re.sub(r'if\s*\([^)]+\)\s*\{\s*free\((?:optional|outcome)\);\s*\}', '', content)
    content = re.sub(r'free\((?:optional|outcome)\);\s*\}', '}', content)

    # 4. zt_hash_text implicit declaration
    # Move `zt_hash_text` up.
    hash_func = r'static uint32_t zt_hash_text\(const zt_text \*key\)\s*\{.*?return hash;\s*\}'
    match = re.search(hash_func, content, flags=re.DOTALL)
    if match:
        code = match.group(0)
        content = content.replace(code, '') # erase from bottom
        # insert near the top, e.g., after `#include <string.h>`
        content = content.replace('#include <string.h>', '#include <string.h>\n\n' + code)

    # 5. `zt_optional_*_new` and `zt_outcome_*_new` which assign to `optional = (zt_optional_text *)calloc(...)`
    # The compiler complains about `optional = (zt_optional_text *)calloc(...)`
    # Let's completely eradicate ANY function ending in `_new(void)` that returns an optional or outcome!
    content = re.sub(r'static\s+zt_optional_[a-z0-9_]+\s+zt_optional_[a-z0-9_]+_new\(void\)\s*\{[^}]+\}\n?', '', content, flags=re.DOTALL)
    content = re.sub(r'static\s+zt_outcome_[a-z0-9_]+\s+zt_outcome_[a-z0-9_]+_new\(void\)\s*\{[^}]+\}\n?', '', content, flags=re.DOTALL)

    # 6. zt_text_from_utf8_bytes has conflicting types: `zt_outcome_text_text *zt_text_from_utf8_bytes`
    content = re.sub(r'zt_outcome_text_text\s+\*zt_text_from_utf8_bytes', 'zt_outcome_text_text zt_text_from_utf8_bytes', content)
    content = content.replace('return zt_outcome_text_text_failure_message(message);', 
                              'return zt_outcome_text_text_failure_message(message);') # already correct return type for failure_message
    
    # 7. zt_list_text_get_optional conflicting types
    content = re.sub(r'zt_optional_text\s+\*zt_list_text_get_optional', 'zt_optional_text zt_list_text_get_optional', content)
    
    # 8. zt_map_text_text_get_optional conflicting types
    content = re.sub(r'zt_optional_text\s+\*zt_map_text_text_get_optional', 'zt_optional_text zt_map_text_text_get_optional', content)

    # Wait, did we miss zt_list_i64_get_optional? Let's fix all of them to be sure.
    content = re.sub(r'zt_optional_[a-z0-9_]+\s+\*zt_list_i64_get_optional', 'zt_optional_i64 zt_list_i64_get_optional', content)

    # 9. In `zt_release`, the `case ZT_HEAP_OPTIONAL_TEXT: zt_free_optional_text(...) break;`
    content = re.sub(r'case\s+ZT_HEAP_OPTIONAL_[A-Z0-9_]+:\s+zt_free_optional_[a-z0-9_]+\([^)]+\);\s*break;', '', content)
    content = re.sub(r'case\s+ZT_HEAP_OUTCOME_[A-Z0-9_]+:\s+zt_free_outcome_[a-z0-9_]+\([^)]+\);\s*break;', '', content)
    # Also without cast:
    content = re.sub(r'case\s+ZT_HEAP_OPTIONAL_[A-Z0-9_]+:\s*break;', '', content)
    content = re.sub(r'case\s+ZT_HEAP_OUTCOME_[A-Z0-9_]+:\s*break;', '', content)

    # Any remaining calls to `zt_free_optional_*` or `zt_free_outcome_*`
    content = re.sub(r'zt_free_optional_[a-z0-9_]+\([^)]+\);', '', content)
    content = re.sub(r'zt_free_outcome_[a-z0-9_]+\([^)]+\);', '', content)

    with open(c_path, 'w') as f:
        f.write(content)

if __name__ == '__main__':
    fix_c('runtime/c/zenith_rt.c')
