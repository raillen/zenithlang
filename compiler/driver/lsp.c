#include "compiler/utils/arena.h"
#include "compiler/utils/string_pool.h"
#include "compiler/frontend/parser/parser.h"
#include "compiler/semantic/binder/binder.h"
#include "compiler/semantic/types/checker.h"
#include "compiler/tooling/formatter.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#define LSP_EXE_SUFFIX ".exe"
#else
#define LSP_EXE_SUFFIX ""
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

static void append_completion_item(
        lsp_sb *sb,
        int *first,
        const char *label,
        int kind,
        const char *detail,
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
            find_type_for_binding_in_node(node->as.for_stmt.body, name, line, character, candidate);
            break;
        case ZT_AST_REPEAT_STMT:
            find_type_for_binding_in_node(node->as.repeat_stmt.body, name, line, character, candidate);
            break;
        case ZT_AST_MATCH_STMT:
            find_type_for_binding_in_list(node->as.match_stmt.cases, name, line, character, candidate);
            break;
        case ZT_AST_MATCH_CASE:
            find_type_for_binding_in_node(node->as.match_case.body, name, line, character, candidate);
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

static const zt_ast_node *find_struct_decl_in_doc(lsp_doc *doc, const char *type_name) {
    char *base;
    const zt_ast_node *result = NULL;
    size_t i;
    if (doc == NULL || type_name == NULL || !doc->has_parse || doc->parse.root == NULL) return NULL;
    if (doc->parse.root->kind != ZT_AST_FILE) return NULL;
    base = type_base_name(type_name);
    if (base == NULL) return NULL;
    for (i = 0; i < doc->parse.root->as.file.declarations.count; i += 1) {
        const zt_ast_node *decl = doc->parse.root->as.file.declarations.items[i];
        if (decl != NULL &&
            decl->kind == ZT_AST_STRUCT_DECL &&
            decl->as.struct_decl.name != NULL &&
            strcmp(decl->as.struct_decl.name, base) == 0) {
            result = decl;
            break;
        }
    }
    free(base);
    return result;
}

static const zt_ast_node *find_struct_decl(lsp_doc *doc, const char *type_name) {
    const zt_ast_node *found;
    lsp_doc *other;
    found = find_struct_decl_in_doc(doc, type_name);
    if (found != NULL) return found;
    for (other = g_docs; other != NULL; other = other->next) {
        if (other == doc) continue;
        found = find_struct_decl_in_doc(other, type_name);
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

static void append_apply_method_completions_from_doc(lsp_sb *sb, int *first, lsp_doc *doc, const char *receiver_type) {
    char *base;
    size_t i;
    if (doc == NULL || receiver_type == NULL || !doc->has_parse || doc->parse.root == NULL) return;
    if (doc->parse.root->kind != ZT_AST_FILE) return;
    base = type_base_name(receiver_type);
    if (base == NULL) return;
    for (i = 0; i < doc->parse.root->as.file.declarations.count; i += 1) {
        const zt_ast_node *decl = doc->parse.root->as.file.declarations.items[i];
        size_t m;
        if (decl == NULL || decl->kind != ZT_AST_APPLY_DECL || decl->as.apply_decl.target_name == NULL) continue;
        if (strcmp(decl->as.apply_decl.target_name, base) != 0) continue;
        for (m = 0; m < decl->as.apply_decl.methods.count; m += 1) {
            const zt_ast_node *method = decl->as.apply_decl.methods.items[m];
            lsp_sb insert_text;
            if (method == NULL || method->kind != ZT_AST_FUNC_DECL || method->as.func_decl.name == NULL) continue;
            sb_init(&insert_text);
            sb_append(&insert_text, method->as.func_decl.name);
            sb_append(&insert_text, "(${0})");
            append_completion_item(
                sb,
                first,
                method->as.func_decl.name,
                2,
                method->as.func_decl.is_mutating ? "metodo mutating via apply" : "metodo via apply",
                insert_text.data,
                2);
            free(insert_text.data);
        }
    }
    free(base);
}

static void append_apply_method_completions(lsp_sb *sb, int *first, lsp_doc *doc, const char *receiver_type) {
    lsp_doc *other;
    append_apply_method_completions_from_doc(sb, first, doc, receiver_type);
    for (other = g_docs; other != NULL; other = other->next) {
        if (other == doc) continue;
        append_apply_method_completions_from_doc(sb, first, other, receiver_type);
    }
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
            break;
        case ZT_AST_CONST_DECL:
            if (node->as.const_decl.name != NULL && ast_node_starts_before_position(node, line, character)) {
                append_completion_item(sb, first, node->as.const_decl.name, 21, "const local", NULL, 0);
            }
            break;
        case ZT_AST_IF_STMT:
            append_local_completions_from_node(sb, first, node->as.if_stmt.then_block, line, character);
            append_local_completions_from_node(sb, first, node->as.if_stmt.else_block, line, character);
            break;
        case ZT_AST_WHILE_STMT:
            append_local_completions_from_node(sb, first, node->as.while_stmt.body, line, character);
            break;
        case ZT_AST_FOR_STMT:
            append_local_completions_from_node(sb, first, node->as.for_stmt.body, line, character);
            break;
        case ZT_AST_REPEAT_STMT:
            append_local_completions_from_node(sb, first, node->as.repeat_stmt.body, line, character);
            break;
        case ZT_AST_MATCH_STMT:
            append_local_completions_from_list(sb, first, node->as.match_stmt.cases, line, character);
            break;
        case ZT_AST_MATCH_CASE:
            append_local_completions_from_node(sb, first, node->as.match_case.body, line, character);
            break;
        default:
            break;
    }
}

static void append_builtin_completions(lsp_sb *sb, int *first) {
    append_completion_item(sb, first, "namespace", 14, "declara namespace do arquivo", NULL, 0);
    append_completion_item(sb, first, "import", 14, "importa outro namespace", "import ${1:modulo} as ${2:alias}", 2);
    append_completion_item(sb, first, "func", 14, "declara funcao", "func ${1:nome}(${2}) -> ${3:void}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "return", 14, "retorna de uma funcao", NULL, 0);
    append_completion_item(sb, first, "var", 14, "binding mutavel local", "var ${1:nome}: ${2:tipo} = ${0:valor}", 2);
    append_completion_item(sb, first, "const", 14, "binding imutavel", "const ${1:nome}: ${2:tipo} = ${0:valor}", 2);
    append_completion_item(sb, first, "public", 14, "exporta API do namespace", NULL, 0);
    append_completion_item(sb, first, "struct", 14, "declara struct", "struct ${1:Nome}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "trait", 14, "declara trait", "trait ${1:Nome}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "enum", 14, "declara enum", "enum ${1:Nome}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "if", 14, "controle condicional", "if ${1:condicao}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "match", 14, "pattern matching", "match ${1:valor}\ncase ${2:padrao}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "while", 14, "loop condicional", "while ${1:condicao}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "for", 14, "loop por colecao", "for ${1:item} in ${2:colecao}\n    ${0}\nend", 2);
    append_completion_item(sb, first, "end", 14, "fecha bloco", NULL, 0);

    append_completion_item(sb, first, "int", 7, "tipo inteiro", NULL, 0);
    append_completion_item(sb, first, "float", 7, "tipo decimal", NULL, 0);
    append_completion_item(sb, first, "bool", 7, "tipo booleano", NULL, 0);
    append_completion_item(sb, first, "text", 7, "tipo texto", NULL, 0);
    append_completion_item(sb, first, "bytes", 7, "tipo bytes", NULL, 0);
    append_completion_item(sb, first, "void", 7, "sem valor de retorno", NULL, 0);
    append_completion_item(sb, first, "optional", 7, "tipo optional<T>", "optional<${1:T}>", 2);
    append_completion_item(sb, first, "result", 7, "tipo result<T, E>", "result<${1:T}, ${2:core.Error}>", 2);
    append_completion_item(sb, first, "list", 7, "tipo list<T>", "list<${1:T}>", 2);
    append_completion_item(sb, first, "map", 7, "tipo map<K, V>", "map<${1:K}, ${2:V}>", 2);
    append_completion_item(sb, first, "core.Error", 7, "tipo de erro padrao", NULL, 0);

    append_completion_item(sb, first, "true", 12, "literal bool", NULL, 0);
    append_completion_item(sb, first, "false", 12, "literal bool", NULL, 0);
    append_completion_item(sb, first, "none", 12, "ausencia optional", NULL, 0);
    append_completion_item(sb, first, "success", 3, "cria result success", "success(${0})", 2);
    append_completion_item(sb, first, "error", 3, "cria result error", "error(${0})", 2);
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

static void append_import_path_completions(lsp_sb *sb, int *first, lsp_doc *doc) {
    static const char *std_modules[] = {
        "std.bytes",
        "std.collections",
        "std.concurrent",
        "std.format",
        "std.fs",
        "std.fs.path",
        "std.io",
        "std.json",
        "std.lazy",
        "std.math",
        "std.net",
        "std.os",
        "std.os.process",
        "std.random",
        "std.test",
        "std.text",
        "std.time",
        "std.validate",
        NULL
    };
    size_t i;
    lsp_doc *other;
    const char *current_ns = doc_module_name(doc);

    for (i = 0; std_modules[i] != NULL; i += 1) {
        append_completion_item(sb, first, std_modules[i], 9, "stdlib", NULL, 0);
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

static void append_doc_symbol_completions(lsp_sb *sb, int *first, lsp_doc *doc) {
    size_t i;
    if (doc == NULL || !doc->has_parse || doc->parse.root == NULL || doc->parse.root->kind != ZT_AST_FILE) return;
    for (i = 0; i < doc->parse.root->as.file.declarations.count; i += 1) {
        const zt_ast_node *decl = doc->parse.root->as.file.declarations.items[i];
        const char *name = node_symbol_name(decl);
        if (name != NULL) {
            append_completion_item(
                sb,
                first,
                name,
                completion_kind_for_node(decl),
                node_symbol_kind(decl),
                NULL,
                0);
        }
    }
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
        if (name == NULL) continue;
        if (!node_is_public(decl) && !namespace_is_std(import_path)) continue;
        sb_init(&detail);
        sb_append(&detail, import_path);
        sb_append(&detail, ".");
        sb_append(&detail, name);
        sb_init(&insert_text);
        if (decl->kind == ZT_AST_FUNC_DECL) {
            sb_append(&insert_text, name);
            sb_append(&insert_text, "(${0})");
        }
        append_completion_item(
            sb,
            first,
            name,
            completion_kind_for_node(decl),
            detail.data,
            decl->kind == ZT_AST_FUNC_DECL ? insert_text.data : NULL,
            decl->kind == ZT_AST_FUNC_DECL ? 2 : 0);
        free(insert_text.data);
        free(detail.data);
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

static void handle_initialize(const char *id_raw) {
    char *result = lsp_strdup(
        "{\"capabilities\":{"
        "\"textDocumentSync\":1,"
        "\"hoverProvider\":true,"
        "\"definitionProvider\":true,"
        "\"documentFormattingProvider\":true,"
        "\"completionProvider\":{\"resolveProvider\":false,\"triggerCharacters\":[\".\"]},"
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
                handle_initialize(id_raw);
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
            } else if (strcmp(method, "textDocument/formatting") == 0) {
                handle_formatting(id_raw, msg);
                id_raw = NULL;
            } else if (strcmp(method, "textDocument/completion") == 0) {
                handle_completion(id_raw, msg);
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
    lsp_log("Compass LSP exited");
    return 0;
}
