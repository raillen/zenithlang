#include "compiler/driver/doc_show.h"
#include "compiler/project/ztproj.h"
#include "compiler/utils/l10n.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int zt_resolve_project_paths(
        const char *input_path,
        char *project_root,
        size_t project_root_capacity,
        char *manifest_path,
        size_t manifest_path_capacity);
void zt_apply_manifest_lang(const zt_project_manifest *manifest);

static char *doc_read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    long file_size;
    char *buffer;

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
    return buffer;
}

static int doc_join_path(char *dest, size_t capacity, const char *left, const char *right) {
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

    memcpy(dest + left_len, right, right_len + 1);
    return 1;
}

static int doc_path_exists(const char *path) {
    struct stat info;
    if (path == NULL || stat(path, &info) != 0) return 0;
    return 1;
}

static void doc_ns_to_path(const char *ns, char *path, size_t capacity) {
    size_t i;
    for (i = 0; ns[i] && i < capacity - 1; i++) {
        path[i] = (ns[i] == '.') ? '/' : ns[i];
    }
    path[i] = '\0';
}

typedef struct doc_symbol_entry {
    char name[128];
    char kind[32];
} doc_symbol_entry;

typedef struct doc_symbol_list {
    doc_symbol_entry *items;
    size_t count;
    size_t capacity;
} doc_symbol_list;

static void doc_symbol_list_init(doc_symbol_list *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

static void doc_symbol_list_push(doc_symbol_list *list, const char *name, const char *kind) {
    if (list->count >= list->capacity) {
        size_t new_cap = list->capacity == 0 ? 16 : list->capacity * 2;
        doc_symbol_entry *new_items = (doc_symbol_entry *)realloc(list->items, new_cap * sizeof(doc_symbol_entry));
        if (new_items == NULL) return;
        list->items = new_items;
        list->capacity = new_cap;
    }
    strncpy(list->items[list->count].name, name, sizeof(list->items[list->count].name) - 1);
    list->items[list->count].name[sizeof(list->items[list->count].name) - 1] = '\0';
    strncpy(list->items[list->count].kind, kind, sizeof(list->items[list->count].kind) - 1);
    list->items[list->count].kind[sizeof(list->items[list->count].kind) - 1] = '\0';
    list->count++;
}

static void doc_symbol_list_dispose(doc_symbol_list *list) {
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

static int doc_lex_scan_zt(const char *source, size_t source_len, doc_symbol_list *symbols) {
    size_t i;
    size_t j;
    char name_buf[128];

    i = 0;
    while (i < source_len) {
        if (source[i] == '-' && i + 1 < source_len && source[i + 1] == '-') {
            while (i < source_len && source[i] != '\n') i++;
            continue;
        }

        if (source[i] == '"') {
            i++;
            while (i < source_len && source[i] != '"') {
                if (source[i] == '\\' && i + 1 < source_len) i++;
                i++;
            }
            if (i < source_len) i++;
            continue;
        }

        if (source[i] == '#' && i + 2 < source_len && source[i + 1] == '#' && source[i + 2] == '#') {
            i += 3;
            while (i + 2 < source_len && !(source[i] == '#' && source[i + 1] == '#' && source[i + 2] == '#')) i++;
            if (i + 2 < source_len) i += 3;
            continue;
        }

        if ((source[i] >= 'a' && source[i] <= 'z') || (source[i] >= 'A' && source[i] <= 'Z') || source[i] == '_') {
            j = 0;
            while (i < source_len && j < sizeof(name_buf) - 1 &&
                   ((source[i] >= 'a' && source[i] <= 'z') ||
                    (source[i] >= 'A' && source[i] <= 'Z') ||
                    (source[i] >= '0' && source[i] <= '9') ||
                    source[i] == '_')) {
                name_buf[j++] = source[i++];
            }
            name_buf[j] = '\0';

            while (i < source_len && (source[i] == ' ' || source[i] == '\t')) i++;

            if (strcmp(name_buf, "func") == 0) {
                j = 0;
                while (i < source_len && j < sizeof(name_buf) - 1 &&
                       ((source[i] >= 'a' && source[i] <= 'z') ||
                        (source[i] >= 'A' && source[i] <= 'Z') ||
                        (source[i] >= '0' && source[i] <= '9') ||
                        source[i] == '_')) {
                    name_buf[j++] = source[i++];
                }
                name_buf[j] = '\0';
                if (j > 0) doc_symbol_list_push(symbols, name_buf, "func");
            } else if (strcmp(name_buf, "struct") == 0) {
                j = 0;
                while (i < source_len && j < sizeof(name_buf) - 1 &&
                       ((source[i] >= 'a' && source[i] <= 'z') ||
                        (source[i] >= 'A' && source[i] <= 'Z') ||
                        (source[i] >= '0' && source[i] <= '9') ||
                        source[i] == '_')) {
                    name_buf[j++] = source[i++];
                }
                name_buf[j] = '\0';
                if (j > 0) doc_symbol_list_push(symbols, name_buf, "struct");
            } else if (strcmp(name_buf, "enum") == 0) {
                j = 0;
                while (i < source_len && j < sizeof(name_buf) - 1 &&
                       ((source[i] >= 'a' && source[i] <= 'z') ||
                        (source[i] >= 'A' && source[i] <= 'Z') ||
                        (source[i] >= '0' && source[i] <= '9') ||
                        source[i] == '_')) {
                    name_buf[j++] = source[i++];
                }
                name_buf[j] = '\0';
                if (j > 0) doc_symbol_list_push(symbols, name_buf, "enum");
            } else if (strcmp(name_buf, "trait") == 0) {
                j = 0;
                while (i < source_len && j < sizeof(name_buf) - 1 &&
                       ((source[i] >= 'a' && source[i] <= 'z') ||
                        (source[i] >= 'A' && source[i] <= 'Z') ||
                        (source[i] >= '0' && source[i] <= '9') ||
                        source[i] == '_')) {
                    name_buf[j++] = source[i++];
                }
                name_buf[j] = '\0';
                if (j > 0) doc_symbol_list_push(symbols, name_buf, "trait");
            } else if (strcmp(name_buf, "namespace") == 0) {
                j = 0;
                while (i < source_len && j < sizeof(name_buf) - 1 &&
                       ((source[i] >= 'a' && source[i] <= 'z') ||
                        (source[i] >= 'A' && source[i] <= 'Z') ||
                        (source[i] >= '0' && source[i] <= '9') ||
                        source[i] == '_' || source[i] == '.')) {
                    name_buf[j++] = source[i++];
                }
                name_buf[j] = '\0';
                if (j > 0) doc_symbol_list_push(symbols, name_buf, "namespace");
            }
            continue;
        }

        i++;
    }
    return (int)symbols->count;
}

static int doc_symbol_exists(const doc_symbol_list *symbols, const char *name) {
    size_t i;
    for (i = 0; i < symbols->count; i++) {
        if (strcmp(symbols->items[i].name, name) == 0) return 1;
    }
    return 0;
}

static const char *doc_symbol_kind(const doc_symbol_list *symbols, const char *name) {
    size_t i;
    for (i = 0; i < symbols->count; i++) {
        if (strcmp(symbols->items[i].name, name) == 0) return symbols->items[i].kind;
    }
    return NULL;
}

static void doc_locale_dir(const char *lang_str, char *locale_dir, size_t capacity) {
    if (lang_str == NULL || lang_str[0] == '\0') {
        locale_dir[0] = '\0';
        return;
    }
    snprintf(locale_dir, capacity, "%s", lang_str);
}

static void doc_locale_base(const char *locale, char *base, size_t capacity) {
    size_t i;
    if (locale == NULL || locale[0] == '\0') {
        base[0] = '\0';
        return;
    }
    for (i = 0; locale[i] && locale[i] != '-' && locale[i] != '_' && i < capacity - 1; i++) {
        base[i] = locale[i];
    }
    base[i] = '\0';
}

static int doc_find_zdoc_path(
    const char *zdoc_root,
    const char *ns_path,
    const char *lang_override,
    char *out_path,
    size_t out_capacity)
{
    char locale_dir[64];
    char locale_base[64];
    char zdoc_file[256];
    char candidate[768];

    snprintf(zdoc_file, sizeof(zdoc_file), "%s.zdoc", ns_path);

    if (lang_override != NULL && lang_override[0] != '\0') {
        doc_locale_dir(lang_override, locale_dir, sizeof(locale_dir));

        snprintf(candidate, sizeof(candidate), "%s/%s.zdoc", locale_dir, ns_path);
        if (doc_join_path(out_path, out_capacity, zdoc_root, candidate) && doc_path_exists(out_path)) {
            return 1;
        }

        doc_locale_base(lang_override, locale_base, sizeof(locale_base));
        if (strcmp(locale_base, locale_dir) != 0) {
            snprintf(candidate, sizeof(candidate), "%s/%s.zdoc", locale_base, ns_path);
            if (doc_join_path(out_path, out_capacity, zdoc_root, candidate) && doc_path_exists(out_path)) {
                return 1;
            }
        }
    }

    {
        zt_lang current = zt_l10n_current_lang();
        const char *auto_locale = NULL;
        const char *auto_base = NULL;
        if (current == ZT_LANG_PT) { auto_locale = "pt_BR"; auto_base = "pt"; }
        else if (current == ZT_LANG_ES) { auto_locale = "es"; auto_base = "es"; }
        else if (current == ZT_LANG_JA) { auto_locale = "ja"; auto_base = "ja"; }
        else if (current == ZT_LANG_EN) { auto_locale = "en"; auto_base = "en"; }

        if (auto_locale != NULL && (lang_override == NULL || lang_override[0] == '\0')) {
            snprintf(candidate, sizeof(candidate), "%s/%s.zdoc", auto_locale, ns_path);
            if (doc_join_path(out_path, out_capacity, zdoc_root, candidate) && doc_path_exists(out_path)) {
                return 1;
            }
            if (auto_base != NULL && strcmp(auto_base, auto_locale) != 0) {
                snprintf(candidate, sizeof(candidate), "%s/%s.zdoc", auto_base, ns_path);
                if (doc_join_path(out_path, out_capacity, zdoc_root, candidate) && doc_path_exists(out_path)) {
                    return 1;
                }
            }
        }
    }

    if (doc_join_path(out_path, out_capacity, zdoc_root, zdoc_file) && doc_path_exists(out_path)) {
        return 1;
    }

    return 0;
}

static void print_doc_header(const char *symbol, const char *kind, const char *locale_name) {
    zt_lang lang = zt_l10n_current_lang();
    (void)lang;

    printf("\n");
    printf("\xF0\x9F\x93\x96 %s", symbol);
    if (kind != NULL) {
        printf("  (%s)", kind);
    }
    printf("\n");
    printf("\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\n");
    if (locale_name != NULL && locale_name[0] != '\0') {
        printf("\xF0\x9F\x8C\x90 %s\n\n", locale_name);
    } else {
        printf("\n");
    }
}

static void print_doc_block(const char *content, size_t content_len) {
    size_t i;
    size_t line_start;
    int line_count;

    line_count = 0;
    for (i = 0; i < content_len; i++) {
        if (content[i] == '\n') line_count++;
    }
    if (content_len > 0 && content[content_len - 1] != '\n') line_count++;

    (void)line_count;

    line_start = 0;
    for (i = 0; i <= content_len; i++) {
        if (i == content_len || content[i] == '\n') {
            size_t line_len = i - line_start;

            if (line_len == 0) {
                printf("\n");
            } else {
                printf("   ");
                printf("%.*s\n", (int)line_len, content + line_start);
            }
            line_start = i + 1;
        }
    }
}

static void print_doc_footer(void) {
    printf("\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\n");
}

static int find_target_block(const char *target, const char *file_text,
                              const char **out_start, size_t *out_len) {
    char target_sig[256];
    const char *pos;
    const char *block_start;
    const char *block_end;
    size_t sig_len;

    snprintf(target_sig, sizeof(target_sig), "--- @target: %s", target);
    sig_len = strlen(target_sig);

    pos = file_text;
    while ((pos = strstr(pos, target_sig)) != NULL) {
        if (pos != file_text && pos[-1] != '\n') {
            pos += sig_len;
            continue;
        }

        block_start = pos + sig_len;

        if (*block_start == '\r') block_start++;
        if (*block_start == '\n') block_start++;

        block_end = strstr(block_start, "\n--- ");
        if (block_end == NULL) {
            block_end = strstr(block_start, "\n---\n");
        }
        if (block_end == NULL) {
            block_end = strstr(block_start, "\n---\r\n");
        }
        if (block_end == NULL) {
            size_t text_len = strlen(block_start);
            const char *end_marker = strstr(block_start, "---");
            if (end_marker != NULL) {
                block_end = end_marker;
            } else {
                block_end = block_start + text_len;
            }
        }

        {
            const char *trailing = block_end;
            while (*trailing == '\n' || *trailing == '\r') trailing++;
            if (strncmp(trailing, "---", 3) == 0) {
                *out_start = block_start;
                *out_len = (size_t)(block_end - block_start);
                return 1;
            }
        }

        *out_start = block_start;
        *out_len = (size_t)(block_end - block_start);
        return 1;
    }

    return 0;
}

int zt_handle_doc_show(const char *symbol, const char *lang_override) {
    zt_project_parse_result project;
    char project_root[512] = "";
    char manifest_path[512] = "";
    char zdoc_root_full[512] = "";

    char namespace_buf[256] = "";
    char target_name[128] = "";
    char ns_path[256];
    char zdoc_path[768] = "";
    char zt_path[768] = "";
    char zt_root_full[512] = "";
    char *file_text = NULL;
    char *zt_text = NULL;

    const char *last_dot;
    size_t ns_len;

    const char *block_start;
    size_t block_len;

    const char *locale_label = NULL;

    doc_symbol_list symbols;
    int symbol_found;

    last_dot = strrchr(symbol, '.');
    if (!last_dot) {
        fprintf(stderr, "\xE2\x9D\x8C error: invalid symbol. Expected format: std.math.clamp\n");
        return 1;
    }

    ns_len = (size_t)(last_dot - symbol);
    if (ns_len >= sizeof(namespace_buf)) ns_len = sizeof(namespace_buf) - 1;
    strncpy(namespace_buf, symbol, ns_len);
    namespace_buf[ns_len] = '\0';

    strncpy(target_name, last_dot + 1, sizeof(target_name) - 1);
    target_name[sizeof(target_name) - 1] = '\0';

    if (!zt_resolve_project_paths("", project_root, sizeof(project_root), manifest_path, sizeof(manifest_path))) {
        fprintf(stderr, "\xE2\x9A\xA0 warning: no zenith.ztproj found. Using defaults...\n");
        strcpy(project_root, ".");
        strcpy(zdoc_root_full, "zdoc");
        strcpy(zt_root_full, "src");
    } else {
        if (!zt_project_load_file(manifest_path, &project)) {
            fprintf(stderr, "\xE2\x9D\x8C error: unable to read manifest.\n");
            return 1;
        }
        zt_apply_manifest_lang(&project.manifest);
        doc_join_path(zdoc_root_full, sizeof(zdoc_root_full), project_root, project.manifest.zdoc_root);
        doc_join_path(zt_root_full, sizeof(zt_root_full), project_root, project.manifest.source_root);
    }

    doc_ns_to_path(namespace_buf, ns_path, sizeof(ns_path));

    if (!doc_find_zdoc_path(zdoc_root_full, ns_path, lang_override, zdoc_path, sizeof(zdoc_path))) {
        fprintf(stderr, "\xE2\x9D\x8C error: zdoc file not found for namespace '%s'\n", namespace_buf);
        return 1;
    }

    file_text = doc_read_file(zdoc_path);
    if (!file_text) {
        fprintf(stderr, "\xE2\x9D\x8C error: unable to read zdoc file: %s\n", zdoc_path);
        return 1;
    }

    {
        char zt_file[512];
        snprintf(zt_file, sizeof(zt_file), "%s.zt", ns_path);
        if (doc_join_path(zt_path, sizeof(zt_path), zt_root_full, zt_file)) {
            zt_text = doc_read_file(zt_path);
        }
    }

    doc_symbol_list_init(&symbols);
    if (zt_text != NULL) {
        doc_lex_scan_zt(zt_text, strlen(zt_text), &symbols);
    }

    symbol_found = doc_symbol_exists(&symbols, target_name);

    if (!symbol_found && zt_text != NULL) {
        fprintf(stderr, "\xE2\x9A\xA0 warning: symbol '%s' not found in source '%s'\n", target_name, zt_path);
    }

    if (!find_target_block(target_name, file_text, &block_start, &block_len)) {
        fprintf(stderr, "\xE2\x9D\x8C error: target '%s' not found in zdoc module.\n", target_name);
        free(file_text);
        free(zt_text);
        doc_symbol_list_dispose(&symbols);
        return 1;
    }

    if (lang_override != NULL && lang_override[0] != '\0') {
        zt_lang lang = zt_l10n_from_str(lang_override);
        if (lang == ZT_LANG_PT) locale_label = "Portugues";
        else if (lang == ZT_LANG_EN) locale_label = "English";
        else if (lang == ZT_LANG_ES) locale_label = "Espanol";
        else if (lang == ZT_LANG_JA) locale_label = "Japanese";
        else locale_label = lang_override;
    }

    print_doc_header(target_name, doc_symbol_kind(&symbols, target_name), locale_label);
    print_doc_block(block_start, block_len);
    print_doc_footer();

    free(file_text);
    free(zt_text);
    doc_symbol_list_dispose(&symbols);
    return 0;
}