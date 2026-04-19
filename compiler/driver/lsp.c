#include "compiler/utils/arena.h"
#include "compiler/utils/string_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "compiler/utils/cJSON.h"
#include "compiler/frontend/parser/parser.h"
#include "compiler/semantic/binder/binder.h"
#include "compiler/semantic/types/checker.h"

// Basic logger for debugging
void lsp_log(const char *msg) {
    FILE *f = fopen("lsp.log", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

// Reads exactly 'length' bytes from stdin
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

// Reads the next JSON-RPC message from stdin
static char *read_message(void) {
    char line[1024];
    size_t content_length = 0;

    // Read headers until \r\n
    while (fgets(line, sizeof(line), stdin)) {
        if (strcmp(line, "\r\n") == 0 || strcmp(line, "\n") == 0) {
            break;
        }
        if (strncmp(line, "Content-Length:", 15) == 0) {
            content_length = (size_t)strtoul(line + 15, NULL, 10);
        }
    }

    if (content_length == 0) {
        return NULL; // Connection closed or invalid header
    }

    return read_exact(content_length);
}

// Sends a JSON-RPC message to stdout
static void send_message(cJSON *msg) {
    char *json_str = cJSON_PrintUnformatted(msg);
    if (json_str) {
        size_t len = strlen(json_str);
        fprintf(stdout, "Content-Length: %zu\r\n\r\n%s", len, json_str);
        fflush(stdout);
        free(json_str);
    }
}

// Handle the 'initialize' request
static void handle_initialize(cJSON *id) {
    lsp_log("Handling 'initialize'");
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "jsonrpc", "2.0");
    if (id) {
        if (cJSON_IsNumber(id)) {
            cJSON_AddNumberToObject(response, "id", id->valuedouble);
        } else if (cJSON_IsString(id)) {
            cJSON_AddStringToObject(response, "id", id->valuestring);
        } else {
            cJSON_AddNullToObject(response, "id");
        }
    } else {
        cJSON_AddNullToObject(response, "id");
    }

    cJSON *result = cJSON_CreateObject();
    cJSON *capabilities = cJSON_CreateObject();
    
    // Add text document sync capability (Full = 1)
    cJSON_AddNumberToObject(capabilities, "textDocumentSync", 1);
    
    // Add hover capability
    cJSON_AddBoolToObject(capabilities, "hoverProvider", 1);

    cJSON_AddItemToObject(result, "capabilities", capabilities);
    cJSON_AddItemToObject(response, "result", result);

    send_message(response);
    cJSON_Delete(response);
}

static void append_diagnostics(cJSON *diagnostics_array, const zt_diag_list *diag_list) {
    for (size_t i = 0; i < diag_list->count; i++) {
        zt_diag *diag = &diag_list->items[i];
        
        cJSON *diag_obj = cJSON_CreateObject();
        
        // Map severity
        int lsp_severity = 1; // Error
        switch (diag->severity) {
            case ZT_DIAG_SEVERITY_ERROR: lsp_severity = 1; break;
            case ZT_DIAG_SEVERITY_WARNING: lsp_severity = 2; break;
            case ZT_DIAG_SEVERITY_NOTE: lsp_severity = 3; break;
            case ZT_DIAG_SEVERITY_HELP: lsp_severity = 4; break;
        }
        cJSON_AddNumberToObject(diag_obj, "severity", lsp_severity);
        
        cJSON *range = cJSON_CreateObject();
        cJSON *start = cJSON_CreateObject();
        cJSON_AddNumberToObject(start, "line", diag->span.line > 0 ? diag->span.line - 1 : 0);
        cJSON_AddNumberToObject(start, "character", diag->span.column_start > 0 ? diag->span.column_start - 1 : 0);
        
        cJSON *end = cJSON_CreateObject();
        cJSON_AddNumberToObject(end, "line", diag->span.line > 0 ? diag->span.line - 1 : 0);
        cJSON_AddNumberToObject(end, "character", diag->span.column_end > 0 ? diag->span.column_end - 1 : 0);
        
        cJSON_AddItemToObject(range, "start", start);
        cJSON_AddItemToObject(range, "end", end);
        cJSON_AddItemToObject(diag_obj, "range", range);
        
        cJSON_AddStringToObject(diag_obj, "message", diag->message);
        cJSON_AddStringToObject(diag_obj, "source", "zenith");
        
        cJSON_AddItemToArray(diagnostics_array, diag_obj);
    }
}

extern zt_arena global_arena;
extern zt_string_pool global_pool;

void process_document(const char *uri, const char *text) {
    zt_parser_result parse_result = zt_parse(&global_arena, &global_pool, uri, text, strlen(text));
    
    cJSON *notification = cJSON_CreateObject();
    cJSON_AddStringToObject(notification, "jsonrpc", "2.0");
    cJSON_AddStringToObject(notification, "method", "textDocument/publishDiagnostics");
    
    cJSON *params = cJSON_CreateObject();
    cJSON_AddStringToObject(params, "uri", uri);
    
    cJSON *diagnostics = cJSON_CreateArray();
    
    append_diagnostics(diagnostics, &parse_result.diagnostics);
    
    if (parse_result.root) {
        zt_bind_result bind_result = zt_bind_file(parse_result.root);
        append_diagnostics(diagnostics, &bind_result.diagnostics);
        
        zt_check_result check_result = zt_check_file(parse_result.root);
        append_diagnostics(diagnostics, &check_result.diagnostics);
        
        zt_check_result_dispose(&check_result);
        zt_bind_result_dispose(&bind_result);
    }
    
    cJSON_AddItemToObject(params, "diagnostics", diagnostics);
    cJSON_AddItemToObject(notification, "params", params);
    
    send_message(notification);
    cJSON_Delete(notification);
    
    zt_parser_result_dispose(&parse_result);
}

int main(void) {
    // Disable buffering on stdin/stdout to prevent blocking
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    lsp_log("LSP Started");

    while (1) {
        char *msg_str = read_message();
        if (!msg_str) {
            lsp_log("Failed to read message or connection closed. Exiting.");
            break;
        }

        cJSON *req = cJSON_Parse(msg_str);
        if (req) {
            cJSON *method = cJSON_GetObjectItem(req, "method");
            cJSON *id = cJSON_GetObjectItem(req, "id");

            if (cJSON_IsString(method)) {
                if (strcmp(method->valuestring, "initialize") == 0) {
                    handle_initialize(id);
                } else if (strcmp(method->valuestring, "shutdown") == 0) {
                    lsp_log("Handling 'shutdown'");
                    cJSON *response = cJSON_CreateObject();
                    cJSON_AddStringToObject(response, "jsonrpc", "2.0");
                    if (id) { cJSON_AddItemToObject(response, "id", cJSON_Duplicate(id, 1)); }
                    cJSON_AddNullToObject(response, "result");
                    send_message(response);
                    cJSON_Delete(response);
                } else if (strcmp(method->valuestring, "exit") == 0) {
                    lsp_log("Handling 'exit'");
                    cJSON_Delete(req);
                    free(msg_str);
                    break;
                } else if (strcmp(method->valuestring, "textDocument/didOpen") == 0) {
                    cJSON *params = cJSON_GetObjectItem(req, "params");
                    cJSON *textDoc = cJSON_GetObjectItem(params, "textDocument");
                    cJSON *uri = cJSON_GetObjectItem(textDoc, "uri");
                    cJSON *text = cJSON_GetObjectItem(textDoc, "text");
                    if (cJSON_IsString(uri) && cJSON_IsString(text)) {
                        process_document(uri->valuestring, text->valuestring);
                    }
                } else if (strcmp(method->valuestring, "textDocument/didChange") == 0) {
                    cJSON *params = cJSON_GetObjectItem(req, "params");
                    cJSON *textDoc = cJSON_GetObjectItem(params, "textDocument");
                    cJSON *uri = cJSON_GetObjectItem(textDoc, "uri");
                    cJSON *contentChanges = cJSON_GetObjectItem(params, "contentChanges");
                    if (cJSON_IsString(uri) && cJSON_IsArray(contentChanges)) {
                        cJSON *change = cJSON_GetArrayItem(contentChanges, 0);
                        cJSON *text = cJSON_GetObjectItem(change, "text");
                        if (cJSON_IsString(text)) {
                            process_document(uri->valuestring, text->valuestring);
                        }
                    }
                } else {
                    // Ignore other methods for now
                }
            }
            cJSON_Delete(req);
        } else {
            lsp_log("Failed to parse JSON");
        }
        
        free(msg_str);
    }

    lsp_log("LSP Exited");
    return 0;
}
