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

static void assert_bytes_equals(
    const char *name,
    const zt_bytes *value,
    const uint8_t *expected,
    size_t expected_len) {
    if (value->len != expected_len) {
        fprintf(stderr, "falha no teste %s\n", name);
        fprintf(stderr, "tamanho esperado: %zu\n", expected_len);
        fprintf(stderr, "tamanho recebido: %zu\n", value->len);
        exit(1);
    }

    if (expected_len > 0 && memcmp(value->data, expected, expected_len) != 0) {
        fprintf(stderr, "falha no teste %s\n", name);
        fprintf(stderr, "conteudo de bytes inesperado\n");
        exit(1);
    }
}

static void test_text_and_bytes_boundary_copy(void) {
    static const uint8_t payload[] = {1u, 2u, 3u, 4u};
    static const uint8_t expected_copy[] = {1u, 2u, 3u, 4u};
    zt_text *text = zt_text_from_utf8_literal("alpha");
    zt_text *text_copy = zt_thread_boundary_copy_text(text);
    zt_bytes *bytes = zt_bytes_from_array(payload, sizeof(payload));
    zt_bytes *bytes_copy = zt_thread_boundary_copy_bytes(bytes);

    assert_true("thread_copy_text_distinct", text_copy != text);
    assert_true("thread_copy_text_data_distinct", text_copy->data != text->data);
    assert_text_equals("thread_copy_text_value", text_copy, "alpha");

    assert_true("thread_copy_bytes_distinct", bytes_copy != bytes);
    assert_true("thread_copy_bytes_data_distinct", bytes_copy->data != bytes->data);
    assert_bytes_equals("thread_copy_bytes_value", bytes_copy, expected_copy, sizeof(expected_copy));

    bytes->data[0] = 9u;
    assert_bytes_equals(
        "thread_copy_bytes_after_original_mutation",
        bytes_copy,
        expected_copy,
        sizeof(expected_copy));

    zt_release(text);
    assert_text_equals("thread_copy_text_after_original_release", text_copy, "alpha");

    zt_release(text_copy);
    zt_release(bytes);
    zt_release(bytes_copy);
}

static void test_list_i64_boundary_copy(void) {
    zt_int items[] = {1, 2, 3};
    zt_list_i64 *list = zt_list_i64_from_array(items, 3);
    zt_list_i64 *copy = zt_thread_boundary_copy_list_i64(list);

    assert_true("thread_copy_list_i64_distinct", copy != list);
    assert_true("thread_copy_list_i64_len", zt_list_i64_len(copy) == 3);
    assert_true("thread_copy_list_i64_first", zt_list_i64_get(copy, 0) == 1);

    zt_list_i64_set(list, 0, 99);

    assert_true("thread_copy_list_i64_original_mutated", zt_list_i64_get(list, 0) == 99);
    assert_true("thread_copy_list_i64_copy_stable", zt_list_i64_get(copy, 0) == 1);

    zt_release(list);
    zt_release(copy);
}

static void test_list_text_boundary_copy(void) {
    zt_text *first = zt_text_from_utf8_literal("one");
    zt_text *second = zt_text_from_utf8_literal("two");
    zt_text *replacement = zt_text_from_utf8_literal("changed");
    zt_text *items[] = {first, second};
    zt_list_text *list = zt_list_text_from_array(items, 2);
    zt_list_text *copy = zt_thread_boundary_copy_list_text(list);
    zt_text *orig0;
    zt_text *copy0;

    assert_true("thread_copy_list_text_distinct", copy != list);
    assert_true("thread_copy_list_text_len", zt_list_text_len(copy) == 2);

    zt_list_text_set(list, 0, replacement);

    orig0 = zt_list_text_get(list, 0);
    copy0 = zt_list_text_get(copy, 0);

    assert_text_equals("thread_copy_list_text_original_mutated", orig0, "changed");
    assert_text_equals("thread_copy_list_text_copy_stable", copy0, "one");
    assert_true("thread_copy_list_text_item_distinct", orig0 != copy0);

    zt_release(orig0);
    zt_release(copy0);
    zt_release(list);
    zt_release(copy);
    zt_release(replacement);
    zt_release(first);
    zt_release(second);
}

static void test_map_boundary_copy(void) {
    zt_map_text_text *map = zt_map_text_text_new();
    zt_text *key = zt_text_from_utf8_literal("mode");
    zt_text *value = zt_text_from_utf8_literal("alpha");
    zt_text *updated = zt_text_from_utf8_literal("beta");
    zt_map_text_text *copy;
    zt_text *original_value;
    zt_text *copy_value;
    zt_text *copy_value_after_update;

    zt_map_text_text_set(map, key, value);
    copy = zt_thread_boundary_copy_map_text_text(map);

    original_value = zt_map_text_text_get(map, key);
    copy_value = zt_map_text_text_get(copy, key);

    assert_true("thread_copy_map_distinct", copy != map);
    assert_true("thread_copy_map_len", zt_map_text_text_len(copy) == 1);
    assert_true("thread_copy_map_value_distinct", original_value != copy_value);
    assert_text_equals("thread_copy_map_original_value", original_value, "alpha");
    assert_text_equals("thread_copy_map_copy_value", copy_value, "alpha");

    zt_release(original_value);
    zt_release(copy_value);

    zt_map_text_text_set(map, key, updated);

    original_value = zt_map_text_text_get(map, key);
    copy_value_after_update = zt_map_text_text_get(copy, key);

    assert_text_equals("thread_copy_map_original_mutated", original_value, "beta");
    assert_text_equals("thread_copy_map_copy_stable", copy_value_after_update, "alpha");

    zt_release(original_value);
    zt_release(copy_value_after_update);
    zt_release(map);
    zt_release(copy);
    zt_release(updated);
    zt_release(value);
    zt_release(key);
}

int main(void) {
    test_text_and_bytes_boundary_copy();
    test_list_i64_boundary_copy();
    test_list_text_boundary_copy();
    test_map_boundary_copy();
    printf("Runtime thread boundary copy tests OK\n");
    return 0;
}
