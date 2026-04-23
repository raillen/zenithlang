/*
 * project.c - Project source management for the Zenith compiler driver.
 *
 * Responsibilities:
 *   - Source file list management (init, dispose, push, discover)
 *   - Namespace utilities (resolve, validate, exists, find)
 *   - Import cycle detection
 *   - Project source parsing
 *   - Combined AST building
 *   - Since filter (git diff)
 */

#include "compiler/driver/driver_internal.h"
/* ── Project source file management (kept in main.c) ──────────── */

void zt_project_source_file_list_init(zt_project_source_file_list *list) {
    if (list == NULL) return;
    memset(list, 0, sizeof(*list));
}

void zt_project_source_file_list_dispose(zt_project_source_file_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i += 1) {
        free(list->items[i].source_text);
        list->items[i].source_text = NULL;
        if (list->items[i].parsed_ready) {
            zt_parser_result_dispose(&list->items[i].parsed);
            list->items[i].parsed_ready = 0;
        }
    }
    free(list->items);
    memset(list, 0, sizeof(*list));
}

int zt_load_since_filter(
        zt_driver_context *ctx,
        const char *project_root,
        const char *since_ref) {
    const char *argv[8];
    char capture_path[512];
    FILE *capture;
    char line[1024];
    int status;
    int spawn_failed = 0;

    if (ctx == NULL) return 0;

    zt_path_filter_list_dispose(&ctx->since_filter);

    if (since_ref == NULL || since_ref[0] == '\0') {
        return 1;
    }

    if (project_root == NULL || project_root[0] == '\0') {
        fprintf(stderr, "error: --since requires a project root\n");
        return 0;
    }

    if (!zt_git_ref_is_safe(since_ref)) {
        fprintf(stderr, "error: invalid --since ref '%s'\n", since_ref);
        return 0;
    }

    if (!zt_native_make_temp_path(capture_path, sizeof(capture_path), "since-filter", ".log")) {
        fprintf(stderr, "error: cannot prepare --since capture file\n");
        return 0;
    }

    argv[0] = "git";
    argv[1] = "-C";
    argv[2] = project_root;
    argv[3] = "diff";
    argv[4] = "--name-only";
    argv[5] = since_ref;
    argv[6] = "--";
    argv[7] = NULL;

    status = zt_native_spawn_process(argv, capture_path, &spawn_failed);
    if (spawn_failed || status != 0) {
        zt_native_remove_file_if_exists(capture_path);
        fprintf(stderr, "error: git diff failed for --since ref '%s'\n", since_ref);
        return 0;
    }

    capture = fopen(capture_path, "rb");
    if (capture == NULL) {
        zt_native_remove_file_if_exists(capture_path);
        fprintf(stderr, "error: cannot read git output for --since\n");
        return 0;
    }

    while (fgets(line, sizeof(line), capture) != NULL) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[len - 1] = '\0';
            len -= 1;
        }
        if (line[0] == '\0') continue;
        if (line[0] == '.' && line[1] == '/') {
            memmove(line, line + 2, strlen(line + 2) + 1);
        }
        zt_normalize_path_separators(line);
        if (!zt_path_filter_list_push(&ctx->since_filter, line)) {
            fclose(capture);
            zt_native_remove_file_if_exists(capture_path);
            fprintf(stderr, "error: out of memory while loading --since filter\n");
            zt_path_filter_list_dispose(&ctx->since_filter);
            return 0;
        }
    }

    if (fclose(capture) != 0) {
        zt_native_remove_file_if_exists(capture_path);
        fprintf(stderr, "error: cannot close git output for --since\n");
        zt_path_filter_list_dispose(&ctx->since_filter);
        return 0;
    }

    zt_native_remove_file_if_exists(capture_path);
    return 1;
}

int zt_project_source_file_list_push(zt_project_source_file_list *list, const char *path) {
    zt_project_source_file *new_items;
    size_t new_capacity;
    char normalized[512];
    size_t i;

    if (list == NULL || path == NULL) return 0;

    if (!zt_copy_text(normalized, sizeof(normalized), path)) return 0;
    zt_normalize_path_inplace(normalized);

    /* printf("DEBUG: pushing source file: %s (original: %s)\n", normalized, path); */

    for (i = 0; i < list->count; i += 1) {
        if (strcmp(list->items[i].path, normalized) == 0) {
            /* printf("DEBUG: skipped duplicate: %s\n", normalized); */
            return 1; /* Already in list */
        }
    }

    if (list->count >= list->capacity) {
        new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        new_items = (zt_project_source_file *)realloc(list->items, new_capacity * sizeof(zt_project_source_file));
        if (new_items == NULL) return 0;
        list->items = new_items;
        list->capacity = new_capacity;
    }

    memset(&list->items[list->count], 0, sizeof(list->items[list->count]));
    if (!zt_copy_text(list->items[list->count].path, sizeof(list->items[list->count].path), normalized)) {
        return 0;
    }
    list->count += 1;
    return 1;
}

int zt_project_discover_zt_files(const char *directory, zt_project_source_file_list *files) {
#ifdef _WIN32
    char pattern[768];
    intptr_t handle;
    struct _finddata_t data;

    if (!zt_join_path(pattern, sizeof(pattern), directory, "*")) {
        fprintf(stderr, "error: source directory path is too long\n");
        return 0;
    }

    handle = _findfirst(pattern, &data);
    if (handle == -1) {
        fprintf(stderr, "error: cannot scan source directory '%s'\n", directory);
        return 0;
    }

    do {
        char child_path[768];

        if (strcmp(data.name, ".") == 0 || strcmp(data.name, "..") == 0) {
            continue;
        }

        if (!zt_join_path(child_path, sizeof(child_path), directory, data.name)) {
            _findclose(handle);
            fprintf(stderr, "error: source path is too long\n");
            return 0;
        }

        if ((data.attrib & _A_SUBDIR) != 0) {
            if (!zt_project_discover_zt_files(child_path, files)) {
                _findclose(handle);
                return 0;
            }
        } else if (zt_path_has_extension(child_path, ".zt")) {
            if (!zt_project_source_file_list_push(files, child_path)) {
                _findclose(handle);
                fprintf(stderr, "error: out of memory while collecting source files\n");
                return 0;
            }
        }
    } while (_findnext(handle, &data) == 0);

    _findclose(handle);
    return 1;
#else
    DIR *dir;
    struct dirent *entry;

    dir = opendir(directory);
    if (dir == NULL) {
        fprintf(stderr, "error: cannot scan source directory '%s'\n", directory);
        return 0;
    }

    while ((entry = readdir(dir)) != NULL) {
        char child_path[768];

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (!zt_join_path(child_path, sizeof(child_path), directory, entry->d_name)) {
            closedir(dir);
            fprintf(stderr, "error: source path is too long\n");
            return 0;
        }

        if (zt_path_is_dir(child_path)) {
            if (!zt_project_discover_zt_files(child_path, files)) {
                closedir(dir);
                return 0;
            }
        } else if (zt_path_has_extension(child_path, ".zt")) {
            if (!zt_project_source_file_list_push(files, child_path)) {
                closedir(dir);
                fprintf(stderr, "error: out of memory while collecting source files\n");
                return 0;
            }
        }
    }

    closedir(dir);
    return 1;
#endif
}

int zt_namespace_to_relative_path(const char *namespace_name, char *dest, size_t capacity) {
    size_t out = 0;
    size_t i;

    if (namespace_name == NULL || dest == NULL || capacity == 0) return 0;

    for (i = 0; namespace_name[i] != '\0'; i += 1) {
        char ch = namespace_name[i] == '.' ? '/' : namespace_name[i];
        if (out + 1 >= capacity) return 0;
        dest[out++] = ch;
    }

    if (out + 3 >= capacity) return 0;
    dest[out++] = '.';
    dest[out++] = 'z';
    dest[out++] = 't';
    dest[out] = '\0';
    return 1;
}


const char *zt_last_namespace_segment(const char *namespace_name) {
    const char *dot;
    if (namespace_name == NULL) return NULL;
    dot = strrchr(namespace_name, '.');
    return dot != NULL ? dot + 1 : namespace_name;
}

const zt_ast_node *zt_find_entry_root(const zt_project_source_file_list *files, const char *entry_namespace) {
    size_t i;
    if (files == NULL || entry_namespace == NULL) return NULL;
    for (i = 0; i < files->count; i += 1) {
        const zt_ast_node *root = files->items[i].parsed.root;
        if (root != NULL &&
                root->kind == ZT_AST_FILE &&
                root->as.file.module_name != NULL &&
                strcmp(root->as.file.module_name, entry_namespace) == 0) {
            return root;
        }
    }
    return NULL;
}

int zt_namespace_exists(const zt_project_source_file_list *files, const char *namespace_name) {
    return zt_find_entry_root(files, namespace_name) != NULL;
}

int zt_namespace_is_std(const char *namespace_name) {
    return namespace_name != NULL && strncmp(namespace_name, "std.", 4) == 0;
}

size_t zt_find_namespace_index(const zt_project_source_file_list *files, const char *namespace_name) {
    size_t i;
    if (files == NULL || namespace_name == NULL) return (size_t)-1;
    for (i = 0; i < files->count; i += 1) {
        const zt_ast_node *root = files->items[i].parsed.root;
        if (root != NULL &&
                root->kind == ZT_AST_FILE &&
                root->as.file.module_name != NULL &&
                strcmp(root->as.file.module_name, namespace_name) == 0) {
            return i;
        }
    }
    return (size_t)-1;
}

static void zt_print_import_cycle(
        const zt_project_source_file_list *files,
        const size_t *stack,
        size_t depth,
        size_t repeated_index,
        const zt_ast_node *import_decl) {
    size_t start = 0;
    size_t i;

    for (i = 0; i < depth; i += 1) {
        if (stack[i] == repeated_index) {
            start = i;
            break;
        }
    }

    fprintf(
        stderr,
        "%s:%zu:%zu: project error: import cycle detected: ",
        import_decl != NULL && import_decl->span.source_name != NULL ? import_decl->span.source_name : "<project>",
        import_decl != NULL ? import_decl->span.line : 1,
        import_decl != NULL ? import_decl->span.column_start : 1);

    for (i = start; i < depth; i += 1) {
        const zt_ast_node *root = files->items[stack[i]].parsed.root;
        if (i != start) fprintf(stderr, " -> ");
        fprintf(stderr, "%s", root != NULL && root->as.file.module_name != NULL ? root->as.file.module_name : "<unknown>");
    }

    {
        const zt_ast_node *root = files->items[repeated_index].parsed.root;
        fprintf(stderr, " -> %s\n", root != NULL && root->as.file.module_name != NULL ? root->as.file.module_name : "<unknown>");
    }
}

static int zt_detect_import_cycle_visit(
        const zt_project_source_file_list *files,
        size_t index,
        unsigned char *marks,
        size_t *stack,
        size_t depth) {
    const zt_ast_node *root;
    size_t import_index;

    if (files == NULL || marks == NULL || stack == NULL || index >= files->count) return 1;

    if (marks[index] == 2) return 1;
    if (marks[index] == 1) return 0;

    marks[index] = 1;
    stack[depth] = index;
    root = files->items[index].parsed.root;

    if (root != NULL && root->kind == ZT_AST_FILE) {
        for (import_index = 0; import_index < root->as.file.imports.count; import_index += 1) {
            const zt_ast_node *import_decl = root->as.file.imports.items[import_index];
            size_t target_index;
            if (import_decl == NULL || import_decl->kind != ZT_AST_IMPORT_DECL) continue;

            target_index = zt_find_namespace_index(files, import_decl->as.import_decl.path);
            if (target_index == (size_t)-1) continue;

            if (marks[target_index] == 1) {
                zt_print_import_cycle(files, stack, depth + 1, target_index, import_decl);
                return 0;
            }

            if (!zt_detect_import_cycle_visit(files, target_index, marks, stack, depth + 1)) {
                return 0;
            }
        }
    }

    marks[index] = 2;
    return 1;
}

int zt_detect_import_cycles(const zt_project_source_file_list *files) {
    unsigned char *marks;
    size_t *stack;
    size_t i;
    int ok = 1;

    if (files == NULL) return 0;

    marks = (unsigned char *)calloc(files->count > 0 ? files->count : 1, sizeof(unsigned char));
    stack = (size_t *)calloc(files->count > 0 ? files->count : 1, sizeof(size_t));
    if (marks == NULL || stack == NULL) {
        fprintf(stderr, "project error: out of memory while checking import cycles\n");
        free(marks);
        free(stack);
        return 0;
    }

    for (i = 0; i < files->count; i += 1) {
        if (marks[i] == 0 && !zt_detect_import_cycle_visit(files, i, marks, stack, 0)) {
            ok = 0;
            break;
        }
    }

    free(marks);
    free(stack);
    return ok;
}

static const char *zt_alias_for_namespace(
        const zt_project_source_file_list *files,
        const char *entry_namespace,
        const char *namespace_name) {
    const zt_ast_node *entry_root;
    size_t i;

    entry_root = zt_find_entry_root(files, entry_namespace);
    if (entry_root != NULL) {
        for (i = 0; i < entry_root->as.file.imports.count; i += 1) {
            const zt_ast_node *import_decl = entry_root->as.file.imports.items[i];
            if (import_decl != NULL &&
                    import_decl->kind == ZT_AST_IMPORT_DECL &&
                    import_decl->as.import_decl.path != NULL &&
                    strcmp(import_decl->as.import_decl.path, namespace_name) == 0) {
                return import_decl->as.import_decl.alias != NULL
                    ? import_decl->as.import_decl.alias
                    : zt_last_namespace_segment(namespace_name);
            }
        }
    }

    return zt_last_namespace_segment(namespace_name);
}

static int zt_prefix_ast_name(zt_string_pool *pool, const char **name, const char *prefix) {
    size_t prefix_len;
    size_t name_len;
    char combined[1024];

    if (name == NULL || *name == NULL || prefix == NULL || prefix[0] == '\0') return 1;
    if (strchr(*name, '.') != NULL) return 1;

    prefix_len = strlen(prefix);
    name_len = strlen(*name);
    if (prefix_len + 1 + name_len >= sizeof(combined)) return 0;

    memcpy(combined, prefix, prefix_len);
    combined[prefix_len] = '.';
    memcpy(combined + prefix_len + 1, *name, name_len + 1);

    *name = zt_string_pool_intern(pool, combined);
    return 1;
}

static int zt_prefix_declaration_for_namespace(zt_string_pool *pool, zt_ast_node *decl, const char *prefix) {
    size_t i;

    if (decl == NULL || prefix == NULL || prefix[0] == '\0') return 1;

    switch (decl->kind) {
        case ZT_AST_FUNC_DECL:
            return zt_prefix_ast_name(pool, &decl->as.func_decl.name, prefix);
        case ZT_AST_STRUCT_DECL:
            return zt_prefix_ast_name(pool, &decl->as.struct_decl.name, prefix);
        case ZT_AST_TRAIT_DECL:
            return zt_prefix_ast_name(pool, &decl->as.trait_decl.name, prefix);
        case ZT_AST_ENUM_DECL:
            return zt_prefix_ast_name(pool, &decl->as.enum_decl.name, prefix);
        case ZT_AST_CONST_DECL:
            return zt_prefix_ast_name(pool, &decl->as.const_decl.name, prefix);
        case ZT_AST_VAR_DECL:
            return zt_prefix_ast_name(pool, &decl->as.var_decl.name, prefix);
        case ZT_AST_EXTERN_DECL:
            for (i = 0; i < decl->as.extern_decl.functions.count; i += 1) {
                zt_ast_node *func = decl->as.extern_decl.functions.items[i];
                if (func != NULL && !zt_prefix_ast_name(pool, &func->as.func_decl.name, prefix)) return 0;
            }
            return 1;
        default:
            return 1;
    }
}

int zt_validate_source_namespaces(
        const zt_project_source_file_list *files,
        const zt_project_manifest *manifest,
        zt_diag_list *diagnostics) {
    size_t i;

    if (files == NULL || manifest == NULL || diagnostics == NULL) return 0;

    if (zt_project_manifest_kind(manifest) == ZT_PROJECT_KIND_APP &&
            !zt_namespace_exists(files, manifest->app_entry)) {
        zt_diag_list_add(diagnostics, ZT_DIAG_PROJECT_MISSING_ENTRY, zt_source_span_unknown(),
            "app.entry namespace '%s' was not found under source.root", manifest->app_entry);
        return 0;
    }

    for (i = 0; i < files->count; i += 1) {
        const zt_ast_node *root = files->items[i].parsed.root;
        char expected_suffix[512];
        size_t import_index;

        if (root == NULL || root->kind != ZT_AST_FILE || root->as.file.module_name == NULL) {
            zt_diag_list_add(diagnostics, ZT_DIAG_PROJECT_ERROR, zt_source_span_make(files->items[i].path, 1, 1, 1),
                "source file has no namespace");
            return 0;
        }

        if (!zt_namespace_to_relative_path(root->as.file.module_name, expected_suffix, sizeof(expected_suffix))) {
            zt_diag_list_add(diagnostics, ZT_DIAG_PROJECT_ERROR, zt_source_span_make(files->items[i].path, 1, 1, 1),
                "namespace '%s' is too long", root->as.file.module_name);
            return 0;
        }

        if (!zt_path_suffix_matches(files->items[i].path, expected_suffix)) {
            zt_diag_list_add(diagnostics, ZT_DIAG_PROJECT_INVALID_NAMESPACE, zt_source_span_make(files->items[i].path, 1, 1, 1),
                "namespace '%s' must match source path suffix '%s'",
                root->as.file.module_name, expected_suffix);
            return 0;
        }

        for (import_index = 0; import_index < root->as.file.imports.count; import_index += 1) {
            const zt_ast_node *import_decl = root->as.file.imports.items[import_index];
            if (import_decl == NULL || import_decl->kind != ZT_AST_IMPORT_DECL) continue;
            if (zt_namespace_is_std(import_decl->as.import_decl.path)) continue;
            if (!zt_namespace_exists(files, import_decl->as.import_decl.path)) {
                zt_diag_list_add(diagnostics, ZT_DIAG_PROJECT_UNRESOLVED_IMPORT, import_decl->span,
                    "import '%s' was not found under source.root", import_decl->as.import_decl.path);
                return 0;
            }
        }
    }

    if (!zt_detect_import_cycles(files)) {
        zt_diag_list_add(diagnostics, ZT_DIAG_PROJECT_IMPORT_CYCLE, zt_source_span_unknown(), "import cycle detected");
        return 0;
    }

    return 1;
}

int zt_parse_project_sources(zt_driver_context *ctx, zt_project_source_file_list *files) {
    size_t i;

    if (files == NULL) return 0;

    for (i = 0; i < files->count; i += 1) {
        files->items[i].source_text = zt_read_file(files->items[i].path);
        if (files->items[i].source_text == NULL) return 0;

        files->items[i].parsed = zt_parse(&global_arena, &global_pool, 
            files->items[i].path,
            files->items[i].source_text,
            strlen(files->items[i].source_text));
        files->items[i].parsed_ready = 1;

        if (files->items[i].parsed.diagnostics.count != 0) {
            zt_print_diagnostics(ctx, "parser", &files->items[i].parsed.diagnostics);
            return 0;
        }

        free(files->items[i].source_text);
        files->items[i].source_text = NULL;
    }

    return 1;
}

zt_ast_node *zt_build_combined_project_ast(
        zt_project_source_file_list *files,
        const zt_project_manifest *manifest) {
    zt_ast_node *combined;
    size_t i;

    if (files == NULL || manifest == NULL) return NULL;

    combined = zt_ast_make(&global_arena, ZT_AST_FILE, zt_source_span_unknown());
    if (combined == NULL) return NULL;

    combined->as.file.module_name = zt_heap_strdup(zt_project_manifest_entry_namespace(manifest));
    combined->as.file.imports = zt_ast_node_list_make();
    combined->as.file.declarations = zt_ast_node_list_make();

    for (i = 0; i < files->count; i += 1) {
        zt_ast_node *root = files->items[i].parsed.root;
        const char *namespace_name;
        const char *prefix;
        size_t import_index;
        size_t decl_index;

        if (root == NULL || root->kind != ZT_AST_FILE) continue;

        namespace_name = root->as.file.module_name;
        prefix = strcmp(namespace_name, zt_project_manifest_entry_namespace(manifest)) == 0
            ? NULL
            : zt_alias_for_namespace(files, zt_project_manifest_entry_namespace(manifest), namespace_name);

        for (import_index = 0; import_index < root->as.file.imports.count; import_index += 1) {
            zt_ast_node_list_push(&global_arena, &combined->as.file.imports, root->as.file.imports.items[import_index]);
        }

        for (decl_index = 0; decl_index < root->as.file.declarations.count; decl_index += 1) {
            zt_ast_node *decl = root->as.file.declarations.items[decl_index];
            if (prefix != NULL && !zt_prefix_declaration_for_namespace(&global_pool, decl, prefix)) {
                
                return NULL;
            }
            zt_ast_node_list_push(&global_arena, &combined->as.file.declarations, decl);
        }

        root->as.file.imports.count = 0;
        root->as.file.declarations.count = 0;
    }

    return combined;
}
