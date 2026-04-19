def remove_lines(c_path):
    with open(c_path, 'r') as f:
        lines = f.readlines()

    to_remove = [
        300, 308, 316, 324,
        362, 363, 366, 367, 370, 371, 374, 375, 378, 379, 382, 383, 386, 387, 390, 391, 394, 395, 398, 399
    ]
    # Convert to 0-indexed
    to_remove = set(x - 1 for x in to_remove)

    # Let's also verify that we aren't leaving empty `{` open from the `if`. 
    # Ah, the `if` ends with `{`. It has no body, the body was deleted earlier.
    # What about the matching `}` for those `if`s? Did I leave them behind?
    # If the file has:
    # 300: if (...) {
    # 301: }
    # Let's check if line 301 is a `}`. If I just delete 300, 301 becomes dangling.

    # It's much safer to use a regex to remove:
    # `if \(outcome\.is_success && outcome\.value != NULL\) \{(\s*\})?`
    # Let's just do a regex replace over the whole string.

    content = "".join(lines)
    import re
    
    # Remove the specific trailing `if` statements and their optional trailing `}`
    content = re.sub(r'if\s*\(outcome\.is_success && outcome\.value != NULL\)\s*\{\s*\}?', '', content)
    
    # Let's remove the dangling `}` at those specific lines
    lines = content.split('\n')
    
    # Since I don't know the exact lines anymore after regex replace, I will parse the file and track brace depth.
    # Any `}` that drops brace depth below 0 is dangling and should be removed.
    # Actually, functions are at depth 0. If we see a `}` at depth 0, it's dangling.
    out_lines = []
    depth = 0
    for line in lines:
        d = depth
        for char in line:
            if char == '{': depth += 1
            elif char == '}': depth -= 1
            
        if depth < 0:
            # We found a dangling `}`. Just ignore this line (or at least ignore the `}`).
            # Since the line might be just `    }` or `}`, we can just not add it.
            # But wait, what if depth drops to -1 mid-line?
            # It's always a standalone `}` in this file.
            depth = 0
            continue
            
        out_lines.append(line)
        
    with open(c_path, 'w') as f:
        f.write('\n'.join(out_lines))

if __name__ == '__main__':
    remove_lines('runtime/c/zenith_rt.c')
