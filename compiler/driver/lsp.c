#include "compiler/utils/arena.h"
#include "compiler/utils/string_pool.h"
#include "compiler/frontend/lexer/lexer.h"
#include "compiler/frontend/parser/parser.h"
#include "compiler/semantic/binder/binder.h"
#include "compiler/semantic/types/checker.h"
#include "compiler/tooling/formatter.h"
#include "compiler/utils/l10n.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#define LSP_EXE_SUFFIX ".exe"
#else
#include <unistd.h>
#define LSP_EXE_SUFFIX ""
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef struct lsp_sb {
    char *data;
    size_t len;
    size_t cap;
} lsp_sb;

typedef struct lsp_doc {
    char *uri;
    char *path;
    char *text;
    int is_stdlib;
    zt_arena arena;
    zt_string_pool pool;
    zt_parser_result parse;
    zt_bind_result bind;
    zt_check_result check;
    int has_parse;
    int has_bind;
    int has_check;
    struct lsp_doc *next;
} lsp_doc;

static lsp_doc *g_docs = NULL;
static int g_stdlib_docs_loaded = 0;
static char *g_workspace_root = NULL;

typedef struct lsp_stdlib_module {
    const char *name;
    const char *relative_path;
} lsp_stdlib_module;

static const lsp_stdlib_module g_stdlib_modules[] = {
    {"std.bytes", "stdlib/std/bytes.zt"},
    {"std.collections", "stdlib/std/collections.zt"},
    {"std.console", "stdlib/std/console.zt"},
    {"std.concurrent", "stdlib/std/concurrent.zt"},
    {"std.format", "stdlib/std/format.zt"},
    {"std.fs", "stdlib/std/fs.zt"},
    {"std.fs.path", "stdlib/std/fs/path.zt"},
    {"std.io", "stdlib/std/io.zt"},
    {"std.json", "stdlib/std/json.zt"},
    {"std.lazy", "stdlib/std/lazy.zt"},
    {"std.list", "stdlib/std/list.zt"},
    {"std.map", "stdlib/std/map.zt"},
    {"std.math", "stdlib/std/math.zt"},
    {"std.net", "stdlib/std/net.zt"},
    {"std.os", "stdlib/std/os.zt"},
    {"std.os.process", "stdlib/std/os/process.zt"},
    {"std.random", "stdlib/std/random.zt"},
    {"std.test", "stdlib/std/test.zt"},
    {"std.text", "stdlib/std/text.zt"},
    {"std.time", "stdlib/std/time.zt"},
    {"std.validate", "stdlib/std/validate.zt"},
    {NULL, NULL}
};

static char *lsp_strdup(const char *text) {
    size_t len;
    char *copy;
    if (text == NULL) return NULL;
    len = strlen(text);
    copy = (char *)malloc(len + 1);
    if (copy == NULL) return NULL;
    memcpy(copy, text, len + 1);
    return copy;
}

static char *lsp_strndup(const char *text, size_t len) {
    char *copy;
    if (text == NULL) return NULL;
    copy = (char *)malloc(len + 1);
    if (copy == NULL) return NULL;
    memcpy(copy, text, len);
    copy[len] = '\0';
    return copy;
}

static void sb_init(lsp_sb *sb) {
    sb->cap = 256;
    sb->len = 0;
    sb->data = (char *)malloc(sb->cap);
    if (sb->data != NULL) sb->data[0] = '\0';
}

static int sb_reserve(lsp_sb *sb, size_t extra) {
    size_t need;
    char *grown;
    if (sb == NULL || sb->data == NULL) return 0;
    need = sb->len + extra + 1;
    if (need <= sb->cap) return 1;
    while (sb->cap < need) sb->cap *= 2;
    grown = (char *)realloc(sb->data, sb->cap);
    if (grown == NULL) return 0;
    sb->data = grown;
    return 1;
}

static int sb_append_len(lsp_sb *sb, const char *text, size_t len) {
    if (text == NULL) return 1;
    if (!sb_reserve(sb, len)) return 0;
    memcpy(sb->data + sb->len, text, len);
    sb->len += len;
    sb->data[sb->len] = '\0';
    return 1;
}

static int sb_append(lsp_sb *sb, const char *text) {
    return sb_append_len(sb, text, text != NULL ? strlen(text) : 0);
}

static int sb_appendf(lsp_sb *sb, const char *fmt, ...) {
    va_list args;
    va_list copy;
    int needed;
    if (sb == NULL || fmt == NULL) return 0;
    va_start(args, fmt);
    va_copy(copy, args);
    needed = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);
    if (needed < 0) {
        va_end(args);
        return 0;
    }
    if (!sb_reserve(sb, (size_t)needed)) {
        va_end(args);
        return 0;
    }
    vsnprintf(sb->data + sb->len, sb->cap - sb->len, fmt, args);
    sb->len += (size_t)needed;
    va_end(args);
    return 1;
}

static int sb_append_json_string(lsp_sb *sb, const char *text) {
    const unsigned char *p;
    if (!sb_append(sb, "\"")) return 0;
    if (text != NULL) {
        for (p = (const unsigned char *)text; *p != '\0'; p += 1) {
            switch (*p) {
                case '\\': if (!sb_append(sb, "\\\\")) return 0; break;
                case '"': if (!sb_append(sb, "\\\"")) return 0; break;
                case '\b': if (!sb_append(sb, "\\b")) return 0; break;
                case '\f': if (!sb_append(sb, "\\f")) return 0; break;
                case '\n': if (!sb_append(sb, "\\n")) return 0; break;
                case '\r': if (!sb_append(sb, "\\r")) return 0; break;
                case '\t': if (!sb_append(sb, "\\t")) return 0; break;
                default:
                    if (*p < 0x20) {
                        if (!sb_appendf(sb, "\\u%04x", (unsigned int)*p)) return 0;
                    } else {
                        if (!sb_append_len(sb, (const char *)p, 1)) return 0;
                    }
                    break;
            }
        }
    }
    return sb_append(sb, "\"");
}

static char *sb_take(lsp_sb *sb) {
    char *data = sb->data;
    sb->data = NULL;
    sb->len = 0;
    sb->cap = 0;
    return data;
}

static void lsp_log(const char *msg) {
    FILE *f = fopen("lsp.log", "a");
    if (f != NULL) {
        fprintf(f, "%s\n", msg != NULL ? msg : "<null>");
        fclose(f);
    }
}

static char *read_exact(size_t length) {
    char *buffer = (char *)malloc(length + 1);
    size_t read_bytes;
    if (buffer == NULL) return NULL;
    read_bytes = fread(buffer, 1, length, stdin);
    if (read_bytes != length) {
        free(buffer);
        return NULL;
    }
    buffer[length] = '\0';
    return buffer;
}

static char *read_message(void) {
    char line[1024];
    size_t content_length = 0;
    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (strcmp(line, "\r\n") == 0 || strcmp(line, "\n") == 0) break;
        if (strncmp(line, "Content-Length:", 15) == 0) {
            content_length = (size_t)strtoul(line + 15, NULL, 10);
        }
    }
    if (content_length == 0) return NULL;
    return read_exact(content_length);
}

static void send_raw_json(const char *json) {
    size_t len = json != NULL ? strlen(json) : 4;
    fprintf(stdout, "Content-Length: %zu\r\n\r\n%s", len, json != NULL ? json : "null");
    fflush(stdout);
}

static void send_response_take(const char *id_raw, char *result_json) {
    lsp_sb sb;
    sb_init(&sb);
    sb_append(&sb, "{\"jsonrpc\":\"2.0\",\"id\":");
    sb_append(&sb, id_raw != NULL ? id_raw : "null");
    sb_append(&sb, ",\"result\":");
    sb_append(&sb, result_json != NULL ? result_json : "null");
    sb_append(&sb, "}");
    send_raw_json(sb.data);
    free(sb.data);
    free(result_json);
}

static void send_notification_take(const char *method, char *params_json) {
    lsp_sb sb;
    sb_init(&sb);
    sb_append(&sb, "{\"jsonrpc\":\"2.0\",\"method\":");
    sb_append_json_string(&sb, method);
    if (params_json != NULL) {
        sb_append(&sb, ",\"params\":");
        sb_append(&sb, params_json);
    }
    sb_append(&sb, "}");
    send_raw_json(sb.data);
    free(sb.data);
    free(params_json);
}

static const char *skip_ws(const char *p) {
    while (p != NULL && (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')) p += 1;
    return p;
}

static char *json_unescape_string_at(const char *quote, const char **out_end) {
    lsp_sb sb;
    const char *p;
    if (quote == NULL || *quote != '"') return NULL;
    sb_init(&sb);
    p = quote + 1;
    while (*p != '\0') {
        if (*p == '"') {
            if (out_end != NULL) *out_end = p + 1;
            return sb_take(&sb);
        }
        if (*p == '\\') {
            p += 1;
            switch (*p) {
                case '"': sb_append_len(&sb, "\"", 1); break;
                case '\\': sb_append_len(&sb, "\\", 1); break;
                case '/': sb_append_len(&sb, "/", 1); break;
                case 'b': sb_append_len(&sb, "\b", 1); break;
                case 'f': sb_append_len(&sb, "\f", 1); break;
                case 'n': sb_append_len(&sb, "\n", 1); break;
                case 'r': sb_append_len(&sb, "\r", 1); break;
                case 't': sb_append_len(&sb, "\t", 1); break;
                case 'u':
                    if (isxdigit((unsigned char)p[1]) && isxdigit((unsigned char)p[2]) &&
                            isxdigit((unsigned char)p[3]) && isxdigit((unsigned char)p[4])) {
                        char hex[5];
                        long value;
                        memcpy(hex, p + 1, 4);
                        hex[4] = '\0';
                        value = strtol(hex, NULL, 16);
                        if (value > 0 && value < 128) {
                            char ch = (char)value;
                            sb_append_len(&sb, &ch, 1);
                        } else {
                            sb_append_len(&sb, "?", 1);
                        }
                        p += 4;
                    }
                    break;
                default:
                    if (*p != '\0') sb_append_len(&sb, p, 1);
                    break;
            }
            if (*p != '\0') p += 1;
            continue;
        }
        sb_append_len(&sb, p, 1);
        p += 1;
    }
    free(sb.data);
    return NULL;
}

static char *json_get_string_key(const char *json, const char *key) {
    char pattern[96];
    const char *p;
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = strstr(json, pattern);
    while (p != NULL) {
        const char *colon = strchr(p + strlen(pattern), ':');
        if (colon == NULL) return NULL;
        colon = skip_ws(colon + 1);
        if (colon != NULL && *colon == '"') return json_unescape_string_at(colon, NULL);
        p = strstr(p + 1, pattern);
    }
    return NULL;
}

static int json_get_number_key(const char *json, const char *key, int fallback_value) {
    char pattern[96];
    const char *p;
    const char *colon;
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = strstr(json, pattern);
    if (p == NULL) return fallback_value;
    colon = strchr(p + strlen(pattern), ':');
    if (colon == NULL) return fallback_value;
    colon = skip_ws(colon + 1);
    return (int)strtol(colon, NULL, 10);
}

static int json_get_bool_key(const char *json, const char *key, int fallback_value) {
    char pattern[96];
    const char *p;
    const char *colon;
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = strstr(json, pattern);
    if (p == NULL) return fallback_value;
    colon = strchr(p + strlen(pattern), ':');
    if (colon == NULL) return fallback_value;
    colon = skip_ws(colon + 1);
    if (strncmp(colon, "true", 4) == 0) return 1;
    if (strncmp(colon, "false", 5) == 0) return 0;
    return fallback_value;
}

static char *json_get_id_raw(const char *json) {
    const char *p = strstr(json, "\"id\"");
    const char *start;
    const char *end;
    if (p == NULL) return NULL;
    p = strchr(p + 4, ':');
    if (p == NULL) return NULL;
    start = skip_ws(p + 1);
    if (start == NULL) return NULL;
    if (*start == '"') {
        int escaped = 0;
        end = start + 1;
        while (*end != '\0') {
            if (!escaped && *end == '"') {
                end += 1;
                break;
            }
            escaped = (!escaped && *end == '\\') ? 1 : 0;
            if (*end != '\\') escaped = 0;
            end += 1;
        }
        return lsp_strndup(start, (size_t)(end - start));
    }
    end = start;
    while (*end != '\0' && *end != ',' && *end != '}' && !isspace((unsigned char)*end)) end += 1;
    return lsp_strndup(start, (size_t)(end - start));
}

static char *uri_to_path_alloc(const char *uri) {
    const char *rest;
    lsp_sb sb;
    if (uri == NULL) return lsp_strdup("");
    rest = uri;
    if (strncmp(uri, "file://", 7) == 0) {
        rest = uri + 7;
        if (rest[0] == '/' && isalpha((unsigned char)rest[1]) && rest[2] == ':') rest += 1;
    }
    sb_init(&sb);
    while (*rest != '\0') {
        if (*rest == '%' && isxdigit((unsigned char)rest[1]) && isxdigit((unsigned char)rest[2])) {
            char hex[3];
            char ch;
            hex[0] = rest[1];
            hex[1] = rest[2];
            hex[2] = '\0';
            ch = (char)strtol(hex, NULL, 16);
            sb_append_len(&sb, &ch, 1);
            rest += 3;
        } else {
            char ch = *rest == '/' ? '\\' : *rest;
            sb_append_len(&sb, &ch, 1);
            rest += 1;
        }
    }
    return sb_take(&sb);
}

static char *path_to_file_uri_alloc(const char *path) {
    const char *p;
    lsp_sb sb;
    if (path == NULL) return lsp_strdup("file:///");
    sb_init(&sb);
    sb_append(&sb, path[0] == '/' ? "file://" : "file:///");
    p = path;
    while (*p != '\0') {
        char ch = *p == '\\' ? '/' : *p;
        switch (ch) {
            case ' ': sb_append(&sb, "%20"); break;
            case '#': sb_append(&sb, "%23"); break;
            case '%': sb_append(&sb, "%25"); break;
            case '?': sb_append(&sb, "%3F"); break;
            default: sb_append_len(&sb, &ch, 1); break;
        }
        p += 1;
    }
    return sb_take(&sb);
}

static char *read_file_text_alloc(const char *path) {
    FILE *file;
    long length;
    char *text;
    size_t read_count;
    if (path == NULL) return NULL;
    file = fopen(path, "rb");
    if (file == NULL) return NULL;
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    length = ftell(file);
    if (length < 0) {
        fclose(file);
        return NULL;
    }
    rewind(file);
    text = (char *)malloc((size_t)length + 1);
    if (text == NULL) {
        fclose(file);
        return NULL;
    }
    read_count = fread(text, 1, (size_t)length, file);
    fclose(file);
    if (read_count != (size_t)length) {
        free(text);
        return NULL;
    }
    text[length] = '\0';
    return text;
}

static void doc_clear_analysis(lsp_doc *doc) {
    if (doc == NULL) return;
    if (doc->has_check) {
        zt_check_result_dispose(&doc->check);
        doc->has_check = 0;
    }
    if (doc->has_bind) {
        zt_bind_result_dispose(&doc->bind);
        doc->has_bind = 0;
    }
    if (doc->has_parse) {
        zt_parser_result_dispose(&doc->parse);
        doc->has_parse = 0;
    }
    zt_arena_dispose(&doc->arena);
    memset(&doc->arena, 0, sizeof(doc->arena));
    memset(&doc->pool, 0, sizeof(doc->pool));
}

static void doc_parse_only(lsp_doc *doc) {
    if (doc == NULL || doc->text == NULL) return;
    doc_clear_analysis(doc);
    zt_arena_init(&doc->arena, 64 * 1024);
    zt_string_pool_init(&doc->pool, &doc->arena);
    doc->parse = zt_parse(&doc->arena, &doc->pool, doc->path != NULL ? doc->path : doc->uri, doc->text, strlen(doc->text));
    doc->has_parse = 1;
}

static void doc_reparse(lsp_doc *doc) {
    if (doc == NULL || doc->text == NULL) return;
    doc_clear_analysis(doc);
    zt_arena_init(&doc->arena, 64 * 1024);
    zt_string_pool_init(&doc->pool, &doc->arena);
    doc->parse = zt_parse(&doc->arena, &doc->pool, doc->path != NULL ? doc->path : doc->uri, doc->text, strlen(doc->text));
    doc->has_parse = 1;
    if (doc->parse.root != NULL) {
        doc->bind = zt_bind_file(doc->parse.root);
        doc->has_bind = 1;
        doc->check = zt_check_file(doc->parse.root);
        doc->has_check = 1;
    }
}

static lsp_doc *find_doc(const char *uri) {
    lsp_doc *doc;
    for (doc = g_docs; doc != NULL; doc = doc->next) {
        if (doc->uri != NULL && uri != NULL && strcmp(doc->uri, uri) == 0) return doc;
    }
    return NULL;
}

static lsp_doc *upsert_doc(const char *uri, const char *text) {
    lsp_doc *doc = find_doc(uri);
    if (doc == NULL) {
        doc = (lsp_doc *)calloc(1, sizeof(lsp_doc));
        if (doc == NULL) return NULL;
        doc->uri = lsp_strdup(uri);
        doc->path = uri_to_path_alloc(uri);
        doc->next = g_docs;
        g_docs = doc;
    }
    free(doc->text);
    doc->text = lsp_strdup(text != NULL ? text : "");
    doc_reparse(doc);
    return doc;
}

static lsp_doc *upsert_stdlib_doc(const char *uri, const char *path, const char *text) {
    lsp_doc *doc = find_doc(uri);
    if (doc == NULL) {
        doc = (lsp_doc *)calloc(1, sizeof(lsp_doc));
        if (doc == NULL) return NULL;
        doc->uri = lsp_strdup(uri);
        doc->next = g_docs;
        g_docs = doc;
    }
    doc->is_stdlib = 1;
    free(doc->path);
    free(doc->text);
    doc->path = lsp_strdup(path);
    doc->text = lsp_strdup(text != NULL ? text : "");
    doc_parse_only(doc);
    return doc;
}

static void ensure_stdlib_docs_loaded(void) {
    char cwd[PATH_MAX];
    size_t i;
    if (g_stdlib_docs_loaded) return;
#ifdef _WIN32
    if (_getcwd(cwd, sizeof(cwd)) == NULL) cwd[0] = '\0';
#else
    if (getcwd(cwd, sizeof(cwd)) == NULL) cwd[0] = '\0';
#endif
    for (i = 0; g_stdlib_modules[i].name != NULL; i += 1) {
        char path[PATH_MAX * 2];
        char *text;
        char *uri;
        text = NULL;
        if (g_workspace_root != NULL && g_workspace_root[0] != '\0') {
            snprintf(path, sizeof(path), "%s/%s", g_workspace_root, g_stdlib_modules[i].relative_path);
            text = read_file_text_alloc(path);
        }
        if (text == NULL && cwd[0] != '\0') {
            snprintf(path, sizeof(path), "%s/%s", cwd, g_stdlib_modules[i].relative_path);
            text = read_file_text_alloc(path);
        }
        if (text == NULL && cwd[0] != '\0') {
            snprintf(path, sizeof(path), "%s/../%s", cwd, g_stdlib_modules[i].relative_path);
            text = read_file_text_alloc(path);
        }
        if (text == NULL && cwd[0] != '\0') {
            snprintf(path, sizeof(path), "%s/../../%s", cwd, g_stdlib_modules[i].relative_path);
            text = read_file_text_alloc(path);
        }
        if (text == NULL) {
            snprintf(path, sizeof(path), "%s", g_stdlib_modules[i].relative_path);
            text = read_file_text_alloc(path);
        }
        if (text == NULL) continue;
        uri = path_to_file_uri_alloc(path);
        upsert_stdlib_doc(uri, path, text);
        free(uri);
        free(text);
    }
    g_stdlib_docs_loaded = 1;
}

static void free_doc(lsp_doc *doc) {
    if (doc == NULL) return;
    doc_clear_analysis(doc);
    free(doc->uri);
    free(doc->path);
    free(doc->text);
    free(doc);
}

static void remove_doc(const char *uri) {
    lsp_doc **link = &g_docs;
    while (*link != NULL) {
        lsp_doc *doc = *link;
        if (doc->uri != NULL && uri != NULL && strcmp(doc->uri, uri) == 0) {
            if (doc->is_stdlib) return;
            *link = doc->next;
            free_doc(doc);
            return;
        }
        link = &doc->next;
    }
}

static void free_all_docs(void) {
    lsp_doc *doc = g_docs;
    while (doc != NULL) {
        lsp_doc *next = doc->next;
        free_doc(doc);
        doc = next;
    }
    g_docs = NULL;
}

static int lsp_line(size_t line) {
    return line > 0 ? (int)line - 1 : 0;
}

static int lsp_col(size_t col) {
    return col > 0 ? (int)col - 1 : 0;
}

static void append_diag_list_json(lsp_sb *sb, const zt_diag_list *list, int *first) {
    size_t i;
    if (list == NULL) return;
    for (i = 0; i < list->count; i += 1) {
        const zt_diag *d = &list->items[i];
        int severity = 1;
        int start_line = lsp_line(d->span.line);
        int start_col = lsp_col(d->span.column_start);
        int end_line = start_line;
        int end_col = lsp_col(d->span.column_end);
        if (end_line < start_line) end_line = start_line;
        if (end_line == start_line && end_col <= start_col) end_col = start_col + 1;
        if (d->severity == ZT_DIAG_SEVERITY_WARNING) severity = 2;
        else if (d->severity == ZT_DIAG_SEVERITY_NOTE) severity = 3;
        else if (d->severity == ZT_DIAG_SEVERITY_HELP) severity = 4;
        if (!*first) sb_append(sb, ",");
        *first = 0;
        sb_append(sb, "{\"range\":{\"start\":{");
        sb_appendf(sb, "\"line\":%d,\"character\":%d", start_line, start_col);
        sb_append(sb, "},\"end\":{");
        sb_appendf(sb, "\"line\":%d,\"character\":%d", end_line, end_col);
        sb_append(sb, "}},\"severity\":");
        sb_appendf(sb, "%d", severity);
        sb_append(sb, ",\"code\":");
        sb_append_json_string(sb, zt_diag_code_stable(d->code));
        sb_append(sb, ",\"source\":\"zenith\",\"message\":");
        sb_append_json_string(sb, d->message);
        if (d->suggestion[0] != '\0') {
            sb_append(sb, ",\"data\":{\"suggestion\":");
            sb_append_json_string(sb, d->suggestion);
            sb_append(sb, "}");
        }
        sb_append(sb, "}");
    }
}

static void publish_diagnostics(lsp_doc *doc) {
    lsp_sb sb;
    int first = 1;
    if (doc == NULL || doc->uri == NULL) return;
    sb_init(&sb);
    sb_append(&sb, "{\"uri\":");
    sb_append_json_string(&sb, doc->uri);
    sb_append(&sb, ",\"diagnostics\":[");
    if (doc->has_parse) append_diag_list_json(&sb, &doc->parse.diagnostics, &first);
    if (doc->has_bind) append_diag_list_json(&sb, &doc->bind.diagnostics, &first);
    if (doc->has_check) append_diag_list_json(&sb, &doc->check.diagnostics, &first);
    sb_append(&sb, "]}");
    send_notification_take("textDocument/publishDiagnostics", sb_take(&sb));
}

static void publish_empty_diagnostics(const char *uri) {
    lsp_sb sb;
    sb_init(&sb);
    sb_append(&sb, "{\"uri\":");
    sb_append_json_string(&sb, uri);
    sb_append(&sb, ",\"diagnostics\":[]}");
    send_notification_take("textDocument/publishDiagnostics", sb_take(&sb));
}

static int offset_from_position(const char *text, int line, int character) {
    int cur_line = 0;
    int cur_char = 0;
    int offset = 0;
    if (text == NULL) return 0;
    while (text[offset] != '\0') {
        if (cur_line == line && cur_char == character) return offset;
        if (text[offset] == '\n') {
            cur_line += 1;
            cur_char = 0;
        } else {
            cur_char += 1;
        }
        offset += 1;
    }
    return offset;
}

static void position_from_offset(const char *text, int offset, int *line, int *character) {
    int cur_line = 0;
    int cur_char = 0;
    int i = 0;
    if (text != NULL && offset > 0) {
        while (text[i] != '\0' && i < offset) {
            if (text[i] == '\n') {
                cur_line += 1;
                cur_char = 0;
            } else {
                cur_char += 1;
            }
            i += 1;
        }
    }
    if (line != NULL) *line = cur_line;
    if (character != NULL) *character = cur_char;
}

static int word_char(int ch) {
    return isalnum((unsigned char)ch) || ch == '_' || ch == '.';
}

static int ident_char(int ch) {
    return isalnum((unsigned char)ch) || ch == '_';
}

static char *word_at_position(const char *text, int line, int character) {
    int offset;
    int start;
    int end;
    if (text == NULL) return NULL;
    offset = offset_from_position(text, line, character);
    if (offset > 0 && text[offset] != '\0' && !word_char(text[offset]) && word_char(text[offset - 1])) offset -= 1;
    if (!word_char(text[offset])) return NULL;
    start = offset;
    end = offset;
    while (start > 0 && word_char(text[start - 1])) start -= 1;
    while (text[end] != '\0' && word_char(text[end])) end += 1;
    return lsp_strndup(text + start, (size_t)(end - start));
}

static char *ident_segment_at_position(const char *text, int line, int character, int *start_line, int *start_col, int *end_col) {
    int offset;
    int start;
    int end;
    int found_line = 0;
    int found_col = 0;
    if (text == NULL) return NULL;
    offset = offset_from_position(text, line, character);
    if (offset > 0 && text[offset] != '\0' && !ident_char(text[offset]) && ident_char(text[offset - 1])) offset -= 1;
    if (!ident_char(text[offset])) return NULL;
    start = offset;
    end = offset;
    while (start > 0 && ident_char(text[start - 1])) start -= 1;
    while (text[end] != '\0' && ident_char(text[end])) end += 1;
    position_from_offset(text, start, &found_line, &found_col);
    if (start_line != NULL) *start_line = found_line;
    if (start_col != NULL) *start_col = found_col;
    if (end_col != NULL) *end_col = found_col + (end - start);
    return lsp_strndup(text + start, (size_t)(end - start));
}

static int completion_member_dot_offset(const char *text, int line, int character) {
    int offset = offset_from_position(text, line, character);
    if (text == NULL || offset <= 0) return -1;
    while (offset > 0 && isspace((unsigned char)text[offset - 1]) && text[offset - 1] != '\n') {
        offset -= 1;
    }
    while (offset > 0 && ident_char(text[offset - 1])) {
        offset -= 1;
    }
    while (offset > 0 && isspace((unsigned char)text[offset - 1]) && text[offset - 1] != '\n') {
        offset -= 1;
    }
    if (offset > 0 && text[offset - 1] == '.') return offset - 1;
    return -1;
}

static int completion_is_member_access(const char *text, int line, int character) {
    return completion_member_dot_offset(text, line, character) >= 0;
}

static char *completion_receiver_name(const char *text, int line, int character) {
    int offset = completion_member_dot_offset(text, line, character);
    int end;
    int start;
    if (text == NULL || offset <= 0) return NULL;
    while (offset > 0 && isspace((unsigned char)text[offset - 1]) && text[offset - 1] != '\n') {
        offset -= 1;
    }
    end = offset;
    start = end;
    while (start > 0 && ident_char(text[start - 1])) start -= 1;
    if (end <= start) return NULL;
    return lsp_strndup(text + start, (size_t)(end - start));
}

static int completion_is_import_path(const char *text, int line, int character) {
    int offset = offset_from_position(text, line, character);
    int line_start;
    int cursor;
    const char *p;
    if (text == NULL || offset < 0) return 0;
    line_start = offset;
    while (line_start > 0 && text[line_start - 1] != '\n') line_start -= 1;
    p = text + line_start;
    cursor = offset - line_start;

    while (cursor > 0 && isspace((unsigned char)*p) && *p != '\n') {
        p += 1;
        cursor -= 1;
    }
    if (cursor < 6 || strncmp(p, "import", 6) != 0) return 0;
    p += 6;
    cursor -= 6;
    if (cursor > 0 && ident_char((unsigned char)*p)) return 0;

    while (cursor > 0) {
        if (cursor >= 4 &&
            isspace((unsigned char)p[0]) &&
            p[1] == 'a' &&
            p[2] == 's' &&
            isspace((unsigned char)p[3])) {
            return 0;
        }
        p += 1;
        cursor -= 1;
    }
    return 1;
}

static const char *node_symbol_name(const zt_ast_node *node) {
    if (node == NULL) return NULL;
    switch (node->kind) {
        case ZT_AST_FUNC_DECL: return node->as.func_decl.name;
        case ZT_AST_STRUCT_DECL: return node->as.struct_decl.name;
        case ZT_AST_TRAIT_DECL: return node->as.trait_decl.name;
        case ZT_AST_ENUM_DECL: return node->as.enum_decl.name;
        case ZT_AST_VAR_DECL: return node->as.var_decl.name;
        case ZT_AST_CONST_DECL: return node->as.const_decl.name;
        default: return NULL;
    }
}

static const char *node_symbol_kind(const zt_ast_node *node) {
    if (node == NULL) return "symbol";
    switch (node->kind) {
        case ZT_AST_FUNC_DECL: return "func";
        case ZT_AST_STRUCT_DECL: return "struct";
        case ZT_AST_TRAIT_DECL: return "trait";
        case ZT_AST_ENUM_DECL: return "enum";
        case ZT_AST_VAR_DECL: return "var";
        case ZT_AST_CONST_DECL: return "const";
        default: return "symbol";
    }
}

static int node_is_public(const zt_ast_node *node) {
    if (node == NULL) return 0;
    switch (node->kind) {
        case ZT_AST_FUNC_DECL: return node->as.func_decl.is_public;
        case ZT_AST_STRUCT_DECL: return node->as.struct_decl.is_public;
        case ZT_AST_TRAIT_DECL: return node->as.trait_decl.is_public;
        case ZT_AST_ENUM_DECL: return node->as.enum_decl.is_public;
        case ZT_AST_CONST_DECL: return node->as.const_decl.is_public;
        case ZT_AST_VAR_DECL: return node->as.var_decl.is_public;
        case ZT_AST_EXTERN_DECL: return node->as.extern_decl.is_public;
        default: return 1;
    }
}

static const char *doc_module_name(lsp_doc *doc) {
    if (doc == NULL || !doc->has_parse || doc->parse.root == NULL || doc->parse.root->kind != ZT_AST_FILE) return NULL;
    return doc->parse.root->as.file.module_name;
}

static int namespace_is_std(const char *name) {
    return name != NULL && strncmp(name, "std.", 4) == 0;
}

static const char *lsp_doc_text(const char *pt, const char *en) {
    return zt_l10n_current_lang() == ZT_LANG_PT ? pt : en;
}

static const char *lsp_doc_heading_what(void) {
    return lsp_doc_text("**O que faz**", "**What it does**");
}

static const char *lsp_doc_heading_documentation(void) {
    return lsp_doc_text("**Documentacao**", "**Documentation**");
}

static const char *lsp_doc_heading_signature(void) {
    return lsp_doc_text("**Assinatura**", "**Signature**");
}

static const char *lsp_doc_heading_usage(void) {
    return lsp_doc_text("**Uso basico**", "**Basic use**");
}

static const char *lsp_doc_module_label(void) {
    return lsp_doc_text("Modulo", "Module");
}

static const char *lsp_missing_function_doc_text(int stdlib_symbol) {
    if (stdlib_symbol) {
        return lsp_doc_text(
            "Documentacao detalhada ainda nao foi escrita para esta funcao da stdlib.",
            "Detailed documentation has not been written for this stdlib function yet.");
    }
    return lsp_doc_text(
        "Sem documentacao local. Adicione comentarios '--' imediatamente acima da funcao para aparecer aqui.",
        "No local documentation yet. Add '--' comments immediately above the function to show help here.");
}

static char *lsp_namespace_to_zdoc_path(const char *module) {
    lsp_sb sb;
    const char *p;
    char ch[2];
    if (module == NULL || module[0] == '\0') return NULL;
    sb_init(&sb);
    ch[1] = '\0';
    for (p = module; *p != '\0'; p += 1) {
        ch[0] = *p == '.' ? '/' : *p;
        sb_append(&sb, ch);
    }
    return sb_take(&sb);
}

static char *lsp_read_relative_file_from_roots(const char *relative_path) {
    char cwd[PATH_MAX];
    char path[PATH_MAX * 2];
    char *text;
    if (relative_path == NULL || relative_path[0] == '\0') return NULL;
#ifdef _WIN32
    if (_getcwd(cwd, sizeof(cwd)) == NULL) cwd[0] = '\0';
#else
    if (getcwd(cwd, sizeof(cwd)) == NULL) cwd[0] = '\0';
#endif
    if (g_workspace_root != NULL && g_workspace_root[0] != '\0') {
        snprintf(path, sizeof(path), "%s/%s", g_workspace_root, relative_path);
        text = read_file_text_alloc(path);
        if (text != NULL) return text;
    }
    if (cwd[0] != '\0') {
        snprintf(path, sizeof(path), "%s/%s", cwd, relative_path);
        text = read_file_text_alloc(path);
        if (text != NULL) return text;
        snprintf(path, sizeof(path), "%s/../%s", cwd, relative_path);
        text = read_file_text_alloc(path);
        if (text != NULL) return text;
        snprintf(path, sizeof(path), "%s/../../%s", cwd, relative_path);
        text = read_file_text_alloc(path);
        if (text != NULL) return text;
    }
    return read_file_text_alloc(relative_path);
}

static const char *lsp_current_locale_dir(void) {
    zt_lang lang = zt_l10n_current_lang();
    if (lang == ZT_LANG_PT) return "pt_BR";
    if (lang == ZT_LANG_ES) return "es";
    if (lang == ZT_LANG_JA) return "ja";
    return "en";
}

static void lsp_locale_base(const char *locale, char *base, size_t capacity) {
    size_t i;
    if (base == NULL || capacity == 0) return;
    if (locale == NULL || locale[0] == '\0') {
        base[0] = '\0';
        return;
    }
    for (i = 0; locale[i] != '\0' && locale[i] != '-' && locale[i] != '_' && i < capacity - 1; i += 1) {
        base[i] = locale[i];
    }
    base[i] = '\0';
}

static char *lsp_read_zdoc_text_for_module_path(const char *ns_path) {
    const char *locale;
    char base[64];
    char relative[PATH_MAX];
    char *text;
    if (ns_path == NULL || ns_path[0] == '\0') return NULL;
    locale = lsp_current_locale_dir();
    snprintf(relative, sizeof(relative), "stdlib/zdoc/%s/%s.zdoc", locale, ns_path);
    text = lsp_read_relative_file_from_roots(relative);
    if (text != NULL) return text;
    lsp_locale_base(locale, base, sizeof(base));
    if (base[0] != '\0' && strcmp(base, locale) != 0) {
        snprintf(relative, sizeof(relative), "stdlib/zdoc/%s/%s.zdoc", base, ns_path);
        text = lsp_read_relative_file_from_roots(relative);
        if (text != NULL) return text;
    }
    snprintf(relative, sizeof(relative), "stdlib/zdoc/%s.zdoc", ns_path);
    return lsp_read_relative_file_from_roots(relative);
}

static int lsp_zdoc_target_name_matches(const char *start, const char *end, const char *target) {
    size_t len;
    if (start == NULL || end == NULL || target == NULL) return 0;
    while (start < end && isspace((unsigned char)*start)) start += 1;
    while (end > start && isspace((unsigned char)*(end - 1))) end -= 1;
    len = (size_t)(end - start);
    return strlen(target) == len && strncmp(start, target, len) == 0;
}

static char *lsp_trimmed_copy(const char *start, size_t len) {
    const char *end;
    if (start == NULL) return NULL;
    end = start + len;
    while (start < end && isspace((unsigned char)*start)) start += 1;
    while (end > start && isspace((unsigned char)*(end - 1))) end -= 1;
    if (end <= start) return NULL;
    return lsp_strndup(start, (size_t)(end - start));
}

static char *lsp_extract_zdoc_target(const char *text, const char *target) {
    const char *cursor;
    const char *marker = "--- @target:";
    size_t marker_len;
    if (text == NULL || target == NULL) return NULL;
    cursor = text;
    marker_len = strlen(marker);
    while ((cursor = strstr(cursor, marker)) != NULL) {
        const char *name_start;
        const char *line_end;
        const char *content_start;
        const char *block_end;
        name_start = cursor + marker_len;
        line_end = strchr(name_start, '\n');
        if (line_end == NULL) line_end = text + strlen(text);
        if (lsp_zdoc_target_name_matches(name_start, line_end, target)) {
            content_start = *line_end == '\n' ? line_end + 1 : line_end;
            block_end = strstr(content_start, "\n---");
            if (block_end == NULL) block_end = text + strlen(text);
            return lsp_trimmed_copy(content_start, (size_t)(block_end - content_start));
        }
        cursor = *line_end == '\n' ? line_end + 1 : line_end;
    }
    return NULL;
}

static char *lsp_zdoc_documentation_for_symbol(const char *module, const char *target) {
    char *ns_path;
    char *zdoc_text;
    char *documentation;
    if (module == NULL || target == NULL || !namespace_is_std(module)) return NULL;
    ns_path = lsp_namespace_to_zdoc_path(module);
    if (ns_path == NULL) return NULL;
    zdoc_text = lsp_read_zdoc_text_for_module_path(ns_path);
    free(ns_path);
    if (zdoc_text == NULL) return NULL;
    documentation = lsp_extract_zdoc_target(zdoc_text, target);
    free(zdoc_text);
    return documentation;
}

static const char *last_namespace_segment(const char *name) {
    const char *dot;
    if (name == NULL) return NULL;
    dot = strrchr(name, '.');
    return dot != NULL ? dot + 1 : name;
}

static int docs_same_namespace(lsp_doc *a, lsp_doc *b) {
    const char *a_ns = doc_module_name(a);
    const char *b_ns = doc_module_name(b);
    return a_ns != NULL && b_ns != NULL && strcmp(a_ns, b_ns) == 0;
}

static const zt_ast_node *find_import_decl_for_alias(lsp_doc *doc, const char *alias) {
    size_t i;
    if (doc == NULL || alias == NULL || !doc->has_parse || doc->parse.root == NULL || doc->parse.root->kind != ZT_AST_FILE) return NULL;
    for (i = 0; i < doc->parse.root->as.file.imports.count; i += 1) {
        const zt_ast_node *import_decl = doc->parse.root->as.file.imports.items[i];
        const char *path;
        const char *local_name;
        if (import_decl == NULL || import_decl->kind != ZT_AST_IMPORT_DECL) continue;
        path = import_decl->as.import_decl.path;
        local_name = import_decl->as.import_decl.alias != NULL
            ? import_decl->as.import_decl.alias
            : last_namespace_segment(path);
        if (local_name != NULL && strcmp(local_name, alias) == 0) return import_decl;
    }
    return NULL;
}

static const char *import_path_for_alias(lsp_doc *doc, const char *alias) {
    const zt_ast_node *import_decl = find_import_decl_for_alias(doc, alias);
    if (import_decl == NULL || import_decl->kind != ZT_AST_IMPORT_DECL) return NULL;
    return import_decl->as.import_decl.path;
}

static const char *import_alias_for_path(lsp_doc *doc, const char *path) {
    size_t i;
    if (doc == NULL || path == NULL || !doc->has_parse || doc->parse.root == NULL || doc->parse.root->kind != ZT_AST_FILE) return NULL;
    for (i = 0; i < doc->parse.root->as.file.imports.count; i += 1) {
        const zt_ast_node *import_decl = doc->parse.root->as.file.imports.items[i];
        if (import_decl == NULL || import_decl->kind != ZT_AST_IMPORT_DECL) continue;
        if (import_decl->as.import_decl.path == NULL || strcmp(import_decl->as.import_decl.path, path) != 0) continue;
        return import_decl->as.import_decl.alias != NULL
            ? import_decl->as.import_decl.alias
            : last_namespace_segment(path);
    }
    return NULL;
}

static lsp_doc *find_doc_by_namespace(const char *namespace_name) {
    lsp_doc *doc;
    if (namespace_name == NULL) return NULL;
    for (doc = g_docs; doc != NULL; doc = doc->next) {
        const char *doc_ns = doc_module_name(doc);
        if (doc_ns != NULL && strcmp(doc_ns, namespace_name) == 0) return doc;
    }
    return NULL;
}

static int symbol_matches(const char *symbol, const char *word) {
    const char *last_dot;
    if (symbol == NULL || word == NULL) return 0;
    if (strcmp(symbol, word) == 0) return 1;
    last_dot = strrchr(word, '.');
    return last_dot != NULL && strcmp(symbol, last_dot + 1) == 0;
}

static const zt_ast_node *find_symbol_in_doc(lsp_doc *doc, const char *word) {
    size_t i;
    if (doc == NULL || !doc->has_parse || doc->parse.root == NULL || word == NULL) return NULL;
    if (doc->parse.root->kind != ZT_AST_FILE) return NULL;
    for (i = 0; i < doc->parse.root->as.file.declarations.count; i += 1) {
        const zt_ast_node *decl = doc->parse.root->as.file.declarations.items[i];
        if (symbol_matches(node_symbol_name(decl), word)) return decl;
    }
    return NULL;
}

static const zt_ast_node *find_symbol(lsp_doc *doc, const char *word) {
    const zt_ast_node *found;
    lsp_doc *other;
    if (word == NULL) return NULL;
    if (strchr(word, '.') != NULL) {
        char *qualified = lsp_strdup(word);
        char *dot;
        const char *import_path;
        if (qualified == NULL) return NULL;
        dot = strchr(qualified, '.');
        *dot = '\0';
        import_path = import_path_for_alias(doc, qualified);
        if (import_path != NULL) {
            lsp_doc *owner = find_doc_by_namespace(import_path);
            found = find_symbol_in_doc(owner, dot + 1);
            if (found != NULL && (node_is_public(found) || namespace_is_std(import_path))) {
                free(qualified);
                return found;
            }
        }
        free(qualified);
        return NULL;
    }
    found = find_symbol_in_doc(doc, word);
    if (found != NULL) return found;
    for (other = g_docs; other != NULL; other = other->next) {
        if (other == doc) continue;
        if (!docs_same_namespace(doc, other)) continue;
        found = find_symbol_in_doc(other, word);
        if (found != NULL) return found;
    }
    return NULL;
}

static const zt_ast_node *find_symbol_with_doc(lsp_doc *doc, const char *word, lsp_doc **owner) {
    const zt_ast_node *found;
    lsp_doc *other;
    if (owner != NULL) *owner = NULL;
    if (word == NULL) return NULL;
    if (strchr(word, '.') != NULL) {
        char *qualified = lsp_strdup(word);
        char *dot;
        const char *import_path;
        if (qualified == NULL) return NULL;
        dot = strchr(qualified, '.');
        *dot = '\0';
        import_path = import_path_for_alias(doc, qualified);
        if (import_path != NULL) {
            lsp_doc *imported_doc = find_doc_by_namespace(import_path);
            found = find_symbol_in_doc(imported_doc, dot + 1);
            if (found != NULL && (node_is_public(found) || namespace_is_std(import_path))) {
                if (owner != NULL) *owner = imported_doc;
                free(qualified);
                return found;
            }
        }
        free(qualified);
        return NULL;
    }
    found = find_symbol_in_doc(doc, word);
    if (found != NULL) {
        if (owner != NULL) *owner = doc;
        return found;
    }
    for (other = g_docs; other != NULL; other = other->next) {
        if (other == doc) continue;
        if (!docs_same_namespace(doc, other)) continue;
        found = find_symbol_in_doc(other, word);
        if (found != NULL) {
            if (owner != NULL) *owner = other;
            return found;
        }
    }
    return NULL;
}

static void append_formatted_node(lsp_sb *sb, const zt_ast_node *node) {
    char *formatted;
    if (node == NULL) return;
    formatted = zt_format_node_to_string(node);
    if (formatted != NULL) {
        sb_append(sb, formatted);
        free(formatted);
    }
}

static char *hover_result_json(lsp_doc *doc, const char *word) {
    const zt_ast_node *node = find_symbol(doc, word);
    lsp_sb md;
    lsp_sb json;
    size_t i;
    if (node == NULL) return NULL;
    sb_init(&md);
    sb_append(&md, "```zenith\n");
    if (node->kind == ZT_AST_FUNC_DECL) {
        sb_append(&md, "func ");
        sb_append(&md, node->as.func_decl.name);
        sb_append(&md, "(");
        for (i = 0; i < node->as.func_decl.params.count; i += 1) {
            if (i > 0) sb_append(&md, ", ");
            append_formatted_node(&md, node->as.func_decl.params.items[i]);
        }
        sb_append(&md, ")");
        if (node->as.func_decl.return_type != NULL) {
            sb_append(&md, " -> ");
            append_formatted_node(&md, node->as.func_decl.return_type);
        }
    } else if (node->kind == ZT_AST_VAR_DECL) {
        sb_append(&md, "var ");
        sb_append(&md, node->as.var_decl.name);
        if (node->as.var_decl.type_node != NULL) {
            sb_append(&md, ": ");
            append_formatted_node(&md, node->as.var_decl.type_node);
        }
    } else if (node->kind == ZT_AST_CONST_DECL) {
        sb_append(&md, "const ");
        sb_append(&md, node->as.const_decl.name);
        if (node->as.const_decl.type_node != NULL) {
            sb_append(&md, ": ");
            append_formatted_node(&md, node->as.const_decl.type_node);
        }
    } else {
        sb_append(&md, node_symbol_kind(node));
        sb_append(&md, " ");
        sb_append(&md, node_symbol_name(node));
    }
    sb_append(&md, "\n```\n");
    sb_append(&md, node_symbol_kind(node));
    sb_append(&md, " declarado neste arquivo.");

    sb_init(&json);
    sb_append(&json, "{\"contents\":{\"kind\":\"markdown\",\"value\":");
    sb_append_json_string(&json, md.data);
    sb_append(&json, "}}");
    free(md.data);
    return sb_take(&json);
}

static char *definition_result_json(lsp_doc *doc, const char *word) {
    lsp_doc *owner = NULL;
    const zt_ast_node *node = find_symbol_with_doc(doc, word, &owner);
    lsp_sb sb;
    int start_line;
    int start_col;
    int end_line;
    int end_col;
    if (owner == NULL || node == NULL) return NULL;
    start_line = lsp_line(node->span.line);
    start_col = lsp_col(node->span.column_start);
    end_line = start_line;
    end_col = lsp_col(node->span.column_end);
    if (end_line == start_line && end_col <= start_col) end_col = start_col + 1;
    sb_init(&sb);
    sb_append(&sb, "{\"uri\":");
    sb_append_json_string(&sb, owner->uri);
    sb_append(&sb, ",\"range\":{\"start\":{");
    sb_appendf(&sb, "\"line\":%d,\"character\":%d", start_line, start_col);
    sb_append(&sb, "},\"end\":{");
    sb_appendf(&sb, "\"line\":%d,\"character\":%d", end_line, end_col);
    sb_append(&sb, "}}}");
    return sb_take(&sb);
}

static void document_end_position(const char *text, int *line, int *character) {
    int l = 0;
    int c = 0;
    const char *p;
    if (text != NULL) {
        for (p = text; *p != '\0'; p += 1) {
            if (*p == '\n') {
                l += 1;
                c = 0;
            } else {
                c += 1;
            }
        }
    }
    *line = l;
    *character = c;
}

static char *formatting_result_json(lsp_doc *doc) {
    char *formatted;
    int end_line;
    int end_char;
    lsp_sb sb;
    if (doc == NULL || !doc->has_parse || doc->parse.root == NULL) return lsp_strdup("[]");
    formatted = zt_format_node_to_string(doc->parse.root);
    if (formatted == NULL) return lsp_strdup("[]");
    if (doc->text != NULL && strcmp(formatted, doc->text) == 0) {
        free(formatted);
        return lsp_strdup("[]");
    }
    document_end_position(doc->text, &end_line, &end_char);
    sb_init(&sb);
    sb_append(&sb, "[{\"range\":{\"start\":{\"line\":0,\"character\":0},\"end\":{");
    sb_appendf(&sb, "\"line\":%d,\"character\":%d", end_line, end_char);
    sb_append(&sb, "}},\"newText\":");
    sb_append_json_string(&sb, formatted);
    sb_append(&sb, "}]");
    free(formatted);
    return sb_take(&sb);
}

static void append_completion_item_full(
        lsp_sb *sb,
        int *first,
        const char *label,
        int kind,
        const char *detail,
        const char *documentation,
        const char *insert_text,
        int insert_text_format) {
    if (!*first) sb_append(sb, ",");
    *first = 0;
    sb_append(sb, "{\"label\":");
    sb_append_json_string(sb, label);
    sb_append(sb, ",\"kind\":");
    sb_appendf(sb, "%d", kind);
    if (detail != NULL) {
        sb_append(sb, ",\"detail\":");
        sb_append_json_string(sb, detail);
    }
    if (documentation != NULL) {
        sb_append(sb, ",\"documentation\":");
        sb_append_json_string(sb, documentation);
    }
    if (insert_text != NULL) {
        sb_append(sb, ",\"insertText\":");
        sb_append_json_string(sb, insert_text);
    }
    if (insert_text_format > 0) {
        sb_append(sb, ",\"insertTextFormat\":");
        sb_appendf(sb, "%d", insert_text_format);
    }
    sb_append(sb, "}");
}

static void append_completion_item(
        lsp_sb *sb,
        int *first,
        const char *label,
        int kind,
        const char *detail,
        const char *insert_text,
        int insert_text_format) {
    append_completion_item_full(sb, first, label, kind, detail, NULL, insert_text, insert_text_format);
}

static int ast_node_starts_before_position(const zt_ast_node *node, int line, int character) {
    int node_line;
    int node_col;
    if (node == NULL || node->span.line == 0) return 1;
    node_line = lsp_line(node->span.line);
    node_col = lsp_col(node->span.column_start);
    return node_line < line || (node_line == line && node_col <= character);
}

static void set_type_candidate(char **candidate, const zt_ast_node *type_node) {
    char *formatted;
    if (candidate == NULL || type_node == NULL) return;
    formatted = zt_format_node_to_string(type_node);
    if (formatted == NULL) return;
    free(*candidate);
    *candidate = formatted;
}

static int type_starts_with_name(const char *type_name, const char *name);

static void find_type_for_binding_in_node(
        const zt_ast_node *node,
        const char *name,
        int line,
        int character,
        char **candidate);

static void find_type_for_binding_in_list(
        zt_ast_node_list list,
        const char *name,
        int line,
        int character,
        char **candidate) {
    size_t i;
    for (i = 0; i < list.count; i += 1) {
        find_type_for_binding_in_node(list.items[i], name, line, character, candidate);
    }
}

static void find_type_for_binding_in_node(
        const zt_ast_node *node,
        const char *name,
        int line,
        int character,
        char **candidate) {
    size_t i;
    if (node == NULL || name == NULL || candidate == NULL) return;

    switch (node->kind) {
        case ZT_AST_FILE:
            find_type_for_binding_in_list(node->as.file.declarations, name, line, character, candidate);
            break;
        case ZT_AST_FUNC_DECL:
            for (i = 0; i < node->as.func_decl.params.count; i += 1) {
                const zt_ast_node *param = node->as.func_decl.params.items[i];
                if (param != NULL &&
                    param->kind == ZT_AST_PARAM &&
                    param->as.param.name != NULL &&
                    strcmp(param->as.param.name, name) == 0) {
                    set_type_candidate(candidate, param->as.param.type_node);
                }
            }
            find_type_for_binding_in_node(node->as.func_decl.body, name, line, character, candidate);
            break;
        case ZT_AST_BLOCK:
            find_type_for_binding_in_list(node->as.block.statements, name, line, character, candidate);
            break;
        case ZT_AST_VAR_DECL:
            if (node->as.var_decl.name != NULL &&
                strcmp(node->as.var_decl.name, name) == 0 &&
                ast_node_starts_before_position(node, line, character)) {
                set_type_candidate(candidate, node->as.var_decl.type_node);
            }
            break;
        case ZT_AST_CONST_DECL:
            if (node->as.const_decl.name != NULL &&
                strcmp(node->as.const_decl.name, name) == 0 &&
                ast_node_starts_before_position(node, line, character)) {
                set_type_candidate(candidate, node->as.const_decl.type_node);
            }
            break;
        case ZT_AST_IF_STMT:
            find_type_for_binding_in_node(node->as.if_stmt.then_block, name, line, character, candidate);
            find_type_for_binding_in_node(node->as.if_stmt.else_block, name, line, character, candidate);
            break;
        case ZT_AST_WHILE_STMT:
            find_type_for_binding_in_node(node->as.while_stmt.body, name, line, character, candidate);
            break;
        case ZT_AST_FOR_STMT:
            if (node->as.for_stmt.item_name != NULL &&
                strcmp(node->as.for_stmt.item_name, name) == 0 &&
                ast_node_starts_before_position(node, line, character)) {
                /* The checker knows the precise item type; the LSP keeps the binding visible here. */
            }
            if (node->as.for_stmt.index_name != NULL &&
                strcmp(node->as.for_stmt.index_name, name) == 0 &&
                ast_node_starts_before_position(node, line, character)) {
                free(*candidate);
                *candidate = lsp_strdup("int");
            }
            find_type_for_binding_in_node(node->as.for_stmt.body, name, line, character, candidate);
            break;
        case ZT_AST_USING_STMT:
            if (node->as.using_stmt.name != NULL &&
                strcmp(node->as.using_stmt.name, name) == 0 &&
                ast_node_starts_before_position(node, line, character)) {
                /* The current public checker API does not expose inferred expression types. */
            }
            find_type_for_binding_in_node(node->as.using_stmt.body, name, line, character, candidate);
            break;
        case ZT_AST_CLOSURE_EXPR:
            for (i = 0; i < node->as.closure_expr.params.count; i += 1) {
                const zt_ast_node *param = node->as.closure_expr.params.items[i];
                if (param != NULL &&
                    param->kind == ZT_AST_PARAM &&
                    param->as.param.name != NULL &&
                    strcmp(param->as.param.name, name) == 0) {
                    set_type_candidate(candidate, param->as.param.type_node);
                }
            }
            find_type_for_binding_in_node(node->as.closure_expr.body, name, line, character, candidate);
            break;
        case ZT_AST_REPEAT_STMT:
            find_type_for_binding_in_node(node->as.repeat_stmt.body, name, line, character, candidate);
            break;
        case ZT_AST_MATCH_STMT:
            find_type_for_binding_in_list(node->as.match_stmt.cases, name, line, character, candidate);
            break;
        case ZT_AST_MATCH_CASE:
            find_type_for_binding_in_list(node->as.match_case.patterns, name, line, character, candidate);
            find_type_for_binding_in_node(node->as.match_case.body, name, line, character, candidate);
            break;
        case ZT_AST_VALUE_BINDING:
            if (node->as.value_binding.name != NULL &&
                strcmp(node->as.value_binding.name, name) == 0 &&
                ast_node_starts_before_position(node, line, character)) {
                set_type_candidate(candidate, node->as.value_binding.type_node);
            }
            break;
        default:
            break;
    }
}

static char *completion_receiver_type(lsp_doc *doc, const char *receiver, int line, int character) {
    char *candidate = NULL;
    if (doc == NULL || receiver == NULL || !doc->has_parse || doc->parse.root == NULL) return NULL;
    find_type_for_binding_in_node(doc->parse.root, receiver, line, character, &candidate);
    return candidate;
}

static const char *skip_type_space(const char *type_name) {
    while (type_name != NULL && isspace((unsigned char)*type_name)) type_name += 1;
    return type_name;
}

static int type_starts_with_name(const char *type_name, const char *name) {
    size_t len;
    const char *type;
    if (type_name == NULL || name == NULL) return 0;
    type = skip_type_space(type_name);
    len = strlen(name);
    if (strncmp(type, name, len) != 0) return 0;
    type += len;
    while (isspace((unsigned char)*type)) type += 1;
    return *type == '\0' || *type == '<';
}

static char *type_base_name(const char *type_name) {
    const char *start = skip_type_space(type_name);
    const char *end = start;
    if (start == NULL || *start == '\0') return NULL;
    while (*end != '\0' && *end != '<' && !isspace((unsigned char)*end)) end += 1;
    if (end <= start) return NULL;
    return lsp_strndup(start, (size_t)(end - start));
}

static int split_qualified_type_base(const char *type_name, char **qualifier, char **short_name) {
    char *base;
    char *dot;
    if (qualifier != NULL) *qualifier = NULL;
    if (short_name != NULL) *short_name = NULL;
    base = type_base_name(type_name);
    if (base == NULL) return 0;
    dot = strrchr(base, '.');
    if (dot == NULL || dot == base || dot[1] == '\0') {
        free(base);
        return 0;
    }
    *dot = '\0';
    if (qualifier != NULL) *qualifier = lsp_strdup(base);
    if (short_name != NULL) *short_name = lsp_strdup(dot + 1);
    free(base);
    if ((qualifier != NULL && *qualifier == NULL) || (short_name != NULL && *short_name == NULL)) {
        if (qualifier != NULL) {
            free(*qualifier);
            *qualifier = NULL;
        }
        if (short_name != NULL) {
            free(*short_name);
            *short_name = NULL;
        }
        return 0;
    }
    return 1;
}

static lsp_doc *find_doc_for_type_qualifier(lsp_doc *doc, const char *qualifier) {
    const char *import_path;
    lsp_doc *qualified_doc;
    if (qualifier == NULL) return NULL;
    import_path = import_path_for_alias(doc, qualifier);
    if (import_path != NULL) {
        qualified_doc = find_doc_by_namespace(import_path);
        if (qualified_doc != NULL) return qualified_doc;
    }
    return find_doc_by_namespace(qualifier);
}

static const zt_ast_node *find_struct_decl_named_in_doc(lsp_doc *doc, const char *type_name, int public_only) {
    const zt_ast_node *result = NULL;
    const char *doc_ns;
    size_t i;
    if (doc == NULL || type_name == NULL || !doc->has_parse || doc->parse.root == NULL) return NULL;
    if (doc->parse.root->kind != ZT_AST_FILE) return NULL;
    doc_ns = doc_module_name(doc);
    for (i = 0; i < doc->parse.root->as.file.declarations.count; i += 1) {
        const zt_ast_node *decl = doc->parse.root->as.file.declarations.items[i];
        if (decl != NULL &&
            decl->kind == ZT_AST_STRUCT_DECL &&
            decl->as.struct_decl.name != NULL &&
            strcmp(decl->as.struct_decl.name, type_name) == 0 &&
            (!public_only || node_is_public(decl) || namespace_is_std(doc_ns))) {
            result = decl;
            break;
        }
    }
    return result;
}

static const zt_ast_node *find_struct_decl_in_doc(lsp_doc *doc, const char *type_name, int public_only) {
    char *base;
    const zt_ast_node *result;
    base = type_base_name(type_name);
    if (base == NULL) return NULL;
    result = find_struct_decl_named_in_doc(doc, base, public_only);
    free(base);
    return result;
}

static const zt_ast_node *find_struct_decl(lsp_doc *doc, const char *type_name) {
    const zt_ast_node *found;
    lsp_doc *other;
    char *qualifier = NULL;
    char *short_name = NULL;
    if (split_qualified_type_base(type_name, &qualifier, &short_name)) {
        lsp_doc *owner = find_doc_for_type_qualifier(doc, qualifier);
        found = find_struct_decl_named_in_doc(owner, short_name, 1);
        free(qualifier);
        free(short_name);
        if (found != NULL) return found;
        return NULL;
    }
    found = find_struct_decl_in_doc(doc, type_name, 0);
    if (found != NULL) return found;
    for (other = g_docs; other != NULL; other = other->next) {
        if (other == doc) continue;
        found = find_struct_decl_in_doc(other, type_name, 0);
        if (found != NULL) return found;
    }
    return NULL;
}

static void append_struct_field_completions(lsp_sb *sb, int *first, const zt_ast_node *struct_decl) {
    size_t i;
    if (struct_decl == NULL || struct_decl->kind != ZT_AST_STRUCT_DECL) return;
    for (i = 0; i < struct_decl->as.struct_decl.fields.count; i += 1) {
        const zt_ast_node *field = struct_decl->as.struct_decl.fields.items[i];
        lsp_sb detail;
        if (field == NULL || field->kind != ZT_AST_STRUCT_FIELD || field->as.struct_field.name == NULL) continue;
        sb_init(&detail);
        sb_append(&detail, struct_decl->as.struct_decl.name != NULL ? struct_decl->as.struct_decl.name : "struct");
        sb_append(&detail, ".");
        sb_append(&detail, field->as.struct_field.name);
        if (field->as.struct_field.type_node != NULL) {
            sb_append(&detail, ": ");
            append_formatted_node(&detail, field->as.struct_field.type_node);
        }
        append_completion_item(sb, first, field->as.struct_field.name, 5, detail.data, NULL, 0);
        free(detail.data);
    }
}

static void append_function_signature_text(lsp_sb *sb, const char *qualifier, const zt_ast_node *func_decl);
static char *function_documentation_markdown(lsp_doc *owner, const char *usage_qualifier, const zt_ast_node *func_decl);

static void append_apply_method_completions_from_doc(
        lsp_sb *sb,
        int *first,
        lsp_doc *doc,
        const char *receiver_type,
        int public_only) {
    char *base;
    const char *doc_ns;
    size_t i;
    if (doc == NULL || receiver_type == NULL || !doc->has_parse || doc->parse.root == NULL) return;
    if (doc->parse.root->kind != ZT_AST_FILE) return;
    base = type_base_name(receiver_type);
    if (base == NULL) return;
    doc_ns = doc_module_name(doc);
    for (i = 0; i < doc->parse.root->as.file.declarations.count; i += 1) {
        const zt_ast_node *decl = doc->parse.root->as.file.declarations.items[i];
        size_t m;
        if (decl == NULL || decl->kind != ZT_AST_APPLY_DECL || decl->as.apply_decl.target_name == NULL) continue;
        if (strcmp(decl->as.apply_decl.target_name, base) != 0) continue;
        for (m = 0; m < decl->as.apply_decl.methods.count; m += 1) {
            const zt_ast_node *method = decl->as.apply_decl.methods.items[m];
            lsp_sb detail;
            lsp_sb insert_text;
            char *documentation;
            if (method == NULL || method->kind != ZT_AST_FUNC_DECL || method->as.func_decl.name == NULL) continue;
            if (public_only && !node_is_public(method) && !namespace_is_std(doc_ns)) continue;
            sb_init(&detail);
            append_function_signature_text(&detail, NULL, method);
            documentation = function_documentation_markdown(doc, "objeto", method);
            sb_init(&insert_text);
            sb_append(&insert_text, method->as.func_decl.name);
            sb_append(&insert_text, "(${0})");
            append_completion_item_full(
                sb,
                first,
                method->as.func_decl.name,
                2,
                detail.data != NULL && detail.data[0] != '\0'
                    ? detail.data
                    : (method->as.func_decl.is_mutating ? "metodo mutating via apply" : "metodo via apply"),
                documentation,
                insert_text.data,
                2);
            free(insert_text.data);
            free(documentation);
            free(detail.data);
        }
    }
    free(base);
}

static void append_apply_method_completions(lsp_sb *sb, int *first, lsp_doc *doc, const char *receiver_type) {
    lsp_doc *other;
    char *qualifier = NULL;
    char *short_name = NULL;
    if (split_qualified_type_base(receiver_type, &qualifier, &short_name)) {
        lsp_doc *owner = find_doc_for_type_qualifier(doc, qualifier);
        append_apply_method_completions_from_doc(sb, first, owner, short_name, 1);
        free(qualifier);
        free(short_name);
        return;
    }
    append_apply_method_completions_from_doc(sb, first, doc, receiver_type, 0);
    for (other = g_docs; other != NULL; other = other->next) {
        if (other == doc) continue;
        append_apply_method_completions_from_doc(sb, first, other, receiver_type, 0);
    }
}

static const zt_ast_node *find_apply_method_in_doc(
        lsp_doc *doc,
        const char *receiver_type,
        const char *method_name,
        int public_only) {
    char *base;
    const zt_ast_node *result = NULL;
    const char *doc_ns;
    size_t i;
    if (doc == NULL || receiver_type == NULL || method_name == NULL || !doc->has_parse || doc->parse.root == NULL) return NULL;
    if (doc->parse.root->kind != ZT_AST_FILE) return NULL;
    base = type_base_name(receiver_type);
    if (base == NULL) return NULL;
    doc_ns = doc_module_name(doc);
    for (i = 0; i < doc->parse.root->as.file.declarations.count; i += 1) {
        const zt_ast_node *decl = doc->parse.root->as.file.declarations.items[i];
        size_t m;
        if (decl == NULL || decl->kind != ZT_AST_APPLY_DECL || decl->as.apply_decl.target_name == NULL) continue;
        if (strcmp(decl->as.apply_decl.target_name, base) != 0) continue;
        for (m = 0; m < decl->as.apply_decl.methods.count; m += 1) {
            const zt_ast_node *method = decl->as.apply_decl.methods.items[m];
            if (method == NULL || method->kind != ZT_AST_FUNC_DECL || method->as.func_decl.name == NULL) continue;
            if (strcmp(method->as.func_decl.name, method_name) != 0) continue;
            if (public_only && !node_is_public(method) && !namespace_is_std(doc_ns)) continue;
            result = method;
            break;
        }
        if (result != NULL) break;
    }
    free(base);
    return result;
}

static const zt_ast_node *find_apply_method(lsp_doc *doc, const char *receiver_type, const char *method_name, lsp_doc **owner) {
    lsp_doc *other;
    const zt_ast_node *found;
    char *qualifier = NULL;
    char *short_name = NULL;
    if (owner != NULL) *owner = NULL;
    if (split_qualified_type_base(receiver_type, &qualifier, &short_name)) {
        lsp_doc *qualified_owner = find_doc_for_type_qualifier(doc, qualifier);
        found = find_apply_method_in_doc(qualified_owner, short_name, method_name, 1);
        if (found != NULL && owner != NULL) *owner = qualified_owner;
        free(qualifier);
        free(short_name);
        return found;
    }
    found = find_apply_method_in_doc(doc, receiver_type, method_name, 0);
    if (found != NULL) {
        if (owner != NULL) *owner = doc;
        return found;
    }
    for (other = g_docs; other != NULL; other = other->next) {
        if (other == doc) continue;
        found = find_apply_method_in_doc(other, receiver_type, method_name, 0);
        if (found != NULL) {
            if (owner != NULL) *owner = other;
            return found;
        }
    }
    return NULL;
}

static void append_local_completions_from_node(
        lsp_sb *sb,
        int *first,
        const zt_ast_node *node,
        int line,
        int character);

static void append_local_completions_from_list(
        lsp_sb *sb,
        int *first,
        zt_ast_node_list list,
        int line,
        int character) {
    size_t i;
    for (i = 0; i < list.count; i += 1) {
        append_local_completions_from_node(sb, first, list.items[i], line, character);
    }
}

static void append_local_completions_from_node(
        lsp_sb *sb,
        int *first,
        const zt_ast_node *node,
        int line,
        int character) {
    size_t i;
    if (node == NULL) return;
    switch (node->kind) {
        case ZT_AST_FILE:
            append_local_completions_from_list(sb, first, node->as.file.declarations, line, character);
            break;
        case ZT_AST_FUNC_DECL:
            for (i = 0; i < node->as.func_decl.params.count; i += 1) {
                const zt_ast_node *param = node->as.func_decl.params.items[i];
                lsp_sb detail;
                if (param == NULL || param->kind != ZT_AST_PARAM || param->as.param.name == NULL) continue;
                sb_init(&detail);
                sb_append(&detail, "param");
                if (param->as.param.type_node != NULL) {
                    sb_append(&detail, ": ");
                    append_formatted_node(&detail, param->as.param.type_node);
                }
                append_completion_item(sb, first, param->as.param.name, 6, detail.data, NULL, 0);
                free(detail.data);
            }
            append_local_completions_from_node(sb, first, node->as.func_decl.body, line, character);
            break;
        case ZT_AST_BLOCK:
            append_local_completions_from_list(sb, first, node->as.block.statements, line, character);
            break;
        case ZT_AST_VAR_DECL:
            if (node->as.var_decl.name != NULL && ast_node_starts_before_position(node, line, character)) {
                append_completion_item(sb, first, node->as.var_decl.name, 6, "var local", NULL, 0);
            }
            append_local_completions_from_node(sb, first, node->as.var_decl.init_value, line, character);
            break;
        case ZT_AST_CONST_DECL:
            if (node->as.const_decl.name != NULL && ast_node_starts_before_position(node, line, character)) {
                append_completion_item(sb, first, node->as.const_decl.name, 21, "const local", NULL, 0);
            }
            append_local_completions_from_node(sb, first, node->as.const_decl.init_value, line, character);
            break;
        case ZT_AST_IF_STMT:
            append_local_completions_from_node(sb, first, node->as.if_stmt.then_block, line, character);
            append_local_completions_from_node(sb, first, node->as.if_stmt.else_block, line, character);
            break;
        case ZT_AST_WHILE_STMT:
            append_local_completions_from_node(sb, first, node->as.while_stmt.body, line, character);
            break;
        case ZT_AST_FOR_STMT:
            if (node->as.for_stmt.item_name != NULL && ast_node_starts_before_position(node, line, character)) {
                append_completion_item(sb, first, node->as.for_stmt.item_name, 6, "item do for", NULL, 0);
            }
            if (node->as.for_stmt.index_name != NULL && ast_node_starts_before_position(node, line, character)) {
                append_completion_item(sb, first, node->as.for_stmt.index_name, 6, "indice/chave do for", NULL, 0);
            }
            append_local_completions_from_node(sb, first, node->as.for_stmt.body, line, character);
            break;
        case ZT_AST_USING_STMT:
            if (node->as.using_stmt.name != NULL && ast_node_starts_before_position(node, line, character)) {
                append_completion_item(sb, first, node->as.using_stmt.name, 6, "binding using", NULL, 0);
            }
            append_local_completions_from_node(sb, first, node->as.using_stmt.body, line, character);
            break;
        case ZT_AST_CLOSURE_EXPR:
            for (i = 0; i < node->as.closure_expr.params.count; i += 1) {
                const zt_ast_node *param = node->as.closure_expr.params.items[i];
                lsp_sb detail;
                if (param == NULL || param->kind != ZT_AST_PARAM || param->as.param.name == NULL) continue;
                sb_init(&detail);
                sb_append(&detail, "param closure");
                if (param->as.param.type_node != NULL) {
                    sb_append(&detail, ": ");
                    append_formatted_node(&detail, param->as.param.type_node);
                }
                append_completion_item(sb, first, param->as.param.name, 6, detail.data, NULL, 0);
                free(detail.data);
            }
            append_local_completions_from_node(sb, first, node->as.closure_expr.body, line, character);
            break;
        case ZT_AST_REPEAT_STMT:
            append_local_completions_from_node(sb, first, node->as.repeat_stmt.body, line, character);
            break;
        case ZT_AST_MATCH_STMT:
            append_local_completions_from_list(sb, first, node->as.match_stmt.cases, line, character);
            break;
        case ZT_AST_MATCH_CASE:
            append_local_completions_from_list(sb, first, node->as.match_case.patterns, line, character);
            append_local_completions_from_node(sb, first, node->as.match_case.body, line, character);
            break;
        case ZT_AST_EXPR_STMT:
            append_local_completions_from_node(sb, first, node->as.expr_stmt.expr, line, character);
            break;
        case ZT_AST_RETURN_STMT:
            append_local_completions_from_node(sb, first, node->as.return_stmt.value, line, character);
            break;
        case ZT_AST_ASSIGN_STMT:
            append_local_completions_from_node(sb, first, node->as.assign_stmt.value, line, character);
            break;
        case ZT_AST_INDEX_ASSIGN_STMT:
            append_local_completions_from_node(sb, first, node->as.index_assign_stmt.object, line, character);
            append_local_completions_from_node(sb, first, node->as.index_assign_stmt.index, line, character);
            append_local_completions_from_node(sb, first, node->as.index_assign_stmt.value, line, character);
            break;
        case ZT_AST_FIELD_ASSIGN_STMT:
            append_local_completions_from_node(sb, first, node->as.field_assign_stmt.object, line, character);
            append_local_completions_from_node(sb, first, node->as.field_assign_stmt.value, line, character);
            break;
        case ZT_AST_BINARY_EXPR:
            append_local_completions_from_node(sb, first, node->as.binary_expr.left, line, character);
            append_local_completions_from_node(sb, first, node->as.binary_expr.right, line, character);
            break;
        case ZT_AST_UNARY_EXPR:
            append_local_completions_from_node(sb, first, node->as.unary_expr.operand, line, character);
            break;
        case ZT_AST_CALL_EXPR:
            append_local_completions_from_node(sb, first, node->as.call_expr.callee, line, character);
            append_local_completions_from_list(sb, first, node->as.call_expr.positional_args, line, character);
            for (i = 0; i < node->as.call_expr.named_args.count; i += 1) {
                append_local_completions_from_node(sb, first, node->as.call_expr.named_args.items[i].value, line, character);
            }
            break;
        case ZT_AST_FIELD_EXPR:
            append_local_completions_from_node(sb, first, node->as.field_expr.object, line, character);
            break;
        case ZT_AST_INDEX_EXPR:
            append_local_completions_from_node(sb, first, node->as.index_expr.object, line, character);
            append_local_completions_from_node(sb, first, node->as.index_expr.index, line, character);
            break;
        case ZT_AST_SLICE_EXPR:
            append_local_completions_from_node(sb, first, node->as.slice_expr.object, line, character);
            append_local_completions_from_node(sb, first, node->as.slice_expr.start, line, character);
            append_local_completions_from_node(sb, first, node->as.slice_expr.end, line, character);
            break;
        case ZT_AST_SUCCESS_EXPR:
            append_local_completions_from_node(sb, first, node->as.success_expr.value, line, character);
            break;
        case ZT_AST_ERROR_EXPR:
            append_local_completions_from_node(sb, first, node->as.error_expr.value, line, character);
            break;
        case ZT_AST_LIST_EXPR:
        case ZT_AST_SET_EXPR:
            append_local_completions_from_list(sb, first, node->kind == ZT_AST_LIST_EXPR ? node->as.list_expr.elements : node->as.set_expr.elements, line, character);
            break;
        case ZT_AST_MAP_EXPR:
            for (i = 0; i < node->as.map_expr.entries.count; i += 1) {
                append_local_completions_from_node(sb, first, node->as.map_expr.entries.items[i].key, line, character);
                append_local_completions_from_node(sb, first, node->as.map_expr.entries.items[i].value, line, character);
            }
            break;
        case ZT_AST_STRUCT_LITERAL_EXPR:
            for (i = 0; i < node->as.struct_literal_expr.fields.count; i += 1) {
                append_local_completions_from_node(sb, first, node->as.struct_literal_expr.fields.items[i].value, line, character);
            }
            break;
        case ZT_AST_FMT_EXPR:
            append_local_completions_from_list(sb, first, node->as.fmt_expr.parts, line, character);
            break;
        case ZT_AST_GROUPED_EXPR:
            append_local_completions_from_node(sb, first, node->as.grouped_expr.inner, line, character);
            break;
        case ZT_AST_IF_EXPR:
            append_local_completions_from_node(sb, first, node->as.if_expr.condition, line, character);
            append_local_completions_from_node(sb, first, node->as.if_expr.then_expr, line, character);
            append_local_completions_from_node(sb, first, node->as.if_expr.else_expr, line, character);
            break;
        case ZT_AST_VALUE_BINDING:
            if (node->as.value_binding.name != NULL && ast_node_starts_before_position(node, line, character)) {
                append_completion_item(sb, first, node->as.value_binding.name, 6, "binding de match", NULL, 0);
            }
            break;
        case ZT_AST_MATCH_BINDING:
            if (node->as.match_binding.param_name != NULL && ast_node_starts_before_position(node, line, character)) {
                append_completion_item(sb, first, node->as.match_binding.param_name, 6, "binding de match", NULL, 0);
            }
            break;
        default:
            break;
    }
}

static char *builtin_documentation_markdown(const char *signature, const char *summary, const char *usage) {
    lsp_sb doc;
    sb_init(&doc);
    if (summary != NULL && summary[0] != '\0') {
        sb_append(&doc, lsp_doc_heading_what());
        sb_append(&doc, "\n");
        sb_append(&doc, summary);
        sb_append(&doc, "\n\n");
    }
    if (signature != NULL && signature[0] != '\0') {
        sb_append(&doc, lsp_doc_heading_signature());
        sb_append(&doc, "\n```zt\n");
        sb_append(&doc, signature);
        sb_append(&doc, "\n```\n\n");
    }
    if (usage != NULL && usage[0] != '\0') {
        sb_append(&doc, lsp_doc_heading_usage());
        sb_append(&doc, "\n```zt\n");
        sb_append(&doc, usage);
        sb_append(&doc, "\n```");
    }
    return sb_take(&doc);
}

static void append_builtin_function_completion(
        lsp_sb *sb,
        int *first,
        const char *label,
        const char *signature,
        const char *summary,
        const char *usage,
        const char *insert_text) {
    char *documentation = builtin_documentation_markdown(signature, summary, usage);
    append_completion_item_full(sb, first, label, 3, signature, documentation, insert_text, insert_text != NULL ? 2 : 0);
    free(documentation);
}

static void append_builtin_completions(lsp_sb *sb, int *first) {
    static const char *numeric_conversions[] = {
        "int", "int8", "int16", "int32", "int64",
        "u8", "u16", "u32", "u64",
        "uint8", "uint16", "uint32", "uint64",
        "float", "float32", "float64",
        NULL
    };
    size_t i;

    append_completion_item(sb, first, "namespace", 14, "declara namespace do arquivo", NULL, 0);
    append_completion_item(sb, first, "import", 14, "importa outro namespace", "import ${1:modulo} as ${2:alias}", 2);
    append_completion_item(sb, first, "as", 14, "define alias de import", NULL, 0);
    append_completion_item(sb, first, "func", 14, "declara funcao", "func ${1:nome}(${2}) -> ${3:void}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "return", 14, "retorna de uma funcao", NULL, 0);
    append_completion_item(sb, first, "var", 14, "binding mutavel local", "var ${1:nome}: ${2:tipo} = ${0:valor}", 2);
    append_completion_item(sb, first, "const", 14, "binding imutavel", "const ${1:nome}: ${2:tipo} = ${0:valor}", 2);
    append_completion_item(sb, first, "public", 14, "exporta API do namespace", NULL, 0);
    append_completion_item(sb, first, "mut", 14, "marca metodo mutating ou receiver mutavel", NULL, 0);
    append_completion_item(sb, first, "struct", 14, "declara struct", "struct ${1:Nome}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "trait", 14, "declara trait", "trait ${1:Nome}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "apply", 14, "implementa trait para tipo", "apply ${1:Trait} to ${2:Tipo}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "to", 14, "liga apply ao tipo alvo", NULL, 0);
    append_completion_item(sb, first, "enum", 14, "declara enum", "enum ${1:Nome}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "if", 14, "controle condicional", "if ${1:condicao}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "else", 14, "ramo alternativo", NULL, 0);
    append_completion_item(sb, first, "match", 14, "pattern matching", "match ${1:valor}\ncase ${2:padrao}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "case", 14, "ramo de match", "case ${1:padrao}\n    ${0}", 2);
    append_completion_item(sb, first, "default", 14, "ramo padrao de match", "default\n    ${0}", 2);
    append_completion_item(sb, first, "while", 14, "loop condicional", "while ${1:condicao}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "for", 14, "loop por colecao", "for ${1:item} in ${2:colecao}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "in", 14, "separa binding e iteravel no for", NULL, 0);
    append_completion_item(sb, first, "repeat", 14, "repete um bloco N vezes", "repeat ${1:n} times\n    ${0}\nend", 2);
    append_completion_item(sb, first, "times", 14, "marca contagem do repeat", NULL, 0);
    append_completion_item(sb, first, "break", 14, "sai do loop atual", NULL, 0);
    append_completion_item(sb, first, "continue", 14, "vai para a proxima iteracao", NULL, 0);
    append_completion_item(sb, first, "where", 14, "contrato local de valor", "where ${1:valor} is ${0:condicao}", 2);
    append_completion_item(sb, first, "is", 14, "operador de contrato/padrao", NULL, 0);
    append_completion_item(sb, first, "and", 14, "conjuncao booleana", NULL, 0);
    append_completion_item(sb, first, "or", 14, "disjuncao booleana", NULL, 0);
    append_completion_item(sb, first, "not", 14, "negacao booleana", NULL, 0);
    append_completion_item(sb, first, "extern", 14, "declara bloco externo", "extern ${1:c}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "attr", 14, "atributo de declaracao", "attr ${1:nome}", 2);
    append_completion_item(sb, first, "type", 14, "alias de tipo", "type ${1:Nome} = ${0:Tipo}", 2);
    append_completion_item(sb, first, "capture", 14, "binding capturado para closure", "capture ${1:nome}: ${2:tipo} = ${0:valor}", 2);
    append_completion_item(sb, first, "using", 14, "escopo com limpeza explicita", "using ${1:nome} = ${2:valor}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "self", 6, "receiver implicito em metodo", NULL, 0);
    append_completion_item(sb, first, "end", 14, "fecha bloco", NULL, 0);

    append_builtin_function_completion(
        sb,
        first,
        "fmt",
        "fmt \"texto {valor}\" -> text",
        lsp_doc_text(
            "Interpolacao de text. Avalia os trechos entre chaves e converte cada valor por TextRepresentable<T>. Use para montar mensagens legiveis sem concatenacao manual.",
            "Text interpolation. Evaluates expressions inside braces and converts each value through TextRepresentable<T>. Use it to build readable messages without manual concatenation."),
        "fmt \"Ola {nome}\"",
        "fmt \"${1:texto {valor}}\"");
    append_builtin_function_completion(
        sb,
        first,
        "len()",
        "len(value) -> int",
        lsp_doc_text(
            "Retorna a quantidade de itens de um valor composto: caracteres de text, bytes de bytes, elementos de list<T> ou pares de map<K, V>.",
            "Returns the item count of a compound value: characters in text, bytes in bytes, elements in list<T>, or pairs in map<K, V>."),
        "len(value)",
        "len(${1:value})");
    append_builtin_function_completion(
        sb,
        first,
        "to_text()",
        "to_text(value) -> text",
        lsp_doc_text(
            "Converte para text quando o tipo implementa TextRepresentable<T>. Use para exibicao, logs e mensagens.",
            "Converts to text when the type implements TextRepresentable<T>. Use it for display, logs, and messages."),
        "to_text(value)",
        "to_text(${1:value})");
    append_builtin_function_completion(
        sb,
        first,
        "check()",
        "check(condition, message?) -> void",
        lsp_doc_text(
            "Assercao simples de runtime. Se condition for false, a execucao falha. Use message para explicar a falha.",
            "Simple runtime assertion. If condition is false, execution fails. Use message to explain the failure."),
        "check(condition, message?)",
        "check(${1:condition}, ${2:message})");
    append_builtin_function_completion(
        sb,
        first,
        "todo()",
        "todo(message) -> void",
        lsp_doc_text(
            "Marca um caminho conhecido como incompleto e falha em runtime com mensagem clara.",
            "Marks a known incomplete path and fails at runtime with a clear message."),
        "todo(message)",
        "todo(${1:message})");
    append_builtin_function_completion(
        sb,
        first,
        "unreachable()",
        "unreachable(message) -> void",
        lsp_doc_text(
            "Marca um caminho que deveria ser impossivel e falha se ele for executado.",
            "Marks a path that should be impossible and fails if it is executed."),
        "unreachable(message)",
        "unreachable(${1:message})");
    append_builtin_function_completion(
        sb,
        first,
        "panic()",
        "panic(message) -> void",
        lsp_doc_text(
            "Interrompe a execucao imediatamente com uma mensagem. Use para estados impossiveis ou falhas irrecuperaveis.",
            "Stops execution immediately with a message. Use it for impossible states or unrecoverable failures."),
        "panic(message)",
        "panic(${1:message})");
    for (i = 0; numeric_conversions[i] != NULL; i += 1) {
        lsp_sb label;
        lsp_sb insert_text;
        lsp_sb detail;
        char *documentation;
        sb_init(&label);
        sb_init(&insert_text);
        sb_init(&detail);
        sb_append(&label, numeric_conversions[i]);
        sb_append(&label, "()");
        sb_append(&insert_text, numeric_conversions[i]);
        sb_append(&insert_text, "(${1:value})");
        sb_append(&detail, numeric_conversions[i]);
        sb_append(&detail, "(value) -> ");
        sb_append(&detail, numeric_conversions[i]);
        documentation = builtin_documentation_markdown(
            detail.data,
            lsp_doc_text(
                "Conversao numerica explicita para este tipo. Use quando a mudanca de representacao for intencional e visivel no codigo.",
                "Explicit numeric conversion to this type. Use it when the representation change is intentional and visible in code."),
            insert_text.data);
        append_completion_item_full(sb, first, label.data, 3, detail.data, documentation, insert_text.data, 2);
        free(documentation);
        free(label.data);
        free(insert_text.data);
        free(detail.data);
    }

    append_completion_item(sb, first, "int", 7, "tipo inteiro", NULL, 0);
    append_completion_item(sb, first, "int8", 7, "tipo inteiro 8-bit", NULL, 0);
    append_completion_item(sb, first, "int16", 7, "tipo inteiro 16-bit", NULL, 0);
    append_completion_item(sb, first, "int32", 7, "tipo inteiro 32-bit", NULL, 0);
    append_completion_item(sb, first, "int64", 7, "tipo inteiro 64-bit", NULL, 0);
    append_completion_item(sb, first, "u8", 7, "tipo inteiro unsigned 8-bit", NULL, 0);
    append_completion_item(sb, first, "u16", 7, "tipo inteiro unsigned 16-bit", NULL, 0);
    append_completion_item(sb, first, "u32", 7, "tipo inteiro unsigned 32-bit", NULL, 0);
    append_completion_item(sb, first, "u64", 7, "tipo inteiro unsigned 64-bit", NULL, 0);
    append_completion_item(sb, first, "uint8", 7, "tipo inteiro unsigned 8-bit", NULL, 0);
    append_completion_item(sb, first, "uint16", 7, "tipo inteiro unsigned 16-bit", NULL, 0);
    append_completion_item(sb, first, "uint32", 7, "tipo inteiro unsigned 32-bit", NULL, 0);
    append_completion_item(sb, first, "uint64", 7, "tipo inteiro unsigned 64-bit", NULL, 0);
    append_completion_item(sb, first, "float", 7, "tipo decimal", NULL, 0);
    append_completion_item(sb, first, "float32", 7, "tipo decimal 32-bit", NULL, 0);
    append_completion_item(sb, first, "float64", 7, "tipo decimal 64-bit", NULL, 0);
    append_completion_item(sb, first, "bool", 7, "tipo booleano", NULL, 0);
    append_completion_item(sb, first, "text", 7, "tipo texto", NULL, 0);
    append_completion_item(sb, first, "bytes", 7, "tipo bytes", NULL, 0);
    append_completion_item(sb, first, "void", 7, "sem valor de retorno", NULL, 0);
    append_completion_item(sb, first, "optional", 7, "tipo optional<T>", "optional<${1:T}>", 2);
    append_completion_item(sb, first, "result", 7, "tipo result<T, E>", "result<${1:T}, ${2:core.Error}>", 2);
    append_completion_item(sb, first, "list", 7, "tipo list<T>", "list<${1:T}>", 2);
    append_completion_item(sb, first, "map", 7, "tipo map<K, V>", "map<${1:K}, ${2:V}>", 2);
    append_completion_item(sb, first, "set", 7, "tipo set<T>", "set<${1:T}>", 2);
    append_completion_item(sb, first, "any", 7, "despacho dinamico por trait", "any<${1:Trait}>", 2);
    append_completion_item(sb, first, "dyn", 7, "forma antiga de despacho dinamico; prefira any", "any<${1:Trait}>", 2);
    append_completion_item(sb, first, "grid2d", 7, "tipo grid2d<T>", "grid2d<${1:T}>", 2);
    append_completion_item(sb, first, "pqueue", 7, "tipo pqueue<T>", "pqueue<${1:T}>", 2);
    append_completion_item(sb, first, "circbuf", 7, "tipo circbuf<T>", "circbuf<${1:T}>", 2);
    append_completion_item(sb, first, "btreemap", 7, "tipo btreemap<K, V>", "btreemap<${1:K}, ${2:V}>", 2);
    append_completion_item(sb, first, "btreeset", 7, "tipo btreeset<T>", "btreeset<${1:T}>", 2);
    append_completion_item(sb, first, "grid3d", 7, "tipo grid3d<T>", "grid3d<${1:T}>", 2);
    append_completion_item(sb, first, "core.Error", 7, "tipo de erro padrao", NULL, 0);

    append_completion_item(sb, first, "true", 12, "literal bool", NULL, 0);
    append_completion_item(sb, first, "false", 12, "literal bool", NULL, 0);
    append_completion_item(sb, first, "none", 12, "ausencia optional", NULL, 0);
    append_completion_item(sb, first, "some", 12, "padrao optional com valor", "some(${0})", 2);
    append_builtin_function_completion(
        sb,
        first,
        "success",
        "success(value) -> result<T, E>",
        lsp_doc_text(
            "Cria o ramo de sucesso de um result<T, E>. Use para retornar um valor valido de uma operacao que pode falhar.",
            "Creates the success branch of result<T, E>. Use it to return a valid value from an operation that can fail."),
        "success(value)",
        "success(${0})");
    append_builtin_function_completion(
        sb,
        first,
        "error",
        "error(value) -> result<T, E>",
        lsp_doc_text(
            "Cria o ramo de erro de um result<T, E>. Use para retornar a causa de falha sem lancar excecao.",
            "Creates the error branch of result<T, E>. Use it to return the failure cause without throwing an exception."),
        "error(value)",
        "error(${0})");
}

static int completion_kind_for_node(const zt_ast_node *node) {
    if (node == NULL) return 6;
    switch (node->kind) {
        case ZT_AST_FUNC_DECL: return 3;
        case ZT_AST_STRUCT_DECL: return 22;
        case ZT_AST_TRAIT_DECL: return 8;
        case ZT_AST_ENUM_DECL: return 13;
        case ZT_AST_CONST_DECL: return 21;
        case ZT_AST_VAR_DECL: return 6;
        default: return 6;
    }
}

static int symbol_kind_for_node(const zt_ast_node *node) {
    if (node == NULL) return 13;
    switch (node->kind) {
        case ZT_AST_FUNC_DECL: return 12;
        case ZT_AST_STRUCT_DECL: return 23;
        case ZT_AST_TRAIT_DECL: return 11;
        case ZT_AST_ENUM_DECL: return 10;
        case ZT_AST_STRUCT_FIELD: return 8;
        case ZT_AST_TRAIT_METHOD: return 6;
        case ZT_AST_ENUM_VARIANT: return 22;
        case ZT_AST_CONST_DECL: return 14;
        case ZT_AST_VAR_DECL: return 13;
        case ZT_AST_EXTERN_DECL: return 2;
        case ZT_AST_APPLY_DECL: return 23;
        default: return 13;
    }
}

static void append_lsp_range(lsp_sb *sb, const zt_source_span *span) {
    int start_line = 0;
    int start_col = 0;
    int end_line = 0;
    int end_col = 1;
    if (span != NULL && span->line > 0) {
        start_line = lsp_line(span->line);
        start_col = lsp_col(span->column_start);
        end_line = start_line;
        end_col = lsp_col(span->column_end);
        if (end_col <= start_col) end_col = start_col + 1;
    }
    sb_append(sb, "{\"start\":{");
    sb_appendf(sb, "\"line\":%d,\"character\":%d", start_line, start_col);
    sb_append(sb, "},\"end\":{");
    sb_appendf(sb, "\"line\":%d,\"character\":%d", end_line, end_col);
    sb_append(sb, "}}");
}

static void symbol_display_name(const zt_ast_node *node, char *buffer, size_t capacity) {
    const char *name = node_symbol_name(node);
    if (buffer == NULL || capacity == 0) return;
    buffer[0] = '\0';
    if (name != NULL) {
        snprintf(buffer, capacity, "%s", name);
        return;
    }
    if (node == NULL) {
        snprintf(buffer, capacity, "symbol");
        return;
    }
    switch (node->kind) {
        case ZT_AST_STRUCT_FIELD:
            snprintf(buffer, capacity, "%s", node->as.struct_field.name != NULL ? node->as.struct_field.name : "field");
            break;
        case ZT_AST_TRAIT_METHOD:
            snprintf(buffer, capacity, "%s", node->as.trait_method.name != NULL ? node->as.trait_method.name : "method");
            break;
        case ZT_AST_ENUM_VARIANT:
            snprintf(buffer, capacity, "%s", node->as.enum_variant.name != NULL ? node->as.enum_variant.name : "variant");
            break;
        case ZT_AST_EXTERN_DECL:
            snprintf(buffer, capacity, "extern %s", node->as.extern_decl.binding != NULL ? node->as.extern_decl.binding : "binding");
            break;
        case ZT_AST_APPLY_DECL:
            if (node->as.apply_decl.trait_name != NULL) {
                snprintf(buffer, capacity, "apply %s for %s",
                    node->as.apply_decl.trait_name,
                    node->as.apply_decl.target_name != NULL ? node->as.apply_decl.target_name : "type");
            } else {
                snprintf(buffer, capacity, "apply %s",
                    node->as.apply_decl.target_name != NULL ? node->as.apply_decl.target_name : "type");
            }
            break;
        default:
            snprintf(buffer, capacity, "%s", node_symbol_kind(node));
            break;
    }
}

static const zt_ast_node_list *symbol_children_for_node(const zt_ast_node *node) {
    if (node == NULL) return NULL;
    switch (node->kind) {
        case ZT_AST_STRUCT_DECL: return &node->as.struct_decl.fields;
        case ZT_AST_TRAIT_DECL: return &node->as.trait_decl.methods;
        case ZT_AST_ENUM_DECL: return &node->as.enum_decl.variants;
        case ZT_AST_APPLY_DECL: return &node->as.apply_decl.methods;
        case ZT_AST_EXTERN_DECL: return &node->as.extern_decl.functions;
        default: return NULL;
    }
}

static void append_document_symbol(lsp_sb *sb, int *first, const zt_ast_node *node) {
    char name[256];
    const zt_ast_node_list *children;
    size_t i;
    int child_first = 1;
    if (node == NULL) return;
    symbol_display_name(node, name, sizeof(name));
    if (name[0] == '\0') return;
    if (!*first) sb_append(sb, ",");
    *first = 0;
    sb_append(sb, "{\"name\":");
    sb_append_json_string(sb, name);
    sb_append(sb, ",\"kind\":");
    sb_appendf(sb, "%d", symbol_kind_for_node(node));
    sb_append(sb, ",\"detail\":");
    sb_append_json_string(sb, node_symbol_kind(node));
    sb_append(sb, ",\"range\":");
    append_lsp_range(sb, &node->span);
    sb_append(sb, ",\"selectionRange\":");
    append_lsp_range(sb, &node->span);
    children = symbol_children_for_node(node);
    if (children != NULL && children->count > 0) {
        sb_append(sb, ",\"children\":[");
        for (i = 0; i < children->count; i += 1) {
            append_document_symbol(sb, &child_first, children->items[i]);
        }
        sb_append(sb, "]");
    }
    sb_append(sb, "}");
}

static char *document_symbol_result_json(lsp_doc *doc) {
    lsp_sb sb;
    int first = 1;
    size_t i;
    sb_init(&sb);
    sb_append(&sb, "[");
    if (doc != NULL && doc->has_parse && doc->parse.root != NULL && doc->parse.root->kind == ZT_AST_FILE) {
        for (i = 0; i < doc->parse.root->as.file.declarations.count; i += 1) {
            append_document_symbol(&sb, &first, doc->parse.root->as.file.declarations.items[i]);
        }
    }
    sb_append(&sb, "]");
    return sb_take(&sb);
}

static int ascii_contains_ci(const char *text, const char *query) {
    size_t text_len;
    size_t query_len;
    size_t i;
    if (text == NULL || query == NULL || query[0] == '\0') return 1;
    text_len = strlen(text);
    query_len = strlen(query);
    if (query_len > text_len) return 0;
    for (i = 0; i + query_len <= text_len; i += 1) {
        size_t j;
        int ok = 1;
        for (j = 0; j < query_len; j += 1) {
            if (tolower((unsigned char)text[i + j]) != tolower((unsigned char)query[j])) {
                ok = 0;
                break;
            }
        }
        if (ok) return 1;
    }
    return 0;
}

static void append_workspace_symbol(lsp_sb *sb, int *first, lsp_doc *doc, const zt_ast_node *node, const char *query) {
    char name[256];
    const char *container;
    if (doc == NULL || node == NULL || doc->uri == NULL) return;
    symbol_display_name(node, name, sizeof(name));
    if (name[0] == '\0' || !ascii_contains_ci(name, query)) return;
    if (!*first) sb_append(sb, ",");
    *first = 0;
    container = doc_module_name(doc);
    sb_append(sb, "{\"name\":");
    sb_append_json_string(sb, name);
    sb_append(sb, ",\"kind\":");
    sb_appendf(sb, "%d", symbol_kind_for_node(node));
    if (container != NULL) {
        sb_append(sb, ",\"containerName\":");
        sb_append_json_string(sb, container);
    }
    sb_append(sb, ",\"location\":{\"uri\":");
    sb_append_json_string(sb, doc->uri);
    sb_append(sb, ",\"range\":");
    append_lsp_range(sb, &node->span);
    sb_append(sb, "}}");
}

static char *workspace_symbol_result_json(const char *query) {
    lsp_sb sb;
    int first = 1;
    lsp_doc *doc;
    sb_init(&sb);
    sb_append(&sb, "[");
    for (doc = g_docs; doc != NULL; doc = doc->next) {
        size_t i;
        if (doc == NULL || !doc->has_parse || doc->parse.root == NULL || doc->parse.root->kind != ZT_AST_FILE) continue;
        for (i = 0; i < doc->parse.root->as.file.declarations.count; i += 1) {
            append_workspace_symbol(&sb, &first, doc, doc->parse.root->as.file.declarations.items[i], query);
        }
    }
    sb_append(&sb, "]");
    return sb_take(&sb);
}

static void append_reference_location(lsp_sb *sb, int *first, const char *uri, int start_line, int start_col, int end_line, int end_col) {
    if (sb == NULL || first == NULL || uri == NULL) return;
    if (!*first) sb_append(sb, ",");
    *first = 0;
    sb_append(sb, "{\"uri\":");
    sb_append_json_string(sb, uri);
    sb_append(sb, ",\"range\":{\"start\":{");
    sb_appendf(sb, "\"line\":%d,\"character\":%d", start_line, start_col);
    sb_append(sb, "},\"end\":{");
    sb_appendf(sb, "\"line\":%d,\"character\":%d", end_line, end_col);
    sb_append(sb, "}}}");
}

static void append_reference_declaration(lsp_sb *sb, int *first, lsp_doc *owner, const zt_ast_node *node) {
    int start_line;
    int start_col;
    int end_col;
    if (owner == NULL || owner->uri == NULL || node == NULL) return;
    start_line = lsp_line(node->span.line);
    start_col = lsp_col(node->span.column_start);
    end_col = lsp_col(node->span.column_end);
    if (end_col <= start_col) end_col = start_col + 1;
    append_reference_location(sb, first, owner->uri, start_line, start_col, start_line, end_col);
}

static int reference_match_has_boundaries(const char *text, int offset, int length) {
    int before;
    int after;
    if (text == NULL || offset < 0 || length <= 0) return 0;
    before = offset > 0 ? (unsigned char)text[offset - 1] : 0;
    after = (unsigned char)text[offset + length];
    if (before != 0 && (ident_char(before) || before == '.')) return 0;
    if (after != 0 && ident_char(after)) return 0;
    return 1;
}

static void append_reference_text_matches(lsp_sb *sb, int *first, lsp_doc *doc, const char *needle, int skip_line) {
    const char *cursor;
    size_t needle_len;
    if (doc == NULL || doc->uri == NULL || doc->text == NULL || needle == NULL || needle[0] == '\0') return;
    needle_len = strlen(needle);
    cursor = doc->text;
    while ((cursor = strstr(cursor, needle)) != NULL) {
        int offset = (int)(cursor - doc->text);
        int line = 0;
        int character = 0;
        position_from_offset(doc->text, offset, &line, &character);
        if (line != skip_line && reference_match_has_boundaries(doc->text, offset, (int)needle_len)) {
            append_reference_location(
                sb,
                first,
                doc->uri,
                line,
                character,
                line,
                character + (int)needle_len
            );
        }
        cursor += needle_len;
    }
}

static char *qualified_reference_name(const char *alias, const char *name) {
    lsp_sb sb;
    if (alias == NULL || alias[0] == '\0' || name == NULL || name[0] == '\0') return NULL;
    sb_init(&sb);
    sb_append(&sb, alias);
    sb_append(&sb, ".");
    sb_append(&sb, name);
    return sb_take(&sb);
}

static char *references_result_json(lsp_doc *doc, const char *word, int include_declaration) {
    lsp_doc *owner = NULL;
    const zt_ast_node *node = find_symbol_with_doc(doc, word, &owner);
    const char *name = node_symbol_name(node);
    const char *owner_ns = doc_module_name(owner);
    int public_visible = node_is_public(node) || namespace_is_std(owner_ns);
    int declaration_line = node != NULL ? lsp_line(node->span.line) : -1;
    lsp_doc *candidate;
    lsp_sb sb;
    int first = 1;

    sb_init(&sb);
    sb_append(&sb, "[");
    if (node != NULL && owner != NULL && name != NULL && name[0] != '\0') {
        if (include_declaration) {
            append_reference_declaration(&sb, &first, owner, node);
        }
        for (candidate = g_docs; candidate != NULL; candidate = candidate->next) {
            int same_namespace = docs_same_namespace(candidate, owner);
            if (candidate == owner || same_namespace) {
                append_reference_text_matches(
                    &sb,
                    &first,
                    candidate,
                    name,
                    candidate == owner ? declaration_line : -1
                );
            }
            if (candidate != owner && owner_ns != NULL && public_visible) {
                const char *alias = import_alias_for_path(candidate, owner_ns);
                char *qualified = qualified_reference_name(alias, name);
                if (qualified != NULL) {
                    append_reference_text_matches(&sb, &first, candidate, qualified, -1);
                    free(qualified);
                }
            }
        }
    }
    sb_append(&sb, "]");
    return sb_take(&sb);
}

static char *call_target_at_position(const char *text, int line, int character, int *active_parameter) {
    int offset = offset_from_position(text, line, character);
    int depth = 0;
    int open_offset = -1;
    int i;
    int start;
    int end;
    int param = 0;
    if (active_parameter != NULL) *active_parameter = 0;
    if (text == NULL || offset <= 0) return NULL;

    for (i = offset - 1; i >= 0; i -= 1) {
        if (text[i] == ')') {
            depth += 1;
        } else if (text[i] == '(') {
            if (depth == 0) {
                open_offset = i;
                break;
            }
            depth -= 1;
        }
    }
    if (open_offset <= 0) return NULL;

    depth = 0;
    for (i = open_offset + 1; i < offset; i += 1) {
        if (text[i] == '(' || text[i] == '[' || text[i] == '{') {
            depth += 1;
        } else if ((text[i] == ')' || text[i] == ']' || text[i] == '}') && depth > 0) {
            depth -= 1;
        } else if (text[i] == ',' && depth == 0) {
            param += 1;
        }
    }
    if (active_parameter != NULL) *active_parameter = param;

    end = open_offset;
    while (end > 0 && isspace((unsigned char)text[end - 1]) && text[end - 1] != '\n') end -= 1;
    start = end;
    while (start > 0 && word_char(text[start - 1])) start -= 1;
    if (end <= start) return NULL;
    return lsp_strndup(text + start, (size_t)(end - start));
}

static int lsp_is_numeric_conversion_name(const char *name) {
    static const char *numeric_conversions[] = {
        "int", "int8", "int16", "int32", "int64",
        "u8", "u16", "u32", "u64",
        "uint8", "uint16", "uint32", "uint64",
        "float", "float32", "float64",
        NULL
    };
    size_t i;
    if (name == NULL) return 0;
    for (i = 0; numeric_conversions[i] != NULL; i += 1) {
        if (strcmp(name, numeric_conversions[i]) == 0) return 1;
    }
    return 0;
}

static char *builtin_signature_help_result_json(const char *target, int active_parameter) {
    const char *label = NULL;
    const char *param = "value";
    const char *summary = NULL;
    const char *usage = NULL;
    lsp_sb json;
    int active = active_parameter > 0 ? 0 : active_parameter;
    if (target == NULL) return NULL;
    if (lsp_is_numeric_conversion_name(target)) {
        lsp_sb conversion_label;
        lsp_sb conversion_usage;
        char *documentation;
        sb_init(&conversion_label);
        sb_init(&conversion_usage);
        sb_append(&conversion_label, target);
        sb_append(&conversion_label, "(value) -> ");
        sb_append(&conversion_label, target);
        sb_append(&conversion_usage, target);
        sb_append(&conversion_usage, "(value)");
        label = conversion_label.data;
        documentation = builtin_documentation_markdown(
            label,
            lsp_doc_text(
                "Conversao numerica explicita para este tipo. Use quando a mudanca de representacao for intencional e visivel no codigo.",
                "Explicit numeric conversion to this type. Use it when the representation change is intentional and visible in code."),
            conversion_usage.data);
        sb_init(&json);
        sb_append(&json, "{\"signatures\":[{\"label\":");
        sb_append_json_string(&json, label);
        sb_append(&json, ",\"documentation\":");
        sb_append_json_string(&json, documentation);
        sb_append(&json, ",\"parameters\":[{\"label\":\"value\"}]}],\"activeSignature\":0,\"activeParameter\":");
        sb_appendf(&json, "%d", active);
        sb_append(&json, "}");
        free(documentation);
        free(conversion_usage.data);
        free(conversion_label.data);
        return sb_take(&json);
    }
    if (strcmp(target, "len") == 0) {
        label = "len(value) -> int";
        summary = lsp_doc_text(
            "Retorna a quantidade de itens de um valor composto: caracteres de text, bytes de bytes, elementos de list<T> ou pares de map<K, V>.",
            "Returns the item count of a compound value: characters in text, bytes in bytes, elements in list<T>, or pairs in map<K, V>.");
        usage = "len(value)";
    } else if (strcmp(target, "to_text") == 0) {
        label = "to_text(value) -> text";
        summary = lsp_doc_text(
            "Converte para text quando o tipo implementa TextRepresentable<T>. Use para exibicao, logs e mensagens.",
            "Converts to text when the type implements TextRepresentable<T>. Use it for display, logs, and messages.");
        usage = "to_text(value)";
    } else if (strcmp(target, "check") == 0) {
        label = "check(condition, message?) -> void";
        param = "condition";
        summary = lsp_doc_text(
            "Assercao simples de runtime. Se condition for false, a execucao falha. Use message para explicar a falha.",
            "Simple runtime assertion. If condition is false, execution fails. Use message to explain the failure.");
        usage = "check(condition, message?)";
    } else if (strcmp(target, "todo") == 0) {
        label = "todo(message) -> void";
        param = "message";
        summary = lsp_doc_text(
            "Marca um caminho conhecido como incompleto e falha em runtime com mensagem clara.",
            "Marks a known incomplete path and fails at runtime with a clear message.");
        usage = "todo(message)";
    } else if (strcmp(target, "unreachable") == 0) {
        label = "unreachable(message) -> void";
        param = "message";
        summary = lsp_doc_text(
            "Marca um caminho que deveria ser impossivel e falha se ele for executado.",
            "Marks a path that should be impossible and fails if it is executed.");
        usage = "unreachable(message)";
    } else if (strcmp(target, "panic") == 0) {
        label = "panic(message) -> void";
        param = "message";
        summary = lsp_doc_text(
            "Interrompe a execucao imediatamente com uma mensagem. Use para estados impossiveis ou falhas irrecuperaveis.",
            "Stops execution immediately with a message. Use it for impossible states or unrecoverable failures.");
        usage = "panic(message)";
    } else if (strcmp(target, "success") == 0) {
        label = "success(value) -> result<T, E>";
        summary = lsp_doc_text(
            "Cria o ramo de sucesso de um result<T, E>. Use para retornar um valor valido de uma operacao que pode falhar.",
            "Creates the success branch of result<T, E>. Use it to return a valid value from an operation that can fail.");
        usage = "success(value)";
    } else if (strcmp(target, "error") == 0) {
        label = "error(value) -> result<T, E>";
        summary = lsp_doc_text(
            "Cria o ramo de erro de um result<T, E>. Use para retornar a causa de falha sem lancar excecao.",
            "Creates the error branch of result<T, E>. Use it to return the failure cause without throwing an exception.");
        usage = "error(value)";
    }
    if (label == NULL) return NULL;
    {
        char *documentation = builtin_documentation_markdown(label, summary, usage);
    sb_init(&json);
    sb_append(&json, "{\"signatures\":[{\"label\":");
    sb_append_json_string(&json, label);
    sb_append(&json, ",\"documentation\":");
    sb_append_json_string(&json, documentation);
    sb_append(&json, ",\"parameters\":[{\"label\":");
    sb_append_json_string(&json, param);
    sb_append(&json, "}]}],\"activeSignature\":0,\"activeParameter\":");
    sb_appendf(&json, "%d", active);
    sb_append(&json, "}");
        free(documentation);
    }
    return sb_take(&json);
}

static char *signature_help_result_json(lsp_doc *doc, const char *target, int line, int character, int active_parameter) {
    lsp_doc *owner = NULL;
    const zt_ast_node *node = NULL;
    lsp_sb label;
    lsp_sb params;
    lsp_sb json;
    size_t i;
    int params_first = 1;
    int param_count;
    int active;
    char *usage_qualifier = NULL;
    char *documentation = NULL;
    char *builtin = builtin_signature_help_result_json(target, active_parameter);
    if (builtin != NULL) return builtin;
    if (target != NULL) {
        const char *dot = strrchr(target, '.');
        if (dot != NULL && dot > target && dot[1] != '\0') {
            char *receiver = lsp_strndup(target, (size_t)(dot - target));
            char *receiver_type = receiver != NULL ? completion_receiver_type(doc, receiver, line, character) : NULL;
            if (receiver_type != NULL) {
                node = find_apply_method(doc, receiver_type, dot + 1, &owner);
                if (node != NULL) usage_qualifier = lsp_strdup(receiver);
            }
            free(receiver_type);
            free(receiver);
        }
    }
    if (node == NULL) {
        node = find_symbol_with_doc(doc, target, &owner);
        if (target != NULL) {
            const char *dot = strrchr(target, '.');
            if (dot != NULL && dot > target) usage_qualifier = lsp_strndup(target, (size_t)(dot - target));
        }
    }
    if (node == NULL || node->kind != ZT_AST_FUNC_DECL) {
        free(usage_qualifier);
        return NULL;
    }
    documentation = function_documentation_markdown(owner, usage_qualifier, node);

    sb_init(&label);
    sb_init(&params);
    sb_append(&label, "func ");
    sb_append(&label, node->as.func_decl.name != NULL ? node->as.func_decl.name : "function");
    sb_append(&label, "(");
    sb_append(&params, "[");
    for (i = 0; i < node->as.func_decl.params.count; i += 1) {
        char *formatted = zt_format_node_to_string(node->as.func_decl.params.items[i]);
        const char *param_label = formatted != NULL ? formatted : "param";
        if (i > 0) sb_append(&label, ", ");
        sb_append(&label, param_label);
        if (!params_first) sb_append(&params, ",");
        params_first = 0;
        sb_append(&params, "{\"label\":");
        sb_append_json_string(&params, param_label);
        sb_append(&params, "}");
        free(formatted);
    }
    sb_append(&params, "]");
    sb_append(&label, ")");
    if (node->as.func_decl.return_type != NULL) {
        char *ret = zt_format_node_to_string(node->as.func_decl.return_type);
        if (ret != NULL) {
            sb_append(&label, " -> ");
            sb_append(&label, ret);
            free(ret);
        }
    }

    param_count = (int)node->as.func_decl.params.count;
    active = active_parameter;
    if (active < 0) active = 0;
    if (param_count > 0 && active >= param_count) active = param_count - 1;
    if (param_count == 0) active = 0;

    sb_init(&json);
    sb_append(&json, "{\"signatures\":[{\"label\":");
    sb_append_json_string(&json, label.data);
    if (documentation != NULL) {
        sb_append(&json, ",\"documentation\":");
        sb_append_json_string(&json, documentation);
    }
    sb_append(&json, ",\"parameters\":");
    sb_append(&json, params.data);
    sb_append(&json, "}],\"activeSignature\":0,\"activeParameter\":");
    sb_appendf(&json, "%d", active);
    sb_append(&json, "}");
    free(label.data);
    free(params.data);
    free(documentation);
    free(usage_qualifier);
    return sb_take(&json);
}

static int rename_name_is_valid(const char *name) {
    const char *p;
    if (name == NULL || name[0] == '\0') return 0;
    if (!(isalpha((unsigned char)name[0]) || name[0] == '_')) return 0;
    for (p = name + 1; *p != '\0'; p += 1) {
        if (!ident_char((unsigned char)*p)) return 0;
    }
    return 1;
}

static int line_bounds_for_offset(const char *text, int offset, int *line_start, int *line_end) {
    int start;
    int end;
    if (text == NULL || offset < 0) return 0;
    start = offset;
    while (start > 0 && text[start - 1] != '\n') start -= 1;
    end = offset;
    while (text[end] != '\0' && text[end] != '\n') end += 1;
    if (line_start != NULL) *line_start = start;
    if (line_end != NULL) *line_end = end;
    return 1;
}

static int offset_is_in_string_or_comment(const char *text, int offset) {
    int line_start = 0;
    int line_end = 0;
    int in_string = 0;
    int escaped = 0;
    int i;
    if (!line_bounds_for_offset(text, offset, &line_start, &line_end)) return 0;
    for (i = line_start; i < offset && i < line_end; i += 1) {
        unsigned char ch = (unsigned char)text[i];
        unsigned char next = (unsigned char)text[i + 1];
        if (!in_string && ch == '/' && next == '/') return 1;
        if (ch == '"' && !escaped) in_string = !in_string;
        escaped = in_string && ch == '\\' && !escaped;
        if (ch != '\\') escaped = 0;
    }
    return in_string;
}

static int line_start_offset(const char *text, int target_line) {
    int line = 0;
    int offset = 0;
    if (text == NULL || target_line <= 0) return 0;
    while (text[offset] != '\0' && line < target_line) {
        if (text[offset] == '\n') line += 1;
        offset += 1;
    }
    return offset;
}

static int declaration_name_range(lsp_doc *doc, const zt_ast_node *node, const char *name, int *line, int *start_col, int *end_col) {
    int target_line;
    int start_offset;
    int end_offset;
    const char *cursor;
    size_t name_len;
    if (doc == NULL || doc->text == NULL || node == NULL || name == NULL || name[0] == '\0') return 0;
    target_line = lsp_line(node->span.line);
    start_offset = line_start_offset(doc->text, target_line);
    end_offset = start_offset;
    while (doc->text[end_offset] != '\0' && doc->text[end_offset] != '\n') end_offset += 1;
    name_len = strlen(name);
    cursor = doc->text + start_offset;
    while (cursor < doc->text + end_offset) {
        const char *match = strstr(cursor, name);
        int offset;
        int found_line = 0;
        int found_col = 0;
        if (match == NULL || match >= doc->text + end_offset) break;
        offset = (int)(match - doc->text);
        if (reference_match_has_boundaries(doc->text, offset, (int)name_len)) {
            position_from_offset(doc->text, offset, &found_line, &found_col);
            if (line != NULL) *line = found_line;
            if (start_col != NULL) *start_col = found_col;
            if (end_col != NULL) *end_col = found_col + (int)name_len;
            return 1;
        }
        cursor = match + name_len;
    }
    return 0;
}

static void append_rename_text_edit(lsp_sb *sb, int *first, int *count, int start_line, int start_col, int end_line, int end_col, const char *new_name) {
    if (sb == NULL || first == NULL || count == NULL || new_name == NULL) return;
    if (!*first) sb_append(sb, ",");
    *first = 0;
    *count += 1;
    sb_append(sb, "{\"range\":{\"start\":{");
    sb_appendf(sb, "\"line\":%d,\"character\":%d", start_line, start_col);
    sb_append(sb, "},\"end\":{");
    sb_appendf(sb, "\"line\":%d,\"character\":%d", end_line, end_col);
    sb_append(sb, "}},\"newText\":");
    sb_append_json_string(sb, new_name);
    sb_append(sb, "}");
}

static void append_rename_text_matches(lsp_sb *sb, int *first, int *count, lsp_doc *doc, const char *needle, const char *new_name, int skip_line, int replace_offset, int replace_len) {
    const char *cursor;
    size_t needle_len;
    if (doc == NULL || doc->text == NULL || needle == NULL || needle[0] == '\0') return;
    needle_len = strlen(needle);
    cursor = doc->text;
    while ((cursor = strstr(cursor, needle)) != NULL) {
        int offset = (int)(cursor - doc->text);
        int line = 0;
        int character = 0;
        if (reference_match_has_boundaries(doc->text, offset, (int)needle_len) && !offset_is_in_string_or_comment(doc->text, offset)) {
            position_from_offset(doc->text, offset + replace_offset, &line, &character);
            if (line != skip_line) {
                append_rename_text_edit(
                    sb,
                    first,
                    count,
                    line,
                    character,
                    line,
                    character + replace_len,
                    new_name
                );
            }
        }
        cursor += needle_len;
    }
}

static char *rename_edits_for_doc(lsp_doc *candidate, lsp_doc *owner, const zt_ast_node *node, const char *name, const char *new_name) {
    const char *owner_ns = doc_module_name(owner);
    int public_visible = node_is_public(node) || namespace_is_std(owner_ns);
    int declaration_line = node != NULL ? lsp_line(node->span.line) : -1;
    lsp_sb edits;
    int first = 1;
    int count = 0;
    sb_init(&edits);
    if (candidate == owner) {
        int line = 0;
        int start_col = 0;
        int end_col = 0;
        if (declaration_name_range(owner, node, name, &line, &start_col, &end_col)) {
            append_rename_text_edit(&edits, &first, &count, line, start_col, line, end_col, new_name);
        }
    }
    if (candidate == owner || docs_same_namespace(candidate, owner)) {
        append_rename_text_matches(
            &edits,
            &first,
            &count,
            candidate,
            name,
            new_name,
            candidate == owner ? declaration_line : -1,
            0,
            (int)strlen(name)
        );
    }
    if (candidate != owner && owner_ns != NULL && public_visible) {
        const char *alias = import_alias_for_path(candidate, owner_ns);
        char *qualified = qualified_reference_name(alias, name);
        if (qualified != NULL) {
            append_rename_text_matches(
                &edits,
                &first,
                &count,
                candidate,
                qualified,
                new_name,
                -1,
                (int)strlen(alias) + 1,
                (int)strlen(name)
            );
            free(qualified);
        }
    }
    if (count == 0) {
        free(edits.data);
        return NULL;
    }
    return sb_take(&edits);
}

static char *rename_result_json(lsp_doc *doc, const char *word, const char *new_name) {
    lsp_doc *owner = NULL;
    const zt_ast_node *node = find_symbol_with_doc(doc, word, &owner);
    const char *name = node_symbol_name(node);
    lsp_doc *candidate;
    lsp_sb json;
    int first_doc = 1;
    if (!rename_name_is_valid(new_name)) return NULL;
    if (node == NULL || owner == NULL || owner->uri == NULL || name == NULL || name[0] == '\0') return NULL;
    sb_init(&json);
    sb_append(&json, "{\"changes\":{");
    for (candidate = g_docs; candidate != NULL; candidate = candidate->next) {
        char *edits = rename_edits_for_doc(candidate, owner, node, name, new_name);
        if (edits == NULL) continue;
        if (!first_doc) sb_append(&json, ",");
        first_doc = 0;
        sb_append_json_string(&json, candidate->uri);
        sb_append(&json, ":[");
        sb_append(&json, edits);
        sb_append(&json, "]");
        free(edits);
    }
    sb_append(&json, "}}");
    return sb_take(&json);
}

static char *prepare_rename_result_json(lsp_doc *doc, const char *word, const char *segment, int start_line, int start_col, int end_col) {
    lsp_doc *owner = NULL;
    const zt_ast_node *node = find_symbol_with_doc(doc, word, &owner);
    const char *name = node_symbol_name(node);
    lsp_sb json;
    if (node == NULL || owner == NULL || name == NULL || segment == NULL) return NULL;
    if (strcmp(name, segment) != 0) return NULL;
    sb_init(&json);
    sb_append(&json, "{\"range\":{\"start\":{");
    sb_appendf(&json, "\"line\":%d,\"character\":%d", start_line, start_col);
    sb_append(&json, "},\"end\":{");
    sb_appendf(&json, "\"line\":%d,\"character\":%d", start_line, end_col);
    sb_append(&json, "}},\"placeholder\":");
    sb_append_json_string(&json, name);
    sb_append(&json, "}");
    return sb_take(&json);
}

enum {
    SEM_TOKEN_NAMESPACE = 0,
    SEM_TOKEN_TYPE = 1,
    SEM_TOKEN_FUNCTION = 2,
    SEM_TOKEN_VARIABLE = 3,
    SEM_TOKEN_PROPERTY = 4,
    SEM_TOKEN_KEYWORD = 5,
    SEM_TOKEN_MODIFIER = 6,
    SEM_TOKEN_STRING = 7,
    SEM_TOKEN_NUMBER = 8
};

enum {
    SEM_MOD_DECLARATION = 1 << 0,
    SEM_MOD_READONLY = 1 << 1,
    SEM_MOD_PUBLIC = 1 << 2
};

static int semantic_builtin_type(const char *text) {
    static const char *types[] = {
        "int", "int8", "int16", "int32", "int64",
        "u8", "u16", "u32", "u64",
        "uint8", "uint16", "uint32", "uint64",
        "float", "float32", "float64", "bool", "text", "bytes", "void",
        "list", "map", "set", "optional", "result",
        "grid2d", "pqueue", "circbuf", "btreemap", "btreeset", "grid3d",
        "Error", NULL
    };
    size_t i;
    if (text == NULL) return 0;
    for (i = 0; types[i] != NULL; i += 1) {
        if (strcmp(text, types[i]) == 0) return 1;
    }
    return 0;
}

static int semantic_token_kind_is_type(zt_token_kind kind) {
    switch (kind) {
        case ZT_TOKEN_OPTIONAL:
        case ZT_TOKEN_RESULT:
        case ZT_TOKEN_LIST:
        case ZT_TOKEN_MAP:
        case ZT_TOKEN_SET:
        case ZT_TOKEN_GRID2D:
        case ZT_TOKEN_PQUEUE:
        case ZT_TOKEN_CIRCBUF:
        case ZT_TOKEN_BTREEMAP:
        case ZT_TOKEN_BTREESET:
        case ZT_TOKEN_GRID3D:
        case ZT_TOKEN_VOID:
            return 1;
        default:
            return 0;
    }
}

static int semantic_token_kind_is_modifier(zt_token_kind kind) {
    switch (kind) {
        case ZT_TOKEN_PUBLIC:
        case ZT_TOKEN_MUT:
        case ZT_TOKEN_CAPTURE:
        case ZT_TOKEN_DYN:
        case ZT_TOKEN_ANY:
            return 1;
        default:
            return 0;
    }
}

static void append_semantic_token(lsp_sb *sb, int *first, int *last_line, int *last_start, int line, int start, int length, int token_type, int modifiers) {
    int delta_line;
    int delta_start;
    if (sb == NULL || first == NULL || last_line == NULL || last_start == NULL || length <= 0) return;
    delta_line = line - *last_line;
    delta_start = delta_line == 0 ? start - *last_start : start;
    if (!*first) sb_append(sb, ",");
    *first = 0;
    sb_appendf(sb, "%d,%d,%d,%d,%d", delta_line, delta_start, length, token_type, modifiers);
    *last_line = line;
    *last_start = start;
}

static char *semantic_tokens_result_json(lsp_doc *doc) {
    lsp_sb data;
    lsp_sb json;
    int first = 1;
    int last_line = 0;
    int last_start = 0;
    int expect_decl = 0;
    int public_next = 0;
    int namespace_path = 0;
    int previous_line = -1;
    zt_lexer *lexer;
    if (doc == NULL || doc->text == NULL) return lsp_strdup("{\"data\":[]}");
    lexer = zt_lexer_make(doc->path != NULL ? doc->path : doc->uri, doc->text, strlen(doc->text));
    if (lexer == NULL) return lsp_strdup("{\"data\":[]}");
    sb_init(&data);
    for (;;) {
        zt_token token = zt_lexer_next_token(lexer);
        int line;
        int col;
        int length;
        int token_type = -1;
        int modifiers = 0;
        if (token.kind == ZT_TOKEN_EOF) break;
        if (token.kind == ZT_TOKEN_COMMENT || token.kind == ZT_TOKEN_LEX_ERROR) continue;
        line = lsp_line(token.span.line);
        col = lsp_col(token.span.column_start);
        length = (int)token.length;
        if (length <= 0) continue;
        if (previous_line >= 0 && line != previous_line) {
            expect_decl = 0;
            public_next = 0;
            namespace_path = 0;
        }
        previous_line = line;

        if (token.kind == ZT_TOKEN_STRING_LITERAL ||
            token.kind == ZT_TOKEN_STRING_PART ||
            token.kind == ZT_TOKEN_STRING_END ||
            token.kind == ZT_TOKEN_TRIPLE_QUOTED_TEXT) {
            token_type = SEM_TOKEN_STRING;
        } else if (token.kind == ZT_TOKEN_INT_LITERAL || token.kind == ZT_TOKEN_FLOAT_LITERAL) {
            token_type = SEM_TOKEN_NUMBER;
        } else if (semantic_token_kind_is_type(token.kind)) {
            token_type = SEM_TOKEN_TYPE;
        } else if (zt_token_kind_is_keyword(token.kind)) {
            token_type = semantic_token_kind_is_modifier(token.kind) ? SEM_TOKEN_MODIFIER : SEM_TOKEN_KEYWORD;
            if (token.kind == ZT_TOKEN_NAMESPACE || token.kind == ZT_TOKEN_IMPORT) namespace_path = 1;
            else if (token.kind == ZT_TOKEN_AS) namespace_path = 0;
            else if (token.kind == ZT_TOKEN_PUBLIC) public_next = 1;
            else if (token.kind == ZT_TOKEN_FUNC) expect_decl = SEM_TOKEN_FUNCTION;
            else if (token.kind == ZT_TOKEN_STRUCT ||
                     token.kind == ZT_TOKEN_TRAIT ||
                     token.kind == ZT_TOKEN_ENUM ||
                     token.kind == ZT_TOKEN_TYPE) {
                expect_decl = SEM_TOKEN_TYPE;
            } else if (token.kind == ZT_TOKEN_VAR || token.kind == ZT_TOKEN_CAPTURE) {
                expect_decl = SEM_TOKEN_VARIABLE;
            } else if (token.kind == ZT_TOKEN_CONST) {
                expect_decl = SEM_TOKEN_VARIABLE | (SEM_MOD_READONLY << 8);
            } else if (token.kind != ZT_TOKEN_DOT) {
                if (token.kind != ZT_TOKEN_PUBLIC && token.kind != ZT_TOKEN_MUT) public_next = 0;
            }
        } else if (token.kind == ZT_TOKEN_IDENTIFIER) {
            if (expect_decl != 0) {
                token_type = expect_decl & 0xff;
                modifiers = SEM_MOD_DECLARATION | ((expect_decl >> 8) & 0xff);
                if (public_next) modifiers |= SEM_MOD_PUBLIC;
                expect_decl = 0;
                public_next = 0;
            } else if (namespace_path) {
                token_type = SEM_TOKEN_NAMESPACE;
            } else {
                token_type = SEM_TOKEN_VARIABLE;
            }
        } else if (token.kind == ZT_TOKEN_DOT) {
            continue;
        } else {
            namespace_path = 0;
        }

        if (token.kind == ZT_TOKEN_IDENTIFIER && token_type == SEM_TOKEN_VARIABLE) {
            char ident[128];
            size_t ident_len = token.length < sizeof(ident) - 1 ? token.length : sizeof(ident) - 1;
            memcpy(ident, token.text, ident_len);
            ident[ident_len] = '\0';
            if (semantic_builtin_type(ident)) {
                token_type = SEM_TOKEN_TYPE;
            }
        }

        if (token_type >= 0) {
            append_semantic_token(&data, &first, &last_line, &last_start, line, col, length, token_type, modifiers);
        }
    }
    zt_lexer_dispose(lexer);
    sb_init(&json);
    sb_append(&json, "{\"data\":[");
    sb_append(&json, data.data);
    sb_append(&json, "]}");
    free(data.data);
    return sb_take(&json);
}

static void append_import_path_completions(lsp_sb *sb, int *first, lsp_doc *doc) {
    size_t i;
    lsp_doc *other;
    const char *current_ns = doc_module_name(doc);

    for (i = 0; g_stdlib_modules[i].name != NULL; i += 1) {
        append_completion_item(sb, first, g_stdlib_modules[i].name, 9, "stdlib", NULL, 0);
    }

    for (other = g_docs; other != NULL; other = other->next) {
        const char *module_name = doc_module_name(other);
        if (module_name == NULL) continue;
        if (current_ns != NULL && strcmp(current_ns, module_name) == 0) continue;
        if (namespace_is_std(module_name)) continue;
        append_completion_item(sb, first, module_name, 9, "modulo do projeto", NULL, 0);
    }
}

static void append_import_alias_completions(lsp_sb *sb, int *first, lsp_doc *doc) {
    size_t i;
    if (doc == NULL || !doc->has_parse || doc->parse.root == NULL || doc->parse.root->kind != ZT_AST_FILE) return;
    for (i = 0; i < doc->parse.root->as.file.imports.count; i += 1) {
        const zt_ast_node *import_decl = doc->parse.root->as.file.imports.items[i];
        const char *path;
        const char *alias;
        if (import_decl == NULL || import_decl->kind != ZT_AST_IMPORT_DECL) continue;
        path = import_decl->as.import_decl.path;
        alias = import_decl->as.import_decl.alias != NULL
            ? import_decl->as.import_decl.alias
            : last_namespace_segment(path);
        if (alias == NULL) continue;
        append_completion_item(sb, first, alias, 9, path, NULL, 0);
    }
}

static void append_function_signature_text(lsp_sb *sb, const char *qualifier, const zt_ast_node *func_decl);
static void append_function_call_snippet(lsp_sb *sb, const char *qualifier, const zt_ast_node *func_decl);
static char *function_documentation_markdown(lsp_doc *owner, const char *usage_qualifier, const zt_ast_node *func_decl);

static void append_doc_symbol_completions(lsp_sb *sb, int *first, lsp_doc *doc) {
    size_t i;
    if (doc == NULL || !doc->has_parse || doc->parse.root == NULL || doc->parse.root->kind != ZT_AST_FILE) return;
    for (i = 0; i < doc->parse.root->as.file.declarations.count; i += 1) {
        const zt_ast_node *decl = doc->parse.root->as.file.declarations.items[i];
        const char *name = node_symbol_name(decl);
        if (name != NULL) {
            lsp_sb detail;
            lsp_sb insert_text;
            char *documentation = NULL;
            sb_init(&detail);
            sb_init(&insert_text);
            if (decl->kind == ZT_AST_FUNC_DECL) {
                append_function_signature_text(&detail, NULL, decl);
                append_function_call_snippet(&insert_text, NULL, decl);
                documentation = function_documentation_markdown(doc, NULL, decl);
            } else {
                sb_append(&detail, node_symbol_kind(decl));
            }
            append_completion_item_full(
                sb,
                first,
                name,
                completion_kind_for_node(decl),
                detail.data,
                documentation,
                decl->kind == ZT_AST_FUNC_DECL ? insert_text.data : NULL,
                decl->kind == ZT_AST_FUNC_DECL ? 2 : 0);
            free(documentation);
            free(insert_text.data);
            free(detail.data);
        }
    }
}

static void append_function_signature_text(lsp_sb *sb, const char *qualifier, const zt_ast_node *func_decl) {
    size_t i;
    if (sb == NULL || func_decl == NULL || func_decl->kind != ZT_AST_FUNC_DECL) return;
    if (qualifier != NULL && qualifier[0] != '\0') {
        sb_append(sb, qualifier);
        sb_append(sb, ".");
    }
    sb_append(sb, func_decl->as.func_decl.name != NULL ? func_decl->as.func_decl.name : "function");
    sb_append(sb, "(");
    for (i = 0; i < func_decl->as.func_decl.params.count; i += 1) {
        char *formatted = zt_format_node_to_string(func_decl->as.func_decl.params.items[i]);
        if (i > 0) sb_append(sb, ", ");
        sb_append(sb, formatted != NULL ? formatted : "param");
        free(formatted);
    }
    sb_append(sb, ")");
    if (func_decl->as.func_decl.return_type != NULL) {
        sb_append(sb, " -> ");
        append_formatted_node(sb, func_decl->as.func_decl.return_type);
    }
}

static int line_bounds_for_line(const char *text, int target_line, int *line_start, int *line_end) {
    int line = 0;
    int offset = 0;
    if (text == NULL || target_line < 0) return 0;
    while (text[offset] != '\0' && line < target_line) {
        if (text[offset] == '\n') line += 1;
        offset += 1;
    }
    if (line != target_line) return 0;
    if (line_start != NULL) *line_start = offset;
    while (text[offset] != '\0' && text[offset] != '\n') offset += 1;
    if (line_end != NULL) *line_end = offset;
    return 1;
}

static void append_trimmed_comment_text(lsp_sb *sb, const char *start, const char *end) {
    const char *p = start;
    const char *tail = end;
    if (sb == NULL || start == NULL || end == NULL || end < start) return;
    while (p < end && isspace((unsigned char)*p)) p += 1;
    if (p + 2 <= end && p[0] == '-' && p[1] == '-') {
        p += 2;
    } else if (p + 3 <= end && p[0] == '/' && p[1] == '/' && p[2] == '/') {
        p += 3;
    }
    while (p < end && isspace((unsigned char)*p)) p += 1;
    while (tail > p && isspace((unsigned char)tail[-1])) tail -= 1;
    if (tail > p) sb_append_len(sb, p, (size_t)(tail - p));
}

static int line_is_doc_comment(const char *start, const char *end) {
    const char *p = start;
    if (start == NULL || end == NULL || end < start) return 0;
    while (p < end && isspace((unsigned char)*p)) p += 1;
    return (p + 2 <= end && p[0] == '-' && p[1] == '-') ||
           (p + 3 <= end && p[0] == '/' && p[1] == '/' && p[2] == '/');
}

static int line_is_blank(const char *start, const char *end) {
    const char *p = start;
    if (start == NULL || end == NULL || end < start) return 1;
    while (p < end) {
        if (!isspace((unsigned char)*p)) return 0;
        p += 1;
    }
    return 1;
}

static char *leading_doc_comment_for_node(lsp_doc *doc, const zt_ast_node *node) {
    int decl_line;
    int line;
    int first_comment_line = -1;
    int last_comment_line = -1;
    lsp_sb out;
    if (doc == NULL || doc->text == NULL || node == NULL || node->span.line == 0) return NULL;
    decl_line = lsp_line(node->span.line);
    for (line = decl_line - 1; line >= 0; line -= 1) {
        int start = 0;
        int end = 0;
        const char *line_start;
        const char *line_end;
        if (!line_bounds_for_line(doc->text, line, &start, &end)) break;
        line_start = doc->text + start;
        line_end = doc->text + end;
        if (line_is_doc_comment(line_start, line_end)) {
            first_comment_line = line;
            if (last_comment_line < 0) last_comment_line = line;
            continue;
        }
        if (line_is_blank(line_start, line_end) && first_comment_line < 0) {
            continue;
        }
        break;
    }
    if (first_comment_line < 0 || last_comment_line < 0) return NULL;
    sb_init(&out);
    for (line = first_comment_line; line <= last_comment_line; line += 1) {
        int start = 0;
        int end = 0;
        if (!line_bounds_for_line(doc->text, line, &start, &end)) continue;
        if (out.len > 0) sb_append(&out, "\n");
        append_trimmed_comment_text(&out, doc->text + start, doc->text + end);
    }
    if (out.len == 0) {
        free(out.data);
        return NULL;
    }
    return sb_take(&out);
}

static char *generated_function_summary(lsp_doc *owner, const zt_ast_node *func_decl) {
    const char *module = doc_module_name(owner);
    (void)func_decl;
    return lsp_strdup(lsp_missing_function_doc_text(module != NULL && namespace_is_std(module)));
}

static void append_function_declaration_signature(lsp_sb *sb, const zt_ast_node *func_decl) {
    if (sb == NULL || func_decl == NULL || func_decl->kind != ZT_AST_FUNC_DECL) return;
    sb_append(sb, "func ");
    append_function_signature_text(sb, NULL, func_decl);
}

static void append_function_usage_text(lsp_sb *sb, const char *qualifier, const zt_ast_node *func_decl) {
    size_t i;
    int first_arg = 1;
    if (sb == NULL || func_decl == NULL || func_decl->kind != ZT_AST_FUNC_DECL) return;
    if (qualifier != NULL && qualifier[0] != '\0') {
        sb_append(sb, qualifier);
        sb_append(sb, ".");
    }
    sb_append(sb, func_decl->as.func_decl.name != NULL ? func_decl->as.func_decl.name : "function");
    sb_append(sb, "(");
    for (i = 0; i < func_decl->as.func_decl.params.count; i += 1) {
        const zt_ast_node *param = func_decl->as.func_decl.params.items[i];
        const char *name = "value";
        if (param == NULL || param->kind != ZT_AST_PARAM) continue;
        if (param->as.param.default_value != NULL) continue;
        if (!first_arg) sb_append(sb, ", ");
        first_arg = 0;
        if (param->as.param.name != NULL && param->as.param.name[0] != '\0') name = param->as.param.name;
        sb_append(sb, name);
    }
    sb_append(sb, ")");
}

static char *function_documentation_markdown(lsp_doc *owner, const char *usage_qualifier, const zt_ast_node *func_decl) {
    lsp_sb doc;
    lsp_sb signature;
    lsp_sb usage;
    char *comment;
    char *summary;
    char *zdoc;
    const char *module;
    const char *func_name;
    if (func_decl == NULL || func_decl->kind != ZT_AST_FUNC_DECL) return NULL;
    module = doc_module_name(owner);
    func_name = func_decl->as.func_decl.name;
    comment = leading_doc_comment_for_node(owner, func_decl);
    zdoc = comment == NULL ? lsp_zdoc_documentation_for_symbol(module, func_name) : NULL;
    summary = comment != NULL ? comment : (zdoc == NULL ? generated_function_summary(owner, func_decl) : NULL);
    sb_init(&signature);
    append_function_declaration_signature(&signature, func_decl);
    sb_init(&usage);
    append_function_usage_text(&usage, usage_qualifier, func_decl);
    sb_init(&doc);
    if (zdoc != NULL && zdoc[0] != '\0') {
        sb_append(&doc, lsp_doc_heading_documentation());
        sb_append(&doc, "\n");
        sb_append(&doc, zdoc);
        sb_append(&doc, "\n\n");
    } else if (summary != NULL && summary[0] != '\0') {
        sb_append(&doc, lsp_doc_heading_what());
        sb_append(&doc, "\n");
        sb_append(&doc, summary);
        sb_append(&doc, "\n\n");
    }
    sb_append(&doc, lsp_doc_heading_signature());
    sb_append(&doc, "\n```zt\n");
    sb_append(&doc, signature.data != NULL ? signature.data : "");
    sb_append(&doc, "\n```\n\n");
    sb_append(&doc, lsp_doc_heading_usage());
    sb_append(&doc, "\n```zt\n");
    sb_append(&doc, usage.data != NULL ? usage.data : "");
    sb_append(&doc, "\n```");
    if (module != NULL) {
        sb_append(&doc, "\n\n");
        sb_append(&doc, lsp_doc_module_label());
        sb_append(&doc, ": `");
        sb_append(&doc, module);
        sb_append(&doc, "`");
    }
    free(signature.data);
    free(usage.data);
    free(summary);
    free(zdoc);
    return sb_take(&doc);
}

static void append_function_call_snippet(lsp_sb *sb, const char *qualifier, const zt_ast_node *func_decl) {
    size_t i;
    int arg_index = 1;
    int first_arg = 1;
    if (sb == NULL || func_decl == NULL || func_decl->kind != ZT_AST_FUNC_DECL) return;
    if (qualifier != NULL && qualifier[0] != '\0') {
        sb_append(sb, qualifier);
        sb_append(sb, ".");
    }
    sb_append(sb, func_decl->as.func_decl.name != NULL ? func_decl->as.func_decl.name : "function");
    sb_append(sb, "(");
    for (i = 0; i < func_decl->as.func_decl.params.count; i += 1) {
        const zt_ast_node *param = func_decl->as.func_decl.params.items[i];
        const char *name = "value";
        if (param == NULL || param->kind != ZT_AST_PARAM) continue;
        if (param->as.param.default_value != NULL) continue;
        if (!first_arg) sb_append(sb, ", ");
        first_arg = 0;
        if (param->as.param.name != NULL && param->as.param.name[0] != '\0') name = param->as.param.name;
        sb_appendf(sb, "${%d:%s}", arg_index, name);
        arg_index += 1;
    }
    sb_append(sb, ")");
}

static void append_module_member_completions(lsp_sb *sb, int *first, lsp_doc *doc, const char *alias) {
    const char *import_path = import_path_for_alias(doc, alias);
    lsp_doc *module_doc;
    size_t i;
    if (import_path == NULL) return;
    module_doc = find_doc_by_namespace(import_path);
    if (module_doc == NULL || !module_doc->has_parse || module_doc->parse.root == NULL || module_doc->parse.root->kind != ZT_AST_FILE) return;
    for (i = 0; i < module_doc->parse.root->as.file.declarations.count; i += 1) {
        const zt_ast_node *decl = module_doc->parse.root->as.file.declarations.items[i];
        const char *name = node_symbol_name(decl);
        lsp_sb detail;
        lsp_sb insert_text;
        char *documentation = NULL;
        if (name == NULL) continue;
        if (!node_is_public(decl)) continue;
        sb_init(&detail);
        if (decl->kind == ZT_AST_FUNC_DECL) {
            append_function_signature_text(&detail, import_path, decl);
            documentation = function_documentation_markdown(module_doc, alias, decl);
        } else {
            sb_append(&detail, import_path);
            sb_append(&detail, ".");
            sb_append(&detail, name);
        }
        sb_init(&insert_text);
        if (decl->kind == ZT_AST_FUNC_DECL) {
            append_function_call_snippet(&insert_text, NULL, decl);
        }
        append_completion_item_full(
            sb,
            first,
            name,
            completion_kind_for_node(decl),
            detail.data,
            documentation,
            decl->kind == ZT_AST_FUNC_DECL ? insert_text.data : NULL,
            decl->kind == ZT_AST_FUNC_DECL ? 2 : 0);
        free(documentation);
        free(insert_text.data);
        free(detail.data);
    }
}

static void append_imported_module_shortcut_completions(lsp_sb *sb, int *first, lsp_doc *doc) {
    size_t i;
    if (doc == NULL || !doc->has_parse || doc->parse.root == NULL || doc->parse.root->kind != ZT_AST_FILE) return;
    for (i = 0; i < doc->parse.root->as.file.imports.count; i += 1) {
        const zt_ast_node *import_decl = doc->parse.root->as.file.imports.items[i];
        const char *path;
        const char *alias;
        lsp_doc *module_doc;
        size_t d;
        if (import_decl == NULL || import_decl->kind != ZT_AST_IMPORT_DECL) continue;
        path = import_decl->as.import_decl.path;
        alias = import_decl->as.import_decl.alias != NULL
            ? import_decl->as.import_decl.alias
            : last_namespace_segment(path);
        if (path == NULL || alias == NULL) continue;
        module_doc = find_doc_by_namespace(path);
        if (module_doc == NULL || !module_doc->has_parse || module_doc->parse.root == NULL || module_doc->parse.root->kind != ZT_AST_FILE) continue;
        for (d = 0; d < module_doc->parse.root->as.file.declarations.count; d += 1) {
            const zt_ast_node *decl = module_doc->parse.root->as.file.declarations.items[d];
            const char *name = node_symbol_name(decl);
            lsp_sb label;
            lsp_sb detail;
            lsp_sb insert_text;
            char *documentation = NULL;
            if (name == NULL || !node_is_public(decl)) continue;
            sb_init(&label);
            sb_init(&detail);
            sb_init(&insert_text);
            if (decl->kind == ZT_AST_FUNC_DECL) {
                sb_append(&label, name);
                sb_append(&label, "()");
                append_function_signature_text(&detail, path, decl);
                append_function_call_snippet(&insert_text, alias, decl);
                documentation = function_documentation_markdown(module_doc, alias, decl);
            } else {
                sb_append(&label, name);
                sb_append(&detail, path);
                sb_append(&detail, ".");
                sb_append(&detail, name);
                sb_append(&insert_text, alias);
                sb_append(&insert_text, ".");
                sb_append(&insert_text, name);
            }
            append_completion_item_full(
                sb,
                first,
                label.data,
                completion_kind_for_node(decl),
                detail.data,
                documentation,
                insert_text.data,
                decl->kind == ZT_AST_FUNC_DECL ? 2 : 0);
            free(documentation);
            free(label.data);
            free(detail.data);
            free(insert_text.data);
        }
    }
}

static void append_member_completions(lsp_sb *sb, int *first, lsp_doc *doc, const char *receiver, const char *receiver_type) {
    if (receiver_type == NULL && import_path_for_alias(doc, receiver) != NULL) {
        append_module_member_completions(sb, first, doc, receiver);
    } else if (type_starts_with_name(receiver_type, "list")) {
        append_completion_item(
            sb,
            first,
            "get",
            2,
            "list<T>.get(index: int) -> optional<T>",
            "get(${1:index})",
            2);
    } else if (type_starts_with_name(receiver_type, "map")) {
        append_completion_item(
            sb,
            first,
            "get",
            2,
            "map<K,V>.get(key: K) -> optional<V>",
            "get(${1:key})",
            2);
    } else if (type_starts_with_name(receiver_type, "core.Error")) {
        append_completion_item(sb, first, "code", 5, "core.Error.code: text", NULL, 0);
        append_completion_item(sb, first, "message", 5, "core.Error.message: text", NULL, 0);
        append_completion_item(sb, first, "context", 5, "core.Error.context: optional<text>", NULL, 0);
    } else if (receiver_type != NULL) {
        append_struct_field_completions(sb, first, find_struct_decl(doc, receiver_type));
        append_apply_method_completions(sb, first, doc, receiver_type);
    }
}

static char *completion_result_json(
        lsp_doc *doc,
        const char *receiver,
        const char *receiver_type,
        int member_access,
        int import_path,
        int line,
        int character) {
    lsp_sb sb;
    int first = 1;
    lsp_doc *other;
    sb_init(&sb);
    sb_append(&sb, "[");

    if (import_path) {
        append_import_path_completions(&sb, &first, doc);
        sb_append(&sb, "]");
        return sb_take(&sb);
    }

    if (member_access) {
        append_member_completions(&sb, &first, doc, receiver, receiver_type);
        sb_append(&sb, "]");
        return sb_take(&sb);
    }

    append_builtin_completions(&sb, &first);
    append_import_alias_completions(&sb, &first, doc);
    append_imported_module_shortcut_completions(&sb, &first, doc);
    if (doc != NULL && doc->has_parse && doc->parse.root != NULL) {
        append_local_completions_from_node(&sb, &first, doc->parse.root, line, character);
    }
    append_doc_symbol_completions(&sb, &first, doc);
    for (other = g_docs; other != NULL; other = other->next) {
        if (other == doc) continue;
        if (!docs_same_namespace(doc, other)) continue;
        append_doc_symbol_completions(&sb, &first, other);
    }
    sb_append(&sb, "]");
    return sb_take(&sb);
}

static void handle_initialize(const char *id_raw, const char *msg) {
    char *root_uri = json_get_string_key(msg, "rootUri");
    char *locale = json_get_string_key(msg, "locale");
    if (locale != NULL) {
        zt_lang lang = zt_l10n_from_str(locale);
        if (lang != ZT_LANG_UNSPECIFIED) zt_l10n_set_lang(lang);
        free(locale);
    }
    if (root_uri != NULL) {
        free(g_workspace_root);
        g_workspace_root = uri_to_path_alloc(root_uri);
        free(root_uri);
    }
    ensure_stdlib_docs_loaded();
    char *result = lsp_strdup(
        "{\"capabilities\":{"
        "\"textDocumentSync\":1,"
        "\"hoverProvider\":true,"
        "\"definitionProvider\":true,"
        "\"referencesProvider\":true,"
        "\"renameProvider\":{\"prepareProvider\":true},"
        "\"signatureHelpProvider\":{\"triggerCharacters\":[\"(\",\",\"]},"
        "\"documentFormattingProvider\":true,"
        "\"documentSymbolProvider\":true,"
        "\"workspaceSymbolProvider\":true,"
        "\"semanticTokensProvider\":{\"legend\":{\"tokenTypes\":[\"namespace\",\"type\",\"function\",\"variable\",\"property\",\"keyword\",\"modifier\",\"string\",\"number\"],\"tokenModifiers\":[\"declaration\",\"readonly\",\"public\"]},\"full\":true,\"range\":false},"
        "\"completionProvider\":{\"resolveProvider\":false,\"triggerCharacters\":[\".\",\":\",\"<\"]},"
        "\"executeCommandProvider\":{\"commands\":[\"zenith.check\",\"zenith.build\",\"zenith.run\"]}"
        "},\"serverInfo\":{\"name\":\"Compass LSP\",\"version\":\"0.1.0\"}}"
    );
    send_response_take(id_raw, result);
}

static void handle_hover_or_definition(const char *id_raw, const char *msg, int definition) {
    char *uri = json_get_string_key(msg, "uri");
    int line = json_get_number_key(msg, "line", 0);
    int character = json_get_number_key(msg, "character", 0);
    lsp_doc *doc = find_doc(uri);
    char *word = doc != NULL ? word_at_position(doc->text, line, character) : NULL;
    char *result = NULL;
    if (definition) result = definition_result_json(doc, word);
    else result = hover_result_json(doc, word);
    send_response_take(id_raw, result != NULL ? result : lsp_strdup("null"));
    free(uri);
    free(word);
}

static void handle_references(const char *id_raw, const char *msg) {
    char *uri = json_get_string_key(msg, "uri");
    int line = json_get_number_key(msg, "line", 0);
    int character = json_get_number_key(msg, "character", 0);
    int include_declaration = json_get_bool_key(msg, "includeDeclaration", 0);
    lsp_doc *doc = find_doc(uri);
    char *word = doc != NULL ? word_at_position(doc->text, line, character) : NULL;
    char *result = references_result_json(doc, word, include_declaration);
    send_response_take(id_raw, result);
    free(uri);
    free(word);
}

static void handle_signature_help(const char *id_raw, const char *msg) {
    char *uri = json_get_string_key(msg, "uri");
    int line = json_get_number_key(msg, "line", 0);
    int character = json_get_number_key(msg, "character", 0);
    int active_parameter = 0;
    lsp_doc *doc = find_doc(uri);
    char *target = doc != NULL ? call_target_at_position(doc->text, line, character, &active_parameter) : NULL;
    char *result = signature_help_result_json(doc, target, line, character, active_parameter);
    send_response_take(id_raw, result != NULL ? result : lsp_strdup("null"));
    free(uri);
    free(target);
}

static void handle_rename(const char *id_raw, const char *msg) {
    char *uri = json_get_string_key(msg, "uri");
    char *new_name = json_get_string_key(msg, "newName");
    int line = json_get_number_key(msg, "line", 0);
    int character = json_get_number_key(msg, "character", 0);
    lsp_doc *doc = find_doc(uri);
    char *word = doc != NULL ? word_at_position(doc->text, line, character) : NULL;
    char *result = rename_result_json(doc, word, new_name);
    send_response_take(id_raw, result != NULL ? result : lsp_strdup("null"));
    free(uri);
    free(new_name);
    free(word);
}

static void handle_prepare_rename(const char *id_raw, const char *msg) {
    char *uri = json_get_string_key(msg, "uri");
    int line = json_get_number_key(msg, "line", 0);
    int character = json_get_number_key(msg, "character", 0);
    int start_line = 0;
    int start_col = 0;
    int end_col = 0;
    lsp_doc *doc = find_doc(uri);
    char *word = doc != NULL ? word_at_position(doc->text, line, character) : NULL;
    char *segment = doc != NULL ? ident_segment_at_position(doc->text, line, character, &start_line, &start_col, &end_col) : NULL;
    char *result = prepare_rename_result_json(doc, word, segment, start_line, start_col, end_col);
    send_response_take(id_raw, result != NULL ? result : lsp_strdup("null"));
    free(uri);
    free(word);
    free(segment);
}

static void handle_formatting(const char *id_raw, const char *msg) {
    char *uri = json_get_string_key(msg, "uri");
    lsp_doc *doc = find_doc(uri);
    char *result = formatting_result_json(doc);
    send_response_take(id_raw, result);
    free(uri);
}

static void handle_completion(const char *id_raw, const char *msg) {
    char *uri = json_get_string_key(msg, "uri");
    int line = json_get_number_key(msg, "line", 0);
    int character = json_get_number_key(msg, "character", 0);
    lsp_doc *doc = find_doc(uri);
    int member_access = doc != NULL ? completion_is_member_access(doc->text, line, character) : 0;
    int import_path = doc != NULL ? completion_is_import_path(doc->text, line, character) : 0;
    char *receiver = member_access && doc != NULL ? completion_receiver_name(doc->text, line, character) : NULL;
    char *receiver_type = receiver != NULL ? completion_receiver_type(doc, receiver, line, character) : NULL;
    char *result = completion_result_json(doc, receiver, receiver_type, member_access, import_path, line, character);
    send_response_take(id_raw, result);
    free(receiver_type);
    free(receiver);
    free(uri);
}

static void handle_document_symbol(const char *id_raw, const char *msg) {
    char *uri = json_get_string_key(msg, "uri");
    lsp_doc *doc = find_doc(uri);
    char *result = document_symbol_result_json(doc);
    send_response_take(id_raw, result);
    free(uri);
}

static void handle_semantic_tokens_full(const char *id_raw, const char *msg) {
    char *uri = json_get_string_key(msg, "uri");
    lsp_doc *doc = find_doc(uri);
    char *result = semantic_tokens_result_json(doc);
    send_response_take(id_raw, result);
    free(uri);
}

static void handle_workspace_symbol(const char *id_raw, const char *msg) {
    char *query = json_get_string_key(msg, "query");
    char *result = workspace_symbol_result_json(query != NULL ? query : "");
    send_response_take(id_raw, result);
    free(query);
}

static void handle_execute_command(const char *id_raw) {
    send_notification_take("window/showMessage", lsp_strdup("{\"type\":3,\"message\":\"Zenith command requested. Use the VSCode command for terminal execution.\"}"));
    send_response_take(id_raw, lsp_strdup("{\"applied\":true}"));
}

int main(void) {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    lsp_log("Compass LSP started");

    for (;;) {
        char *msg = read_message();
        char *method;
        char *id_raw;
        if (msg == NULL) break;
        method = json_get_string_key(msg, "method");
        id_raw = json_get_id_raw(msg);
        if (method != NULL) {
            if (strcmp(method, "initialize") == 0) {
                handle_initialize(id_raw, msg);
            } else if (strcmp(method, "shutdown") == 0) {
                send_response_take(id_raw, lsp_strdup("null"));
                id_raw = NULL;
            } else if (strcmp(method, "exit") == 0) {
                free(method);
                free(id_raw);
                free(msg);
                break;
            } else if (strcmp(method, "textDocument/didOpen") == 0 || strcmp(method, "textDocument/didChange") == 0) {
                char *uri = json_get_string_key(msg, "uri");
                char *text = json_get_string_key(msg, "text");
                lsp_doc *doc = NULL;
                if (uri != NULL && text != NULL) doc = upsert_doc(uri, text);
                if (doc != NULL) publish_diagnostics(doc);
                free(uri);
                free(text);
            } else if (strcmp(method, "textDocument/didClose") == 0) {
                char *uri = json_get_string_key(msg, "uri");
                if (uri != NULL) {
                    publish_empty_diagnostics(uri);
                    remove_doc(uri);
                }
                free(uri);
            } else if (strcmp(method, "textDocument/hover") == 0) {
                handle_hover_or_definition(id_raw, msg, 0);
                id_raw = NULL;
            } else if (strcmp(method, "textDocument/definition") == 0) {
                handle_hover_or_definition(id_raw, msg, 1);
                id_raw = NULL;
            } else if (strcmp(method, "textDocument/references") == 0) {
                handle_references(id_raw, msg);
                id_raw = NULL;
            } else if (strcmp(method, "textDocument/signatureHelp") == 0) {
                handle_signature_help(id_raw, msg);
                id_raw = NULL;
            } else if (strcmp(method, "textDocument/rename") == 0) {
                handle_rename(id_raw, msg);
                id_raw = NULL;
            } else if (strcmp(method, "textDocument/prepareRename") == 0) {
                handle_prepare_rename(id_raw, msg);
                id_raw = NULL;
            } else if (strcmp(method, "textDocument/formatting") == 0) {
                handle_formatting(id_raw, msg);
                id_raw = NULL;
            } else if (strcmp(method, "textDocument/completion") == 0) {
                handle_completion(id_raw, msg);
                id_raw = NULL;
            } else if (strcmp(method, "textDocument/documentSymbol") == 0) {
                handle_document_symbol(id_raw, msg);
                id_raw = NULL;
            } else if (strcmp(method, "textDocument/semanticTokens/full") == 0) {
                handle_semantic_tokens_full(id_raw, msg);
                id_raw = NULL;
            } else if (strcmp(method, "workspace/symbol") == 0) {
                handle_workspace_symbol(id_raw, msg);
                id_raw = NULL;
            } else if (strcmp(method, "workspace/executeCommand") == 0) {
                handle_execute_command(id_raw);
                id_raw = NULL;
            } else if (id_raw != NULL) {
                send_response_take(id_raw, lsp_strdup("null"));
                id_raw = NULL;
            }
        }
        free(method);
        free(id_raw);
        free(msg);
    }

    free_all_docs();
    free(g_workspace_root);
    lsp_log("Compass LSP exited");
    return 0;
}
