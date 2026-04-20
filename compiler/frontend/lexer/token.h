#ifndef ZENITH_NEXT_COMPILER_FRONTEND_LEXER_TOKEN_H
#define ZENITH_NEXT_COMPILER_FRONTEND_LEXER_TOKEN_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum zt_token_kind {
    ZT_TOKEN_EOF,

    ZT_TOKEN_NAMESPACE,
    ZT_TOKEN_IMPORT,
    ZT_TOKEN_AS,
    ZT_TOKEN_FUNC,
    ZT_TOKEN_END,
    ZT_TOKEN_CONST,
    ZT_TOKEN_VAR,
    ZT_TOKEN_RETURN,
    ZT_TOKEN_IF,
    ZT_TOKEN_ELSE,
    ZT_TOKEN_WHILE,
    ZT_TOKEN_FOR,
    ZT_TOKEN_IN,
    ZT_TOKEN_REPEAT,
    ZT_TOKEN_TIMES,
    ZT_TOKEN_BREAK,
    ZT_TOKEN_CONTINUE,
    ZT_TOKEN_STRUCT,
    ZT_TOKEN_TRAIT,
    ZT_TOKEN_APPLY,
    ZT_TOKEN_TO,
    ZT_TOKEN_ENUM,
    ZT_TOKEN_MATCH,
    ZT_TOKEN_CASE,
    ZT_TOKEN_DEFAULT,
    ZT_TOKEN_PUBLIC,
    ZT_TOKEN_ATTR,
    ZT_TOKEN_WHERE,
    ZT_TOKEN_IS,
    ZT_TOKEN_AND,
    ZT_TOKEN_OR,
    ZT_TOKEN_NOT,
    ZT_TOKEN_TRUE,
    ZT_TOKEN_FALSE,
    ZT_TOKEN_NONE,
    ZT_TOKEN_SUCCESS,
    ZT_TOKEN_KW_ERROR,
    ZT_TOKEN_OPTIONAL,
    ZT_TOKEN_RESULT,
    ZT_TOKEN_LIST,
    ZT_TOKEN_MAP,
    ZT_TOKEN_GRID2D,
    ZT_TOKEN_PQUEUE,
    ZT_TOKEN_CIRCBUF,
    ZT_TOKEN_BTREEMAP,
    ZT_TOKEN_BTREESET,
    ZT_TOKEN_GRID3D,
    ZT_TOKEN_EXTERN,
    ZT_TOKEN_DYN,
    ZT_TOKEN_VOID,
    ZT_TOKEN_MUT,
    ZT_TOKEN_SELF,

    ZT_TOKEN_IDENTIFIER,
    ZT_TOKEN_INT_LITERAL,
    ZT_TOKEN_FLOAT_LITERAL,
    ZT_TOKEN_STRING_LITERAL,
    ZT_TOKEN_STRING_PART,
    ZT_TOKEN_STRING_END,
    ZT_TOKEN_TRIPLE_QUOTED_TEXT,

    ZT_TOKEN_PLUS,
    ZT_TOKEN_MINUS,
    ZT_TOKEN_STAR,
    ZT_TOKEN_SLASH,
    ZT_TOKEN_PERCENT,
    ZT_TOKEN_EQ,
    ZT_TOKEN_EQEQ,
    ZT_TOKEN_NEQ,
    ZT_TOKEN_LT,
    ZT_TOKEN_LTE,
    ZT_TOKEN_GT,
    ZT_TOKEN_GTE,
    ZT_TOKEN_ARROW,
    ZT_TOKEN_DOT,
    ZT_TOKEN_DOTDOT,
    ZT_TOKEN_COMMA,
    ZT_TOKEN_COLON,
    ZT_TOKEN_SEMICOLON,
    ZT_TOKEN_QUESTION,
    ZT_TOKEN_ASSIGN,
    ZT_TOKEN_BANG,

    ZT_TOKEN_LPAREN,
    ZT_TOKEN_RPAREN,
    ZT_TOKEN_LBRACKET,
    ZT_TOKEN_RBRACKET,
    ZT_TOKEN_LBRACE,
    ZT_TOKEN_RBRACE,

    ZT_TOKEN_COMMENT,
    ZT_TOKEN_LEX_ERROR
} zt_token_kind;

typedef struct zt_source_span {
    const char *source_name;
    size_t line;
    size_t column_start;
    size_t column_end;
} zt_source_span;

typedef struct zt_token {
    zt_token_kind kind;
    zt_source_span span;
    const char *text;
    size_t length;
} zt_token;

const char *zt_token_kind_name(zt_token_kind kind);

int zt_token_kind_is_keyword(zt_token_kind kind);

int zt_token_kind_is_literal(zt_token_kind kind);

zt_source_span zt_source_span_make(const char *source_name, size_t line, size_t column_start, size_t column_end);

zt_source_span zt_source_span_unknown(void);

#ifdef __cplusplus
}
#endif

#endif
