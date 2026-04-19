#include "compiler/frontend/lexer/lexer.h"
#include "compiler/semantic/diagnostics/diagnostics.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct zt_keyword_entry {
    const char *text;
    zt_token_kind kind;
} zt_keyword_entry;

static zt_keyword_entry zt_keywords[] = {
    {"namespace", ZT_TOKEN_NAMESPACE},
    {"import", ZT_TOKEN_IMPORT},
    {"as", ZT_TOKEN_AS},
    {"func", ZT_TOKEN_FUNC},
    {"end", ZT_TOKEN_END},
    {"const", ZT_TOKEN_CONST},
    {"var", ZT_TOKEN_VAR},
    {"return", ZT_TOKEN_RETURN},
    {"if", ZT_TOKEN_IF},
    {"else", ZT_TOKEN_ELSE},
    {"while", ZT_TOKEN_WHILE},
    {"for", ZT_TOKEN_FOR},
    {"in", ZT_TOKEN_IN},
    {"repeat", ZT_TOKEN_REPEAT},
    {"times", ZT_TOKEN_TIMES},
    {"break", ZT_TOKEN_BREAK},
    {"continue", ZT_TOKEN_CONTINUE},
    {"struct", ZT_TOKEN_STRUCT},
    {"trait", ZT_TOKEN_TRAIT},
    {"apply", ZT_TOKEN_APPLY},
    {"to", ZT_TOKEN_TO},
    {"enum", ZT_TOKEN_ENUM},
    {"match", ZT_TOKEN_MATCH},
    {"case", ZT_TOKEN_CASE},
    {"default", ZT_TOKEN_DEFAULT},
    {"public", ZT_TOKEN_PUBLIC},
    {"where", ZT_TOKEN_WHERE},
    {"is", ZT_TOKEN_IS},
    {"and", ZT_TOKEN_AND},
    {"or", ZT_TOKEN_OR},
    {"not", ZT_TOKEN_NOT},
    {"true", ZT_TOKEN_TRUE},
    {"false", ZT_TOKEN_FALSE},
    {"none", ZT_TOKEN_NONE},
    {"success", ZT_TOKEN_SUCCESS},
    {"error", ZT_TOKEN_KW_ERROR},
    {"optional", ZT_TOKEN_OPTIONAL},
    {"result", ZT_TOKEN_RESULT},
    {"list", ZT_TOKEN_LIST},
    {"map", ZT_TOKEN_MAP},
    {"extern", ZT_TOKEN_EXTERN},
    {"void", ZT_TOKEN_VOID},
    {"mut", ZT_TOKEN_MUT},
    {"self", ZT_TOKEN_SELF},
    {NULL, ZT_TOKEN_EOF}
};

struct zt_lexer {
    const char *source_name;
    const char *source_text;
    size_t source_length;
    size_t position;
    size_t line;
    size_t column;
    zt_diag_list *diagnostics;  /* Optional: diagnostics list for errors */
};

zt_lexer *zt_lexer_make(const char *source_name, const char *source_text, size_t source_length) {
    zt_lexer *lexer = (zt_lexer *)calloc(1, sizeof(zt_lexer));
    if (lexer == NULL) return NULL;

    lexer->source_name = source_name;
    lexer->source_text = source_text;
    lexer->source_length = source_length;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->diagnostics = NULL;  /* No diagnostics by default */

    return lexer;
}

void zt_lexer_set_diagnostics(zt_lexer *lexer, zt_diag_list *diagnostics) {
    if (lexer != NULL) {
        lexer->diagnostics = diagnostics;
    }
}

void zt_lexer_dispose(zt_lexer *lexer) {
    if (lexer != NULL) {
        free(lexer);
    }
}

static char zt_lexer_peek(const zt_lexer *lexer) {
    if (lexer->position >= lexer->source_length) return '\0';
    return lexer->source_text[lexer->position];
}

static char zt_lexer_peek_next(const zt_lexer *lexer) {
    if (lexer->position + 1 >= lexer->source_length) return '\0';
    return lexer->source_text[lexer->position + 1];
}

static char zt_lexer_advance(zt_lexer *lexer) {
    char ch;

    if (lexer->position >= lexer->source_length) return '\0';

    ch = lexer->source_text[lexer->position];
    lexer->position += 1;

    if (ch == '\n') {
        lexer->line += 1;
        lexer->column = 1;
    } else {
        lexer->column += 1;
    }

    return ch;
}

static void zt_lexer_skip_trivia(zt_lexer *lexer) {
    while (lexer->position < lexer->source_length) {
        char ch = zt_lexer_peek(lexer);

        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
            zt_lexer_advance(lexer);
            continue;
        }

        break;
    }
}

static zt_token_kind zt_lexer_lookup_keyword(const char *text, size_t length) {
    size_t i;
    for (i = 0; zt_keywords[i].text != NULL; i++) {
        size_t kw_len = strlen(zt_keywords[i].text);
        if (kw_len == length && strncmp(zt_keywords[i].text, text, length) == 0) {
            return zt_keywords[i].kind;
        }
    }
    return ZT_TOKEN_IDENTIFIER;
}

static zt_token zt_lexer_make_token(zt_lexer *lexer, zt_token_kind kind, size_t start_pos, size_t start_line, size_t start_column, const char *text, size_t text_len) {
    zt_token token;
    token.kind = kind;
    token.span = zt_source_span_make(lexer->source_name, start_line, start_column, lexer->column);
    token.text = text;
    token.length = text_len;
    (void)start_pos;
    return token;
}

static zt_token zt_lexer_read_string(zt_lexer *lexer, size_t start_line, size_t start_column) {
    size_t start_pos = lexer->position;
    zt_lexer_advance(lexer);

    while (lexer->position < lexer->source_length) {
        char ch = zt_lexer_peek(lexer);

        if (ch == '"') {
            zt_lexer_advance(lexer);
            return zt_lexer_make_token(lexer, ZT_TOKEN_STRING_LITERAL, start_pos, start_line, start_column, lexer->source_text + start_pos, lexer->position - start_pos);
        }

        if (ch == '\0' || ch == '\n') {
            break;
        }

        if (ch == '\\' && lexer->position + 1 < lexer->source_length) {
            char next_ch = lexer->source_text[lexer->position + 1];
            if (next_ch == '{') {
                zt_lexer_advance(lexer);
                zt_lexer_advance(lexer);
                return zt_lexer_make_token(lexer, ZT_TOKEN_STRING_PART, start_pos, start_line, start_column, lexer->source_text + start_pos, lexer->position - start_pos);
            }
            zt_lexer_advance(lexer);
        }

        zt_lexer_advance(lexer);
    }
    
    return zt_lexer_make_token(lexer, ZT_TOKEN_STRING_LITERAL, start_pos, start_line, start_column, lexer->source_text + start_pos, lexer->position - start_pos);
}

zt_token zt_lexer_resume_string(zt_lexer *lexer) {
    size_t start_line = lexer->line;
    size_t start_column = lexer->column;
    size_t start_pos = lexer->position;

    while (lexer->position < lexer->source_length) {
        char ch = zt_lexer_peek(lexer);

        if (ch == '"') {
            zt_lexer_advance(lexer);
            return zt_lexer_make_token(lexer, ZT_TOKEN_STRING_END, start_pos, start_line, start_column, lexer->source_text + start_pos, lexer->position - start_pos);
        }

        if (ch == '\0' || ch == '\n') {
            break;
        }

        if (ch == '\\' && lexer->position + 1 < lexer->source_length) {
            char next_ch = lexer->source_text[lexer->position + 1];
            if (next_ch == '{') {
                zt_lexer_advance(lexer);
                zt_lexer_advance(lexer);
                return zt_lexer_make_token(lexer, ZT_TOKEN_STRING_PART, start_pos, start_line, start_column, lexer->source_text + start_pos, lexer->position - start_pos);
            }
            zt_lexer_advance(lexer);
        }

        zt_lexer_advance(lexer);
    }
    
    return zt_lexer_make_token(lexer, ZT_TOKEN_STRING_END, start_pos, start_line, start_column, lexer->source_text + start_pos, lexer->position - start_pos);
}

static zt_token zt_lexer_read_triple_quoted(zt_lexer *lexer, size_t start_line, size_t start_column) {
    size_t start_pos = lexer->position;

    zt_lexer_advance(lexer);
    zt_lexer_advance(lexer);
    zt_lexer_advance(lexer);

    while (lexer->position < lexer->source_length) {
        if (zt_lexer_peek(lexer) == '"' &&
            lexer->position + 1 < lexer->source_length &&
            lexer->source_text[lexer->position + 1] == '"' &&
            lexer->position + 2 < lexer->source_length &&
            lexer->source_text[lexer->position + 2] == '"') {
            zt_lexer_advance(lexer);
            zt_lexer_advance(lexer);
            zt_lexer_advance(lexer);
            break;
        }

        zt_lexer_advance(lexer);
    }

    return zt_lexer_make_token(lexer, ZT_TOKEN_TRIPLE_QUOTED_TEXT, start_pos, start_line, start_column, lexer->source_text + start_pos, lexer->position - start_pos);
}

static zt_token zt_lexer_read_number(zt_lexer *lexer, size_t start_line, size_t start_column) {
    size_t start_pos = lexer->position;
    int is_float = 0;
    int is_hex = 0;
    int is_binary = 0;
    (void)is_hex;
    (void)is_binary;

    if (zt_lexer_peek(lexer) == '0' && zt_lexer_peek_next(lexer) == 'x') {
        is_hex = 1;
        zt_lexer_advance(lexer);
        zt_lexer_advance(lexer);

        while (lexer->position < lexer->source_length) {
            char ch = zt_lexer_peek(lexer);
            if (isxdigit((unsigned char)ch) || ch == '_') {
                zt_lexer_advance(lexer);
            } else {
                break;
            }
        }
    } else if (zt_lexer_peek(lexer) == '0' && zt_lexer_peek_next(lexer) == 'b') {
        is_binary = 1;
        zt_lexer_advance(lexer);
        zt_lexer_advance(lexer);

        while (lexer->position < lexer->source_length) {
            char ch = zt_lexer_peek(lexer);
            if (ch == '0' || ch == '1' || ch == '_') {
                zt_lexer_advance(lexer);
            } else {
                break;
            }
        }
    } else {
        while (lexer->position < lexer->source_length) {
            char ch = zt_lexer_peek(lexer);
            if (isdigit((unsigned char)ch) || ch == '_') {
                zt_lexer_advance(lexer);
            } else {
                break;
            }
        }

        if (zt_lexer_peek(lexer) == '.' && zt_lexer_peek_next(lexer) != '.') {
            is_float = 1;
            zt_lexer_advance(lexer);

            while (lexer->position < lexer->source_length) {
                char ch = zt_lexer_peek(lexer);
                if (isdigit((unsigned char)ch) || ch == '_') {
                    zt_lexer_advance(lexer);
                } else {
                    break;
                }
            }
        }

        if (zt_lexer_peek(lexer) == 'e' || zt_lexer_peek(lexer) == 'E') {
            is_float = 1;
            zt_lexer_advance(lexer);
            if (zt_lexer_peek(lexer) == '+' || zt_lexer_peek(lexer) == '-') {
                zt_lexer_advance(lexer);
            }
            while (lexer->position < lexer->source_length) {
                char ch = zt_lexer_peek(lexer);
                if (isdigit((unsigned char)ch) || ch == '_') {
                    zt_lexer_advance(lexer);
                } else {
                    break;
                }
            }
        }
    }

    return zt_lexer_make_token(lexer, is_float ? ZT_TOKEN_FLOAT_LITERAL : ZT_TOKEN_INT_LITERAL, start_pos, start_line, start_column, lexer->source_text + start_pos, lexer->position - start_pos);
}

static zt_token zt_lexer_read_identifier(zt_lexer *lexer, size_t start_line, size_t start_column) {
    size_t start_pos = lexer->position;

    while (lexer->position < lexer->source_length) {
        char ch = zt_lexer_peek(lexer);
        if (isalnum((unsigned char)ch) || ch == '_') {
            zt_lexer_advance(lexer);
        } else {
            break;
        }
    }

    zt_token_kind kind = zt_lexer_lookup_keyword(lexer->source_text + start_pos, lexer->position - start_pos);
    return zt_lexer_make_token(lexer, kind, start_pos, start_line, start_column, lexer->source_text + start_pos, lexer->position - start_pos);
}

zt_token zt_lexer_next_token(zt_lexer *lexer) {
    size_t start_line;
    size_t start_column;
    char ch;
    char next;

    if (lexer == NULL) {
        zt_token token;
        token.kind = ZT_TOKEN_EOF;
        token.span = zt_source_span_unknown();
        token.text = "";
        token.length = 0;
        return token;
    }

    zt_lexer_skip_trivia(lexer);

    if (lexer->position >= lexer->source_length) {
        zt_token token;
        token.kind = ZT_TOKEN_EOF;
        token.span = zt_source_span_make(lexer->source_name, lexer->line, lexer->column, lexer->column);
        token.text = "";
        token.length = 0;
        return token;
    }

    start_line = lexer->line;
    start_column = lexer->column;
    ch = zt_lexer_peek(lexer);

    if (ch == '"') {
        if (lexer->position + 2 < lexer->source_length &&
            lexer->source_text[lexer->position + 1] == '"' &&
            lexer->source_text[lexer->position + 2] == '"') {
            return zt_lexer_read_triple_quoted(lexer, start_line, start_column);
        }
        return zt_lexer_read_string(lexer, start_line, start_column);
    }

    if (isdigit((unsigned char)ch)) {
        return zt_lexer_read_number(lexer, start_line, start_column);
    }

    if (isalpha((unsigned char)ch) || ch == '_') {
        return zt_lexer_read_identifier(lexer, start_line, start_column);
    }

    zt_lexer_advance(lexer);
    next = zt_lexer_peek(lexer);

    switch (ch) {
        case '+': return zt_lexer_make_token(lexer, ZT_TOKEN_PLUS, 0, start_line, start_column, "+", 1);
        case '*': return zt_lexer_make_token(lexer, ZT_TOKEN_STAR, 0, start_line, start_column, "*", 1);
        case '/': return zt_lexer_make_token(lexer, ZT_TOKEN_SLASH, 0, start_line, start_column, "/", 1);
        case '%': return zt_lexer_make_token(lexer, ZT_TOKEN_PERCENT, 0, start_line, start_column, "%", 1);
        case '(': return zt_lexer_make_token(lexer, ZT_TOKEN_LPAREN, 0, start_line, start_column, "(", 1);
        case ')': return zt_lexer_make_token(lexer, ZT_TOKEN_RPAREN, 0, start_line, start_column, ")", 1);
        case '[': return zt_lexer_make_token(lexer, ZT_TOKEN_LBRACKET, 0, start_line, start_column, "[", 1);
        case ']': return zt_lexer_make_token(lexer, ZT_TOKEN_RBRACKET, 0, start_line, start_column, "]", 1);
        case '{': return zt_lexer_make_token(lexer, ZT_TOKEN_LBRACE, 0, start_line, start_column, "{", 1);
        case '}': return zt_lexer_make_token(lexer, ZT_TOKEN_RBRACE, 0, start_line, start_column, "}", 1);
        case ',': return zt_lexer_make_token(lexer, ZT_TOKEN_COMMA, 0, start_line, start_column, ",", 1);
        case '?': return zt_lexer_make_token(lexer, ZT_TOKEN_QUESTION, 0, start_line, start_column, "?", 1);

        case '-':
            if (next == '>') {
                zt_lexer_advance(lexer);
                return zt_lexer_make_token(lexer, ZT_TOKEN_ARROW, 0, start_line, start_column, "->", 2);
            }
            if (next == '-') {
                size_t start_pos = lexer->position - 1; // '-' was already advanced
                zt_lexer_advance(lexer); // second '-'
                if (zt_lexer_peek(lexer) == '-') {
                    zt_lexer_advance(lexer); // third '-'
                    while (lexer->position < lexer->source_length) {
                        if (zt_lexer_peek(lexer) == '-' &&
                            lexer->position + 1 < lexer->source_length &&
                            lexer->source_text[lexer->position + 1] == '-' &&
                            lexer->position + 2 < lexer->source_length &&
                            lexer->source_text[lexer->position + 2] == '-') {
                            zt_lexer_advance(lexer);
                            zt_lexer_advance(lexer);
                            zt_lexer_advance(lexer);
                            break;
                        }
                        zt_lexer_advance(lexer);
                    }
                } else {
                    while (lexer->position < lexer->source_length && zt_lexer_peek(lexer) != '\n') {
                        zt_lexer_advance(lexer);
                    }
                }
                return zt_lexer_make_token(lexer, ZT_TOKEN_COMMENT, start_pos, start_line, start_column, lexer->source_text + start_pos, lexer->position - start_pos);
            }
            return zt_lexer_make_token(lexer, ZT_TOKEN_MINUS, 0, start_line, start_column, "-", 1);

        case '.':
            if (next == '.') {
                zt_lexer_advance(lexer);
                return zt_lexer_make_token(lexer, ZT_TOKEN_DOTDOT, 0, start_line, start_column, "..", 2);
            }
            return zt_lexer_make_token(lexer, ZT_TOKEN_DOT, 0, start_line, start_column, ".", 1);

        case ':':
            return zt_lexer_make_token(lexer, ZT_TOKEN_COLON, 0, start_line, start_column, ":", 1);

        case '=':
            if (next == '=') {
                zt_lexer_advance(lexer);
                return zt_lexer_make_token(lexer, ZT_TOKEN_EQEQ, 0, start_line, start_column, "==", 2);
            }
            return zt_lexer_make_token(lexer, ZT_TOKEN_EQ, 0, start_line, start_column, "=", 1);

        case '!':
            if (next == '=') {
                zt_lexer_advance(lexer);
                return zt_lexer_make_token(lexer, ZT_TOKEN_NEQ, 0, start_line, start_column, "!=", 2);
            }
            return zt_lexer_make_token(lexer, ZT_TOKEN_BANG, 0, start_line, start_column, "!", 1);

        case '<':
            if (next == '=') {
                zt_lexer_advance(lexer);
                return zt_lexer_make_token(lexer, ZT_TOKEN_LTE, 0, start_line, start_column, "<=", 2);
            }
            return zt_lexer_make_token(lexer, ZT_TOKEN_LT, 0, start_line, start_column, "<", 1);

        case '>':
            if (next == '=') {
                zt_lexer_advance(lexer);
                return zt_lexer_make_token(lexer, ZT_TOKEN_GTE, 0, start_line, start_column, ">=", 2);
            }
            return zt_lexer_make_token(lexer, ZT_TOKEN_GT, 0, start_line, start_column, ">", 1);

        default: {
            char buf[2] = {ch, '\0'};
            return zt_lexer_make_token(lexer, ZT_TOKEN_LEX_ERROR, 0, start_line, start_column, buf, 1);
        }
    }
}

zt_source_span zt_lexer_current_span(const zt_lexer *lexer) {
    if (lexer == NULL) return zt_source_span_unknown();
    return zt_source_span_make(lexer->source_name, lexer->line, lexer->column, lexer->column);
}

int zt_lexer_is_at_end(const zt_lexer *lexer) {
    if (lexer == NULL) return 1;
    return lexer->position >= lexer->source_length;
}

