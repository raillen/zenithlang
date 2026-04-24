#include "compiler/frontend/parser/parser.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct zt_parser {
    zt_lexer *lexer;
    zt_token current;
    zt_token peek;
    int has_peek;
    zt_parser_result *result;
    zt_arena *arena;
    zt_string_pool *pool;
    /* Context tracking for better error messages */
    int in_function_body;
    int in_type_position;
    int in_expression_position;
    zt_token *pending_comments;
    size_t pending_comment_count;
    size_t pending_comment_capacity;
    size_t nesting_depth;
} zt_parser;

#define ZT_PARSER_MAX_NESTING_DEPTH 512u

static zt_token zt_parser_next_non_comment_token(zt_parser *p) {
    zt_token token;
    while (1) {
        token = zt_lexer_next_token(p->lexer);
        if (token.kind == ZT_TOKEN_COMMENT) {
            if (p->pending_comment_count == p->pending_comment_capacity) {
                size_t new_cap = p->pending_comment_capacity == 0 ? 16 : p->pending_comment_capacity * 2;
                zt_token *new_comments = zt_arena_alloc(p->arena, new_cap * sizeof(zt_token));
                if (p->pending_comment_count > 0) {
                    memcpy(new_comments, p->pending_comments, p->pending_comment_count * sizeof(zt_token));
                }
                p->pending_comments = new_comments;
                p->pending_comment_capacity = new_cap;
            }
            p->pending_comments[p->pending_comment_count++] = token;
        } else {
            return token;
        }
    }
}

static zt_token zt_parser_advance(zt_parser *p) {
    zt_token prev = p->current;
    if (p->has_peek) {
        p->current = p->peek;
        p->has_peek = 0;
    } else {
        p->current = zt_parser_next_non_comment_token(p);
    }
    return prev;
}

static void zt_parser_fill_peek(zt_parser *p) {
    if (!p->has_peek) {
        p->peek = zt_parser_next_non_comment_token(p);
        p->has_peek = 1;
    }
}

static int zt_parser_check(zt_parser *p, zt_token_kind kind) {
    return p->current.kind == kind;
}

static int zt_parser_match(zt_parser *p, zt_token_kind kind) {
    if (zt_parser_check(p, kind)) {
        zt_parser_advance(p);
        return 1;
    }
    return 0;
}

static int zt_parser_is_declaration_start(zt_token_kind kind) {
    switch (kind) {
        case ZT_TOKEN_PUBLIC:
        case ZT_TOKEN_ATTR:
        case ZT_TOKEN_MUT:
        case ZT_TOKEN_FUNC:
        case ZT_TOKEN_STRUCT:
        case ZT_TOKEN_TRAIT:
        case ZT_TOKEN_APPLY:
        case ZT_TOKEN_ENUM:
        case ZT_TOKEN_EXTERN:
        case ZT_TOKEN_CONST:
        case ZT_TOKEN_VAR:
            return 1;
        default:
            return 0;
    }
}

static int zt_parser_is_member_start(zt_token_kind kind) {
    return kind == ZT_TOKEN_PUBLIC || kind == ZT_TOKEN_MUT || kind == ZT_TOKEN_FUNC;
}

static void zt_parser_sync_to_declaration(zt_parser *p) {
    while (!zt_parser_check(p, ZT_TOKEN_EOF) && !zt_parser_is_declaration_start(p->current.kind)) {
        zt_parser_advance(p);
    }
}

static void zt_parser_sync_to_member_or_end(zt_parser *p) {
    while (!zt_parser_check(p, ZT_TOKEN_END) &&
           !zt_parser_check(p, ZT_TOKEN_EOF) &&
           !zt_parser_is_member_start(p->current.kind)) {
        zt_parser_advance(p);
    }
}

static zt_token zt_parser_expect(zt_parser *p, zt_token_kind kind) {
    if (p->current.kind == kind) {
        return zt_parser_advance(p);
    }
    
    /* Provide helpful suggestions for common errors */
    const char *suggestion = NULL;
    if (kind == ZT_TOKEN_ARROW && p->current.kind == ZT_TOKEN_MINUS) {
        /* Check if next token might be '>' (user typed "- >" instead of "->") */
        zt_parser_fill_peek(p);
        if (p->peek.kind == ZT_TOKEN_GT) {
            suggestion = "use '->' for return type (no space allowed between '-' and '>')";
        }
    } else if (kind == ZT_TOKEN_IDENTIFIER && p->current.kind == ZT_TOKEN_GT) {
        suggestion = "unexpected '>', check for unclosed generic type '<...>'";
    } else if (kind == ZT_TOKEN_COLON && p->current.kind == ZT_TOKEN_SEMICOLON) {
        suggestion = "use ':' for type annotations, not ';'";
    }
    
    char message[512];
    if (suggestion != NULL) {
        snprintf(message, sizeof(message), "expected %s but got %s (%s)",
                zt_token_kind_name(kind), zt_token_kind_name(p->current.kind), suggestion);
    } else {
        snprintf(message, sizeof(message), "expected %s but got %s",
                zt_token_kind_name(kind), zt_token_kind_name(p->current.kind));
    }
    
    zt_diag_list_add(&p->result->diagnostics, ZT_DIAG_UNEXPECTED_TOKEN, p->current.span, "%s", message);
    if (p->current.kind != ZT_TOKEN_EOF) {
        return zt_parser_advance(p);
    }
    return p->current;
}

static void zt_parser_error_at(zt_parser *p, const char *message) {
    zt_diag_list_add(&p->result->diagnostics, ZT_DIAG_SYNTAX_ERROR, p->current.span, "%s", message);
}

static void zt_parser_error_contextual(zt_parser *p, const char *base_message, const char *context_message) {
    /* Use contextual message if available, otherwise use base message */
    const char *message = (context_message != NULL) ? context_message : base_message;
    zt_diag_list_add(&p->result->diagnostics, ZT_DIAG_SYNTAX_ERROR, p->current.span, "%s", message);
}

static int zt_parser_push_nesting(zt_parser *p, zt_source_span span, const char *label) {
    if (p == NULL) return 0;
    if (p->nesting_depth >= ZT_PARSER_MAX_NESTING_DEPTH) {
        zt_diag_list_add(
            &p->result->diagnostics,
            ZT_DIAG_STRUCTURE_LIMIT_EXCEEDED,
            span,
            "%s nesting exceeds compiler limit (%u levels)",
            label != NULL ? label : "parser",
            (unsigned)ZT_PARSER_MAX_NESTING_DEPTH);
        return 0;
    }
    p->nesting_depth += 1;
    return 1;
}

static void zt_parser_pop_nesting(zt_parser *p) {
    if (p != NULL && p->nesting_depth > 0) {
        p->nesting_depth -= 1;
    }
}

static zt_ast_node *zt_parser_ast_make(zt_parser *p, zt_ast_kind kind, zt_source_span span);

static const char *zt_parser_intern_unescaped(zt_parser *p, const char *text, size_t len) {
    char *buf = (char *)malloc(len + 1);
    size_t out_len = 0;
    size_t i = 0;
    while (i < len) {
        if (text[i] == '\\' && i + 1 < len) {
            i++;
            if (text[i] == 'n') buf[out_len++] = '\n';
            else if (text[i] == 't') buf[out_len++] = '\t';
            else if (text[i] == 'r') buf[out_len++] = '\r';
            else if (text[i] == '\\') buf[out_len++] = '\\';
            else if (text[i] == '"') buf[out_len++] = '"';
            else buf[out_len++] = text[i];
        } else {
            buf[out_len++] = text[i];
        }
        i++;
    }
    const char *interned = (char *)zt_string_pool_intern_len(p->pool, buf, out_len);
    free(buf);
    return interned;
}

static zt_ast_node *zt_parser_make_string_expr_from_token(zt_parser *p, zt_token tok) {
    const char *content;
    size_t content_len;
    zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_STRING_EXPR, tok.span);
    if (node == NULL) return NULL;

    if (tok.kind == ZT_TOKEN_TRIPLE_QUOTED_TEXT && tok.length >= 6) {
        content = tok.text + 3;
        content_len = tok.length - 6;
    } else if (tok.length >= 2) {
        content = tok.text + 1;
        content_len = tok.length - 2;
    } else {
        content = tok.text;
        content_len = tok.length;
    }

    node->as.string_expr.value = zt_parser_intern_unescaped(p, content, content_len);
    return node;
}

#define ZT_PARSER_MAX_NAME_PATH_LEN 1024

static void zt_parser_note_name_path_too_long(zt_parser *p, zt_source_span span, const char *label) {
    if (p == NULL) return;
    zt_diag_list_add(
        &p->result->diagnostics,
        ZT_DIAG_TOKEN_TOO_LONG,
        span,
        "%s is too long (max %d characters)",
        label != NULL ? label : "name",
        ZT_PARSER_MAX_NAME_PATH_LEN);
}

static int zt_parser_append_to_name_path(
        zt_parser *p,
        char *buf,
        size_t buf_cap,
        size_t *len,
        const char *text,
        size_t text_len,
        zt_source_span span,
        int *overflowed,
        const char *label) {
    if (buf == NULL || len == NULL || buf_cap == 0) return 0;
    if (overflowed != NULL && *overflowed) return 0;

    if (*len + text_len + 1 > buf_cap) {
        if (overflowed != NULL && !*overflowed) {
            *overflowed = 1;
            zt_parser_note_name_path_too_long(p, span, label);
        }
        return 0;
    }

    if (text_len > 0 && text != NULL) {
        memcpy(buf + *len, text, text_len);
        *len += text_len;
    }
    buf[*len] = '\0';
    return 1;
}

static int zt_parser_append_char_to_name_path(
        zt_parser *p,
        char *buf,
        size_t buf_cap,
        size_t *len,
        char ch,
        zt_source_span span,
        int *overflowed,
        const char *label) {
    return zt_parser_append_to_name_path(p, buf, buf_cap, len, &ch, 1, span, overflowed, label);
}

static int zt_parser_token_is_identifier_literal(const zt_token *tok, const char *text, size_t length) {
    if (tok == NULL || text == NULL || tok->kind != ZT_TOKEN_IDENTIFIER) return 0;
    return tok->length == length && strncmp(tok->text, text, length) == 0;
}

static int zt_parser_hex_digit_value(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return 10 + (ch - 'a');
    if (ch >= 'A' && ch <= 'F') return 10 + (ch - 'A');
    return -1;
}

static char *zt_parser_normalize_hex_bytes(zt_parser *p, const char *text, size_t length, zt_source_span span) {
    char normalized[1024];
    size_t out_len = 0;
    size_t i;

    if (text == NULL) { text = ""; length = 0; }

    for (i = 0; i < length; i++) {
        unsigned char ch = (unsigned char)text[i];
        if (isspace(ch) || ch == '_') {
            continue;
        }
        if (zt_parser_hex_digit_value((char)ch) < 0) {
            zt_diag_list_add(&p->result->diagnostics, ZT_DIAG_SYNTAX_ERROR, span, "invalid hexadecimal byte literal character '%c'", isprint(ch) ? ch : '?');
            return (char *)zt_string_pool_intern_len(p->pool, "", 0);
        }
        if (out_len + 1 >= sizeof(normalized)) {
            zt_diag_list_add(&p->result->diagnostics, ZT_DIAG_SYNTAX_ERROR, span, "hexadecimal byte literal is too large");
            return (char *)zt_string_pool_intern_len(p->pool, "", 0);
        }
        normalized[out_len++] = (char)toupper(ch);
    }

    if ((out_len % 2) != 0) {
        zt_diag_list_add(&p->result->diagnostics, ZT_DIAG_SYNTAX_ERROR, span, "hexadecimal byte literal requires an even number of digits");
        return (char *)zt_string_pool_intern_len(p->pool, "", 0);
    }

    normalized[out_len] = '\0';
    return (char *)zt_string_pool_intern(p->pool, normalized);
}

/*
 * Returns non-zero when `kind` is allowed to absorb the parser's
 * pending leading comments.
 *
 * Only declarations, statements and a few structural nodes
 * (struct fields, enum variants, trait methods, match cases) accept
 * leading comments. Expression, type and helper nodes DO NOT —
 * without this filter, a comment sitting above `func foo(value: int)`
 * is stolen by the first internal AST node the parser builds (the
 * `TYPE_SIMPLE("int")` of the first parameter), which then formats
 * into garbage like `value: -- comment\nint`.
 *
 * Leaving internal nodes out of this whitelist keeps the pending
 * comments in the parser state until a declaration/statement-level
 * node is created, where they belong.
 */
static int zt_ast_kind_accepts_leading_comments(zt_ast_kind kind) {
    switch (kind) {
        case ZT_AST_NAMESPACE_DECL:
        case ZT_AST_IMPORT_DECL:
        case ZT_AST_FUNC_DECL:
        case ZT_AST_STRUCT_DECL:
        case ZT_AST_TRAIT_DECL:
        case ZT_AST_APPLY_DECL:
        case ZT_AST_ENUM_DECL:
        case ZT_AST_EXTERN_DECL:
        case ZT_AST_STRUCT_FIELD:
        case ZT_AST_ENUM_VARIANT:
        case ZT_AST_TRAIT_METHOD:
        case ZT_AST_MATCH_CASE:
        case ZT_AST_VAR_DECL:
        case ZT_AST_CONST_DECL:
        case ZT_AST_ASSIGN_STMT:
        case ZT_AST_INDEX_ASSIGN_STMT:
        case ZT_AST_FIELD_ASSIGN_STMT:
        case ZT_AST_IF_STMT:
        case ZT_AST_WHILE_STMT:
        case ZT_AST_FOR_STMT:
        case ZT_AST_REPEAT_STMT:
        case ZT_AST_RETURN_STMT:
        case ZT_AST_MATCH_STMT:
        case ZT_AST_BREAK_STMT:
        case ZT_AST_CONTINUE_STMT:
        case ZT_AST_EXPR_STMT:
            return 1;
        default:
            return 0;
    }
}

static zt_ast_node *zt_parser_ast_make(zt_parser *p, zt_ast_kind kind, zt_source_span span) {
    zt_ast_node *node = (zt_ast_node *)zt_arena_alloc(p->arena, sizeof(zt_ast_node));
    if (node != NULL) {
        memset(node, 0, sizeof(zt_ast_node));
        node->kind = kind;
        node->span = span;
        if (p->pending_comment_count > 0 && zt_ast_kind_accepts_leading_comments(kind)) {
            node->comments = p->pending_comments;
            node->comment_count = p->pending_comment_count;
            p->pending_comments = NULL;
            p->pending_comment_count = 0;
            p->pending_comment_capacity = 0;
        }
    }
    return node;
}

/*
 * Scoped leading-comment tracking.
 *
 * The parser populates `pending_comments` eagerly as `advance()`
 * consumes tokens, and `ast_make` greedily attaches ALL pending
 * comments to the newly created node. This creates two related
 * problems for the formatter:
 *
 *   1. A comment sitting BEFORE a declaration gets stolen by the
 *      first WHITELISTED inner AST node (e.g. a `RETURN_STMT` inside
 *      a function body absorbs the `-- doc` that belonged on the
 *      enclosing `FUNC_DECL`).
 *
 *   2. A comment appearing AFTER a node (between this node's last
 *      token and the first token of the next sibling) gets absorbed
 *      by the current node's closing `ast_make`, because `advance()`
 *      already gathered it into pending before the node was created.
 *
 * Both issues cause `fmt(fmt(x)) != fmt(x)`: each pass shifts the
 * comment onto a different node.
 *
 * The helpers below implement a simple scoped pattern used at each
 * loop that creates sibling declarations or statements:
 *
 *   `zt_parser_stash_leading(p)`    snapshots the comments that were
 *                                    pending BEFORE this scope and
 *                                    clears `pending`, so inner
 *                                    parse_X work starts with an
 *                                    empty pending list and cannot
 *                                    steal the outer-leading comments.
 *
 *   `zt_parser_reclaim_node_trailing(p, node)`   after the inner
 *                                    parse returns, moves every
 *                                    comment attached to `node`
 *                                    back onto `pending`. These were
 *                                    accumulated DURING the inner
 *                                    parse and therefore belong to
 *                                    the NEXT sibling.
 *
 *   `zt_parser_apply_stash_to_node(p, node, stash)`  finally attaches
 *                                    the previously snapshotted
 *                                    outer-leading comments to
 *                                    `node` as its real leading.
 *
 * Applying these at `zt_parse`'s declarations/imports loops and at
 * `parse_block_ex`'s statements loop restores a predictable
 * "comments stay attached to the node they lead" invariant, which
 * is what the idempotence property requires.
 */

typedef struct zt_leading_stash {
    zt_token *comments;
    size_t count;
    size_t capacity;
} zt_leading_stash;

static zt_leading_stash zt_parser_stash_leading(zt_parser *p) {
    zt_leading_stash s;
    s.comments = p->pending_comments;
    s.count = p->pending_comment_count;
    s.capacity = p->pending_comment_capacity;
    p->pending_comments = NULL;
    p->pending_comment_count = 0;
    p->pending_comment_capacity = 0;
    return s;
}

static void zt_parser_reclaim_node_trailing(zt_parser *p, zt_ast_node *node) {
    size_t trailing;
    zt_token *new_pending;

    if (node == NULL || node->comment_count == 0) return;

    trailing = node->comment_count;
    new_pending = (zt_token *)zt_arena_alloc(p->arena, trailing * sizeof(zt_token));
    if (new_pending == NULL) {
        /* Out of memory: keep the comments on the node rather than
         * silently drop them. The formatter may emit them in the
         * wrong place but no information is lost. */
        return;
    }
    memcpy(new_pending, node->comments, trailing * sizeof(zt_token));
    p->pending_comments = new_pending;
    p->pending_comment_count = trailing;
    p->pending_comment_capacity = trailing;

    node->comments = NULL;
    node->comment_count = 0;
}

static void zt_parser_apply_stash_to_node(
        zt_parser *p,
        zt_ast_node *node,
        zt_leading_stash stash) {
    size_t total;
    zt_token *merged;

    if (stash.count == 0) return;

    if (node == NULL) {
        /* We have no node to attach to; restore the stash as pending
         * so the next sibling can adopt it. */
        p->pending_comments = stash.comments;
        p->pending_comment_count = stash.count;
        p->pending_comment_capacity = stash.capacity;
        return;
    }

    total = stash.count + node->comment_count;
    merged = (zt_token *)zt_arena_alloc(p->arena, total * sizeof(zt_token));
    if (merged == NULL) return;
    memcpy(merged, stash.comments, stash.count * sizeof(zt_token));
    if (node->comment_count > 0) {
        memcpy(&merged[stash.count], node->comments, node->comment_count * sizeof(zt_token));
    }
    node->comments = merged;
    node->comment_count = total;
}

static zt_ast_node *zt_parser_parse_type(zt_parser *p);
static zt_ast_node *zt_parser_parse_expression(zt_parser *p);
static zt_ast_node *zt_parser_parse_block(zt_parser *p);
static zt_ast_node *zt_parser_parse_statement(zt_parser *p);
static zt_ast_node_list zt_parser_parse_params(zt_parser *p);
static zt_ast_node_list zt_parser_parse_generic_constraints(zt_parser *p);

static void zt_parser_push_fmt_text_part(
        zt_parser *p,
        zt_ast_node_list *parts,
        zt_source_span span,
        const char *text,
        size_t len) {
    zt_ast_node *node;
    if (p == NULL || parts == NULL || text == NULL || len == 0) return;
    node = zt_parser_ast_make(p, ZT_AST_STRING_EXPR, span);
    if (node == NULL) return;
    node->as.string_expr.value = zt_parser_intern_unescaped(p, text, len);
    zt_ast_node_list_push(p->arena, parts, node);
}

static int zt_parser_find_fmt_expr_end(const char *text, size_t length, size_t start, size_t *out_end) {
    size_t i = start;
    int depth = 1;
    int in_string = 0;
    int in_triple = 0;
    int escaped = 0;

    if (out_end != NULL) *out_end = 0;
    if (text == NULL || start >= length) return 0;

    while (i < length) {
        char ch = text[i];

        if (in_string) {
            if (escaped) {
                escaped = 0;
                i += 1;
                continue;
            }
            if (ch == '\\') {
                escaped = 1;
                i += 1;
                continue;
            }
            if (in_triple) {
                if (ch == '"' && i + 2 < length && text[i + 1] == '"' && text[i + 2] == '"') {
                    in_string = 0;
                    in_triple = 0;
                    i += 3;
                    continue;
                }
                i += 1;
                continue;
            }
            if (ch == '"') {
                in_string = 0;
                i += 1;
                continue;
            }
            i += 1;
            continue;
        }

        if (ch == '"') {
            if (i + 2 < length && text[i + 1] == '"' && text[i + 2] == '"') {
                in_string = 1;
                in_triple = 1;
                i += 3;
                continue;
            }
            in_string = 1;
            in_triple = 0;
            i += 1;
            continue;
        }

        if (ch == '{') {
            depth += 1;
            i += 1;
            continue;
        }

        if (ch == '}') {
            depth -= 1;
            if (depth == 0) {
                if (out_end != NULL) *out_end = i;
                return 1;
            }
            i += 1;
            continue;
        }

        i += 1;
    }

    return 0;
}

static zt_ast_node *zt_parser_parse_expression_fragment(
        zt_parser *p,
        const char *source_name,
        const char *text,
        size_t length,
        zt_source_span fallback_span) {
    zt_lexer *lexer;
    zt_parser sub;
    zt_ast_node *expr;

    if (p == NULL || text == NULL || length == 0) return NULL;

    lexer = zt_lexer_make(source_name != NULL ? source_name : "<fmt>", text, length);
    if (lexer == NULL) return NULL;

    zt_lexer_set_diagnostics(lexer, &p->result->diagnostics);
    memset(&sub, 0, sizeof(sub));
    sub.lexer = lexer;
    sub.result = p->result;
    sub.arena = p->arena;
    sub.pool = p->pool;
    sub.current = zt_parser_next_non_comment_token(&sub);
    expr = zt_parser_parse_expression(&sub);

    if (expr == NULL || sub.current.kind != ZT_TOKEN_EOF) {
        zt_diag_list_add(
            &p->result->diagnostics,
            ZT_DIAG_SYNTAX_ERROR,
            fallback_span,
            "invalid expression inside fmt interpolation");
    }

    zt_lexer_dispose(lexer);
    return expr;
}

static zt_ast_node *zt_parser_parse_fmt_expr_from_token(zt_parser *p, zt_token fmt_tok, zt_token text_tok) {
    const char *content = NULL;
    size_t content_len = 0;
    zt_ast_node *fmt_node;
    zt_ast_node_list parts = zt_ast_node_list_make();
    char *literal_buf = NULL;
    size_t literal_len = 0;
    size_t i = 0;

    if (p == NULL) return NULL;

    if (text_tok.kind == ZT_TOKEN_TRIPLE_QUOTED_TEXT && text_tok.length >= 6) {
        content = text_tok.text + 3;
        content_len = text_tok.length - 6;
    } else if (text_tok.length >= 2) {
        content = text_tok.text + 1;
        content_len = text_tok.length - 2;
    } else {
        content = text_tok.text;
        content_len = text_tok.length;
    }

    fmt_node = zt_parser_ast_make(p, ZT_AST_FMT_EXPR, fmt_tok.span);
    if (fmt_node == NULL) return NULL;
    fmt_node->as.fmt_expr.parts = parts;

    literal_buf = (char *)malloc(content_len + 1);
    if (literal_buf == NULL) {
        zt_diag_list_add(&p->result->diagnostics, ZT_DIAG_SYNTAX_ERROR, fmt_tok.span, "out of memory while parsing fmt expression");
        return fmt_node;
    }

    while (i < content_len) {
        char ch = content[i];

        if (ch == '{') {
            size_t expr_start;
            size_t expr_end;
            size_t trimmed_start;
            size_t trimmed_end;
            zt_ast_node *expr_node;

            if (i + 1 < content_len && content[i + 1] == '{') {
                literal_buf[literal_len++] = '{';
                i += 2;
                continue;
            }

            zt_parser_push_fmt_text_part(p, &fmt_node->as.fmt_expr.parts, text_tok.span, literal_buf, literal_len);
            literal_len = 0;

            expr_start = i + 1;
            if (!zt_parser_find_fmt_expr_end(content, content_len, expr_start, &expr_end)) {
                zt_diag_list_add(
                    &p->result->diagnostics,
                    ZT_DIAG_SYNTAX_ERROR,
                    text_tok.span,
                    "unterminated fmt interpolation expression");
                break;
            }

            trimmed_start = expr_start;
            trimmed_end = expr_end;
            while (trimmed_start < trimmed_end && isspace((unsigned char)content[trimmed_start])) trimmed_start += 1;
            while (trimmed_end > trimmed_start && isspace((unsigned char)content[trimmed_end - 1])) trimmed_end -= 1;

            if (trimmed_start >= trimmed_end) {
                zt_diag_list_add(
                    &p->result->diagnostics,
                    ZT_DIAG_SYNTAX_ERROR,
                    text_tok.span,
                    "fmt interpolation expression cannot be empty");
            } else {
                expr_node = zt_parser_parse_expression_fragment(
                    p,
                    text_tok.span.source_name,
                    content + trimmed_start,
                    trimmed_end - trimmed_start,
                    text_tok.span);
                if (expr_node != NULL) {
                    expr_node->span = text_tok.span;
                    zt_ast_node_list_push(p->arena, &fmt_node->as.fmt_expr.parts, expr_node);
                }
            }

            i = expr_end + 1;
            continue;
        }

        if (ch == '}') {
            if (i + 1 < content_len && content[i + 1] == '}') {
                literal_buf[literal_len++] = '}';
                i += 2;
                continue;
            }
            zt_diag_list_add(
                &p->result->diagnostics,
                ZT_DIAG_SYNTAX_ERROR,
                text_tok.span,
                "unescaped '}' in fmt literal; use '}}' for a literal brace");
            literal_buf[literal_len++] = '}';
            i += 1;
            continue;
        }

        literal_buf[literal_len++] = ch;
        i += 1;
    }

    zt_parser_push_fmt_text_part(p, &fmt_node->as.fmt_expr.parts, text_tok.span, literal_buf, literal_len);
    free(literal_buf);
    return fmt_node;
}

static int zt_parser_is_type_name(zt_token_kind kind) {
    return kind == ZT_TOKEN_IDENTIFIER ||
           kind == ZT_TOKEN_OPTIONAL ||
           kind == ZT_TOKEN_RESULT ||
           kind == ZT_TOKEN_LIST ||
           kind == ZT_TOKEN_MAP ||
           kind == ZT_TOKEN_GRID2D ||
           kind == ZT_TOKEN_PQUEUE ||
           kind == ZT_TOKEN_CIRCBUF ||
           kind == ZT_TOKEN_BTREEMAP ||
           kind == ZT_TOKEN_BTREESET ||
           kind == ZT_TOKEN_GRID3D ||
           kind == ZT_TOKEN_VOID ||
           kind == ZT_TOKEN_SELF;
}

static zt_token zt_parser_expect_type_name(zt_parser *p) {
    if (zt_parser_is_type_name(p->current.kind)) {
        return zt_parser_advance(p);
    }
    return zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
}

static char *zt_parser_parse_type_name_path(zt_parser *p, zt_source_span *out_span) {
    zt_token name_tok = zt_parser_expect_type_name(p);

    char buf[ZT_PARSER_MAX_NAME_PATH_LEN + 1];
    size_t len = 0;
    int overflowed = 0;

    if (out_span != NULL) {
        *out_span = name_tok.span;
    }

    buf[0] = '\0';
    zt_parser_append_to_name_path(p, buf, sizeof(buf), &len, name_tok.text, name_tok.length, name_tok.span, &overflowed, "type name");

    while (zt_parser_match(p, ZT_TOKEN_DOT)) {
        zt_token part_tok = zt_parser_expect_type_name(p);
        zt_parser_append_char_to_name_path(p, buf, sizeof(buf), &len, '.', part_tok.span, &overflowed, "type name");
        zt_parser_append_to_name_path(p, buf, sizeof(buf), &len, part_tok.text, part_tok.length, part_tok.span, &overflowed, "type name");
    }

    return (char *)zt_string_pool_intern(p->pool, buf);
}

static zt_ast_node *zt_parser_parse_type(zt_parser *p) {
    zt_ast_node *result = NULL;
    zt_source_span type_span = p->current.span;
    if (!zt_parser_push_nesting(p, type_span, "type")) {
        return NULL;
    }
    if (zt_parser_match(p, ZT_TOKEN_DYN)) {
        zt_ast_node *inner = NULL;
        if (zt_parser_match(p, ZT_TOKEN_LT)) {
            inner = zt_parser_parse_type(p);
            zt_parser_expect(p, ZT_TOKEN_GT);
        } else {
            inner = zt_parser_parse_type(p);
        }
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_TYPE_DYN, type_span);
        if (node != NULL) {
            node->as.type_dyn.inner_type = inner;
        }
        result = node;
        zt_parser_pop_nesting(p);
        return result;
    }

    /* Callable type: `func(T1, T2) -> R` (Decision 089). */
    if (zt_parser_match(p, ZT_TOKEN_FUNC)) {
        zt_ast_node_list params = zt_ast_node_list_make();
        zt_ast_node *return_type = NULL;
        zt_parser_expect(p, ZT_TOKEN_LPAREN);
        if (!zt_parser_check(p, ZT_TOKEN_RPAREN)) {
            zt_ast_node_list_push(p->arena, &params, zt_parser_parse_type(p));
            while (zt_parser_match(p, ZT_TOKEN_COMMA)) {
                zt_ast_node_list_push(p->arena, &params, zt_parser_parse_type(p));
            }
        }
        zt_parser_expect(p, ZT_TOKEN_RPAREN);
        if (zt_parser_match(p, ZT_TOKEN_ARROW)) {
            return_type = zt_parser_parse_type(p);
        }
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_TYPE_CALLABLE, type_span);
        if (node != NULL) {
            node->as.type_callable.params = params;
            node->as.type_callable.return_type = return_type;
        }
        result = node;
        zt_parser_pop_nesting(p);
        return result;
    }

    zt_source_span name_span = zt_source_span_unknown();
    char *type_name = zt_parser_parse_type_name_path(p, &name_span);
    zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_TYPE_SIMPLE, name_span);
    if (node == NULL) {
        zt_parser_pop_nesting(p);
        return NULL;
    }
    node->as.type_simple.name = type_name;

    if (zt_parser_check(p, ZT_TOKEN_LT)) {
        zt_parser_advance(p);
        zt_ast_node_list type_args = zt_ast_node_list_make();
        if (!zt_parser_check(p, ZT_TOKEN_GT)) {
            zt_ast_node_list_push(p->arena, &type_args, zt_parser_parse_type(p));
            while (zt_parser_match(p, ZT_TOKEN_COMMA)) {
                zt_ast_node_list_push(p->arena, &type_args, zt_parser_parse_type(p));
            }
        }
        zt_parser_expect(p, ZT_TOKEN_GT);

        zt_ast_node *generic = zt_parser_ast_make(p, ZT_AST_TYPE_GENERIC, name_span);
        if (generic == NULL) {
            zt_parser_pop_nesting(p);
            return NULL;
        }
        generic->as.type_generic.name = node->as.type_simple.name;
        node->as.type_simple.name = NULL;
        generic->as.type_generic.type_args = type_args;
        result = generic;
        zt_parser_pop_nesting(p);
        return result;
    }

    result = node;
    zt_parser_pop_nesting(p);
    return result;
}

static int zt_is_named_arg_label_token(zt_token_kind kind) {
    return kind == ZT_TOKEN_IDENTIFIER || kind == ZT_TOKEN_TO;
}

static int zt_is_named_arg_ahead(zt_parser *p) {
    if (!zt_is_named_arg_label_token(p->current.kind)) return 0;
    zt_parser_fill_peek(p);
    return p->peek.kind == ZT_TOKEN_COLON;
}

static zt_ast_node *zt_parser_parse_primary(zt_parser *p) {
    zt_token tok = p->current;

    if (tok.kind == ZT_TOKEN_INT_LITERAL) {
        zt_parser_advance(p);
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_INT_EXPR, tok.span);
        if (node == NULL) return NULL;
        node->as.int_expr.value = (char *)zt_string_pool_intern_len(p->pool, tok.text, tok.length);
        return node;
    }

    if (tok.kind == ZT_TOKEN_FLOAT_LITERAL) {
        zt_parser_advance(p);
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_FLOAT_EXPR, tok.span);
        if (node == NULL) return NULL;
        node->as.float_expr.value = (char *)zt_string_pool_intern_len(p->pool, tok.text, tok.length);
        return node;
    }

    if (zt_parser_token_is_identifier_literal(&tok, "fmt", 3)) {
        zt_parser_fill_peek(p);
        if (p->peek.kind == ZT_TOKEN_STRING_LITERAL || p->peek.kind == ZT_TOKEN_TRIPLE_QUOTED_TEXT) {
            zt_token fmt_tok = tok;
            zt_parser_advance(p);
            tok = zt_parser_advance(p);
            return zt_parser_parse_fmt_expr_from_token(p, fmt_tok, tok);
        }
    }

    if (tok.kind == ZT_TOKEN_STRING_LITERAL || tok.kind == ZT_TOKEN_TRIPLE_QUOTED_TEXT) {
        zt_parser_advance(p);
        return zt_parser_make_string_expr_from_token(p, tok);
    }

    if (tok.kind == ZT_TOKEN_TRUE || tok.kind == ZT_TOKEN_FALSE) {
        zt_parser_advance(p);
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_BOOL_EXPR, tok.span);
        if (node == NULL) return NULL;
        node->as.bool_expr.value = (tok.kind == ZT_TOKEN_TRUE) ? 1 : 0;
        return node;
    }

    if (tok.kind == ZT_TOKEN_NONE) {
        zt_parser_advance(p);
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_NONE_EXPR, tok.span);
        return node;
    }

    if (tok.kind == ZT_TOKEN_SUCCESS) {
        zt_parser_advance(p);
        zt_parser_expect(p, ZT_TOKEN_LPAREN);
        zt_ast_node *value = NULL;
        if (!zt_parser_check(p, ZT_TOKEN_RPAREN)) {
            value = zt_parser_parse_expression(p);
        }
        zt_parser_expect(p, ZT_TOKEN_RPAREN);
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_SUCCESS_EXPR, tok.span);
        if (node == NULL) return value;
        node->as.success_expr.value = value;
        return node;
    }

    if (tok.kind == ZT_TOKEN_KW_ERROR) {
        zt_parser_advance(p);
        zt_parser_expect(p, ZT_TOKEN_LPAREN);
        zt_ast_node *value = NULL;
        if (!zt_parser_check(p, ZT_TOKEN_RPAREN)) {
            value = zt_parser_parse_expression(p);
        }
        zt_parser_expect(p, ZT_TOKEN_RPAREN);
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_ERROR_EXPR, tok.span);
        if (node == NULL) return value;
        node->as.error_expr.value = value;
        return node;
    }

    if (tok.kind == ZT_TOKEN_LPAREN) {
        zt_parser_advance(p);
        zt_ast_node *inner = zt_parser_parse_expression(p);
        zt_parser_expect(p, ZT_TOKEN_RPAREN);
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_GROUPED_EXPR, tok.span);
        if (node == NULL) return NULL;
        node->as.grouped_expr.inner = inner;
        return node;
    }

    if (tok.kind == ZT_TOKEN_LBRACKET) {
        zt_parser_advance(p);
        zt_ast_node_list elements = zt_ast_node_list_make();
        if (!zt_parser_check(p, ZT_TOKEN_RBRACKET)) {
            zt_ast_node_list_push(p->arena, &elements, zt_parser_parse_expression(p));
            while (zt_parser_match(p, ZT_TOKEN_COMMA)) {
                if (zt_parser_check(p, ZT_TOKEN_RBRACKET)) break;
                zt_ast_node_list_push(p->arena, &elements, zt_parser_parse_expression(p));
            }
        }
        zt_parser_expect(p, ZT_TOKEN_RBRACKET);
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_LIST_EXPR, tok.span);
        if (node == NULL) return NULL;
        node->as.list_expr.elements = elements;
        return node;
    }

    if (tok.kind == ZT_TOKEN_LBRACE) {
        zt_parser_advance(p);
        zt_ast_map_entry_list entries = zt_ast_map_entry_list_make();
        if (!zt_parser_check(p, ZT_TOKEN_RBRACE)) {
            {
                zt_ast_node *key = zt_parser_parse_expression(p);
                zt_parser_expect(p, ZT_TOKEN_COLON);
                zt_ast_node *val = zt_parser_parse_expression(p);
                zt_ast_map_entry entry;
                entry.span = tok.span;
                entry.key = key;
                entry.value = val;
                zt_ast_map_entry_list_push(p->arena, &entries, entry);
            }
            while (zt_parser_match(p, ZT_TOKEN_COMMA)) {
                if (zt_parser_check(p, ZT_TOKEN_RBRACE)) break;
                zt_ast_node *key = zt_parser_parse_expression(p);
                zt_parser_expect(p, ZT_TOKEN_COLON);
                zt_ast_node *val = zt_parser_parse_expression(p);
                zt_ast_map_entry entry;
                entry.span = tok.span;
                entry.key = key;
                entry.value = val;
                zt_ast_map_entry_list_push(p->arena, &entries, entry);
            }
        }
        zt_parser_expect(p, ZT_TOKEN_RBRACE);
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_MAP_EXPR, tok.span);
        if (node == NULL) return NULL;
        node->as.map_expr.entries = entries;
        return node;
    }

    if (tok.kind == ZT_TOKEN_MINUS || tok.kind == ZT_TOKEN_NOT) {
        zt_parser_advance(p);
        zt_ast_node *operand = zt_parser_parse_primary(p);
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_UNARY_EXPR, tok.span);
        if (node == NULL) return NULL;
        node->as.unary_expr.op = tok.kind;
        node->as.unary_expr.operand = operand;
        return node;
    }

    if (zt_parser_token_is_identifier_literal(&tok, "hex", 3)) {
        zt_parser_fill_peek(p);
        if (zt_parser_token_is_identifier_literal(&p->peek, "bytes", 5)) {
            zt_parser_advance(p);
            zt_parser_advance(p);
            if (p->current.kind != ZT_TOKEN_STRING_LITERAL && p->current.kind != ZT_TOKEN_TRIPLE_QUOTED_TEXT) {
                zt_parser_expect(p, ZT_TOKEN_STRING_LITERAL);
                return NULL;
            }
            tok = zt_parser_advance(p);
            zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_BYTES_EXPR, tok.span);
            if (node == NULL) return NULL;
            size_t inner_offset = (tok.kind == ZT_TOKEN_TRIPLE_QUOTED_TEXT) ? 3 : 1;
            size_t inner_len = (tok.length > 2 * inner_offset) ? (tok.length - (2 * inner_offset)) : 0;
            if (tok.length < 2 * inner_offset) inner_offset = 0;
            node->as.bytes_expr.value = zt_parser_normalize_hex_bytes(p, tok.text + inner_offset, inner_len, tok.span);
            return node;
        }
    }

    if (tok.kind == ZT_TOKEN_IDENTIFIER || tok.kind == ZT_TOKEN_SELF || tok.kind == ZT_TOKEN_TO) {
        zt_parser_advance(p);
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_IDENT_EXPR, tok.span);
        if (node == NULL) return NULL;
        node->as.ident_expr.name = (char *)zt_string_pool_intern_len(p->pool, tok.text, tok.length);
        return node;
    }

    /* Contextual error messages for better UX */
    if (p->in_function_body) {
        zt_parser_error_contextual(p, "expected expression", "expected expression in statement");
    } else if (p->in_type_position) {
        zt_parser_error_contextual(p, "expected expression", "expected type expression");
    } else {
        zt_parser_error_at(p, "expected expression");
    }
    zt_parser_advance(p);
    return NULL;
}

typedef enum zt_precedence_level {
    ZT_PREC_OR,
    ZT_PREC_AND,
    ZT_PREC_EQUALITY,
    ZT_PREC_COMPARISON,
    ZT_PREC_ADDITION,
    ZT_PREC_MULTIPLICATION,
    ZT_PREC_UNARY,
    ZT_PREC_PRIMARY
} zt_precedence_level;

static int zt_is_binary_op(zt_token_kind kind) {
    switch (kind) {
        case ZT_TOKEN_PLUS: case ZT_TOKEN_MINUS: case ZT_TOKEN_STAR:
        case ZT_TOKEN_SLASH: case ZT_TOKEN_PERCENT:
        case ZT_TOKEN_EQEQ: case ZT_TOKEN_NEQ:
        case ZT_TOKEN_LT: case ZT_TOKEN_LTE:
        case ZT_TOKEN_GT: case ZT_TOKEN_GTE:
        case ZT_TOKEN_AND: case ZT_TOKEN_OR:
            return 1;
        default:
            return 0;
    }
}

static zt_precedence_level zt_binary_precedence(zt_token_kind kind) {
    switch (kind) {
        case ZT_TOKEN_OR: return ZT_PREC_OR;
        case ZT_TOKEN_AND: return ZT_PREC_AND;
        case ZT_TOKEN_EQEQ: case ZT_TOKEN_NEQ: return ZT_PREC_EQUALITY;
        case ZT_TOKEN_LT: case ZT_TOKEN_LTE: case ZT_TOKEN_GT: case ZT_TOKEN_GTE: return ZT_PREC_COMPARISON;
        case ZT_TOKEN_PLUS: case ZT_TOKEN_MINUS: return ZT_PREC_ADDITION;
        case ZT_TOKEN_STAR: case ZT_TOKEN_SLASH: case ZT_TOKEN_PERCENT: return ZT_PREC_MULTIPLICATION;
        default: return ZT_PREC_PRIMARY;
    }
}

static zt_ast_node *zt_parser_parse_postfix(zt_parser *p) {
    zt_ast_node *expr = zt_parser_parse_primary(p);

    while (expr != NULL) {
        if (zt_parser_check(p, ZT_TOKEN_LPAREN)) {
            zt_token call_tok = p->current;
            zt_parser_advance(p);
            zt_ast_node_list positional = zt_ast_node_list_make();
            zt_ast_named_arg_list named = zt_ast_named_arg_list_make();
            int in_named = 0;

            if (!zt_parser_check(p, ZT_TOKEN_RPAREN)) {
                do {
                    if (!in_named && zt_is_named_arg_ahead(p)) {
                        in_named = 1;
                    }
                    if (in_named) {
                        zt_token name_tok = p->current;
                        if (!zt_is_named_arg_label_token(name_tok.kind)) {
                            zt_parser_error_contextual(p, "expected named argument label", "named argument labels must be identifiers or 'to'");
                            name_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
                        } else {
                            zt_parser_advance(p);
                        }
                        zt_parser_expect(p, ZT_TOKEN_COLON);
                        zt_ast_node *val = zt_parser_parse_expression(p);
                        zt_ast_named_arg arg;
                        arg.span = name_tok.span;
                        arg.name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);
                        arg.value = val;
                        zt_ast_named_arg_list_push(p->arena, &named, arg);
                    } else {
                        zt_ast_node_list_push(p->arena, &positional, zt_parser_parse_expression(p));
                    }
                } while (zt_parser_match(p, ZT_TOKEN_COMMA));
            }

            zt_parser_expect(p, ZT_TOKEN_RPAREN);
            zt_ast_node *call = zt_parser_ast_make(p, ZT_AST_CALL_EXPR, call_tok.span);
            if (call == NULL) return expr;
            call->as.call_expr.callee = expr;
            call->as.call_expr.positional_args = positional;
            call->as.call_expr.named_args = named;
            expr = call;
        } else if (zt_parser_check(p, ZT_TOKEN_DOT)) {
            zt_token dot_tok = p->current;
            zt_parser_advance(p);
            zt_token field_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
            char *field_name = (char *)zt_string_pool_intern_len(p->pool, field_tok.text, field_tok.length);
            zt_ast_node *field = zt_parser_ast_make(p, ZT_AST_FIELD_EXPR, dot_tok.span);
            if (field == NULL) {
                free(field_name);
                return expr;
            }
            field->as.field_expr.object = expr;
            field->as.field_expr.field_name = field_name;
            expr = field;
        } else if (zt_parser_check(p, ZT_TOKEN_LBRACKET)) {
            zt_token bracket_tok = p->current;
            zt_ast_node *start = NULL;
            zt_ast_node *end = NULL;
            int is_slice = 0;
            zt_parser_advance(p);

            if (zt_parser_check(p, ZT_TOKEN_DOTDOT)) {
                is_slice = 1;
                zt_parser_advance(p);
                if (!zt_parser_check(p, ZT_TOKEN_RBRACKET)) {
                    end = zt_parser_parse_expression(p);
                }
            } else {
                start = zt_parser_parse_expression(p);
                if (zt_parser_check(p, ZT_TOKEN_DOTDOT)) {
                    is_slice = 1;
                    zt_parser_advance(p);
                    if (!zt_parser_check(p, ZT_TOKEN_RBRACKET)) {
                        end = zt_parser_parse_expression(p);
                    }
                }
            }

            zt_parser_expect(p, ZT_TOKEN_RBRACKET);
            if (is_slice) {
                zt_ast_node *slice = zt_parser_ast_make(p, ZT_AST_SLICE_EXPR, bracket_tok.span);
                if (slice == NULL) return expr;
                slice->as.slice_expr.object = expr;
                slice->as.slice_expr.start = start;
                slice->as.slice_expr.end = end;
                expr = slice;
            } else {
                zt_ast_node *idx = zt_parser_ast_make(p, ZT_AST_INDEX_EXPR, bracket_tok.span);
                if (idx == NULL) return expr;
                idx->as.index_expr.object = expr;
                idx->as.index_expr.index = start;
                expr = idx;
            }
        } else if (zt_parser_check(p, ZT_TOKEN_QUESTION)) {
            zt_parser_advance(p);
            zt_ast_node *unwrap = zt_parser_ast_make(p, ZT_AST_UNARY_EXPR, expr->span);
            if (unwrap == NULL) return expr;
            unwrap->as.unary_expr.op = ZT_TOKEN_QUESTION;
            unwrap->as.unary_expr.operand = expr;
            expr = unwrap;
        } else {
            break;
        }
    }

    return expr;
}

static zt_ast_node *zt_parser_parse_binary(zt_parser *p, zt_precedence_level min_prec) {
    zt_ast_node *left = zt_parser_parse_postfix(p);
    if (left == NULL) return NULL;

    while (zt_is_binary_op(p->current.kind)) {
        zt_precedence_level prec = zt_binary_precedence(p->current.kind);
        if ((int)prec < (int)min_prec) break;

        zt_token op_tok = p->current;
        zt_parser_advance(p);
        zt_ast_node *right = zt_parser_parse_binary(p, (zt_precedence_level)((int)prec + 1));

        zt_ast_node *binary = zt_parser_ast_make(p, ZT_AST_BINARY_EXPR, left->span);
        if (binary == NULL) return left;
        binary->as.binary_expr.left = left;
        binary->as.binary_expr.right = right;
        binary->as.binary_expr.op = op_tok.kind;
        left = binary;
    }

    return left;
}

static zt_ast_node *zt_parser_parse_expression(zt_parser *p) {
    zt_ast_node *result;
    if (!zt_parser_push_nesting(p, p->current.span, "expression")) {
        return NULL;
    }
    result = zt_parser_parse_binary(p, ZT_PREC_OR);
    zt_parser_pop_nesting(p);
    return result;
}

static zt_ast_node_list zt_parser_parse_params(zt_parser *p) {
    zt_ast_node_list params = zt_ast_node_list_make();
    zt_parser_expect(p, ZT_TOKEN_LPAREN);

    if (!zt_parser_check(p, ZT_TOKEN_RPAREN)) {
        do {
            zt_token name_tok = p->current;
            if (!(name_tok.kind == ZT_TOKEN_IDENTIFIER || name_tok.kind == ZT_TOKEN_TO)) {
                zt_parser_error_contextual(p, "expected parameter name", "parameter names must be identifiers (or 'to')");
                name_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
            } else {
                zt_parser_advance(p);
            }
            zt_parser_expect(p, ZT_TOKEN_COLON);
            zt_ast_node *type_node = zt_parser_parse_type(p);
            zt_ast_node *default_value = NULL;
            if (zt_parser_match(p, ZT_TOKEN_EQ)) {
                default_value = zt_parser_parse_expression(p);
            }

            zt_ast_node *where_clause = NULL;
            if (zt_parser_match(p, ZT_TOKEN_WHERE)) {
                zt_ast_node *cond = zt_parser_parse_expression(p);
                where_clause = zt_parser_ast_make(p, ZT_AST_WHERE_CLAUSE, name_tok.span);
                if (where_clause != NULL) {
                    where_clause->as.where_clause.param_name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);
                    where_clause->as.where_clause.condition = cond;
                }
            }

            zt_ast_node *param = zt_parser_ast_make(p, ZT_AST_PARAM, name_tok.span);
            if (param != NULL) {
                param->as.param.name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);
                param->as.param.type_node = type_node;
                param->as.param.default_value = default_value;
                param->as.param.where_clause = where_clause;
            }
            zt_ast_node_list_push(p->arena, &params, param);
        } while (zt_parser_match(p, ZT_TOKEN_COMMA));
    }

    zt_parser_expect(p, ZT_TOKEN_RPAREN);
    return params;
}

static zt_ast_node_list zt_parser_parse_type_params(zt_parser *p) {
    zt_ast_node_list type_params = zt_ast_node_list_make();
    if (!zt_parser_match(p, ZT_TOKEN_LT)) return type_params;

    if (!zt_parser_check(p, ZT_TOKEN_GT)) {
        do {
            zt_token name_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
            zt_ast_node *tp = zt_parser_ast_make(p, ZT_AST_TYPE_SIMPLE, name_tok.span);
            if (tp != NULL) {
                tp->as.type_simple.name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);
            }
            zt_ast_node_list_push(p->arena, &type_params, tp);
        } while (zt_parser_match(p, ZT_TOKEN_COMMA));
    }

    zt_parser_expect(p, ZT_TOKEN_GT);
    return type_params;
}

static zt_ast_node *zt_parser_parse_generic_constraint(zt_parser *p) {
    zt_token type_param_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
    zt_parser_expect(p, ZT_TOKEN_IS);
    zt_ast_node *trait_type = zt_parser_parse_type(p);
    zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_GENERIC_CONSTRAINT, type_param_tok.span);
    if (node != NULL) {
        node->as.generic_constraint.type_param_name = (char *)zt_string_pool_intern_len(p->pool, type_param_tok.text, type_param_tok.length);
        node->as.generic_constraint.trait_type = trait_type;
    }
    return node;
}

static zt_ast_node_list zt_parser_parse_generic_constraints(zt_parser *p) {
    zt_ast_node_list constraints = zt_ast_node_list_make();
    if (!zt_parser_match(p, ZT_TOKEN_WHERE)) {
        return constraints;
    }

    zt_ast_node_list_push(p->arena, &constraints, zt_parser_parse_generic_constraint(p));
    while (zt_parser_match(p, ZT_TOKEN_AND)) {
        zt_ast_node_list_push(p->arena, &constraints, zt_parser_parse_generic_constraint(p));
    }

    return constraints;
}

static zt_ast_node *zt_parser_parse_block_ex(zt_parser *p, int stop_at_case) {
    zt_ast_node *block;
    zt_source_span span = p->current.span;
    zt_ast_node_list stmts = zt_ast_node_list_make();
    if (!zt_parser_push_nesting(p, span, "block")) {
        return NULL;
    }

    while (!zt_parser_check(p, ZT_TOKEN_END) &&
           !zt_parser_check(p, ZT_TOKEN_ELSE) &&
           !(stop_at_case && zt_parser_check(p, ZT_TOKEN_CASE)) &&
           !(stop_at_case && zt_parser_check(p, ZT_TOKEN_DEFAULT)) &&
           !zt_parser_check(p, ZT_TOKEN_EOF)) {
        zt_leading_stash stash = zt_parser_stash_leading(p);
        zt_ast_node *stmt = zt_parser_parse_statement(p);
        zt_parser_reclaim_node_trailing(p, stmt);
        zt_parser_apply_stash_to_node(p, stmt, stash);
        if (stmt != NULL) {
            zt_ast_node_list_push(p->arena, &stmts, stmt);
        }
    }

    block = zt_parser_ast_make(p, ZT_AST_BLOCK, span);
    if (block != NULL) {
        block->as.block.statements = stmts;
    }
    zt_parser_pop_nesting(p);
    return block;
}

static zt_ast_node *zt_parser_parse_block(zt_parser *p) {
    return zt_parser_parse_block_ex(p, 0);
}

static zt_ast_node *zt_parser_parse_if_stmt(zt_parser *p) {
    zt_ast_node *node;
    zt_token if_tok = p->current;
    if (!zt_parser_push_nesting(p, if_tok.span, "if")) {
        return NULL;
    }
    zt_parser_advance(p);
    zt_ast_node *condition = zt_parser_parse_expression(p);
    zt_ast_node *then_block = zt_parser_parse_block(p);
    zt_ast_node *else_block = NULL;

    if (zt_parser_match(p, ZT_TOKEN_ELSE)) {
        if (zt_parser_check(p, ZT_TOKEN_IF)) {
            else_block = zt_parser_parse_if_stmt(p);
        } else {
            else_block = zt_parser_parse_block(p);
            zt_parser_expect(p, ZT_TOKEN_END);
        }
    } else {
        zt_parser_expect(p, ZT_TOKEN_END);
    }

    node = zt_parser_ast_make(p, ZT_AST_IF_STMT, if_tok.span);
    if (node != NULL) {
        node->as.if_stmt.condition = condition;
        node->as.if_stmt.then_block = then_block;
        node->as.if_stmt.else_block = else_block;
    }
    zt_parser_pop_nesting(p);
    return node;
}

static zt_ast_node *zt_parser_parse_while_stmt(zt_parser *p) {
    zt_token while_tok = p->current;
    zt_parser_advance(p);
    zt_ast_node *condition = zt_parser_parse_expression(p);
    zt_ast_node *body = zt_parser_parse_block(p);
    zt_parser_expect(p, ZT_TOKEN_END);

    zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_WHILE_STMT, while_tok.span);
    if (node != NULL) {
        node->as.while_stmt.condition = condition;
        node->as.while_stmt.body = body;
    }
    return node;
}

static zt_ast_node *zt_parser_parse_for_stmt(zt_parser *p) {
    zt_token for_tok = p->current;
    zt_parser_advance(p);
    zt_token item_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
    char *index_name = NULL;

    if (zt_parser_match(p, ZT_TOKEN_COMMA)) {
        zt_token index_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
        index_name = (char *)zt_string_pool_intern_len(p->pool, index_tok.text, index_tok.length);
    }
    zt_parser_expect(p, ZT_TOKEN_IN);
    zt_ast_node *iterable = zt_parser_parse_expression(p);
    zt_ast_node *body = zt_parser_parse_block(p);
    zt_parser_expect(p, ZT_TOKEN_END);

    zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_FOR_STMT, for_tok.span);
    if (node != NULL) {
        node->as.for_stmt.item_name = (char *)zt_string_pool_intern_len(p->pool, item_tok.text, item_tok.length);
        node->as.for_stmt.index_name = index_name;
        node->as.for_stmt.iterable = iterable;
        node->as.for_stmt.body = body;
    }
    return node;
}

static zt_ast_node *zt_parser_parse_repeat_stmt(zt_parser *p) {
    zt_token repeat_tok = p->current;
    zt_parser_advance(p);
    zt_ast_node *count = zt_parser_parse_expression(p);
    zt_parser_expect(p, ZT_TOKEN_TIMES);
    zt_ast_node *body = zt_parser_parse_block(p);
    zt_parser_expect(p, ZT_TOKEN_END);

    zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_REPEAT_STMT, repeat_tok.span);
    if (node != NULL) {
        node->as.repeat_stmt.count = count;
        node->as.repeat_stmt.body = body;
    }
    return node;
}

static zt_ast_node *zt_parser_parse_match_pattern(zt_parser *p) {
    zt_token tok = p->current;
    if (zt_parser_token_is_identifier_literal(&tok, "value", 5)) {
        zt_parser_fill_peek(p);
        if (p->peek.kind == ZT_TOKEN_IDENTIFIER) {
            zt_token name_tok = p->peek;
            zt_parser_advance(p);
            zt_parser_advance(p);
            zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_VALUE_BINDING, tok.span);
            if (node != NULL) {
                node->as.value_binding.name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);
                node->as.value_binding.type_node = NULL;
            }
            return node;
        }
    }
    return zt_parser_parse_expression(p);
}

static zt_ast_node *zt_parser_parse_match_stmt(zt_parser *p) {
    zt_ast_node *node;
    zt_token match_tok = p->current;
    if (!zt_parser_push_nesting(p, match_tok.span, "match")) {
        return NULL;
    }
    zt_parser_advance(p);
    zt_ast_node *subject = zt_parser_parse_expression(p);
    zt_ast_node_list cases = zt_ast_node_list_make();

    while (zt_parser_check(p, ZT_TOKEN_CASE)) {
        zt_token case_tok = p->current;
        zt_parser_advance(p);

        zt_ast_node_list patterns = zt_ast_node_list_make();
        int is_default = 0;

        if (zt_parser_match(p, ZT_TOKEN_DEFAULT)) {
            is_default = 1;
        } else {
            zt_ast_node_list_push(p->arena, &patterns, zt_parser_parse_match_pattern(p));
            while (zt_parser_match(p, ZT_TOKEN_COMMA)) {
                zt_ast_node_list_push(p->arena, &patterns, zt_parser_parse_match_pattern(p));
            }
        }

        zt_parser_expect(p, ZT_TOKEN_ARROW);
        zt_ast_node *body = zt_parser_parse_block_ex(p, 1);
        zt_ast_node *case_node = zt_parser_ast_make(p, ZT_AST_MATCH_CASE, case_tok.span);
        if (case_node != NULL) {
            case_node->as.match_case.patterns = patterns;
            case_node->as.match_case.body = body;
            case_node->as.match_case.is_default = is_default;
        }
        zt_ast_node_list_push(p->arena, &cases, case_node);
    }

    zt_parser_expect(p, ZT_TOKEN_END);

    node = zt_parser_ast_make(p, ZT_AST_MATCH_STMT, match_tok.span);
    if (node != NULL) {
        node->as.match_stmt.subject = subject;
        node->as.match_stmt.cases = cases;
    }
    zt_parser_pop_nesting(p);
    return node;
}

static zt_ast_node *zt_parser_parse_statement(zt_parser *p) {
    zt_token tok = p->current;

    if (tok.kind == ZT_TOKEN_CONST) {
        zt_parser_advance(p);
        zt_token name_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
        zt_parser_expect(p, ZT_TOKEN_COLON);
        zt_ast_node *type_node = zt_parser_parse_type(p);
        zt_ast_node *init_value = NULL;
        if (zt_parser_match(p, ZT_TOKEN_EQ)) {
            init_value = zt_parser_parse_expression(p);
        }
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_CONST_DECL, tok.span);
        if (node != NULL) {
            node->as.const_decl.name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);
            node->as.const_decl.type_node = type_node;
            node->as.const_decl.init_value = init_value;
            node->as.const_decl.is_public = 0;
            node->as.const_decl.is_module_level = 0;
        }
        return node;
    }

    if (tok.kind == ZT_TOKEN_VAR) {
        zt_parser_advance(p);
        zt_token name_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
        zt_parser_expect(p, ZT_TOKEN_COLON);
        zt_ast_node *type_node = zt_parser_parse_type(p);
        zt_ast_node *init_value = NULL;
        if (zt_parser_match(p, ZT_TOKEN_EQ)) {
            init_value = zt_parser_parse_expression(p);
        }
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_VAR_DECL, tok.span);
        if (node != NULL) {
            node->as.var_decl.name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);
            node->as.var_decl.type_node = type_node;
            node->as.var_decl.init_value = init_value;
            node->as.var_decl.is_public = 0;
            node->as.var_decl.is_module_level = 0;
        }
        return node;
    }

    if (tok.kind == ZT_TOKEN_RETURN) {
        zt_parser_advance(p);
        zt_ast_node *value = NULL;
        if (!zt_parser_check(p, ZT_TOKEN_END) &&
            !zt_parser_check(p, ZT_TOKEN_ELSE) &&
            !zt_parser_check(p, ZT_TOKEN_EOF)) {
            value = zt_parser_parse_expression(p);
        }
        zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_RETURN_STMT, tok.span);
        if (node != NULL) {
            node->as.return_stmt.value = value;
        }
        return node;
    }

    if (tok.kind == ZT_TOKEN_IF) {
        return zt_parser_parse_if_stmt(p);
    }

    if (tok.kind == ZT_TOKEN_WHILE) {
        return zt_parser_parse_while_stmt(p);
    }

    if (tok.kind == ZT_TOKEN_FOR) {
        return zt_parser_parse_for_stmt(p);
    }

    if (tok.kind == ZT_TOKEN_REPEAT) {
        return zt_parser_parse_repeat_stmt(p);
    }

    if (tok.kind == ZT_TOKEN_MATCH) {
        return zt_parser_parse_match_stmt(p);
    }

    if (tok.kind == ZT_TOKEN_BREAK) {
        zt_parser_advance(p);
        return zt_parser_ast_make(p, ZT_AST_BREAK_STMT, tok.span);
    }

    if (tok.kind == ZT_TOKEN_CONTINUE) {
        zt_parser_advance(p);
        return zt_parser_ast_make(p, ZT_AST_CONTINUE_STMT, tok.span);
    }

    zt_ast_node *expr = zt_parser_parse_expression(p);
    if (expr == NULL) return NULL;

    if (zt_parser_check(p, ZT_TOKEN_EQ) && expr->kind == ZT_AST_IDENT_EXPR) {
        zt_parser_advance(p);
        zt_ast_node *value = zt_parser_parse_expression(p);
        zt_ast_node *assign = zt_parser_ast_make(p, ZT_AST_ASSIGN_STMT, expr->span);
        if (assign != NULL) {
            assign->as.assign_stmt.name = expr->as.ident_expr.name;
            expr->as.ident_expr.name = NULL;
            assign->as.assign_stmt.value = value;
        }
        
        return assign;
    }

    if (zt_parser_check(p, ZT_TOKEN_EQ) && expr->kind == ZT_AST_INDEX_EXPR) {
        zt_parser_advance(p);
        zt_ast_node *value = zt_parser_parse_expression(p);
        zt_ast_node *assign = zt_parser_ast_make(p, ZT_AST_INDEX_ASSIGN_STMT, expr->span);
        if (assign != NULL) {
            assign->as.index_assign_stmt.object = expr->as.index_expr.object;
            assign->as.index_assign_stmt.index = expr->as.index_expr.index;
            expr->as.index_expr.object = NULL;
            expr->as.index_expr.index = NULL;
            assign->as.index_assign_stmt.value = value;
        }
        
        return assign;
    }

    if (zt_parser_check(p, ZT_TOKEN_EQ) && expr->kind == ZT_AST_FIELD_EXPR) {
        zt_parser_advance(p);
        zt_ast_node *value = zt_parser_parse_expression(p);
        zt_ast_node *assign = zt_parser_ast_make(p, ZT_AST_FIELD_ASSIGN_STMT, expr->span);
        if (assign != NULL) {
            assign->as.field_assign_stmt.object = expr->as.field_expr.object;
            assign->as.field_assign_stmt.field_name = expr->as.field_expr.field_name;
            expr->as.field_expr.object = NULL;
            expr->as.field_expr.field_name = NULL;
            assign->as.field_assign_stmt.value = value;
        }
        
        return assign;
    }

    zt_ast_node *stmt = zt_parser_ast_make(p, ZT_AST_EXPR_STMT, expr->span);
    if (stmt != NULL) {
        stmt->as.expr_stmt.expr = expr;
    } else {
        
    }
    return stmt;
}

static zt_ast_node *zt_parser_parse_func_decl(zt_parser *p, int is_public, int is_mutating) {
    zt_token func_tok = p->current;
    zt_parser_advance(p);
    zt_token name_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
    char *name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);

    zt_ast_node_list type_params = zt_parser_parse_type_params(p);
    zt_ast_node_list params = zt_parser_parse_params(p);

    zt_ast_node *return_type = NULL;
    if (zt_parser_match(p, ZT_TOKEN_ARROW)) {
        return_type = zt_parser_parse_type(p);
    }

    zt_ast_node_list constraints = zt_parser_parse_generic_constraints(p);

    /* Mark that we're entering function body */
    p->in_function_body = 1;
    zt_ast_node *body = zt_parser_parse_block(p);
    p->in_function_body = 0;
    
    zt_parser_expect(p, ZT_TOKEN_END);

    zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_FUNC_DECL, func_tok.span);
    if (node == NULL) {
        free(name);
        return NULL;
    }
    node->as.func_decl.name = name;
    node->as.func_decl.type_params = type_params;
    node->as.func_decl.params = params;
    node->as.func_decl.constraints = constraints;
    node->as.func_decl.return_type = return_type;
    node->as.func_decl.body = body;
    node->as.func_decl.is_public = is_public;
    node->as.func_decl.is_mutating = is_mutating;
    node->as.func_decl.is_test = 0;
    return node;
}

static zt_ast_node *zt_parser_parse_struct_field(zt_parser *p) {
    zt_token name_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
    zt_parser_expect(p, ZT_TOKEN_COLON);
    zt_ast_node *type_node = zt_parser_parse_type(p);
    zt_ast_node *default_value = NULL;
    zt_ast_node *where_clause = NULL;

    if (zt_parser_match(p, ZT_TOKEN_EQ)) {
        default_value = zt_parser_parse_expression(p);
    }

    if (zt_parser_check(p, ZT_TOKEN_WHERE)) {
        zt_parser_advance(p);
        zt_ast_node *cond = zt_parser_parse_expression(p);
        where_clause = zt_parser_ast_make(p, ZT_AST_WHERE_CLAUSE, name_tok.span);
        if (where_clause != NULL) {
            where_clause->as.where_clause.param_name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);
            where_clause->as.where_clause.condition = cond;
        }
    }

    zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_STRUCT_FIELD, name_tok.span);
    if (node != NULL) {
        node->as.struct_field.name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);
        node->as.struct_field.type_node = type_node;
        node->as.struct_field.default_value = default_value;
        node->as.struct_field.where_clause = where_clause;
    }
    return node;
}

static zt_ast_node *zt_parser_parse_struct_decl(zt_parser *p, int is_public) {
    zt_token struct_tok = p->current;
    zt_parser_advance(p);
    zt_token name_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
    zt_ast_node_list type_params = zt_parser_parse_type_params(p);
    zt_ast_node_list constraints = zt_parser_parse_generic_constraints(p);
    zt_ast_node_list fields = zt_ast_node_list_make();

    while (!zt_parser_check(p, ZT_TOKEN_END) && !zt_parser_check(p, ZT_TOKEN_EOF)) {
        zt_ast_node_list_push(p->arena, &fields, zt_parser_parse_struct_field(p));
    }
    zt_parser_expect(p, ZT_TOKEN_END);

    zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_STRUCT_DECL, struct_tok.span);
    if (node != NULL) {
        node->as.struct_decl.name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);
        node->as.struct_decl.type_params = type_params;
        node->as.struct_decl.constraints = constraints;
        node->as.struct_decl.fields = fields;
        node->as.struct_decl.is_public = is_public;
    }
    return node;
}

static zt_ast_node *zt_parser_parse_trait_method(zt_parser *p, int is_mutating) {
    zt_token func_tok = p->current;
    zt_parser_advance(p);
    zt_token name_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
    char *name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);

    zt_ast_node_list params = zt_parser_parse_params(p);
    zt_ast_node *return_type = NULL;
    if (zt_parser_match(p, ZT_TOKEN_ARROW)) {
        return_type = zt_parser_parse_type(p);
    }

    zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_TRAIT_METHOD, func_tok.span);
    if (node == NULL) {
        free(name);
        return NULL;
    }
    node->as.trait_method.name = name;
    node->as.trait_method.params = params;
    node->as.trait_method.return_type = return_type;
    node->as.trait_method.is_mutating = is_mutating;
    return node;
}

static zt_ast_node *zt_parser_parse_trait_decl(zt_parser *p, int is_public) {
    zt_token trait_tok = p->current;
    zt_parser_advance(p);
    zt_token name_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
    zt_ast_node_list type_params = zt_parser_parse_type_params(p);
    zt_ast_node_list constraints = zt_parser_parse_generic_constraints(p);
    zt_ast_node_list methods = zt_ast_node_list_make();

    while (!zt_parser_check(p, ZT_TOKEN_END) && !zt_parser_check(p, ZT_TOKEN_EOF)) {
        int is_mutating = 0;
        if (zt_parser_check(p, ZT_TOKEN_MUT)) {
            is_mutating = 1;
            zt_parser_advance(p);
        }
        if (zt_parser_check(p, ZT_TOKEN_FUNC)) {
            zt_ast_node_list_push(p->arena, &methods, zt_parser_parse_trait_method(p, is_mutating));
        } else {
            zt_parser_error_contextual(p, "expected func in trait body",
                "trait body can only contain method declarations (func)");
            zt_parser_advance(p);
            zt_parser_sync_to_member_or_end(p);
        }
    }
    zt_parser_expect(p, ZT_TOKEN_END);

    zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_TRAIT_DECL, trait_tok.span);
    if (node != NULL) {
        node->as.trait_decl.name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);
        node->as.trait_decl.type_params = type_params;
        node->as.trait_decl.constraints = constraints;
        node->as.trait_decl.methods = methods;
        node->as.trait_decl.is_public = is_public;
    }
    return node;
}

static zt_ast_node *zt_parser_parse_apply_decl(zt_parser *p) {
    zt_token apply_tok = p->current;
    zt_parser_advance(p);
    zt_token first_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
    zt_ast_node_list first_type_params = zt_ast_node_list_make();
    zt_ast_node_list trait_type_params = zt_ast_node_list_make();
    zt_ast_node_list target_type_params = zt_ast_node_list_make();
    char *trait_name = NULL;
    char *target_name = NULL;

    if (zt_parser_check(p, ZT_TOKEN_LT)) {
        first_type_params = zt_parser_parse_type_params(p);
    }

    if (zt_parser_match(p, ZT_TOKEN_TO)) {
        zt_token target_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
        trait_name = (char *)zt_string_pool_intern_len(p->pool, first_tok.text, first_tok.length);
        trait_type_params = first_type_params;
        target_name = (char *)zt_string_pool_intern_len(p->pool, target_tok.text, target_tok.length);
        if (zt_parser_check(p, ZT_TOKEN_LT)) {
            target_type_params = zt_parser_parse_type_params(p);
        }
    } else {
        target_name = (char *)zt_string_pool_intern_len(p->pool, first_tok.text, first_tok.length);
        target_type_params = first_type_params;
    }

    zt_ast_node_list constraints = zt_parser_parse_generic_constraints(p);
    zt_ast_node_list methods = zt_ast_node_list_make();

    while (!zt_parser_check(p, ZT_TOKEN_END) && !zt_parser_check(p, ZT_TOKEN_EOF)) {
        int is_public = 0;
        int is_mutating = 0;

        if (zt_parser_check(p, ZT_TOKEN_PUBLIC)) {
            is_public = 1;
            zt_parser_advance(p);
        }
        if (zt_parser_check(p, ZT_TOKEN_MUT)) {
            is_mutating = 1;
            zt_parser_advance(p);
        }

        if (zt_parser_check(p, ZT_TOKEN_FUNC)) {
            zt_ast_node_list_push(p->arena, &methods, zt_parser_parse_func_decl(p, is_public, is_mutating));
        } else {
            zt_parser_error_contextual(p, "expected func in apply body",
                "apply body can only contain method implementations (func)");
            zt_parser_advance(p);
            zt_parser_sync_to_member_or_end(p);
        }
    }
    zt_parser_expect(p, ZT_TOKEN_END);

    zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_APPLY_DECL, apply_tok.span);
    if (node != NULL) {
        node->as.apply_decl.trait_name = trait_name;
        node->as.apply_decl.trait_type_params = trait_type_params;
        node->as.apply_decl.target_name = target_name;
        node->as.apply_decl.target_type_params = target_type_params;
        node->as.apply_decl.constraints = constraints;
        node->as.apply_decl.methods = methods;
    }
    return node;
}

static zt_ast_node *zt_parser_parse_enum_decl(zt_parser *p, int is_public) {
    zt_token enum_tok = p->current;
    zt_parser_advance(p);
    zt_token name_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
    zt_ast_node_list type_params = zt_parser_parse_type_params(p);
    zt_ast_node_list constraints = zt_parser_parse_generic_constraints(p);
    zt_ast_node_list variants = zt_ast_node_list_make();

    while (!zt_parser_check(p, ZT_TOKEN_END) && !zt_parser_check(p, ZT_TOKEN_EOF)) {
        zt_token var_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
        zt_ast_node_list fields = zt_ast_node_list_make();

        if (zt_parser_check(p, ZT_TOKEN_LPAREN)) {
            zt_parser_advance(p);
            if (!zt_parser_check(p, ZT_TOKEN_RPAREN)) {
                do {
                    if (p->current.kind == ZT_TOKEN_IDENTIFIER) {
                        zt_parser_fill_peek(p);
                    }
                    if (p->current.kind == ZT_TOKEN_IDENTIFIER && p->has_peek && p->peek.kind == ZT_TOKEN_COLON) {
                        zt_token field_name_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
                        zt_parser_expect(p, ZT_TOKEN_COLON);
                        zt_ast_node *field_type = zt_parser_parse_type(p);
                        zt_ast_node *field = zt_parser_ast_make(p, ZT_AST_PARAM, field_name_tok.span);
                        if (field != NULL) {
                            field->as.param.name = (char *)zt_string_pool_intern_len(p->pool, field_name_tok.text, field_name_tok.length);
                            field->as.param.type_node = field_type;
                            field->as.param.default_value = NULL;
                        }
                        zt_ast_node_list_push(p->arena, &fields, field);
                    } else {
                        zt_ast_node_list_push(p->arena, &fields, zt_parser_parse_type(p));
                    }
                } while (zt_parser_match(p, ZT_TOKEN_COMMA));
            }
            zt_parser_expect(p, ZT_TOKEN_RPAREN);
        }

        zt_ast_node *variant = zt_parser_ast_make(p, ZT_AST_ENUM_VARIANT, var_tok.span);
        if (variant != NULL) {
            variant->as.enum_variant.name = (char *)zt_string_pool_intern_len(p->pool, var_tok.text, var_tok.length);
            variant->as.enum_variant.fields = fields;
        }
        zt_ast_node_list_push(p->arena, &variants, variant);
    }
    zt_parser_expect(p, ZT_TOKEN_END);

    zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_ENUM_DECL, enum_tok.span);
    if (node != NULL) {
        node->as.enum_decl.name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);
        node->as.enum_decl.type_params = type_params;
        node->as.enum_decl.constraints = constraints;
        node->as.enum_decl.variants = variants;
        node->as.enum_decl.is_public = is_public;
    }
    return node;
}

static zt_ast_node *zt_parser_parse_extern_func(zt_parser *p) {
    zt_token func_tok = p->current;
    zt_parser_advance(p);
    zt_token name_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
    char *name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);
    zt_ast_node_list params = zt_parser_parse_params(p);
    zt_ast_node *return_type = NULL;
    if (zt_parser_match(p, ZT_TOKEN_ARROW)) {
        return_type = zt_parser_parse_type(p);
    }

    zt_ast_node *func = zt_parser_ast_make(p, ZT_AST_FUNC_DECL, func_tok.span);
    if (func == NULL) {
        free(name);
        return NULL;
    }
    func->as.func_decl.name = name;
    func->as.func_decl.type_params = zt_ast_node_list_make();
    func->as.func_decl.params = params;
    func->as.func_decl.return_type = return_type;
    func->as.func_decl.body = NULL;
    func->as.func_decl.is_public = 1;
    func->as.func_decl.is_mutating = 0;
    func->as.func_decl.is_test = 0;
    return func;
}

static zt_ast_node *zt_parser_parse_extern_decl(zt_parser *p) {
    zt_token extern_tok = p->current;
    zt_parser_advance(p);
    zt_token binding_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
    zt_ast_node_list functions = zt_ast_node_list_make();

    while (!zt_parser_check(p, ZT_TOKEN_END) && !zt_parser_check(p, ZT_TOKEN_EOF)) {
        if (zt_parser_check(p, ZT_TOKEN_PUBLIC)) {
            zt_parser_advance(p);
        }
        if (zt_parser_check(p, ZT_TOKEN_FUNC)) {
            zt_ast_node_list_push(p->arena, &functions, zt_parser_parse_extern_func(p));
        } else {
            zt_parser_error_contextual(p, "expected func in extern body",
                "extern body can only contain function declarations (func)");
            zt_parser_advance(p);
            zt_parser_sync_to_member_or_end(p);
        }
    }
    zt_parser_expect(p, ZT_TOKEN_END);

    zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_EXTERN_DECL, extern_tok.span);
    if (node != NULL) {
        node->as.extern_decl.binding = (char *)zt_string_pool_intern_len(p->pool, binding_tok.text, binding_tok.length);
        node->as.extern_decl.functions = functions;
        node->as.extern_decl.is_public = 0;
    }
    return node;
}

static zt_ast_node *zt_parser_parse_declaration(zt_parser *p) {
    int is_public = 0;
    int is_test_attr = 0;
    int consumed = 1;

    while (consumed) {
        consumed = 0;

        if (zt_parser_check(p, ZT_TOKEN_PUBLIC) && !is_public) {
            is_public = 1;
            zt_parser_advance(p);
            consumed = 1;
            continue;
        }

        if (zt_parser_check(p, ZT_TOKEN_ATTR)) {
            zt_parser_advance(p);
            {
                zt_token attr_name = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
                if (zt_parser_token_is_identifier_literal(&attr_name, "test", 4)) {
                    is_test_attr = 1;
                } else {
                    zt_diag_list_add(&p->result->diagnostics, ZT_DIAG_SYNTAX_ERROR, attr_name.span,
                        "unsupported attr '%.*s' (MVP supports only 'attr test')",
                        (int)attr_name.length,
                        attr_name.text != NULL ? attr_name.text : "");
                }
            }
            consumed = 1;
            continue;
        }
    }

    if (p->current.kind == ZT_TOKEN_MUT) {
        zt_parser_error_at(p, "mut func is only allowed inside trait/apply method declarations");
        zt_parser_advance(p);
    }

    switch (p->current.kind) {
        case ZT_TOKEN_FUNC: {
            zt_ast_node *func = zt_parser_parse_func_decl(p, is_public, 0);
            if (func != NULL) {
                func->as.func_decl.is_test = is_test_attr;
            }
            return func;
        }
        case ZT_TOKEN_STRUCT:
            if (is_test_attr) {
                zt_parser_error_at(p, "attr test is only valid on func declarations");
            }
            return zt_parser_parse_struct_decl(p, is_public);
        case ZT_TOKEN_TRAIT:
            if (is_test_attr) {
                zt_parser_error_at(p, "attr test is only valid on func declarations");
            }
            return zt_parser_parse_trait_decl(p, is_public);
        case ZT_TOKEN_APPLY:
            if (is_test_attr) {
                zt_parser_error_at(p, "attr test is only valid on func declarations");
            }
            return zt_parser_parse_apply_decl(p);
        case ZT_TOKEN_ENUM:
            if (is_test_attr) {
                zt_parser_error_at(p, "attr test is only valid on func declarations");
            }
            return zt_parser_parse_enum_decl(p, is_public);
        case ZT_TOKEN_EXTERN:
            if (is_test_attr) {
                zt_parser_error_at(p, "attr test is only valid on func declarations");
            }
            return zt_parser_parse_extern_decl(p);
        case ZT_TOKEN_CONST: {
            zt_token const_tok = p->current;
            zt_parser_advance(p);
            zt_token name_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
            zt_parser_expect(p, ZT_TOKEN_COLON);
            zt_ast_node *type_node = zt_parser_parse_type(p);
            zt_ast_node *init_value = NULL;
            zt_ast_node *node;
            if (is_test_attr) {
                zt_parser_error_at(p, "attr test is only valid on func declarations");
            }
            if (zt_parser_match(p, ZT_TOKEN_EQ)) {
                init_value = zt_parser_parse_expression(p);
            }
            node = zt_parser_ast_make(p, ZT_AST_CONST_DECL, const_tok.span);
            if (node != NULL) {
                node->as.const_decl.name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);
                node->as.const_decl.type_node = type_node;
                node->as.const_decl.init_value = init_value;
                node->as.const_decl.is_public = is_public;
                node->as.const_decl.is_module_level = 1;
            }
            return node;
        }
        case ZT_TOKEN_VAR: {
            zt_token var_tok = p->current;
            zt_parser_advance(p);
            zt_token name_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
            zt_parser_expect(p, ZT_TOKEN_COLON);
            zt_ast_node *type_node = zt_parser_parse_type(p);
            zt_ast_node *init_value = NULL;
            zt_ast_node *node;
            if (is_test_attr) {
                zt_parser_error_at(p, "attr test is only valid on func declarations");
            }
            if (zt_parser_match(p, ZT_TOKEN_EQ)) {
                init_value = zt_parser_parse_expression(p);
            }
            node = zt_parser_ast_make(p, ZT_AST_VAR_DECL, var_tok.span);
            if (node != NULL) {
                node->as.var_decl.name = (char *)zt_string_pool_intern_len(p->pool, name_tok.text, name_tok.length);
                node->as.var_decl.type_node = type_node;
                node->as.var_decl.init_value = init_value;
                node->as.var_decl.is_public = is_public;
                node->as.var_decl.is_module_level = 1;
            }
            return node;
        }
        default:
            zt_parser_error_contextual(p, "expected declaration",
                "expected top-level declaration (func, struct, trait, apply, enum, extern, const, or var)");
            zt_parser_advance(p);
            return NULL;
    }
}

static zt_ast_node *zt_parser_parse_import(zt_parser *p) {
    zt_token import_tok = p->current;
    zt_parser_advance(p);

    char path_buf[ZT_PARSER_MAX_NAME_PATH_LEN + 1];
    size_t path_len = 0;
    int overflowed = 0;

    zt_token part = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
    path_buf[0] = '\0';
    zt_parser_append_to_name_path(p, path_buf, sizeof(path_buf), &path_len, part.text, part.length, part.span, &overflowed, "import path");

    while (zt_parser_match(p, ZT_TOKEN_DOT)) {
        zt_parser_append_char_to_name_path(p, path_buf, sizeof(path_buf), &path_len, '.', part.span, &overflowed, "import path");
        part = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
        zt_parser_append_to_name_path(p, path_buf, sizeof(path_buf), &path_len, part.text, part.length, part.span, &overflowed, "import path");
    }

    char *alias = NULL;
    if (zt_parser_match(p, ZT_TOKEN_AS)) {
        zt_token alias_tok = zt_parser_expect(p, ZT_TOKEN_IDENTIFIER);
        alias = (char *)zt_string_pool_intern_len(p->pool, alias_tok.text, alias_tok.length);
    }

    zt_ast_node *node = zt_parser_ast_make(p, ZT_AST_IMPORT_DECL, import_tok.span);
    if (node != NULL) {
        node->as.import_decl.path = (char *)zt_string_pool_intern(p->pool, path_buf);
        node->as.import_decl.alias = alias;
    }
    return node;
}

zt_parser_result zt_parse(zt_arena *arena, zt_string_pool *pool, const char *source_name, const char *source_text, size_t source_length) {
    zt_parser_result result;
    result.root = NULL;
    result.diagnostics = zt_diag_list_make();
    
    if (source_name == NULL || source_text == NULL) return result;

    zt_lexer *lexer = zt_lexer_make(source_name, source_text, source_length);
    if (lexer == NULL) return result;

    zt_lexer_set_diagnostics(lexer, &result.diagnostics);

    zt_parser parser;
    parser.lexer = lexer;
    memset(&parser.peek, 0, sizeof(parser.peek));
    parser.has_peek = 0;
    parser.result = &result;
    parser.arena = arena;
    parser.pool = pool;
    parser.in_function_body = 0;
    parser.in_type_position = 0;
    parser.in_expression_position = 0;
    parser.pending_comments = NULL;
    parser.pending_comment_count = 0;
    parser.pending_comment_capacity = 0;
    parser.nesting_depth = 0;
    parser.current = zt_parser_next_non_comment_token(&parser);

    zt_ast_node_list imports = zt_ast_node_list_make();
    zt_ast_node_list declarations = zt_ast_node_list_make();
    char *module_name = NULL;

    if (zt_parser_check(&parser, ZT_TOKEN_NAMESPACE)) {
        zt_parser_advance(&parser);
        zt_token ns = zt_parser_expect(&parser, ZT_TOKEN_IDENTIFIER);

        char ns_buf[ZT_PARSER_MAX_NAME_PATH_LEN + 1];
        size_t ns_len = 0;
        int overflowed = 0;

        ns_buf[0] = '\0';
        zt_parser_append_to_name_path(&parser, ns_buf, sizeof(ns_buf), &ns_len, ns.text, ns.length, ns.span, &overflowed, "namespace");

        while (zt_parser_match(&parser, ZT_TOKEN_DOT)) {
            zt_parser_append_char_to_name_path(&parser, ns_buf, sizeof(ns_buf), &ns_len, '.', ns.span, &overflowed, "namespace");
            zt_token part = zt_parser_expect(&parser, ZT_TOKEN_IDENTIFIER);
            zt_parser_append_to_name_path(&parser, ns_buf, sizeof(ns_buf), &ns_len, part.text, part.length, part.span, &overflowed, "namespace");
        }
        module_name = (char *)zt_string_pool_intern(pool, ns_buf);
    }

    while (zt_parser_check(&parser, ZT_TOKEN_IMPORT)) {
        zt_leading_stash stash = zt_parser_stash_leading(&parser);
        zt_ast_node *import = zt_parser_parse_import(&parser);
        zt_parser_reclaim_node_trailing(&parser, import);
        zt_parser_apply_stash_to_node(&parser, import, stash);
        zt_ast_node_list_push(parser.arena, &imports, import);
    }

    while (!zt_parser_check(&parser, ZT_TOKEN_EOF)) {
        if (zt_parser_check(&parser, ZT_TOKEN_PUBLIC) ||
            zt_parser_check(&parser, ZT_TOKEN_ATTR) ||
            zt_parser_check(&parser, ZT_TOKEN_MUT) ||
            zt_parser_check(&parser, ZT_TOKEN_FUNC) ||
            zt_parser_check(&parser, ZT_TOKEN_STRUCT) ||
            zt_parser_check(&parser, ZT_TOKEN_TRAIT) ||
            zt_parser_check(&parser, ZT_TOKEN_APPLY) ||
            zt_parser_check(&parser, ZT_TOKEN_ENUM) ||
            zt_parser_check(&parser, ZT_TOKEN_EXTERN) ||
            zt_parser_check(&parser, ZT_TOKEN_CONST) ||
            zt_parser_check(&parser, ZT_TOKEN_VAR)) {
            zt_leading_stash stash = zt_parser_stash_leading(&parser);
            zt_ast_node *decl = zt_parser_parse_declaration(&parser);
            zt_parser_reclaim_node_trailing(&parser, decl);
            zt_parser_apply_stash_to_node(&parser, decl, stash);
            if (decl != NULL) {
                zt_ast_node_list_push(parser.arena, &declarations, decl);
            }
        } else {
            zt_parser_error_at(&parser, "expected declaration");
            zt_parser_sync_to_declaration(&parser);
            if (!zt_parser_check(&parser, ZT_TOKEN_EOF) && !zt_parser_is_declaration_start(parser.current.kind)) {
                zt_parser_advance(&parser);
            }
        }
    }

    zt_ast_node *file = zt_parser_ast_make(&parser, ZT_AST_FILE, zt_source_span_make(source_name, 1, 1, 1));
    if (file != NULL) {
        file->as.file.module_name = module_name;
        file->as.file.imports = imports;
        file->as.file.declarations = declarations;
    }
    result.root = file;

    zt_lexer_dispose(lexer);
    return result;
}

void zt_parser_result_dispose(zt_parser_result *result) {
    if (result == NULL) return;
    if (result->root != NULL) {
        // disposed by arena
    }
    zt_diag_list_dispose(&result->diagnostics);
}

