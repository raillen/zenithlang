import re

def clean_c(c_path):
    with open(c_path, 'r') as f:
        lines = f.readlines()

    out_lines = []
    skip_until = None
    brace_depth = 0

    for i, line in enumerate(lines):
        if skip_until == 'func_end':
            brace_depth += line.count('{')
            brace_depth -= line.count('}')
            if brace_depth == 0:
                skip_until = None
            continue

        if skip_until == 'case_end':
            # Stop skipping at the next `case ` or `default:` or `}` (if it matches the switch)
            if re.match(r'\s*(case\s+ZT_HEAP_|default:)', line):
                skip_until = None
                # Don't continue, we want to process this line
            elif re.match(r'\s*\}\s*$', line): # End of switch
                skip_until = None
                out_lines.append(line)
                continue
            else:
                continue

        # Check for function to remove
        if re.match(r'^static\s+void\s+zt_free_(optional|outcome)_[a-z0-9_]+\(', line):
            skip_until = 'func_end'
            brace_depth = line.count('{') - line.count('}')
            if brace_depth == 0 and '{' in line:
                skip_until = None
            continue

        # Check for cases to remove in zt_release
        if re.match(r'\s*case\s+ZT_HEAP_(OPTIONAL|OUTCOME)_[A-Z0-9_]+:', line):
            skip_until = 'case_end'
            continue

        # Fix zt_text_from_utf8_bytes
        if 'zt_outcome_text_text *zt_text_from_utf8_bytes' in line:
            line = line.replace('zt_outcome_text_text *zt_text_from_utf8_bytes', 'zt_outcome_text_text zt_text_from_utf8_bytes')

        # Fix static static
        if 'static static uint32_t zt_hash_text' in line:
            line = line.replace('static static uint32_t', 'static uint32_t')

        out_lines.append(line)

    # Let's join and do a final sweep to remove dangling standalone `}` that might have been left if there were nested brackets 
    # but since we process line by line and track brace_depth correctly it should be fine.
    
    with open(c_path, 'w') as f:
        f.writelines(out_lines)

if __name__ == '__main__':
    clean_c('runtime/c/zenith_rt.c')
