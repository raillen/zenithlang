#include "runtime/c/zenith_rt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void assert_true(const char *name, int condition) {
    if (!condition) {
        fprintf(stderr, "falha no teste %s\n", name);
        exit(1);
    }
}

static void assert_text_equals(const char *name, const zt_text *value, const char *expected) {
    const char *actual = zt_text_data(value);

    if (strcmp(actual, expected) != 0) {
        fprintf(stderr, "falha no teste %s\n", name);
        fprintf(stderr, "esperado: %s\n", expected);
        fprintf(stderr, "recebido: %s\n", actual);
        exit(1);
    }
}

static void test_text_index_utf8_codepoint(void) {
    const char utf8_raw[] = {
        'A',
        (char)0xC3, (char)0xA9,
        (char)0xE4, (char)0xB8, (char)0xAD,
        (char)0xF0, (char)0x9F, (char)0x99, (char)0x82,
        'B'
    };
    const char expected_accent[] = { (char)0xC3, (char)0xA9, '\0' };
    const char expected_cjk[] = { (char)0xE4, (char)0xB8, (char)0xAD, '\0' };
    const char expected_emoji[] = { (char)0xF0, (char)0x9F, (char)0x99, (char)0x82, '\0' };
    zt_text *value = zt_text_from_utf8(utf8_raw, sizeof(utf8_raw));
    zt_text *accent = zt_text_index(value, 1);
    zt_text *cjk = zt_text_index(value, 2);
    zt_text *emoji = zt_text_index(value, 3);

    assert_true("text_len_utf8_whole", zt_text_len(value) == 5);
    assert_text_equals("text_index_utf8_accent", accent, expected_accent);
    assert_text_equals("text_index_utf8_cjk", cjk, expected_cjk);
    assert_text_equals("text_index_utf8_emoji", emoji, expected_emoji);
    assert_true("text_len_utf8_emoji", zt_text_len(emoji) == 1);

    zt_release(emoji);
    zt_release(cjk);
    zt_release(accent);
    zt_release(value);
}

static void test_text_slice_utf8_codepoint(void) {
    const char utf8_raw[] = {
        'A',
        (char)0xC3, (char)0xA9,
        (char)0xE4, (char)0xB8, (char)0xAD,
        (char)0xF0, (char)0x9F, (char)0x99, (char)0x82,
        'B'
    };
    const char expected_middle[] = {
        (char)0xC3, (char)0xA9,
        (char)0xE4, (char)0xB8, (char)0xAD,
        (char)0xF0, (char)0x9F, (char)0x99, (char)0x82,
        '\0'
    };
    const char expected_tail[] = {
        (char)0xF0, (char)0x9F, (char)0x99, (char)0x82,
        'B',
        '\0'
    };
    zt_text *value = zt_text_from_utf8(utf8_raw, sizeof(utf8_raw));
    zt_text *middle = zt_text_slice(value, 1, 3);
    zt_text *tail = zt_text_slice(value, 3, -1);
    zt_text *empty = zt_text_slice(value, 20, -1);

    assert_text_equals("text_slice_utf8_middle", middle, expected_middle);
    assert_text_equals("text_slice_utf8_tail", tail, expected_tail);
    assert_text_equals("text_slice_utf8_empty", empty, "");
    assert_true("text_slice_utf8_middle_len", zt_text_len(middle) == 3);
    assert_true("text_slice_utf8_tail_len", zt_text_len(tail) == 2);
    assert_true("text_slice_utf8_empty_len", zt_text_len(empty) == 0);

    zt_release(empty);
    zt_release(tail);
    zt_release(middle);
    zt_release(value);
}

int main(void) {
    test_text_index_utf8_codepoint();
    test_text_slice_utf8_codepoint();
    printf("Runtime UTF-8 text slice tests OK\n");
    return 0;
}
