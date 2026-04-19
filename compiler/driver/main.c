#include "compiler/utils/arena.h"
#include "compiler/utils/string_pool.h"
#include "compiler/frontend/parser/parser.h"
#include "compiler/hir/lowering/from_ast.h"
#include "compiler/project/ztproj.h"
#include "compiler/project/zdoc.h"
#include "compiler/semantic/binder/binder.h"
#include "compiler/semantic/types/checker.h"
#include "compiler/targets/c/emitter.h"
#include "compiler/zir/lowering/from_hir.h"
#include "compiler/zir/parser.h"
#include "compiler/zir/verifier.h"

#include "compiler/semantic/diagnostics/diagnostics.h"
#include "compiler/tooling/formatter.h"

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define ZT_MKDIR(path) _mkdir(path)
#define ZT_GETCWD(buffer, size) _getcwd((buffer), (int)(size))
#else
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>
#define ZT_MKDIR(path) mkdir((path), 0755)
#define ZT_GETCWD(buffer, size) getcwd((buffer), (size))
#endif

static int zt_ci_mode_enabled = 0;

static zt_arena global_arena;
static zt_string_pool global_pool;

static char *zt_read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    long file_size;
    char *buffer;

    if (file == NULL) {
        fprintf(stderr, "error: cannot open '%s'\n", path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        fprintf(stderr, "error: cannot seek '%s'\n", path);
        return NULL;
    }

    file_size = ftell(file);
    if (file_size < 0) {
        fclose(file);
        fprintf(stderr, "error: cannot read size of '%s'\n", path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        fprintf(stderr, "error: cannot seek '%s'\n", path);
        return NULL;
    }

    buffer = (char *)malloc((size_t)file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        fprintf(stderr, "error: out of memory reading '%s'\n", path);
        return NULL;
    }

    if (file_size > 0 && fread(buffer, 1, (size_t)file_size, file) != (size_t)file_size) {
        free(buffer);
        fclose(file);
        fprintf(stderr, "error: cannot read '%s'\n", path);
        return NULL;
    }

    buffer[file_size] = '\0';
    fclose(file);
    return buffer;
}

static int zt_write_file(const char *path, const char *content) {
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "error: cannot write '%s'\n", path);
        return 0;
    }

    if (content != NULL && fwrite(content, 1, strlen(content), file) != strlen(content)) {
        fclose(file);
        fprintf(stderr, "error: cannot write all content to '%s'\n", path);
        return 0;
    }

    fclose(file);
    return 1;
}

static int zt_copy_text(char *dest, size_t capacity, const char *text) {
    size_t length;

    if (dest == NULL || capacity == 0 || text == NULL) return 0;
    length = strlen(text);
    if (length + 1 > capacity) return 0;
    memcpy(dest, text, length + 1);
    return 1;
}

static char *zt_heap_strdup(const char *text) {
    size_t length;
    char *copy;

    if (text == NULL) return NULL;
    length = strlen(text);
    copy = (char *)malloc(length + 1);
    if (copy == NULL) return NULL;
    memcpy(copy, text, length + 1);
    return copy;
}

static int zt_path_has_extension(const char *path, const char *extension) {
    const char *dot;

    if (path == NULL || extension == NULL) return 0;
    dot = strrchr(path, '.');
    return dot != NULL && strcmp(dot, extension) == 0;
}

static int zt_path_is_dir(const char *path) {
    struct stat info;

    if (path == NULL || stat(path, &info) != 0) return 0;

#ifdef _WIN32
    return (info.st_mode & S_IFDIR) != 0;
#else
    return S_ISDIR(info.st_mode);
#endif
}

static int zt_path_is_file(const char *path) {
    struct stat info;

    if (path == NULL || stat(path, &info) != 0) return 0;

#ifdef _WIN32
    return (info.st_mode & S_IFDIR) == 0;
#else
    return !S_ISDIR(info.st_mode);
#endif
}

static int zt_join_path(char *dest, size_t capacity, const char *left, const char *right) {
    size_t left_len;
    size_t right_len;
    int needs_sep;

    if (dest == NULL || capacity == 0 || left == NULL || right == NULL) return 0;

    left_len = strlen(left);
    right_len = strlen(right);
    needs_sep = left_len > 0 && left[left_len - 1] != '/' && left[left_len - 1] != '\\';

    if (left_len + (needs_sep ? 1u : 0u) + right_len + 1u > capacity) return 0;

    memcpy(dest, left, left_len);
    dest[left_len] = '\0';

    if (needs_sep) {
        dest[left_len] = '/';
        dest[left_len + 1u] = '\0';
        left_len += 1u;
    }

    memcpy(dest + left_len, right, right_len + 1u);
    return 1;
}

static int zt_dirname(char *dest, size_t capacity, const char *path) {
    const char *slash;
    const char *backslash;
    const char *last_sep;
    size_t length;

    if (dest == NULL || capacity == 0 || path == NULL) return 0;

    slash = strrchr(path, '/');
    backslash = strrchr(path, '\\');
    last_sep = slash;
    if (backslash != NULL && (last_sep == NULL || backslash > last_sep)) {
        last_sep = backslash;
    }

    if (last_sep == NULL) {
        return zt_copy_text(dest, capacity, ".");
    }

    length = (size_t)(last_sep - path);
    if (length == 0) length = 1;
    if (length + 1u > capacity) return 0;

    memcpy(dest, path, length);
    dest[length] = '\0';
    return 1;
}

static int zt_get_current_dir(char *dest, size_t capacity) {
    if (dest == NULL || capacity == 0) return 0;
    if (ZT_GETCWD(dest, capacity) == NULL) return 0;
    return 1;
}

static int zt_find_project_root_from_cwd(char *project_root, size_t capacity) {
    char current[512];
    char parent[512];
    char manifest_path[768];
    size_t guard;

    if (project_root == NULL || capacity == 0) return 0;
    if (!zt_get_current_dir(current, sizeof(current))) return 0;

    for (guard = 0; guard < 256; guard += 1) {
        if (zt_join_path(manifest_path, sizeof(manifest_path), current, "zenith.ztproj") &&
                zt_path_is_file(manifest_path)) {
            return zt_copy_text(project_root, capacity, current);
        }

        if (!zt_dirname(parent, sizeof(parent), current)) return 0;
        if (strcmp(parent, current) == 0) break;
        if (!zt_copy_text(current, sizeof(current), parent)) return 0;
    }

    return 0;
}

static int zt_replace_extension(char *dest, size_t capacity, const char *path, const char *extension) {
    const char *dot;
    size_t base_len;
    size_t ext_len;

    if (dest == NULL || capacity == 0 || path == NULL || extension == NULL) return 0;

    dot = strrchr(path, '.');
    base_len = dot != NULL ? (size_t)(dot - path) : strlen(path);
    ext_len = strlen(extension);

    if (base_len + ext_len + 1u > capacity) return 0;

    memcpy(dest, path, base_len);
    memcpy(dest + base_len, extension, ext_len + 1u);
    return 1;
}

static int zt_make_dirs(const char *path) {
    char buffer[512];
    char *cursor;

    if (path == NULL || path[0] == '\0') return 0;
    if (zt_path_is_dir(path)) return 1;
    if (!zt_copy_text(buffer, sizeof(buffer), path)) return 0;

    for (cursor = buffer; *cursor != '\0'; cursor += 1) {
        if (*cursor == '/' || *cursor == '\\') {
            char saved = *cursor;

            if (cursor == buffer) continue;
            if (cursor == buffer + 2 && buffer[1] == ':') continue;

            *cursor = '\0';
            if (buffer[0] != '\0' && !zt_path_is_dir(buffer)) {
                if (ZT_MKDIR(buffer) != 0 && errno != EEXIST) {
                    *cursor = saved;
                    return 0;
                }
            }
            *cursor = saved;
        }
    }

    if (!zt_path_is_dir(buffer)) {
        if (ZT_MKDIR(buffer) != 0 && errno != EEXIST) return 0;
    }

    return 1;
}

typedef struct zt_project_source_file {
    char path[512];
    char *source_text;
    zt_parser_result parsed;
    int parsed_ready;
} zt_project_source_file;

typedef struct zt_project_source_file_list {
    zt_project_source_file *items;
    size_t count;
    size_t capacity;
} zt_project_source_file_list;

static void zt_project_source_file_list_init(zt_project_source_file_list *list) {
    if (list == NULL) return;
    memset(list, 0, sizeof(*list));
}

static void zt_project_source_file_list_dispose(zt_project_source_file_list *list) {
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

static int zt_project_source_file_list_push(zt_project_source_file_list *list, const char *path) {
    zt_project_source_file *new_items;
    size_t new_capacity;

    if (list == NULL || path == NULL) return 0;
    if (list->count >= list->capacity) {
        new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        new_items = (zt_project_source_file *)realloc(list->items, new_capacity * sizeof(zt_project_source_file));
        if (new_items == NULL) return 0;
        list->items = new_items;
        list->capacity = new_capacity;
    }

    memset(&list->items[list->count], 0, sizeof(list->items[list->count]));
    if (!zt_copy_text(list->items[list->count].path, sizeof(list->items[list->count].path), path)) {
        return 0;
    }
    list->count += 1;
    return 1;
}

static int zt_project_discover_zt_files(const char *directory, zt_project_source_file_list *files) {
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

static int zt_namespace_to_relative_path(const char *namespace_name, char *dest, size_t capacity) {
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

static int zt_path_char_equal(char left, char right) {
    if ((left == '/' || left == '\\') && (right == '/' || right == '\\')) return 1;
#ifdef _WIN32
    return tolower((unsigned char)left) == tolower((unsigned char)right);
#else
    return left == right;
#endif
}

static int zt_path_suffix_matches(const char *path, const char *suffix) {
    size_t path_len;
    size_t suffix_len;
    size_t offset;
    size_t i;

    if (path == NULL || suffix == NULL) return 0;

    path_len = strlen(path);
    suffix_len = strlen(suffix);
    if (suffix_len > path_len) return 0;
    offset = path_len - suffix_len;

    if (offset > 0 && path[offset - 1] != '/' && path[offset - 1] != '\\') {
        return 0;
    }

    for (i = 0; i < suffix_len; i += 1) {
        if (!zt_path_char_equal(path[offset + i], suffix[i])) return 0;
    }

    return 1;
}

static const char *zt_last_namespace_segment(const char *namespace_name) {
    const char *dot;
    if (namespace_name == NULL) return NULL;
    dot = strrchr(namespace_name, '.');
    return dot != NULL ? dot + 1 : namespace_name;
}

static const zt_ast_node *zt_find_entry_root(const zt_project_source_file_list *files, const char *entry_namespace) {
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

static int zt_namespace_exists(const zt_project_source_file_list *files, const char *namespace_name) {
    return zt_find_entry_root(files, namespace_name) != NULL;
}

static int zt_namespace_is_std(const char *namespace_name) {
    return namespace_name != NULL && strncmp(namespace_name, "std.", 4) == 0;
}

static size_t zt_find_namespace_index(const zt_project_source_file_list *files, const char *namespace_name) {
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

static int zt_detect_import_cycles(const zt_project_source_file_list *files) {
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

static int zt_validate_source_namespaces(
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

static int zt_parse_project_sources(zt_project_source_file_list *files) {
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
            zt_diag_render_detailed_list(stderr, "parser", &files->items[i].parsed.diagnostics);
            return 0;
        }

        free(files->items[i].source_text);
        files->items[i].source_text = NULL;
    }

    return 1;
}

static zt_ast_node *zt_build_combined_project_ast(
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


static void zt_print_usage(const char *program) {
    fprintf(stderr, "Usage: %s <command> [input] [options]\n", program);
    fprintf(stderr, "\n");
    fprintf(stderr, "Canonical commands:\n");
    fprintf(stderr, "  check [project|zenith.ztproj]         Validate project pipeline\n");
    fprintf(stderr, "  build [project|zenith.ztproj] [-o <output>]\n");
    fprintf(stderr, "                                        Build app executable with gcc\n");
    fprintf(stderr, "  run [project|zenith.ztproj] [-o <output>]\n");
    fprintf(stderr, "                                        Build and execute app executable\n");
    fprintf(stderr, "  test [project|zenith.ztproj]          Bootstrap alias to check\n");
    fprintf(stderr, "  fmt [project|zenith.ztproj] [--check] Bootstrap formatter gate\n");
    fprintf(stderr, "  doc check [project|zenith.ztproj]     Validate .zdoc files and links\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Bootstrap/compat commands:\n");
    fprintf(stderr, "  project-info [project|zenith.ztproj]  Load manifest and print project info\n");
    fprintf(stderr, "  verify [project|zenith.ztproj|file.zir]\n");
    fprintf(stderr, "  emit-c [project|zenith.ztproj|file.zir]\n");
    fprintf(stderr, "  build [file.zir] [-o <output>] [--run]\n");
    fprintf(stderr, "  doc-check [project|zenith.ztproj]\n");
    fprintf(stderr, "  parse <file.zir>                      Parse ZIR and print module info\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --run                Execute the compiled binary after build\n");
    fprintf(stderr, "  -o <output>          Output executable path for build\n");
    fprintf(stderr, "  --check              Check-only mode for fmt\n");
    fprintf(stderr, "  --ci                 Use short deterministic diagnostic/output mode\n");
}

static void zt_print_diagnostics(const char *stage, const zt_diag_list *diagnostics);

static zt_diag_code zt_diag_code_from_project_error(zt_project_error_code code) {
    switch (code) {
        case ZT_PROJECT_INVALID_INPUT: return ZT_DIAG_PROJECT_INVALID_INPUT;
        case ZT_PROJECT_INVALID_SECTION: return ZT_DIAG_PROJECT_INVALID_SECTION;
        case ZT_PROJECT_UNKNOWN_KEY: return ZT_DIAG_PROJECT_UNKNOWN_KEY;
        case ZT_PROJECT_INVALID_ASSIGNMENT: return ZT_DIAG_PROJECT_INVALID_ASSIGNMENT;
        case ZT_PROJECT_INVALID_STRING: return ZT_DIAG_PROJECT_INVALID_STRING;
        case ZT_PROJECT_MISSING_FIELD: return ZT_DIAG_PROJECT_MISSING_FIELD;
        case ZT_PROJECT_INVALID_KIND: return ZT_DIAG_PROJECT_INVALID_KIND;
        case ZT_PROJECT_INVALID_TARGET: return ZT_DIAG_PROJECT_INVALID_TARGET;
        case ZT_PROJECT_INVALID_PROFILE: return ZT_DIAG_PROJECT_INVALID_PROFILE;
        case ZT_PROJECT_PATH_TOO_LONG: return ZT_DIAG_PROJECT_PATH_TOO_LONG;
        case ZT_PROJECT_TOO_MANY_DEPENDENCIES: return ZT_DIAG_PROJECT_TOO_MANY_DEPENDENCIES;
        case ZT_PROJECT_OK: return ZT_DIAG_PROJECT_ERROR;
        default: return ZT_DIAG_PROJECT_ERROR;
    }
}

static void zt_print_project_parse_error(const char *manifest_path, const zt_project_parse_result *project) {
    zt_diag_list diagnostics = zt_diag_list_make();
    const char *message = "unable to read project manifest";
    zt_diag_code code = ZT_DIAG_PROJECT_INVALID_INPUT;
    const char *source_name = manifest_path != NULL && manifest_path[0] != '\0'
        ? manifest_path
        : "<project>";
    size_t line = 1;
    zt_source_span span;

    if (project != NULL) {
        if (project->code != ZT_PROJECT_OK) {
            code = zt_diag_code_from_project_error(project->code);
        }
        if (project->message[0] != '\0') {
            message = project->message;
        }
        if (project->line_number > 0) {
            line = (size_t)project->line_number;
        }
    }

    span = zt_source_span_make(source_name, line, 1, 1);
    zt_diag_list_add(&diagnostics, code, span, "%s", message);
    zt_print_diagnostics("project", &diagnostics);
    zt_diag_list_dispose(&diagnostics);
}

static void zt_print_diagnostics(const char *stage, const zt_diag_list *diagnostics) {
    if (zt_ci_mode_enabled) {
        zt_diag_render_ci_list(stderr, stage, diagnostics);
    } else {
        zt_diag_render_detailed_list(stderr, stage, diagnostics);
    }
}

static void zt_print_single_diag(
        const char *stage,
        zt_diag_code code,
        zt_source_span span,
        const char *format,
        ...) {
    zt_diag_list diagnostics = zt_diag_list_make();
    char message[512];
    va_list args;

    if (format == NULL) return;

    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    zt_diag_list_add(&diagnostics, code, span, "%s", message);
    zt_print_diagnostics(stage, &diagnostics);
    zt_diag_list_dispose(&diagnostics);
}

static zt_diag_code zt_diag_code_from_zir_parse_error(zir_parse_error_code code) {
    switch (code) {
        case ZIR_PARSE_INVALID_INPUT:
        case ZIR_PARSE_EXPECTED_MODULE:
        case ZIR_PARSE_EXPECTED_FUNC:
        case ZIR_PARSE_EXPECTED_BLOCK:
        case ZIR_PARSE_EXPECTED_ARROW:
        case ZIR_PARSE_EXPECTED_COLON:
        case ZIR_PARSE_EXPECTED_LPAREN:
        case ZIR_PARSE_EXPECTED_RPAREN:
        case ZIR_PARSE_EXPECTED_IDENTIFIER:
        case ZIR_PARSE_TOO_MANY_PARAMS:
        case ZIR_PARSE_TOO_MANY_BLOCKS:
        case ZIR_PARSE_TOO_MANY_INSTRUCTIONS:
        case ZIR_PARSE_TOO_MANY_FUNCTIONS:
        case ZIR_PARSE_INVALID_TERMINATOR:
        case ZIR_PARSE_UNKNOWN_LINE:
        case ZIR_PARSE_OUT_OF_MEMORY:
        case ZIR_PARSE_OK:
        default:
            return ZT_DIAG_ZIR_PARSE_ERROR;
    }
}

static zt_diag_code zt_diag_code_from_zir_verifier(zir_verifier_code code) {
    switch (code) {
        case ZIR_VERIFIER_INVALID_INPUT:
        case ZIR_VERIFIER_INVALID_TYPE_ANY:
        case ZIR_VERIFIER_INVALID_TARGET_LEAK:
        case ZIR_VERIFIER_USE_BEFORE_DEFINITION:
        case ZIR_VERIFIER_UNKNOWN_BLOCK:
        case ZIR_VERIFIER_INVALID_TERMINATOR:
        case ZIR_VERIFIER_DUPLICATE_BLOCK_LABEL:
        case ZIR_VERIFIER_INVALID_RETURN:
        case ZIR_VERIFIER_OK:
        default:
            return ZT_DIAG_ZIR_VERIFY_ERROR;
    }
}

static zt_diag_code zt_diag_code_from_c_emit_error(c_emit_error_code code) {
    switch (code) {
        case C_EMIT_INVALID_INPUT:
        case C_EMIT_UNSUPPORTED_TYPE:
        case C_EMIT_UNSUPPORTED_EXPR:
        case C_EMIT_INVALID_MAIN_SIGNATURE:
        case C_EMIT_OK:
        default:
            return ZT_DIAG_BACKEND_C_EMIT_ERROR;
    }
}

static zt_diag_code zt_diag_code_from_zdoc_code(zt_zdoc_code code) {
    switch (code) {
        case ZT_ZDOC_MALFORMED_BLOCK: return ZT_DIAG_DOC_MALFORMED_BLOCK;
        case ZT_ZDOC_MISSING_TARGET: return ZT_DIAG_DOC_MISSING_TARGET;
        case ZT_ZDOC_MISSING_PAGE: return ZT_DIAG_DOC_MISSING_PAGE;
        case ZT_ZDOC_UNRESOLVED_TARGET: return ZT_DIAG_DOC_UNRESOLVED_TARGET;
        case ZT_ZDOC_UNRESOLVED_LINK: return ZT_DIAG_DOC_UNRESOLVED_LINK;
        case ZT_ZDOC_INVALID_GUIDE_TAG: return ZT_DIAG_DOC_INVALID_GUIDE_TAG;
        case ZT_ZDOC_INVALID_PAIRED_TAG: return ZT_DIAG_DOC_INVALID_PAIRED_TAG;
        case ZT_ZDOC_MISSING_PUBLIC_DOC: return ZT_DIAG_DOC_MISSING_PUBLIC_DOC;
        default: return ZT_DIAG_DOC_MALFORMED_BLOCK;
    }
}

static void zt_append_zdoc_diagnostics(zt_diag_list *dest, const zt_zdoc_diagnostic_list *diagnostics) {
    size_t i;

    if (dest == NULL || diagnostics == NULL) return;

    for (i = 0; i < diagnostics->count; i += 1) {
        const zt_zdoc_diagnostic *diag = &diagnostics->items[i];
        zt_diag_severity severity =
            diag->severity == ZT_ZDOC_WARNING ? ZT_DIAG_SEVERITY_WARNING : ZT_DIAG_SEVERITY_ERROR;
        zt_source_span span = zt_source_span_make(
            diag->path[0] != '\0' ? diag->path : "<zdoc>",
            diag->line > 0 ? diag->line : 1,
            diag->column > 0 ? diag->column : 1,
            diag->column > 0 ? diag->column : 1);
        zt_diag_list_add_severity(
            dest,
            zt_diag_code_from_zdoc_code(diag->code),
            severity,
            span,
            "%s",
            diag->message[0] != '\0' ? diag->message : "invalid zdoc");
    }
}

static int zt_resolve_project_paths(
        const char *input_path,
        char *project_root,
        size_t project_root_capacity,
        char *manifest_path,
        size_t manifest_path_capacity) {
    if (input_path == NULL || input_path[0] == '\0') {
        if (!zt_find_project_root_from_cwd(project_root, project_root_capacity)) {
            zt_print_single_diag(
                "project",
                ZT_DIAG_PROJECT_INVALID_INPUT,
                zt_source_span_unknown(),
                "could not find zenith.ztproj from current directory");
            return 0;
        }

        if (!zt_join_path(manifest_path, manifest_path_capacity, project_root, "zenith.ztproj")) {
            zt_print_single_diag(
                "project",
                ZT_DIAG_PROJECT_PATH_TOO_LONG,
                zt_source_span_unknown(),
                "project manifest path is too long");
            return 0;
        }

        return 1;
    }

    if (zt_path_is_dir(input_path)) {
        if (!zt_copy_text(project_root, project_root_capacity, input_path)) {
            zt_print_single_diag(
                "project",
                ZT_DIAG_PROJECT_PATH_TOO_LONG,
                zt_source_span_unknown(),
                "project root path is too long");
            return 0;
        }

        if (!zt_join_path(manifest_path, manifest_path_capacity, project_root, "zenith.ztproj")) {
            zt_print_single_diag(
                "project",
                ZT_DIAG_PROJECT_PATH_TOO_LONG,
                zt_source_span_unknown(),
                "project manifest path is too long");
            return 0;
        }

        return 1;
    }

    if (zt_path_has_extension(input_path, ".ztproj")) {
        if (!zt_copy_text(manifest_path, manifest_path_capacity, input_path)) {
            zt_print_single_diag(
                "project",
                ZT_DIAG_PROJECT_PATH_TOO_LONG,
                zt_source_span_unknown(),
                "project manifest path is too long");
            return 0;
        }

        if (!zt_dirname(project_root, project_root_capacity, input_path)) {
            zt_print_single_diag(
                "project",
                ZT_DIAG_PROJECT_PATH_TOO_LONG,
                zt_source_span_unknown(),
                "project root path is too long");
            return 0;
        }

        return 1;
    }

    zt_print_single_diag(
        "project",
        ZT_DIAG_PROJECT_INVALID_INPUT,
        zt_source_span_unknown(),
        "project input must be a directory or a .ztproj file");
    return 0;
}

typedef struct zt_project_compile_result {
    zt_project_manifest manifest;
    char project_root[512];
    char manifest_path[512];
    char entry_path[512];
    zt_project_source_file_list source_files;
    zir_lower_result zir;
    int has_zir;
} zt_project_compile_result;

static void zt_project_compile_result_init(zt_project_compile_result *result) {
    memset(result, 0, sizeof(*result));
}

static void zt_project_compile_result_dispose(zt_project_compile_result *result) {
    if (result == NULL) return;
    if (result->has_zir) {
        zir_lower_result_dispose(&result->zir);
        result->has_zir = 0;
    }
    zt_project_source_file_list_dispose(&result->source_files);
}

static int zt_compile_project(const char *input_path, zt_project_compile_result *out) {
    zt_project_parse_result project;
    char source_root_path[512];
    zt_ast_node *program_root = NULL;
    zt_bind_result bound;
    zt_check_result checked;
    zt_hir_lower_result hir = { 0 };
    zir_lower_result zir;
    zir_verifier_result verifier;
    int program_ready = 0;
    int bound_ready = 0;
    int checked_ready = 0;
    int hir_ready = 0;
    int zir_ready = 0;

    zt_project_compile_result_init(out);
    zt_project_source_file_list_init(&out->source_files);

    if (!zt_resolve_project_paths(
            input_path,
            out->project_root,
            sizeof(out->project_root),
            out->manifest_path,
            sizeof(out->manifest_path))) {
        return 0;
    }

    if (!zt_project_load_file(out->manifest_path, &project)) {
        zt_print_project_parse_error(out->manifest_path, &project);
        return 0;
    }

    out->manifest = project.manifest;

    if (zt_project_manifest_kind(&out->manifest) == ZT_PROJECT_KIND_APP &&
            !zt_project_resolve_entry_source_path(
                &out->manifest,
                out->project_root,
                out->entry_path,
                sizeof(out->entry_path))) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_PATH_TOO_LONG, zt_source_span_unknown(), "app.entry source path is too long");
        return 0;
    }

    if (!zt_join_path(source_root_path, sizeof(source_root_path), out->project_root, out->manifest.source_root)) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_PATH_TOO_LONG, zt_source_span_unknown(), "source.root path is too long");
        goto fail;
    }

    if (!zt_path_is_dir(source_root_path)) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_INVALID_INPUT, zt_source_span_unknown(), "source.root '%s' is not a directory", source_root_path);
        goto fail;
    }

    if (!zt_project_discover_zt_files(source_root_path, &out->source_files)) {
        goto fail;
    }

    /* Load only the specific stdlib modules that are actually imported */
    {
        const char *stdlib_base = getenv("ZENITH_HOME");
        char default_base[768];
        char import_buf[256];
        size_t si;

        if (stdlib_base == NULL) {
            strcpy(default_base, "stdlib");
            stdlib_base = default_base;
        } else {
            if (!zt_join_path(default_base, sizeof(default_base), stdlib_base, "stdlib")) {
                strcpy(default_base, "stdlib");
            }
            stdlib_base = default_base;
        }

        for (si = 0; si < out->source_files.count; si++) {
            const char *src_path = out->source_files.items[si].path;
            FILE *sf = fopen(src_path, "r");
            if (sf != NULL) {
                char line[512];
                while (fgets(line, sizeof(line), sf) != NULL) {
                    /* Look for: import std.X or import std.X as Y */
                    const char *imp = strstr(line, "import std.");
                    if (imp != NULL) {
                        const char *mod_start = imp + 7; /* skip "import " -> "std.X..." */
                        const char *end = mod_start;
                        char mod_path[768];
                        size_t j;
                        int already_loaded;

                        while (*end && *end != ' ' && *end != '\t' && *end != '\n' && *end != '\r') end++;
                        if ((size_t)(end - mod_start) < sizeof(import_buf)) {
                            memcpy(import_buf, mod_start, end - mod_start);
                            import_buf[end - mod_start] = '\0';

                            /* Convert std.io -> stdlib_base/std/io.zt */
                            snprintf(mod_path, sizeof(mod_path), "%s", stdlib_base);
                            for (j = 0; import_buf[j]; j++) {
                                if (import_buf[j] == '.') import_buf[j] = '/';
                            }
                            if (strlen(mod_path) + 1 + strlen(import_buf) + 3 < sizeof(mod_path)) {
                                strcat(mod_path, "/");
                                strcat(mod_path, import_buf);
                                strcat(mod_path, ".zt");
                            }

                            /* Check if already loaded */
                            already_loaded = 0;
                            for (j = 0; j < out->source_files.count; j++) {
                                if (strcmp(out->source_files.items[j].path, mod_path) == 0) {
                                    already_loaded = 1;
                                    break;
                                }
                            }

                            if (!already_loaded && zt_path_is_file(mod_path)) {
                                zt_project_source_file_list_push(&out->source_files, mod_path);
                            }
                        }
                    }
                }
                fclose(sf);
            }
        }
    }

    if (out->source_files.count == 0) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_ERROR, zt_source_span_unknown(), "source.root '%s' contains no .zt files", source_root_path);
        goto fail;
    }

    if (!zt_parse_project_sources(&out->source_files)) {
        goto fail;
    }

    zt_diag_list proj_diags = zt_diag_list_make();
    if (!zt_validate_source_namespaces(&out->source_files, &out->manifest, &proj_diags)) {
        zt_print_diagnostics("project", &proj_diags);
        zt_diag_list_dispose(&proj_diags);
        goto fail;
    }
    zt_diag_list_dispose(&proj_diags);

    program_root = zt_build_combined_project_ast(&out->source_files, &out->manifest);
    if (program_root == NULL) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_ERROR, zt_source_span_unknown(), "unable to aggregate project sources");
        goto fail;
    }
    program_ready = 1;

    bound = zt_bind_file(program_root);
    bound_ready = 1;
    if (bound.diagnostics.count != 0) {
        zt_print_diagnostics("binding", &bound.diagnostics);
        goto fail;
    }

    checked = zt_check_file(program_root);
    checked_ready = 1;
    if (checked.diagnostics.count != 0) {
        zt_print_diagnostics("type", &checked.diagnostics);
        goto fail;
    }

    hir = zt_lower_ast_to_hir(program_root);
    hir_ready = 1;
    if (hir.diagnostics.count != 0 || hir.module == NULL) {
        zt_print_diagnostics("hir", &hir.diagnostics);
        goto fail;
    }

    zir = zir_lower_hir_to_zir(hir.module);
    zir_ready = 1;
    if (zir.diagnostics.count != 0) {
        zt_print_diagnostics("zir", &zir.diagnostics);
        goto fail;
    }

    if (!zir_verify_module(&zir.module, &verifier)) {
        zt_source_span span = zt_source_span_make("<zir>", 1, 1, 1);
        if (verifier.has_span) {
            const char *source_name = verifier.source_name != NULL && verifier.source_name[0] != '\0'
                ? verifier.source_name
                : "<zir>";
            size_t line = verifier.line > 0 ? verifier.line : 1;
            size_t column = verifier.column > 0 ? verifier.column : 1;
            span = zt_source_span_make(source_name, line, column, column);
        }
        zt_print_single_diag(
            "zir.verify",
            zt_diag_code_from_zir_verifier(verifier.code),
            span,
            "%s",
            verifier.message[0] != '\0' ? verifier.message : "ZIR verification failed");
        goto fail;
    }

    out->zir = zir;
    out->has_zir = 1;
    zir_ready = 0;

    if (hir_ready) zt_hir_lower_result_dispose(&hir);
    if (checked_ready) zt_check_result_dispose(&checked);
    if (bound_ready) zt_bind_result_dispose(&bound);
    if (program_ready) 
    return 1;

fail:
    if (zir_ready) zir_lower_result_dispose(&zir);
    if (hir_ready) zt_hir_lower_result_dispose(&hir);
    if (checked_ready) zt_check_result_dispose(&checked);
    if (bound_ready) zt_bind_result_dispose(&bound);
    if (program_ready) 
    zt_project_source_file_list_dispose(&out->source_files);
    return 0;
}

static int zt_emit_module_to_c(
        const zir_module *module_decl,
        c_emitter *emitter,
        c_emit_result *out_error) {
    c_emit_result emit_result;

    c_emitter_init(emitter);
    c_emit_result_init(&emit_result);

    if (!c_emitter_emit_module(emitter, module_decl, &emit_result)) {
        if (out_error != NULL) {
            *out_error = emit_result;
        }
        c_emitter_dispose(emitter);
        return 0;
    }

    if (out_error != NULL) {
        out_error->ok = 1;
        out_error->code = C_EMIT_OK;
        out_error->message[0] = '\0';
    }

    return 1;
}

static int zt_compile_c_file(const char *c_path, const char *exe_path) {
    char compile_cmd[2048];
    int compile_result;

    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -Wall -Wextra -I. -o \"%s\" \"%s\" runtime/c/zenith_rt.c",
             exe_path, c_path);

    if (!zt_ci_mode_enabled) {
        printf("compiling: %s\n", compile_cmd);
    }
    compile_result = system(compile_cmd);
    if (compile_result != 0) {
        zt_print_single_diag(
            "backend.c.emit",
            ZT_DIAG_BACKEND_C_EMIT_ERROR,
            zt_source_span_make(c_path != NULL ? c_path : "<c>", 1, 1, 1),
            "native compilation failed with code %d",
            compile_result);
        return 0;
    }

    if (!zt_ci_mode_enabled) {
        printf("built: %s\n", exe_path);
    }
    return 1;
}

static void zt_normalize_system_path(const char *input, char *output, size_t output_size) {
    size_t i = 0;

    if (output_size == 0) return;

    while (i + 1 < output_size && input[i] != '\0') {
#ifdef _WIN32
        output[i] = (input[i] == '/') ? '\\' : input[i];
#else
        output[i] = input[i];
#endif
        i += 1;
    }

    output[i] = '\0';
}

static int zt_decode_process_exit(int status) {
#ifdef _WIN32
    return status;
#else
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return status;
#endif
}

static int zt_run_executable(const char *exe_path) {
    char run_cmd[1024];
    char system_path[1000];
    int run_result;
    int exit_code;

    zt_normalize_system_path(exe_path, system_path, sizeof(system_path));

    snprintf(run_cmd, sizeof(run_cmd), "\"%s\"", system_path);
    if (!zt_ci_mode_enabled) {
        printf("running: %s\n", system_path);
    }
    run_result = system(run_cmd);
    if (run_result < 0) {
        zt_print_single_diag(
            "runtime",
            ZT_DIAG_PROJECT_INVALID_INPUT,
            zt_source_span_unknown(),
            "failed to execute '%s'",
            system_path);
        return -1;
    }

    exit_code = zt_decode_process_exit(run_result);
    if (!zt_ci_mode_enabled) {
        printf("exit code: %d\n", exit_code);
    }
    return exit_code;
}
static int zt_collect_project_sources(
        const char *input_path,
        zt_project_manifest *manifest,
        char *project_root,
        size_t project_root_capacity,
        zt_project_source_file_list *source_files) {
    zt_project_parse_result project;
    char manifest_path[512];
    char source_root_path[512];

    if (manifest == NULL || project_root == NULL || source_files == NULL) return 0;

    zt_project_source_file_list_init(source_files);

    if (!zt_resolve_project_paths(
            input_path,
            project_root,
            project_root_capacity,
            manifest_path,
            sizeof(manifest_path))) {
        return 0;
    }

    if (!zt_project_load_file(manifest_path, &project)) {
        zt_print_project_parse_error(manifest_path, &project);
        return 0;
    }

    *manifest = project.manifest;

    if (!zt_join_path(source_root_path, sizeof(source_root_path), project_root, manifest->source_root)) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_PATH_TOO_LONG, zt_source_span_unknown(), "source.root path is too long");
        return 0;
    }

    if (!zt_path_is_dir(source_root_path)) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_INVALID_INPUT, zt_source_span_unknown(), "source.root '%s' is not a directory", source_root_path);
        return 0;
    }

    if (!zt_project_discover_zt_files(source_root_path, source_files)) {
        zt_project_source_file_list_dispose(source_files);
        return 0;
    }

    if (source_files->count == 0) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_ERROR, zt_source_span_unknown(), "source.root '%s' contains no .zt files", source_root_path);
        zt_project_source_file_list_dispose(source_files);
        return 0;
    }

    if (!zt_parse_project_sources(source_files)) {
        zt_project_source_file_list_dispose(source_files);
        return 0;
    }

    {
        zt_diag_list proj_diags = zt_diag_list_make();
        if (!zt_validate_source_namespaces(source_files, manifest, &proj_diags)) {
            zt_print_diagnostics("project", &proj_diags);
            zt_diag_list_dispose(&proj_diags);
            zt_project_source_file_list_dispose(source_files);
            return 0;
        }
        zt_diag_list_dispose(&proj_diags);
    }

    return 1;
}

static int zt_handle_doc_check(const char *input_path) {
    zt_project_manifest manifest;
    char project_root[512];
    zt_project_source_file_list source_files;
    zt_zdoc_source_unit *units = NULL;
    zt_zdoc_diagnostic_list diagnostics;
    zt_diag_list rendered = zt_diag_list_make();
    size_t error_count;
    size_t warning_count;
    size_t i;

    if (!zt_collect_project_sources(
            input_path,
            &manifest,
            project_root,
            sizeof(project_root),
            &source_files)) {
        return 1;
    }

    units = (zt_zdoc_source_unit *)calloc(source_files.count > 0 ? source_files.count : 1, sizeof(zt_zdoc_source_unit));
    if (units == NULL) {
        zt_print_single_diag(
            "doc",
            ZT_DIAG_DOC_MALFORMED_BLOCK,
            zt_source_span_unknown(),
            "out of memory while preparing zdoc check");
        zt_project_source_file_list_dispose(&source_files);
        return 1;
    }

    for (i = 0; i < source_files.count; i += 1) {
        units[i].source_path = source_files.items[i].path;
        units[i].root = source_files.items[i].parsed.root;
    }

    zt_zdoc_diagnostic_list_init(&diagnostics);
    if (!zt_zdoc_check_project(
            project_root,
            manifest.source_root,
            manifest.zdoc_root,
            units,
            source_files.count,
            &diagnostics)) {
        zt_print_single_diag(
            "doc",
            ZT_DIAG_DOC_MALFORMED_BLOCK,
            zt_source_span_unknown(),
            "zdoc checker internal failure");
        zt_zdoc_diagnostic_list_dispose(&diagnostics);
        free(units);
        zt_project_source_file_list_dispose(&source_files);
        return 1;
    }

    zt_append_zdoc_diagnostics(&rendered, &diagnostics);
    zt_print_diagnostics("doc", &rendered);

    error_count = zt_zdoc_diagnostic_error_count(&diagnostics);
    warning_count = zt_zdoc_diagnostic_warning_count(&diagnostics);

    if (error_count == 0) {
        if (warning_count == 0) {
            printf("doc check ok\n");
        } else {
            printf("doc check ok (%zu warnings)\n", warning_count);
        }
    }

    zt_diag_list_dispose(&rendered);
    zt_zdoc_diagnostic_list_dispose(&diagnostics);
    free(units);
    zt_project_source_file_list_dispose(&source_files);
    return error_count == 0 ? 0 : 1;
}

static int zt_handle_fmt(const char *input_path, int check_only) {
    zt_project_manifest manifest;
    char project_root[512];
    zt_project_source_file_list source_files;
    int diff_count = 0;
    size_t i;

    if (!zt_collect_project_sources(
            input_path,
            &manifest,
            project_root,
            sizeof(project_root),
            &source_files)) {
        return 1;
    }

    for (i = 0; i < source_files.count; i++) {
        char *formatted = zt_format_node_to_string(source_files.items[i].parsed.root);
        if (!formatted) continue;

        if (check_only) {
            char *original = zt_read_file(source_files.items[i].path);
            if (original) {
                if (strcmp(original, formatted) != 0) {
                    diff_count++;
                    if (!zt_ci_mode_enabled) {
                        printf("diff: %s\n", source_files.items[i].path);
                    }
                }
                free(original);
            }
        } else {
            char *original = zt_read_file(source_files.items[i].path);
            if (original) {
                if (strcmp(original, formatted) != 0) {
                    zt_write_file(source_files.items[i].path, formatted);
                    if (!zt_ci_mode_enabled) {
                        printf("formatted: %s\n", source_files.items[i].path);
                    }
                }
                free(original);
            }
        }
        free(formatted);
    }

    zt_project_source_file_list_dispose(&source_files);

    if (check_only) {
        if (diff_count > 0) {
            printf("%s", zt_ci_mode_enabled ? "fmt check failed\n" : "fmt check failed: differences found\n");
            return 1;
        } else {
            printf("%s", zt_ci_mode_enabled ? "fmt ok\n" : "fmt check ok\n");
        }
    } else {
        printf("%s", zt_ci_mode_enabled ? "fmt ok\n" : "fmt ok\n");
    }

    return 0;
}

static int zt_handle_project_info(const char *input_path) {
    zt_project_parse_result project;
    char project_root[512];
    char manifest_path[512];
    char entry_path[512];
    zt_project_kind kind;

    entry_path[0] = '\0';

    if (!zt_resolve_project_paths(input_path, project_root, sizeof(project_root), manifest_path, sizeof(manifest_path))) {
        return 1;
    }

    if (!zt_project_load_file(manifest_path, &project)) {
        zt_print_project_parse_error(manifest_path, &project);
        return 1;
    }

    kind = zt_project_manifest_kind(&project.manifest);
    if (kind == ZT_PROJECT_KIND_APP &&
            !zt_project_resolve_entry_source_path(&project.manifest, project_root, entry_path, sizeof(entry_path))) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_PATH_TOO_LONG, zt_source_span_unknown(), "app.entry source path is too long");
        return 1;
    }

    printf("project %s\n", project.manifest.project_name);
    printf("  kind: %s\n", project.manifest.project_kind);
    printf("  version: %s\n", project.manifest.version);
    printf("  root: %s\n", project_root);
    printf("  manifest: %s\n", manifest_path);
    printf("  source.root: %s\n", project.manifest.source_root);
    if (kind == ZT_PROJECT_KIND_APP) {
        printf("  app.entry: %s\n", project.manifest.app_entry);
        printf("  entry_path: %s\n", entry_path);
    } else if (kind == ZT_PROJECT_KIND_LIB) {
        printf("  lib.root_namespace: %s\n", project.manifest.lib_root_namespace);
    }
    printf("  build.target: %s\n", project.manifest.build_target);
    printf("  build.profile: %s\n", project.manifest.build_profile);
    printf("  build.output: %s\n", project.manifest.build_output);
    printf("  test.root: %s\n", project.manifest.test_root);
    printf("  zdoc.root: %s\n", project.manifest.zdoc_root);
    printf("  dependencies: %zu\n", project.manifest.dependency_count);
    if (project.manifest.dependency_count > 0) {
        size_t i;
        for (i = 0; i < project.manifest.dependency_count; i += 1) {
            printf("    %s = %s\n", project.manifest.dependencies[i].name, project.manifest.dependencies[i].spec);
        }
    }
    printf("  dev_dependencies: %zu\n", project.manifest.dev_dependency_count);
    if (project.manifest.dev_dependency_count > 0) {
        size_t i;
        for (i = 0; i < project.manifest.dev_dependency_count; i += 1) {
            printf("    %s = %s\n", project.manifest.dev_dependencies[i].name, project.manifest.dev_dependencies[i].spec);
        }
    }
    return 0;
}

static int zt_handle_project_command(const char *command, const char *input_path, const char *output_path, int run_output) {
    zt_project_compile_result compiled;
    c_emitter emitter;
    c_emit_result emit_result;
    char output_dir[512];
    char c_name[192];
    char c_path[768];
    char default_exe_name[192];
    char exe_path[768];

    if (!zt_compile_project(input_path, &compiled)) {
        return 1;
    }

    if (strcmp(command, "verify") == 0) {
        printf("%s", zt_ci_mode_enabled ? "check ok\n" : "verification ok\n");
        zt_project_compile_result_dispose(&compiled);
        return 0;
    }

    if (strcmp(command, "build") == 0 &&
            zt_project_manifest_kind(&compiled.manifest) == ZT_PROJECT_KIND_LIB) {
        fprintf(stderr, "project error: build for lib projects is not implemented in the bootstrap driver; use verify or emit-c\n");
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    if (!zt_emit_module_to_c(&compiled.zir.module, &emitter, &emit_result)) {
        zt_print_single_diag(
            "backend.c.emit",
            zt_diag_code_from_c_emit_error(emit_result.code),
            zt_source_span_make("<zir>", 1, 1, 1),
            "%s",
            emit_result.message[0] != '\0' ? emit_result.message : "C emission failed");
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    if (strcmp(command, "emit-c") == 0) {
        printf("%s\n", c_emitter_text(&emitter));
        c_emitter_dispose(&emitter);
        zt_project_compile_result_dispose(&compiled);
        return 0;
    }

    if (strcmp(command, "build") != 0) {
        fprintf(stderr, "unknown command: %s\n", command);
        c_emitter_dispose(&emitter);
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    if (!zt_join_path(output_dir, sizeof(output_dir), compiled.project_root, compiled.manifest.output_dir)) {
        fprintf(stderr, "error: output directory path is too long\n");
        c_emitter_dispose(&emitter);
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    if (!zt_make_dirs(output_dir)) {
        fprintf(stderr, "error: cannot create output directory '%s'\n", output_dir);
        c_emitter_dispose(&emitter);
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    snprintf(c_name, sizeof(c_name), "%s.c", compiled.manifest.output_name);
    if (!zt_join_path(c_path, sizeof(c_path), output_dir, c_name)) {
        fprintf(stderr, "error: C output path is too long\n");
        c_emitter_dispose(&emitter);
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    if (output_path != NULL) {
        if (!zt_copy_text(exe_path, sizeof(exe_path), output_path)) {
            fprintf(stderr, "error: executable output path is too long\n");
            c_emitter_dispose(&emitter);
            zt_project_compile_result_dispose(&compiled);
            return 1;
        }
    } else {
        snprintf(default_exe_name, sizeof(default_exe_name), "%s.exe", compiled.manifest.output_name);
        if (!zt_join_path(exe_path, sizeof(exe_path), output_dir, default_exe_name)) {
            fprintf(stderr, "error: executable output path is too long\n");
            c_emitter_dispose(&emitter);
            zt_project_compile_result_dispose(&compiled);
            return 1;
        }
    }

    if (!zt_write_file(c_path, c_emitter_text(&emitter))) {
        c_emitter_dispose(&emitter);
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    c_emitter_dispose(&emitter);

    if (!zt_compile_c_file(c_path, exe_path)) {
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    if (zt_ci_mode_enabled && !run_output) {
        printf("build ok\n");
    }

    if (run_output) {
        int run_exit = zt_run_executable(exe_path);
        zt_project_compile_result_dispose(&compiled);
        return run_exit < 0 ? 1 : run_exit;
    }

    zt_project_compile_result_dispose(&compiled);
    return 0;
}

static int zt_handle_test(const char *input_path) {
    int status = zt_handle_project_command("verify", input_path, NULL, 0);
    if (status != 0) return status;

    if (zt_ci_mode_enabled) {
        printf("test ok\n");
    } else {
        printf("test bootstrap: no dedicated test runner yet; executed check pipeline only\n");
    }
    return 0;
}

static int zt_handle_zir_command(const char *command, const char *input_path, const char *output_path, int run_output) {
    char *input_text;
    size_t input_length;
    zir_parse_result parse_result;
    zir_verifier_result verifier_result;
    c_emitter emitter;
    c_emit_result emit_result;

    input_text = zt_read_file(input_path);
    if (input_text == NULL) {
        return 1;
    }
    input_length = strlen(input_text);

    if (!zir_parse_module(input_text, input_length, &parse_result)) {
        size_t line = parse_result.line_number > 0 ? (size_t)parse_result.line_number : 1;
        zt_source_span span = zt_source_span_make(input_path, line, 1, 1);
        zt_print_single_diag(
            "zir.parse",
            zt_diag_code_from_zir_parse_error(parse_result.code),
            span,
            "%s",
            parse_result.message[0] != '\0' ? parse_result.message : "unable to parse ZIR module");
        free(input_text);
        return 1;
    }

    if (strcmp(command, "parse") == 0) {
        size_t fi;

        printf("module %s\n", parse_result.module.name);
        for (fi = 0; fi < parse_result.module.function_count; fi += 1) {
            const zir_function *func = &parse_result.module.functions[fi];
            size_t pi;
            size_t bi;

            printf("  func %s(", func->name);
            for (pi = 0; pi < func->param_count; pi += 1) {
                if (pi > 0) printf(", ");
                printf("%s: %s", func->params[pi].name, func->params[pi].type_name);
            }
            printf(") -> %s\n", func->return_type);

            for (bi = 0; bi < func->block_count; bi += 1) {
                const zir_block *block = &func->blocks[bi];
                size_t ii;

                printf("    block %s (%zu instructions)\n", block->label, block->instruction_count);
                for (ii = 0; ii < block->instruction_count; ii += 1) {
                    const zir_instruction *instr = &block->instructions[ii];
                    if (instr->kind == ZIR_INSTR_ASSIGN) {
                        printf("      %s: %s = %s\n", instr->dest_name, instr->type_name, instr->expr_text);
                    } else {
                        printf("      %s\n", instr->expr_text);
                    }
                }
            }
        }

        zir_parse_result_dispose(&parse_result);
        free(input_text);
        return 0;
    }

    if (!zir_verify_module(&parse_result.module, &verifier_result)) {
        zt_source_span span = zt_source_span_make(input_path, 1, 1, 1);
        if (verifier_result.has_span) {
            const char *source_name = verifier_result.source_name != NULL && verifier_result.source_name[0] != '\0'
                ? verifier_result.source_name
                : input_path;
            size_t line = verifier_result.line > 0 ? verifier_result.line : 1;
            size_t column = verifier_result.column > 0 ? verifier_result.column : 1;
            span = zt_source_span_make(source_name, line, column, column);
        }
        zt_print_single_diag(
            "zir.verify",
            zt_diag_code_from_zir_verifier(verifier_result.code),
            span,
            "%s",
            verifier_result.message[0] != '\0' ? verifier_result.message : "ZIR verification failed");
        zir_parse_result_dispose(&parse_result);
        free(input_text);
        return 1;
    }

    if (strcmp(command, "verify") == 0) {
        printf("verification ok\n");
        zir_parse_result_dispose(&parse_result);
        free(input_text);
        return 0;
    }

    if (!zt_emit_module_to_c(&parse_result.module, &emitter, &emit_result)) {
        zt_print_single_diag(
            "backend.c.emit",
            zt_diag_code_from_c_emit_error(emit_result.code),
            zt_source_span_make(input_path, 1, 1, 1),
            "%s",
            emit_result.message[0] != '\0' ? emit_result.message : "C emission failed");
        zir_parse_result_dispose(&parse_result);
        free(input_text);
        return 1;
    }

    if (strcmp(command, "emit-c") == 0) {
        printf("%s\n", c_emitter_text(&emitter));
        c_emitter_dispose(&emitter);
        zir_parse_result_dispose(&parse_result);
        free(input_text);
        return 0;
    }

    if (strcmp(command, "build") == 0) {
        char c_path[512];
        char exe_path[512];

        if (!zt_replace_extension(c_path, sizeof(c_path), input_path, ".c")) {
            fprintf(stderr, "error: C output path is too long\n");
            c_emitter_dispose(&emitter);
            zir_parse_result_dispose(&parse_result);
            free(input_text);
            return 1;
        }

        if (output_path != NULL) {
            if (!zt_copy_text(exe_path, sizeof(exe_path), output_path)) {
                fprintf(stderr, "error: executable output path is too long\n");
                c_emitter_dispose(&emitter);
                zir_parse_result_dispose(&parse_result);
                free(input_text);
                return 1;
            }
        } else if (!zt_replace_extension(exe_path, sizeof(exe_path), input_path, ".exe")) {
            fprintf(stderr, "error: executable output path is too long\n");
            c_emitter_dispose(&emitter);
            zir_parse_result_dispose(&parse_result);
            free(input_text);
            return 1;
        }

        if (!zt_write_file(c_path, c_emitter_text(&emitter))) {
            c_emitter_dispose(&emitter);
            zir_parse_result_dispose(&parse_result);
            free(input_text);
            return 1;
        }

        c_emitter_dispose(&emitter);

        if (!zt_compile_c_file(c_path, exe_path)) {
            zir_parse_result_dispose(&parse_result);
            free(input_text);
            return 1;
        }

        if (zt_ci_mode_enabled && !run_output) {
            printf("build ok\n");
        }

        if (run_output) {
            int run_exit = zt_run_executable(exe_path);
            zir_parse_result_dispose(&parse_result);
            free(input_text);
            return run_exit < 0 ? 1 : run_exit;
        }

        zir_parse_result_dispose(&parse_result);
        free(input_text);
        return 0;
    }

    fprintf(stderr, "unknown command: %s\n", command);
    c_emitter_dispose(&emitter);
    zir_parse_result_dispose(&parse_result);
    free(input_text);
    return 1;
}

int main(int argc, char *argv[]) {
    zt_arena_init(&global_arena, 1048576);
    zt_string_pool_init(&global_pool, &global_arena);

    const char *command;
    const char *effective_command;
    const char *input_path = NULL;
    int run_output = 0;
    const char *output_path = NULL;
    int fmt_check = 0;
    int ci_mode = 0;
    int parse_start = 2;
    int project_input_optional = 0;
    int i;

    if (argc < 2) {
        zt_print_usage(argv[0]);
        return 1;
    }

    command = argv[1];
    effective_command = command;

    if (strcmp(command, "doc") == 0) {
        if (argc < 3 || strcmp(argv[2], "check") != 0) {
            zt_print_usage(argv[0]);
            return 1;
        }
        effective_command = "doc-check";
        parse_start = 3;
    }

    if (strcmp(effective_command, "check") == 0) {
        effective_command = "verify";
        project_input_optional = 1;
    } else if (strcmp(effective_command, "run") == 0) {
        effective_command = "build";
        run_output = 1;
        project_input_optional = 1;
    } else if (strcmp(effective_command, "test") == 0 ||
            strcmp(effective_command, "fmt") == 0 ||
            strcmp(effective_command, "project-info") == 0 ||
            strcmp(effective_command, "doc-check") == 0 ||
            strcmp(effective_command, "verify") == 0 ||
            strcmp(effective_command, "emit-c") == 0 ||
            strcmp(effective_command, "build") == 0) {
        project_input_optional = 1;
    } else if (strcmp(effective_command, "parse") != 0) {
        zt_print_single_diag(
            "project",
            ZT_DIAG_PROJECT_INVALID_INPUT,
            zt_source_span_unknown(),
            "unknown command: %s",
            command);
        zt_print_usage(argv[0]);
        return 1;
    }

    for (i = parse_start; i < argc; i += 1) {
        if (strcmp(argv[i], "--") == 0) {
            if (i + 1 < argc) {
                fprintf(stderr, "warning: trailing arguments after '--' are not supported in the bootstrap driver and were ignored\n");
            }
            break;
        }
        if (strcmp(argv[i], "--run") == 0) {
            run_output = 1;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_path = argv[i + 1];
            i += 1;
        } else if (strcmp(argv[i], "--check") == 0) {
            fmt_check = 1;
        } else if (strcmp(argv[i], "--ci") == 0) {
            ci_mode = 1;
        } else if (argv[i][0] == '-') {
            zt_print_single_diag(
                "project",
                ZT_DIAG_PROJECT_INVALID_INPUT,
                zt_source_span_unknown(),
                "unknown option: %s",
                argv[i]);
            zt_print_usage(argv[0]);
            return 1;
        } else if (input_path == NULL) {
            input_path = argv[i];
        } else {
            zt_print_single_diag(
                "project",
                ZT_DIAG_PROJECT_INVALID_INPUT,
                zt_source_span_unknown(),
                "unexpected positional argument: %s",
                argv[i]);
            zt_print_usage(argv[0]);
            return 1;
        }
    }

    zt_ci_mode_enabled = ci_mode;

    if (fmt_check && strcmp(effective_command, "fmt") != 0) {
        zt_print_single_diag(
            "project",
            ZT_DIAG_PROJECT_INVALID_INPUT,
            zt_source_span_unknown(),
            "option --check is only valid for fmt");
        return 1;
    }

    if (output_path != NULL && strcmp(effective_command, "build") != 0) {
        zt_print_single_diag(
            "project",
            ZT_DIAG_PROJECT_INVALID_INPUT,
            zt_source_span_unknown(),
            "option -o is only valid for build/run");
        return 1;
    }

    if (run_output && strcmp(effective_command, "build") != 0) {
        zt_print_single_diag(
            "project",
            ZT_DIAG_PROJECT_INVALID_INPUT,
            zt_source_span_unknown(),
            "option --run is only valid for build/run");
        return 1;
    }

    if (input_path == NULL && project_input_optional) {
        input_path = "";
    }

    if (strcmp(effective_command, "project-info") == 0) {
        return zt_handle_project_info(input_path);
    }

    if (strcmp(effective_command, "doc-check") == 0) {
        if (input_path != NULL && input_path[0] != '\0' && zt_path_has_extension(input_path, ".zir")) {
            zt_print_single_diag(
                "doc",
                ZT_DIAG_PROJECT_INVALID_INPUT,
                zt_source_span_unknown(),
                "doc-check expects a project path, not .zir");
            return 1;
        }
        return zt_handle_doc_check(input_path);
    }

    if (strcmp(effective_command, "fmt") == 0) {
        if (input_path != NULL && input_path[0] != '\0' && zt_path_has_extension(input_path, ".zir")) {
            zt_print_single_diag(
                "formatter",
                ZT_DIAG_PROJECT_INVALID_INPUT,
                zt_source_span_unknown(),
                "fmt expects a project path, not .zir");
            return 1;
        }
        return zt_handle_fmt(input_path, fmt_check);
    }

    if (strcmp(effective_command, "test") == 0) {
        return zt_handle_test(input_path);
    }

    if (input_path != NULL && input_path[0] != '\0' && zt_path_has_extension(input_path, ".zir")) {
        return zt_handle_zir_command(effective_command, input_path, output_path, run_output);
    }

    if (strcmp(effective_command, "parse") == 0) {
        zt_print_single_diag(
            "zir.parse",
            ZT_DIAG_PROJECT_INVALID_INPUT,
            zt_source_span_unknown(),
            "parse expects <file.zir>");
        zt_print_usage(argv[0]);
        return 1;
    }

    if (strcmp(effective_command, "emit-c") == 0 ||
            strcmp(effective_command, "build") == 0 ||
            strcmp(effective_command, "verify") == 0) {
        return zt_handle_project_command(effective_command, input_path, output_path, run_output);
    }

    zt_print_single_diag(
        "project",
        ZT_DIAG_PROJECT_INVALID_INPUT,
        zt_source_span_unknown(),
        "unknown command or unsupported input: %s",
        command);
    zt_print_usage(argv[0]);
    return 1;
}

