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

static void test_shared_text_runtime(void) {
    zt_text *value = zt_text_from_utf8_literal("shared");
    zt_shared_text *shared = zt_shared_text_new(value);
    zt_shared_text *alias;
    const zt_text *borrowed;
    zt_text *snapshot;

    assert_true("shared_text_new_ref_count", zt_shared_text_ref_count(shared) == 1);
    assert_true("shared_text_new_retains_value", value->header.rc == 2);

    alias = zt_shared_text_retain(shared);
    assert_true("shared_text_retain_ref_count", zt_shared_text_ref_count(shared) == 2);

    borrowed = zt_shared_text_borrow(shared);
    assert_text_equals("shared_text_borrow_value", borrowed, "shared");

    snapshot = zt_shared_text_snapshot(shared);
    assert_text_equals("shared_text_snapshot_value", snapshot, "shared");
    assert_true("shared_text_snapshot_distinct", snapshot != borrowed);
    assert_true("shared_text_snapshot_rc", snapshot->header.rc == 1);

    zt_release(value);
    assert_text_equals("shared_text_borrow_after_release", zt_shared_text_borrow(shared), "shared");

    zt_release(snapshot);
    zt_shared_text_release(alias);
    assert_true("shared_text_release_ref_count", zt_shared_text_ref_count(shared) == 1);
    zt_shared_text_release(shared);
}

static void test_shared_bytes_runtime(void) {
    static const uint8_t source[] = {0x10u, 0x20u, 0x30u, 0x40u};
    zt_bytes *value = zt_bytes_from_array(source, sizeof(source));
    zt_shared_bytes *shared = zt_shared_bytes_new(value);
    zt_shared_bytes *alias;
    const zt_bytes *borrowed;
    zt_bytes *snapshot;

    assert_true("shared_bytes_new_ref_count", zt_shared_bytes_ref_count(shared) == 1);
    assert_true("shared_bytes_new_retains_value", value->header.rc == 2);

    alias = zt_shared_bytes_retain(shared);
    assert_true("shared_bytes_retain_ref_count", zt_shared_bytes_ref_count(shared) == 2);

    borrowed = zt_shared_bytes_borrow(shared);
    assert_bytes_equals("shared_bytes_borrow_value", borrowed, source, sizeof(source));

    snapshot = zt_shared_bytes_snapshot(shared);
    assert_bytes_equals("shared_bytes_snapshot_value", snapshot, source, sizeof(source));
    assert_true("shared_bytes_snapshot_distinct", snapshot != borrowed);
    assert_true("shared_bytes_snapshot_data_distinct", snapshot->data != borrowed->data);
    assert_true("shared_bytes_snapshot_rc", snapshot->header.rc == 1);

    zt_release(value);
    assert_bytes_equals(
        "shared_bytes_borrow_after_release",
        zt_shared_bytes_borrow(shared),
        source,
        sizeof(source));

    zt_release(snapshot);
    zt_shared_bytes_release(alias);
    assert_true("shared_bytes_release_ref_count", zt_shared_bytes_ref_count(shared) == 1);
    zt_shared_bytes_release(shared);
}

static void test_thread_boundary_deep_copy_runtime(void) {
    static const uint8_t payload[] = {1u, 2u, 3u};
    zt_text *text = zt_text_from_utf8_literal("alpha");
    zt_text *text_copy = zt_thread_boundary_copy_text(text);
    zt_bytes *bytes = zt_bytes_from_array(payload, sizeof(payload));
    zt_bytes *bytes_copy = zt_thread_boundary_copy_bytes(bytes);

    zt_text *first = zt_text_from_utf8_literal("one");
    zt_text *second = zt_text_from_utf8_literal("two");
    zt_text *items[] = {first, second};
    zt_list_text *list = zt_list_text_from_array(items, 2);
    zt_list_text *list_copy = zt_thread_boundary_copy_list_text(list);

    zt_text *orig0;
    zt_text *copy0;

    assert_true("thread_copy_text_distinct", text_copy != text);
    assert_true("thread_copy_text_data_distinct", text_copy->data != text->data);
    assert_text_equals("thread_copy_text_value", text_copy, "alpha");

    assert_true("thread_copy_bytes_distinct", bytes_copy != bytes);
    assert_true("thread_copy_bytes_data_distinct", bytes_copy->data != bytes->data);
    assert_bytes_equals("thread_copy_bytes_value", bytes_copy, payload, sizeof(payload));

    assert_true("thread_copy_list_text_distinct", list_copy != list);
    assert_true("thread_copy_list_text_len", zt_list_text_len(list_copy) == 2);

    orig0 = zt_list_text_get(list, 0);
    copy0 = zt_list_text_get(list_copy, 0);
    assert_true("thread_copy_list_text_item_distinct", copy0 != orig0);
    assert_text_equals("thread_copy_list_text_item_value", copy0, "one");

    zt_release(orig0);
    zt_release(copy0);

    zt_release(text);
    zt_release(text_copy);
    zt_release(bytes);
    zt_release(bytes_copy);
    zt_release(list);
    zt_release(list_copy);
    zt_release(first);
    zt_release(second);
}

static void test_dyn_text_repr_runtime(void) {
    zt_dyn_text_repr *boxed_int = zt_dyn_text_repr_from_i64(7);
    zt_dyn_text_repr *boxed_bool = zt_dyn_text_repr_from_bool(true);
    zt_dyn_text_repr *boxed_float = zt_dyn_text_repr_from_float(2.5);
    zt_dyn_text_repr *boxed_text = zt_dyn_text_repr_from_text_owned(zt_text_from_utf8_literal("ok"));

    zt_text *text_int = zt_dyn_text_repr_to_text(boxed_int);
    zt_text *text_bool = zt_dyn_text_repr_to_text(boxed_bool);
    zt_text *text_float = zt_dyn_text_repr_to_text(boxed_float);
    zt_text *text_text = zt_dyn_text_repr_to_text(boxed_text);

    zt_dyn_text_repr *cloned_text = zt_thread_boundary_copy_dyn_text_repr(boxed_text);
    zt_text *cloned_text_value = zt_dyn_text_repr_to_text(cloned_text);

    zt_dyn_text_repr *owned_items[] = {
        zt_dyn_text_repr_from_i64(1),
        zt_dyn_text_repr_from_bool(true),
        zt_dyn_text_repr_from_float(2.5),
        zt_dyn_text_repr_from_text_owned(zt_text_from_utf8_literal("ok"))
    };
    zt_list_dyn_text_repr *list = zt_list_dyn_text_repr_from_array_owned(owned_items, 4);
    zt_list_dyn_text_repr *list_copy = zt_thread_boundary_copy_list_dyn_text_repr(list);
    zt_dyn_text_repr *orig0;
    zt_dyn_text_repr *copy0;
    zt_text *copy0_text;

    assert_text_equals("dyn_box_int", text_int, "7");
    assert_text_equals("dyn_box_bool", text_bool, "true");
    assert_text_equals("dyn_box_float", text_float, "2.5");
    assert_text_equals("dyn_box_text", text_text, "ok");
    assert_true("dyn_box_len_int", zt_dyn_text_repr_text_len(boxed_int) == 1);
    assert_true("dyn_box_len_text", zt_dyn_text_repr_text_len(boxed_text) == 2);

    assert_true("dyn_clone_distinct", cloned_text != boxed_text);
    assert_text_equals("dyn_clone_value", cloned_text_value, "ok");

    assert_true("dyn_list_len", zt_list_dyn_text_repr_len(list) == 4);
    assert_true("dyn_list_copy_distinct", list_copy != list);
    assert_true("dyn_list_copy_len", zt_list_dyn_text_repr_len(list_copy) == 4);

    orig0 = zt_list_dyn_text_repr_get(list, 0);
    copy0 = zt_list_dyn_text_repr_get(list_copy, 0);
    assert_true("dyn_list_item_distinct", copy0 != orig0);
    copy0_text = zt_dyn_text_repr_to_text(copy0);
    assert_text_equals("dyn_list_item_value", copy0_text, "1");

    zt_release(copy0_text);
    zt_release(orig0);
    zt_release(copy0);

    zt_release(text_int);
    zt_release(text_bool);
    zt_release(text_float);
    zt_release(text_text);
    zt_release(cloned_text_value);

    zt_release(boxed_int);
    zt_release(boxed_bool);
    zt_release(boxed_float);
    zt_release(boxed_text);
    zt_release(cloned_text);
    zt_release(list);
    zt_release(list_copy);
}

int main(void) {
    test_shared_text_runtime();
    test_shared_bytes_runtime();
    test_thread_boundary_deep_copy_runtime();
    test_dyn_text_repr_runtime();
    puts("Runtime shared ref tests OK");
    return 0;
}

