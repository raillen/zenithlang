/*
 * paths.c — File I/O and path manipulation utilities for the Zenith compiler driver.
 *
 * This module provides pure utility functions with no dependencies on other
 * driver modules. It is safe to call from any driver component.
 */

#include "compiler/driver/driver_internal.h"

/* ── File I/O ─────────────────────────────────────────────────────── */

char *zt_read_file(const char *path) {
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

    rewind(file);

    buffer = (char *)malloc((size_t)file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        fprintf(stderr, "error: out of memory reading '%s'\n", path);
        return NULL;
    }

    if (file_size > 0 && fread(buffer, 1, (size_t)file_size, file) != (size_t)file_size) {
        fclose(file);
        free(buffer);
        fprintf(stderr, "error: cannot read '%s'\n", path);
        return NULL;
    }

    buffer[file_size] = '\0';
    fclose(file);
    return buffer;
}

int zt_write_file(const char *path, const char *content) {
    FILE *file;

    if (path == NULL || content == NULL) return 0;

    file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "error: cannot write '%s'\n", path);
        return 0;
    }

    if (fwrite(content, 1, strlen(content), file) != strlen(content)) {
        fclose(file);
        fprintf(stderr, "error: cannot write all content to '%s'\n", path);
        return 0;
    }
    fclose(file);
    return 1;
}

/* ── String helpers ───────────────────────────────────────────────── */

int zt_copy_text(char *dest, size_t capacity, const char *text) {
    size_t length;

    if (dest == NULL || capacity == 0 || text == NULL) return 0;

    length = strlen(text);
    if (length + 1 > capacity) return 0;

    memcpy(dest, text, length + 1);
    return 1;
}

char *zt_heap_strdup(const char *text) {
    size_t length;
    char *copy;

    if (text == NULL) return NULL;

    length = strlen(text);
    copy = (char *)malloc(length + 1);
    if (copy == NULL) return NULL;

    memcpy(copy, text, length + 1);
    return copy;
}

/* ── Path queries ────────────────────────────────────────────────── */

int zt_path_has_extension(const char *path, const char *extension) {
    size_t path_len;
    size_t ext_len;

    if (path == NULL || extension == NULL) return 0;

    path_len = strlen(path);
    ext_len = strlen(extension);
    if (ext_len > path_len) return 0;

    return strcmp(path + path_len - ext_len, extension) == 0;
}

int zt_path_is_dir(const char *path) {
    struct stat st;

    if (path == NULL || path[0] == '\0') return 0;

#ifdef _WIN32
    if (_stat(path, (struct _stat *)&st) != 0) return 0;
#else
    if (stat(path, &st) != 0) return 0;
#endif

    return (st.st_mode & S_IFDIR) != 0;
}

int zt_path_is_file(const char *path) {
    struct stat st;

    if (path == NULL || path[0] == '\0') return 0;

#ifdef _WIN32
    if (_stat(path, (struct _stat *)&st) != 0) return 0;
#else
    if (stat(path, &st) != 0) return 0;
#endif

    return (st.st_mode & S_IFREG) != 0;
}

/* ── Path manipulation ───────────────────────────────────────────── */

int zt_join_path(char *dest, size_t capacity, const char *left, const char *right) {
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

int zt_dirname(char *dest, size_t capacity, const char *path) {
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

int zt_get_current_dir(char *dest, size_t capacity) {
    if (dest == NULL || capacity == 0) return 0;
    if (ZT_GETCWD(dest, capacity) == NULL) return 0;
    return 1;
}

int zt_find_project_root_from_cwd(char *project_root, size_t capacity) {
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

int zt_replace_extension(char *dest, size_t capacity, const char *path, const char *extension) {
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

/* ── Path normalization ──────────────────────────────────────────── */

void zt_normalize_path_separators(char *path) {
    char *cursor;
    if (path == NULL) return;
    for (cursor = path; *cursor != '\0'; cursor += 1) {
        if (*cursor == '\\') *cursor = '/';
    }
}

void zt_normalize_path_inplace(char *path) {
    char *cursor;
    if (path == NULL) return;
    for (cursor = path; *cursor != '\0'; cursor += 1) {
        if (*cursor == '\\') *cursor = '/';
    }
    if (path[0] == '.' && path[1] == '/') {
        memmove(path, path + 2, strlen(path + 2) + 1);
    }
}

int zt_path_char_equal(char left, char right) {
    if ((left == '/' || left == '\\') && (right == '/' || right == '\\')) return 1;
#ifdef _WIN32
    return tolower((unsigned char)left) == tolower((unsigned char)right);
#else
    return left == right;
#endif
}

int zt_path_suffix_matches(const char *path, const char *suffix) {
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

void zt_normalize_system_path(const char *input, char *output, size_t output_size) {
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

/* ── Git ref validation ──────────────────────────────────────────── */

int zt_git_ref_is_safe(const char *ref) {
    size_t i;
    if (ref == NULL || ref[0] == '\0') return 0;
    for (i = 0; ref[i] != '\0'; i += 1) {
        char ch = ref[i];
        if (isalnum((unsigned char)ch) || ch == '.' || ch == '-' || ch == '_' || ch == '/' || ch == '~' || ch == '^') continue;
        return 0;
    }
    return 1;
}

/* ── Directory creation ──────────────────────────────────────────── */

int zt_make_dirs(const char *path) {
    char buffer[1024];
    size_t i;
    size_t length;

    if (path == NULL || path[0] == '\0') return 0;

    length = strlen(path);
    if (length + 1 > sizeof(buffer)) return 0;

    memcpy(buffer, path, length + 1);
    zt_normalize_path_separators(buffer);

    for (i = 1; i < length; i += 1) {
        if (buffer[i] == '/') {
            buffer[i] = '\0';
            if (!zt_path_is_dir(buffer)) {
                if (ZT_MKDIR(buffer) != 0 && errno != EEXIST) return 0;
            }
            buffer[i] = '/';
        }
    }

    if (!zt_path_is_dir(buffer)) {
        if (ZT_MKDIR(buffer) != 0 && errno != EEXIST) return 0;
    }

    return 1;
}

/* ── Path filter list ────────────────────────────────────────────── */

void zt_path_filter_list_dispose(zt_path_filter_list *list) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i += 1) {
        free(list->items[i]);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

int zt_path_filter_list_push(zt_path_filter_list *list, const char *entry) {
    char **new_items;
    size_t new_capacity;
    char *copy;

    if (list == NULL || entry == NULL) return 0;

    if (list->count >= list->capacity) {
        new_capacity = list->capacity == 0 ? 16 : list->capacity * 2;
        new_items = (char **)realloc(list->items, new_capacity * sizeof(char *));
        if (new_items == NULL) return 0;
        list->items = new_items;
        list->capacity = new_capacity;
    }

    copy = zt_heap_strdup(entry);
    if (copy == NULL) return 0;

    list->items[list->count] = copy;
    list->count += 1;
    return 1;
}
