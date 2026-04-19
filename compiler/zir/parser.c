#include "compiler/zir/parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ZIR_PARSER_MAX_LINE 8192
#define ZIR_PARSER_MAX_PARAMS 32
#define ZIR_PARSER_MAX_FUNCTIONS 64
#define ZIR_PARSER_MAX_BLOCKS 32
#define ZIR_PARSER_MAX_INSTRUCTIONS 256

static void zir_parse_set_error(zir_parse_result *result, zir_parse_error_code code, int line, const char *message) {
    result->ok = 0;
    result->code = code;
    result->line_number = line;
    snprintf(result->message, sizeof(result->message), "%s", message != NULL ? message : "");
}

const char *zir_parse_error_code_name(zir_parse_error_code code) {
    switch (code) {
        case ZIR_PARSE_OK: return "zir.parse.ok";
        case ZIR_PARSE_INVALID_INPUT: return "zir.parse.invalid_input";
        case ZIR_PARSE_EXPECTED_MODULE: return "zir.parse.expected_module";
        case ZIR_PARSE_EXPECTED_FUNC: return "zir.parse.expected_func";
        case ZIR_PARSE_EXPECTED_BLOCK: return "zir.parse.expected_block";
        case ZIR_PARSE_EXPECTED_ARROW: return "zir.parse.expected_arrow";
        case ZIR_PARSE_EXPECTED_COLON: return "zir.parse.expected_colon";
        case ZIR_PARSE_EXPECTED_LPAREN: return "zir.parse.expected_lparen";
        case ZIR_PARSE_EXPECTED_RPAREN: return "zir.parse.expected_rparen";
        case ZIR_PARSE_EXPECTED_IDENTIFIER: return "zir.parse.expected_identifier";
        case ZIR_PARSE_TOO_MANY_PARAMS: return "zir.parse.too_many_params";
        case ZIR_PARSE_TOO_MANY_BLOCKS: return "zir.parse.too_many_blocks";
        case ZIR_PARSE_TOO_MANY_INSTRUCTIONS: return "zir.parse.too_many_instructions";
        case ZIR_PARSE_TOO_MANY_FUNCTIONS: return "zir.parse.too_many_functions";
        case ZIR_PARSE_INVALID_TERMINATOR: return "zir.parse.invalid_terminator";
        case ZIR_PARSE_UNKNOWN_LINE: return "zir.parse.unknown_line";
        case ZIR_PARSE_OUT_OF_MEMORY: return "zir.parse.out_of_memory";
        default: return "zir.parse.unknown";
    }
}

void zir_parse_result_init(zir_parse_result *result) {
    memset(result, 0, sizeof(*result));
    result->ok = 1;
    result->code = ZIR_PARSE_OK;
    result->line_number = 0;
}

void zir_parse_result_dispose(zir_parse_result *result) {
    (void)result;
}

static int zir_is_blank(const char *s) {
    while (*s) {
        if (!isspace((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

static char *zir_trim_inplace(char *s) {
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    {
        char *end = s + strlen(s) - 1;
        while (end > s && isspace((unsigned char)*end)) end--;
        end[1] = '\0';
    }
    return s;
}

static const char *zir_skip_trim(const char *s) {
    while (isspace((unsigned char)*s)) s++;
    return s;
}

static char *zir_strdup(const char *s) {
    if (s == NULL) return strdup("");
    return strdup(s);
}

static int zir_starts_with(const char *s, const char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

static int zir_parse_type_at(const char *s, char *dest, size_t dest_capacity) {
    size_t i = 0;
    size_t out = 0;

    while (isspace((unsigned char)s[i])) i++;

    while (s[i] && out + 1 < dest_capacity) {
        if (s[i] == '<') {
            dest[out++] = '<';
            i++;
            continue;
        }
        if (s[i] == '>') {
            dest[out++] = '>';
            i++;
            if (isspace((unsigned char)s[i]) || s[i] == '\0' || s[i] == ',' || s[i] == ')') {
                break;
            }
            continue;
        }
        if (s[i] == ',') {
            dest[out++] = ',';
            i++;
            while (isspace((unsigned char)s[i])) {
                if (out + 1 < dest_capacity) dest[out++] = ' ';
                i++;
            }
            continue;
        }
        if (isspace((unsigned char)s[i]) && !isalnum((unsigned char)s[i + 1]) && s[i + 1] != '<') {
            break;
        }
        if (isspace((unsigned char)s[i]) && s[i + 1] == '\0') {
            break;
        }
        if (s[i] == ')' || s[i] == ',') {
            break;
        }
        dest[out++] = s[i++];
    }

    dest[out] = '\0';
    return (int)i;
}

static int zir_find_colon_for_assign(const char *line) {
    int depth_angle = 0;
    int depth_paren = 0;
    int depth_bracket = 0;
    size_t i = 0;

    while (line[i]) {
        char ch = line[i];
        if (ch == '<') depth_angle++;
        else if (ch == '>') depth_angle--;
        else if (ch == '(') depth_paren++;
        else if (ch == ')') depth_paren--;
        else if (ch == '[') depth_bracket++;
        else if (ch == ']') depth_bracket--;

        if (ch == ':' && depth_angle == 0 && depth_paren == 0 && depth_bracket == 0) {
            int next_is_colon = (line[i + 1] == ':');
            int prev_is_alpha = (i > 0 && (isalnum((unsigned char)line[i - 1]) || line[i - 1] == '>' || line[i - 1] == '_'));

            if (prev_is_alpha && !next_is_colon) {
                return (int)i;
            }
        }
        i++;
    }
    return -1;
}

static int zir_find_equals_after_type(const char *line, int colon_pos) {
    size_t i = (size_t)colon_pos + 1;
    int depth_angle = 0;

    while (isspace((unsigned char)line[i])) i++;

    while (line[i]) {
        if (line[i] == '<') depth_angle++;
        else if (line[i] == '>') {
            depth_angle--;
            if (depth_angle < 0) break;
        } else if (line[i] == '=' && depth_angle == 0) {
            return (int)i;
        }
        i++;
    }
    return -1;
}

static int zir_parse_params(const char *text, zir_param *params, size_t *param_count, size_t max_params, zir_parse_result *result) {
    size_t count = 0;
    const char *p = text;
    int depth = 0;

    while (*p) {
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0') break;

        if (*p == ')') break;
        if (*p == ',') { p++; continue; }

        if (count >= max_params) {
            zir_parse_set_error(result, ZIR_PARSE_TOO_MANY_PARAMS, 0, "too many parameters");
            return 0;
        }

        const char *name_start = p;
        while (*p && *p != ':' && *p != ',' && *p != ')') p++;
        if (*p != ':') {
            zir_parse_set_error(result, ZIR_PARSE_EXPECTED_COLON, 0, "expected ':' after parameter name");
            return 0;
        }

        char name_buf[128];
        size_t name_len = (size_t)(p - name_start);
        {
            while (name_len > 0 && isspace((unsigned char)name_start[name_len - 1])) name_len--;
        }
        if (name_len >= sizeof(name_buf)) name_len = sizeof(name_buf) - 1;
        memcpy(name_buf, name_start, name_len);
        name_buf[name_len] = '\0';

        p++;
        while (isspace((unsigned char)*p)) p++;

        const char *type_start = p;
        depth = 0;
        while (*p) {
            if (*p == '<') depth++;
            else if (*p == '>') { depth--; if (depth < 0) depth = 0; }
            else if ((*p == ',' || *p == ')') && depth == 0) break;
            p++;
        }

        char type_buf[128];
        size_t type_len = (size_t)(p - type_start);
        while (type_len > 0 && isspace((unsigned char)type_start[type_len - 1])) type_len--;
        if (type_len >= sizeof(type_buf)) type_len = sizeof(type_buf) - 1;
        memcpy(type_buf, type_start, type_len);
        type_buf[type_len] = '\0';

        params[count] = zir_make_param(zir_strdup(name_buf), zir_strdup(type_buf), NULL);
        count++;

        if (*p == ',') p++;
    }

    *param_count = count;
    return 1;
}

static int zir_starts_with_keyword(const char *line, const char *keyword) {
    size_t len = strlen(keyword);
    if (strncmp(line, keyword, len) != 0) return 0;
    if (len == 0) return 1;
    if (keyword[len - 1] == ' ') return 1;
    return line[len] == '\0' || isspace((unsigned char)line[len]);
}

static int zir_is_terminator_line(const char *trimmed) {
    return zir_starts_with_keyword(trimmed, "return ") ||
           zir_starts_with_keyword(trimmed, "return\0") ||
           strcmp(trimmed, "return") == 0 ||
           zir_starts_with_keyword(trimmed, "jump ") ||
           zir_starts_with_keyword(trimmed, "branch_if ") ||
           zir_starts_with_keyword(trimmed, "panic ") ||
           strcmp(trimmed, "panic") == 0 ||
           strcmp(trimmed, "unreachable") == 0;
}

static zir_terminator zir_parse_terminator_text(const char *trimmed) {
    if (zir_starts_with_keyword(trimmed, "return")) {
        const char *value = trimmed + 6;
        while (isspace((unsigned char)*value)) value++;
        if (*value == '\0') return zir_make_return_terminator(zir_strdup(""));
        return zir_make_return_terminator(zir_strdup(value));
    }

    if (zir_starts_with_keyword(trimmed, "jump ")) {
        const char *label = trimmed + 5;
        while (isspace((unsigned char)*label)) label++;
        {
            char label_buf[128];
            size_t i = 0;
            while (*label && !isspace((unsigned char)*label) && i + 1 < sizeof(label_buf)) {
                label_buf[i++] = *label++;
            }
            label_buf[i] = '\0';
            return zir_make_jump_terminator(zir_strdup(label_buf));
        }
    }

    if (zir_starts_with_keyword(trimmed, "branch_if ")) {
        const char *p = trimmed + 10;
        char cond_buf[128] = {0};
        char then_buf[128] = {0};
        char else_buf[128] = {0};
        size_t i;

        while (isspace((unsigned char)*p)) p++;
        i = 0;
        while (*p && *p != ',' && i + 1 < sizeof(cond_buf)) cond_buf[i++] = *p++;
        cond_buf[i] = '\0';
        if (*p == ',') p++;
        while (isspace((unsigned char)*p)) p++;

        i = 0;
        while (*p && *p != ',' && i + 1 < sizeof(then_buf)) then_buf[i++] = *p++;
        then_buf[i] = '\0';
        if (*p == ',') p++;
        while (isspace((unsigned char)*p)) p++;

        i = 0;
        while (*p && !isspace((unsigned char)*p) && i + 1 < sizeof(else_buf)) else_buf[i++] = *p++;
        else_buf[i] = '\0';

        return zir_make_branch_if_terminator(zir_strdup(cond_buf), zir_strdup(then_buf), zir_strdup(else_buf));
    }

    if (zir_starts_with_keyword(trimmed, "panic")) {
        const char *msg = trimmed + 5;
        while (isspace((unsigned char)*msg)) msg++;
        if (*msg == '"') {
            msg++;
            size_t len = strlen(msg);
            if (len > 0 && msg[len - 1] == '"') {
                char *msg_copy = zir_strdup(msg);
                msg_copy[len - 1] = '\0';
                return zir_make_panic_terminator(msg_copy);
            }
        }
        return zir_make_panic_terminator(zir_strdup(msg));
    }

    if (strcmp(trimmed, "unreachable") == 0) {
        return zir_make_unreachable_terminator();
    }

    return zir_make_return_terminator(zir_strdup(""));
}

int zir_parse_module(const char *text, size_t length, zir_parse_result *result) {
    char line_buf[ZIR_PARSER_MAX_LINE];
    size_t pos = 0;
    int line_number = 0;
    size_t total_instructions = 0;
    size_t total_params = 0;

    zir_parse_result_init(result);

    if (text == NULL || length == 0) {
        zir_parse_set_error(result, ZIR_PARSE_INVALID_INPUT, 0, "input text is empty");
        return 0;
    }

    while (pos < length) {
        size_t line_start = pos;
        size_t line_len = 0;

        while (pos < length && text[pos] != '\n') {
            pos++;
            line_len++;
        }
        if (pos < length) pos++;

        if (line_len >= sizeof(line_buf)) line_len = sizeof(line_buf) - 1;
        memcpy(line_buf, text + line_start, line_len);
        line_buf[line_len] = '\0';
        while (line_len > 0 && line_buf[line_len - 1] == '\r') {
            line_buf[line_len - 1] = '\0';
            line_len--;
        }
        line_number++;

        {
            char *trimmed = zir_trim_inplace(line_buf);
            if (zir_is_blank(trimmed) || zir_starts_with(trimmed, "#")) continue;

            if (zir_starts_with_keyword(trimmed, "module ")) {
                const char *name = trimmed + 7;
                while (isspace((unsigned char)*name)) name++;
                result->module.name = zir_strdup(name);
                continue;
            }

            if (zir_starts_with_keyword(trimmed, "func ")) {
                if (result->module.function_count >= ZIR_PARSER_MAX_FUNCTIONS) {
                    zir_parse_set_error(result, ZIR_PARSE_TOO_MANY_FUNCTIONS, line_number, "too many functions");
                    return 0;
                }

                const char *p = trimmed + 5;
                while (isspace((unsigned char)*p)) p++;

                char func_name[128];
                size_t ni = 0;
                while (*p && *p != '(' && ni + 1 < sizeof(func_name)) func_name[ni++] = *p++;
                func_name[ni] = '\0';

                if (*p != '(') {
                    zir_parse_set_error(result, ZIR_PARSE_EXPECTED_LPAREN, line_number, "expected '(' after function name");
                    return 0;
                }
                p++;

                const char *paren_start = p;
                int depth = 1;
                while (*p && depth > 0) {
                    if (*p == '(') depth++;
                    else if (*p == ')') depth--;
                    p++;
                }

                char params_text[1024];
                size_t plen = (size_t)(p - paren_start - 1);
                if (plen >= sizeof(params_text)) plen = sizeof(params_text) - 1;
                memcpy(params_text, paren_start, plen);
                params_text[plen] = '\0';

zir_param *param_ptr = &result->param_storage[total_params];
                size_t param_count = 0;
                if (!zir_parse_params(params_text, param_ptr, &param_count, ZIR_PARSER_MAX_PARAMS - total_params, result)) {
                    return 0;
                }
                total_params += param_count;

                const char *arrow = strstr(p, "->");
                if (arrow == NULL) {
                    zir_parse_set_error(result, ZIR_PARSE_EXPECTED_ARROW, line_number, "expected '->' before return type");
                    return 0;
                }
                p = arrow + 2;
                while (isspace((unsigned char)*p)) p++;

                char return_type[128];
                int rt_len = zir_parse_type_at(p, return_type, sizeof(return_type));
                (void)rt_len;

                result->function_storage[result->module.function_count].name = zir_strdup(func_name);
                result->function_storage[result->module.function_count].params = param_ptr;
                result->function_storage[result->module.function_count].param_count = param_count;
                result->function_storage[result->module.function_count].return_type = zir_strdup(return_type);
                result->function_storage[result->module.function_count].blocks = (const zir_block *)&result->block_storage[result->module.function_count * ZIR_PARSER_MAX_BLOCKS];
                result->function_storage[result->module.function_count].block_count = 0;
                result->module.function_count++;
                continue;
            }

            if (zir_starts_with_keyword(trimmed, "block ")) {
                if (result->module.function_count == 0) {
                    zir_parse_set_error(result, ZIR_PARSE_EXPECTED_FUNC, line_number, "block outside function");
                    return 0;
                }

                zir_function *current_func = &result->function_storage[result->module.function_count - 1];
                if (current_func->block_count >= ZIR_PARSER_MAX_BLOCKS) {
                    zir_parse_set_error(result, ZIR_PARSE_TOO_MANY_BLOCKS, line_number, "too many blocks");
                    return 0;
                }

                const char *label_start = trimmed + 6;
                while (isspace((unsigned char)*label_start)) label_start++;
                char label_buf[128];
                size_t li = 0;
                while (*label_start && *label_start != ':') {
                    if (li + 1 < sizeof(label_buf)) label_buf[li++] = *label_start;
                    label_start++;
                }
                label_buf[li] = '\0';

                zir_block *current_block = (zir_block *)&current_func->blocks[current_func->block_count];
                current_block->label = zir_strdup(label_buf);
                current_block->instructions = &result->instruction_storage[total_instructions];
                current_block->instruction_count = 0;
                current_func->block_count++;
                continue;
            }

            if (zir_is_terminator_line(trimmed)) {
                if (result->module.function_count == 0) continue;

                zir_function *current_func = &result->function_storage[result->module.function_count - 1];
                if (current_func->block_count == 0) continue;

                zir_block *current_block = (zir_block *)&current_func->blocks[current_func->block_count - 1];
                current_block->terminator = zir_parse_terminator_text(trimmed);
                continue;
            }

            {
                int colon_pos = zir_find_colon_for_assign(trimmed);
                if (colon_pos >= 0) {
                    int equals_pos = zir_find_equals_after_type(trimmed, colon_pos);

                    if (equals_pos >= 0) {
                        char dest_buf[128];
                        char type_buf[128];
                        size_t di = 0;

                        {
                            const char *d = trimmed;
                            while (d < trimmed + colon_pos && di + 1 < sizeof(dest_buf)) {
                                dest_buf[di++] = *d++;
                            }
                            dest_buf[di] = '\0';
                        }

                        {
                            const char *t = trimmed + colon_pos + 1;
                            while (isspace((unsigned char)*t)) t++;
                            int ti = zir_parse_type_at(t, type_buf, sizeof(type_buf));
                            const char *after_type = t + ti;
                            while (isspace((unsigned char)*after_type)) after_type++;

                            const char *expr_start = trimmed + equals_pos + 1;
                            while (isspace((unsigned char)*expr_start)) expr_start++;

                            if (result->module.function_count == 0) continue;
                            zir_function *current_func = &result->function_storage[result->module.function_count - 1];
                            if (current_func->block_count == 0) continue;
                            zir_block *current_block = (zir_block *)&current_func->blocks[current_func->block_count - 1];

                            if (current_block->instruction_count >= ZIR_PARSER_MAX_INSTRUCTIONS || total_instructions >= sizeof(result->instruction_storage) / sizeof(result->instruction_storage[0])) {
                                zir_parse_set_error(result, ZIR_PARSE_TOO_MANY_INSTRUCTIONS, line_number, "too many instructions");
                                return 0;
                            }

                            result->instruction_storage[total_instructions] = zir_make_assign_instruction(
                                zir_strdup(zir_trim_inplace(dest_buf)),
                                zir_strdup(zir_trim_inplace(type_buf)),
                                zir_strdup(zir_skip_trim(expr_start))
                            );
                            current_block->instruction_count++;
                            total_instructions++;
                            continue;
                        }
                    }
                }

                {
                    if (result->module.function_count == 0) continue;
                    zir_function *current_func = &result->function_storage[result->module.function_count - 1];
                    if (current_func->block_count == 0) continue;
                    zir_block *current_block = (zir_block *)&current_func->blocks[current_func->block_count - 1];

                    if (total_instructions >= sizeof(result->instruction_storage) / sizeof(result->instruction_storage[0])) {
                        zir_parse_set_error(result, ZIR_PARSE_TOO_MANY_INSTRUCTIONS, line_number, "too many instructions");
                        return 0;
                    }

                    result->instruction_storage[total_instructions] = zir_make_effect_instruction(zir_strdup(trimmed));
                    current_block->instruction_count++;
                    total_instructions++;
                    continue;
                }
            }
        }
    }

    result->module.functions = result->function_storage;
    return result->ok;
}
