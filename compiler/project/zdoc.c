#include "compiler/project/zdoc.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <dirent.h>
#endif

typedef struct zt_zdoc_file_item {
    char path[512];
} zt_zdoc_file_item;

typedef struct zt_zdoc_file_list {
    zt_zdoc_file_item *items;
    size_t count;
    size_t capacity;
} zt_zdoc_file_list;

typedef struct zt_zdoc_symbol_item {
    char name[256];
    char source_path[512];
    int is_public;
} zt_zdoc_symbol_item;

typedef struct zt_zdoc_symbol_list {
    zt_zdoc_symbol_item *items;
    size_t count;
    size_t capacity;
} zt_zdoc_symbol_list;

typedef struct zt_zdoc_public_symbol_item {
    char name[256];
    char qualified_name[256];
    char source_path[512];
    size_t line;
    int documented;
} zt_zdoc_public_symbol_item;

typedef struct zt_zdoc_public_symbol_list {
    zt_zdoc_public_symbol_item *items;
    size_t count;
    size_t capacity;
} zt_zdoc_public_symbol_list;

static int zt_copy_text(char *dest, size_t capacity, const char *text) {
    size_t length;

    if (dest == NULL || capacity == 0 || text == NULL) return 0;
    length = strlen(text);
    if (length + 1 > capacity) return 0;
    memcpy(dest, text, length + 1);
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

static int zt_path_equal(const char *left, const char *right) {
    size_t i = 0;
    size_t j = 0;
    if (left == NULL || right == NULL) return 0;
    
    while (left[i] == '.' && (left[i+1] == '/' || left[i+1] == '\\')) i += 2;
    while (right[j] == '.' && (right[j+1] == '/' || right[j+1] == '\\')) j += 2;
    
    while (left[i] != '\0' && right[j] != '\0') {
        if (!zt_path_char_equal(left[i], right[j])) return 0;
        i += 1;
        j += 1;
    }
    return left[i] == '\0' && right[j] == '\0';
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

static int zt_path_is_dir(const char *path) {
    struct stat info;

    if (path == NULL || stat(path, &info) != 0) return 0;

#ifdef _WIN32
    return (info.st_mode & S_IFDIR) != 0;
#else
    return S_ISDIR(info.st_mode);
#endif
}

static int zt_path_has_extension(const char *path, const char *extension) {
    const char *dot;

    if (path == NULL || extension == NULL) return 0;
    dot = strrchr(path, '.');
    return dot != NULL && strcmp(dot, extension) == 0;
}

static int zt_strip_prefix_path(
        const char *path,
        const char *prefix,
        char *dest,
        size_t capacity) {
    size_t i;
    size_t prefix_len;

    if (path == NULL || prefix == NULL || dest == NULL || capacity == 0) return 0;

    prefix_len = strlen(prefix);
    for (i = 0; i < prefix_len; i += 1) {
        if (path[i] == '\0') return 0;
        if (!zt_path_char_equal(path[i], prefix[i])) return 0;
    }

    if (path[prefix_len] == '/' || path[prefix_len] == '\\') {
        prefix_len += 1;
    } else if (path[prefix_len] != '\0') {
        return 0;
    }

    return zt_copy_text(dest, capacity, path + prefix_len);
}

static char *zt_read_file_text(const char *path, size_t *out_length) {
    FILE *file;
    long file_size;
    char *buffer;

    if (out_length != NULL) *out_length = 0;

    file = fopen(path, "rb");
    if (file == NULL) return NULL;

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    file_size = ftell(file);
    if (file_size < 0) {
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    buffer = (char *)malloc((size_t)file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    if (file_size > 0 && fread(buffer, 1, (size_t)file_size, file) != (size_t)file_size) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[file_size] = '\0';
    fclose(file);

    if (out_length != NULL) *out_length = (size_t)file_size;
    return buffer;
}


static void zt_trim_inplace(char *text) {
    char *start;
    char *end;

    if (text == NULL) return;

    start = text;
    while (*start != '\0' && isspace((unsigned char)*start)) start += 1;

    if (start != text) {
        memmove(text, start, strlen(start) + 1);
    }

    end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1])) end -= 1;
    *end = '\0';
}

static int zt_zdoc_diagnostic_push(
        zt_zdoc_diagnostic_list *list,
        zt_zdoc_severity severity,
        zt_zdoc_code code,
        const char *path,
        size_t line,
        size_t column,
        const char *message,
        const char *help);

static int zt_zdoc_file_list_push(zt_zdoc_file_list *list, const char *path) {
    zt_zdoc_file_item *items;
    size_t capacity;

    if (list == NULL || path == NULL) return 0;

    if (list->count >= list->capacity) {
        capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        items = (zt_zdoc_file_item *)realloc(list->items, capacity * sizeof(*items));
        if (items == NULL) return 0;
        list->items = items;
        list->capacity = capacity;
    }

    memset(&list->items[list->count], 0, sizeof(list->items[list->count]));
    if (!zt_copy_text(list->items[list->count].path, sizeof(list->items[list->count].path), path)) {
        return 0;
    }

    list->count += 1;
    return 1;
}

static void zt_zdoc_file_list_dispose(zt_zdoc_file_list *list) {
    if (list == NULL) return;
    free(list->items);
    memset(list, 0, sizeof(*list));
}

static int zt_discover_zdoc_files(const char *directory, zt_zdoc_file_list *files) {
#ifdef _WIN32
    char pattern[768];
    intptr_t handle;
    struct _finddata_t data;

    if (!zt_join_path(pattern, sizeof(pattern), directory, "*")) {
        return 0;
    }

    handle = _findfirst(pattern, &data);
    if (handle == -1) {
        return 1;
    }

    do {
        char child_path[768];

        if (strcmp(data.name, ".") == 0 || strcmp(data.name, "..") == 0) continue;

        if (!zt_join_path(child_path, sizeof(child_path), directory, data.name)) {
            _findclose(handle);
            return 0;
        }

        if ((data.attrib & _A_SUBDIR) != 0) {
            if (!zt_discover_zdoc_files(child_path, files)) {
                _findclose(handle);
                return 0;
            }
        } else if (zt_path_has_extension(child_path, ".zdoc")) {
            if (!zt_zdoc_file_list_push(files, child_path)) {
                _findclose(handle);
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
    if (dir == NULL) return 1;

    while ((entry = readdir(dir)) != NULL) {
        char child_path[768];

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        if (!zt_join_path(child_path, sizeof(child_path), directory, entry->d_name)) {
            closedir(dir);
            return 0;
        }

        if (zt_path_is_dir(child_path)) {
            if (!zt_discover_zdoc_files(child_path, files)) {
                closedir(dir);
                return 0;
            }
        } else if (zt_path_has_extension(child_path, ".zdoc")) {
            if (!zt_zdoc_file_list_push(files, child_path)) {
                closedir(dir);
                return 0;
            }
        }
    }

    closedir(dir);
    return 1;
#endif
}

static int zt_symbol_equal(const zt_zdoc_symbol_item *symbol, const char *name, const char *source_path) {
    if (symbol == NULL || name == NULL || source_path == NULL) return 0;
    return strcmp(symbol->name, name) == 0 && zt_path_equal(symbol->source_path, source_path);
}

static int zt_zdoc_symbol_list_push(zt_zdoc_symbol_list *list, const char *name, const char *source_path, int is_public) {
    zt_zdoc_symbol_item *items;
    size_t i;
    size_t capacity;

    if (list == NULL || name == NULL || source_path == NULL || name[0] == '\0') return 0;

    for (i = 0; i < list->count; i += 1) {
        if (zt_symbol_equal(&list->items[i], name, source_path)) {
            if (is_public) {
                list->items[i].is_public = 1;
            }
            return 1;
        }
    }

    if (list->count >= list->capacity) {
        capacity = list->capacity == 0 ? 16 : list->capacity * 2;
        items = (zt_zdoc_symbol_item *)realloc(list->items, capacity * sizeof(*items));
        if (items == NULL) return 0;
        list->items = items;
        list->capacity = capacity;
    }

    memset(&list->items[list->count], 0, sizeof(list->items[list->count]));
    if (!zt_copy_text(list->items[list->count].name, sizeof(list->items[list->count].name), name)) return 0;
    if (!zt_copy_text(list->items[list->count].source_path, sizeof(list->items[list->count].source_path), source_path)) return 0;
    list->items[list->count].is_public = is_public ? 1 : 0;

    list->count += 1;
    return 1;
}

static void zt_zdoc_symbol_list_dispose(zt_zdoc_symbol_list *list) {
    if (list == NULL) return;
    free(list->items);
    memset(list, 0, sizeof(*list));
}

static int zt_zdoc_public_symbol_equal(
        const zt_zdoc_public_symbol_item *item,
        const char *name,
        const char *source_path) {
    if (item == NULL || name == NULL || source_path == NULL) return 0;
    return strcmp(item->name, name) == 0 && zt_path_equal(item->source_path, source_path);
}

static int zt_zdoc_public_symbol_list_push(
        zt_zdoc_public_symbol_list *list,
        const char *name,
        const char *qualified_name,
        const char *source_path,
        size_t line) {
    zt_zdoc_public_symbol_item *items;
    size_t i;
    size_t capacity;

    if (list == NULL || name == NULL || qualified_name == NULL || source_path == NULL || name[0] == '\0') return 0;

    for (i = 0; i < list->count; i += 1) {
        if (zt_zdoc_public_symbol_equal(&list->items[i], name, source_path)) {
            return 1;
        }
    }

    if (list->count >= list->capacity) {
        capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        items = (zt_zdoc_public_symbol_item *)realloc(list->items, capacity * sizeof(*items));
        if (items == NULL) return 0;
        list->items = items;
        list->capacity = capacity;
    }

    memset(&list->items[list->count], 0, sizeof(list->items[list->count]));
    if (!zt_copy_text(list->items[list->count].name, sizeof(list->items[list->count].name), name)) return 0;
    if (!zt_copy_text(list->items[list->count].qualified_name, sizeof(list->items[list->count].qualified_name), qualified_name)) return 0;
    if (!zt_copy_text(list->items[list->count].source_path, sizeof(list->items[list->count].source_path), source_path)) return 0;
    list->items[list->count].line = line > 0 ? line : 1;
    list->items[list->count].documented = 0;

    list->count += 1;
    return 1;
}

static void zt_zdoc_public_symbol_list_dispose(zt_zdoc_public_symbol_list *list) {
    if (list == NULL) return;
    free(list->items);
    memset(list, 0, sizeof(*list));
}

static void zt_mark_public_symbol_documented(
        zt_zdoc_public_symbol_list *list,
        const char *source_path,
        const char *target) {
    size_t i;

    if (list == NULL || source_path == NULL || target == NULL || target[0] == '\0') return;

    for (i = 0; i < list->count; i += 1) {
        zt_zdoc_public_symbol_item *item = &list->items[i];
        if (!zt_path_equal(item->source_path, source_path)) continue;
        if (strcmp(item->name, target) == 0 || strcmp(item->qualified_name, target) == 0) {
            item->documented = 1;
        }
    }
}

static int zt_emit_missing_public_doc_warnings(
        const zt_zdoc_public_symbol_list *list,
        zt_zdoc_diagnostic_list *diagnostics) {
    size_t i;

    if (list == NULL || diagnostics == NULL) return 0;

    for (i = 0; i < list->count; i += 1) {
        const zt_zdoc_public_symbol_item *item = &list->items[i];
        char message[256];

        if (item->documented) continue;

        snprintf(message, sizeof(message), "public symbol '%.180s' is missing ZDoc", item->qualified_name);
        if (!zt_zdoc_diagnostic_push(
                diagnostics,
                ZT_ZDOC_WARNING,
                ZT_ZDOC_MISSING_PUBLIC_DOC,
                item->source_path,
                item->line,
                1,
                message,
                "Add a paired ZDoc block using --- @target: <symbol> in the mirrored .zdoc file.")) {
            return 0;
        }
    }

    return 1;
}

static int zt_symbol_exists(
        const zt_zdoc_symbol_list *symbols,
        const char *name,
        const char *source_path,
        int allow_local) {
    size_t i;

    if (symbols == NULL || name == NULL) return 0;

    for (i = 0; i < symbols->count; i += 1) {
        const zt_zdoc_symbol_item *symbol = &symbols->items[i];
        if (strcmp(symbol->name, name) != 0) continue;

        if (symbol->source_path[0] == '\0') return 1;
        if (allow_local && source_path != NULL && zt_path_equal(symbol->source_path, source_path)) return 1;
    }

    return 0;
}

static int zt_symbol_exists_local(
        const zt_zdoc_symbol_list *symbols,
        const char *name,
        const char *source_path) {
    size_t i;

    if (symbols == NULL || name == NULL || source_path == NULL) return 0;

    for (i = 0; i < symbols->count; i += 1) {
        const zt_zdoc_symbol_item *symbol = &symbols->items[i];
        if (strcmp(symbol->name, name) != 0) continue;
        if (zt_path_equal(symbol->source_path, source_path)) return 1;
    }

    return 0;
}

static int zt_add_symbol_pair(
        zt_zdoc_symbol_list *symbols,
        const char *source_path,
        const char *namespace_name,
        const char *name,
        int is_public) {
    char qualified[256];
    size_t namespace_len;
    size_t name_len;

    if (symbols == NULL || source_path == NULL || namespace_name == NULL || name == NULL) return 0;

    namespace_len = strlen(namespace_name);
    name_len = strlen(name);

    if (!zt_zdoc_symbol_list_push(symbols, name, source_path, is_public)) return 0;

    if (namespace_len + 1 + name_len + 1 > sizeof(qualified)) return 0;
    memcpy(qualified, namespace_name, namespace_len);
    qualified[namespace_len] = '.';
    memcpy(qualified + namespace_len + 1, name, name_len + 1);

    if (!zt_zdoc_symbol_list_push(symbols, qualified, source_path, is_public)) return 0;

    if (is_public) {
        if (!zt_zdoc_symbol_list_push(symbols, name, "", 1)) return 0;
        if (!zt_zdoc_symbol_list_push(symbols, qualified, "", 1)) return 0;
    }

    return 1;
}

static int zt_collect_source_symbols(
        const zt_zdoc_source_unit *sources,
        size_t source_count,
        zt_zdoc_symbol_list *symbols,
        zt_zdoc_public_symbol_list *public_symbols) {
    size_t i;

    if (symbols == NULL || public_symbols == NULL) return 0;

    for (i = 0; i < source_count; i += 1) {
        const zt_zdoc_source_unit *unit = &sources[i];
        const zt_ast_node *root;
        const char *namespace_name;
        size_t decl_index;

        if (unit->root == NULL || unit->source_path == NULL) continue;
        root = unit->root;
        if (root->kind != ZT_AST_FILE || root->as.file.module_name == NULL) continue;

        namespace_name = root->as.file.module_name;

        for (decl_index = 0; decl_index < root->as.file.declarations.count; decl_index += 1) {
            const zt_ast_node *decl = root->as.file.declarations.items[decl_index];
            const char *decl_name = NULL;
            int decl_is_public = 0;

            if (decl == NULL) continue;

            if (decl->kind == ZT_AST_FUNC_DECL && decl->as.func_decl.name != NULL) {
                decl_name = decl->as.func_decl.name;
                decl_is_public = decl->as.func_decl.is_public;
            } else if (decl->kind == ZT_AST_STRUCT_DECL && decl->as.struct_decl.name != NULL) {
                size_t field_index;
                decl_name = decl->as.struct_decl.name;
                decl_is_public = decl->as.struct_decl.is_public;

                for (field_index = 0; field_index < decl->as.struct_decl.fields.count; field_index += 1) {
                    const zt_ast_node *field = decl->as.struct_decl.fields.items[field_index];
                    char field_name[256];

                    if (field == NULL || field->kind != ZT_AST_STRUCT_FIELD || field->as.struct_field.name == NULL) continue;
                    if (snprintf(field_name, sizeof(field_name), "%s.%s", decl->as.struct_decl.name, field->as.struct_field.name) >= (int)sizeof(field_name)) {
                        return 0;
                    }
                    if (!zt_add_symbol_pair(symbols, unit->source_path, namespace_name, field_name, decl_is_public)) return 0;
                }
            } else if (decl->kind == ZT_AST_ENUM_DECL && decl->as.enum_decl.name != NULL) {
                size_t variant_index;
                decl_name = decl->as.enum_decl.name;
                decl_is_public = decl->as.enum_decl.is_public;

                for (variant_index = 0; variant_index < decl->as.enum_decl.variants.count; variant_index += 1) {
                    const zt_ast_node *variant = decl->as.enum_decl.variants.items[variant_index];
                    char variant_name[256];

                    if (variant == NULL || variant->kind != ZT_AST_ENUM_VARIANT || variant->as.enum_variant.name == NULL) continue;
                    if (snprintf(variant_name, sizeof(variant_name), "%s.%s", decl->as.enum_decl.name, variant->as.enum_variant.name) >= (int)sizeof(variant_name)) {
                        return 0;
                    }
                    if (!zt_add_symbol_pair(symbols, unit->source_path, namespace_name, variant_name, decl_is_public)) return 0;
                }
            } else if (decl->kind == ZT_AST_TRAIT_DECL && decl->as.trait_decl.name != NULL) {
                decl_name = decl->as.trait_decl.name;
                decl_is_public = decl->as.trait_decl.is_public;
            } else if (decl->kind == ZT_AST_EXTERN_DECL && decl->as.extern_decl.binding != NULL) {
                decl_name = decl->as.extern_decl.binding;
                decl_is_public = decl->as.extern_decl.is_public;
            }

            if (decl_name != NULL) {
                char qualified[256];
                size_t namespace_len = strlen(namespace_name);
                size_t decl_len = strlen(decl_name);

                if (!zt_add_symbol_pair(symbols, unit->source_path, namespace_name, decl_name, decl_is_public)) return 0;

                if (decl_is_public) {
                    if (namespace_len + 1 + decl_len + 1 > sizeof(qualified)) return 0;
                    memcpy(qualified, namespace_name, namespace_len);
                    qualified[namespace_len] = '.';
                    memcpy(qualified + namespace_len + 1, decl_name, decl_len + 1);

                    if (!zt_zdoc_public_symbol_list_push(
                            public_symbols,
                            decl_name,
                            qualified,
                            unit->source_path,
                            decl->span.line > 0 ? decl->span.line : 1)) {
                        return 0;
                    }
                }
            }
        }
    }

    return 1;
}

void zt_zdoc_diagnostic_list_init(zt_zdoc_diagnostic_list *list) {
    if (list == NULL) return;
    memset(list, 0, sizeof(*list));
}

void zt_zdoc_diagnostic_list_dispose(zt_zdoc_diagnostic_list *list) {
    if (list == NULL) return;
    free(list->items);
    memset(list, 0, sizeof(*list));
}

size_t zt_zdoc_diagnostic_error_count(const zt_zdoc_diagnostic_list *list) {
    size_t i;
    size_t count = 0;

    if (list == NULL) return 0;
    for (i = 0; i < list->count; i += 1) {
        if (list->items[i].severity == ZT_ZDOC_ERROR) count += 1;
    }
    return count;
}

size_t zt_zdoc_diagnostic_warning_count(const zt_zdoc_diagnostic_list *list) {
    size_t i;
    size_t count = 0;

    if (list == NULL) return 0;
    for (i = 0; i < list->count; i += 1) {
        if (list->items[i].severity == ZT_ZDOC_WARNING) count += 1;
    }
    return count;
}

const char *zt_zdoc_severity_name(zt_zdoc_severity severity) {
    return severity == ZT_ZDOC_WARNING ? "warning" : "error";
}

const char *zt_zdoc_code_name(zt_zdoc_code code) {
    switch (code) {
        case ZT_ZDOC_MALFORMED_BLOCK: return "doc.malformed_block";
        case ZT_ZDOC_MISSING_TARGET: return "doc.missing_target";
        case ZT_ZDOC_MISSING_PAGE: return "doc.missing_page";
        case ZT_ZDOC_UNRESOLVED_TARGET: return "doc.unresolved_target";
        case ZT_ZDOC_UNRESOLVED_LINK: return "doc.unresolved_link";
        case ZT_ZDOC_INVALID_GUIDE_TAG: return "doc.invalid_guide_tag";
        case ZT_ZDOC_INVALID_PAIRED_TAG: return "doc.invalid_paired_tag";
        case ZT_ZDOC_MISSING_PUBLIC_DOC: return "doc.missing_public_doc";
        default: return "doc.unknown";
    }
}

static int zt_zdoc_diagnostic_push(
        zt_zdoc_diagnostic_list *list,
        zt_zdoc_severity severity,
        zt_zdoc_code code,
        const char *path,
        size_t line,
        size_t column,
        const char *message,
        const char *help) {
    zt_zdoc_diagnostic *items;
    size_t capacity;

    if (list == NULL) return 0;

    if (list->count >= list->capacity) {
        capacity = list->capacity == 0 ? 16 : list->capacity * 2;
        items = (zt_zdoc_diagnostic *)realloc(list->items, capacity * sizeof(*items));
        if (items == NULL) return 0;
        list->items = items;
        list->capacity = capacity;
    }

    memset(&list->items[list->count], 0, sizeof(list->items[list->count]));
    list->items[list->count].severity = severity;
    list->items[list->count].code = code;
    list->items[list->count].line = line > 0 ? line : 1;
    list->items[list->count].column = column > 0 ? column : 1;

    zt_copy_text(list->items[list->count].path, sizeof(list->items[list->count].path), path != NULL ? path : "<zdoc>");
    zt_copy_text(list->items[list->count].message, sizeof(list->items[list->count].message), message != NULL ? message : "invalid zdoc block");
    if (help != NULL) {
        zt_copy_text(list->items[list->count].help, sizeof(list->items[list->count].help), help);
    }

    list->count += 1;
    return 1;
}

static int zt_extract_tag_value(const char *header_line, const char *tag, char *dest, size_t capacity) {
    const char *pos;
    const char *cursor;
    size_t length;

    if (header_line == NULL || tag == NULL || dest == NULL || capacity == 0) return 0;

    pos = strstr(header_line, tag);
    if (pos == NULL) {
        dest[0] = '\0';
        return 1;
    }

    cursor = pos + strlen(tag);
    while (*cursor != '\0' && isspace((unsigned char)*cursor)) cursor += 1;

    length = 0;
    while (cursor[length] != '\0' && !isspace((unsigned char)cursor[length])) length += 1;

    if (length + 1 > capacity) return 0;
    memcpy(dest, cursor, length);
    dest[length] = '\0';
    return 1;
}

static int zt_starts_with_guides(const char *relative_path) {
    static const char *prefix = "guides";
    size_t i;

    if (relative_path == NULL) return 0;

    for (i = 0; prefix[i] != '\0'; i += 1) {
        if (relative_path[i] == '\0') return 0;
        if (!zt_path_char_equal(relative_path[i], prefix[i])) return 0;
    }

    if (relative_path[i] == '\0') return 1;
    if (relative_path[i] == '/' || relative_path[i] == '\\') return 1;
    return 0;
}

static int zt_ascii_equal_ci(char left, char right) {
    return tolower((unsigned char)left) == tolower((unsigned char)right);
}
static int zt_text_equal_ci_with_sep(
        const char *left,
        size_t left_length,
        const char *right) {
    size_t i;
    if (left == NULL || right == NULL) return 0;
    for (i = 0; i < left_length; i += 1) {
        if (right[i] == '\0') return 0;
        if ((left[i] == '-' || left[i] == '_') &&
                (right[i] == '-' || right[i] == '_')) {
            continue;
        }
        if (!zt_ascii_equal_ci(left[i], right[i])) return 0;
    }
    return right[left_length] == '\0';
}
static int zt_is_locale_segment(const char *segment, size_t segment_length) {
    static const char *known_locales[] = {
        "en",
        "es",
        "ja",
        "jp",
        "pt",
        "pt-br",
        "pt_br"
    };
    size_t i;
    if (segment == NULL || segment_length == 0) return 0;
    for (i = 0; i < sizeof(known_locales) / sizeof(known_locales[0]); i += 1) {
        if (zt_text_equal_ci_with_sep(segment, segment_length, known_locales[i])) {
            return 1;
        }
    }
    return 0;
}
static int zt_normalize_zdoc_relative_path(
        const char *relative_path,
        char *dest,
        size_t capacity) {
    const char *sep;
    size_t head_length;
    if (relative_path == NULL || dest == NULL || capacity == 0) return 0;
    sep = relative_path;
    while (*sep != '\0' && *sep != '/' && *sep != '\\') sep += 1;
    head_length = (size_t)(sep - relative_path);
    if (*sep == '\0' || !zt_is_locale_segment(relative_path, head_length)) {
        return zt_copy_text(dest, capacity, relative_path);
    }
    return zt_copy_text(dest, capacity, sep + 1);
}

static int zt_make_paired_source_path(
        char *dest,
        size_t capacity,
        const char *project_root,
        const char *source_root,
        const char *relative_zdoc_path) {
    char source_relative[512];
    char *dot;
    char source_root_path[512];

    if (!zt_copy_text(source_relative, sizeof(source_relative), relative_zdoc_path)) return 0;

    dot = strrchr(source_relative, '.');
    if (dot == NULL || strcmp(dot, ".zdoc") != 0) return 0;
    strcpy(dot, ".zt");

    if (!zt_join_path(source_root_path, sizeof(source_root_path), project_root, source_root)) return 0;
    if (!zt_join_path(dest, capacity, source_root_path, source_relative)) return 0;

    return 1;
}

static int zt_resolve_target(
        const zt_zdoc_symbol_list *symbols,
        const char *target,
        int is_guide,
        const char *paired_source_path) {
    if (target == NULL || target[0] == '\0') return 0;
    if (!is_guide && strcmp(target, "namespace") == 0) return 1;
    if (is_guide) return 0;
    return zt_symbol_exists_local(symbols, target, paired_source_path);
}

static int zt_resolve_link(
        const zt_zdoc_symbol_list *symbols,
        const char *link_name,
        int is_guide,
        const char *paired_source_path) {
    if (link_name == NULL || link_name[0] == '\0') return 0;
    return zt_symbol_exists(symbols, link_name, paired_source_path, !is_guide);
}

static int zt_scan_links(
        const char *line,
        size_t line_number,
        const char *doc_path,
        int is_guide,
        const char *paired_source_path,
        const zt_zdoc_symbol_list *symbols,
        zt_zdoc_diagnostic_list *diagnostics) {
    const char *cursor;

    if (line == NULL) return 1;

    cursor = line;
    while (*cursor != '\0') {
        const char *found = strstr(cursor, "@link");
        if (found == NULL) break;

        if ((found == line || !isalnum((unsigned char)found[-1])) && isspace((unsigned char)found[5])) {
            const char *name_start = found + 5;
            char link_name[128];
            size_t out = 0;

            while (*name_start != '\0' && isspace((unsigned char)*name_start)) name_start += 1;

            while (name_start[out] != '\0' &&
                    (isalnum((unsigned char)name_start[out]) || name_start[out] == '_' || name_start[out] == '.')) {
                if (out + 1 >= sizeof(link_name)) break;
                link_name[out] = name_start[out];
                out += 1;
            }
            while (out > 0 && link_name[out - 1] == '.') {
                out -= 1;
            }
            link_name[out] = '\0';

            if (out > 0 && !zt_resolve_link(symbols, link_name, is_guide, paired_source_path)) {
                char message[256];
                snprintf(message, sizeof(message), "unresolved zdoc link '%s'", link_name);
                if (!zt_zdoc_diagnostic_push(
                        diagnostics,
                        ZT_ZDOC_WARNING,
                        ZT_ZDOC_UNRESOLVED_LINK,
                        doc_path,
                        line_number,
                        (size_t)(found - line) + 1,
                        message,
                        "Check the symbol name or add the symbol to the documented API.")) {
                    return 0;
                }
            }

            cursor = name_start + out;
        } else {
            cursor = found + 5;
        }
    }

    return 1;
}

static int zt_check_zdoc_file(
        const char *path,
        const char *project_root,
        const char *source_root,
        const char *zdoc_root_path,
        const zt_zdoc_symbol_list *symbols,
        zt_zdoc_public_symbol_list *public_symbols,
        zt_zdoc_diagnostic_list *diagnostics) {
    char relative_path[512];
    char normalized_relative_path[512];
    char paired_source_path[512];
    int is_guide;
    char *text;
    size_t text_length;
    size_t pos = 0;
    size_t line_number = 0;

    int in_block = 0;
    size_t block_line = 0;
    char block_target[128] = {0};
    char block_page[128] = {0};

    if (!zt_strip_prefix_path(path, zdoc_root_path, relative_path, sizeof(relative_path))) {
        return zt_zdoc_diagnostic_push(
            diagnostics,
            ZT_ZDOC_ERROR,
            ZT_ZDOC_MALFORMED_BLOCK,
            path,
            1,
            1,
            "zdoc file is outside zdoc.root",
            "Move this file under zdoc.root or fix zdoc.root in zenith.ztproj.");
    }

    if (!zt_normalize_zdoc_relative_path(relative_path, normalized_relative_path, sizeof(normalized_relative_path))) {
        return zt_zdoc_diagnostic_push(
            diagnostics,
            ZT_ZDOC_ERROR,
            ZT_ZDOC_MALFORMED_BLOCK,
            path,
            1,
            1,
            "unable to normalize zdoc relative path",
            "Use shorter paths under zdoc.root.");
    }

    is_guide = zt_starts_with_guides(normalized_relative_path);

    paired_source_path[0] = '\0';
    if (!is_guide) {
        if (!zt_make_paired_source_path(
                paired_source_path,
                sizeof(paired_source_path),
                project_root,
                source_root,
                normalized_relative_path)) {
            return zt_zdoc_diagnostic_push(
                diagnostics,
                ZT_ZDOC_ERROR,
                ZT_ZDOC_MALFORMED_BLOCK,
                path,
                1,
                1,
                "paired zdoc path does not map to a .zt source file",
                "Use mirrored paths under zdoc.root with .zdoc extension.");
        }
    }

    text = zt_read_file_text(path, &text_length);
    if (text == NULL) {
        return zt_zdoc_diagnostic_push(
            diagnostics,
            ZT_ZDOC_ERROR,
            ZT_ZDOC_MALFORMED_BLOCK,
            path,
            1,
            1,
            "unable to read zdoc file",
            "Check file permissions and path.");
    }

    while (pos < text_length) {
        char line[1024];
        char trimmed[1024];
        size_t start = pos;
        size_t len;

        while (pos < text_length && text[pos] != '\n') pos += 1;
        len = pos - start;
        if (pos < text_length && text[pos] == '\n') pos += 1;
        line_number += 1;

        if (len >= sizeof(line)) len = sizeof(line) - 1;
        memcpy(line, text + start, len);
        line[len] = '\0';

        if (len > 0 && line[len - 1] == '\r') line[len - 1] = '\0';

        if (!zt_copy_text(trimmed, sizeof(trimmed), line)) {
            free(text);
            return 0;
        }
        zt_trim_inplace(trimmed);

        if (!in_block) {
            if (trimmed[0] == '\0') continue;

            if (strncmp(trimmed, "---", 3) == 0) {
                in_block = 1;
                block_line = line_number;
                block_target[0] = '\0';
                block_page[0] = '\0';

                if (!zt_extract_tag_value(trimmed, "@target:", block_target, sizeof(block_target)) ||
                        !zt_extract_tag_value(trimmed, "@page:", block_page, sizeof(block_page))) {
                    free(text);
                    return 0;
                }
                continue;
            }

            if (!zt_zdoc_diagnostic_push(
                    diagnostics,
                    ZT_ZDOC_ERROR,
                    ZT_ZDOC_MALFORMED_BLOCK,
                    path,
                    line_number,
                    1,
                    "text outside zdoc block",
                    "Wrap documentation content inside --- blocks.")) {
                free(text);
                return 0;
            }
            continue;
        }

        if (strcmp(trimmed, "---") == 0) {
            if (is_guide) {
                if (block_page[0] == '\0') {
                    if (!zt_zdoc_diagnostic_push(
                            diagnostics,
                            ZT_ZDOC_ERROR,
                            ZT_ZDOC_MISSING_PAGE,
                            path,
                            block_line,
                            1,
                            "guide zdoc block is missing @page",
                            "Add @page: <slug> to the opening --- line in zdoc/guides files.")) {
                        free(text);
                        return 0;
                    }
                }
                if (block_target[0] != '\0') {
                    if (!zt_zdoc_diagnostic_push(
                            diagnostics,
                            ZT_ZDOC_ERROR,
                            ZT_ZDOC_INVALID_GUIDE_TAG,
                            path,
                            block_line,
                            1,
                            "guide zdoc block cannot declare @target",
                            "Use @page in zdoc/guides and reserve @target for paired API docs.")) {
                        free(text);
                        return 0;
                    }
                }
            } else {
                if (block_target[0] == '\0') {
                    if (!zt_zdoc_diagnostic_push(
                            diagnostics,
                            ZT_ZDOC_ERROR,
                            ZT_ZDOC_MISSING_TARGET,
                            path,
                            block_line,
                            1,
                            "paired zdoc block is missing @target",
                            "Add @target: <symbol> to the opening --- line.")) {
                        free(text);
                        return 0;
                    }
                } else if (!zt_resolve_target(symbols, block_target, is_guide, paired_source_path)) {
                    char message[256];
                    snprintf(message, sizeof(message), "zdoc target '%s' was not resolved", block_target);
                    if (!zt_zdoc_diagnostic_push(
                            diagnostics,
                            ZT_ZDOC_ERROR,
                            ZT_ZDOC_UNRESOLVED_TARGET,
                            path,
                            block_line,
                            1,
                            message,
                            "Use namespace, local symbol name, or a qualified symbol that exists.")) {
                        free(text);
                        return 0;
                    }
                } else {
                    zt_mark_public_symbol_documented(public_symbols, paired_source_path, block_target);
                }

                if (block_page[0] != '\0') {
                    if (!zt_zdoc_diagnostic_push(
                            diagnostics,
                            ZT_ZDOC_ERROR,
                            ZT_ZDOC_INVALID_PAIRED_TAG,
                            path,
                            block_line,
                            1,
                            "paired zdoc block cannot declare @page",
                            "Use @target in paired API docs and reserve @page for zdoc/guides.")) {
                        free(text);
                        return 0;
                    }
                }
            }

            in_block = 0;
            block_line = 0;
            block_target[0] = '\0';
            block_page[0] = '\0';
            continue;
        }

        if (!zt_scan_links(
                line,
                line_number,
                path,
                is_guide,
                is_guide ? NULL : paired_source_path,
                symbols,
                diagnostics)) {
            free(text);
            return 0;
        }
    }

    if (in_block) {
        if (!zt_zdoc_diagnostic_push(
                diagnostics,
                ZT_ZDOC_ERROR,
                ZT_ZDOC_MALFORMED_BLOCK,
                path,
                block_line,
                1,
                "unterminated zdoc block",
                "Close the block with a trailing --- line.")) {
            free(text);
            return 0;
        }
    }

    free(text);
    return 1;
}

int zt_zdoc_check_project(
        const char *project_root,
        const char *source_root,
        const char *zdoc_root,
        const zt_zdoc_source_unit *sources,
        size_t source_count,
        zt_zdoc_diagnostic_list *diagnostics) {
    char zdoc_root_path[512];
    zt_zdoc_file_list files;
    zt_zdoc_symbol_list symbols;
    zt_zdoc_public_symbol_list public_symbols;
    size_t i;

    if (project_root == NULL || source_root == NULL || zdoc_root == NULL || diagnostics == NULL) {
        return 0;
    }

    zt_zdoc_diagnostic_list_init(diagnostics);
    memset(&files, 0, sizeof(files));
    memset(&symbols, 0, sizeof(symbols));
    memset(&public_symbols, 0, sizeof(public_symbols));

    if (!zt_join_path(zdoc_root_path, sizeof(zdoc_root_path), project_root, zdoc_root)) {
        zt_zdoc_diagnostic_push(
            diagnostics,
            ZT_ZDOC_ERROR,
            ZT_ZDOC_MALFORMED_BLOCK,
            "<project>",
            1,
            1,
            "zdoc.root path is too long",
            "Shorten project root or zdoc.root path.");
        return 1;
    }

    if (!zt_collect_source_symbols(sources, source_count, &symbols, &public_symbols)) {
        zt_zdoc_file_list_dispose(&files);
        zt_zdoc_symbol_list_dispose(&symbols);
        zt_zdoc_public_symbol_list_dispose(&public_symbols);
        zt_zdoc_diagnostic_push(
            diagnostics,
            ZT_ZDOC_ERROR,
            ZT_ZDOC_MALFORMED_BLOCK,
            "<project>",
            1,
            1,
            "out of memory while collecting source symbols",
            "Retry with fewer sources or free system memory.");
        return 1;
    }

    if (!zt_path_is_dir(zdoc_root_path)) {
        int ok = zt_emit_missing_public_doc_warnings(&public_symbols, diagnostics);
        zt_zdoc_symbol_list_dispose(&symbols);
        zt_zdoc_public_symbol_list_dispose(&public_symbols);
        return ok;
    }

    if (!zt_discover_zdoc_files(zdoc_root_path, &files)) {
        zt_zdoc_file_list_dispose(&files);
        zt_zdoc_symbol_list_dispose(&symbols);
        zt_zdoc_public_symbol_list_dispose(&public_symbols);
        zt_zdoc_diagnostic_push(
            diagnostics,
            ZT_ZDOC_ERROR,
            ZT_ZDOC_MALFORMED_BLOCK,
            zdoc_root_path,
            1,
            1,
            "unable to scan zdoc.root",
            "Check directory permissions and path length.");
        return 1;
    }

    for (i = 0; i < files.count; i += 1) {
        if (!zt_check_zdoc_file(
                files.items[i].path,
                project_root,
                source_root,
                zdoc_root_path,
                &symbols,
                &public_symbols,
                diagnostics)) {
            zt_zdoc_file_list_dispose(&files);
            zt_zdoc_symbol_list_dispose(&symbols);
            zt_zdoc_public_symbol_list_dispose(&public_symbols);
            return 1;
        }
    }

    if (!zt_emit_missing_public_doc_warnings(&public_symbols, diagnostics)) {
        zt_zdoc_file_list_dispose(&files);
        zt_zdoc_symbol_list_dispose(&symbols);
        zt_zdoc_public_symbol_list_dispose(&public_symbols);
        return 0;
    }

    zt_zdoc_file_list_dispose(&files);
    zt_zdoc_symbol_list_dispose(&symbols);
    zt_zdoc_public_symbol_list_dispose(&public_symbols);
    return 1;
}
