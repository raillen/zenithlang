#include "compiler/frontend/lexer/token.h"
#include "compiler/frontend/lexer/lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT_EQ(actual, expected, msg) do { \
    tests_run++; \
    if ((actual) == (expected)) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected %d, got %d\n", msg, (int)(expected), (int)(actual)); } \
} while(0)

#define ASSERT_STR_EQ(actual_tok, expected, msg) do { \
    tests_run++; \
    if (strlen(expected) == (actual_tok).length && strncmp((actual_tok).text, (expected), (actual_tok).length) == 0) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected \"%s\", got \"%.*s\"\n", msg, (expected), (int)(actual_tok).length, (actual_tok).text); } \
} while(0)

static void test_keywords(void) {
    const char *source = "namespace import as func end const var return if else while for in repeat times break continue struct trait apply to enum match case default public where is and or not true false none success error optional result list map extern void mut self";
    zt_lexer *lexer = zt_lexer_make("test", source, strlen(source));
    zt_token t;

    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_NAMESPACE, "namespace");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IMPORT, "import");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_AS, "as");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_FUNC, "func");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_END, "end");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_CONST, "const");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_VAR, "var");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_RETURN, "return");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IF, "if");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_ELSE, "else");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_WHILE, "while");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_FOR, "for");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IN, "in");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_REPEAT, "repeat");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_TIMES, "times");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_BREAK, "break");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_CONTINUE, "continue");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_STRUCT, "struct");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_TRAIT, "trait");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_APPLY, "apply");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_TO, "to");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_ENUM, "enum");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_MATCH, "match");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_CASE, "case");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_DEFAULT, "default");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_PUBLIC, "public");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_WHERE, "where");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IS, "is");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_AND, "and keyword");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_OR, "or keyword");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_NOT, "not keyword");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_TRUE, "true");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_FALSE, "false");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_NONE, "none");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_SUCCESS, "success");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_KW_ERROR, "error");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_OPTIONAL, "optional");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_RESULT, "result");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_LIST, "list");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_MAP, "map");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_EXTERN, "extern");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_VOID, "void");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_MUT, "mut");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_SELF, "self");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_EOF, "eof after keywords");

    zt_lexer_dispose(lexer);
}

static void test_identifiers(void) {
    const char *source = "hello _bar x123 MyType";
    zt_lexer *lexer = zt_lexer_make("test", source, strlen(source));
    zt_token t;

    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "hello id"); ASSERT_STR_EQ(t, "hello", "hello text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "_bar id"); ASSERT_STR_EQ(t, "_bar", "_bar text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "x123 id"); ASSERT_STR_EQ(t, "x123", "x123 text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "MyType id"); ASSERT_STR_EQ(t, "MyType", "MyType text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_EOF, "eof after ids");

    zt_lexer_dispose(lexer);
}

static void test_integers(void) {
    const char *source = "0 42 1_000 0xFF 0b1010";
    zt_lexer *lexer = zt_lexer_make("test", source, strlen(source));
    zt_token t;

    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_INT_LITERAL, "0"); ASSERT_STR_EQ(t, "0", "0 text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_INT_LITERAL, "42"); ASSERT_STR_EQ(t, "42", "42 text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_INT_LITERAL, "1_000"); ASSERT_STR_EQ(t, "1_000", "1_000 text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_INT_LITERAL, "0xFF"); ASSERT_STR_EQ(t, "0xFF", "0xFF text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_INT_LITERAL, "0b1010"); ASSERT_STR_EQ(t, "0b1010", "0b1010 text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_EOF, "eof after ints");

    zt_lexer_dispose(lexer);
}

static void test_floats(void) {
    const char *source = "3.14 1e10 2.5e-3";
    zt_lexer *lexer = zt_lexer_make("test", source, strlen(source));
    zt_token t;

    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_FLOAT_LITERAL, "3.14"); ASSERT_STR_EQ(t, "3.14", "3.14 text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_FLOAT_LITERAL, "1e10"); ASSERT_STR_EQ(t, "1e10", "1e10 text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_FLOAT_LITERAL, "2.5e-3"); ASSERT_STR_EQ(t, "2.5e-3", "2.5e-3 text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_EOF, "eof after floats");

    zt_lexer_dispose(lexer);
}

static void test_strings(void) {
    const char *source = "\"hello\" \"with\\nescapes\"";
    zt_lexer *lexer = zt_lexer_make("test", source, strlen(source));
    zt_token t;

    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_STRING_LITERAL, "hello string"); ASSERT_STR_EQ(t, "\"hello\"", "hello string text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_STRING_LITERAL, "escape string"); ASSERT_STR_EQ(t, "\"with\\nescapes\"", "escape string text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_EOF, "eof after strings");

    zt_lexer_dispose(lexer);
}

static void test_triple_quoted(void) {
    const char *source = "\"\"\"hello\nworld\"\"\"";
    zt_lexer *lexer = zt_lexer_make("test", source, strlen(source));
    zt_token t;

    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_TRIPLE_QUOTED_TEXT, "triple quoted"); ASSERT_STR_EQ(t, "\"\"\"hello\nworld\"\"\"", "triple text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_EOF, "eof after triple");

    zt_lexer_dispose(lexer);
}

static void test_operators(void) {
    const char *source = "+ - * / % == != < <= > >= -> . .. , : ? ! = ( ) [ ] { }";
    zt_lexer *lexer = zt_lexer_make("test", source, strlen(source));
    zt_token t;

    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_PLUS, "+");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_MINUS, "-");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_STAR, "*");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_SLASH, "/");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_PERCENT, "%");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_EQEQ, "==");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_NEQ, "!=");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_LT, "<");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_LTE, "<=");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_GT, ">");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_GTE, ">=");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_ARROW, "->");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_DOT, ".");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_DOTDOT, "..");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_COMMA, ",");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_COLON, ":");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_QUESTION, "?");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_BANG, "!");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_EQ, "=");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_LPAREN, "(");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_RPAREN, ")");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_LBRACKET, "[");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_RBRACKET, "]");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_LBRACE, "{");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_RBRACE, "}");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_EOF, "eof after ops");

    zt_lexer_dispose(lexer);
}

static void test_line_comments(void) {
    const char *source = "x -- this is a comment\ny";
    zt_lexer *lexer = zt_lexer_make("test", source, strlen(source));
    zt_token t;

    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "x before comment"); ASSERT_STR_EQ(t, "x", "x text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_COMMENT, "comment token"); ASSERT_STR_EQ(t, "-- this is a comment", "comment text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "y after comment"); ASSERT_STR_EQ(t, "y", "y text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_EOF, "eof after comment");

    zt_lexer_dispose(lexer);
}

static void test_block_comments(void) {
    const char *source = "x --- block comment --- y";
    zt_lexer *lexer = zt_lexer_make("test", source, strlen(source));
    zt_token t;

    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "x before block"); ASSERT_STR_EQ(t, "x", "x text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_COMMENT, "comment token"); ASSERT_STR_EQ(t, "--- block comment ---", "comment text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "y after block"); ASSERT_STR_EQ(t, "y", "y text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_EOF, "eof after block");

    zt_lexer_dispose(lexer);
}

static void test_multiline_block_comments(void) {
    const char *source = "x ---\nmulti\nline\n---\ny";
    zt_lexer *lexer = zt_lexer_make("test", source, strlen(source));
    zt_token t;

    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "x before multiline"); ASSERT_STR_EQ(t, "x", "x text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_COMMENT, "comment token"); ASSERT_STR_EQ(t, "---\nmulti\nline\n---", "comment text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "y after multiline"); ASSERT_STR_EQ(t, "y", "y text");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_EOF, "eof after multiline");

    zt_lexer_dispose(lexer);
}

static void test_spans(void) {
    const char *source = "namespace app";
    zt_lexer *lexer = zt_lexer_make("test.zt", source, strlen(source));
    zt_token t;

    t = zt_lexer_next_token(lexer);
    ASSERT_EQ(t.kind, ZT_TOKEN_NAMESPACE, "namespace span");
    ASSERT_EQ((int)t.span.line, 1, "namespace line");
    ASSERT_EQ((int)t.span.column_start, 1, "namespace col start");

    t = zt_lexer_next_token(lexer);
    ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "app span");
    ASSERT_EQ((int)t.span.line, 1, "app line");
    ASSERT_STR_EQ(t, "app", "app text");

    zt_lexer_dispose(lexer);
}

static void test_small_program(void) {
    const char *source = "namespace app.main\n\nfunc greet(name: text) -> text\n    return name\nend";
    zt_lexer *lexer = zt_lexer_make("test", source, strlen(source));
    zt_token t;

    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_NAMESPACE, "ns");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "app"); ASSERT_STR_EQ(t, "app", "app");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_DOT, "dot");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "main"); ASSERT_STR_EQ(t, "main", "main");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_FUNC, "func");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "greet"); ASSERT_STR_EQ(t, "greet", "greet");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_LPAREN, "(");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "name"); ASSERT_STR_EQ(t, "name", "name");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_COLON, ":");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "text"); ASSERT_STR_EQ(t, "text", "text_type");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_RPAREN, ")");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_ARROW, "->");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "text_ret"); ASSERT_STR_EQ(t, "text", "text_ret");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_RETURN, "return");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_IDENTIFIER, "name_ret"); ASSERT_STR_EQ(t, "name", "name_ret");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_END, "end");
    t = zt_lexer_next_token(lexer); ASSERT_EQ(t.kind, ZT_TOKEN_EOF, "eof");

    zt_lexer_dispose(lexer);
}

int main(void) {
    test_keywords();
    test_identifiers();
    test_integers();
    test_floats();
    test_strings();
    test_triple_quoted();
    test_operators();
    test_line_comments();
    test_block_comments();
    test_multiline_block_comments();
    test_spans();
    test_small_program();

    printf("Lexer tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}

