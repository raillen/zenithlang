#include "compiler/frontend/lexer/token.h"

const char *zt_token_kind_name(zt_token_kind kind) {
    switch (kind) {
        case ZT_TOKEN_EOF: return "eof";
        case ZT_TOKEN_NAMESPACE: return "namespace";
        case ZT_TOKEN_IMPORT: return "import";
        case ZT_TOKEN_AS: return "as";
        case ZT_TOKEN_FUNC: return "func";
        case ZT_TOKEN_END: return "end";
        case ZT_TOKEN_CONST: return "const";
        case ZT_TOKEN_VAR: return "var";
        case ZT_TOKEN_RETURN: return "return";
        case ZT_TOKEN_IF: return "if";
        case ZT_TOKEN_ELSE: return "else";
        case ZT_TOKEN_WHILE: return "while";
        case ZT_TOKEN_FOR: return "for";
        case ZT_TOKEN_IN: return "in";
        case ZT_TOKEN_REPEAT: return "repeat";
        case ZT_TOKEN_TIMES: return "times";
        case ZT_TOKEN_BREAK: return "break";
        case ZT_TOKEN_CONTINUE: return "continue";
        case ZT_TOKEN_STRUCT: return "struct";
        case ZT_TOKEN_TRAIT: return "trait";
        case ZT_TOKEN_APPLY: return "apply";
        case ZT_TOKEN_TO: return "to";
        case ZT_TOKEN_ENUM: return "enum";
        case ZT_TOKEN_MATCH: return "match";
        case ZT_TOKEN_CASE: return "case";
        case ZT_TOKEN_DEFAULT: return "default";
        case ZT_TOKEN_PUBLIC: return "public";
        case ZT_TOKEN_ATTR: return "attr";
        case ZT_TOKEN_WHERE: return "where";
        case ZT_TOKEN_IS: return "is";
        case ZT_TOKEN_AND: return "and";
        case ZT_TOKEN_OR: return "or";
        case ZT_TOKEN_NOT: return "not";
        case ZT_TOKEN_TRUE: return "true";
        case ZT_TOKEN_FALSE: return "false";
        case ZT_TOKEN_NONE: return "none";
        case ZT_TOKEN_SUCCESS: return "success";
        case ZT_TOKEN_KW_ERROR: return "error";
        case ZT_TOKEN_OPTIONAL: return "optional";
        case ZT_TOKEN_RESULT: return "result";
        case ZT_TOKEN_LIST: return "list";
        case ZT_TOKEN_MAP: return "map";
        case ZT_TOKEN_GRID2D: return "grid2d";
        case ZT_TOKEN_PQUEUE: return "pqueue";
        case ZT_TOKEN_CIRCBUF: return "circbuf";
        case ZT_TOKEN_BTREEMAP: return "btreemap";
        case ZT_TOKEN_BTREESET: return "btreeset";
        case ZT_TOKEN_GRID3D: return "grid3d";
        case ZT_TOKEN_EXTERN: return "extern";
        case ZT_TOKEN_DYN: return "dyn";
        case ZT_TOKEN_VOID: return "void";
        case ZT_TOKEN_MUT: return "mut";
        case ZT_TOKEN_SELF: return "self";
        case ZT_TOKEN_IDENTIFIER: return "identifier";
        case ZT_TOKEN_INT_LITERAL: return "int_literal";
        case ZT_TOKEN_FLOAT_LITERAL: return "float_literal";
        case ZT_TOKEN_STRING_LITERAL: return "string_literal";
        case ZT_TOKEN_TRIPLE_QUOTED_TEXT: return "triple_quoted_text";
        case ZT_TOKEN_PLUS: return "+";
        case ZT_TOKEN_MINUS: return "-";
        case ZT_TOKEN_STAR: return "*";
        case ZT_TOKEN_SLASH: return "/";
        case ZT_TOKEN_PERCENT: return "%";
        case ZT_TOKEN_EQ: return "=";
        case ZT_TOKEN_EQEQ: return "==";
        case ZT_TOKEN_NEQ: return "!=";
        case ZT_TOKEN_LT: return "<";
        case ZT_TOKEN_LTE: return "<=";
        case ZT_TOKEN_GT: return ">";
        case ZT_TOKEN_GTE: return ">=";
        case ZT_TOKEN_ARROW: return "->";
        case ZT_TOKEN_DOT: return ".";
        case ZT_TOKEN_DOTDOT: return "..";
        case ZT_TOKEN_COMMA: return ",";
        case ZT_TOKEN_COLON: return ":";
        case ZT_TOKEN_QUESTION: return "?";
        case ZT_TOKEN_ASSIGN: return ":=";
        case ZT_TOKEN_BANG: return "!";
        case ZT_TOKEN_LPAREN: return "(";
        case ZT_TOKEN_RPAREN: return ")";
        case ZT_TOKEN_LBRACKET: return "[";
        case ZT_TOKEN_RBRACKET: return "]";
        case ZT_TOKEN_LBRACE: return "{";
        case ZT_TOKEN_RBRACE: return "}";
        case ZT_TOKEN_COMMENT: return "comment";
        case ZT_TOKEN_LEX_ERROR: return "lex_error";
        default: return "unknown";
    }
}

int zt_token_kind_is_keyword(zt_token_kind kind) {
    return kind >= ZT_TOKEN_NAMESPACE && kind <= ZT_TOKEN_SELF;
}

int zt_token_kind_is_literal(zt_token_kind kind) {
    return kind >= ZT_TOKEN_IDENTIFIER && kind <= ZT_TOKEN_TRIPLE_QUOTED_TEXT;
}

zt_source_span zt_source_span_make(const char *source_name, size_t line, size_t column_start, size_t column_end) {
    zt_source_span span;
    span.source_name = source_name;
    span.line = line;
    span.column_start = column_start;
    span.column_end = column_end;
    return span;
}

zt_source_span zt_source_span_unknown(void) {
    zt_source_span span;
    span.source_name = NULL;
    span.line = 0;
    span.column_start = 0;
    span.column_end = 0;
    return span;
}

