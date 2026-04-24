#include "compiler/utils/arena.h"
#include "compiler/utils/string_pool.h"
#include "compiler/frontend/lexer/token.h"
#include "compiler/frontend/parser/parser.h"
#include "compiler/semantic/binder/binder.h"
#include "compiler/semantic/types/checker.h"
#include "compiler/tooling/formatter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "compiler/utils/cJSON.h"

/* ---------------------------------------------------------------------------
 * Logger
 * --------------------------------------------------------------------------- */
static void lsp_log(const char *msg) {
    FILE *f = fopen("lsp.log", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

/* ---------------------------------------------------------------------------
 * JSON-RPC transport
 * --------------------------------------------------------------------------- */
static char *read_exact(size_t length) {
    char *buffer = (char *)malloc(length + 1);
    if (!buffer) return NULL;
    size_t read_bytes = fread(buffer, 1, length, stdin);
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

    while (fgets(line, sizeof(line), stdin)) {
        if (strcmp(line, "\r\n") == 0 || strcmp(line, "\n") == 0) {
            break;
        }
        if (strncmp(line, "Content-Length:", 15) == 0) {
            content_length = (size_t)strtoul(line + 15, NULL, 10);
        }
    }

    if (content_length == 0) return NULL;
    return read_exact(content_length);
}

static void send_json(cJSON *msg) {
    char *json_str = cJSON_PrintUnformatted(msg);
    if (json_str) {
        size_t len = strlen(json_str);
        fprintf(stdout, "Content-Length: %zu\r\n\r\n%s", len, json_str);
        fflush(stdout);
        free(json_str);
    }
}

static void send_response(cJSON *id, cJSON *result) {
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "jsonrpc", "2.0");
    if (id) {
        if (cJSON_IsNumber(id)) cJSON_AddNumberToObject(response, "id", id->valuedouble);
        else if (cJSON_IsString(id)) cJSON_AddStringToObject(response, "id", id->valuestring);
        else cJSON_AddNullToObject(response, "id");
    } else {
        cJSON_AddNullToObject(response, "id");
    }
    if (result) cJSON_AddItemToObject(response, "result", result);
    else cJSON_AddNullToObject(response, "result");
    send_json(response);
    cJSON_Delete(response);
}

static void send_notification(const char *method, cJSON *params) {
    cJSON *notif = cJSON_CreateObject();
    cJSON_AddStringToObject(notif, "jsonrpc", "2.0");
    cJSON_AddStringToObject(notif, "method", method);
    if (params) cJSON_AddItemToObject(notif, "params", params);
    send_json(notif);
    cJSON_Delete(notif);
}

/* ---------------------------------------------------------------------------
 * Helpers
 * --------------------------------------------------------------------------- */
extern zt_arena global_arena;
extern zt_string_pool global_pool;

static void uri_to_path(const char *uri, char *path, size_t capacity) {
    /* file:///C:/... -> C:/...  or  file:///home/... -> /home/... */
    const char *prefix = "file://";
    if (strncmp(uri, prefix, 7) == 0) {
        const char *rest = uri + 7;
        /* Skip leading slash on Windows paths (file:///C:...) */
        if (rest[0] == '/' && isalpha((unsigned char)rest[1]) && rest[2] == ':') {
            rest++;
        }
        snprintf(path, capacity, "%s", rest);
    } else {
        snprintf(path, capacity, "%s", uri);
    }
}

static int position_in_range(const zt_source_span *span, int line, int col) {
    if (span == NULL) return 0;
    int s_line = (int)span->line;
    int s_col = (int)span->column_start;
    int e_line = (int)span->end_line > 0 ? (int)span->end_line : s_line;
    int e_col = (int)span->end_column > 0 ? (int)span->end_column : s_col;
    /* LSP uses 0-based lines/cols; our spans are 1-based */
    int lsp_line = line + 1;
    int lsp_col = col + 1;
    if (lsp_line < s_line || lsp_line > e_line) return 0;
    if (lsp_line == s_line && lsp_col < s_col) return 0;
    if (lsp_line == e_line && lsp_col > e_col) return 0;
    return 1;
}

/* ---------------------------------------------------------------------------
 * Diagnostics
 * --------------------------------------------------------------------------- */
static void append_diagnostics(cJSON *arr, const zt_diag_list *list) {
    for (size_t i = 0; i < list->count; i++) {
        zt_diag *d = &list->items[i];
        cJSON *obj = cJSON_CreateObject();

        int sev = 1;
        switch (d->severity) {
            case ZT_DIAG_SEVERITY_ERROR: sev = 1; break;
            case ZT_DIAG_SEVERITY_WARNING: sev = 2; break;
            case ZT_DIAG_SEVERITY_NOTE: sev = 3; break;
            case ZT_DIAG_SEVERITY_HELP: sev = 4; break;
        }
        cJSON_AddNumberToObject(obj, "severity", sev);

        cJSON *range = cJSON_CreateObject();
        cJSON *start = cJSON_CreateObject();
        cJSON_AddNumberToObject(start, "line", d->span.line > 0 ? (int)d->span.line - 1 : 0);
        cJSON_AddNumberToObject(start, "character", d->span.column_start > 0 ? (int)d->span.column_start - 1 : 0);
        cJSON *end = cJSON_CreateObject();
        cJSON_AddNumberToObject(end, "line", d->span.line > 0 ? (int)d->span.line - 1 : 0);
        cJSON_AddNumberToObject(end, "character", d->span.column_end > 0 ? (int)d->span.column_end - 1 : 0);
        cJSON_AddItemToObject(range, "start", start);
        cJSON_AddItemToObject(range, "end", end);
        cJSON_AddItemToObject(obj, "range", range);

        cJSON_AddStringToObject(obj, "message", d->message);
        cJSON_AddStringToObject(obj, "source", "zenith");

        cJSON_AddItemToArray(arr, obj);
    }
}

/* ---------------------------------------------------------------------------
 * Document store (simple single-document cache)
 * --------------------------------------------------------------------------- */
static char *g_doc_uri = NULL;
static char *g_doc_text = NULL;
static zt_parser_result g_parse = {0};
static zt_bind_result g_bind = {0};
static zt_check_result g_check = {0};
static int g_has_doc = 0;

static void free_doc(void) {
    if (g_has_doc) {
        free(g_doc_uri); g_doc_uri = NULL;
        free(g_doc_text); g_doc_text = NULL;
        zt_parser_result_dispose(&g_parse);
        zt_bind_result_dispose(&g_bind);
        zt_check_result_dispose(&g_check);
        g_has_doc = 0;
    }
}

static int open_or_update_doc(const char *uri, const char *text) {
    free_doc();
    g_doc_uri = strdup(uri);
    g_doc_text = strdup(text);
    if (!g_doc_uri || !g_doc_text) { free_doc(); return 0; }

    g_parse = zt_parse(&global_arena, &global_pool, uri, text, strlen(text));
    if (g_parse.root) {
        g_bind = zt_bind_file(g_parse.root);
        g_check = zt_check_file(g_parse.root);
    }
    g_has_doc = 1;
    return 1;
}

static void publish_diagnostics(void) {
    if (!g_has_doc) return;
    cJSON *diags = cJSON_CreateArray();
    append_diagnostics(diags, &g_parse.diagnostics);
    if (g_parse.root) {
        append_diagnostics(diags, &g_bind.diagnostics);
        append_diagnostics(diags, &g_check.diagnostics);
    }
    cJSON *params = cJSON_CreateObject();
    cJSON_AddStringToObject(params, "uri", g_doc_uri);
    cJSON_AddItemToObject(params, "diagnostics", diags);
    send_notification("textDocument/publishDiagnostics", params);
}

/* ---------------------------------------------------------------------------
 * Hover
 * --------------------------------------------------------------------------- */
static cJSON *handle_hover(cJSON *params) {
    if (!g_has_doc || !g_parse.root) return NULL;

    cJSON *textDoc = cJSON_GetObjectItem(params, "textDocument");
    cJSON *pos = cJSON_GetObjectItem(params, "position");
    if (!textDoc || !pos) return NULL;

    int line = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(pos, "line"));
    int col = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(pos, "character"));

    /* Walk AST to find node at position */
    const zt_ast_node *target = NULL;
    for (size_t i = 0; i < g_parse.root->as.module.decls.count; i++) {
        const zt_ast_node *decl = g_parse.root->as.module.decls.items[i];
        if (decl && position_in_range(&decl->span, line, col)) {
            target = decl;
            /* Try to find more specific child */
            if (decl->as.func_decl.body) {
                /* For now, use the function decl itself */
            }
        }
    }

    if (!target) return NULL;

    /* Build hover content */
    char hover_text[512];
    const char *kind_str = "symbol";
    switch (target->kind) {
        case ZT_AST_FUNC_DECL:
            kind_str = "func";
            snprintf(hover_text, sizeof(hover_text), "```zenith\nfunc %s(", target->as.func_decl.name);
            for (size_t p = 0; p < target->as.func_decl.params.count; p++) {
                const zt_ast_node *param = target->as.func_decl.params.items[p];
                if (p > 0) strncat(hover_text, ", ", sizeof(hover_text) - strlen(hover_text) - 1);
                if (param) {
                    strncat(hover_text, param->as.param.name, sizeof(hover_text) - strlen(hover_text) - 1);
                    if (param->as.param.type_node) {
                        strncat(hover_text, ": ", sizeof(hover_text) - strlen(hover_text) - 1);
                        /* Simplified type display */
                        strncat(hover_text, "<type>", sizeof(hover_text) - strlen(hover_text) - 1);
                    }
                }
            }
            strncat(hover_text, ")", sizeof(hover_text) - strlen(hover_text) - 1);
            if (target->as.func_decl.return_type) {
                strncat(hover_text, " -> <type>", sizeof(hover_text) - strlen(hover_text) - 1);
            }
            strncat(hover_text, "\n```", sizeof(hover_text) - strlen(hover_text) - 1);
            break;
        case ZT_AST_STRUCT_DECL:
            kind_str = "struct";
            snprintf(hover_text, sizeof(hover_text), "```zenith\nstruct %s\n```", target->as.struct_decl.name);
            break;
        case ZT_AST_TRAIT_DECL:
            kind_str = "trait";
            snprintf(hover_text, sizeof(hover_text), "```zenith\ntrait %s\n```", target->as.trait_decl.name);
            break;
        case ZT_AST_VAR_DECL:
            kind_str = "var";
            snprintf(hover_text, sizeof(hover_text), "```zenith\nvar %s\n```", target->as.var_decl.name);
            break;
        case ZT_AST_CONST_DECL:
            kind_str = "const";
            snprintf(hover_text, sizeof(hover_text), "```zenith\nconst %s\n```", target->as.const_decl.name);
            break;
        default:
            return NULL;
    }

    cJSON *result = cJSON_CreateObject();
    cJSON *contents = cJSON_CreateObject();
    cJSON_AddStringToObject(contents, "kind", "markdown");
    cJSON_AddStringToObject(contents, "value", hover_text);
    cJSON_AddItemToObject(result, "contents", contents);

    cJSON *range = cJSON_CreateObject();
    cJSON *start = cJSON_CreateObject();
    cJSON_AddNumberToObject(start, "line", target->span.line > 0 ? (int)target->span.line - 1 : 0);
    cJSON_AddNumberToObject(start, "character", target->span.column_start > 0 ? (int)target->span.column_start - 1 : 0);
    cJSON *end = cJSON_CreateObject();
    cJSON_AddNumberToObject(end, "line", target->span.line > 0 ? (int)target->span.line - 1 : 0);
    cJSON_AddNumberToObject(end, "character", target->span.column_end > 0 ? (int)target->span.column_end - 1 : 0);
    cJSON_AddItemToObject(range, "start", start);
    cJSON_AddItemToObject(range, "end", end);
    cJSON_AddItemToObject(result, "range", range);

    return result;
}

/* ---------------------------------------------------------------------------
 * Go-to-Definition
 * --------------------------------------------------------------------------- */
static cJSON *handle_definition(cJSON *params) {
    if (!g_has_doc || !g_parse.root) return NULL;

    cJSON *textDoc = cJSON_GetObjectItem(params, "textDocument");
    cJSON *pos = cJSON_GetObjectItem(params, "position");
    if (!textDoc || !pos) return NULL;

    int line = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(pos, "line"));
    int col = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(pos, "character"));

    /* Walk declarations to find definition at position */
    for (size_t i = 0; i < g_parse.root->as.module.decls.count; i++) {
        const zt_ast_node *decl = g_parse.root->as.module.decls.items[i];
        if (decl && position_in_range(&decl->span, line, col)) {
            /* Found a definition - return its location */
            cJSON *loc = cJSON_CreateObject();
            cJSON_AddStringToObject(loc, "uri", g_doc_uri);
            cJSON *range = cJSON_CreateObject();
            cJSON *start = cJSON_CreateObject();
            cJSON_AddNumberToObject(start, "line", decl->span.line > 0 ? (int)decl->span.line - 1 : 0);
            cJSON_AddNumberToObject(start, "character", decl->span.column_start > 0 ? (int)decl->span.column_start - 1 : 0);
            cJSON *end = cJSON_CreateObject();
            cJSON_AddNumberToObject(end, "line", decl->span.line > 0 ? (int)decl->span.line - 1 : 0);
            cJSON_AddNumberToObject(end, "character", decl->span.column_end > 0 ? (int)decl->span.column_end - 1 : 0);
            cJSON_AddItemToObject(range, "start", start);
            cJSON_AddItemToObject(range, "end", end);
            cJSON_AddItemToObject(loc, "range", range);
            return loc;
        }
    }

    return NULL;
}

/* ---------------------------------------------------------------------------
 * Formatting
 * --------------------------------------------------------------------------- */
static cJSON *handle_formatting(cJSON *params) {
    if (!g_has_doc || !g_doc_text) return NULL;

    char *formatted = zt_format_string(g_doc_text);
    if (!formatted) return NULL;

    /* If no changes needed, return empty */
    if (strcmp(formatted, g_doc_text) == 0) {
        free(formatted);
        return cJSON_CreateArray();
    }

    /* Return a single full-document edit */
    cJSON *arr = cJSON_CreateArray();
    cJSON *edit = cJSON_CreateObject();

    cJSON *range = cJSON_CreateObject();
    cJSON *start = cJSON_CreateObject();
    cJSON_AddNumberToObject(start, "line", 0);
    cJSON_AddNumberToObject(start, "character", 0);
    cJSON *end = cJSON_CreateObject();
    /* Count lines in original */
    int line_count = 0;
    for (const char *p = g_doc_text; *p; p++) { if (*p == '\n') line_count++; }
    cJSON_AddNumberToObject(end, "line", line_count);
    cJSON_AddNumberToObject(end, "character", 0);
    cJSON_AddItemToObject(range, "start", start);
    cJSON_AddItemToObject(range, "end", end);
    cJSON_AddItemToObject(edit, "range", range);

    cJSON_AddStringToObject(edit, "newText", formatted);
    cJSON_AddItemToArray(arr, edit);

    free(formatted);
    return arr;
}

/* ---------------------------------------------------------------------------
 * Request handlers
 * --------------------------------------------------------------------------- */
static void handle_initialize(cJSON *id) {
    lsp_log("Handling 'initialize'");
    cJSON *caps = cJSON_CreateObject();
    cJSON_AddNumberToObject(caps, "textDocumentSync", 1); /* Full */
    cJSON_AddBoolToObject(caps, "hoverProvider", 1);
    cJSON_AddBoolToObject(caps, "definitionProvider", 1);
    cJSON_AddBoolToObject(caps, "documentFormattingProvider", 1);

    cJSON *result = cJSON_CreateObject();
    cJSON_AddItemToObject(result, "capabilities", caps);
    send_response(id, result);
}

/* ---------------------------------------------------------------------------
 * Main loop
 * --------------------------------------------------------------------------- */
int main(void) {
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    lsp_log("LSP Started");

    while (1) {
        char *msg_str = read_message();
        if (!msg_str) {
            lsp_log("Connection closed. Exiting.");
            break;
        }

        cJSON *req = cJSON_Parse(msg_str);
        if (req) {
            cJSON *method = cJSON_GetObjectItem(req, "method");
            cJSON *id = cJSON_GetObjectItem(req, "id");

            if (cJSON_IsString(method)) {
                const char *meth = method->valuestring;

                if (strcmp(meth, "initialize") == 0) {
                    handle_initialize(id);
                } else if (strcmp(meth, "shutdown") == 0) {
                    send_response(id, NULL);
                } else if (strcmp(meth, "exit") == 0) {
                    cJSON_Delete(req);
                    free(msg_str);
                    break;
                } else if (strcmp(meth, "textDocument/didOpen") == 0) {
                    cJSON *params = cJSON_GetObjectItem(req, "params");
                    cJSON *textDoc = cJSON_GetObjectItem(params, "textDocument");
                    cJSON *uri = cJSON_GetObjectItem(textDoc, "uri");
                    cJSON *text = cJSON_GetObjectItem(textDoc, "text");
                    if (cJSON_IsString(uri) && cJSON_IsString(text)) {
                        open_or_update_doc(uri->valuestring, text->valuestring);
                        publish_diagnostics();
                    }
                } else if (strcmp(meth, "textDocument/didChange") == 0) {
                    cJSON *params = cJSON_GetObjectItem(req, "params");
                    cJSON *textDoc = cJSON_GetObjectItem(params, "textDocument");
                    cJSON *uri = cJSON_GetObjectItem(textDoc, "uri");
                    cJSON *changes = cJSON_GetObjectItem(params, "contentChanges");
                    if (cJSON_IsString(uri) && cJSON_IsArray(changes)) {
                        cJSON *change = cJSON_GetArrayItem(changes, 0);
                        cJSON *text = cJSON_GetObjectItem(change, "text");
                        if (cJSON_IsString(text)) {
                            open_or_update_doc(uri->valuestring, text->valuestring);
                            publish_diagnostics();
                        }
                    }
                } else if (strcmp(meth, "textDocument/didClose") == 0) {
                    free_doc();
                } else if (strcmp(meth, "textDocument/hover") == 0) {
                    cJSON *params = cJSON_GetObjectItem(req, "params");
                    cJSON *result = handle_hover(params);
                    send_response(id, result);
                    if (result) cJSON_Delete(result);
                } else if (strcmp(meth, "textDocument/definition") == 0) {
                    cJSON *params = cJSON_GetObjectItem(req, "params");
                    cJSON *result = handle_definition(params);
                    send_response(id, result);
                    if (result) cJSON_Delete(result);
                } else if (strcmp(meth, "textDocument/formatting") == 0) {
                    cJSON *params = cJSON_GetObjectItem(req, "params");
                    cJSON *result = handle_formatting(params);
                    send_response(id, result);
                    if (result) cJSON_Delete(result);
                } else {
                    lsp_log("Unknown method");
                }
            }
            cJSON_Delete(req);
        } else {
            lsp_log("Failed to parse JSON");
        }
        free(msg_str);
    }

    free_doc();
    lsp_log("LSP Exited");
    return 0;
}
