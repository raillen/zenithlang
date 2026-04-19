import os
import re

TEST_SETUP = """    zt_arena test_arena;
    zt_string_pool test_pool;
    zt_arena_init(&test_arena);
    zt_string_pool_init(&test_pool, &test_arena);
"""

# REF REFACTOR TESTS
for root, _, files in os.walk('tests'):
    for file in files:
        if file.endswith('.c'):
            path = os.path.join(root, file)
            with open(path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()

            if 'zt_parse(' not in content:
                continue

            orig = content

            def replacer(match):
                func_start = match.group(1)
                func_body = match.group(2)
                if 'zt_parse(' in func_body:
                    if 'zt_arena test_arena;' not in func_body:
                        func_body = TEST_SETUP + func_body
                    func_body = re.sub(r'zt_parse\(([^,]+?),\s*([^,]+?),\s*([^)]+?)\)', r'zt_parse(&test_arena, &test_pool, \1, \2, \3)', func_body)
                    func_body = func_body.replace('zt_parser_result_dispose(&r);', 'zt_parser_result_dispose(&r);\n    zt_arena_dispose(&test_arena);')
                    func_body = func_body.replace('zt_parser_result_dispose(&result);', 'zt_parser_result_dispose(&result);\n    zt_arena_dispose(&test_arena);')
                return func_start + '{' + func_body + '}'

            content = re.sub(r'((?:static\s+)?void\s+test_[a-zA-Z0-9_]+\([^)]*\)\s*)\{([^}]+)\}', replacer, content, flags=re.DOTALL)

            if 'zt_arena test_arena;' in content and 'arena.h' not in content:
                content = '#include "compiler/utils/arena.h"\n#include "compiler/utils/string_pool.h"\n' + content

            # Fallback for remaining zt_parse calls
            content = re.sub(r'(?<!_)zt_parse\([^&]([^,]+?),\s*([^,]+?),\s*([^)]+?)\)', r'zt_parse(&test_arena, &test_pool, \1, \2, \3)', content)

            if orig != content:
                with open(path, 'w', encoding='utf-8') as f:
                    f.write(content)

# driver logic refactor
for path in ['compiler/driver/main.c', 'compiler/driver/lsp.c']:
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    orig = content
    if 'arena.h' not in content:
        content = '#include "compiler/utils/arena.h"\n#include "compiler/utils/string_pool.h"\n' + content

    if 'main.c' in path:
        # zt_parse_project_sources uses zt_parse. So it needs global_arena and global_pool exported or passed to it.
        # Actually compiler/driver/main.c global variables
        if 'zt_arena global_arena;' not in content:
            content = content.replace('static int zt_ci_mode_enabled = 0;', 'static int zt_ci_mode_enabled = 0;\n\nstatic zt_arena global_arena;\nstatic zt_string_pool global_pool;')
            content = content.replace('int main(int argc, char *argv[]) {', 'int main(int argc, char *argv[]) {\n    zt_arena_init(&global_arena);\n    zt_string_pool_init(&global_pool, &global_arena);\n')
            content = content.replace('files->items[i].parsed = zt_parse(', 'files->items[i].parsed = zt_parse(&global_arena, &global_pool, ')
    
    if 'lsp.c' in path: # assume it just parses directly in some functions
        if 'zt_arena global_arena;' not in content:
            content = content.replace('#include "compiler/driver/lsp.h"\n', '#include "compiler/driver/lsp.h"\n\nstatic zt_arena global_arena;\nstatic zt_string_pool global_pool;\n')
            content = content.replace('void zt_lsp_serve(void) {', 'void zt_lsp_serve(void) {\n    zt_arena_init(&global_arena);\n    zt_string_pool_init(&global_pool, &global_arena);\n')
            content = content.replace('zt_parse(uri, text, strlen(text))', 'zt_parse(&global_arena, &global_pool, uri, text, strlen(text))')

    if orig != content:
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

print('Done')
