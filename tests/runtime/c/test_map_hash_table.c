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

static zt_text *make_textf(const char *prefix, size_t index) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%s%04zu", prefix, index);
    return zt_text_from_utf8_literal(buffer);
}

static void test_map_large_lookup_and_overwrite(void) {
    enum { COUNT = 2048 };
    zt_map_text_text *map = zt_map_text_text_new();
    size_t index;

    for (index = 0; index < COUNT; index += 1) {
        zt_text *key = make_textf("k", index);
        zt_text *value = make_textf("v", index);
        zt_map_text_text_set(map, key, value);
        zt_release(value);
        zt_release(key);
    }

    assert_true("map_large_len", zt_map_text_text_len(map) == COUNT);

    for (index = 0; index < COUNT; index += 137) {
        zt_text *key = make_textf("k", index);
        zt_text *value = zt_map_text_text_get(map, key);
        zt_text *expected = make_textf("v", index);
        assert_true("map_large_lookup", zt_text_eq(value, expected));
        zt_release(expected);
        zt_release(value);
        zt_release(key);
    }

    {
        zt_text *key = make_textf("k", 420);
        zt_text *updated = zt_text_from_utf8_literal("updated");
        zt_text *value;
        zt_map_text_text_set(map, key, updated);
        value = zt_map_text_text_get(map, key);
        assert_true("map_overwrite_len", zt_map_text_text_len(map) == COUNT);
        assert_true("map_overwrite_value", zt_text_eq(value, updated));
        zt_release(value);
        zt_release(updated);
        zt_release(key);
    }

    {
        zt_text *first_key = zt_map_text_text_key_at(map, 0);
        zt_text *expected_key = make_textf("k", 0);
        assert_true("map_iteration_order_key", zt_text_eq(first_key, expected_key));
        zt_release(expected_key);
        zt_release(first_key);
    }

    zt_release(map);
}

static void test_map_set_owned_keeps_cow_semantics(void) {
    zt_map_text_text *original = zt_map_text_text_new();
    zt_map_text_text *clone;
    zt_text *base_key = zt_text_from_utf8_literal("base");
    zt_text *base_value = zt_text_from_utf8_literal("one");
    zt_text *new_key = zt_text_from_utf8_literal("extra");
    zt_text *new_value = zt_text_from_utf8_literal("two");
    zt_optional_text original_extra;
    zt_optional_text clone_extra;

    zt_map_text_text_set(original, base_key, base_value);
    zt_retain(original);
    clone = zt_map_text_text_set_owned(original, new_key, new_value);

    original_extra = zt_map_text_text_get_optional(original, new_key);
    clone_extra = zt_map_text_text_get_optional(clone, new_key);

    assert_true("map_cow_original_missing", !original_extra.is_present);
    assert_true("map_cow_clone_present", clone_extra.is_present);
    assert_true("map_cow_clone_value", zt_text_eq(clone_extra.value, new_value));
    assert_true("map_cow_original_len", zt_map_text_text_len(original) == 1);
    assert_true("map_cow_clone_len", zt_map_text_text_len(clone) == 2);

    zt_release(clone);
    zt_release(original);
    zt_release(new_value);
    zt_release(new_key);
    zt_release(base_value);
    zt_release(base_key);
}

int main(void) {
    test_map_large_lookup_and_overwrite();
    test_map_set_owned_keeps_cow_semantics();
    printf("Runtime map hash table tests OK\n");
    return 0;
}
