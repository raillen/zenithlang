def clean_global(c_path):
    with open(c_path, 'r') as f:
        content = f.read()

    content = content.replace('    if (outcome.is_success && outcome.value != NULL) {\n', '')
    content = content.replace('    if (outcome.is_success && outcome.value != NULL) {', '')
    
    lines = content.split('\n')
    out_lines = []
    depth = 0
    for line in lines:
        line_depth_change = 0
        for char in line:
            if char == '{': line_depth_change += 1
            elif char == '}': line_depth_change -= 1
            
        if depth == 0 and line_depth_change < 0:
            # This line has more } than { and we are at depth 0 -> dangling }
            # Let's just not add this line.
            # Assuming it only contains }
            pass
        else:
            out_lines.append(line)
            depth += line_depth_change

    with open(c_path, 'w') as f:
        f.write('\n'.join(out_lines))

if __name__ == '__main__':
    clean_global('runtime/c/zenith_rt.c')
